#pragma once

#include "task.h"
#include "common.h"

struct sA7EnvEntity54ParticlesArg
{
    sVec3_FP* m0_pPosition;
    u16 m4_count;
    u16 m6_val;
};

// 0605d45a — create the 0x38-byte particle sub-task owned by sA7EnvEntity54.
void a7EnvEntity54Particles_create(p_workArea parent, sA7EnvEntity54ParticlesArg* pArg);
