#include "PDS.h"
#include "urchin.h"
#include "BTL_A3_UrchinFormation.h"
#include "kernel/fileBundle.h"
#include "kernel/animation.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "battle/battleEngine.h"
#include "battle/battleDebug.h"
#include "battle/battleTargetable.h"
#include "battle/battleDragon.h"
#include "battle/battleDamageDisplay.h"
#include "battle/battleEnemyLifeMeter.h"
#include "commonOverlay.h"
#include "BTL_A3_data.h"
#include "kernel/graphicalObject.h"
#include "audio/systemSounds.h"
#include "battle/battleDebris.h"
#include "mainMenuDebugTasks.h"

void springDampedStep(sVec3_FP* pCurrent, sVec3_FP* pDelta, sVec3_FP* pTarget, s32 pDeltaFactor, s32 pDistanceToTargetFactor, s8 translationOrRotation); // TODO: cleanup
s32 checkTargetablesForDamage(p_workArea pThis, std::vector<sBattleTargetable>& param2, s16 entriesToParse, s16& param4); // TODO: cleanup

struct sUrchin : public s_workAreaTemplateWithArgWithCopy<sUrchin, sGenericFormationPerTypeData*>
{
    sVec3_FP m8;
    sVec3_FP m14_positionCurrent;
    sVec3_FP m20_positionTarget;
    sVec3_FP m2C_positionDelta;
    sVec3_FP m38_rotationCurrent;
    sVec3_FP m44_rotationTarget;
    sVec3_FP m50_rotationDelta;
    s_3dModel m5C_model;
    s8 mAC;
    s8 mAD;
    s8 mAE;
    s8 mAF;
    s8 mB0_flags;
    s8 mB1;
    s8 mB2;
    s16 mB4;
    s16 mB6_numTargetables;
    s16 mB8_delay;
    s16 mBC_damage;
    std::vector<sBattleTargetable> mC0_targetable;
    std::vector<sVec3_FP> mC4_position;
    p_workArea mC8;
    sGenericFormationPerTypeData* mCC;
    sSharedFormationState* mD0;
    p_workArea mD4;
    p_workArea mD8;
    //size 0xDC
};

void initBattleTargetables(p_workArea pThis, s_3dModel* pModel, s16* param3, std::vector<sBattleTargetable>& param4, std::vector<sVec3_FP>& param5); // TODO: cleanup

bool updateUrchinAnimationSequence(sUrchin* pThis, u16 param_2, int param_3, int param_4, int param_5)
{
    if (param_3)
    {
        int numFramesInAnimation;
        if (pThis->m5C_model.m30_pCurrentAnimation == nullptr)
        {
            numFramesInAnimation = 0;
        }
        else
        {
            numFramesInAnimation = pThis->m5C_model.m30_pCurrentAnimation->m4_numFrames;
        }
        if (pThis->m5C_model.m16_previousAnimationFrame < numFramesInAnimation - 1)
        {
            return false;
        }
    }

    if (param_2 == 0)
    {
        if (param_4)
        {
            initAnimation(&pThis->m5C_model, nullptr);
        }
    }
    else
    {
        initAnimation(&pThis->m5C_model, pThis->m0_fileBundle->getAnimation(param_2));
        if (param_5)
        {
            int numFrameToSkip = randomNumber() & 0x1F;
            for (int i = 0; i < numFrameToSkip; i++)
            {
                stepAnimation(&pThis->m5C_model);
            }
        }
    }
    return true;
}

void Urchin_init(sUrchin* pThis, sGenericFormationPerTypeData* pConfig)
{
    pThis->mCC = pConfig;

    sModelHierarchy* pHierarchy = pThis->m0_fileBundle->getModelHierarchy(pConfig->m8_modelOffset);
    sStaticPoseData* pStaticPose = pThis->m0_fileBundle->getStaticPose(pConfig->mA_poseOffset, pHierarchy->countNumberOfBones());

    init3DModelRawData(pThis, &pThis->m5C_model, 0, pThis->m0_fileBundle, pConfig->m8_modelOffset, nullptr, pStaticPose, nullptr, pConfig->mC_hotspotDefinitions);
    initBattleTargetables(pThis, &pThis->m5C_model, &pThis->mB6_numTargetables, pThis->mC0_targetable, pThis->mC4_position);
    updateUrchinAnimationSequence(pThis, pConfig->m1C[0].m1C_animationOffset, 0, 1, 1);
    pThis->mC8 = createEnemyLifeMeterTask(&pThis->m8, 0, &pThis->mB4, pConfig->m0_enemyTypeId);

    if (!pConfig->m1C[0].m0.isNull())
    {
        pThis->mD8 = gCurrentBattleOverlay->invokeCreateEffect(pConfig->m1C[0].m0, pThis);
    }

    if ((gBattleManager->m4 == 8) && (gBattleManager->m6_subBattleId == 4)) {
        gBattleManager->m10_battleOverlay->m4_battleEngine->m22F_battleRadarLockIcon =  0xB;
    }
}

std::array<std::array<s8,4>, 4> enemyQuadrantsTable = {
    {
        {
            0x2,
            0x3,
            0x0,
            0x1,
        },
        {
            0x1,
            0x2,
            0x3,
            0x0,
        },
        {
            0x0,
            0x1,
            0x2,
            0x3,
        },
        {
            0x3,
            0x0,
            0x1,
            0x2,
        }
    }
};

