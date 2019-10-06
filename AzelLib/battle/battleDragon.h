#pragma once

struct s_battleDragon_8C
{
    struct s_battleDragon* m0;
    sVec3_FP* m4;
    sVec3_FP* m8;
    s32 mC;
    sVec3_FP m10;
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
    s_battleDragon_8C m8C;
    sVec3_FP m1A4;
    s32 m1C8;
    s16 m1CC;
    s16 m1CE;
    s32 m24C;
    // size 0x250
};

void battleEngine_createDragonTask(s_workAreaCopy* parent);
