#pragma once

// This file needs to stay in sync with Ymir's copy

#include <atomic>
#include <cstddef>
#include <cstdint>

namespace azelval {

using u8 = std::uint8_t;
using u32 = std::uint32_t;

// Protocol identity

inline constexpr u32 kMailboxMagic = 0x415A4D42u;   // 'AZMB' little-endian
inline constexpr u32 kProtocolVersion = 1u;

// Shared-memory region names

inline constexpr char kMailboxName[]  = "Azel.Validation.Mailbox";
inline constexpr char kWramLowName[]  = "Azel.Validation.WramLo";
inline constexpr char kWramHighName[] = "Azel.Validation.WramHi";

// Saturn work RAM geometry

inline constexpr u32 kWramLowBase  = 0x00200000u;   // 1 MiB low work RAM (slow)
inline constexpr u32 kWramHighBase = 0x06000000u;   // 1 MiB high work RAM (fast)
inline constexpr u32 kWramSize     = 0x00100000u;   // 1 MiB each

// CPU selector

enum class Cpu : u32 {
    Master = 0,
    Slave  = 1,
};

// Commands (client -> server)

enum class Command : u32 {
    None = 0,
    Reset,            // hard reset of the Saturn
    RunUntilAddress,  // cpu, address -> run until a breakpoint at address fires (or timeout)
    StepInstruction,  // cpu -> single-step one instruction
    GetReg,           // cpu, index -> regValue
    SetReg,           // cpu, index, value
    GetAllRegs,       // cpu -> regs[]
    SetBreakpoint,    // cpu, address
    RemoveBreakpoint, // cpu, address
    Peek,             // address, length(<=kPayloadSize) -> payload (raw big-endian WRAM/bus bytes)
    Poke,             // address, length(<=kPayloadSize), payload -> writes through the bus (bypass cache)
    FillRange,        // address, length, value(low byte) -> fills a range with a byte (stack-zero, patches)
    MirrorWram,       // refresh both WRAM mirror regions from emulator state
    Detach,           // client is going away; server returns to normal frame execution
};

// Response status (server -> client)

enum class Status : u32 {
    Ok = 0,
    BreakpointHit,    // RunUntilAddress reached the requested address
    Timeout,          // RunUntilAddress exhausted its frame budget without hitting the address
    WrongCpu,         // a breakpoint fired, but on the other CPU (should keep running; reported for diagnostics)
    Error,            // malformed command / out-of-range / unsupported
};

// SH-2 register file indices; also the order of the GetAllRegs regs[] array

enum RegIndex : u32 {
    REG_R0 = 0,   // ... through REG_R0 + 15
    REG_R15 = 15, // stack pointer
    REG_PC = 16,
    REG_PR = 17,
    REG_SR = 18,
    REG_GBR = 19,
    REG_VBR = 20,
    REG_MACH = 21,
    REG_MACL = 22,
    REG_COUNT = 23,
};

// Mailbox layout

inline constexpr u32 kPayloadSize = 4096u;

// POD only: no std::atomic members, so no constructor ever runs over the mapped memory.
// The sequence fields are plain u32, accessed via std::atomic_ref in the helpers below.
struct Mailbox {
    // Identity / liveness (written by server at creation; pids exchanged at attach)
    u32 magic;        // == kMailboxMagic once the server has initialised the region
    u32 version;      // == kProtocolVersion
    u32 serverPid;    // Ymir process id (0 until set)
    u32 clientPid;    // Azel process id (0 until set)

    // Handshake sequence counters (release/acquire)
    u32 requestSeq;   // bumped by client after a request is fully written
    u32 responseSeq;  // bumped by server after the response is fully written (== requestSeq when done)

    // Request payload (written by client, read by server)
    u32 command;      // Command
    u32 cpu;          // Cpu
    u32 address;      // target address
    u32 index;        // register index (GetReg/SetReg)
    u32 length;       // byte length (Peek/Poke/FillRange)
    u32 value;        // SetReg value / FillRange fill byte (low 8 bits)

    // Response payload (written by server, read by client)
    u32 status;       // Status
    u32 stoppedPc;    // PC where execution stopped (RunUntilAddress / StepInstruction)
    u32 stoppedMaster;// 1 if the stop was on the master SH-2, 0 if slave
    u32 regValue;     // GetReg result
    u32 regs[REG_COUNT]; // GetAllRegs result

    // Shared bulk payload (Peek result / Poke source)
    u8  payload[kPayloadSize];
};

// Atomic access helpers (release/acquire over the mapped uint32 sequence fields)

static_assert(std::atomic_ref<u32>::is_always_lock_free,
              "validation mailbox requires lock-free 32-bit atomics");

inline u32 SeqLoadAcquire(const u32 &field) {
    return std::atomic_ref<u32>(const_cast<u32 &>(field)).load(std::memory_order_acquire);
}

inline void SeqStoreRelease(u32 &field, u32 value) {
    std::atomic_ref<u32>(field).store(value, std::memory_order_release);
}

// Layout sanity checks (catch accidental drift between the two copies)

static_assert(offsetof(Mailbox, magic) == 0);
static_assert(offsetof(Mailbox, requestSeq) == 16);
static_assert(offsetof(Mailbox, responseSeq) == 20);
static_assert(offsetof(Mailbox, command) == 24);
static_assert(offsetof(Mailbox, status) == 48);
static_assert(offsetof(Mailbox, regs) == 64);
static_assert(offsetof(Mailbox, payload) == 64 + REG_COUNT * 4);
static_assert(sizeof(Mailbox) == 64 + REG_COUNT * 4 + kPayloadSize);

} // namespace azelval
