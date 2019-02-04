#include "PDS.h"

s_fieldTaskWorkArea* fieldTaskPtr = NULL;

u8 fieldVdp1Buffer[0x4000];
u8 fieldMainBuffer[0x68000];

struct s_vramListIntermediary : s_vdp1AllocatorNode
{
    void* m_0;
    u16 m_4;
    u16 m_6;
};

struct s_vramListIntermediary2
{
    void* m_0;
    void* m_vdp1Memory;
    u16 size;
    s_vdp1AllocatorNode m_list[0x40];
};

struct s_vramList
{
    s_vdp1AllocatorNode m_node;
    s_vramListIntermediary m_intermediary;
    s_vramListIntermediary2 m_intermediary2;
};

void allocateVramList(s_workArea* pWorkArea, u8* pBuffer, u32 bufferSize)
{
    s_vramList* r5 = static_cast<s_vramList*>(allocateHeapForTask(pWorkArea, sizeof(s_vramList)));

    r5->m_node.m_nextAllocator = vdp1AllocatorHead;
    vdp1AllocatorHead = &r5->m_node;

    s_vramListIntermediary* r7 = &r5->m_intermediary;
    s_vramListIntermediary2* r4 = &r5->m_intermediary2;

    r7->m_0 = r4;
    r7->m_4 = 0;
    r7->m_6 = 0;

    r4->m_0 = 0;
    r4->m_vdp1Memory = pBuffer;
    r4->size = bufferSize >> 3;

    r5->m_node.m_nextNode = r4->m_list;

    for (u32 i = 0; i < 0x3F; i++)
    {
        r4->m_list[i].m_nextAllocator = &r4->m_list[i + 1];
    }

    r4->m_list[0x3F].m_nextAllocator = NULL;
}

void initFieldMemoryArea(u8* buffer, u32 bufferSize)
{
    fieldTaskPtr->m8_pSubFieldData->memoryArea_bottom = buffer;
    fieldTaskPtr->m8_pSubFieldData->memoryArea_edge = buffer;
    fieldTaskPtr->m8_pSubFieldData->memoryArea_top = buffer + bufferSize;
}

void initFieldVdp1Area(u8* buffer, u32 bufferSize)
{
    fieldTaskPtr->m8_pSubFieldData->characterArea_bottom = buffer;
    fieldTaskPtr->m8_pSubFieldData->characterArea_edge = buffer;
    fieldTaskPtr->m8_pSubFieldData->characterArea_top = buffer + bufferSize;
}

void loadCommonFieldResources()
{
    initDramAllocator(fieldTaskPtr, playerDataMemoryBuffer, sizeof(playerDataMemoryBuffer), NULL);

    allocateVramList(fieldTaskPtr->m4_overlayTaskData, fieldVdp1Buffer, sizeof(fieldVdp1Buffer));

    initFieldMemoryArea(fieldMainBuffer, sizeof(fieldMainBuffer));
    initFieldVdp1Area(getVdp1Pointer(0x25C1C800), 0x5F800);
}

bool findMandatoryFileOnDisc(const char* fileName)
{
    FILE* fHandle = fopen(fileName, "rb");

    if (fHandle == NULL)
        return false;

    fclose(fHandle);
    return true;
}

u32 getFileSizeFromFileId(const char* fileName)
{
    FILE* fHandle = fopen(fileName, "rb");

    if (fHandle == NULL)
        return 0;

    fseek(fHandle, 0, SEEK_END);
    u32 fileSize = ftell(fHandle);

    fclose(fHandle);
    return fileSize;
}

void checkFilesExists(const s_MCB_CGB* fileList)
{
    u32* MCBSizes = fieldTaskPtr->m8_pSubFieldData->MCBFilesSizes;
    u32* CGBSizes = fieldTaskPtr->m8_pSubFieldData->CGBFilesSizes;

    while (fileList->MCB || fileList->CGB)
    {
        assert(findMandatoryFileOnDisc(fileList->MCB));
        *(MCBSizes++) = getFileSizeFromFileId(fileList->MCB);

        assert(findMandatoryFileOnDisc(fileList->CGB));
        *(CGBSizes++) = getFileSizeFromFileId(fileList->CGB);

        fileList++;
    }
}

