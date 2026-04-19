#include "PDS.h"
#include "battleManager.h"
#include "battle/battleDebugList.h"
#include "battle/battleOverlay.h"
#include "commonOverlay.h"

sBattleManager* gBattleManager = NULL;

void computeDragonSprAndAglFromCursor(); // from mainMenuDebugTasks.cpp
/*
sBattleManager* gBattleManager
{
    return gBattleManager;
}
*/
void battleManager_Init(sBattleManager* pThis, s8 arg)
{
    gBattleManager = pThis;

    pThis->m0_status = 0;
    pThis->m2_currentBattleOverlayId = -1;
    pThis->m4_previousBattleOverlayId = -1;
    pThis->m6_subBattleId = 0;
    pThis->m8_previousSubBattleId = 0;
    pThis->mA_pendingBattleOverlayId = 0;
    pThis->mC = arg;

    PDS_warningOnce("sLoadingTaskWorkArea::Init recheck, code seems to be duplicated, loop might be incorrect, weird test with exts.b")
        //TODO: is this correct?
        //if (arg)
    {
        s32 r6 = 1000;
        s32 r4 = r6;
        do
        {
            if (r4 >= r6)
            {
                mainGameState.clearBit(0xC3E + r4);
            }
            else
            {
                mainGameState.clearBit(r4);
            }
        } while (--r4);
    }
}

void loadingTaskUpdateSub0(sBattleManager* pThis)
{
    if (pThis->mC == 0)
    {
        pThis->mA_pendingBattleOverlayId = pThis->m4_previousBattleOverlayId;
        pThis->m6_subBattleId = pThis->m8_previousSubBattleId;
        pThis->m2_currentBattleOverlayId = -1;

        terminateCurrentBattle(pThis->m10_subTask_debugList);
        pThis->m10_subTask_debugList = startBattlePrgTask(pThis);
    }
}

void remapBattleOverlayId(sBattleManager* pThis)
{
    pThis->m4_previousBattleOverlayId = pThis->m2_currentBattleOverlayId;
    pThis->m8_previousSubBattleId = pThis->m6_subBattleId;

    switch (pThis->m4_previousBattleOverlayId)
    {
    case 0:
        switch (pThis->m6_subBattleId) {
        case 0xa:
            pThis->m2_currentBattleOverlayId = 0x10;
            pThis->m6_subBattleId = 0;
            break;
        case 0xb:
            pThis->m2_currentBattleOverlayId = 0xf;
            pThis->m6_subBattleId = 0;
            break;
        case 0xc:
            pThis->m2_currentBattleOverlayId = 0xf;
            pThis->m6_subBattleId = 1;
            break;
        case 0xd:
            pThis->m2_currentBattleOverlayId = 0xf;
            pThis->m6_subBattleId = 2;
            break;
        case 0xe:
            pThis->m2_currentBattleOverlayId = 0xf;
            pThis->m6_subBattleId = 3;
            break;
        case 0xf:
            pThis->m2_currentBattleOverlayId = 0xf;
            pThis->m6_subBattleId = 4;
            break;
        case 0x10:
            pThis->m2_currentBattleOverlayId = 0xf;
            pThis->m6_subBattleId = 5;
            break;
        }
        break;
    case 1:
        switch (pThis->m6_subBattleId) {
        case 0x10:
            pThis->m2_currentBattleOverlayId = 0x12;
            pThis->m6_subBattleId = 0;
            break;
        case 0x11:
            pThis->m2_currentBattleOverlayId = 0x11;
            pThis->m6_subBattleId = 0;
            break;
        }
        break;
    case 2:
        switch (pThis->m6_subBattleId) {
        case 7:
            pThis->m2_currentBattleOverlayId = 0x13;
            pThis->m6_subBattleId = 0;
            break;
        case 8:
            pThis->m2_currentBattleOverlayId = 0xe;
            pThis->m6_subBattleId = 0;
            break;
        }
        break;
    case 5:
        switch (pThis->m6_subBattleId) {
        case 0xd:
            pThis->m2_currentBattleOverlayId = 0x15;
            pThis->m6_subBattleId = 0;
            break;
        case 0xe:
            pThis->m2_currentBattleOverlayId = 0x15;
            pThis->m6_subBattleId = 1;
            break;
        case 0xf:
            pThis->m2_currentBattleOverlayId = 0x15;
            pThis->m6_subBattleId = 2;
            break;
        case 0x10:
            pThis->m2_currentBattleOverlayId = 0x15;
            pThis->m6_subBattleId = 3;
            break;
        case 0x11:
            pThis->m2_currentBattleOverlayId = 0x15;
            pThis->m6_subBattleId = 4;
            break;
        case 0x12:
            pThis->m2_currentBattleOverlayId = 0x15;
            pThis->m6_subBattleId = 5;
            break;
        case 0x13:
            pThis->m2_currentBattleOverlayId = 0x15;
            pThis->m6_subBattleId = 6;
            break;
        case 0x14:
            pThis->m2_currentBattleOverlayId = 0x15;
            pThis->m6_subBattleId = 7;
            break;
        case 0x15:
            pThis->m2_currentBattleOverlayId = 0x14;
            pThis->m6_subBattleId = 0;
            break;
        case 0x16:
            pThis->m2_currentBattleOverlayId = 0xe;
            pThis->m6_subBattleId = 1;
            break;
        }
        break;
    case 6:
        if (pThis->m6_subBattleId ==0xe) {
            pThis->m2_currentBattleOverlayId = 0x16;
            pThis->m6_subBattleId = 0;
        }
        break;
    case 7:
        if (pThis->m6_subBattleId ==0xe) {
            pThis->m2_currentBattleOverlayId = 0x17;
            pThis->m6_subBattleId = 0;
        }
        break;
    case 8:
        if (pThis->m6_subBattleId ==0xe) {
            pThis->m2_currentBattleOverlayId = 0xe;
            pThis->m6_subBattleId = 2;
        }
        break;
    case 9:
        if (pThis->m6_subBattleId ==5) {
            pThis->m2_currentBattleOverlayId = 0x18;
            pThis->m6_subBattleId = 0;
        }
        break;
    case 10:
        if (pThis->m6_subBattleId ==0xf) {
            pThis->m2_currentBattleOverlayId = 0x19;
            pThis->m6_subBattleId = 0;
        }
        break;
    case 0xb:
        if (pThis->m6_subBattleId ==0xf) {
            pThis->m2_currentBattleOverlayId = 0x1a;
            pThis->m6_subBattleId = 0;
        }
        break;
    }
}

