#include "PDS.h"
#include "a7_beamSpark.h"

// 0x30-byte effect sibling task spawned by a7CellObj2_Update state 2.
// Task definition at FLD_A7::06087174 = {init=06069ec4, update=06069f16, draw=06069f58, delete=0}.

struct sA7BeamSparkArg
{
    sVec3_FP m0_pos;   // 0x00
    u32      mC;       // 0x0C
    u32      m10;      // 0x10
    u32      m14;      // 0x14 (gap in original caller — stack-uninitialized)
    u32      m18;      // 0x18
    u32      m1C;      // 0x1C
    u32      m20;      // 0x20
    u32      m24;      // 0x24
    s32      m28;      // 0x28
    u16      m2C;      // 0x2C
    u16      m2E;      // 0x2E
    // size 0x30
};

struct sA7BeamSpark : public s_workAreaTemplateWithArg<sA7BeamSpark, sA7BeamSparkArg*>
{
    sVec3_FP m0_pos;   // 0x00
    u32      mC;       // 0x0C
    u32      m10;      // 0x10
    u32      m14;      // 0x14
    u32      m18;      // 0x18
    u32      m1C;      // 0x1C
    u32      m20;      // 0x20
    u32      m24;      // 0x24
    s32      m28;      // 0x28
    u16      m2C;      // 0x2C
    u16      m2E;      // 0x2E (masked with 0x7FFF on init)
    // size 0x30
};

// 0606a400 — alternate update installed when mC == 0 && m10 == 0
static void a7BeamSpark_UpdateAlt_0606a400(sA7BeamSpark* pThis)
{
    Unimplemented();
}

// 06069f16 — scalar accumulator update (unused by cellObj2 state 2 which always
// hits the UpdateAlt path; kept faithful for the mC!=0 || m10!=0 branch).
static void a7BeamSpark_Update_06069f16(sA7BeamSpark* pThis)
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

// 06069f58
static void a7BeamSpark_Draw_06069f58(sA7BeamSpark* pThis)
{
    Unimplemented();
}

// 06069ec4
static void a7BeamSpark_Init_06069ec4(sA7BeamSpark* pThis, sA7BeamSparkArg* pArg)
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
        pThis->m_UpdateMethod = &a7BeamSpark_UpdateAlt_0606a400;
    }
}

// 0606a64c
static void a7BeamSpark_spawn_0606a64c(p_workArea parent, sA7BeamSparkArg* pArg)
{
    static const sA7BeamSpark::TypedTaskDefinition definition = {
        &a7BeamSpark_Init_06069ec4,
        &a7BeamSpark_Update_06069f16,
        &a7BeamSpark_Draw_06069f58,
        nullptr,
    };
    createSiblingTaskWithArg<sA7BeamSpark>(parent, pArg, &definition);
}

// 06055aea
void a7BeamEmitter_spawnSpark_06055aea(p_workArea parent, sVec3_FP* pPos)
{
    sA7BeamSparkArg arg;
    arg.m0_pos = *pPos;
    arg.mC  = 0;
    arg.m10 = 0;
    arg.m14 = 0;          // gap in original caller (stack-uninitialized); zero for determinism
    arg.m18 = 0x14;
    arg.m1C = 0x800;
    arg.m20 = 0x14CC;
    arg.m24 = 0x4000;
    arg.m28 = (s32)0xFFFFFD9A;
    arg.m2C = 0xFFFF;
    arg.m2E = 0x8421;
    a7BeamSpark_spawn_0606a64c(parent, &arg);
}
