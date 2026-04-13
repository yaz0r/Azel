#include "PDS.h"
#include "fieldAnimRingSubTask.h"

// Animated ring lerp subtask — shared across all field overlays.
// Moved from field_a7/a7_envEntity2C.cpp; byte-identical in A3.

static inline s32 performDivision(s32 divisor, s32 dividend) { return dividend / divisor; }

struct sAnimRingSubTask : public s_workAreaTemplateWithArg<sAnimRingSubTask, sAnimRingArg*>
{
    s32      m0_pad0;
    s32      m4_pad1;
    sVec3_FP m8_center;
    sVec3_FP m14_current;
    sVec3_FP m20_target;
    sVec3_FP m2C_stepPerFrame;
    sVec3_FP m38_direction;
    s32      m44;
    s32      m48_lifetime;
    // Saturn size 0x4c
};

// 0607fafc (A7) / 06081530 (A3) — alternate update (mode-1)
static void animRingSubTask_UpdateAlt(sAnimRingSubTask* pThis)
{
    pThis->m44++;
    if (pThis->m48_lifetime <= pThis->m44)
    {
        pThis->getTask()->markFinished();
    }
}

// 0607fb28 (A7) / 0608155c (A3) — alternate draw (mode-1)
static void animRingSubTask_DrawAlt(sAnimRingSubTask* /*pThis*/)
{
    Unimplemented();
}

// 0607f8ae (A7) / 060812e2 (A3) — default update: lerp position
static void animRingSubTask_Update(sAnimRingSubTask* pThis)
{
    pThis->m44++;
    if (pThis->m44 < pThis->m48_lifetime)
    {
        pThis->m14_current.m0_X = fixedPoint(pThis->m14_current.m0_X.m_value + pThis->m2C_stepPerFrame.m0_X.m_value);
        pThis->m14_current.m4_Y = fixedPoint(pThis->m14_current.m4_Y.m_value + pThis->m2C_stepPerFrame.m4_Y.m_value);
        pThis->m14_current.m8_Z = fixedPoint(pThis->m14_current.m8_Z.m_value + pThis->m2C_stepPerFrame.m8_Z.m_value);
    }
    else
    {
        pThis->m14_current = pThis->m20_target;
        pThis->getTask()->markFinished();
    }
}

// 0607f904 (A7) / 06081338 (A3) — default draw
static void animRingSubTask_Draw(sAnimRingSubTask* /*pThis*/)
{
    Unimplemented();
}

// 0607f824 (A7) / 06081258 (A3) — init
static void animRingSubTask_Init(sAnimRingSubTask* pThis, sAnimRingArg* pArg)
{
    pThis->m8_center   = pArg->m00_center;
    pThis->m14_current = pArg->m0C_ring0;
    pThis->m20_target  = pArg->m18_ring1;
    pThis->m38_direction = pArg->m24_direction;

    if (pArg->m34_mode == 0)
    {
        pThis->m2C_stepPerFrame.m0_X =
            fixedPoint(performDivision(pArg->m30_lifetime,
                pThis->m20_target.m0_X.m_value - pThis->m14_current.m0_X.m_value));
        pThis->m2C_stepPerFrame.m4_Y =
            fixedPoint(performDivision(pArg->m30_lifetime,
                pThis->m20_target.m4_Y.m_value - pThis->m14_current.m4_Y.m_value));
        pThis->m2C_stepPerFrame.m8_Z =
            fixedPoint(performDivision(pArg->m30_lifetime,
                pThis->m20_target.m8_Z.m_value - pThis->m14_current.m8_Z.m_value));
    }
    else if (pArg->m34_mode == 1)
    {
        pThis->m_UpdateMethod = &animRingSubTask_UpdateAlt;
        pThis->m_DrawMethod   = &animRingSubTask_DrawAlt;
    }

    pThis->m44          = 0;
    pThis->m48_lifetime = pArg->m30_lifetime;
}

// 0607fe24 (A7) / 06081858 (A3) — spawn
void spawnAnimRingSubTask(p_workArea parent, sAnimRingArg* pArg)
{
    static sAnimRingSubTask::TypedTaskDefinition td = {
        &animRingSubTask_Init,
        &animRingSubTask_Update,
        &animRingSubTask_Draw,
        nullptr,
    };
    createSubTaskWithArg<sAnimRingSubTask>(parent, pArg, &td);
}
