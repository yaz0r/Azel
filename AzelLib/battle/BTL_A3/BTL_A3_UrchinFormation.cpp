#include "PDS.h"
#include "BTL_A3_UrchinFormation.h"
#include "BTL_A3_data.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "battle/battleEngine.h"
#include "battle/battleDebug.h"
#include "battle/battleEngineSub0.h"
#include "battle/battleTextDisplay.h"
#include "urchin.h"

#include "kernel/graphicalObject.h"
#include "audio/soundDriver.h" // TODO cleanup

s32 createBattleIntroTaskSub1(); //TODO cleanup
void battleEngine_PlayAttackCamera(int param1); // TODO cleanup
void battleEngine_displayAttackName(int param1, int param2, int param3); // TODO cleanup

s32 playBattleSoundEffect(s32 effectIndex) // TODO cleanup
{
    enqueuePlaySoundEffect(effectIndex, 2, 0, 0);
    return 0;
}

struct BTL_A3_UrchinFormation : public s_workAreaTemplateWithArg<BTL_A3_UrchinFormation, const sGenericFormationData*>
{
    s8 m1_state;
    s8 m2_subState;
    s8 m3_formationSize;
    s8 m4_formationPhase;
    s8 m6_attackFlag;
    s8 m7_attackMode;
    s16 m14_timer;
    sBTL_A3_UrchinFormation_18 m18;
    const sGenericFormationData* m30_config;
    s8 m34_attackingTypeIndex;
    s8 m38_attackOrderReversed;
    s8 m39_sourceEnemyIndex;
    s8 m3A_weakestEnemyIndex;
    s8 m3C_attackTimer;
    p_workArea m40_pendingTask;
    std::array<s8, 3> m44_prevSoundEffects;
    std::array<s8, 3> m47_beamHitHandled; // size unknown, should be formation size
    // size 0x4C
};

void formationCopyParams(sBTL_A3_UrchinFormation_18* pDest, const std::vector<sVec3_FP>& pSource, int count)
{
    for (int i = 0; i < 3; i++)
    {
        pDest->m4[i] = 1;
        pDest->m7[i] = 0;
        pDest->mA[i] = 0;
    }
    pDest->mD[3] = 1;

    for (int i = 0; i < count; i++)
    {
        pDest->m14[i].m0.zeroize();
        pDest->m14[i].mC = pSource[i];
        pDest->m14[i].m19 = 0;
        pDest->m14[i].m18 = 4;
        pDest->m14[i].m1C = 0;
    }
}

void BTL_A3_UrchinFormation_Init(BTL_A3_UrchinFormation* pThis, const sGenericFormationData* config)
{
    pThis->m30_config = config;
    pThis->m3_formationSize = config->m0_formationSize;
    pThis->m18.m14.resize(pThis->m3_formationSize);
    formationCopyParams(&pThis->m18, config->m10_formationSubData[0]->m0_perEnemyPosition, pThis->m3_formationSize);
    if (createBattleIntroTaskSub1() == 0)
    {
        pThis->m1_state = 1;
        pThis->m2_subState = 0;
    }
    else
    {
        if (gBattleManager->m10_battleOverlay->m4_battleEngine->m230 != 8)
        {
            pThis->m1_state = 0;
            pThis->m2_subState = 0;
            return;
        }
        pThis->m1_state = 1;
        pThis->m2_subState = 0;
    }

    bool bVar3 = false;
    bool bVar2 = false;
    int numModelIndex = 0;
    for (int i = 0; i < 3; i++)
    {
        pThis->m44_prevSoundEffects[i] = -1;
        sGenericFormationPerTypeData* pEnemyEntry = config->m4_perTypeParams[i];
        if (pEnemyEntry)
        {
            allocateNPC(pThis, pEnemyEntry->m1_fileBundleIndex);
            pThis->m18.mD[i] = config->m18[i];
            for (int j = 0; j < config->m1_perTypeCount[i]; j++)
            {
                createUrchin(pEnemyEntry, pThis->m18, numModelIndex, i);
                numModelIndex++;
            }

            if (config->m20[i] != -1)
            {
                assert(0);
            }

            if (pEnemyEntry->m24)
            {
                bVar3 = true;
            }

            if (pEnemyEntry->m2)
            {
                bVar2 = true;
            }
        }
    }

    if (bVar2 && bVar3)
    {
        pThis->m7_attackMode = 2;
    }
    pThis->m14_timer = 0x3C;
    if (pThis->m30_config->m15 == 0)
    {
        pThis->m38_attackOrderReversed = 1;
    }
    else
    {
        pThis->m38_attackOrderReversed = 0;
    }

    displayFormationName(config->m15, config->m16, config->m17);
    if ((gBattleManager->m4 == 7) &&
        (((gBattleManager->m6_subBattleId == 4 || (gBattleManager->m6_subBattleId == 8)) ||
            (gBattleManager->m6_subBattleId == 9))))
    {
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m10000 = 1;
    }
}

