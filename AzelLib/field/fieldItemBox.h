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
    sVec3_FP m60;
    sVec3_FP m6C_rotation;
    fixedPoint m78_scale;
    fixedPoint m7C;
    s32 m80_bitIndex;
    s16 m84_savePointIndex;
    s16 m86;
    s16 m88_receivedItemId;
    s8 m8A_receivedItemQuantity;
    s8 m8B_LCSType;
    s8 m8C;
    s8 m8D;
    s32 m90;
    s32 m94;
    s_3dModel m98_3dModel;
    s16 mE8;
    s16 mEA_wasRendered;
    p_workArea mEC_savePointParticlesTask;
    //size: F0
};

extern sVec3_FP LCSItemBox_Table6[];

s8 LCSItemBox_shouldSpin(s_itemBoxType1* pThis);
void LCSItemBox_UpdateType0Sub0(s_itemBoxType1* pThis, s32 r5, s32 r6, fixedPoint r7);

p_workArea fieldA3_1_createItemBoxes_Sub1(struct s_itemBoxDefinition* r4);

