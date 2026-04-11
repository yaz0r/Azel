#include "PDS.h"
#include "a7_envEntity54Particles.h"
#include "o_fld_a7.h"
#include "kernel/vdp1AnimatedQuad.h"
#include "trigo.h"

// Lazy-parsed VDP1 quad template used by the entity54 particle system.
static const std::vector<sVdp1Quad>* a7Entity54_getSpriteQuadList()
{
    static std::vector<sVdp1Quad> s_quadList;
    if (s_quadList.empty())
    {
        s_quadList = initVdp1Quad(gFLD_A7->getSaturnPtr(0x06080CFC));
    }
    return &s_quadList;
}

// Per-particle state — 0x44 bytes.
struct sA7EnvEntity54Particle
{
    sVec3_FP m0_position;    // 0x00 — x/y/z fixed point
    sVec3_FP mC_velocity;    // 0x0C
    s32 m18;                 // 0x18
    s32 m1C_gravity;         // 0x1C — added to velocity Y each frame (mode 1)
    s32 m20;                 // 0x20
    s32 m24_angle;           // 0x24 — orbital angle (mode 1)
    s32 m28_angVel;          // 0x28 — angular velocity accumulator
    s32 m2C_angAccel;        // 0x2C — angular acceleration
    s32 m30_radius;          // 0x30
    s32 m34_radiusDelta;     // 0x34
    u16 m38_lifetime;        // 0x38
    u16 m3A_mode;            // 0x3A — 0 = swirl, 1 = orbital
    sAnimatedQuad m3C_quad;  // 0x3C — particleInitSub state
    // size 0x44
};

// 0x38-byte sub-task work area. Offsets 0x00..0x07 temporarily hold
// s_memoryAreaOutput during init (getMemoryArea writes there so the particle
// loop can read the characterArea), then are reused: m0 becomes a spawn
// state counter and m4 becomes the wind-target. Init explicitly zeros m4/m8
// after the particle loop; m0 is left alone (and the spawn helper self-resets
// it from garbage quickly).
struct sA7EnvEntity54Particles : public s_workAreaTemplate<sA7EnvEntity54Particles>
{
    s32                      m0_spawnState;   // 0x00 (shared with m0_bundle during init)
    s32                      m4_windTargetX;  // 0x04 (shared with m4_characterArea during init)
    s32                      m8_windCurrentX; // 0x08
    sA7EnvEntity54Particle*  mC_particles;    // 0x0C
    u8*                      m10_freeList;    // 0x10
    s32                      m14_freeCount;   // 0x14
    u16                      m18_count;       // 0x18
    u16                      m1A_lifetimeSeed;// 0x1A
    s32                      m1C;             // 0x1C
    s32                      m20_angle;       // 0x20
    s32                      m24;             // 0x24
    s32                      m28_sinScaled;   // 0x28
    s32                      m2C;             // 0x2C
    s32                      m30_cosScaled;   // 0x30
    sVec3_FP*                m34_pCenterPos;  // 0x34
    // size 0x38
};

// Stack-local spawn descriptor passed from the spawner to cfcc/d0ac. Layout
// mirrors the Saturn stack frame {local_30[0..2], puStack_24, uStack_20,
// pad, uStack_18} — 28 bytes.
struct sA7Entity54SpawnArg
{
    s32 m0;                                     // 0x00 — pos.X or angle seed
    s32 m4;                                     // 0x04 — pos.Y (always 0)
    s32 m8;                                     // 0x08 — pos.Z (reused as angle by d0ac)
    const std::vector<sVdp1Quad>* mC_pSprite;   // 0x0C — sprite quad list
    s32 m10_distance;                           // 0x10
    s32 m14_pad;                                // 0x14 — gap
    u16 m18_lifetime;                           // 0x18
};

