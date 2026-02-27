#pragma once

struct sAttackCommand
{
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
    std::array<s8, 4> m14_quadrantFlags;
    u16 m1C_animationOffset;
    s8 m1E_quadrantAttackDirections; // rotated as a bitfield and ORed to compute per-quadrant safety/danger
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
    s8 m18_knockbackStrength;
    std::vector<sGenericFormationPerTypeDataSub1C> m1C;
    u32 m24;
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
    s8 m1C_flags;
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

p_workArea Create_BTL_A3_UrchinFormationConfig(p_workArea parent, u32 arg);

