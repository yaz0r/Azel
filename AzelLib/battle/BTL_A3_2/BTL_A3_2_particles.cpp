#include "PDS.h"
#include "BTL_A3_2_particles.h"
#include "BTL_A3_2_data.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "kernel/graphicalObject.h"
#include "kernel/vdp1Allocator.h"

// 0607457c
static void battleParticle_updateVariant1(sBattleParticle* pThis)
{
    pThis->m1C_velocity += pThis->m28_acceleration;
    pThis->m10_position += pThis->m1C_velocity;

    if (pThis->m42_state == 0)
    {
        pThis->m42_state = 1;
    }
    else if (pThis->m42_state == 1)
    {
        int result = sGunShotTask_UpdateSub4(&pThis->m8_quad);
        if (result & 2)
        {
            pThis->getTask()->markFinished();
        }
    }
}

// 060745f4
static void battleParticle_updateVariant2(sBattleParticle* pThis)
{
    pThis->m1C_velocity += pThis->m28_acceleration;
    pThis->m10_position += pThis->m1C_velocity;

    sGunShotTask_UpdateSub4(&pThis->m8_quad);

    if (--pThis->m40_lifetime < 0)
    {
        pThis->getTask()->markFinished();
    }
}

// 06074654
static void battleParticle_draw(sBattleParticle* pThis)
{
    drawProjectedParticle(&pThis->m8_quad, &pThis->m10_position);
}

// 06074664
static void battleParticle_drawWithScale(sBattleParticle* pThis)
{
    // TODO: scale support — for now just draw normally
    drawProjectedParticle(&pThis->m8_quad, &pThis->m10_position);
}

// 06074676
static void battleParticle_drawWithGouraud(sBattleParticle* pThis)
{
    drawProjectedParticleWithGouraud(&pThis->m8_quad, &pThis->m10_position, pThis->m38_gouraud.data());
}

// 060746f4
sBattleParticle* createBattleParticle(s_workAreaCopy* parent, const std::vector<sVdp1Quad>* spriteData,
    sVec3_FP* position, sVec3_FP* velocity, sVec3_FP* acceleration,
    fixedPoint scale, quadColor* gouraud, s16 lifetime)
{
    static const sBattleParticle::TypedTaskDefinition defVariant1 = {
        nullptr, &battleParticle_updateVariant1, &battleParticle_draw, nullptr
    };
    static const sBattleParticle::TypedTaskDefinition defVariant2 = {
        nullptr, &battleParticle_updateVariant2, &battleParticle_draw, nullptr
    };

    const auto* def = (lifetime == 0) ? &defVariant1 : &defVariant2;
    sBattleParticle* pThis = createSubTaskWithCopy<sBattleParticle>(parent, def);
    if (!pThis) return nullptr;

    pThis->m10_position = *position;
    if (velocity) pThis->m1C_velocity = *velocity;
    if (acceleration) pThis->m28_acceleration = *acceleration;
    pThis->m34_scale = scale;

    particleInitSub(&pThis->m8_quad, parent->m4_vd1Allocation->m4_vdp1Memory, spriteData);
    pThis->m40_lifetime = lifetime;

    if (lifetime != 0)
    {
        u32 skipFrames = randomNumber() & 0x1F;
        for (u32 i = 0; i < skipFrames; i++)
        {
            sGunShotTask_UpdateSub4(&pThis->m8_quad);
        }
    }

    if (gouraud)
    {
        pThis->m38_gouraud = *gouraud;
        pThis->m_DrawMethod = &battleParticle_drawWithGouraud;
    }
    else if (scale != 0x10000)
    {
        pThis->m_DrawMethod = &battleParticle_drawWithScale;
    }

    return pThis;
}

// 060751b0
void spawnHitSpark(s_workAreaCopy* parent, sVec3_FP* position, sVec3_FP* velocity, fixedPoint scale, s8 type)
{
    int spriteIndex;
    if (type == 0)
    {
        spriteIndex = (gBattleManager->m2_currentBattleOverlayId == 1) ? 3 : 2;
    }
    else if (type == 1 || type != 2)
    {
        spriteIndex = (gBattleManager->m2_currentBattleOverlayId == 1) ? 5 : 4;
    }
    else
    {
        spriteIndex = (gBattleManager->m2_currentBattleOverlayId == 1) ? 8 : 7;
    }

    // Read sprite data pointer from table at 0x060ac830
    sSaturnPtr spriteDataPtr = readSaturnEA(g_BTL_A3_2->getSaturnPtr(0x060ac830) + spriteIndex * 4);
    static std::vector<sVdp1Quad> spriteData;
    spriteData = initVdp1Quad(spriteDataPtr);

    createBattleParticle((s_workAreaCopy*)dramAllocatorEnd[0].mC_fileBundle, &spriteData, position, velocity, nullptr, scale, nullptr, 0);
}
