#pragma once

#include "battle/battleTargetable.h"
struct sArachnothSubModel
{
    npcFileDeleter* m0;
    sSaturnPtr m4;
    s_3dModel m8_model;
    std::vector<sBattleTargetable> m58_targetables;
    std::vector<sVec3_FP> m5C_targetablesPosition;
    s32 m60_numBones;
    s32 m64;
    s_workAreaCopy* m68;
    s32 m6C;
    u32 m70_flags;
    s32 m74;
    s32 m7C;
    void (*m80)(s_workAreaCopy* parent, sBattleTargetable* targetable);
    void (*m84)(s_workAreaCopy* parent, sBattleTargetable* targetable);
    void (*m88)(s_workAreaCopy* parent, sBattleTargetable* targetable);
    void (*m8C)(s_workAreaCopy* parent, sBattleTargetable* targetable);
    //size 0x90

    // This is to keep a copy of the bundle data after serialization
    sHotpointBundle* hotpointBundle; // TODO: handle deletion (currently leaking)
};

void arachnothCreateSubModel(sArachnothSubModel* pThis, s_workAreaCopy* pParent, npcFileDeleter* param_3, s8 param_4, sSaturnPtr param_5);
void arachnothCreateSubModel2(sArachnothSubModel* pThis, s_workAreaCopy* pParent, npcFileDeleter* param_3, s8 param_4, sSaturnPtr param_5, sSaturnPtr param_6, sSaturnPtr param_7);
void arachnothInitSubModelAnimation(sArachnothSubModel* pThis, s32, s32);
typedef void (*arachnothSubModelCallback)(s_workAreaCopy*, sBattleTargetable*);
void arachnoth_setSubModelCallbacks(sArachnothSubModel* pThis, arachnothSubModelCallback param_2, arachnothSubModelCallback param_3, arachnothSubModelCallback param_4, arachnothSubModelCallback param_5);
void arachnothSubModelFunction0(s_workAreaCopy* parent, sBattleTargetable* targetable);
void arachnothSubModelFunction1(s_workAreaCopy* parent, sBattleTargetable* targetable);
void arachnothSubModelFunction2(s_workAreaCopy* parent, sBattleTargetable* targetable);
void arachnoth_rotateTargetableFlags(sArachnothSubModel* pThis, s32);
s32 arachnoth_processSubModelDamage(sArachnothSubModel* pThis);
