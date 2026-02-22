#include "PDS.h"
#include "twn_e006.h"
#include "town/town.h"
#include "town/townScript.h"
#include "town/townEdge.h"
#include "town/townMainLogic.h"
#include "kernel/vdp1Allocator.h"
#include "kernel/animation.h"
#include "kernel/fileBundle.h"
#include "audio/soundDriver.h"
#include "audio/systemSounds.h"
#include "town/townCamera.h"

struct TWN_E006_data* gTWN_E006 = nullptr;
struct TWN_E006_data : public sTownOverlay
{
    TWN_E006_data();
    static void makeCurrent()
    {
        if (gTWN_E006 == NULL)
        {
            gTWN_E006 = new TWN_E006_data();
        }
        gCurrentTownOverlay = gTWN_E006;
    }

    sTownObject* createObjectTaskFromEA_siblingTaskWithEAArgWithCopy(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg) override;
    sTownObject* createObjectTaskFromEA_subTaskWithEAArg(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg) override;
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
    temp.fHandle = fopen(name.c_str(), "rb");
    return temp;
}

void GFS_GetFileSize(GfsHn handle, s32* sectorSize, s32* numSector, s32* lastSize)
{
    u32 position = ftell(handle.fHandle);

    fseek(handle.fHandle, 0, SEEK_END);
    u32 size = ftell(handle.fHandle);

    fseek(handle.fHandle, position, SEEK_SET);

    *sectorSize = 0x800;
    *numSector = size / 0x800;
    *lastSize = size - (*numSector * 0x800);
}

s32 numReadBytes = 0;

s32 gNumSectors = 0;

s32 GFS_NwExecOne(GfsHn hanle)
{
    return 1;
}

void GFS_NwGetStat(GfsHn handle, s32* stat, s32* nbytes)
{
    *nbytes = numReadBytes;
    Unimplemented();
}

s32 GFS_NwFread(GfsHn handle, s32 numSectors, u8* buffer, s32 bufferSize)
{
    gNumSectors = numSectors;
    assert(bufferSize >= numSectors * 0x800);
    numReadBytes = numSectors * 0x800;
    return fread(buffer, numSectors, 0x800, handle.fHandle);
}

void GFS_NwCdRead(GfsHn handle)
{
    Unimplemented();
}

sStreamingFile* initStreamingHandle(sStreamingFile* param_1, u8* buffer, u32 bufferSize, u8* audioBuffer, u32 audioBufferSize)
{
    param_1->m0_buffer = buffer;
    param_1->m4_bufferSize = bufferSize;
    addToMemoryLayout(buffer, bufferSize);
    param_1->m8 = 0;
    param_1->m10 = 0;
    param_1->m14 = 0;
    param_1->m18_audioBuffer = audioBuffer;
    param_1->m1C_audioBufferSize = audioBufferSize;
    param_1->m20 = 0;
    param_1->m24 = 7;
    param_1->m194 = param_1->m0_buffer;
    (param_1->m28).m0 = 0;
    (param_1->m28).m4 = 0;
    (param_1->m28).m8 = 0;
    (param_1->m28).mC = 0;
    (param_1->m28).m10 = 0;
    (param_1->m28).m1C = 0;
    param_1->m28.m20_vdp2StringControl = pVdp2StringControl;
    (param_1->m28).m28 = 0;
    (param_1->m28).m24_currentPositionInFile = 0;
    u8* puVar2 = param_1->m0_buffer;
    (param_1->m28).m34_pBufferRead = puVar2;
    (param_1->m28).m2C_pBufferWrite = puVar2;
    u32 uVar1 = param_1->m4_bufferSize & 0x7ff;
    (param_1->m28).m4C = uVar1 + 0x1800;
    if (param_1->m4_bufferSize <= uVar1 + 0x2000) {
        return (sStreamingFile*)0x0;
    }
    (param_1->m28).m48 = param_1->m4_bufferSize - (param_1->m28).m4C;
    (param_1->m28).m3C = param_1->m0_buffer;
    puVar2 = (param_1->m28).m3C + (param_1->m28).m48;
    (param_1->m28).m40 = puVar2;
    (param_1->m28).m30 = puVar2;
    (param_1->m28).m38 = puVar2;
    (param_1->m28).m50 = 0;
    (param_1->m28).m5C_audioBuffer = param_1->m18_audioBuffer;
    (param_1->m28).m64_sampleIndex = 0;
    (param_1->m28).m68_writePosition = 0;
    (param_1->m28).m6C = 0;
    (param_1->m28).m84_frameIndex = 0;
    (param_1->m28).m88 = 0;
    param_1->m18C = 0;

    param_1->m190_emptyBytes = 0;
    (param_1->m28).m90 = 0x400;
    (param_1->m28).m94 = 1;
    (param_1->m28).m98[0] = 0;
    (param_1->m28).m98[1] = 0;

    for(int i=0; i<0x10; i++)
    {
        param_1->mC8_cutsceneEntities[i].m0_compressionType = 0;
        param_1->mC8_cutsceneEntities[i].m4_pOwner = 0;
        param_1->mC8_cutsceneEntities[i].m8_p3dModel = 0;
    }
    return param_1;
}

void GFS_SetTransPara(GfsHn hanfle, s32 numSectors)
{
    Unimplemented();
}

void prereadStreamingData(sStreamingFile* param_1)
{
    if (0 < (int)(param_1->m198).m14_numSectors) {
        GFS_NwCdRead((param_1->m198).m0_gfsHandle);
    }
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
        if (psVar2 != nullptr) {
            (psVar2->m198).m0_gfsHandle = dVar1;
            GFS_GetFileSize(dVar1, &sectorSize, &numSectors, &lastSize);
            (psVar2->m198).m14_numSectors = numSectors;
            (psVar2->m198).m18_fileSize = (numSectors - 1) * sectorSize + (lastSize & ~3);
            (psVar2->m198).m8_readIsPending = 0;
            (psVar2->m198).m4_transfertSectorSize = 0x14;
            GFS_SetTransPara(dVar1, 0x14);
            prereadStreamingData(psVar2);
            psVar2->m28.m0 = 2;
        }
    }
    return psVar2;
}

