#include "PDS.h"
#include "battleDamageDisplay.h"
#include "battleTargetable.h"

// Task for displaying enemy/dragon damage numbers.
// Separate from sDamageNumber (battleDamageNumber.cpp) — uses different font/color VDP1 tables.
// Font data at BTL_A3::060b1f7c (mode 0) and BTL_A3::060b1f04 (mode 1)
// Color data at BTL_A3::060b1ff4 (8 bytes per colorType)
struct sDamageDisplayTask : public s_workAreaTemplateWithCopy<sDamageDisplayTask>
{
    s32 m8;
    s32 mC;
    sVec3_FP m10_position;
    s16 m1C;
    s16 m1E_frameCounter;
    s16 m20_lifetime;
    s16 m22_damageValue;
    char m24_digits[10];
    s16 m2E_screenX;
    s16 m30_screenY;
    s16 m32_currentWidth;
    s16 m34_currentHeight;
    s16 m36_targetWidth;
    s16 m38_targetHeight;
    s16 m3A_projectedX;
    s16 m3C_projectedY;
    s16 m3E_offsetX;
    s16 m40_offsetY;
    u16 m42_colorType;
    s32 m44_animMode;
    // size 0x48
};

// BTL_A3::0605bdcc
static void damageDisplay_projectToScreen(sDamageDisplayTask* pThis, sVec3_FP* viewPos)
{
    fixedPoint nearPlane, farPlane;
    getVdp1ClippingPlanes(nearPlane, farPlane);

    if (viewPos->m8_Z < nearPlane || viewPos->m8_Z > farPlane)
    {
        pThis->m3A_projectedX = 0;
        pThis->m3C_projectedY = 0;
    }
    else
    {
        s16 projParam[2];
        s16 heightParam[2];
        getVdp1ProjectionParams(&projParam[0], &projParam[1]);
        heightParam[0] = projParam[0];
        heightParam[1] = projParam[1];

        pThis->m3A_projectedX = (s16)setDividend(projParam[0], viewPos->m0_X, viewPos->m8_Z);
        pThis->m3C_projectedY = (s16)setDividend(heightParam[0], viewPos->m4_Y, viewPos->m8_Z);

        s16 screenRes[4];
        getVdp1ScreenResolution(screenRes);

        if (pThis->m3C_projectedY > screenRes[1] - 0x18)
            pThis->m3C_projectedY = screenRes[1] - 0x18;
        if (pThis->m3C_projectedY < screenRes[3] + pThis->m38_targetHeight + 0x28)
            pThis->m3C_projectedY = screenRes[3] + pThis->m38_targetHeight + 0x28;

        if (pThis->m3A_projectedX < screenRes[0] + 8)
            pThis->m3A_projectedX = screenRes[0] + 8;
        if (pThis->m3A_projectedX > screenRes[2] + pThis->m36_targetWidth * -3 - 8)
            pThis->m3A_projectedX = screenRes[2] + pThis->m36_targetWidth * -3 - 8;
    }

    pThis->m2E_screenX = pThis->m3A_projectedX;
    pThis->m30_screenY = pThis->m3C_projectedY;
}

// BTL_A3::0605bf08
static void damageDisplay_formatAndPosition(sDamageDisplayTask* pThis, s32 animMode)
{
    static const char digitLookup[] = "0123456789";
    s32 numDigits = 0;
    s16 value = pThis->m22_damageValue;

    if (value > 9999)
        pThis->m24_digits[numDigits++] = digitLookup[(value / 10000) % 10];
    if (value > 999)
        pThis->m24_digits[numDigits++] = digitLookup[(value / 1000) % 10];
    if (value > 99)
        pThis->m24_digits[numDigits++] = digitLookup[(value / 100) % 10];
    if (value >= 10)
        pThis->m24_digits[numDigits++] = digitLookup[(value / 10) % 10];
    if (value >= 0)
        pThis->m24_digits[numDigits++] = digitLookup[value % 10];
    pThis->m24_digits[numDigits] = 0;

    pThis->m44_animMode = animMode;
    if (animMode == 0)
    {
        pThis->m32_currentWidth  = 8;
        pThis->m34_currentHeight = 0xB;
        pThis->m36_targetWidth   = 8;
        pThis->m38_targetHeight  = 0xB;
        pThis->m20_lifetime = 0x1E;
    }
    else if (animMode == 1)
    {
        pThis->m32_currentWidth  = 8;
        pThis->m34_currentHeight = 0;
        pThis->m36_targetWidth   = 8;
        pThis->m38_targetHeight  = 0x10;
        pThis->m20_lifetime = 0x1E;
    }

    damageDisplay_projectToScreen(pThis, &pThis->m10_position);
}