static int loadBattleOverlay_debug(sBattleManager* pThis)
{
    vdp2DebugPrintSetPosition(3, 0x18);
    vdp2PrintStatus.m10_palette = 0xd000;
    drawLineLargeFont("LOADING...");
    pThis->m2_currentBattleOverlayId = pThis->mA_pendingBattleOverlayId;
    pThis->mA_pendingBattleOverlayId = -1;
    remapBattleOverlayId(pThis);

    if (gCommonFile->battleOverlaySetup[pThis->m2_currentBattleOverlayId].m4_prg.length() == 0)
    {
        return 0;
    }
    terminateCurrentBattle(pThis->m10_subTask_debugList);
    createBattleOverlayTask(pThis);
    return 1;
}

void battleManager_battleFinished(sBattleManager* pThis)
{
    terminateCurrentBattle(pThis->m10_battleOverlay);
    graphicEngineStatus.m40AC.m1_isMenuAllowed = 0;
    pauseEngine[2] = 0;
}

static void battleManager_Update(sBattleManager* pThis)
{
    switch (pThis->m0_status)
    {
    case 0:
        loadingTaskUpdateSub0(pThis);
        pThis->m0_status++;
        break;
    case 1: // ready state, waiting for battle
        break;
    case 2: // start battle
        if (loadBattleOverlay_debug(pThis) != 0)
            return;
        pThis->m0_status = 1;
        break;
    case 3: // battle running
        break;
    case 4: // end of battle
        battleManager_battleFinished(pThis);
        pThis->m0_status = 1;
        break;
    case 5:
        pThis->m0_status = 0;
        break;
    default:
        assert(0);
    }
}

static void battleManager_Delete(sBattleManager*)
{
    PDS_unimplemented("sLoadingTaskWorkArea::Delete");
}

void startDebugBattle(s32 battleOverlayId)
{
    if (gBattleManager->m0_status == 1)
    {
        // Max out dragon stats for debug battles
        s_gameStats& gs = mainGameState.gameStats;
        gs.m0_level = 50;
        gs.m1_dragonLevel = DR_LEVEL_7_SOLO_WING;
        gs.m18_statAxisScale = 100;
        gs.m12_classMaxHP = 600;
        gs.m16_classMaxBP = 200;
        gs.mB8_maxHP = gs.m12_classMaxHP + dragonPerLevelMaxHPBP[gs.m1_dragonLevel].maxHP;
        gs.mBA_maxBP = gs.m16_classMaxBP + dragonPerLevelMaxHPBP[gs.m1_dragonLevel].maxBP;
        gs.m10_currentHP = gs.mB8_maxHP;
        gs.m14_currentBP = gs.mBA_maxBP;
        gs.mA_weaponType = (eItems)0xB; // bone slasher
        gs.mC_laserPower = 100;
        gs.mE_gunPower = 100;
        computeDragonSprAndAglFromCursor();

        // Give healing items
        mainGameState.consumables[5] = 99; // elixir minor
        mainGameState.consumables[6] = 99; // berserk micro

        // Unlock all berserks (items 0x91 through 0xB3)
        for (s16 i = 0x91; i <= 0xB3; i++)
        {
            mainGameState.setItemCount((eItems)i, 1);
        }

        gBattleManager->m0_status = 2;
        gBattleManager->mA_pendingBattleOverlayId = battleOverlayId;
    }
}

sBattleManager* createBattleManager(p_workArea parentTask, s8 arg)
{
    static const sBattleManager::TypedTaskDefinition definition = {
        &battleManager_Init,
        &battleManager_Update,
        nullptr,
        &battleManager_Delete,
    };
    return createSiblingTaskWithArg<sBattleManager, s8>(parentTask, arg, &definition);
}