void urchin_updateGunHit(sUrchin* pThis)
{
    s8 cVar1 = enemyQuadrantsTable[pThis->mD0->mD[pThis->mAF]][gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant];

    if ((pThis->mB0_flags & 8) == 0)
    {
        if ((pThis->mB0_flags & 0x10) == 0)
        {
            if ((cVar1 != 0) && (gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode == eBattleModes::m0_shootEnemyWithGun) && (gBattleManager->m10_battleOverlay->m4_battleEngine->m38D_battleSubMode == 4))
            {
                for (int i = 0; i < pThis->mB6_numTargetables; i++)
                {
                    if (pThis->mC0_targetable[i].m5A == gBattleManager->m10_battleOverlay->m4_battleEngine->m398_currentSelectedEnemy + 1)
                    {
                        pThis->mB8_delay = gBattleManager->m10_battleOverlay->m4_battleEngine->m398_currentSelectedEnemy * 2 + 0x1E;
                        pThis->mB0_flags |= 8;
                        pThis->mAD = 0;
                        return;
                    }
                }
            }
        }
        else
        {
            stepAnimation(&pThis->m5C_model);
            if (pThis->mB8_delay-- < 0)
            {
                for (int i = 0; i < pThis->mB6_numTargetables; i++)
                {
                    pThis->mC0_targetable[i].m50_flags &= ~0x100000;
                }
                pThis->mB0_flags &= ~0x10;
                pThis->mD0->m14[pThis->mAE].m18_statusFlags |= 0x20;
            }
        }
    }
    else
    {
        if (pThis->mB8_delay-- < 0)
        {
            s32 stack40;
            s32 stack36;
            switch (cVar1)
            {
            case 1:
                if ((randomNumber() & 0x8000000) == 0)
                {
                    stack40 = randomNumber() & 0xFFF;
                }
                else
                {
                    stack40 = randomNumber() | 0xF000;
                }

                if ((randomNumber() & 0x8000000) == 0)
                {
                    stack36 = randomNumber() & 0xFFF;
                }
                else
                {
                    stack36 = randomNumber() | 0xF000;
                }
                break;
            case 2:
                stack40 = 0;
                if ((randomNumber() & 0x8000000) == 0)
                {
                    stack36 = randomNumber() & 0xFFF;
                }
                else
                {
                    stack36 = randomNumber() | 0xF000;
                }
                break;
            default:
                stack40 = 0;
                stack36 = 0;
                break;
            }
            pThis->m2C_positionDelta[1] += MTH_Mul((u32)pThis->mCC->m18_knockbackStrength << 12, getSin(stack40));
            pThis->m2C_positionDelta[0] += MTH_Mul(MTH_Mul((u32)pThis->mCC->m18_knockbackStrength << 12, getCos(stack40)), getSin(stack36));
            pThis->m2C_positionDelta[2] += MTH_Mul(MTH_Mul((u32)pThis->mCC->m18_knockbackStrength << 12, getCos(stack40)), getCos(stack36));

            for (int i = 0; i < pThis->mB6_numTargetables; i++)
            {
                pThis->mC0_targetable[i].m50_flags |= 0x100000;
            }

            pThis->mB8_delay = (gBattleManager->m10_battleOverlay->m4_battleEngine->m398_currentSelectedEnemy * 2) + 0xF;

            pThis->mB0_flags &= ~0x80;
            pThis->mB0_flags |= 0x10;
        }
    }
}

void urchin_updateHomingLaserHit(sUrchin* pThis)
{
    s8 cVar1 = enemyQuadrantsTable[pThis->mD0->mD[pThis->mAF]][gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant];

    if ((pThis->mB0_flags & 8) == 0)
    {
        if ((pThis->mB0_flags & 0x10) == 0)
        {
            if ((cVar1 != 0) && (gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode == eBattleModes::m3_shootEnemeyWithHomingLaser) && (gBattleManager->m10_battleOverlay->m4_battleEngine->m38D_battleSubMode == 4))
            {
                for (int i = 0; i < pThis->mB6_numTargetables; i++)
                {
                    if ((pThis->mC0_targetable[i].m5A <= gBattleManager->m10_battleOverlay->m4_battleEngine->m394) && (pThis->mC0_targetable[i].m5A != 0))
                    {
                        pThis->mB8_delay = gBattleManager->m10_battleOverlay->m4_battleEngine->m394 * 2 + 0xF;
                        pThis->mB0_flags |= 8;
                        return;
                    }
                }
            }
        }
        else
        {
            stepAnimation(&pThis->m5C_model);
            if (pThis->mB8_delay-- < 0)
            {
                for (int i = 0; i < pThis->mB6_numTargetables; i++)
                {
                    pThis->mC0_targetable[i].m50_flags &= ~0x100000;
                }
                pThis->mB0_flags &= ~0x10;
                pThis->mD0->m14[pThis->mAE].m18_statusFlags |= 0x20;
            }
        }
    }
    else
    {
        if (pThis->mB8_delay-- < 0)
        {
            s32 stack40 = 0;
            s32 stack36 = 0;
            switch (cVar1)
            {
            case 1:
                if ((randomNumber() & 0x8000000) == 0) stack40 = randomNumber() & 0xFFF;
                else stack40 = randomNumber() | 0xF000;
                if ((randomNumber() & 0x8000000) == 0) stack36 = randomNumber() & 0xFFF;
                else stack36 = randomNumber() | 0xF000;
                break;
            case 2:
                stack40 = 0;
                if ((randomNumber() & 0x8000000) == 0) stack36 = randomNumber() & 0xFFF;
                else stack36 = randomNumber() | 0xF000;
                break;
            default: stack40 = 0; stack36 = 0; break;
            }
            pThis->m2C_positionDelta[1] += MTH_Mul((u32)pThis->mCC->m18_knockbackStrength << 12, getSin(stack40));
            pThis->m2C_positionDelta[0] += MTH_Mul(MTH_Mul((u32)pThis->mCC->m18_knockbackStrength << 12, getCos(stack40)), getSin(stack36));
            pThis->m2C_positionDelta[2] += MTH_Mul(MTH_Mul((u32)pThis->mCC->m18_knockbackStrength << 12, getCos(stack40)), getCos(stack36));

            for (int i = 0; i < pThis->mB6_numTargetables; i++)
            {
                pThis->mC0_targetable[i].m50_flags |= 0x100000;
            }

            pThis->mB8_delay = gBattleManager->m10_battleOverlay->m4_battleEngine->m394 * 2 + 0x19;
            pThis->mB0_flags = (pThis->mB0_flags & ~0x08) | 0x10;
        }
    }
}

