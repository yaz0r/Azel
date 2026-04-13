#include "PDS.h"
#include "fieldSpline.h"

// Natural cubic spline system — shared across all field overlays.
// Moved from field_a7/a7_beam.cpp; byte-identical in A3, A5, B2, C8, etc.

// 0607712c (A7) / 06078b60 (A3) — allocate 7 parallel int arrays
bool fieldSpline_alloc(p_workArea pTask, sFieldSplineAlloc* pDesc, s32 count)
{
    pDesc->m0_count = count;
    s32* pHeap = (s32*)allocateHeapForTask(pTask, (u32)count * 7 * sizeof(s32));
    if (pHeap == nullptr)
    {
        return false;
    }
    pDesc->m4_arcLen = pHeap;
    pDesc->m8_srcX   = pHeap + count;
    pDesc->mC_srcY   = pHeap + count * 2;
    pDesc->m10_srcZ  = pHeap + count * 3;
    pDesc->m14_y2X   = pHeap + count * 4;
    pDesc->m18_y2Y   = pHeap + count * 5;
    pDesc->m1C_y2Z   = pHeap + count * 6;
    return true;
}

// 06077488 (A7) / 06078ebc (A3) — natural cubic spline second-derivative precompute
// (Thomas algorithm). Natural boundaries y2[0] = y2[n-1] = 0.
static void fieldSpline_precompute(s32 count, s32* arcLen, s32* src, s32* y2)
{
    s32* scratch = (s32*)allocateHeap((u32)count * 2 * sizeof(s32));
    if (scratch == nullptr)
    {
        return;
    }
    s32* d = scratch;
    s32* h = scratch + count;

    y2[0] = 0;
    y2[count - 1] = 0;

    for (s32 i = 0; i < count - 1; i++)
    {
        h[i] = arcLen[i + 1] - arcLen[i];
        d[i] = FP_Div(src[i + 1] - src[i], fixedPoint(h[i] << 4)).m_value;
    }

    y2[1] = h[1] - d[0];
    d[0]  = (arcLen[2] - arcLen[0]) * 2;

    for (s32 i = 1; i < count - 2; i++)
    {
        s32 t = FP_Div(h[i], fixedPoint(d[i - 1])).m_value;
        y2[i + 1] = (h[i + 1] - h[i]) - MTH_Mul(fixedPoint(y2[i]), fixedPoint(t)).m_value;
        d[i]      = (arcLen[i + 2] - arcLen[i]) * 2 - MTH_Mul(fixedPoint(h[i]), fixedPoint(t)).m_value;
    }

    s32 tail = MTH_Mul(fixedPoint(h[count - 2]), fixedPoint(y2[count - 1] << 4)).m_value;
    y2[count - 2] -= tail;

    for (s32 k = count - 2; k > 0; k--)
    {
        s32 backTmp = MTH_Mul(fixedPoint(d[k] << 4), fixedPoint(y2[k + 1])).m_value;
        y2[k] -= backTmp;
    }

    freeHeap(scratch);
}

// 060772a8 (A7) / 06078cdc (A3) — build arc-length table then precompute each axis
void fieldSpline_finalize(sFieldSplineAlloc* pAlloc)
{
    s32 count = pAlloc->m0_count;
    s32* arcLen = pAlloc->m4_arcLen;
    s32* srcX = pAlloc->m8_srcX;
    s32* srcY = pAlloc->mC_srcY;
    s32* srcZ = pAlloc->m10_srcZ;

    arcLen[0] = 0;
    for (s32 i = 1; i < count; i++)
    {
        sVec3_FP prev;
        prev.m0_X = fixedPoint(srcX[i - 1]);
        prev.m4_Y = fixedPoint(srcY[i - 1]);
        prev.m8_Z = fixedPoint(srcZ[i - 1]);
        sVec3_FP cur;
        cur.m0_X = fixedPoint(srcX[i]);
        cur.m4_Y = fixedPoint(srcY[i]);
        cur.m8_Z = fixedPoint(srcZ[i]);
        arcLen[i] = arcLen[i - 1] + vecDistance(prev, cur).asS32();
    }

    fixedPoint total = fixedPoint(arcLen[count - 1]);
    for (s32 i = 0; i < count; i++)
    {
        arcLen[i] = FP_Div(arcLen[i], total).m_value;
    }

    fieldSpline_precompute(count, arcLen, srcX, pAlloc->m14_y2X);
    fieldSpline_precompute(count, arcLen, srcY, pAlloc->m18_y2Y);
    fieldSpline_precompute(count, arcLen, srcZ, pAlloc->m1C_y2Z);
}

// 060776e0 (A7) / 06079114 (A3) — natural cubic spline evaluator per axis
s32 fieldSpline_evalAxis(s32 t, s32 count, s32* arcLen, s32* src, s32* y2)
{
    s32 i = 0;
    s32 last = count - 1;
    if (last > 0)
    {
        do
        {
            if (t < arcLen[i + 1]) break;
            i++;
        } while (i < last);
    }

    s32 h16 = (arcLen[i + 1] - arcLen[i]) * 16;
    s32 t16 = (t - arcLen[i]) * 16;

    s32* pY2  = &y2[i];
    s32* pSrc = &src[i];

    s32 A  = setDividend(pY2[1] - pY2[0], t16, h16);
    s32 T1 = MTH_Mul(fixedPoint(A + 3 * pY2[0]), fixedPoint(t16)).m_value;

    T1 += FP_Div(pSrc[1] - pSrc[0], fixedPoint(h16)).m_value;

    s32 T3 = MTH_Mul(fixedPoint(2 * pY2[0] + pY2[1]), fixedPoint(h16)).m_value;

    s32 T4 = MTH_Mul(fixedPoint(T1 - T3), fixedPoint(t16)).m_value;
    return pSrc[0] + T4;
}

// 06077638 (A7) / 0607906c (A3) — evaluate all 3 axes
void fieldSpline_eval(s32 t, s32* pOutX, s32* pOutY, s32* pOutZ, sFieldSplineAlloc* pAlloc)
{
    *pOutX = fieldSpline_evalAxis(t, pAlloc->m0_count, pAlloc->m4_arcLen, pAlloc->m8_srcX,  pAlloc->m14_y2X);
    *pOutY = fieldSpline_evalAxis(t, pAlloc->m0_count, pAlloc->m4_arcLen, pAlloc->mC_srcY,  pAlloc->m18_y2Y);
    *pOutZ = fieldSpline_evalAxis(t, pAlloc->m0_count, pAlloc->m4_arcLen, pAlloc->m10_srcZ, pAlloc->m1C_y2Z);
}
