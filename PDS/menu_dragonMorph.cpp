#include "PDS.h"

struct s_lightSetup
{
    fixedPoint m0;
    fixedPoint m4;
    fixedPoint m8;
    fixedPoint mC;
    u32 m10;
    u32 m14;
    u32 m18;
};

s_lightSetup lightSetup;

struct s_dragonMenuDragonWorkAreaSub1
{
    s_graphicEngineStatus_405C m120;
    u32 m170[3];
    s_lightSetup lightSetup; // 17C
};

struct s_dragonMenuDragonWorkArea : public s_workArea
{
    s_loadDragonWorkArea* m0; //0
    const sDragonData3* m4; //4
    u16 m8; //8
    u32 mC; //C
    sVec3_FP modelTranslation; //10
    sVec3_FP modelRotation; //1C
    u8* m28; //28
    u32 m2C; //2C
    u16 m30; //30
    s_dragonMenuDragonWorkAreaSub1 m34; //34
};

const sVec3_FP defaultCameraVectors[3] = {
    {0, 0x4000, 0},
    {0, 0x4000, 0x10000},
    {0, 0x10000, 0},
};

void dragonMenuDragonInitSub1(s_dragonMenuDragonWorkAreaSub1* r4)
{
    memcpy_dma(&graphicEngineStatus.m405C, &r4->m120, sizeof(s_graphicEngineStatus_405C));

    r4->m170[0] = getVdp1VramU16(graphicEngineStatus.m405C.setLocalCoordinatesEA + 0xC);
    r4->m170[1] = getVdp1VramU16(graphicEngineStatus.m405C.setClippingCoordinatesEA + 0xC);
    r4->m170[2] = getVdp1VramU16(graphicEngineStatus.m405C.setClippingCoordinatesEA + 0x14);

    memcpy_dma(&lightSetup, &r4->lightSetup, sizeof(s_lightSetup));

    pushProjectionStack();
}

u32 dragonMenuDragonInitSub2Sub1(s_3dModel* pDragonStateData1, u32 interpolationLength)
{
    if ((interpolationLength > 0) && (pDragonStateData1->m38 == 0))
    {
        if (pDragonStateData1->m48_poseDataInterpolation.size())
        {
            for (int i = 0; i < pDragonStateData1->m12_numBones; i++)
            {
                pDragonStateData1->m48_poseDataInterpolation[i].m24_halfTranslation = pDragonStateData1->m2C_poseData[i].m0_translation;
                pDragonStateData1->m48_poseDataInterpolation[i].m30_halfRotation = pDragonStateData1->m2C_poseData[i].mC_rotation;
                pDragonStateData1->m48_poseDataInterpolation[i].m3C_halfScale = pDragonStateData1->m2C_poseData[i].m18_scale;
            }
        }
        else
        {
            pDragonStateData1->m48_poseDataInterpolation.resize(pDragonStateData1->m12_numBones);
            if (pDragonStateData1->m48_poseDataInterpolation.size() == 0)
            {
                pDragonStateData1->m4C_interpolationStep = 0;
                pDragonStateData1->m4E_interpolationLength = 0;
                return 0;
            }

            for (int i = 0; i < pDragonStateData1->m12_numBones; i++)
            {
                pDragonStateData1->m48_poseDataInterpolation[i].m24_halfTranslation = pDragonStateData1->m2C_poseData[i].m0_translation;
                pDragonStateData1->m48_poseDataInterpolation[i].m0_translation = pDragonStateData1->m2C_poseData[i].m0_translation;
                pDragonStateData1->m48_poseDataInterpolation[i].m30_halfRotation = pDragonStateData1->m2C_poseData[i].mC_rotation;
                pDragonStateData1->m48_poseDataInterpolation[i].mC_rotation = pDragonStateData1->m2C_poseData[i].mC_rotation;
                pDragonStateData1->m48_poseDataInterpolation[i].m3C_halfScale = pDragonStateData1->m2C_poseData[i].m18_scale;
                pDragonStateData1->m48_poseDataInterpolation[i].m18_scale = pDragonStateData1->m2C_poseData[i].m18_scale;
            }
        }

        pDragonStateData1->m4C_interpolationStep = 0;
        pDragonStateData1->m4E_interpolationLength = interpolationLength;
        return 1;
    }
    else
    {
        pDragonStateData1->m4C_interpolationStep = 0;
        pDragonStateData1->m4E_interpolationLength = 0;
        return 0;
    }
}

