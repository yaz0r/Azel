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

void battleTargetable_updatePosition(sBattleTargetable* pThis);
sVec3_FP* getBattleTargetablePosition(sBattleTargetable& param1);