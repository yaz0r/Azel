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
#include "battle/battleDamageDisplay.h"
#include "battle/battleEnemyLifeMeter.h"
#include "commonOverlay.h"
#include "BTL_A3_data.h"
#include "kernel/graphicalObject.h"

void Baldor_updateSub1(sVec3_FP* pCurrent, sVec3_FP* pDelta, sVec3_FP* pTarget, s32 pDeltaFactor, s32 pDistanceToTargetFactor, s8 translationOrRotation); // TODO: cleanup
s32 Baldor_updateSub0Sub0(p_workArea pThis, std::vector<sBattleTargetable>& param2, s16 entriesToParse, s16& param4); // TODO: cleanup

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

void Baldor_initSub0Sub1(p_workArea pThis, s_3dModel* pModel, s16* param3, std::vector<sBattleTargetable>& param4, std::vector<sVec3_FP>& param5); // TODO: cleanup

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
    Baldor_initSub0Sub1(pThis, &pThis->m5C_model, &pThis->mB6_numTargetables, pThis->mC0_targetable, pThis->mC4_position);
    updateUrchinAnimationSequence(pThis, pConfig->m1C[0].m1C_animationOffset, 0, 1, 1);
    pThis->mC8 = createEnemyLifeMeterTask(&pThis->m8, 0, &pThis->mB4, pConfig->m0_enemyTypeId);

    if (!pConfig->m1C[0].m0.isNull())
    {
        Unimplemented();
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

void urchinUpdateSub0(sUrchin* pThis)
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
        assert(0); // untested
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
                assert(0);
                break;
            }
            pThis->m2C_positionDelta[1] += MTH_Mul(pThis->mCC->m18_knockbackStrength * 0x10000, getSin(stack40));
            pThis->m2C_positionDelta[0] += MTH_Mul(MTH_Mul(pThis->mCC->m18_knockbackStrength * 0x10000, getCos(stack40)), getSin(stack36));
            pThis->m2C_positionDelta[2] += MTH_Mul(MTH_Mul(pThis->mCC->m18_knockbackStrength * 0x10000, getCos(stack40)), getCos(stack36));

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

void urchinUpdateSub1(sUrchin* pThis)
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
                        pThis->mB8_delay = gBattleManager->m10_battleOverlay->m4_battleEngine->m394 * 4 + 0xF;
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
        assert(0);
    }
}

void urchinUpdateSub2(sUrchin* pThis)
{
    if ((BattleEngineSub0_UpdateSub0() == 0) && (pThis->mB1 !=  pThis->mD0->m7_attackAnimIndex[pThis->mAF]))
    {
        assert(0);
    }

    if (pThis->mD0->m14[pThis->mAF].m19_attackFlags & 2)
    {
        assert(0);
    }
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
                for (int j = 0; j < (*pModel->m40)[i].m4_count; j++)
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
        assert(0);
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
                    pThis->mC0_targetable[i].m50_flags |= readSaturnU32(puVar6 + 10);
                }

                puVar5 += 0x10;
                puVar6 += 0x10;

            } while (puVar5.m_offset < g_BTL_A3->getSaturnPtr(0x60AC264).m_offset);
        }
    }
}

void Urchin_update(sUrchin* pThis)
{
    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x1B])
    {
        assert(0);
    }

    pThis->m8 = gBattleManager->m10_battleOverlay->m4_battleEngine->mC_battleCenter + pThis->m14_positionCurrent;
    pThis->mD0->m14[pThis->mAE].m0_worldPosition = pThis->m8;

    if ((pThis->mAC == 6) && (pThis->mAD == 6))
    {
        assert(0);
    }

    if ((pThis->mD0->m14[pThis->mAE].m18_statusFlags & 4) == 0)
    {
        pThis->mD0->m4_typeAlive[pThis->mAF] = 1;

        if (pThis->mCC->m2)
        {
            pThis->mD0->mD[3] = 1;
        }

        if (pThis->mD0->mD[5] & 1)
        {
            assert(0);
        }

        if (pThis->mD0->mD[5] & 6)
        {
            Urchin_updateSub0(pThis);
        }

        stepAnimation(&pThis->m5C_model);

        if (pThis->mD8)
        {
            assert(0);
        }

        if (pThis->mD4)
        {
            assert(0);
        }

        switch (pThis->mAC)
        {
        case 0:
            Baldor_updateSub1(&pThis->m14_positionCurrent, &pThis->m2C_positionDelta, &pThis->m20_positionTarget, 0x1999, 0x28F, 0);

            switch (gBattleManager->m4)
            {
            case 5:
                assert(0);
            case 8:
                assert(0);
            default:
                Baldor_updateSub1(&pThis->m38_rotationCurrent, &pThis->m50_rotationDelta, &pThis->m44_rotationTarget, 0x1999, 0x28F, 1);
                break;
            }

            urchinUpdateSub0(pThis);
            urchinUpdateSub1(pThis);
            urchinUpdateSub2(pThis);

            break;
        default:
            assert(0);
        }

        urchinUpdateSub3(&pThis->m5C_model, pThis->mC4_position);

        if (pThis->mCC->m38)
        {
            assert(0);
        }

        s16 local_30;
        if (Baldor_updateSub0Sub0(pThis, pThis->mC0_targetable, pThis->mB6_numTargetables, local_30) == 0)
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
            assert(0);
        }

        urchinUpdateSub4(pThis);

        if (BattleEngineSub0_UpdateSub0() == 0)
        {
            pThis->mD0->m14[pThis->mAE].m18_statusFlags &= ~0x20;
        }

        pThis->mD0->m14[pThis->mAE].m1A_hpRatio = FP_Div(pThis->mB4 * 0x640000, fixedPoint::fromInteger(readSaturnS16(gCommonFile->getSaturnPtr(0x0020179c) + pThis->mCC->m0_enemyTypeId * 4))).toInteger();

        if (pThis->mD0->m14[pThis->mAE].m1C_flags != 0)
        {
            assert(0);
        }

        if (pThis->mB2 && gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m1000)
        {
            createDamageDisplayTask(pThis, pThis->mBC_damage, &pThis->m8, 1);
            pThis->mBC_damage = 0;
            pThis->mB2 = 0;
        }
    }
}

void Urchin_draw(sUrchin* pThis)
{
    pushCurrentMatrix();
    translateCurrentMatrix(pThis->m8);
    rotateCurrentMatrixYXZ(pThis->m38_rotationCurrent);
    if (pThis->mB0_flags & 2)
    {
        assert(0);
    }
    pThis->m5C_model.m18_drawFunction(&pThis->m5C_model);
    if (pThis->mB0_flags & 2)
    {
        assert(0);
    }
    popMatrix();
}

void Urchin_delete(sUrchin* pThis)
{
    Unimplemented();
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
