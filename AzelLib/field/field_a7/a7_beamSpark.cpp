#include "PDS.h"
#include "a7_beamSpark.h"
#include "field/fieldBeamSpark.h"

// Beam spark core (init/update/draw/spawn) moved to shared fieldBeamSpark.cpp.
// This file only contains the A7-specific spawn wrapper.

// 06055aea — A7 beam emitter spark spawner (overlay-specific arg setup)
void a7BeamEmitter_spawnSpark_06055aea(p_workArea parent, sVec3_FP* pPos)
{
    sBeamSparkArg arg;
    arg.m0_pos = *pPos;
    arg.mC  = 0;
    arg.m10 = 0;
    arg.m14 = 0;
    arg.m18 = 0x14;
    arg.m1C = 0x800;
    arg.m20 = 0x14CC;
    arg.m24 = 0x4000;
    arg.m28 = (s32)0xFFFFFD9A;
    arg.m2C = 0xFFFF;
    arg.m2E = 0x8421;
    beamSpark_spawn(parent, &arg);
}
