#pragma once

#include "task.h"
#include "common.h"

// 060573c6 — build a 7-dword spawn descriptor (count=0x3C, jitter=0x80000,
// randBase=0x28000, randMask=0x3FFFF) and create the effect sibling task.
p_workArea a7CreateEffectTask(p_workArea parent, sVec3_FP* pPos,
                              const sSaturnPtr& configEA, s32 param4);
