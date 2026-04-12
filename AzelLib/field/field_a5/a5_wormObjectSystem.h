#pragma once

#include "kernel/vdp1AnimatedQuad.h"

// Per-particle record used by the sand/worm particle system.
struct sA5WormParticle
{
    sAnimatedQuad m0_quad;
    sVec3_FP m8_position;
    s32 m14_velocityX;
    s32 m18_velocityY;
    s32 m1C_velocityZ;
    s32 m20_pad0;
    s32 m24_gravityY;
    s32 m28_pad1;
    s32 m2C_orbitAngle;
    s32 m30_orbitAngleSpeed;
    s32 m34_pad2;
    s32 m38_orbitRadius;
    s32 m3C_orbitRadiusSpeed;
    s8 m40_lifetime;
    u8 m41_type;
    u8 m42_pad[2];
    // size 0x44
};

// Worm sand particle pool (Saturn size 0x372C).
struct sA5WormObjectSystem : public s_workAreaTemplate<sA5WormObjectSystem>
{
    s_memoryAreaOutput m0_memoryArea;
    s16 m8_freeIndices[0xC4];
    s16 m190_sortedIndices[0xC4];
    sA5WormParticle m318_particles[0xC4];
    s16 m3728_drawCount;
    // Saturn size 0x372C
};

// Get the worm object system from field-specific data.
// m8_pWormData points to &system.m8_freeIndices[0] (offset 8 into struct).
inline sA5WormObjectSystem* getWormObjectSystem()
{
    s_fieldSpecificData_A5* pFieldData = (s_fieldSpecificData_A5*)getFieldTaskPtr()->mC;
    return reinterpret_cast<sA5WormObjectSystem*>((u8*)pFieldData->m8_pWormData - offsetof(sA5WormObjectSystem, m8_freeIndices));
}
