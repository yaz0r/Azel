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

void checkFilesExists(const char** fileList)
{
    assert(0);
}

void setupFileList(const char** fileList)
{
    fieldTaskPtr->pSubFieldData->fileList = fileList;

    checkFilesExists(fileList);

    assert(0);
}
