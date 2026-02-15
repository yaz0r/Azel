#include "PDS.h"
#include "vdp1Allocator.h"

struct s_vdp1Allocator
{
    s_vdp1Allocator* m0_nextAllocator;
    s_vdp1AllocationNode* m4_nextNode;
    s_vdp1AllocationNode m8_freeHead;
    s_vdp1AllocationNode m10_allocatedHead;
    s_vdp1AllocationNode m18_list[0x40];
    //size 0x218
};

s_vdp1Allocator* vdp1AllocatorHead = nullptr;

void initVdp1Ram(s_workArea* pWorkArea, u32 vdp1MemoryEA, u32 bufferSize)
{
    s_vdp1Allocator* r5 = static_cast<s_vdp1Allocator*>(allocateHeapForTask(pWorkArea, sizeof(s_vdp1Allocator)));

    r5->m0_nextAllocator = vdp1AllocatorHead;
    vdp1AllocatorHead = r5;

    s_vdp1AllocationNode* r7 = &r5->m8_freeHead;
    s_vdp1AllocationNode* r4 = &r5->m10_allocatedHead;

    r7->m_0 = r4;
    r7->m4_vdp1Memory = 0;
    r7->m6_size = 0;

    r4->m_0 = 0;
    r4->m4_vdp1Memory = vdp1MemoryEA >> 3;
    r4->m6_size = bufferSize >> 3;

    r5->m4_nextNode = r5->m18_list;

    for (u32 i = 0; i < 0x3F; i++)
    {
        r5->m18_list[i].m_0 = &r5->m18_list[i + 1];
    }

    r5->m18_list[0x3F].m_0 = NULL;
}

s_vdp1AllocationNode* vdp1Allocate(u32 size)
{
    if (size == 0)
        return nullptr;

    size = ((size + 0x1F) & ~0x1F) >> 3;

    s_vdp1AllocationNode* r7 = &vdp1AllocatorHead->m8_freeHead;
    while (r7->m_0)
    {
        s_vdp1AllocationNode* r5 = r7->m_0;
        if (r5->m6_size >= size)
        {
            if (r5->m6_size == size)
            {
                r7->m_0 = r5->m_0;
            }
            else
            {
                s_vdp1AllocationNode* r7 = vdp1AllocatorHead->m4_nextNode;
                s32 newSize = r5->m6_size - size;
                vdp1AllocatorHead->m4_nextNode = r7->m_0;
                r5->m6_size = newSize;
                r7->m4_vdp1Memory = newSize + r5->m4_vdp1Memory;
                r5 = r7;
                r7->m6_size = size;
            }
            r5->m_0 = nullptr;
            return r5;
        }
        r7 = r5;
    }

    return nullptr;
}
