#include "PDS.h"
#include "battleDragon.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"
#include "battleDebug.h"
#include "battleGrid.h"
#include "kernel/fileBundle.h"
#include "kernel/debug/trace.h"

void s_battleDragon_InitSub4Sub0()
{
    updateAndInterpolateAnimation(&gDragonState->m28_dragon3dModel);
    updateAnimationMatrices(&gDragonState->m78_animData, &gDragonState->m28_dragon3dModel);
}

void s_battleDragon_InitSub4(int arg0, int arg1)
{
    playAnimation(&gDragonState->m28_dragon3dModel, gDragonState->m0_pDragonModelBundle->getAnimation(gDragonState->m20_dragonAnimOffsets[arg0]), arg1);
    s_battleDragon_InitSub4Sub0();
}

void s_battleDragon_InitSub5Sub0(s_battleDragon_8C* pThis)
{
    transformAndAddVec(*pThis->m4, pThis->m10, cameraProperties2.m28[0]);
}

void s_battleDragon_InitSub5(s_battleDragon_8C* pThis, s_battleDragon* param2, sVec3_FP* param3, s32 param4, s32 param5)
{
    pThis->m0 = param2;
    pThis->m4 = param3;
    pThis->m4C = param4;
    pThis->m40 = *param3;
    s_battleDragon_InitSub5Sub0(pThis);
    pThis->m8 = &pThis->m10;
    pThis->mC = 0;
    pThis->m50 = param5;
    pThis->m58 = 0;
    pThis->m5A = 0;
    pThis->m5E = 0;
    pThis->m5F = 0;
    pThis->m5C = 0;
    pThis->m54 = 0;
    pThis->m60 = 10;
}

void s_battleDragon_InitSub3(s_loadRiderWorkArea* pRider, s16 param2, s32 param3)
{
    playAnimationGeneric(&pRider->m18_3dModel, pRider1State->m0_riderBundle->getAnimation(param2), param3);
    updateAndInterpolateAnimation(&pRider->m18_3dModel);
}

void s_battleDragon_InitSub1(s_loadRiderWorkArea* pRider, s16 param2, s32 param3)
{
    playAnimationGeneric(&pRider->m18_3dModel, pRider1State->m0_riderBundle->getAnimation(param2), param3);
    updateAndInterpolateAnimation(&pRider->m18_3dModel);
}

static void s_battleDragon_Init(s_battleDragon* pThis)
{
    getBattleManager()->m10_battleOverlay->m18_dragon = pThis;

    pThis->m8_position = getBattleManager()->m10_battleOverlay->m4_battleEngine->m104_dragonStartPosition;
    resetMatrixStack();
    pThis->m1CE = 0;

    if (s_battleDragon_InitSub0() == 0)
    {
        assert(0);
    }
    else
    {
        s_battleDragon_InitSub3(pRider1State, readSaturnS16(gCurrentBattleOverlay->getSaturnPtr(0x60ae368) + getBattleManager()->m10_battleOverlay->m4_battleEngine->m22C_battleDirection * 2), 0);
        s_battleDragon_InitSub3(pRider2State, readSaturnS16(gCurrentBattleOverlay->getSaturnPtr(0x60ae370) + getBattleManager()->m10_battleOverlay->m4_battleEngine->m22C_battleDirection * 2), 0);
        pThis->m1CC = 2;
    }

    s_battleDragon_InitSub4(pThis->m1CC, 0);
    s_battleDragon_InitSub5(&pThis->m8C, pThis, &pThis->m1A4, 0x1000, 0xf0000001);
    pThis->m1C8 = 0x111111;

    if ((mainGameState.gameStats.m1_dragonLevel == 6) && (pThis->m24C == 0))
    {
        assert(0);
    }
    else
    {
        pThis->m24C = 0;
    }

    FunctionUnimplemented();
}

void s_battleDragon_UpdateSub0()
{
    if (gDragonState->m88)
    {
        FunctionUnimplemented();
    }
}

