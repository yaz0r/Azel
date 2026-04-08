#pragma once

#include "kernel/moduleManager.h"

struct s_fieldCameraConfig
{
    sVec3_FP m0_min;
    sVec3_FP mC_max;
    s32 m18[7];
    s32 m34[7];
    fixedPoint m50;
    fixedPoint m54;
    // size: 0x58
};
s_fieldCameraConfig* readCameraConfig(sSaturnPtr EA);

struct sFieldCameraZone
{
    sVec3_FP m0_center;
    sVec3_FP* mC_pPosition;
    sVec3_FP* m10_pPosition2;
    s32 m14_triggerRadius;
    s32 m18_maxDistanceSquare;
    // size 0x20?
};

struct sFieldCameraManager : public s_workAreaTemplate<sFieldCameraManager>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &fieldCameraManagerInit, NULL, NULL, NULL};
        return &taskDefinition;
    }

    static void fieldCameraManagerInit(sFieldCameraManager*);

    s32 m0_nextCamera;
    s32 m4_currentCamera;
    s32 m8_numFramesOnCurrentCamera;
    s32 mC;
    std::array<s_fieldCameraConfig,8> m10; // unknown size
    s32 m2D0_activeZoneIndex;
    s32 m2D4_candidateZoneIndex;
    s32 m2D8_zoneDwellCounter;
    s32 m2DC_numCameraZones;
    s32 m2E0_forcedZoneIndex;
    std::array<sFieldCameraZone,5> m2E4_cameraZones;
    sVec3_FP* m370_cutsceneLookAtPtr;
    sVec3_FP* m374_cutsceneCameraPos;
    s32 m378_cutsceneFrameCounter;
    s32 m37C_isCutsceneCameraActive;
    sMatrix4x3 m384_viewMatrix;
    sMatrix4x3 m3B4_projectionMatrix;
    std::array<sFieldCameraStatus,2> m3E4_cameraSlots;
    u8 m50C_activeCameraSlot;
    u8 m50D_isInitialized; // 50D
    u8 m50E_followModeIndex; // 50E
};

struct s_scriptData1
{
    s32 m0;
    sVec3_FP m4;
    sVec3_FP m10;
    sVec3_FP m1C;
    sVec3_FP m28;
    fixedPoint m34;
    fixedPoint m38;
    //size: 3C
};

struct s_scriptData2
{
    u32 m0;
    u32 m4;
    u32 m8;
    u32 mC;
    u32 m10;
    u32 m14;
    u32 m18;
    u32 m1C;
    u32 m20;
    //size: 24
};

struct s_scriptData3
{
    fixedPoint m0_duration;
    sVec3_FP m4_pos;
    fixedPoint m10_rotationDuration;
    sVec3_FP m14_rot;
};

struct s_animDataFrame
{
    s8 m0;
    s8 m1;
    s16 m2;
};

struct s_multiChoice
{
    s16* m0_choiceTable;
    s32 m4_currentChoice;
    s32 m8_numChoices;
    s32 mC;

    // size 0x10
};

struct s_riderAnimTask : public s_workAreaTemplate<s_riderAnimTask>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { NULL, &s_riderAnimTask::Update, NULL, &s_riderAnimTask::Delete};
        return &taskDefinition;
    }
    static void Update(s_riderAnimTask*);
    static void Delete(s_riderAnimTask*);

    s32 m0_status;
    s32 m4_riderIndex;
    s32 m8_delay;
    s32 mC_previousFrame;
    s_animDataFrame* m10_animSequence; // no sure
    s_loadRiderWorkArea* m14_riderState;
    s_loadRiderWorkArea* m18;
    const s32* m1C;
};

void cutsceneTaskInitSub2(p_workArea r4, std::vector<s_scriptData1>& r5, s32 r6, sVec3_FP* r7, u32 arg0);
void cutsceneTaskInitSub3(p_workArea r4, std::vector<s_scriptData2>& r5, s32 r6, sVec3_FP* r7, u32 arg0);

