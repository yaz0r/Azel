#include "PDS.h"
#include "o_fld_a5.h"
#include "a5_wormObjectSystem.h"
#include "kernel/fileBundle.h"

// Saturn table at FLD_A5::060997F4 — 4 pointers into the quad-data pool used
// as particle sources. The init picks one at random per slot.
//   [0] 0x06099014
//   [1] 0x060990F4
//   [2] 0x060990F4 (duplicate on purpose — weighted 2/4)
//   [3] 0x060991D4
static constexpr u32 kWormParticleQuadTableAddr = 0x060997F4;

// Cached quad vectors corresponding to the four Saturn entries above.
static std::vector<sVdp1Quad> s_wormParticleQuads[4];

static const std::vector<sVdp1Quad>* a5WormParticleSystem_getQuadList(u32 index)
{
    if (s_wormParticleQuads[index].empty())
    {
        u32 ea = readSaturnU32(gFLD_A5->getSaturnPtr(kWormParticleQuadTableAddr + index * 4));
        s_wormParticleQuads[index] = initVdp1Quad(gFLD_A5->getSaturnPtr(ea));
    }
    return &s_wormParticleQuads[index];
}

// 06058E8E
static void a5WormObjectSystem_Init(sA5WormObjectSystem* p)
{
    getMemoryArea(&p->m0_memoryArea, 1);

    // Expose the in-task particle pool to the field-specific data so the
    // exit entity can push new particles into it.
    s_fieldSpecificData_A5* pFieldData = (s_fieldSpecificData_A5*)getFieldTaskPtr()->mC;
    pFieldData->m8_pWormData = (void*)p->m8_freeIndices;

    p->m3728_drawCount = 0;

    u16 vdp1Memory = (u16)((p->m0_memoryArea.m4_characterArea - 0x25C00000) >> 3);
    for (s32 i = 0; i < 0xC4; i += 2)
    {
        p->m8_freeIndices[i] = (s16)i;
        u32 randIdx0 = randomNumber() & 3;
        particleInitSub(&p->m318_particles[i].m0_quad, vdp1Memory, a5WormParticleSystem_getQuadList(randIdx0));

        p->m8_freeIndices[i + 1] = (s16)(i + 1);
        u32 randIdx1 = randomNumber() & 3;
        particleInitSub(&p->m318_particles[i + 1].m0_quad, vdp1Memory, a5WormParticleSystem_getQuadList(randIdx1));
    }
}

// 060583f0 — mode 0: add exit direction to position
static void a5WormParticle_Clamp_Mode0(sVec3_FP* pPos)
{
    sA5ExitEntity* pExit = ((s_fieldSpecificData_A5*)getFieldTaskPtr()->mC)->m4_pExitEntity;
    pPos->m0_X = fixedPoint(pPos->m0_X.asS32() + pExit->mC_direction.m0_X.asS32());
    pPos->m4_Y = fixedPoint(pPos->m4_Y.asS32() + pExit->mC_direction.m4_Y.asS32());
    pPos->m8_Z = fixedPoint(pPos->m8_Z.asS32() + pExit->mC_direction.m8_Z.asS32());
}

// 0605841c — mode 1: spiral drift around exit target
static void a5WormParticle_Clamp_Mode1(sVec3_FP* pPos)
{
    sA5ExitEntity* pExit = ((s_fieldSpecificData_A5*)getFieldTaskPtr()->mC)->m4_pExitEntity;
    s32 dx = pPos->m0_X.asS32() - pExit->m1C_exitTargetX_mode1;
    s32 dz = pPos->m8_Z.asS32() - pExit->m20_exitTargetZ_mode1;
    pPos->m0_X = fixedPoint(pPos->m0_X.asS32() + ((dz >> 4) - (dx >> 5)));
    pPos->m8_Z = fixedPoint(pPos->m8_Z.asS32() - ((dz >> 5) + (dx >> 4)));
}

// 0605846e — position clamp dispatch (per exit mode)
static void a5WormParticle_ClampPosition(sVec3_FP* pPos)
{
    s32 mode = ((s_fieldSpecificData_A5*)getFieldTaskPtr()->mC)->m4_pExitEntity->m2C_exitMode;
    switch (mode)
    {
    case 0: a5WormParticle_Clamp_Mode0(pPos); break;
    case 1: a5WormParticle_Clamp_Mode1(pPos); break;
    case 2: break; // 06058466 — empty
    case 3: break; // 0605846a — empty
    default: break;
    }
}

// 06058F48 — type 0: linear particle with gravity
static s32 a5WormParticle_Update_Type0(sA5WormParticle* p)
{
    p->m18_velocityY += p->m24_gravityY;
    p->m8_position.m0_X = fixedPoint(p->m8_position.m0_X.asS32() + p->m14_velocityX);
    p->m8_position.m4_Y = fixedPoint(p->m8_position.m4_Y.asS32() + p->m18_velocityY);
    p->m8_position.m8_Z = fixedPoint(p->m8_position.m8_Z.asS32() + p->m1C_velocityZ);
    a5WormParticle_ClampPosition(&p->m8_position);
    return 0;
}

