#pragma once

#include "battle/battleDragon.h" // todo: clean

struct sBaldor_68_30
{
    sBaldor_68_30* m0;
    sVec3_FP m4;
    sVec3_FP m10_translation;
    sVec3_FP m1C;
    sVec3_FP m28;
    sVec3_FP m34;
    s16 m40;
    sVec3_FP m44;
    s32 m50;
    //size 0x54
};

struct sBaldor_68
{
    sVec3_FP m0_translation;
    sVec3_FP mC_rotation;
    sVec3_FP m18;
    void (*m24_update)(sBaldor_68_30*, sVec3_FP*, sVec3_FP*, sVec3_FP*);
    void (*m28_draw)();
    void (*m2C_delete)();
    std::vector<sBaldor_68_30> m30;
    //size 0x34
};

struct sBaldor : public s_workAreaTemplateWithArgWithCopy<sBaldor, struct sFormationData*>
{
    s8 m8_mode;
    s8 mA_indexInFormation;
    s16 mC;
    s8 m10_HP;
    std::vector<s_battleDragon_8C> m14;
    std::vector<sVec3_FP> m18;
    std::array<sVec3_FP*, 3> m1C;
    std::array<sVec3_FP*, 3> m28;
    struct sFormationData* m34_formationEntry;
    s_3dModel* m38_3dModel;
    sSaturnPtr m3C_dataPtr;
    p_workArea m40;
    sVec3_FP m44;
    sVec3_FP m50;
    sVec3_FP m5C;
    sBaldor_68* m68;
    sVec3_FP m6C;
    sVec3_FP m78;
    // 0x94
};

sBaldor* createBaldor(s_workAreaCopy* parent, struct sFormationData* pFormationEntry);
