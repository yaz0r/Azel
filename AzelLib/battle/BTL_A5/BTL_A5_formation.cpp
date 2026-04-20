#include "PDS.h"
#include "BTL_A5_formation.h"
#include "BTL_A5_data.h"
#include "BTL_A5_enemy.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "battle/battleEngine.h"
#include "battle/battleEngineSub0.h"
#include "battle/battleTextDisplay.h"
#include "kernel/graphicalObject.h"
#include "town/town.h"

void battleEngine_UpdateSub1Sub0(s32 param_1);

// 0605c9dc
static void BTL_A5_initFormation_20(sBTL_A5_FormationTask* pThis)
{
    allocateNPC(pThis, 2);
    pThis->mE0_entityGroup.mC_formationNameIndex = 0xF;
    gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m2 = 0x78;
    battleEngine_UpdateSub1Sub0(0);
    displayFormationName(0, 1, 9);

    pThis->mE0_entityGroup.m8_dataTable = g_BTL_A5->getSaturnPtr(0x060b0d24);
    pThis->m1CC_dataTable2 = g_BTL_A5->getSaturnPtr(0x060b0f04);

    pThis->mF0_flag = 0;
    pThis->m1D4_flag = 0;
    pThis->mE0_entityGroup.m4_positionBlock = &pThis->m2C_posBlock;
    pThis->mE0_entityGroup.mE_numEntities = 0x14;
    pThis->mE0_entityGroup.mF_deadCount = 0;
    pThis->mE0_entityGroup.m0_entityArray = (p_workArea*)allocateHeapForTask(pThis, pThis->mE0_entityGroup.mE_numEntities * sizeof(p_workArea));

    s8 quadrant = gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant;
    s32 leadIdx;
    if (quadrant == 0) leadIdx = 5;
    else if (quadrant == 1) leadIdx = 7;
    else if (quadrant == 3) leadIdx = 6;
    else leadIdx = 5;

    for (int i = 0; i < pThis->mE0_entityGroup.mE_numEntities; i++)
    {
        pThis->mE0_entityGroup.m0_entityArray[i] = BTL_A5_createEnemy(pThis, 0x060b0d24, i, (leadIdx == i) ? 1 : 0);
        if (pThis->mE0_entityGroup.m0_entityArray[i] == nullptr)
        {
            pThis->mE0_entityGroup.mF_deadCount++;
        }
    }
}

// 0605cb54
static void BTL_A5_initFormation_10(sBTL_A5_FormationTask* pThis)
{
    allocateNPC(pThis, 2);
    pThis->mE0_entityGroup.mC_formationNameIndex = 0xF;
    gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m2 = 0x78;
    battleEngine_UpdateSub1Sub0(0);
    displayFormationName(0, 1, 9);

    pThis->mE0_entityGroup.m8_dataTable = g_BTL_A5->getSaturnPtr(0x060b0f18);
    pThis->m1CC_dataTable2 = g_BTL_A5->getSaturnPtr(0x060b0f04);

    pThis->mF0_flag = 0;
    pThis->m1D4_flag = 1;
    pThis->mE0_entityGroup.m4_positionBlock = &pThis->m2C_posBlock;
    pThis->mE0_entityGroup.mE_numEntities = 10;
    pThis->mE0_entityGroup.mF_deadCount = 0;
    pThis->mE0_entityGroup.m0_entityArray = (p_workArea*)allocateHeapForTask(pThis, pThis->mE0_entityGroup.mE_numEntities * sizeof(p_workArea));

    for (int i = 0; i < pThis->mE0_entityGroup.mE_numEntities; i++)
    {
        pThis->mE0_entityGroup.m0_entityArray[i] = BTL_A5_createEnemy(pThis, 0x060b0f18, i, 1);
        if (pThis->mE0_entityGroup.m0_entityArray[i] == nullptr)
        {
            pThis->mE0_entityGroup.mF_deadCount++;
        }
    }
}

// 0605cd10
static void BTL_A5_formationTask_Update(sBTL_A5_FormationTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    pEngine->m3B2_numBattleFormationRunning++;

    // clean up dead entity pointers
    for (int i = 0; i < pThis->mE0_entityGroup.mE_numEntities; i++)
    {
        p_workArea entity = pThis->mE0_entityGroup.m0_entityArray[i];
        if (entity == nullptr) continue;
        if (entity->getTask()->m14_flags & TASK_FLAGS_FINISHED)
        {
            pThis->mE0_entityGroup.m0_entityArray[i] = nullptr;
        }
    }

    // check if all enemies dead
    if (pThis->mE0_entityGroup.mF_deadCount >= pThis->mE0_entityGroup.mE_numEntities)
    {
        pThis->m28_state = 0xB;
        pEngine->m3CC->m8 = 0;
        pEngine->m3CC->m0 = 0;
        pEngine->m3CC->m2 = 0;
        pEngine->m3CC->m4 = 0;
        pThis->m1D4_flag = 1;
    }

    // state machine
    switch (pThis->m28_state)
    {
    case 0:
        if (battleEngine_isBattleIntroFinished())
        {
            battleEngine_SetBattleMode(eBattleModes::m7);
            pThis->m28_state = 1;
        }
        break;
    case 1:
        if (pEngine->m188_flags.m2000)
        {
            pThis->m28_state = 2;
        }
        break;
    case 2:
    {
        s16 counter = pThis->m24_counter1++;
        if (counter > 0x54)
        {
            pThis->m24_counter1 = 0;
            pEngine->m188_flags.m100_attackAnimationFinished = 1;
            pThis->m28_state = 4;
        }
        break;
    }
    case 3:
        if (battleEngine_isPlayerTurnActive())
            return;
        break;
    case 4:
        if (!pEngine->m188_flags.m400000)
        {
            pThis->m28_state = 3;
            pEngine->m3CC->m8 = 0;
            pEngine->m3CC->m0 = 0;
        }
        break;
    case 0xB:
    {
        s16 counter = pThis->m26_counter2++;
        if (counter > 0x4B)
        {
            pThis->m26_counter2 = 0;
            decreaseNPCRefCount(2);
            pThis->getTask()->markFinished();
        }
        break;
    }
    default:
        pThis->m28_state = 0xB;
        break;
    }
}

// 0605cc8c
void BTL_A5_createFormation(s_workAreaCopy* pParent, u32 arg0)
{
    static const sBTL_A5_FormationTask::TypedTaskDefinition def = {
        nullptr,
        &BTL_A5_formationTask_Update,
        nullptr,
        nullptr,
    };

    sBTL_A5_FormationTask* pTask = createSubTaskWithCopy<sBTL_A5_FormationTask>(pParent, &def);
    if (!pTask)
        return;

    s16 subBattleId = gBattleManager->m6_subBattleId;
    if (subBattleId == 0 || subBattleId == 1)
    {
        pTask->m28_state = 0;
        BTL_A5_initFormation_20(pTask);
    }
    else if (subBattleId == 2)
    {
        pTask->m28_state = 3;
        BTL_A5_initFormation_20(pTask);
    }
    else if (subBattleId == 3)
    {
        pTask->m28_state = 3;
        BTL_A5_initFormation_10(pTask);
    }
    else
    {
        pTask->m28_state = 0;
        BTL_A5_initFormation_20(pTask);
    }

    battleEngine_FlagQuadrantBitForSafety(4);
    battleEngine_FlagQuadrantBitForDanger(1);
    pTask->m2C_posBlock.m78_interpRate = 0xA3D;
}
