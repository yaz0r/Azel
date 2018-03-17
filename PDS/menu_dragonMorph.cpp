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
    sVec3_FP field_10;
    fixedPoint field_1C;
    fixedPoint field_20;
    fixedPoint field_24;
    u8* field_28;
    u32 field_2C;
    u16 field_30;
    s_dragonMenuDragonWorkAreaSub1 field_34;
};

const sVec3_FP defaultCameraVectors[3] = {
    {0, 0x4000, 0},
    {0, 0x4000, 0x10000},
    {0, 0x10000, 0},
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
        setVdp1VramU16(graphicEngineStatus.field_405C.setClippingCoordinatesEA + 0x14, graphicEngineStatus.field_405C.VDP1_X2);
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

fixedPoint FP_Pow2(fixedPoint r4)
{
    s64 result = ((s64)r4.asS32()) * ((s64)r4.asS32());
    return fixedPoint::fromS32(result >> 16);
}

s32 generateCameraMatrixSub3(s32 r4, s32 r5)
{
    // same as not function, but not on FP
    return 0;
}

// this is atan2
s32 generateCameraMatrixSub1Sub3(s32 r4, s32 r5)
{
    if (r5)
    {
        u32 r1 = 0;
        if (r4 < 0)
        {
            r4 = -r4;
            r1 = 1;
        }

        if (r5 < 0)
        {
            r5 = -r5;
            r1 += 2;
        }

        assert(r4 == 0);
        assert(r5 == 0x10000);

        return 0;

        if (r4 > r5)
        {
            r5 ^= r4;
            r4 ^= r5;
            r5 ^= r4;
            r1 += 1;
        }

        r4 <<= 10;
        r5 <<= 10;
        // TO BE CONTINUED

    }
    if (r4)
    {
        if (r4 >= 0)
        {
            return 0x400 << 16; //90
        }
        else
        {
            return 0xC00 << 16; //270
        }
    }

    return 0;
}

void generateCameraMatrixSub1(sVec3_FP& r4, u32(&r5)[2])
{
    if ((r4[0] == 0) && (r4[2] == 0))
    {
        assert(0);
    }
    else
    {

        s32 r0 = sqrt_F(FP_Pow2(r4[0]) + FP_Pow2(r4[2]));

        if (r4[1] >= 0)
        {
            r5[0] = generateCameraMatrixSub1Sub3(r4[1], r0);
        }
        else
        {
            r5[2] = generateCameraMatrixSub1Sub3(-r4[1], -r0);
        }

        r5[1] = generateCameraMatrixSub1Sub3(-r4[0], -r4[2]);
    }
}

void transformVec(sVec3_FP& r4, sVec3_FP& r5, sMatrix4x3& r6)
{
    s64 mac = 0;
    mac += (s64)r6.matrix[0] * (s64)r4[0].asS32();
    mac += (s64)r6.matrix[1] * (s64)r4[1].asS32();
    mac += (s64)r6.matrix[2] * (s64)r4[2].asS32();
    r5[0] = mac >> 16;

    mac = 0;
    mac += (s64)r6.matrix[4] * (s64)r4[0].asS32();
    mac += (s64)r6.matrix[5] * (s64)r4[1].asS32();
    mac += (s64)r6.matrix[6] * (s64)r4[2].asS32();
    r5[1] = mac >> 16;

    mac = 0;
    mac += (s64)r6.matrix[8] * (s64)r4[0].asS32();
    mac += (s64)r6.matrix[9] * (s64)r4[1].asS32();
    mac += (s64)r6.matrix[10] * (s64)r4[2].asS32();
    r5[2] = mac >> 16;
}

void generateCameraMatrix(s_cameraProperties2* r4, const sVec3_FP& r13, const sVec3_FP& r6, const sVec3_FP& r7)
{
    u32 var_4[2];
    sVec3_FP var_C;
    sVec3_FP var_18;
    sVec3_FP var_24;
    sMatrix4x3 var_30;

    var_18[0] = r6[0] - r13[0];
    var_18[1] = r6[1] - r13[1];
    var_18[2] = r6[2] - r13[2];

    var_C[0] = r7[0] - r13[0];
    var_C[1] = r7[1] - r13[1];
    var_C[2] = r7[2] - r13[2];

    r4->field_20[0] = r4->field_C[0];
    r4->field_20[1] = r4->field_C[1];
    r4->field_20[2] = r4->field_C[2];

    r4->field_14[0] = r4->field_0[0];
    r4->field_14[1] = r4->field_0[1];
    r4->field_14[2] = r4->field_0[2];

    generateCameraMatrixSub1(var_18, var_4);

    r4->field_C[0] = var_4[0] >> 16;
    r4->field_C[1] = var_4[1] >> 16;

    initMatrixToIdentity(&var_30);
    rotateMatrixX(-r4->field_C[0], &var_30);
    rotateMatrixY(-r4->field_C[1], &var_30);

    transformVec(var_C, var_24, var_30);

    r4->field_C[2] = generateCameraMatrixSub3(-var_24[0], var_24[1]);

    r4->field_0[0] = r13[0];
    var_24[0] = -r13[0];
    
    r4->field_0[1] = r13[1];
    var_24[1] = -r13[1];

    r4->field_0[2] = r13[2];
    var_24[2] = -r13[2];

    resetMatrixStack();

    rotateCurrentMatrixZ(-r4->field_C[2]);
    rotateCurrentMatrixX(-r4->field_C[0]);

    copyMatrix(pCurrentMatrix, &r4->field_88);

    rotateCurrentMatrixY(-r4->field_C[1]);

    translateCurrentMatrix(&var_24);

    rotateMatrixY(0x800, &r4->field_88);

    copyMatrix(&r4->field_28[0], &r4->field_28[1]);

    initMatrixToIdentity(&r4->field_28[0]);
    translateMatrix(&r4->field_0, &r4->field_28[0]);
    rotateMatrixY(r4->field_C[1], &r4->field_28[0]);
    rotateMatrixX(r4->field_C[0], &r4->field_28[0]);
    rotateMatrixZ(r4->field_C[2], &r4->field_28[0]);

    r4->field_28[0].matrix[2] = -r4->field_28[0].matrix[2];
    r4->field_28[0].matrix[6] = -r4->field_28[0].matrix[6];
    r4->field_28[0].matrix[10] = -r4->field_28[0].matrix[10];
}

void resetCameraProperties2(s_cameraProperties2* r4)
{
    r4->field_0[2] = 0;
    r4->field_0[1] = 0;
    r4->field_0[0] = 0;

    r4->field_C[2] = 0;
    r4->field_C[1] = 0;
    r4->field_C[0] = 0;

    r4->field_14[0] = r4->field_0[0];
    r4->field_14[1] = r4->field_0[1];
    r4->field_14[2] = r4->field_0[2];

    r4->field_20[0] = r4->field_C[0];
    r4->field_20[1] = r4->field_C[1];
    r4->field_20[2] = r4->field_C[2];

    initMatrixToIdentity(&r4->field_28[0]);
    initMatrixToIdentity(&r4->field_28[1]);
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
    pWorkArea->field_10[1] = 0x4000;
    pWorkArea->field_10[2] = 0xA000;
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

    generateLightFalloffMap(0x30102, 0, 0);

    initVDP1Projection(fixedPoint(0x1C71C71), 0);

    resetCameraProperties2(&cameraProperties2);

    generateCameraMatrix(&cameraProperties2, defaultCameraVectors[0], defaultCameraVectors[1], defaultCameraVectors[2]);
}

void dragonMenuDragonUpdate(p_workArea pTypelessWorkArea)
{
    unimplemented("dragonMenuDragonUpdate");
}

void dragonMenuDragonDrawSub1(s_dragonStateSubData1* r4, u32 r5, u32 r6, sVec3_FP* r7, sVec3_FP* arg8)
{
    sMatrix4x3 var_28;

    initMatrixToIdentity(&var_28);
    translateMatrix(r7, &var_28);
    rotateMatrixYXZ(arg8, &var_28);

    if (r6)
    {
        unimplemented("parts of dragonMenuDragonDrawSub1");
    }

    pushCurrentMatrix();
    multiplyCurrentMatrix(&var_28);

    r4->field_C = r5;

    r4->drawFunction(r4);

    popMatrix();
}

void dragonMenuDragonDraw(p_workArea pTypelessWorkArea)
{
    s_dragonMenuDragonWorkArea* pWorkArea = static_cast<s_dragonMenuDragonWorkArea*>(pTypelessWorkArea);

    // this might be very incorrect
    sVec3_FP rotationVector;
    rotationVector[0] = fixedPoint::fromS32(0);
    rotationVector[1] = fixedPoint::fromS32(0);
    rotationVector[2] = fixedPoint::fromS32(0);
    dragonMenuDragonDrawSub1(&gDragonState->dragonStateSubData1, gDragonState->field_14, gDragonState->field_18, &pWorkArea->field_10, &rotationVector);
}

void dragonMenuDragonDelete(p_workArea pTypelessWorkArea)
{
    unimplemented("dragonMenuDragonDelete");
}

s_taskDefinition dragonMenuDragonTaskDefinition = { dragonMenuDragonInit, dragonMenuDragonUpdate, dragonMenuDragonDraw, dragonMenuDragonDelete, "dragonMenuDragon" };

p_workArea createDragonMenuMorhTask(p_workArea pWorkArea)
{
    return createSubTask(pWorkArea, &dragonMenuDragonTaskDefinition, new s_dragonMenuDragonWorkArea);
}
