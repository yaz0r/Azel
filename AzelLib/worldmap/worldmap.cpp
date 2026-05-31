#include "PDS.h"
#include "worldmap.h"
#include "3dEngine.h"
#include "kernel/graphicalObject.h"
#include "kernel/vdp1Allocator.h"
#include "audio/soundDriver.h"
#include "audio/systemSounds.h"
#include "kernel/loadSavegameScreen.h"
#include "kernel/fade.h"
#include "mainMenuDebugTasks.h"
#include "menu_dragonMorph.h"
#include "menu_dragon.h"
#include "dragonData.h"

void unloadFnt();

static const char* worldmapFilesToLoad[] = {
    "WORLDMAP.MCB",
    "WORLDMAP.CGB",
    nullptr
};

static sSaturnMemoryFile* g_worldmapPrg = nullptr;

static sSaturnPtr worldmapGetSaturnPtr(u32 address)
{
    return g_worldmapPrg->getSaturnPtr(address);
}

// Forward declarations
struct sWorldMapCamera;
struct sWorldMapRootTask;

static s32 worldMapCamera_handleNavigation(sWorldMapCamera* pThis);
static void worldMapCamera_drawDebugDirections(sWorldMapCamera* pThis);
static s32 worldMapCamera_findTargetInDirection(sWorldMapCamera* pThis, s32 direction);
static u32 worldMapCamera_getAngleBetweenTargets(sVec3_FP* pos1, sVec3_FP* pos2);
static u32 worldMapCamera_filterAngleByDirection(u32 angle, s32 cameraRotX, s32 direction);
static void worldMapCamera_setMessageMode(sWorldMapCamera* pThis, u8 mode);
static void worldMapCamera_prepareForTravel(sWorldMapCamera* pThis);
static void worldMapCamera_returnToSelection(sWorldMapCamera* pThis);
static void worldMapCamera_updateTextDisplay(sWorldMapCamera* pThis);
static void worldMapCamera_drawArrows(sWorldMapCamera* pThis);
static void worldMapCamera_toggleTargetMode(sWorldMapCamera* pThis);
static void worldMapCamera_updateNameDisplay(sWorldMapCamera* pThis, s32 targetIndex);
static void worldMapCamera_deleteNameTask(sWorldMapCamera* pThis);
static void worldMapCamera_updateNamePosition(sWorldMapCamera* pThis);
static s32 worldMapCamera_checkFleetAdvance(s32 param);
static void worldMapCamera_debugPrintBinary(u32 value, s32 numBits);
static void worldMapCamera_debugPrintDistance(u32 value);
static void worldMapCamera_debugPrintAngle(u32 value);

// ======== Camera sub-task (0x60) ========

struct sWorldMapCamera : public s_workAreaTemplateWithArgWithCopy<sWorldMapCamera, p_workArea>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sWorldMapCamera::Init, nullptr, &sWorldMapCamera::Draw, &sWorldMapCamera::Delete };
        return &taskDefinition;
    }

    // 06057234
    static void Init(sWorldMapCamera* pThis, p_workArea rootTask)
    {
        pThis->m8_rootTask = rootTask;
        loadFnt("WORLDMAP.FNT");
        setupVDP2StringRendering(2, 0x16, 0x28, 4);
        vdp2StringContext.m0 = 0;
        playSystemSoundEffect(4);
        pThis->m4C_cameraRotX = 0x3DDDDDD;
        pThis->m50_cameraRotY = 0x0EEEEEE;
        pThis->m54_cameraHeight = 0x113000;
        pThis->m10_targetTable = worldmapGetSaturnPtr(0x06059688);
        pThis->m14_numTargets = 0x21;

        if ((mainGameState.bitField[0x1B] & 8) == 0)
        {
            pThis->m10_targetTable = worldmapGetSaturnPtr(0x06059688);
            pThis->m14_numTargets = 0x21;
        }
        else
        {
            pThis->m10_targetTable = worldmapGetSaturnPtr(0x06059b2c);
            pThis->m14_numTargets = 4;
            s32 gameStatus = readPackedBits(mainGameState.bitField, 0x87, 6) + 0x50;
            if (gameStatus == 0x56)
            {
                pThis->m58_selectedTarget = 3;
            }
            else if (gameStatus == 0x67)
            {
                pThis->m58_selectedTarget = 2;
            }
            else if (gameStatus == 0x73)
            {
                pThis->m58_selectedTarget = 1;
            }
        }
    }

    // 06057DB8
    static void Draw(sWorldMapCamera* pThis);

    // 060586C2
    static void Delete(sWorldMapCamera* pThis) { unloadFnt(); }

    p_workArea m8_rootTask;
    p_workArea mC_nameTask;
    sSaturnPtr m10_targetTable;
    s32 m14_numTargets;
    s32 m18_scrollTimer;
    s16 m1C_selection;
    s16 m1E_arrowAnim;
    s8 m20_arrowDir;
    u8 m21_pad[3];
    sSaturnPtr m24_namePos;
    s32 m28_nameScreenX;
    s32 m2C_nameScreenY;
    s32 m30_nameScale;
    s32 m34_interpStartX;
    s32 m38_interpStartY;
    s32 m3C_interpStartZ;
    s32 m40_interpFactor;
    s16 m44_stringHalfWidth;
    s16 m46_storedTarget;
    s8 m48_messageMode;
    s8 m49_nextMessageMode;
    u8 m4A_pad[2];
    s32 m4C_cameraRotX;
    s32 m50_cameraRotY;
    s32 m54_cameraHeight;
    s16 m58_selectedTarget;
    s16 m5A_counter;
    s8 m5C_state;
    u8 m5D_flags;
    u8 m5E_pad[2];
    // size 0x60
};

