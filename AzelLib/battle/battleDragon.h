#pragma once

#include "battleTargetable.h"

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
    sVec3_FP mF0;
    std::array<sVec3_FP, 6> mFC_hotpoints;
    sVec3_FP m144; // not sure of type yet
    sVec3_FP m1A4;
    u32 m1C0_statusModifiers;
    u32 m1C4;
    s32 m1C8;
    s16 m1CC_currentAnimation;
    s16 m1CE_positionInAnimList;
    s16 m1D0;
    s16 m1D4_damageTaken;
    s16 m1D6;
    fixedPoint m208;
    s32 m20C;
    s16 m210;
    std::array<s8, 4> m244;
    std::array<s8, 4> m248;
    p_workArea m24C_lightWingEffectTask;
    // size 0x250
};

void battleEngine_createDragonTask(s_workAreaCopy* parent);

