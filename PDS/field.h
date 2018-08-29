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
    s32 m14;
    s32 m18_maxDistanceSquare;
    // size 0x20?
};

struct s_fieldOverlaySubTaskWorkArea : public s_workArea
{
    s32 m0_nextCamera;
    s32 m4_currentCamera;
    s32 m8_numFramesOnCurrentCamera;
    s32 mC;
    s_fieldCameraConfig m10[8]; // unknown size
    s32 m2D0;
    s32 m2D4;
    s32 m2D8;
    s32 m2DC;
    s32 m2E0;
    s_fieldOverlaySubTaskWorkArea2E4 m2E4[5];
    sMatrix4x3 m384;
    sMatrix4x3 m3B4;
    sFieldCameraStatus m3E4[2];
    u8 m50C;
    u8 m50D; // 50D
    u8 m50E; // 50E
};

struct s_scriptData1
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
    u32 m24;
    u32 m28;
    u32 m2C;
    u32 m30;
    u32 m34;
    u32 m38;
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
};

struct s_scriptData3
{
    s32 m0;
    s32 m4;
    s32 m8;
    s32 mC;
    s32 m10;
    s32 m14;
    s32 m18;
    s32 m1C;
};

struct s_scriptData4
{
    u8 m0;
    u8 m1;
    u16 m2;
};

struct s_vdp2StringTask : public s_workArea
{
    static s_taskDefinition* getTaskDefinition()
    {
        static s_taskDefinition taskDefinition = { NULL, s_vdp2StringTask::StaticUpdate, NULL, s_vdp2StringTask::StaticDelete, "vdp2StringTask" };
        return &taskDefinition;
    }
    static void StaticUpdate(p_workArea pWorkArea)
    {
        s_vdp2StringTask* pThis = ConvertType(pWorkArea);
        pThis->Update();
    }
    static void StaticDelete(p_workArea pWorkArea)
    {
        s_vdp2StringTask* pThis = ConvertType(pWorkArea);
        pThis->Delete();
    }
    static s_vdp2StringTask* ConvertType(p_workArea pWorkArea)
    {
        return static_cast<s_vdp2StringTask*>(pWorkArea);
    }

    void Update() override;
    void Delete() override;

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

struct s_cinematicBarTask : public s_workArea
{
    static s_taskDefinition* getTaskDefinition()
    {
        static s_taskDefinition taskDefinition = { s_cinematicBarTask::StaticInit, s_cinematicBarTask::StaticUpdate, s_cinematicBarTask::StaticDraw, NULL, "cinematicBarTask" };
        return &taskDefinition;
    }
    static void StaticInit(p_workArea pWorkArea)
    {
        ConvertType(pWorkArea)->Init();
    }
    static void StaticUpdate(p_workArea pWorkArea)
    {
        ConvertType(pWorkArea)->Update();
    }
    static void StaticDraw(p_workArea pWorkArea)
    {
        ConvertType(pWorkArea)->Draw();
    }
    static s_cinematicBarTask* ConvertType(p_workArea pWorkArea)
    {
        return static_cast<s_cinematicBarTask*>(pWorkArea);
    }

    void Init(void* pArgument = NULL) override
    {
        m8 = 0;
        mB = 0;
        m9 = 0;
        mC = 0;
        mA = 0;
        mD = 0;
    }
    void Update() override;
    void Draw() override;

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

struct s_multiChoiceTask2 : public s_workArea
{
    static s_taskDefinition* getTaskDefinition()
    {
        static s_taskDefinition taskDefinition = { NULL, s_multiChoiceTask2::StaticUpdate, s_multiChoiceTask2::StaticDraw, s_multiChoiceTask2::StaticDelete, "s_multiChoiceTask2" };
        return &taskDefinition;
    }
    static void StaticUpdate(p_workArea pWorkArea)
    {
        ConvertType(pWorkArea)->Update();
    }
    static void StaticDraw(p_workArea pWorkArea)
    {
        ConvertType(pWorkArea)->Draw();
    }
    static void StaticDelete(p_workArea pWorkArea)
    {
        ConvertType(pWorkArea)->Delete();
    }
    static s_multiChoiceTask2* ConvertType(p_workArea pWorkArea)
    {
        return static_cast<s_multiChoiceTask2*>(pWorkArea);
    }

    void Update() override;
    void Draw() override;
    void Delete() override
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

struct s_fieldScriptWorkArea : public s_workArea
{
    void Update() override;

    static void StaticUpdate(p_workArea pWorkArea)
    {
        s_fieldScriptWorkArea* pThis = ConvertType(pWorkArea);
        pThis->Update();
    }

    static s_fieldScriptWorkArea* ConvertType(p_workArea pWorkArea)
    {
        return static_cast<s_fieldScriptWorkArea*>(pWorkArea);
    }

    void fieldScriptTaskUpdateSub2();
    void fieldScriptTaskUpdateSub3();
    sSaturnPtr runFieldScript();
    sSaturnPtr callNative(sSaturnPtr);
    s_cinematicBarTask* s_fieldScriptWorkArea::startCinmaticBarTask();

    sSaturnPtr* m0_pScripts; //0
    sSaturnPtr m4_currentScript;
    sSaturnPtr* m8_stackPointer;
    sSaturnPtr mC_stack[8];
    s32 m2C; // dunno what that is yet

    s_cinematicBarTask* m30_cinematicBarTask;
    u32 m34;
    s_vdp2StringTask* m38_dialogStringTask;
    s_multiChoiceTask2* m3C_multichoiceTask;
    s32 m40;
    s_multiChoice* m44_multiChoiceData;
    s32 m4C_PCMPlaying;
    s32 m50_scriptDelay;
    s32 m54_currentResult;
    s32 m58;

