#include "PDS.h"
#include "a7_beam.h"
#include "field/fieldSpline.h"
#include "a7_beamChargeWobble.h"
#include "o_fld_a7.h"
#include "kernel/rayDisplay.h"

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
    sFieldSplineAlloc m11C_alloc;          // 0x11C — 32 bytes
    u16                  m13C_frameCounter;  // 0x13C
    u8                   m13E_head;          // 0x13E
    u8                   m13F_tail;          // 0x13F
    u8                   m140_flags;         // 0x140 — bit 0: use tail sprite slot 2 instead of 1
    u8                   m141_pad[3];        // 0x141..0x143
    // size 0x144
};

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
    sSceneParticleDesc desc = {};
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
    fieldSpline_eval(pThis->m104_progress,
        &pHeadSlot->m0_X.m_value, &pHeadSlot->m4_Y.m_value, &pHeadSlot->m8_Z.m_value,
        &pThis->m11C_alloc);

    if (pThis->m13E_head == pThis->m13F_tail)
    {
        pThis->m140_flags |= 1;
        pThis->m13F_tail = (u8)performModulo(0x15, (s32)pThis->m13F_tail + 1);
    }
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

    // displayRaySegmentFromWorldSpace reads TWO consecutive vec3s from its argument,
    // so we keep the (prev, curr) pair adjacent in a 2-element local array.
    sVec3_FP segment[2];
    segment[0] = ring[pThis->m13E_head];
    s8 idx = (s8)(pThis->m13E_head) - 1;
    if (idx < 0) idx += 21;
    segment[1] = ring[idx];
    {
        u16 cmdsrca, cmdsize;
        fetchSpriteParams(0, cmdsrca, cmdsize);
        displayRaySegmentFromWorldSpace(segment, width, cmdsrca, cmdsize, cmdcolr, colorMode);
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
            displayRaySegmentFromWorldSpace(segment, width, cmdsrca, cmdsize, cmdcolr, colorMode);
            idx--;
            if (idx < 0) idx += 21;
        }
        segment[0] = segment[1];
        segment[1] = ring[idx]; // ring[tail] — final reload, matches Saturn
        u16 cmdsrca, cmdsize;
        fetchSpriteParams((pThis->m140_flags & 1) ? 4 : 2, cmdsrca, cmdsize);
        displayRaySegmentFromWorldSpace(segment, width, cmdsrca, cmdsize, cmdcolr, colorMode);
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

    if (!fieldSpline_alloc(pThis, &pThis->m11C_alloc, 6))
    {
        pThis->getTask()->markFinished();
        return;
    }

    a7Beam_stateInit_060553bc(pThis, (s8)pArg->m1C_shape);
    fieldSpline_finalize(&pThis->m11C_alloc);
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
