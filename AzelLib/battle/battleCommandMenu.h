#pragma once

struct sBattleCommandMenu : public s_workAreaTemplate<sBattleCommandMenu>
{
    s8 m0;
    s8 m1;
    s8 m2_mode;
    std::array<s8, 6> m4;
    s16 mC;
    s16 mE;
    s16 m14;
    s16 m16;
    s32 m1C;
    u32 m20;
    struct s1C0 // TODO: is this the same as s_battleOverlay_20_sub ?
    {
        s32 m0_currentValue;
        s32 m4;
        s32 m8;
        s32 mC;
        fixedPoint m10_currentStepValue;
        s32 m14_stepIncrement;
        s16 m18;
    } m1C0;
    sVec3_FP m1DC;
    //size 0x1e8
};

void createBattleCommandMenu(p_workArea parent);