void playAnimation(s_3dModel* pDragonStateData1, u8* r5, u32 r6)
{
    if (dragonMenuDragonInitSub2Sub1(pDragonStateData1, r6) && ((pDragonStateData1->mA & 0x200) == 0))
    {
        assert(0);
    }
    else
    {
        dragonFieldTaskInitSub3Sub1(pDragonStateData1, r5);
    }
}

void dragonMenuDragonInitSub3(s3DModelAnimData* pDragonStateData2)
{
    for (int i = 0; i < pDragonStateData2->countAnims; i++)
    {
        pDragonStateData2->m8_runtimeAnimData[i].m0_root.zero();
        pDragonStateData2->m8_runtimeAnimData[i].m_vec_C.zero();
        pDragonStateData2->m8_runtimeAnimData[i].m_vec_18.zero();
        pDragonStateData2->m8_runtimeAnimData[i].m24_rootDelta.zero();
    }
}

void updateDragonMovementFromControllerType1Sub2Sub1(s3DModelAnimData* r4, fixedPoint r5)
{
    r4->m8_runtimeAnimData->m24_rootDelta[0] += r5;
}

void updateDragonMovementFromControllerType1Sub2Sub2(s3DModelAnimData* r4, fixedPoint r5)
{
    r4->m8_runtimeAnimData->m24_rootDelta[1] += r5;
}

void updateDragonMovementFromControllerType1Sub2Sub3(s3DModelAnimData* r4, fixedPoint r5)
{
    r4->m8_runtimeAnimData->m24_rootDelta[2] += r5;
}

void setupVdp1LocalCoordinatesAndClipping()
{
    setVdp1VramU16(graphicEngineStatus.m405C.setLocalCoordinatesEA + 0xC, graphicEngineStatus.m405C.localCoordinatesX);
    setVdp1VramU16(graphicEngineStatus.m405C.setLocalCoordinatesEA + 0xE, graphicEngineStatus.m405C.localCoordinatesY);

    if (graphicEngineStatus.m405C.VDP1_X1 > 0)
    {
        setVdp1VramU16(graphicEngineStatus.m405C.setClippingCoordinatesEA + 0xC, graphicEngineStatus.m405C.VDP1_X1);
    }
    else
    {
        setVdp1VramU16(graphicEngineStatus.m405C.setClippingCoordinatesEA + 0xC, 0);
    }

    if (graphicEngineStatus.m405C.VDP1_Y1 > 0)
    {
        setVdp1VramU16(graphicEngineStatus.m405C.setClippingCoordinatesEA + 0xE, graphicEngineStatus.m405C.VDP1_Y1);
    }
    else
    {
        setVdp1VramU16(graphicEngineStatus.m405C.setClippingCoordinatesEA + 0xE, 0);
    }

    if (graphicEngineStatus.m405C.VDP1_X2 < 0x160)
    {
        setVdp1VramU16(graphicEngineStatus.m405C.setClippingCoordinatesEA + 0x14, graphicEngineStatus.m405C.VDP1_X2);
    }
    else
    {
        setVdp1VramU16(graphicEngineStatus.m405C.setClippingCoordinatesEA + 0x14, 0x160);
    }

    if (graphicEngineStatus.m405C.VDP1_Y2 < 0xE0)
    {
        setVdp1VramU16(graphicEngineStatus.m405C.setClippingCoordinatesEA + 0x16, graphicEngineStatus.m405C.VDP1_Y2);
    }
    else
    {
        setVdp1VramU16(graphicEngineStatus.m405C.setClippingCoordinatesEA + 0x16, 0xE0);
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
    lightSetup.m10 = r4;
    lightSetup.m14 = r5;
    lightSetup.m18 = r6;

    generateMasterLightFalloffMap(r4, r5, r6);

    //addSlaveCommand(0,0,0, copyFalloffTableToSlave);
}

void generateCameraMatrixSub1(sVec3_FP& r4, fixedPoint(&r5)[2])
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
            r5[0] = atan2_FP(r4[1], r0);
        }
        else
        {
            r5[0] = -atan2_FP(-r4[1], r0);
        }

        r5[1] = atan2_FP(-r4[0], -r4[2]);
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

