#pragma once

#include "battle/battleFormation.h"

struct BTL_A3_BaldorFormation : public s_workAreaTemplateWithArg<BTL_A3_BaldorFormation, u32>
{
    s8 m0_formationState;       // 0x00
    s8 m1_formationSubState;    // 0x01
    std::vector<sFormationData> m4_formationData; // 0x04 (Saturn: 4-byte pointer)
    s16 m8;                     // 0x08
    s16 m10;                    // 0x10: wait timer (decremented in attack state)
    s8 m12_formationSize;       // 0x12
    // size 0x14
};

p_workArea Create_BTL_A3_BaldorFormation(p_workArea parent, u32 arg);
