#include "PDS.h"
#include "battleEngineSub0.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"
#include "battleHud.h"

void BattleEngineSub0_Update(s_battleEngineSub0* pThis)
{
    int iVar2;
    sBattleManager* psVar1 = gBattleManager;
    if ((!psVar1->m10_battleOverlay->m4_battleEngine->m188_flags.m8_showingBattleResultScreen) &&
        (((iVar2 = BattleEngineSub0_UpdateSub0(), iVar2 == 0 ||
        (psVar1 = gBattleManager,
            (psVar1->m10_battleOverlay->m4_battleEngine->m188_flags.m1000000_dragonMoving) != 0)) &&
            (pThis->m8 == 0))))
    {
        if (pThis->m0 < 0)
        {
            pThis->m0 = 0;
        }
        if (pThis->m2 != 0)
        {
            if ((gBattleManager->m10_battleOverlay->m4_battleEngine->m388 & 0x400) == 0)
            {
                pThis->m0 += 1;
            }
            if (pThis->m2 <= pThis->m0)
            {
                pThis->m0 = pThis->m2;
                pThis->m8 = 1;
                pThis->m4++;
            }
        }
    }
}

void BattleEngineSub0_Draw(s_battleEngineSub0* pThis)
{
    if (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m8_showingBattleResultScreen)
    {
        if (gBattleManager->m10_battleOverlay->m20_battleHud->m0 == 1)
        {
            assert(0);
        }
    }
}

s_battleEngineSub0* createBattleEngineSub0(p_workArea parent)
{
    static const s_battleEngineSub0::TypedTaskDefinition definition = {
        nullptr,
        &BattleEngineSub0_Update,
        &BattleEngineSub0_Draw,
        nullptr
    };
    s_battleEngineSub0* pNewTask = createSubTask<s_battleEngineSub0>(parent, &definition);

    pNewTask->m0 = 0;
    pNewTask->m2 = 0;
    pNewTask->m4 = 0;
    pNewTask->m8 = 0;

    return pNewTask;
}
