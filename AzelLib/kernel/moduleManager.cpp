#include "PDS.h"
#include "moduleManager.h"
#include "movie/movie.h"
#include "town/town.h"
#include "loadSavegameScreen.h"
#include "battle/battleManager.h"
#include "audio/soundDriver.h"
#include "commonOverlay.h"
#include "dragonData.h"
#include "dragonRider.h"
#include "worldmap/worldmap.h"

std::array<u8, 0x104> battleResults;
u8 array_250000[0x20000];

s_moduleManager* gModuleManager = nullptr;

s_gameStatus gGameStatus;

sSaveGameStatus gSaveGameStatus;

p_workArea(*overlayDispatchTable[])(p_workArea, s32) = {
    loadMovieOverlay,   // 0: movie
    loadTown,           // 1: town
    loadTown2,          // 2: town (alt entry)
    loadField,          // 3: field
    loadBattle,         // 4: battle
    loadWorldOverlay,   // 5: worldmap
    NULL,               // 6: unused
    NULL,               // 7: unused
    NULL,               // 8: unused
    initLoadSavegameScreen, // 9: save/load screen
    NULL,
};

void exitMenuTaskSub1TaskInitSub1()
{
    battleResults.fill(0);
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

    mainGameState.gameStats.m10_currentHP = mainGameState.gameStats.mB8_maxHP;
    mainGameState.gameStats.m14_currentBP = mainGameState.gameStats.mBA_maxBP;

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
    gGameStatus.m3_loadingSaveFile = 0;
    gGameStatus.m4_gameStatus = 0;
    gGameStatus.m6_previousGameStatus = 0;
    gGameStatus.m8_nextGameStatus = 0;

    initNewGameState();

    if (menuID == 3)
    {
        //Tutorial 2
        mainGameState.gameStats.m1_dragonLevel = DR_LEVEL_1_VALIANT_WING;
    }
    else
    {
        mainGameState.gameStats.m1_dragonLevel = DR_LEVEL_0_BASIC_WING;
    }

    mainGameState.gameStats.m2_rider1 = 1;
    mainGameState.gameStats.m3_rider2 = 0;

    gSaveGameStatus.m8_gameMode = 0;
    gSaveGameStatus.m9_fieldIndex = 0;
    gSaveGameStatus.mA_subFieldIndex = 0;
    gSaveGameStatus.mB_entryPointIndex = 0;
    
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
    case 2: // tutorial 1
        setNextGameStatus(0x71);
        return;
    case 3: // tutorial 2
        setNextGameStatus(0x72);
        return;
    default:
        assert(0);
    }

    assert(0);
}

