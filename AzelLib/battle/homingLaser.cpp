#include "PDS.h"
#include "homingLaser.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"
#include "battleDragon.h"
#include "battleOverlay_C.h"
#include "mainMenuDebugTasks.h" // TODO: cleanup
#include "kernel/graphicalObject.h"
#include "BTL_A3/baldor.h" // TODO: cleanup
#include "kernel/vdp1Allocator.h"
#include "gunShotRootTask.h" // TODO: cleanup
#include "audio/systemSounds.h"
#include "kernel/debug/trace.h"
#include "commonOverlay.h"
#include "battleGenericData.h"
#include "kernel/rayDisplay.h"
#include "battle/particleEffect.h"
#include "battle/BTL_A3/BTL_A3_data.h"

struct sHomingLaserRootTask : public s_workAreaTemplateWithCopy<sHomingLaserRootTask>
{
    struct sHomingLaserRootTask_sub
    {
        sBattleTargetable* m0_targetable;
        sVec3_FP* m4_pLaserSource;
        s8 m8_pLaserId;
    };
    std::vector<sHomingLaserRootTask_sub> m8_individualLaser;
    s16 m64;
    s16 m6C_status;
    s16 m6E_numMaxEnemies;
    // size 0x70
};

struct sHomingLaserTask : public s_workAreaTemplateWithArgWithCopy<sHomingLaserTask, sHomingLaserRootTask::sHomingLaserRootTask_sub*>
{
    sVec3_FP m10_laserPosition;
    sVec3_FP m1C;
    fixedPoint m24;
    fixedPoint m30;
    sVec3_FP m34_laserDelta;
    sVec3_FP m40;
    sVec3_FP m4C;
    std::vector<s32> m58;
    std::vector<s32> m5C_laserArc;
    std::vector<sVec3_FP> m60;
    std::vector<sVec3_FP> m64_laserTrajectory;
    s32 m68;
    s32 m6C_numFramesToDestination;
    s32 m70_totalLaserNumFrames;
    s8 m7E_status;
    sBattleTargetable* m84_targetable;
    s8 m80_laserIndexInGroup;
    sVec3_FP* m88_targetablePosition;
    sVec3_FP* m8C_laserSource;
    sVec3_FP* m90_dragonPosition;
    sVec2_FP m94;
    struct sF0
    {
        std::vector<sVec3_FP> m0_laserNodePosition;
        s32 m4_numLaserNodes;
        s32 m8;
        s32 mC;
        const sLaserData* m10_laserData;
        u16 m14;

        std::vector<quadColor> m18_color;
    }mF0;
    // size 0x10C
};

void sHomingLaserTask_InitSub0(sHomingLaserTask::sF0* pThis, sHomingLaserTask* param_2, sVec3_FP* param_3, u16 param_4, const sLaserData* pLaserData)
{
    pThis->m14 = param_4;
    pThis->m10_laserData = pLaserData;
    pThis->m4_numLaserNodes = pLaserData->m20_numLaserNodes;
    pThis->m18_color.clear();
    pThis->m0_laserNodePosition.resize(pThis->m4_numLaserNodes);
    for (int i=0; i<pThis->m4_numLaserNodes; i++)
    {
        pThis->m0_laserNodePosition[i] = *param_3;
    }
    pThis->m8 = 0;
    pThis->mC = 0;
}

void sHomingLaserTask_InitSub1(sHomingLaserTask::sF0* pThis, sHomingLaserTask* param_2, sVec3_FP* param_3, u16 param_4, sVec3_FP* param_5, sVec3_FP* param_6)
{
    pThis->m18_color.resize(pThis->m4_numLaserNodes);
    sVec3_FP local_28 = FP_Div(*param_5 - *param_6, fixedPoint::fromInteger(pThis->m4_numLaserNodes - 1));

    std::array<sVec3_FP, 10> local_a0;
    local_a0[0] = *param_5 - local_28;
    for (int i=1; i<pThis->m4_numLaserNodes; i++)
    {
        local_a0[i] = local_a0[i - 1] - local_28;
    }

    for (int i=0; i< pThis->m4_numLaserNodes; i++)
    {
        for (int j=0; j<3; j++)
        {
            pThis->m18_color[j][2] = (local_a0[i][2].toInteger() << 10) | (local_a0[i][1].toInteger() << 5) | (local_a0[i][0].toInteger()) | 0x8000;
            pThis->m18_color[j][3] = (local_a0[i][2].toInteger() << 10) | (local_a0[i][1].toInteger() << 5) | (local_a0[i][0].toInteger()) | 0x8000;
        }
    }
}

