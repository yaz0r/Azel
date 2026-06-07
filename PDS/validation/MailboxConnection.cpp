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

namespace {

constexpr std::uint64_t kCommandTimeoutMs = 180000; // generous: the first RunUntilAddress drives a full game boot

void breakOnError(const char *msg) {
    std::fprintf(stderr, "[validation] %s\n", msg);
    if (IsDebuggerPresent()) {
        __debugbreak();
    }
}

std::uint8_t *mapNamedRegion(const char *name, std::size_t size, void *&outHandle, bool waitForCreation) {
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
    return static_cast<std::uint8_t *>(base);
}

} // namespace

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

azelval::Status MailboxConnection::exec(azelval::Command command, Cpu cpu, std::uint32_t address, std::uint32_t index,
                                        std::uint32_t length, std::uint32_t value) {
    if (m_mailbox == nullptr) {
        return azelval::Status::Error;
    }
    azelval::Mailbox &mb = *m_mailbox;
    mb.command = static_cast<std::uint32_t>(command);
    mb.cpu = static_cast<std::uint32_t>(cpu);
    mb.address = address;
    mb.index = index;
    mb.length = length;
    mb.value = value;

    const std::uint32_t seq = ++m_seq;
    azelval::SeqStoreRelease(mb.requestSeq, seq);

    const std::uint64_t start = GetTickCount64();
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

std::uint8_t *MailboxConnection::mirrorPtr(std::uint32_t address, std::size_t size) {
    const std::uint32_t a = address & 0x07FFFFFFu; // strip cache-through alias bit
    if (a >= azelval::kWramHighBase && a < 0x08000000u) {
        const std::uint32_t off = (a - azelval::kWramHighBase) % azelval::kWramSize;
        if (off + size <= azelval::kWramSize) {
            return m_wramHi + off;
        }
        return nullptr; // straddles the 1 MiB mirror boundary; fall back to Peek
    }
    if (a >= azelval::kWramLowBase && a < azelval::kWramLowBase + azelval::kWramSize) {
        const std::uint32_t off = a - azelval::kWramLowBase;
        if (off + size <= azelval::kWramSize) {
            return m_wramLo + off;
        }
    }
    return nullptr;
}

void MailboxConnection::readMemory(std::uint32_t address, void *buffer, std::size_t size) {
    if (std::uint8_t *m = mirrorPtr(address, size)) {
        std::memcpy(buffer, m, size);
        return;
    }
    // Not WRAM-backed (or straddling): read through the bus via Peek, chunked to the payload size.
    auto *out = static_cast<std::uint8_t *>(buffer);
    std::uint32_t a = address;
    std::size_t remaining = size;
    while (remaining > 0) {
        const std::uint32_t chunk =
            static_cast<std::uint32_t>(remaining < azelval::kPayloadSize ? remaining : azelval::kPayloadSize);
        exec(azelval::Command::Peek, Cpu::Master, a, 0, chunk, 0);
        std::memcpy(out, m_mailbox->payload, chunk);
        out += chunk;
        a += chunk;
        remaining -= chunk;
    }
}

void MailboxConnection::writeMemory(std::uint32_t address, const void *buffer, std::size_t size) {
    const auto *in = static_cast<const std::uint8_t *>(buffer);
    std::uint32_t a = address;
    std::size_t remaining = size;
    while (remaining > 0) {
        const std::uint32_t chunk =
            static_cast<std::uint32_t>(remaining < azelval::kPayloadSize ? remaining : azelval::kPayloadSize);
        std::memcpy(m_mailbox->payload, in, chunk);
        exec(azelval::Command::Poke, Cpu::Master, a, 0, chunk, 0);
        in += chunk;
        a += chunk;
        remaining -= chunk;
    }
}

std::uint8_t MailboxConnection::readU8(std::uint32_t address) {
    std::uint8_t b = 0;
    readMemory(address, &b, 1);
    return b;
}
std::int8_t MailboxConnection::readS8(std::uint32_t address) {
    return static_cast<std::int8_t>(readU8(address));
}
std::uint16_t MailboxConnection::readU16(std::uint32_t address) {
    std::uint8_t b[2] = {0, 0};
    readMemory(address, b, 2);
    return static_cast<std::uint16_t>((b[0] << 8) | b[1]); // big-endian guest
}
std::int16_t MailboxConnection::readS16(std::uint32_t address) {
    return static_cast<std::int16_t>(readU16(address));
}
std::uint32_t MailboxConnection::readU32(std::uint32_t address) {
    std::uint8_t b[4] = {0, 0, 0, 0};
    readMemory(address, b, 4);
    return (static_cast<std::uint32_t>(b[0]) << 24) | (static_cast<std::uint32_t>(b[1]) << 16) |
           (static_cast<std::uint32_t>(b[2]) << 8) | static_cast<std::uint32_t>(b[3]);
}
std::int32_t MailboxConnection::readS32(std::uint32_t address) {
    return static_cast<std::int32_t>(readU32(address));
}

void MailboxConnection::writeU8(std::uint32_t address, std::uint8_t value) {
    writeMemory(address, &value, 1);
}
void MailboxConnection::writeU16(std::uint32_t address, std::uint16_t value) {
    std::uint8_t b[2] = {static_cast<std::uint8_t>(value >> 8), static_cast<std::uint8_t>(value)};
    writeMemory(address, b, 2);
}
void MailboxConnection::writeU32(std::uint32_t address, std::uint32_t value) {
    std::uint8_t b[4] = {static_cast<std::uint8_t>(value >> 24), static_cast<std::uint8_t>(value >> 16),
                         static_cast<std::uint8_t>(value >> 8), static_cast<std::uint8_t>(value)};
    writeMemory(address, b, 4);
}

void MailboxConnection::fillRange(std::uint32_t address, std::uint32_t length, std::uint8_t value, Cpu cpu) {
    exec(azelval::Command::FillRange, cpu, address, 0, length, value);
}

void MailboxConnection::mirrorWram() {
    exec(azelval::Command::MirrorWram, Cpu::Master, 0, 0, 0, 0);
}

std::uint32_t MailboxConnection::getRegister(std::uint32_t index, Cpu cpu) {
    exec(azelval::Command::GetReg, cpu, 0, index, 0, 0);
    return m_mailbox->regValue;
}
void MailboxConnection::setRegister(std::uint32_t index, std::uint32_t value, Cpu cpu) {
    exec(azelval::Command::SetReg, cpu, 0, index, 0, value);
}
void MailboxConnection::getAllRegisters(std::uint32_t out[azelval::REG_COUNT], Cpu cpu) {
    exec(azelval::Command::GetAllRegs, cpu, 0, 0, 0, 0);
    std::memcpy(out, m_mailbox->regs, azelval::REG_COUNT * sizeof(std::uint32_t));
}

void MailboxConnection::setBreakpoint(std::uint32_t address, Cpu cpu) {
    exec(azelval::Command::SetBreakpoint, cpu, address, 0, 0, 0);
}
void MailboxConnection::removeBreakpoint(std::uint32_t address, Cpu cpu) {
    exec(azelval::Command::RemoveBreakpoint, cpu, address, 0, 0, 0);
}

azelval::Status MailboxConnection::executeUntilAddress(std::uint32_t address, Cpu cpu) {
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

std::uint64_t MailboxConnection::stepInstruction(Cpu cpu) {
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
