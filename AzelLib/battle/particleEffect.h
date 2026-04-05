#pragma once

#include "kernel/vdp1AnimatedQuad.h"
void createParticleEffect(npcFileDeleter* pParent, const std::vector<sVdp1Quad>* pVdp1Quad, const sVec3_FP* pPosition, sVec3_FP* pVelocity, sVec3_FP* pAcceleration, s32 scale, const quadColor* gouraudColors, s16 lifetime);
