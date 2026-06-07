#include "MailboxConnection.h"

#include <cstdio>
#include <cstring>

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <intrin.h>

MailboxConnection *g_validationConnection = nullptr;

// Generous: the first RunUntilAddress drives a full game boot
static constexpr u64 kCommandTimeoutMs = 180000;

static void breakOnError(const char *msg) {
    std::fprintf(stderr, "[validation] %s\n", msg);
    if (IsDebuggerPresent()) {
        __debugbreak();
    }
}

static u8 *mapNamedRegion(const char *name, std::size_t size, void *&outHandle, bool waitForCreation) {
    HANDLE handle = nullptr;
    const int kAttempts = waitForCreation ? 1000 : 1; // up to ~10s waiting for the server to create the region
    for (int i = 0; i < kAttempts && handle == nullptr; ++i) {
        handle = OpenFileMappingA(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, name);
        if (handle == nullptr && waitForCreation) {
            Sleep(10);
        }
    }
    if (handle == nullptr) {
        outHandle = nullptr;
        return nullptr;
    }
    void *base = MapViewOfFile(handle, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, size);
    if (base == nullptr) {
        CloseHandle(handle);
        outHandle = nullptr;
        return nullptr;
    }
    outHandle = handle;
    return static_cast<u8 *>(base);
}

MailboxConnection::~MailboxConnection() {
    close();
}

bool MailboxConnection::open() {
    auto *mailboxBytes = mapNamedRegion(azelval::kMailboxName, sizeof(azelval::Mailbox), m_mailboxHandle, true);
    if (mailboxBytes == nullptr) {
        breakOnError("could not open validation mailbox (is Ymir running with --validation?)");
        return false;
    }
    m_mailbox = reinterpret_cast<azelval::Mailbox *>(mailboxBytes);

    // Wait for the server to publish its magic, then validate the protocol version.
    bool ready = false;
    for (int i = 0; i < 1000; ++i) {
        if (azelval::SeqLoadAcquire(m_mailbox->magic) == azelval::kMailboxMagic) {
            ready = true;
            break;
        }
        Sleep(10);
    }
    if (!ready) {
        breakOnError("validation mailbox never became ready");
        close();
        return false;
    }
    if (m_mailbox->version != azelval::kProtocolVersion) {
        breakOnError("validation protocol version mismatch");
        close();
        return false;
    }

    m_wramLo = mapNamedRegion(azelval::kWramLowName, azelval::kWramSize, m_wramLoHandle, true);
    m_wramHi = mapNamedRegion(azelval::kWramHighName, azelval::kWramSize, m_wramHiHandle, true);
    if (m_wramLo == nullptr || m_wramHi == nullptr) {
        breakOnError("could not map validation WRAM mirror regions");
        close();
        return false;
    }

    // Publish our PID and open a handle to the server for liveness checks.
    m_mailbox->clientPid = GetCurrentProcessId();
    m_seq = azelval::SeqLoadAcquire(m_mailbox->responseSeq);
    if (m_mailbox->serverPid != 0) {
        m_serverProcess = OpenProcess(SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, m_mailbox->serverPid);
    }
    return true;
}

void MailboxConnection::close() {
    if (m_mailbox != nullptr) {
        UnmapViewOfFile(m_mailbox);
        m_mailbox = nullptr;
    }
    if (m_mailboxHandle != nullptr) {
        CloseHandle(m_mailboxHandle);
        m_mailboxHandle = nullptr;
    }
    if (m_wramLo != nullptr) {
        UnmapViewOfFile(m_wramLo);
        m_wramLo = nullptr;
    }
    if (m_wramLoHandle != nullptr) {
        CloseHandle(m_wramLoHandle);
        m_wramLoHandle = nullptr;
    }
    if (m_wramHi != nullptr) {
        UnmapViewOfFile(m_wramHi);
        m_wramHi = nullptr;
    }
    if (m_wramHiHandle != nullptr) {
        CloseHandle(m_wramHiHandle);
        m_wramHiHandle = nullptr;
    }
    if (m_serverProcess != nullptr) {
        CloseHandle(m_serverProcess);
        m_serverProcess = nullptr;
    }
}

