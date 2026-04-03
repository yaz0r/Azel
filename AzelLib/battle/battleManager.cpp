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
    pThis->m4 = -1;
    pThis->m6_subBattleId = 0;
    pThis->m8 = 0;
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
        pThis->mA_pendingBattleOverlayId = pThis->m4;
        pThis->m6_subBattleId = pThis->m8;
        pThis->m2_currentBattleOverlayId = -1;

        terminateCurrentBattle(pThis->m10_subTask_debugList);
        pThis->m10_subTask_debugList = startBattlePrgTask(pThis);
    }
}

void loadBattleOverlaySub0(sBattleManager* pThis)
{
    pThis->m4 = pThis->m2_currentBattleOverlayId;
    pThis->m8 = pThis->m6_subBattleId;

    int uVar1;
    int iVar2 = pThis->m6_subBattleId;
    int uVar4 = pThis->m2_currentBattleOverlayId;
    int sVar3 = pThis->m6_subBattleId;

    switch (pThis->m4)
    {
    case 0:
        if (iVar2 == 10) {
            uVar4 = 0x10;
            sVar3 = 0;
        }
        else {
            if (iVar2 != 0xb) {
                if (iVar2 == 0xc) {
                    uVar4 = 0xf;
                    sVar3 = 1;
                    break;
                }
                if (iVar2 == 0xd) {
                    uVar4 = 0xf;
                    sVar3 = 2;
                    break;
                }
                uVar1 = 0xf;
                if (iVar2 == 0xe) {
                    uVar4 = uVar1;
                    sVar3 = 3;
                    break;
                }
                if (iVar2 != 0xf) {
                    if (iVar2 == 0x10) {
                        uVar4 = 0xf;
                        sVar3 = 5;
                    }
                    break;
                }
                uVar4 = 0xf;
                sVar3 = 4;
                break;
            }
            uVar4 = 0xf;
            sVar3 = 0;
        }
        break;
    default:
        assert(0);
    }

    pThis->m2_currentBattleOverlayId = uVar4;
    pThis->m6_subBattleId = sVar3;
}

static int loadBattleOverlay_debug(sBattleManager* pThis)
{
    vdp2DebugPrintSetPosition(3, 0x18);
    vdp2PrintStatus.m10_palette = 0xd000;
    drawLineLargeFont("LOADING...");
    pThis->m2_currentBattleOverlayId = pThis->mA_pendingBattleOverlayId;
    pThis->mA_pendingBattleOverlayId = -1;
    loadBattleOverlaySub0(pThis);

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