s32 e006Task0Var0 = 0;

void getNextStreamingBuffer(sStreamingFile* iParm1, u8** pBuffer, int* pBufferSize)
{
    s32 iVar1;
    s32 iVar2;

    if ((iParm1->m28.m34_pBufferRead == iParm1->m28.m2C_pBufferWrite) && (iParm1->m28.m24_currentPositionInFile != iParm1->m28.m28)) {
        *pBufferSize = 0;
        *pBuffer = nullptr;
        return;
    }
    iVar2 = iParm1->m28.m30 - iParm1->m28.m2C_pBufferWrite;
    if (((intptr_t)iParm1->m28.m2C_pBufferWrite < (intptr_t)iParm1->m28.m34_pBufferRead) && (iVar1 = iParm1->m28.m34_pBufferRead - iParm1->m28.m2C_pBufferWrite, iVar1 < iVar2))
    {
        iVar2 = iVar1;
    }
    *pBufferSize = iVar2;
    *pBuffer = iParm1->m28.m2C_pBufferWrite;
    return;
}

void updateStreamingFileReadSub0(sStreamingFile* iParm1)
{
    Unimplemented();
}

void incrementReadPosition(sStreamingFile* param_1, s32 param_2)
{
    (param_1->m28).m24_currentPositionInFile += param_2;
    (param_1->m28).m2C_pBufferWrite += param_2;
}

void handleReadBufferOverflow(sStreamingFile* param_1)
{
    u8* puVar1;

    puVar1 = (param_1->m28).m2C_pBufferWrite;
    if ((((param_1->m28).m30 <= puVar1) && ((param_1->m28).m34_pBufferRead <= puVar1)) && (2 < (param_1->m28).m0))
    {
        (param_1->m28).m2C_pBufferWrite = (param_1->m28).m3C;
        (param_1->m28).m30 = (param_1->m28).m3C + ((param_1->m28).m48 & ~0x7FF);
    }
    return;
}

void updateStreamingFileRead(sStreamingFile* iParm1)
{
    s32 iVar1;
    sStreamingFile_198* psVar4;
    s32 bufferSize;
    u8* buffer;

    psVar4 = &iParm1->m198;
    if ((iParm1->m198).m18_fileSize <= iParm1->m28.m24_currentPositionInFile) {
        return;
    }
    if (((iParm1->m198).m8_readIsPending == 0) && (getNextStreamingBuffer(iParm1, &buffer, &bufferSize), 0x7ff < bufferSize)) {
        s32 numSectorsToRead;
        if (bufferSize < (iParm1->m198).m4_transfertSectorSize * 0x800) {
            numSectorsToRead = performDivision(0x800, bufferSize);
        }
        else {
            numSectorsToRead = (iParm1->m198).m4_transfertSectorSize;
        }
        iVar1 = GFS_NwFread(psVar4->m0_gfsHandle, numSectorsToRead, buffer, numSectorsToRead * 0x800);
        if (-1 < iVar1) {
            (iParm1->m198).mC_pendingReadSize = numSectorsToRead * 0x800;
            (iParm1->m198).m10_numBytesReadForCurrentRequest = 0;
            (iParm1->m198).m8_readIsPending = 1;
        }
        else
        {
            updateStreamingFileReadSub0(iParm1);
            return;
        }
        
    }

    if ((iParm1->m198).m8_readIsPending == 1) {
        s32 completed = GFS_NwExecOne(psVar4->m0_gfsHandle);
        if (completed < 0)
        {
            updateStreamingFileReadSub0(iParm1);
            return;
        }

        s32 stat;
        s32 nbyte;
        GFS_NwGetStat(psVar4->m0_gfsHandle, &stat, &nbyte);

        u32 numBytesReadForCurrentRequest = nbyte - (iParm1->m198).m10_numBytesReadForCurrentRequest & ~3;
        if ((0 < nbyte) && (0 < numBytesReadForCurrentRequest)) {
            (iParm1->m198).m10_numBytesReadForCurrentRequest += numBytesReadForCurrentRequest;
            incrementReadPosition(iParm1, numBytesReadForCurrentRequest);
        }

        if (((iParm1->m198).mC_pendingReadSize <= nbyte) || (completed == 0)) {
            (iParm1->m198).m8_readIsPending = 0;
        }
    }
    handleReadBufferOverflow(iParm1);
}

void executeCutsceneCommandsSub0Sub0(sStreamingFile* pThis, int param_2)
{
    (pThis->m28).m28 += param_2;
    (pThis->m28).m34_pBufferRead += param_2;
    if ((pThis->m28).m38 <= (pThis->m28).m34_pBufferRead) {
        (pThis->m28).m34_pBufferRead = (pThis->m28).m34_pBufferRead + (intptr_t)((pThis->m28).m3C + -(intptr_t)(pThis->m28).m38);
        (pThis->m28).m38 = (pThis->m28).m30;
    }
    return;
}

