#pragma once

struct sNPCE8
{
    sVec3_FP m0_position;
    sVec3_FP mC_rotation;
};

struct sMainLogic_74
{
    s8 m0;
    s8 m1;
    s8 m2;
    fixedPoint m4;
    s32 m8;
    sVec3_FP m14;
    sVec3_FP m20;
    s32 m2C;
};

struct sNPC
{
    s8 mC;
    s8 mD;
    s8 mE;
    s8 mF;
    sSaturnPtr m10_InitPtr;
    void (*m14_updateFunction)(sNPC*);
    sSaturnPtr m18;
    s32 m1C;
    sVec2_FP* m20;
    s32 m30;
    s_3dModel m34_3dModel;
    sMainLogic_74 m84;
    sNPCE8 mE8;
    sVec3_FP* mB4_pPosition;
    sVec3_FP* mB8_pRotation;
    p_workArea mBC_pOwner;
    s32 mC0;
    u8* mC4;
};

struct sNpcData
{
    s32 m0_numExtraScriptsIterations;
    fixedPoint m54_activationNear;
    fixedPoint m58_activationFar;
    s8 m5C;
    s8 m5D;
    s8 m5E;
    sSaturnPtr m60;
    sSaturnPtr m64_scriptList;
    s32 m68_nulLCSTargets;
    sSaturnPtr m6C_LCSTargets;
    std::array<sNPC*, 8> m70_npcPointerArray;
    s32 mF0;
    s32 mF4;
    s32 mF8;
    s32 mFC;
    s32 m100;
    sSaturnPtr m104_scriptPtr;
    s32 m108;
    sNPC* m10C;
    s32 m116;
    s32 m118_currentResult;
    std::array<sSaturnPtr, 0x10>::iterator m11C_currentStackPointer;
    std::array<sSaturnPtr, 0x10> m120_stack;
    s32 m164;
    s32 m168;
    s_vdp2StringTask* m16C_displayStringTask;
    s32 m170;
};

extern sNpcData npcData0;

struct townDebugTask2Function : public s_workAreaTemplate<townDebugTask2Function>
{
    static void Update(townDebugTask2Function* pThis);
    //size 0
};

extern townDebugTask2Function* townDebugTask2;
extern u8 townBuffer[0xB0000];

p_workArea loadTown(p_workArea r4, s32 r5);
void loadTownPrg(s8 r4, s8 r5);

void startScriptTask(p_workArea r4);

void mainLogicInitSub0(sMainLogic_74*, s32);
void mainLogicInitSub1(sMainLogic_74*, sSaturnPtr, sSaturnPtr);

struct npcFileDeleter : public s_workAreaTemplateWithCopy<npcFileDeleter>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { nullptr, nullptr, nullptr, &npcFileDeleter::Delete };
        return &taskDefinition;
    }

    static void Delete(npcFileDeleter* pThis)
    {
        TaskUnimplemented();
    }

    //copy 0/4
    s16 m8;
    s16 mA;
    s32 mC;
    //size 0x10
};

npcFileDeleter* allocateNPC(struct sScriptTask* r4, s32 r5);

s32 initNPC(s32 arg);
