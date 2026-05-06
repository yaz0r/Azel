#include "PDS.h"
#include "worldmap.h"
#include "3dEngine.h"
#include "kernel/graphicalObject.h"
#include "kernel/vdp1Allocator.h"
#include "audio/soundDriver.h"
#include "kernel/loadSavegameScreen.h"
#include "mainMenuDebugTasks.h"
#include "menu_dragonMorph.h"
#include "dragonData.h"

static const char* worldmapFilesToLoad[] = {
    "WORLDMAP.MCB",
    "WORLDMAP.CGB",
    nullptr
};

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
            Unimplemented(); // FUN_060586c8 — create camera sub-task (0x60)
            Unimplemented(); // FUN_06055206 — create map display sub-task
            Unimplemented(); // FUN_06056f88 — create environment sub-task
            Unimplemented(); // FUN_060559d0 — create dragon control sub-task
            Unimplemented(); // createBattleEngineTask — battle encounter support
        }

        if (gDragonState == nullptr)
        {
            loadDragon(pThis);
        }

        pThis->m18_saveScreenTask = initLoadSavegameScreen(pThis, 0);
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
    p_workArea m14;
    p_workArea m18_saveScreenTask;
    // size 0x1C
};

// 06054000
static void worldMapOverlayEntryPoint(p_workArea pThis)
{
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
