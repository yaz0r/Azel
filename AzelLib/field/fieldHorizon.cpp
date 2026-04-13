#include "PDS.h"
#include "fieldHorizon.h"
#include "shared/vdp2PlaneTask.h"
#include "VDP2.h"
#include "battle/BTL_A3/BTL_A3_map6.h"

// Forward declare VDP1 gauge draw function
void drawGaugeVdp1(u16 mode, std::array<sVec2_S16, 4>& params, u16 color, fixedPoint depth);

// 0607653c (A7) / 06077f70 (A3) — compute horizon Y from X
static s32 horizonScreenY(sCoefficientTableData& coeff, s32 screenHeight, s32 xPos, s32 scrollValue)
{
    s32 numerator = ((scrollValue + 100) * 0x10000 - (coeff.m54 & 0x7FFFFFFF)) - coeff.m5C * xPos;
    return screenHeight - FP_Div(numerator, fixedPoint(coeff.m58)).getInteger();
}

// 060764f8 (A7) / 06077f2c (A3) — compute horizon X from Y
static s32 horizonScreenX(sCoefficientTableData& coeff, s32 yPos, s32 scrollValue)
{
    s32 numerator = ((scrollValue + 100) * 0x10000 - (coeff.m54 & 0x7FFFFFFF)) - coeff.m58 * yPos;
    return FP_Div(numerator, fixedPoint(coeff.m5C)).getInteger() - 0xB0;
}

// 06076584 (A7) / 06077fb8 (A3) — draw horizon gauge line on VDP1
void drawHorizon(s32 scrollValue, u16 color)
{
    s32 screenHeight = (VDP2Regs_.m4_TVSTAT & 1) == 0 ? 0x70 : 0x80;

    s8 coeffIdx = (s8)(gRotationPassState.m0_planeIndex * 2 + (s32)vdp2Controls.m0_doubleBufferIndex);
    sCoefficientTableData& coeff = gCoefficientTables[0][coeffIdx];

    s32 adjustedScroll = scrollValue;
    if ((VDP2Regs_.m4_TVSTAT & 1) != 0)
    {
        adjustedScroll = scrollValue + 0x10;
    }

    s32 maxDist = screenHeight + 8;

    s32 leftY = horizonScreenY(coeff, screenHeight, -8, adjustedScroll);
    s32 absLeftY = leftY < 0 ? -leftY : leftY;

    std::array<sVec2_S16, 4> gaugeQuad;
    u32 pointsNeeded = (u32)(maxDist < absLeftY);

    if (maxDist >= absLeftY)
    {
        gaugeQuad[2][0] = -0xB8;
        gaugeQuad[0][0] = -0xB8;
        gaugeQuad[0][1] = (s16)leftY + 2;
        gaugeQuad[2][1] = (s16)leftY - 2;
    }

    s32 rightY = horizonScreenY(coeff, screenHeight, 0x168, adjustedScroll);
    s32 absRightY = rightY < 0 ? -rightY : rightY;

    u32 remaining = pointsNeeded;
    if (absRightY <= maxDist)
    {
        remaining = pointsNeeded - 1;
        gaugeQuad[pointsNeeded * 2 + 4 > 7 ? 3 : pointsNeeded] = { 0xB8, (s16)(rightY - 2) };
        gaugeQuad[(1 - pointsNeeded)] = { 0xB8, (s16)(rightY + 2) };
    }

    if ((s32)remaining >= 0)
    {
        s32 topX = horizonScreenX(coeff, -8, adjustedScroll);
        s32 absTopX = topX < 0 ? -topX : topX;
        if (absTopX < 0xB9)
        {
            s16 edgeY = (s16)screenHeight + 8;
            gaugeQuad[(1 - remaining)] = { (s16)(topX - 2), edgeY };
            gaugeQuad[remaining == 0 ? 2 : 3] = { (s16)(topX + 2), edgeY };
            remaining--;
        }
    }

    if ((s32)remaining >= 0)
    {
        s32 botX = horizonScreenX(coeff, screenHeight * 2 + 8, adjustedScroll);
        s32 absBotX = botX < 0 ? -botX : botX;
        if (absBotX < 0xB9)
        {
            s16 edgeY = -8 - (s16)screenHeight;
            gaugeQuad[(1 - remaining)] = { (s16)(botX - 2), edgeY };
            gaugeQuad[remaining == 0 ? 2 : 3] = { (s16)(botX + 2), edgeY };
            remaining--;
        }
    }

    if ((s32)remaining < 0)
    {
        drawGaugeVdp1(0xC0, gaugeQuad, color, graphicEngineStatus.m405C.m14_farClipDistance - 1);
    }
}