void setupFileList(const s_MCB_CGB* fileList)
{
    fieldTaskPtr->m8_pSubFieldData->fileList = fileList;

    checkFilesExists(fileList);

    fieldTaskPtr->m8_pSubFieldData->memoryArea[0] = fieldTaskPtr->m8_pSubFieldData->memoryArea_bottom;
    fieldTaskPtr->m8_pSubFieldData->memoryArea[1] = fieldTaskPtr->m8_pSubFieldData->memoryArea[0] + fieldTaskPtr->m8_pSubFieldData->MCBFilesSizes[0]; // TODO: should be aligned
    fieldTaskPtr->m8_pSubFieldData->memoryArea[2] = fieldTaskPtr->m8_pSubFieldData->memoryArea[1] + fieldTaskPtr->m8_pSubFieldData->MCBFilesSizes[1]; // TODO: should be aligned

    fieldTaskPtr->m8_pSubFieldData->characterArea[0] = fieldTaskPtr->m8_pSubFieldData->characterArea_bottom;
    fieldTaskPtr->m8_pSubFieldData->characterArea[1] = fieldTaskPtr->m8_pSubFieldData->characterArea[0] + fieldTaskPtr->m8_pSubFieldData->CGBFilesSizes[0]; // TODO: should be aligned
    fieldTaskPtr->m8_pSubFieldData->characterArea[2] = fieldTaskPtr->m8_pSubFieldData->characterArea[1] + fieldTaskPtr->m8_pSubFieldData->CGBFilesSizes[1]; // TODO: should be aligned

    u32 currentFileIndex = 2;
    const s_MCB_CGB* currentFileList = fileList + currentFileIndex;

    u32 largestMCB = 0;
    u32 largestCGB = 0;

    while (currentFileList->MCB)
    {
        largestMCB = std::max(fieldTaskPtr->m8_pSubFieldData->MCBFilesSizes[currentFileIndex], largestMCB);
        largestCGB = std::max(fieldTaskPtr->m8_pSubFieldData->CGBFilesSizes[currentFileIndex], largestCGB);

        currentFileList++;
        currentFileIndex++;
    }

    fieldTaskPtr->m8_pSubFieldData->memoryArea_edge = fieldTaskPtr->m8_pSubFieldData->memoryArea[2] + largestMCB; // TODO: should be aligned
    fieldTaskPtr->m8_pSubFieldData->characterArea_edge = fieldTaskPtr->m8_pSubFieldData->characterArea[2] + largestCGB; // TODO: should be aligned
}

s32 getFieldMemoryAreaRemain()
{
    return fieldTaskPtr->m8_pSubFieldData->memoryArea_top - fieldTaskPtr->m8_pSubFieldData->memoryArea_edge;
}

s32 getFieldCharacterAreaRemain()
{
    return fieldTaskPtr->m8_pSubFieldData->characterArea_top - fieldTaskPtr->m8_pSubFieldData->characterArea_edge;
}

void loadFileFromFileList(u32 index)
{
    const s_MCB_CGB* pFileData = &fieldTaskPtr->m8_pSubFieldData->fileList[index];

    int slot = std::min(2, (int)index);

    if (pFileData->MCB)
    {
        loadFile(pFileData->MCB, fieldTaskPtr->m8_pSubFieldData->memoryArea[slot], fieldTaskPtr->m8_pSubFieldData->characterArea[slot]);
    }

    if (pFileData->CGB)
    {
        loadFile(pFileData->CGB, fieldTaskPtr->m8_pSubFieldData->characterArea[slot], NULL);
    }
}

void getMemoryArea(s_memoryAreaOutput* pOutput, u32 areaIndex)
{
    if (areaIndex > 2)
    {
        areaIndex = 2;
    }

    pOutput->m0_mainMemory = fieldTaskPtr->m8_pSubFieldData->memoryArea[areaIndex];
    pOutput->m4_characterArea = fieldTaskPtr->m8_pSubFieldData->characterArea[areaIndex];
}

struct s_exitCutsceneTask : public s_workAreaTemplate<s_exitCutsceneTask>
{
    static void Update(s_exitCutsceneTask* pThis)
    {
        assert(0);
    }

    // size 0x18
};

void startExitFieldCutscene(p_workArea parent, s_cameraScript* pScript, s32 param, s32 exitIndex, s32 arg0)
{
    createSubTaskFromFunction<s_exitCutsceneTask>(parent, &s_exitCutsceneTask::Update);
}

void initFieldDragonLight()
{
    s_dragonTaskWorkArea* pDragonTask = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    pDragonTask->mC8_normalLightColor.m0 = 0x10;
    pDragonTask->mC8_normalLightColor.m1 = 0x10;
    pDragonTask->mC8_normalLightColor.m2 = 0x10;

    pDragonTask->mCB_falloffColor0.m0 = 9;
    pDragonTask->mCB_falloffColor0.m1 = 9;
    pDragonTask->mCB_falloffColor0.m2 = 9;

    pDragonTask->mCE_falloffColor1.m0 = -5;
    pDragonTask->mCE_falloffColor1.m1 = -5;
    pDragonTask->mCE_falloffColor1.m2 = -5;

    pDragonTask->mD1_falloffColor2.m0 = 0x19;
    pDragonTask->mD1_falloffColor2.m1 = 0x19;
    pDragonTask->mD1_falloffColor2.m2 = 0x19;

    pDragonTask->mC0_lightRotationAroundDragon = 0xC000000;
}