// 060586c8
static p_workArea createWorldMapCamera(s_workAreaCopy* pNpcParent, p_workArea rootTask)
{
    return createSubTaskWithArgWithCopy<sWorldMapCamera>(pNpcParent, rootTask);
}

// ======== Map Display sub-task (0x210) ========

struct sWorldMapDisplay : public s_workAreaTemplateWithCopy<sWorldMapDisplay>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sWorldMapDisplay::Init, &sWorldMapDisplay::Update, &sWorldMapDisplay::Draw, nullptr };
        return &taskDefinition;
    }

    // 06054504
    static void Init(sWorldMapDisplay* pThis) { Unimplemented(); }
    // 06054F9E
    static void Update(sWorldMapDisplay* pThis) { Unimplemented(); }
    // 060551AC
    static void Draw(sWorldMapDisplay* pThis) { Unimplemented(); }

    p_workArea m4_rootTask;
    u8 m8_pad[0x210 - 8];
    // size 0x210
};

// 06055206
static p_workArea createWorldMapDisplay(s_workAreaCopy* pNpcParent, p_workArea rootTask)
{
    sWorldMapDisplay* pTask = createSubTaskWithCopy<sWorldMapDisplay>(pNpcParent);
    if (pTask)
    {
        pTask->m4_rootTask = rootTask;
    }
    return pTask;
}

// ======== Environment sub-task (0x70) ========

struct sWorldMapEnv : public s_workAreaTemplate<sWorldMapEnv>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { nullptr, &sWorldMapEnv::Update, nullptr, nullptr };
        return &taskDefinition;
    }
    // 06056928
    static void Update(sWorldMapEnv* pThis) { Unimplemented(); }

    p_workArea m0_rootTask;
    u8 m4_pad[0x70 - 4];
    // size 0x70
};

// 06056f88
static p_workArea createWorldMapEnv(p_workArea rootTask)
{
    sWorldMapEnv* pTask = createSubTask<sWorldMapEnv>(rootTask);
    if (pTask)
    {
        pTask->m0_rootTask = rootTask;
    }
    return pTask;
}

// ======== Dragon Control sub-task (0x2C) ========

struct sWorldMapDragon : public s_workAreaTemplateWithArgWithCopy<sWorldMapDragon, p_workArea>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sWorldMapDragon::Init, nullptr, &sWorldMapDragon::Draw, nullptr };
        return &taskDefinition;
    }

    // 06055740
    static void Init(sWorldMapDragon* pThis, p_workArea rootTask) { Unimplemented(); }
    // 06055924
    static void Draw(sWorldMapDragon* pThis) { Unimplemented(); }

    u8 m8_pad[0x2C - 8];
    // size 0x2C
};

// 060559d0
static p_workArea createWorldMapDragon(s_workAreaCopy* pNpcParent, p_workArea rootTask)
{
    return createSubTaskWithArgWithCopy<sWorldMapDragon>(pNpcParent, rootTask);
}

// ======== Root task ========

struct sWorldMapRootTask : public s_workAreaTemplate<sWorldMapRootTask>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = {
            &sWorldMapRootTask::Init,
            nullptr,
            nullptr,
            &sWorldMapRootTask::Delete
        };
        return &taskDefinition;
    }

    // 06054074
    static void Init(sWorldMapRootTask* pThis)
    {
        resetTempAllocators();
        initDramAllocator(pThis, townBuffer, 0xB0000, worldmapFilesToLoad);
        initVdp1Ram(pThis, 0x25C18800, 0x62E00);

        npcFileDeleter* pNpcParent = allocateNPC(pThis, 0);
        if (pNpcParent)
        {
            pThis->mC_cameraTask = createWorldMapCamera(pNpcParent, pThis);
            pThis->m0_mapDisplayTask = createWorldMapDisplay(pNpcParent, pThis);
            pThis->m10_envTask = createWorldMapEnv(pThis);
            pThis->m4_dragonControlTask = createWorldMapDragon(pNpcParent, pThis);
            Unimplemented(); // createBattleEngineTask — battle encounter support
        }

        if (gDragonState == nullptr)
        {
            loadDragon(pThis);
        }

        pThis->m14_saveScreenTask = initLoadSavegameScreen(pThis, 0);
        pauseEngine[2] = 1;
    }

    // 06054100
    static void Delete(sWorldMapRootTask* pThis)
    {
        freeRamResources(pThis);
    }

    p_workArea m0_mapDisplayTask;
    p_workArea m4_dragonControlTask;
    p_workArea m8_battleEngineTask;
    p_workArea mC_cameraTask;
    p_workArea m10_envTask;
    p_workArea m14_saveScreenTask;
    s16 m18_padMode;
    u8 m1A_pad[2];
    // size 0x1C
};

