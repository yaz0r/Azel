#pragma once

// Script-wait task (Saturn size 0x14) — shared across all field overlays.
// Waits for a multichoice dialog to appear then close, dispatches callback on delete.
struct sScriptWaitTask : public s_workAreaTemplate<sScriptWaitTask>
{
    s32 m0;
    void(*m4_callback)();
    s32 m8;
    s32 mC_state;
    s32 m10_callbackArg;
    // size 0x14
};

sScriptWaitTask* createScriptWaitTask(s32 param1, void(*callback)(), s32 callbackArg);
