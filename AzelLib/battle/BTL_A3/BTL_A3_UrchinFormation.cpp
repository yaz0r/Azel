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
#include "kernel/vdp1Allocator.h"
#include "audio/soundDriver.h" // TODO cleanup
#include "audio/systemSounds.h"

s32 createBattleIntroTaskSub1(); //TODO cleanup
extern std::array<std::array<s8, 4>, 4> enemyQuadrantsTable; //TODO cleanup
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
    std::array<std::array<s8, 4>, 3> m8_quadrantAttackCounters; // 0x08: per-type, per-quadrant attack counters
    s16 m14_timer;
    sSharedFormationState m18;
    const sGenericFormationData* m30_config;
    s8 m34_attackingTypeIndex;
    s16 m36_meleeAttackTimer;
    s8 m38_attackOrderReversed;
    s8 m39_sourceEnemyIndex;
    s8 m3A_weakestEnemyIndex;
    s16 m3C_rangedAttackTimer;
    p_workArea m40_pendingTask;
    std::array<s8, 3> m44_prevSoundEffects;
    std::array<s8, 3> m47_beamHitHandled; // size unknown, should be formation size
    // size 0x4C
};

void formationCopyParams(sSharedFormationState* pDest, const std::vector<sVec3_FP>& pSource, int count)
{
    for (int i = 0; i < 3; i++)
    {
        pDest->m4_typeAlive[i] = 1;
        pDest->m7_attackAnimIndex[i] = 0;
        pDest->mA[i] = 0;
    }
    pDest->mD[3] = 1;

    for (int i = 0; i < count; i++)
    {
        pDest->m14[i].m0_worldPosition.zeroize();
        pDest->m14[i].mC_initialPosition = pSource[i];
        pDest->m14[i].m19_attackFlags = 0;
        pDest->m14[i].m18_statusFlags = 4;
        pDest->m14[i].m1C_flags = 0;
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
            pThis->m18.mD[i] = config->m18_initialDirections[i];
            for (int j = 0; j < config->m1_perTypeCount[i]; j++)
            {
                createUrchin(pEnemyEntry, pThis->m18, numModelIndex, i);
                numModelIndex++;
            }

            if (config->m20_deathSoundEffect[i] != -1)
            {
                // Check if this sound was already loaded by a previous type
                bool alreadyLoaded = false;
                for (int k = 0; k < i; k++)
                {
                    if (pThis->m44_prevSoundEffects[k] == config->m20_deathSoundEffect[i])
                    {
                        alreadyLoaded = true;
                        break;
                    }
                }
                if (!alreadyLoaded)
                {
                    playPCM(pThis, (s16)config->m20_deathSoundEffect[i]);
                    pThis->m44_prevSoundEffects[i] = config->m20_deathSoundEffect[i];
                }
            }

            if (pEnemyEntry->m24_beamHitAttackData)
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
    if (pThis->m30_config->m15_formationNameIndex == 0)
    {
        pThis->m38_attackOrderReversed = 1;
    }
    else
    {
        pThis->m38_attackOrderReversed = 0;
    }

    displayFormationName(config->m15_formationNameIndex, config->m16, config->m17);
    if ((gBattleManager->m4 == 7) &&
        (((gBattleManager->m6_subBattleId == 4 || (gBattleManager->m6_subBattleId == 8)) ||
            (gBattleManager->m6_subBattleId == 9))))
    {
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m10000 = 1;
    }
}

void BTL_A3_UrchinFormation_Update_Mode0(BTL_A3_UrchinFormation* pThis)
{
    const sGenericFormationData* config = pThis->m30_config;
    switch (pThis->m2_subState)
    {
    case 0:
    {
        // Load resources for each enemy type
        for (int i = 0; i < 3; i++)
        {
            pThis->m44_prevSoundEffects[i] = -1;
            sGenericFormationPerTypeData* pEnemyEntry = config->m4_perTypeParams[i];
            if (pEnemyEntry)
            {
                allocateNPC(pThis, pEnemyEntry->m1_fileBundleIndex);
                pThis->m18.mD[i] = config->m18_initialDirections[i];

                if (config->m20_deathSoundEffect[i] != -1)
                {
                    bool alreadyLoaded = false;
                    for (int k = 0; k < i; k++)
                    {
                        if (pThis->m44_prevSoundEffects[k] == config->m20_deathSoundEffect[i])
                        {
                            alreadyLoaded = true;
                            break;
                        }
                    }
                    if (!alreadyLoaded)
                    {
                        playPCM(pThis, (s16)config->m20_deathSoundEffect[i]);
                        pThis->m44_prevSoundEffects[i] = config->m20_deathSoundEffect[i];
                    }
                }

                if (pEnemyEntry->m24_beamHitAttackData)
                {
                    // bVar3 flag — handled after loop (same as Init)
                }

                if (pEnemyEntry->m2)
                {
                    // bVar2 flag — handled after loop
                }
            }
        }
        pThis->m14_timer = 0x3C;
        if (config->m15_formationNameIndex == 0)
            pThis->m38_attackOrderReversed = 1;
        else
            pThis->m38_attackOrderReversed = 0;
        pThis->m2_subState = 1;
        // Store a countdown timer at m0 (reusing first byte of struct area)
        // Ghidra: *param_1 = 'Z' (0x5A = 90 frames)
        break;
    }
    case 1:
    {
        // Wait for file loading to complete, then create urchin entities
        if (fileInfoStruct.m2C_allocatedHead != 0)
            return;

        int numModelIndex = 0;
        for (int i = 0; i < 3; i++)
        {
            sGenericFormationPerTypeData* pEnemyEntry = config->m4_perTypeParams[i];
            if (pEnemyEntry)
            {
                for (int j = 0; j < config->m1_perTypeCount[i]; j++)
                {
                    createUrchin(pEnemyEntry, pThis->m18, numModelIndex, i);
                    numModelIndex++;
                }
            }
        }
        displayFormationName(config->m15_formationNameIndex, config->m16, config->m17);
        pThis->m1_state = 1;
        pThis->m2_subState = 0;
        break;
    }
    default:
        break;
    }
}

// 06078108
bool BTL_A3_UrchinFormation_Update_Mode1Sub0(BTL_A3_UrchinFormation* pThis)
{
    if (pThis->m18.mD[3] == 0)
    {
        pThis->m18.mD[4] = 1;
    }

    if ((pThis->m7_attackMode == 2) && battleEngine_isBattleIntroFinished() && (pThis->m18.mD[3] == 0))
    {
        for (int i = 0; i < pThis->m3_formationSize; i++)
        {
            s8 typeIndex = pThis->m18.m14[i].m18_statusFlags & 3;
            s32 quadrant = enemyQuadrantsTable[pThis->m18.mD[typeIndex]][gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant];

            if ((pThis->m18.m14[i].m18_statusFlags & 8) == 0)
            {
                sGenericFormationPerTypeDataSub1C* pBeamData = pThis->m30_config->m4_perTypeParams[typeIndex]->m24_beamHitAttackData;
                if (pBeamData)
                {
                    if (pBeamData->m14_quadrantFlags[quadrant] != 0)
                    {
                        pThis->m47_beamHitHandled[i] = 1;
                        pThis->m18.mA[typeIndex] = pThis->m18.m7_attackAnimIndex[typeIndex];
                        s8 nextAnim = pBeamData->m18_nextAnimIndex[quadrant];
                        if (nextAnim == -1)
                        {
                            nextAnim = pThis->m18.mA[typeIndex];
                        }
                        pThis->m18.m7_attackAnimIndex[typeIndex] = nextAnim;
                        gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m2 = pBeamData->m20_timerValue;
                        for (int q = 0; q < 4; q += 2)
                        {
                            pThis->m8_quadrantAttackCounters[typeIndex][q] = 0;
                            pThis->m8_quadrantAttackCounters[typeIndex][q + 1] = 0;
                        }
                    }
                    if ((pThis->m18.m0_currentAttack == nullptr) && (pBeamData->m4[quadrant] != nullptr) &&
                        (pBeamData->m4[quadrant]->m8_type != 1))
                    {
                        pThis->m18.m0_currentAttack = pBeamData->m4[quadrant];
                        pThis->m34_attackingTypeIndex = typeIndex;
                        pThis->m7_attackMode = 1;
                    }
                }
            }
        }
        if (pThis->m18.m0_currentAttack != nullptr)
        {
            return true;
        }
        pThis->m7_attackMode = 0;
    }

    return false;
}

// 06078252
bool BTL_A3_UrchinFormation_Update_Mode1Sub1(BTL_A3_UrchinFormation* pThis)
{
    if (battleEngine_isBattleIntroFinished())
    {
        for (int i = 0; i < pThis->m3_formationSize; i++)
        {
            s8 typeIndex = pThis->m18.m14[i].m18_statusFlags & 3;
            s32 quadrant = enemyQuadrantsTable[pThis->m18.mD[typeIndex]][gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant];

            if (((pThis->m18.m14[i].m18_statusFlags & 8) == 0) &&
                (pThis->m18.m14[i].m18_statusFlags & 0x20) &&
                (pThis->m47_beamHitHandled[i] == 0))
            {
                sGenericFormationPerTypeDataSub1C* pBeamData = pThis->m30_config->m4_perTypeParams[typeIndex]->m20_beamHitCounterData;
                if (pBeamData)
                {
                    gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m2 = pBeamData->m20_timerValue;
                    if (pBeamData->m14_quadrantFlags[quadrant] != 0)
                    {
                        pThis->m18.mA[typeIndex] = pThis->m18.m7_attackAnimIndex[typeIndex];
                        s8 nextAnim = pBeamData->m18_nextAnimIndex[quadrant];
                        if (nextAnim == -1)
                        {
                            nextAnim = pThis->m18.mA[typeIndex];
                        }
                        pThis->m18.m7_attackAnimIndex[typeIndex] = nextAnim;
                        for (int q = 0; q < 4; q += 2)
                        {
                            pThis->m8_quadrantAttackCounters[typeIndex][q] = 0;
                            pThis->m8_quadrantAttackCounters[typeIndex][q + 1] = 0;
                        }
                    }
                    if ((pThis->m18.m0_currentAttack == nullptr) && (pBeamData->m4[quadrant] != nullptr) &&
                        (pBeamData->m4[quadrant]->m8_type != 1))
                    {
                        pThis->m18.m0_currentAttack = pBeamData->m4[quadrant];
                        pThis->m34_attackingTypeIndex = typeIndex;
                        pThis->m6_attackFlag = 1;
                    }
                }
            }
        }
    }

    return pThis->m18.m0_currentAttack != nullptr;
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
        s8 typeIndex = enemtyTable1[pThis->m38_attackOrderReversed][i];
        if (pThis->m18.m4_typeAlive[typeIndex])
        {
            s32 quadrant = enemyQuadrantsTable[pThis->m18.mD[typeIndex]][gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant];
            sGenericFormationPerTypeDataSub1C& animData = pThis->m30_config->m4_perTypeParams[typeIndex]->m1C[pThis->m18.m7_attackAnimIndex[typeIndex]];
            sAttackCommand* pAttack = animData.m4[quadrant];
            if (pAttack)
            {
                if (animData.m14_quadrantFlags[quadrant])
                {
                    // Quadrant attack counter threshold reached → advance to next attack anim
                    s8& counter = pThis->m8_quadrantAttackCounters[typeIndex][quadrant];
                    counter++;
                    if (counter >= animData.m14_quadrantFlags[quadrant])
                    {
                        pThis->m18.mA[typeIndex] = pThis->m18.m7_attackAnimIndex[typeIndex];
                        s8 nextAnim = animData.m18_nextAnimIndex[quadrant];
                        if (nextAnim == -1)
                        {
                            nextAnim = pThis->m18.mA[typeIndex];
                        }
                        pThis->m18.m7_attackAnimIndex[typeIndex] = nextAnim;
                        // Update battle timer
                        gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m2 =
                            pThis->m30_config->m4_perTypeParams[typeIndex]->m1C[nextAnim].m20_timerValue;
                        // Clear all quadrant counters for this type
                        for (int q = 0; q < 4; q++)
                        {
                            pThis->m8_quadrantAttackCounters[typeIndex][q] = 0;
                        }
                    }
                }
                if (pAttack->m8_type != 1)
                {
                    pThis->m18.m0_currentAttack = pAttack;
                    pThis->m34_attackingTypeIndex = typeIndex;
                    return true;
                }
            }
        }
    }

    return false;
}

