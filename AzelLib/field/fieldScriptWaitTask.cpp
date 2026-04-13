#include "PDS.h"
#include "fieldScriptWaitTask.h"

// Script-wait task — shared across all field overlays.
// Moved from field_a7/a7_envEntity2C.cpp and field_a5/o_fld_a5.cpp.

static void scriptWaitTask_Init(sScriptWaitTask*) {}

// 06070046 (A7) / 06071a7a (A3)
static void scriptWaitTask_Draw(sScriptWaitTask* pThis)
{
    s_fieldScriptWorkArea* pScript = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE;
    getFieldTaskPtr()->m28_status |= 0x40;

    if (pThis->mC_state == 0)
    {
        if (pScript->m3C_multichoiceTask != nullptr)
            pThis->mC_state++;
    }
    else if (pThis->mC_state == 1)
    {
        if (pScript->m3C_multichoiceTask == nullptr)
            pThis->getTask()->markFinished();
    }
}

// 060700bc (A7) / 06071af0 (A3)
static void scriptWaitTask_Delete(sScriptWaitTask* pThis)
{
    if (getFieldTaskPtr()->m8_pSubFieldData == 0)
        return;

    s_fieldScriptWorkArea* pScript = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE;
    getFieldTaskPtr()->m28_status &= ~0x40;

    s32 result = pScript->m54_currentResult;
    if (result >= 0)
    {
        void(**fnTable)() = (void(**)())&pThis->m4_callback;
        if (fnTable[result] != nullptr)
            fnTable[result]();
    }
}

// 0607011c (A7) / 06071b50 (A3)
sScriptWaitTask* createScriptWaitTask(s32 param1, void(*callback)(), s32 callbackArg)
{
    static sScriptWaitTask::TypedTaskDefinition td = {
        &scriptWaitTask_Init, nullptr, &scriptWaitTask_Draw, &scriptWaitTask_Delete
    };
    sScriptWaitTask* pTask = createSubTask<sScriptWaitTask>(
        (p_workArea)getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE, &td);
    if (pTask)
    {
        pTask->m0 = param1;
        pTask->m4_callback = callback;
        pTask->m10_callbackArg = callbackArg;
    }
    return pTask;
}