void BTL_A3_UrchinFormation_Update_Mode0(BTL_A3_UrchinFormation* pThis)
{
    switch (pThis->m2_subState)
    {
    case 0:
        assert(0);
        break;
    default:
        assert(0);
        break;
    }
}

bool BTL_A3_UrchinFormation_Update_Mode1Sub0(BTL_A3_UrchinFormation* pThis)
{
    if (pThis->m18.mD[3] == 0)
    {
        pThis->m18.mD[4] = 1;
    }

    if ((pThis->m7_attackMode == 2) && battleEngine_UpdateSub7Sub0Sub0())
    {
        assert(0);
    }

    return false;

}

extern std::array<std::array<s8, 4>, 4> enemyQuadrantsTable; //TODO cleanup

bool BTL_A3_UrchinFormation_Update_Mode1Sub1(BTL_A3_UrchinFormation* pThis)
{
    if (battleEngine_UpdateSub7Sub0Sub0())
    {
        for (int i = 0; i < pThis->m3_formationSize; i++)
        {
            int uVar6 = enemyQuadrantsTable[pThis->m18.mD[0] + pThis->m18.m14[i].m18 & 3][gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant];

            if (((pThis->m18.m14[i].m18 & 8) == 0) && (pThis->m18.m14[i].m18 & 0x20) && (pThis->m47_beamHitHandled[i] == 0))
            {
                assert(0);
            }
        }
    }

    return (pThis->m18.m0 == nullptr) ^ 1; // TODO: rewrite that
}

static const std::array< const std::array<s8, 3>, 2> enemtyTable1 = {
    {
        {2,1,0},
        {0,1,2},
    }
};

bool BTL_A3_UrchinFormation_Update_Mode1Sub2(BTL_A3_UrchinFormation* pThis)
{
    if (pThis->m30_config->m14)
    {
        pThis->m38_attackOrderReversed = (pThis->m38_attackOrderReversed == 0);
    }

    for (int i = 0; i < 3; i++)
    {
        int cVar1 = pThis->m18.m4[enemtyTable1[pThis->m38_attackOrderReversed][i]];
        if (pThis->m18.m4[cVar1])
        {
            int uVar6 = enemyQuadrantsTable[pThis->m18.mD[0] + cVar1][gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant];
            sGenericFormationPerTypeDataSub1C& iVar5 = pThis->m30_config->m4_perTypeParams[cVar1]->m1C[pThis->m18.m7[cVar1]];
            sAttackCommand* iVar9 = iVar5.m4[uVar6];
            if (iVar9)
            {
                if (iVar5.m14[uVar6])
                {
                    assert(0);
                }
                if (iVar9->m8 != 1)
                {
                    pThis->m18.m0 = iVar9;
                    pThis->m34_attackingTypeIndex = cVar1;
                    return true;
                }
            }
        }
    }

    return false;
}

