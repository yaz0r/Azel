#include "PDS.h"
#include "fieldBeamSpark.h"

// Beam spark effect — shared across all field overlays.
// Moved from field_a7/a7_beamSpark.cpp; byte-identical in A3.

// 0606a400 (A7) / 0606be34 (A3) — alternate update installed when mC == 0 && m10 == 0
void beamSpark_UpdateAlt(sBeamSpark* pThis)
{
    Unimplemented();
}

// 06069f16 (A7) / 0606b94a (A3) — scalar accumulator update
void beamSpark_Update(sBeamSpark* pThis)
{
    pThis->m24 += pThis->m28;
    pThis->m1C += pThis->m24;
    pThis->m2C -= pThis->m2E;
    pThis->m18--;
    if ((s32)pThis->m18 < 1)
    {
        pThis->getTask()->markFinished();
    }
}

// 06069f58 (A7) / 0606b98c (A3) — draw
void beamSpark_Draw(sBeamSpark* pThis)
{
    Unimplemented();
}

// 06069ec4 (A7) / 0606b8f8 (A3) — init
void beamSpark_Init(sBeamSpark* pThis, sBeamSparkArg* pArg)
{
    pThis->m0_pos = pArg->m0_pos;
    pThis->mC  = pArg->mC;
    pThis->m10 = pArg->m10;
    pThis->m14 = pArg->m14;
    pThis->m18 = pArg->m18;
    pThis->m1C = pArg->m1C;
    pThis->m20 = pArg->m20;
    pThis->m24 = pArg->m24;
    pThis->m28 = pArg->m28;
    pThis->m2C = pArg->m2C;
    pThis->m2E = pArg->m2E & 0x7FFF;

    if (pArg->mC == 0 && pArg->m10 == 0)
    {
        pThis->m_UpdateMethod = &beamSpark_UpdateAlt;
    }
}

// 0606a64c (A7) — spawn beam spark sibling task
void beamSpark_spawn(p_workArea parent, sBeamSparkArg* pArg)
{
    static const sBeamSpark::TypedTaskDefinition definition = {
        &beamSpark_Init,
        &beamSpark_Update,
        &beamSpark_Draw,
        nullptr,
    };
    createSiblingTaskWithArg<sBeamSpark>(parent, pArg, &definition);
}
