#include "PDS.h"
#include "particleEffect.h"
#include "kernel/graphicalObject.h"
#include "kernel/vdp1Allocator.h"
#include "kernel/vdp1AnimatedQuad.h"

struct sParticleTask : public s_workAreaTemplateWithCopy<sParticleTask> {

    sAnimatedQuad m8;
    sVec3_FP m10;
    sVec3_FP m1C;
    sVec3_FP m28;
    s32 m34;
    std::array<s16, 4> m38;
    s16 m40;
    u8 m42 = 0;
    // size 0x44;
};

void sParticleTask_update0(sParticleTask* pThis) {
    pThis->m1C += pThis->m28;
    pThis->m10 += pThis->m1C;

    switch (pThis->m42) {
    case 0:
        pThis->m42++;
        break;
    case 1:
        if (sGunShotTask_UpdateSub4(&pThis->m8) & 2) {
            pThis->m_DrawMethod = nullptr;
            pThis->getTask()->markFinished();
        }
        break;
    default:
        assert(0);
    }
}

void sParticleTask_update1(sParticleTask* pThis) {
    pThis->m1C += pThis->m28;
    pThis->m10 += pThis->m1C;
    sGunShotTask_UpdateSub4(&pThis->m8);
    if (--pThis->m40 < 0) {
        pThis->getTask()->markFinished();
    }
}

void sParticleTask_drawScaled(sParticleTask* pThis) {
    vdp1DrawQuadScaled(&pThis->m8, &pThis->m10, pThis->m34);
}

void sParticleTask_draw(sParticleTask* pThis) {
    drawProjectedParticle(&pThis->m8, &pThis->m10);
}

void createParticleEffect(npcFileDeleter* param1, const std::vector<sVdp1Quad>* param2, const sVec3_FP* param3, sVec3_FP* param4, sVec3_FP* param5, s32 param6, s32 param7, s16 param8)
{
    sParticleTask* pNewParticleEffect = nullptr;
    if (param8 == 0) {
        static const sParticleTask::TypedTaskDefinition particleTaskDefinition0 =
        {
            nullptr,
            &sParticleTask_update0,
            &sParticleTask_draw,
            nullptr,
        };
        pNewParticleEffect = createSubTaskWithCopy<sParticleTask>(param1, &particleTaskDefinition0);
    }
    else {
        static const sParticleTask::TypedTaskDefinition particleTaskDefinition1 =
        {
            nullptr,
            &sParticleTask_update1,
            &sParticleTask_draw,
            nullptr,
        };
        pNewParticleEffect = createSubTaskWithCopy<sParticleTask>(param1, &particleTaskDefinition1);
    }
    if (pNewParticleEffect == nullptr)
        return;

    pNewParticleEffect->m10 = *param3;
    if (param4) {
        pNewParticleEffect->m1C = *param4;
    }
    if (param5) {
        pNewParticleEffect->m28 = *param5;
    }
    pNewParticleEffect->m34 = param6;
    particleInitSub(&pNewParticleEffect->m8, param1->m4_vd1Allocation->m4_vdp1Memory, param2);

    pNewParticleEffect->m40 = param8;

    if (param8) {
        int count = randomNumber() & 0x1F;
        for (int i = 0; i < count; i++)
        {
            sGunShotTask_UpdateSub4(&pNewParticleEffect->m8);
        }
    }
    if (param7 == 0) {
        if (param6 != 0x10000) {
            pNewParticleEffect->m_DrawMethod = sParticleTask_drawScaled;
        }
    }
    else {
        for (int i = 0; i < 4; i++) {
            assert(0); // param7 is a pointer to something
        }
    }
}
