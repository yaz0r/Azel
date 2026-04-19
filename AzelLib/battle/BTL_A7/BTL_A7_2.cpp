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

struct sAtolmBossTask : public s_workAreaTemplateWithArgWithCopy<sAtolmBossTask, sSaturnPtr>
{
    // First 0x490 bytes are s_battleEngine
    // Boss-specific data follows
    u8 m_data[0xAA8 - 0x490];
    // size 0xAA8
};

// 06058acc
static void atolmBossTask_Init(sAtolmBossTask* pThis, sSaturnPtr battleData)
{
    Unimplemented();
}

// 060590a0
static void atolmBossTask_Update(sAtolmBossTask* pThis)
{
    Unimplemented();
}

// 060591dc
static void atolmBossTask_Draw(sAtolmBossTask* pThis)
{
    Unimplemented();
}

// 060591fe
static void atolmBossTask_Delete(sAtolmBossTask* pThis)
{
    Unimplemented();
}

// 06058abc
static void createAtolmBossTask(p_workArea parent, sSaturnPtr battleData)
{
    static const sAtolmBossTask::TypedTaskDefinition def = {
        &atolmBossTask_Init,
        &atolmBossTask_Update,
        &atolmBossTask_Draw,
        &atolmBossTask_Delete,
    };
    createSubTaskWithArgWithCopy<sAtolmBossTask>((s_workAreaCopy*)parent, battleData, &def);
}

// 06054034
static void battle_A7_2_func0(p_workArea pThis)
{
    initMemoryForBattle(pThis, assetList);

    sSaturnPtr BTL_A7_2_Data = g_BTL_A7_2->getSaturnPtr(0x060a51bc);
    createAtolmBossTask(pThis, BTL_A7_2_Data);
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
