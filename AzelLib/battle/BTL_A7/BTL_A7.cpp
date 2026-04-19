#include "PDS.h"
#include "BTL_A7.h"
#include "BTL_A7_data.h"
#include "battle/BTL_A3/BTL_A3.h"
#include "battle/battleManager.h"
#include "battle/battleMainTask.h"
#include "battle/battleEngine.h"
#include "kernel/graphicalObject.h"
#include "kernel/debug/trace.h"
#include "audio/soundDriver.h"

// BTL_A7::0605400a
void battle_A7_initMusic(p_workArea pThis)
{
    s_fieldTaskWorkArea* pFieldTask = getFieldTaskPtr();
    if (pFieldTask != nullptr && pFieldTask != (s_fieldTaskWorkArea*)0x53454741)
    {
        if (pFieldTask->m2C_currentFieldIndex != 4)
        {
            loadSoundBanks(0x10, 0);
            return;
        }
    }
    loadSoundBanks(0xc, 0);
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
    "A7CMN.CGB",

    "BMP_A7_0.MCB",
    "BMP_A7_0.CGB",

    "CYCLONE.MCB",
    "CYCLONE.CGB",

    "OZL.MCB",
    "OZL.CGB",

    "SDON.MCB",
    "SDON.CGB",

    "STK01.MCB",
    "STK01.CGB",

    "STK02.MCB",
    "STK02.CGB",

    "STK03.MCB",
    "STK03.CGB",

    "STK12.MCB",
    "STK12.CGB",

    nullptr,
    nullptr,
};

// BTL_A7::06054068
void battle_A7_func0(p_workArea pThis)
{
    initMemoryForBattle(pThis, assetList);
    allocateNPC(pThis, 6);

    sSaturnPtr BTL_A7_Data = g_BTL_A7->getSaturnPtr(0x060a8414);

    createBattleEngineTask(pThis, BTL_A7_Data);
}

// BTL_A7::06054000
p_workArea overlayStart_BTL_A7(p_workArea parent)
{
    if (g_BTL_A7 == NULL)
    {
        g_BTL_A7 = new BTL_A7_data();
    }

    gCurrentBattleOverlay = g_BTL_A7;
    startTrace("BTL_A7.trace.txt");
    return createBattleMainTask(parent, &battle_A7_initMusic, battle_A7_func0);
}