// 0605cfcc — spawn a particle at an absolute position with random XZ velocity
static void a7EnvEntity54Particles_spawnAtPos_0605cfcc(sA7EnvEntity54Particles* pThis, sA7Entity54SpawnArg* arg)
{
    if (pThis->m14_freeCount < (s32)(s16)pThis->m18_count)
    {
        s32 idx = pThis->m14_freeCount;
        pThis->m14_freeCount = idx + 1;
        u32 slot = pThis->m10_freeList[idx];
        sA7EnvEntity54Particle* p = &pThis->mC_particles[slot];

        p->m0_position.m0_X = fixedPoint(arg->m0);
        p->m0_position.m4_Y = fixedPoint(arg->m4);
        p->m0_position.m8_Z = fixedPoint(arg->m8);
        p->m1C_gravity = 0x51;

        u16 ang = (u16)((u32)randomNumber() >> 16) & 0xFFF;
        p->mC_velocity.m0_X = fixedPoint(MTH_Mul(fixedPoint(0x2000), getSin(ang)).m_value);
        p->mC_velocity.m4_Y = fixedPoint(0);
        p->mC_velocity.m8_Z = fixedPoint(MTH_Mul(fixedPoint(0x2000), getCos(ang)).m_value);

        p->m3C_quad.m0_quad = arg->mC_pSprite;
        p->m38_lifetime = arg->m18_lifetime;
        p->m3A_mode = 0;
    }
}

// 0605d0ac — spawn a particle at a polar offset from the center
static void a7EnvEntity54Particles_spawnPolar_0605d0ac(sA7EnvEntity54Particles* pThis, sA7Entity54SpawnArg* arg)
{
    if (pThis->m14_freeCount < (s32)(s16)pThis->m18_count)
    {
        s32 idx = pThis->m14_freeCount;
        pThis->m14_freeCount = idx + 1;
        u32 slot = pThis->m10_freeList[idx];
        sA7EnvEntity54Particle* p = &pThis->mC_particles[slot];

        p->m24_angle = arg->m10_distance;   // distance stored into angle slot per Ghidra
        p->m28_angVel = 0x2D82D8;
        p->m2C_angAccel = (s32)0xFFFFE2E0;  // -0x1D20
        p->m30_radius = arg->m0;            // angle stored into radius slot
        p->m34_radiusDelta = 0xB33;
        p->m1C_gravity = (s32)((slot & 1) * -0x14 - 0x51);
        p->mC_velocity.m4_Y = fixedPoint(0x2000);
        p->m0_position.m4_Y = fixedPoint(0);

        // Angle source is arg->m8 — stale stack data in practice, preserved
        // literally to match the Saturn binary.
        u16 ang = (u16)((u32)arg->m8 >> 16) & 0xFFF;
        p->m0_position.m0_X = fixedPoint(pThis->m34_pCenterPos->m0_X.m_value
            + MTH_Mul(fixedPoint(arg->m0), getSin(ang)).m_value);
        p->m0_position.m8_Z = fixedPoint(pThis->m34_pCenterPos->m8_Z.m_value
            + MTH_Mul(fixedPoint(arg->m0), getCos(ang)).m_value);

        p->m3C_quad.m0_quad = arg->mC_pSprite;
        p->m38_lifetime = arg->m18_lifetime;
        p->m3A_mode = 1;
    }
}