// 06079f9c
s8 urchinUpdateSub2(sUrchin* pThis)
{
    if ((battleEngine_isPlayerTurnActive() == 0) && (pThis->mB1 != pThis->mD0->m7_attackAnimIndex[pThis->mAF]))
    {
        s8 newAnimIndex = pThis->mD0->m7_attackAnimIndex[pThis->mAF];
        u16 newAnimOffset = pThis->mCC->m1C[newAnimIndex].m1C_animationOffset;
        u16 curAnimOffset = pThis->mCC->m1C[pThis->mB1].m1C_animationOffset;
        if (curAnimOffset != newAnimOffset)
        {
            updateUrchinAnimationSequence(pThis, newAnimOffset, 0, 1, 1);
        }
        if (pThis->mD8 != nullptr)
        {
            pThis->mD8->getTask()->markFinished();
        }
        if (!pThis->mCC->m1C[newAnimIndex].m0.isNull())
        {
            pThis->mD8 = gCurrentBattleOverlay->invokeCreateEffect(pThis->mCC->m1C[newAnimIndex].m0, pThis);
        }
        pThis->mB1 = newAnimIndex;
    }

    s8 result = 0x19;
    if (pThis->mD0->m14[pThis->mAE].m19_attackFlags & 2)
    {
        pThis->mD0->m14[pThis->mAE].m18_statusFlags |= 0x10;
        pThis->mD0->m14[pThis->mAE].m19_attackFlags = 0;
        pThis->mAD = 0;

        s8 attackType = pThis->mD0->m0_currentAttack->m8_type;
        if (attackType == 0)
        {
            pThis->mD0->m14[pThis->mAE].m18_statusFlags &= ~0x10;
            result = 0x18;
        }
        else if (attackType == 2)
        {
            result = (s8)0xAC;
            pThis->mAC = 1;
        }
        else if (attackType == 3)
        {
            result = (s8)0xAC;
            pThis->mAC = 2;
        }
        else if (attackType == 5)
        {
            result = (s8)0xAC;
            pThis->mAC = 4;
        }
        else if (attackType == 6)
        {
            result = (s8)0xAC;
            pThis->mAC = 5;
        }
        else if (attackType == 7)
        {
            result = (s8)0xAC;
            pThis->mAC = 6;
        }
    }
    return result;
}

void urchinUpdateSub3(s_3dModel* pModel, std::vector<sVec3_FP>& pPosition)
{
    if (pModel->m40)
    {
        int outputIndex = 0;
        for (int i=0; i<pModel->m12_numBones; i++)
        {
            if (pModel->m44_hotpointData[i].size())
            {
                for (u32 j = 0; j < (*pModel->m40)[i].m4_count; j++)
                {
                    pPosition[outputIndex++] = pModel->m44_hotpointData[i][j];
                }
            }
        }
    }
}

void urchinUpdateSub4(sUrchin* pThis)
{
    int uVar1 = pThis->mCC->m28[enemyQuadrantsTable[pThis->mD0->mD[pThis->mAF]][gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant]];

    for (int i = 0; i < pThis->mB6_numTargetables; i++)
    {
        pThis->mC0_targetable[i].m60 = uVar1;
    }
}

void Urchin_updateSub0(sUrchin* pThis)
{
    int iVar3 = 0;
    switch (pThis->mD0->mD[5] & 6)
    {
    case 2:
        iVar3 = 1;
        break;
    case 4:
        iVar3 = 3;
        break;
    case 6:
        iVar3 = 2;
        break;
    default:
        break;
    }

    pThis->m44_rotationTarget[1] = pThis->mD0->mD[pThis->mAF] << 26;

    for (int i = 0; i < pThis->mB6_numTargetables; i++)
    {
        u32 targetableFlag = pThis->mC0_targetable[i].m50_flags;
        if (((targetableFlag & 0xf0000000) == 0) && ((targetableFlag & 1) == 0))
        {
            pThis->mC0_targetable[i].m50_flags &= ~0xf0000000;

            sSaturnPtr puVar5 = g_BTL_A3->getSaturnPtr(0x60AC254);
            sSaturnPtr puVar6 = g_BTL_A3->getSaturnPtr(0x60AC254 + iVar3 * 4);

            do 
            {
                if (targetableFlag & readSaturnU32(puVar5 + 0))
                {
                    pThis->mC0_targetable[i].m50_flags |= readSaturnU32(puVar6 + 0);
                }

                if (targetableFlag & readSaturnU32(puVar5 + 4))
                {
                    pThis->mC0_targetable[i].m50_flags |= readSaturnU32(puVar6 + 4);
                }

                if (targetableFlag & readSaturnU32(puVar5 + 8))
                {
                    pThis->mC0_targetable[i].m50_flags |= readSaturnU32(puVar6 + 8);
                }

                if (targetableFlag & readSaturnU32(puVar5 + 12))
                {
                    pThis->mC0_targetable[i].m50_flags |= readSaturnU32(puVar6 + 12);
                }

                puVar5 += 0x10;
                puVar6 += 0x10;

            } while (puVar5.m_offset < g_BTL_A3->getSaturnPtr(0x60AC264).m_offset);
        }
    }
}

struct sEnemyLifeMeterTask;
void processHitTargetables(p_workArea pThis, std::vector<sBattleTargetable>& param2, int param3, int param4, sEnemyLifeMeterTask* param5); // TODO cleanup
sVec3_FP* getHitKnockbackDirection(std::vector<sBattleTargetable>& param1, int param2); // TODO cleanup
void createExplosionAtPosition(sVec3_FP* param_1, sVec3_FP* param_2, int param_3, u8 param_4); // TODO cleanup

