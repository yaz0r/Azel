#include "PDS.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "battle/battleEngine.h"
#include "battle/battleEngineSub0.h"
#include "battle/battleFormation.h"
#include "battle/battleDebug.h"
#include "BTL_A3_BaldorFormation.h"
#include "baldor.h"

#include "town/town.h" // TODO: cleanup

// internal name: Hebi (snake)

struct BTL_A3_BaldorFormation : public s_workAreaTemplateWithArg<BTL_A3_BaldorFormation, u32>
{
    s8 m0_formationState;
    s8 m1_formationSubState;
    std::vector<sFormationData> m4_formationData;
    s16 m8;
    s16 m10;
    s8 m12_formationSize;
    // size 0x14
};

void displayFormationName(short uParm1, char uParm2, char uParm3)
{
    if (gBattleManager->m10_battleOverlay->m14)
    {
        assert(0);
    }
}

void BTL_A3_BaldorFormation_Init(BTL_A3_BaldorFormation* pThis, u32 formationID)
{
    allocateNPC(pThis, 10);

    npcFileDeleter* pBuffer = dramAllocatorEnd[10].mC_fileBundle;

    if (formationID == 0)
    {
        pThis->m12_formationSize = 1;
    }
    else
    {
        if ((formationID == 1) || (formationID == 2))
        {
            pThis->m12_formationSize = 2;
        }
    }

    pThis->m4_formationData = allocateFormationData(pThis, pThis->m12_formationSize);

    sSaturnPtr formationDataSource = gCurrentBattleOverlay->getSaturnPtr(0x60a809c) + 4 * formationID;
    for (int i = 0; i < pThis->m12_formationSize; i++)
    {
        pThis->m4_formationData[i].m0_translation.mC_target = readSaturnVec3(readSaturnEA(formationDataSource) + 0xC * i + 0);
        pThis->m4_formationData[i].m0_translation.m18 = readSaturnVec3(readSaturnEA(formationDataSource) + 0xC * i + 0);
        pThis->m4_formationData[i].m24_rotation.mC_target[1] = 0x8000000;

        sBaldor* pBaldor = createBaldor(pBuffer, &pThis->m4_formationData[i]);
        pBaldor->mA_indexInFormation = i;
    }

    battleEngine_FlagQuadrantForSafety(2);
    battleEngine_FlagQuadrantForDanger(0);
    pThis->m8 = 0x3C;
    displayFormationName(0, 1, 9);
    pThis->m0_formationState = 0;
    pThis->m1_formationSubState = 0;
}

void battleEngine_PlayAttackCamera(int param1)
{
    battleEngine_SetBattleMode(eBattleModes::m8_playAttackCamera);
    gBattleManager->m10_battleOverlay->m4_battleEngine->m433_attackCameraIndex = param1;
}

void battleEngine_displayAttackName(int param1, int param2, int param3)
{
    if (gBattleManager->m10_battleOverlay->m14)
    {
        assert(0);
    }
}