// 0605ab04
static void battleEngine_initiateEnemyMoveDragon(s8 direction, s16 agility)
{
    s_battleEngine* pBE = gBattleManager->m10_battleOverlay->m4_battleEngine;
    pBE->m22E_dragonMoveDirection = direction;
    battleEngine_SetBattleMode(eBattleModes::mB_enemyMovingDragon);
    pBE->m38D_battleSubMode = 2;
    pBE->m27C_dragonMovementInterpolator1.m68_rate = agility;
    pBE->m2E8_dragonMovementInterpolator2.m68_rate = agility;
    gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m400000 = 1;
    pBE->m38E = 0;
}

// 06078440
void BTL_A3_UrchinFormation_Update_Mode1Sub5(sAttackCommand* pAttack)
{
    u8 flagBits = pAttack->m9_flags & 0x38;
    s8 mode;
    switch (flagBits)
    {
    case 0x08: mode = 2; break;
    case 0x10: mode = 1; break;
    case 0x18: mode = 3; break;
    case 0x20: mode = 5; break;
    case 0x28: mode = 4; break;
    default: return;
    }
    battleEngine_initiateEnemyMoveDragon(mode, 0x3C);
}

void BTL_A3_UrchinFormation_Update_Mode1Sub6(BTL_A3_UrchinFormation* pThis)
{
    sSaturnPtr cameraList = pThis->m18.m0_currentAttack->m4_cameraList;
    if (cameraList.isNull())
    {
        if (pThis->m18.m0_currentAttack->m8_type == 5)
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
            battleEngine_SetBattleMode(battleModeTable[pThis->m18.m0_currentAttack->m8_type]);
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
            cameraId = gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x1C];
        }

        battleEngine_PlayAttackCamera(readSaturnU8(cameraList + cameraId));
    }

    if (pThis->m18.m0_currentAttack->mA_attackDisplayName != -1)
    {
        battleEngine_displayAttackName(pThis->m18.m0_currentAttack->mA_attackDisplayName, 0x1E, 0);
    }
}

