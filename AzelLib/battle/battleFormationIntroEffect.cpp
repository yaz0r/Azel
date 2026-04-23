#include "PDS.h"
#include "battleFormationIntroEffect.h"
#include "battleFormationBase.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"
#include "battleDebug.h"
#include "town/townCutscene.h"

// 06077384  BTL_X0_createIntroEffect
// Shared in-battle intro/attack-cutscene task.
// param 1: parent workArea
// param 2: sSaturnPtr to a table of 12-byte entries
// param 3: init flag (0 at every known call site)
p_workArea formationCreateIntroEffectImpl(sFormationTaskBase* pParent, sSaturnPtr data, s8 flag)
{
    if (data.m_file == nullptr || data.m_offset == 0)
        return nullptr;

    sFormationIntroEffectTask* pThis = createSubTaskWithArg<sFormationIntroEffectTask>(
        (p_workArea)pParent, data);
    if (pThis == nullptr)
        return nullptr;

    // Saturn sets update to LAB_BTL_X0__06077410 when flag < 0. No known call site uses this.
    // m2_flag is set inside Init (matching original order: flag first, then SetupString overrides).

    return pThis;
}

p_workArea formationCreateIntroEffect(sFormationTaskBase* pThis, sSaturnPtr data)
{
    return formationCreateIntroEffectImpl(pThis, data, 0);
}

// Reads a 12-byte entry from the table at dataPtr + index*12 into the task's
// m10/m14/m18 fields (mirrors the raw 4-byte copies in 06077384 / 06077460).
static void formationIntroEffect_LoadEntry(sFormationIntroEffectTask* pThis, s32 index)
{
    sSaturnPtr entry = pThis->m1C_dataPtr + (u32)(index * 12);
    pThis->m10_battleStringIndex = readSaturnS16(entry + 0);
    pThis->m12_displayCounter = readSaturnS16(entry + 2);
    pThis->m14_cutsceneFilenameRaw = readSaturnU32(entry + 4);
    pThis->m18_continuationFlag = readSaturnS16(entry + 8);
    pThis->m1A_pad = readSaturnU16(entry + 10);
}

// Reads entry[index].battleStringIndex (s16 at entry+0).
static s16 formationIntroEffect_GetEntryStringIndex(sFormationIntroEffectTask* pThis, s32 index)
{
    sSaturnPtr entry = pThis->m1C_dataPtr + (u32)(index * 12);
    return readSaturnS16(entry + 0);
}

// Reads entry[index].continuationFlag (s16 at entry+8).
static s16 formationIntroEffect_GetEntryFlag(sFormationIntroEffectTask* pThis, s32 index)
{
    sSaturnPtr entry = pThis->m1C_dataPtr + (u32)(index * 12);
    return readSaturnS16(entry + 8);
}

// Reads the current battle's VDP2 text for a given string index. The text
// table lives at overlayBattleData[subBattleId].mC_textDisplayData, indexed
// by u32 Saturn pointers to strings.
static std::string formationIntroEffect_GetStringForIndex(s16 stringIndex)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    sOverlayBattleEntry entry = sOverlayBattleEntry::read(
        pEngine->m3A8_overlayBattledata + (s8)pEngine->m3B0_subBattleId * 0x20);
    sSaturnPtr stringPtr = readSaturnEA(entry.mC_textDisplayData + stringIndex * 4);
    return readSaturnString(stringPtr);
}

// 060774fc  formationIntroEffect_ResetFrameCounters
// Marks the frame counters so cutscene progress comparisons never trigger.
static void formationIntroEffect_ResetFrameCounters(sFormationIntroEffectTask* pThis)
{
    pThis->m4_totalFrames = 0x7FFFFFFF;
    pThis->m8_currentFrame = 0;
    pThis->mC_remainingFrames = 0;
}

// 06077214
void sAtolmBattleIntroTask::Init(sAtolmBattleIntroTask* pThis, u32 filenameRaw)
{
    pThis->m1D4_buffer = dramAllocate(0x8000);
    pThis->m1D8_state = 0;

    sSaturnPtr filenamePtr = sSaturnPtr::createFromRaw(filenameRaw, gCurrentBattleOverlay);
    const char* filename = (const char*)getSaturnPtr(filenamePtr);

    s32 sampleRate = (azelCdNumber == 3) ? 18000 : 22050;

    sStreamingFile* psVar1 = openPCMFileForStreaming(&pThis->m4, pThis->m1D4_buffer, 0x8000, &pThis->m1BC_metadata, filename, sampleRate);
    pThis->m0 = psVar1;
}

// 0607728e
void sAtolmBattleIntroTask::Draw(sAtolmBattleIntroTask* pThis)
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

