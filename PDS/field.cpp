#include "PDS.h"

s_fieldTaskWorkArea* fieldTaskPtr = NULL;

u8 fieldVdp1Buffer[0x4000];
u8 fieldMainBuffer[0x68000];
u8 vdp1FieldArea[0x5F800];

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
    fieldTaskPtr->pSubFieldData->memoryArea_bottom = buffer;
    fieldTaskPtr->pSubFieldData->memoryArea_edge = buffer;
    fieldTaskPtr->pSubFieldData->memoryArea_top = buffer + bufferSize;
}

void initFieldVdp1Area(u8* buffer, u32 bufferSize)
{
    fieldTaskPtr->pSubFieldData->characterArea_bottom = buffer;
    fieldTaskPtr->pSubFieldData->characterArea_edge = buffer;
    fieldTaskPtr->pSubFieldData->characterArea_top = buffer + bufferSize;
}

void loadCommonFieldResources()
{
    initDramAllocator(fieldTaskPtr, playerDataMemoryBuffer, sizeof(playerDataMemoryBuffer), NULL);

    allocateVramList(fieldTaskPtr->overlayTaskData, fieldVdp1Buffer, sizeof(fieldVdp1Buffer));

    initFieldMemoryArea(fieldMainBuffer, sizeof(fieldMainBuffer));
    initFieldVdp1Area(vdp1FieldArea, sizeof(vdp1FieldArea));
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
    u32* MCBSizes = fieldTaskPtr->pSubFieldData->MCBFilesSizes;
    u32* CGBSizes = fieldTaskPtr->pSubFieldData->CGBFilesSizes;

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
    fieldTaskPtr->pSubFieldData->fileList = fileList;

    checkFilesExists(fileList);

    fieldTaskPtr->pSubFieldData->memoryArea[0] = fieldTaskPtr->pSubFieldData->memoryArea_bottom;
    fieldTaskPtr->pSubFieldData->memoryArea[1] = fieldTaskPtr->pSubFieldData->memoryArea[0] + fieldTaskPtr->pSubFieldData->MCBFilesSizes[0]; // TODO: should be aligned
    fieldTaskPtr->pSubFieldData->memoryArea[2] = fieldTaskPtr->pSubFieldData->memoryArea[1] + fieldTaskPtr->pSubFieldData->MCBFilesSizes[1]; // TODO: should be aligned

    fieldTaskPtr->pSubFieldData->characterArea[0] = fieldTaskPtr->pSubFieldData->characterArea_bottom;
    fieldTaskPtr->pSubFieldData->characterArea[1] = fieldTaskPtr->pSubFieldData->characterArea[0] + fieldTaskPtr->pSubFieldData->CGBFilesSizes[0]; // TODO: should be aligned
    fieldTaskPtr->pSubFieldData->characterArea[2] = fieldTaskPtr->pSubFieldData->characterArea[1] + fieldTaskPtr->pSubFieldData->CGBFilesSizes[1]; // TODO: should be aligned

    u32 currentFileIndex = 2;
    const s_MCB_CGB* currentFileList = fileList + currentFileIndex;

    u32 largestMCB = 0;
    u32 largestCGB = 0;

    while (currentFileList->MCB)
    {
        largestMCB = std::max(fieldTaskPtr->pSubFieldData->MCBFilesSizes[currentFileIndex], largestMCB);
        largestCGB = std::max(fieldTaskPtr->pSubFieldData->CGBFilesSizes[currentFileIndex], largestCGB);

        currentFileList++;
        currentFileIndex++;
    }

    fieldTaskPtr->pSubFieldData->memoryArea_edge = fieldTaskPtr->pSubFieldData->memoryArea[2] + largestMCB; // TODO: should be aligned
    fieldTaskPtr->pSubFieldData->characterArea_edge = fieldTaskPtr->pSubFieldData->characterArea[2] + largestCGB; // TODO: should be aligned
}

s32 getFieldMemoryAreaRemain()
{
    return fieldTaskPtr->pSubFieldData->memoryArea_top - fieldTaskPtr->pSubFieldData->memoryArea_edge;
}

s32 getFieldCharacterAreaRemain()
{
    return fieldTaskPtr->pSubFieldData->characterArea_top - fieldTaskPtr->pSubFieldData->characterArea_edge;
}

void loadFileFromFileList(u32 index)
{
    const s_MCB_CGB* pFileData = &fieldTaskPtr->pSubFieldData->fileList[index];

    int slot = std::min(2, (int)index);

    if (pFileData->MCB)
    {
        loadFile(pFileData->MCB, fieldTaskPtr->pSubFieldData->memoryArea[slot], fieldTaskPtr->pSubFieldData->characterArea[slot]);
    }

    if (pFileData->CGB)
    {
        loadFile(pFileData->CGB, fieldTaskPtr->pSubFieldData->characterArea[slot], NULL);
    }
}

void getMemoryArea(s_memoryAreaOutput* pOutput, u32 areaIndex)
{
    if (areaIndex > 2)
    {
        areaIndex = 2;
    }

    pOutput->mainMemory = fieldTaskPtr->pSubFieldData->memoryArea[areaIndex];
    pOutput->characterArea = fieldTaskPtr->pSubFieldData->characterArea[areaIndex];
}
