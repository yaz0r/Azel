#pragma once

#include "fieldItemBoxDefinition.h"

struct s_itemBoxType1 : public s_workAreaTemplateWithArg<s_itemBoxType1, struct s_itemBoxDefinition*>
{
    s_memoryAreaOutput m0;
    sLCSTarget m8_LCSTarget;
    s8 m20;
    s8 m21;
    sVec3_FP m3C_pos;
    sVec3_FP m48_boundingMin;
    sVec3_FP m54_boundingMax;
    sVec3_FP m60_renderPosition;
    sVec3_FP m6C_rotation;
    fixedPoint m78_scale;
    fixedPoint m7C_invScale;
    s32 m80_bitIndex;
    s16 m84_modelIdx;
    s16 m86;
    s16 m88_poseIdx;
    s8 m8A_param9;
    s8 m8B_LCSType;
    s8 m8C_param10;
    s8 m8D_visibilityFlag;
    s32 m90;
    s32 m94;
    s_3dModel m98_3dModel;
    s16 mE8_animCountdown;
    s16 mEA_state;
    p_workArea mEC_savePointParticlesTask;
    //size: F0
};

extern sVec3_FP LCSItemBox_Table6[];

s8 LCSItemBox_shouldSpin(s_itemBoxType1* pThis);
void LCSItemBox_UpdateType0Sub0(s_fileBundle* pBundle, s32 hierarchyKey, s32 poseKey, fixedPoint scale);

p_workArea createFieldItemBox(struct s_itemBoxDefinition* r4);

