#pragma once

struct sUrchinFormationDataSub
{
    s8 m1;
    s8 m2;
    u16 m8;
    u16 mA;
    std::vector<s_RiderDefinitionSub>* mC;
    u32 m24;
};

struct sUrchinFormationDataSub_10
{
    std::vector<sVec3_FP> m0;
};

struct sUrchinFormationData
{
    s8 m0;
    std::array<s8, 3> m1;
    std::array<sUrchinFormationDataSub*, 3> m4;
    sUrchinFormationDataSub_10* m10;
    s8 m14;
    s8 m15;
    s8 m16;
    s8 m17;
    std::array<s8, 8> m18;// unknown array size
    std::array<s8, 3> m20;// unknown array size

};

struct sBTL_A3_UrchinFormation_18_14
{
    sVec3_FP m0;
    sVec3_FP mC;
    s8 m18;
    s8 m19;
    s8 m1C;
    //size 0x20
};

struct sBTL_A3_UrchinFormation_18
{
    void* m0;
    std::array<s8, 3> m4;
    std::array<s8, 3> m7;
    std::array<s8, 3> mA;
    std::array<s8, 2> mD;
    s8 m10;
    sBTL_A3_UrchinFormation_18_14* m14;
};

p_workArea Create_BTL_A3_UrchinFormationConfig(p_workArea parent, u32 arg);

