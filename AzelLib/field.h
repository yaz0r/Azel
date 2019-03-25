#pragma once

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

struct s_fieldOverlaySubTaskWorkArea2E4
{
    sVec3_FP m0;
    sVec3_FP* mC_pPosition;
    sVec3_FP* m10_pPosition2;
    s32 m14;
    s32 m18_maxDistanceSquare;
    // size 0x20?
};

struct s_fieldOverlaySubTaskWorkArea : public s_workAreaTemplate<s_fieldOverlaySubTaskWorkArea>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &fieldOverlaySubTaskInit, NULL, NULL, NULL};
        return &taskDefinition;
    }

    static void fieldOverlaySubTaskInit(s_fieldOverlaySubTaskWorkArea*);

    s32 m0_nextCamera;
    s32 m4_currentCamera;
    s32 m8_numFramesOnCurrentCamera;
    s32 mC;
    std::array<s_fieldCameraConfig,8> m10; // unknown size
    s32 m2D0;
    s32 m2D4;
    s32 m2D8;
    s32 m2DC;
    s32 m2E0;
    std::array<s_fieldOverlaySubTaskWorkArea2E4,5> m2E4;
    sMatrix4x3 m384;
    sMatrix4x3 m3B4;
    std::array<sFieldCameraStatus,2> m3E4;
    u8 m50C;
    u8 m50D; // 50D
    u8 m50E; // 50E
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

struct s_vdp2StringTask : public s_workAreaTemplate<s_vdp2StringTask>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { NULL, &s_vdp2StringTask::Update, NULL, &s_vdp2StringTask::Delete};
        return &taskDefinition;
    }

    static void Update(s_vdp2StringTask*);
    static void Delete(s_vdp2StringTask*);

    void UpdateSub1();

    // variables
    u8 m0_status;
    s8 m2_durationMode;
    s16 mA_duration;
    s_vdp2StringTask** m10;
    s16 m14_x;
    s16 m16_y;
    s16 m1A_width;
    s16 m1C_height;
    sSaturnPtr m24_string;
    // size 2C
};

struct s_cinematicBarTask : public s_workAreaTemplate<s_cinematicBarTask>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &s_cinematicBarTask::Init, &s_cinematicBarTask::Update, &s_cinematicBarTask::Draw, NULL};
        return &taskDefinition;
    }

    static void Init(s_cinematicBarTask* pThis)
    {
        pThis->m8 = 0;
        pThis->mB = 0;
        pThis->m9 = 0;
        pThis->mC = 0;
        pThis->mA = 0;
        pThis->mD = 0;
    }
    static void Update(s_cinematicBarTask* pThis);
    static void Draw(s_cinematicBarTask* pThis);

    void interpolateCinematicBar();
    void interpolateCinematicBarSub1();
    void cinematicBarTaskSub0(s32 r5);

    u8 m0_status;
    s8 m1;
    s8 m2;
    s8 m3;
    s8 m4;
    s8 m8;
    s8 m9;
    s8 mA;
    s8 mB;
    s8 mC;
    s8 mD;
    s8 m11;
    //size 0x13
};

struct s_multiChoiceTask2 : public s_workAreaTemplate< s_multiChoiceTask2>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { NULL, &s_multiChoiceTask2::Update, &s_multiChoiceTask2::Draw, &s_multiChoiceTask2::Delete};
        return &taskDefinition;
    }

    static void Update(s_multiChoiceTask2*);
    static void Draw(s_multiChoiceTask2*);
    static void Delete(s_multiChoiceTask2*)
    {
        assert(0);
    }

    void drawMultiChoice();

    u8 m0_Status;
    s8 m1;
    s8 m2_defaultResult;
    s8 m3;
    s8 m4;
    s8 m5_selectedEntry;
    s8 m6_numEntries;
    s8 m7;
    s8 m8;
    s32* mC_result;
    s_multiChoiceTask2** m10;
    s16 m14_x;
    s16 m16_y;
    s16 m1A_width;
    s16 m1C_height;
    sSaturnPtr m24_strings;
    s16* m28_colors;
    //size 0x2C
};