void s_battleDragon_UpdateSub1(s_battleDragon* pThis)
{
    s_battleEngine* pBattleEngine = getBattleManager()->m10_battleOverlay->m4_battleEngine;
    if (getBattleManager()->m10_battleOverlay->m10_inBattleDebug->mFlags[0xF])
    {
        assert(0);
    }

    sVec3_FP iVar3 = pThis->m8_position;
    sVec3_FP local_44 = pBattleEngine->m104_dragonStartPosition + pBattleEngine->m164;
    local_44 -= pBattleEngine->m234;
    
    pushCurrentMatrix();
    translateCurrentMatrix(pBattleEngine->m234);
    rotateCurrentMatrixZYX(pBattleEngine->m220);
    sVec3_FP auStack56;
    transformAndAddVecByCurrentMatrix(&local_44, &auStack56);
    transformAndAddVec(auStack56, pThis->m8_position, cameraProperties2.m28[0]);

    if (isTraceEnabled())
    {
        addTraceLog(pBattleEngine->m234, "pBattleEngine->m234");
        addTraceLog(pBattleEngine->m220, "pBattleEngine->m220");
        addTraceLog(cameraProperties2.m28[0], "cameraProperties2.m28[0]");
        addTraceLog(pThis->m8_position, "DragonPosition");
    }

    popMatrix();
    pThis->m5C_deltaPosition = pThis->m8_position - iVar3;
}

void s_battleDragon_UpdateSub2Sub0(s_battleDragon* pThis)
{
    int uVar5 = pThis->m84 & 0x1F;
    int uVar7 = pThis->m84 & 0x20;
    if (mainGameState.gameStats.m1_dragonLevel == 8)
    {
        assert(0);
    }
    
    if (pThis->m1C4 & 0x40)
    {
        assert(0);
    }

    if (uVar7 == 0)
    {
        pThis->m1C4 &= ~1;
    }
    else
    {
        pThis->m1C4 |= 1;
    }

    if ((pThis->m84 & 2) == 0)
    {
        if ((pThis->m84 & 4) == 0)
        {
            if (pThis->m84 & 1)
            {
                assert(0);
            }
            if (pThis->m84 & 0x10)
            {
                assert(0);
            }
            if(pThis->m84 & 0x8)
            {
                assert(0);
            }
        }
        else
        {
            pThis->m1CE = -1;
            pThis->m1C4 |= 4;
        }
    }
    else
    {
        pThis->m1CE = -1;
        pThis->m1C4 |= 2;
    }

    if ((pThis->m1C4 & 2) == 0)
    {
        if ((pThis->m1C4 & 4) == 0)
        {
            if (uVar5)
                return;

            if ((mainGameState.gameStats.maxHP / 4) == 0)
            {
                FunctionUnimplemented();
            }
            else
            {
                FunctionUnimplemented();
            }

            s_battleDragon_InitSub4Sub0();
            if (gDragonState->mC_dragonType == 8)
            {
                assert(0);
            }
            else
            {
                int iVar2 = pThis->m1CC;
                if (((((iVar2 == 0) || (iVar2 == 1)) || (iVar2 == 5)) || (iVar2 == 6)) &&
                    ((gDragonState->m28_dragon3dModel).m16_previousAnimationFrame == 0xc)) {
                    playSoundEffect(0xb);
                }
            }
            return;
        }
        else
        {
            pThis->m1CC = 0x11;
            s_battleDragon_InitSub4(pThis->m1CC, 9);
            pThis->m1CE = -1;
            pThis->m1C4 &= ~4;
        }
    }
    else
    {
        pThis->m1CC = 0x10;
        playSoundEffect(0x12);
        s_battleDragon_InitSub4(pThis->m1CC, 9);
        pThis->m1CE = -1;
        pThis->m1C4 &= ~2;
    }
}

void s_battleDragon_UpdateSub2Sub1Sub0()
{
    if (gDragonState->mC_dragonType != 8)
    {
        u16 offset;
        if (s_battleDragon_InitSub0() == 0)
        {
            offset = 0x5C;
        }
        else
        {
            offset = readSaturnS16(gCurrentBattleOverlay->getSaturnPtr(0x60ae368) + getBattleManager()->m10_battleOverlay->m4_battleEngine->m22C_battleDirection * 2);
        }
        s_battleDragon_InitSub3(pRider1State, offset, 0);
    }
    else
    {
        if (s_battleDragon_InitSub0())
        {
            s_battleDragon_InitSub1(pRider1State, readSaturnS16(gCurrentBattleOverlay->getSaturnPtr(0x60ae378) + getBattleManager()->m10_battleOverlay->m4_battleEngine->m22C_battleDirection * 2), 10);
        }
        else
        {
            s_battleDragon_InitSub1(pRider1State, 0x114, 10);
        }
    }
}

void s_battleDragon_UpdateSub2Sub1Sub1()
{
    u16 offset;
    if (s_battleDragon_InitSub0() == 0)
    {
        offset = 0x5C;
    }
    else
    {
        offset = readSaturnS16(gCurrentBattleOverlay->getSaturnPtr(0x60ae370) + getBattleManager()->m10_battleOverlay->m4_battleEngine->m22C_battleDirection * 2);
    }
    s_battleDragon_InitSub3(pRider2State, offset, 0xF);
}

