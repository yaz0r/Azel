#include "PDS.h"
#include "fieldPCMTask.h"
#include "audio/pcmPlayer.h"

// 0606afbc (A3)
void sFieldPCMTask::Init(sFieldPCMTask* pThis, sSaturnPtr pPcmNameEA)
{
    getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m4C_PCMTask = pThis;
    pThis->m_soloudHandle = playPCMFile((const char*)getSaturnPtr(pPcmNameEA), (azelCdNumber == 3) ? 18000 : 22050);
}

// 0606b044 (A3)
void sFieldPCMTask::Draw(sFieldPCMTask* pThis)
{
    if (!gSoloud.isValidVoiceHandle(pThis->m_soloudHandle))
    {
        pThis->getTask()->markFinished();
    }
}

// 0606b09a (A3)
void sFieldPCMTask::Delete(sFieldPCMTask* pThis)
{
    if (gSoloud.isValidVoiceHandle(pThis->m_soloudHandle))
    {
        gSoloud.stop(pThis->m_soloudHandle);
    }

    if (getFieldTaskPtr()->m8_pSubFieldData)
    {
        getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m4C_PCMTask = nullptr;
    }
}

// 0606b114 (A3)
s32 fieldPlayPCM(sSaturnPtr pPcmNameEA)
{
    s_fieldScriptWorkArea* pScript = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE;
    if (findMandatoryFileOnDisc((const char*)getSaturnPtr(pPcmNameEA)) && (pScript->m4C_PCMTask == nullptr))
    {
        createSubTaskWithArg<sFieldPCMTask>(pScript, pPcmNameEA);
    }
    return 0;
}
