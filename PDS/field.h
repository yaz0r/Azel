#pragma once

struct s_FieldSubTaskWorkArea : public s_workArea
{
    u8* memoryArea_edge; // 18
    u8* characterArea_edge; // 1C
    u8* memoryArea_bottom; // 20
    u8* memoryArea_top; // 24
    u8* characterArea_bottom; // 28
    u8* characterArea_top; // 2C
    const char** fileList; // 30
    u16 field_354; // 354
    u16 fieldSubTaskStatus; // 358
    void* pUpdateFunction2; // 35C
    u16 fieldDebuggerWho; // 370

    void* pUpdateFunction1; // 374
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
                                   // size: 0x50
};

extern s_fieldTaskWorkArea* fieldTaskPtr;

void loadCommonFieldResources();
void setupFileList(const char** fileList);

