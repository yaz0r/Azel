#include "PDS.h"
#include "BTL_A3.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "battle/battleMainTask.h"

void battle_A3_initMusic(p_workArea pThis)
{
    if ((getBattleManager()->m6_subBattleId == 8) || (getBattleManager()->m6_subBattleId == 9))
    {
        playMusic(6, 0);
        getBattleManager()->m10_battleOverlay->m3 = 1;
    }
    else
    {
        playMusic(4, 0);
        getBattleManager()->m10_battleOverlay->m3 = 1;
    }
    playPCM(pThis, 100);
}

void battle_A3_func0(p_workArea)
{
    FunctionUnimplemented();
}

p_workArea overlayStart_BTL_A3(p_workArea parent)
{
    return createBattleMainTask(parent, &battle_A3_initMusic, battle_A3_func0);
}