struct s_cutsceneTask : public s_workAreaTemplateWithArg<s_cutsceneTask, struct s_cutsceneData*>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &s_cutsceneTask::Init, &s_cutsceneTask::Update, NULL, NULL};
        return &taskDefinition;
    }
    static void Init(s_cutsceneTask*, struct s_cutsceneData* argument);
    static void Update(s_cutsceneTask*);

    u32 m0;
    u32 m4_changeField;
    s32 m8_fieldIndex;
    s32 mC_fieldParam;
    s32 m10_fieldExitIndex;
    s16 m14;
    u32 m18_frameCount;

    //size = 0x1C
};

struct s_cutsceneTask2 : public s_workAreaTemplateWithArg<s_cutsceneTask2, std::vector<s_scriptData1>*>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &s_cutsceneTask2::Init, &s_cutsceneTask2::Update, &s_cutsceneTask2::Draw, NULL};
        return &taskDefinition;
    }
    static void Init(s_cutsceneTask2*, std::vector<s_scriptData1>* argument);
    static void Update(s_cutsceneTask2*);
    static void Draw(s_cutsceneTask2*)
    {
        PDS_unimplemented("s_cutsceneTask2::Draw");
    }

    u32 m0;
    std::vector<s_scriptData1>* m4;
    sVec3_FP m8;
    sVec3_FP m14;
    s32 m20;
    sVec3_FP* m24;
    sVec3_FP m28;
    fixedPoint m34;
    s32 m38;
    s_scriptData1* m3C;
    s32 m40;
    sVec3_FP m44;
    sVec3_FP m50;
    fixedPoint m5C;
    s32 m60;
    //size = 0x64

    s32 UpdateSub0();
    void UpdateSub1();
};

struct s_fieldScriptWorkArea78
{
    sVec3_FP m3C;// unk
    sVec3_FP m48;// unk
};

struct s_fieldScriptWorkArea : public s_workAreaTemplate<s_fieldScriptWorkArea>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &s_fieldScriptWorkArea::Init, &s_fieldScriptWorkArea::Update, NULL, NULL};
        return &taskDefinition;
    }
    static void Init(s_fieldScriptWorkArea*);
    static void Update(s_fieldScriptWorkArea*);

    void fieldScriptTaskUpdateSub2();
    void fieldScriptTaskUpdateSub3();
    sSaturnPtr runFieldScript();
    sSaturnPtr callNative(sSaturnPtr);

    sSaturnPtr* m0_pScripts; //0
    sSaturnPtr m4_currentScript;
    sSaturnPtr* m8_stackPointer;
    sSaturnPtr mC_stack[8];
    s32 m2C_bitToSet; // dunno what that is yet

    struct s_cinematicBarTask* m30_cinematicBarTask;
    u32 m34;
    s_vdp2StringTask* m38_dialogStringTask;
    struct s_multiChoiceTask* m3C_multichoiceTask;
    struct s_receivedItemTask* m40_receivedItemTask;
    s_multiChoice* m44_multiChoiceData;
    s_workArea* m48_cutsceneTask; // s_cutsceneTask or s_cutsceneTask2 or s_cutsceneTask3
    s32 m4C_PCMPlaying;
    s32 m50_scriptDelay;
    s32 m54_currentResult;
    s32 m58;
    s32 m5C;
    s32 m60_canSkipScript;
    s32 m64;

    u32 m6C;
    u32 m70;
    s_fieldScriptWorkArea78* m78;
    s32 m7C;
    p_workArea m80;
    sVec3_FP* m84;

    std::vector<s_scriptData1>* m88;
    std::vector<s_scriptData2>* m8C;
    std::vector<s_scriptData3>* m90;
    std::vector<s_animDataFrame>* m94;
};

struct s_memoryAreaOutput
{
    struct s_fileBundle* m0_mainMemoryBundle;
    u32 m4_characterArea;
};

struct s_cameraScript
{
    sVec3_FP m0_position;
    sVec3_FP mC_rotation;
    fixedPoint m18;
    fixedPoint m1C;
    s32 m20_length;
    sVec3_FP m24_pos2;
    s32 m30_thresholdDistance;
};

