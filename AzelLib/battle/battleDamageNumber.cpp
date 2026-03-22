#include "PDS.h"
#include "battleDamageNumber.h"
#include "battleTargetable.h"
#include "kernel/graphicalObject.h"
#include "kernel/vdp1Allocator.h"

struct sDamageNumber : public s_workAreaTemplateWithCopy<sDamageNumber>
{
    sVec3_FP m10_position;
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
    s16 m42_colorType;
    s32 m44_animMode;
    // size 0x48
};

// Font sprite lookup: 10 entries of {pad[2], CMDSRCA[2], pad[2], CMDCOLR[2], CMDSIZE[2], pad[2]} = 12 bytes each
// Located at 0x060b28b8 (mode 0) and 0x060b2840 (mode 1)
// Color types at 0x060b2930: 4 quadColors (8 bytes each)

void getVdp1ScreenResolution(s16 (&screenResolution)[4]);

// 0605f438
static void damageNumber_projectToScreen(sDamageNumber* pThis, sVec3_FP* viewPos)
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
        heightParam[0] = projParam[0]; // reuse for height
        heightParam[1] = projParam[1];

        pThis->m3A_projectedX = (s16)setDividend(projParam[0], viewPos->m0_X, viewPos->m8_Z);
        pThis->m3C_projectedY = (s16)setDividend(heightParam[0], viewPos->m4_Y, viewPos->m8_Z);

        s16 screenRes[4];
        getVdp1ScreenResolution(screenRes);

        // Clamp Y
        if (pThis->m3C_projectedY > screenRes[1] - 0x18)
            pThis->m3C_projectedY = screenRes[1] - 0x18;
        if (pThis->m3C_projectedY < screenRes[3] + pThis->m38_targetHeight + 0x28)
            pThis->m3C_projectedY = screenRes[3] + pThis->m38_targetHeight + 0x28;

        // Clamp X
        if (pThis->m3A_projectedX < screenRes[0] + 8)
            pThis->m3A_projectedX = screenRes[0] + 8;
        if (pThis->m3A_projectedX > screenRes[2] + pThis->m36_targetWidth * -3 - 8)
            pThis->m3A_projectedX = screenRes[2] + pThis->m36_targetWidth * -3 - 8;
    }

    pThis->m2E_screenX = pThis->m3A_projectedX;
    pThis->m30_screenY = pThis->m3C_projectedY;
}

// 0605f574
static void damageNumber_formatAndPosition(sDamageNumber* pThis, s32 animMode)
{
    static const char digitLookup[] = "0123456789";
    s32 numDigits = 0;
    s16 value = pThis->m22_damageValue;

    if (value > 9999)
    {
        pThis->m24_digits[numDigits++] = digitLookup[(value / 10000) % 10];
    }
    if (value > 999)
    {
        pThis->m24_digits[numDigits++] = digitLookup[(value / 1000) % 10];
    }
    if (value > 99)
    {
        pThis->m24_digits[numDigits++] = digitLookup[(value / 100) % 10];
    }
    if (value >= 10)
    {
        pThis->m24_digits[numDigits++] = digitLookup[(value / 10) % 10];
    }
    if (value >= 0)
    {
        pThis->m24_digits[numDigits++] = digitLookup[value % 10];
    }
    pThis->m24_digits[numDigits] = 0;

    pThis->m44_animMode = animMode;
    if (animMode == 0)
    {
        pThis->m32_currentWidth = 8;
        pThis->m34_currentHeight = 0xB;
        pThis->m36_targetWidth = 8;
        pThis->m38_targetHeight = 0xB;
        pThis->m20_lifetime = 0x1E;
    }
    else if (animMode == 1)
    {
        pThis->m32_currentWidth = 8;
        pThis->m34_currentHeight = 0;
        pThis->m36_targetWidth = 8;
        pThis->m38_targetHeight = 0x10;
        pThis->m20_lifetime = 0x1E;
    }

    damageNumber_projectToScreen(pThis, &pThis->m10_position);
}

// 0605f6bc
static void damageNumber_update(sDamageNumber* pThis)
{
    s16 frame = pThis->m1E_frameCounter;
    pThis->m1E_frameCounter = frame + 1;
    if (frame > pThis->m20_lifetime)
    {
        pThis->getTask()->markFinished();
    }

    // Animate width toward target
    if (pThis->m32_currentWidth < pThis->m36_targetWidth)
        pThis->m32_currentWidth++;
    else
        pThis->m32_currentWidth = pThis->m36_targetWidth;

    // Animate height toward target
    if (pThis->m34_currentHeight < pThis->m38_targetHeight)
        pThis->m34_currentHeight += 2;
    else
        pThis->m34_currentHeight = pThis->m38_targetHeight;

    // Current screen position = projected base + offset
    pThis->m2E_screenX = pThis->m3A_projectedX + pThis->m3E_offsetX;
    pThis->m30_screenY = pThis->m3C_projectedY + pThis->m40_offsetY;

    // Mode 0: float upward
    if (pThis->m44_animMode == 0)
    {
        pThis->m40_offsetY++;
    }
}