// 06058F9C — type 1: orbiting particle around exit position
static s32 a5WormParticle_Update_Type1(sA5WormParticle* p)
{
    sA5ExitEntity* pExit = ((s_fieldSpecificData_A5*)getFieldTaskPtr()->mC)->m4_pExitEntity;

    p->m18_velocityY += p->m24_gravityY;
    p->m8_position.m4_Y = fixedPoint(p->m8_position.m4_Y.asS32() + p->m18_velocityY);

    p->m30_orbitAngleSpeed += p->m34_pad2;
    p->m2C_orbitAngle += p->m30_orbitAngleSpeed;

    p->m38_orbitRadius += p->m3C_orbitRadiusSpeed;
    u32 angleIdx = ((u32)p->m2C_orbitAngle >> 16) & 0xFFF;
    p->m8_position.m0_X = fixedPoint(MTH_Mul(fixedPoint(p->m38_orbitRadius), getSin(angleIdx)).asS32() + pExit->m1C_exitTargetX_mode1);
    p->m8_position.m8_Z = fixedPoint(MTH_Mul(fixedPoint(p->m38_orbitRadius), getCos(angleIdx)).asS32() + pExit->m20_exitTargetZ_mode1);
    return 0;
}

// 06059016 — type 2: linear particle, removed when below height threshold
static s32 a5WormParticle_Update_Type2(sA5WormParticle* p)
{
    sA5ExitEntity* pExit = ((s_fieldSpecificData_A5*)getFieldTaskPtr()->mC)->m4_pExitEntity;

    p->m8_position.m0_X = fixedPoint(p->m8_position.m0_X.asS32() + p->m14_velocityX);
    p->m8_position.m4_Y = fixedPoint(p->m8_position.m4_Y.asS32() + p->m18_velocityY);
    p->m8_position.m8_Z = fixedPoint(p->m8_position.m8_Z.asS32() + p->m1C_velocityZ);

    if (p->m8_position.m4_Y.asS32() <= pExit->m28_exitTargetZ_mode3)
    {
        return 1; // below threshold — remove
    }
    return 0;
}

typedef s32(*a5WormParticleUpdateFunc)(sA5WormParticle*);
static const a5WormParticleUpdateFunc a5WormParticleUpdateTable[] = {
    &a5WormParticle_Update_Type0,
    &a5WormParticle_Update_Type1,
    &a5WormParticle_Update_Type2,
};

// 06058E40 — remove particle from draw list (swap-remove)
static void a5WormParticle_Remove(sA5WormObjectSystem* pSystem, s32 drawIndex)
{
    if (pSystem->m3728_drawCount > 0)
    {
        pSystem->m3728_drawCount--;
        pSystem->m8_freeIndices[pSystem->m3728_drawCount] = pSystem->m190_sortedIndices[drawIndex];
        pSystem->m190_sortedIndices[drawIndex] = pSystem->m190_sortedIndices[pSystem->m3728_drawCount];
    }
}

// 06059054
static void a5WormObjectSystem_Update(sA5WormObjectSystem* p)
{
    // Debug display
    if ((getFieldTaskPtr()->m8_pSubFieldData->m370_fieldDebuggerWho & 4) != 0
        && getFieldTaskPtr()->m8_pSubFieldData->m37E_debugMenuStatus2_a == 0
        && getFieldTaskPtr()->m8_pSubFieldData->m369 == 0)
    {
        // Debug sand particle count + mode display (skipped)
    }

    for (s32 i = 0; i < p->m3728_drawCount; i++)
    {
        s16 idx = p->m190_sortedIndices[i];
        sA5WormParticle& particle = p->m318_particles[idx];

        // Per-type update dispatch
        s32 result = a5WormParticleUpdateTable[particle.m41_type](&particle);
        if (result != 0)
        {
            a5WormParticle_Remove(p, i);
            i--;
            continue;
        }

        // Advance animation
        s32 animResult = sGunShotTask_UpdateSub4(&particle.m0_quad);
        if ((animResult & 2) != 0)
        {
            particle.m40_lifetime--;
            if (particle.m40_lifetime < 1)
            {
                a5WormParticle_Remove(p, i);
                i--;
            }
        }
    }
}

// 0605918E
static void a5WormObjectSystem_Draw(sA5WormObjectSystem* p)
{
    for (s32 i = 0; i < p->m3728_drawCount; i++)
    {
        s16 idx = p->m190_sortedIndices[i];
        sA5WormParticle& particle = p->m318_particles[idx];
        drawProjectedParticle(&particle.m0_quad, &particle.m8_position);
    }
}

// 060591de
void createA5_wormObjectTask(p_workArea parent)
{
    static sA5WormObjectSystem::TypedTaskDefinition td = { &a5WormObjectSystem_Init, &a5WormObjectSystem_Update, &a5WormObjectSystem_Draw, nullptr };
    createSubTask<sA5WormObjectSystem>(parent, &td);
}
