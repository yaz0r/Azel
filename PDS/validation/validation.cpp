#include "PDS.h"

#include "validation.h"

#include <cstdint>
#include <cstdio>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

// The mailbox is Windows-only; elsewhere validation is a no-op
#ifdef _WIN32
bool enableValidation = true;
#else
bool enableValidation = false;
#endif

static MailboxConnection g_connectionInstance;
static u64 g_contextMask = 0;

void validateAssert(bool result) {
#ifdef _WIN32
    if (!result) {
        static bool bBreakOnValidationError = true;
        if (IsDebuggerPresent() && bBreakOnValidationError) {
            __debugbreak();
            bBreakOnValidationError = false;
        }
    }
#else
    (void)result;
#endif
}

bool isValidationContextEnabled(validationContext context) {
    return (g_contextMask & (1ull << static_cast<int>(context))) != 0;
}
void enableValidationContext(validationContext context) {
    g_contextMask |= (1ull << static_cast<int>(context));
}
void disableValidationContext(validationContext context) {
    g_contextMask &= ~(1ull << static_cast<int>(context));
}

// Typed comparison helpers

// `what` labels the source: memory address or register
static void compareAndReport(const char *what, u32 where, s64 emu, s64 pds) {
    if (emu != pds) {
        std::printf("[validation] MISMATCH %s %08X: PDS=%lld (0x%llX)  emu=%lld (0x%llX)  delta(PDS-emu)=%lld\n", what,
                    where, (long long)pds, (unsigned long long)(u64)pds, (long long)emu,
                    (unsigned long long)(u64)emu, (long long)(pds - emu));
    }
    validateAssert(emu == pds);
}

void validate(u32 base, const u8 &value) {
    compareAndReport("@", base, g_validationConnection->readU8(base), value);
}
void validate(u32 base, const s8 &value) {
    compareAndReport("@", base, g_validationConnection->readS8(base), value);
}
void validate(u32 base, const u16 &value) {
    compareAndReport("@", base, g_validationConnection->readU16(base), value);
}
void validate(u32 base, const s16 &value) {
    compareAndReport("@", base, g_validationConnection->readS16(base), value);
}
void validate(u32 base, const u32 &value) {
    compareAndReport("@", base, g_validationConnection->readU32(base), value);
}
void validate(u32 base, const s32 &value) {
    compareAndReport("@", base, g_validationConnection->readS32(base), value);
}

void validate(u32 base, const fixedPoint &value) {
    compareAndReport("@", base, g_validationConnection->readS32(base), value.asS32());
}
void validate(u32 base, const sVec3_FP &value) {
    validate(base + 0, value[0]);
    validate(base + 4, value[1]);
    validate(base + 8, value[2]);
}
void validate(u32 base, const sMatrix4x3 &value) {
    // Row-major 3x4 fixedPoint matrix; same element order as the legacy addTraceLog(sMatrix4x3) dump.
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 4; ++col) {
            validate(base + (row * 4 + col) * 4, value.m[row][col]);
        }
    }
}

void validateRegister(u32 regIndex, u32 value) {
    compareAndReport("reg", regIndex, g_validationConnection->getRegister(regIndex), value);
}

// Bootstrap

void validationInit() {
    if (!enableValidation) {
        return;
    }
    if (!g_connectionInstance.open()) {
        std::printf("[validation] disabled (no Ymir mailbox -- run Ymir with --validation)\n");
        return;
    }
    g_validationConnection = &g_connectionInstance;

    enableValidationContext(VCTX_Base);

    u32 regs[azelval::REG_COUNT] = {};
    g_validationConnection->getAllRegisters(regs);
    std::printf("[validation] connected to Ymir. MSH2 PC=%08X PR=%08X SR=%08X R15(SP)=%08X\n", regs[azelval::REG_PC],
                regs[azelval::REG_PR], regs[azelval::REG_SR], regs[azelval::REG_R15]);

    {
        const u32 scratch = azelval::kWramHighBase + 0x10000; // 0x06010000
        const u32 marker = 0x12345678u;
        g_validationConnection->writeU32(scratch, marker); // Poke (server mirrors WRAM afterwards)

        const u32 got = g_validationConnection->readU32(scratch);
        u8 raw[4] = {};
        g_validationConnection->readMemory(scratch, raw, 4);
        const bool bigEndian = (raw[0] == 0x12 && raw[1] == 0x34 && raw[2] == 0x56 && raw[3] == 0x78);

        std::printf("[validation] RAM round-trip @ %08X: wrote %08X, read %08X, raw=%02X %02X %02X %02X -> %s\n",
                    scratch, marker, got, raw[0], raw[1], raw[2], raw[3],
                    (got == marker && bigEndian) ? "PASS" : "FAIL");
        validateAssert(got == marker && bigEndian);
    }

    g_validationConnection->resetTarget();

    enableValidationContext(VCTX_Town);
    enableValidationHooks();
    std::printf("[validation] enabled\n");
}

void validationShutdown() {
    if (g_validationConnection != nullptr) {
        g_connectionInstance.close();
        g_validationConnection = nullptr;
    }
}