void transformVecByCurrentMatrix(sVec3_FP& r4, sVec3_FP& r5)
{
    transformVec(r4, r5, *pCurrentMatrix);
}

void generateCameraMatrix(s_cameraProperties2* r4, const sVec3_FP& r13, const sVec3_FP& r6, const sVec3_FP& r7)
{
    fixedPoint var_4[2];
    sVec3_FP var_24;
    sMatrix4x3 var_30;

    sVec3_FP var_18 = r6 - r13;
    sVec3_FP var_C = r7 - r13;

    r4->m20[0] = r4->m_rotation[0];
    r4->m20[1] = r4->m_rotation[1];
    r4->m20[2] = r4->m_rotation[2];

    r4->m14 = r4->m0_position;

    generateCameraMatrixSub1(var_18, var_4);

    r4->m_rotation[0] = var_4[0] >> 16;
    r4->m_rotation[1] = var_4[1] >> 16;

    initMatrixToIdentity(&var_30);
    rotateMatrixX(-r4->m_rotation[0], &var_30);
    rotateMatrixY(-r4->m_rotation[1], &var_30);

    transformVec(var_C, var_24, var_30);

    r4->m_rotation[2] = atan2(-var_24[0], var_24[1]);

    r4->m0_position[0] = r13[0];
    var_24[0] = -r13[0];
    
    r4->m0_position[1] = r13[1];
    var_24[1] = -r13[1];

    r4->m0_position[2] = r13[2];
    var_24[2] = -r13[2];

    resetMatrixStack();

    rotateCurrentMatrixZ(-r4->m_rotation[2]);
    rotateCurrentMatrixX(-r4->m_rotation[0]);

    copyMatrix(pCurrentMatrix, &r4->m88);

    rotateCurrentMatrixY(-r4->m_rotation[1]);

    translateCurrentMatrix(&var_24);

    rotateMatrixY(0x800, &r4->m88);

    copyMatrix(&r4->m28[0], &r4->m28[1]);

    initMatrixToIdentity(&r4->m28[0]);
    translateMatrix(&r4->m0_position, &r4->m28[0]);
    rotateMatrixY(r4->m_rotation[1], &r4->m28[0]);
    rotateMatrixX(r4->m_rotation[0], &r4->m28[0]);
    rotateMatrixZ(r4->m_rotation[2], &r4->m28[0]);

    r4->m28[0].matrix[2] = -r4->m28[0].matrix[2];
    r4->m28[0].matrix[6] = -r4->m28[0].matrix[6];
    r4->m28[0].matrix[10] = -r4->m28[0].matrix[10];
}

void resetCameraProperties2(s_cameraProperties2* r4)
{
    r4->m0_position[2] = 0;
    r4->m0_position[1] = 0;
    r4->m0_position[0] = 0;

    r4->m_rotation[2] = 0;
    r4->m_rotation[1] = 0;
    r4->m_rotation[0] = 0;

    r4->m14[0] = r4->m0_position[0];
    r4->m14[1] = r4->m0_position[1];
    r4->m14[2] = r4->m0_position[2];

    r4->m20[0] = r4->m_rotation[0];
    r4->m20[1] = r4->m_rotation[1];
    r4->m20[2] = r4->m_rotation[2];

    initMatrixToIdentity(&r4->m28[0]);
    initMatrixToIdentity(&r4->m28[1]);
}