void BTL_A3_UrchinFormation_Update_Mode1Sub6(BTL_A3_UrchinFormation* pThis)
{
    sSaturnPtr cameraList = pThis->m18.m0->m4_cameraList;
    if (cameraList.isNull())
    {
        if (pThis->m18.m0->m8 == 5)
        {
            battleEngine_PlayAttackCamera(0);
        }
        else
        {
            static const std::array<eBattleModes, 8> battleModeTable = {
                m8_playAttackCamera,
                m8_playAttackCamera,
                m7,
                m8_playAttackCamera,
                m8_playAttackCamera,
                m7,
                m7,
                m7,
            };
            battleEngine_SetBattleMode(battleModeTable[pThis->m18.m0->m8]);
        }
    }
    else
    {
        int iVar2 = 0;
        while (readSaturnU8(cameraList + iVar2) != 0xE)
        {
            iVar2++;
        }
        int cameraId = performModulo2(iVar2, randomNumber());

        if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x1B])
        {
            assert(0);
        }

        battleEngine_PlayAttackCamera(readSaturnU8(cameraList + cameraId));
    }

    if (pThis->m18.m0->mA_attackDisplayName != -1)
    {
        battleEngine_displayAttackName(pThis->m18.m0->mA_attackDisplayName, 0x1E, 0);
    }
}

void BTL_A3_UrchinFormation_Update_Mode1Sub3(BTL_A3_UrchinFormation* pThis)
{
    pThis->m4_formationPhase++;
    if (pThis->m30_config->m10_formationSubData[pThis->m4_formationPhase] == nullptr)
    {
        pThis->m4_formationPhase = 0;
    }
    assert(0);
}

void BTL_A3_UrchinFormation_Update_Mode1Sub4(BTL_A3_UrchinFormation* pThis, sAttackCommand* param2)
{
    switch (param2->m9 & 6)
    {
    case 2:
        pThis->m18.mD[5] &= ~6;
        pThis->m18.mD[5] |= 2;
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2_needToSortEnemiesByDistanceFromDragon = 1;
        pThis->m18.mD[pThis->m34_attackingTypeIndex + 1] &= 3;
        break;
    case 4:
        pThis->m18.mD[5] &= ~6;
        pThis->m18.mD[5] |= 4;
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2_needToSortEnemiesByDistanceFromDragon = 1;
        pThis->m18.mD[pThis->m34_attackingTypeIndex + 3] &= 3;
        break;
    case 6:
        pThis->m18.mD[5] &= ~6;
        pThis->m18.mD[5] |= 6;
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2_needToSortEnemiesByDistanceFromDragon = 1;
        pThis->m18.mD[pThis->m34_attackingTypeIndex + 2] &= 3;
        break;
    default:
        assert(0);
    }

    if (gBattleManager->m4 == 5)
    {
        assert(0);
    }
    else if (gBattleManager->m4 == 7)
    {
        assert(0);
    }
}