struct s_multiChoice
{
    s16* m0_choiceTable;
    s32 m4_currentChoice;
    s32 m8_numChoices;
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
    s32 mC;
    s_animDataFrame* m10_animSequence; // no sure
    s_loadRiderWorkArea* m14_riderState;
    s_loadRiderWorkArea* m18;
    const s32* m1C;
};

struct s_cutsceneTask : public s_workAreaTemplateWithArg<s_cutsceneTask, struct s_cutsceneData*>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &s_cutsceneTask::Init, &s_cutsceneTask::Update, NULL, NULL};
        return &taskDefinition;
    }
    static void Init(s_cutsceneTask*, struct s_cutsceneData* argument);
    static void Update(s_cutsceneTask*);

    void cutsceneTaskInitSub2(std::vector<s_scriptData1>& r5, s32 r6, sVec3_FP* r7, u32 arg0);
    void cutsceneTaskInitSub3(std::vector<s_scriptData2>& r5, s32 r6, sVec3_FP* r7, u32 arg0);

    u32 m0;
    u32 m4;
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
    s_cinematicBarTask* startCinmaticBarTask();

    sSaturnPtr* m0_pScripts; //0
    sSaturnPtr m4_currentScript;
    sSaturnPtr* m8_stackPointer;
    sSaturnPtr mC_stack[8];
    s32 m2C_bitToSet; // dunno what that is yet

    s_cinematicBarTask* m30_cinematicBarTask;
    u32 m34;
    s_vdp2StringTask* m38_dialogStringTask;
    s_multiChoiceTask2* m3C_multichoiceTask;
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
    s_cutsceneTask* m80;
    sVec3_FP* m84;

    std::vector<s_scriptData1>* m88;
    std::vector<s_scriptData2>* m8C;
    std::vector<s_scriptData3>* m90;
    std::vector<s_animDataFrame>* m94;
};

struct s_memoryAreaOutput
{
    u8* m0_mainMemory;
    u8* m4_characterArea;
};

struct s_dragonTaskWorkArea_1F0
{
    s32 m_0;
    s32 m_4;
    u32 m_8;
    s32 m_C;
    s16 m_E;
    s16 m_10;
};

struct s_dragonTaskWorkArea_48
{
    sMatrix4x3 m0_matrix; //0

    u32 m30; //30
    u32 m34; //34
    u32 m38; //38
    u8 m3C;
};

struct s_cameraScript
{
    sVec3_FP m0_position;
    sVec3_FP mC_rotation;
    fixedPoint m18;
    fixedPoint m1C;
    s32 m20;
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

struct s_RGB8
{
    s8 m0;
    s8 m1;
    s8 m2;