struct s_cutsceneData
{
    std::vector<s_scriptData3> m0;
    std::vector<s_scriptData1> m4;
    std::vector<s_scriptData2> m4bis;
    u8 m8;
};

#include "field/fieldDragon.h"

struct s_grid1
{
    sSaturnPtr EA;
    std::array<u16, 5> m0_modelsOffets;
    sVec3_FP m4;
    sVec3_S16_12_4 m10;
    s16 m16;
    // size 18
};

struct s_grid2
{
    sSaturnPtr m0;
    sVec3_FP m4;
    // size 10
};

struct s_grid3
{
    // size 10
};

struct s_DataTable3
{
    std::vector<std::vector<s_grid1>> m0_environmentGrid;
    std::vector<std::vector<s_grid2>> m4;
    s_grid3** m8;
    u32 mC;
    s32 m10_gridSize[2]; // 10
    s32 m18_cellDimensions[2];
    u32 m20;
    //u32 m24[];
};

struct s_DataTable2Sub0
{
    sSaturnPtr m0_function;
    sVec3_FP m4_position;
    sVec3_S16_12_4 m10_rotation;
    s32 m18;
    sSaturnPtr m1C_modelData;
    // size is 0x20
};

struct s_DataTable2
{
    std::vector<std::vector<s_DataTable2Sub0>> m0;
    s32 m4;
    struct {
        s32 m0;
        s32 m4;
    }m8;
};

struct s_visdibilityCellTask : public s_workAreaTemplate<s_visdibilityCellTask>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { NULL, &s_visdibilityCellTask::fieldGridTask_Update, &s_visdibilityCellTask::gridCellDraw_untextured, NULL};
        return &taskDefinition;
    }

    static void fieldGridTask_Update(s_visdibilityCellTask*);
    static void gridCellDraw_untextured(s_visdibilityCellTask*);
    static void gridCellDraw_collision(s_visdibilityCellTask*);
    static void gridCellDraw_normal(s_visdibilityCellTask*);

    s_memoryAreaOutput m0_memoryLayout; // 0
    std::vector<s_grid1>* m8_pEnvironmentCell; // 8
    std::vector<s_grid2>* mC_pCell2_billboards; // 0xC
    s_grid3* m10_pCell3; // 0x10
    u32 m14_index; // 14
}; // size is 0x18

struct sCameraVisibility
{
    s8 m0;
    u8 m1;
};

struct s_visibilityGridWorkArea_68
{
    struct sProcessed3dModel* m0_model;
    sMatrix4x3 m4_matrix;
    fixedPoint m34;
};

struct s_visibilityGridWorkArea_1294
{
    s32 m0_processedQuadsForCollision;
    s32 m4_processedQuadsForCollision2;
    s32 m8_processedQuadsForCollision3;
    s32 mC;
    s32 m10;
    s32 m14;
};

struct s_visibilityGridWorkArea_688
{
    // size 0xC
};

struct s_visibilityGridWorkArea_5A8
{
    sVec3_FP m0_position;
    sVec3_FP mC_normal;
    fixedPoint m18_penetrationDistance;
    // size 0x1C
};

#include "LCS.h"

struct s_randomBattleWorkArea : public s_workAreaTemplate<s_randomBattleWorkArea>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { randomBattleTaskInit, NULL, randomBattleTaskDraw, NULL};
        return &taskDefinition;
    }

    static void randomBattleTaskInit(s_randomBattleWorkArea*);
    static void randomBattleTaskDraw(s_randomBattleWorkArea*);

    void(*m0)();
    u8 m4;
    u8 m5;
    // size 8
};

#include "shared/vdp2PlaneTask.h"

