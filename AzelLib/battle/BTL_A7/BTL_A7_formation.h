#pragma once

#include "battle/battleFormationBase.h"

struct sBTL_A7_FormationTask : public s_workAreaTemplateWithArgAndBase<sBTL_A7_FormationTask, sFormationTaskBase>
{
    u8 mF0_pad6[4];
    s8 mF4_attackPatternIndex;
    s8 mF5_flag;
    s8 mF6_attackCounter;
    u8 mF7_pad7;
    // size 0xF8
};

void BTL_A7_createFormation(s_workAreaCopy* pParent, u32 arg0);
