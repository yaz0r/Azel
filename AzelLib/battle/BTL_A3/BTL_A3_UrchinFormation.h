#pragma once

struct sAttackCommand
{
    sSaturnPtr m0_attackParams;
    sSaturnPtr m4_cameraList;
    s8 m8_type;
    u8 m9_flags;
    s8 mA_attackDisplayName;
    // size 0xC
};

struct sGenericFormationPerTypeDataSub1C
{
    sSaturnPtr m0;
    std::array<sAttackCommand*, 4> m4;
    std::array<s8, 4> m14_quadrantFlags; // per-quadrant counter thresholds (0 = no counter)
    std::array<s8, 4> m18_nextAnimIndex; // per-quadrant next attack anim index (-1 = keep current)
    u16 m1C_animationOffset;
    s8 m1E_quadrantAttackDirections; // rotated as a bitfield and ORed to compute per-quadrant safety/danger
    u16 m20_timerValue;
    // size 0x24
};

struct sGenericFormationPerTypeData
{
    s8 m0_enemyTypeId;
    s8 m1_fileBundleIndex;
    s8 m2;
    s16 m4;
    u16 m8_modelOffset;
    u16 mA_poseOffset;
    sHotpointBundle* mC_hotspotDefinitions;
    std::array<s8, 4> m10_laserHitQuadrantFlags; // per-quadrant laser hit vulnerability (Saturn offset 0x10)
    std::array<s8, 4> m14_gunHitQuadrantFlags;   // per-quadrant gun hit vulnerability (Saturn offset 0x14)
    s8 m18_knockbackStrength;
    std::vector<sGenericFormationPerTypeDataSub1C> m1C;
    sGenericFormationPerTypeDataSub1C* m20_beamHitCounterData;
    sGenericFormationPerTypeDataSub1C* m24_beamHitAttackData;
    std::array<s8, 4> m28;
    s8 m38;
};

struct sGenericFormationSubData
{
    std::vector<sVec3_FP> m0_perEnemyPosition;
};

struct sGenericFormationData
{
    s8 m0_formationSize;
    std::array<s8, 3> m1_perTypeCount;
    std::array<sGenericFormationPerTypeData*, 3> m4_perTypeParams;
    std::vector<sGenericFormationSubData*> m10_formationSubData;
    s8 m14;
    s8 m15_formationNameIndex;
    s8 m16;
    s8 m17;
    std::array<s8, 3> m18_initialDirections;// unknown array size
    s32 m1C;
    std::array<s8, 3> m20_deathSoundEffect;// unknown array size

};

struct sPerEnemySlot
{
    sVec3_FP m0_worldPosition;
    sVec3_FP mC_initialPosition;
    s8 m18_statusFlags;
    s8 m19_attackFlags;
    s8 m1A_hpRatio;
    u16 m1C_flags;
    //size 0x20
};

struct sSharedFormationState
{
    sAttackCommand* m0_currentAttack;
    std::array<s8, 3> m4_typeAlive;
    std::array<s8, 3> m7_attackAnimIndex;
    std::array<s8, 3> mA;
    std::array<s8, 6> mD;
    std::vector<sPerEnemySlot> m14;
};

struct BTL_A3_UrchinFormation : public s_workAreaTemplateWithArg<BTL_A3_UrchinFormation, const sGenericFormationData*>
{
    s8 m1_state;
    s8 m2_subState;
    s8 m3_formationSize;
    s8 m4_formationPhase;
    s8 m6_attackFlag;
    s8 m7_attackMode;
    std::array<std::array<s8, 4>, 3> m8_quadrantAttackCounters; // 0x08: per-type, per-quadrant attack counters
    s16 m14_timer;
    sSharedFormationState m18;
    const sGenericFormationData* m30_config;
    s8 m34_attackingTypeIndex;
    s16 m36_meleeAttackTimer;
    s8 m38_attackOrderReversed;
    s8 m39_sourceEnemyIndex;
    s8 m3A_weakestEnemyIndex;
    s16 m3C_rangedAttackTimer;
    p_workArea m40_pendingTask;
    std::array<s8, 3> m44_prevSoundEffects;
    std::array<s8, 5> m47_beamHitHandled; // up to 5 entries (0x47..0x4B within Saturn size 0x4C)
    // size 0x4C
};

p_workArea Create_BTL_A3_UrchinFormation(p_workArea parent, const sGenericFormationData* config);
p_workArea Create_BTL_A3_UrchinFormationConfig(p_workArea parent, u32 arg);

