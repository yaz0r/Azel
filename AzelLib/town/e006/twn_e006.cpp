#include "PDS.h"
#include "twn_e006.h"
#include "town/town.h"
#include "town/townScript.h"
#include "town/townEdge.h"
#include "town/townMainLogic.h"
#include "kernel/vdp1Allocator.h"
#include "kernel/animation.h"

struct TWN_E006_data : public sTownOverlay
{
    void init() override;
    sTownObject* createObjectTaskFromEA_siblingTaskWithEAArgWithCopy(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg) override;
    sTownObject* createObjectTaskFromEA_subTaskWithEAArg(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg) override;
};
TWN_E006_data* gTWN_E006 = nullptr;

void e006_cameraUpdate(sCameraTask* pThis)
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

void e006_cameraDraw(sCameraTask* pThis)
{
    sVec3_FP stack16;
    transformVecByCurrentMatrix(pThis->m14, stack16);
    setupLight(stack16[0], stack16[1], stack16[2], pThis->m10.toU32());
}

s32 scriptFunction_605bb24(s32 r4, s32 r5)
{
    sVec3_FP r4Value = readSaturnVec3(sSaturnPtr::createFromRaw(r4, gTWN_E006)); //todo: that could be a vec2
    sSaturnPtr r5Ptr = sSaturnPtr::createFromRaw(r5, gTWN_E006);
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

    cameraTaskPtr->m_UpdateMethod = e006_cameraUpdate;
    cameraTaskPtr->m_DrawMethod = e006_cameraDraw;

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

struct sStreamingFile_28
{
    s32 m0;
    std::array<s32, 2> m98;
};

struct GfsHn
{
    FILE* fHandle;
};

struct sStreamingFile_198
{
    GfsHn m0_gfsHandle;
    u32 m4_transfertSectorSize;
    u32 m8;
    u32 mC;
    u32 m10;
    u32 m14_numSectors;
    u32 m18_fileSize;
    //size 0x20?
};

struct sStreamingFile
{
    sStreamingFile_28 m28;
    u32 m4C;
    u32 m50;
    u32 m54;
    u32 m58;
    u32 m5C;
    sStreamingFile_198 m198;
    //size 0x1B8?
};

struct sStreamingParams
{
    sStreamingFile* m0_streamingFile;
    s32 m4_streamingFileSize;
    u8* m8_buffer;
    s32 mC_bufferSize;
    u8* m10_audioBuffer;
    s32 m14_audioBufferSize;
};

GfsHn GFS_Open(const std::string& name)
{
    GfsHn temp;
    temp.fHandle = fopen(name.c_str());
    return temp;
}

void GFS_GetFileSize(GfsHn hanfle, s32* sectorSize, s32* numSector, s32* lastSize)
{
    FunctionUnimplemented();
}

sStreamingFile* initStreamingHandle(sStreamingFile* param_1, u8* buffer, u32 bufferSize, u8* audioBuffer, u32 audioBufferSize)
{
    FunctionUnimplemented();
    return param_1;
}

void GFS_SetTransPara(GfsHn hanfle, s32 numSectors)
{
    FunctionUnimplemented();
}

void prereadStreamingData(sStreamingFile*)
{
    FunctionUnimplemented();
}

sStreamingFile* openFileForStreaming(sStreamingParams* param_1, const std::string& param_2)

{
    GfsHn dVar1;
    sStreamingFile* psVar2;
    s32 numSectors;
    s32 sectorSize;
    s32 lastSize;
    s32 local_14;

    dVar1 = GFS_Open(param_2);
    if (dVar1.fHandle == 0) {
        return NULL;
    }
    else {
        psVar2 = initStreamingHandle(param_1->m0_streamingFile, param_1->m8_buffer, param_1->mC_bufferSize, param_1->m10_audioBuffer, param_1->m14_audioBufferSize);
        if (psVar2 != (sStreamingFile*)0x0) {
            (psVar2->m198).m0_gfsHandle = dVar1;
            GFS_GetFileSize(dVar1, &sectorSize, &numSectors, &lastSize);
            (psVar2->m198).m14_numSectors = numSectors;
            (psVar2->m198).m18_fileSize = (numSectors - 1) * sectorSize + (lastSize & ~3);
            (psVar2->m198).m8 = 0;
            (psVar2->m198).m4_transfertSectorSize = 0x14;
            GFS_SetTransPara(dVar1, 0x14);
            prereadStreamingData(psVar2);
            psVar2->m28.m0 = 2;
        }
    }
    return psVar2;
}

s32 e006Task0Var0 = 0;

void getNextStreamingBuffer(sStreamingFile* iParm1, void** pBuffer, int* pBufferSize)
{
    s32 iVar1;
    s32 iVar2;

    if ((iParm1->m5C == iParm1->m54) && (iParm1->m4C != iParm1->m50)) {
        *pBufferSize = 0;
        *pBuffer = (void*)0x0;
        return;
    }
    iVar2 = iParm1->m58 - iParm1->m54;
    if (((u32)iParm1->m54 < (u32)iParm1->m5C) && (iVar1 = iParm1->m5C - iParm1->m54, iVar1 < iVar2))
    {
        iVar2 = iVar1;
    }
    *pBufferSize = iVar2;
    *pBuffer = (void*)iParm1->m54;
    return;
}

void updateStreamingFileRead(sStreamingFile* iParm1)
{
    s32 iVar1;
    s32 nSectors;
    u32 uVar3;
    sStreamingFile_198* psVar4;
    s32 bufferSize;
    void* buffer;

    psVar4 = &iParm1->m198;
    if ((iParm1->m198).m18_fileSize <= iParm1->m4C) {
        return;
    }
    if (((iParm1->m198).m8 == 0) && (getNextStreamingBuffer(iParm1, &buffer, &bufferSize), 0x7ff < bufferSize)) {
        if (bufferSize < (iParm1->m198).m4_transfertSectorSize * 0x800) {
            nSectors = performDivision(0x800, bufferSize);
        }
        else {
            nSectors = (iParm1->m198).m4_transfertSectorSize;
        }
        iVar1 = GFS_NwFread(psVar4->m0_gfsHandle, nSectors, buffer, nSectors * 0x800);
        if (-1 < iVar1) {
            (iParm1->m198).mC = nSectors * 0x800;
            (iParm1->m198).m10 = 0;
            (iParm1->m198).m8 = 1;
        }
        else
        {
            updateStreamingFileReadSub0(iParm1);
            return;
        }
        
    }

    if ((iParm1->m198).m8 == 1) {
        nSectors = GFS_NwExecOne(psVar4->m0_gfsHandle);
        if (nSectors < 0)
        {
            updateStreamingFileReadSub0(iParm1);
            return;
        }

        s32 stat;
        s32 nbyte;
        GFS_NwGetStat(psVar4->m0_gfsHandle, &stat, &nbyte);
        if ((0 < nbyte) && (uVar3 = nbyte - (iParm1->m198).m10 & ~3, 0 < (int)uVar3)) {
            (iParm1->m198).m10 = (iParm1->m198).m10 + uVar3;
            updateStreamingFileReadSub1(iParm1, uVar3);
        }
        if (((iParm1->m198).mC <= nbyte) || (nSectors == 0)) {
            (iParm1->m198).m8 = 0;
        }
    }
    updateStreamingFileReadSub2(iParm1);
}

void updateCutsceneStreaming(sStreamingFile* r4)
{
    updateStreamingFileRead(r4);

    FunctionUnimplemented();
}

void pauseCutscene(sStreamingFile* r4, s32 r5)
{
    FunctionUnimplemented();
}

void cutsceneUpdateInputSub0(sStreamingFile* r4, s32 r5, s32 r6)
{
    r4->m28.m98[0] = r5;
    r4->m28.m98[1] = r6;
}

// TODO: kernel
s32 scriptFunction_605861eSub0Sub0(sStreamingFile* r4)
{
    return r4->m28.m0;
}

struct sE006Task0 : public s_workAreaTemplateWithArg<sE006Task0, s32>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sE006Task0::Init, nullptr, &sE006Task0::Draw, &sE006Task0::Delete };
        return &taskDefinition;
    }

    static void Init(sE006Task0* pThis, s32 arg)
    {
        pThis->m1D4 = dramAllocate(0x20000);

        sStreamingParams streamingParams;
        streamingParams.m0_streamingFile = &pThis->m4;
        streamingParams.m4_streamingFileSize = sizeof(pThis->m4);
        streamingParams.m8_buffer = pThis->m1D4;
        streamingParams.mC_bufferSize = 0x20000;
        streamingParams.m10_audioBuffer = nullptr;//TODO: figure this out
        streamingParams.m14_audioBufferSize = 0x8000;

        std::string streamFileName = readSaturnString(sSaturnPtr::createFromRaw(arg, gTWN_E006));
        findMandatoryFileOnDisc(streamFileName.c_str());
        pThis->m0 = openFileForStreaming(&streamingParams, streamFileName);

    }

    static void cutsceneDrawSub0(sE006Task0* pThis, s32 r5)
    {
        s32 iVar3 = 0x400;
        if (g_fadeControls.m0_fade0.m20_stopped == 0)
        {
            if (scriptFunction_605861eSub0Sub0(pThis->m0) == 1)
            {
                vdp2DebugPrintSetPosition(0x13, 0xd);
                drawLineLargeFont("     ");
                pauseEngine[0] = 0;
                pauseCutscene(pThis->m0, 1);
            }
        }
        else {
            if ((graphicEngineStatus.m4514.m0_inputDevices[1].m0_current.m8_newButtonDown & 8) != 0) {
                u8 bVar1 = pauseEngine[0] == 0;
                if (bVar1) {
                    vdp2DebugPrintSetPosition(0x13, 0xd);
                    drawLineLargeFont("PAUSE");
                }
                else {
                    vdp2DebugPrintSetPosition(0x13, 0xd);
                    drawLineLargeFont("     ");
                }
                pauseEngine[0] = bVar1;
                pauseCutscene(pThis->m0, !bVar1);
            }
            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m0_inputType == 2) {
                if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m4 == 0) {
                    if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m5 != 0) {
                        iVar3 = 0x400 - ((int)(((int)graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m5 & 0xffU) * 5) >> 1);
                    }
                }
                else {
                    iVar3 = ((int)graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m4 & 0xffU) * (r5 + -1) * 4 + 0x400;
                }
            }
            else {
                if (((int)(short)graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & 0x8000U) == 0) {
                    if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & 0x800) != 0) {
                        iVar3 = 0x155;
                    }
                }
                else {
                    iVar3 = r5 * 0x3fd;
                }
            }
        }
        cutsceneUpdateInputSub0(pThis->m0, iVar3, 1);
    }

    static void Draw(sE006Task0* pThis)
    {

        if ((g_fadeControls.m0_fade0.m20_stopped != 0) &&
            (((readKeyboardToggle(0x87) != 0 && ((e006Task0Var0 & 4) == 0)) ||
            ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 8) != 0))))
        {
            npcData0.mF0 = 1;
        }

        cutsceneDrawSub0(pThis, 7);
        updateCutsceneStreaming(pThis->m0);
        if (scriptFunction_605861eSub0Sub0(pThis->m0) == 5)
        {
            pThis->getTask()->markFinished();
        }
    }

    static void Delete(sE006Task0* pThis)
    {
        FunctionUnimplemented();
    }

    sStreamingFile* m0;
    sStreamingFile m4;
    u8* m1D4;
    // size: 0x1d8
};

