#pragma once

struct sFieldCameraStatus
{
    s32 angle_y;
};

struct s_fieldOverlaySubTaskWorkArea : public s_workArea
{
    sFieldCameraStatus field_3E4[1]; // size isn't known yet
    u8 field_50C;
    u8 field_50D; // 50D
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
    void* pScripts; //0

    void* field_8;
    void* field_2C; // dunno what that is yet

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
    s32 matrix[4 * 3]; //0

    u32 field_30; //30
    u32 field_34; //34
    u32 field_38; //38
    u8 field_3C;
};

struct s_dragonTaskWorkArea : s_workArea
{
    s_memoryAreaOutput field_0;

    s32 posX; // 8
    s32 posY; // C
    s32 posZ; // 10

    s32 angleX; // 20
    s32 angleY; // 24
    s32 angleZ; // 28

    s_dragonTaskWorkArea_48 field_48;

    s32 matrix[4 * 3]; // 88

    u32 field_C0;
    u32 field_C4;
    u8 field_C8[3];
    u8 field_CB[3];
    u8 field_CE[3];
    u8 field_D1[3];
    u8 field_D4[3];

    void(*field_F0); //F0

    u32 field_130;
    u32 field_134;
    u32 field_138;
    u32 field_13C;
    u32 field_140;
    u32 field_144;
    u32 field_148;
    u32 field_14C;

    u32 field_150;
    u32 field_154;

    u32 field_178[4];

    u32 field_1B8;
    u32 field_1BC;

    u32 field_1CC;

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
};

struct s_DataTable3
{
    u8* field_0;
    u8* field_4;
    u8* field_8;
    u32 field_C;
    u32 field_10;
    u32 field_14;
    u32 field_18;
    u32 field_1C;
    u32 field_20;
    //u32 field_24[];
};

struct s_fieldCameraTask1WorkArea : public s_workArea
{
    s32 field_18;
    s32 field_1C;
    u32 field_20;
    u32 field_24;
    u32 field_28;
    u8* field_2C;
    s_DataTable3* field_30; // 30
    u8* field_34; // field_34
    p_workArea field_38; // 38

    u16 field_12F2; // 12F2
    void(*field_12F8); // 12F8
    void(*field_12FC); // 12F8
    u8 field_1300;
    //size: 1304
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
    s_fieldOverlaySubTaskWorkArea* field_334;
    s_dragonTaskWorkArea* pDragonTask; // 338
    s_fieldCameraTask1WorkArea* pFieldCameraTask1; // 348
    s_fieldScriptWorkArea* ptrToE; // 34C
    u16 field_354; // 354
    u16 fieldSubTaskStatus; // 358
    void (*pUpdateFunction2)(); // 35C
    void(*pUpdateFunction3)(); // 35C
    u8 field_369; // 369
    u16 fieldDebuggerWho; // 370
    void(*pUpdateFunction1)(); // 374
    u8 debugMenuStatus1_a; // 37C
    u8 debugMenuStatus1_b; // 37D
    u8 debugMenuStatus2_a; // 37E
    u8 debugMenuStatus3; //380
};

struct s_fieldTaskWorkArea : public s_workArea
{
    s_workArea* field_0; // 0
    s_workArea* overlayTaskData;//8
    s_FieldSubTaskWorkArea* pSubFieldData; // 0x8
    u32 fStatus; // 0x28
    s16 currentFieldIndex; // 0x2C
    s16 currentSubFieldIndex; // 0x2E;
    s16 field_30; // 0x30
    s16 field_32; // 0x32
    u8 field_35; // 0x35
    u16 fieldIndexMenuSelection; // 0x36
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
