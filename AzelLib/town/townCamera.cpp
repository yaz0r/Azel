#include "PDS.h"
#include "town.h"
#include "townCamera.h"

void townCamera_update(sCameraTask* pThis)
{
    if ((npcData0.mFC & 1) == 0)
    {
        pThis->m4++;
        if (pThis->m4 > 5400)
        {
            pThis->m4 = 5400;
        }
    }
}

void townCamera_draw(sCameraTask* pThis)
{
    sVec3_FP stack16;
    transformVecByCurrentMatrix(pThis->m14, stack16);
    setupLight(stack16[0], stack16[1], stack16[2], pThis->m10.toU32());
}

s32 townCamera_setup(s32 r4, s32 r5)
{
    sVec3_FP r4Value = readSaturnVec3(sSaturnPtr::createFromRaw(r4, gCurrentTownOverlay)); //todo: that could be a vec2
    sSaturnPtr r5Ptr = sSaturnPtr::createFromRaw(r5, gCurrentTownOverlay);
    cameraTaskPtr->m8 = r5Ptr;

    sMatrix4x3 var4;
    initMatrixToIdentity(&var4);
    rotateMatrixShiftedY(r4Value[1], &var4);
    rotateMatrixShiftedX(r4Value[0], &var4);

    cameraTaskPtr->m14[0] = var4.m[0][3];
    cameraTaskPtr->m14[1] = var4.m[1][3];
    cameraTaskPtr->m14[2] = var4.m[2][3];

    cameraTaskPtr->m10 = readSaturnRGB8(r5Ptr);
    cameraTaskPtr->m30 = 0x8000;

    generateLightFalloffMap(readSaturnRGB8(r5Ptr + 3).toU32(), readSaturnRGB8(r5Ptr + 6).toU32(), readSaturnRGB8(r5Ptr + 9).toU32());

    cameraTaskPtr->m_UpdateMethod = townCamera_update;
    cameraTaskPtr->m_DrawMethod = townCamera_draw;

    if (g_fadeControls.m_4C <= g_fadeControls.m_4D)
    {
        vdp2Controls.m20_registers[0].m112_CLOFSL = 0x10;
        vdp2Controls.m20_registers[1].m112_CLOFSL = 0x10;
    }

    resetProjectVector();
    cameraTaskPtr->m2 = 0;
    cameraTaskPtr->m0 = 0;
    return 0;
}

s32 SetupColorOffset(s32 r4)
{
    if (g_fadeControls.m_4D >= g_fadeControls.m_4C)
    {
        vdp2Controls.m20_registers[0].m112_CLOFSL = r4;
        vdp2Controls.m20_registers[1].m112_CLOFSL = r4;
    }

    return 0;
}

s32 TwnFadeOut(s32 arg0)
{
    cameraTaskPtr->m1 = 0;
    fadePalette(&g_fadeControls.m0_fade0, convertColorToU32ForFade(g_fadeControls.m0_fade0.m0_color), 0x8000, arg0);
    fadePalette(&g_fadeControls.m24_fade1, convertColorToU32ForFade(g_fadeControls.m24_fade1.m0_color), 0x8000, arg0);
    graphicEngineStatus.m40AC.m1_isMenuAllowed = 0;
    return 0;
}

u16 TwnFadeInComputeColor(sSaturnPtr ptr, u32 factor)
{
    s32 r = MTH_Mul(fixedPoint((s32)readSaturnS8(ptr + 0)), fixedPoint(factor));
    s8 d3 = readSaturnS8(ptr + 3);
    s32 g = MTH_Mul(fixedPoint((s32)readSaturnS8(ptr + 1)), fixedPoint(factor));
    s8 d4 = readSaturnS8(ptr + 4);
    s32 b = MTH_Mul(fixedPoint((s32)readSaturnS8(ptr + 2)), fixedPoint(factor));

    s32 rVal = ((r + d3) >> 1) + 8;
    s32 gVal = ((g + d4) >> 1) + 8;
    s32 bVal = ((b + readSaturnS8(ptr + 5)) >> 1) + 8;

    return (u16)(bVal * 0x400 | gVal * 0x20 | rVal | 0x8000);
}