// 0605d1a4 — round-robin spawner: states 1,2 spawn absolute-position particles;
// states 3,4 spawn polar particles; state 4 resets the counter to 0.
static void a7EnvEntity54Particles_spawner_0605d1a4(sA7EnvEntity54Particles* pThis)
{
    sA7Entity54SpawnArg arg;  // stack-local, contents uninitialized until state fills them

    s32 state = pThis->m0_spawnState + 1;
    pThis->m0_spawnState = state;

    if (state == 1 || state == 2)
    {
        arg.m4 = 0;
        arg.m0 = pThis->m34_pCenterPos->m0_X.m_value + (s32)(randomNumber() & 0xFFFFF) - 0x80000;
        arg.m8 = pThis->m34_pCenterPos->m8_Z.m_value + (s32)(randomNumber() & 0xFFFFF) - 0x80000;
        arg.mC_pSprite = a7Entity54_getSpriteQuadList();
        arg.m18_lifetime = pThis->m1A_lifetimeSeed;
        a7EnvEntity54Particles_spawnAtPos_0605cfcc(pThis, &arg);
    }
    else if (state == 3)
    {
        arg.m0 = (s32)(randomNumber() & 0xFFFF);
        arg.m10_distance = (s32)(randomNumber() & 0xFFFFFFF);
        arg.mC_pSprite = a7Entity54_getSpriteQuadList();
        arg.m18_lifetime = 0xD;
        a7EnvEntity54Particles_spawnPolar_0605d0ac(pThis, &arg);
    }
    else
    {
        if (state == 4)
        {
            arg.m0 = (s32)(randomNumber() & 0xFFFF);
            arg.m10_distance = (s32)(randomNumber() & 0xFFFFFFF);
            arg.mC_pSprite = a7Entity54_getSpriteQuadList();
            arg.m18_lifetime = 0xD;
            a7EnvEntity54Particles_spawnPolar_0605d0ac(pThis, &arg);
        }
        pThis->m0_spawnState = 0;
    }
}

// 0605d2b4 — particle system update
static void a7EnvEntity54Particles_Update_0605d2b4(sA7EnvEntity54Particles* pThis)
{
    sA7EnvEntity54Particle* p = pThis->mC_particles;
    for (s32 i = 0; i < (s16)pThis->m18_count; i++)
    {
        if (p->m38_lifetime != 0)
        {
            u32 subResult = (u32)sGunShotTask_UpdateSub4(&p->m3C_quad);
            bool expired = (subResult & 2) != 0 && --p->m38_lifetime == 0;
            if (!expired)
            {
                if (p->m3A_mode == 0)
                {
                    p->mC_velocity.m4_Y = fixedPoint(p->mC_velocity.m4_Y.m_value + p->m1C_gravity);
                    p->m0_position.m0_X = fixedPoint(p->m0_position.m0_X.m_value + p->mC_velocity.m0_X.m_value);
                    p->m0_position.m4_Y = fixedPoint(p->m0_position.m4_Y.m_value + p->mC_velocity.m4_Y.m_value);
                    p->m0_position.m8_Z = fixedPoint(p->m0_position.m8_Z.m_value + p->mC_velocity.m8_Z.m_value);
                    s32 dx = p->m0_position.m0_X.m_value - pThis->m34_pCenterPos->m0_X.m_value;
                    s32 dz = p->m0_position.m8_Z.m_value - pThis->m34_pCenterPos->m8_Z.m_value;
                    p->m0_position.m0_X = fixedPoint(p->m0_position.m0_X.m_value + (dz >> 4) - (dx >> 5));
                    p->m0_position.m8_Z = fixedPoint(p->m0_position.m8_Z.m_value - (dz >> 5) - (dx >> 4));
                }
                else if (p->m3A_mode == 1)
                {
                    p->mC_velocity.m4_Y = fixedPoint(p->mC_velocity.m4_Y.m_value + p->m1C_gravity);
                    p->m0_position.m4_Y = fixedPoint(p->m0_position.m4_Y.m_value + p->mC_velocity.m4_Y.m_value);
                    p->m28_angVel += p->m2C_angAccel;
                    p->m24_angle += p->m28_angVel;
                    p->m30_radius += p->m34_radiusDelta;
                    u16 ang = (u16)((u32)p->m24_angle >> 16) & 0xFFF;
                    p->m0_position.m0_X = fixedPoint(pThis->m34_pCenterPos->m0_X.m_value
                        + MTH_Mul(fixedPoint(p->m30_radius), getSin(ang)).m_value);
                    p->m0_position.m8_Z = fixedPoint(pThis->m34_pCenterPos->m8_Z.m_value
                        + MTH_Mul(fixedPoint(p->m30_radius), getCos(ang)).m_value);
                }
            }
            else
            {
                // Particle died: push index onto the free list
                pThis->m14_freeCount--;
                pThis->m10_freeList[pThis->m14_freeCount] = (u8)i;
            }
        }
        p++;
    }

    a7EnvEntity54Particles_spawner_0605d1a4(pThis);

    s32 v = pThis->m8_windCurrentX + ((pThis->m4_windTargetX - pThis->m8_windCurrentX) >> 4);
    pThis->m8_windCurrentX = v;
    u16 windAng = (u16)((u32)pThis->m20_angle >> 16) & 0xFFF;
    pThis->m28_sinScaled = MTH_Mul(fixedPoint(v), getSin(windAng)).m_value;
    pThis->m30_cosScaled = MTH_Mul(fixedPoint(pThis->m8_windCurrentX), getCos(windAng)).m_value;
}

