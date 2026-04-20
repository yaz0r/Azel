#include "PDS.h"
#include "BTL_A5_dual.h"
#include "BTL_A5_data.h"
#include "BTL_A5_enemy.h"
#include "BTL_A5_single.h"
#include "BTL_A5_formation.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "battle/battleEngine.h"
#include "battle/battleEngineSub0.h"
#include "battle/battleTextDisplay.h"
#include "kernel/graphicalObject.h"
#include "audio/systemSounds.h"
#include "town/town.h"

void battleEngine_UpdateSub1Sub0(s32 param_1);

struct sBTL_A5_DualFormation : public s_workAreaTemplateWithArgAndBase<sBTL_A5_DualFormation, sBTL_A5_FormationTask>
{
    sEntityGroup m1D0_entityGroup2;
    u8 m1E0_pad5[3];
    u8 m1E3_flag2;
    u8 m1E4_pad6[0x27C - 0x1E4];
    u8 m27C_variant;
    u8 m27D_pad7[3];
    s32 m280_phase;
    s32 m284_allSwarmDead;
    s32 m288_allSingleDead;
    // size 0x28C
};

// 0605f3a4
static void BTL_A5_dualFormation_Update(sBTL_A5_DualFormation* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    pEngine->m3B2_numBattleFormationRunning++;

    for (int i = 0; i < pThis->mE0_entityGroup.mE_numEntities; i++)
    {
        p_workArea e = pThis->mE0_entityGroup.m0_entityArray[i];
        if (e && e->getTask()->m14_flags & TASK_FLAGS_FINISHED)
            pThis->mE0_entityGroup.m0_entityArray[i] = nullptr;
    }
    for (int i = 0; i < pThis->m1D0_entityGroup2.mE_numEntities; i++)
    {
        p_workArea e = pThis->m1D0_entityGroup2.m0_entityArray[i];
        if (e && e->getTask()->m14_flags & TASK_FLAGS_FINISHED)
            pThis->m1D0_entityGroup2.m0_entityArray[i] = nullptr;
    }

    // check if swarm group all dead
    bool swarmAllDead = true;
    for (int i = 0; i < pThis->mE0_entityGroup.mE_numEntities; i++)
        if (pThis->mE0_entityGroup.m0_entityArray[i]) { swarmAllDead = false; break; }
    if (swarmAllDead && pThis->mE0_entityGroup.mF_deadCount >= pThis->mE0_entityGroup.mE_numEntities)
    {
        pThis->m284_allSwarmDead = 1;
        battleEngine_FlagQuadrantBitForSafety(0);
        battleEngine_FlagQuadrantBitForSafety(1);
        battleEngine_FlagQuadrantBitForDanger(0xE);
    }

    // check if single group all dead
    bool singleAllDead = true;
    for (int i = 0; i < pThis->m1D0_entityGroup2.mE_numEntities; i++)
        if (pThis->m1D0_entityGroup2.m0_entityArray[i]) { singleAllDead = false; break; }
    if (singleAllDead && pThis->m1D0_entityGroup2.mF_deadCount >= pThis->m1D0_entityGroup2.mE_numEntities)
    {
        pThis->m288_allSingleDead = 1;
        battleEngine_FlagQuadrantBitForSafety(0);
        battleEngine_FlagQuadrantBitForSafety(4);
        battleEngine_FlagQuadrantBitForDanger(1);
    }

    // both groups dead → battle complete
    if (pThis->m284_allSwarmDead == 1 && pThis->m288_allSingleDead == 1)
    {
        pThis->m28_state = 10;
        pEngine->m3CC->m8 = 0;
        pEngine->m3CC->m0 = 0;
        pEngine->m3CC->m2 = 0;
        pEngine->m3CC->m4 = 0;
    }

    // auto-scroll positions
    pThis->mA8_position.m0_X = pThis->mA8_position.m0_X + pEngine->m1A0_battleAutoScrollDelta.m0_X;
    pThis->mA8_position.m4_Y = pThis->mA8_position.m4_Y + pEngine->m1A0_battleAutoScrollDelta.m4_Y;
    pThis->mA8_position.m8_Z = pThis->mA8_position.m8_Z + pEngine->m1A0_battleAutoScrollDelta.m8_Z;

    switch (pThis->m28_state)
    {
    case 0:
        if (battleEngine_isPlayerTurnActive())
            return;
        break;
    case 1:
        if (!pEngine->m188_flags.m400000)
        {
            pThis->m28_state = 0;
            pEngine->m3CC->m8 = 0;
            pEngine->m3CC->m0 = 0;
        }
        break;
    case 10:
    {
        s16 c = pThis->m26_counter2++;
        if (c > 0x4B)
        {
            pThis->m26_counter2 = 0;
            decreaseNPCRefCount(2);
            decreaseNPCRefCount(0xC);
            pThis->getTask()->markFinished();
        }
        break;
    }
    default:
        pThis->m28_state = 10;
        break;
    }
}