void dragonMenuDragonInit(p_workArea pTypelessWorkArea)
{
    s_dragonMenuDragonWorkArea* pWorkArea = static_cast<s_dragonMenuDragonWorkArea*>(pTypelessWorkArea);

    dragonMenuDragonInitSub1(&pWorkArea->m34);

    pWorkArea->m30 = getVdp2VramU16(0x25002);
    pWorkArea->m28 = gDragonState->m28_dragon3dModel.m30_pCurrentAnimation;
    pWorkArea->m2C = gDragonState->m28_dragon3dModel.m16;

    pWorkArea->m0 = loadDragonModel(pWorkArea, mainGameState.gameStats.m1_dragonLevel);

    pWorkArea->m4 = &dragonData3[mainGameState.gameStats.m1_dragonLevel];
    pWorkArea->modelTranslation[1] = 0x4000;
    pWorkArea->modelTranslation[2] = 0xA000;
    pWorkArea->modelRotation[0] = fixedPoint(0xE38E38);
    pWorkArea->modelRotation[1] = fixedPoint(0x638E38E);
    pWorkArea->modelRotation[2] = fixedPoint(0xF555555);

    playAnimation(&gDragonState->m28_dragon3dModel, gDragonState->m0_pDragonModelRawData + READ_BE_U32(gDragonState->m0_pDragonModelRawData + gDragonState->m20_dragonAnimOffsets[0]), 0); // Todo: is the [0] correct?

    dragonMenuDragonInitSub3(&gDragonState->m78_animData);

    graphicEngineStatus.m405C.localCoordinatesX = 120;
    graphicEngineStatus.m405C.localCoordinatesY = 112;

    setupVdp1LocalCoordinatesAndClipping();

    graphicEngineStatus.m406C = 0x2000;
    graphicEngineStatus.m408C = FP_Div(0x10000, 0x2000);

    graphicEngineStatus.m4070_farClipDistance = 0x100000;
    graphicEngineStatus.m4094 = FP_Div(0x8000, 0x100000);

    graphicEngineStatus.m4090 = graphicEngineStatus.m4094 << 8;

    generateLightFalloffMap(0x30102, 0, 0);

    initVDP1Projection(fixedPoint(0x1C71C71), 0);

    resetCameraProperties2(&cameraProperties2);

    generateCameraMatrix(&cameraProperties2, defaultCameraVectors[0], defaultCameraVectors[1], defaultCameraVectors[2]);
}

struct sCurrentLightVector
{
    sVec3_FP m_lightVector;

    u16 m_color[3];
}currentLightVector_M;

void setLightVector_M(fixedPoint r4, fixedPoint r5, fixedPoint r6, u32 r7)
{
    currentLightVector_M.m_lightVector[0] = r4;
    currentLightVector_M.m_lightVector[1] = r5;
    currentLightVector_M.m_lightVector[2] = r6;

    currentLightVector_M.m_color[2] = r7 & 0xFF;
    currentLightVector_M.m_color[1] = (r7 >> 8) & 0xFF;
    currentLightVector_M.m_color[0] = (r7 >> 16) & 0xFF;
}

void setupLight(fixedPoint r4, fixedPoint r5, fixedPoint r6, u32 r7)
{
    lightSetup.m0 = r4;
    lightSetup.m4 = r5;
    lightSetup.m8 = r6;
    lightSetup.mC = r7;

    setLightVector_M(-r4 >> 4, -r5 >> 4, -r6 >> 4, r7);

    //addSlaveCommand(X, X, X, setLightVector_S);
}

