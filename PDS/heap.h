#pragma once

struct s_heapNode
{
    s_heapNode* m_nextNode; // or the linked list of dependent allocation (if it's an allocated task)
    u32 m_size;

    void* getUserData()
    {
        return this + 1;
    }
};

void initHeap();
void* allocateHeap(u32 size);
void* allocateHeapForTask(struct s_workArea* pWorkArea, u32 size);
void freeHeap(void* pUserData);
