#include "PDS.h"
#include "BTL_A5_2.h"
#include "BTL_A5_2_data.h"
#include "battle/BTL_A3/BTL_A3.h"
#include "battle/battleManager.h"
#include "battle/battleMainTask.h"
#include "battle/battleEngine.h"
#include "kernel/graphicalObject.h"
#include "kernel/debug/trace.h"
#include "audio/soundDriver.h"

// BTL_A5_2::0605400a
static void battle_A5_2_initMusic(p_workArea pThis)
{
    loadSoundBanks(10, 0);
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

    "RAHAB.MCB",
    "RAHAB.CGB",

    "SABAKU_B.MCB",
    "SABAKU_B.CGB",

    "RAHAB.BDB",
    (char*)-1,

    nullptr,
    nullptr,
};

// BTL_A5_2::06054034
static void battle_A5_2_func0(p_workArea pThis)
{
    initMemoryForBattle(pThis, assetList);

    sSaturnPtr BTL_A5_2_Data = g_BTL_A5_2->getSaturnPtr(0x60aeec4);

    createBattleEngineTask(pThis, BTL_A5_2_Data);
}

// BTL_A5_2::06054000
p_workArea overlayStart_BTL_A5_2(p_workArea parent)
{
    if (g_BTL_A5_2 == NULL)
    {
        g_BTL_A5_2 = new BTL_A5_2_data();
    }

    gCurrentBattleOverlay = g_BTL_A5_2;
    startTrace("BTL_A5_2.trace.txt");
    return createBattleMainTask(parent, &battle_A5_2_initMusic, battle_A5_2_func0);
}
