#include "PDS.h"
#include "twn_e014.h"
#include "town/town.h"
#include "audio/soundDriver.h"
#include "kernel/vdp1Allocator.h"
#include "town/townMainLogic.h"
#include "town/townCamera.h"

#include "town/e006/twn_e006.h" // TODO cleanup
void setupVdp1Proj(fixedPoint fov); // TODO: cleanup

// https://www.youtube.com/watch?v=Txks9hG21qs&feature=youtu.be&t=3345

struct TWN_E014_data* gTWN_E014 = nullptr;

struct TWN_E014_data : public sTownOverlay
{
    TWN_E014_data();
    static void makeCurrent()
    {
        if (gTWN_E014 == NULL)
        {
            gTWN_E014 = new TWN_E014_data();
        }
        gCurrentTownOverlay = gTWN_E014;
    }

    sTownObject* createObjectTaskFromEA_siblingTaskWithEAArgWithCopy(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg) override;
    sTownObject* createObjectTaskFromEA_subTaskWithEAArg(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg) override;
};

static const char* listOfFilesToLoad[] = {
    "COMMON3.MCB",
    "COMMON3.CGB",
    "EVT014MP.MCB",
    "EVT014MP.CGB",
    nullptr
};

void startE014BackgroundTask(p_workArea)
{
    FunctionUnimplemented();

    // hack: to remove title screen
    reinitVdp2();
}

TWN_E014_data::TWN_E014_data() : sTownOverlay("TWN_E014.PRG")
{
    mTownSetups.push_back(readTownSetup(getSaturnPtr(0x0605e114), 1));

    overlayScriptFunctions.m_zeroArg[0x6057724] = &e006_scriptFunction_60573d8;
    overlayScriptFunctions.m_zeroArg[0x6056c64] = &e006_scriptFunction_6056918;
    overlayScriptFunctions.m_zeroArg[0x6057746] = &e006_scriptFunction_605861eSub0;
    overlayScriptFunctions.m_zeroArg[0x6057784] = &e006_scriptFunction_6057438;

    overlayScriptFunctions.m_oneArg[0x605c0d8] = &TwnFadeIn;
    overlayScriptFunctions.m_oneArg[0x605C150] = &TwnFadeOut;
    overlayScriptFunctions.m_oneArg[0x60576FC] = &createEPKPlayer;
    overlayScriptFunctions.m_oneArg[0x605896a] = &setupDragonEntityForCutscene;

    overlayScriptFunctions.m_twoArg[0x605be70] = &townCamera_setup;
}

sTownObject* TWN_E014_data::createObjectTaskFromEA_siblingTaskWithEAArgWithCopy(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg)
{
    assert(definitionEA.m_file == this);
    assert(arg.m_file == this);

    switch (definitionEA.m_offset)
    {
    default:
        assert(0);
        break;
    }
    return nullptr;
}

sTownObject* TWN_E014_data::createObjectTaskFromEA_subTaskWithEAArg(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg)
{
    return nullptr;
}

p_workArea overlayStart_TWN_E014(p_workArea pUntypedThis, u32 arg)
{
    gTWN_E014->makeCurrent();

    townDebugTask2Function* pThis = static_cast<townDebugTask2Function*>(pUntypedThis);

    pThis->m_DeleteMethod = &townOverlayDelete;

    loadSoundBanks(-1, 0);
    loadSoundBanks(68, 0);

    fadeOutAllSequences();

    graphicEngineStatus.m405C.m10_nearClipDistance = 0x800;
    graphicEngineStatus.m405C.m30_oneOverNearClip = FP_Div(0x10000, graphicEngineStatus.m405C.m10_nearClipDistance);

    graphicEngineStatus.m405C.m14_farClipDistance = 0x200000;
    graphicEngineStatus.m405C.m38_oneOverFarClip = FP_Div(0x8000, graphicEngineStatus.m405C.m14_farClipDistance);

    graphicEngineStatus.m405C.m34_oneOverFarClip256 = graphicEngineStatus.m405C.m38_oneOverFarClip << 8;

    initDramAllocator(pThis, townBuffer, sizeof(townBuffer), listOfFilesToLoad);

    initVdp1Ram(pThis, 0x25C18800, 0x63800);

    registerNpcs(gTWN_E014->mTownSetups, gTWN_E014->getSaturnPtr(0x605414c), arg);

    startScriptTask(pThis);

    startE014BackgroundTask(pThis);

    startMainLogic(pThis);

    setupVdp1Proj(0x16c16c1);

    startCameraTask(pThis);

    return pThis;
}
