#pragma once

#include "battle/battleDragon.h" // todo: clean

struct sBaltor_68_30
{
    sBaltor_68_30* m0;
    s32 m4;
    s32 m8;
    s32 mC;
    sVec3_FP m10;
    s32 m1C;
    s32 m20;
    s32 m24;
    s32 m28;
    s32 m2C;
    s32 m30;
    s32 m34;
    s32 m38;
    s32 m3C;
    s16 m40;
    s32 m44;
    s32 m48;
    s32 m4C;
    s32 m50;
    //size 0x54
};

struct sBaltor_68
{
    s32 m0;
    s32 m4;
    s32 m8;
    s32 mC;
    s32 m10;
    s32 m14;
    s32 m18;
    s32 m1C;
    s32 m20;
    void (*m24_update)();
    void (*m28_draw)();
    void (*m2C_delete)();
    std::vector<sBaltor_68_30> m30;
    //size 0x34
};

struct sBaltor : public s_workAreaTemplateWithArgWithCopy<sBaltor, struct sFormationData*>
{
    s8 m2;
    s8 mA_indexInFormation;
    s16 mC;
    s8 m10;
    std::vector<s_battleDragon_8C> m14;
    std::vector<sVec3_FP> m18;
    std::array<sVec3_FP*, 3> m1C;
    std::array<sVec3_FP*, 3> m28;
    struct sFormationData* m34_formationEntry;
    s_3dModel* m38_3dModel;
    sSaturnPtr m3C_dataPtr;
    p_workArea m40;
    s8 m48;
    s8 m49;
    sBaltor_68* m68;
    u32 m6C;
    u32 m70;
    u32 m74;
    // 0x94
};

sBaltor* createBaltor(s_workAreaCopy* parent, struct sFormationData* pFormationEntry);
