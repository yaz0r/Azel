#include "PDS.h"
#include "BTL_A5.h"
#include "BTL_A5_data.h"
#include "battle/BTL_A3/BTL_A3.h"
#include "battle/battleManager.h"
#include "battle/battleMainTask.h"
#include "battle/battleEngine.h"
#include "kernel/graphicalObject.h"
#include "kernel/debug/trace.h"
#include "audio/soundDriver.h"

// BTL_A5::0605400a
void battle_A5_initMusic(p_workArea pThis)
{
    loadSoundBanks(8, 0);
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

    "A5CMN.MCB",
    "A5CMN.CGB",

    (char*)-1,
    "SCRBTLA5.CGB",

    "OKAFUGU.MCB",
    "OKAFUGU.CGB",

    "SNB.MCB",
    "SNB.CGB",

    "MUR.MCB",
    "MUR.CGB",

    "SABAKU02.MCB",
    "SABAKU02.CGB",

    "SABAKU04.MCB",
    "SABAKU04.CGB",

    "SABAKU_B.MCB",
    "SABAKU_B.CGB",

    "SHOKU.MCB",
    "SHOKU.CGB",

    "PGORAIAS.MCB",
    "PGORAIAS.CGB",

    nullptr,
    nullptr,
};

// BTL_A5::06054034
void battle_A5_func0(p_workArea pThis)
{
    initMemoryForBattle(pThis, assetList);
    allocateNPC(pThis, 6);

    sSaturnPtr BTL_A5_Data = g_BTL_A5->getSaturnPtr(0x60ac9f8);

    createBattleEngineTask(pThis, BTL_A5_Data);
}

// BTL_A5::06054000
p_workArea overlayStart_BTL_A5(p_workArea parent)
{
    if (g_BTL_A5 == NULL)
    {
        g_BTL_A5 = new BTL_A5_data();
    }

    gCurrentBattleOverlay = g_BTL_A5;
    startTrace("BTL_A5.trace.txt");
    return createBattleMainTask(parent, &battle_A5_initMusic, battle_A5_func0);
}