bool MailboxConnection::serverAlive() const {
    if (m_serverProcess == nullptr) {
        return true; // unknown; assume alive
    }
    DWORD code = 0;
    if (GetExitCodeProcess(m_serverProcess, &code)) {
        return code == STILL_ACTIVE;
    }
    return true;
}

azelval::Status MailboxConnection::exec(azelval::Command command, Cpu cpu, u32 address, u32 index,
                                        u32 length, u32 value) {
    if (m_mailbox == nullptr) {
        return azelval::Status::Error;
    }
    azelval::Mailbox &mb = *m_mailbox;
    mb.command = static_cast<u32>(command);
    mb.cpu = static_cast<u32>(cpu);
    mb.address = address;
    mb.index = index;
    mb.length = length;
    mb.value = value;

    const u32 seq = ++m_seq;
    azelval::SeqStoreRelease(mb.requestSeq, seq);

    const u64 start = GetTickCount64();
    int spins = 0;
    while (azelval::SeqLoadAcquire(mb.responseSeq) != seq) {
        if (++spins < 4000) {
            _mm_pause();
        } else {
            SwitchToThread();
            if (!serverAlive()) {
                breakOnError("Ymir validation server vanished while waiting for a response");
                return azelval::Status::Error;
            }
            if (GetTickCount64() - start > kCommandTimeoutMs) {
                breakOnError("validation command timed out");
                return azelval::Status::Error;
            }
        }
    }

    m_lastStoppedPc = mb.stoppedPc;
    return static_cast<azelval::Status>(mb.status);
}

u8 *MailboxConnection::mirrorPtr(u32 address, std::size_t size) {
    const u32 a = address & 0x07FFFFFFu; // strip cache-through alias bit
    if (a >= azelval::kWramHighBase && a < 0x08000000u) {
        const u32 off = (a - azelval::kWramHighBase) % azelval::kWramSize;
        if (off + size <= azelval::kWramSize) {
            return m_wramHi + off;
        }
        return nullptr; // straddles the 1 MiB mirror boundary; fall back to Peek
    }
    if (a >= azelval::kWramLowBase && a < azelval::kWramLowBase + azelval::kWramSize) {
        const u32 off = a - azelval::kWramLowBase;
        if (off + size <= azelval::kWramSize) {
            return m_wramLo + off;
        }
    }
    return nullptr;
}

void MailboxConnection::readMemory(u32 address, void *buffer, std::size_t size) {
    if (u8 *m = mirrorPtr(address, size)) {
        std::memcpy(buffer, m, size);
        return;
    }
    // Not WRAM-backed (or straddling): read through the bus via Peek, chunked to the payload size.
    auto *out = static_cast<u8 *>(buffer);
    u32 a = address;
    std::size_t remaining = size;
    while (remaining > 0) {
        const u32 chunk =
            static_cast<u32>(remaining < azelval::kPayloadSize ? remaining : azelval::kPayloadSize);
        exec(azelval::Command::Peek, Cpu::Master, a, 0, chunk, 0);
        std::memcpy(out, m_mailbox->payload, chunk);
        out += chunk;
        a += chunk;
        remaining -= chunk;
    }
}

void MailboxConnection::writeMemory(u32 address, const void *buffer, std::size_t size) {
    const auto *in = static_cast<const u8 *>(buffer);
    u32 a = address;
    std::size_t remaining = size;
    while (remaining > 0) {
        const u32 chunk =
            static_cast<u32>(remaining < azelval::kPayloadSize ? remaining : azelval::kPayloadSize);
        std::memcpy(m_mailbox->payload, in, chunk);
        exec(azelval::Command::Poke, Cpu::Master, a, 0, chunk, 0);
        in += chunk;
        a += chunk;
        remaining -= chunk;
    }
}

