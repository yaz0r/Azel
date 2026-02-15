#pragma once

struct npcFileDeleter : public s_workAreaTemplateWithCopy<npcFileDeleter>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { nullptr, nullptr, nullptr, &npcFileDeleter::Delete };
        return &taskDefinition;
    }

    static void Delete(npcFileDeleter* pThis)
    {
        Unimplemented();
    }

    ////////////////////////////
    static TypedTaskDefinition* getTypedTaskDefinition_townObject()
    {
        static TypedTaskDefinition taskDefinition = { nullptr, &npcFileDeleter::Update_TownObject, nullptr, &npcFileDeleter::Delete };
        return &taskDefinition;
    }

    static void Update_TownObject(npcFileDeleter* pThis)
    {
        Unimplemented();
    }

    //////////////////////////////


    //copy 0/4
    s16 m8;
    s16 mA;
    s32 mC;
    //size 0x10
};

npcFileDeleter* allocateNPC(p_workArea r4, s32 r5);
npcFileDeleter* loadNPCFile2(p_workArea r4, const std::string& ramFileName, s32 ramFileSize, s32 index);
