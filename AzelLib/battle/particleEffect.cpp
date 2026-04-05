#include "PDS.h"
#include "particleEffect.h"
#include "kernel/graphicalObject.h"
#include "kernel/vdp1Allocator.h"
#include "kernel/vdp1AnimatedQuad.h"

struct sParticleTask : public s_workAreaTemplateWithCopy<sParticleTask> {

    sAnimatedQuad m8_quad;
    sVec3_FP m10_position;
    sVec3_FP m1C_velocity;
    sVec3_FP m28_acceleration;
    s32 m34_scale;
    quadColor m38_gouraudColors;
    s16 m40_lifetime;
    u8 m42_state = 0;
    // size 0x44;
};

void sParticleTask_update0(sParticleTask* pThis) {
    pThis->m1C_velocity += pThis->m28_acceleration;
    pThis->m10_position += pThis->m1C_velocity;

    switch (pThis->m42_state) {
    case 0:
        pThis->m42_state++;
        break;
    case 1:
        if (sGunShotTask_UpdateSub4(&pThis->m8_quad) & 2) {
            pThis->m_DrawMethod = nullptr;
            pThis->getTask()->markFinished();
        }
        break;
    default:
        assert(0);
    }
}

void sParticleTask_update1(sParticleTask* pThis) {
    pThis->m1C_velocity += pThis->m28_acceleration;
    pThis->m10_position += pThis->m1C_velocity;
    sGunShotTask_UpdateSub4(&pThis->m8_quad);
    if (--pThis->m40_lifetime < 0) {
        pThis->getTask()->markFinished();
    }
}

void sParticleTask_drawScaled(sParticleTask* pThis) {
    vdp1DrawQuadScaled(&pThis->m8_quad, &pThis->m10_position, pThis->m34_scale);
}

void sParticleTask_draw(sParticleTask* pThis) {
    drawProjectedParticle(&pThis->m8_quad, &pThis->m10_position);
}

// BTL_A3::0607100a
void sParticleTask_drawGouraud(sParticleTask* pThis) {
    drawProjectedParticleWithGouraud(&pThis->m8_quad, &pThis->m10_position, &pThis->m38_gouraudColors);

}

void createParticleEffect(npcFileDeleter* pParent, const std::vector<sVdp1Quad>* pVdp1Quad, const sVec3_FP* pPosition, sVec3_FP* pVelocity, sVec3_FP* pAcceleration, s32 scale, const quadColor* gouraudColors, s16 lifetime)
{
    sParticleTask* pNewParticleEffect = nullptr;
    if (lifetime == 0) {
        static const sParticleTask::TypedTaskDefinition particleTaskDefinition0 =
        {
            nullptr,
            &sParticleTask_update0,
            &sParticleTask_draw,
            nullptr,
        };
        pNewParticleEffect = createSubTaskWithCopy<sParticleTask>(pParent, &particleTaskDefinition0);
    }
    else {
        static const sParticleTask::TypedTaskDefinition particleTaskDefinition1 =
        {
            nullptr,
            &sParticleTask_update1,
            &sParticleTask_draw,
            nullptr,
        };
        pNewParticleEffect = createSubTaskWithCopy<sParticleTask>(pParent, &particleTaskDefinition1);
    }
    if (pNewParticleEffect == nullptr)
        return;

    pNewParticleEffect->m10_position= *pPosition;
    if (pVelocity) {
        pNewParticleEffect->m1C_velocity= *pVelocity;
    }
    if (pAcceleration) {
        pNewParticleEffect->m28_acceleration= *pAcceleration;
    }
    pNewParticleEffect->m34_scale= scale;
    particleInitSub(&pNewParticleEffect->m8_quad, pParent->m4_vd1Allocation->m4_vdp1Memory, pVdp1Quad);

    pNewParticleEffect->m40_lifetime= lifetime;

    if (lifetime) {
        int count = randomNumber() & 0x1F;
        for (int i = 0; i < count; i++)
        {
            sGunShotTask_UpdateSub4(&pNewParticleEffect->m8_quad);
        }
    }
    if (gouraudColors == nullptr) {
        if (scale != 0x10000) {
            pNewParticleEffect->m_DrawMethod = sParticleTask_drawScaled;
        }
    }
    else {
        pNewParticleEffect->m38_gouraudColors = *gouraudColors;
        pNewParticleEffect->m_DrawMethod = sParticleTask_drawGouraud;
    }
}
