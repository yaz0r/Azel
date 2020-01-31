#pragma once

struct s_battleDragon_8C
{
    struct s_battleDragon* m0;
    sVec3_FP* m4;
    sVec3_FP* m8;
    s32 mC;
    sVec3_FP m10;
    sVec3_FP m1C;
    sVec3_FP m28;
    s32 m4C;
    sVec3_FP m40;
    s32 m50;
    s32 m54;
    s16 m58;
    s16 m5A;
    s16 m5C;
    s8 m5E;
    s8 m5F;
    s8 m60;
};

struct s_battleDragon : public s_workAreaTemplateWithCopy<s_battleDragon>
{
    //0/4 are copy
    sVec3_FP m8_position;
    sVec3_FP m14_rotation;
    sVec3_FP m44_deltaRotation;
    sVec3_FP m5C_deltaPosition;
    sVec3_FP m74_targetRotation;
    s32 m84;
    s32 m88;
    s_battleDragon_8C m8C;
    u32 mDC;
    sVec3_FP mF0;
    std::array<sVec3_FP, 6> mFC_hotpoints;
    sVec3_FP m1A4;
    u32 m1C0;
    u32 m1C4;
    s32 m1C8;
    s16 m1CC;
    s16 m1CE;
    s16 m1D0;
    s32 m24C;
    // size 0x250
};

void battleEngine_createDragonTask(s_workAreaCopy* parent);

void s_battleDragon_InitSub5Sub0(s_battleDragon_8C* pThis);
