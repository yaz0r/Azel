#include "PDS.h"
#include "BTL_A7_formation.h"
#include "BTL_A7_data.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "battle/battleEngine.h"
#include "battle/battleEngineSub0.h"
#include "battle/battleTextDisplay.h"
#include "kernel/graphicalObject.h"
#include "town/town.h"
#include "commonOverlay.h"
#include "battle/battleDebug.h"

void battleEngine_UpdateSub1Sub0(s32 param_1);
void battleEngine_displayAttackName(int param1, int param2, int param3);
void battleEngine_PlayAttackCamera(int param1);

static const s8 attackPatternTable[] = {
    0, 0, 2, 1, 4, 0, 3, 2, 4, 0
};

// 06057f10
static void BTL_A7_selectAttackPattern(sBTL_A7_FormationTask* pThis)
{
    s8 pattern = attackPatternTable[pThis->mF4_attackPatternIndex];
    if (pattern == 0)
    {
        battleEngine_initiateEnemyMoveDragon(1, 0x18);
    }
    else if (pattern == 1)
    {
        battleEngine_initiateEnemyMoveDragon(2, 0x18);
    }
    else if (pattern == 2)
    {
        pThis->m2C_posBlock.m28_altitudeOffset += 0x4000;
        battleEngine_initiateEnemyMoveDragon(3, 0x18);
    }
    else if (pattern == 3)
    {
        pThis->m2C_posBlock.m28_altitudeOffset += -0x6000;
        battleEngine_initiateEnemyMoveDragon(3, 0x18);
    }
    else if (pattern == 4 || pattern == 5)
    {
        battleEngine_initiateEnemyMoveDragon(3, 0x18);
    }

    s8 idx = pThis->mF4_attackPatternIndex;
    pThis->mF4_attackPatternIndex = idx + 1;
    if (idx > 8)
    {
        pThis->mF4_attackPatternIndex = 0;
    }
}

// 06057f92
static void BTL_A7_handleEnemyTurn(sBTL_A7_FormationTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    s8 quadrant = pEngine->m22C_dragonCurrentQuadrant;

    if (quadrant == 0)
    {
        s8 count = pThis->mF6_attackCounter;
        pThis->mF6_attackCounter = count + 1;
        if (count < 5)
        {
            BTL_A7_selectAttackPattern(pThis);
        }
        else
        {
            pThis->mF6_attackCounter = 0;
            Unimplemented(); // 0609eaf2 - trigger enemy attack animation
            battleEngine_SetBattleMode(eBattleModes::m7);
            pThis->m28_state = 7;
        }
    }
    else if (quadrant == 1 || quadrant == 3)
    {
        pThis->mF5_flag = 1;
        BTL_A7_selectAttackPattern(pThis);
    }
    else if (quadrant == 2)
    {
        if (pThis->mF5_flag != 0)
        {
            pThis->mF5_flag = 0;
            u32 rng = randomNumber();
            s32 cameraType = (rng & 1) == 0 ? 1 : 2;
            battleEngine_PlayAttackCamera(cameraType);
            battleEngine_FlagQuadrantBitForAttack(4);
            pThis->m28_state = 4;
            goto done;
        }
    }

done:
    pEngine->m3CC->m8 = 0;
    *(s16*)pEngine->m3CC = 0;
}

