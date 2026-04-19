#pragma once

struct sBTL_A5_FormationTask : public s_workAreaTemplateWithCopy<sBTL_A5_FormationTask>
{
    s16 m24_counter1;
    s16 m26_counter2;
    u8 m28_state;
    u8 m29_pad[3];
    sVec3_FP m2C_basePosition;
    u8 m38_pad2[0xA4 - 0x38];
    s32 mA4_soundEffectId;
    sVec3_FP mA8_position;
    u8 mB4_pad3[0xC0 - 0xB4];
    sVec3_FP mC0_position2;
    u8 mCC_pad4[0xE0 - 0xCC];
    p_workArea* mE0_entityArray;
    s32 mE4_positionPtr;
    sSaturnPtr mE8_dataTable;
    s16 mEC_formationNameIndex;
    s8 mEE_numEntities;
    s8 mEF_deadCount;
    u8 mF0_flag;
    u8 mF1_pad5[3];
    u8 mF4_entityPositions[0x1CC - 0xF4];
    sSaturnPtr m1CC_dataTable2;
    s32 m1D4_flag;
    // size 0x1D8
};

void BTL_A5_createFormation(s_workAreaCopy* pParent, u32 arg0);
