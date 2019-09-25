#include "PDS.h"
#include "battleManager.h"
#include "battle/battleDebugList.h"
#include "battle/battleOverlay.h"

sBattleManager* gBattleManager = NULL;

sBattleManager* getBattleManager()
{
    return gBattleManager;
}

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

static void  loadBattleOverlaySub0(sBattleManager* pThis)
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
        }
        break;
    default:
        assert(0);
    }

    pThis->m2_currentBattleOverlayId = uVar4;
    pThis->m6_subBattleId = sVar3;
}

static int loadBattleOverlay(sBattleManager* pThis)
{
    vdp2DebugPrintSetPosition(3, 0x18);
    vdp2PrintStatus.m10_palette = 0xd000;
    drawLineLargeFont("LOADING...");
    pThis->m2_currentBattleOverlayId = pThis->mA_pendingBattleOverlayId;
    pThis->mA_pendingBattleOverlayId = -1;
    loadBattleOverlaySub0(pThis);

    sSaturnPtr battleOverlaySetup = gCommonFile.getSaturnPtr(0x2005dc);
    if (readSaturnU32(battleOverlaySetup + pThis->m2_currentBattleOverlayId * 0x14 + 4) == 0)
    {
        return 0;
    }
    terminateCurrentBattle(pThis->m10_subTask_debugList);
    createBattleOverlayTask(pThis);
    return 1;
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
        if (loadBattleOverlay(pThis) != 0)
            return;
        pThis->m0_status = 1;
        break;
    case 3: // battle running
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