void BTL_A3_UrchinFormation_Update_Mode1(BTL_A3_UrchinFormation* pThis)
{
    pThis->m18.mD[5] = 0;
    pThis->m18.m0 = nullptr;

    if (!BTL_A3_UrchinFormation_Update_Mode1Sub0(pThis) && !BTL_A3_UrchinFormation_Update_Mode1Sub1(pThis))
    {
        if (gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m8 == 0)
            return;

        if (BattleEngineSub0_UpdateSub0())
            return;
    }

    gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m8 = 0;
    gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m0 = 0;

    if ((pThis->m7_attackMode != 1) && (pThis->m6_attackFlag == 0) && !BTL_A3_UrchinFormation_Update_Mode1Sub2(pThis))
    {
        return;
    }

    if (pThis->m18.m0 == nullptr)
    {
        return;
    }

    if (pThis->m18.m0->m9 & 0x80)
    {
        assert(0);
    }

    if (pThis->m18.m0->m8 == 0)
    {
        if ((pThis->m18.m0->m9 & 0x80) == 0)
        {
            if (pThis->m18.m0->m9 & 0x1)
            {
                BTL_A3_UrchinFormation_Update_Mode1Sub3(pThis);
            }
            if (pThis->m18.m0->m9 & 0x6)
            {
                BTL_A3_UrchinFormation_Update_Mode1Sub4(pThis, pThis->m18.m0);
            }
            if (pThis->m18.m0->m9 & 0x38)
            {
                assert(0);
                //BTL_A3_UrchinFormation_Update_Mode1Sub5(pThis->m18.m0);
            }
        }
        pThis->m6_attackFlag = 0;
        return;
    }

    BTL_A3_UrchinFormation_Update_Mode1Sub6(pThis);

    pThis->m1_state = 2;
    pThis->m2_subState = 1;
}

void BTL_A3_UrchinFormation_Update_Mode2Sub0(BTL_A3_UrchinFormation* pThis, sBTL_A3_UrchinFormation_18_14*, sBTL_A3_UrchinFormation_18_14*)
{
    Unimplemented();
}

void BTL_A3_UrchinFormation_Update_Mode2(BTL_A3_UrchinFormation* pThis)
{
    switch (pThis->m2_subState)
    {
    case 0x1E:
        for (int i=0; i<pThis->m3_formationSize; i++)
        {
            pThis->m18.m14[i].m19 &= ~1;
            pThis->m18.m14[i].m19 &= ~2;
        }
        pThis->m6_attackFlag = 0;
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished = 1;
        if (pThis->m18.m0->m9 & 0x80)
        {
            return;
        }
        if ((pThis->m18.m0->m9 & 0x38) == 0)
        {
            if (pThis->m18.m0->m9 & 0x1)
            {
                BTL_A3_UrchinFormation_Update_Mode1Sub3(pThis);
            }
            if (pThis->m18.m0->m9 & 0x6)
            {
                BTL_A3_UrchinFormation_Update_Mode1Sub4(pThis, pThis->m18.m0);
            }
            pThis->m2_subState = 0x20;
            return;
        }
        pThis->m2_subState++;
        break;
    case 0x20:
        pThis->m1_state = 1;
        pThis->m2_subState = 0;
        return;
    case 0:
    case 0x1F:
        assert(0);
    default:
        break;
    }

    s8 cVar1 = pThis->m18.m0->m8;
    switch (cVar1)
    {
    case 0:
        pThis->m2_subState = 0x1E;
        return;
    case 1:
        if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2000)
        {
            assert(0);
        }
        break;
    case 3:
        for (int i=0; i<pThis->m3_formationSize; i++)
        {
            if (pThis->m18.m14[i].m18 & 0x10)
            {
                return;
            }
        }
        {
            int iVar7 = 200;
            int ivar9 = pThis->m3_formationSize;
            for (int i = 0; i < pThis->m3_formationSize; i++)
            {
                if (((pThis->m18.m14[i].m18 & 0x8) == 0) && (pThis->m18.m14[i].m1A < iVar7))
                {
                    iVar7 = pThis->m18.m14[i].m1A;
                    ivar9 = i;
                }
            }
            pThis->m3A_weakestEnemyIndex = ivar9;
            BTL_A3_UrchinFormation_Update_Mode2Sub0(pThis, &pThis->m18.m14[pThis->m39_sourceEnemyIndex], &pThis->m18.m14[pThis->m3A_weakestEnemyIndex]);
            Unimplemented(); //something I don't get yet with the sound index
            pThis->m3C_attackTimer = 0x3C;
            pThis->m2_subState++;
            break;
        }
    default:
        assert(0);
    }
}

