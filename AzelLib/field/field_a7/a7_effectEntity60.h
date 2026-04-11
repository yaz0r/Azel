#pragma once

#include "task.h"
#include "fixedPoint.h"
#include "common.h"

// Effect entity spawned as a sibling by a7EnvEntity60 — a 4-position water ring
// animation with a single driven position (others stay at their init Y height).
struct sA7EffectEntity60 : public s_workAreaTemplate<sA7EffectEntity60>
{
    s_memoryAreaOutput m0_memoryArea;  // 0x00 (size 0x08)
    sVec3_FP           m8_pos0;        // 0x08
    sVec3_FP           m14_pos1;       // 0x14
    sVec3_FP           m20_pos2;       // 0x20
    sVec3_FP           m2C_pos3;       // 0x2C
    s32                m38_angleB;     // 0x38 — rotation phase B
    s32                m3C_progress;   // 0x3C — rotation phase A (kill trigger at 0x38E38E3)
    s32                m40_fade;       // 0x40 — ramps up then down
    s32                m44_scaleY;     // 0x44 — Y-axis row scale (init 0x10000)
    s32                m48;            // 0x48
    s32                m4C_animIdx0;   // 0x4C — iterates 0..0x16
    s32                m50_animIdx1;   // 0x50
    s32                m54_animIdx2;   // 0x54
    s32                m58_animIdx3;   // 0x58
    s32                m5C_state;      // 0x5C — 0 = fade-in, 1 = wait, 2 = fade-out
    // size 0x60
};

void a7EffectEntity60_Init(sA7EffectEntity60* pThis);
void a7EffectEntity60_Update(sA7EffectEntity60* pThis);
void a7EffectEntity60_Draw(sA7EffectEntity60* pThis);