// 06076994
static void urchinUpdateMode1(sUrchin* pThis)
{
    sSaturnPtr attackParams = pThis->mD0->m0_currentAttack->m0_attackParams;
    s8 subState = pThis->mAD;

    if (subState != 0)
    {
        if (subState == 1)
        {
            pThis->mB8_delay++;
            if (pThis->mB8_delay >= readSaturnU16(attackParams + 0x20))
            {
                pThis->mAD++;
            }
        }
        else if (subState == 2)
        {
            // Move toward target
            pThis->m14_positionCurrent = pThis->m14_positionCurrent + pThis->m2C_positionDelta;
            pThis->m2C_positionDelta[1] = pThis->m2C_positionDelta[1] + readSaturnS32(attackParams + 0x1C);
            pThis->mB8_delay++;
            if (pThis->mB8_delay >= (s16)(readSaturnU16(attackParams + 0x20) + readSaturnU16(attackParams + 0x12)))
            {
                pThis->mAD++;
            }
        }
        else if (subState == 3)
        {
            pThis->mB8_delay++;
            if (pThis->mB8_delay >= (s16)(readSaturnU16(attackParams + 0x20) + readSaturnU16(attackParams + 0x12) + readSaturnU16(attackParams + 0x14)))
            {
                if (readSaturnS8(attackParams + 0x24) != 0)
                {
                    pThis->mAD = 6;
                }
                else if (readSaturnS8(attackParams + 0x25) != 0)
                {
                    // No return movement
                    pThis->m2C_positionDelta[0] = 0;
                    pThis->m2C_positionDelta[1] = 0;
                    pThis->m2C_positionDelta[2] = 0;
                }
                else
                {
                    // Compute return velocity
                    s32 returnDuration = (s32)readSaturnU16(attackParams + 0x12) << 16;
                    pThis->m2C_positionDelta[0] = FP_Div(pThis->m20_positionTarget[0] - pThis->m14_positionCurrent[0], fixedPoint(returnDuration));
                    pThis->m2C_positionDelta[2] = FP_Div(pThis->m20_positionTarget[2] - pThis->m14_positionCurrent[2], fixedPoint(returnDuration));
                    pThis->m2C_positionDelta[1] = FP_Div(pThis->m20_positionTarget[1] - pThis->m14_positionCurrent[1], fixedPoint(returnDuration));
                    pThis->mAD++;
                }
            }
        }
        else if (subState == 4)
        {
            // Return to original position
            pThis->m14_positionCurrent = pThis->m14_positionCurrent + pThis->m2C_positionDelta;
            pThis->mB8_delay++;
            if (pThis->mB8_delay >= (s16)(readSaturnU16(attackParams + 0x20) + readSaturnU16(attackParams + 0x12) + readSaturnU16(attackParams + 0x14) + readSaturnU16(attackParams + 0x16)))
            {
                pThis->m2C_positionDelta[0] = 0;
                pThis->m2C_positionDelta[1] = 0;
                pThis->m2C_positionDelta[2] = 0;
                pThis->mAD++;
            }
        }
        else if (subState == 5)
        {
            pThis->mB8_delay++;
            if (pThis->mB8_delay >= (s16)(readSaturnU16(attackParams + 0x20) + readSaturnU16(attackParams + 0x12) + readSaturnU16(attackParams + 0x14) + readSaturnU16(attackParams + 0x16) + readSaturnU16(attackParams + 0x22)))
            {
                pThis->mD0->m14[pThis->mAE].m18_statusFlags &= ~0x10;
                pThis->mAC = 0;
                pThis->mAD = 0;
            }
        }
        else if (subState == 6)
        {
            // Skip to death: clear delta, set mAC=6
            pThis->m2C_positionDelta.zeroize();
            pThis->mAC = 6;
            pThis->mAD = 0;
        }
        goto mode1_epilogue;
    }

    // subState 0: init
    pThis->mB8_delay = -1;
    if (!(gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2000))
        goto mode1_epilogue;

    battleEngine_FlagQuadrantForAttack(gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant);

    if (readSaturnS8(attackParams + 0x25) == 0)
    {
        u16 chargeDuration = readSaturnU16(attackParams + 0x12);
        sVec3_FP dragonPos;
        dragonPos[0] = gBattleManager->m10_battleOverlay->m18_dragon->m8_position[0];
        dragonPos[1] = gBattleManager->m10_battleOverlay->m18_dragon->m8_position[1];
        dragonPos[2] = gBattleManager->m10_battleOverlay->m18_dragon->m8_position[2];

        s8 dragonQuadrant = gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant;
        s32 offset = readSaturnS32(attackParams + 0x18);
        if (dragonQuadrant == 0)
        {
            dragonPos[2] -= offset;
        }
        else if (dragonQuadrant == 1)
        {
            dragonPos[0] -= offset;
        }
        else if (dragonQuadrant == 2)
        {
            dragonPos[2] += offset;
        }
        else if (dragonQuadrant == 3)
        {
            dragonPos[0] += offset;
        }

        s32 duration = (s32)chargeDuration << 16;
        pThis->m2C_positionDelta[0] = FP_Div(dragonPos[0] - pThis->m14_positionCurrent[0], fixedPoint(duration));
        pThis->m2C_positionDelta[2] = FP_Div(dragonPos[2] - pThis->m14_positionCurrent[2], fixedPoint(duration));
        pThis->m2C_positionDelta[1] = FP_Div(dragonPos[1] - pThis->m14_positionCurrent[1], fixedPoint(duration)) - (readSaturnS32(attackParams + 0x1C) >> 1) * chargeDuration;
    }
    else
    {
        pThis->m2C_positionDelta[0] = 0;
        pThis->m2C_positionDelta[1] = 0;
        pThis->m2C_positionDelta[2] = 0;
    }

    pThis->mB8_delay = 0;
    pThis->mAD++;

mode1_epilogue:
    // Check for attack animation trigger
    if (pThis->mB8_delay == readSaturnU16(attackParams + 0x10) && readSaturnS16(attackParams + 0x0E) != 0)
    {
        initAnimation(&pThis->m5C_model, pThis->m0_fileBundle->getAnimation(readSaturnU16(attackParams + 0x0E)));
        pThis->mB0_flags |= 0x20;
    }
    else if ((pThis->mB0_flags & 0x20) && updateUrchinAnimationSequence(pThis, pThis->mCC->m1C[pThis->mB1].m1C_animationOffset, 1, 1, 1))
    {
        pThis->mB0_flags &= ~0x20;
    }

    // Check for sound/effect trigger
    if (pThis->mB8_delay == readSaturnU16(attackParams + 0x0C) && !readSaturnEA(attackParams + 0x08).isNull())
    {
        gCurrentBattleOverlay->invokeCreateEffect(readSaturnEA(attackParams + 0x08), pThis);
    }
}

