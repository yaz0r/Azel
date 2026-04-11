#include "PDS.h"
#include "a7_beam.h"
#include "a7_beamChargeWobble.h"
#include "a7_sceneParticle.h"
#include "o_fld_a7.h"
#include "kernel/rayDisplay.h"

// 0x144-byte trail sibling task spawned by a7CellObj2 state 0. Holds a 21-slot
// ring buffer of sVec3_FP positions starting at m8 (which aliases the initial
// trail position). Task definition at FLD_A7::060843e0.

// 8-int alloc descriptor returned by alloc_7buffers: {count, arcLen,
// 3 source-position arrays, 3 natural-cubic-spline second-derivative tables}.
// The "y2" tables are populated by a7Beam_resamplePrecompute_06077488
// and consumed by the runtime spline evaluator a7Beam_evalAxis_060776e0.
struct sA7BeamSplineAlloc
{
    s32  m0_count;
    s32* m4_arcLen;   // cumulative arc-length (normalized)
    s32* m8_srcX;
    s32* mC_srcY;
    s32* m10_srcZ;
    s32* m14_y2X;
    s32* m18_y2Y;
    s32* m1C_y2Z;
};

struct sA7Beam : public s_workAreaTemplateWithArg<sA7Beam, sA7BeamArg*>
{
    s_memoryAreaOutput   m0_memoryArea;      // 0x00..0x07 — bundle + VDP1 character area
    sVec3_FP             m8_ring[21];        // 0x08..0x103 — ring buffer, m8_ring[0] is also the position
    s32                  m104_progress;      // 0x104
    s32                  m108_stepDelta;     // 0x108
    u32                  m10C_paramA;        // 0x10C
    u32                  m110_paramB;        // 0x110
    u32                  m114_paramC;        // 0x114 — Saturn EA: u16 sprite char table
    u32                  m118_paramD;        // 0x118 — Saturn EA: u16 sprite size table
    sA7BeamSplineAlloc m11C_alloc;          // 0x11C — 32 bytes
    u16                  m13C_frameCounter;  // 0x13C
    u8                   m13E_head;          // 0x13E
    u8                   m13F_tail;          // 0x13F
    u8                   m140_flags;         // 0x140 — bit 0: use tail sprite slot 2 instead of 1
    u8                   m141_pad[3];        // 0x141..0x143
    // size 0x144
};

// 0607712c — allocate 7 parallel int arrays of `count` entries into pDesc
static bool a7Beam_allocSpline_0607712c(p_workArea pTask, sA7BeamSplineAlloc* pDesc, s32 count)
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

// 060553bc — state-init: populates the 6-entry srcX/Y/Z keyframe arrays
static void a7Beam_stateInit_060553bc(sA7Beam* pThis, s8 stateIndex)
{
    s32* pX = pThis->m11C_alloc.m8_srcX;
    s32* pY = pThis->m11C_alloc.mC_srcY;
    s32* pZ = pThis->m11C_alloc.m10_srcZ;
    s32 x0 = pThis->m8_ring[0].m0_X.m_value;
    s32 y0 = pThis->m8_ring[0].m4_Y.m_value;
    s32 z0 = pThis->m8_ring[0].m8_Z.m_value;

    if (stateIndex == 0)
    {
        // Rising plume: 6 keyframes with increasing XZ jitter and climbing Y
        pX[0] = x0;                                                pY[0] = y0;              pZ[0] = z0;
        pX[1] = (s32)(randomNumber() & 0xFFFF)  + x0 - 0x8000;     pY[1] = y0 + 0x96000;    pZ[1] = (s32)(randomNumber() & 0xFFFF)  + z0 - 0x8000;
        pX[2] = (s32)(randomNumber() & 0x1FFFF) + x0 - 0x10000;    pY[2] = y0 + 0x12C000;   pZ[2] = (s32)(randomNumber() & 0x1FFFF) + z0 - 0x10000;
        pX[3] = (s32)(randomNumber() & 0x3FFFF) + x0 - 0x20000;    pY[3] = y0 + 0x1C2000;   pZ[3] = (s32)(randomNumber() & 0x3FFFF) + z0 - 0x20000;
        pX[4] = (s32)(randomNumber() & 0x3FFFF) + x0 - 0x20000;    pY[4] = y0 + 0x258000;   pZ[4] = (s32)(randomNumber() & 0x3FFFF) + z0 - 0x20000;
        pX[5] = (s32)(randomNumber() & 0x3FFFF) + x0 - 0x20000;    pY[5] = y0 + 0x2EE000;   pZ[5] = (s32)(randomNumber() & 0x3FFFF) + z0 - 0x20000;
    }
    else if (stateIndex == 1)
    {
        // Clean arc: XZ converge to 0 while Y follows a 0/64/C8/C8/64/19 hump
        pX[0] = x0;              pY[0] = 0;          pZ[0] = z0;
        pX[1] = (x0 * 3) / 4;    pY[1] = 0x64000;    pZ[1] = (z0 * 3) / 4;
        pX[2] = x0 / 2;          pY[2] = 0xC8000;    pZ[2] = z0 / 2;
        pX[3] = x0 / 4;          pY[3] = 0xC8000;    pZ[3] = z0 / 4;
        pX[4] = x0 / 4;          pY[4] = 0x64000;    pZ[4] = z0 / 4;
        pX[5] = 0;               pY[5] = 0x19000;    pZ[5] = 0;
    }
}

