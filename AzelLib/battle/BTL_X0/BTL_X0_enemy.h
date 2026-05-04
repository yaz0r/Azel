#pragma once

#include "battle/battleEnemyModels.h"
#include "battle/interpolators/vec2FPInterpolator.h"

struct s_battleDragon;

struct sBTL_X0_EnemyModel : public s_workAreaTemplateWithCopy<sBTL_X0_EnemyModel>
{
    s_fileBundle* m0_fileBundle;
    u8 m4_pad[4];
    p_workArea m8_parentFormation;
    u8 mC_pad[4];
    p_workArea m10_lifeMeterTask;
    u32 m14_flags;  // bit 0 = skip animation step; bit 23 (0x800000) = apply conditional light color
    u8 m18_pad[4];
    sVec3_FP m1C_lifeMeterPosition;
    sVec3_FP m28_rotation;
    sVec3_FP m34_fallVelocity;
    sVec3_FP m40_fallAcceleration;
    u8 m4C_pad0[0xC];
    sVec3_FP m58_fallRotVelocity;
    u8 m64_pad1[0x18];
    sVec3_FP m7C_position;
    u8 m88_pad[0x10];
    // m98_models[0] = Atolm body (main enemy dragon) — has hotpoint bundle for targeting
    // m98_models[1] = Atolm debug/alternate model — drawn in place of [0] when m30D_flag1 is set (debug toggle 0xC5)
    // m98_models[2] = Azel (rider) — drawn at a world-space position anchored to body bone 10's hotpoint 0 (Atolm's head)
    std::vector<s_3dModel> m98_models;
    u8 mAA_modelCount;
    u8 mAB_pad[0x41];
    s16 mEC_hpMax;
    s16 mEE_hpCurrent;
    s16 mF0_frameCounter;
    s16 mF2_animIndex;
    s16 mF4_projectileIndex;
    u8 mF6_pad[6];
    s8 mFC_idleState;
    u8 mFD_pad[0xAF];
    // sVec2FPInterpolator: 0x3A bytes covering 0x1AC-0x1E5. Saturn reuses its fields under aliases:
    //   m1B8_targetPosition   = m1AC_interpolator.mC_startValue       (0x1B8 = 0x1AC + 0x0C)
    //   m1D0_randomOffset     = m1AC_interpolator.m24_targetValue     (0x1D0 = 0x1AC + 0x24)
    //   m1E4_timer            = m1AC_interpolator.m38_interpolationLength (0x1E4 = 0x1AC + 0x38)
    sVec2FPInterpolator m1AC_interpolator;
    u8 m1E6_pad2[0x22];  // 0x1E6-0x207
    s16 mF8_targetableCount;
    u8 mFA_pad2[0xA6];
    void* m1A0_targetableArray;
    void* m1A4_targetablePositionData;
    void* m1A8_attackDataBuffer;  // 0x1A8 — heap buffer: 0x54 bytes for variants 0/1/3, 0xB4 bytes for variant 2
    u8 m1AC_padAfterAttackBuf[0xC0];
    sVec3_FP m26C_scale;  // 0x26C - Ghidra Draw reads scale rows at 0x26C/0x270/0x274
    u8 m278_pad[0x18];
    sVec3_FP m290_cameraAngle;
    u8 m29C_pad[0xC];
    sVec3_FP m2A8_cameraRotation;
    sVec3_FP m2B4_cameraRotVelocity;
    u8 m2C0_pad[0xC];
    sVec3_FP m2CC_cameraAngle2;
    u8 m2D8_pad[0xC];
    sVec3_FP m2E4_cameraRotation2;
    sVec3_FP m2F0_cameraRotVelocity2;
    u8 m2FC_pad[6];
    s16 m302_damageValue;
    s8 m304_state;
    u8 m305_attackQuadrantBit;
    s8 m306_dangerQuadrant;
    u8 m307_subState;
    s8 m308_variantIndex;
    u8 m309_cameraEffectType;
    s8 m30A_commandIndex;
    s8 m30B_subCommand;
    s8 m30C_flag0;
    s8 m30D_flag1;
    s8 m30E_flag2;
    s8 m30F_flag3;
    s8 m310_attackActive;
    s8 m311_animPending;
    u8 m312_pad[2];
    p_workArea m314_attackSubTask;
    fixedPoint m318_animInterp;
    fixedPoint m31C_animInterpSpeed;
    u8 m320_pad[4];
    // size 0x324
};