// 06057DB8
void sWorldMapCamera::Draw(sWorldMapCamera* pThis)
{
    sWorldMapRootTask* pRoot = (sWorldMapRootTask*)pThis->m8_rootTask;
    sSaturnPtr targetTable = pThis->m10_targetTable;

    if (readKeyboardToggle(0x8d))
    {
        clearVdp2TextMemory();
        s16 padMode = pRoot->m18_padMode;
        pRoot->m18_padMode = padMode + 1;
        if (padMode + 1 > 7)
        {
            pRoot->m18_padMode = 0;
        }
    }

    if ((s16)graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & (s16)0x8000)
    {
        Unimplemented(); // increment map display m20C debug counter
    }
    if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 0x800)
    {
        Unimplemented(); // decrement map display m20C debug counter
    }

    switch (pThis->m5C_state)
    {
    case 1:
    {
        if ((s8)g_fadeControls.m_4C <= (s8)g_fadeControls.m_4D)
        {
            vdp2Controls.m20_registers[1].m112_CLOFSL = 0;
            vdp2Controls.m20_registers[0].m112_CLOFSL = 0;
        }
        u32 color = convertColorToU32ForFade(g_fadeControls.m0_fade0.m0_color);
        fadePalette(&g_fadeControls.m0_fade0, color, (s16)g_fadeControls.m_48, 0x1e);
        pThis->m18_scrollTimer = 0x60;
        pThis->m5D_flags = 0;
        if ((mainGameState.bitField[0x1B] & 8) == 0)
        {
            pThis->m58_selectedTarget = (s16)readPackedBits(mainGameState.bitField, 0x87, 6);
            pThis->m5D_flags |= 1;
        }
        else
        {
            Unimplemented(); // set map display m20D |= 0x40
            pThis->m5D_flags &= 0xFE;
        }
        Unimplemented(); // env task m6D flags setup
        Unimplemented(); // save screen task m3C = 0
        Unimplemented(); // map display m1EC, m20D, m1E4 setup
        Unimplemented(); // dragon control m24/m28 setup
        Unimplemented(); // battle engine m68 flags
        worldMapCamera_setMessageMode(pThis, 3);
        pThis->m48_messageMode = 3;
        pThis->m49_nextMessageMode = (s8)0xFF;
        pThis->m1C_selection = -1;
        pThis->m5A_counter = 0xF;
        pThis->m5C_state = 2;
        break;
    }

    case 2:
    {
        s16 count = pThis->m5A_counter;
        pThis->m5A_counter = count - 1;
        if (count - 1 == 0)
        {
            pThis->m5C_state = 3;
        }
        break;
    }

    case 3:
    {
        if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 6) != 0)
        {
            sSaturnPtr entry = targetTable + (s32)pThis->m58_selectedTarget * 0x24;
            s16 condition = readSaturnS16(entry);
            if (condition < 1)
            {
                pThis->m5D_flags |= 8;
            }
            else
            {
                if ((bitMasks[condition & 7] & mainGameState.bitField[(u32)condition >> 3]) == 0)
                {
                    playSystemSoundEffect(5);
                    goto case3_navigation;
                }
                pThis->m5D_flags |= 8;
            }
            pThis->m5D_flags &= ~4;
            pThis->m5C_state = 7;
            break;
        }

    case3_navigation:
        if ((pThis->m5D_flags & 1) == 0)
        {
            goto case3_normalNav;
        }

        if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 0x2000) != 0)
        {
            s32 fleet = worldMapCamera_checkFleetAdvance(0);
            if (fleet >= 0)
            {
                worldMapCamera_prepareForTravel(pThis);
                pThis->m1C_selection = 0;
                pThis->m5C_state = 4;
                break;
            }
        }
        if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 0x4000) != 0)
        {
            s32 fleet = worldMapCamera_checkFleetAdvance(1);
            if (fleet >= 0)
            {
                worldMapCamera_prepareForTravel(pThis);
                pThis->m1C_selection = 1;
                pThis->m5C_state = 4;
                break;
            }
        }

    case3_normalNav:
        {
            s32 navResult = worldMapCamera_handleNavigation(pThis);
            if (navResult == 0)
            {
                if (pRoot->m18_padMode == 0)
                {
                    if (readKeyboardToggle(0x9a))
                    {
                        Unimplemented(); // map display m1E4 = 2
                    }
                    if (readKeyboardToggle(0x9b))
                    {
                        mainGameState.setPackedBits(0x87, 6, (s32)pThis->m58_selectedTarget);
                        pThis->m5C_state = 0;
                    }
                    if (readKeyboardToggle(0xb4))
                    {
                        pThis->m5D_flags |= 8;
                        pThis->m5D_flags |= 4;
                        pThis->m5C_state = 7;
                    }
                    if (readKeyboardToggle(0xab))
                    {
                        worldMapCamera_prepareForTravel(pThis);
                        pThis->m5C_state = 5;
                    }
                }
            }
            else
            {
                worldMapCamera_returnToSelection(pThis);
            }
        }
        break;
    }

    case 4:
    {
        if (pThis->m1C_selection == 0)
        {
            Unimplemented(); // dragon control m24=0, m28=0
            worldMapCamera_setMessageMode(pThis, 4);
        }
        else if (pThis->m1C_selection == 1)
        {
            Unimplemented(); // dragon control m24=1, m28=0
            worldMapCamera_setMessageMode(pThis, 5);
        }
        playSystemSoundEffect(3);
        pThis->m5C_state = 5;
        break;
    }

    case 5:
    {
        if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 0xF7) != 0)
        {
            playSystemSoundEffect(4);
            worldMapCamera_returnToSelection(pThis);
            pThis->m1C_selection = -1;
            pThis->m5C_state = 3;
            break;
        }
        if (pRoot->m18_padMode != 0)
            break;

        if (readKeyboardToggle(0xb1))
        {
            worldMapCamera_returnToSelection(pThis);
            pThis->m5C_state = 3;
            break;
        }

        if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 0x2000) != 0)
        {
            s32 fleet = worldMapCamera_checkFleetAdvance(0);
            if (fleet >= 0 && pThis->m1C_selection != 0)
            {
                playSystemSoundEffect(2);
                Unimplemented(); // dragon control m24=0, m28=0
                worldMapCamera_setMessageMode(pThis, 4);
                pThis->m1C_selection = 0;
                break;
            }
        }
        if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 0x4000) != 0)
        {
            s32 fleet = worldMapCamera_checkFleetAdvance(1);
            if (fleet >= 0 && pThis->m1C_selection != 1)
            {
                playSystemSoundEffect(2);
                Unimplemented(); // dragon control m24=1, m28=0
                worldMapCamera_setMessageMode(pThis, 5);
                pThis->m1C_selection = 1;
                break;
            }
        }
        break;
    }

    case 6:
    {
        if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 0xF7) != 0)
        {
            u32 color = convertColorToU32ForFade(g_fadeControls.m0_fade0.m0_color);
            fadePalette(&g_fadeControls.m0_fade0, color, 0xC210, 0x1e);
            mainGameState.setPackedBits(0x87, 6, (s32)pThis->m58_selectedTarget);
            pThis->m5C_state = 0;
        }
        break;
    }

    case 7:
    {
        clearVdp2TextArea();
        playSystemSoundEffect(0);
        sSaturnPtr entry = targetTable + (s32)pThis->m58_selectedTarget * 0x24;
        s16 gameStatus = readSaturnS16(entry + 2);
        if (gameStatus < 0)
        {
            pThis->m5C_state = (s8)0xFF;
            u32 color = convertColorToU32ForFade(g_fadeControls.m0_fade0.m0_color);
            fadePalette(&g_fadeControls.m0_fade0, color, 0, 0x1e);
            break;
        }
        Unimplemented(); // env task m6D flags
        pThis->m5D_flags &= ~1;
        s8 destType = readSaturnS8(entry + 5);
        if (destType == 0 || destType == 1)
        {
            Unimplemented(); // save screen m3C = 2, map display m1E4 = 3
            pThis->m5C_state = 8;
        }
        else if (destType == 2 || destType == 3)
        {
            Unimplemented(); // map display m1E4 = 2
            pThis->m5A_counter = 0xF;
            pThis->m5C_state = 9;
        }

        if ((pThis->m5D_flags & 4) == 0)
        {
            s8 soundIndex = readSaturnS8(entry + 4);
            if (soundIndex > 0)
            {
                if (destType == 0)
                {
                    Unimplemented(); // FUN_06011e0a — load field overlay
                }
                else if (destType == 1 || destType == 2 || destType == 3)
                {
                    Unimplemented(); // FUN_06030100 — load town overlay
                }
            }
        }
        break;
    }

    case 8:
    {
        Unimplemented(); // check save screen m3D, wait for save complete
        sSaturnPtr entry = targetTable + (s32)pThis->m58_selectedTarget * 0x24;
        s16 gameStatus = readSaturnS16(entry + 2);
        u32 color = convertColorToU32ForFade(g_fadeControls.m0_fade0.m0_color);
        if (gameStatus == 0x66)
        {
            fadePalette(&g_fadeControls.m0_fade0, color, 0xFFFF, 0x3C);
        }
        else
        {
            fadePalette(&g_fadeControls.m0_fade0, color, 0x8000, 0x3C);
        }
        pThis->m5C_state = 10;
        break;
    }

    case 9:
    {
        s16 count = pThis->m5A_counter;
        pThis->m5A_counter = count - 1;
        if (count - 1 > 0)
            break;
        u32 color = convertColorToU32ForFade(g_fadeControls.m0_fade0.m0_color);
        fadePalette(&g_fadeControls.m0_fade0, color, 0x8000, 0x1e);
        pThis->m5C_state = 10;
        break;
    }

    case 10:
    {
        if (g_fadeControls.m0_fade0.m20_stopped == 0)
            break;
        if ((pThis->m5D_flags & 4) == 0)
        {
            sSaturnPtr entry = targetTable + (s32)pThis->m58_selectedTarget * 0x24;
            setNextGameStatus((s32)readSaturnS16(entry + 2));
            pThis->getTask()->markFinished();
            break;
        }
        mainGameState.setPackedBits(0x87, 6, (s32)pThis->m58_selectedTarget);
        pThis->m5C_state = 6;
        break;
    }

    case (s8)0xFF:
    {
        if (g_fadeControls.m0_fade0.m20_stopped == 0)
            break;
        worldMapCamera_toggleTargetMode(pThis);
        worldMapCamera_returnToSelection(pThis);
        if ((s8)g_fadeControls.m_4C <= (s8)g_fadeControls.m_4D)
        {
            vdp2Controls.m20_registers[1].m112_CLOFSL = 0;
            vdp2Controls.m20_registers[0].m112_CLOFSL = 0;
        }
        u32 color = convertColorToU32ForFade(g_fadeControls.m0_fade0.m0_color);
        fadePalette(&g_fadeControls.m0_fade0, color, (s16)g_fadeControls.m_48, 0x1e);
        pThis->m5C_state = 0;
        // fall through
    }
    case 0:
    {
        sWorldMapEnv* pEnv = (sWorldMapEnv*)pRoot->m10_envTask;
        if (pEnv && (((u8*)pEnv)[0x6D] & 4))
        {
            pThis->m5C_state++;
        }
        break;
    }

    default:
        break;
    }

    worldMapCamera_updateTextDisplay(pThis);
    worldMapCamera_drawArrows(pThis);

    if (enableDebugTask)
    {
        vdp2DebugPrintSetPosition(2, 4);
        Unimplemented(); // debug: print pad mode string
        vdp2DebugPrintSetPosition(2, -6);
        vdp2PrintfSmallFont("select   %2d/%2d", (s32)pThis->m58_selectedTarget, pThis->m14_numTargets);
        vdp2DebugPrintSetPosition(2, -5);
        vdp2PrintfSmallFont("mode     %2d", (s32)pThis->m5C_state);
        vdp2DebugPrintSetPosition(2, -4);
        vdp2PrintfSmallFont("mes_mode %2d", (s32)pThis->m48_messageMode);
        vdp2DebugPrintSetPosition(2, -3);
        worldMapCamera_debugPrintBinary(pThis->m5D_flags, 8);
    }

    if (pRoot->m18_padMode == 0)
    {
        if (enableDebugTask)
        {
            vdp2DebugPrintSetPosition(2, 8);
            vdp2PrintfSmallFont("DistMax   ");
            worldMapCamera_debugPrintDistance(pThis->m54_cameraHeight);
            vdp2DebugPrintSetPosition(2, 9);
            vdp2PrintfSmallFont("AngleRange");
            worldMapCamera_debugPrintAngle(pThis->m4C_cameraRotX);
            vdp2DebugPrintSetPosition(2, 10);
            vdp2PrintfSmallFont("AngleRate ");
            worldMapCamera_debugPrintAngle(pThis->m50_cameraRotY);
        }

        if (readKeyboardToggle(0x95))
        {
            Unimplemented(); // toggle env m6D bit 0
        }
        if (readKeyboardTable1(0x10d))
        {
            pThis->m54_cameraHeight += 0x1000;
        }
        if (readKeyboardTable1(0x106))
        {
            pThis->m54_cameraHeight -= 0x1000;
        }
        if (readKeyboardTable1(0x10b))
        {
            pThis->m4C_cameraRotX += 0xb60b6;
        }
        if (readKeyboardTable1(0x10c))
        {
            pThis->m4C_cameraRotX -= 0xb60b6;
        }
        if (readKeyboardTable1(0x109))
        {
            pThis->m50_cameraRotY += 0xb60b6;
        }
        if (readKeyboardTable1(0x10a))
        {
            pThis->m50_cameraRotY -= 0xb60b6;
        }
        if (readKeyboardToggle(0xc5))
        {
            pThis->m5D_flags ^= 0x10;
        }
        if (readKeyboardToggle(0x96))
        {
            Unimplemented(); // toggle battle engine m68 bit 0
        }
        if (readKeyboardToggle(0x9e))
        {
            Unimplemented(); // toggle battle engine m68 bit 1
        }
        if (readKeyboardToggle(0xa6))
        {
            Unimplemented(); // toggle battle engine m68 bit 2
        }
        if (readKeyboardToggle(0xa5))
        {
            Unimplemented(); // toggle battle engine m68 bit 3
        }
        if (readKeyboardToggle(0xae))
        {
            Unimplemented(); // toggle battle engine m68 bit 4
        }
    }
}

