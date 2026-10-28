#pragma once

#include "town/townCutscene.h"

struct sPCMVoiceTask : public s_workAreaTemplateWithArg<sPCMVoiceTask, sSaturnPtr>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &Init, nullptr, &Draw, &Delete };
        return &taskDefinition;
    }

    static void Init(sPCMVoiceTask* pThis, sSaturnPtr pFilenameEA);
    static void Draw(sPCMVoiceTask* pThis);
    static void Delete(sPCMVoiceTask* pThis);

    sStreamingFile* m0;
    sStreamingFile m4; // 0x1B8 bytes on Saturn
    sPCMStreamMetadata m1BC_metadata;
    u8* m1D4_buffer;
    s32 m1D8_state;
    // size 0x1DC
};
