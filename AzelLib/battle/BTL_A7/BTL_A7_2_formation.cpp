#include "PDS.h"
#include "BTL_A7_2_formation.h"
#include "BTL_A7_2_enemy.h"
#include "BTL_A7_2_data.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "battle/battleEngine.h"
#include "battle/battleEngineSub0.h"
#include "battle/battleTextDisplay.h"
#include "battle/battleDebug.h"
#include "battle/battleDragon.h"
#include "kernel/graphicalObject.h"
#include "town/town.h"
#include "commonOverlay.h"

void battleEngine_UpdateSub1Sub0(s32 param_1);
void battleEngine_displayAttackName(int param1, int param2, int param3);
void battleEngine_PlayAttackCamera(int param1);
s32 playBattleSoundEffect(s32 effectIndex);

// 06055868
static void BTL_A7_2_formationStartBattleIntro(sBTL_A7_2_FormationTask* pThis)
{
    battleEngine_SetBattleMode(eBattleModes::m9);
    if (gBattleManager->m10_battleOverlay->m4_battleEngine->m3D0 == nullptr)
    {
        Unimplemented(); // createItemVisualEffect
    }
}

// 0609b2b8 - unknown init sub
static void formationInitSub(sBTL_A7_2_FormationTask* pThis)
{
    Unimplemented();
}

// 06054C82 - main enemy turn callback (quadrant-based attack selection)
static void BTL_A7_2_handleEnemyTurn(sFormationTaskBase* pBase)
{
    sBTL_A7_2_FormationTask* pThis = static_cast<sBTL_A7_2_FormationTask*>(pBase);
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    pEngine->m3CC->m8 = 0;
    *(s16*)pEngine->m3CC = 0;

    s8 quadrant = pEngine->m22C_dragonCurrentQuadrant;

    if (quadrant == 0)
    {
        s32 hasFlag = formationCheckEntityFlag(&pThis->mF4_entityGroup2, 8);
        battleEngine_initiateEnemyMoveDragon(hasFlag == 0 ? 1 : 2, 0x4B);
        pThis->m28_state = 0;
    }
    else if (quadrant == 1)
    {
        s32 hasFlag8 = formationCheckEntityFlag(&pThis->mF4_entityGroup2, 8);
        if (hasFlag8 != 0)
        {
            s32 hasFlag7 = formationCheckEntityFlag(&pThis->mF4_entityGroup2, 7);
            if (hasFlag7 == 0)
            {
                pThis->mCC_attackType = 9;
                pThis->mCD_displayMode = 1;
                pThis->mD0_attackNameIndex = 0x19;
            }
            else
            {
                pThis->mCC_attackType = 10;
                pThis->mCD_displayMode = 1;
                pThis->mD0_attackNameIndex = 0x18;
            }
            pThis->mCE_attackDuration = 0x1E;
            pThis->mD4_attackFlag = 0;
            battleEngine_PlayAttackCamera(3);
            pThis->m28_state = 2;
            return;
        }
        s32 hasFlag4 = formationCheckEntityFlag(&pThis->mF4_entityGroup2, 4);
        if (hasFlag4 == 0)
        {
            s32 hasFlagC = formationCheckEntityFlag(&pThis->mF4_entityGroup2, 0xC);
            if (hasFlagC == 0)
            {
                pThis->mCC_attackType = 2;
                pThis->mCD_displayMode = 1;
                pThis->mD0_attackNameIndex = 0x17;
                pThis->mCE_attackDuration = 0x3C;
                pThis->mD4_attackFlag = 0;
                battleEngine_PlayAttackCamera(3);
                pThis->m28_state = 2;
                return;
            }
            battleEngine_initiateEnemyMoveDragon(2, 0x4B);
        }
        else
        {
            battleEngine_initiateEnemyMoveDragon(2, 0x4B);
        }
    }
    else if (quadrant == 2)
    {
        s32 hasFlagC = formationCheckEntityFlag(&pThis->mF4_entityGroup2, 0xC);
        if (hasFlagC != 0)
        {
            s32 hasFlagB = formationCheckEntityFlag(&pThis->mF4_entityGroup2, 0xB);
            if (hasFlagB == 0)
            {
                pThis->mCC_attackType = 0xD;
                pThis->mCD_displayMode = 1;
                pThis->mD0_attackNameIndex = 0x19;
            }
            else
            {
                pThis->mCC_attackType = 0xE;
                pThis->mCD_displayMode = 1;
                pThis->mD0_attackNameIndex = 0x18;
            }
            pThis->mCE_attackDuration = 0x1E;
            pThis->mD4_attackFlag = 0;
            battleEngine_PlayAttackCamera(3);
            pThis->m28_state = 2;
            return;
        }
        s32 hasFlag4 = formationCheckEntityFlag(&pThis->mF4_entityGroup2, 4);
        if (hasFlag4 == 0)
        {
            s32 hasFlag8 = formationCheckEntityFlag(&pThis->mF4_entityGroup2, 8);
            if (hasFlag8 == 0)
            {
                pThis->mCC_attackType = 2;
                pThis->mCD_displayMode = 1;
                pThis->mD0_attackNameIndex = 0x17;
                pThis->mCE_attackDuration = 0x3C;
                pThis->mD4_attackFlag = 0;
                battleEngine_PlayAttackCamera(3);
                pThis->m28_state = 2;
                return;
            }
            battleEngine_initiateEnemyMoveDragon(1, 0x4B);
        }
        else
        {
            battleEngine_initiateEnemyMoveDragon(2, 0x4B);
        }
    }
    else if (quadrant == 3)
    {
        s32 hasFlag4 = formationCheckEntityFlag(&pThis->mF4_entityGroup2, 4);
        if (hasFlag4 != 0)
        {
            s32 hasFlag3 = formationCheckEntityFlag(&pThis->mF4_entityGroup2, 3);
            if (hasFlag3 == 0)
            {
                pThis->mCC_attackType = 5;
                pThis->mCD_displayMode = 1;
                pThis->mD0_attackNameIndex = 0x19;
            }
            else
            {
                pThis->mCC_attackType = 6;
                pThis->mCD_displayMode = 1;
                pThis->mD0_attackNameIndex = 0x18;
            }
            pThis->mCE_attackDuration = 0x1E;
            pThis->mD4_attackFlag = 0;
            battleEngine_PlayAttackCamera(3);
            pThis->m28_state = 2;
            return;
        }
        s32 hasFlag8 = formationCheckEntityFlag(&pThis->mF4_entityGroup2, 8);
        if (hasFlag8 == 0)
        {
            s32 hasFlagC = formationCheckEntityFlag(&pThis->mF4_entityGroup2, 0xC);
            if (hasFlagC == 0)
            {
                pThis->mCC_attackType = 2;
                pThis->mCD_displayMode = 1;
                pThis->mD0_attackNameIndex = 0x17;
                pThis->mCE_attackDuration = 0x3C;
                pThis->mD4_attackFlag = 0;
                battleEngine_PlayAttackCamera(3);
                pThis->m28_state = 2;
                return;
            }
            battleEngine_initiateEnemyMoveDragon(1, 0x4B);
        }
        else
        {
            battleEngine_initiateEnemyMoveDragon(1, 0x4B);
        }
    }
    else
    {
        return;
    }
    pThis->m28_state = 0;
}

