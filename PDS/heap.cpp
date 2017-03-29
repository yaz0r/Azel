#include "PDS.h"

s_heapNode heapRoot;

u8 heap[0x30000];

void initHeap()
{
    s_heapNode* heapStart = (s_heapNode*)heap;

    heapRoot.m_nextNode = heapStart;
    heapRoot.m_size = 0;

    heapStart->m_nextNode = NULL;
    heapStart->m_size = 0x30000;
}

void* allocateHeap(u32 size)
{
    if (size == 0)
        return NULL;

    size += 0x17; // does that still work with 64bit pointers?
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
            return pCurrentBlock + 1; // return the user data of that block
        }
    }

    return NULL;
}

void* allocateHeapForTask(s_workArea* pWorkArea, u32 size)
{
    if (size == 0)
        return NULL;

    s_heapNode* pTaskHeapNode = pWorkArea->getTask()->getHeapNode();

    size += 0x17; // does that still work with 64bit pointers?
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
            pCurrentBlock->m_nextNode = pTaskHeapNode->m_nextNode;
            pTaskHeapNode->m_nextNode = pCurrentBlock;
            return pCurrentBlock + 1; // return the user data of that block
        }
    }

    return NULL;
}

void freeHeap(void* pUserData)
{
    if (pUserData == NULL)
        return;


}