// 06077488 — natural cubic spline second-derivative precompute (Thomas algorithm).
// Companion to the splint evaluator a7Beam_evalAxis_060776e0. Uses natural
// boundaries y2[0] = y2[n-1] = 0 and the Saturn-specific factor-of-2 + <<4
// scaling so the resulting y2[] feed directly into the splint formula.
static void a7Beam_resamplePrecompute_06077488(s32 count, s32* arcLen, s32* src, s32* y2)
{
    // Two parallel scratch arrays of `count` entries: d[] then h[].
    s32* scratch = (s32*)allocateHeap((u32)count * 2 * sizeof(s32));
    if (scratch == nullptr)
    {
        return;
    }
    s32* d = scratch;
    s32* h = scratch + count;

    // Natural boundary conditions
    y2[0] = 0;
    y2[count - 1] = 0;

    // Pass 1: compute h[i] and the fixed-point slopes d[i]
    for (s32 i = 0; i < count - 1; i++)
    {
        h[i] = arcLen[i + 1] - arcLen[i];
        d[i] = FP_Div(src[i + 1] - src[i], fixedPoint(h[i] << 4)).m_value;
    }

    // Forward sweep init
    y2[1] = h[1] - d[0];
    d[0]  = (arcLen[2] - arcLen[0]) * 2;

    // Forward sweep (Thomas elimination)
    for (s32 i = 1; i < count - 2; i++)
    {
        s32 t = FP_Div(h[i], fixedPoint(d[i - 1])).m_value;
        y2[i + 1] = (h[i + 1] - h[i]) - MTH_Mul(fixedPoint(y2[i]), fixedPoint(t)).m_value;
        d[i]      = (arcLen[i + 2] - arcLen[i]) * 2 - MTH_Mul(fixedPoint(h[i]), fixedPoint(t)).m_value;
    }

    // Tail: y2[n-2] -= h[n-2]*16 * y2[n-1] (y2[n-1] is zero, but preserved for fidelity)
    s32 tail = MTH_Mul(fixedPoint(h[count - 2]), fixedPoint(y2[count - 1] << 4)).m_value;
    y2[count - 2] -= tail;

    // Back-substitution: y2[k] -= d[k]*16 * y2[k+1] for k = n-2 .. 1
    for (s32 k = count - 2; k > 0; k--)
    {
        s32 backTmp = MTH_Mul(fixedPoint(d[k] << 4), fixedPoint(y2[k + 1])).m_value;
        y2[k] -= backTmp;
    }

    freeHeap(scratch);
}

