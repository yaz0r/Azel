#include "PDS.h"

static const u32 HEAP_MAGIC = 0xCAFEC0DE;

#ifdef USE_LEGACY_TASK_HEAP
s_heapNode heapRoot;

u8* heap = nullptr;;

void initHeap()
{
    heap = new u8[0x30000];
    s_heapNode* heapStart = (s_heapNode*)heap;

    memset(heap, 0xFF, 0x30000);

    heapRoot.m_nextNode = heapStart;
    heapRoot.m_size = 0;

    heapStart->m_nextNode = NULL;
    heapStart->m_size = 0x30000;
}

void* allocateHeap(u32 size)
{
    if (size == 0)
        return NULL;

    assert(sizeof(s_heapNode) <= 0x10);

    // account for header size
    size += 0x10;

    // align to 16
    size += 0xF;
    size &= 0xFFFFFFF0;

    s_heapNode* previousBlock = &heapRoot;

    while (s_heapNode* pCurrentBlock = previousBlock->m_nextNode)
    {
        if (pCurrentBlock->m_size >= size)
        {
            if (pCurrentBlock->m_size != size)
            {
                // split block
                s_heapNode* pNewNode = (s_heapNode*)(((u8*)pCurrentBlock) + size);
                previousBlock->m_nextNode = pNewNode;
                u32 leftoverSize = pCurrentBlock->m_size - size;
                pNewNode->m_nextNode = pCurrentBlock->m_nextNode;
                pNewNode->m_size = leftoverSize;
                pCurrentBlock->m_size = size;
            }
            else
            {
                previousBlock->m_nextNode = pCurrentBlock->m_nextNode;
            }

            // pCurrentBlock is a block of expected size
            pCurrentBlock->m_nextNode = NULL;

            u8* userData = ((u8*)pCurrentBlock) + 0x10;

            memset(userData, 0xCC, pCurrentBlock->m_size - 0x10);

            return userData; // return the user data of that block
        }

        previousBlock = pCurrentBlock;
    }

    return NULL;
}

void* allocateHeapForTask(s_workArea* pWorkArea, u32 size)
{
    if (size == 0)
        return NULL;

    assert(sizeof(s_heapNode) <= 0x10);

    // account for header size
    size += 0x10;

    // align to 16
    size += 0xF;
    size &= 0xFFFFFFF0;

    s_heapNode* pTaskHeapNode = pWorkArea->getTask()->getHeapNode();
    s_heapNode* previousBlock = &heapRoot;

    while (s_heapNode* pCurrentBlock = previousBlock->m_nextNode)
    {
        if (pCurrentBlock->m_size >= size)
        {
            if (pCurrentBlock->m_size != size)
            {
                // split block
                s_heapNode* pNewNode = (s_heapNode*)(((u8*)pCurrentBlock) + size);
                previousBlock->m_nextNode = pNewNode;
                u32 leftoverSize = pCurrentBlock->m_size - size;
                pNewNode->m_nextNode = pCurrentBlock->m_nextNode;
                pNewNode->m_size = leftoverSize;
                pCurrentBlock->m_size = size;
            }
            else
            {
                previousBlock->m_nextNode = pCurrentBlock->m_nextNode;
            }

            // pCurrentBlock is a block of expected size
            pCurrentBlock->m_nextNode = pTaskHeapNode->m_nextNode;
            pTaskHeapNode->m_nextNode = pCurrentBlock;

            u8* userData = ((u8*)pCurrentBlock) + 0x10;

            memset(userData, 0xCC, pCurrentBlock->m_size - 0x10);

            return userData; // return the user data of that block
        }

        previousBlock = pCurrentBlock;
    }

    return NULL;
}

void freeHeap(void* pUserData)
{
    if (pUserData == NULL)
        return;


}
#else
void initHeap()
{
}

void* allocateHeap(u32 size)
{
    return new u8[size];
}

void* allocateHeapForTask(s_workArea* pWorkArea, u32 size)
{
    s_heapNode** pTaskHeapNode = &pWorkArea->getTask()->m_heapNode;
    while (*pTaskHeapNode)
    {
        pTaskHeapNode = &(*pTaskHeapNode)->m_nextNode;
    }
    u32 allocatedSize = size + sizeof(s_heapNode);
    u8* rawBuffer = new u8[allocatedSize];

    memset(rawBuffer, 0, allocatedSize);

    s_heapNode* newNode = (s_heapNode*)rawBuffer;
    newNode->m_magic = HEAP_MAGIC;
    newNode->m_nextNode = nullptr;
    newNode->m_size = size;

    *pTaskHeapNode = newNode;

    return newNode->getUserData();
}

void freeHeap(void* pUserData)
{
    delete[](u8*)pUserData;
}

void freeHeapForTask(struct s_workArea* pWorkArea, void* pUserData)
{
    s_heapNode* pHeapNode = (s_heapNode*)((u8*)pUserData - sizeof(s_heapNode));
    assert(pHeapNode->m_magic == HEAP_MAGIC);

    delete[](u8*)pHeapNode;
}
#endif