// 06075d5e
static void urchinUpdateMode2(sUrchin* pThis)
{
    sSaturnPtr attackParams = pThis->mD0->m0_currentAttack->m0_attackParams;
    switch (pThis->mAD)
    {
    case 0:
    {
        if (!(gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2000))
            return;

        if (readSaturnS16(attackParams + 0x0E) != 0)
        {
            initAnimation(&pThis->m5C_model, pThis->m0_fileBundle->getAnimation(readSaturnU16(attackParams + 0x0E)));
            pThis->mB0_flags |= 0x20;
        }
        if (!readSaturnEA(attackParams + 0x08).isNull())
        {
            pThis->mB8_delay = readSaturnS16(attackParams + 0x0C);
        }
        else
        {
            pThis->mB8_delay = -1;
        }
        if (pThis->mD4 != nullptr)
        {
            pThis->mD4->getTask()->markFinished();
            pThis->mD4 = nullptr;
        }
        // Flag quadrants for attack based on attack flags
        {
            s8 dir = pThis->mD0->mD[pThis->mAF];
            u8 attackFlags = readSaturnU8(attackParams + 4);
            for (int q = 0; q < 4; q++)
            {
                s8 mapped = enemyQuadrantsTable[dir][q];
                if (attackFlags & (1 << q))
                {
                    battleEngine_FlagQuadrantForAttack(mapped);
                }
            }
        }
        pThis->mAD++;
        return;
    }
    case 1:
    {
        if (pThis->mB8_delay >= 0)
        {
            pThis->mB8_delay--;
            if (pThis->mB8_delay == 0 && !readSaturnEA(attackParams + 0x08).isNull())
            {
                pThis->mD4 = gCurrentBattleOverlay->invokeCreateEffect(readSaturnEA(attackParams + 0x08), pThis);
            }
        }
        if ((pThis->mB0_flags & 0x20) &&
            updateUrchinAnimationSequence(pThis, pThis->mCC->m1C[pThis->mD0->m7_attackAnimIndex[pThis->mAF]].m1C_animationOffset, 1, 1, 0))
        {
            pThis->mB0_flags &= ~0x20;
        }
        if ((pThis->mB8_delay < 0) && (pThis->mD4 == nullptr) && !(pThis->mB0_flags & 0x20))
        {
            pThis->mD0->m14[pThis->mAE].m18_statusFlags &= ~0x10;
            pThis->mAD++;
        }
        return;
    }
    case 2:
        pThis->mAC = 0;
        pThis->mAD = 0;
        return;
    default:
        return;
    }
}

// 0607610a
static void urchinUpdateMode4(sUrchin* pThis)
{
    sSaturnPtr attackParams = pThis->mD0->m0_currentAttack->m0_attackParams;
    switch (pThis->mAD)
    {
    case 0:
    {
        pThis->mB8_delay = -1;
        if (!(gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2000))
            return;
        if (readSaturnS16(attackParams + 0x06) != 0)
        {
            initAnimation(&pThis->m5C_model, pThis->m0_fileBundle->getAnimation(readSaturnU16(attackParams + 0x06)));
            pThis->mB0_flags |= 0x20;
        }
        pThis->mAD++;
        pThis->mB8_delay = 0;
        return;
    }
    case 1:
    {
        if ((pThis->mB0_flags & 0x20) &&
            updateUrchinAnimationSequence(pThis, pThis->mCC->m1C[pThis->mD0->m7_attackAnimIndex[pThis->mAF]].m1C_animationOffset, 1, 1, 0))
        {
            pThis->mB0_flags &= ~0x20;
        }
        if (!(pThis->mB0_flags & 0x20))
        {
            pThis->mD0->m14[pThis->mAE].m18_statusFlags &= ~0x10;
            pThis->mAD++;
        }
        pThis->mB8_delay++;
        return;
    }
    case 2:
        pThis->mAC = 0;
        pThis->mAD = 0;
        pThis->mB8_delay++;
        return;
    default:
        pThis->mB8_delay++;
        return;
    }
}

