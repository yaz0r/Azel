#pragma once

#include "battle/interpolators/FPInterpolator.h"

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
    struct sBattleCommandMenu* m28_battleCommandMenu;
    u32 m2C;
    sFPInterpolator m30;
    sFPInterpolator m4C;
    //size 0x68
};

void battleEngine_CreateHud1(struct npcFileDeleter*);