void     s_battleDragon_UpdateSub2Sub1(s_battleDragon* pThis)
{
    s32 uVar4 = pThis->m84;
    if (gDragonState->mC_dragonType == 8)
    {
        assert(0);
    }

    if (uVar4 & 0x2E00)
    {
        assert(0);
    }

    if (uVar4 & 0x5000)
    {
        assert(0);
    }

    if ((pThis->m1C4 & 0x80) == 0)
    {
        if (pThis->m1D0 < 1)
        {
            pThis->m1D0 = 0;
            if ((getBattleManager()->m10_battleOverlay->m18_dragon->m84 & 0x100) == 0)
            {
                if (pThis->m84 & 0x18)
                {
                    s_battleDragon_InitSub3(pRider1State, 0x58, 5);
                }
            }
            else
            {
                s_battleDragon_UpdateSub2Sub1Sub0();
            }
        }
        else
        {
            assert(0);
        }
    }
    else
    {
        assert(0);
    }

    updateAndInterpolateAnimation(&pRider1State->m18_3dModel);
    if ((pThis->m1C4 & 0x100) == 0)
    {
        if ((getBattleManager()->m10_battleOverlay->m18_dragon->m84 & 0x100) == 0)
        {
            if (pThis->m84 & 0x18)
            {
                s_battleDragon_InitSub3(pRider2State, 0x58, 5);
            }
        }
        else
        {
            s_battleDragon_UpdateSub2Sub1Sub1();
        }
    }
    else
    {
        assert(0);
    }

    updateAndInterpolateAnimation(&pRider2State->m18_3dModel);
}

void s_battleDragon_UpdateSub2(s_battleDragon* pThis)
{
    int gridMode = getBattleManager()->m10_battleOverlay->m8_gridTask->m2;
    if ((gridMode == 3) || (gridMode != 4))
        return;

    pThis->m44_deltaRotation[0] += MTH_Mul(pThis->m74_targetRotation[0] - pThis->m14_rotation[0], 0x51E);
    pThis->m44_deltaRotation[0] -= MTH_Mul(pThis->m44_deltaRotation[0], 0x3333);
    pThis->m14_rotation[0] += pThis->m44_deltaRotation[0];

    pThis->m44_deltaRotation[1] += MTH_Mul(pThis->m74_targetRotation[1] - pThis->m14_rotation[1], 0x51E);
    pThis->m44_deltaRotation[1] -= MTH_Mul(pThis->m44_deltaRotation[1], 0x3333);
    pThis->m14_rotation[1] += pThis->m44_deltaRotation[1];

    pThis->m44_deltaRotation[2] += MTH_Mul(pThis->m74_targetRotation[2] - pThis->m14_rotation[2], 0x51E);
    pThis->m44_deltaRotation[2] -= MTH_Mul(pThis->m44_deltaRotation[2], 0x3333);
    pThis->m14_rotation[2] += pThis->m44_deltaRotation[2];

    s_battleDragon_UpdateSub2Sub0(pThis);
    s_battleDragon_UpdateSub2Sub1(pThis);
}

void s_battleDragon_UpdateSub3(s_battleDragon* pThis)
{
    s_battleEngine* pBattleEngine = getBattleManager()->m10_battleOverlay->m4_battleEngine;

    fixedPoint stepY;
    fixedPoint stepX;
    fixedPoint stepZ;
    fixedPoint rotationX;
    fixedPoint rotationZ;
    if (s_battleDragon_InitSub0() == 0)
    {
        assert(0);
    }
    else
    {
        stepX = MTH_Mul_5_6(getCos(pBattleEngine->m440_battleDirectionAngle.getInteger()), -pThis->m1C8, pBattleEngine->m1BC_dragonYaw);
        stepY = MTH_Mul(-pThis->m1C8, pBattleEngine->m1B8_dragonPitch);
        stepZ = MTH_Mul_5_6(getCos(pBattleEngine->m440_battleDirectionAngle.getInteger()), -pThis->m1C8, pBattleEngine->m1BC_dragonYaw);

        rotationX = MTH_Mul(0xE38E38, pBattleEngine->m1B8_dragonPitch);
        MTH_Mul_5_6(getCos(pBattleEngine->m440_battleDirectionAngle.getInteger()), -0x2000000, pBattleEngine->m1BC_dragonYaw);
        rotationZ = MTH_Mul_5_6(getCos(pBattleEngine->m440_battleDirectionAngle.getInteger()), -0x1555555, -pBattleEngine->m1BC_dragonYaw);
    }

    if ((pThis->m1C4 & 8) == 0)
    {
        pThis->m74_targetRotation[0] = rotationX;
        pThis->m74_targetRotation[2] = rotationZ;
    }

    incrementAnimationRootY(&gDragonState->m78_animData, stepY);
    incrementAnimationRootX(&gDragonState->m78_animData, stepX);
    incrementAnimationRootZ(&gDragonState->m78_animData, stepZ);
}

