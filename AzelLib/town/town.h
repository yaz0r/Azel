#pragma once

struct sNPCE8
{
    sVec3_FP m0_position;
    sVec3_FP mC_rotation;
    sVec3_FP m18_stepTranslationInWorld;
    sVec3_FP m24_stepRotation;
    sVec3_FP m30_stepTranslation;
    sVec3_FP m3C_targetPosition;
    sVec3_FP m48_targetRotation;
    sVec3_FP m54_oldPosition;
};

struct sMainLogic_74
{
    s8 m0;
    s8 m1;
    s16 m2_collisionLayersBitField;
    fixedPoint m4;
    sVec3_FP m8_position;
    sVec3_FP m14;
    sVec3_FP m20;
    s32 m2C;
    sVec3_FP* m30_pPosition;
    sVec3_FP* m34_pRotation;
    p_workArea m38_pOwner;
    s32 m3C;
    u8* m40;
    s32 m44;
    sMainLogic_74* m48;
    sVec3_FP m4C;
    sVec3_FP m58_collisionSolveTranslation;
};

struct sNPC
{
    u8* m8_MCBInDram;
    s8 mC;
    s8 mD;
    s8 mE_controlState;
    s8 mF; // 0x80 = enable shadow
    sSaturnPtr m10_InitPtr;
    void (*m14_updateFunction)(sNPC*);
    sSaturnPtr m18;
    s32 m1C;
    sVec2_FP m20;
    fixedPoint m28;
    fixedPoint m2C_currentAnimation;
    sSaturnPtr m30_animationTable;
    s_3dModel m34_3dModel;
    sMainLogic_74 m84;
    sNPCE8 mE8;
};

struct sNpcData4
{
    s32 m0;
    s32 m4;
    p_workArea m8;
    sVec3_S16 mC;
    //size 0x14
};

struct sNpcData
{
    s32 m0_numExtraScriptsIterations;
    std::array<sNpcData4, 4> m4;
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
    sVec3_FP* m160_pEdgePosition;
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
        FunctionUnimplemented();
    }

    //copy 0/4
    s16 m8;
    s16 mA;
    s32 mC;
    //size 0x10
};

npcFileDeleter* allocateNPC(struct sScriptTask* r4, s32 r5);

extern p_workArea townVar0;

s32 initNPC(s32 arg);
s32 initNPCFromStruct(s32);
void mainLogicUpdateSub0(fixedPoint r4_x, fixedPoint r5_y);

struct sInitNPCSub0Var0Sub144
{
    sInitNPCSub0Var0Sub144* m0_next;
    //size 0xC
};

struct sEnvironmentTask : public s_workAreaTemplateWithArgWithCopy<sEnvironmentTask, sSaturnPtr>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sEnvironmentTask::Init, nullptr, &sEnvironmentTask::Draw, nullptr };
        return &taskDefinition;
    }

    static void Init(sEnvironmentTask* pThis, sSaturnPtr arg);
    static void Draw(sEnvironmentTask* pThis);

    //size 0x18
    sSaturnPtr m8;
    sVec3_FP mC_position;
};


struct sTownGrid
{
    s32 m0_sizeX;
    s32 m4_sizeY;
    s32 m8;
    s32 mC;
    s32 m10_currentX;
    s32 m14_currentY;
    void(*m18_createCell)(s32, sTownGrid*);
    void(*m1C)();
    void(*m20_deleteCell)(s32, sTownGrid*);
    void(*m24)();
    fixedPoint m28_cellSize;
    fixedPoint m2C;
    fixedPoint m30_worldToCellIndex;
    npcFileDeleter* m34;
    sSaturnPtr m38_EnvironmentSetup;
    s32* m3C;
    std::array<std::array<sEnvironmentTask*, 8>, 8> m40_cellTasks;
    std::vector<sSaturnPtr> m140;
    sInitNPCSub0Var0Sub144* m144;
    std::array<sInitNPCSub0Var0Sub144, 0x40> m148;

};

extern sTownGrid gTownGrid;

// todo: kernel
s32 MTH_Mul32(fixedPoint a, fixedPoint b);