void BTL_A3_UrchinFormation_Update_Mode1Sub3(BTL_A3_UrchinFormation* pThis)
{
    pThis->m4_formationPhase++;
    if (pThis->m30_config->m10_formationSubData[pThis->m4_formationPhase] == nullptr)
    {
        pThis->m4_formationPhase = 0;
    }

    // Update each enemy's initial position from the new phase data
    sGenericFormationSubData* pPhaseData = pThis->m30_config->m10_formationSubData[pThis->m4_formationPhase];
    for (int i = 0; i < pThis->m3_formationSize; i++)
    {
        pThis->m18.m14[i].mC_initialPosition = pPhaseData->m0_perEnemyPosition[i];
    }

    pThis->m18.mD[5] |= 1;
}

void BTL_A3_UrchinFormation_Update_Mode1Sub4(BTL_A3_UrchinFormation* pThis, sAttackCommand* param2)
{
    u8 rotFlags = param2->m9_flags & 6;
    u8 rotAmount;
    switch (rotFlags)
    {
    case 2:
        pThis->m18.mD[5] &= ~6;
        pThis->m18.mD[5] |= 2;
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2_needToSortEnemiesByDistanceFromDragon = 1;
        rotAmount = pThis->m18.mD[pThis->m34_attackingTypeIndex] + 1;
        pThis->m18.mD[pThis->m34_attackingTypeIndex] = rotAmount & 3;
        break;
    case 4:
        pThis->m18.mD[5] &= ~6;
        pThis->m18.mD[5] |= 4;
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2_needToSortEnemiesByDistanceFromDragon = 1;
        rotAmount = pThis->m18.mD[pThis->m34_attackingTypeIndex] + 3;
        pThis->m18.mD[pThis->m34_attackingTypeIndex] = rotAmount & 3;
        break;
    case 6:
        pThis->m18.mD[5] &= ~6;
        pThis->m18.mD[5] |= 6;
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2_needToSortEnemiesByDistanceFromDragon = 1;
        rotAmount = pThis->m18.mD[pThis->m34_attackingTypeIndex] + 2;
        pThis->m18.mD[pThis->m34_attackingTypeIndex] = rotAmount & 3;
        break;
    default:
        break;
    }

    // Play formation-specific sound effect
    s16 battleType = gBattleManager->m4;
    s8 subBattleId = gBattleManager->m10_battleOverlay->m4_battleEngine->m3B0_subBattleId;
    if (battleType == 5)
    {
        if (subBattleId == 4 || subBattleId == 5 || subBattleId == 8 || subBattleId == 9 || subBattleId == 10)
        {
            playSystemSoundEffect(0x6F);
        }
    }
    else if (battleType == 7)
    {
        if (subBattleId == 1)
        {
            playSystemSoundEffect(0x70);
        }
    }
}

void BTL_A3_UrchinFormation_Update_Mode1(BTL_A3_UrchinFormation* pThis)
{
    pThis->m18.mD[5] = 0;
    pThis->m18.m0_currentAttack = nullptr;

    if (!BTL_A3_UrchinFormation_Update_Mode1Sub0(pThis) && !BTL_A3_UrchinFormation_Update_Mode1Sub1(pThis))
    {
        if (gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m8 == 0)
            return;

        if (battleEngine_isPlayerTurnActive())
            return;
    }

    gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m8 = 0;
    gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m0 = 0;

    if ((pThis->m7_attackMode != 1) && (pThis->m6_attackFlag == 0) && !BTL_A3_UrchinFormation_Update_Mode1Sub2(pThis))
    {
        return;
    }

    if (pThis->m18.m0_currentAttack == nullptr)
    {
        return;
    }

    // Handle flags & 0x80 path (pre-attack effects)
    if (pThis->m18.m0_currentAttack->m9_flags & 0x80)
    {
        if (pThis->m18.m0_currentAttack->m9_flags & 0x1)
        {
            BTL_A3_UrchinFormation_Update_Mode1Sub3(pThis);
        }
        if (pThis->m18.m0_currentAttack->m9_flags & 0x6)
        {
            BTL_A3_UrchinFormation_Update_Mode1Sub4(pThis, pThis->m18.m0_currentAttack);
        }
        if (pThis->m18.m0_currentAttack->m9_flags & 0x38)
        {
            BTL_A3_UrchinFormation_Update_Mode1Sub5(pThis->m18.m0_currentAttack);
            pThis->m1_state = 2;
            pThis->m2_subState = 0;
            return;
        }
    }

    if (pThis->m18.m0_currentAttack->m8_type == 0)
    {
        if ((pThis->m18.m0_currentAttack->m9_flags & 0x80) == 0)
        {
            if (pThis->m18.m0_currentAttack->m9_flags & 0x1)
            {
                BTL_A3_UrchinFormation_Update_Mode1Sub3(pThis);
            }
            if (pThis->m18.m0_currentAttack->m9_flags & 0x6)
            {
                BTL_A3_UrchinFormation_Update_Mode1Sub4(pThis, pThis->m18.m0_currentAttack);
            }
            if (pThis->m18.m0_currentAttack->m9_flags & 0x38)
            {
                BTL_A3_UrchinFormation_Update_Mode1Sub5(pThis->m18.m0_currentAttack);
            }
        }
        pThis->m6_attackFlag = 0;
        return;
    }

    BTL_A3_UrchinFormation_Update_Mode1Sub6(pThis);

    pThis->m1_state = 2;
    pThis->m2_subState = 1;
}