    s32 m60;
    s32 m64;

    u32 m6C;
    u32 m70;
    s32 m78;
    s32 m7C;
    s_cutsceneTask* m80;
    sVec3_FP* m84;

    s_scriptData1* m88;
    s_scriptData2* m8C;
    s_scriptData3* m90;
    s_scriptData4* m94;
};

struct s_memoryAreaOutput
{
    u8* m0_mainMemory;
    u8* m4_characterArea;
};

struct s_dragonTaskWorkArea_1F0
{
    u32 m_0;
    u32 m_4;
    u32 m_8;
    u32 m_C;
    u16 m_E;
    u16 m_10;
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

struct s_cutsceneTask : public s_workArea
{
    static s_taskDefinitionWithArg* getTaskDefinition()
    {
        static s_taskDefinitionWithArg taskDefinition = { s_cutsceneTask::StaticInit, s_cutsceneTask::StaticUpdate, NULL, NULL, "s_cutsceneTask" };
        return &taskDefinition;
    }
    static s_cutsceneTask* ConvertType(p_workArea pWorkArea)
    {
        return static_cast<s_cutsceneTask*>(pWorkArea);
    }
    static void StaticInit(p_workArea pWorkArea, void* argument)
    {
        ConvertType(pWorkArea)->Init(argument);
    }
    static void StaticUpdate(p_workArea pWorkArea)
    {
        ConvertType(pWorkArea)->Update();
    }

    void Init(void* argument) override;
    void Update() override;

    void cutsceneTaskInitSub2(void* r5, s32 r6,  sVec3_FP* r7, u32 arg0);

    u32 m0;
};

struct s_cutsceneData
{
    s_scriptData3* m0;
    void* m4;
    u8 m8;
};

struct s_dragonTaskWorkArea : s_workArea
{
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

    fixedPoint mC0;
    fixedPoint mC4;
    u8 mC8[3];
    u8 mCB[3];
    u8 mCE[3];
    u8 mD1[3];
    u8 mD4[3];

    u8 m_EB;
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

    fixedPoint m178[4];

    sVec3_FP m188;

    sVec3_FP m194;

    sVec3_FP m1A0;
    sVec3_FP m1AC;

    u32 m1B8;
    u32 m1BC;

    u32 m_1C4;

    u32 m1CC;
    s_cameraScript* m1D0_cameraScript;
    s_cutsceneData* m1D4_cutsceneData;
    s_cutsceneTask* m1D8_cutscene;
    s_scriptData3* m1E4;
    s32 m1E8_cameraScriptDelay;
    u32 m1EC;
    s_dragonTaskWorkArea_1F0 m1F0;

    u32 m208;
    u32 m20C;
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

struct s_visdibilityCellTask : public s_workArea
{
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

struct s_visibilityGridWorkArea : public s_workArea
{
    sVec3_FP m0_position; // 0
    sVec3_FP mC;
    s32 m18_cameraGridLocation[2]; // 18 Grid location
    s32 m20_cellDimensions[2];
    u32 m28;
    fixedPoint* m2C_depthRangeTable;
    s_DataTable3* m30; // 30
    std::vector<std::vector<sCameraVisibility>>* m34_cameraVisibilityTable; // m34
    p_workArea m38; // 38
    s_visdibilityCellTask** m3C_cellRenderingTasks; // 3C (an array of tasks)
    s_visibilityGridWorkArea_68* m44;
    s_visibilityGridWorkArea_68 m68[24]; // size highly unsure.
    u32 m128C_vdp2VramOffset2;
    u32 m1290_vdp2VramOffset;
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
    s32(*m12FC)(sVec3_FP* r4, s32 r5); // 12F8
    u8 m1300;
    //size: 1304
};

struct s_LCSTask : public s_workArea
{
    s_memoryAreaOutput m0;
    u32 m8;
    u32 mC;
    u32 m10;
    u32 m814;
    s8 m83F;
    void* m9C0;
    u32 m9C4;
    u32 m9C8;
    u32 m9CC;
    // size 0x9DC
};

struct s_randomBattleWorkArea : public s_workArea
{
    u8 m4;
    // size 8
};

struct s_PaletteTaskWorkArea : public s_workArea
{
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

struct s_fieldPaletteTaskWorkArea : public s_workArea
{
    s_fieldPaletteTaskWorkSub* m78;
};

struct s_FieldSubTaskWorkArea : public s_workArea
{
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
    s_PaletteTaskWorkArea* m33C_pPaletteTask;
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
    u16 fieldDebuggerWho; // 370
    void(*pUpdateFunction1)(); // 374
    u8 debugMenuStatus1[2]; // 37C
    u8 debugMenuStatus2_a; // 37E
    u8 debugMenuStatus3; //380
};

struct s_fieldTaskWorkArea_C : public s_workArea
{
    u8 m130;
    p_workArea m168;
    // size 16C?
};

struct s_fieldTaskWorkArea : public s_workArea
{
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
    s_scriptData1* m40; // 0x40
    s_scriptData2* m44; // 0x44
    s_scriptData3* m48; // 0x48
    s_scriptData4* m4C; // 0x4C
                                   // size: 0x50
};

extern s_fieldTaskWorkArea* fieldTaskPtr;

void loadCommonFieldResources();
void setupFileList(const s_MCB_CGB* fileList);

s32 getFieldMemoryAreaRemain();
s32 getFieldCharacterAreaRemain();

void loadFileFromFileList(u32 index);

void getMemoryArea(s_memoryAreaOutput* pOutput, u32 areaIndex);

s_fieldTaskWorkArea* getFieldTaskPtr();

void setupFieldCameraConfigs(s_fieldCameraConfig* r4, u32 r5);
