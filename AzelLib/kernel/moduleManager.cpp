#include "PDS.h"
#include "moduleManager.h"
#include "town/town.h"
#include "loadSavegameScreen.h"
#include "battle/battleManager.h"
#include "audio/soundDriver.h"

u8 array_24BCA0[0x104];
u8 array_250000[0x20000];

s_moduleManager* gModuleManager = nullptr;

s_gameStatus gGameStatus;

s32 saveVarGameMode;
s32 saveVarFieldIndex;
s32 saveVarSubFieldIndex;
s32 saveVarSavePointIndex;

p_workArea(*overlayDispatchTable[])(p_workArea, s32) = {
    NULL,
    loadTown,
    loadTown2,
    loadField,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    initLoadSavegameScreen,
    NULL,
};

void exitMenuTaskSub1TaskInitSub1()
{
    memset(array_24BCA0, 0, 0x104);
}

u32 getPanzerZweiPlayTime(u32 slot)
{
    return 0;
}

void initNewGameState()
{
    mainGameState.reset();
    mainGameState.setPackedBits(2, 3, 0);

    if (VDP2Regs_.m4_TVSTAT & 1)
    {
        mainGameState.setBit(5);
    }
    else
    {
        mainGameState.clearBit(5);
    }

    mainGameState.clearBit(6);
    mainGameState.setBit(0x2B * 8 + 2);
    mainGameState.setBit(0x2D * 8 + 5);

    mainGameState.gameStats.m18_statAxisScale = 400;

    updateDragonStatsFromLevel();

    mainGameState.gameStats.m10_currentHP = mainGameState.gameStats.maxHP;
    mainGameState.gameStats.m14_currentBP = mainGameState.gameStats.maxBP;

    strcpy(mainGameState.gameStats.mA5_dragonName, "Dragon");

    u32 zweiPlayTime = getPanzerZweiPlayTime(0);
    u32 zweiPlayTimeSlot1 = getPanzerZweiPlayTime(1);
    if (zweiPlayTimeSlot1 > zweiPlayTime)
    {
        zweiPlayTime = zweiPlayTimeSlot1;
    }

    if (zweiPlayTime)
    {
        assert(0);
    }
}

s32 setNextGameStatus(s32 r4)
{
    PDS_Log("Requesting new game state: %d\n", r4);
    //assert(gGameStatus.m8_nextGameStatus == 0);
    if (gGameStatus.m8_nextGameStatus == 0)
    {
        gGameStatus.m8_nextGameStatus = r4;
        gGameStatus.m2 = 0;
    }
    else
    {
        PDS_Log("[ERROR] Requesting new game state failed to change to: %d!\n", r4);
    }

    return 0;
}

void moduleManager_Init(s_moduleManager* pWorkArea, s32 menuID)
{
    //HACK: keep track of this as we need to clear the sub task pointer when the overlay returns
    gModuleManager = pWorkArea;

    pWorkArea->m8 = 0;
    pWorkArea->mC = 0;

    gGameStatus.m0_gameMode = -1;
    gGameStatus.m1 = -1;
    gGameStatus.m3 = 0;
    gGameStatus.m4_gameStatus = 0;
    gGameStatus.m6_previousGameStatus = 0;
    gGameStatus.m8_nextGameStatus = 0;

    initNewGameState();

    if (menuID == 3)
    {
        mainGameState.gameStats.m1_dragonLevel = DR_LEVEL_1_VALIANT_WING;
    }
    else
    {
        mainGameState.gameStats.m1_dragonLevel = DR_LEVEL_0_BASIC_WING;
    }

    mainGameState.gameStats.m2_rider1 = 1;
    mainGameState.gameStats.m3_rider2 = 0;

    saveVarGameMode = 0;
    saveVarFieldIndex = 0;
    saveVarSubFieldIndex = 0;
    saveVarSavePointIndex = 0;

    exitMenuTaskSub1TaskInitSub1();

    createMenuTask(pWorkArea);
    createFieldTask(pWorkArea, 1);
    createBattleManager(pWorkArea, 1);
    resetTempAllocators();
    initDramAllocator(pWorkArea, array_250000, 0x28000, 0);

    loadDragon(pWorkArea);
    loadCurrentRider(pWorkArea);
    loadCurrentRider2(pWorkArea);
    freeRamResource();

    if (keyboardIsKeyDown(0xF6))
    {
        assert(0);
    }

    switch (menuID)
    {
    case 0: //new game
        setNextGameStatus(1);
        return;
    case 1: //continue
        setNextGameStatus(0x4A);
        return;
    case 2:
        setNextGameStatus(0x71);
        return;
    case 3:
        setNextGameStatus(0x72);
        return;
    default:
        assert(0);
    }

    assert(0);
}