sE006Task0* e006Task0 = nullptr;

s32 scriptFunction_60573b0(s32 r4)
{
    npcData0.mF4 = 1;
    npcData0.mF0 = 0;

    e006Task0 = createSubTaskWithArg<sE006Task0>(twnMainLogicTask, r4);

    return 0;
}

s32 scriptFunction_605861eSub0()
{
    if ((e006Task0 != nullptr) && (npcData0.mF0 == 0))
    {
        s32 iVar1 = scriptFunction_605861eSub0Sub0(e006Task0->m0);
        if ((iVar1 != 5) && (iVar1 != -1))
            return 0;
    }

    return 1;
}

struct sE006Task1 : public s_workAreaTemplate<sE006Task1>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { nullptr, &sE006Task1::Update, &sE006Task1::Draw, &sE006Task1::Delete };
        return &taskDefinition;
    }

    static void Update(sE006Task1* pThis)
    {
        FunctionUnimplemented();
    }

    static void Draw(sE006Task1* pThis)
    {
        FunctionUnimplemented();
    }

    static void Delete(sE006Task1* pThis)
    {
        FunctionUnimplemented();
    }

    sSaturnPtr m0;
    s32 m8;
    // size: 0xc
};

// TODO: kernel
void setupCutsceneDragonSub0(sE006Task0* r4, u32 r5, u32 r6, sE006Task1* r7, s_3dModel* pModel)
{
    FunctionUnimplemented();
}