u8 MailboxConnection::readU8(u32 address) {
    u8 b = 0;
    readMemory(address, &b, 1);
    return b;
}
s8 MailboxConnection::readS8(u32 address) {
    return static_cast<s8>(readU8(address));
}
u16 MailboxConnection::readU16(u32 address) {
    u8 b[2] = {0, 0};
    readMemory(address, b, 2);
    return static_cast<u16>((b[0] << 8) | b[1]); // big-endian guest
}
s16 MailboxConnection::readS16(u32 address) {
    return static_cast<s16>(readU16(address));
}
u32 MailboxConnection::readU32(u32 address) {
    u8 b[4] = {0, 0, 0, 0};
    readMemory(address, b, 4);
    return (static_cast<u32>(b[0]) << 24) | (static_cast<u32>(b[1]) << 16) |
           (static_cast<u32>(b[2]) << 8) | static_cast<u32>(b[3]);
}
s32 MailboxConnection::readS32(u32 address) {
    return static_cast<s32>(readU32(address));
}

void MailboxConnection::writeU8(u32 address, u8 value) {
    writeMemory(address, &value, 1);
}
void MailboxConnection::writeU16(u32 address, u16 value) {
    u8 b[2] = {static_cast<u8>(value >> 8), static_cast<u8>(value)};
    writeMemory(address, b, 2);
}
void MailboxConnection::writeU32(u32 address, u32 value) {
    u8 b[4] = {static_cast<u8>(value >> 24), static_cast<u8>(value >> 16),
                         static_cast<u8>(value >> 8), static_cast<u8>(value)};
    writeMemory(address, b, 4);
}

void MailboxConnection::fillRange(u32 address, u32 length, u8 value, Cpu cpu) {
    exec(azelval::Command::FillRange, cpu, address, 0, length, value);
}

void MailboxConnection::mirrorWram() {
    exec(azelval::Command::MirrorWram, Cpu::Master, 0, 0, 0, 0);
}

u32 MailboxConnection::getRegister(u32 index, Cpu cpu) {
    exec(azelval::Command::GetReg, cpu, 0, index, 0, 0);
    return m_mailbox->regValue;
}
void MailboxConnection::setRegister(u32 index, u32 value, Cpu cpu) {
    exec(azelval::Command::SetReg, cpu, 0, index, 0, value);
}
void MailboxConnection::getAllRegisters(u32 out[azelval::REG_COUNT], Cpu cpu) {
    exec(azelval::Command::GetAllRegs, cpu, 0, 0, 0, 0);
    std::memcpy(out, m_mailbox->regs, azelval::REG_COUNT * sizeof(u32));
}

void MailboxConnection::setBreakpoint(u32 address, Cpu cpu) {
    exec(azelval::Command::SetBreakpoint, cpu, address, 0, 0, 0);
}
void MailboxConnection::removeBreakpoint(u32 address, Cpu cpu) {
    exec(azelval::Command::RemoveBreakpoint, cpu, address, 0, 0, 0);
}

azelval::Status MailboxConnection::executeUntilAddress(u32 address, Cpu cpu) {
    const azelval::Status st = exec(azelval::Command::RunUntilAddress, cpu, address, 0, 0, 0);
    char msg[192];
    if (st == azelval::Status::BreakpointHit) {
        if (m_lastStoppedPc != address) {
            std::snprintf(msg, sizeof(msg),
                          "executeUntilAddress: expected to stop at %08X but stopped at %08X "
                          "(guest reached a hooked address out of order)",
                          address, m_lastStoppedPc);
            breakOnError(msg);
        }
    } else {
        std::snprintf(msg, sizeof(msg), "executeUntilAddress: never reached %08X (status %u, last PC %08X)", address,
                      static_cast<unsigned>(st), m_lastStoppedPc);
        breakOnError(msg);
    }
    return st;
}

u64 MailboxConnection::stepInstruction(Cpu cpu) {
    exec(azelval::Command::StepInstruction, cpu, 0, 0, 0, 0);
    return 0;
}

void MailboxConnection::resetTarget() {
    exec(azelval::Command::Reset, Cpu::Master, 0, 0, 0, 0);
}

#else // !_WIN32

MailboxConnection *g_validationConnection = nullptr;
MailboxConnection::~MailboxConnection() {}
bool MailboxConnection::open() {
    return false;
}
void MailboxConnection::close() {}

#endif
