#pragma once

#include "town/town.h"

struct sTownDragon : public s_workAreaTemplateWithArg<sTownDragon, sSaturnPtr>, sTownObject
{
    //0-C : sTownObject
    s8 mC_dragonType;
    s8 mD_drawExtras;
    s8 mE;
    s8 mF_affinityLevel;
    s8 m10_modeOffset;
    s8 m11_subState;
    s8 m12_eventFlag;
    s16 m14_readyState;
    s16 m16_timer;
    npcFileDeleter* m1C;
    sSaturnPtr m20_scriptEA;
    void* m24;
    s32 m3C;
    s32 m40;
    sSaturnPtr m48_entityEA;
    sVec3_FP m4C_basePosition;
    sVec3_FP m58_position;
    sVec3_FP m64_rotation;
    sCollisionBody m70_collisionBody;
    s16 mD4_cursorX;
    s16 mD6_cursorY;
    fixedPoint mD8_heightOffset;
    //size: 0xE8
};

sTownObject* createTownDragon(p_workArea parent, sSaturnPtr arg);
