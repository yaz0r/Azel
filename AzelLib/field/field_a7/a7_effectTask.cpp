#include "PDS.h"
#include "a7_effectTask.h"

// FLD_A7 particle-trail effect task spawned by a7CreateEffectTask.
// Sibling work area is 0x30 bytes and owns three heap-allocated arrays:
// - particles[count]    (0x18 bytes each: base + current sVec3_FP)
// - u32 arrayA[count]
// - u32 arrayB[count]

struct sA7Particle
{
    sVec3_FP m0_base;     // 0x00
    sVec3_FP mC_current;  // 0x0C
    // size 0x18
};

struct sA7EffectTask : public s_workAreaTemplate<sA7EffectTask>
{
    s32 m0_count;            // 0x00
    s32 m4_tick;             // 0x04
    sA7Particle* m8_particles; // 0x08
    sVec3_FP* mC_pPos;       // 0x0C
    s32 m10_jitter;          // 0x10
    u32* m14_arrayA;         // 0x14
    u32* m18_arrayB;         // 0x18
    s32 m1C_param4;          // 0x1C
    s16 m20_cfg0;            // 0x20
    s16 m22_cfg1;            // 0x22
    s16 m24_cfg2;            // 0x24
    s16 m26_cfg3;            // 0x26
    u32 m28;                 // 0x28
    u8 m2C_flags;            // 0x2C
    // size 0x30
};

// Local spawn descriptor built on the stack by a7CreateEffectTask and
// consumed by a7EffectTask_spawnerCore_06057296.
struct sA7EffectSpawnDesc
{
    s32 m0_count;
    sVec3_FP* m4_pPos;
    s32 m8_jitter;
    s32 mC_randBase;
    u32 m10_randMask;
    sSaturnPtr m14_configEA;
    s32 m1C_param4;
};

// 0606fe10 — signed random in [-range/2, range/2)
static s32 a7SignedRandom_0606fe10(s32 range)
{
    s32 r = randomNumber();
    r = performModulo2(range, r);
    return r - (range >> 1);
}

// Forward declaration — defined after the phase update functions
static s32 a7EffectTask_particleStep_06057054(sA7Particle* pParticle, u32* pArrayA, u32 arrayBLifetime);

// 06057158 — final-stage update: wind down the particle pool by pulling tail slots
// over dead particles, kill the task when m4_tick reaches zero.
static void a7EffectTask_UpdatePhase2_06057158(sA7EffectTask* pThis)
{
    sA7Particle* pParticle = pThis->m8_particles;
    u32* pArrayA = pThis->m14_arrayA;
    u32* pArrayB = pThis->m18_arrayB;

    for (s32 i = pThis->m4_tick; i > 0; i--)
    {
        if (pParticle->m0_base.m4_Y.m_value < 0x258001)
        {
            a7EffectTask_particleStep_06057054(pParticle, pArrayA, *pArrayB);
        }
        else if (pThis->m4_tick == 0)
        {
            pThis->getTask()->markFinished();
        }
        else
        {
            s32 tailIdx = pThis->m4_tick - 1;
            pThis->m4_tick = tailIdx;
            *pParticle = pThis->m8_particles[tailIdx];
            *pArrayA = pThis->m14_arrayA[tailIdx];
            *pArrayB = pThis->m18_arrayB[tailIdx];
        }

        pParticle++;
        pArrayA++;
        pArrayB++;
    }
}

