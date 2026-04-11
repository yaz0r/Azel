#include "PDS.h"
#include "a7_sceneParticle.h"
#include "kernel/vdp1AnimatedQuad.h"

// A7 scene-particle update/draw handlers and allocator.
// These operate on ring-buffer slots of sFieldSceneManagerEntry (0x34 bytes each)
// held in sFieldSceneManager::m8_entries.

// 06076f28 — physics step: pos += vel; then advance the animated quad
s32 a7SceneParticle_UpdatePhysics_06076f28(sFieldSceneManagerEntry* pEntry)
{
    pEntry->m0_position.m0_X = fixedPoint(pEntry->m0_position.m0_X.m_value + pEntry->mC_velocity.m0_X.m_value);
    pEntry->m0_position.m4_Y = fixedPoint(pEntry->m0_position.m4_Y.m_value + pEntry->mC_velocity.m4_Y.m_value);
    pEntry->m0_position.m8_Z = fixedPoint(pEntry->m0_position.m8_Z.m_value + pEntry->mC_velocity.m8_Z.m_value);
    return sGunShotTask_UpdateSub4(&pEntry->m2C_quad) & 2;
}

// 06076fe8 — projected particle draw
static void a7SceneParticle_DrawProjected_06076fe8(sFieldSceneManagerEntry* pEntry)
{
    drawProjectedParticle(&pEntry->m2C_quad, &pEntry->m0_position);
}

// 06076ff6 — billboard particle draw
static void a7SceneParticle_DrawBillboard_06076ff6(sFieldSceneManagerEntry* pEntry)
{
    // TODO: writeBillBoardToVDP1 not yet implemented — fall back to the
    // projected draw (matches the A3 overlay's particleDrawBillboard pattern).
    drawProjectedParticle(&pEntry->m2C_quad, &pEntry->m0_position);
}

// 06077008 — allocate a scene-particle slot
s32 a7SceneParticle_allocate(sFieldSceneManager* pManager, sA7SceneParticleDesc* pDesc, s32 mode)
{
    if (pManager->m18 >= pManager->m14_count)
    {
        return 0;
    }

    void* pPayload = nullptr;
    if (pDesc->m18_payloadSize != 0)
    {
        pPayload = allocateHeapForTask(pManager, pDesc->m18_payloadSize);
        if (pPayload == nullptr)
        {
            return 0;
        }
    }

    // Advance the ring head until a free slot (m28_drawFunc == nullptr) is found
    while (pManager->mC_currentEntry->m28_drawFunc != nullptr)
    {
        pManager->m10 += 1;
        pManager->mC_currentEntry += 1;
        if (pManager->m14_count <= pManager->m10)
        {
            pManager->m10 = 0;
            pManager->mC_currentEntry = pManager->m8_entries;
        }
    }

    sFieldSceneManagerEntry* pSlot = pManager->mC_currentEntry;

    u16 vdp1Memory = (u16)(((u32)pManager->m0_memoryArea.m4_characterArea - 0x25C00000) >> 3);
    particleInitSub(&pSlot->m2C_quad, vdp1Memory, pDesc->m8_pQuadList);

    pSlot->m0_position = *pDesc->m0_pPosition;
    pSlot->mC_velocity = *pDesc->m4_pVelocity;
    pSlot->m24_updateFunc = pDesc->m14_updateFunc;
    pSlot->m20_vdp1Block = pPayload;

    if (pPayload != nullptr)
    {
        memcpy(pPayload, pDesc->m1C_pPayloadSrc, pDesc->m18_payloadSize);
    }

    if (mode == 0)
    {
        pSlot->m28_drawFunc = &a7SceneParticle_DrawProjected_06076fe8;
    }
    else
    {
        pSlot->m18_paramA = pDesc->mC_paramA;
        pSlot->m1C_paramB = pDesc->m10_paramB;
        pSlot->m28_drawFunc = &a7SceneParticle_DrawBillboard_06076ff6;
    }

    pManager->m18 += 1;
    return 1;
}

// 060770e2 — thin wrapper used by a7SpawnedEntity_Init's particle loop
void a7SceneParticle_spawnProjected(sFieldSceneManager* pManager, sA7SceneParticleDesc* pDesc,
                                    sVec3_FP* pPosition, sVec3_FP* pVelocity)
{
    pDesc->m14_updateFunc = &a7SceneParticle_UpdatePhysics_06076f28;
    pDesc->m0_pPosition = pPosition;
    pDesc->m4_pVelocity = pVelocity;
    pDesc->m18_payloadSize = 0;
    a7SceneParticle_allocate(pManager, pDesc, 0);
}