// 0605d27e — particle system draw
static void a7EnvEntity54Particles_Draw_0605d27e(sA7EnvEntity54Particles* pThis)
{
    sA7EnvEntity54Particle* p = pThis->mC_particles;
    for (s32 i = pThis->m18_count; i > 0; i--)
    {
        if (p->m38_lifetime != 0)
        {
            drawProjectedParticle(&p->m3C_quad, &p->m0_position);
        }
        p++;
    }
}

// 0605d45a
void a7EnvEntity54Particles_create(p_workArea parent, sA7EnvEntity54ParticlesArg* pArg)
{
    static const sA7EnvEntity54Particles::TypedTaskDefinition definition = {
        nullptr,
        &a7EnvEntity54Particles_Update_0605d2b4,
        &a7EnvEntity54Particles_Draw_0605d27e,
        nullptr,
    };

    sA7EnvEntity54Particles* pTask = createSubTask<sA7EnvEntity54Particles>(parent, &definition);
    if (pTask == nullptr)
    {
        return;
    }

    pTask->m18_count = pArg->m4_count;
    pTask->mC_particles = (sA7EnvEntity54Particle*)allocateHeapForTask(pTask, (u32)pArg->m4_count * sizeof(sA7EnvEntity54Particle));
    pTask->m10_freeList = (u8*)allocateHeapForTask(pTask, pArg->m4_count);

    if (pTask->mC_particles == nullptr || pTask->m10_freeList == nullptr)
    {
        pTask->getTask()->markFinished();
        return;
    }

    pTask->m34_pCenterPos = pArg->m0_pPosition;
    pTask->m1A_lifetimeSeed = pArg->m6_val;

    // The Saturn init calls getMemoryArea(pTask, 4), aliasing m0/m4 as an
    // s_memoryAreaOutput for the duration of the particle loop so particleInitSub
    // can derive vdp1Memory from characterArea. Reuse the same memory layout in C++.
    s_memoryAreaOutput* pArea = reinterpret_cast<s_memoryAreaOutput*>(&pTask->m0_spawnState);
    getMemoryArea(pArea, 4);

    pTask->m14_freeCount = 0;

    u16 vdp1Memory = (u16)(((u32)pArea->m4_characterArea - 0x25C00000) >> 3);
    for (u32 i = 0; i < pTask->m18_count; i++)
    {
        pTask->m10_freeList[i] = (u8)i;
        sA7EnvEntity54Particle* pParticle = &pTask->mC_particles[i];
        particleInitSub(&pParticle->m3C_quad, vdp1Memory, a7Entity54_getSpriteQuadList());
        pParticle->m38_lifetime = 0;
        pParticle->m3A_mode = 0;
    }

    // Repurpose m4/m8 as wind state (leaving m0 holding the bundle ptr garbage,
    // which the spawner helper will reset on its first invocation).
    pTask->m4_windTargetX = 0;
    pTask->m8_windCurrentX = 0;
    pTask->m1C = 0;
    pTask->m20_angle = 0x01C71C71;
    pTask->m24 = 0;
    u16 sinIdx = (u16)((u32)pTask->m20_angle >> 16) & 0xFFF;
    pTask->m28_sinScaled = MTH_Mul(fixedPoint(pTask->m8_windCurrentX), getSin(sinIdx)).m_value;
    pTask->m2C = 0;
    pTask->m30_cosScaled = MTH_Mul(fixedPoint(pTask->m8_windCurrentX), getCos(sinIdx)).m_value;
}
