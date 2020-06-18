#pragma once

struct sGenericFormationPerTypeData
{
    s8 m0;
    s8 m1_fileBundleIndex;
    s8 m2;
    s16 m4;
    u16 m8_modelOffset;
    u16 mA_poseOffset;
    std::vector<s_hotpointDefinition>* mC_hotspotDefinitions;
    s8 m18;
    sSaturnPtr m1C;
    u32 m24;
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
    sGenericFormationSubData* m10_formationSubData;
    s8 m14;
    s8 m15;
    s8 m16;
    s8 m17;
    std::array<s8, 3> m18;// unknown array size
    s32 m1C;
    std::array<s8, 3> m20;// unknown array size

};

struct sBTL_A3_UrchinFormation_18_14
{
    sVec3_FP m0;
    sVec3_FP mC;
    s8 m18;
    s8 m19;
    s8 m1A;
    s8 m1C;
    //size 0x20
};

struct sBTL_A3_UrchinFormation_18
{
    sSaturnPtr m0;
    std::array<s8, 3> m4;
    std::array<s8, 3> m7;
    std::array<s8, 3> mA;
    std::array<s8, 2> mD;
    s8 m10;
    s8 m11;
    s8 m12;
    std::vector<sBTL_A3_UrchinFormation_18_14> m14;
};

p_workArea Create_BTL_A3_UrchinFormationConfig(p_workArea parent, u32 arg);

