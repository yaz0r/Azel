#include "PDS.h"
#include "graphicalObject.h"
#include "vdp1Allocator.h"
#include "fileBundle.h"

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

void npcFileDeleterCleanup(npcFileDeleter* pThis);

void npcFileDeleter::Delete(npcFileDeleter* pThis)
{
    npcFileDeleterCleanup(pThis);

    if (pThis->mC >= 0 && dramAllocatorEnd[pThis->mC].mC_fileBundle == pThis) {
        dramAllocatorEnd[pThis->mC].m8_refcount = 0;
        dramAllocatorEnd[pThis->mC].mC_fileBundle = nullptr;
    }
}

// equivalent of loadDragonSub1Sub1 for npcFileDeleter
void npcFileDeleterCleanup(npcFileDeleter* pThis)
{
    // On Saturn, pending async file handles (m8, mA) would be cancelled here
    // Our loading is synchronous so these are always -1

    if (pThis->m0_fileBundle) {
        dramFree(pThis->m0_fileBundle->getRawBuffer());
        pThis->m0_fileBundle = nullptr;
    }
    if (pThis->m4_vd1Allocation) {
        vdp1Free((u8*)pThis->m4_vd1Allocation);
        pThis->m4_vd1Allocation = nullptr;
    }
}