s16 dragonMorphVar0;
s16 dragonMorphVar1;
s16 dragonMorphVar2;
s16 dragonMorphVar3;

std::array<s32, 3> morphDragonAccumulator;

void morphDragonSub1Sub0(s32 param_1, s32 param_2, s32 param_3)
{
    dragonMorphVar0 = param_3 / 4;
    dragonMorphVar1 = param_2 / 4;
    dragonMorphVar2 = param_1 >> 0x12;
    dragonMorphVar3 = param_1 / 4;

    morphDragonAccumulator[0] = param_1;
    morphDragonAccumulator[1] = param_2;
    morphDragonAccumulator[2] = param_3;
}

void morphDragonSub1(s32 param_1, s32 param_2)
{
    s32 dVar2;
    s32 iVar1;
    s32 iVar3;

    if ((param_1 == 0) && (param_2 == 0))
    {
        dVar2 = 0x10000;
        iVar1 = 0;
        iVar3 = 0;
    }
    else
    {
        iVar1 = FP_Div(0x10000, (param_1 + param_2) * 0x800);
        dVar2 = MTH_Mul((0x800 - param_1) * param_1 + (0x800 - param_2) * param_2, iVar1);
        iVar3 = MTH_Mul(param_1 * param_1, iVar1);
        iVar1 = MTH_Mul(param_2 * param_2, iVar1);
    }

    morphDragonSub1Sub0(dVar2, iVar3, iVar1);
}

void updateDragonStats(int type, sVec3_FP* pOutput)
{
    auto& pDragonLevelStats = gCommonFile->dragonLevelStats[mainGameState.gameStats.m1_dragonLevel];

    std::array<s8, 3>::iterator pcVar4 = pDragonLevelStats.m18.begin();
    std::array<s8, 3>::iterator pcVar5 = pDragonLevelStats.m12.begin();
    std::array<s8, 3>::iterator pcVar6 = pDragonLevelStats.m0.begin();

    if (type)
    {
        pcVar4 = pDragonLevelStats.m1B.begin();
        pcVar5 = pDragonLevelStats.m15.begin();
        pcVar6 = pDragonLevelStats.m3.begin();
    }

    s16 iVar1 = mainGameState.gameStats.m1A_dragonCursorX;
    s16 iVar2 = mainGameState.gameStats.m1C_dragonCursorY;

    if (iVar1 < 0)
    {
        iVar1 = -iVar1;
        pcVar4 = pDragonLevelStats.mC.begin();
        if (type)
        {
            pcVar4 = pDragonLevelStats.mF.begin();
        }
    }

    if (iVar2 < 0)
    {
        iVar2 = -iVar2;
        pcVar5 = pDragonLevelStats.m6.begin();
        if (type)
        {
            pcVar5 = pDragonLevelStats.m9.begin();
        }
    }

    morphDragonSub1(iVar1, iVar2);

    (*pOutput)[0] = pcVar6[0] * morphDragonAccumulator[0] + pcVar4[0] * morphDragonAccumulator[1] + pcVar5[0] * morphDragonAccumulator[2];
    (*pOutput)[1] = pcVar6[1] * morphDragonAccumulator[0] + pcVar4[1] * morphDragonAccumulator[1] + pcVar5[1] * morphDragonAccumulator[2];
    (*pOutput)[2] = pcVar6[2] * morphDragonAccumulator[0] + pcVar4[2] * morphDragonAccumulator[1] + pcVar5[2] * morphDragonAccumulator[2];
}

