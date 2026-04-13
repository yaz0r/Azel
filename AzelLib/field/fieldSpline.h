#pragma once

// Natural cubic spline system — shared across all field overlays.
// Used by beam effects, camera scripts, and other path interpolation.

struct sFieldSplineAlloc
{
    s32  m0_count;   // number of keyframes
    s32* m4_arcLen;  // cumulative arc-length (normalized to [0, 0x10000])
    s32* m8_srcX;    // X keyframe positions
    s32* mC_srcY;    // Y keyframe positions
    s32* m10_srcZ;   // Z keyframe positions
    s32* m14_y2X;    // X second derivatives (Thomas algorithm output)
    s32* m18_y2Y;    // Y second derivatives
    s32* m1C_y2Z;    // Z second derivatives
    // size 0x20
};

// 0607712c (A7) / 06078b60 (A3) — allocate 7 parallel int arrays into pDesc
bool fieldSpline_alloc(p_workArea pTask, sFieldSplineAlloc* pDesc, s32 count);

// 060772a8 (A7) / 06078cdc (A3) — build arc-length table and precompute second derivatives
void fieldSpline_finalize(sFieldSplineAlloc* pAlloc);

// 060776e0 (A7) / 06079114 (A3) — evaluate one axis at parameter t
s32 fieldSpline_evalAxis(s32 t, s32 count, s32* arcLen, s32* src, s32* y2);

// 06077638 (A7) / 0607906c (A3) — evaluate all 3 axes at parameter t
void fieldSpline_eval(s32 t, s32* pOutX, s32* pOutY, s32* pOutZ, sFieldSplineAlloc* pAlloc);
