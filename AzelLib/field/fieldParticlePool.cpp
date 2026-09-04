#include "PDS.h"
#include "fieldParticlePool.h"

// Shared scene manager update — iterates entries, calls per-entry update, cleans up finished ones
static void particlePoolUpdate(sParticlePoolManager* pThis)
{
    sParticleSlot* pEntry = pThis->m8_slotsBase;

    // Debug display (only when debug flags active)
    if ((getFieldTaskPtr()->m8_pSubFieldData->m370_fieldDebuggerWho & 2) != 0
        && getFieldTaskPtr()->m8_pSubFieldData->m37E_debugMenuStatus2_a == 0
        && getFieldTaskPtr()->m8_pSubFieldData->m369 == 0)
    {
        if (pThis->m1C_peakActiveCount < pThis->m18_activeCount)
            pThis->m1C_peakActiveCount = pThis->m18_activeCount;
        vdp2PrintStatus.m10_palette = 0x8000;
        vdp2DebugPrintSetPosition(1, 0x1A);
        vdp2PrintfSmallFont("%03d<%03d ", pThis->m18_activeCount, pThis->m1C_peakActiveCount);
    }

    for (s32 i = 0; i < pThis->m14_maxParticles; i++)
    {
        if (pEntry->m28_drawFunc != nullptr)
        {
            s32 result = pEntry->m24_updateFunc(pEntry);
            if (result != 0)
            {
                pThis->m18_activeCount--;
                if (pEntry->m20_heapData != nullptr)
                {
                    pEntry->m20_heapData = nullptr;
                }
                pEntry->m28_drawFunc = nullptr;
            }
        }
        pEntry++;
    }
}

// Shared scene manager draw — iterates entries, calls per-entry draw for active ones
static void particlePoolDraw(sParticlePoolManager* pThis)
{
    sParticleSlot* pEntry = pThis->m8_slotsBase;
    for (s32 i = 0; i < pThis->m14_maxParticles; i++)
    {
        if (pEntry->m28_drawFunc != nullptr)
        {
            pEntry->m28_drawFunc(pEntry);
        }
        pEntry++;
    }
}

// Shared creation function
sParticlePoolManager* createParticlePoolTask(p_workArea parent, s32 areaIndex, s32 count)
{
    sParticlePoolManager* pTask = createSubTaskFromFunction<sParticlePoolManager>(parent, (void(*)(sParticlePoolManager*))nullptr);
    if (pTask != nullptr)
    {
        getMemoryArea(&pTask->m0_memoryArea, areaIndex);
        pTask->m14_maxParticles = count;
        sParticleSlot* pEntries = (sParticleSlot*)allocateHeapForTask(pTask, count * sizeof(sParticleSlot));
        pTask->m8_slotsBase = pEntries;
        if (pEntries == nullptr)
        {
            pTask->getTask()->markFinished();
            return nullptr;
        }

        for (s32 i = 0; i < count; i++)
        {
            pEntries[i].m28_drawFunc = nullptr;
        }

        pTask->mC_currentSlot = pTask->m8_slotsBase;
        pTask->m18_activeCount = 0;
        pTask->m10_currentIndex = 0;
        pTask->m1C_peakActiveCount = 0;
        pTask->m_UpdateMethod = &particlePoolUpdate;
        pTask->m_DrawMethod = &particlePoolDraw;
    }
    return pTask;
}

// ============================================================================
// Scene particle system (shared across all field overlays)
// Moved from field_a7/a7_sceneParticle.cpp
// ============================================================================

// 06076f28 (A7) / 0607895c (A3) — physics step: pos += vel, advance animated quad
s32 particleUpdateMoving(sParticleSlot* pEntry)
{
    pEntry->m0_position.m0_X = fixedPoint(pEntry->m0_position.m0_X.m_value + pEntry->mC_velocity.m0_X.m_value);
    pEntry->m0_position.m4_Y = fixedPoint(pEntry->m0_position.m4_Y.m_value + pEntry->mC_velocity.m4_Y.m_value);
    pEntry->m0_position.m8_Z = fixedPoint(pEntry->m0_position.m8_Z.m_value + pEntry->mC_velocity.m8_Z.m_value);
    return sGunShotTask_UpdateSub4(&pEntry->m2C_animQuad) & 2;
}

// 06076fe8 (A7) / 06078a1c (A3) — projected particle draw
static void particleDrawSimple(sParticleSlot* pEntry)
{
    drawProjectedParticle(&pEntry->m2C_animQuad, &pEntry->m0_position);
}

// 06076ff6 (A7) / 06078a2a (A3) — billboard particle draw
static void particleDrawBillboard(sParticleSlot* pEntry)
{
    drawProjectedParticle(&pEntry->m2C_animQuad, &pEntry->m0_position);
}

// 06077008 (A7) / 06078a3c (A3) — allocate a scene-particle slot
s32 spawnParticleInPool(sParticlePoolManager* pManager, sParticleSpawnConfig* pDesc, s32 mode)
{
    if (pManager->m18_activeCount >= pManager->m14_maxParticles)
        return 0;

    void* pPayload = nullptr;
    if (pDesc->m18_heapSize != 0)
    {
        pPayload = allocateHeapForTask(pManager, pDesc->m18_heapSize);
        if (pPayload == nullptr)
            return 0;
    }

    while (pManager->mC_currentSlot->m28_drawFunc != nullptr)
    {
        pManager->m10_currentIndex += 1;
        pManager->mC_currentSlot += 1;
        if (pManager->m14_maxParticles <= pManager->m10_currentIndex)
        {
            pManager->m10_currentIndex = 0;
            pManager->mC_currentSlot = pManager->m8_slotsBase;
        }
    }

    sParticleSlot* pSlot = pManager->mC_currentSlot;

    u16 vdp1Memory = (u16)(((u32)pManager->m0_memoryArea.m4_characterArea - 0x25C00000) >> 3);
    particleInitSub(&pSlot->m2C_animQuad, vdp1Memory, pDesc->m8_pQuadData);

    pSlot->m0_position = *pDesc->m0_pPosition;
    pSlot->mC_velocity = *pDesc->m4_pVelocity;
    pSlot->m24_updateFunc = pDesc->m14_updateFunc;
    pSlot->m20_heapData = pPayload;

    if (pPayload != nullptr)
        memcpy(pPayload, pDesc->m1C_heapData, pDesc->m18_heapSize);

    if (mode == 0)
        pSlot->m28_drawFunc = &particleDrawSimple;
    else
    {
        pSlot->m18_velocityScaleX = pDesc->mC_velocityScaleX;
        pSlot->m1C_velocityScaleY = pDesc->m10_velocityScaleY;
        pSlot->m28_drawFunc = &particleDrawBillboard;
    }

    pManager->m18_activeCount += 1;
    return 1;
}

// 060770e2 (A7) / 06078b16 (A3) — spawn projected particle
void spawnParticleProjected(sParticlePoolManager* pManager, sParticleSpawnConfig* pDesc,
                                   sVec3_FP* pPosition, sVec3_FP* pVelocity)
{
    pDesc->m14_updateFunc = &particleUpdateMoving;
    pDesc->m0_pPosition = pPosition;
    pDesc->m4_pVelocity = pVelocity;
    pDesc->m18_heapSize = 0;
    spawnParticleInPool(pManager, pDesc, 0);
}
