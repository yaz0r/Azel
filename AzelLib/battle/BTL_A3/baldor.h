#pragma once

#include "battle/battleDragon.h" // todo: clean
#include "battle/battleFormation.h"
#include "kernel/monsterPart.h"

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
    sMonsterBody* m68_body;
    sVec3_FP m6C_oscillationPhase;
    sVec3_FP m78_movementVector;
    p_workArea m90_attackTask;
    // 0x94
};

sBaldor* createBaldor(s_workAreaCopy* parent, struct sFormationData* pFormationEntry);
void Baldor_initSub0(sBaldorBase* pThis, sSaturnPtr dataPtr, struct sFormationData* pFormationEntry, s32 arg);
void Baldor_update(sBaldorBase* pThisBase);
void Baldor_draw(sBaldorBase* pThisBase);

// baldor-specific segmented-body physics callbacks — installed by
// monsterBody_loadPartData(body, 1, ...). Kept here because they're only
// used by BTL_A3 Baldor (and they live in baldor.cpp).
void baldorPart_update(sMonsterBodyPart* pThis, const sVec3_FP* pTranslation, const sVec3_FP* pRotation, const sVec3_FP* param4);
void baldorPart_draw(struct s_fileBundle* pBundle, sMonsterBodyPart* pBodyPart);
void baldorPart_drawPart(struct s_fileBundle* pBundle, sMonsterBodyPart* pBodyPart);