void moduleManager_Update(s_moduleManager*)
{
    mainGameState.gameStats.m3C_frameCounter += vblankData.mC_numFramesPresented;
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
        mainGameState.setBit(4 * 8 + 0);
        setNextGameStatus(2);
        break;
    case 2: // name entry
        setNextGameStatus(3);
        break;
    case 3: // resurrection video
        mainGameState.setBit(4 * 8 + 1);
        setNextGameStatus(4);
        break;
    case 4: // ruins
        setNextGameStatus(5);
        break;
    case 5: // elevator video
        mainGameState.setBit(4 * 8 + 2);
        setNextGameStatus(0x50);
        break;
    case 6: // initiate captain scene
        mainGameState.setBit(4 * 8 + 3);
        mainGameState.setBit(10 * 8 + 6);
        setNextGameStatus(0x51);
        break;
    case 7: // after captain scene
        mainGameState.setBit(4 * 8 + 4);
        mainGameState.setBit(24 * 8 + 1);
        mainGameState.setBit(12 * 8 + 6);
        mainGameState.setBit(12 * 8 + 7);
        mainGameState.setBit(13 * 8 + 1);
        setNextGameStatus(0x53);
        break;
    case 8: // after cutscene meeting Gash
        mainGameState.setBit(4 * 8 + 5);
        setNextGameStatus(9);
        break;
    case 9:
        mainGameState.setBit(10 * 8 + 7);
        setNextGameStatus(10);
        break;
    case 10:
        mainGameState.setBit(4 * 8 + 6);
        mainGameState.setBit(13 * 8 + 2);
        mainGameState.clearBit(13 * 8 + 0);
        setNextGameStatus(0x4F);
        break;
    case 0x4A: // load savegame?
        if (gSaveGameStatus.m4_version == 0)
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

    case 0x48:
    case 0x71:
    case 0x72:
    case 0x74:
        return -1;

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
            if (readKeyboardTable1(0xE7))
                pWorkArea->m6_debugGameStatus--;
            if (readKeyboardTable1(0xE4))
                pWorkArea->m6_debugGameStatus++;

            vdp2DebugPrintSetPosition(0x15, 1);
            vdp2PrintStatus.m10_palette = 0xC000;
            vdp2PrintfSmallFont("%02x", (s32)pWorkArea->m6_debugGameStatus);

            if (keyboardIsKeyDown(0xE7) && readKeyboardToggle(0xE4))
                setNextGameStatus(pWorkArea->m6_debugGameStatus);
            if (readKeyboardToggle(0xE7) && keyboardIsKeyDown(0xE4))
                setNextGameStatus(pWorkArea->m6_debugGameStatus);
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
        if ((readSaturnS8(gCommonFile->getSaturnPtr(0x212EAC + gGameStatus.m4_gameStatus * 2)) == 4) && (gBattleManager->mE == 2))
        {
            setNextGameStatus(0x49);
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
            fadePalette(&g_fadeControls.m0_fade0, convertColorToU32ForFade(g_fadeControls.m0_fade0.m0_color), 0, 30);
            fadePalette(&g_fadeControls.m24_fade1, convertColorToU32ForFade(g_fadeControls.m24_fade1.m0_color), 0, 30);
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
            fadePalette(&g_fadeControls.m0_fade0, convertColorToU32ForFade(g_fadeControls.m0_fade0.m0_color), 0, 30);
            fadePalette(&g_fadeControls.m24_fade1, convertColorToU32ForFade(g_fadeControls.m24_fade1.m0_color), 0, 30);
            break;
        }

        assert(gGameStatus.m8_nextGameStatus >= 0);
        gGameStatus.m6_previousGameStatus = gGameStatus.m4_gameStatus;
        gGameStatus.m4_gameStatus = gGameStatus.m8_nextGameStatus;
        gGameStatus.m8_nextGameStatus = 0;

        PDS_Log("Switching to Game Satus %d\n", gGameStatus.m4_gameStatus);

        { static const s8 gameStatusTable[][2] = {
            {0, 0x00}, // 0x00 movie
            {0, 0x00}, // 0x01 movie
            {3, 0x00}, // 0x02 FLD_D5 (Name Entry)
            {0, 0x01}, // 0x03 movie
            {1, 0x10}, // 0x04 TWN_ZOAH
            {0, 0x02}, // 0x05 movie
            {1, 0x13}, // 0x06 TWN_E006
            {0, 0x03}, // 0x07 movie
            {1, 0x14}, // 0x08 TWN_E011
            {4, 0x00}, // 0x09 battle
            {1, 0x15}, // 0x0A TWN_E014
            {4, 0x01}, // 0x0B battle
            {1, 0x16}, // 0x0C town
            {1, 0x17}, // 0x0D town
            {4, 0x02}, // 0x0E battle
            {0, 0x05}, // 0x0F movie
            {4, 0x03}, // 0x10 battle
            {5, 0x04}, // 0x11 worldmap
            {0, 0x06}, // 0x12 movie
            {0, 0x07}, // 0x13 movie
            {0, 0x08}, // 0x14 movie
            {4, 0x04}, // 0x15 battle
            {3, 0x08}, // 0x16 FLD_B5
            {0, 0x09}, // 0x17 movie
            {4, 0x05}, // 0x18 battle
            {1, 0x18}, // 0x19 town
            {4, 0x06}, // 0x1A battle
            {1, 0x19}, // 0x1B town
            {0, 0x0A}, // 0x1C movie
            {0, 0x0B}, // 0x1D movie
            {0, 0x0C}, // 0x1E movie
            {4, 0x07}, // 0x1F battle
            {0, 0x0D}, // 0x20 movie
            {5, 0x05}, // 0x21 worldmap
            {4, 0x08}, // 0x22 battle
            {4, 0x09}, // 0x23 battle
            {0, 0x0E}, // 0x24 movie
            {3, 0x0C}, // 0x25 FLD_C4
            {4, 0x0A}, // 0x26 battle
            {0, 0x0F}, // 0x27 movie
            {4, 0x0C}, // 0x28 battle
            {1, 0x1A}, // 0x29 town
            {4, 0x0D}, // 0x2A battle
            {1, 0x1B}, // 0x2B town
            {0, 0x10}, // 0x2C movie
            {4, 0x0E}, // 0x2D battle
            {0, 0x11}, // 0x2E movie
            {1, 0x1C}, // 0x2F town
            {5, 0x06}, // 0x30 worldmap
            {0, 0x12}, // 0x31 movie
            {0, 0x13}, // 0x32 movie
            {0, 0x14}, // 0x33 movie
            {4, 0x0F}, // 0x34 battle
            {4, 0x10}, // 0x35 battle
            {0, 0x15}, // 0x36 movie
            {0, 0x16}, // 0x37 movie
            {0, 0x17}, // 0x38 movie
            {4, 0x11}, // 0x39 battle
            {4, 0x12}, // 0x3A battle
            {4, 0x13}, // 0x3B battle
            {4, 0x14}, // 0x3C battle
            {4, 0x15}, // 0x3D battle
            {4, 0x16}, // 0x3E battle
            {0, 0x18}, // 0x3F movie
            {0, 0x19}, // 0x40 movie
            {0, 0x1A}, // 0x41 movie
            {0, 0x1B}, // 0x42 movie
            {0, 0x1C}, // 0x43 movie
            {0, 0x1D}, // 0x44 movie
            {0, 0x1E}, // 0x45 movie
            {7, 0x00}, // 0x46
            {0, 0x1F}, // 0x47 movie
            {8, 0x00}, // 0x48
            {3, 0x14}, // 0x49 FLD_D5 (Game Over alt)
            {9, 0x00}, // 0x4A save/load screen
            {5, 0x00}, // 0x4B worldmap
            {5, 0x01}, // 0x4C worldmap
            {5, 0x02}, // 0x4D worldmap
            {5, 0x03}, // 0x4E worldmap
            {6, 0x00}, // 0x4F return to title
            {3, 0x01}, // 0x50 FLD_A3 (Excavation A2)
            {2, 0x0F}, // 0x51 TWN_EXCA
            {2, 0x07}, // 0x52 TWN_CAMP sub=7
            {3, 0x02}, // 0x53 FLD_A3 (Excavation A3)
            {0, 0x04}, // 0x54 movie
            {2, 0x01}, // 0x55 TWN_CAMP sub=1
            {3, 0x03}, // 0x56 FLD_A5 (Desert)
            {2, 0x09}, // 0x57 TWN_RUIN sub=0
            {2, 0x02}, // 0x58 TWN_CAMP sub=2
            {2, 0x0A}, // 0x59 TWN_RUIN sub=1
            {2, 0x03}, // 0x5A TWN_CAMP sub=3
            {2, 0x0B}, // 0x5B TWN_RUIN sub=2
            {2, 0x04}, // 0x5C TWN_CAMP sub=4
            {2, 0x0C}, // 0x5D TWN_RUIN sub=3
            {2, 0x0D}, // 0x5E TWN_RUIN sub=4
            {2, 0x05}, // 0x5F TWN_CAMP sub=5
            {2, 0x0E}, // 0x60 TWN_RUIN sub=5
            {3, 0x04}, // 0x61 FLD_A7 (Tower)
            {3, 0x05}, // 0x62 FLD_B1
            {3, 0x06}, // 0x63 FLD_B2 (B3 area)
            {3, 0x07}, // 0x64 FLD_B2 (Oasis)
            {2, 0x12}, // 0x65 TWN_SEEK
            {4, 0x0B}, // 0x66 battle
            {3, 0x0B}, // 0x67 FLD_C2
            {3, 0x0D}, // 0x68 FLD_C4
            {3, 0x0F}, // 0x69 FLD_C8 (Tower)
            {3, 0x12}, // 0x6A FLD_C8 (Tower D4)
            {3, 0x09}, // 0x6B FLD_B5 (Forest)
            {3, 0x0A}, // 0x6C FLD_B6
            {3, 0x10}, // 0x6D FLD_D2
            {2, 0x11}, // 0x6E TWN_CARA
            {2, 0x06}, // 0x6F TWN_CAMP sub=6
            {3, 0x13}, // 0x70 FLD_D5 (Game Over)
            {3, 0x15}, // 0x71 FLD_A3 (Tutorial BT0)
            {3, 0x16}, // 0x72 FLD_A7 (Tutorial BT1)
            {2, 0x08}, // 0x73 TWN_CAMP sub=8
            {10, 0x00}, // 0x74
        };

        if (gGameStatus.m4_gameStatus < (sizeof(gameStatusTable) / sizeof(gameStatusTable[0])))
        {
            gGameStatus.m0_gameMode = gameStatusTable[gGameStatus.m4_gameStatus][0];
            gGameStatus.m1 = gameStatusTable[gGameStatus.m4_gameStatus][1];
        }
        else
        {
            gGameStatus.m0_gameMode = readSaturnS8(gCommonFile->getSaturnPtr(0x212EAC + gGameStatus.m4_gameStatus * 2));
            gGameStatus.m1 = readSaturnS8(gCommonFile->getSaturnPtr(0x212EAC + gGameStatus.m4_gameStatus * 2) + 1);
        }

        if (gGameStatus.m6_previousGameStatus == 74)
        {
            gGameStatus.m3_loadingSaveFile = 1;
        }
        else
        {
            gGameStatus.m3_loadingSaveFile = 0;
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
        }
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

void setupSaveParams(s32 fieldIndex, s32 subFieldIndex, s32 entryPointIndex)
{
    graphicEngineStatus.m40AC.m0_menuId = 8; // trigger the save menu
    gSaveGameStatus.m8_gameMode = mainGameState.readPackedBits(0x87, 6) + 0x50;
    gSaveGameStatus.m9_fieldIndex = fieldIndex;
    gSaveGameStatus.mA_subFieldIndex = subFieldIndex;
    gSaveGameStatus.mB_entryPointIndex = entryPointIndex;
}
