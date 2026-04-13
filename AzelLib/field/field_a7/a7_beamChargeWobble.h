#pragma once

#include "task.h"
#include "common.h"
#include "a7_sceneParticle.h"
#include <vector>

struct sVdp1Quad;

// Arg struct passed to the 0x1C charge-wobble subtask from the beam emitter.
struct sA7BeamChargeWobbleArg
{
    sVec3_FP m0_pos;      // raised position (y += 0x80000)
    sSaturnPtr mC_templateEA;
};

// 06055ada — wrap createSubTaskWithArg for the 0x1C beam-charge-wobble subtask
void a7BeamEmitter_spawnChargeWobble_06055ada(p_workArea parent, sA7BeamChargeWobbleArg* pArg);

// 0605533c — spawn a jittered billboard scene particle. Shared by the charge
// wobble subtask and the beam ring-buffer update.
void a7BeamChargeWobble_spawn_0605533c(p_workArea parent, sVec3_FP* pPos,
                                       fixedPoint sinValue, sA7SceneParticleDesc* pOutDesc);

// Lazy-parses and caches a VDP1 quad list keyed by Saturn EA.
const std::vector<sVdp1Quad>* a7GetOrParseQuadList(const sSaturnPtr& ea);