// 0605f10c
static void BTL_A5_dualFormation_Init(sBTL_A5_DualFormation* pThis)
{
    u32 r = randomNumber();
    bool variant = (r & 1) != 7;
    pThis->m27C_variant = variant ? 1 : 0;

    allocateNPC(pThis, 2);
    allocateNPC(pThis, 0xC);

    gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m2 = 0x78;
    battleEngine_UpdateSub1Sub0(0);
    displayFormationName(0xD, 1, 9);

    pThis->mE0_entityGroup.m8_dataTable = g_BTL_A5->getSaturnPtr(0x060b11c8);
    pThis->m1CC_dataTable2 = g_BTL_A5->getSaturnPtr(0x060b13a8);

    // swarm group: 10 enemies
    pThis->mE0_entityGroup.mE_numEntities = 10;
    pThis->mE0_entityGroup.mF_deadCount = 0;
    pThis->mE0_entityGroup.m0_entityArray = (p_workArea*)allocateHeapForTask(pThis, pThis->mE0_entityGroup.mE_numEntities * sizeof(p_workArea));

    for (int i = 0; i < pThis->mE0_entityGroup.mE_numEntities; i++)
    {
        pThis->mE0_entityGroup.m0_entityArray[i] = BTL_A5_createEnemy(pThis, 0x060b11c8, i, 0);
        if (pThis->mE0_entityGroup.m0_entityArray[i] == nullptr)
            pThis->mE0_entityGroup.mF_deadCount++;
    }

    pThis->m284_allSwarmDead = 0;
    pThis->m280_phase = 0;

    // single group: 3 lead enemies
    pThis->m1D0_entityGroup2.mE_numEntities = 3;
    pThis->m1D0_entityGroup2.mF_deadCount = 0;
    pThis->m1D0_entityGroup2.m0_entityArray = (p_workArea*)allocateHeapForTask(pThis, pThis->m1D0_entityGroup2.mE_numEntities * sizeof(p_workArea));

    pThis->m1D0_entityGroup2.m8_dataTable = g_BTL_A5->getSaturnPtr(0x060b13b4 + (variant ? 0x24 : 0));
    pThis->m1E3_flag2 = 0;

    for (int i = 0; i < pThis->m1D0_entityGroup2.mE_numEntities; i++)
    {
        pThis->m1D0_entityGroup2.m0_entityArray[i] = BTL_A5_createEnemy(pThis, 0x060b13b4, i, 1);
        if (pThis->m1D0_entityGroup2.m0_entityArray[i] == nullptr)
            pThis->m1D0_entityGroup2.mF_deadCount++;
    }

    pThis->m288_allSingleDead = 0;
}

// 0605f31a
void BTL_A5_createFormation_dual(s_workAreaCopy* pParent, u32 arg0)
{
    static const sBTL_A5_DualFormation::TypedTaskDefinition def = {
        nullptr,
        &BTL_A5_dualFormation_Update,
        nullptr,
        nullptr,
    };

    sBTL_A5_DualFormation* pTask = createSubTaskWithCopy<sBTL_A5_DualFormation>(pParent, &def);
    if (!pTask) return;

    BTL_A5_dualFormation_Init(pTask);

    battleEngine_FlagQuadrantBitForSafety(5);
    pTask->m2C_posBlock.m78_interpRate = 0xA3D;
    pTask->m28_state = 0;
}
