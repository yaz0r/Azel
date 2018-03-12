#include "PDS.h"

struct s_lightSetup
{
    u32 field_10;
    u32 field_14;
    u32 field_18;
};

s_lightSetup lightSetup;

struct s_dragonMenuDragonWorkAreaSub1
{
    s_graphicEngineStatus_405C field_120;
    u32 field_170[3];
    s_lightSetup lightSetup; // 17C
};

struct s_dragonMenuDragonWorkArea : public s_workArea
{
    s_loadDragonWorkArea* field_0;
    const sDragonData3* field_4;
    u32 field_14;
    u32 field_18;
    fixedPoint field_1C;
    fixedPoint field_20;
    fixedPoint field_24;
    u8* field_28;
    u32 field_2C;
    u16 field_30;
    s_dragonMenuDragonWorkAreaSub1 field_34;
};

void dragonMenuDragonInitSub1(s_dragonMenuDragonWorkAreaSub1* r4)
{
    memcpy_dma(&graphicEngineStatus.field_405C, &r4->field_120, sizeof(s_graphicEngineStatus_405C));

    r4->field_170[0] = getVdp1VramU16(graphicEngineStatus.field_405C.setLocalCoordinatesEA + 0xC);
    r4->field_170[1] = getVdp1VramU16(graphicEngineStatus.field_405C.setClippingCoordinatesEA + 0xC);
    r4->field_170[2] = getVdp1VramU16(graphicEngineStatus.field_405C.setClippingCoordinatesEA + 0x14);

    memcpy_dma(&lightSetup, &r4->lightSetup, sizeof(s_lightSetup));

    pushProjectionStack();
}

u32 dragonMenuDragonInitSub2Sub1(s_dragonStateSubData1* pDragonStateData1, u32 r5)
{
    if ((r5 > 0) && (pDragonStateData1->field_38 == 0))
    {
        if (pDragonStateData1->field_48)
        {
            assert(0);
        }
        else
        {
            assert(0);
        }

        pDragonStateData1->field_4C = 0;
        pDragonStateData1->field_4E = r5;
        return 1;
    }
    else
    {
        pDragonStateData1->field_4C = 0;
        pDragonStateData1->field_4E = 0;
        return 0;
    }
}

void dragonMenuDragonInitSub2(s_dragonStateSubData1* pDragonStateData1, u8* r5, u32 r6)
{
    if (dragonMenuDragonInitSub2Sub1(pDragonStateData1, r6))
    {
        assert(0);
    }
    else
    {
        dragonFieldTaskInitSub3Sub1(pDragonStateData1, r5);
    }
}

void dragonMenuDragonInitSub3(s_dragonStateSubData2* pDragonStateData2)
{
    for (int i = 0; i < pDragonStateData2->countAnims; i++)
    {
        memset(&pDragonStateData2->field_8[i].matrix, 0, sizeof(sMatrix4x3));
    }
}

void setupVdp1LocalCoordinatesAndClipping()
{
    setVdp1VramU16(graphicEngineStatus.field_405C.setLocalCoordinatesEA + 0xC, graphicEngineStatus.field_405C.localCoordinatesX);
    setVdp1VramU16(graphicEngineStatus.field_405C.setLocalCoordinatesEA + 0xE, graphicEngineStatus.field_405C.localCoordinatesY);

    if (graphicEngineStatus.field_405C.VDP1_X1 > 0)
    {
        setVdp1VramU16(graphicEngineStatus.field_405C.setClippingCoordinatesEA + 0xC, graphicEngineStatus.field_405C.VDP1_X1);
    }
    else
    {
        setVdp1VramU16(graphicEngineStatus.field_405C.setClippingCoordinatesEA + 0xC, 0);
    }

    if (graphicEngineStatus.field_405C.VDP1_Y1 > 0)
    {
        setVdp1VramU16(graphicEngineStatus.field_405C.setClippingCoordinatesEA + 0xE, graphicEngineStatus.field_405C.VDP1_Y1);
    }
    else
    {
        setVdp1VramU16(graphicEngineStatus.field_405C.setClippingCoordinatesEA + 0xE, 0);
    }

    if (graphicEngineStatus.field_405C.VDP1_X2 < 0x160)
    {
        setVdp1VramU16(graphicEngineStatus.field_405C.setClippingCoordinatesEA + 0x14, graphicEngineStatus.field_405C.VDP1_Y1);
    }
    else
    {
        setVdp1VramU16(graphicEngineStatus.field_405C.setClippingCoordinatesEA + 0x14, 0x160);
    }

    if (graphicEngineStatus.field_405C.VDP1_Y2 < 0xE0)
    {
        setVdp1VramU16(graphicEngineStatus.field_405C.setClippingCoordinatesEA + 0x16, graphicEngineStatus.field_405C.VDP1_Y2);
    }
    else
    {
        setVdp1VramU16(graphicEngineStatus.field_405C.setClippingCoordinatesEA + 0x16, 0xE0);
    }
}