void sHomingLaserTask_Init(sHomingLaserTask* pThis, sHomingLaserRootTask::sHomingLaserRootTask_sub* arg)
{
    pThis->m4_vd1Allocation = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation;
    pThis->m84_targetable = arg->m0_targetable;

    if ((pThis->m84_targetable == nullptr) || (pThis->m84_targetable->m50_flags & 0x140001))
    {
        pThis->getTask()->markFinished();
        return;
    }

    pThis->m8C_laserSource = arg->m4_pLaserSource;
    pThis->m88_targetablePosition = getBattleTargetablePosition(*pThis->m84_targetable);
    pThis->m80_laserIndexInGroup = arg->m8_pLaserId;
    pThis->m90_dragonPosition = &gBattleManager->m10_battleOverlay->m18_dragon->m8_position;
    pThis->m10_laserPosition = *pThis->m8C_laserSource;
    pThis->m6C_numFramesToDestination = pThis->m80_laserIndexInGroup * 2 + 0xF;
    pThis->m70_totalLaserNumFrames = pThis->m80_laserIndexInGroup * 2 + 0xF;

    addTraceLog(*pThis->m88_targetablePosition, "targetablePosition");
    addTraceLog(pThis->m10_laserPosition, "m10_laserPosition");

    sVec3_FP local_18 = *pThis->m88_targetablePosition - pThis->m10_laserPosition;

    pThis->m4C[1] = MTH_Mul(0x4000, sqrt_F(MTH_Product3d_FP(local_18, local_18)));

    pThis->m30 = performModulo2(0x5b05b0, randomNumber()) - 0x2D82D8;
    computeLookAt(local_18, pThis->m94);
    pThis->m24 = readSaturnFP(g_BTL_GenericData->getSaturnPtr(0x60AD510) + pThis->m80_laserIndexInGroup * 4);

    pushCurrentMatrix();
    translateCurrentMatrix(pThis->m10_laserPosition);
    rotateCurrentMatrixShiftedY(-pThis->m94[1]);
    rotateCurrentMatrixShiftedX(-pThis->m94[0]);
    rotateCurrentMatrixShiftedZ(pThis->m24);
    sVec3_FP sStack36;
    transformAndAddVecByCurrentMatrix(&pThis->m4C, &sStack36);
    transformAndAddVec(sStack36, pThis->m40, cameraProperties2.m28[0]);
    popMatrix();

    pThis->m40 -= pThis->m10_laserPosition;
    pThis->m60.resize(10);
    pThis->m64_laserTrajectory.resize(10);
    pThis->m58.resize(1);
    pThis->m5C_laserArc.resize(1);

    for (int i=0; i<10; i++)
    {
        pThis->m64_laserTrajectory[i].zeroize();
    }

    pThis->m58[0] = 0;
    pThis->m5C_laserArc[0] = 0;

    playSystemSoundEffect(8);

    sHomingLaserTask_InitSub0(&pThis->mF0, pThis, pThis->m90_dragonPosition, pThis->m4_vd1Allocation->m4_vdp1Memory, &g_BTL_GenericData->mLaserData);

    sVec3_FP local_34;
    sVec3_FP local_40;
    updateDragonStats(1, &local_34);
    updateDragonStats(0, &local_40);

    sHomingLaserTask_InitSub1(&pThis->mF0, pThis, pThis->m90_dragonPosition, pThis->m4_vd1Allocation->m4_vdp1Memory, &local_34, &local_40);
}

void sHomingLaserTask_UpdateSub0(sHomingLaserTask* pThis)
{
    if ((gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 0x80) && (pThis->m6C_numFramesToDestination < 0xB))
    {
        randomNumber();

        sVec3_FP local_20;
        local_20[0] = performModulo2(0x111, randomNumber()) - 0x88;
        local_20[1] = performModulo2(0x111, randomNumber()) - 0x88;
        local_20[2] = performModulo2(0x111, randomNumber()) - 0x88;

        createParticleEffect(dramAllocatorEnd[0].mC_fileBundle, &g_BTL_GenericData->m_0x60b0658_animatedQuad, &pThis->m60[0], &gBattleManager->m10_battleOverlay->m4_battleEngine->m1A0_battleAutoScrollDelta, &local_20, 0x10000, 0, 0);
    }
}