// 060b71e8 task definition: {null, 06057e04, null, 060586d0}
struct sBTL_X0_AttackSubTask : public s_workAreaTemplate<sBTL_X0_AttackSubTask>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { nullptr, &Update, nullptr, &Delete };
        return &taskDefinition;
    }

    static void Update(sBTL_X0_AttackSubTask* pThis);
    static void Delete(sBTL_X0_AttackSubTask* pThis);

    sBTL_X0_EnemyModel* m0_parentEnemy;
    void* m4_attackDataBuffer;
    sVec3_FP m8_positions[6];
    fixedPoint m50_rotX;
    fixedPoint m54_rotY;
    fixedPoint m58_rotZ;
    u16 m5C_timer;
    u16 m5E_partCmd[4];
    u8 m66_partOrder[4];
    u8 m6A_targetPart;
    u8 m6B_lastCommand;
    u8 m6C_variant;
    u8 m6D_pad[3];
    s32 m70_partAlive[4];
    s32 m80_partError[4];
    s32 m90_partComplete[4];
    s32 mA0_partState[4];
    s32 mB0_value;
    s32 mB4_trigger;
    s32 mB8_pass;
    // Saturn size 0xBC
};

// 060b73d8 task definition: {null, 06059444, 06059b12, 06059b8e}
// Per-part sub-task for attack sub-parts (appendages). Saturn size 0x308.
// Minimal definition — will be expanded when per-part logic is implemented.
struct sBTL_X0_AttackSubPart : public s_workAreaTemplateWithCopy<sBTL_X0_AttackSubPart>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { nullptr, &Update, &Draw, &Delete };
        return &taskDefinition;
    }

    static void Update(sBTL_X0_AttackSubPart* pThis);
    static void Draw(sBTL_X0_AttackSubPart* pThis);
    static void Delete(sBTL_X0_AttackSubPart* pThis);

    u32 m14_flags;
    u8 mFC_state;
    sBTL_X0_AttackSubTask* m154_parentAttackTask;
    s32 m208_value;
    s32 m29C_value;
    s32 m2A0_value;
    u8 m300_partIndex;
    u8 m301_flag;
    s32 m304_state;
    // Saturn size 0x308
};

// 060b7474 task definition: {null, 0605df00, 0605e148, null}
struct sBTL_X0_Projectile : public s_workAreaTemplateWithCopy<sBTL_X0_Projectile>
{
    static void Update(sBTL_X0_Projectile* pThis);
    static void Draw(sBTL_X0_Projectile* pThis);

    s_fileBundle* m4_fileBundle;
    sVec3_FP m8_position;
    u8 m14_pad[0xC];
    sVec3_FP m20_velocity;
    sVec3_FP m2C_acceleration;
    u8 m38_pad[0x10];
    sVec3_FP* m48_pDragonPosition;
    u8 m4C_pad[0x40];
    s_battleDragon* m8C_pDragon;
    sVec3_FP* m90_pDragonPosition2;
    u8 m94_trailData[0x4C];
    s32 mE0_value;
    s16 mE4_timer1;
    s16 mE6_timer2;
    u8 mE8_pad[4];
    s8 mEC_state;
    u8 mED_pad[3];
    s16 mF0_damageType;
    u8 mF2_pad[2];
    // Saturn size 0xF4
};

// 060b7524 task definition: {null, 0605e8b0, null, 0605f1d8}
struct sBTL_X0_MultiBeamTask : public s_workAreaTemplate<sBTL_X0_MultiBeamTask>
{
    static void Update(sBTL_X0_MultiBeamTask* pThis);
    static void Delete(sBTL_X0_MultiBeamTask* pThis);

    void* m0_hotpoint0;
    void* m4_hotpoint1;
    void* m8_hotpoint2;
    sVec3_FP mC_targetPosition;
    sVec3_FP m18_cameraPosition;
    sVec3_FP m24_pad0;
    sVec3_FP m30_pad1;
    sVec3_FP m3C_dragonPos;
    s8 m48_cameraActive;
    s32 m4C_targetRotation;
    fixedPoint m50_rotVelocity;
    s32 m54_currentRotation;
    u8 m58_state0;
    s8 m59_variantIndex;
    s8 m5A_quadrant;
    s8 m5B_phase;
    s8 m5C_fireRate;
    u8 m5D_pad;
    s16 m5E_frameCounter;
    u8 m60_pad[4];
    s_3dModel* m64_pModel0;
    s_3dModel* m68_pModel2;
    s32 m6C_cameraFlag;
    // Saturn size 0x70
};

p_workArea BTL_X0_createEnemyModels(s_workArea* pFormation, s8 variant);
p_workArea BTL_X0_createEnemyModels2(s_workArea* pFormation);