// 06054728 - state 0: battle intro sequence for main body
static void BTL_A7_2_formationState0(sBTL_A7_2_FormationTask* pThis)
{
    pThis->m190_targetPosition.m0_X = pThis->m2C_posBlock.m0_position.m0_X + readSaturnS32(pThis->mE0_entityGroup.m8_dataTable);
    pThis->m190_targetPosition.m4_Y = pThis->m2C_posBlock.m0_position.m4_Y + readSaturnS32(pThis->mE0_entityGroup.m8_dataTable + 4);
    pThis->m190_targetPosition.m8_Z = pThis->m2C_posBlock.m0_position.m8_Z + readSaturnS32(pThis->mE0_entityGroup.m8_dataTable + 8);

    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;

    switch (pThis->m185_stateStep)
    {
    case 0:
        if (battleEngine_isBattleIntroFinished())
        {
            BTL_A7_2_formationStartBattleIntro(pThis);
            pThis->m185_stateStep++;
        }
        break;
    case 1:
        if (pEngine->m188_flags.m2000)
        {
            pThis->m188_introTask = formationCreateIntroEffect(pThis, g_BTL_A7_2->getSaturnPtr(0x060a4728));
            pThis->m185_stateStep++;
        }
        break;
    case 2:
        if (pThis->m188_introTask == nullptr || (pThis->m188_introTask->getTask()->m14_flags & TASK_FLAGS_FINISHED))
        {
            pThis->mE0_entityGroup.m8_dataTable = g_BTL_A7_2->getSaturnPtr(0x060a4634);
            pThis->mF4_entityGroup2.m8_dataTable = g_BTL_A7_2->getSaturnPtr(0x060a4670);
            pThis->m185_stateStep++;
        }
        break;
    case 3:
        if (pEngine->m3D0 == nullptr)
        {
            pThis->m185_stateStep++;
            pEngine->m188_flags.m100_attackAnimationFinished = 1;
        }
        break;
    case 4:
        if (battleEngine_isPlayerTurnActive() == 0)
        {
            battleEngine_restoreCameraDefault();
            pThis->m185_stateStep = 0;
            pThis->m184_updateState++;
            pThis->m28_state = 0;
        }
        break;
    }
}

