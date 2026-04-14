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
