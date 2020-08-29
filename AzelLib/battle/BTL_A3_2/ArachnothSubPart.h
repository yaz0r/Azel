#pragma once

#include "battle/battleTargetable.h"
struct sArachnothSubModel
{
    npcFileDeleter* m0;
    sSaturnPtr m4;
    s_3dModel m8_model;
    std::vector<sBattleTargetable> m58_targetables;
    std::vector<sVec3_FP> m5C_targetablesPosition;
    s32 m60;
    s32 m64;
    s_workAreaCopy* m68;
    s32 m6C;
    s32 m74;
    s32 m7C;
    s32 m80;
    s32 m84;
    s32 m88;
    s32 m8C;
    //size 0x90

    // This is to keep a copy of the bundle data after serialization
    sHotpointBundle* hotpointBundle; // TODO: handle deletion (currently leaking)
};

void arachnothCreateSubModel(sArachnothSubModel* pThis, s_workAreaCopy* pParent, npcFileDeleter* param_3, s8 param_4, sSaturnPtr param_5);
void arachnothCreateSubModel2(sArachnothSubModel* pThis, s_workAreaCopy* pParent, npcFileDeleter* param_3, s8 param_4, sSaturnPtr param_5, sSaturnPtr param_6, sSaturnPtr param_7);
void arachnothInitSubModelAnimation(sArachnothSubModel* pThis, s32, s32);
void arachnothInitSubModelFunctions(sArachnothSubModel* pThis, s32, void (*param_2)(sArachnothSubModel*, s32), void (*param_3)(sArachnothSubModel*, s32), void (*param_4)(sArachnothSubModel*, s32));
void arachnothSubModelFunction0(sArachnothSubModel* pThis, s32);
void arachnothSubModelFunction1(sArachnothSubModel* pThis, s32);
void arachnothSubModelFunction2(sArachnothSubModel* pThis, s32);
void createArachnothFormationSub0(sArachnothSubModel* pThis, s32);
s32 arachnothSubPartGetDamage(sArachnothSubModel* pThis);