// TODO: kernel
void setupCutsceneDragon(sE006Task0* r4, u32 r5, sE006Task1* r6, s_3dModel* pModel)
{
    setupCutsceneDragonSub0(r4, r5, 3, r6, pModel);
}

s32 scriptFunction_605861e(s32 r4)
{
    sSaturnPtr r4Ptr = sSaturnPtr::createFromRaw(r4, gTWN_E006);
    if (scriptFunction_605861eSub0() == 0)
    {
        sE006Task1* pNewSubTask = createSubTask<sE006Task1>(currentResTask);
        pNewSubTask->m0 = r4Ptr;
        pNewSubTask->m8 = 0x80000000;

        setupCutsceneDragon(e006Task0, readSaturnU8(r4Ptr + 1), pNewSubTask, &gDragonState->m28_dragon3dModel);
        npcData0.m70_npcPointerArray[readSaturnU8(r4Ptr + 0)].workArea = pNewSubTask;
        setupModelAnimation(&gDragonState->m28_dragon3dModel, nullptr);
    }
    return 0;
}

s32 scriptFunction_605838C(s32 r4)
{
    if (g_fadeControls.m_4D >= g_fadeControls.m_4C)
    {
        vdp2Controls.m20_registers[0].m112_CLOFSL = r4;
        vdp2Controls.m20_registers[1].m112_CLOFSL = r4;
    }

    return 0;
}

