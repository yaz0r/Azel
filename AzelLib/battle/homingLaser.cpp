#include "PDS.h"
#include "homingLaser.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"
#include "battleDragon.h"
#include "battleOverlay_C.h"
#include "mainMenuDebugTasks.h" // TODO: cleanup
#include "town/town.h" // TODO: cleanup
#include "BTL_A3/baldor.h" // TODO: cleanup
#include "kernel/vdp1Allocator.h"
#include "gunShotRootTask.h" // TODO: cleanup

struct sHomingLaserRootTask : public s_workAreaTemplateWithCopy<sHomingLaserRootTask>
{
    struct sHomingLaserRootTask_sub
    {
        sBattleTargetable* m0_targetable;
        sVec3_FP* m4;
        s8 m8;
    };
    std::vector<sHomingLaserRootTask_sub> m8;
    s16 m64;
    s16 m6C_status;
    s16 m6E_numMaxEnemies;
    // size 0x70
};

struct sHomingLaserTask : public s_workAreaTemplateWithArgWithCopy<sHomingLaserTask, sHomingLaserRootTask::sHomingLaserRootTask_sub*>
{
    sVec3_FP m10;
    sVec3_FP m1C;
    fixedPoint m24;
    fixedPoint m30;
    sVec3_FP m34;
    sVec3_FP m40;
    sVec3_FP m4C;
    std::vector<s32> m58;
    std::vector<s32> m5C;
    std::vector<sVec3_FP> m60;
    std::vector<sVec3_FP> m64;
    s32 m68;
    s32 m6C;
    s32 m70;
    s8 m7E;
    sBattleTargetable* m84_targetable;
    s8 m80;
    sVec3_FP* m88_targetablePosition;
    sVec3_FP* m8C;
    sVec3_FP* m90_dragonPosition;
    sVec2_FP m94;
    struct sF0
    {
        std::vector<sVec3_FP> m0;
        s32 m4_numEntries;
        s32 m8;
        s32 mC;
        sSaturnPtr m10;
        u16 m14;

        struct sF0Color
        {
            u32 m0;
            u16 m4_color0;
            u16 m4_color1;
        };

        std::vector<sF0Color> m18_color;
    }mF0;
    // size 0x10C
};

void sHomingLaserTask_InitSub0(sHomingLaserTask::sF0* pThis, sHomingLaserTask* param_2, sVec3_FP* param_3, u16 param_4, sSaturnPtr param_5)
{
    pThis->m14 = param_4;
    pThis->m10 = param_5;
    pThis->m4_numEntries = readSaturnS32(param_5 + 0x20);
    pThis->m18_color.clear();
    pThis->m0.resize(pThis->m4_numEntries);
    for (int i=0; i<pThis->m4_numEntries; i++)
    {
        pThis->m0[i] = *param_3;
    }
    pThis->m8 = 0;
    pThis->mC = 0;
}