void BTL_A3_BaldorFormation_Update(BTL_A3_BaldorFormation* pThis)
{
    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x1B])
    {
        assert(0);
    }

    gBattleManager->m10_battleOverlay->m4_battleEngine->m3B2_numBattleFormationRunning++;

    switch (pThis->m0_formationState)
    {
    case 0:
        break;
    case 1:
    case 2:
        switch (pThis->m1_formationSubState)
        {
        case 0:
            if (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2000) {
                pThis->m10 = 18;
                pThis->m1_formationSubState++;
            }
            break;
        case 1:
            if (--pThis->m10 > 0)
            {
                return;
            }
            for (int i = 0; i < pThis->m12_formationSize; i++)
            {
                if ((pThis->m4_formationData[i].m48 & 1) != 0) {
                    pThis->m4_formationData[i].m49 = 1;
                    pThis->m1_formationSubState++;
                    return;
                }
            }
            pThis->m1_formationSubState = 10;
            return;
        case 2:
            for (int i = 0; i < pThis->m12_formationSize; i++)
            {
                if ((pThis->m4_formationData[i].m48 & 1) != 0) {
                    return;
                }
            }
            pThis->m1_formationSubState = 1;
            return;
        case 10:
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished = 1;
            for (int i = 0; i < pThis->m12_formationSize; i++)
            {
                pThis->m4_formationData[i].m49 = 0;
            }
            pThis->m0_formationState = 0;
            pThis->m1_formationSubState = 0;
            return;
        default:
            assert(0);
            break;
        }
        return;
    default:
        assert(0);
        break;
    }

    bool bFormationDefeated = false;
    for (int i = 0; i < pThis->m12_formationSize; i++)
    {
        if ((pThis->m4_formationData[i].m48 & 4) == 0)
        {
            bFormationDefeated = true;
            break;
        }
    }

    if (!bFormationDefeated)
    {
        if (--pThis->m8 < 0)
        {
            pThis->getTask()->markFinished();
        }
    }

    if (gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m8 == 0)
        return;

    if (BattleEngineSub0_UpdateSub0() != 0)
        return;

    gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m8 = 0;
    gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m0 = 0;

    if (!bFormationDefeated)
        return;

    switch (gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant)
    {
    case 0:
        // Dragon is behind the formation, move it in front of it
        gBattleManager->m10_battleOverlay->m4_battleEngine->m22E_dragonMoveDirection = 3;
        battleEngine_SetBattleMode(eBattleModes::mB_enemyMovingDragon);
        gBattleManager->m10_battleOverlay->m4_battleEngine->m38D_battleSubMode = 2;
        gBattleManager->m10_battleOverlay->m4_battleEngine->m27C_dragonMovementInterpolator1.m68_rate = 0x3C;
        gBattleManager->m10_battleOverlay->m4_battleEngine->m2E8_dragonMovementInterpolator2.m68_rate = 0x3C;
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m400000 = 1;
        gBattleManager->m10_battleOverlay->m4_battleEngine->m38E = 0;
        break;
    case 1:
    case 3:
        // Dragon is on the side of the formation, just attack
        if ((randomNumber() & 1) == 0) {
            battleEngine_PlayAttackCamera(2);
        }
        else
        {
            battleEngine_PlayAttackCamera(7);
        }
        for (int i = 0; i < pThis->m12_formationSize; i++)
        {
            pThis->m4_formationData[i].m49 = 3;
        }
        battleEngine_displayAttackName(2, 0x1E, 0);
        battleEngine_FlagQuadrantForAttack(1);
        battleEngine_FlagQuadrantForAttack(3);
        pThis->m0_formationState = 2;
        break;
    case 2:
    {
        // Dragon is in front of formation, critical attack
        switch (performModulo2(3, randomNumber()))
        {
        case 0:
            battleEngine_SetBattleMode(eBattleModes::m7);
            break;
        case 1:
            battleEngine_PlayAttackCamera(8);
            break;
        case 2:
            battleEngine_PlayAttackCamera(1);
            break;
        default:
            assert(0);
        }

        int numEntriesFound = 0;
        for (int i = 0; i < pThis->m12_formationSize; i++)
        {
            if ((pThis->m4_formationData[i].m48 & 4) == 0) {
                pThis->m4_formationData[i].m49 = 2;
                numEntriesFound++;
                if (numEntriesFound > 1)
                    break;
            }
        }

        battleEngine_displayAttackName(1, 0x1E, 0);
        battleEngine_FlagQuadrantForAttack(2);
        pThis->m0_formationState = 1;
        break;
    }
    default:
        assert(0);
    }
}

p_workArea Create_BTL_A3_BaldorFormation(p_workArea parent, u32 arg)
{
    static const BTL_A3_BaldorFormation::TypedTaskDefinition definition = {
        BTL_A3_BaldorFormation_Init,
        BTL_A3_BaldorFormation_Update,
        nullptr,
        nullptr,
    };

    return createSubTaskWithArg<BTL_A3_BaldorFormation>(parent, arg, &definition);
}