// 0607897a — melee/physical attack execution (types 2,3,6,7)
static void urchinFormation_executeAttack(BTL_A3_UrchinFormation* pThis)
{
    sAttackCommand* pAttack = pThis->m18.m0_currentAttack;
    switch (pThis->m2_subState)
    {
    case 1:
    {
        if (!(gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2000))
            return;
        int attackCount = 0;
        for (int i = 0; i < pThis->m3_formationSize; i++)
        {
            u8 statusFlags = pThis->m18.m14[i].m18_statusFlags;
            if (((statusFlags & 3) == pThis->m34_attackingTypeIndex) && ((statusFlags & 8) == 0))
            {
                if (pThis->m6_attackFlag == 0)
                {
                    attackCount++;
                    pThis->m18.m14[i].m19_attackFlags = 1;
                }
                else if (pThis->m18.m14[i].m18_statusFlags & 0x20)
                {
                    pThis->m18.m14[i].m19_attackFlags = 1;
                    attackCount++;
                }
            }
            if (attackCount >= readSaturnU8(pAttack->m0_attackParams + 1))
                break;
        }
        if (attackCount == 0)
        {
            pThis->m2_subState = 0x1E;
        }
        else
        {
            pThis->m2_subState++;
            pThis->m36_meleeAttackTimer = 0;
        }
        return;
    }
    case 2:
    {
        pThis->m36_meleeAttackTimer--;
        if (pThis->m36_meleeAttackTimer >= 0)
            return;

        // Count currently active attacking enemies (with m10 flag)
        int activeCount = 0;
        for (int i = 0; i < pThis->m3_formationSize; i++)
        {
            if (pThis->m18.m14[i].m18_statusFlags & 0x10)
            {
                activeCount++;
            }
        }

        s8 maxActive = readSaturnU8(pAttack->m0_attackParams + 3);
        if (activeCount < maxActive)
        {
            for (int i = 0; i < pThis->m3_formationSize; i++)
            {
                if (pThis->m18.m14[i].m19_attackFlags & 1)
                {
                    pThis->m18.m14[i].m19_attackFlags = 2;
                    pThis->m18.m14[i].m19_attackFlags &= ~1;
                    s16 delay = readSaturnS16(pAttack->m0_attackParams + 0);
                    if (delay != 0)
                    {
                        pThis->m36_meleeAttackTimer = delay;
                        break;
                    }
                    activeCount++;
                    if (activeCount >= maxActive)
                        break;
                }
            }

            // Check if any still waiting to attack
            for (int i = 0; i < pThis->m3_formationSize; i++)
            {
                if (pThis->m18.m14[i].m19_attackFlags & 1)
                {
                    return;
                }
            }

            // All dispatched — clear beam hit flags if needed
            if (pThis->m6_attackFlag != 0)
            {
                for (int i = 0; i < pThis->m3_formationSize; i++)
                {
                    pThis->m18.m14[i].m18_statusFlags &= ~0x20;
                }
            }
            pThis->m2_subState++;
        }
        return;
    }
    case 3:
    {
        for (int i = 0; i < pThis->m3_formationSize; i++)
        {
            if (pThis->m18.m14[i].m18_statusFlags & 0x10)
            {
                return;
            }
        }
        pThis->m6_attackFlag = 0;
        pThis->m2_subState = 0x1E;
        return;
    }
    default:
        return;
    }
}

// Projectile particle pair: position + velocity for two particles
struct sProjectileParticlePair
{
    sVec3_FP m0_posA;
    sVec3_FP mC_velA;
    sVec3_FP m18_posB;
    sVec3_FP m24_velB;
    // size 0x30
};

struct sProjectileTask : public s_workAreaTemplate<sProjectileTask>
{
    sPerEnemySlot* m0_target;
    std::array<sProjectileParticlePair, 16> m4_particles;
    sAnimatedQuad m304_quad;
    s16 m30C_timer;
    // size 0x310
};

static std::vector<sVdp1Quad> s_projectileSpriteData;

// 0607933c
static void sProjectileTask_Update(sProjectileTask* pThis)
{
    sVec3_FP& autoScroll = gBattleManager->m10_battleOverlay->m4_battleEngine->m1A0_battleAutoScrollDelta;
    sVec3_FP* targetPos = &pThis->m0_target->m0_worldPosition;

    for (int i = 0; i < 16; i++)
    {
        sProjectileParticlePair& pair = pThis->m4_particles[i];

        // Update particle A
        pair.m0_posA = pair.m0_posA + autoScroll;
        pair.mC_velA = pair.mC_velA - MTH_Mul(fixedPoint(0x7ae), pair.mC_velA);
        sVec3_FP homingA;
        homingA.m0_X = MTH_Mul(fixedPoint(0x20c), targetPos->m0_X - pair.m0_posA.m0_X);
        homingA.m4_Y = MTH_Mul(fixedPoint(0x20c), targetPos->m4_Y - pair.m0_posA.m4_Y);
        homingA.m8_Z = MTH_Mul(fixedPoint(0x20c), targetPos->m8_Z - pair.m0_posA.m8_Z);
        pair.mC_velA = pair.mC_velA + homingA;
        pair.m0_posA = pair.m0_posA + pair.mC_velA;

        // Update particle B
        pair.m18_posB = pair.m18_posB + autoScroll;
        pair.m24_velB = pair.m24_velB - MTH_Mul(fixedPoint(0x7ae), pair.m24_velB);
        sVec3_FP homingB;
        homingB.m0_X = MTH_Mul(fixedPoint(0x20c), targetPos->m0_X - pair.m18_posB.m0_X);
        homingB.m4_Y = MTH_Mul(fixedPoint(0x20c), targetPos->m4_Y - pair.m18_posB.m4_Y);
        homingB.m8_Z = MTH_Mul(fixedPoint(0x20c), targetPos->m8_Z - pair.m18_posB.m8_Z);
        pair.m24_velB = pair.m24_velB + homingB;
        pair.m18_posB = pair.m18_posB + pair.m24_velB;
    }

    pThis->m30C_timer--;
    if (pThis->m30C_timer < 0)
    {
        pThis->getTask()->markFinished();
    }
}

// 0607963e
static void sProjectileTask_Draw(sProjectileTask* pThis)
{
    for (int i = 0; i < 16; i++)
    {
        sProjectileParticlePair& pair = pThis->m4_particles[i];
        sGunShotTask_UpdateSub4(&pThis->m304_quad);
        drawProjectedParticle(&pThis->m304_quad, &pair.m0_posA);
        sGunShotTask_UpdateSub4(&pThis->m304_quad);
        drawProjectedParticle(&pThis->m304_quad, &pair.m18_posB);
    }
}

// 06078e72
static void urchinFormation_createProjectileTask(BTL_A3_UrchinFormation* pThis, sPerEnemySlot* pSource, sPerEnemySlot* pTarget)
{
    static const sProjectileTask::TypedTaskDefinition definition = {
        nullptr,
        sProjectileTask_Update,
        sProjectileTask_Draw,
        nullptr,
    };

    sProjectileTask* pTask = createSubTask<sProjectileTask>(pThis, &definition);
    pTask->m0_target = pTarget;

    // Init sprite data from Saturn ROM if not already done
    if (s_projectileSpriteData.empty())
    {
        s_projectileSpriteData = initVdp1Quad(g_BTL_A3->getSaturnPtr(0x060af4a0));
    }

    u16 vdp1Memory = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory;
    particleInitSub(&pTask->m304_quad, vdp1Memory, &s_projectileSpriteData);

    // Initialize 16 pairs of particles with random velocities, starting at source position
    for (int i = 0; i < 16; i++)
    {
        sProjectileParticlePair& pair = pTask->m4_particles[i];
        pair.m0_posA = pSource->m0_worldPosition;
        pair.mC_velA.m0_X = fixedPoint((s32)(randomNumber() & 0x1FFF) - 0xFFF);
        pair.mC_velA.m4_Y = fixedPoint((s32)(randomNumber() & 0x1FFF) - 0xFFF);
        pair.mC_velA.m8_Z = fixedPoint((s32)(randomNumber() & 0x1FFF) - 0xFFF);

        pair.m18_posB = pSource->m0_worldPosition;
        pair.m24_velB.m0_X = fixedPoint((s32)(randomNumber() & 0x1FFF) - 0xFFF);
        pair.m24_velB.m4_Y = fixedPoint((s32)(randomNumber() & 0x1FFF) - 0xFFF);
        pair.m24_velB.m8_Z = fixedPoint((s32)(randomNumber() & 0x1FFF) - 0xFFF);
    }

    pTask->m30C_timer = 0x3C;
}

