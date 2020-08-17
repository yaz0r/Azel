#include "PDS.h"
#include "twn_e014.h"
#include "town/town.h"
#include "audio/soundDriver.h"
#include "kernel/vdp1Allocator.h"
#include "town/townMainLogic.h"

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

static void townOverlayDelete(townDebugTask2Function* pThis)
{
    FunctionUnimplemented();
}

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

// TODO: shared in EPKs
void e014_cameraUpdate(sCameraTask* pThis)
{
    if ((npcData0.mFC & 1) == 0)
    {
        pThis->m4++;
        if (pThis->m4 > 5400)
        {
            pThis->m4 = 5400;
        }
    }
}

// TODO: shared in EPKs
void e014_cameraDraw(sCameraTask* pThis)
{
    sVec3_FP stack16;
    transformVecByCurrentMatrix(pThis->m14, stack16);
    setupLight(stack16[0], stack16[1], stack16[2], pThis->m10.toU32());
}

s32 scriptFunction_605be70(s32 r4, s32 r5)
{
    sVec3_FP r4Value = readSaturnVec3(sSaturnPtr::createFromRaw(r4, gTWN_E014)); //todo: that could be a vec2
    sSaturnPtr r5Ptr = sSaturnPtr::createFromRaw(r5, gTWN_E014);
    cameraTaskPtr->m8 = r5Ptr;

    sMatrix4x3 var4;
    initMatrixToIdentity(&var4);
    rotateMatrixShiftedY(r4Value[1], &var4);
    rotateMatrixShiftedX(r4Value[0], &var4);

    cameraTaskPtr->m14[0] = var4.matrix[3];
    cameraTaskPtr->m14[1] = var4.matrix[7];
    cameraTaskPtr->m14[2] = var4.matrix[11];

    cameraTaskPtr->m10 = readSaturnRGB8(r5Ptr);
    cameraTaskPtr->m30 = 0x8000;

    generateLightFalloffMap(readSaturnRGB8(r5Ptr + 3).toU32(), readSaturnRGB8(r5Ptr + 6).toU32(), readSaturnRGB8(r5Ptr + 9).toU32());

    cameraTaskPtr->m_UpdateMethod = e014_cameraUpdate;
    cameraTaskPtr->m_DrawMethod = e014_cameraDraw;

    if (g_fadeControls.m_4C <= g_fadeControls.m_4D)
    {
        vdp2Controls.m20_registers[0].m112_CLOFSL = 0x10;
        vdp2Controls.m20_registers[1].m112_CLOFSL = 0x10;
    }

    resetProjectVector();
    cameraTaskPtr->m2 = 0;
    cameraTaskPtr->m0 = 0;
    return 0;
}

s32 e011_scriptFunction_0605ce38(int iParm1); // TODO: clean

TWN_E014_data::TWN_E014_data() : sTownOverlay("TWN_E014.PRG")
{
    mTownSetups.push_back(readTownSetup(getSaturnPtr(0x0605e114), 1));

    overlayScriptFunctions.m_zeroArg[0x6057724] = &e006_scriptFunction_60573d8;
    overlayScriptFunctions.m_zeroArg[0x6056c64] = &e006_scriptFunction_6056918;
    overlayScriptFunctions.m_zeroArg[0x6057746] = &e006_scriptFunction_605861eSub0;
    overlayScriptFunctions.m_zeroArg[0x6057784] = &e006_scriptFunction_6057438;

    overlayScriptFunctions.m_oneArg[0x605C150] = &TwnFadeOut;
    overlayScriptFunctions.m_oneArg[0x60576FC] = &createEPKPlayer;

    overlayScriptFunctions.m_twoArg[0x605be70] = &scriptFunction_605be70;
    overlayScriptFunctions.m_oneArg[0x605896a] = &setupDragonEntityForCutscene;
    overlayScriptFunctions.m_oneArg[0x605c0d8] = &e011_scriptFunction_0605ce38;
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
