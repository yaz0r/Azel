#pragma once

#include "field.h"
#include "kernel/vdp1AnimatedQuad.h"

struct sParticleSlot
{
    sVec3_FP m0_position;   // 0x00
    sVec3_FP mC_velocity;   // 0x0C
    s32 m18_velocityScaleX;         // 0x18
    s32 m1C_velocityScaleY;         // 0x1C
    void* m20_heapData;   // 0x20
    s32(*m24_updateFunc)(sParticleSlot*);
    void(*m28_drawFunc)(sParticleSlot*);
    sAnimatedQuad m2C_animQuad; // 0x2C — size 8
    // size 0x34
};

struct sParticlePoolManager : public s_workAreaTemplate<sParticlePoolManager>
{
    s_memoryAreaOutput m0_memoryArea;
    sParticleSlot* m8_slotsBase;
    sParticleSlot* mC_currentSlot;
    s32 m10_currentIndex;
    s32 m14_maxParticles;
    s32 m18_activeCount;
    s32 m1C_peakActiveCount;
    // size 0x20
};

sParticlePoolManager* createParticlePoolTask(p_workArea parent, s32 areaIndex, s32 count);

// Particle spawn config, built on the stack by callers
struct sParticleSpawnConfig
{
    sVec3_FP* m0_pPosition;
    sVec3_FP* m4_pVelocity;
    const std::vector<sVdp1Quad>* m8_pQuadData;
    s32 mC_velocityScaleX;
    s32 m10_velocityScaleY;
    s32 (*m14_updateFunc)(sParticleSlot*);
    s32 m18_heapSize;
    void* m1C_heapData;
};


s32 particleUpdateMoving(sParticleSlot* pEntry);
s32 spawnParticleInPool(sParticlePoolManager* pManager, sParticleSpawnConfig* pDesc, s32 mode);
void spawnParticleProjected(sParticlePoolManager* pManager, sParticleSpawnConfig* pDesc,
                                   sVec3_FP* pPosition, sVec3_FP* pVelocity);
