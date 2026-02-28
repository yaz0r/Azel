#pragma once

#include "town/town.h"

struct sTownDragon : public s_workAreaTemplateWithArg<sTownDragon, sSaturnPtr>, sTownObject
{
    //0-C : sTownObject
    s8 mC;
    s8 mD;
    s8 mE;
    s8 mF;
    s8 m10;
    s8 m11;
    s8 m12;
    s16 m14;
    s16 m16;
    npcFileDeleter* m1C;
    sSaturnPtr m20;
    void* m24;
    s32 m3C;
    s32 m40;
    sSaturnPtr m48;
    sVec3_FP m4C;
    sVec3_FP m58;
    sVec3_FP m64;
    sMainLogic_74 m70;
    s16 mD4_cursorX;
    s16 mD6_cursorY;
    fixedPoint mD8;
    //size: 0xE8
};

sTownObject* createTownDragon(p_workArea parent, sSaturnPtr arg);