// 0605725c — phase-1 update: full particle loop, swaps to phase 2 once m2C_flags is set
static void a7EffectTask_UpdatePhase1_0605725c(sA7EffectTask* pThis)
{
    if ((s8)pThis->m2C_flags != 0)
    {
        pThis->m_UpdateMethod = &a7EffectTask_UpdatePhase2_06057158;
    }

    sA7Particle* pParticle = pThis->m8_particles;
    u32* pArrayA = pThis->m14_arrayA;
    u32* pArrayB = pThis->m18_arrayB;

    for (s32 i = pThis->m4_tick; i > 0; i--)
    {
        if (pParticle->m0_base.m4_Y.m_value < 0x258001)
        {
            a7EffectTask_particleStep_06057054(pParticle, pArrayA, *pArrayB);
        }
        else
        {
            s32 baseX = pThis->mC_pPos->m0_X.m_value;
            s32 jitterX = a7SignedRandom_0606fe10(pThis->m10_jitter);
            pParticle->mC_current.m0_X = fixedPoint(baseX + jitterX);
            pParticle->m0_base.m0_X    = fixedPoint(baseX + jitterX);
            pParticle->mC_current.m4_Y = fixedPoint(0);
            pParticle->m0_base.m4_Y    = fixedPoint(0);

            s32 baseZ = pThis->mC_pPos->m8_Z.m_value;
            s32 jitterZ = a7SignedRandom_0606fe10(pThis->m10_jitter);
            pParticle->mC_current.m8_Z = fixedPoint(baseZ + jitterZ);
            pParticle->m0_base.m8_Z    = fixedPoint(baseZ + jitterZ);

            *pArrayA = 0;

            if (pThis->m1C_param4 != 0)
            {
                auto callback = (void(*)(sA7EffectTask*, sA7Particle*))(uintptr_t)pThis->m1C_param4;
                callback(pThis, pParticle);
            }
        }

        pParticle++;
        pArrayA++;
        pArrayB++;
    }
}

// 06057054 — per-particle physics step (Y accumulator + velocity ramp)
static s32 a7EffectTask_particleStep_06057054(sA7Particle* pParticle, u32* pArrayA, u32 arrayBLifetime)
{
    if (*pArrayA < arrayBLifetime)
    {
        *pArrayA += 0x800;
    }
    s32 baseY = pParticle->m0_base.m4_Y.m_value;
    if (baseY < 0x50000)
    {
        pParticle->m0_base.m4_Y = fixedPoint(baseY + (s32)*pArrayA);
        return pParticle->m0_base.m4_Y.m_value;
    }
    baseY += (s32)*pArrayA;
    pParticle->m0_base.m4_Y = fixedPoint(baseY);
    pParticle->mC_current.m4_Y = fixedPoint(pParticle->mC_current.m4_Y.m_value + (s32)*pArrayA);
    return baseY;
}

// 06057270 — main particle update
static void a7EffectTask_Update_06057270(sA7EffectTask* pThis)
{
    pThis->m28 = pThis->m28 + 1;
    if ((pThis->m28 & 3) == 0)
    {
        if (pThis->m4_tick < pThis->m0_count)
        {
            pThis->m4_tick++;
        }
        else
        {
            pThis->m_UpdateMethod = &a7EffectTask_UpdatePhase1_0605725c;
        }
    }

    sA7Particle* pParticle = pThis->m8_particles;
    u32* pArrayA = pThis->m14_arrayA;
    u32* pArrayB = pThis->m18_arrayB;

    for (s32 i = pThis->m4_tick; i > 0; i--)
    {
        if (pParticle->m0_base.m4_Y.m_value < 0x258001)
        {
            a7EffectTask_particleStep_06057054(pParticle, pArrayA, *pArrayB);
        }
        else
        {
            s32 baseX = pThis->mC_pPos->m0_X.m_value;
            s32 jitterX = a7SignedRandom_0606fe10(pThis->m10_jitter);
            pParticle->mC_current.m0_X = fixedPoint(baseX + jitterX);
            pParticle->m0_base.m0_X    = fixedPoint(baseX + jitterX);
            pParticle->mC_current.m4_Y = fixedPoint(0);
            pParticle->m0_base.m4_Y    = fixedPoint(0);

            s32 baseZ = pThis->mC_pPos->m8_Z.m_value;
            s32 jitterZ = a7SignedRandom_0606fe10(pThis->m10_jitter);
            pParticle->mC_current.m8_Z = fixedPoint(baseZ + jitterZ);
            pParticle->m0_base.m8_Z    = fixedPoint(baseZ + jitterZ);

            *pArrayA = 0;

            if (pThis->m1C_param4 != 0)
            {
                auto callback = (void(*)(sA7EffectTask*, sA7Particle*))(uintptr_t)pThis->m1C_param4;
                callback(pThis, pParticle);
            }
        }

        pParticle++;
        pArrayA++;
        pArrayB++;
    }
}

// 06056e7c — draw a trail segment from a transformed base vec3 with color/avgZ/cfg
static void a7EffectTask_drawSegment_06056e7c(sVec3_FP* pBaseView, u16 color, s32 avgZ, s16* pCfg)
{
    Unimplemented();
}

