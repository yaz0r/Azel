#pragma once

#include "kernel/vdp1AnimatedQuad.h"

struct sBattleParticle : public s_workAreaTemplateWithCopy<sBattleParticle>
{
    sAnimatedQuad m8_quad;
    sVec3_FP m10_position;
    sVec3_FP m1C_velocity;
    sVec3_FP m28_acceleration;
    fixedPoint m34_scale;
    quadColor m38_gouraud;
    s16 m40_lifetime;
    u8 m42_state;
    // size 0x44
};

sBattleParticle* createBattleParticle(s_workAreaCopy* parent, const std::vector<sVdp1Quad>* spriteData,
    sVec3_FP* position, sVec3_FP* velocity, sVec3_FP* acceleration,
    fixedPoint scale, quadColor* gouraud, s16 lifetime);

void spawnHitSpark(s_workAreaCopy* parent, sVec3_FP* position, sVec3_FP* velocity, fixedPoint scale, s8 type);