s32 executeCutsceneCommandsSub0(sStreamingFile* pThis)
{
    if (pThis->m28.m48 < pThis->m28.m44_headerSize)
    {
        return -1;
    }
    pThis->m28.m44_headerSize = READ_BE_U32(pThis->m194);
    if (pThis->m4_bufferSize < pThis->m28.m44_headerSize + 0x1000) {
        return -1;
    }

    u32 iVar3 = pThis->m4_bufferSize - (pThis->m28).m44_headerSize;
    u32 uVar2 = iVar3 - 0x1000 & ~0x7FF;
    (pThis->m28).m48 = uVar2;
    (pThis->m28).m4C = iVar3 - uVar2;
    u8* puVar1 = pThis->m0_buffer;
    (pThis->m28).m3C = puVar1 + (pThis->m28).m44_headerSize;
    (pThis->m28).m40 = puVar1 + (pThis->m28).m44_headerSize + (pThis->m28).m48;
    (pThis->m28).m60 = READ_BE_U32(pThis->m194 + 8) >> 0x10;
    if (pThis->m194[5] == 8) {
        if (pThis->m194[4] != 2) {
            uVar2 = pThis->m1C_audioBufferSize;
            goto LAB_0600ecca;
        }
        uVar2 = pThis->m1C_audioBufferSize;
    }
    else {
        if (pThis->m194[4] == 2) {
            uVar2 = pThis->m1C_audioBufferSize >> 1;
        }
        else {
            uVar2 = pThis->m1C_audioBufferSize;
        }
    }

    uVar2 = uVar2 >> 1;
LAB_0600ecca:
    uVar2 = uVar2 & 0xfffff000;
    if (uVar2 >> 0xc < 2) {
        return -1;
    }
    if (pThis->m194[5] != 8) {
        uVar2 = uVar2 << 1;
    }
    (pThis->m28).m54 = uVar2;
    (pThis->m28).m58 = pThis->m18_audioBuffer + uVar2;
    (pThis->m28).m0 = 3;
    executeCutsceneCommandsSub0Sub0(pThis, READ_BE_U32(pThis->m194));
    return 0;
}

void cutsceneFillCommandBuffer(sStreamingFile* param_1, u8* param_2)
{
    u32  uVar2;

    if ((param_1->m28).m38 < param_2 + 0x10) {
        u8* puVar4 = (param_1->m28).m3C;
        u8* puVar3 = (param_1->m28).m38;
        uVar2 = (intptr_t)(param_2 + 0x10 + -(intptr_t)puVar3) >> 2;
        do {
            uVar2 = uVar2 - 1;
            
            WRITE_BE_U16(puVar3, READ_BE_U32(puVar4));

            puVar4 = puVar4 + 4;
            puVar3 = puVar3 + 4;
        } while (uVar2 != 0);
    }
    return;
}

struct sCutsceneCommandDefaultTask : public s_workAreaTemplate<sCutsceneCommandDefaultTask>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { nullptr, nullptr, &sCutsceneCommandDefaultTask::Draw, &sCutsceneCommandDefaultTask::Delete };
        return &taskDefinition;
    }

    static void Draw(sCutsceneCommandDefaultTask* pThis)
    {
        pushCurrentMatrix();
        pThis->m8.m18_drawFunction(&pThis->m8);
        popMatrix();
    }

    static void Delete(sCutsceneCommandDefaultTask* pThis)
    {
        Unimplemented();
    }

    s_fileBundle* m0;
    s_vdp1AllocationNode* m4;
    s_3dModel m8;
    //size: 0x58
};

void displayOutOfMemory(u32, u32)
{
    assert(0);
}

void cutsceneCommandDefaultSub0(sCutsceneCommandDefaultTask* r4, u32 iParm2, u32 iParm3)
{
    u8* unaff_r13 = nullptr;
    s_vdp1AllocationNode* unaff_r14 = nullptr;

    if (iParm2 != 0)
    {
        unaff_r13 = dramAllocate(iParm2);
        if(unaff_r13 == 0)
        {
            displayOutOfMemory(0, 1);
        }
    }
    if (iParm3 != 0)
    {
        unaff_r14 = vdp1Allocate(iParm3);
        if (unaff_r14 == 0)
        {
            displayOutOfMemory(0, 2);
        }
    }
    r4->m0 = new s_fileBundle(unaff_r13);
    r4->m4 = unaff_r14;
    return;
}

// TODO: kernel
void setupCutsceneDragonSub0(sStreamingFile* r4, u32 index, u32 param_3, p_workArea param_4, s_3dModel* pModel)
{
    if (index < 0x11)
    {
        r4->mC8_cutsceneEntities[index].m0_compressionType = param_3;
        r4->mC8_cutsceneEntities[index].m4_pOwner = param_4;
        r4->mC8_cutsceneEntities[index].m8_p3dModel = pModel;
    }
}

void cutsceneCommandDefaultSub1(sStreamingFile* psParm1, u8* pSource, u8* pDest, u32 size)
{
    if ((psParm1->m28).m38 <= pSource) {
        pSource = pSource + -(intptr_t)(psParm1->m28).m38 + (intptr_t)(psParm1->m28).m3C;
    }
    u32 uVar3 = (intptr_t)pSource + (size - (intptr_t)(psParm1->m28).m38);
    if ((int)uVar3 < 0) {
        uVar3 = 0;
    }
    if (size - uVar3 != 0) {
        u32 uVar1 = size - uVar3 >> 2;
        memcpy(pDest, pSource, uVar1 * 4);
        pDest += uVar1 * 4;
        pSource += uVar1 * 4;
    }
    if (uVar3 != 0) {
        u8* puVar4 = (psParm1->m28).m3C;
        uVar3 = uVar3 >> 2;
        memcpy(pDest, puVar4, uVar3 * 4);
    }
}