s32 sHomingLaserTask_UpdateSub1(s16 param_1)
{
    s32 iVar1 = FP_Div((int)(short)mainGameState.gameStats.mBE_dragonAtt, 200);
    iVar1 = MTH_Mul(0x9999, iVar1);
    iVar1 = MTH_Mul((int)param_1 << 0x10, iVar1 + 0xb333);
    s32 iVar2 = FP_Div(iVar1 + 0x8000, 0xa0000);
    s32 iVar3 = randomNumber();
    iVar3 = performModulo2(iVar2 * 2 + 1, iVar3);
    iVar1 = iVar1 + 0x8000 + (iVar3 - iVar2) + 0x8000;

    if (gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 0x80)
    {
        iVar1 = MTH_Mul(iVar1, 0x14CCC);
    }
    if (gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 0x10)
    {
        iVar1 = MTH_Mul(iVar1, 0x8000);
    }

    return fixedPoint::toInteger(iVar1 + 0x8000);
}

void createGunAndLaserDamage3dModel(s_workAreaCopy* pParent, sVec3_FP*, sVec3_FP*, fixedPoint)
{
    Unimplemented();
}

void sHomingLaserTask_Update(sHomingLaserTask* pThis)
{
    switch (pThis->m7E_status)
    {
    case 0:
        pThis->m7E_status++;
        // fall through
    case 1:
        // if target is inactive
        if ((pThis->m84_targetable == nullptr) || (pThis->m84_targetable->m50_flags & 0x140001))
        {
            pThis->m7E_status = 3;
            pThis->m34_laserDelta = pThis->m64_laserTrajectory[0] - pThis->m64_laserTrajectory[1];
            pThis->m6C_numFramesToDestination = 0x1E;
        }
        else
        {
            for (int i=8; i>=0; i--)
            {
                pThis->m64_laserTrajectory[i + 1] = pThis->m64_laserTrajectory[i];
            }

            if (pThis->m6C_numFramesToDestination < 1)
            {
                pThis->m64_laserTrajectory[0] = *pThis->m88_targetablePosition - *pThis->m8C_laserSource;
                pThis->m34_laserDelta = pThis->m64_laserTrajectory[0] - pThis->m64_laserTrajectory[1];
                pThis->m6C_numFramesToDestination = 0x1E;
                pThis->m5C_laserArc[0] = 0x10000;
                pThis->m7E_status++;

                int iVar8;
                if (mainGameState.gameStats.m1_dragonLevel == 8)
                {
                    iVar8 = 0xf00000;
                }
                else
                {
                    iVar8 = fixedPoint::fromInteger(mainGameState.gameStats.mC_laserPower);
                }

                int uVar3 = sHomingLaserTask_UpdateSub1(fixedPoint::toInteger(FP_Div(iVar8, fixedPoint::fromInteger(readSaturnS16(g_BTL_GenericData->getSaturnPtr(0x60AD4E4) + gDragonState->mC_dragonType * 2))) + 0x8000));
                applyDamageToEnnemy(pThis->m84_targetable, sGunShotTask_UpdateSub1Sub2(pThis->m84_targetable, uVar3, 1), pThis->m88_targetablePosition, 2, pThis->m34_laserDelta, 0x1000);

                if ((pThis->m84_targetable->m60 == 0) || (pThis->m84_targetable->m50_flags & 0x400))
                {
                    sVec2_FP asStack84;
                    computeLookAt(pThis->m34_laserDelta, asStack84);
                    
                    sVec3_FP dStack96;
                    dStack96[0] = asStack84[0];
                    dStack96[1] = asStack84[1];
                    dStack96[2] = randomNumber();
                    createGunAndLaserDamage3dModel(pThis, pThis->m88_targetablePosition, &dStack96, 0x4CCC);

                    pThis->m34_laserDelta = MTH_Mul(-0x10000, pThis->m34_laserDelta);

                    playSystemSoundEffect(0x16);
                    pThis->m7E_status = 3;
                }
            }
            else
            {
                fixedPoint stepInLaser = fixedPoint::fromInteger(pThis->m70_totalLaserNumFrames - pThis->m6C_numFramesToDestination);
                fixedPoint ratioInLaser = FP_Div(stepInLaser, fixedPoint::fromInteger(pThis->m70_totalLaserNumFrames));
                pThis->m10_laserPosition = MTH_Mul(ratioInLaser, *pThis->m88_targetablePosition - *pThis->m8C_laserSource) + *pThis->m8C_laserSource;
                pThis->m5C_laserArc[0] = getSin(MTH_Mul(FP_Div(0x8000000, fixedPoint::fromInteger(pThis->m70_totalLaserNumFrames)), stepInLaser).toInteger());

                pushCurrentMatrix();
                translateCurrentMatrix(pThis->m10_laserPosition);
                sVec3_FP local_40 = MTH_Mul(pThis->m5C_laserArc[0], pThis->m40);
                rotateCurrentMatrixYXZ(pThis->m1C);

                sVec3_FP sStack52;
                transformAndAddVecByCurrentMatrix(&local_40, &sStack52);
                transformAndAddVec(sStack52, local_40, cameraProperties2.m28[0]);
                popMatrix();

                pThis->m64_laserTrajectory[0] = local_40 - *pThis->m8C_laserSource;

                pThis->m6C_numFramesToDestination--;

                sHomingLaserTask_UpdateSub0(pThis);
            }
        }
        break;
    case 2:
        for (int i = 8; i >= 0; i--)
        {
            pThis->m64_laserTrajectory[i + 1] = pThis->m64_laserTrajectory[i];
        }
        pThis->m64_laserTrajectory[0] += pThis->m34_laserDelta;
        if (gBattleManager->m10_battleOverlay->mC_targetSystem->m204_cameraMaxAltitude < pThis->m60[0][1])
        {
            if (pThis->m6C_numFramesToDestination-- < 0)
            {
                pThis->getTask()->markFinished();
            }
            else
            {
                if (pThis->m6C_numFramesToDestination == 0x1D)
                {
                    pThis->mF0.mC = 1;
                }
            }
        }
        else
        {
            pThis->getTask()->markFinished();
        }
        break;
    case 3:
        Unimplemented();
        break;
    default:
        assert(0);
        break;
    }

    pThis->m24 += pThis->m30;
    pThis->m68++;
    if (pThis->mF0.m8 <= pThis->mF0.m4_numLaserNodes - 1)
    {
        pThis->mF0.m8 += pThis->mF0.mC;
    }

    if (isTraceEnabled())
    {
        addTraceLog(pThis->m10_laserPosition, "laserPosition");
        addTraceLog(pThis->m5C_laserArc[0], "laserArc");
        addTraceLog(*pThis->m8C_laserSource, "laserSource");
        addTraceLog(pThis->m40, "m40");
        for (int i = 0; i < 10; i++)
        {
            addTraceLog(pThis->m64_laserTrajectory[i], "trajectory");
        }
    }
}


