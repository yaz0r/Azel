#pragma once

#include "task.h"
#include "common.h"

// 06073500 — create an A7 camera script subtask. Local A7 overlay copy of
// startExitFieldCutscene with a different script interpretation (uses
// m24_pos2 instead of m0_position for the camera target).
void a7StartCameraScript(p_workArea parent, const sSaturnPtr& scriptDataEA,
                         s32 param3, s32 param4, s16 param5);
