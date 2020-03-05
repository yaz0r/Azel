#pragma once

#include "battle/battleDragon.h" // todo: clean
#include "battle/battleFormation.h"

struct sBaldor_68_30
{
    sBaldor_68_30* m0;
    sVec3_FP m4;
    sVec3_FP m10_translation;
    sVec3_FP m1C;
    sVec3_FP m28;
    sVec3_FP m34;
    s16 m40;
    sVec3_FP m44;
    s32 m50;
    //size 0x54
};

struct sBaldor_68
{
    sVec3_FP m0_translation;
    sVec3_FP mC_rotation;
    sVec3_FP m18;
    void (*m24_update)(sBaldor_68_30*, sVec3_FP*, sVec3_FP*, sVec3_FP*);
    void (*m28_draw)(struct sBaldor*, sBaldor_68_30*);
    void (*m2C_delete)();
    std::vector<sBaldor_68_30> m30;
    //size 0x34
};

struct sBaldor : public s_workAreaTemplateWithArgWithCopy<sBaldor, struct sFormationData*>
{
    s8 m8_mode;
    s8 m9;
    s8 mA_indexInFormation;
    s8 mB;
    s16 mC;
    s16 mE;
    s8 m10_HP;
    s8 m12;
    std::vector<s_battleDragon_8C> m14;
    std::vector<sVec3_FP> m18;
    pvecControl m1C_translation;
    pvecControl m28_rotation;
    struct sFormationData* m34_formationEntry;
    s_3dModel* m38_3dModel;
    sSaturnPtr m3C_dataPtr;
    p_workArea m40;
    sVec3_FP m44_translationTarget;
    sVec3_FP m50_translationDelta;
    sVec3_FP m5C_rotationDelta;
    sBaldor_68* m68;
    sVec3_FP m6C;
    sVec3_FP m78;
    p_workArea m90_attackTask;
    // 0x94
};

sBaldor* createBaldor(s_workAreaCopy* parent, struct sFormationData* pFormationEntry);

sVec3_FP* Baldor_updateSub0Sub2Sub1(s_battleDragon_8C& param1); // TODO: cleanup
