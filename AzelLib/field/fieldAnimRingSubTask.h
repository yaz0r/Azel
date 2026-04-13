#pragma once

// Animated ring lerp subtask — shared across all field overlays.
// A short-lived task that lerps a position from ring0 to ring1 over a lifetime,
// with optional alternate mode that just counts frames.

struct sAnimRingArg
{
    sVec3_FP m00_center;
    sVec3_FP m0C_ring0;
    sVec3_FP m18_ring1;
    sVec3_FP m24_direction;
    s32      m30_lifetime;
    s32      m34_mode;  // 0 = lerp with draw, 1 = count-only with alt draw
};

void spawnAnimRingSubTask(p_workArea parent, sAnimRingArg* pArg);