void sHomingLaserTask_InitSub1(sHomingLaserTask::sF0* pThis, sHomingLaserTask* param_2, sVec3_FP* param_3, u16 param_4, sVec3_FP* param_5, sVec3_FP* param_6)
{
    pThis->m18_color.resize(pThis->m4_numEntries);
    sVec3_FP local_28 = FP_Div(*param_5 - *param_6, fixedPoint::fromInteger(pThis->m4_numEntries - 1));

    std::array<sVec3_FP, 10> local_a0;
    local_a0[0] = *param_5 - local_28;
    for (int i=1; i<pThis->m4_numEntries; i++)
    {
        local_a0[i] = local_a0[i - 1] - local_28;
    }

    for (int i=0; i< pThis->m4_numEntries; i++)
    {
        for (int j=0; j<3; j++)
        {
            pThis->m18_color[j].m4_color0 = (local_a0[i][2].toInteger() << 10) | (local_a0[i][1].toInteger() << 5) | (local_a0[i][0].toInteger() << 5) | 0x8000;
            pThis->m18_color[j].m4_color1 = (local_a0[i][2].toInteger() << 10) | (local_a0[i][1].toInteger() << 5) | (local_a0[i][0].toInteger() << 5) | 0x8000;
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
    auto& pDragonLevelStats = gCommonFile.dragonLevelStats[mainGameState.gameStats.m1_dragonLevel];

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
    pThis->m4_vd1Allocation = dramAllocatorEnd[0].mC_buffer->m4_vd1Allocation;
    pThis->m84_targetable = arg->m0_targetable;

    if ((pThis->m84_targetable == nullptr) || (pThis->m84_targetable->m50 & 0x140001))
    {
        pThis->getTask()->markFinished();
        return;
    }

    pThis->m8C = arg->m4;
    pThis->m88_targetablePosition = getBattleTargetablePosition(*pThis->m84_targetable);
    pThis->m80 = arg->m8;
    pThis->m90_dragonPosition = &gBattleManager->m10_battleOverlay->m18_dragon->m8_position;
    pThis->m10 = *pThis->m8C;
    pThis->m6C = pThis->m80 * 2 + 0xF;
    pThis->m70 = pThis->m80 * 2 + 0xF;

    sVec3_FP local_18 = *pThis->m88_targetablePosition - pThis->m10;

    pThis->m4C[1] = MTH_Mul(0x4000, sqrt_F(MTH_Product3d_FP(local_18, local_18)));

    pThis->m30 = performModulo2(0x5b05b0, randomNumber()) - 0x2D82D8;
    computeVectorAngles(local_18, pThis->m94);
    pThis->m24 = readSaturnFP(gCurrentBattleOverlay->getSaturnPtr(0x60AD510) + pThis->m80 * 4);

    pushCurrentMatrix();
    translateCurrentMatrix(pThis->m10);
    rotateCurrentMatrixShiftedY(-pThis->m94[1]);
    rotateCurrentMatrixShiftedX(-pThis->m94[0]);
    rotateCurrentMatrixShiftedZ(pThis->m24);
    sVec3_FP sStack36;
    transformAndAddVecByCurrentMatrix(&pThis->m4C, &sStack36);
    transformAndAddVec(sStack36, pThis->m40, cameraProperties2.m28[0]);
    popMatrix();

    pThis->m40 -= pThis->m10;
    pThis->m60.resize(10);
    pThis->m64.resize(10);
    pThis->m58.resize(1);
    pThis->m5C.resize(1);

    for (int i=0; i<10; i++)
    {
        pThis->m64[i].zeroize();
    }

    pThis->m58[0] = 0;
    pThis->m5C[0] = 0;

    playSoundEffect(8);

    sHomingLaserTask_InitSub0(&pThis->mF0, pThis, pThis->m90_dragonPosition, pThis->m4_vd1Allocation->m4_vdp1Memory, gCurrentBattleOverlay->getSaturnPtr(0x60ADCE4));

    sVec3_FP local_34;
    sVec3_FP local_40;
    updateDragonStats(1, &local_34);
    updateDragonStats(0, &local_40);

    sHomingLaserTask_InitSub1(&pThis->mF0, pThis, pThis->m90_dragonPosition, pThis->m4_vd1Allocation->m4_vdp1Memory, &local_34, &local_40);
}

void Baldor_updateSub0Sub2Sub2Sub0(npcFileDeleter* param1, sSaturnPtr param2, const std::vector<sVec3_FP>& param3, sVec3_FP* param4, sVec3_FP* param5, s32 param6, s32 param7, s32 param8)
{
    FunctionUnimplemented();
}

void sHomingLaserTask_UpdateSub0(sHomingLaserTask* pThis)
{
    if ((gBattleManager->m10_battleOverlay->m18_dragon->m1C0 & 0x80) && (pThis->m6C < 0xB))
    {
        randomNumber();

        sVec3_FP local_20;
        local_20[0] = performModulo2(0x111, randomNumber()) - 0x88;
        local_20[1] = performModulo2(0x111, randomNumber()) - 0x88;
        local_20[2] = performModulo2(0x111, randomNumber()) - 0x88;

        Baldor_updateSub0Sub2Sub2Sub0(dramAllocatorEnd[0].mC_buffer, gCurrentBattleOverlay->getSaturnPtr(0x060b0658), pThis->m60, &gBattleManager->m10_battleOverlay->m4_battleEngine->m1A0, &local_20, 0x10000, 0, 0);
    }
}

s32 sHomingLaserTask_UpdateSub1(s16 param_1)
{
    s32 iVar1 = FP_Div((int)(short)mainGameState.gameStats.dragonAtt, 200);
    iVar1 = MTH_Mul(0x9999, iVar1);
    iVar1 = MTH_Mul((int)param_1 << 0x10, iVar1 + 0xb333);
    s32 iVar2 = FP_Div(iVar1 + 0x8000, 0xa0000);
    s32 iVar3 = randomNumber();
    iVar3 = performModulo2(iVar2 * 2 + 1, iVar3);
    iVar1 = iVar1 + 0x8000 + (iVar3 - iVar2) + 0x8000;

    if (gBattleManager->m10_battleOverlay->m18_dragon->m1C0 & 0x80)
    {
        iVar1 = MTH_Mul(iVar1, 0x14CCC);
    }
    if (gBattleManager->m10_battleOverlay->m18_dragon->m1C0 & 0x10)
    {
        iVar1 = MTH_Mul(iVar1, 0x8000);
    }

    return fixedPoint::toInteger(iVar1 + 0x8000);
}

void sGunShotTask_UpdateSub2Sub2(s_workAreaCopy* pParent, sVec3_FP*, sVec3_FP*, fixedPoint)
{
    FunctionUnimplemented();
}

void sGunShotTask_UpdateSub2Sub0(sBattleTargetable* param_1, s16, sVec3_FP*, s8, fixedPoint, fixedPoint, fixedPoint, fixedPoint)
{
    FunctionUnimplemented();
}

void sHomingLaserTask_Update(sHomingLaserTask* pThis)
{
    switch (pThis->m7E)
    {
    case 0:
        pThis->m7E++;
        break;
    case 1:
        // if target is inactive
        if ((pThis->m84_targetable == nullptr) || (pThis->m84_targetable->m50 & 0x140001))
        {
            pThis->m7E = 3;
            pThis->m34 = pThis->m64[0] - pThis->m64[1];
            pThis->m6C = 0x1E;
        }
        else
        {
            for (int i=0; i<8; i++)
            {
                pThis->m64[i + 1] = pThis->m64[i];
            }

            if (pThis->m6C < 1)
            {
                pThis->m64[0] = *pThis->m88_targetablePosition - *pThis->m8C;
                pThis->m34 = pThis->m64[0] - pThis->m64[1];
                pThis->m6C = 0x1E;
                pThis->m5C[0] = 0x10000;
                pThis->m7E++;

                int iVar8;
                if (mainGameState.gameStats.m1_dragonLevel == 8)
                {
                    iVar8 = 0xf00000;
                }
                else
                {
                    iVar8 = fixedPoint::fromInteger(mainGameState.gameStats.mC_laserPower);
                }

                int uVar3 = sHomingLaserTask_UpdateSub1(fixedPoint::toInteger(FP_Div(iVar8, fixedPoint::fromInteger(readSaturnS16(gCurrentBattleOverlay->getSaturnPtr(0x60AD4E4) + gDragonState->mC_dragonType * 2))) + 0x8000));
                sGunShotTask_UpdateSub2Sub0(pThis->m84_targetable, sGunShotTask_UpdateSub1Sub2(pThis->m84_targetable, uVar3, 1), pThis->m88_targetablePosition, 2,
                    pThis->m34[0], pThis->m34[1], pThis->m34[2], 0x1000);

                if ((pThis->m84_targetable->m60 == 0) || (pThis->m84_targetable->m50 & 0x400))
                {
                    sVec2_FP asStack84;
                    computeVectorAngles(pThis->m34, asStack84);
                    
                    sVec3_FP dStack96;
                    dStack96[0] = asStack84[0];
                    dStack96[1] = asStack84[1];
                    dStack96[2] = randomNumber();
                    sGunShotTask_UpdateSub2Sub2(pThis, pThis->m88_targetablePosition, &dStack96, 0x4CCC);

                    pThis->m34 = MTH_Mul(-0x10000, pThis->m34);

                    playSoundEffect(0x16);
                    pThis->m7E = 3;
                }
            }
            else
            {
                pThis->m10 = MTH_Mul(FP_Div(fixedPoint::fromInteger(pThis->m70 - pThis->m6C), fixedPoint::fromInteger(pThis->m70)), *pThis->m88_targetablePosition - *pThis->m8C) + *pThis->m8C;
                pThis->m5C[0] = getSin(FP_Div(0x8000000, fixedPoint::fromInteger(pThis->m70)));

                pushCurrentMatrix();
                translateCurrentMatrix(pThis->m10);
                sVec3_FP local_40 = MTH_Mul(pThis->m5C[0], pThis->m40);
                rotateCurrentMatrixYXZ(pThis->m1C);

                sVec3_FP sStack52;
                transformAndAddVecByCurrentMatrix(&local_40, &sStack52);
                transformAndAddVec(sStack52, local_40, cameraProperties2.m28[0]);
                popMatrix();

                pThis->m64[0] = local_40 - *pThis->m8C;

                pThis->m6C--;

                sHomingLaserTask_UpdateSub0(pThis);
            }
        }
        break;
    default:
        break;
        assert(0);
    }

    pThis->m24 += pThis->m30;
    pThis->m68++;
    if (pThis->mF0.m8 <= pThis->mF0.m4_numEntries - 1)
    {
        pThis->mF0.m8 += pThis->mF0.mC;
    }
}

s32 sHomingLaserTask_DrawSub1Sub0Sub0(std::array<sVec3_FP, 2>& param_1, sSaturnPtr param_2, s_graphicEngineStatus_405C& param_3, sMatrix4x3* param_5)
{
    if (isGunShotVisible(param_1, param_3))
    {
        fixedPoint ratio0 = FP_Div(0x10000, param_1[0][2]);
        fixedPoint iVar3 = MTH_Mul_5_6(param_3.m18_widthScale, param_1[0][0], ratio0);
        fixedPoint iVar4 = MTH_Mul_5_6(param_3.m1C_heightScale, param_1[0][1], ratio0);



        assert(0);

        return 1;
    }

    return 0;
}

void sHomingLaserTask_DrawSub1Sub0(std::array<sVec3_FP, 2>& param1, sSaturnPtr param_2, u16 param_3, s16 param_4, s16 param_5, sHomingLaserTask::sF0::sF0Color& param_6, s32 param_7)
{
    std::array<sVec3_FP, 2> sStack32;

    transformAndAddVecByCurrentMatrix(&param1[0], &sStack32[0]);
    transformAndAddVecByCurrentMatrix(&param1[1], &sStack32[1]);

    sMatrix4x3 sStack80;

    if (sHomingLaserTask_DrawSub1Sub0Sub0(sStack32, param_2, graphicEngineStatus.m405C, &sStack80))
    {
        if (sGunShotTask_DrawSub1Sub1(&sStack80, graphicEngineStatus.m405C)
        {
            sGunShotTask_DrawSub1Sub3(&sStack80, sStack80[2], param_3, param_4, param_5, param_6, param_7);
        }
    }
}

void sHomingLaserTask_DrawSub1(sHomingLaserTask::sF0* pThis)
{
    if (pThis->m8 >= pThis->m4_numEntries - 1)
    {
        return;
    }

    int iVar1 = pThis->m8;
    int iVar3 = pThis->m4_numEntries;

    std::array<sVec3_FP, 2> uStack64;
    uStack64[0] = pThis->m0[iVar3 - 2];
    uStack64[1] = pThis->m0[iVar3 - 1];

    sSaturnPtr local_28 = pThis->m10 + 0x18 + (iVar3 - 2) * 4;
    sSaturnPtr local_24 = pThis->m10 + 0x18 + (iVar3 - 1) * 4;

    std::vector<sHomingLaserTask::sF0::sF0Color>* iVar4;
    if (pThis->m18_color.size() == 0)
    {
        assert(0);
        //iVar4 = pThis->m10
    }
    else
    {
        iVar4 = &pThis->m18_color;
    }
    sSaturnPtr iVar5 = pThis->m10;

    sHomingLaserTask_DrawSub1Sub0(uStack64, local_28, pThis->m14 + readSaturnS16(iVar5 + 8), readSaturnS16(iVar5 + 0xE), readSaturnS16(iVar5 + 18) + pThis->m14, (*iVar4)[iVar3 - 2], 8);
    assert(0);
}

void sHomingLaserTask_Draw(sHomingLaserTask* pThis)
{
    for (int i = 0; i < 8; i++)
    {
        sVec3_FP local_2c = *pThis->m8C - *pThis->m90_dragonPosition;

        pushCurrentMatrix();
        translateCurrentMatrix(pThis->m90_dragonPosition);
        local_2c += pThis->m64[i];

        sVec3_FP sStack56;
        transformAndAddVecByCurrentMatrix(&local_2c, &sStack56);
        transformAndAddVec(sStack56, pThis->m60[i], cameraProperties2.m28[0]);
        pThis->mF0.m0[i] = pThis->m60[i];
        popMatrix();
    }

    if (pThis->m7E == 3)
    {
        assert(0);
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
                if (!(gBattleManager->m10_battleOverlay->mC_targetSystem->m0_enemyTargetables[i]->m4_targetable->m50 & 0x40000))
                {
                    pThis->m8[i].m0_targetable = gBattleManager->m10_battleOverlay->mC_targetSystem->m0_enemyTargetables[i]->m4_targetable;
                    pThis->m8[i].m4 = &gBattleManager->m10_battleOverlay->m18_dragon->m144;
                    pThis->m8[i].m8 = i;
                    createSiblingTaskWithArgWithCopy<sHomingLaserTask, sHomingLaserRootTask::sHomingLaserRootTask_sub*>(pThis, &pThis->m8[i], &homingLaserDefinition);
                }
            }
            else
            {
                // Use targetable 0
                if (!(gBattleManager->m10_battleOverlay->mC_targetSystem->m0_enemyTargetables[i]->m4_targetable->m50 & 0x40000))
                {
                    pThis->m8[i].m0_targetable = gBattleManager->m10_battleOverlay->mC_targetSystem->m0_enemyTargetables[0]->m4_targetable;
                    pThis->m8[i].m4 = &gBattleManager->m10_battleOverlay->m18_dragon->m144;
                    pThis->m8[i].m8 = i;
                    createSiblingTaskWithArgWithCopy<sHomingLaserTask, sHomingLaserRootTask::sHomingLaserRootTask_sub*>(pThis, &pThis->m8[i], &homingLaserDefinition);
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

    pNewTask->m8.resize(pNewTask->m6E_numMaxEnemies);
}