struct s_FieldSubTaskWorkArea : public s_workAreaTemplate<s_FieldSubTaskWorkArea>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &s_FieldSubTaskWorkArea::Init, &s_FieldSubTaskWorkArea::Update, &s_FieldSubTaskWorkArea::Draw, &s_FieldSubTaskWorkArea::Delete};
        return &taskDefinition;
    }

    static void Init(s_FieldSubTaskWorkArea*);
    static void Update(s_FieldSubTaskWorkArea*);
    static void Draw(s_FieldSubTaskWorkArea*);
    static void Delete(s_FieldSubTaskWorkArea*);

    std::array<struct s_fileBundle*, 3> m0_bundles;
    std::array<u32, 3> mC_characterArea; // C
    u8* m18_memoryArea_edge; // 18
    u32 m1C_characterArea_edge; // 1C
    u8* m20_memoryArea_bottom; // 20
    u8* m24_memoryArea_top; // 24
    u32 m28_characterArea_bottom; // 28
    u32 m2C_characterArea_top; // 2C
    const s_MCB_CGB* m30_fileList; // 30
    u32 m34_MCBFilesSizes[32]; // 34
    u32 m1B4_CGBFilesSizes[32]; // 1B4
    sFieldCameraManager* m334;
    s_dragonTaskWorkArea* m338_pDragonTask; // 338
    struct s_FieldRadar* m33C_fieldRadar;
    s_LCSTask* m340_pLCS;
    s_randomBattleWorkArea* m344_randomBattleTask;
    struct s_visibilityGridWorkArea* m348_pFieldCameraTask1; // 348
    s_fieldScriptWorkArea* m34C_ptrToE; // 34C
    sVdp2PlaneTask* m350_fieldPaletteTask;
    u16 m354; // 354
    u16 fieldSubTaskStatus; // 358
    void (*pUpdateFunction2)(); // 35C
    void(*pUpdateFunction3)(); // 35C
    u8 m369; // 369
    u8 m36C;
    u32 m370_fieldDebuggerWho; // 370
    void(*m374_pUpdateFunction1)(); // 374
    u8 m37C_debugMenuStatus1[2]; // 37C
    u8 m37E_debugMenuStatus2_a; // 37E
    u8 m380_debugMenuStatus3; //380
};

struct s_fieldSpecificData_A3 : public s_workAreaTemplate<s_fieldSpecificData_A3>
{
    fixedPoint m0;
    s32 m8;
    std::array<p_workArea, 20> mC; // size probably incorrect
    std::array<u32, 56> m50; //not sure of size
    std::array<s8, 8> m94_A3_0_exits;
    std::array<s8, 3> m9C_A3_1_exits;
    std::array<s8, 4> m9F_A3_2_exits;
    s8 mA3_conanaNestExit;
    std::array<fixedPoint, 7> mA4;
    std::array<sVec3_FP, 18> mC0; // not sure of size
    u8 m130_conanaNestCutsceneTrigger;

    // A3_2 positional sound data
    s32 m134; // sound 0x6A state
    s32 m138; // sound 0x6B state
    sVec3_FP m13C_sound6APos; // position for sound 0x6A
    sVec3_FP m148_sound6BPos; // position for sound 0x6B

    // A3_0 mine carts
    s32 m154;
    sVec3_FP m158;

    struct fieldA3_2_crashedImpertialShip* m164_A3_2_crashedImperialShipTask;
    struct sParticlePoolManager* m168_particlePool;
    // size 16C?
};

struct s_fieldTaskWorkArea : public s_workAreaTemplateWithArg<s_fieldTaskWorkArea, s32>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &s_fieldTaskWorkArea::fieldTaskInit, &s_fieldTaskWorkArea::fieldTaskUpdate, NULL, &s_fieldTaskWorkArea::fieldTaskDelete};
        return &taskDefinition;
    }
    
    static void fieldTaskInit(s_fieldTaskWorkArea*, s32);
    static void fieldTaskUpdate(s_fieldTaskWorkArea*);
    static void fieldTaskDelete(s_fieldTaskWorkArea*);

    s_workArea* m0; // 0
    s_workArea* m4_overlayTaskData;//4
    s_FieldSubTaskWorkArea* m8_pSubFieldData; // 0x8
    p_workArea mC; // overlay-specific field data task (cast to overlay's type)
    u32 m28_status; // 0x28 (0x40: a multichoice dialog callback is pending)
    s16 m2C_currentFieldIndex; // 0x2C
    s16 m2E_currentSubFieldIndex; // 0x2E;
    s16 m30_fieldEntryPoint; // 0x30
    s16 m32_previousSubField; // 0x32
    s8 m34_radarDangerLevel;
    s8 m35;
    s16 m36_fieldIndexMenuSelection; // 0x36
    s16 m38_subFieldIndexMenuSelection; // 0x38
    s16 m3A; // 0x3A
    u8 m3C_fieldTaskState; // 0x3C
    s8 m3D; // 0x3D
    u8 updateDragonAndRiderOnInit; // 0x3E
    std::vector<s_scriptData1> m40; // 0x40
    std::vector<s_scriptData2> m44; // 0x44
    std::vector<s_scriptData3> m48; // 0x48
    std::vector<s_animDataFrame> m4C; // 0x4C
                                   // size: 0x50
};

