#include "PDS.h"
#include "fieldDialog.h"

struct s_dialogCallbackTask : public s_workAreaTemplate<s_dialogCallbackTask>
{
    static void Init(s_dialogCallbackTask*)
    {
        //empty
    }

    static void Draw(s_dialogCallbackTask* pThis)
    {
        s_fieldScriptWorkArea* pScript = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE;
        getFieldTaskPtr()->m28_status |= 0x40;
        switch (pThis->mC)
        {
        case 0: // wait for multichoice dialog
            if (pScript->m3C_multichoiceTask)
            {
                pThis->mC++;
            }
            break;
        case 1: // wait for multichoice dialog to complete
            if (pScript->m3C_multichoiceTask == nullptr)
            {
                pThis->getTask()->markFinished(); // delete will trigger the callback
            }
            break;
        default:
            assert(0);
        }
    }

    static void Delete(s_dialogCallbackTask* pThis)
    {
        if (getFieldTaskPtr()->m8_pSubFieldData)
        {
            s_fieldScriptWorkArea* pScript = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE;
            getFieldTaskPtr()->m28_status &= ~0x40;

            if (pScript->m54_currentResult > -1)
            {
                if (pThis->m0_callbacks[pScript->m54_currentResult])
                {
                    pThis->m0_callbacks[pScript->m54_currentResult](pThis->m10_entryPointIndex);
                }
            }
        }
    }

    std::array<tDialogCallback, 2> m0_callbacks;
    u32 mC;
    s32 m10_entryPointIndex;
    //size 0x14
};

void registerCallbackForDialog(tDialogCallback callback1, tDialogCallback callback2, s32 r6)
{
    static const s_dialogCallbackTask::TypedTaskDefinition definition = {
        s_dialogCallbackTask::Init,
        nullptr,
        s_dialogCallbackTask::Draw,
        s_dialogCallbackTask::Delete
    };
    s_dialogCallbackTask* pCallbackTask = createSubTask<s_dialogCallbackTask>(getFieldTaskPtr(), &definition);
    pCallbackTask->m0_callbacks[0] = callback1;
    pCallbackTask->m0_callbacks[1] = callback2;
    pCallbackTask->m10_entryPointIndex = r6;
}