void updateAnimationMatricesSub1(s3DModelAnimData* r4, s_3dModel* r5)
{
    u32 r9 = r5->m12_numBones;
    if (r9)
    {
        std::vector<sMatrix4x3>::iterator r14 = r4->m4_boneMatrices->begin();
        if (r5->m48_poseDataInterpolation.size())
        {
            std::vector<sPoseDataInterpolation>::iterator r13 = r5->m48_poseDataInterpolation.begin();

            do
            {
                initMatrixToIdentity(&(*r14));
                translateMatrix(&r13->m0_translation, &(*r14));
                rotateMatrixZYX(&r13->mC_rotation, &(*r14));
                r13++;
                r14++;
            } while (--r9);
        }
        else
        {
            std::vector<sPoseData>::iterator r13 = r5->m2C_poseData.begin();

            do 
            {
                initMatrixToIdentity(&(*r14));
                translateMatrix(&r13->m0_translation, &(*r14));
                rotateMatrixZYX(&r13->mC_rotation, &(*r14));
                r13++;
                r14++;
            } while (--r9);
        }
    }
}

void updateAnimationMatricesSub2Sub1(s_runtimeAnimData* r4, sVec3_FP& r5, sVec3_FP& r6)
{
    r4->m0_root.m_value[0] = MTH_Mul(r4->m_factors.m_vec0[0], r6.m_value[0] - r5.m_value[0]) - MTH_Mul(0x10000 - r4->m_factors.m_vecC[0], r4->m_vec_C.m_value[0]);
    r4->m0_root.m_value[1] = MTH_Mul(r4->m_factors.m_vec0[1], r6.m_value[1] - r5.m_value[1]) - MTH_Mul(0x10000 - r4->m_factors.m_vecC[1], r4->m_vec_C.m_value[1]);
    r4->m0_root.m_value[2] = MTH_Mul(r4->m_factors.m_vec0[2], r6.m_value[2] - r5.m_value[2]) - MTH_Mul(0x10000 - r4->m_factors.m_vecC[2], r4->m_vec_C.m_value[2]);
}

void updateAnimationMatricesSub2Sub2(s_runtimeAnimData* r4)
{
    r4->m_vec_C += r4->m0_root;
}

void updateAnimationMatricesSub2Sub3(s_runtimeAnimData* r4)
{
    r4->m_vec_18 += r4->m_vec_C;

    if (r4->m_vec_18.m_value[0].asS32() & 0x8000000)
    {
        r4->m_vec_18.m_value[0] = r4->m_vec_18.m_value[0].asS32() | 0xF0000000;
    }
    else
    {
        r4->m_vec_18.m_value[0] = r4->m_vec_18.m_value[0].asS32() & 0xFFFFFFF;
    }

    if (r4->m_vec_18.m_value[1].asS32() & 0x8000000)
    {
        r4->m_vec_18.m_value[1] = r4->m_vec_18.m_value[1].asS32() | 0xF0000000;
    }
    else
    {
        r4->m_vec_18.m_value[1] = r4->m_vec_18.m_value[1].asS32() & 0xFFFFFFF;
    }

    if (r4->m_vec_18.m_value[2].asS32() & 0x8000000)
    {
        r4->m_vec_18.m_value[2] = r4->m_vec_18.m_value[2].asS32() | 0xF0000000;
    }
    else
    {
        r4->m_vec_18.m_value[2] = r4->m_vec_18.m_value[2].asS32() & 0xFFFFFFF;
    }

    if (r4->m_vec_18.m_value[0] > r4->m_factors.m_max[0])
    {
        r4->m_vec_18.m_value[0] = r4->m_factors.m_max[0];
        r4->m_vec_C[0] = 0;
    }
    else
    {
        if (r4->m_vec_18.m_value[0] < r4->m_factors.m_min[0])
        {
            r4->m_vec_18.m_value[0] = r4->m_factors.m_min[0];
            r4->m_vec_C[0] = 0;
        }
    }

    if (r4->m_vec_18.m_value[1] > r4->m_factors.m_max[1])
    {
        r4->m_vec_18.m_value[1] = r4->m_factors.m_max[1];
        r4->m_vec_C[1] = 0;
    }
    else
    {
        if (r4->m_vec_18.m_value[1] < r4->m_factors.m_min[1])
        {
            r4->m_vec_18.m_value[1] = r4->m_factors.m_min[1];
            r4->m_vec_C[1] = 0;
        }
    }

    if (r4->m_vec_18.m_value[2] > r4->m_factors.m_max[2])
    {
        r4->m_vec_18.m_value[2] = r4->m_factors.m_max[2];
        r4->m_vec_C[2] = 0;
    }
    else
    {
        if (r4->m_vec_18.m_value[2] < r4->m_factors.m_min[2])
        {
            r4->m_vec_18.m_value[2] = r4->m_factors.m_min[2];
            r4->m_vec_C[2] = 0;
        }
    }
}

