#include "PDS.h"
#include "BTL_A5_3.h"
#include "BTL_A5_3_data.h"
#include "battle/BTL_A3/BTL_A3.h"
#include "battle/battleManager.h"
#include "battle/battleMainTask.h"
#include "battle/battleEngine.h"
#include "kernel/graphicalObject.h"
#include "kernel/debug/trace.h"
#include "audio/soundDriver.h"

// BTL_A5_3::0605400a
static void battle_A5_3_initMusic(p_workArea pThis)
{
    loadSoundBanks(9, 0);
    gBattleManager->m10_battleOverlay->m3 = 1;
    playPCM(pThis, 100);
}

static const char* assetList[] = {
    "BATTLE.MCB",
    "BATTLE.CGB",

    "TUP.MCB",
    "TUP.CGB",

    "ENCAM.BDB",
    (char*)-1,

    (char*)-1,
    "SCRBTLA5.CGB",

    "ZUSU.MCB",
    "ZUSU.CGB",

    nullptr,
    nullptr,
};

// BTL_A5_3::06054034
static void battle_A5_3_func0(p_workArea pThis)
{
    initMemoryForBattle(pThis, assetList);

    sSaturnPtr BTL_A5_3_Data = g_BTL_A5_3->getSaturnPtr(0x60a3a98);

    createBattleEngineTask(pThis, BTL_A5_3_Data);
}

// BTL_A5_3::06054000
p_workArea overlayStart_BTL_A5_3(p_workArea parent)
{
    if (g_BTL_A5_3 == NULL)
    {
        g_BTL_A5_3 = new BTL_A5_3_data();
    }

    gCurrentBattleOverlay = g_BTL_A5_3;
    startTrace("BTL_A5_3.trace.txt");
    return createBattleMainTask(parent, &battle_A5_3_initMusic, battle_A5_3_func0);
}
