#pragma once

void startExitFieldCutscene(p_workArea parent, s_cameraScript* pScript, s32 param, s32 exitIndex, s32 arg0);

// 06073500 (A7) / 06074f34 (A3) — variant that reads camera script from sSaturnPtr
void startCameraScriptCutscene(p_workArea parent, const sSaturnPtr& scriptDataEA,
                                s32 param3, s32 param4, s16 param5);

// 0607ce18 (A5) / 06075224 (A3) — start exit cutscene using current field index
void startExitCutsceneForCurrentField(sSaturnPtr pEffectData, s32 param3, s32 param4, s16 param5);

// 0607cdcc (A5) / 060751d8 (A3) — start exit cutscene with cached cutscene data
void startExitCutsceneCached(sSaturnPtr pEffectData, s32 fieldIndex, s32 fieldParam, s32 exitIndex, s16 arg0);