sVec3_FP updateAnimationMatricesSub2Vec = { 0,0,0 };

void updateAnimationMatricesSub2(s3DModelAnimData* r4)
{
    s_runtimeAnimData* r14 = r4->m8_runtimeAnimData;

    updateAnimationMatricesSub2Sub1(r14, r14->m_vec_18, updateAnimationMatricesSub2Vec);

    r14->m0_root += r14->m24_rootDelta;

    updateAnimationMatricesSub2Sub2(r14);
    updateAnimationMatricesSub2Sub3(r14);

    r14->m24_rootDelta[0] = 0;
    r14->m24_rootDelta[1] = 0;
    r14->m24_rootDelta[2] = 0;
}

void updateAnimationMatricesSub3(s3DModelAnimData* r4)
{
    PDS_unimplemented("updateAnimationMatricesSub3");
}

void updateAnimationMatrices(s3DModelAnimData* r4, s_3dModel* r5)
{
    updateAnimationMatricesSub1(r4, r5);
    updateAnimationMatricesSub2(r4);
    updateAnimationMatricesSub3(r4);
}

struct s_animLoop
{
    u16 m_count;
    u16 m_values[];
};

s_animLoop dragonAnimLoop_0 = {
    1,
    {0}
};

s_animLoop dragonAnimLoop_0_2 = {
    2,
    { 0, 2 }
};

s_animLoop dragonAnimLoop_0_2_2 = {
    3,
    { 0, 2, 2 }
};

s_animLoop dragonAnimLoop_0_0_2 = {
    3,
    { 0, 0, 2 }
};

s_animLoop dragonAnimLoop_0_2_2_2 = {
    4,
    { 0, 2, 2, 2 }
};

s_animLoop dragonAnimLoop_0_0_2_2 = {
    4,
    { 0, 0, 2, 2 }
};

s_animLoop* dragonAnimLoop[DR_LEVEL_MAX][DR_ARCHETYPE_MAX] =
{
    //DR_LEVEL_0_BASIC_WING
    {
        &dragonAnimLoop_0,
        &dragonAnimLoop_0,
        &dragonAnimLoop_0,
        &dragonAnimLoop_0,
        &dragonAnimLoop_0,
    },

    {
        &dragonAnimLoop_0,
        &dragonAnimLoop_0,
        &dragonAnimLoop_0_2,
        &dragonAnimLoop_0_2_2,
        &dragonAnimLoop_0_0_2,
    },

    {
        &dragonAnimLoop_0,
        &dragonAnimLoop_0,
        &dragonAnimLoop_0_2,
        &dragonAnimLoop_0_2_2,
        &dragonAnimLoop_0_0_2,
    },

    {
        &dragonAnimLoop_0_2,
        &dragonAnimLoop_0,
        &dragonAnimLoop_0_2_2,
        &dragonAnimLoop_0_2_2_2,
        &dragonAnimLoop_0_0_2,
    },

    {
        &dragonAnimLoop_0_2,
        &dragonAnimLoop_0,
        &dragonAnimLoop_0_2_2,
        &dragonAnimLoop_0_2_2_2,
        &dragonAnimLoop_0_0_2,
    },

    {
        &dragonAnimLoop_0_2,
        &dragonAnimLoop_0,
        &dragonAnimLoop_0_2_2,
        &dragonAnimLoop_0_2_2_2,
        &dragonAnimLoop_0_0_2,
    },

    {
        &dragonAnimLoop_0_2,
        &dragonAnimLoop_0_2,
        &dragonAnimLoop_0_2,
        &dragonAnimLoop_0_2,
        &dragonAnimLoop_0_2,
    },

    {
        &dragonAnimLoop_0,
        &dragonAnimLoop_0_0_2,
        &dragonAnimLoop_0_0_2_2,
        &dragonAnimLoop_0_2_2,
        &dragonAnimLoop_0,
    },

    {
        &dragonAnimLoop_0,
        &dragonAnimLoop_0,
        &dragonAnimLoop_0,
        &dragonAnimLoop_0,
        &dragonAnimLoop_0,
    },
};