    u32 toU32()
    {
        return ((m2 & 0xFF) << 16) | ((m1 & 0xFF) << 8) | (m0 & 0xFF);
    }
};

struct s_dragonTaskWorkArea : s_workAreaTemplateWithArg<s_dragonTaskWorkArea, s32>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &s_dragonTaskWorkArea::Init, &s_dragonTaskWorkArea::Update, &s_dragonTaskWorkArea::Draw, &s_dragonTaskWorkArea::Delete};
        return &taskDefinition;
    }

    static void Init(s_dragonTaskWorkArea*, s32 arg);
    static void Update(s_dragonTaskWorkArea*);
    static void Draw(s_dragonTaskWorkArea*);
    static void Delete(s_dragonTaskWorkArea*)
    {
        PDS_unimplemented("");
    }

    s_memoryAreaOutput m0;

    sVec3_FP m8_pos; // 8
    sVec3_FP m14_oldPos; // 14
    sVec3_FP m20_angle; // 20-24-28
    fixedPoint m30;
    sVec3_FP m3C;

    s_dragonTaskWorkArea_48 m48;

    sMatrix4x3 m88_matrix; // 88
    u32 mB8;
    sVec3_FP* mBC;

    fixedPoint mC0_lightRotationAroundDragon;
    fixedPoint mC4;
    s_RGB8 mC8_normalLightColor;
    s_RGB8 mCB_falloffColor0;
    s_RGB8 mCE_falloffColor1;
    s_RGB8 mD1_falloffColor2;
    s_RGB8 mD4;
    s_RGB8 m_E8_specialColor;
    u8 m_EB_useSpecialColor;
    u8 m_EC;

    void(*mF0)(s_dragonTaskWorkArea*); //F0
    void(*mF4)(s_dragonTaskWorkArea*);
    u32 mF8_Flags; // F8 Flags
    u32 mFC; // FC
    u32 m100;
    u32 m104_dragonScriptStatus;
    s32 m108;
    sVec3_FP m10C_hotSpot2;
    sVec3_FP m118_hotSpot3;
    sVec3_FP m124_hotSpot4;
    fixedPoint m130_minX;
    fixedPoint m134_minY;
    fixedPoint m138_minZ;
    fixedPoint m13C_maxX;
    fixedPoint m140_maxY;
    fixedPoint m144_maxZ;
    fixedPoint m148_pitchMin;
    fixedPoint m14C_pitchMax;

    u32 m150;
    fixedPoint m154_dragonSpeed;
    fixedPoint m158;
    s32 m15C_dragonSpeedIncrement;

    sVec3_FP m160_deltaTranslation;
    sVec3_FP m16C_deltaRotation;
    fixedPoint m178[4];

    sVec3_FP m188;

    sVec3_FP m194;

    sVec3_FP m1A0;
    sVec3_FP m1AC;

    u32 m1B8;
    u32 m1BC;

    u32 m_1C4;

    fixedPoint m1CC;
    s_cameraScript* m1D0_cameraScript;
    s_cutsceneData* m1D4_cutsceneData;
    s_cutsceneTask* m1D8_cutscene;
    s_riderAnimTask* m1DC_ridersAnimation[2];
    s_scriptData3* m1E4_cutsceneKeyFrame;
    s16 m1E8_cameraScriptDelay;
    s16 m1EA;
    u32 m1EC;
    s16 m1EE;
    s_dragonTaskWorkArea_1F0 m1F0;
    sVec2_S16 m200_LCSCursorScreenSpaceCoordinates;
    fixedPoint m208;
    fixedPoint m20C;
    u32 m210;
    u32 m214;
    fixedPoint m21C_DragonSpeedValues[5];
    u32 m228;
    u32 m22C;
    fixedPoint m230;
    u8 m234;
    s8 m235_dragonSpeedIndex;

    u8 m237;
    u8 m238;
    u8 m239;

    u8 m23A_dragonAnimation;
    u8 m23B;
    u8 m23C;
    u8 m244;
    u8 m245;
    u8 m246;
    u8 m247;
    u8 m248;
    u8 m249;
    u8 m24A_runningCameraScript; // has camera script
    u32 m250;
    u32 m254;
    u32 m258;
    u8 m25B;
    u8 m25C;
    s8 m25D;
    s8 m25E;
};

struct s_grid1
{
    sSaturnPtr m0;
    sVec3_FP m4;
    s16 m10[3];
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
    s_grid1** m0_environmentGrid;
    s_grid2** m4;
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
    std::array<s16, 3> m10_rotation;
    s32 m18;
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
    s_grid1* m8_pEnvironmentCell; // 8
    s_grid2* mC_pCell2_billboards; // 0xC
    s_grid3* pCell3; // 0x10
    u32 index; // 14
}; // size is 0x18

struct sCameraVisibility
{
    s8 m0;
    u8 m1;
};

struct s_visibilityGridWorkArea_68
{
    u8* m0;
    sMatrix4x3 m4;
    u32 m34;
};

