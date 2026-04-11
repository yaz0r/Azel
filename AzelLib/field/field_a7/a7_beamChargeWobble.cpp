#include "PDS.h"
#include "a7_beamChargeWobble.h"
#include "a7_sceneParticle.h"
#include "o_fld_a7.h"
#include "trigo.h"
#include "kernel/vdp1AnimatedQuad.h"
#include <vector>

// 0x1c-byte sine-wobble subtask spawned from a7CellObj2 state 0x3C.
// Task definition at FLD_A7::060843f0 = {init=06055a48, update=06055a64, draw=0, delete=0}.
struct sA7BeamChargeWobble : public s_workAreaTemplateWithArg<sA7BeamChargeWobble, sA7BeamChargeWobbleArg*>
{
    sVec3_FP m0_pos;         // 0x00
    s32 mC_angle;            // 0x0C
    fixedPoint m10_sinValue; // 0x10
    sSaturnPtr m14_templateEA; // 0x14 (Saturn 4 bytes)
    s16 m18_frame;           // 0x18
    // size 0x1C
};

// 0605533c — spawn a single billboard scene particle with jittered origin.
// Shared with the beam ring-buffer update (same Saturn address).
void a7BeamChargeWobble_spawn_0605533c(p_workArea /*parent*/, sVec3_FP* pPos,
                                       fixedPoint sinValue, sA7SceneParticleDesc* pOutDesc)
{
    sVec3_FP pos;
    pos.m0_X = fixedPoint((s32)(randomNumber() & 0xFFFF) + pPos->m0_X.m_value - 0x8000);
    pos.m4_Y = fixedPoint((s32)(randomNumber() & 0xFFFF) + pPos->m4_Y.m_value - 0x8000);
    pos.m8_Z = fixedPoint((s32)(randomNumber() & 0x3FFF) + pPos->m8_Z.m_value);

    sVec3_FP rot;
    rot.m0_X = fixedPoint(0);
    rot.m4_Y = fixedPoint(0);
    rot.m8_Z = fixedPoint(0);

    pOutDesc->m14_updateFunc = &a7SceneParticle_UpdatePhysics_06076f28;
    pOutDesc->m0_pPosition = &pos;
    pOutDesc->m4_pVelocity = &rot;
    pOutDesc->m18_payloadSize = 0;
    pOutDesc->mC_paramA = sinValue.m_value;
    pOutDesc->m10_paramB = 0;

    sFieldSceneManager* pManager = (sFieldSceneManager*)getFieldSpecificData_A7()->m280;
    a7SceneParticle_allocate(pManager, pOutDesc, 1);
}

// 06055a48
static void a7BeamChargeWobble_Init_06055a48(sA7BeamChargeWobble* pThis, sA7BeamChargeWobbleArg* pArg)
{
    pThis->m0_pos = pArg->m0_pos;
    pThis->mC_angle = 0;
    pThis->m10_sinValue = fixedPoint(0);
    pThis->m14_templateEA = pArg->mC_templateEA;
    pThis->m18_frame = 0;
}

// Lazy parse-and-cache of VDP1 quad lists keyed by Saturn EA.
const std::vector<sVdp1Quad>* a7GetOrParseQuadList(const sSaturnPtr& ea)
{
    static std::vector<std::pair<u32, std::vector<sVdp1Quad>>> cache;
    for (auto& entry : cache)
    {
        if (entry.first == ea.m_offset)
        {
            return &entry.second;
        }
    }
    cache.emplace_back(ea.m_offset, initVdp1Quad(ea));
    return &cache.back().second;
}

// 06055a64
static void a7BeamChargeWobble_Update_06055a64(sA7BeamChargeWobble* pThis)
{
    if (pThis->m18_frame < 0x5A)
    {
        pThis->m18_frame++;
        if ((pThis->m18_frame & 7) == 0)
        {
            sA7SceneParticleDesc desc = {};
            desc.m8_pQuadList = a7GetOrParseQuadList(pThis->m14_templateEA);
            a7BeamChargeWobble_spawn_0605533c((p_workArea)pThis, &pThis->m0_pos, pThis->m10_sinValue, &desc);
        }
        else
        {
            pThis->mC_angle += 0x157DC9;
            u16 idx = (u16)((u32)pThis->mC_angle >> 16) & 0xFFF;
            pThis->m10_sinValue = MTH_Mul(fixedPoint(0x10000), getSin(idx));
        }
    }
    else
    {
        pThis->getTask()->markFinished();
    }
}

// 06055ada
void a7BeamEmitter_spawnChargeWobble_06055ada(p_workArea parent, sA7BeamChargeWobbleArg* pArg)
{
    static const sA7BeamChargeWobble::TypedTaskDefinition definition = {
        &a7BeamChargeWobble_Init_06055a48,
        &a7BeamChargeWobble_Update_06055a64,
        nullptr,
        nullptr,
    };
    createSubTaskWithArg<sA7BeamChargeWobble>(parent, pArg, &definition);
}
