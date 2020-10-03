#include "PDS.h"
#include "twn_e011.h"
#include "town/e006/twn_e006.h" // TODO cleanup
#include "town/town.h"
#include "town/townScript.h"
#include "town/townEdge.h"
#include "town/townMainLogic.h"
#include "kernel/vdp1Allocator.h"
#include "kernel/animation.h"
#include "kernel/fileBundle.h"
#include "kernel/graphicalObject.h"
#include "mainMenuDebugTasks.h"
#include "audio/soundDriver.h"
#include "town/townCamera.h"

// https://youtu.be/Txks9hG21qs?t=2980

void setupVdp1Proj(fixedPoint fov); // TODO: cleanup

struct TWN_E011_data* gTWN_E011 = nullptr;

struct TWN_E011_data : public sTownOverlay
{
    TWN_E011_data();
    static void makeCurrent()
    {
        if (gTWN_E011 == NULL)
        {
            gTWN_E011 = new TWN_E011_data();
        }
        gCurrentTownOverlay = gTWN_E011;
    }

    sTownObject* createObjectTaskFromEA_siblingTaskWithEAArgWithCopy(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg) override;
    sTownObject* createObjectTaskFromEA_subTaskWithEAArg(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg) override;
};

u32 modulateColor(sSaturnPtr r4, u32 r5)
{
    Unimplemented();
    return 0;
}

u32 modulateColorByEvent(sCameraTask* cameraTaskPtr, s32 r5)
{
    Unimplemented();
    return 0;
}

struct sE011Task1 : public s_workAreaTemplate<sE011Task1>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { nullptr, &sE011Task1::Update, &sE011Task1::Draw, &sE011Task1::Delete };
        return &taskDefinition;
    }

    static void Update(sE011Task1* pThis)
    {
        sSaturnPtr dragonDataTable = sSaturnPtr::createFromRaw(0x06062078, gTWN_E011);
        sSaturnPtr dragonDataTableEntry = readSaturnEA(dragonDataTable + 4 * gDragonState->mC_dragonType);
        sSaturnPtr pcVar1 = (dragonDataTableEntry + readSaturnU8(pThis->m8 + 3) * 2);

        transformAndAddVec(gDragonState->m28_dragon3dModel.m44_hotpointData[readSaturnU8(pcVar1)][readSaturnU8(pcVar1 + 1)], pThis->m5C, cameraProperties2.m28[0]);
    }

    static void Draw(sE011Task1* pThis)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(pThis->m5C);
        pThis->mC_3dModel.m2C_poseData[0].m0_translation.zeroize();
        pThis->mC_3dModel.m18_drawFunction(&pThis->mC_3dModel);
        popMatrix();
    }

    static void Delete(sE011Task1* pThis)
    {
        Unimplemented();
    }

    sSaturnPtr m8;
    s_3dModel mC_3dModel;
    sVec3_FP m5C;
    // 0x6C
};

s32 e011_scriptFunction_6059af0(s32 r4)
{
    sSaturnPtr r4Ptr = sSaturnPtr::createFromRaw(r4, gCurrentTownOverlay);
    if (e006_scriptFunction_605861eSub0() == 0)
    {
        sE011Task1* pNewSubTask = createSubTask<sE011Task1>(currentResTask);
        s_fileBundle* pBundle = dramAllocatorEnd[readSaturnU8(r4Ptr)].mC_fileBundle->m0_fileBundle;
        pNewSubTask->m8 = r4Ptr;

        s16 hierarchyIndex = readSaturnS16(r4Ptr + 4);
        sModelHierarchy* pHierarchy = pBundle->getModelHierarchy(hierarchyIndex);
        sStaticPoseData* pPose = pBundle->getStaticPose(readSaturnU16(r4Ptr + 6), pHierarchy->countNumberOfBones());
        //pHierarchy->patchFilePointers(dramAllocatorEnd[readSaturnU8(r4Ptr)].mC_buffer->m4_vd1Allocation->m4_vdp1Memory);
        init3DModelRawData(pNewSubTask, &pNewSubTask->mC_3dModel, 0, pBundle, hierarchyIndex, nullptr, pPose, nullptr, nullptr);

        npcData0.m70_npcPointerArray[readSaturnU8(r4Ptr + 1)].workArea = pNewSubTask;
        setupCutsceneModelType2(e006Task0->m0, readSaturnU8(r4Ptr + 2), pNewSubTask, &pNewSubTask->mC_3dModel);
    }
    return 0;
}

s32 e011_scriptFunction_06059b7a(s32 param1)
{
    if ((e006_scriptFunction_605861eSub0() == 0) && (npcData0.m70_npcPointerArray[param1].workArea != nullptr))
    {
        npcData0.m70_npcPointerArray[param1].workArea->getTask()->markFinished();
        npcData0.m70_npcPointerArray[param1].workArea = nullptr;
    }

    return 0xBADF00D;
}

TWN_E011_data::TWN_E011_data() : sTownOverlay("TWN_E011.PRG")
{
    overlayScriptFunctions.m_zeroArg[0x6058484] = &e006_scriptFunction_60573d8;
    overlayScriptFunctions.m_zeroArg[0x60579c4] = &e006_scriptFunction_6056918;
    overlayScriptFunctions.m_zeroArg[0x60584a6] = &e006_scriptFunction_605861eSub0;

    overlayScriptFunctions.m_oneArg[0x605ceb0] = &TwnFadeOut;
    overlayScriptFunctions.m_oneArg[0x605ce38] = &TwnFadeIn;
    overlayScriptFunctions.m_oneArg[0x605845c] = &createEPKPlayer;
    overlayScriptFunctions.m_oneArg[0x6059af0] = &e011_scriptFunction_6059af0;
    overlayScriptFunctions.m_oneArg[0x60596ca] = &setupDragonEntityForCutscene;
    overlayScriptFunctions.m_oneArg[0x6059b7a] = &e011_scriptFunction_06059b7a;

    overlayScriptFunctions.m_twoArg[0x605cbd0] = &townCamera_setup;

    mTownSetups.push_back(readTownSetup(getSaturnPtr(0x0605ef14), 1));
    mTownSetups.push_back(readTownSetup(getSaturnPtr(0x0605ef20), 1));
}

sTownObject* TWN_E011_data::createObjectTaskFromEA_siblingTaskWithEAArgWithCopy(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg)
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

sTownObject* TWN_E011_data::createObjectTaskFromEA_subTaskWithEAArg(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg)
{
    return nullptr;
}

static const char* listOfFilesToLoad[] = {
    "COMMON3.MCB",
    "COMMON3.CGB",
    "EVT011MP.MCB",
    "EVT011MP.CGB",
    nullptr
};

static void startE011BackgroundTask(p_workArea pThis)
{
    Unimplemented();

    // hack: to remove title screen
    reinitVdp2();
}

p_workArea overlayStart_TWN_E011(p_workArea pUntypedThis, u32 arg)
{
    gTWN_E011->makeCurrent();
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

    registerNpcs(gTWN_E011->mTownSetups, gTWN_E011->getSaturnPtr(0x605414c), arg);

    startScriptTask(pThis);

    startE011BackgroundTask(pThis);

    startMainLogic(pThis);

    setupVdp1Proj(0x1c71c71);

    startCameraTask(pThis);

    return pThis;
}