// 06076232
static void urchinUpdateMode5(sUrchin* pThis)
{
    sSaturnPtr attackParams = pThis->mD0->m0_currentAttack->m0_attackParams;
    stepAnimation(&pThis->m5C_model);
    switch (pThis->mAD)
    {
    case 0:
    {
        if (!(gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2000))
            return;

        // Delete all targetables
        for (int i = 0; i < pThis->mB6_numTargetables; i++)
        {
            deleteTargetable(&pThis->mC0_targetable[i]);
        }
        pThis->mB6_numTargetables = 0;

        // Offset target position based on dragon quadrant and attack flags
        s8 dragonQuadrant = gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant;
        u8 attackFlag = readSaturnU8(attackParams + 4) & 1;

        sSaturnPtr offsetTable = g_BTL_A3->getSaturnPtr(0x060ac294 + dragonQuadrant * 0xC + attackFlag * 0x30);
        pThis->m20_positionTarget[0] += readSaturnS32(offsetTable + 0) * 0x400;
        pThis->m20_positionTarget[1] += readSaturnS32(offsetTable + 4) * 0x400;
        pThis->m20_positionTarget[2] += readSaturnS32(offsetTable + 8) * 0x400;

        if (pThis->mCC->m28[4] == 4)
        {
            pThis->mB8_delay = 300;
            pThis->m2C_positionDelta[0] = 0;
            pThis->m2C_positionDelta[1] = fixedPoint(0xFFFFFF50);
            pThis->m2C_positionDelta[2] = 0;
        }
        else
        {
            pThis->mB8_delay = 0x3C;
            pThis->m2C_positionDelta[0] = FP_Div(pThis->m20_positionTarget[0] - pThis->m14_positionCurrent[0], fixedPoint::fromInteger(pThis->mB8_delay));
            pThis->m2C_positionDelta[1] = 0;
            pThis->m2C_positionDelta[2] = FP_Div(pThis->m20_positionTarget[2] - pThis->m14_positionCurrent[2], fixedPoint::fromInteger(pThis->mB8_delay));
        }
        pThis->mAD++;
        return;
    }
    case 1:
    {
        pThis->m14_positionCurrent = pThis->m14_positionCurrent + pThis->m2C_positionDelta;
        pThis->mB8_delay--;
        if (pThis->mB8_delay < 0)
        {
            if ((gBattleManager->m4 == 7) &&
                (gBattleManager->m6_subBattleId == 4 || gBattleManager->m6_subBattleId == 8 || gBattleManager->m6_subBattleId == 9))
            {
                gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m10000 = 0;
            }
            pThis->mD0->m14[pThis->mAE].m18_statusFlags &= ~0x10;
            pThis->mD0->m14[pThis->mAE].m18_statusFlags |= 8;
            pThis->mD0->m14[pThis->mAE].m18_statusFlags |= 4;

            if (pThis->mC8 != nullptr)
            {
                pThis->mC8->getTask()->markFinished();
                pThis->mC8 = nullptr;
            }
            pThis->getTask()->markFinished();
        }
        return;
    }
    default:
        return;
    }
}

// 06076b24
static void urchinUpdateMode6(sUrchin* pThis)
{
    switch (pThis->mAD)
    {
    case 0:
    {
        // Delete all targetables
        for (int i = 0; i < pThis->mB6_numTargetables; i++)
        {
            deleteTargetable(&pThis->mC0_targetable[i]);
        }
        pThis->mB6_numTargetables = 0;

        // Play death sound
        if (pThis->mCC->m28[2] != -1)
        {
            playSystemSoundEffect(pThis->mCC->m28[2]);
        }

        // Kill pending effect task
        if (pThis->mC8 != nullptr)
        {
            pThis->mC8->getTask()->markFinished();
        }
        pThis->mAD++;
        break;
    }
    case 1:
        break;
    default:
        return;
    }

    s8 deathMode = pThis->mCC->m28[0];
    if (deathMode == '\0')
    {
        createExplosionAtPosition(&pThis->m8, &gBattleManager->m10_battleOverlay->m4_battleEngine->m1A0_battleAutoScrollDelta, 0x50000, 0);
        pThis->mD0->m14[pThis->mAE].m18_statusFlags &= ~0x10;
        pThis->mD0->m14[pThis->mAE].m18_statusFlags |= 4;
        pThis->getTask()->markFinished();
    }
    else if (deathMode != '\x01')
    {
        if (deathMode == '\x02')
        {
            createExplosionAtPosition(&pThis->m8, &gBattleManager->m10_battleOverlay->m4_battleEngine->m1A0_battleAutoScrollDelta, 0x50000, 0);
        }
        else if (deathMode != '\x03')
        {
            if (deathMode == '\x04')
            {
                pThis->mD0->m14[pThis->mAE].m18_statusFlags &= ~0x10;
                pThis->mD0->m14[pThis->mAE].m18_statusFlags |= 4;
                pThis->mAD = 3;
            }
            else
            {
                if (deathMode != '\x05') goto LAB_06076d1c;
                pThis->mD0->m14[pThis->mAE].m18_statusFlags &= ~0x10;
                pThis->mD0->m14[pThis->mAE].m18_statusFlags |= 4;
                pThis->mAD++;
            }
            return;
        }
LAB_06076d1c:
        pThis->mD0->m14[pThis->mAE].m18_statusFlags &= ~0x10;
        pThis->mD0->m14[pThis->mAE].m18_statusFlags |= 4;
        createBoneDebrisExplosion(
            (s_workAreaCopy*)dramAllocatorEnd[(u8)pThis->mCC->m1_fileBundleIndex].mC_fileBundle,
            &pThis->m5C_model, &pThis->m8, &pThis->m38_rotationCurrent,
            nullptr, nullptr, -44, nullptr, 0, 0, 0, 0x10000, 0);
        pThis->getTask()->markFinished();
        return;
    }
    pThis->mD0->m14[pThis->mAE].m18_statusFlags &= ~0x10;
    pThis->mD0->m14[pThis->mAE].m18_statusFlags |= 4;
    pThis->getTask()->markFinished();
}