bool BTL_A3_UrchinFormation_UpdateSub0(BTL_A3_UrchinFormation* pThis)
{
    if (pThis->m1_state != 3)
    {
        bool bVar1 = false;
        std::array<int, 3> local_2c;

        for (int i = 0; i < 3; i++)
        {
            if (pThis->m18.m4[i] == 0)
            {
                local_2c[i] = -1;
            }
            else
            {
                bVar1 = true;
                local_2c[i] = pThis->m30_config->m20[i];
            }
        }

        for (int i = 0; i < 3; i++)
        {
            if (pThis->m44_prevSoundEffects[i] != -1)
            {
                bool bVar4 = false;
                for (int j = 0; j < 3; j++)
                {
                    if (pThis->m44_prevSoundEffects[i] == local_2c[j])
                    {
                        bVar4 = true;
                        break;
                    }
                }

                if (!bVar4)
                {
                    playBattleSoundEffect(pThis->m44_prevSoundEffects[i]);
                }
            }
        }

        if (bVar1)
        {
            return true;
        }

        pThis->m1_state = 3;
        pThis->m2_subState = 0;
    }

    return false;
}

// This is responsible to setup the quadrant danger/safety
void BTL_A3_UrchinFormation_UpdateSub1(BTL_A3_UrchinFormation* pThis)
{
    for (int i=0; i<4; i++)
    {
        int uVar3 = 0;
        for (int j = 0; j < 3; j++)
        {
            if (pThis->m18.m4[j] && pThis->m30_config->m4_perTypeParams[j])
            {
                static const std::array<s8, 4> tempArray =
                {
                    6,4,2,0
                };
                int uVar1 = rotateRightR0ByR1(pThis->m30_config->m4_perTypeParams[j]->m1C[pThis->m18.m7[j]].m1E, tempArray[enemyQuadrantsTable[pThis->m18.mD[j]][i]]);
                uVar3 |= uVar1 & 3;
            }
        }

        switch (uVar3)
        {
        case 0:
            battleEngine_FlagQuadrantForDanger(i);
            break;
        case 1:
        case 3:
            battleEngine_FlagQuadrantForSafety(i);
            break;
        default:
            assert(0);
            break;
        }
    }
}

