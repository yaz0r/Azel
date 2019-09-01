#include "PDS.h"
#include "twn_e011.h"
#include "town/e006/twn_e006.h"
#include "town/town.h"
#include "town/townScript.h"
#include "town/townEdge.h"
#include "town/townMainLogic.h"
#include "kernel/vdp1Allocator.h"
#include "kernel/animation.h"
#include "kernel/fileBundle.h"
#include "mainMenuDebugTasks.h"

struct TWN_E011_data : public sTownOverlay
{
    void init() override;
    sTownObject* createObjectTaskFromEA_siblingTaskWithEAArgWithCopy(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg) override;
    sTownObject* createObjectTaskFromEA_subTaskWithEAArg(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg) override;
};
TWN_E011_data* gTWN_E011 = nullptr;

// TODO: shared in EPKs
void e011_cameraUpdate(sCameraTask* pThis)
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
void e011_cameraDraw(sCameraTask* pThis)
{
    sVec3_FP stack16;
    transformVecByCurrentMatrix(pThis->m14, stack16);
    setupLight(stack16[0], stack16[1], stack16[2], pThis->m10.toU32());
}

u32 modulateColor(sSaturnPtr r4, u32 r5)
{
    FunctionUnimplemented();
    return 0;
}

u32 modulateColorByEvent(sCameraTask* cameraTaskPtr, s32 r5)
{
    FunctionUnimplemented();
    return 0;
}

s32 e011_scriptFunction_0605ce38(int iParm1)

{
    u32 unaff_r12;

    switch (cameraTaskPtr->m0)
    {
    case 0:
    case 2:
        unaff_r12 = modulateColor(cameraTaskPtr->m8, cameraTaskPtr->m30);
        break;
    case 1:
        unaff_r12 = modulateColorByEvent(cameraTaskPtr, cameraTaskPtr->m4 + iParm1);
        break;
    default:
        assert(0);
        break;
    }
    
    fadePalette(&g_fadeControls.m0_fade0, convertColorToU32(g_fadeControls.m0_fade0.m0_color), 0xc210, iParm1);
    fadePalette(&g_fadeControls.m24_fade1, convertColorToU32(g_fadeControls.m24_fade1.m0_color), unaff_r12, iParm1);
    cameraTaskPtr->m1 = 1;
    return 1;
}

s32 scriptFunction_605cbd0(s32 r4, s32 r5)
{
    sVec3_FP r4Value = readSaturnVec3(sSaturnPtr::createFromRaw(r4, gTWN_E011)); //todo: that could be a vec2
    sSaturnPtr r5Ptr = sSaturnPtr::createFromRaw(r5, gTWN_E011);
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

    cameraTaskPtr->m_UpdateMethod = e011_cameraUpdate;
    cameraTaskPtr->m_DrawMethod = e011_cameraDraw;

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

        transformAndAddVec(gDragonState->m28_dragon3dModel.m44[readSaturnU8(pcVar1)][readSaturnU8(pcVar1 + 1)], pThis->m5C, cameraProperties2.m28[0]);
    }

    static void Draw(sE011Task1* pThis)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(pThis->m5C);
        pThis->mC_3dModel.m2C_poseData[0].m0_translation.zero();
        pThis->mC_3dModel.m18_drawFunction(&pThis->mC_3dModel);
        popMatrix();
    }

    static void Delete(sE011Task1* pThis)
    {
        FunctionUnimplemented();
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
        s_fileBundle* pBundle = dramAllocatorEnd[readSaturnU8(r4Ptr)].mC_buffer->m0_dramAllocation;
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

void TWN_E011_data::init()
{
    gCurrentTownOverlay = this;

    overlayScriptFunctions.m_zeroArg[0x6058484] = &e006_scriptFunction_60573d8;
    overlayScriptFunctions.m_zeroArg[0x60579c4] = &e006_scriptFunction_6056918;
    overlayScriptFunctions.m_zeroArg[0x60584a6] = &e006_scriptFunction_605861eSub0;

    overlayScriptFunctions.m_oneArg[0x605ceb0] = &TwnFadeOut;
    overlayScriptFunctions.m_oneArg[0x605845c] = &createEPKPlayer;
    overlayScriptFunctions.m_oneArg[0x6059af0] = &e011_scriptFunction_6059af0;
    overlayScriptFunctions.m_oneArg[0x60596ca] = &setupDragonEntityForCutscene;
    overlayScriptFunctions.m_oneArg[0x605ce38] = &e011_scriptFunction_0605ce38;
    overlayScriptFunctions.m_oneArg[0x6059b7a] = &e011_scriptFunction_06059b7a;

    overlayScriptFunctions.m_twoArg[0x605cbd0] = &scriptFunction_605cbd0;
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

static void townOverlayDelete(townDebugTask2Function* pThis)
{
    FunctionUnimplemented();
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
    FunctionUnimplemented();

    // hack: to remove title screen
    reinitVdp2();
}

void setupVdp1Proj(fixedPoint fov);

p_workArea overlayStart_TWN_E011(p_workArea pUntypedThis, u32 arg)
{
    // load data
    if (gTWN_E011 == NULL)
    {
        FILE* fHandle = fopen("TWN_E011.PRG", "rb");
        assert(fHandle);

        fseek(fHandle, 0, SEEK_END);
        u32 fileSize = ftell(fHandle);

        fseek(fHandle, 0, SEEK_SET);
        u8* fileData = new u8[fileSize];
        fread(fileData, fileSize, 1, fHandle);
        fclose(fHandle);

        gTWN_E011 = new TWN_E011_data();
        gTWN_E011->m_name = "TWN_E011.PRG";
        gTWN_E011->m_data = fileData;
        gTWN_E011->m_dataSize = fileSize;
        gTWN_E011->m_base = 0x6054000;

        gTWN_E011->init();
    }
    townDebugTask2Function* pThis = static_cast<townDebugTask2Function*>(pUntypedThis);

    pThis->m_DeleteMethod = &townOverlayDelete;

    playMusic(-1, 0);
    playMusic(68, 0);

    stopAllSounds();

    graphicEngineStatus.m405C.m10_nearClipDistance = 0x800;
    graphicEngineStatus.m405C.m30 = FP_Div(0x10000, graphicEngineStatus.m405C.m10_nearClipDistance);

    graphicEngineStatus.m405C.m14_farClipDistance = 0x200000;
    graphicEngineStatus.m405C.m38 = FP_Div(0x8000, graphicEngineStatus.m405C.m14_farClipDistance);

    graphicEngineStatus.m405C.m34 = graphicEngineStatus.m405C.m38 << 8;

    initDramAllocator(pThis, townBuffer, sizeof(townBuffer), listOfFilesToLoad);

    initVdp1Ram(pThis, 0x25C18800, 0x63800);

    registerNpcs(gTWN_E011->getSaturnPtr(0x0605ef14), gTWN_E011->getSaturnPtr(0x605414c), arg);

    startScriptTask(pThis);

    startE011BackgroundTask(pThis);

    startMainLogic(pThis);

    setupVdp1Proj(0x1c71c71);

    startCameraTask(pThis);

    return pThis;
}