// ======== Camera helper functions ========

// 060572c6
static void worldMapCamera_setMessageMode(sWorldMapCamera* pThis, u8 mode)
{
    if (pThis->m48_messageMode == 0)
    {
        pThis->m48_messageMode = mode;
        pThis->m49_nextMessageMode = (s8)0xFF;
    }
    else
    {
        pThis->m49_nextMessageMode = mode;
    }
}

// 060572e2
static void worldMapCamera_prepareForTravel(sWorldMapCamera* pThis)
{
    sWorldMapRootTask* pRoot = (sWorldMapRootTask*)pThis->m8_rootTask;
    Unimplemented(); // env task m6D &= ~1
    Unimplemented(); // map display m20D &= ~0x20
    Unimplemented(); // map display m1E4 = 5
    worldMapCamera_setMessageMode(pThis, 6);
}

// 06057308
static void worldMapCamera_returnToSelection(sWorldMapCamera* pThis)
{
    sWorldMapRootTask* pRoot = (sWorldMapRootTask*)pThis->m8_rootTask;
    Unimplemented(); // env task m6D |= 1
    Unimplemented(); // map display m20D |= 0x20
    Unimplemented(); // map display m1E4 = 4
    Unimplemented(); // dragon control m24 = -1, m28 = 0
    Unimplemented(); // battle engine m68 |= 0x10
    Unimplemented(); // map display m1EC = selectedTarget
    worldMapCamera_setMessageMode(pThis, 3);
}