s32 sGunShotTask_DrawSub1Sub1(sScreenQuad3*, s_graphicEngineStatus_405C&)
{
    Unimplemented();

    return 1;
}

bool bDisplayDebugLaser = true;
void sHomingLaserTask_DrawSub1Sub0(std::array<sVec3_FP, 2>& param1, std::array<fixedPoint, 2>& param_2, u16 param_3, s16 param_4, s16 param_5, const quadColor* param_6, s32 param_7)
{
    if (bDisplayDebugLaser)
    {
        drawDebugLine(param1[0], param1[1]);
    }

    std::array<sVec3_FP, 2> sStack32;
    transformAndAddVecByCurrentMatrix(&param1[0], &sStack32[0]);
    transformAndAddVecByCurrentMatrix(&param1[1], &sStack32[1]);

    sScreenQuad3 sStack80;

    if (rayComputeDisplayMatrix_2Width(sStack32, param_2, graphicEngineStatus.m405C, sStack80))
    {
        if (sGunShotTask_DrawSub1Sub1(&sStack80, graphicEngineStatus.m405C))
        {
            sendRaySegmentToVdp1(sStack80, sStack32[1][2], param_3, param_4, param_5, param_6, param_7);
        }
    }
}

void sHomingLaserTask_DrawSub1(sHomingLaserTask::sF0* pThis)
{
    if (pThis->m8 >= pThis->m4_numLaserNodes - 1)
    {
        return;
    }

    int iVar1 = pThis->m8;
    int iVar3 = pThis->m4_numLaserNodes;

    {
        int indexA = iVar3 - 2;
        int indexB = iVar3 - 1;

        std::array<sVec3_FP, 2> uStack64;
        uStack64[0] = pThis->m0_laserNodePosition[indexA];
        uStack64[1] = pThis->m0_laserNodePosition[indexB];

        std::array<fixedPoint, 2> local_28;
        local_28[0] = pThis->m10_laserData->m18_vertices[indexA];
        local_28[1] = pThis->m10_laserData->m18_vertices[indexB];

        const quadColor* iVar4;
        if (pThis->m18_color.size() == 0)
        {
            iVar4 = &pThis->m10_laserData->m1C_colors[indexA];
        }
        else
        {
            iVar4 = &pThis->m18_color[indexA];
        }
        sHomingLaserTask_DrawSub1Sub0(uStack64, local_28, pThis->m14 + pThis->m10_laserData->m8, pThis->m10_laserData->mE, pThis->m14 + pThis->m10_laserData->m14, iVar4, 8);
    }

    iVar3 = pThis->m4_numLaserNodes - 2;
    while (iVar3 > pThis->m8 + (pThis->m8 < 1))
    {
        int indexA = iVar3 - 1;
        int indexB = iVar3 - 0;

        std::array<sVec3_FP, 2> uStack64;
        uStack64[0] = pThis->m0_laserNodePosition[indexA];
        uStack64[1] = pThis->m0_laserNodePosition[indexB];

        std::array<fixedPoint, 2> local_28;
        local_28[0] = pThis->m10_laserData->m18_vertices[indexA];
        local_28[1] = pThis->m10_laserData->m18_vertices[indexB];

        const quadColor* iVar4;
        if (pThis->m18_color.size() == 0)
        {
            iVar4 = &pThis->m10_laserData->m1C_colors[indexA];
        }
        else
        {
            iVar4 = &pThis->m18_color[indexA];
        }

        sHomingLaserTask_DrawSub1Sub0(uStack64, local_28, pThis->m14 + pThis->m10_laserData->m6, pThis->m10_laserData->mC, pThis->m14 + pThis->m10_laserData->m12, iVar4, 8);
        iVar3--;
    }
    if (pThis->m8 < 1)
    {
        int indexA = iVar3 - 0;
        int indexB = iVar3 - 1;

        std::array<sVec3_FP, 2> uStack64;
        uStack64[0] = pThis->m0_laserNodePosition[indexA];
        uStack64[1] = pThis->m0_laserNodePosition[indexB];

        std::array<fixedPoint, 2> local_28;
        local_28[0] = pThis->m10_laserData->m18_vertices[indexA];
        local_28[1] = pThis->m10_laserData->m18_vertices[indexB];

        const quadColor* iVar4;
        if (pThis->m18_color.size() == 0)
        {
            iVar4 = &pThis->m10_laserData->m1C_colors[indexA];
        }
        else
        {
            iVar4 = &pThis->m18_color[indexA];
        }

        sHomingLaserTask_DrawSub1Sub0(uStack64, local_28, pThis->m14 + pThis->m10_laserData->m4, pThis->m10_laserData->mA, pThis->m14 + pThis->m10_laserData->m10, iVar4, 8);
    }
}

