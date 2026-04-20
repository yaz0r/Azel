#include "PDS.h"
#include "BTL_X0.h"
#include "BTL_X0_data.h"
#include "battle/BTL_A3/BTL_A3.h"
#include "battle/battleManager.h"
#include "battle/battleMainTask.h"
#include "battle/battleEngine.h"
#include "kernel/graphicalObject.h"
#include "kernel/debug/trace.h"
#include "audio/soundDriver.h"

// 0605400a
static void battle_X0_initMusic(p_workArea pThis)
{
    s16 subBattleId = gBattleManager->m6_subBattleId;
    if (subBattleId == 0)
    {
        loadSoundBanks(0x39, 0);
        gBattleManager->m10_battleOverlay->m3 = 1;
        playPCM(pThis, 100);
    }
    else if (subBattleId == 1)
    {
        loadSoundBanks(0x39, 0);
        if (findSound(100) < 0)
        {
            gBattleManager->m10_battleOverlay->m3 = 1;
            playPCM(pThis, 100);
        }
    }
    else if (subBattleId == 2)
    {
        loadSoundBanks(0x39, 0);
    }
}

static const char* assetList[] = {
    "BATTLE.MCB",
    "BATTLE.CGB",

    "TUP.MCB",
    "TUP.CGB",

    "ENCAM.BDB",
    (char*)-1,

    "AD.MCB",
    "AD.CGB",

    "AD3.MCB",
    "AD3.CGB",

    (char*)-1,
    "AD_EFF.CGB",

    nullptr,
    nullptr,
};

// 06054082
static void battle_X0_func0(p_workArea pThis)
{
    initMemoryForBattle(pThis, assetList);

    sSaturnPtr BTL_X0_Data = g_BTL_X0->getSaturnPtr(0x060b286c);
    createBattleEngineTask(pThis, BTL_X0_Data);
}

// 06054000
p_workArea overlayStart_BTL_X0(p_workArea parent)
{
    if (g_BTL_X0 == NULL)
    {
        g_BTL_X0 = new BTL_X0_data();
    }

    gCurrentBattleOverlay = g_BTL_X0;
    startTrace("BTL_X0.trace.txt");
    return createBattleMainTask(parent, &battle_X0_initMusic, battle_X0_func0);
}
