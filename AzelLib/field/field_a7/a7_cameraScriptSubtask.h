#pragma once

#include "field/exitField.h"

// a7StartCameraScript — now calls shared startCameraScriptCutscene (exitField.cpp)
inline void a7StartCameraScript(p_workArea parent, const sSaturnPtr& scriptDataEA,
                                s32 param3, s32 param4, s16 param5)
{
    startCameraScriptCutscene(parent, scriptDataEA, param3, param4, param5);
}