// 060770f0
void Urchin_update(sUrchin* pThis)
{
    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x1B])
    {
        // Debug display — not critical
    }

    pThis->m8 = gBattleManager->m10_battleOverlay->m4_battleEngine->mC_battleCenter + pThis->m14_positionCurrent;
    pThis->mD0->m14[pThis->mAE].m0_worldPosition = pThis->m8;

    // Falling death animation (mAC==6, mAD==3)
    if ((pThis->mAC == 6) && (pThis->mAD == 3))
    {
        if ((gBattleManager->m4 == 7) &&
            (gBattleManager->m10_battleOverlay->m4_battleEngine->m3B0_subBattleId == 6))
        {
            pThis->m8[1] = pThis->m8[1] + fixedPoint(-0x199);
        }
        else
        {
            pThis->m8[1] = pThis->m8[1] + fixedPoint(-0x1000);
        }
        if ((randomNumber() & 3) != 0)
        {
            if ((gBattleManager->m4 == 7) &&
                (gBattleManager->m10_battleOverlay->m4_battleEngine->m3B0_subBattleId == 6))
            {
                pThis->m8[1] = pThis->m8[1] + fixedPoint(-0x199);
            }
            else
            {
                pThis->m8[1] = pThis->m8[1] + fixedPoint(-0x1000);
            }
            createExplosionAtPosition(&pThis->m8, &pThis->m2C_positionDelta, 0x50000, 2);
        }
        if (pThis->m8[1] < fixedPoint(0))
        {
            pThis->mAD = 2;
        }
    }

    if ((pThis->mD0->m14[pThis->mAE].m18_statusFlags & 4) == 0)
    {
        pThis->mD0->m4_typeAlive[pThis->mAF] = 1;

        if (pThis->mCC->m2)
        {
            pThis->mD0->mD[3] = 1;
        }

        // Update target position from formation data
        if (pThis->mD0->mD[5] & 1)
        {
            pThis->m20_positionTarget = pThis->mD0->m14[pThis->mAE].mC_initialPosition;
        }

        if (pThis->mD0->mD[5] & 6)
        {
            Urchin_updateSub0(pThis);
        }

        stepAnimation(&pThis->m5C_model);

        // Check if pending tasks finished
        if (pThis->mD8 && pThis->mD8->getTask()->isFinished())
        {
            pThis->mD8 = nullptr;
        }
        if (pThis->mD4 && pThis->mD4->getTask()->isFinished())
        {
            pThis->mD4 = nullptr;
        }

        switch (pThis->mAC)
        {
        case 0:
        {
            s32 rotDeltaFactor = 0x1999;
            s32 rotDistFactor = 0x28F;
            springDampedStep(&pThis->m14_positionCurrent, &pThis->m2C_positionDelta, &pThis->m20_positionTarget, 0x1999, 0x28F, 0);

            if (gBattleManager->m4 == 5)
            {
                s8 subBattleId = gBattleManager->m10_battleOverlay->m4_battleEngine->m3B0_subBattleId;
                if (subBattleId == 4 || subBattleId == 5 || subBattleId == 8 || subBattleId == 9 || subBattleId == 10)
                {
                    rotDeltaFactor = 0x2666;
                }
            }
            else if (gBattleManager->m4 == 8)
            {
                s8 subBattleId = gBattleManager->m10_battleOverlay->m4_battleEngine->m3B0_subBattleId;
                if (subBattleId == 1 || subBattleId == 2 || subBattleId == 3)
                {
                    rotDeltaFactor = 0x2666;
                    rotDistFactor = 0x51E;
                }
                else if (subBattleId == 5)
                {
                    rotDeltaFactor = 0x4000;
                    rotDistFactor = 0xCCC;
                }
            }

            springDampedStep(&pThis->m38_rotationCurrent, &pThis->m50_rotationDelta, &pThis->m44_rotationTarget, rotDeltaFactor, rotDistFactor, 1);
            urchin_updateGunHit(pThis);
            urchin_updateHomingLaserHit(pThis);
            urchinUpdateSub2(pThis);
            break;
        }
        case 1:
            urchinUpdateMode1(pThis);
            break;
        case 2:
            urchinUpdateMode2(pThis);
            break;
        case 4:
            urchinUpdateMode4(pThis);
            break;
        case 5:
            urchinUpdateMode5(pThis);
            break;
        case 6:
            urchinUpdateMode6(pThis);
            break;
        default:
            break;
        }

        urchinUpdateSub3(&pThis->m5C_model, pThis->mC4_position);

        // Post-update: m38 flag check (death trigger from formation)
        if (pThis->mCC->m38 && !(pThis->mD0->m14[pThis->mAE].m18_statusFlags & 8) && pThis->mD0->mD[4] != 0)
        {
            pThis->mD0->m14[pThis->mAE].m18_statusFlags |= 8;
            if (pThis->mC8)
            {
                p_workArea lifeMeter = pThis->mC8;
                // Mark life meter flags for death
                ((u8*)lifeMeter)[0x31] |= 1;
                ((u8*)lifeMeter)[0x31] |= 8;
            }
            pThis->mC8 = nullptr;
            pThis->mAC = 6;
            pThis->mAD = 0;
        }

        // Damage handling
        s16 local_30;
        if (checkTargetablesForDamage(pThis, pThis->mC0_targetable, pThis->mB6_numTargetables, local_30) == 0)
        {
            if (pThis->mB0_flags & 4)
            {
                if (pThis->mCC->m4 == 0)
                {
                    pThis->mB0_flags &= ~4;
                }
                else if (updateUrchinAnimationSequence(pThis, pThis->mCC->m1C[pThis->mB1].m1C_animationOffset, 1, 1, 1))
                {
                    pThis->mB0_flags &= ~4;
                }
            }
        }
        else
        {
            // Took damage
            pThis->mB0_flags |= 2;
            pThis->mD0->m14[pThis->mAE].m18_statusFlags |= 0x20;
            pThis->mB2 = 1;
            pThis->mBC_damage += local_30;
            pThis->mB4 -= local_30;
            if (pThis->mB4 < 1)
            {
                pThis->mB4 = 0;
                processHitTargetables(pThis, pThis->mC0_targetable, pThis->mB6_numTargetables, 0, (sEnemyLifeMeterTask*)pThis->mC8);
                createDamageDisplayTask(pThis, pThis->mBC_damage, &pThis->m8, 0);
                pThis->mD0->m14[pThis->mAE].m18_statusFlags |= 8;
                if (pThis->mC8)
                {
                    ((u8*)pThis->mC8)[0x31] |= 1;
                    ((u8*)pThis->mC8)[0x31] |= 8;
                }
                pThis->mC8 = nullptr;
                pThis->mAC = 6;
                pThis->mAD = 0;
                goto post_damage;
            }

            sVec3_FP* knockbackDir = getHitKnockbackDirection(pThis->mC0_targetable, pThis->mB6_numTargetables);
            if (knockbackDir)
            {
                pThis->m2C_positionDelta[0] += MTH_Mul(fixedPoint(pThis->mCC->m18_knockbackStrength * 0x10000), (*knockbackDir)[0]);
                pThis->m2C_positionDelta[1] += MTH_Mul(fixedPoint(pThis->mCC->m18_knockbackStrength * 0x10000), (*knockbackDir)[1]);
                pThis->m2C_positionDelta[2] += MTH_Mul(fixedPoint(pThis->mCC->m18_knockbackStrength * 0x10000), (*knockbackDir)[2]);

                pThis->m14_positionCurrent[0] += MTH_Mul(fixedPoint(pThis->mCC->m18_knockbackStrength * 0x10000 + 0x10000), fixedPoint(randomNumber() & 0xFFFF));
                pThis->m14_positionCurrent[1] += MTH_Mul(fixedPoint(pThis->mCC->m18_knockbackStrength * 0x10000 + 0x10000), fixedPoint(randomNumber() & 0xFFFF));
                pThis->m14_positionCurrent[2] += MTH_Mul(fixedPoint(pThis->mCC->m18_knockbackStrength * 0x10000 + 0x10000), fixedPoint(randomNumber() & 0xFFFF));
            }

            processHitTargetables(pThis, pThis->mC0_targetable, pThis->mB6_numTargetables, 1, (sEnemyLifeMeterTask*)pThis->mC8);
            pThis->mB0_flags |= 4;
            if (pThis->mCC->m4 != 0)
            {
                initAnimation(&pThis->m5C_model, pThis->m0_fileBundle->getAnimation(pThis->mCC->m4));
            }
            if (pThis->mCC->m28[1] != -1)
            {
                playSystemSoundEffect(pThis->mCC->m28[1]);
            }
        }
post_damage:

        urchinUpdateSub4(pThis);

        if (battleEngine_isPlayerTurnActive() == 0)
        {
            pThis->mD0->m14[pThis->mAE].m18_statusFlags &= ~0x20;
        }

        pThis->mD0->m14[pThis->mAE].m1A_hpRatio = FP_Div(pThis->mB4 * 0x640000, fixedPoint::fromInteger(readSaturnS16(gCommonFile->getSaturnPtr(0x0020179c) + pThis->mCC->m0_enemyTypeId * 4))).toInteger();

        // Healing from m1C_flags
        if (pThis->mD0->m14[pThis->mAE].m1C_flags != 0)
        {
            s16 maxHp = readSaturnS16(gCommonFile->getSaturnPtr(0x0020179c) + pThis->mCC->m0_enemyTypeId * 4);
            pThis->mB4 += (s16)pThis->mD0->m14[pThis->mAE].m1C_flags;
            if (pThis->mB4 > maxHp)
            {
                pThis->mB4 = maxHp;
            }
            if (pThis->mC8)
            {
                ((u8*)pThis->mC8)[0x31] |= 2;
            }
            createDamageDisplayTask(pThis, -(s16)pThis->mD0->m14[pThis->mAE].m1C_flags, &pThis->m8, 0);
            pThis->mD0->m14[pThis->mAE].m1C_flags = 0;
        }

        if (pThis->mB2 && gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m1000)
        {
            createDamageDisplayTask(pThis, pThis->mBC_damage, &pThis->m8, 1);
            pThis->mBC_damage = 0;
            pThis->mB2 = 0;
        }
    }
}