void sHomingLaserTask_DrawSub0(sHomingLaserTask::sF0* pThis, int, int)
{
    Unimplemented();
}

void sHomingLaserTask_Draw(sHomingLaserTask* pThis)
{
    for (int i = 9; i >= 0; i--)
    {
        sVec3_FP local_2c = *pThis->m8C_laserSource - *pThis->m90_dragonPosition;

        pushCurrentMatrix();
        translateCurrentMatrix(pThis->m90_dragonPosition);
        local_2c += pThis->m64_laserTrajectory[i];

        sVec3_FP sStack56;
        transformAndAddVecByCurrentMatrix(&local_2c, &sStack56);
        transformAndAddVec(sStack56, pThis->m60[i], cameraProperties2.m28[0]);
        pThis->mF0.m0_laserNodePosition[i] = pThis->m60[i];
        popMatrix();
    }

    if (pThis->m7E_status == 3)
    {
        sHomingLaserTask_DrawSub0(&pThis->mF0, pThis->m6C_numFramesToDestination, 0x1E);
    }
    else
    {
        sHomingLaserTask_DrawSub1(&pThis->mF0);
    }
}

static const sHomingLaserTask::TypedTaskDefinition homingLaserDefinition =
{
    &sHomingLaserTask_Init,
    &sHomingLaserTask_Update,
    &sHomingLaserTask_Draw,
    nullptr,
};