struct s_visibilityGridWorkArea_1294
{
    s32 m4;
    s32 m8;
    s32 mC;
    s32 m10;
    s32 m14;
};

struct s_visibilityGridWorkArea : public s_workAreaTemplate<s_visibilityGridWorkArea>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { fieldCameraTask1Init, fieldCameraTask1Update, fieldCameraTask1Draw, NULL};
        return &taskDefinition;
    }

    static void fieldCameraTask1Init(s_visibilityGridWorkArea*);
    static void fieldCameraTask1Update(s_visibilityGridWorkArea*);
    static void fieldCameraTask1Draw(s_visibilityGridWorkArea*);

    sVec3_FP m0_position; // 0
    sVec3_FP mC;
    s32 m18_cameraGridLocation[2]; // 18 Grid location
    s32 m20_cellDimensions[2];
    u32 m28;
    std::vector<fixedPoint>* m2C_depthRangeTable;
    s_DataTable3* m30; // 30
    std::vector<std::vector<sCameraVisibility>>* m34_cameraVisibilityTable; // m34
    p_workArea m38; // 38
    s_visdibilityCellTask** m3C_cellRenderingTasks; // 3C (an array of tasks)
    std::array<s_visibilityGridWorkArea_68, 24>::iterator m44;
    std::array<s_visibilityGridWorkArea_68, 24> m68; // size highly unsure.
    u32 m128C_vdp2VramOffset2;
    u32 m1290_vdp2VramOffset;
    s_visibilityGridWorkArea_1294 m1294;
    sVec3_FP m12AC;
    sVec3_FP m12B8;
    sVec3_FP m12C4;
    sVec3_FP m12D0;
    s32 m12DC;
    u16 m12E0;
    u16 m12E2;
    u16 m12E4;
    u16 m12F0;
    u16 m12F2_renderMode; // 12F2
    u8 updateVisibleCells;
    u8(*m12F8_convertCameraPositionToGrid)(s_visibilityGridWorkArea* pFieldCameraTask1); // 12F8
    s32(*m12FC_isObjectClipped)(sVec3_FP* r4, s32 r5); // 12F8
    u8 m1300;
    //size: 1304
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
    // size 8
};

struct s_FieldRadar : public s_workAreaTemplate<s_FieldRadar>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { dragonFieldSubTask2Init, dragonFieldSubTask2Update, dragonFieldSubTask2Draw, NULL};
        return &taskDefinition;
    }

    static void dragonFieldSubTask2Init(s_FieldRadar*);
    static void dragonFieldSubTask2Update(s_FieldRadar*);
    static void dragonFieldSubTask2Draw(s_FieldRadar*);

    s_memoryAreaOutput m0;
    s16 m8;
    s16 mA;
    s16 mC;
    s16 mE;
    s16 m10;
    s16 m12;
    s16 m28;
    s16 m2A;
    s32 m30;
    s32 m34;
    u8* m3C;
    s8 m4C;
    s32 m50;
    s32 m54;
    s32 m58;
    s8 m5A;
    s32 m5C;
    s32 m60;
    s32 m64;
    fixedPoint m68;
};

struct s_fieldPaletteTaskWorkSub
{
    s32 m0;
    s32 m4;
    s32 m8;
    s32 mC;
    s32 m10;
    s32 m14;
};

