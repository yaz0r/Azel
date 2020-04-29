#pragma once

#include "battle/battleDragon.h" // todo: clean
#include "battle/battleFormation.h"

struct sBaldor_68_30
{
    sBaldor_68_30* m0_child;
    sVec3_FP m4;
    sVec3_FP m10_translation;
    sVec3_FP m1C;
    sVec3_FP m28;
    sVec3_FP m34;
    s16 m40;
    sVec3_FP m44;
    fixedPoint m50;
    //size 0x54
};

struct sBaldor_68
{
    sVec3_FP m0_translation;
    sVec3_FP mC_rotation;
    sVec3_FP m18;
    void (*m24_update)(sBaldor_68_30*, const sVec3_FP*, const sVec3_FP*, const sVec3_FP*);
    void (*m28_draw)(struct sBaldor*, sBaldor_68_30*);
    void (*m2C_delete)();
    std::vector<sBaldor_68_30> m30;
    //size 0x34
};

struct sBaldor : public s_workAreaTemplateWithArgWithCopy<sBaldor, struct sFormationData*>
{
    s8 m8_mode;
    s8 m9_attackStatus;
    s8 mA_indexInFormation;
    s8 mB;
    s16 mC_numTargetables;
    s16 mE_damageValue;
    s8 m10_HP;
    s8 m12;
    std::vector<sBattleTargetable> m14_targetable;
    std::vector<sVec3_FP> m18_position;
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