void moduleManager_Update(s_moduleManager*)
{
    mainGameState.gameStats.frameCounter += vblankData.mC;
}

s32 exitMenuTaskSub1TaskDrawSub1(p_workArea pWorkArea, s32 index)
{
    p_workArea var_8 = pWorkArea;
    s32 var_C = index;
    s_gameStats* var_10 = &mainGameState.gameStats;
    s32 var_14 = mainGameState.gameStats.m1_dragonLevel;
    s32 var_18 = mainGameState.gameStats.m2_rider1;
    s32 r15 = mainGameState.gameStats.m3_rider2;

    PDS_Log("Evaluate new game status for %d\n", index);

    // 7: captain scene FMV

    switch (index)
    {
    case 0:
        break;
    case 1: // intro video
        mainGameState.setBit(4, 0);
        setNextGameStatus(2);
        break;
    case 2: // name entry
        setNextGameStatus(3);
        break;
    case 3: // resurrection video
        mainGameState.setBit(4, 1);
        setNextGameStatus(4);
        break;
    case 4: // ruins
        setNextGameStatus(5);
        break;
    case 5: // elevator video
        mainGameState.setBit(4, 2);
        setNextGameStatus(0x50);
        break;
    case 6: // initiate captain scene
        mainGameState.setBit(4, 3);
        mainGameState.setBit(10, 6);
        setNextGameStatus(0x51);
        break;
    case 7: // after captain scene
        mainGameState.setBit(4, 4);
        mainGameState.setBit(24, 1);
        mainGameState.setBit(12, 6);
        mainGameState.setBit(12, 7);
        mainGameState.setBit(13, 1);
        setNextGameStatus(0x53);
        break;
    case 8: // after cutscene meeting Gash
        mainGameState.setBit(4, 5);
        setNextGameStatus(9);
        break;
    case 9:
        mainGameState.setBit(10, 7);
        setNextGameStatus(10);
        break;
    case 10:
        mainGameState.setBit(4, 6);
        mainGameState.setBit(13, 2);
        mainGameState.clearBit(13, 0);
        setNextGameStatus(0x4F);
        break;
    case 0x4A: // load savegame?
        if (savegameVar0 == nullptr)
        {
            return -1;
        }
        break;
    case 0x50: // above excavation
    case 0x51: // captain scene intro (gameplay)
    case 0x52:
    case 0x53: // excavation site #3 (tutorial)
        setNextGameStatus(0x4F);
        break;

    case 0x4F:
        break;
    default:
        assert(0);
    }

    // has dragon level changed?
    if (mainGameState.gameStats.m1_dragonLevel != var_14)
    {
        assert(0);
    }

    // has rider1 changed
    if (mainGameState.gameStats.m2_rider1 != var_18)
    {
        assert(0);
    }

    // has rider2 changed
    if (mainGameState.gameStats.m3_rider2 != r15)
    {
        assert(0);
    }

    return 0;
}