void dragonMenuDragonUpdate(p_workArea pTypelessWorkArea)
{
    s_dragonMenuDragonWorkArea* pWorkArea = static_cast<s_dragonMenuDragonWorkArea*>(pTypelessWorkArea);

    gDragonState->m10_cursorX = mainGameState.gameStats.dragonCursorX;
    gDragonState->m12_cursorY = mainGameState.gameStats.dragonCursorY;
    gDragonState->m1C_dragonArchetype = mainGameState.gameStats.dragonArchetype;

    fixedPoint r0 = sqrt_F(FP_Pow2(performDivision(0x880, mainGameState.gameStats.dragonCursorX << 16)) + FP_Pow2(performDivision(0x880, mainGameState.gameStats.dragonCursorY << 16)));

    fixedPoint r6;
    s32 r10;
    s32 r9;

    if (r0)
    {
        assert(0);
    }
    else
    {
        setupLight(0, 0, 0x10000, 0x161918);
    }

    if (graphicEngineStatus.m4514.m0[0].m0_current.m6_buttonDown & 0x800)
    {
        pWorkArea->modelRotation[1] += 0x16C16C;
    }

    if (graphicEngineStatus.m4514.m0[0].m0_current.mE & 0x800)
    {
        pWorkArea->modelRotation[1] -= 0x5B05B0;
    }

    if (graphicEngineStatus.m4514.m0[0].m0_current.m6_buttonDown & 0x8000)
    {
        pWorkArea->modelRotation[1] += 0x16C16C;
    }

    if (graphicEngineStatus.m4514.m0[0].m0_current.mE & 0x8000)
    {
        pWorkArea->modelRotation[1] += 0x16C16C;
    }

    if (graphicEngineStatus.m4514.m0[1].m0_current.m6_buttonDown & 0x8000)
    {
        assert(0);
    }
    else
    {
        if (graphicEngineStatus.m4514.m0[1].m0_current.m6_buttonDown & 0x10)
        {
            assert(0);
        }
        if (graphicEngineStatus.m4514.m0[1].m0_current.m6_buttonDown & 0x20)
        {
            assert(0);
        }
        if (graphicEngineStatus.m4514.m0[1].m0_current.m6_buttonDown & 0x40)
        {
            assert(0);
        }
        if (graphicEngineStatus.m4514.m0[1].m0_current.m6_buttonDown & 0x80)
        {
            assert(0);
        }
        if (graphicEngineStatus.m4514.m0[1].m0_current.m6_buttonDown & 0x4000)
        {
            assert(0);
        }
        if (graphicEngineStatus.m4514.m0[1].m0_current.m6_buttonDown & 0x4)
        {
            assert(0);
        }
    }

    if (readKeyboardToggle(0x10B))
    {
        assert(0);
    }

    if (readKeyboardToggle(0x10C))
    {
        assert(0);
    }

    if (readKeyboardToggle(0x107))
    {
        assert(0);
    }

    if (readKeyboardToggle(0x108))
    {
        assert(0);
    }

    if (pWorkArea->m8 == 0)
    {
        int r3;
        if (gDragonState->m28_dragon3dModel.m30_pCurrentAnimation == NULL)
        {
            r3 = 0;
        }
        else
        {
            r3 = READ_BE_S16(gDragonState->m28_dragon3dModel.m30_pCurrentAnimation + 4);
        }
        r3--;
        if (gDragonState->m28_dragon3dModel.m16 >= r3)
        {
            s_animLoop* pAnimLoop = dragonAnimLoop[gDragonState->mC_dragonType][gDragonState->m1C_dragonArchetype];

            if (pWorkArea->mC + 1 < pAnimLoop->m_count)
            {
                pWorkArea->mC++;
            }
            else
            {
                pWorkArea->mC = 0;
            }

            u16 animIndex = pAnimLoop->m_values[pWorkArea->mC];
            u32 animOffset = gDragonState->m20_dragonAnimOffsets[animIndex];
            playAnimation(&gDragonState->m28_dragon3dModel, gDragonState->m0_pDragonModelRawData + READ_BE_U32(gDragonState->m0_pDragonModelRawData + animOffset), 0);
        }
    }

    updateAndInterpolateAnimation(&gDragonState->m28_dragon3dModel);
    updateAnimationMatrices(&gDragonState->m78_animData, &gDragonState->m28_dragon3dModel);
    morphDragon(pWorkArea->m0, &gDragonState->m28_dragon3dModel, pWorkArea->m0->MCBOffsetInDram, pWorkArea->m4, mainGameState.gameStats.dragonCursorX, mainGameState.gameStats.dragonCursorY);
}

