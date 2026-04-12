#include "PDS.h"
#include "o_fld_a5.h"
#include "a5_wormObjectSystem.h"
#include "field/fieldDragon.h"
#include "trigo.h"

// Dragon sand dust spawner (Saturn size 0x4).
struct sDragonSandDustTask : public s_workAreaTemplate<sDragonSandDustTask>
{
    s32 m0_counter;
    // Saturn size 0x4
};

// Stack-local spawn descriptor matching the Saturn stack layout.
struct sDragonSandDustSpawnDesc
{
    s32 m0_posX;                             // [0]
    s32 m4_posY;                             // [1]
    s32 m8_posZ;                             // [2]
    s32 mC_yawAngle;                         // [3]
    s32 m10_remainingCount;                  // [4]
    s32 m14_lifetime;                        // [5]
    const std::vector<sVdp1Quad>* m18_pQuadList; // [6]
};

// Saturn quad table at FLD_A5::060997F4
static constexpr u32 kDustQuadTableAddr = 0x060997F4;
static std::vector<sVdp1Quad> s_dustQuads[4];

static const std::vector<sVdp1Quad>* getDustQuadList(u32 index)
{
    if (s_dustQuads[index].empty())
    {
        u32 ea = readSaturnU32(gFLD_A5->getSaturnPtr(kDustQuadTableAddr + index * 4));
        s_dustQuads[index] = initVdp1Quad(gFLD_A5->getSaturnPtr(ea));
    }
    return &s_dustQuads[index];
}

// 06058ae0
static void dragonSandDust_spawnParticle(sDragonSandDustSpawnDesc* param_1)
{
    sA5WormObjectSystem* pSystem = getWormObjectSystem();

    param_1->m10_remainingCount++;
    while (param_1->m10_remainingCount != 0 && pSystem->m3728_drawCount < 0xC4)
    {
        s16 drawCount = pSystem->m3728_drawCount;

        // Allocate: copy free index to sorted list
        pSystem->m190_sortedIndices[drawCount] = pSystem->m8_freeIndices[drawCount];

        // Get particle slot
        sA5WormParticle& particle = pSystem->m318_particles[pSystem->m190_sortedIndices[drawCount]];

        particle.m8_position.m0_X = fixedPoint(param_1->m0_posX);
        particle.m8_position.m4_Y = fixedPoint(param_1->m4_posY);
        particle.m8_position.m8_Z = fixedPoint(param_1->m8_posZ);

        u32 uVar3 = randomNumber();
        s32 iVar4 = param_1->mC_yawAngle;

        // Redundant position write (matches Ghidra)
        particle.m8_position.m0_X = fixedPoint(param_1->m0_posX);
        particle.m8_position.m4_Y = fixedPoint(param_1->m4_posY);
        particle.m8_position.m8_Z = fixedPoint(param_1->m8_posZ);

        particle.m24_gravityY = 0x51;

        u16 uVar1 = (u16)(((uVar3 & 0xFFFFFFF) >> 1) + iVar4 - 0x4000000 >> 16) & 0xFFF;
        particle.m14_velocityX = MTH_Mul(fixedPoint(0x1000), getSin(uVar1)).m_value;
        particle.m18_velocityY = 0;
        particle.m1C_velocityZ = MTH_Mul(fixedPoint(0x1000), getCos(uVar1)).m_value;

        particle.m0_quad.m0_quad = param_1->m18_pQuadList;
        particle.m40_lifetime = (s8)param_1->m14_lifetime;
        particle.m41_type = 0;

        param_1->m10_remainingCount--;
        pSystem->m3728_drawCount++;
    }
}

// 060589f4
static void dragonSandDustUpdate(sDragonSandDustTask* pThis)
{
    sVec3_FP dragonPos;
    getDragonPosition(&dragonPos);

    s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    s32 speedVal = pDragon->m154_dragonSpeed.asS32() * 0x1E;
    s16 speedShift = (s16)(speedVal >> 0x11);

    if (dragonPos.m4_Y.asS32() < 0x19000)
    {
        u32 threshold = rotateRightR0ByR1((u32)dragonPos.m4_Y.asS32(), speedShift + 0xC);
        pThis->m0_counter++;
        if ((s32)threshold <= pThis->m0_counter)
        {
            pThis->m0_counter = 0;

            sVec3_FP dragonAngle;
            getDragonAngle(&dragonAngle);

            s32 offset = speedVal >> 3;
            u32 angleIdx = ((u32)dragonAngle.m4_Y.asS32() >> 16) & 0xFFF;
            dragonPos.m0_X = fixedPoint(dragonPos.m0_X.asS32() - MTH_Mul(fixedPoint(offset), getSin(angleIdx)).asS32());
            dragonPos.m8_Z = fixedPoint(dragonPos.m8_Z.asS32() - MTH_Mul(fixedPoint(offset), getCos(angleIdx)).asS32());

            sDragonSandDustSpawnDesc desc;
            desc.m0_posX = dragonPos.m0_X.m_value;
            desc.m4_posY = 0;
            desc.m8_posZ = dragonPos.m8_Z.m_value;
            desc.mC_yawAngle = dragonAngle.m4_Y.asS32();
            desc.m10_remainingCount = speedShift;
            desc.m14_lifetime = 1;

            u32 rng = randomNumber();
            desc.m18_pQuadList = getDustQuadList(rng & 3);

            dragonSandDust_spawnParticle(&desc);
        }
    }
}

// 06058ad8
void createA5_wormDustTask(p_workArea parent)
{
    createSubTaskFromFunction<sDragonSandDustTask>(parent, &dragonSandDustUpdate);
}