struct sFadeSubTask : public s_workAreaTemplate<sFadeSubTask>
{
    u16 m0_color1;
    u16 m2_color2;
    u16 m4_duration;
    u16 m6_color3;
    u16 m8_color4;
    s16 mA_duration2;
    s8 mC_state;
};

// 0605b654
static void sFadeSubTask_Update(sFadeSubTask* pThis)
{
    if (pThis->mC_state == 0)
    {
        g_fadeControls.m_4D = 0;
        if (g_fadeControls.m_4C < 1)
        {
            vdp2Controls.m20_registers[1].m112_CLOFSL = 0;
            vdp2Controls.m20_registers[0].m112_CLOFSL = 0;
        }
        fadePalette(&g_fadeControls.m0_fade0, pThis->m0_color1, pThis->m2_color2, pThis->m4_duration);
        g_fadeControls.m_4D = 5;
        pThis->mC_state++;
    }
    else if (pThis->mC_state == 1)
    {
        if (g_fadeControls.m0_fade0.m20_stopped)
        {
            g_fadeControls.m_4D = 0;
            if (g_fadeControls.m_4C < 1)
            {
                vdp2Controls.m20_registers[1].m112_CLOFSL = 0;
                vdp2Controls.m20_registers[0].m112_CLOFSL = 0;
            }
            fadePalette(&g_fadeControls.m0_fade0, pThis->m6_color3, pThis->m8_color4, pThis->mA_duration2);
            g_fadeControls.m_4D = 5;
            pThis->getTask()->markFinished();
        }
    }
}

// 0605b61c
static void urchinFormation_createFadeTask(p_workArea pParent, u16 color1, u16 color2, u16 duration, u16 color3, u16 color4, s16 duration2)
{
    static const sFadeSubTask::TypedTaskDefinition definition = {
        nullptr,
        sFadeSubTask_Update,
        nullptr,
        nullptr,
    };

    sFadeSubTask* pTask = createSiblingTask<sFadeSubTask>(pParent, &definition);
    pTask->m0_color1 = color1;
    pTask->m2_color2 = color2;
    pTask->m4_duration = duration;
    pTask->m6_color3 = color3;
    pTask->m8_color4 = color4;
    pTask->mA_duration2 = duration2;
    pTask->mC_state = 0;
}

// Impact effect orbital point
struct sImpactOrbitalPoint
{
    sVec3_FP m0_position;
    sVec3_FP mC_velocity;
    sVec3_FP m18_orbitalPosition;
    // size 0x24
};

// Impact effect particle
struct sImpactParticle
{
    sVec3_FP m0_acceleration;
    sVec3_FP mC_velocity;
    sVec3_FP m18_position;
    sAnimatedQuad m24_quad;
    s32 m2C_yVelocity;
    s32 m30_orbitalIndex;
    u16 m34_startFrame;
    u16 m36_state;
    // size 0x38
};

struct sImpactEffectTask : public s_workAreaTemplate<sImpactEffectTask>
{
    sImpactParticle* mC_particles;
    sImpactOrbitalPoint* m10_orbitalPoints;
    sPerEnemySlot* m14_target;
    sVec3_FP* m18_positionSource;
    s32 m1C_radius;
    s32 m20_numOrbitalPoints;
    s32 m24_numParticles;
    u16 m2C_frameCounter;
    s32 m30_param7;
    s32 m34_param8;
    // size 0x38
};

// 0608d5a0
static void impactParticle_init(sImpactParticle* p, int numOrbitalPoints)
{
    p->m0_acceleration.zeroize();
    p->mC_velocity.zeroize();
    p->m18_position.zeroize();
    p->m30_orbitalIndex = MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(numOrbitalPoints - 1));
    p->m34_startFrame = (u16)MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x32)).getInteger();
    p->m2C_yVelocity = -0x28 - MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0xa4));
    p->m36_state = 0;
}

// 0608d60c
static void impactParticle_update(sImpactParticle* p)
{
    if (p->m36_state == 1)
    {
        fixedPoint repelForce(0x28f);
        p->m0_acceleration.m0_X = MTH_Mul(fixedPoint(-p->m18_position.m0_X), repelForce);
        p->m0_acceleration.m4_Y = MTH_Mul(fixedPoint(-p->m18_position.m4_Y), repelForce);
        p->m0_acceleration.m8_Z = MTH_Mul(fixedPoint(-p->m18_position.m8_Z), repelForce);
        p->mC_velocity = p->mC_velocity + p->m0_acceleration;

        fixedPoint friction(0xccc);
        p->mC_velocity = p->mC_velocity - MTH_Mul(friction, p->mC_velocity);
        p->m18_position = p->m18_position + p->mC_velocity;
        p->m0_acceleration.zeroize();
    }
    sGunShotTask_UpdateSub4(&p->m24_quad);
}

