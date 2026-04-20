#pragma once

#include "battle/battleFormationBase.h"

struct sBTL_A5_FormationTask : public s_workAreaTemplateWithArgAndBase<sBTL_A5_FormationTask, sFormationTaskBase>
{
    u8 mF0_flag;
    u8 mF1_pad5[3];
    u8 mF4_entityPositions[0x1CC - 0xF4];
    sSaturnPtr m1CC_dataTable2;
    s32 m1D4_flag;
    // size 0x1D8
};

void BTL_A5_createFormation(s_workAreaCopy* pParent, u32 arg0);
