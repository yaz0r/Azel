#pragma once

struct npcFileDeleter : public s_workAreaTemplateWithCopy<npcFileDeleter>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { nullptr, nullptr, nullptr, &npcFileDeleter::Delete };
        return &taskDefinition;
    }

    static void Delete(npcFileDeleter* pThis);

    ////////////////////////////
    static TypedTaskDefinition* getTypedTaskDefinition_townObject()
    {
        static TypedTaskDefinition taskDefinition = { nullptr, &npcFileDeleter::Update_TownObject, nullptr, &npcFileDeleter::Delete };
        return &taskDefinition;
    }

    static void Update_TownObject(npcFileDeleter* pThis)
    {
        // Check if main file has finished loading
        if (pThis->m8 >= 0)
        {
            pThis->m8 = -1;
        }
        // Check if VDP1 file has finished loading
        if (pThis->mA >= 0)
        {
            pThis->mA = -1;
        }
        // Once both files are loaded, stop updating
        if (pThis->m8 < 0 && pThis->mA < 0)
        {
            pThis->m_UpdateMethod = nullptr;
        }
    }

    //////////////////////////////


    //copy 0/4
    s16 m8;
    s16 mA;
    s32 mC;
    //size 0x10
};

void npcFileDeleterCleanup(npcFileDeleter* pThis);
npcFileDeleter* allocateNPC(p_workArea r4, s32 r5);
npcFileDeleter* loadNPCFile(p_workArea r4, const std::string& ramFileName, s32 ramFileSize, const std::string& vramFileName, s32 vramFileSize, s32 arg);
npcFileDeleter* loadNPCFile2(p_workArea r4, const std::string& ramFileName, s32 ramFileSize, s32 index);
