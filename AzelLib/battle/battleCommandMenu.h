#pragma once

#include "battle/interpolators/FPInterpolator.h"

struct sBattleCommandMenu : public s_workAreaTemplate<sBattleCommandMenu>
{
    s8 m0_selectedBattleCommand;
    s8 m1_numBattleCommands;
    s8 m2_mode;
    s8 m3;
    std::array<s8, 6> m4_enabledBattleCommands;
    s16 mC;
    s16 mE;
    s16 m14;
    s16 m16;
    s16 m18_oldDragonAtk;
    s16 m1A_oldDragonDef;
    s32 m1C;
    u32 m20;
    s32 m24;
    sFPInterpolator m1C0_scrollInterpolator;
    sVec3_FP m1DC;
    //size 0x1e8
};

void createBattleCommandMenu(p_workArea parent);