s32 scriptFunction_60573d8()
{
    FunctionUnimplemented();
    return 0;
}

s32 scriptFunction_6056918()
{
    twnMainLogicTask->m_UpdateMethod = nullptr;
    twnMainLogicTask->m_DrawMethod = nullptr;
    return 0;
}

s32 scriptFunction_6056926()
{
    twnMainLogicTask->m_UpdateMethod = &sMainLogic::Update;
    twnMainLogicTask->m_DrawMethod = &sMainLogic::Draw;
    return 0;
}

s32 scriptFunction_6057438()
{
    FunctionUnimplemented();
    return 0;
}

s32 scriptFunction_6057470()
{
    FunctionUnimplemented();
    return 0;
}

void TWN_E006_data::init()
{
    gCurrentTownOverlay = this;

    overlayScriptFunctions.m_zeroArg[0x60573d8] = &scriptFunction_60573d8;
    overlayScriptFunctions.m_zeroArg[0x6056918] = &scriptFunction_6056918;
    overlayScriptFunctions.m_zeroArg[0x6057438] = &scriptFunction_6057438;
    overlayScriptFunctions.m_zeroArg[0x6057470] = &scriptFunction_6057470;
    overlayScriptFunctions.m_zeroArg[0x6056926] = &scriptFunction_6056926;

    overlayScriptFunctions.m_oneArg[0x60573b0] = &scriptFunction_60573b0;
    overlayScriptFunctions.m_oneArg[0x605861e] = &scriptFunction_605861e;
    overlayScriptFunctions.m_oneArg[0x605838C] = &scriptFunction_605838C;
    overlayScriptFunctions.m_oneArg[0x605be04] = &TwnFadeOut;
    overlayScriptFunctions.m_oneArg[0x605bd8c] = &TwnFadeIn;

    overlayScriptFunctions.m_twoArg[0x605bb24] = &scriptFunction_605bb24;
}

sTownObject* TWN_E006_data::createObjectTaskFromEA_siblingTaskWithEAArgWithCopy(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg)
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

sTownObject* TWN_E006_data::createObjectTaskFromEA_subTaskWithEAArg(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg)
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
    "EXCAVAMP.MCB",
    "EXCAVAMP.CGB",
    nullptr
};

static void startE006BackgroundTask(p_workArea pThis)
{
    FunctionUnimplemented();

    // hack: to remove title screen
    reinitVdp2();
}

void setupVdp1Proj(fixedPoint fov)
{
    initVDP1Projection(fov, 0);
}

p_workArea overlayStart_TWN_E006(p_workArea pUntypedThis, u32 arg)
{
    // load data
    if (gTWN_E006 == NULL)
    {
        FILE* fHandle = fopen("TWN_E006.PRG", "rb");
        assert(fHandle);

        fseek(fHandle, 0, SEEK_END);
        u32 fileSize = ftell(fHandle);

        fseek(fHandle, 0, SEEK_SET);
        u8* fileData = new u8[fileSize];
        fread(fileData, fileSize, 1, fHandle);
        fclose(fHandle);

        gTWN_E006 = new TWN_E006_data();
        gTWN_E006->m_name = "TWN_E006.PRG";
        gTWN_E006->m_data = fileData;
        gTWN_E006->m_dataSize = fileSize;
        gTWN_E006->m_base = 0x6054000;

        gTWN_E006->init();
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

    registerNpcs(gTWN_E006->getSaturnPtr(0x605e1c0), gTWN_E006->getSaturnPtr(0x605414c), arg);

    startScriptTask(pThis);

    startE006BackgroundTask(pThis);

    startMainLogic(pThis);

    setupVdp1Proj(0x1c71c71);

    startCameraTask(pThis);

    return pThis;
}