s32 cutsceneCommandDefault(sStreamingFile* psParm1)
{
    u8* local_r13_28 = (psParm1->m28).m34_pBufferRead;
    sCutsceneCommandDefaultTask* piVar2 = createSubTask<sCutsceneCommandDefaultTask>(currentResTask);
    setupCutsceneDragonSub0(psParm1, READ_BE_U32(local_r13_28 + 4) + -0x10, 1, piVar2, &piVar2->m8);
    u32 iVar6 = READ_BE_U32(local_r13_28 + 8) + -0x10;
    u32 iVar5 = READ_BE_U32(local_r13_28) - iVar6;
    cutsceneCommandDefaultSub0(piVar2, iVar6, iVar5);

    u32 uVar3 = 0;

    if (iVar5 != 0) {
        u16 sVar1 = piVar2->m4->m4_vdp1Memory;
        cutsceneCommandDefaultSub1(psParm1, local_r13_28 + READ_BE_U32(local_r13_28 + 8), getVdp1Pointer(0x25C00000 + ((int)sVar1 & 0xffffU) * 8), iVar5);
        addToMemoryLayout(getVdp1Pointer(0x25C00000 + ((int)sVar1 & 0xffffU) * 8), iVar5);
    }
    if (iVar6 != 0) {
        cutsceneCommandDefaultSub1(psParm1, local_r13_28 + 0x10, piVar2->m0->getRawBuffer(), iVar6);
        addToMemoryLayout(piVar2->m0->getRawBuffer(), iVar6);
        if (iVar5 == 0) {
            uVar3 = 0;
        }
        else {
            uVar3 = piVar2->m4->m4_vdp1Memory;
        }
        //patchFilePointers(iVar4->getRawBuffer(), uVar3);
    }

    u16 modelIndex = READ_BE_U16(local_r13_28 + 0xc);
    sModelHierarchy* pHierarchy = piVar2->m0->getModelHierarchy(modelIndex);
    sStaticPoseData* pStaticPose = piVar2->m0->getStaticPose(READ_BE_U16(local_r13_28 + 0xe), pHierarchy->countNumberOfBones());
    pHierarchy->patchFilePointers(piVar2->m4->m4_vdp1Memory);

    init3DModelRawData(piVar2, &piVar2->m8, 0, piVar2->m0, modelIndex, 0, pStaticPose, 0, 0);
    return 0;
}

u32 readCPUTimer()
{
    static u32 cpuTimer = 0x12345;
    return ++cpuTimer;
}

void cutsceneCommand0Sub0(sStreamingFile* param_1)
{
    param_1->m28.m78 = 0;
    param_1->m28.m7C = 0;
    param_1->m28.m80_cpuTimer = readCPUTimer();
}

void cutsceneCommand0Sub1(sStreamingFile* param_1)
{
    assert(0);
}

u8* cutsceneCommand0Sub5(sStreamingFile* param_1, u8* param_2)
{
    u8* local_r1_100;
    u8* local_r2_48;
    u8* puVar1;
    u8* local_r2_100;
    u8* local_r4_34;
    u8* local_r6_62;
    u8* ppuVar2;
    u32 uVar3;

    if (param_2 < (param_1->m28).m38) {
        if ((param_1->m28).m38 < param_2 + 8) {
            local_r6_62 = (param_1->m28).m3C;
            local_r4_34 = (param_1->m28).m38;
            uVar3 = (u32)(param_2 + 8 -local_r4_34) >> 2;
            memcpy(local_r4_34, local_r6_62, uVar3 * 4);
            local_r4_34 += uVar3 * 4;
            local_r6_62 += uVar3 * 4;

            uVar3 = (READ_BE_U32(param_2) - 8U) >> 2;
            memcpy(local_r4_34, local_r6_62, uVar3 * 4);
            local_r4_34 += uVar3 * 4;
            local_r6_62 += uVar3 * 4;
        }
        else {
            if ((param_1->m28).m38 < param_2 + READ_BE_U32(param_2)) {
                local_r1_100 = (param_1->m28).m3C;
                ppuVar2 = (param_1->m28).m38;
                uVar3 = (param_2 + READ_BE_U32(param_2) -ppuVar2) >> 2;
                memcpy(ppuVar2, local_r1_100, uVar3 *4);
            }
        }
        return param_2;
    }
    return param_2 + -(intptr_t)(param_1->m28).m38 + (intptr_t)(param_1->m28).m3C;
}

int shiftLeft32(int param_1, int param_2)
{
    return param_1 << param_2;

    switch (param_2) {
    case 0:
        return param_1;
    case 6:
        param_1 = param_1 << 2;
    case 4:
        param_1 = param_1 << 2;
    case 2:
        return param_1 << 2;
    case 7:
        param_1 = param_1 << 2;
    case 5:
        param_1 = param_1 << 2;
    case 3:
        param_1 = param_1 << 2;
    case 1:
        return param_1 << 1;
    case 0xe:
        param_1 = param_1 << 2;
    case 0xc:
        param_1 = param_1 << 2;
    case 10:
        param_1 = param_1 << 2;
    case 8:
        return param_1 << 8;
    case 0xf:
        param_1 = param_1 << 2;
    case 0xd:
        param_1 = param_1 << 2;
    case 0xb:
        param_1 = param_1 << 2;
    case 9:
        return param_1 << 9;
    case 0x16:
        param_1 = param_1 << 2;
    case 0x14:
        param_1 = param_1 << 2;
    case 0x12:
        param_1 = param_1 << 2;
    case 0x10:
        return param_1 << 0x10;
    case 0x17:
        param_1 = param_1 << 2;
    case 0x15:
        param_1 = param_1 << 2;
    case 0x13:
        param_1 = param_1 << 2;
    case 0x11:
        return param_1 << 0x11;
    case 0x1c:
        param_1 = param_1 << 2;
    case 0x1a:
        param_1 = param_1 << 2;
    case 0x18:
        return param_1 << 0x18;
    case 0x1d:
        param_1 = param_1 << 2;
    case 0x1b:
        param_1 = param_1 << 2;
    case 0x19:
        return param_1 << 0x19;
    case 0x1e:
        return 0;
    case 0x1f:
        return 0;
    default:
        return 0;
    }
}

void cutsceneCommand0Sub2_updateCamera(u8* param_1)
{
    sVec3_FP local_10;
    local_10[0] = shiftLeft32(READ_BE_S16(param_1), READ_BE_U16(param_1 + 6) >> 0xc);
    local_10[1] = shiftLeft32(READ_BE_S16(param_1 + 2), READ_BE_U16(param_1 + 8) >> 0xc);
    local_10[2] = shiftLeft32(READ_BE_S16(param_1 + 4), READ_BE_U16(param_1 + 10) >> 0xc);

    sVec3_S16 local_18;
    local_18[0] = READ_BE_U16(param_1 + 6) & 0xfff;
    local_18[1] = READ_BE_U16(param_1 + 8) & 0xfff;
    local_18[2] = READ_BE_U16(param_1 + 10) & 0xfff;
    updateEngineCamera(&cameraProperties2, local_10, local_18);
}