s32 battleEngine_getNumActiveEnemies()
{
    int iVar4 = 0;
    for (int i = 0; i < 0x80; i++)
    {
        if (gBattleManager->m10_battleOverlay->m4_battleEngine->m49C_enemies[i].m0_isActive)
        {
            iVar4++;
        }
    }
    return iVar4;
}



void homingLaserRootTask_Update(sHomingLaserRootTask* pThis)
{
    switch (pThis->m6C_status)
    {
    case 0:
        if (pThis->m6E_numMaxEnemies < 0)
        {
            pThis->m6E_numMaxEnemies = 1;
        }

        for (int i=0; i<pThis->m6E_numMaxEnemies; i++)
        {
            if (gBattleManager->m10_battleOverlay->m4_battleEngine->m3A0_LaserType == 0)
            {
                // Use all targetables
                if (!(gBattleManager->m10_battleOverlay->mC_targetSystem->m0_enemyTargetables[i]->m4_targetable->m50_flags & 0x40000))
                {
                    pThis->m8_individualLaser[i].m0_targetable = gBattleManager->m10_battleOverlay->mC_targetSystem->m0_enemyTargetables[i]->m4_targetable;
                    pThis->m8_individualLaser[i].m4_pLaserSource = &gBattleManager->m10_battleOverlay->m18_dragon->mFC_hotpoints[2];
                    pThis->m8_individualLaser[i].m8_pLaserId = i;
                    createSubTaskWithArgWithCopy<sHomingLaserTask, sHomingLaserRootTask::sHomingLaserRootTask_sub*>(pThis, &pThis->m8_individualLaser[i], &homingLaserDefinition);
                }
            }
            else
            {
                // Use targetable 0
                if (!(gBattleManager->m10_battleOverlay->mC_targetSystem->m0_enemyTargetables[i]->m4_targetable->m50_flags & 0x40000))
                {
                    pThis->m8_individualLaser[i].m0_targetable = gBattleManager->m10_battleOverlay->mC_targetSystem->m0_enemyTargetables[0]->m4_targetable;
                    pThis->m8_individualLaser[i].m4_pLaserSource = &gBattleManager->m10_battleOverlay->m18_dragon->mFC_hotpoints[2];
                    pThis->m8_individualLaser[i].m8_pLaserId = i;
                    createSubTaskWithArgWithCopy<sHomingLaserTask, sHomingLaserRootTask::sHomingLaserRootTask_sub*>(pThis, &pThis->m8_individualLaser[i], &homingLaserDefinition);
                }
            }
        }

        pThis->m64 = (pThis->m6E_numMaxEnemies * 2) + 0x2D;
        pThis->m6C_status++;
        break;
    case 1:
        if (pThis->m64-- < 0)
        {
            pThis->m6C_status++;
        }
        break;
    case 2:
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished = 1;
        pThis->getTask()->markFinished();
        break;
    default:
        assert(0);
    }
}

void battleEngine_createHomingLaserRootTask(s_workAreaCopy* pParent, s32 numMaxEnemies)
{
    static const sHomingLaserRootTask::TypedTaskDefinition definition = {
        nullptr,
        &homingLaserRootTask_Update,
        nullptr,
        nullptr
    };

    sHomingLaserRootTask* pNewTask = createSubTaskWithCopy<sHomingLaserRootTask>(pParent, &definition);

    if ((gBattleManager->m10_battleOverlay->m4_battleEngine->m3A0_LaserType & 0x30) >> 4 == 0)
    {
        pNewTask->m6E_numMaxEnemies = std::min(battleEngine_getNumActiveEnemies(), numMaxEnemies);
    }
    else
    {
        pNewTask->m6E_numMaxEnemies = numMaxEnemies;
    }

    pNewTask->m8_individualLaser.resize(pNewTask->m6E_numMaxEnemies);
}