// 06057634
static void worldMapCamera_deleteNameTask(sWorldMapCamera* pThis)
{
    if (pThis->mC_nameTask != nullptr)
    {
        pThis->mC_nameTask->getTask()->markFinished();
        pThis->mC_nameTask = nullptr;
    }
}

// 06057658
static void worldMapCamera_updateNameDisplay(sWorldMapCamera* pThis, s32 targetIndex)
{
    Unimplemented(); // read target name string, compute string length, create name display task
}

// 060574e4
static void worldMapCamera_updateNamePosition(sWorldMapCamera* pThis)
{
    Unimplemented(); // transform name position through 3D matrix, clamp to screen bounds
}

// 06055772
static s32 worldMapCamera_checkFleetAdvance(s32 param)
{
    if (param == 0)
    {
        if (mainGameState.bitField[9] & 0x10) return -1;
        if (mainGameState.bitField[9] & 0x40) return 4;
        if (mainGameState.bitField[0xC] & 0x40) return 3;
        if (mainGameState.bitField[8] & 0x40) return 2;
        if (mainGameState.bitField[7] & 0x10) return 1;
        if (mainGameState.bitField[5] & 1) return 0;
        return -1;
    }
    else
    {
        if (mainGameState.bitField[0xC] & 0x40) return -1;
        if (mainGameState.bitField[0xF] & 1) return 3;
        if (mainGameState.bitField[0xB] & 4) return 2;
        if (mainGameState.bitField[5] & 0x40) return 1;
        if (mainGameState.bitField[4] & 8) return 0;
        return -1;
    }
}

