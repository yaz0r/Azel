#include "PDS.h"
#include "battleOverlay_20.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "kernel/vdp1Allocator.h"
#include "town/town.h" // todo: for npcFileDeleter
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"

void s_battleOverlay_20_update(s_battleOverlay_20* pThis)
{
    pThis->m2C++;
    switch (pThis->m10)
    {
    case 0:
        if (BattleEngineSub0_UpdateSub0())
        {
            if (getBattleManager()->m10_battleOverlay->m4_battleEngine->m38C == 2)
                return;
            if (getBattleManager()->m10_battleOverlay->m4_battleEngine->m38C == 0xE)
                return;
        }
        if (getBattleManager()->m10_battleOverlay->m4_battleEngine->m188_flags & 0x80000)
        {
            assert(0);
        }
        break;
    default:
        assert(0);
        break;
    }
}

void s_battleOverlay_20_draw(s_battleOverlay_20*)
{
    if (getBattleManager()->m10_battleOverlay->m4_battleEngine->m188_flags & 8)
    {
        assert(0);
    }
}

void s_battleOverlay_20_delete(s_battleOverlay_20*)
{
    FunctionUnimplemented();
}

void createBattleHPOrBpDisplayTask(npcFileDeleter* parent, u16*, u16*, u16*, s32)
{
    FunctionUnimplemented();
}

void createPGTask(npcFileDeleter* parent, u16*, s32)
{
    FunctionUnimplemented();
}

void createBattleOverlay_task20(npcFileDeleter* parent)
{
    static const s_battleOverlay_20::TypedTaskDefinition definition = {
        nullptr,
        &s_battleOverlay_20_update,
        &s_battleOverlay_20_draw,
        &s_battleOverlay_20_delete,
    };

    s_battleOverlay_20* pNewTask = createSubTask<s_battleOverlay_20>(parent, &definition);

    getBattleManager()->m10_battleOverlay->m20 = pNewTask;

    pNewTask->m14_vdp1Memory = parent->m4_vd1Allocation->m4_vdp1Memory;
    pNewTask->m16 = 0;
    pNewTask->m18 = 0x47;
    pNewTask->m1A = 0;
    pNewTask->m1C = 0x47;
    pNewTask->m1E = 0x31;
    pNewTask->m20 = 0xb3;
    pNewTask->m22 = 0x66;
    pNewTask->m24 = 0xb3;

    createBattleHPOrBpDisplayTask(parent, &mainGameState.gameStats.currentHP, &mainGameState.gameStats.maxHP, &pNewTask->m1E, 1);
    createBattleHPOrBpDisplayTask(parent, &mainGameState.gameStats.currentBP, &mainGameState.gameStats.maxBP, &pNewTask->m22, 0);
    createPGTask(parent, &pNewTask->m16, 0);

    pNewTask->m0 = 0;
    pNewTask->m28 = 0;
    pNewTask->m10 = 0;
}