// BTL_A3::0605c050
static void damageDisplay_update(sDamageDisplayTask* pThis)
{
    s16 frame = pThis->m1E_frameCounter;
    pThis->m1E_frameCounter = frame + 1;
    if (frame > pThis->m20_lifetime)
        pThis->getTask()->markFinished();

    if (pThis->m32_currentWidth < pThis->m36_targetWidth)
        pThis->m32_currentWidth++;
    else
        pThis->m32_currentWidth = pThis->m36_targetWidth;

    if (pThis->m34_currentHeight < pThis->m38_targetHeight)
        pThis->m34_currentHeight += 2;
    else
        pThis->m34_currentHeight = pThis->m38_targetHeight;

    pThis->m2E_screenX = pThis->m3A_projectedX + pThis->m3E_offsetX;
    pThis->m30_screenY = pThis->m3C_projectedY + pThis->m40_offsetY;

    if (pThis->m44_animMode == 0)
        pThis->m40_offsetY++;
}

// BTL_A3::0605c0de
static void damageDisplay_draw(sDamageDisplayTask* pThis)
{
    // Renders digits using font tables at 060b1f7c (mode 0) / 060b1f04 (mode 1)
    // and color table at 060b1ff4 via graphicEngineStatus.vdp1Context_Master
    Unimplemented();
}

// BTL_A3::0605bec4
void createDamageDisplayTask(s_workAreaCopy* pParent, s16 param2_damageValue, sVec3_FP* param3, s32 param4)
{
    static const sDamageDisplayTask::TypedTaskDefinition definition = {
        nullptr, &damageDisplay_update, &damageDisplay_draw, nullptr
    };

    sDamageDisplayTask* pThis = createSiblingTaskWithCopy<sDamageDisplayTask>(pParent, &definition);
    if (!pThis) return;

    if (param2_damageValue < 0)
    {
        pThis->m42_colorType = 1;
        pThis->m22_damageValue = -param2_damageValue;
    }
    else if (param2_damageValue == 0)
    {
        pThis->m42_colorType = 3;
        pThis->m22_damageValue = 0;
    }
    else
    {
        pThis->m42_colorType = 0;
        pThis->m22_damageValue = param2_damageValue;
    }

    transformAndAddVecByCurrentMatrix(param3, &pThis->m10_position);
    damageDisplay_formatAndPosition(pThis, param4);
}

// BTL_A3::0605bb9a
void createDamageDisplayNumber(s_workAreaCopy* pParent, s16 param2_damageValue, sVec3_FP* param3, s32 param4)
{
    static const sDamageDisplayTask::TypedTaskDefinition definition = {
        nullptr, &damageDisplay_update, &damageDisplay_draw, nullptr
    };

    sDamageDisplayTask* pThis = createSiblingTaskWithCopy<sDamageDisplayTask>(pParent, &definition);
    if (!pThis) return;

    if (param2_damageValue < 1)
    {
        pThis->m42_colorType = 1;
        pThis->m22_damageValue = -param2_damageValue;
    }
    else if (param2_damageValue == 0)
    {
        pThis->m42_colorType = 3;
        pThis->m22_damageValue = 0;
    }
    else
    {
        pThis->m42_colorType = 0;
        pThis->m22_damageValue = param2_damageValue;
    }

    pThis->m10_position = *param3;
    damageDisplay_formatAndPosition(pThis, param4);
}

// 0605bae8
void createHitSparkEffect(p_workArea pParent, sBattleTargetable& targetable, s32 animMode)
{
    static const sDamageDisplayTask::TypedTaskDefinition definition = {
        nullptr, &damageDisplay_update, &damageDisplay_draw, nullptr
    };

    sDamageDisplayTask* pThis = createSiblingTaskWithCopy<sDamageDisplayTask>((s_workAreaCopy*)pParent, &definition);
    if (!pThis) return;

    pThis->m22_damageValue = targetable.m58;
    if (pThis->m22_damageValue < 0)
    {
        pThis->m42_colorType = 1;
        pThis->m22_damageValue = -pThis->m22_damageValue;
    }
    else if (pThis->m22_damageValue == 0)
    {
        pThis->m42_colorType = 3;
        pThis->m22_damageValue = 3;
    }
    else
    {
        pThis->m42_colorType = 0;
        if ((targetable.m5E_impactForce == 1 || targetable.m5E_impactForce == 2) && targetable.m60 > 10)
        {
            pThis->m42_colorType = 2;
        }
    }

    pThis->m10_position = *targetable.m4_pPosition;
    damageDisplay_formatAndPosition(pThis, animMode);
}