// 060772a8 — build cumulative arc-length table then resample each axis
static void a7Beam_finalizeSpline_060772a8(sA7BeamSplineAlloc* pAlloc)
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

    a7Beam_resamplePrecompute_06077488(count, arcLen, srcX, pAlloc->m14_y2X);
    a7Beam_resamplePrecompute_06077488(count, arcLen, srcY, pAlloc->m18_y2Y);
    a7Beam_resamplePrecompute_06077488(count, arcLen, srcZ, pAlloc->m1C_y2Z);
}

// 060776e0 — natural cubic spline evaluator (Numerical Recipes "splint"), per axis.
// y2[] holds second derivatives pre-divided by 6 (precomputed by 06077488).
static s32 a7Beam_evalAxis_060776e0(s32 t, s32 count, s32* arcLen, s32* src, s32* y2)
{
    // 1. Linear interval search: smallest i such that arcLen[i+1] > t
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

    // 2. Scale h and (t - arcLen[i]) by 16 for fixed-point precision headroom
    s32 h16 = (arcLen[i + 1] - arcLen[i]) * 16;
    s32 t16 = (t - arcLen[i]) * 16;

    s32* pY2  = &y2[i];
    s32* pSrc = &src[i];

    // 3. Cubic correction accumulator (r8 in the Saturn asm):
    //    T1 = ((y2[i+1]-y2[i]) * t16 / h16 + 3*y2[i]) * t16
    s32 A  = setDividend(pY2[1] - pY2[0], t16, h16);
    s32 T1 = MTH_Mul(fixedPoint(A + 3 * pY2[0]), fixedPoint(t16)).m_value;

    // 4. Linear slope: T1 += (src[i+1] - src[i]) / h16
    T1 += FP_Div(pSrc[1] - pSrc[0], fixedPoint(h16)).m_value;

    // 5. Curvature offset: T3 = (2*y2[i] + y2[i+1]) * h16
    s32 T3 = MTH_Mul(fixedPoint(2 * pY2[0] + pY2[1]), fixedPoint(h16)).m_value;

    // 6. y(t) = src[i] + (T1 - T3) * t16
    s32 T4 = MTH_Mul(fixedPoint(T1 - T3), fixedPoint(t16)).m_value;
    return pSrc[0] + T4;
}

// 06077638 — spline evaluator fanout: writes outX/outY/outZ at progress t
static void a7Beam_evalSpline_06077638(s32 t, s32* pOutX, s32* pOutY, s32* pOutZ, sA7BeamSplineAlloc* pAlloc)
{
    *pOutX = a7Beam_evalAxis_060776e0(t, pAlloc->m0_count, pAlloc->m4_arcLen, pAlloc->m8_srcX,  pAlloc->m14_y2X);
    *pOutY = a7Beam_evalAxis_060776e0(t, pAlloc->m0_count, pAlloc->m4_arcLen, pAlloc->mC_srcY,  pAlloc->m18_y2Y);
    *pOutZ = a7Beam_evalAxis_060776e0(t, pAlloc->m0_count, pAlloc->m4_arcLen, pAlloc->m10_srcZ, pAlloc->m1C_y2Z);
}

// 060557e8 — beam wind-down: advance tail until it catches head, then kill the task
static void a7Beam_UpdateFade_060557e8(sA7Beam* pThis)
{
    pThis->m13F_tail = (u8)performModulo(0x15, (u32)pThis->m13F_tail + 1);
    if (pThis->m13E_head == pThis->m13F_tail)
    {
        pThis->m_DrawMethod = nullptr;
        pThis->getTask()->markFinished();
    }
}

// Build a scene-particle descriptor using the beam's paramB as the Saturn EA of
// the VDP1 quad template, then delegate to the shared spawn function (the Saturn
// address 0605533c is shared between the charge wobble and the beam ring update).
static void a7Beam_spawnParticle_trampoline(sA7Beam* pThis, sVec3_FP* pSlot, u16 rndSeed)
{
    sA7SceneParticleDesc desc = {};
    desc.m8_pQuadList = a7GetOrParseQuadList(gFLD_A7->getSaturnPtr(pThis->m110_paramB));
    a7BeamChargeWobble_spawn_0605533c((p_workArea)pThis, pSlot, fixedPoint((s32)rndSeed), &desc);
}