// 06057AAC
static void BTL_A7_formationTask_Update(sBTL_A7_FormationTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    pEngine->m3B2_numBattleFormationRunning++;

    // 0609ec28 - check entity array for finished tasks
    Unimplemented();

    // 0609e414 - update entity positions relative to battle center
    Unimplemented();

    switch (pThis->m28_state)
    {
    case 0:
        if (battleEngine_isBattleIntroFinished())
        {
            battleEngine_SetBattleMode(eBattleModes::m7);
            battleEngine_setupAttackCamera(&pThis->m2C_posBlock.m0_position, 0x50000, 0);
            pThis->m28_state = 1;
        }
        break;

    case 1:
        if (pEngine->m188_flags.m2000)
        {
            battleEngine_displayAttackName(0xF, 0x3C, 0);
            pThis->m28_state = 2;
        }
        break;

    case 2:
    {
        s16 count = pThis->m20;
        pThis->m20 = count + 1;
        if (count > 0x3C)
        {
            pThis->m20 = 0;
            pEngine->m188_flags.m100_attackAnimationFinished = 1;
            pEngine->m3CC->m8 = 0;
            *(s16*)pEngine->m3CC = 0;
            battleEngine_FlagQuadrantBitForSafety(0);
            battleEngine_FlagQuadrantBitForSafety(4);
            battleEngine_FlagQuadrantBitForDanger(0);
            battleEngine_FlagQuadrantBitForDanger(0xB);
            pThis->m28_state = 6;
        }
        break;
    }

    case 3:
    {
        s32 active = battleEngine_isPlayerTurnActive();
        if (active == 1)
            return;
        if (pEngine->m3CC->m8 == 1)
        {
            BTL_A7_handleEnemyTurn(pThis);
        }
        break;
    }

    case 4:
        if (pEngine->m188_flags.m2000)
        {
            battleEngine_displayAttackName(0x10, 0x1E, 0);
            Unimplemented(); // 0609eaf2 - trigger enemy attack with param 3
            pThis->m28_state = 5;
        }
        break;

    case 5:
    {
        s16 count = pThis->m24_counter1;
        pThis->m24_counter1 = count + 1;
        if (count > 0x15)
        {
            pThis->m24_counter1 = 0;
            pEngine->m188_flags.m100_attackAnimationFinished = 1;
            pThis->m28_state = 6;
        }
        break;
    }

    case 6:
    {
        s32 active = battleEngine_isPlayerTurnActive();
        if (active == 0)
        {
            battleEngine_restoreCameraDefault();
            pThis->m28_state = 3;
            pEngine->m3CC->m2 = 0x21;
            pEngine->m3CC->m8 = 0;
            *(s16*)pEngine->m3CC = 0;
        }
        break;
    }

    case 7:
        if (pEngine->m188_flags.m2000)
        {
            battleEngine_displayAttackName(0x11, 0x1E, 0);
            pThis->m28_state = 8;
        }
        break;

    case 8:
    {
        s16 count = pThis->m20;
        pThis->m20 = count + 1;
        if (count > 0x1E)
        {
            pThis->m20 = 0;
            pEngine->m188_flags.m100_attackAnimationFinished = 1;
            pThis->m28_state = 6;
        }
        break;
    }

    case 9:
    {
        s16 count = pThis->m26_counter2;
        pThis->m26_counter2 = count + 1;
        if (count > 0x78)
        {
            pThis->m26_counter2 = 0;
            pThis->getTask()->markFinished();
            decreaseNPCRefCount(0xE);
        }
        break;
    }

    default:
        pThis->m28_state = 9;
        break;
    }

    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x1A] != 0)
    {
        vdp2DebugPrintSetPosition(1, 5);
        vdp2PrintfSmallFont("M:");
    }
}

// 0605797c
static void BTL_A7_formationTask_Init(sBTL_A7_FormationTask* pThis)
{
    allocateNPC(pThis, 0xE);

    pThis->mE0_entityGroup.mC_formationNameIndex = 0x54;
    battleEngine_UpdateSub1Sub0(0);

    s16 nameIndex = pThis->mE0_entityGroup.mC_formationNameIndex;
    sSaturnPtr nameTableEntry = gCommonFile->getSaturnPtr(0x0020179c) + nameIndex * 8;
    displayFormationName(0xE, readSaturnS8(nameTableEntry + 3), readSaturnS8(nameTableEntry + 2));

    pThis->mE0_entityGroup.m8_dataTable = g_BTL_A7->getSaturnPtr(0x060a9f90);
    pThis->mE0_entityGroup.m4_positionBlock = &pThis->m2C_posBlock;

    pThis->mE0_entityGroup.mE_numEntities = 3;
    pThis->mE0_entityGroup.mF_deadCount = 0;

    pThis->mE0_entityGroup.m0_entityArray = (p_workArea*)allocateHeapForTask(pThis, pThis->mE0_entityGroup.mE_numEntities * sizeof(p_workArea));

    for (int i = 0; i < pThis->mE0_entityGroup.mE_numEntities; i++)
    {
        // 06058090 - create enemy entity
        Unimplemented();
        pThis->mE0_entityGroup.m0_entityArray[i] = nullptr;
    }
}

// 06057a2a
void BTL_A7_createFormation(s_workAreaCopy* pParent, u32 arg0)
{
    static const sBTL_A7_FormationTask::TypedTaskDefinition def = {
        nullptr,
        &BTL_A7_formationTask_Update,
        nullptr,
        nullptr,
    };

    sBTL_A7_FormationTask* pTask = createSubTaskWithCopy<sBTL_A7_FormationTask>(pParent, &def);
    if (!pTask)
        return;

    BTL_A7_formationTask_Init(pTask);

    battleEngine_FlagQuadrantBitForSafety(4);
    battleEngine_FlagQuadrantBitForDanger(0xB);
    gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m2 = 0x21;
    pTask->m28_state = 0;
}