// 060772e4
void sAtolmBattleIntroTask::Delete(sAtolmBattleIntroTask* pThis)
{
    stopPCMStreaming(pThis->m0);
    dramFree(pThis->m1D4_buffer);
}

// 0607750c  formationIntroEffect_SetupString
static void formationIntroEffect_SetupString(sFormationIntroEffectTask* pThis)
{
    // vdp2StringContext.field_0 = 0 in Saturn; not needed for our renderer.
    setupVDP2StringRendering(3, 0x19, 0x26, 4);
    std::string str = formationIntroEffect_GetStringForIndex(pThis->m10_battleStringIndex);
    VDP2DrawString(str.c_str());

    pThis->m24_savedCounter = pThis->m12_displayCounter;

    s_battleDebug* pDebug = gBattleManager->m10_battleOverlay->m10_inBattleDebug;
    if (pThis->m14_cutsceneFilenameRaw == 0 || pDebug->mFlags[5] != 0)
    {
        pThis->m2_flag = 1;
    }
    else
    {
        sAtolmBattleIntroTask* pSubTask = createSubTaskWithArg<sAtolmBattleIntroTask>(
            (p_workArea)pThis, pThis->m14_cutsceneFilenameRaw);
        pThis->m20_cutsceneTask = pSubTask;
        pThis->m2_flag = 2;
        pThis->m4_totalFrames = pSubTask->m1BC_metadata.m10_totalFrames;
        pThis->m8_currentFrame = 0;
        pThis->mC_remainingFrames = 0;
    }
}

// 06077672  formationIntroEffect_TryAdvanceStringMidCutscene
// While a cutscene is actively playing (Update's state 0 branch) this updates
// the displayed VDP2 string at the right points. It is only meaningful when a
// real cutscene task is driving playback.
static void formationIntroEffect_TryAdvanceStringMidCutscene(sFormationIntroEffectTask* pThis)
{
    if (pThis->m1_subState == 0)
    {
        s16 counter = pThis->m12_displayCounter;
        pThis->m12_displayCounter = counter - 1;
        if (counter < 0)
        {
            setupVDP2StringRendering(3, 0x19, 0x26, 4);
            clearVdp2TextArea();
            pThis->m1_subState++;
        }
    }
    else if (pThis->m1_subState == 1)
    {
        // If the next entry's continuation flag (entry[curIdx+1].flag) is 0 we
        // advance to the next string while the cutscene keeps playing.
        sSaturnPtr nextFlagPtr = pThis->m1C_dataPtr + (u32)((pThis->m3_entryIndex + 1) * 12 + 8);
        // Guard against reading past the table: verify the next entry's
        // stringIndex is still valid (>= 0). The original reads raw memory.
        s16 nextFlag = readSaturnS16(nextFlagPtr);
        if (nextFlag == 0)
        {
            pThis->m3_entryIndex++;
            formationIntroEffect_LoadEntry(pThis, pThis->m3_entryIndex);
            setupVDP2StringRendering(3, 0x19, 0x26, 4);
            std::string str = formationIntroEffect_GetStringForIndex(pThis->m10_battleStringIndex);
            VDP2DrawString(str.c_str());
            pThis->m1_subState = 0;
        }
        else
        {
            pThis->m1_subState++;
        }
    }
}

// 06077788  formationIntroEffect_HandleSkipKey
// Debug skip: pressing key 0x87 forces state 2 (next-entry-or-finish) and
// terminates any active cutscene subtask.
static void formationIntroEffect_HandleSkipKey(sFormationIntroEffectTask* pThis)
{
    if (!readKeyboardToggle(0x87))
        return;

    pThis->m0_state = 2;
    pThis->m12_displayCounter = 0;
    setupVDP2StringRendering(3, 0x19, 0x26, 4);
    clearVdp2TextArea();

    if (pThis->m20_cutsceneTask != nullptr)
    {
        // The Saturn helper first probes scriptFunction_605861eSub0Sub0 before
        // tearing down the cutscene. Since cutscene playback itself is not yet
        // wired here, mark the subtask finished directly.
        pThis->m20_cutsceneTask->getTask()->markFinished();
        pThis->m20_cutsceneTask = nullptr;
    }
}

