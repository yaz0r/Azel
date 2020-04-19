#pragma once

struct sBattleTargetable
{
    struct s_battleDragon* m0;
    sVec3_FP* m4_pPosition;
    sVec3_FP* m8;
    sVec3_FP* mC;
    sVec3_FP m10_position;
    sVec3_FP m1C;
    sVec3_FP m28;
    sVec3_FP m34;
    s32 m4C;
    sVec3_FP m40;
    u32 m50_flags;
    s32 m54;
    s16 m58;
    s16 m5A;
    s16 m5C;
    s8 m5E;
    s8 m5F;
    s8 m60;

    //size: 64
};

struct s_battleDragon : public s_workAreaTemplateWithCopy<s_battleDragon>
{
    //0/4 are copy
    sVec3_FP m8_position;
    sVec3_FP m14_rotation;
    sVec3_FP m44_deltaRotation;
    sVec3_FP m5C_deltaPosition;
    sVec3_FP m74_targetRotation;
    s32 m84; // 1: play homing laser animation
    s32 m88;
    sBattleTargetable m8C;
    sVec3_FP m90;
    sVec3_FP m9C;
    sVec3_FP mCC;
    u32 mDC;
    sVec3_FP mF0;
    std::array<sVec3_FP, 6> mFC_hotpoints;
    sVec3_FP m144; // not sure of type yet
    sVec3_FP m1A4;
    u32 m1C0;
    u32 m1C4;
    s32 m1C8;
    s16 m1CC_currentAnimation;
    s16 m1CE_positionInAnimList;
    s16 m1D0;
    s16 m1D4;
    s16 m1D6;
    fixedPoint m208;
    s32 m20C;
    s16 m210;
    std::array<s8, 4> m244;
    std::array<s8, 4> m248;
    p_workArea m24C;
    // size 0x250
};

void battleEngine_createDragonTask(s_workAreaCopy* parent);

void battleTargetable_updatePosition(sBattleTargetable* pThis);
