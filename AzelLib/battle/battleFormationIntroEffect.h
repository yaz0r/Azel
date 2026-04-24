#pragma once

// Shared intro-effect task (BTL_X0::06077384 etc.)
//
// Drives in-battle "cutscene + VDP2 text" entries used by enemy attack intros.
// Each intro is described by a table of 12-byte Saturn entries:
//   offset 0 : s16 battleStringIndex (-1 terminates the chain)
//   offset 2 : u16 displayDurationFrames
//   offset 4 : u32 cutsceneFilenamePtr (raw Saturn address, 0 = no cutscene)
//   offset 8 : s16 continuationFlag (0 = last entry, != 0 = chain continues)
//   offset A : u16 padding
//
// The original Saturn task is 0x28 bytes. The C++ mirror uses symbolic member
// names (no enforced byte layout since pointers differ in size on x64).

struct sFormationTaskBase;

#include "town/townCutscene.h"

// 060ba1d0 task definition: {06077214, 00000000, 0607728e, 060772e4}
struct sBattleVoiceOverTask : public s_workAreaTemplateWithArg<sBattleVoiceOverTask, u32>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &Init, nullptr, &Draw, &Delete };
        return &taskDefinition;
    }

    static void Init(sBattleVoiceOverTask* pThis, u32 filenameRaw);
    static void Draw(sBattleVoiceOverTask* pThis);
    static void Delete(sBattleVoiceOverTask* pThis);

    sStreamingFile* m0;              // 0x00
    sStreamingFile m4;               // 0x04 (0x1B8 bytes on Saturn)
    sPCMStreamMetadata m1BC_metadata; // 0x1BC
    u8* m1D4_buffer;                 // 0x1D4
    s32 m1D8_state;                  // 0x1D8
    // Saturn size 0x1DC
};

struct sFormationIntroEffectTask : public s_workAreaTemplateWithArg<sFormationIntroEffectTask, sSaturnPtr>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = {
            &sFormationIntroEffectTask::Init,
            &sFormationIntroEffectTask::Update,
            nullptr,
            &sFormationIntroEffectTask::Delete,
        };
        return &taskDefinition;
    }

    static void Init(sFormationIntroEffectTask* pThis, sSaturnPtr data);
    static void Update(sFormationIntroEffectTask* pThis);
    static void Delete(sFormationIntroEffectTask* pThis);

    // task+0
    u8 m0_state;
    // task+1
    u8 m1_subState;
    // task+2 (init flag; < 0 swaps the update method on Saturn; always 0 at call sites)
    s8 m2_flag;
    // task+3
    u8 m3_entryIndex;
    // task+4
    s32 m4_totalFrames;
    // task+8
    s32 m8_currentFrame;
    // task+C
    s32 mC_remainingFrames;
    // task+0x10
    s16 m10_battleStringIndex;
    // task+0x12
    s16 m12_displayCounter;
    // task+0x14 (raw Saturn filename pointer, or 0 if no cutscene)
    u32 m14_cutsceneFilenameRaw;
    // task+0x18
    s16 m18_continuationFlag;
    // task+0x1A (pad)
    u16 m1A_pad;
    // task+0x1C
    sSaturnPtr m1C_dataPtr;
    // task+0x20 (PCM streaming subtask)
    sBattleVoiceOverTask* m20_cutsceneTask;
    // task+0x24
    s16 m24_savedCounter;
    // task+0x26 (pad)
    u16 m26_pad;
    // Saturn size 0x28
};

p_workArea formationCreateIntroEffectImpl(sFormationTaskBase* pParent, sSaturnPtr data, s8 flag);