// 0608cd6c
static void sImpactEffectTask_Update(sImpactEffectTask* pThis)
{
    pThis->m2C_frameCounter++;
    if (pThis->m2C_frameCounter < 0x46)
    {
        for (int i = 0; i < pThis->m20_numOrbitalPoints; i++)
        {
            sImpactOrbitalPoint& point = pThis->m10_orbitalPoints[i];
            sVec3_FP delta;

            if (i == 0)
            {
                // First point chases last point
                sImpactOrbitalPoint& last = pThis->m10_orbitalPoints[pThis->m20_numOrbitalPoints - 1];
                delta.m0_X = fixedPoint((s32)(last.m18_orbitalPosition.m0_X - point.m18_orbitalPosition.m0_X) & 0xFFFFFFF);
                delta.m4_Y = fixedPoint((s32)(last.m18_orbitalPosition.m4_Y - point.m18_orbitalPosition.m4_Y) & 0xFFFFFFF);
                delta.m8_Z = fixedPoint((s32)(last.m18_orbitalPosition.m8_Z - point.m18_orbitalPosition.m8_Z) & 0xFFFFFFF);
                // Sign extend from 28 bits
                if (delta.m0_X & 0x8000000) delta.m0_X = fixedPoint((s32)delta.m0_X | (s32)0xF0000000);
                if (delta.m4_Y & 0x8000000) delta.m4_Y = fixedPoint((s32)delta.m4_Y | (s32)0xF0000000);
                if (delta.m8_Z & 0x8000000) delta.m8_Z = fixedPoint((s32)delta.m8_Z | (s32)0xF0000000);
            }
            else
            {
                sImpactOrbitalPoint& prev = pThis->m10_orbitalPoints[i - 1];
                delta.m0_X = fixedPoint((s32)(prev.m18_orbitalPosition.m0_X - point.m18_orbitalPosition.m0_X) & 0xFFFFFFF);
                delta.m4_Y = fixedPoint((s32)(prev.m18_orbitalPosition.m4_Y - point.m18_orbitalPosition.m4_Y) & 0xFFFFFFF);
                delta.m8_Z = fixedPoint((s32)(prev.m18_orbitalPosition.m8_Z - point.m18_orbitalPosition.m8_Z) & 0xFFFFFFF);
                if (delta.m0_X & 0x8000000) delta.m0_X = fixedPoint((s32)delta.m0_X | (s32)0xF0000000);
                if (delta.m4_Y & 0x8000000) delta.m4_Y = fixedPoint((s32)delta.m4_Y | (s32)0xF0000000);
                if (delta.m8_Z & 0x8000000) delta.m8_Z = fixedPoint((s32)delta.m8_Z | (s32)0xF0000000);
            }

            fixedPoint chaseForce(0x28f);
            point.m0_position = point.m0_position + MTH_Mul(chaseForce, delta);

            // Also chase next point (or first if last)
            sVec3_FP delta2;
            if (i == pThis->m20_numOrbitalPoints - 1)
            {
                sImpactOrbitalPoint& first = pThis->m10_orbitalPoints[0];
                delta2.m0_X = fixedPoint((s32)(first.m18_orbitalPosition.m0_X - point.m18_orbitalPosition.m0_X) & 0xFFFFFFF);
                delta2.m4_Y = fixedPoint((s32)(first.m18_orbitalPosition.m4_Y - point.m18_orbitalPosition.m4_Y) & 0xFFFFFFF);
                delta2.m8_Z = fixedPoint((s32)(first.m18_orbitalPosition.m8_Z - point.m18_orbitalPosition.m8_Z) & 0xFFFFFFF);
                if (delta2.m0_X & 0x8000000) delta2.m0_X = fixedPoint((s32)delta2.m0_X | (s32)0xF0000000);
                if (delta2.m4_Y & 0x8000000) delta2.m4_Y = fixedPoint((s32)delta2.m4_Y | (s32)0xF0000000);
                if (delta2.m8_Z & 0x8000000) delta2.m8_Z = fixedPoint((s32)delta2.m8_Z | (s32)0xF0000000);
            }
            else
            {
                sImpactOrbitalPoint& next = pThis->m10_orbitalPoints[i + 1];
                delta2.m0_X = fixedPoint((s32)(next.m18_orbitalPosition.m0_X - point.m18_orbitalPosition.m0_X) & 0xFFFFFFF);
                delta2.m4_Y = fixedPoint((s32)(next.m18_orbitalPosition.m4_Y - point.m18_orbitalPosition.m4_Y) & 0xFFFFFFF);
                delta2.m8_Z = fixedPoint((s32)(next.m18_orbitalPosition.m8_Z - point.m18_orbitalPosition.m8_Z) & 0xFFFFFFF);
                if (delta2.m0_X & 0x8000000) delta2.m0_X = fixedPoint((s32)delta2.m0_X | (s32)0xF0000000);
                if (delta2.m4_Y & 0x8000000) delta2.m4_Y = fixedPoint((s32)delta2.m4_Y | (s32)0xF0000000);
                if (delta2.m8_Z & 0x8000000) delta2.m8_Z = fixedPoint((s32)delta2.m8_Z | (s32)0xF0000000);
            }
            point.m0_position = point.m0_position + MTH_Mul(chaseForce, delta2);

            // Apply random jitter occasionally
            if ((randomNumber() & 0x1F) == 0)
            {
                s32 jitterRadius = MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x12345));
                s32 angle1 = MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x10000000));
                s32 jitterY = MTH_Mul(getSin((u16)((u32)angle1 >> 16) & 0xFFF), fixedPoint(jitterRadius));

                s32 angle2 = MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x10000000));
                s32 horiz = MTH_Mul(getCos((u16)((u32)angle2 >> 16) & 0xFFF), fixedPoint(jitterRadius));

                s32 angle3 = MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x10000000));
                s32 jitterZ = MTH_Mul(getSin((u16)((u32)angle3 >> 16) & 0xFFF), fixedPoint(horiz));
                s32 jitterX = MTH_Mul(getCos((u16)((u32)angle3 >> 16) & 0xFFF), fixedPoint(horiz));

                point.m0_position.m0_X = point.m0_position.m0_X + fixedPoint(jitterX);
                point.m0_position.m4_Y = point.m0_position.m4_Y + fixedPoint(jitterY);
                point.m0_position.m8_Z = point.m0_position.m8_Z + fixedPoint(jitterZ);
            }

            // Integrate velocity
            point.mC_velocity = point.mC_velocity + point.m0_position;

            // Apply friction to velocity
            fixedPoint friction(0x3d7);
            point.mC_velocity = point.mC_velocity - MTH_Mul(friction, point.mC_velocity);

            // Update orbital position
            point.m18_orbitalPosition = point.m18_orbitalPosition + point.mC_velocity;
            point.m18_orbitalPosition.m0_X = fixedPoint((s32)point.m18_orbitalPosition.m0_X & 0xFFFFFFF);
            point.m18_orbitalPosition.m4_Y = fixedPoint((s32)point.m18_orbitalPosition.m4_Y & 0xFFFFFFF);
            point.m18_orbitalPosition.m8_Z = fixedPoint((s32)point.m18_orbitalPosition.m8_Z & 0xFFFFFFF);

            // Reset acceleration
            point.m0_position.zeroize();
        }
    }

    // Trigger fade at frame 0x3C
    if ((pThis->m34_param8 == 1) && (pThis->m2C_frameCounter == 0x3C))
    {
        urchinFormation_createFadeTask(pThis, 0xC210, 0xE739, 0x1E, 0xE739, 0xC210, 10);
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished = 1;
    }

    // Update particles
    for (int i = 0; i < pThis->m24_numParticles; i++)
    {
        sImpactParticle& particle = pThis->mC_particles[i];
        if (particle.m34_startFrame <= pThis->m2C_frameCounter)
        {
            if (pThis->m2C_frameCounter == 0x46)
            {
                particle.m36_state = 1;
            }
            impactParticle_update(&particle);
        }
    }
}

// 0608d4c8
static void sImpactEffectTask_Draw(sImpactEffectTask* pThis)
{
    int finishedCount = 0;
    for (int i = 0; i < pThis->m24_numParticles; i++)
    {
        sImpactParticle& particle = pThis->mC_particles[i];
        if (particle.m34_startFrame <= pThis->m2C_frameCounter)
        {
            if (particle.m34_startFrame + 100 < pThis->m2C_frameCounter)
            {
                finishedCount++;
                if (finishedCount == pThis->m24_numParticles)
                {
                    pThis->getTask()->markFinished();
                }
                continue;
            }
            pushCurrentMatrix();
            translateCurrentMatrix(&pThis->m14_target->m0_worldPosition);
            sImpactOrbitalPoint& orbital = pThis->m10_orbitalPoints[particle.m30_orbitalIndex];
            rotateCurrentMatrixYXZ(&orbital.m18_orbitalPosition);
            drawProjectedParticle(&particle.m24_quad, &particle.m18_position);
            popMatrix();
        }
    }
}