// 06055704 — trail update: spawns sub-fx and advances the ring buffer
static void a7Beam_Update_06055704(sA7Beam* pThis)
{
    pThis->m13C_frameCounter += 1;

    if ((pThis->m13C_frameCounter & 1) != 0)
    {
        sVec3_FP* pSlot = &pThis->m8_ring[pThis->m13E_head];
        u16 rndSeed = (u16)(randomNumber() & 0xFFFF);
        a7Beam_spawnParticle_trampoline(pThis, pSlot, rndSeed);
    }

    pThis->m13E_head = (u8)performModulo(0x15, (s32)(s8)(pThis->m13E_head + 1));

    pThis->m108_stepDelta += 0x24;
    pThis->m104_progress  += pThis->m108_stepDelta;

    if (pThis->m104_progress >= 0x10000)
    {
        pThis->m104_progress = 0x10000;
        pThis->m_UpdateMethod = &a7Beam_UpdateFade_060557e8;
    }

    sVec3_FP* pHeadSlot = &pThis->m8_ring[pThis->m13E_head];
    a7Beam_evalSpline_06077638(pThis->m104_progress,
        &pHeadSlot->m0_X.m_value, &pHeadSlot->m4_Y.m_value, &pHeadSlot->m8_Z.m_value,
        &pThis->m11C_alloc);

    if (pThis->m13E_head == pThis->m13F_tail)
    {
        pThis->m140_flags |= 1;
        pThis->m13F_tail = (u8)performModulo(0x15, (s32)pThis->m13F_tail + 1);
    }
}

