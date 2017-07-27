#include "PDS.h"

s_heapNode heapRoot;

u8 heap[0x30000];

void initHeap()
{
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
