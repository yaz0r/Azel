#pragma once

#include "battle/battleDragon.h" // todo: clean
#include "battle/battleFormation.h"

struct sBaldorBodyPart
{
    sBaldorBodyPart* m0_child;
    sVec3_FP m4_worldPosition;
    sVec3_FP m10_translation;
    sVec3_FP m1C_rotation;
    sVec3_FP m28_rotationVelocity;
    sVec3_FP m34_rotationAcceleration;
    s16 m40_modelIndex;
    sVec3_FP m44_springStiffness;
    fixedPoint m50_damping;
    //size 0x54
};

struct sBaldorBody
{
    sVec3_FP m0_translation;
    sVec3_FP mC_rotation;
    sVec3_FP m18_rotationTarget;
    void (*m24_update)(sBaldorBodyPart*, const sVec3_FP*, const sVec3_FP*, const sVec3_FP*);
    void (*m28_draw)(struct sBaldor*, sBaldorBodyPart*);
    void (*m2C_delete)();
    std::vector<sBaldorBodyPart> m30_parts;
    //size 0x34
};

// This is shared with other battle models
struct sBaldorBase : public s_workAreaTemplateWithArgWithCopy<sBaldorBase, struct sFormationData*>
{
    s8 m8_mode;
    s8 m9_attackStatus;
    s8 mA_indexInFormation;
    s8 mB_flags;
    s16 mC_numTargetables;
    s16 mE_damageValue;
    s16 m10_HP;
    s8 m12_damagePending;
    std::vector<sBattleTargetable> m14_targetable;
    std::vector<sVec3_FP> m18_position;
    pvecControl m1C_translation;
    pvecControl m28_rotation;
    struct sFormationData* m34_formationEntry;
    s_3dModel* m38_3dModel;
    sSaturnPtr m3C_dataPtr;
    struct sEnemyLifeMeterTask* m40_enemyLifeMeterTask;
};

struct sBaldor : public sBaldorBase
{
    sVec3_FP m44_translationTarget;
    sVec3_FP m50_translationDelta;
    sVec3_FP m5C_rotationDelta;
    sBaldorBody* m68_body;
    sVec3_FP m6C_oscillationPhase;
    sVec3_FP m78_movementVector;
    p_workArea m90_attackTask;
    // 0x94
};

sBaldor* createBaldor(s_workAreaCopy* parent, struct sFormationData* pFormationEntry);