// 06054BAC - state 1: secondary entities phase
static void BTL_A7_2_formationState1(sBTL_A7_2_FormationTask* pThis)
{
    s32 allDead = formationAreAllEntitiesDead(&pThis->mF4_entityGroup2);
    if (allDead == 1)
    {
        pThis->m184_updateState++;
        pThis->m28_state = 0;
        s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
        pEngine->m3CC->m8 = 0;
        *(s16*)pEngine->m3CC = 0;
        formationCreateTransitionTask(pThis, g_BTL_A7_2->getSaturnPtr(0x060a4688));
        playBattleSoundEffect(0x6C);
    }
    if (pThis->m105_secondaryFlag == 0)
    {
        playBattleSoundEffect(0x6C);
    }
    formationSubStateMachine(pThis, &pThis->mF4_entityGroup2);
}

// 0605486E - state 2: main body solo phase
// 060556e4
static s32 BTL_A7_2_mainBodyIntroSequence(sBTL_A7_2_FormationTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;

    switch (pThis->m185_stateStep)
    {
    case 0:
        if (battleEngine_isBattleIntroFinished())
        {
            battleEngine_SetBattleMode(eBattleModes::m7);
            pThis->mC0_position2 = pThis->m2C_posBlock.m0_position;
            pThis->mC0_position2.m4_Y += 0x28000;
            pThis->mC0_position2.m8_Z -= 0x64000;
            pThis->mA8_position.m0_X = *(s32*)(gBattleManager->m10_battleOverlay->m18_dragon + 8);
            pThis->mA8_position.m4_Y = *(s32*)(gBattleManager->m10_battleOverlay->m18_dragon + 0xC);
            pThis->mA8_position.m8_Z = *(s32*)(gBattleManager->m10_battleOverlay->m18_dragon + 0x10);
            Unimplemented(); // 0609b2b8 - init sub
            formationTriggerAttack(&pThis->mE0_entityGroup, 0xF);
            pThis->m185_stateStep++;
        }
        break;
    case 1:
        if (pEngine->m188_flags.m2000)
        {
            pThis->m188_introTask = formationCreateIntroEffect(pThis, g_BTL_A7_2->getSaturnPtr(0x060a4770));
            pThis->m185_stateStep++;
        }
        break;
    case 2:
        if (pThis->m188_introTask == nullptr || (pThis->m188_introTask->getTask()->m14_flags & TASK_FLAGS_FINISHED))
        {
            pThis->mE0_entityGroup.m8_dataTable = g_BTL_A7_2->getSaturnPtr(0x060a4640);
            pThis->mD8_updateCallback = (formationUpdateCallback)&BTL_A7_2_handleEnemyTurn; // 06054f30 - a different callback for this phase
            pThis->m185_stateStep++;
        }
        break;
    case 3:
        pThis->m185_stateStep++;
        pEngine->m188_flags.m100_attackAnimationFinished = 1;
        break;
    case 4:
        if (battleEngine_isPlayerTurnActive() == 0)
        {
            battleEngine_restoreCameraDefault();
            pThis->m185_stateStep = 0;
            return 1;
        }
        break;
    }
    return 0;
}

// 0605486E - state 2: main body solo phase
static void BTL_A7_2_formationState2(sBTL_A7_2_FormationTask* pThis)
{
    s32 result = BTL_A7_2_mainBodyIntroSequence(pThis);
    if (result == 1)
    {
        pThis->m184_updateState++;
        battleEngine_FlagQuadrantBitForSafety(0);
        battleEngine_FlagQuadrantBitForDanger(0);
        battleEngine_FlagQuadrantBitForDanger(1);
    }
}

// 06054C16 - state 3: main body death/cleanup
static void BTL_A7_2_formationState3(sBTL_A7_2_FormationTask* pThis)
{
    s32 allDead = formationAreAllEntitiesDead(&pThis->mE0_entityGroup);
    if (allDead == 1)
    {
        pThis->m184_updateState++;
        pThis->m28_state = 0;
        s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
        pEngine->m3CC->m8 = 0;
        *(s16*)pEngine->m3CC = 0;
        pEngine->m3CC->m2 = 0;
        pEngine->m3CC->m4 = 0;
    }
    formationSubStateMachine(pThis, &pThis->mE0_entityGroup);
}

typedef void (*formationStateFunc)(sBTL_A7_2_FormationTask*);
static const formationStateFunc stateTable[] = {
    &BTL_A7_2_formationState0,
    &BTL_A7_2_formationState1,
    &BTL_A7_2_formationState2,
    &BTL_A7_2_formationState3,
};