// 06057874
static u32 worldMapCamera_getAngleBetweenTargets(sVec3_FP* pos1, sVec3_FP* pos2)
{
    return atan2_FP(pos2->m0_X - pos1->m0_X, pos2->m8_Z - pos1->m8_Z);
}

// 06057892
static u32 worldMapCamera_filterAngleByDirection(u32 angle, s32 cameraRotX, s32 direction)
{
    u32 maskedAngle = angle & 0xFFFFFFF;

    if (direction == 0)
    {
        if ((s32)maskedAngle >= 0x8000000 - cameraRotX && (s32)maskedAngle <= cameraRotX + 0x8000000)
        {
            if ((s32)(maskedAngle + 0xF8000000) >= 0)
                return maskedAngle + 0xF8000000;
            return 0x8000000 - maskedAngle;
        }
    }
    else if (direction == 1)
    {
        if ((s32)maskedAngle >= 0x4000000 - cameraRotX && (s32)maskedAngle <= cameraRotX + 0x4000000)
        {
            if ((s32)(maskedAngle + 0xFC000000) >= 0)
                return maskedAngle + 0xFC000000;
            return 0x4000000 - maskedAngle;
        }
    }
    else if (direction == 2)
    {
        if ((s32)maskedAngle >= 0x10000000 - cameraRotX || (s32)maskedAngle <= cameraRotX)
        {
            u32 signBit = angle & 0x8000000;
            u32 val = maskedAngle;
            if (signBit) val = maskedAngle | 0xF0000000;
            if ((s32)val >= 0)
            {
                return signBit ? (maskedAngle | 0xF0000000) : maskedAngle;
            }
            return signBit ? -(maskedAngle | 0xF0000000) : -maskedAngle;
        }
    }
    else if (direction == 3)
    {
        if ((s32)maskedAngle >= 0xC000000 - cameraRotX && (s32)maskedAngle <= cameraRotX + 0xC000000)
        {
            if ((s32)(maskedAngle + 0xF4000000) >= 0)
                return maskedAngle + 0xF4000000;
            return 0xC000000 - maskedAngle;
        }
    }
    return 0xFFFFFFFF;
}