void BTL_A3_UrchinFormation_Update(BTL_A3_UrchinFormation* pThis)
{
    if (pThis->m40_pendingTask && pThis->m40_pendingTask->getTask()->isFinished())
    {
        pThis->m40_pendingTask = nullptr;
    }

    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x1B])
    {
        assert(0);
    }

    gBattleManager->m10_battleOverlay->m4_battleEngine->m3B2_numBattleFormationRunning++;

    if (pThis->m1_state)
    {
        if (!BTL_A3_UrchinFormation_UpdateSub0(pThis))
        {
            switch (pThis->m2_subState)
            {
            case 0:
                if (pThis->m30_config->m1C)
                {
                    pThis->m2_subState = 0xA;
                }
                else
                {
                    switch (gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode)
                    {
                    case eBattleModes::m7:
                    case eBattleModes::m8_playAttackCamera:
                    case eBattleModes::m9:
                    case eBattleModes::mA:
                    case eBattleModes::mB_enemyMovingDragon:
                        pThis->m2_subState = 2;
                        break;
                    default:
                        pThis->m2_subState++;
                        break;
                    }
                }
                break;
            case 1:
                if (battleEngine_UpdateSub7Sub0Sub0())
                {
                    battleEngine_SetBattleMode(eBattleModes::m7);
                    pThis->m2_subState++;
                }
                break;
            case 2:
                if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2000)
                {
                    if (pThis->m30_config->m1C)
                    {
                        assert(0);
                    }
                    pThis->m2_subState++;
                }
                break;
            case 3:
                if (pThis->m40_pendingTask == nullptr)
                {
                    pThis->m2_subState = 0xA;
                }
                break;
            case '\n':
                if (BattleEngineSub0_UpdateSub0() && 
                    gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode != eBattleModes::m1_useItem &&
                    gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode != eBattleModes::m2 &&
                    gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode != eBattleModes::m3_shootEnemeyWithHomingLaser &&
                    gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode != eBattleModes::m4_useBerserk &&
                    gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode != eBattleModes::m5 &&
                    gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode != eBattleModes::m6_dragonMoving
                    )
                {
                    gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished = 1;
                }
                if ((gBattleManager->m4 == 9) && (gBattleManager->m10_battleOverlay->m4_battleEngine->m3B0_subBattleId == 4))
                {
                    gBattleManager->m10_battleOverlay->m4_battleEngine->m3B2_numBattleFormationRunning = 0;
                }
                else
                {
                    pThis->m14_timer--;
                    if (pThis->m14_timer < 0)
                    {
                        pThis->getTask()->markFinished();
                    }
                }
                break;
            default:
                assert(0);
                break;
            }

            return;
        }

        if (!BattleEngineSub0_UpdateSub0())
        {
            battleEngine_FlagQuadrantBitForSafety(0);
            battleEngine_FlagQuadrantBitForDanger(0);
            BTL_A3_UrchinFormation_UpdateSub1(pThis);
        }
        else
        {
            if ((gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode != eBattleModes::m7) &&
                (gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode != eBattleModes::m8_playAttackCamera) &&
                (gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode != eBattleModes::m9) &&
                (gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode != eBattleModes::mA) &&
                (gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode != eBattleModes::mB_enemyMovingDragon)
                )
            {
                battleEngine_FlagQuadrantBitForSafety(0);
                battleEngine_FlagQuadrantBitForDanger(0);
                BTL_A3_UrchinFormation_UpdateSub1(pThis);
            }
        }
    }

    switch (pThis->m1_state)
    {
    case 0:
        BTL_A3_UrchinFormation_Update_Mode0(pThis);
        break;
    case 1:
        BTL_A3_UrchinFormation_Update_Mode1(pThis);
        break;
    case 2:
        BTL_A3_UrchinFormation_Update_Mode2(pThis);
        break;
    default:
        assert(0);
        break;
    }

    if (pThis->m1_state)
    {
        for (int i = 0; i < 3; i++)
        {
            pThis->m18.m4[i] = 0;
        }
        pThis->m18.mD[3] = 0;
    }
}

void BTL_A3_UrchinFormation_Delete(BTL_A3_UrchinFormation* pThis)
{
    //empty
}

p_workArea Create_BTL_A3_UrchinFormation(p_workArea parent, const sGenericFormationData* config)
{
    static const BTL_A3_UrchinFormation::TypedTaskDefinition definition = {
        BTL_A3_UrchinFormation_Init,
        BTL_A3_UrchinFormation_Update,
        nullptr,
        BTL_A3_UrchinFormation_Delete,
    };

    return createSubTaskWithArg<BTL_A3_UrchinFormation>(parent, config, &definition);
}

p_workArea Create_BTL_A3_UrchinFormationConfig(p_workArea parent, u32 arg)
{
    switch (arg)
    {
    case 3:
        return Create_BTL_A3_UrchinFormation(parent, g_BTL_A3->m_60A8AE8_urchinFormation); // KURAGE(Urchin) Fx2
    case 4:
    case 5:
        return Create_BTL_A3_UrchinFormation(parent, g_BTL_A3->m_60a8ac4_urchinFormation); // KURAGE(Urchin) M&F
    case 6:
        return Create_BTL_A3_UrchinFormation(parent, g_BTL_A3->m_60a7d34_urchinFormation); // BENITATENA (Pattergo) Zako
    case 7:
        return Create_BTL_A3_UrchinFormation(parent, g_BTL_A3->m_60a7d58_urchinFormation); // BENITATENA (Pattergo) Boss
    default:
        assert(0);
        return nullptr;
    }
}