// 0608ca18
static void urchinFormation_createImpactEffect(BTL_A3_UrchinFormation* pThis, sPerEnemySlot* pTarget, s32 param3, s32 param4, s32 param5, s32 param6, s32 param7, s32 param8)
{
    static const sImpactEffectTask::TypedTaskDefinition definition = {
        nullptr,
        sImpactEffectTask_Update,
        sImpactEffectTask_Draw,
        nullptr,
    };

    sImpactEffectTask* pTask = createSubTask<sImpactEffectTask>(pThis, &definition);

    s32 numParticles = param5;
    s32 numOrbitalPoints = param4;

    pTask->mC_particles = new sImpactParticle[numParticles];
    pTask->m10_orbitalPoints = new sImpactOrbitalPoint[numOrbitalPoints];
    pTask->m14_target = pTarget;
    pTask->m18_positionSource = &pTarget->m0_worldPosition;
    pTask->m1C_radius = param3;
    pTask->m20_numOrbitalPoints = numOrbitalPoints;
    pTask->m24_numParticles = numParticles;
    pTask->m30_param7 = param6;
    pTask->m34_param8 = param7;
    pTask->m2C_frameCounter = 0;

    if (s_projectileSpriteData.empty())
    {
        s_projectileSpriteData = initVdp1Quad(g_BTL_A3->getSaturnPtr(0x060af4a0));
    }
    u16 vdp1Memory = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory;

    // Initialize orbital points with random positions on a sphere
    for (int j = 0; j < numParticles; j++)
    {
        for (int i = 0; i < numOrbitalPoints; i++)
        {
            sImpactOrbitalPoint& point = pTask->m10_orbitalPoints[i];
            point.m0_position.zeroize();
            point.mC_velocity.zeroize();
            point.m18_orbitalPosition.zeroize();

            // Random spherical position for velocity
            s32 elevAngle = MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x10000000));
            s32 elevSin = MTH_Mul(getSin((u16)((u32)elevAngle >> 16) & 0xFFF), fixedPoint(0xaaaaaa));
            point.mC_velocity.m4_Y = elevSin;

            s32 azimAngle = MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x10000000));
            s32 elevCos = MTH_Mul(getCos((u16)((u32)azimAngle >> 16) & 0xFFF), fixedPoint(elevSin));

            s32 azimAngle2 = MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x10000000));
            point.mC_velocity.m8_Z = MTH_Mul(getSin((u16)((u32)azimAngle2 >> 16) & 0xFFF), fixedPoint(elevCos));
            point.mC_velocity.m0_X = MTH_Mul(getCos((u16)((u32)azimAngle2 >> 16) & 0xFFF), fixedPoint(elevCos));

            // Random orbital velocity
            s32 orbRadius = MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x4000000));
            s32 orbAngle1 = MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x10000000));
            point.m18_orbitalPosition.m4_Y = MTH_Mul(getSin((u16)((u32)orbAngle1 >> 16) & 0xFFF), fixedPoint(orbRadius));

            s32 orbAngle2 = MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x10000000));
            s32 orbCos = MTH_Mul(getCos((u16)((u32)orbAngle2 >> 16) & 0xFFF), fixedPoint(orbRadius));

            s32 orbAngle3 = MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x10000000));
            point.m18_orbitalPosition.m8_Z = MTH_Mul(getSin((u16)((u32)orbAngle3 >> 16) & 0xFFF), fixedPoint(orbCos));
            point.m18_orbitalPosition.m0_X = MTH_Mul(getCos((u16)((u32)orbAngle3 >> 16) & 0xFFF), fixedPoint(orbCos));
        }

        // Initialize particle
        impactParticle_init(&pTask->mC_particles[j], numOrbitalPoints);
        particleInitSub(&pTask->mC_particles[j].m24_quad, vdp1Memory, &s_projectileSpriteData);

        // Random position on sphere of given radius
        s32 angle1 = MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x10000000));
        s32 sin1 = MTH_Mul(getSin((u16)((u32)angle1 >> 16) & 0xFFF), fixedPoint(param3));
        pTask->mC_particles[j].m18_position.m4_Y = sin1;

        s32 angle2 = MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x10000000));
        s32 cos1 = MTH_Mul(getCos((u16)((u32)angle2 >> 16) & 0xFFF), fixedPoint(sin1));

        s32 angle3 = MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x10000000));
        pTask->mC_particles[j].m18_position.m8_Z = MTH_Mul(getSin((u16)((u32)angle3 >> 16) & 0xFFF), fixedPoint(cos1));
        pTask->mC_particles[j].m18_position.m0_X = MTH_Mul(getCos((u16)((u32)angle3 >> 16) & 0xFFF), fixedPoint(cos1));
    }
}

