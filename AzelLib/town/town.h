#pragma once

struct sNPCE8
{
    sVec3_FP m0_position;
    sVec3_FP mC_rotation;
};

struct sNPC
{
    sNPCE8 mE8;
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
    s32 m10C;
    s32 m116;
    s32 m118_currentResult;
    std::array<sSaturnPtr, 0x10>::iterator m11C_currentStackPointer;
    std::array<sSaturnPtr, 0x10> m120_stack;
    s32 m164;
    s32 m168;
    s32 m16C;
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

struct sMainLogic_74
{
    s8 m0;
    s8 m1;
    s8 m2;
    fixedPoint m4;
    sVec3_FP m14;
    sVec3_FP m20;
    s32 m2C;
};

void mainLogicInitSub0(sMainLogic_74*, s32);
void mainLogicInitSub1(sMainLogic_74*, sSaturnPtr, sSaturnPtr);

s32 initNPC(s32 arg);