void cutsceneCommand0Sub3Sub0(s_3dModel* pModel, u8* pData)
{
    u32 boneCount = pModel->m12_numBones;
    std::vector<sPoseData>::iterator poseData = pModel->m2C_poseData.begin();

    poseData->m0_translation[0] = shiftLeft32(READ_BE_S16(pData), READ_BE_U16(pData + 6) >> 0xc);
    poseData->m0_translation[1] = shiftLeft32(READ_BE_S16(pData + 2), READ_BE_U16(pData + 8) >> 0xc);
    poseData->m0_translation[2] = shiftLeft32(READ_BE_S16(pData + 4), READ_BE_U16(pData + 10) >> 0xc);
    pData += 6;

    do
    {
        poseData->mC_rotation[0] = (READ_BE_U16(pData + 0) & 0xfff) << 0x10;
        poseData->mC_rotation[1] = (READ_BE_U16(pData + 2) & 0xfff) << 0x10;
        poseData->mC_rotation[2] = (READ_BE_U16(pData + 4) & 0xfff) << 0x10;

        pData += 12;
        poseData++;
    } while (--boneCount);
}

void cutsceneCommand0Sub3Sub1(s_3dModel* pModel, u8* pData)
{
    u32 boneCount = pModel->m12_numBones;
    std::vector<sPoseData>::iterator poseData = pModel->m2C_poseData.begin();

    do
    {
        poseData->m0_translation[0] = shiftLeft32(READ_BE_S16(pData), READ_BE_U16(pData + 6) >> 0xc);
        poseData->m0_translation[1] = shiftLeft32(READ_BE_S16(pData + 2), READ_BE_U16(pData + 8) >> 0xc);
        poseData->m0_translation[2] = shiftLeft32(READ_BE_S16(pData + 4), READ_BE_U16(pData + 10) >> 0xc);

        poseData->mC_rotation[0] = (READ_BE_U16(pData + 6) & 0xfff) << 0x10;
        poseData->mC_rotation[1] = (READ_BE_U16(pData + 8) & 0xfff) << 0x10;
        poseData->mC_rotation[2] = (READ_BE_U16(pData + 10) & 0xfff) << 0x10;

        pData += 12;
        poseData++;
    } while (--boneCount);
}


void cutsceneCommand0Sub3_updateAnimation(sStreamingFile* param_1, u32 param_2, u8* param_3)
{
    sStreamingFile_C8* local_r14_24 = &param_1->mC8_cutsceneEntities[param_2];
    if ((local_r14_24->m0_compressionType != 0) && ((local_r14_24->m8_p3dModel) != 0)) {
        if (local_r14_24->m0_compressionType == 3) {
            cutsceneCommand0Sub3Sub0(local_r14_24->m8_p3dModel, param_3);
            return;
        }
        cutsceneCommand0Sub3Sub1(local_r14_24->m8_p3dModel, param_3);
        return;
    }
    return;
}

// delete a cutscene 3d model
void cutsceneCommand0Sub4_deleteCutsceneEntity(sStreamingFile* param_1, u32 entityIndex)
{
    sStreamingFile_C8& entity = param_1->mC8_cutsceneEntities[entityIndex];
    if ((entity.m0_compressionType != 0) && (entity.m4_pOwner))
    {
        entity.m4_pOwner->getTask()->markFinished();
        entity.m0_compressionType = 0;
        entity.m4_pOwner = nullptr;
        entity.m8_p3dModel = nullptr;
    }
}

s32 cutsceneCommand1(sStreamingFile* param_1)
{
    Unimplemented();
    return 0;
}

s32 cutsceneCommand0(sStreamingFile* param_1)
{
    u8* local_r0_130;
    u8* local_r10_28;
    u8* local_r11_140;
    u8* local_24;

    local_r10_28 = (param_1->m28).m34_pBufferRead;
    if ((param_1->m28).m0 == 4) {
        if ((param_1->m28).m88 == 0) {
            cutsceneCommand0Sub0(param_1);
        }
        if ((u32)(param_1->m28).m88 <= (param_1->m28).m84_frameIndex) {
            if ((param_1->m28).m14 != 0) {
                cutsceneCommand0Sub1(param_1);
                (param_1->m28).m14 = 0;
            }
            local_24 = local_r10_28 + 0x10;
            (param_1->m28).m88 = (param_1->m28).m88 + READ_BE_U32(local_r10_28 + 8);
            while (local_24 < local_r10_28 + READ_BE_U32(local_r10_28)) {
                local_r0_130 = cutsceneCommand0Sub5(param_1, local_24);
                local_r11_140 = local_r0_130 + 8;
                switch (READ_BE_U32(local_r0_130 + 4)) {
                case 0:
                    cutsceneCommand0Sub2_updateCamera(local_r11_140);
                    break;
                case 1:
                    setupVDP2StringRendering(3, 0x19, 0x26, 2);
                    VDP2DrawString((char*)local_r11_140);
                    (param_1->m28).m1C = 1;

                    break;
                case 2:
                    clearVdp2TextArea();
                    (param_1->m28).m1C = 0;
                    break;
                case 0x10:
                case 0x11:
                case 0x12:
                case 0x13:
                case 0x14:
                case 0x15:
                case 0x16:
                case 0x17:
                case 0x18:
                case 0x19:
                case 0x1a:
                case 0x1b:
                case 0x1c:
                case 0x1d:
                case 0x1e:
                case 0x1f:
                    cutsceneCommand0Sub3_updateAnimation(param_1, READ_BE_U32(local_r0_130 + 4) + -0x10, local_r11_140);
                    break;
                case 0x20:
                case 0x21:
                case 0x22:
                case 0x23:
                case 0x24:
                case 0x25:
                case 0x26:
                case 0x27:
                case 0x28:
                case 0x29:
                case 0x2a:
                case 0x2b:
                case 0x2c:
                case 0x2d:
                case 0x2e:
                case 0x2f:
                    cutsceneCommand0Sub4_deleteCutsceneEntity(param_1, READ_BE_U32(local_r0_130 + 4) + -0x20);
                    break;
                default:
                    assert(0);
                }
                local_24 = local_24 + READ_BE_U32(local_r0_130);
            }
            return 0;
        }
    }
    return -1;
}