// 0602e136 — see a7_beam.h for the full signature comment.
//
// This mirrors the Saturn function exactly when the caller supplies a sprite
// (cmdsrca != 0): it's essentially `displayRaySegment(line, width, cmdsrca,
// cmdsize, cmdcolr, &neutralGouraud, colorMode)`, and so benefits from the
// existing 3D path (enqueueRaySegment3D) when `gDirectRayRendering` is set.
//
// Legacy C++ callers that still only pass the vertex pair (and therefore
// carry no sprite/width state) go through a colour-only fallback: transform
// both endpoints, reject behind-camera, then either push a BGFX line prim
// via enqueueRayLine3D (direct path) or emit a VDP1 distorted-line command
// bucket-sorted by average view-space Z (Saturn path).
void vdp1EmitRayVertex_0602e136(sVec3_FP* pVerts,
                                s32 width,
                                u16 cmdsrca,
                                u16 cmdsize,
                                u16 cmdcolr,
                                u16 colorMode)
{
    if (cmdsrca != 0)
    {
        // Real Saturn path: textured ray quad via the shared rayDisplay
        // infrastructure. The 6-arg Saturn original has no per-vertex
        // gouraud (CMDGRA is never written), so we hand in a neutral
        // gouraud quadColor to get the same visual result on both the
        // direct-3D and VDP1 branches of displayRaySegment.
        static const quadColor neutralGouraud = { 0x4210, 0x4210, 0x4210, 0x4210 };
        std::array<sVec3_FP, 2> line = { pVerts[0], pVerts[1] };
        displayRaySegment(line, width, cmdsrca, cmdsize, cmdcolr, &neutralGouraud, colorMode);
        return;
    }

    // --- Colour-only fallback (no sprite state available) -----------------

    // World-space → view-space transform for the two endpoints.
    sVec3_FP viewA;
    sVec3_FP viewB;
    transformAndAddVecByCurrentMatrix(&pVerts[0], &viewA);
    transformAndAddVecByCurrentMatrix(&pVerts[1], &viewB);

    // Behind-camera rejection — if either vertex sits at or behind the near
    // plane, drop the whole segment rather than producing garbage coordinates.
    if (viewA.m8_Z.m_value <= 0 || viewB.m8_Z.m_value <= 0)
    {
        return;
    }

    if (gDirectRayRendering)
    {
        // Per-pixel-depth BGFX line primitive with solid cmdcolr at both
        // endpoints (neutral gouraud = no tint offset).
        enqueueRayLine3D(viewA, viewB, cmdcolr, 0x4210, 0x4210);
        return;
    }

    // Perspective divide, matches the pattern used by battleEnemyLifeMeter.
    fixedPoint invZA = FP_Div(0x10000, viewA.m8_Z);
    fixedPoint invZB = FP_Div(0x10000, viewB.m8_Z);
    fixedPoint projAX = MTH_Mul_5_6(graphicEngineStatus.m405C.m18_widthScale,  viewA.m0_X, invZA);
    fixedPoint projAY = MTH_Mul_5_6(graphicEngineStatus.m405C.m1C_heightScale, viewA.m4_Y, invZA);
    fixedPoint projBX = MTH_Mul_5_6(graphicEngineStatus.m405C.m18_widthScale,  viewB.m0_X, invZB);
    fixedPoint projBY = MTH_Mul_5_6(graphicEngineStatus.m405C.m1C_heightScale, viewB.m4_Y, invZB);

    // Average view-space Z for the bucket-sort key.
    fixedPoint avgZ(
        (viewA.m8_Z.m_value + viewB.m8_Z.m_value) / 2);
    fixedPoint depth = avgZ * graphicEngineStatus.m405C.m38_oneOverFarClip;

    s_vdp1Context& ctx = graphicEngineStatus.m14_vdp1Context[0];
    s_vdp1Command& cmd = *ctx.m0_currentVdp1WriteEA;
    cmd.m0_CMDCTRL = 0x1006; // distorted line
    cmd.m4_CMDPMOD = (u16)(colorMode | 0x480); // match FUN_0602d9bc: colorMode | 0x480
    cmd.m6_CMDCOLR = cmdcolr;
    cmd.mC_CMDXA  =  (s16)projAX.getInteger();
    cmd.mE_CMDYA  = -(s16)projAY.getInteger();
    cmd.m10_CMDXB =  (s16)projBX.getInteger();
    cmd.m12_CMDYB = -(s16)projBY.getInteger();
    cmd.m14_CMDXC =  (s16)projBX.getInteger();
    cmd.m16_CMDYC = -(s16)projBY.getInteger();
    cmd.m18_CMDXD =  (s16)projAX.getInteger();
    cmd.m1A_CMDYD = -(s16)projAY.getInteger();
    cmd.m1C_CMDGRA = 0;

    ctx.m20_pCurrentVdp1Packet->m4_bucketTypes = depth.getInteger();
    ctx.m20_pCurrentVdp1Packet->m6_vdp1EA = &cmd;
    ctx.m20_pCurrentVdp1Packet++;
    ctx.m0_currentVdp1WriteEA++;
    ctx.m1C += 1;
    ctx.mC  += 1;
}