extern s_fieldTaskWorkArea* fieldTaskPtr;

struct s_fieldStartOverlayTask : public s_workAreaTemplate<s_fieldStartOverlayTask>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &s_fieldStartOverlayTask::Init, nullptr, nullptr, &s_fieldStartOverlayTask::Delete};
        return &taskDefinition;
    }

    static void Init(s_fieldStartOverlayTask*);
    static void Delete(s_fieldStartOverlayTask*);
};

struct s_fieldSub1Task : public s_workAreaTemplate<s_fieldSub1Task>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &s_fieldSub1Task::Init, nullptr, nullptr, &s_fieldSub1Task::Delete};
        return &taskDefinition;
    }

    static void Init(s_fieldSub1Task* pThis)
    {
        fieldTaskPtr->m0 = pThis;
        createSubTask<s_fieldStartOverlayTask>(pThis);
    }

    static void Delete(s_fieldSub1Task* pThis)
    {
        fieldTaskPtr->m0 = NULL;
    }
};

struct s_fieldSub0Task : public s_workAreaTemplate<s_fieldSub0Task>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &s_fieldSub0Task::Init, nullptr, nullptr, &s_fieldSub0Task::Delete};
        return &taskDefinition;
    }

    static void Init(s_fieldSub0Task* pThis)
    {
        fieldTaskVar0 = pThis;
        createSubTask<s_fieldSub1Task>(pThis);
    }

    static void Delete(s_fieldSub0Task* pThis)
    {
        fieldTaskVar0 = nullptr;
        
        //FIX:
        if(gModuleManager && gModuleManager->m8)
        {
            assert(gModuleManager->m8 == pThis);
            gModuleManager->m8 = nullptr;
        }
    }
};

void loadCommonFieldResources();
void setupFileList(const s_MCB_CGB* fileList);

s32 getFieldMemoryAreaRemain();
s32 getFieldCharacterAreaRemain();

void unloadFileFromFileList(u32 index);
void loadFileFromFileList(u32 index);

void getMemoryArea(s_memoryAreaOutput* pOutput, u32 areaIndex);

static s_fieldTaskWorkArea* getFieldTaskPtr()
{
    return fieldTaskPtr;
}

void setupFieldCameraConfigs(s_fieldCameraConfig* r4, u32 r5);
void buildDragonRotationMatrix(s_dragonTaskWorkArea_48* r14, sVec3_FP* r13);
void updateDragonCollision(s_dragonTaskWorkArea* r4);
void updateCameraScriptSub0Sub2(s_dragonTaskWorkArea* r4);
void updateCameraScriptSub0(p_workArea r4);
void dragonFieldTaskInitSub4Sub4();

void startExitFieldCutscene2(p_workArea parent, s_cutsceneData* pScript, s32 param, s32 exitIndex, s32 arg0);
void initFieldDragonLight();

void gridCellDraw_normalSub2(s_fileBundle* r4, s32 r5, s32 r6);
void callGridCellDraw_normalSub2(s_fileBundle* r4, s32 r5);
void exitCutsceneTaskUpdateSub0(s32 param, s32 exitNumber, s16 r6);
void exitCutsceneTaskUpdateSub0Sub0();
void exitCutsceneTaskUpdateSub0Sub1(s32 fieldIndex, s32 param, s32 exitNumber, s32 r7);

int findMandatoryFileOnDisc(const char* fileName);
u32 getFileSizeFromFileId(const char* fileName);
