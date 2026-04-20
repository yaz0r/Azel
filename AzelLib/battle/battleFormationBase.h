#pragma once

struct sFormationPositionBlock;

struct sEntityGroup
{
    p_workArea* m0_entityArray;
    sFormationPositionBlock* m4_positionBlock;
    sSaturnPtr m8_dataTable;
    s16 mC_formationNameIndex;
    s8 mE_numEntities;
    s8 mF_deadCount;
    // size 0x10
};

struct sFormationPositionBlock
{
    sVec3_FP m0_position;
    u8 mC_pad[0x18 - 0x0C];
    sVec3_FP m18_velocity;
    u8 m24_pad[4];
    s32 m28_altitudeOffset;
    u8 m2C_pad[0x30 - 0x2C];
    sVec3_FP m30_target;
    u8 m3C_pad[0x60 - 0x3C];
    sVec3_FP m60_currentInterp;
    u8 m6C_pad[0x78 - 0x6C];
    fixedPoint m78_interpRate;
};

struct sFormationTaskBase;
typedef void (*formationUpdateCallback)(sFormationTaskBase*);

struct sFormationTaskBase : public s_workAreaTemplateWithCopy<sFormationTaskBase>
{
    s16 m20;
    s16 m22;
    s16 m24_counter1;
    s16 m26_counter2;
    u8 m28_state;
    u8 m29_pad[3];
    sFormationPositionBlock m2C_posBlock;
    // m2C + 0x78 + 4 = mA8
    sVec3_FP mA8_position;
    u8 mB4_pad[0xC0 - 0xB4];
    sVec3_FP mC0_position2;
    s8 mCC_attackType;
    s8 mCD_displayMode;
    s16 mCE_attackDuration;
    s16 mD0_attackNameIndex;
    u8 mD2_pad[2];
    s32 mD4_attackFlag;
    formationUpdateCallback mD8_updateCallback;
    s8 mDC_attackCamera;
    s8 mDD_moveDirection;
    s16 mDE_moveAgility;
    sEntityGroup mE0_entityGroup;
    // size 0xF0
};

void formationCleanupEntityArray(sEntityGroup* pGroup);
s32 formationAreAllEntitiesDead(sEntityGroup* pGroup);
s32 formationCheckEntityFlag(sEntityGroup* pGroup, u32 flagBit);
void formationUpdatePosition(sFormationPositionBlock* pBlock, sVec3_FP* pBattleCenter);
void formationApplyAutoScroll(sFormationTaskBase* pThis);
void formationSubStateMachine(sFormationTaskBase* pThis, sEntityGroup* pGroup);
void formationTriggerAttack(sEntityGroup* pGroup, s8 attackType);
void formationTriggerAttackFollowup(sEntityGroup* pGroup, s8 attackType);
void formationCreateTransitionTask(sFormationTaskBase* pThis, sSaturnPtr data);
p_workArea formationCreateIntroEffect(sFormationTaskBase* pThis, sSaturnPtr data);
