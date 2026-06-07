#pragma once

// =====================================================================================================================
// Azel <-> Ymir validation mailbox -- SHARED CONTRACT
//
// This header defines the byte-for-byte layout of the shared-memory control block and the well-known names of the
// three shared-memory regions used by the side-by-side validation system.
//
// IMPORTANT: This file MUST be kept identical between the Ymir side
//   (apps/ymir-sdl3/src/app/validation/validation_mailbox.hpp)
// and the Azel side
//   (PDS/validation/validation_mailbox.hpp).
// It is intentionally dependency-free (only <cstdint>/<atomic>/<cstddef>) so the layout is determined purely by the
// platform ABI (x64 Windows) and is therefore identical regardless of which compiler builds each side.
//
// Ymir creates the regions (server) and zero-initialises the mailbox; Azel opens them (client). Synchronisation is a
// single-producer/single-consumer release/acquire handshake on the plain uint32 sequence fields, accessed through
// std::atomic_ref (so we never run an std::atomic constructor over externally-mapped memory).
// =====================================================================================================================

#include <atomic>
#include <cstddef>
#include <cstdint>

namespace azelval {

// --- Protocol identity -----------------------------------------------------------------------------------------------

inline constexpr std::uint32_t kMailboxMagic = 0x415A4D42u;   // 'AZMB' little-endian
inline constexpr std::uint32_t kProtocolVersion = 1u;

// --- Shared-memory region names --------------------------------------------------------------------------------------

inline constexpr char kMailboxName[]  = "Azel.Validation.Mailbox";
inline constexpr char kWramLowName[]  = "Azel.Validation.WramLo";
inline constexpr char kWramHighName[] = "Azel.Validation.WramHi";

// --- Saturn work RAM geometry ----------------------------------------------------------------------------------------

inline constexpr std::uint32_t kWramLowBase  = 0x00200000u;   // 1 MiB low work RAM (slow)
inline constexpr std::uint32_t kWramHighBase = 0x06000000u;   // 1 MiB high work RAM (fast)
inline constexpr std::uint32_t kWramSize     = 0x00100000u;   // 1 MiB each

// --- CPU selector ----------------------------------------------------------------------------------------------------

enum class Cpu : std::uint32_t {
    Master = 0,
    Slave  = 1,
};

// --- Commands (client -> server) -------------------------------------------------------------------------------------

enum class Command : std::uint32_t {
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

// --- Response status (server -> client) ------------------------------------------------------------------------------

enum class Status : std::uint32_t {
    Ok = 0,
    BreakpointHit,    // RunUntilAddress reached the requested address
    Timeout,          // RunUntilAddress exhausted its frame budget without hitting the address
    WrongCpu,         // a breakpoint fired, but on the other CPU (should keep running; reported for diagnostics)
    Error,            // malformed command / out-of-range / unsupported
};

// --- SH-2 register file indices --------------------------------------------------------------------------------------
// R0..R15 occupy indices 0..15. R15 is the stack pointer. R4..R7 carry the first integer arguments; R0 holds the
// integer return value. These indices are also the order of the GetAllRegs `regs[]` array.

enum RegIndex : std::uint32_t {
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

// --- Mailbox layout --------------------------------------------------------------------------------------------------

inline constexpr std::uint32_t kPayloadSize = 4096u;

// POD only: fixed-width integers and a byte array. No std::atomic members -- the sequence fields are plain uint32 and
// are accessed atomically via std::atomic_ref through the helpers below. This guarantees an identical layout on both
// sides with no constructor ever running over the mapped memory.
struct Mailbox {
    // -- Identity / liveness (written by server at creation; pids exchanged at attach) --
    std::uint32_t magic;        // == kMailboxMagic once the server has initialised the region
    std::uint32_t version;      // == kProtocolVersion
    std::uint32_t serverPid;    // Ymir process id (0 until set)
    std::uint32_t clientPid;    // Azel process id (0 until set)

    // -- Handshake sequence counters (release/acquire) --
    std::uint32_t requestSeq;   // bumped by client after a request is fully written
    std::uint32_t responseSeq;  // bumped by server after the response is fully written (== requestSeq when done)

    // -- Request payload (written by client, read by server) --
    std::uint32_t command;      // Command
    std::uint32_t cpu;          // Cpu
    std::uint32_t address;      // target address
    std::uint32_t index;        // register index (GetReg/SetReg)
    std::uint32_t length;       // byte length (Peek/Poke/FillRange)
    std::uint32_t value;        // SetReg value / FillRange fill byte (low 8 bits)

    // -- Response payload (written by server, read by client) --
    std::uint32_t status;       // Status
    std::uint32_t stoppedPc;    // PC where execution stopped (RunUntilAddress / StepInstruction)
    std::uint32_t stoppedMaster;// 1 if the stop was on the master SH-2, 0 if slave
    std::uint32_t regValue;     // GetReg result
    std::uint32_t regs[REG_COUNT]; // GetAllRegs result

    // -- Shared bulk payload (Peek result / Poke source) --
    std::uint8_t  payload[kPayloadSize];
};

// --- Atomic access helpers (release/acquire over the mapped uint32 sequence fields) ----------------------------------

static_assert(std::atomic_ref<std::uint32_t>::is_always_lock_free,
              "validation mailbox requires lock-free 32-bit atomics");

inline std::uint32_t SeqLoadAcquire(const std::uint32_t &field) {
    return std::atomic_ref<std::uint32_t>(const_cast<std::uint32_t &>(field)).load(std::memory_order_acquire);
}

inline void SeqStoreRelease(std::uint32_t &field, std::uint32_t value) {
    std::atomic_ref<std::uint32_t>(field).store(value, std::memory_order_release);
}

// --- Layout sanity checks (catch accidental drift between the two copies) ---------------------------------------------

static_assert(offsetof(Mailbox, magic) == 0);
static_assert(offsetof(Mailbox, requestSeq) == 16);
static_assert(offsetof(Mailbox, responseSeq) == 20);
static_assert(offsetof(Mailbox, command) == 24);
static_assert(offsetof(Mailbox, status) == 48);
static_assert(offsetof(Mailbox, regs) == 64);
static_assert(offsetof(Mailbox, payload) == 64 + REG_COUNT * 4);
static_assert(sizeof(Mailbox) == 64 + REG_COUNT * 4 + kPayloadSize);

} // namespace azelval