// 0605f74a
static void damageNumber_draw(sDamageNumber* pThis)
{
    // Font sprite data tables (mode 0 at 0x060b28b8, mode 1 at 0x060b2840)
    // Each digit: 12 bytes: {u16 pad, u16 CMDSRCA, u16 pad, u16 CMDCOLR, u16 CMDSIZE, u16 pad}
    // Color type table at 0x060b2930: 4 x quadColor (8 bytes each)
    static const quadColor colorTypes[4] = {
        { 0xC210, 0xC210, 0xC210, 0xC210 }, // 0: damage (white)
        { 0x93E4, 0x93E4, 0xA3E8, 0xA3E8 }, // 1: heal (green)
        { 0x909F, 0x909F, 0xA11F, 0xA11F }, // 2: critical (blue)
        { 0xC3FF, 0xC3FF, 0xC3FF, 0xC3FF }, // 3: miss (bright white)
    };

    // Font sprite data extracted from Saturn binary
    // Mode 0: 8x8 digits, CMDSRCA starts at 0x58 incrementing by 0xC
    // Mode 1: 8x16 digits, CMDSRCA starts at 0x50 incrementing by 0xC
    // Both modes: CMDCOLR = 0x2EB8
    struct sDigitSprite { u16 CMDSRCA; u16 CMDCOLR; u16 CMDSIZE; };
    static const sDigitSprite fontMode0[10] = {
        {0x0058, 0x2EB8, 0x0108}, {0x0064, 0x2EB8, 0x0108},
        {0x0070, 0x2EB8, 0x0108}, {0x007C, 0x2EB8, 0x0108},
        {0x0088, 0x2EB8, 0x0108}, {0x0094, 0x2EB8, 0x0108},
        {0x00A0, 0x2EB8, 0x0108}, {0x00AC, 0x2EB8, 0x0108},
        {0x00B8, 0x2EB8, 0x0108}, {0x00C4, 0x2EB8, 0x0108},
    };
    static const sDigitSprite fontMode1[10] = {
        {0x0050, 0x2EB8, 0x0110}, {0x005C, 0x2EB8, 0x0110},
        {0x0068, 0x2EB8, 0x0110}, {0x0074, 0x2EB8, 0x0110},
        {0x0080, 0x2EB8, 0x0110}, {0x008C, 0x2EB8, 0x0110},
        {0x0098, 0x2EB8, 0x0110}, {0x00A4, 0x2EB8, 0x0110},
        {0x00B0, 0x2EB8, 0x0110}, {0x00BC, 0x2EB8, 0x0110},
    };
    const sDigitSprite* fontData = (pThis->m44_animMode == 1) ? fontMode1 : fontMode0;

    u16 vdp1Base = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory;
    const quadColor& colors = colorTypes[pThis->m42_colorType & 3];

    s16 x = pThis->m2E_screenX;
    for (int i = 0; pThis->m24_digits[i] != 0; i++)
    {
        int digitIndex = pThis->m24_digits[i] - '0';
        if (digitIndex < 0 || digitIndex > 9) continue;

        s_vdp1Command& cmd = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        cmd.m0_CMDCTRL = 0x1001; // scaled sprite
        cmd.m4_CMDPMOD = 0x48C;
        cmd.m6_CMDCOLR = fontData[digitIndex].CMDCOLR + vdp1Base;
        cmd.m8_CMDSRCA = fontData[digitIndex].CMDSRCA + vdp1Base;
        cmd.mA_CMDSIZE = fontData[digitIndex].CMDSIZE;
        cmd.mC_CMDXA = x;
        cmd.mE_CMDYA = -pThis->m30_screenY;
        cmd.m14_CMDXC = pThis->m32_currentWidth + x - 1;
        cmd.m16_CMDYC = (pThis->m34_currentHeight - pThis->m30_screenY) - 1;

        s32 outputColorIndex = (s32)(graphicEngineStatus.m14_vdp1Context[0].m10 - graphicEngineStatus.m14_vdp1Context[0].m14->begin());
        quadColor& outputColor = *(graphicEngineStatus.m14_vdp1Context[0].m10++);
        outputColor = colors;
        cmd.m1C_CMDGRA = outputColorIndex;

        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &cmd;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;
        graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
        graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
        graphicEngineStatus.m14_vdp1Context[0].mC += 1;

        x += pThis->m32_currentWidth;
    }
}

// 0605f142
void createDamageNumberFromTargetable(s_workAreaCopy* parent, sBattleTargetable* targetable, s32 animMode)
{
    static const sDamageNumber::TypedTaskDefinition definition = {
        nullptr, &damageNumber_update, &damageNumber_draw, nullptr
    };

    sDamageNumber* pThis = createSiblingTaskWithCopy<sDamageNumber>(parent, &definition);
    if (!pThis) return;

    pThis->m22_damageValue = targetable->m58;
    if (pThis->m22_damageValue < 0)
    {
        pThis->m42_colorType = 1; // heal
        pThis->m22_damageValue = -pThis->m22_damageValue;
    }
    else if (pThis->m22_damageValue == 0)
    {
        pThis->m42_colorType = 3; // miss
    }
    else
    {
        pThis->m42_colorType = 0; // damage
        if ((targetable->m5E_impactForce == 1 || targetable->m5E_impactForce == 2) && targetable->m60 > 10)
        {
            pThis->m42_colorType = 2; // critical
        }
    }

    // Copy targetable position
    pThis->m10_position = *targetable->m4_pPosition;

    damageNumber_formatAndPosition(pThis, animMode);
}

// 0605f0d8
void createDamageNumberFromValue(s_workAreaCopy* parent, s16 damageValue, sVec3_FP* position)
{
    static const sDamageNumber::TypedTaskDefinition definition = {
        nullptr, &damageNumber_update, &damageNumber_draw, nullptr
    };

    sDamageNumber* pThis = createSiblingTaskWithCopy<sDamageNumber>(parent, &definition);
    if (!pThis) return;

    if (damageValue < 0)
    {
        pThis->m42_colorType = 1; // heal
        pThis->m22_damageValue = -damageValue;
    }
    else if (damageValue == 0)
    {
        pThis->m42_colorType = 3; // miss
    }
    else
    {
        pThis->m42_colorType = 0; // damage
        pThis->m22_damageValue = damageValue;
    }

    transformAndAddVecByCurrentMatrix(position, &pThis->m10_position);

    damageNumber_formatAndPosition(pThis, 0);
}