// 06078b96 — ranged/beam attack execution (type 5)
static void urchinFormation_executeRangedAttack(BTL_A3_UrchinFormation* pThis)
{
    sAttackCommand* pAttack = pThis->m18.m0_currentAttack;
    switch (pThis->m2_subState)
    {
    case 1:
    {
        if (!(gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2000))
            return;
        int attackCount = 0;
        for (int i = 0; i < pThis->m3_formationSize; i++)
        {
            u8 statusFlags = pThis->m18.m14[i].m18_statusFlags;
            if (((statusFlags & 3) == pThis->m34_attackingTypeIndex) && ((statusFlags & 8) == 0))
            {
                if (pThis->m6_attackFlag == 0)
                {
                    pThis->m18.m14[i].m19_attackFlags = 1;
                    attackCount++;
                }
                else if (pThis->m18.m14[i].m18_statusFlags & 0x20)
                {
                    pThis->m18.m14[i].m19_attackFlags = 1;
                    attackCount++;
                }
            }
            if (attackCount >= readSaturnU8(pAttack->m0_attackParams + 0))
                break;
        }
        if (attackCount == 0)
        {
            pThis->m2_subState = 0x1E;
        }
        else
        {
            pThis->m2_subState++;
            pThis->m36_meleeAttackTimer = 0;
        }
        return;
    }
    case 2:
    {
        // Find first enemy waiting to attack
        for (int i = 0; i < pThis->m3_formationSize; i++)
        {
            if (pThis->m18.m14[i].m19_attackFlags & 1)
            {
                pThis->m18.m14[i].m19_attackFlags = 2;
                pThis->m18.m14[i].m19_attackFlags &= ~1;
                pThis->m39_sourceEnemyIndex = (s8)i;
                pThis->m2_subState++;
                return;
            }
        }
        pThis->m2_subState = 0x1E;
        return;
    }
    case 3:
    {
        // Wait for any active attacking enemies to finish
        for (int i = 0; i < pThis->m3_formationSize; i++)
        {
            if (pThis->m18.m14[i].m18_statusFlags & 0x10)
            {
                return;
            }
        }

        // Find weakest enemy (lowest HP ratio)
        int weakestHp = 200;
        s8 weakestIndex = (s8)pThis->m3_formationSize;
        for (int i = 0; i < pThis->m3_formationSize; i++)
        {
            if (((pThis->m18.m14[i].m18_statusFlags & 8) == 0) &&
                (pThis->m18.m14[i].m1A_hpRatio < weakestHp))
            {
                weakestHp = pThis->m18.m14[i].m1A_hpRatio;
                weakestIndex = (s8)i;
            }
        }
        pThis->m3A_weakestEnemyIndex = weakestIndex;

        urchinFormation_createProjectileTask(pThis,
            &pThis->m18.m14[pThis->m39_sourceEnemyIndex],
            &pThis->m18.m14[pThis->m3A_weakestEnemyIndex]);

        s8 soundEffect = readSaturnS8(pAttack->m0_attackParams + 4);
        if (soundEffect != -1)
        {
            playSystemSoundEffect(soundEffect);
        }

        pThis->m3C_rangedAttackTimer = 0x3C;
        pThis->m2_subState++;
        return;
    }
    case 4:
    {
        pThis->m3C_rangedAttackTimer--;
        if (pThis->m3C_rangedAttackTimer >= 0)
            return;

        pThis->m18.m14[pThis->m3A_weakestEnemyIndex].m1C_flags = readSaturnU16(pAttack->m0_attackParams + 2);

        urchinFormation_createImpactEffect(pThis,
            &pThis->m18.m14[pThis->m3A_weakestEnemyIndex],
            0x3000, 4, 0x18, 0x20000, 0, 0);

        s8 soundEffect = readSaturnS8(pAttack->m0_attackParams + 4);
        if (soundEffect != -1)
        {
            playSystemSoundEffect(soundEffect);
        }

        pThis->m3C_rangedAttackTimer = 0x1E;
        pThis->m2_subState++;
        return;
    }
    case 5:
    {
        pThis->m3C_rangedAttackTimer--;
        if (pThis->m3C_rangedAttackTimer >= 0)
            return;
        pThis->m2_subState = 2;
        return;
    }
    default:
        return;
    }
}

void BTL_A3_UrchinFormation_Update_Mode2(BTL_A3_UrchinFormation* pThis)
{
    s8 subState = pThis->m2_subState;

    if (subState == 0)
    {
        // Wait for battle intro to finish, then set up attack camera
        if (!battleEngine_isBattleIntroFinished())
            return;
        BTL_A3_UrchinFormation_Update_Mode1Sub6(pThis);
        pThis->m2_subState++;
        return;
    }

    if (subState == 0x1E)
    {
        // Attack animation finished — clear flags and process post-attack effects
        for (int i = 0; i < pThis->m3_formationSize; i++)
        {
            pThis->m18.m14[i].m19_attackFlags &= ~1;
            pThis->m18.m14[i].m19_attackFlags &= ~2;
        }
        pThis->m6_attackFlag = 0;
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished = 1;

        if (pThis->m18.m0_currentAttack->m9_flags & 0x80)
            return;

        if ((pThis->m18.m0_currentAttack->m9_flags & 0x38) == 0)
        {
            if (pThis->m18.m0_currentAttack->m9_flags & 0x1)
                BTL_A3_UrchinFormation_Update_Mode1Sub3(pThis);
            if (pThis->m18.m0_currentAttack->m9_flags & 0x6)
                BTL_A3_UrchinFormation_Update_Mode1Sub4(pThis, pThis->m18.m0_currentAttack);
            pThis->m2_subState = 0x20;
            return;
        }
        pThis->m2_subState++;
        // fall through to type dispatch below
    }
    else if (subState == 0x1F)
    {
        // Post-attack with special effect — wait for intro, then process flags
        if (!battleEngine_isBattleIntroFinished())
            return;
        if (pThis->m18.m0_currentAttack->m9_flags & 0x38)
            BTL_A3_UrchinFormation_Update_Mode1Sub5(pThis->m18.m0_currentAttack);
        if (pThis->m18.m0_currentAttack->m9_flags & 0x1)
            BTL_A3_UrchinFormation_Update_Mode1Sub3(pThis);
        if (pThis->m18.m0_currentAttack->m9_flags & 0x6)
            BTL_A3_UrchinFormation_Update_Mode1Sub4(pThis, pThis->m18.m0_currentAttack);
        pThis->m2_subState++;
        return;
    }
    else if (subState == 0x20)
    {
        // Return to idle (mode 1)
        pThis->m1_state = 1;
        pThis->m2_subState = 0;
        return;
    }
    else
    {
        // Default: dispatch based on attack type
        s8 attackType = pThis->m18.m0_currentAttack->m8_type;
        switch (attackType)
        {
        case 0:
            pThis->m2_subState = 0x1E;
            return;
        case 2:
        case 3:
        case 6:
        case 7:
            urchinFormation_executeAttack(pThis);
            return;
        case 4:
            return; // wait state
        case 5:
            urchinFormation_executeRangedAttack(pThis);
            return;
        default:
            pThis->m2_subState = 0x1E;
            return;
        }
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
            if (pThis->m18.m4_typeAlive[i] == 0)
            {
                local_2c[i] = -1;
            }
            else
            {
                bVar1 = true;
                local_2c[i] = pThis->m30_config->m20_deathSoundEffect[i];
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
            if (pThis->m18.m4_typeAlive[j] && pThis->m30_config->m4_perTypeParams[j])
            {
                static const std::array<s8, 4> tempArray =
                {
                    6,4,2,0
                };
                int uVar1 = rotateRightR0ByR1(pThis->m30_config->m4_perTypeParams[j]->m1C[pThis->m18.m7_attackAnimIndex[j]].m1E_quadrantAttackDirections, tempArray[enemyQuadrantsTable[pThis->m18.mD[j]][i]]);
                uVar3 |= uVar1 & 3;
            }
        }

        switch (uVar3)
        {
        case 0:
            battleEngine_FlagQuadrantForDanger(i);
            break;
        case 3:
            battleEngine_FlagQuadrantForSafety(i);
            break;
        default:
            // cases 1, 2: neither safe nor dangerous
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
        // Debug: force specific attack — not implemented
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
                if (battleEngine_isBattleIntroFinished())
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
                        // Special formation with m1C set — create pending task
                        Unimplemented(); // calls config->m1C as Saturn function pointer; never reached (all formations have m1C=0)
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
                if (battleEngine_isPlayerTurnActive() && 
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
                break;
            }

            return;
        }

        if (!battleEngine_isPlayerTurnActive())
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
    case 3:
        // Death state — all enemies dead, waiting for cleanup
        break;
    default:
        break;
    }

    if (pThis->m1_state)
    {
        for (int i = 0; i < 3; i++)
        {
            pThis->m18.m4_typeAlive[i] = 0;
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