void s_battleDragon_UpdateSub4(s_battleDragon* pThis)
{
    if ((pThis->m1C4 & 8) == 0)
    {
        if (pThis->mDC & 0x80000)
        {
            assert(0);
        }

        if (getBattleManager()->m10_battleOverlay->m4_battleEngine->m188_flags.m1000)
        {
            assert(0);
        }
    }
}

void s_battleDragon_UpdateSub5(s_battleDragon* pThis)
{
    if (pThis->m1C4 & 8)
    {
        assert(0);
    }
}

void s_battleDragon_UpdateSub6()
{
    if (getBattleManager()->m10_battleOverlay->m4_battleEngine->m188_flags.m4000000)
    {
        assert(0);
    }

    if (getBattleManager()->m10_battleOverlay->m4_battleEngine->m188_flags.m2000000)
    {
        assert(0);
    }
}

static void s_battleDragon_Update(s_battleDragon* pThis)
{
    s_battleDragon_UpdateSub0();
    s_battleDragon_UpdateSub1(pThis);
    pThis->m84 = pThis->m88;
    pThis->m88 = 0;
    s_battleDragon_UpdateSub2(pThis);
    pThis->m14_rotation += pThis->m44_deltaRotation;
    s_battleDragon_UpdateSub3(pThis);
    if ((mainGameState.gameStats.m1_dragonLevel == 6) && (pThis->m24C != 0))
    {
        assert(0);
    }
    s_battleDragon_UpdateSub4(pThis);
    s_battleDragon_UpdateSub5(pThis);
    s_battleDragon_UpdateSub6();
}

static u32 s_battleDragon_UpdateAnimationState(s_battleDragon* pThis)
{
    FunctionUnimplemented();
    return 0;
}

fixedPoint s_battleDragon_getRiderRotationSub0(s32 inValue)
{
    if (inValue > -1)
        return (readSaturnS16(gCommonFile.getSaturnPtr(0x021be80) + (inValue / 16))) * 0x10000;
    return (readSaturnS16(gCommonFile.getSaturnPtr(0x021be80) + (-inValue / 16))) * -0x10000;
}

void s_battleDragon_getRiderRotation(sVec3_FP& outputRotation, const sMatrix4x3& inMatrix)
{
    outputRotation[0] = s_battleDragon_getRiderRotationSub0(-inMatrix.matrix[6]);
    outputRotation[1] = atan2_FP(inMatrix.matrix[2], inMatrix.matrix[10]);
    outputRotation[2] = atan2_FP(inMatrix.matrix[4], inMatrix.matrix[5]);
}

void s_battleDragon_UpdateRider1PositionSub0(sVec3_FP& output, int index)
{
    sSaturnPtr pHotpointData = gCurrentBattleOverlay->getSaturnPtr(0x60ade04);
    sSaturnPtr pDragonHotpointData = readSaturnEA(pHotpointData + 4 * gDragonState->mC_dragonType);
    s8 data0 = readSaturnS8(pDragonHotpointData + index * 2 + 0);
    s8 data1 = readSaturnS8(pDragonHotpointData + index * 2 + 1);

    const sVec3_FP* pData = &gDragonState->m28_dragon3dModel.m44_hotpointData[data0][data1];
    if (pData)
    {
        transformAndAddVec(*pData, output, cameraProperties2.m28[0]);
    }
}

void s_battleDragon_UpdateRider1Position()
{
    for (int i = 0; i < 6; i++)
    {
        s_battleDragon_UpdateRider1PositionSub0(getBattleManager()->m10_battleOverlay->m18_dragon->mFC_hotpoints[i], i);
    }
}

