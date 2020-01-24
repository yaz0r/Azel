#pragma once

struct s_battleOverlay_20_sub
{
    s32 m0_currentValue;
    s32 m4;
    s32 m8;
    s32 mC;
    fixedPoint m10_currentStepValue;
    s32 m14_stepIncrement;
    s16 m18;
};

struct s_battleOverlay_20 : public s_workAreaTemplate<s_battleOverlay_20>
{
    s8 m0;
    s16 m10_currentMode;
    s16 m12_nextMode;
    u16 m14_vdp1Memory;
    u16 m16_part1X;
    u16 m18_part1Y;
    s16 m1A_part2X;
    s16 m1C_part2Y;
    u16 m1E;
    u16 m20;
    u16 m22;
    u16 m24;
    s32 m28;
    u32 m2C;
    s_battleOverlay_20_sub m30;
    s_battleOverlay_20_sub m4C;
    //size 0x68
};

void createBattleOverlay_task20(struct npcFileDeleter*);