// 06077460  formationIntroEffect_Update
void sFormationIntroEffectTask::Update(sFormationIntroEffectTask* pThis)
{
    formationIntroEffect_HandleSkipKey(pThis);

    switch (pThis->m0_state)
    {
    case 0:
    {
        // Decide whether the cutscene (if any) has produced a display-ready
        // frame. If yes, tick the mid-cutscene string helper and refresh
        // frame counters from the streaming file. Otherwise advance to the
        // display-duration timer (state 1).
        bool cutsceneReady = false;
        if (pThis->m2_flag == 2 && pThis->m20_cutsceneTask != nullptr)
        {
            s_task* pTask = pThis->m20_cutsceneTask->getTask();
            if ((pTask->m14_flags & TASK_FLAGS_FINISHED) == 0)
            {
                s32 scriptState = scriptFunction_605861eSub0Sub0(pThis->m20_cutsceneTask->m0);
                if (scriptState >= 2)
                    cutsceneReady = true;
            }
        }

        if (!cutsceneReady)
        {
            pThis->m20_cutsceneTask = nullptr;
            pThis->m0_state = 1;
            pThis->m12_displayCounter = pThis->m24_savedCounter;
            formationIntroEffect_ResetFrameCounters(pThis);
            break;
        }

        // Cutscene is running and has hit its display-ready milestone.
        if (pThis->m18_continuationFlag == 0 || pThis->m18_continuationFlag == 1)
        {
            if (pThis->m18_continuationFlag == 1)
                pThis->m24_savedCounter = pThis->m18_continuationFlag;
            formationIntroEffect_TryAdvanceStringMidCutscene(pThis);
        }

        s32 frameIndex = (pThis->m20_cutsceneTask != nullptr)
            ? getCutsceneFrameIndex(pThis->m20_cutsceneTask->m0)
            : 0x7FFFFFFF;
        pThis->m8_currentFrame = frameIndex;
        pThis->mC_remainingFrames = pThis->m4_totalFrames - frameIndex;

        if (pThis->m4_totalFrames == pThis->m8_currentFrame)
        {
            pThis->m20_cutsceneTask = nullptr;
            pThis->m0_state = 1;
            pThis->m12_displayCounter = pThis->m24_savedCounter;
            formationIntroEffect_ResetFrameCounters(pThis);
        }
        break;
    }

    case 1:
    {
        s16 counter = pThis->m12_displayCounter;
        pThis->m12_displayCounter = counter - 1;
        if (counter < 0)
        {
            setupVDP2StringRendering(3, 0x19, 0x26, 4);
            clearVdp2TextArea();
            pThis->m0_state++;
        }
        break;
    }

    case 2:
    {
        pThis->m3_entryIndex++;
        s16 nextStringIndex = formationIntroEffect_GetEntryStringIndex(pThis, pThis->m3_entryIndex);
        s16 currentFlag = formationIntroEffect_GetEntryFlag(pThis, pThis->m3_entryIndex);
        if (nextStringIndex >= 0 && currentFlag != 0)
        {
            formationIntroEffect_LoadEntry(pThis, pThis->m3_entryIndex);
            formationIntroEffect_SetupString(pThis);
            pThis->m0_state = 0;
            pThis->m1_subState = 0;
        }
        else
        {
            pThis->m2_flag = -1;
            pThis->getTask()->markFinished();
        }
        break;
    }

    case 3:
        pThis->getTask()->markFinished();
        break;

    default:
        pThis->getTask()->markFinished();
        break;
    }
}

// part of 06077384
void sFormationIntroEffectTask::Init(sFormationIntroEffectTask* pThis, sSaturnPtr data)
{
    pThis->m0_state = 0;
    pThis->m1_subState = 0;
    pThis->m2_flag = 0;
    pThis->m3_entryIndex = 0;
    pThis->m4_totalFrames = 0x7FFFFFFF;
    pThis->m8_currentFrame = 0;
    pThis->mC_remainingFrames = 0;
    pThis->m10_battleStringIndex = 0;
    pThis->m12_displayCounter = 0;
    pThis->m14_cutsceneFilenameRaw = 0;
    pThis->m18_continuationFlag = 0;
    pThis->m1A_pad = 0;
    pThis->m1C_dataPtr = data;
    pThis->m20_cutsceneTask = nullptr;
    pThis->m24_savedCounter = 0;
    pThis->m26_pad = 0;

    // Load entry[0] and render its string (mirrors the final four statements
    // of BTL_X0_createIntroEffect: FUN_BTL_X0__060774fc + FUN_BTL_X0__0607750c).
    formationIntroEffect_LoadEntry(pThis, 0);
    formationIntroEffect_ResetFrameCounters(pThis);
    formationIntroEffect_SetupString(pThis);
}

// 060777DC  formationIntroEffect_Delete
void sFormationIntroEffectTask::Delete(sFormationIntroEffectTask* pThis)
{
    setupVDP2StringRendering(3, 0x19, 0x26, 4);
    clearVdp2TextArea();
}
