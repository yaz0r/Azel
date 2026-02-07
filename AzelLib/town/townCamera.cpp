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

    cameraTaskPtr->m14[0] = var4.matrix[3];
    cameraTaskPtr->m14[1] = var4.matrix[7];
    cameraTaskPtr->m14[2] = var4.matrix[11];

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

u16 TwnFadeInComputeColor(sSaturnPtr r4, u32 r5)
{
    Unimplemented();
    return 0;
}

s32 TwnFadeIn(s32 arg0)
{
    u16 fadeColor;
    switch (cameraTaskPtr->m0)
    {
    case 0:
        fadeColor = TwnFadeInComputeColor(cameraTaskPtr->m8, cameraTaskPtr->m30);
        break;
    default:
        // TODO: implement other cases
        fadeColor = 0x8000;
        break;
    }

    fadePalette(&g_fadeControls.m0_fade0, convertColorToU32ForFade(g_fadeControls.m0_fade0.m0_color), 0xC210, arg0);
    fadePalette(&g_fadeControls.m24_fade1, convertColorToU32ForFade(g_fadeControls.m24_fade1.m0_color), 0x8000, arg0);

    cameraTaskPtr->m1 = 1;

    return 0;
}
