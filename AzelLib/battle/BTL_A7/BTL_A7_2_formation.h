#pragma once

#include "battle/battleFormationBase.h"

struct sBTL_A7_2_FormationTask : public s_workAreaTemplateWithArgAndBase<sBTL_A7_2_FormationTask, sFormationTaskBase>
{
    u8 mF0_pad5[4];
    sEntityGroup mF4_entityGroup2;
    u8 m104_pad6;
    s8 m105_secondaryFlag;
    u8 m106_pad7[2];
    sFormationPositionBlock m108_posBlock2;
    s8 m184_updateState;
    s8 m185_stateStep;
    u8 m186_pad9[2];
    p_workArea m188_introTask;
    u8 m18C_pad10[4];
    sVec3_FP m190_targetPosition;
    // size 0x19C
};

void BTL_A7_2_createFormation(s_workAreaCopy* pParent, u32 arg0);
