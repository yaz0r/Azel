#pragma once

void initVdp1Ram(s_workArea* pWorkArea, u32 vdp1EA, u32 bufferSize);

struct s_vdp1AllocationNode
{
    s_vdp1AllocationNode* m_0;
    u16 m4_vdp1Memory;
    u16 m6_size;
    //size 8
};

s_vdp1AllocationNode* vdp1Allocate(u32 size);

extern struct s_vdp1Allocator* vdp1AllocatorHead;