// 06056ff8 — iterate particles and submit a trail segment per particle
static void a7EffectTask_Draw_06056ff8(sA7EffectTask* pThis)
{
    for (s32 i = 0; i < pThis->m0_count; i++)
    {
        sA7Particle* pParticle = &pThis->m8_particles[i];

        sVec3_FP baseView;
        sVec3_FP currentView;
        transformAndAddVecByCurrentMatrix(&pParticle->m0_base, &baseView);
        transformAndAddVecByCurrentMatrix(&pParticle->mC_current, &currentView);

        s32 avgZ = (baseView.m8_Z.m_value + currentView.m8_Z.m_value) >> 1;
        a7EffectTask_drawSegment_06056e7c(&baseView, 0xAD42, avgZ, &pThis->m20_cfg0);
    }
}

// 06057296 — spawner core
static sA7EffectTask* a7EffectTask_spawnerCore_06057296(p_workArea parent, sA7EffectSpawnDesc* desc)
{
    sA7EffectTask* pTask = createSiblingTaskFromFunction<sA7EffectTask>(parent, nullptr);
    if (pTask == nullptr)
    {
        return nullptr;
    }

    pTask->m0_count = desc->m0_count;
    pTask->m8_particles = (sA7Particle*)allocateHeapForTask(pTask, desc->m0_count * sizeof(sA7Particle));
    pTask->m14_arrayA = (u32*)allocateHeapForTask(pTask, desc->m0_count * sizeof(u32));
    pTask->m18_arrayB = (u32*)allocateHeapForTask(pTask, desc->m0_count * sizeof(u32));

    if (pTask->m8_particles == nullptr
        || pTask->m14_arrayA == nullptr
        || pTask->m18_arrayB == nullptr)
    {
        pTask->getTask()->markFinished();
    }
    else
    {
        pTask->mC_pPos = desc->m4_pPos;
        pTask->m10_jitter = desc->m8_jitter;

        pTask->m20_cfg0 = readSaturnS16(desc->m14_configEA + 0);
        pTask->m22_cfg1 = readSaturnS16(desc->m14_configEA + 2);
        pTask->m24_cfg2 = readSaturnS16(desc->m14_configEA + 4);
        pTask->m26_cfg3 = readSaturnS16(desc->m14_configEA + 6);
        pTask->m1C_param4 = desc->m1C_param4;

        sA7Particle* pParticle = pTask->m8_particles;
        u32* pArrA = pTask->m14_arrayA;
        u32* pArrB = pTask->m18_arrayB;

        for (s32 n = pTask->m0_count; n > 0; n--)
        {
            s32 jitterX = a7SignedRandom_0606fe10(pTask->m10_jitter);
            pParticle->m0_base.m0_X    = fixedPoint(desc->m4_pPos->m0_X.m_value + jitterX);
            pParticle->mC_current.m0_X = fixedPoint(desc->m4_pPos->m0_X.m_value + jitterX);
            pParticle->m0_base.m4_Y    = fixedPoint(0);
            pParticle->mC_current.m4_Y = fixedPoint(0);

            s32 jitterZ = a7SignedRandom_0606fe10(pTask->m10_jitter);
            pParticle->m0_base.m8_Z    = fixedPoint(desc->m4_pPos->m8_Z.m_value + jitterZ);
            pParticle->mC_current.m8_Z = fixedPoint(desc->m4_pPos->m8_Z.m_value + jitterZ);

            *pArrA = 0;
            *pArrB = (u32)((randomNumber() & desc->m10_randMask) + desc->mC_randBase);

            pParticle++;
            pArrA++;
            pArrB++;
        }

        pTask->m4_tick = 0;
        pTask->m2C_flags = 0;
        pTask->m_UpdateMethod = &a7EffectTask_Update_06057270;
        pTask->m_DrawMethod = &a7EffectTask_Draw_06056ff8;
    }

    return pTask;
}

// 060573c6
p_workArea a7CreateEffectTask(p_workArea parent, sVec3_FP* pPos,
                              const sSaturnPtr& configEA, s32 param4)
{
    sA7EffectSpawnDesc desc;
    desc.m0_count = 0x3C;
    desc.m4_pPos = pPos;
    desc.m8_jitter = 0x80000;
    desc.mC_randBase = 0x28000;
    desc.m10_randMask = 0x3FFFF;
    desc.m14_configEA = configEA;
    desc.m1C_param4 = param4;
    return (p_workArea)a7EffectTask_spawnerCore_06057296(parent, &desc);
}