static s32 clamp5bit(s32 val)
{
    if (val < 0) val = 0;
    if (val > 0x1F) val = 0x1F;
    return val;
}

u16 TwnFadeInComputeColorInterp(s32 time)
{
    sSaturnPtr colorSet0;
    sSaturnPtr colorSet1;
    u32 baseAddr = cameraTaskPtr->mC;

    if (!mainGameState.getBit(8)) {
        colorSet0 = sSaturnPtr::createFromRaw(baseAddr, gCurrentTownOverlay);
        colorSet1 = sSaturnPtr::createFromRaw(baseAddr + 0x10, gCurrentTownOverlay);
    } else {
        colorSet0 = sSaturnPtr::createFromRaw(baseAddr + 0x30, gCurrentTownOverlay);
        colorSet1 = sSaturnPtr::createFromRaw(baseAddr + 0x40, gCurrentTownOverlay);
    }

    if (time >= 0xe10) {
        return TwnFadeInComputeColor(colorSet1 + 0x14, cameraTaskPtr->m30);
    }

    if (time > 0x707) {
        colorSet0 = colorSet0 + 0x10;
        colorSet1 = colorSet1 + 0x10;
        time = time - 0x708;
    }

    fixedPoint t = FP_Div(time, 0x708);
    s32 invT = 0x10000 - (s32)t;

    s32 r0 = (s32)readSaturnS8(colorSet0 + 7) * 2 + (s32)readSaturnS8(colorSet0 + 4);
    s32 g0 = (s32)readSaturnS8(colorSet0 + 8) * 2 + (s32)readSaturnS8(colorSet0 + 5);
    s32 b0 = (s32)readSaturnS8(colorSet0 + 9) * 2 + (s32)readSaturnS8(colorSet0 + 6);

    s32 r1 = (s32)readSaturnS8(colorSet1 + 7) * 2 + (s32)readSaturnS8(colorSet1 + 4);
    s32 g1 = (s32)readSaturnS8(colorSet1 + 8) * 2 + (s32)readSaturnS8(colorSet1 + 5);
    s32 b1 = (s32)readSaturnS8(colorSet1 + 9) * 2 + (s32)readSaturnS8(colorSet1 + 6);

    s32 rVal = (s16)((r1 * (s32)t + r0 * invT) >> 18) + 8;
    s32 gVal = (s16)((g1 * (s32)t + g0 * invT) >> 18) + 8;
    s32 bVal = (s16)((b1 * (s32)t + b0 * invT) >> 18) + 8;

    rVal = clamp5bit(rVal);
    gVal = clamp5bit(gVal);
    bVal = clamp5bit(bVal);

    return (u16)(bVal << 10 | gVal << 5 | rVal | 0x8000);
}

s32 TwnFadeIn(s32 arg0)
{
    u16 fadeColor;
    switch (cameraTaskPtr->m0)
    {
    case 0:
    case 2:
        fadeColor = TwnFadeInComputeColor(cameraTaskPtr->m8, cameraTaskPtr->m30);
        break;
    case 1:
        fadeColor = TwnFadeInComputeColorInterp(cameraTaskPtr->m4 + arg0);
        break;
    default:
        assert(0);
        break;
    }

    fadePalette(&g_fadeControls.m0_fade0, convertColorToU32ForFade(g_fadeControls.m0_fade0.m0_color), 0xC210, arg0);
    fadePalette(&g_fadeControls.m24_fade1, convertColorToU32ForFade(g_fadeControls.m24_fade1.m0_color), fadeColor, arg0);

    cameraTaskPtr->m1 = 1;

    return 0;
}
