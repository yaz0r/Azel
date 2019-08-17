#pragma once

#include <map>

struct sTownObject
{
    // 0-4 are copy
    struct sCellObjectListNode* m8;
};

typedef s32(*scriptFunction_zero_arg)();
typedef s32(*scriptFunction_one_arg)(s32 arg0);
typedef s32(*scriptFunction_one_arg_ptr)(sSaturnPtr arg0);
typedef s32(*scriptFunction_two_arg)(s32 arg0, s32 arg1);
typedef s32(*scriptFunction_four_arg)(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

struct sKernelScriptFunctions
{
    std::map<u32, scriptFunction_zero_arg> m_zeroArg;
    std::map<u32, scriptFunction_one_arg> m_oneArg;
    std::map<u32, scriptFunction_one_arg_ptr> m_oneArgPtr;
    std::map<u32, scriptFunction_two_arg> m_twoArg;
    std::map<u32, scriptFunction_four_arg> m_fourArg;
};

struct sTownOverlay : public sSaturnMemoryFile
{
    virtual void init() = 0;
    virtual sTownObject* createObjectTaskFromEA_siblingTaskWithEAArgWithCopy(struct npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg) = 0;
    virtual sTownObject* createObjectTaskFromEA_subTaskWithEAArg(struct npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg) = 0;

    sKernelScriptFunctions overlayScriptFunctions;
};

extern sTownOverlay* gCurrentTownOverlay;

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

struct sCollisionSetup
{
    s8 m0_collisionType;
    s8 m1;
    s16 m2_collisionLayersBitField;
};

struct sMainLogic_74
{
    sCollisionSetup m0_collisionSetup;
    fixedPoint m4_collisionRadius;
    sVec3_FP m8_position;
    sVec3_FP m14_collisionClip;
    sVec3_FP m20;
    s32 m2C_collisionSetupIndex;
    sVec3_FP* m30_pPosition;
    sVec3_FP* m34_pRotation;
    p_workArea m38_pOwner;
    sSaturnPtr m3C_scriptEA;
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
    s8 mF; // 0x80 = disable shadow
    sSaturnPtr m10_InitPtr;
    void (*m14_updateFunction)(sNPC*);
    sSaturnPtr m18;
    s32 m1C;
    sVec2_FP m20_lookAtAngle;
    fixedPoint m28_animationLeftOver;
    fixedPoint m2C_currentAnimation;
    sSaturnPtr m30_animationTable;
    s_3dModel m34_3dModel;
    sMainLogic_74 m84;
    sNPCE8 mE8;
};

struct sNPCOrTask
{
    void reset()
    {
        asTask = nullptr;
        asNPC = nullptr;
    }

    sNPC* getNPC()
    {
        assert(asNPC);
        return asNPC;
    }

    p_workArea getAsTask()
    {
        assert(asTask);
        return asTask;
    }

    void setNPC(sNPC* pNPC)
    {
        reset();
        asNPC = pNPC;
    }

    void setTask(p_workArea pTask)
    {
        reset();
        asTask = pTask;
    }
private:
    p_workArea asTask;
    sNPC* asNPC;
};

struct sRunningScriptContext
{
    sSaturnPtr m0_scriptPtr;
    s32 m4;
    sNPCOrTask m8_owner;
    sVec3_S16_12_4 mC;
    //size 0x14
};

struct NPCProxy
{
    p_workArea workArea;
    sNPC* pNPC;
};

struct sNpcData
{
    s32 m0_numBackgroundScripts;
    std::array<sRunningScriptContext, 4> m4_backgroundScripts;
    fixedPoint m54_activationNear;
    fixedPoint m58_activationFar;
    s8 m5C;
    s8 m5D;
    s8 m5E;
    sSaturnPtr m60_townSetup;
    sSaturnPtr m64_scriptList;
    s32 m68_numEnvLCSTargets;
    sSaturnPtr m6C_LCSTargets;
    std::array<NPCProxy, 32> m70_npcPointerArray;
    s32 mF0;
    s32 mF4;
    s32 mF8;
    s32 mFC; // 0x10: LCS Active
    s32 m100;
    sRunningScriptContext m104_currentScript;
    s32 m116;
    s32 m118_currentResult;
    std::array<sSaturnPtr, 0x10>::iterator m11C_currentStackPointer;
    std::array<sSaturnPtr, 0x10> m120_stack;
    sVec3_FP* m160_pEdgePosition;
    s_cinematicBarTask* m164_cinematicBars;
    s32 m168;
    union {
        s_vdp2StringTask* m16C_displayStringTask;
        s_receivedItemTask* m16C_receivedItemTask;
    };
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
void mainLogicInitSub1(sMainLogic_74*, const sVec3_FP&, const sVec3_FP&);

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

    ////////////////////////////
    static TypedTaskDefinition* getTypedTaskDefinition_townObject()
    {
        static TypedTaskDefinition taskDefinition = { nullptr, &npcFileDeleter::Update_TownObject, nullptr, &npcFileDeleter::Delete };
        return &taskDefinition;
    }

    static void Update_TownObject(npcFileDeleter* pThis)
    {
        FunctionUnimplemented();
    }

    //////////////////////////////


    //copy 0/4
    s16 m8;
    s16 mA;
    s32 mC;
    //size 0x10
};

npcFileDeleter* allocateNPC(struct sScriptTask* r4, s32 r5);

extern p_workArea townVar0;

s32 initNPC(s32 arg);
s32 initNPCFromStruct(sSaturnPtr);
void mainLogicUpdateSub0(fixedPoint r4_x, fixedPoint r5_y);

struct sCellObjectListNode
{
    sCellObjectListNode* m0_next;
    sSaturnPtr m4;
    sTownObject* m8;
    //size 0xC
};

struct sTownCellTask : public s_workAreaTemplateWithArgWithCopy<sTownCellTask, sSaturnPtr>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sTownCellTask::Init, nullptr, &sTownCellTask::Draw, nullptr };
        return &taskDefinition;
    }

    static void Init(sTownCellTask* pThis, sSaturnPtr arg);
    static void Draw(sTownCellTask* pThis);

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
    npcFileDeleter* m34_dataBuffer;
    sSaturnPtr m38_EnvironmentSetup;
    s32* m3C;
    std::array<std::array<sTownCellTask*, 8>, 8> m40_cellTasks;
    std::vector<sCellObjectListNode*> m140_perCellObjectList;
    sCellObjectListNode* m144_nextFreeObjectListNode;
    std::array<sCellObjectListNode, 0x40> m148_objectListNodes;

};

extern sTownGrid gTownGrid;

s32 isDataLoaded(s32 fileIndex);

// todo: kernel
s32 MTH_Mul32(fixedPoint a, fixedPoint b);

// todo: move out of twn_ruin
void registerNpcs(sSaturnPtr r4_townSetup, sSaturnPtr r5_script, s32 r6);
p_workArea startCameraTask(p_workArea pParent);
extern s32* twnVar1;
extern s32 twnVar2;
void mainLogicUpdateSub3();
void drawLcs();
void updateEdgePosition(sNPC* r4);
s32 TwnFadeOut(s32 arg0);
