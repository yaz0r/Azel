#include "PDS.h"
#include "pcmVoiceTask.h"

// 06077214 (X0)
void sPCMVoiceTask::Init(sPCMVoiceTask* pThis, sSaturnPtr pFilenameEA)
{
    pThis->m1D4_buffer = dramAllocate(0x8000);
    pThis->m0 = openPCMFileForStreaming(&pThis->m4, pThis->m1D4_buffer, 0x8000, &pThis->m1BC_metadata, (const char*)getSaturnPtr(pFilenameEA), (azelCdNumber == 3) ? 18000 : 22050);
}

// 0607728e (X0), 0606b044 (A3)
void sPCMVoiceTask::Draw(sPCMVoiceTask* pThis)
{
    drawPCMDebugDisplay(pThis->m0);

    s32 state = pThis->m1D8_state;
    if (state == 0)
    {
        scriptFunction_60573d8Sub0(pThis->m0);
        s32 streamState = scriptFunction_605861eSub0Sub0(pThis->m0);
        if (streamState > 1)
        {
            pThis->m1D8_state++;
        }
    }
    else if (state == 1)
    {
        s32 streamState = scriptFunction_605861eSub0Sub0(pThis->m0);
        if (streamState == 5)
        {
            pThis->getTask()->markFinished();
        }
    }
}

// 060772e4 (X0)
void sPCMVoiceTask::Delete(sPCMVoiceTask* pThis)
{
    stopPCMStreaming(pThis->m0);
    dramFree(pThis->m1D4_buffer);
}
