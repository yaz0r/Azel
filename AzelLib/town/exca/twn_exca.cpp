#include "PDS.h"
#include "twn_exca.h"
#include "town/town.h"
#include "town/townScript.h"
#include "town/townEdge.h"
#include "town/townMainLogic.h"
#include "town/ruin/twn_ruin.h"
#include "kernel/animation.h"
#include "kernel/fileBundle.h"
#include "kernel/vdp1Allocator.h"
#include "kernel/graphicalObject.h"
#include "audio/soundDriver.h"
#include "town/townCamera.h"
#include "town/townDragon.h"
#include "town/excaEntity.h"


struct TWN_EXCA_data* gTWN_EXCA = NULL;
struct TWN_EXCA_data : public sTownOverlay
{
    static void makeCurrent()
    {
        if (gTWN_EXCA == NULL)
        {
            gTWN_EXCA = new TWN_EXCA_data();
        }
        gCurrentTownOverlay = gTWN_EXCA;
    }

    TWN_EXCA_data() : sTownOverlay("TWN_EXCA.PRG")
    {
        overlayScriptFunctions.m_zeroArg[0x06057f7c] = &scriptFunction_6057058_sub0;
        overlayScriptFunctions.m_zeroArg[0x060584b4] = &hasLoadingCompleted;

        overlayScriptFunctions.m_oneArg[0x605d780] = &TwnFadeOut;
        overlayScriptFunctions.m_oneArg[0x605d708] = &TwnFadeIn;
        overlayScriptFunctions.m_oneArg[0x6059d08] = &SetupColorOffset;

        overlayScriptFunctions.m_twoArg[0x605d4a0] = &townCamera_setup;

        overlayScriptFunctions.m_fourArg[0x605be24] = &setNpcLocation;
        overlayScriptFunctions.m_fourArg[0x605be52] = &setNpcOrientation;

        mTownSetups.push_back(readTownSetup(getSaturnPtr(0x605fd4c), 1));
    }

    sTownObject* createObjectTaskFromEA_siblingTaskWithEAArgWithCopy(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg) override
    {
        assert(definitionEA.m_file == this);
        assert(arg.m_file == this);

        switch (definitionEA.m_offset)
        {
        case 0x0606628c:
            assert(size == 0xE0);
            return createExcaEntity(parent, arg);
        default:
            assert(0);
            break;
        }
        return nullptr;
    }

    sTownObject* createObjectTaskFromEA_subTaskWithEAArg(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg) override
    {
        assert(definitionEA.m_file == this);
        assert(arg.m_file == this);

        switch (definitionEA.m_offset)
        {
        case 0x0606473c:
            assert(size == 0xE8);
            return createTownDragon(parent, arg);
        default:
            assert(0);
            break;
        }
        return nullptr;

    }
};

static const char* listOfFilesToLoad[] = {
    "COMMON3.MCB",
    "COMMON3.CGB",
    "EXCAVAMP.MCB",
    "EXCAVAMP.CGB",
    "Z_A_GS.MCB",
    "Z_A_GS.CGB",
    "X_A_AZ.MCB",
    "X_A_AZ.CGB",
    "X_G_LG.MCB",
    "X_G_LG.CGB",
    "C_DRA0.MCB",
    nullptr
};

static void startExcaBackgroundTask(p_workArea pThis)
{
    Unimplemented();

    // hack: to remove title screen
    reinitVdp2();
}

p_workArea overlayStart_TWN_EXCA(p_workArea pUntypedThis, u32 arg)
{
    gTWN_EXCA->makeCurrent();

    townDebugTask2Function* pThis = static_cast<townDebugTask2Function*>(pUntypedThis);

    pThis->m_DeleteMethod = &townOverlayDelete;

    loadSoundBanks(-1, 0);
    loadSoundBanks(59, 0);

    playPCM(pThis, 100);

    loadFnt("EVTEXCA.FNT");

    graphicEngineStatus.m405C.m10_nearClipDistance = 0x800;
    graphicEngineStatus.m405C.m30_oneOverNearClip = FP_Div(0x10000, graphicEngineStatus.m405C.m10_nearClipDistance);

    graphicEngineStatus.m405C.m14_farClipDistance = 0x100000;
    graphicEngineStatus.m405C.m38_oneOverFarClip = FP_Div(0x8000, graphicEngineStatus.m405C.m14_farClipDistance);

    graphicEngineStatus.m405C.m34_oneOverFarClip256 = graphicEngineStatus.m405C.m38_oneOverFarClip << 8;

    initDramAllocator(pThis, townBuffer, sizeof(townBuffer), listOfFilesToLoad);

    initVdp1Ram(pThis, 0x25C18800, 0x63800);

    registerNpcs(gTWN_EXCA->mTownSetups, gTWN_EXCA->getSaturnPtr(0x6054768), arg);

    startScriptTask(pThis);

    startExcaBackgroundTask(pThis);

    sEdgeTask* pEdgeTask = startEdgeTask(gTWN_EXCA->getSaturnPtr(0x605fd58));

    npcData0.m160_pEdgePosition = &pEdgeTask->m84.m8_position;

    startMainLogic(pThis);

    twnMainLogicTask->m14_EdgeTask = pEdgeTask;

    startCameraTask(pThis);

    twnVar1 = &twnVar2;

    return pThis;
}
