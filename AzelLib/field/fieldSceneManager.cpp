#include "PDS.h"
#include "fieldSceneManager.h"

// Shared scene manager update — iterates entries, calls per-entry update, cleans up finished ones
static void fieldSceneManager_Update(sFieldSceneManager* pThis)
{
    sFieldSceneManagerEntry* pEntry = pThis->m8_entries;

    // Debug display (only when debug flags active)
    if ((getFieldTaskPtr()->m8_pSubFieldData->m370_fieldDebuggerWho & 2) != 0
        && getFieldTaskPtr()->m8_pSubFieldData->m37E_debugMenuStatus2_a == 0
        && getFieldTaskPtr()->m8_pSubFieldData->m369 == 0)
    {
        if (pThis->m1C < pThis->m18)
            pThis->m1C = pThis->m18;
        vdp2PrintStatus.m10_palette = 0x8000;
        vdp2DebugPrintSetPosition(1, 0x1A);
        vdp2PrintfSmallFont("%03d<%03d ", pThis->m18, pThis->m1C);
    }

    for (s32 i = 0; i < pThis->m14_count; i++)
    {
        if (pEntry->m28_drawFunc != nullptr)
        {
            s32 result = pEntry->m24_updateFunc(pEntry);
            if (result != 0)
            {
                pThis->m18--;
                if (pEntry->m20_vdp1Block != nullptr)
                {
                    pEntry->m20_vdp1Block = nullptr;
                }
                pEntry->m28_drawFunc = nullptr;
            }
        }
        pEntry++;
    }
}

// Shared scene manager draw — iterates entries, calls per-entry draw for active ones
static void fieldSceneManager_Draw(sFieldSceneManager* pThis)
{
    sFieldSceneManagerEntry* pEntry = pThis->m8_entries;
    for (s32 i = 0; i < pThis->m14_count; i++)
    {
        if (pEntry->m28_drawFunc != nullptr)
        {
            pEntry->m28_drawFunc(pEntry);
        }
        pEntry++;
    }
}

// Shared creation function
sFieldSceneManager* createFieldSceneManager(p_workArea parent, s32 areaIndex, s32 count)
{
    sFieldSceneManager* pTask = createSubTaskFromFunction<sFieldSceneManager>(parent, (void(*)(sFieldSceneManager*))nullptr);
    if (pTask != nullptr)
    {
        getMemoryArea(&pTask->m0_memoryArea, areaIndex);
        pTask->m14_count = count;
        sFieldSceneManagerEntry* pEntries = (sFieldSceneManagerEntry*)allocateHeapForTask(pTask, count * sizeof(sFieldSceneManagerEntry));
        pTask->m8_entries = pEntries;
        if (pEntries == nullptr)
        {
            pTask->getTask()->markFinished();
            return nullptr;
        }

        for (s32 i = 0; i < count; i++)
        {
            pEntries[i].m28_drawFunc = nullptr;
        }

        pTask->mC_currentEntry = pTask->m8_entries;
        pTask->m18 = 0;
        pTask->m10 = 0;
        pTask->m1C = 0;
        pTask->m_UpdateMethod = &fieldSceneManager_Update;
        pTask->m_DrawMethod = &fieldSceneManager_Draw;
    }
    return pTask;
}

// ============================================================================
// Scene particle system (shared across all field overlays)
// Moved from field_a7/a7_sceneParticle.cpp
// ============================================================================

// 06076f28 (A7) / 0607895c (A3) — physics step: pos += vel, advance animated quad
s32 sceneParticle_updatePhysics(sFieldSceneManagerEntry* pEntry)
{
    pEntry->m0_position.m0_X = fixedPoint(pEntry->m0_position.m0_X.m_value + pEntry->mC_velocity.m0_X.m_value);
    pEntry->m0_position.m4_Y = fixedPoint(pEntry->m0_position.m4_Y.m_value + pEntry->mC_velocity.m4_Y.m_value);
    pEntry->m0_position.m8_Z = fixedPoint(pEntry->m0_position.m8_Z.m_value + pEntry->mC_velocity.m8_Z.m_value);
    return sGunShotTask_UpdateSub4(&pEntry->m2C_quad) & 2;
}

// 06076fe8 (A7) / 06078a1c (A3) — projected particle draw
static void sceneParticle_drawProjected(sFieldSceneManagerEntry* pEntry)
{
    drawProjectedParticle(&pEntry->m2C_quad, &pEntry->m0_position);
}

// 06076ff6 (A7) / 06078a2a (A3) — billboard particle draw
static void sceneParticle_drawBillboard(sFieldSceneManagerEntry* pEntry)
{
    drawProjectedParticle(&pEntry->m2C_quad, &pEntry->m0_position);
}

// 06077008 (A7) / 06078a3c (A3) — allocate a scene-particle slot
s32 sceneParticle_allocate(sFieldSceneManager* pManager, sSceneParticleDesc* pDesc, s32 mode)
{
    if (pManager->m18 >= pManager->m14_count)
        return 0;

    void* pPayload = nullptr;
    if (pDesc->m18_payloadSize != 0)
    {
        pPayload = allocateHeapForTask(pManager, pDesc->m18_payloadSize);
        if (pPayload == nullptr)
            return 0;
    }

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
        memcpy(pPayload, pDesc->m1C_pPayloadSrc, pDesc->m18_payloadSize);

    if (mode == 0)
        pSlot->m28_drawFunc = &sceneParticle_drawProjected;
    else
    {
        pSlot->m18_paramA = pDesc->mC_paramA;
        pSlot->m1C_paramB = pDesc->m10_paramB;
        pSlot->m28_drawFunc = &sceneParticle_drawBillboard;
    }

    pManager->m18 += 1;
    return 1;
}

// 060770e2 (A7) / 06078b16 (A3) — spawn projected particle
void sceneParticle_spawnProjected(sFieldSceneManager* pManager, sSceneParticleDesc* pDesc,
                                   sVec3_FP* pPosition, sVec3_FP* pVelocity)
{
    pDesc->m14_updateFunc = &sceneParticle_updatePhysics;
    pDesc->m0_pPosition = pPosition;
    pDesc->m4_pVelocity = pVelocity;
    pDesc->m18_payloadSize = 0;
    sceneParticle_allocate(pManager, pDesc, 0);
}