void moduleManager_Draw(s_moduleManager* pWorkArea)
{
    switch (pWorkArea->state)
    {
    case 0:
        if (keyboardIsKeyDown(0xE7) || keyboardIsKeyDown(0xE4))
        {
            assert(0);
        }

        // 602739A

        if (gGameStatus.m8_nextGameStatus)
        {
            if (gGameStatus.m2 == 0)
            {
                if (pWorkArea->m8)
                {
                    //60273AA
                    if (!pWorkArea->m8->getTask()->isFinished())
                    {
                        pWorkArea->m8->getTask()->markFinished();
                    }
                }
            }
            else
            {
                //60273F4
                assert(0);
            }
        }
        else
        {
            //602745C
            if (pWorkArea->m8)
            {
                if (!pWorkArea->m8->getTask()->isFinished())
                {
                    return;
                }
            }
        }

        // 06027474
        graphicEngineStatus.m4 = 1;
        pauseEngine[2] = 0;
        graphicEngineStatus.m40AC.m1_isMenuAllowed = 0;
        gGameStatus.m0_gameMode = -1;
        pWorkArea->state++;
        break;
    case 1:
        if ((gGameStatus.m8_nextGameStatus == 0) && pWorkArea->mC)
        {
            assert(0);
        }
        if (*(COMMON_DAT + 0x12EAC + gGameStatus.m4_gameStatus * 2) == 4)
        {
            assert(0);
        }
        else
        {
            if (gGameStatus.m2 == 0)
            {
                if (exitMenuTaskSub1TaskDrawSub1(pWorkArea, gGameStatus.m4_gameStatus) < 0)
                {
                    if (pWorkArea)
                    {
                        pWorkArea->getTask()->markFinished();
                    }
                    return;
                }
            }
        }
        //06027574
        switch (gGameStatus.m8_nextGameStatus)
        {
        default:
            if ((gGameStatus.m2 == 0) && (gGameStatus.m6_previousGameStatus != 0x4F))
            {
                initFileLayoutTable();
            }
        case 8:
        case 0x19:
            fadeOutAllSequences();
        case 0xC:
        case 0x16:
        case 0x25:
            fadePalette(&g_fadeControls.m0_fade0, convertColorToU32(g_fadeControls.m0_fade0.m0_color), 0, 30);
            fadePalette(&g_fadeControls.m24_fade1, convertColorToU32(g_fadeControls.m24_fade1.m0_color), 0, 30);
            break;
        case 9:
        case 0xb:
        case 0xe:
        case 0x10:
        case 0x15:
        case 0x18:
        case 0x1a:
        case 0x1b:
        case 0x1f:
        case 0x22:
        case 0x23:
        case 0x26:
        case 0x28:
        case 0x2a:
        case 0x2d:
        case 0x34:
        case 0x35:
        case 0x38:
        case 0x39:
        case 0x3a:
        case 0x3b:
        case 0x3c:
        case 0x3d:
        case 0x3e:
        case 0x3f:
        case 0x40:
        case 0x41:
        case 0x42:
        case 0x43:
        case 0x44:
        case 0x66:
            break;
        case 0x70:
            initFileLayoutTable();
            fadePalette(&g_fadeControls.m0_fade0, convertColorToU32(g_fadeControls.m0_fade0.m0_color), 0, 30);
            fadePalette(&g_fadeControls.m24_fade1, convertColorToU32(g_fadeControls.m24_fade1.m0_color), 0, 30);
            break;
        }

        gGameStatus.m6_previousGameStatus = gGameStatus.m4_gameStatus;
        gGameStatus.m4_gameStatus = gGameStatus.m8_nextGameStatus;
        gGameStatus.m8_nextGameStatus = 0;

        PDS_Log("Switching to Game Satus %d\n", gGameStatus.m4_gameStatus);

        gGameStatus.m0_gameMode = readSaturnS8(gCommonFile.getSaturnPtr(0x212EAC + gGameStatus.m4_gameStatus * 2));
        gGameStatus.m1 = readSaturnS8(gCommonFile.getSaturnPtr(0x212EAC + gGameStatus.m4_gameStatus * 2) + 1);

        if (gGameStatus.m6_previousGameStatus == 74)
        {
            gGameStatus.m3 = 1;
        }
        else
        {
            gGameStatus.m3 = 0;
        }

        if (overlayDispatchTable[gGameStatus.m0_gameMode])
        {
            PDS_Log("Calling into overlay type %d", gGameStatus.m0_gameMode);
            pWorkArea->m8 = overlayDispatchTable[gGameStatus.m0_gameMode](pWorkArea, gGameStatus.m1);
        }
        else
        {
            PDS_Log("Unimplemented entry in overlayDispatchTable[%d]. Skipping!\n", gGameStatus.m0_gameMode);
        }
        pWorkArea->state = 0;

        break;
    default:
        assert(0);
    }
}

p_workArea createModuleManager(p_workArea pTypelessWorkArea, u32 menuID)
{
    static const s_moduleManager::TypedTaskDefinition taskDefinition = {
        &moduleManager_Init,
        &moduleManager_Update,
        &moduleManager_Draw,
        nullptr
    };

    return createSubTaskWithArg<s_moduleManager, s32>(pTypelessWorkArea, menuID, &taskDefinition);
}

void setupSaveParams(s32 fieldIndex, s32 subFieldIndex, s32 savepointIndex)
{
    graphicEngineStatus.m40AC.m0_menuId = 8; // trigger the save menu
    saveVarGameMode = mainGameState.readPackedBits(0x87, 6) + 0x50;
    saveVarFieldIndex = fieldIndex;
    saveVarSubFieldIndex = subFieldIndex;
    saveVarSavePointIndex = savepointIndex;
}
