#include "PDS.h"

#include "validation/validation.h"
#include "validation/validationHooks_math.h"

constexpr u32 kFPPow2Entry = 0x060359a4;
constexpr u32 kSqrtFEntry = 0x060359ae;

DECLARE_HOOK(FP_Pow2, kFPPow2Entry, fixedPoint, fixedPoint)

fixedPoint FP_Pow2_detour(fixedPoint r4) {
    if (g_validationConnection == nullptr || !isValidationContextEnabled(VCTX_Math)) {
        return FP_Pow2_intercept.callUndetoured(r4);
    }

    g_validationConnection->executeUntilAddress(kFPPow2Entry);
    validateRegister(azelval::REG_R0 + 4, (u32)r4.asS32());
    const u32 returnAddr = g_validationConnection->getRegister(azelval::REG_PR);

    const fixedPoint result = FP_Pow2_intercept.callUndetoured(r4);

    g_validationConnection->executeUntilAddress(returnAddr);
    validateRegister(azelval::REG_R0 + 0, (u32)result.asS32());
    return result;
}

DECLARE_HOOK(sqrt_F, kSqrtFEntry, fixedPoint, fixedPoint)

fixedPoint sqrt_F_detour(fixedPoint r4) {
    if (g_validationConnection == nullptr || !isValidationContextEnabled(VCTX_Math)) {
        return sqrt_F_intercept.callUndetoured(r4);
    }

    g_validationConnection->executeUntilAddress(kSqrtFEntry);
    validateRegister(azelval::REG_R0 + 4, (u32)r4.asS32());
    const u32 returnAddr = g_validationConnection->getRegister(azelval::REG_PR);

    const fixedPoint result = sqrt_F_intercept.callUndetoured(r4);

    g_validationConnection->executeUntilAddress(returnAddr);
    validateRegister(azelval::REG_R0 + 0, (u32)result.asS32());
    return result;
}

void enableMathHooks() {
    FP_Pow2_intercept.enable();
    sqrt_F_intercept.enable();
}
