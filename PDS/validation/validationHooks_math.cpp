#include "PDS.h"

#include "validation/validation.h"
#include "validation/validationHooks_math.h"

namespace {
constexpr std::uint32_t kFPPow2Entry = 0x060359a4;
constexpr std::uint32_t kSqrtFEntry = 0x060359ae;
constexpr std::uint32_t kProduct3dEntry = 0x0603570e;
} // namespace

// 060359a4
DECLARE_HOOK(FP_Pow2, kFPPow2Entry, fixedPoint, fixedPoint)

fixedPoint FP_Pow2_detour(fixedPoint r4) {
    if (g_validationConnection == nullptr || !isValidationContextEnabled(VCTX_Math)) {
        return FP_Pow2_intercept.callUndetoured(r4);
    }

    g_validationConnection->executeUntilAddress(kFPPow2Entry);
    validateRegister(azelval::REG_R0 + 4, (std::uint32_t)r4.asS32());
    const std::uint32_t returnAddr = g_validationConnection->getRegister(azelval::REG_PR);

    const fixedPoint result = FP_Pow2_intercept.callUndetoured(r4);

    g_validationConnection->executeUntilAddress(returnAddr);
    validateRegister(azelval::REG_R0 + 0, (std::uint32_t)result.asS32());
    return result;
}

// 060359ae
DECLARE_HOOK(sqrt_F, kSqrtFEntry, fixedPoint, fixedPoint)

fixedPoint sqrt_F_detour(fixedPoint r4) {
    if (g_validationConnection == nullptr || !isValidationContextEnabled(VCTX_Math)) {
        return sqrt_F_intercept.callUndetoured(r4);
    }

    g_validationConnection->executeUntilAddress(kSqrtFEntry);
    validateRegister(azelval::REG_R0 + 4, (std::uint32_t)r4.asS32());
    const std::uint32_t returnAddr = g_validationConnection->getRegister(azelval::REG_PR);

    const fixedPoint result = sqrt_F_intercept.callUndetoured(r4);

    g_validationConnection->executeUntilAddress(returnAddr);
    validateRegister(azelval::REG_R0 + 0, (std::uint32_t)result.asS32());
    return result;
}

// 0603570e
DECLARE_HOOK(MTH_Product3d_FP, kProduct3dEntry, fixedPoint, const sVec3_FP &, const sVec3_FP &)

fixedPoint MTH_Product3d_FP_detour(const sVec3_FP &r4, const sVec3_FP &r5) {
    if (g_validationConnection == nullptr || !isValidationContextEnabled(VCTX_Math)) {
        return MTH_Product3d_FP_intercept.callUndetoured(r4, r5);
    }

    g_validationConnection->executeUntilAddress(kProduct3dEntry);
    validate(g_validationConnection->getRegister(azelval::REG_R0 + 4), r4);
    validate(g_validationConnection->getRegister(azelval::REG_R0 + 5), r5);
    const std::uint32_t returnAddr = g_validationConnection->getRegister(azelval::REG_PR);

    const fixedPoint result = MTH_Product3d_FP_intercept.callUndetoured(r4, r5);

    g_validationConnection->executeUntilAddress(returnAddr);
    validateRegister(azelval::REG_R0 + 0, (std::uint32_t)result.asS32());
    return result;
}

void enableMathHooks() {
    FP_Pow2_intercept.enable();
    sqrt_F_intercept.enable();
    MTH_Product3d_FP_intercept.enable();
}
