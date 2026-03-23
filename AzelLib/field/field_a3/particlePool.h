#pragma once

struct sAnimatedQuad;

// Particle update function: returns non-zero if particle should be freed
typedef s32 (*particleUpdateFunc)(struct sParticleSlot*);
// Particle draw function
typedef void (*particleDrawFunc)(struct sParticleSlot*);

// Particle pool slot (0x34 bytes per particle)
struct sParticleSlot
{
    sVec3_FP m0_position;           // 0x00
    sVec3_FP mC_velocity;           // 0x0C
    fixedPoint m18_velocityScaleX;   // 0x18
    fixedPoint m1C_velocityScaleY;   // 0x1C
    void* m20_heapData;              // 0x20
    particleUpdateFunc m24_updateFunc; // 0x24
    particleDrawFunc m28_drawFunc;   // 0x28 — null = free slot
    sAnimatedQuad m2C_animQuad;      // 0x2C
    // size 0x34
};

// Particle pool manager — this IS the task work area on Saturn (0x20 bytes data)
struct sParticlePoolManager : public s_workAreaTemplate<sParticlePoolManager>
{
    s_workArea* m0_parentTask;       // 0x00 — s_fileBundle* from getMemoryArea
    u32 m4_vdp1Memory;              // 0x04 — character area from getMemoryArea
    sParticleSlot* m8_slotsBase;    // 0x08
    sParticleSlot* mC_currentSlot;  // 0x0C
    s32 m10_currentIndex;           // 0x10
    s32 m14_maxParticles;           // 0x14
    s32 m18_activeCount;            // 0x18
    s32 m1C_peakActiveCount;        // 0x1C
    // size 0x20
};

// Particle spawn config (built on the stack by callers)
struct sParticleSpawnConfig
{
    sVec3_FP* m0_pPosition;
    sVec3_FP* m4_pVelocity;
    const std::vector<sVdp1Quad>* m8_pQuadData;
    fixedPoint mC_velocityScaleX;
    fixedPoint m10_velocityScaleY;
    particleUpdateFunc m14_updateFunc;
    s32 m18_heapSize;
    void* m1C_heapData;
};

s32 spawnParticleInPool(sParticlePoolManager* pPool, sParticleSpawnConfig* pConfig, s32 useVelocityScale);

// 0607895c — particle update: moves by velocity, expires when animation ends
s32 particleUpdateMoving(sParticleSlot* pSlot);
// 0605a032 — particle update: static position, never expires
s32 particleUpdateStatic(sParticleSlot* pSlot);
