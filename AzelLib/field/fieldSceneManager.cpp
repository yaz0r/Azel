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
