#include "PDS.h"
#include "fieldPCMTask.h"

// 0606afbc (A3)
static void fieldPCMTask_Init(sPCMVoiceTask* pThis, sSaturnPtr pPcmNameEA)
{
    getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m4C_PCMTask = pThis;
    pThis->m1D4_buffer = dramAllocate(0x8000);
    pThis->m0 = openPCMFileForStreaming(&pThis->m4, pThis->m1D4_buffer, 0x8000, &pThis->m1BC_metadata, (const char*)getSaturnPtr(pPcmNameEA), (azelCdNumber == 3) ? 18000 : 22050);
}

// 0606b09a (A3)
static void fieldPCMTask_Delete(sPCMVoiceTask* pThis)
{
    stopPCMStreaming(pThis->m0);
    dramFree(pThis->m1D4_buffer);

    if (getFieldTaskPtr()->m8_pSubFieldData)
    {
        getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m4C_PCMTask = nullptr;
    }
}

// 06093960 (A3)
static const sPCMVoiceTask::TypedTaskDefinition fieldPCMTaskDefinition = { &fieldPCMTask_Init, nullptr, &sPCMVoiceTask::Draw, &fieldPCMTask_Delete };

// 0606b114 (A3)
s32 fieldPlayPCM(sSaturnPtr pPcmNameEA)
{
    s_fieldScriptWorkArea* pScript = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE;
    if (findMandatoryFileOnDisc((const char*)getSaturnPtr(pPcmNameEA)) && (pScript->m4C_PCMTask == nullptr))
    {
        createSubTaskWithArg<sPCMVoiceTask>(pScript, pPcmNameEA, &fieldPCMTaskDefinition);
    }
    return 0;
}