void executeCutsceneCommandsSub1(sStreamingFile* param_1)
{
    u32 uVar1;

    uVar1 = param_1->m8;
    if ((param_1->m198).m18_fileSize <= uVar1) {
        uVar1 = (param_1->m198).m18_fileSize;
    }
    if (uVar1 <= (param_1->m28).m24_currentPositionInFile) {
        (param_1->m28).m0 = 4;
    }
    return;
}

s32 cutsceneCommand2(sStreamingFile* psParm1)
{
    u8* pDest;
    int size;

    u8* piVar1 = (psParm1->m28).m34_pBufferRead;
    size = READ_BE_U32(piVar1) + -0x10;
    pDest = (u8*)allocateHeap(size);
    cutsceneCommandDefaultSub1(psParm1, (u8*)(piVar1 + 4), pDest, size);
    resetVdp2StringsSub1((u16*)pDest);
    return 0;
}


void executeCutsceneCommands(sStreamingFile* psParm1)
{
    psParm1->m18C++;
    int iVar1 = psParm1->m28.m0;
    if (psParm1->m28.m0 == 2)
    {
        if (executeCutsceneCommandsSub0(psParm1) != 0)
            goto  LAB_0600eb14;
    }
    else
    {
        if (iVar1 != 3) {
            if (iVar1 != 4) {
                return;
            }
            if ((psParm1->m28).m8 == 0) {
                Unimplemented();

                // hack!
                (psParm1->m28).m84_frameIndex+=2;
                //assert(0);
                /*
                FUN_0600dedc(psParm1);
                if ((VDP2Regs_.TVSTAT & 1) == 0) {
                    uVar2 = 0x1b4f4f0;
                }
                else {
                    uVar2 = 0x1b1ebfc;
                }
                uVar2 = setDividend((psParm1->m28).m78, *(undefined4*)(puVar3 + 0xc), (uVar2 >> 0xe) + 1 >> 1
                );
                (psParm1->m28).m84_frameIndex = uVar2;
                FUN_0600df52(psParm1);
                if ((psParm1->m28).m98[0] != 0) {
                    (psParm1->m28).m90 = (psParm1->m28).m98[0];
                    (psParm1->m28).m94 = (psParm1->m28).m98[1];
                    FUN_0600e1b2(psParm1, 0xffffffff);
                    (psParm1->m28).m98[0] = 0;
                    (psParm1->m28).m98[1] = 0;
                }
                */
            }
            goto LAB_0600eb14;
        }
    }

    if ((psParm1->m28).m4 != 0) {
        executeCutsceneCommandsSub1(psParm1);
    }
    LAB_0600eb14:
    if ((psParm1->m28).m18 != 0) {
        if ((psParm1->m28).m64_sampleIndex * 0x1000 + (psParm1->m28).m6C < (psParm1->m28).m70) {
            //executeCutsceneCommandsSub1b(psParm1);
            assert(0);
            (psParm1->m28).mC = 0;
        }
        else {
            //executeCutsceneCommandsSub2(psParm1);
            assert(0);
            (psParm1->m28).m50 = 0;
            (psParm1->m28).m18 = 0;
        }
    }

    if (((psParm1->m28).m88 != 0) && (READ_BE_U32(psParm1->m194 + 0x10) <= (psParm1->m28).m84_frameIndex)) {
        updateStreamingFileReadSub0(psParm1);
        return;
    }

    do {
        u32 uVar2 = (psParm1->m28).m24_currentPositionInFile - (psParm1->m28).m28;
        if (uVar2 < 0x10) {
            return;
        }

        if ((int)uVar2 < READ_BE_U32((psParm1->m28).m34_pBufferRead)) {
            return;
        }

        cutsceneFillCommandBuffer(psParm1, (psParm1->m28).m34_pBufferRead);
        int opcode = READ_BE_U32((psParm1->m28).m34_pBufferRead + 4);
        switch (opcode) {
        case 0x0:
            iVar1 = cutsceneCommand0(psParm1);
            break;
        case 0x1:
            iVar1 = cutsceneCommand1(psParm1);
            if (iVar1 != 0)
            {
                psParm1->m28.mC = 1;
                return;
            }
            psParm1->m28.mC = 0;
            break;
        case 0x2:
            iVar1 = cutsceneCommand2(psParm1);
            break;
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
        case 0x14:
        case 0x15:
        case 0x16:
        case 0x17:
        case 0x18:
        case 0x19:
        case 0x1A:
        case 0x1B:
        case 0x1C:
        case 0x1D:
        case 0x1E:
        case 0x1F:
            iVar1 = cutsceneCommandDefault(psParm1);
            break;
        default:
            assert(0);
        }
        if (iVar1 != 0) {
            return;
        }
    def_600EBA2:
        executeCutsceneCommandsSub0Sub0(psParm1, READ_BE_U32((psParm1->m28).m34_pBufferRead));
        (psParm1->m28).m10 = (psParm1->m28).m10 + 1;
    } while (true);
}

void syncCutsceneTimer(sStreamingFile* psParm1)
{
    Unimplemented();
}