void submitModelAndShadowModelToRendering(s_3dModel* p3dModel, u32 modelIndex, u32 shadowModelIndex, sVec3_FP* translation, sVec3_FP* rotation, fixedPoint shadowHeight)
{
    sVec3_FP var_18;
    sMatrix4x3 modelMatrix; // 28 - 58

    initMatrixToIdentity(&modelMatrix);
    translateMatrix(translation, &modelMatrix);
    rotateMatrixYXZ(rotation, &modelMatrix);

    if (shadowModelIndex)
    {
        fixedPoint old4 = modelMatrix.matrix[4];
        fixedPoint old5 = modelMatrix.matrix[5];
        fixedPoint old6 = modelMatrix.matrix[6];
        fixedPoint old7 = modelMatrix.matrix[7];

        modelMatrix.matrix[4] = 0;
        modelMatrix.matrix[5] = 0;
        modelMatrix.matrix[6] = 0;
        modelMatrix.matrix[7] = shadowHeight;

        pushCurrentMatrix();
        multiplyCurrentMatrix(&modelMatrix);
        p3dModel->mC_modelIndexOffset = shadowModelIndex;
        p3dModel->m18_drawFunction(p3dModel);
        popMatrix();

        modelMatrix.matrix[4] = old4;
        modelMatrix.matrix[5] = old5;
        modelMatrix.matrix[6] = old6;
        modelMatrix.matrix[7] = old7;
    }

    pushCurrentMatrix();
    multiplyCurrentMatrix(&modelMatrix);
    p3dModel->mC_modelIndexOffset = modelIndex;
    p3dModel->m18_drawFunction(p3dModel);
    popMatrix();
}

void dragonMenuDragonDraw(p_workArea pTypelessWorkArea)
{
    s_dragonMenuDragonWorkArea* pWorkArea = static_cast<s_dragonMenuDragonWorkArea*>(pTypelessWorkArea);
    submitModelAndShadowModelToRendering(&gDragonState->m28_dragon3dModel, gDragonState->m14_modelIndex, gDragonState->m18_shadowModelIndex, &pWorkArea->modelTranslation, &pWorkArea->modelRotation, 0);
}

void dragonMenuDragonDelete(p_workArea pTypelessWorkArea)
{
    PDS_unimplemented("dragonMenuDragonDelete");
}

s_taskDefinition dragonMenuDragonTaskDefinition = { dragonMenuDragonInit, dragonMenuDragonUpdate, dragonMenuDragonDraw, dragonMenuDragonDelete, "dragonMenuDragon" };

p_workArea createDragonMenuMorhTask(p_workArea pWorkArea)
{
    return createSubTask(pWorkArea, &dragonMenuDragonTaskDefinition, new s_dragonMenuDragonWorkArea);
}
