#include "PDS.h"
#include "graphicalObject.h"
#include "vdp1Allocator.h"

npcFileDeleter* loadNPCFile(p_workArea r4, const std::string& ramFileName, s32 ramFileSize, const std::string& vramFileName, s32 vramFileSize, s32 arg)
{
    npcFileDeleter* r13 = createSubTask<npcFileDeleter>(r4);

    s_vdp1AllocationNode* r14_vdp1Memory = nullptr;
    if (vramFileSize)
    {
        r14_vdp1Memory = vdp1Allocate(vramFileSize);
    }

    s_fileBundle* r12_dramMemory = nullptr;

    if (ramFileSize)
    {
        if (r14_vdp1Memory)
        {
            loadFile(ramFileName.c_str(), &r12_dramMemory, r14_vdp1Memory->m4_vdp1Memory);
        }
        else
        {
            loadFile(ramFileName.c_str(), &r12_dramMemory, 0);
        }
    }

    if (r14_vdp1Memory)
    {
        loadFile(vramFileName.c_str(), getVdp1Pointer((r14_vdp1Memory->m4_vdp1Memory << 3) + 0x25C00000), 0);
    }

    r13->m0_fileBundle = r12_dramMemory;
    r13->m4_vd1Allocation = r14_vdp1Memory;
    r13->mA = -1;
    r13->m8 = -1;
    r13->mC = arg;

    return r13;
}

npcFileDeleter* loadNPCFile2(p_workArea r4, const std::string& ramFileName, s32 ramFileSize, s32 index)
{
    npcFileDeleter* r13 = createSubTask<npcFileDeleter>(r4);

    s_fileBundle* r12_dramMemory = nullptr;
    loadFile(ramFileName.c_str(), &r12_dramMemory, 0);

    r13->m0_fileBundle = r12_dramMemory;
    r13->m4_vd1Allocation = nullptr;
    r13->mA = -1;
    r13->m8 = -1;
    r13->mC = index;

    //TODO: slave cachePurge

    return r13;
}


npcFileDeleter* allocateNPC(p_workArea r4, s32 r5)
{
    s_fileEntry& r14 = dramAllocatorEnd[r5];
    if (r14.m8_refcount++)
    {
        return r14.mC_fileBundle;
    }

    std::string vdp1FileName = "";
    u32 vdp1FileSize = 0;

    if (dramAllocatorEnd.size() > r5 + 1)
    {
        s_fileEntry& vdp1File = dramAllocatorEnd[r5 + 1];
        vdp1FileName = vdp1File.mFileName;
        vdp1FileSize = vdp1File.m4_fileSize;
    }

    r14.mC_fileBundle = loadNPCFile(r4, r14.mFileName, r14.m4_fileSize, vdp1FileName, vdp1FileSize, r5);
    return r14.mC_fileBundle;
}

void freeVdp1Block(npcFileDeleter*, s32)
{
    assert(0);
}

