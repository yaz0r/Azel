#include "PDS.h"
#include "BTL_A3_2.h"
#include "battle/BTL_A3/BTL_A3.h"
#include "BTL_A3_2_data.h"
#include "town/town.h" // TODO: cleanup
#include "battle/battleEngine.h"
#include "battle/battleMainTask.h"
#include "kernel/debug/trace.h"

static const char* assetList[] = {
    "BATTLE.MCB",
    "BATTLE.CGB",

    "TUP.MCB",
    "TUP.CGB",

    "ENCAM.BDB",
    (char*)-1,

    "A3CMN.MCB",
    "A3CMN.CGB",

    "BEMOS.MCB",
    "BEMOS.CGB",

    "KEI_BOSS.MCB",
    "KEI_BOSS.CGB",

    nullptr,
    nullptr,
};


void battle_A3_2_func0(p_workArea pThis)
{
    initMemoryForBattle(pThis, assetList);
    allocateNPC(pThis, 6);

    sSaturnPtr BTL_A3_2_Data = g_BTL_A3_2->getSaturnPtr(0x60a84bc);

    createBattleEngineTask(pThis, BTL_A3_2_Data);
}

p_workArea overlayStart_BTL_A3_2(p_workArea parent)
{
    if (g_BTL_A3_2 == NULL)
    {
        g_BTL_A3_2 = new BTL_A3_2_data();
    }

    gCurrentBattleOverlay = g_BTL_A3_2;
    startTrace("BTL_A3_2.trace.txt");
    return createBattleMainTask(parent, &battle_A3_initMusic, battle_A3_2_func0);
}
