#pragma once

#include <map>

#include "collisionRegistry.h"
#include "kernel/worldGrid.h"

typedef s32(*scriptFunction_zero_arg)();
typedef s32(*scriptFunction_one_arg)(s32 arg0);
typedef s32(*scriptFunction_one_arg_ptr)(sSaturnPtr arg0);
typedef s32(*scriptFunction_two_arg)(s32 arg0, s32 arg1);
typedef s32(*scriptFunction_three_arg)(s32 arg0, s32 arg1, s32 arg2);
typedef s32(*scriptFunction_four_arg)(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

struct sKernelScriptFunctions
{
    std::map<u32, scriptFunction_zero_arg> m_zeroArg;
    std::map<u32, scriptFunction_one_arg> m_oneArg;
    std::map<u32, scriptFunction_one_arg_ptr> m_oneArgPtr;
    std::map<u32, scriptFunction_two_arg> m_twoArg;
    std::map<u32, scriptFunction_three_arg> m_threeArg;
    std::map<u32, scriptFunction_four_arg> m_fourArg;
};

struct sTownGridSetup
{
    s8 m0_width;
    s8 m1_height;
    fixedPoint m4_cellSize;
    const struct sGrid* m8_pGrid;
    s32 mC_numEnvLCSTargets;
    std::vector<sVec3_FP> m10_nLCSTargets;
};

struct sTownSetup
{
    s8 m0;
    const sTownGridSetup* m4_gridSetup;
    std::vector<sSaturnPtr> m8_scripts;
    // size 0xC
};

struct sTownOverlay : public sSaturnMemoryFile
{
    sTownOverlay(const char* fileName) : sSaturnMemoryFile(fileName)
    {
    }

    virtual sTownObject* createObjectTaskFromEA_siblingTaskWithEAArgWithCopy(struct npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg) = 0;
    virtual sTownObject* createObjectTaskFromEA_subTaskWithEAArg(struct npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg) = 0;

    sKernelScriptFunctions overlayScriptFunctions;

    std::vector<const sTownSetup*> mTownSetups;
};

const sTownSetup* readTownSetup(sSaturnPtr ptr, int numScripts);

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

struct sNPC : public sTownObject
{
    // m8_MCBInDram / m8_cellNode inherited from sTownObject union
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
    sCollisionBody m84;
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
    const std::vector<const sTownSetup*>* m60_townSetup;
    const std::vector <sSaturnPtr>* m64_scriptList;
    s32 m68_numEnvLCSTargets;
    const std::vector<sVec3_FP>* m6C_LCSTargets;
    std::array<NPCProxy, 16> m70_npcPointerArray;
    s32 mF0;
    s32 mF4;
    s32 mF8;
    s32 mFC; // 0x10: LCS Active
    s32 m100;
    sRunningScriptContext m104_currentScript;
    s32 m118_currentResult;
    std::array<sSaturnPtr, 0x10>::iterator m11C_currentStackPointer;
    std::array<sSaturnPtr, 0x10> m120_stack;
    sVec3_FP* m160_pEdgePosition;
    s_cinematicBarTask* m164_cinematicBars;
    s32 m168;
    union {
        s_vdp2StringTask* m16C_displayStringTask; //TODO: those are likely the same and should be merged
        s_receivedItemTask* m16C_receivedItemTask;
    };
    struct s_multiChoiceTask* m170_multiChoiceTask;

    //size 0x174
};

extern sNpcData npcData0;

struct townDebugTask2Function : public s_workAreaTemplate<townDebugTask2Function>
{
    static void Update(townDebugTask2Function* pThis);
    //size 0
};

extern townDebugTask2Function* townDebugTask2;

p_workArea loadTown(p_workArea r4, s32 r5);
p_workArea loadTown2(p_workArea r4, s32 r5);

void loadTownPrg(s8 r4, s8 r5);

void startScriptTask(p_workArea r4);

void initTownGrid();

extern p_workArea townVar0;

s32 initNPC(s32 arg);
s32 initNPCFromStruct(sSaturnPtr);

// Aliases for backward compatibility
using sTownCellTask = sWorldGridCellTask;
using sTownGrid = sWorldGrid;
#define gTownGrid gWorldGrid

struct sCameraTask : public s_workAreaTemplate<sCameraTask>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sCameraTask::Init, &sCameraTask::Update, nullptr, nullptr };
        return &taskDefinition;
    }

    static void Init(sCameraTask* pThis)
    {
        if (mainGameState.getBit(1)) {
            pThis->m4 = 5400;
        }
        else {
            pThis->m4 = 0;
        }
    }
    static void Update(sCameraTask* pThis)
    {
        if ((npcData0.mFC & 1) == 0) {
            if (++pThis->m4 > 5400) {
                pThis->m4 = 5400;
            }
            
        }
    }

    u8 m0;
    u8 m1;
    u8 m2;
    s32 m4;
    sSaturnPtr m8;
    u32 mC;
    s_RGB8 m10;
    sVec3_FP m14;
    sVec3_FP m20_lightPosition;
    u32 m2C;
    u32 m30;
    fixedPoint m34_interpolatedLightData[12]; // 0x34-0x64, used by zoahCamera_update
    //size 0x64
};

extern sCameraTask* cameraTaskPtr;
p_workArea startCameraTask(p_workArea pParent);

s32 isDataLoaded(s32 fileIndex);

// todo: move out of twn_ruin
void registerNpcs(const std::vector<const sTownSetup*>& townSetups, sSaturnPtr r5_script, s32 r6);
p_workArea startCameraTask(p_workArea pParent);
extern s32* twnVar1;
extern s32 twnVar2;
void drawLcs();
void updateEdgePosition(sNPC* r4);
s32 TwnFadeIn(s32 arg0);
void removeNPC(p_workArea pThisAsTask, sTownObject* pThis, sSaturnPtr r5);

npcFileDeleter* loadNPCFile2(p_workArea r4, const std::string& ramFileName, s32 ramFileSize, s32 index);
void initNPCSub0(npcFileDeleter* buffer, const sGrid* pGrid, u8 gridSizeX, u8 gridSizeY, fixedPoint cellSize);
fixedPoint generateObjectMatrix(sSaturnPtr r4, sSaturnPtr r5);

void townOverlayDelete(townDebugTask2Function* pThis);