void updateCutsceneStreaming(sStreamingFile* psParm1)
{
    updateStreamingFileRead(psParm1);
    executeCutsceneCommands(psParm1);
    syncCutsceneTimer(psParm1);
    if (enableDebugTask)
    {
        vdp2DebugPrintSetPosition(2, 0xd);
        vdp2PrintfLargeFont("Frame: %5d", (psParm1->m28).m84_frameIndex);
        vdp2DebugPrintSetPosition(2, 0xf);
        vdp2PrintfSmallFont("Sample: %4d", (psParm1->m28).m64_sampleIndex);
        vdp2DebugPrintSetPosition(2, 0x10);
        vdp2PrintfSmallFont("stock: %8d", ((psParm1->m28).m68_writePosition - (psParm1->m28).m6C) + (psParm1->m28).m64_sampleIndex * -0x1000);
        vdp2DebugPrintSetPosition(2, 0x11);
        vdp2PrintfSmallFont("write: %8d", (psParm1->m28).m68_writePosition);
        vdp2DebugPrintSetPosition(2, 0x12);
        vdp2PrintfSmallFont("empty: %4d", psParm1->m190_emptyBytes);
        vdp2DebugPrintSetPosition(2, 0x13);
        vdp2PrintfSmallFont("camera: %2d", psParm1->m188_camera);
    }

    Unimplemented();
}

