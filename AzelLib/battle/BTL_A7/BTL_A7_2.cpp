#include "PDS.h"
#include "BTL_A7_2.h"
#include "BTL_A7_2_data.h"
#include "battle/BTL_A3/BTL_A3.h"
#include "battle/battleManager.h"
#include "battle/battleMainTask.h"
#include "battle/battleEngine.h"
#include "kernel/graphicalObject.h"
#include "kernel/debug/trace.h"
#include "audio/soundDriver.h"

// 0605400a
static void battle_A7_2_initMusic(p_workArea pThis)
{
    loadSoundBanks(0xd, 0);
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

    "AWEN.MCB",
    "AWEN.CGB",

    "KARAA72.MCB",
    "KARAA72.CGB",

    nullptr,
    nullptr,
};

// 06058acc = battleEngine_Init
// 060590a0 = battleEngine_Update
// 060591dc = battleEngine_Draw
// 060591fe = battleEngine_Delete
// 06058abc = createBattleEngineTask

// 06054034
static void battle_A7_2_func0(p_workArea pThis)
{
    initMemoryForBattle(pThis, assetList);

    sSaturnPtr BTL_A7_2_Data = g_BTL_A7_2->getSaturnPtr(0x060a51bc);
    createBattleEngineTask(pThis, BTL_A7_2_Data);
}

// 06054000
p_workArea overlayStart_BTL_A7_2(p_workArea parent)
{
    if (g_BTL_A7_2 == NULL)
    {
        g_BTL_A7_2 = new BTL_A7_2_data();
    }

    gCurrentBattleOverlay = g_BTL_A7_2;
    startTrace("BTL_A7_2.trace.txt");
    return createBattleMainTask(parent, &battle_A7_2_initMusic, battle_A7_2_func0);
}
