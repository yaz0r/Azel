#pragma once

#include "task.h"
#include "common.h"

// Arg struct passed from a7BeamEmitter_Update state 0 to the 0x144 beam task.
// Saturn layout: 7 u32 + 1 byte = 0x1D bytes. The caller (cellObj2 beam emitter)
// extracts paramA..D from its own m2C..m3C fields (converting sSaturnPtrs to raw EAs).
struct sA7BeamArg
{
    sVec3_FP m0_pos;     // 0x00 — initial beam spline start position
    u32      mC_paramA;  // 0x0C
    u32      m10_paramB; // 0x10
    u32      m14_paramC; // 0x14
    u32      m18_paramD; // 0x18
    u8       m1C_shape;  // 0x1C — beam shape selector (0 = rising plume, 1 = arc)
};

// 06055a38 — spawn a beam sibling task from the cellObj2 emitter
void a7BeamEmitter_spawnBeam_06055a38(p_workArea parent, sA7BeamArg* pArg);

// 0602e136 — shared pre-overlay VDP1 ray (textured line strip) submitter.
//
// Saturn ABI (6 args — matches displayRaySegment minus the gouraud slot):
//   r4        = sVec3_FP*  pVerts     (pointer to TWO consecutive world-
//                                      space vertices)
//   r5        = s32        width      (ray half-thickness, Saturn units)
//   r6        = u16        cmdsrca    (VDP1 sprite character address)
//   r7        = u16        cmdsize    (VDP1 sprite CMDSIZE)
//   sp+0      = u16        cmdcolr    (VDP1 palette/direct colour)
//   sp+4      = u16        colorMode  (CMDPMOD colour-calc mode bits)
//
// When `cmdsrca != 0` the function delegates to displayRaySegment which
// fans out to either enqueueRaySegment3D (gDirectRayRendering == true) or
// the VDP1 distorted-sprite path. When `cmdsrca == 0` (legacy 1-arg C++
// callers that don't yet carry sprite state) the function falls back to
// the colour-only line path: it transforms the two vertices and pushes a
// BGFX line primitive via enqueueRayLine3D / emits a VDP1 distorted-line
// command bucket-sorted by average view-space Z.
void vdp1EmitRayVertex_0602e136(sVec3_FP* pVerts,
                                s32 width     = 0,
                                u16 cmdsrca   = 0,
                                u16 cmdsize   = 0,
                                u16 cmdcolr   = 0xFFFF,
                                u16 colorMode = 8);