void pauseCutscene(sStreamingFile* r4, s32 r5)
{
    Unimplemented();
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

void sE006Task0::Init(sE006Task0* pThis, s32 arg)
{
    pThis->m1D4 = dramAllocate(0x20000);

    sStreamingParams streamingParams;
    streamingParams.m0_streamingFile = &pThis->m4;
    streamingParams.m4_streamingFileSize = sizeof(pThis->m4);
    streamingParams.m8_buffer = pThis->m1D4;
    streamingParams.mC_bufferSize = 0x20000;
    streamingParams.m10_audioBuffer = nullptr;//TODO: figure this out
    streamingParams.m14_audioBufferSize = 0x8000;

    std::string streamFileName = readSaturnString(sSaturnPtr::createFromRaw(arg, gCurrentTownOverlay));
    findMandatoryFileOnDisc(streamFileName.c_str());
    pThis->m0 = openFileForStreaming(&streamingParams, streamFileName);

}

void sE006Task0::cutsceneDrawSub0(sE006Task0* pThis, s32 r5)
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

void sE006Task0::Draw(sE006Task0* pThis)
{

    if ((g_fadeControls.m0_fade0.m20_stopped != 0) &&
        (((readKeyboardToggle(KEY_CODE_F12) != 0 && ((e006Task0Var0 & 4) == 0)) ||
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

void sE006Task0::Delete(sE006Task0* pThis)
{
    Unimplemented();
}

sE006Task0* e006Task0 = nullptr;

s32 createEPKPlayer(s32 r4)
{
    npcData0.mF4 = 1;
    npcData0.mF0 = 0;

    e006Task0 = createSubTaskWithArg<sE006Task0>(twnMainLogicTask, r4);

    return 0;
}

s32 e006_scriptFunction_605861eSub0()
{
    if ((e006Task0 != nullptr) && (npcData0.mF0 == 0))
    {
        s32 iVar1 = scriptFunction_605861eSub0Sub0(e006Task0->m0);
        if ((iVar1 != 5) && (iVar1 != -1))
            return 0;
    }

    return 1;
}

struct sDragonCutsceneData
{
    s16 m0;
    s32 m4;
    s32 m8;
};

const std::array<sDragonCutsceneData, DR_LEVEL_MAX> dragonCutsceneData = {
    {
        {
            8,
            0xAA24AE,
            0xEE68F3,
        },
        {
            8,
            0xAA24AE,
            0xEE68F3,
        },
        {
            0x16,
            0xEE6608,
            0x132AA4C,
        },
        {
            0xA,
            0,
            0,
        },
        {
            0xB,
            0x120F336,
            0x165377A,
        },
        {
            0x18,
            0,
            0,
        },
        {
            0x16,
            0,
            0,
        },
        {
            0x7,
            0,
            0,
        },
        {
            0,
            0,
            0,
        },
    }
};

sVec3_FP gDragonCutscenePosition = { 0,0,0 };
sVec3_FP gDragonCutsceneRotation = { 0,0,0 };

struct sE006Task1 : public s_workAreaTemplate<sE006Task1>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { nullptr, &sE006Task1::Update, &sE006Task1::Draw, &sE006Task1::Delete };
        return &taskDefinition;
    }

    static void Update(sE006Task1* pThis)
    {
        u32 iVar2 = (gDragonState->mC_dragonType * 0xc) >> 8;
        s32 iVar3 = gDragonState->m28_dragon3dModel.m2C_poseData[dragonCutsceneData[iVar2].m0].mC_rotation[2];
        fixedPoint uVar1 = iVar3 - pThis->m8;
        if ((dragonCutsceneData[iVar2].m4 <= iVar3) && (iVar3 <= dragonCutsceneData[iVar2].m8 + iVar2)) {
            if ((uVar1.normalized() < -0x222222) && (-1 < readSaturnS16(pThis->m0 + 2))) {
                playSystemSoundEffect(readSaturnS16(pThis->m0 + 2));
            }
        }
        pThis->m8 = iVar3;
        return;
    }

    static void Draw(sE006Task1* pThis)
    {
        for (int i = 0; i < gDragonState->m28_dragon3dModel.m12_numBones; i++)
        {
            initMatrixToIdentity(&gDragonState->m28_dragon3dModel.m3C_boneMatrices[i]);
            translateMatrix(gDragonState->m28_dragon3dModel.m2C_poseData[i].m0_translation, &gDragonState->m28_dragon3dModel.m3C_boneMatrices[i]);
            rotateMatrixZYX(&gDragonState->m28_dragon3dModel.m2C_poseData[i].mC_rotation, &gDragonState->m28_dragon3dModel.m3C_boneMatrices[i]);
        }

        submitModelAndShadowModelToRendering(&gDragonState->m28_dragon3dModel, gDragonState->m14_modelIndex, gDragonState->m18_shadowModelIndex, &gDragonCutscenePosition, &gDragonCutsceneRotation, 0);
    }

    static void Delete(sE006Task1* pThis)
    {
        Unimplemented();
    }

    sSaturnPtr m0;
    s32 m8;
    // size: 0xc
};

// TODO: kernel
void setupCutsceneDragon(sStreamingFile* r4, u32 r5, p_workArea r6, s_3dModel* pModel)
{
    setupCutsceneDragonSub0(r4, r5, 3, r6, pModel);
}

// TODO: kernel
void setupCutsceneModelType2(sStreamingFile* r4, u32 r5, p_workArea r6, s_3dModel* pModel)
{
    setupCutsceneDragonSub0(r4, r5, 2, r6, pModel);
}

s32 setupDragonEntityForCutscene(s32 r4)
{
    sSaturnPtr r4Ptr = sSaturnPtr::createFromRaw(r4, gCurrentTownOverlay);
    if (e006_scriptFunction_605861eSub0() == 0)
    {
        sE006Task1* pNewSubTask = createSubTask<sE006Task1>(currentResTask);
        pNewSubTask->m0 = r4Ptr;
        pNewSubTask->m8 = 0x80000000;

        setupCutsceneDragon(e006Task0->m0, readSaturnU8(r4Ptr + 1), pNewSubTask, &gDragonState->m28_dragon3dModel);
        npcData0.m70_npcPointerArray[readSaturnU8(r4Ptr + 0)].workArea = pNewSubTask;
        setupModelAnimation(&gDragonState->m28_dragon3dModel, nullptr);
    }
    return 0;
}

// TODO: kernel
void scriptFunction_60573d8Sub0(sStreamingFile* r4)
{
    r4->m28.m4 = 1;
}

s32 e006_scriptFunction_60573d8()
{
    npcData0.mF0 = 0;
    if (e006Task0 != nullptr)
    {
        scriptFunction_60573d8Sub0(e006Task0->m0);
    }
    npcData0.mF0 = 0;
    return 0;
}

s32 e006_scriptFunction_6056918()
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

// kernel
s32 udivsi3(s32 r0, s32 r1)
{
    return r1 / r0;
}

s32 getPositionInEDKSub0(sStreamingFile* iParm1)
{
    int iVar2;
    if ((VDP2Regs_.m4_TVSTAT & 1) == 0) {
        iVar2 = 60;
    }
    else {
        iVar2 = 50;
    }
    return udivsi3(READ_BE_U32(iParm1->m194 + 0xc), (READ_BE_U32(iParm1->m194 + 0x10) - (iParm1->m28).m84_frameIndex) * iVar2);
}

s32 getPositionInEDK(sStreamingFile* iParm1)
{
    s32 dVar1;
    int iVar2;

    iVar2 = (iParm1->m28).m0;
    if ((iVar2 < 4) && (iVar2 != 1)) {
        return 0x7fffffff;
    }

    return getPositionInEDKSub0(iParm1);
}

s32 e006_scriptFunction_6057438()
{
    if (e006Task0)
    {
        if (npcData0.mF0 == 0)
        {
            s32 iVar1 = getPositionInEDK(e006Task0->m0);
            return (int)(iVar1 + (u32)(iVar1 < 0)) >> 1;
        }
    }
    return 0;
}

s32 scriptFunction_6057470()
{
    Unimplemented();
    return 0;
}

TWN_E006_data::TWN_E006_data() : sTownOverlay("TWN_E006.PRG")
{
    overlayScriptFunctions.m_zeroArg[0x60573d8] = &e006_scriptFunction_60573d8;
    overlayScriptFunctions.m_zeroArg[0x6056918] = &e006_scriptFunction_6056918;
    overlayScriptFunctions.m_zeroArg[0x6057438] = &e006_scriptFunction_6057438;
    overlayScriptFunctions.m_zeroArg[0x6057470] = &scriptFunction_6057470;
    overlayScriptFunctions.m_zeroArg[0x6056926] = &scriptFunction_6056926;

    overlayScriptFunctions.m_oneArg[0x60573b0] = &createEPKPlayer;
    overlayScriptFunctions.m_oneArg[0x605861e] = &setupDragonEntityForCutscene;
    overlayScriptFunctions.m_oneArg[0x605838C] = &SetupColorOffset;
    overlayScriptFunctions.m_oneArg[0x605be04] = &TwnFadeOut;
    overlayScriptFunctions.m_oneArg[0x605bd8c] = &TwnFadeIn;

    overlayScriptFunctions.m_twoArg[0x605bb24] = &townCamera_setup;

    mTownSetups.push_back(readTownSetup(getSaturnPtr(0x605e1c0), 1));
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

static const char* listOfFilesToLoad[] = {
    "COMMON3.MCB",
    "COMMON3.CGB",
    "EXCAVAMP.MCB",
    "EXCAVAMP.CGB",
    nullptr
};

static void startE006BackgroundTask(p_workArea pThis)
{
    Unimplemented();

    // hack: to remove title screen
    reinitVdp2();
}

void setupVdp1Proj(fixedPoint fov)
{
    initVDP1Projection(fov, 0);
}

p_workArea overlayStart_TWN_E006(p_workArea pUntypedThis, u32 arg)
{
    gTWN_E006->makeCurrent();
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

    registerNpcs(gTWN_E006->mTownSetups, gTWN_E006->getSaturnPtr(0x605414c), arg);

    startScriptTask(pThis);

    startE006BackgroundTask(pThis);

    startMainLogic(pThis);

    setupVdp1Proj(0x1c71c71);

    startCameraTask(pThis);

    return pThis;
}

s32 getCutsceneFrameIndex(sStreamingFile* pStreamingFile)
{
    return pStreamingFile->m28.m84_frameIndex;
}