// 0605462C
static void BTL_A7_2_formationTask_Update(sBTL_A7_2_FormationTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    pEngine->m3B2_numBattleFormationRunning++;

    formationCleanupEntityArray(&pThis->mF4_entityGroup2);
    formationCleanupEntityArray(&pThis->mE0_entityGroup);

    formationUpdatePosition(&pThis->m2C_posBlock, &pEngine->mC_battleCenter);
    formationUpdatePosition(&pThis->m108_posBlock2, &pEngine->mC_battleCenter);

    formationApplyAutoScroll(pThis);

    stateTable[pThis->m184_updateState](pThis);

    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x1A] != 0)
    {
        vdp2DebugPrintSetPosition(1, 6);
        vdp2PrintfSmallFont("%1d", (int)pThis->m184_updateState);
        vdp2DebugPrintSetPosition(4, 6);
        vdp2PrintfSmallFont("%1d", (int)pThis->m185_stateStep);
    }
}

// 060546D2
static void BTL_A7_2_formationTask_Delete(sBTL_A7_2_FormationTask* pThis)
{
    decreaseNPCRefCount(8);
    decreaseNPCRefCount(6);
}

// 06054468
static void BTL_A7_2_formationTask_Init(sBTL_A7_2_FormationTask* pThis)
{
    allocateNPC(pThis, 6);
    allocateNPC(pThis, 8);

    pThis->mE0_entityGroup.mC_formationNameIndex = 0x23;
    pThis->mF4_entityGroup2.mC_formationNameIndex = 0x21;

    gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m2 = 0x5A;
    battleEngine_UpdateSub1Sub0(0);

    s16 nameIndex = pThis->mE0_entityGroup.mC_formationNameIndex;
    sSaturnPtr nameTableEntry = gCommonFile->getSaturnPtr(0x0020179c) + nameIndex * 8;
    displayFormationName(0, readSaturnS8(nameTableEntry + 3), readSaturnS8(nameTableEntry + 2));

    pThis->mE0_entityGroup.m8_dataTable = g_BTL_A7_2->getSaturnPtr(0x060a4628);
    pThis->mF4_entityGroup2.m8_dataTable = g_BTL_A7_2->getSaturnPtr(0x060a4658);

    pThis->mE0_entityGroup.m4_positionBlock = &pThis->m2C_posBlock;
    pThis->mF4_entityGroup2.m4_positionBlock = &pThis->m108_posBlock2;

    pThis->mE0_entityGroup.mE_numEntities = 1;
    pThis->mE0_entityGroup.mF_deadCount = 0;
    pThis->mF4_entityGroup2.mE_numEntities = 2;
    pThis->mF4_entityGroup2.mF_deadCount = 0;

    pThis->mE0_entityGroup.m0_entityArray = (p_workArea*)allocateHeapForTask(pThis, pThis->mE0_entityGroup.mE_numEntities * sizeof(p_workArea));
    pThis->mF4_entityGroup2.m0_entityArray = (p_workArea*)allocateHeapForTask(pThis, pThis->mF4_entityGroup2.mE_numEntities * sizeof(p_workArea));

    for (int i = 0; i < pThis->mE0_entityGroup.mE_numEntities; i++)
    {
        // 060558f0
        pThis->mE0_entityGroup.m0_entityArray[i] = createAtolmBodyEntity(pThis, &pThis->mE0_entityGroup, i);
        if (pThis->mE0_entityGroup.m0_entityArray[i] == nullptr)
            pThis->mE0_entityGroup.mF_deadCount++;
    }

    for (int i = 0; i < pThis->mF4_entityGroup2.mE_numEntities; i++)
    {
        // 0605704c
        pThis->mF4_entityGroup2.m0_entityArray[i] = createAtolmArmEntity(pThis, &pThis->mF4_entityGroup2, i, 0);
        if (pThis->mF4_entityGroup2.m0_entityArray[i] == nullptr)
            pThis->mF4_entityGroup2.mF_deadCount++;
    }

    pThis->mD8_updateCallback = &BTL_A7_2_handleEnemyTurn;
}

// 060545e0
void BTL_A7_2_createFormation(s_workAreaCopy* pParent, u32 arg0)
{
    static const sBTL_A7_2_FormationTask::TypedTaskDefinition def = {
        nullptr,
        &BTL_A7_2_formationTask_Update,
        nullptr,
        &BTL_A7_2_formationTask_Delete,
    };

    sBTL_A7_2_FormationTask* pTask = createSubTaskWithCopy<sBTL_A7_2_FormationTask>(pParent, &def);
    if (!pTask)
        return;

    BTL_A7_2_formationTask_Init(pTask);

    pTask->m2C_posBlock.m78_interpRate = 0xA3D;
    pTask->m184_updateState = 0;
    pTask->m28_state = 0;
    pTask->m185_stateStep = 0;

    battleEngine_FlagQuadrantBitForDanger(0);
    battleEngine_FlagQuadrantBitForDanger(1);
    battleEngine_FlagQuadrantBitForSafety(0);
    battleEngine_FlagQuadrantBitForSafety(0xE);
}
