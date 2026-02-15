#pragma once

struct s_heapNode
{
    u32 m_magic;
    s_heapNode* m_nextNode; // or the linked list of dependent allocation (if it's an allocated task)
    u32 m_size;

    s_heapNode()
    {
        m_nextNode = NULL;
        m_size = 0;
    }

    void* getUserData()
    {
        return this + 1;
    }
};

void initHeap();
void* allocateHeap(u32 size);
void* allocateHeapForTask(struct s_workArea* pWorkArea, u32 size);
void freeHeap(void* pUserData);
void freeHeapForTask(struct s_workArea* pWorkArea, void* pUserData);
