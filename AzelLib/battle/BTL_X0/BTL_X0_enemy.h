#pragma once

#include "battle/battleEnemyModels.h"
#include "battle/interpolators/vec2FPInterpolator.h"

struct sBTL_X0_EnemyModel : public s_workAreaTemplateWithCopy<sBTL_X0_EnemyModel>
{
    s_fileBundle* m0_fileBundle;
    u8 m4_pad[4];
    p_workArea m8_parentFormation;
    u8 mC_pad[4];
    p_workArea m10_lifeMeterTask;
    u8 m14_pad[8];
    sVec3_FP m1C_lifeMeterPosition;
    sVec3_FP m28_rotation;
    u8 m34_pad[0x48];
    sVec3_FP m7C_position;
    u8 m88_pad[0x10];
    std::vector<s_3dModel> m98_models;
    u8 mAA_modelCount;
    u8 mAB_pad[0x41];
    s16 mEC_hpMax;
    s16 mEE_hpCurrent;
    u8 mF0_pad[0xC];
    s8 mFC_idleState;
    u8 mFD_pad[0xAF];
    sVec2FPInterpolator m1AC_interpolator;
    u8 m1E6_pad2[2];
    sVec3_FP m1B8_targetPosition;
    u8 m1C4_pad[0xC];
    sVec3_FP m1D0_randomOffset;
    u8 m1DC_pad[8];
    u16 m1E4_timer;
    u8 m1E6_pad3[0x1C];
    s16 mF8_targetableCount;
    u8 mFA_pad2[0xA6];
    void* m1A0_targetableArray;
    void* m1A4_targetablePositionData;
    u8 m1A8_pad3[0xC0];
    sVec3_FP m268_scale;
    u8 m274_pad[0x90];
    s8 m304_state;
    u8 m305_pad;
    s8 m306_dangerQuadrant;
    u8 m307_pad;
    s8 m308_variantIndex;
    u8 m309_pad;
    s8 m30A_commandIndex;
    s8 m30B_subCommand;
    s8 m30C_flag0;
    s8 m30D_flag1;
    s8 m30E_flag2;
    s8 m30F_flag3;
    s8 m310_attackActive;
    s8 m311_animPending;
    u8 m312_pad[2];
    p_workArea m314_attackSubTask;
    u8 m318_pad[0xC];
    // size 0x324
};

p_workArea BTL_X0_createEnemyModels(s_workArea* pFormation, s8 variant);
p_workArea BTL_X0_createEnemyModels2(s_workArea* pFormation);
