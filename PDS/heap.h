#pragma once

struct s_heapNode
{
    s_heapNode* m_nextNode;
    u32 m_size;

    void* getUserData()
    {
        return this + 1;
    }
};

void initHeap();
void* allocateHeap(u32 size);
void freeHeap(void* pUserData);