// 0605799c
static s32 worldMapCamera_findTargetInDirection(sWorldMapCamera* pThis, s32 direction)
{
    s32 bestTarget = -1;
    s32 bestScore = 0x270F000;
    sSaturnPtr targetTable = pThis->m10_targetTable;

    sVec3_FP currentPos;
    sSaturnPtr currentEntry = targetTable + (s32)pThis->m58_selectedTarget * 0x24;
    currentPos.m0_X = readSaturnS32(currentEntry + 8);
    currentPos.m4_Y = readSaturnS32(currentEntry + 0xC);
    currentPos.m8_Z = readSaturnS32(currentEntry + 0x10);

    for (s32 i = 0; i < pThis->m14_numTargets; i++)
    {
        if (i == pThis->m58_selectedTarget)
            continue;

        sSaturnPtr entry = targetTable + i * 0x24;
        s16 condition = readSaturnS16(entry);
        if (condition >= 1 && (bitMasks[condition & 7] & mainGameState.bitField[(u32)condition >> 3]) == 0)
            continue;

        sVec3_FP targetPos;
        targetPos.m0_X = readSaturnS32(entry + 8);
        targetPos.m4_Y = readSaturnS32(entry + 0xC);
        targetPos.m8_Z = readSaturnS32(entry + 0x10);

        u32 angle = worldMapCamera_getAngleBetweenTargets(&currentPos, &targetPos);
        u32 filtered = worldMapCamera_filterAngleByDirection(angle, pThis->m4C_cameraRotX, direction);
        if ((s32)filtered >= 0)
        {
            fixedPoint divisor;
            divisor.m_value = pThis->m50_cameraRotY;
            s32 score = FP_Div(filtered, divisor);
            s32 dist = vecDistance(currentPos, targetPos);
            score = MTH_Mul(dist, score + 0x10000);
            if (score < pThis->m54_cameraHeight && score <= bestScore)
            {
                bestTarget = i;
                bestScore = score;
            }
        }
    }

    if (bestTarget < 0)
    {
        bestTarget = (s32)pThis->m58_selectedTarget;
    }
    return bestTarget;
}

// 06057ae0
static void worldMapCamera_drawDebugDirections(sWorldMapCamera* pThis)
{
    Unimplemented(); // debug visualization: draw lines to reachable targets in all 4 directions
}

// 06057c38
static s32 worldMapCamera_handleNavigation(sWorldMapCamera* pThis)
{
    sWorldMapRootTask* pRoot = (sWorldMapRootTask*)pThis->m8_rootTask;
    s32 currentTarget = (s32)pThis->m58_selectedTarget;

    if (pThis->m5D_flags & 0x10)
    {
        worldMapCamera_drawDebugDirections(pThis);
    }

    if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.mC_newButtonDown2 & 0x10)
    {
        currentTarget = worldMapCamera_findTargetInDirection(pThis, 0);
    }
    if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.mC_newButtonDown2 & 0x80)
    {
        currentTarget = worldMapCamera_findTargetInDirection(pThis, 1);
    }
    if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.mC_newButtonDown2 & 0x20)
    {
        currentTarget = worldMapCamera_findTargetInDirection(pThis, 2);
    }
    if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.mC_newButtonDown2 & 0x40)
    {
        currentTarget = worldMapCamera_findTargetInDirection(pThis, 3);
    }

    if (pThis->m58_selectedTarget == currentTarget)
    {
        return 0;
    }
    else
    {
        pThis->m58_selectedTarget = (s16)currentTarget;
        playSystemSoundEffect(10);
        if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 0xF0) == 0)
        {
            Unimplemented(); // map display m20D |= 2, m20D |= 8
        }
        else
        {
            Unimplemented(); // map display m20D |= 1
        }
        return 1;
    }
}

// 06057ce0
static void worldMapCamera_toggleTargetMode(sWorldMapCamera* pThis)
{
    if ((mainGameState.bitField[0x1A] & 2) == 0)
        return;

    if (mainGameState.bitField[0x1B] & 8)
    {
        mainGameState.bitField[0x1B] &= ~8;
        Unimplemented(); // map display m20D &= ~0x40
        pThis->m10_targetTable = worldmapGetSaturnPtr(0x06059688);
        pThis->m14_numTargets = 0x21;
        pThis->m58_selectedTarget = (s16)readPackedBits(mainGameState.bitField, 0xDE, 6);
        mainGameState.setPackedBits(0x87, 6, (s32)pThis->m58_selectedTarget);
    }
    else
    {
        mainGameState.bitField[0x1B] |= 8;
        Unimplemented(); // map display m20D |= 0x40
        pThis->m10_targetTable = worldmapGetSaturnPtr(0x06059b2c);
        pThis->m14_numTargets = 4;
        mainGameState.setPackedBits(0xDE, 6, (s32)pThis->m58_selectedTarget);
        pThis->m58_selectedTarget = 0;
    }
}