struct s_fieldPaletteTaskWorkArea : public s_workAreaTemplate<s_fieldPaletteTaskWorkArea>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &s_fieldPaletteTaskWorkArea::Init, nullptr, &s_fieldPaletteTaskWorkArea::Draw, nullptr};
        return &taskDefinition;
    }

    static void Init(s_fieldPaletteTaskWorkArea*);
    static void Draw(s_fieldPaletteTaskWorkArea*);

    s_fieldPaletteTaskWorkSub* m78;
};

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

    u8* memoryArea[3]; // 0
    u8* characterArea[3]; // C
    u8* memoryArea_edge; // 18
    u8* characterArea_edge; // 1C
    u8* memoryArea_bottom; // 20
    u8* memoryArea_top; // 24
    u8* characterArea_bottom; // 28
    u8* characterArea_top; // 2C
    const s_MCB_CGB* fileList; // 30
    u32 MCBFilesSizes[32]; // 34
    u32 CGBFilesSizes[32]; // 1B4
    s_fieldOverlaySubTaskWorkArea* m334;
    s_dragonTaskWorkArea* m338_pDragonTask; // 338
    s_FieldRadar* m33C_pPaletteTask;
    s_LCSTask* m340_pLCS;
    s_randomBattleWorkArea* m344_randomBattleTask;
    s_visibilityGridWorkArea* m348_pFieldCameraTask1; // 348
    s_fieldScriptWorkArea* m34C_ptrToE; // 34C
    s_fieldPaletteTaskWorkArea* m350_fieldPaletteTask;
    u16 m354; // 354
    u16 fieldSubTaskStatus; // 358
    void (*pUpdateFunction2)(); // 35C
    void(*pUpdateFunction3)(); // 35C
    u8 m369; // 369
    u8 m36C;
    u32 m370_fieldDebuggerWho; // 370
    void(*pUpdateFunction1)(); // 374
    u8 debugMenuStatus1[2]; // 37C
    u8 debugMenuStatus2_a; // 37E
    u8 debugMenuStatus3; //380
};

struct s_fieldTaskWorkArea_C : public s_workAreaTemplate<s_fieldTaskWorkArea_C>
{
    fixedPoint m0;
    s32 m8;
    std::array<p_workArea, 20> mC; // size probably incorrect
    std::array<u32, 56> m50; //not sure of size
    std::array<s8, 3> m9C;
    s8 mA3_conanaNestExit;
    std::array<fixedPoint, 7> mA4;
    std::array<sVec3_FP, 18> mC0; // not sure of size
    u8 m130_conanaNestCutsceneTrigger;

    // A3_0 mine carts
    s32 m154;
    sVec3_FP m158;

    p_workArea m168;
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
    s_fieldTaskWorkArea_C* mC; // background task buffer?
    u32 m28_status; // 0x28
    s16 m2C_currentFieldIndex; // 0x2C
    s16 m2E_currentSubFieldIndex; // 0x2E;
    s16 m30; // 0x30
    s16 m32; // 0x32
    u8 m35; // 0x35
    s16 fieldIndexMenuSelection; // 0x36
    s16 subFieldIndexMenuSelection; // 0x38
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

    static void Delete(s_fieldSub0Task*)
    {
        fieldTaskVar0 = nullptr;
    }
};

void loadCommonFieldResources();
void setupFileList(const s_MCB_CGB* fileList);

s32 getFieldMemoryAreaRemain();
s32 getFieldCharacterAreaRemain();

void loadFileFromFileList(u32 index);

void getMemoryArea(s_memoryAreaOutput* pOutput, u32 areaIndex);

static s_fieldTaskWorkArea* getFieldTaskPtr()
{
    return fieldTaskPtr;
}

void setupFieldCameraConfigs(s_fieldCameraConfig* r4, u32 r5);
void dragonFieldTaskInitSub4Sub5(s_dragonTaskWorkArea_48* r14, sVec3_FP* r13);
void dragonFieldTaskInitSub4Sub6(s_dragonTaskWorkArea* r4);
void updateCameraScriptSub0Sub2(s_dragonTaskWorkArea* r4);
void updateCameraScriptSub0(u32 r4);
void dragonFieldTaskInitSub4Sub4();

void startExitFieldCutscene(p_workArea parent, s_cameraScript* pScript, s32 param, s32 exitIndex, s32 arg0);
void initFieldDragonLight();

void gridCellDraw_normalSub2(u8* r4, s32 r5, s32 r6);
void callGridCellDraw_normalSub2(u8* r4, s32 r5);