void setupConditionalLightColor(int); // TODO cleanup
void clearLightColor(); // TODO cleanup

void Urchin_draw(sUrchin* pThis)
{
    pushCurrentMatrix();
    translateCurrentMatrix(pThis->m8);
    rotateCurrentMatrixYXZ(pThis->m38_rotationCurrent);
    if (pThis->mB0_flags & 2)
    {
        s32 quadrant = enemyQuadrantsTable[pThis->mD0->mD[pThis->mAF]][gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant];
        setupConditionalLightColor(pThis->mCC->m28[quadrant]);
    }
    pThis->m5C_model.m18_drawFunction(&pThis->m5C_model);
    if (pThis->mB0_flags & 2)
    {
        clearLightColor();
        pThis->mB0_flags &= ~2;
    }
    popMatrix();
}

// 06077b08
void Urchin_delete(sUrchin* pThis)
{
    if (pThis->mC8)
    {
        pThis->mC8->getTask()->markFinished();
    }
    if (pThis->mD4)
    {
        pThis->mD4->getTask()->markFinished();
    }
    if (pThis->mD8)
    {
        pThis->mD8->getTask()->markFinished();
    }
}

void createUrchin(sGenericFormationPerTypeData* pConfig, sSharedFormationState& param2, int param3, int param4)
{
    static const sUrchin::TypedTaskDefinition definition = {
        Urchin_init,
        Urchin_update,
        Urchin_draw,
        Urchin_delete,
    };

    sUrchin* pNewTask = createSubTaskWithArgWithCopy<sUrchin>(dramAllocatorEnd[pConfig->m1_fileBundleIndex].mC_fileBundle, pConfig, &definition);

    pNewTask->mD0 = &param2;
    pNewTask->mAE = param3;
    pNewTask->mAF = param4;

    sPerEnemySlot* iVar4 = &param2.m14[pNewTask->mAE];
    iVar4->m18_statusFlags |= param4;

    pNewTask->m14_positionCurrent = iVar4->mC_initialPosition;
    pNewTask->m20_positionTarget = iVar4->mC_initialPosition;

    switch (param2.mD[pNewTask->mAF])
    {
    case 0:
        pNewTask->m38_rotationCurrent[1] = 0;
        break;
    case 1:
        pNewTask->m38_rotationCurrent[1] = 0x4000000;
        break;
    case 2:
        pNewTask->m38_rotationCurrent[1] = 0x8000000;
        break;
    case 3:
        pNewTask->m38_rotationCurrent[1] = 0xC000000;
        break;
    default:
        assert(0);
    }

    pNewTask->m44_rotationTarget = pNewTask->m38_rotationCurrent;

    iVar4->m18_statusFlags &= ~4;
}
