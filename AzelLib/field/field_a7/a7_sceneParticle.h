#pragma once

#include "task.h"
#include "common.h"
#include "field/fieldSceneManager.h"

// 8-dword spawn descriptor for the A7 scene-particle allocator.
struct sA7SceneParticleDesc
{
    sVec3_FP* m0_pPosition;
    sVec3_FP* m4_pVelocity;
    const std::vector<sVdp1Quad>* m8_pQuadList;
    s32 mC_paramA;
    s32 m10_paramB;
    s32 (*m14_updateFunc)(sFieldSceneManagerEntry*);
    s32 m18_payloadSize;
    void* m1C_pPayloadSrc;
};

// 06076f28 — default physics update (pos += vel; step animated quad)
s32 a7SceneParticle_UpdatePhysics_06076f28(sFieldSceneManagerEntry* pEntry);

// 06077008 — allocate a particle slot from the scene manager ring and initialize it.
// Returns 1 on success, 0 on failure (ring full or heap exhausted).
s32 a7SceneParticle_allocate(sFieldSceneManager* pManager, sA7SceneParticleDesc* pDesc, s32 mode);

// 060770e2 — thin wrapper: fill pos/vel/update-fn/payloadSize then spawn in mode 0
// (projected draw). Caller pre-fills desc->m8_pQuadList before calling.
void a7SceneParticle_spawnProjected(sFieldSceneManager* pManager, sA7SceneParticleDesc* pDesc,
                                    sVec3_FP* pPosition, sVec3_FP* pVelocity);