// 06055848 — trail draw: walks the ring backward from head to tail.
//
// Saturn constants (pool @ FLD_A7::0605590a):
//   width     = 0x2800
//   cmdcolr   = 0x20A0
//   colorMode = 0x0080
// Per-segment sprite params read as u16s from paramC[X] / paramD[X]:
//   head segment : X = 0
//   middle loop  : X = 2
//   tail segment : X = 2, or 4 when (m140_flags & 1) != 0
// cmdsrca = ((m0_memoryArea.m4_characterArea - 0x25C00000) >> 3) + paramC[X]
// cmdsize = paramD[X]
static void a7Beam_Draw_06055848(sA7Beam* pThis)
{
    sVec3_FP* ring = pThis->m8_ring;

    const s32 width     = 0x2800;
    const u16 cmdcolr   = 0x20A0;
    const u16 colorMode = 0x0080;
    const u16 vdp1Base  = (u16)((pThis->m0_memoryArea.m4_characterArea - 0x25C00000) >> 3);

    auto fetchSpriteParams = [&](u32 byteOffset, u16& cmdsrca, u16& cmdsize)
    {
        // Saturn treats paramC/paramD as Saturn pointers; read each as a u16
        // at the given byte offset inside the FLD_A7 overlay.
        sSaturnPtr pSprite = gFLD_A7->getSaturnPtr(pThis->m114_paramC + byteOffset);
        sSaturnPtr pSize   = gFLD_A7->getSaturnPtr(pThis->m118_paramD + byteOffset);
        cmdsrca = (u16)(vdp1Base + readSaturnU16(pSprite));
        cmdsize = readSaturnU16(pSize);
    };

    // vdp1EmitRayVertex_0602e136 reads TWO consecutive vec3s from its argument,
    // so we keep the (prev, curr) pair adjacent in a 2-element local array.
    sVec3_FP segment[2];
    segment[0] = ring[pThis->m13E_head];
    s8 idx = (s8)(pThis->m13E_head) - 1;
    if (idx < 0) idx += 21;
    segment[1] = ring[idx];
    {
        u16 cmdsrca, cmdsize;
        fetchSpriteParams(0, cmdsrca, cmdsize);
        vdp1EmitRayVertex_0602e136(segment, width, cmdsrca, cmdsize, cmdcolr, colorMode);
    }

    if (idx != (s8)pThis->m13F_tail)
    {
        idx--;
        if (idx < 0) idx += 21;
        while (idx != (s8)pThis->m13F_tail)
        {
            segment[0] = segment[1];
            segment[1] = ring[idx];
            u16 cmdsrca, cmdsize;
            fetchSpriteParams(2, cmdsrca, cmdsize);
            vdp1EmitRayVertex_0602e136(segment, width, cmdsrca, cmdsize, cmdcolr, colorMode);
            idx--;
            if (idx < 0) idx += 21;
        }
        segment[0] = segment[1];
        segment[1] = ring[idx]; // ring[tail] — final reload, matches Saturn
        u16 cmdsrca, cmdsize;
        fetchSpriteParams((pThis->m140_flags & 1) ? 4 : 2, cmdsrca, cmdsize);
        vdp1EmitRayVertex_0602e136(segment, width, cmdsrca, cmdsize, cmdcolr, colorMode);
    }
}

// 0605565c — trail init (task definition at 060843e0, size 0x144)
static void a7Beam_Init_0605565c(sA7Beam* pThis, sA7BeamArg* pArg)
{
    // Saturn's createSiblingTaskWithArg (FUN_06031118) calls `copyTask`
    // which blits the parent task's user area (a7BeamEmitter) into the
    // new sibling, so m0_memoryArea arrives inherited from the emitter.
    // Our C++ createSiblingTaskWithArg uses `new T` and skips that copy,
    // so we grab the same memory area explicitly here (a7BeamEmitter's
    // m0 is filled via getMemoryArea(&m0, 3), matching what we do here).
    getMemoryArea(&pThis->m0_memoryArea, 3);

    pThis->m8_ring[0] = pArg->m0_pos;
    pThis->m104_progress = 0;
    pThis->m108_stepDelta = 0;
    pThis->m13E_head = 0;
    pThis->m13F_tail = 0;
    pThis->m13C_frameCounter = 0;
    pThis->m140_flags = 0;
    pThis->m10C_paramA = pArg->mC_paramA;
    pThis->m110_paramB = pArg->m10_paramB;
    pThis->m114_paramC = pArg->m14_paramC;
    pThis->m118_paramD = pArg->m18_paramD;

    if (!a7Beam_allocSpline_0607712c(pThis, &pThis->m11C_alloc, 6))
    {
        pThis->getTask()->markFinished();
        return;
    }

    a7Beam_stateInit_060553bc(pThis, (s8)pArg->m1C_shape);
    a7Beam_finalizeSpline_060772a8(&pThis->m11C_alloc);
}

// 06055a38 — creator wrapper: task definition @ 060843e0, size 0x144
void a7BeamEmitter_spawnBeam_06055a38(p_workArea parent, sA7BeamArg* pArg)
{
    static const sA7Beam::TypedTaskDefinition definition = {
        &a7Beam_Init_0605565c,
        &a7Beam_Update_06055704,
        &a7Beam_Draw_06055848,
        nullptr,
    };
    createSiblingTaskWithArg<sA7Beam>(parent, pArg, &definition);
}
