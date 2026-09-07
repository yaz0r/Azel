#pragma once

#include "kernel/vdp1AnimatedQuad.h"

struct sA5SandParticle
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

struct sA5SandParticlePool : public s_workAreaTemplate<sA5SandParticlePool>
{
    s_memoryAreaOutput m0_memoryArea;
    s16 m8_freeIndices[0xC4];
    s16 m190_sortedIndices[0xC4];
    sA5SandParticle m318_particles[0xC4];
    s16 m3728_drawCount;
    // Saturn size 0x372C
};

inline sA5SandParticlePool* getSandParticlePool()
{
    s_fieldSpecificData_A5* pFieldData = (s_fieldSpecificData_A5*)getFieldTaskPtr()->mC;
    return reinterpret_cast<sA5SandParticlePool*>((u8*)pFieldData->m8_pSandParticlePool - offsetof(sA5SandParticlePool, m8_freeIndices));
}