static void s_battleDragon_DrawRiderWeapon(s_battleDragon* pThis)
{
    if (mainGameState.gameStats.m1_dragonLevel == 8)
    {
        if (pRider1State->m18_3dModel.m44_hotpointData[3].size() == 0)
        {
            pThis->mF0 = pThis->m8_position;
        }
        else
        {
            transformAndAddVec(pRider1State->m18_3dModel.m44_hotpointData[3][0], pThis->mF0, cameraProperties2.m28[0]);
        }
    }
    else
    {
        if (pRider1State->m18_3dModel.m44_hotpointData[1].size() == 0)
        {
            pThis->mF0 = pThis->m8_position;
        }
        else
        {
            transformAndAddVec(pRider1State->m18_3dModel.m44_hotpointData[1][0], pThis->mF0, cameraProperties2.m28[0]);
        }
    }

    s32 iVar2 = s_battleDragon_InitSub0();
    if ((iVar2 == 0) || ((pThis->m1C4 & 0x200) != 0))
    {
        iVar2 = 0;
    }
    else
    {
        iVar2 = readSaturnS32(gCurrentBattleOverlay->getSaturnPtr(0x060ae3bc) + getBattleManager()->m10_battleOverlay->m4_battleEngine->m22C_battleDirection * 4);
    }

    sVec3_FP weaponRotation = pThis->m14_rotation + sVec3_FP(0, 0x8000000 + iVar2, 0);

    pushCurrentMatrix();
    translateCurrentMatrix(pThis->mF0);
    rotateCurrentMatrixYXZ(weaponRotation);
    addObjectToDrawList(pRider1State->m0_riderBundle->get3DModel(pRider1State->m14_weaponModelIndex));
    popMatrix();
}

static void s_battleDragon_Draw(s_battleDragon* pThis)
{
    if (getBattleManager()->m10_battleOverlay->m4_battleEngine->m188_flags.m8)
    {
        // This setup lights
        //setupLightColor(s_RGB8::fromVector(getBattleManager()->m10_battleOverlay->m8_gridTask->m1F0).toU32());
        //battleEngine_UpdateSub7Sub1Sub0()
        FunctionUnimplemented();
    }

    u32 cVar3 = s_battleDragon_UpdateAnimationState(pThis);
    
    sVec3_FP rotation = pThis->m14_rotation + sVec3_FP(0, 0x8000000, 0);

    if ((getBattleManager()->m2_currentBattleOverlayId == 0) || (getBattleManager()->m2_currentBattleOverlayId == 0x10))
    {
        submitModelAndShadowModelToRendering(&gDragonState->m28_dragon3dModel, gDragonState->m14_modelIndex, 0, &pThis->m8_position, &rotation, 0);
    }
    else
    {
        if (mainGameState.gameStats.m1_dragonLevel == 8)
        {
            submitModelAndShadowModelToRendering(&gDragonState->m28_dragon3dModel, gDragonState->m14_modelIndex, 0, &pThis->m8_position, &rotation, 0);
        }
        else
        {
            submitModelAndShadowModelToRendering(&gDragonState->m28_dragon3dModel, gDragonState->m14_modelIndex, gDragonState->m18_shadowModelIndex, &pThis->m8_position, &rotation, getBattleManager()->m10_battleOverlay->m4_battleEngine->m354[3]);
        }
    }

    sVec3_FP riderRotation;
    s_battleDragon_getRiderRotation(riderRotation, gDragonState->m28_dragon3dModel.m3C_boneMatrices[0]);
    s_battleDragon_UpdateRider1Position();

    if (cVar3)
    {
        assert(0);
    }

    riderRotation += pThis->m14_rotation + sVec3_FP(0, 0x8000000, 0);

    if (pRider1State->mC_riderType)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(getBattleManager()->m10_battleOverlay->m18_dragon->mFC_hotpoints[0]);
        rotateCurrentMatrixYXZ(riderRotation);
        pRider1State->m18_3dModel.m18_drawFunction(&pRider1State->m18_3dModel);
        popMatrix();
    }
    s_battleDragon_DrawRiderWeapon(pThis);

    if (pRider2State->mC_riderType)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(getBattleManager()->m10_battleOverlay->m18_dragon->mFC_hotpoints[1]);
        rotateCurrentMatrixYXZ(riderRotation);
        pRider2State->m18_3dModel.m18_drawFunction(&pRider2State->m18_3dModel);
        popMatrix();
    }

    FunctionUnimplemented();
}

static void s_battleDragon_Delete(s_battleDragon* pThis)
{
    FunctionUnimplemented();
}

void battleEngine_createDragonTask(s_workAreaCopy* parent)
{
    static const s_battleDragon::TypedTaskDefinition definition = {
        &s_battleDragon_Init,
        &s_battleDragon_Update,
        &s_battleDragon_Draw,
        &s_battleDragon_Delete,
    };
    createSubTaskWithCopy<s_battleDragon>(parent, &definition);
}