// 06057708
static void worldMapCamera_updateTextDisplay(sWorldMapCamera* pThis)
{
    sSaturnPtr targetTable = pThis->m10_targetTable;
    s32 mode = (s32)pThis->m48_messageMode;

    if (mode == 0)
    {
        if (pThis->mC_nameTask != nullptr)
        {
            worldMapCamera_updateNamePosition(pThis);
        }
        if ((pThis->m5D_flags & 8) == 0)
        {
            if (pThis->m49_nextMessageMode >= 0)
            {
                pThis->m48_messageMode = pThis->m49_nextMessageMode;
                pThis->m49_nextMessageMode = (s8)0xFF;
            }
        }
        else
        {
            pThis->m34_interpStartX = pThis->m28_nameScreenX;
            pThis->m38_interpStartY = pThis->m2C_nameScreenY;
            pThis->m3C_interpStartZ = pThis->m30_nameScale;
            pThis->m40_interpFactor = 0;
            pThis->m48_messageMode = 1;
            pThis->m49_nextMessageMode = (s8)0xFF;
        }
    }
    else if (mode == 1)
    {
        if (pThis->mC_nameTask != nullptr)
        {
            Unimplemented(); // interpolate name position toward screen center
        }
    }
    else if (mode == 2)
    {
        worldMapCamera_updateNameDisplay(pThis, (s32)pThis->m46_storedTarget);
        sSaturnPtr entry = targetTable + (s32)pThis->m46_storedTarget * 0x24;
        s16 condition = readSaturnS16(entry);
        if (condition == 0x85 && (mainGameState.bitField[0x10] & 8) == 0)
        {
            Unimplemented(); // show alternate string "forbidden" text
        }
        pThis->m48_messageMode = 0;
    }
    else if (mode == 3)
    {
        worldMapCamera_deleteNameTask(pThis);
        clearVdp2TextArea();
        pThis->m46_storedTarget = pThis->m58_selectedTarget;
        pThis->m48_messageMode = 2;
    }
    else if (mode == 4)
    {
        worldMapCamera_deleteNameTask(pThis);
        clearVdp2TextArea();
        vdp2StringContext.m4_cursorX = vdp2StringContext.mC_X + 2;
        vdp2StringContext.m8_cursorY = vdp2StringContext.m10_Y;
        drawObjectName("Imperial Fleet Advance");
        pThis->m48_messageMode = 0;
    }
    else if (mode == 5)
    {
        worldMapCamera_deleteNameTask(pThis);
        clearVdp2TextArea();
        vdp2StringContext.m4_cursorX = vdp2StringContext.mC_X + 2;
        vdp2StringContext.m8_cursorY = vdp2StringContext.m10_Y;
        drawObjectName("Craymen\'s Fleet Advance");
        pThis->m48_messageMode = 0;
    }
    else if (mode == 6)
    {
        worldMapCamera_deleteNameTask(pThis);
        clearVdp2TextArea();
        pThis->m48_messageMode = 0;
    }
}

// 0605738c
static void worldMapCamera_drawArrows(sWorldMapCamera* pThis)
{
    Unimplemented(); // VDP1 arrow sprite rendering for L/R fleet selection indicators
}

// 06056504
static void worldMapCamera_debugPrintBinary(u32 value, s32 numBits)
{
    char buffer[33];
    for (s32 i = 0; i < 32; i++)
    {
        u32 bit = shiftLeft32(1, 31 - i);
        buffer[i] = (value & bit) ? '1' : '0';
    }
    buffer[32] = 0;
    vdp2PrintfSmallFont("%s", buffer + (32 - numBits));
}

// 060563e8
static void worldMapCamera_debugPrintDistance(u32 value)
{
    if ((s32)value < 0)
    {
        value = -(s32)value;
        vdp2PrintfSmallFont("-%4d.%02d ", (s32)value >> 12, (s32)((value & 0xFFF) * 100) >> 12);
    }
    else
    {
        vdp2PrintfSmallFont(" %4d.%02d ", (s32)value >> 12, (s32)((value & 0xFFF) * 100) >> 12);
    }
}

// 06056488
static void worldMapCamera_debugPrintAngle(u32 value)
{
    u32 scaled = ((value & 0xFFFFFFF) >> 12) * 0x168;
    vdp2PrintfSmallFont("%3d.%02d ", (s16)(scaled >> 16), (s16)((scaled & 0xFFFF) * 100 >> 16));
}

// ======== Entry point ========

// 06054000
static void worldMapOverlayEntryPoint(p_workArea pThis)
{
    g_worldmapPrg = new sSaturnMemoryFile("WORLD.PRG");

    reset3dEngine();
    resetCameraProperties2((s_cameraProperties2*)&cameraProperties2);

    graphicEngineStatus.m405C.m10_nearClipDistance = 0x800;
    graphicEngineStatus.m405C.m30_oneOverNearClip = FP_Div(0x10000, graphicEngineStatus.m405C.m10_nearClipDistance);
    graphicEngineStatus.m405C.m14_farClipDistance = 0x400000;
    graphicEngineStatus.m405C.m38_oneOverFarClip = FP_Div(0x8000, graphicEngineStatus.m405C.m14_farClipDistance);
    graphicEngineStatus.m405C.m34_oneOverFarClip256 = graphicEngineStatus.m405C.m38_oneOverFarClip << 8;

    resetProjectVector();

    loadSoundBanks(0x4B, 0);

    createSubTask<sWorldMapRootTask>(pThis);
}

// 06027878
p_workArea loadWorldOverlay(p_workArea pThis, s32 arg)
{
    worldMapOverlayEntryPoint(pThis);
    return pThis;
}
