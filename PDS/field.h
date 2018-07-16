#pragma once

struct s_fieldCameraConfig
{
    sVec3_FP m0_min;
    sVec3_FP mC_max;
    s32 m18[7];
    s32 m34[7];
    s32 m50;
    s32 m54;
    // size: 0x58
};

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
    sMatrix4x3 field_384;
    sMatrix4x3 field_3B4;
    sFieldCameraStatus field_3E4[2];
    u8 m50C;
    u8 m50D; // 50D
    u8 field_50E; // 50E
};

struct s_scriptData1
{
    u32 field_0;
    u32 field_4;
    u32 field_8;
    u32 field_C;
    u32 field_10;
    u32 field_14;
    u32 field_18;
    u32 field_1C;
    u32 field_20;
    u32 field_24;
    u32 field_28;
    u32 field_2C;
    u32 field_30;
    u32 field_34;
    u32 field_38;
};

struct s_scriptData2
{
    u32 field_0;
    u32 field_4;
    u32 field_8;
    u32 field_C;
    u32 field_10;
    u32 field_14;
    u32 field_18;
    u32 field_1C;
    u32 field_20;
};

struct s_scriptData3
{
    u32 field_0;
    u32 field_4;
    u32 field_8;
    u32 field_C;
    u32 field_10;
    u32 field_14;
    u32 field_18;
    u32 field_1C;
};

struct s_scriptData4
{
    u8 field_0;
    u8 field_1;
    u16 field_2;
};

struct s_fieldScriptWorkArea : public s_workArea
{
    void* m0_pScripts; //0
    u32 m4;
    void* field_8;
    void* field_2C; // dunno what that is yet

    u32 m30;
    u32 m34;
    u32 m38;
    u32 m3C;

    u32 field_6C;
    u32 field_70;

    s_scriptData1* field_88;
    s_scriptData2* field_8C;
    s_scriptData3* field_90;
    s_scriptData4* field_94;
};

struct s_memoryAreaOutput
{
    u8* mainMemory;
    u8* characterArea;
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

    u32 field_30; //30
    u32 field_34; //34
    u32 field_38; //38
    u8 field_3C;
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

    fixedPoint field_C0;
    fixedPoint field_C4;
    u8 field_C8[3];
    u8 field_CB[3];
    u8 field_CE[3];
    u8 field_D1[3];
    u8 field_D4[3];

    u8 m_EB;
    u8 m_EC;

    void(*field_F0)(s_dragonTaskWorkArea*); //F0
    void(*mF4)();
    u32 mF8_Flags; // F8 Flags
    u32 field_FC; // FC
    u32 m100;
    u32 m104_dragonScriptStatus;
    fixedPoint m130_minX;
    fixedPoint m134_minY;
    fixedPoint m138_minZ;
    fixedPoint m13C_maxX;
    fixedPoint m140_maxY;
    fixedPoint m144_maxZ;
    fixedPoint m148_pitchMin;
    fixedPoint m14C_pitchMax;

    u32 field_150;
    fixedPoint m154;

    s32 m15C;

    sVec3_FP m160_deltaTranslation;

    fixedPoint field_178[4];

    sVec3_FP m1A0;
    sVec3_FP m1AC;

    u32 field_1B8;
    u32 field_1BC;

    u32 m_1C4;

    u32 field_1CC;
    s_cameraScript* m1D0_cameraScript;
    u32 field_1D4;
    s32 m1E8_cameraScriptDelay;
    s_dragonTaskWorkArea_1F0 field_1F0;

    u32 field_208;
    u32 field_20C;
    u32 field_210;
    u32 field_214;
    s32 field_21C[5];
    u32 field_228;
    u32 field_22C;
    u32 field_230;
    u8 field_234;
    u8 field_235;

    u8 field_237;
    u8 field_238;

    u8 field_23A;
    u8 field_23B;
    u8 field_23C;
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
    u32 m18;
    u32 m1C;
    u32 m20;
    //u32 field_24[];
};

struct s_visdibilityCellTask : public s_workArea
{
    s_memoryAreaOutput m0_memoryLayout; // 0
    s_grid1* m8_pEnvironmentCell; // 8
    s_grid2* pCell2; // 0xC
    s_grid3* pCell3; // 0x10
    u32 index; // 14
}; // size is 0x18

struct sCameraVisibility
{
    s8 field_0;
    u8 field_1;
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
    u32 field_20;
    s32 field_24;
    u32 field_28;
    fixedPoint* m2C_depthRangeTable;
    s_DataTable3* field_30; // 30
    sCameraVisibility** m34_cameraVisibilityTable; // field_34
    p_workArea field_38; // 38
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
    s32(*field_12FC)(sVec3_FP* r4, s32 r5); // 12F8
    u8 field_1300;
    //size: 1304
};

struct s_LCSTask : public s_workArea
{
    s_memoryAreaOutput m0;
    u32 m8;
    u32 mC;
    u32 m10;
    u32 m814;
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
    s_LCSTask* m340_pLCS;
    s_randomBattleWorkArea* m344_randomBattleTask;
    s_visibilityGridWorkArea* m348_pFieldCameraTask1; // 348
    s_fieldScriptWorkArea* m34C_ptrToE; // 34C
    u16 field_354; // 354
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

struct s_fieldTaskWorkArea : public s_workArea
{
    s_workArea* m0; // 0
    s_workArea* m4_overlayTaskData;//4
    s_FieldSubTaskWorkArea* m8_pSubFieldData; // 0x8
    u32 m28_status; // 0x28
    s16 m2C_currentFieldIndex; // 0x2C
    s16 m2E_currentSubFieldIndex; // 0x2E;
    s16 field_30; // 0x30
    s16 m32; // 0x32
    u8 field_35; // 0x35
    s16 fieldIndexMenuSelection; // 0x36
    s16 subFieldIndexMenuSelection; // 0x38
    s16 field_3A; // 0x3A
    u8 fieldTaskState; // 0x3C
    s8 field_3D; // 0x3D
    u8 updateDragonAndRiderOnInit; // 0x3E
    s_scriptData1* field_40; // 0x40
    s_scriptData2* field_44; // 0x44
    s_scriptData3* field_48; // 0x48
    s_scriptData4* field_4C; // 0x4C
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