s32 lightFalloffMap[32][3];

void generateMasterLightFalloffMap(u32 r4, u32 r5, u32 r6)
{
    s32 r9t = 0x8421 * ((s8)(r6)-(s8)(r5));
    s32 r10t = 0x8421 * ((s8)(r6 >> 8) - (s8)(r5 >> 8));
    s32 r11t = 0x8421 * ((s8)(r6 >> 16) - (s8)(r5 >> 16));

    s32 r1t = 0x84210 * ((s8)(r4)-(s8)(r6));
    s32 r5t = ((s32)(s8)(r4)) << 24;
    s32 r2t = 0x84210 * ((s8)(r4 >> 8)-(s8)(r6 >> 8));
    s32 r6t = ((s32)(s8)(r4 >> 8)) << 24;
    s32 r3t = 0x84210 * ((s8)(r4 >> 16)-(s8)(r6 >> 16));
    s32 r7t = ((s32)(s8)(r4 >> 16)) << 24;

    for (int i = 0; i < 32; i++)
    {
        lightFalloffMap[i][0] = r5t;
        lightFalloffMap[i][1] = r6t;
        lightFalloffMap[i][2] = r7t;

        r1t += r9t;
        r2t += r10t;
        r3t += r11t;

        r5t += r1t;
        r6t += r2t;
        r7t += r3t;
    }
}

void generateLightFalloffMap(u32 r4, u32 r5, u32 r6)
{
    lightSetup.field_10 = r4;
    lightSetup.field_14 = r5;
    lightSetup.field_18 = r6;

    generateMasterLightFalloffMap(r4, r5, r6);

    //addSlaveCommand(0,0,0, copyFalloffTableToSlave);
}

void dragonMenuDragonInit(p_workArea pTypelessWorkArea)
{
    s_dragonMenuDragonWorkArea* pWorkArea = static_cast<s_dragonMenuDragonWorkArea*>(pTypelessWorkArea);

    dragonMenuDragonInitSub1(&pWorkArea->field_34);

    pWorkArea->field_30 = getVdp2VramU16(0x25002);
    pWorkArea->field_28 = gDragonState->dragonStateSubData1.pCurrentAnimation;
    pWorkArea->field_2C = gDragonState->dragonStateSubData1.field_16;

    pWorkArea->field_0 = loadDragonModel(pWorkArea, mainGameState.gameStats.dragonLevel);

    pWorkArea->field_4 = &dragonData3[mainGameState.gameStats.dragonLevel];
    pWorkArea->field_14 = 0x4000;
    pWorkArea->field_18 = 0xA000;
    pWorkArea->field_1C = fixedPoint(0xE38E38);
    pWorkArea->field_20 = fixedPoint(0x638E38E);
    pWorkArea->field_24 = fixedPoint(0xF555555);

    dragonMenuDragonInitSub2(&gDragonState->dragonStateSubData1, gDragonState->pDragonModel + READ_BE_U32(gDragonState->pDragonModel + gDragonState->dragonData2[0]), 0); // Todo: is the [0] correct?

    dragonMenuDragonInitSub3(&gDragonState->dragonStateSubData2);

    graphicEngineStatus.field_405C.localCoordinatesX = 0x78;
    graphicEngineStatus.field_405C.localCoordinatesY = 0x70;

    setupVdp1LocalCoordinatesAndClipping();

    graphicEngineStatus.field_406C = 0x2000;
    graphicEngineStatus.field_408C = FP_Div(0x10000, 0x2000);

    graphicEngineStatus.field_4070 = 0x100000;
    graphicEngineStatus.field_4094 = FP_Div(0x8000, 0x100000);

    graphicEngineStatus.field_4090 = graphicEngineStatus.field_4094 << 8;

    generateLightFalloffMap(0x30102, 0, 0);;

    initVDP1Projection(fixedPoint(0x1C71C71), 0);

    //resetCameraProperties2(&cameraProperties2);

    assert(0);
}

void dragonMenuDragonUpdate(p_workArea pTypelessWorkArea)
{
    unimplemented("dragonMenuDragonUpdate");
}

void dragonMenuDragonInitDraw(p_workArea pTypelessWorkArea)
{
    unimplemented("dragonMenuDragonInitDraw");
}

void dragonMenuDragonDelete(p_workArea pTypelessWorkArea)
{
    unimplemented("dragonMenuDragonDelete");
}

s_taskDefinition dragonMenuDragonTaskDefinition = { dragonMenuDragonInit, dragonMenuDragonUpdate, dragonMenuDragonInitDraw, dragonMenuDragonDelete, "dragonMenuDragon" };

p_workArea createDragonMenuMorhTask(p_workArea pWorkArea)
{
    return createSubTask(pWorkArea, &dragonMenuDragonTaskDefinition, new s_dragonMenuDragonWorkArea);
}
