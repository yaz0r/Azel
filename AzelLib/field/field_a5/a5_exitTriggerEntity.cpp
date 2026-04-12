#include "PDS.h"
#include "o_fld_a5.h"
#include "field/fieldModelRender.h"
#include "kernel/fileBundle.h"

// Exit-trigger entity (Saturn size 0x6C). Task definition at FLD_A5::06098ad4
// is {init=null, update=0605661C, draw=060565C2, delete=null}. The creator
// FUN_FLD_A5__06056870 does all the field setup inline after createSubTask.
struct sA5ExitTriggerEntity : public s_workAreaTemplate<sA5ExitTriggerEntity>
{
    s_memoryAreaOutput m0_memoryArea;     // 0x00
    s16 m8_modelA;                         // 0x08
    s16 mA_poseA;                          // 0x0A
    s16 mC_entryA;                         // 0x0C
    s16 mE_modelB;                         // 0x0E
    s16 m10_poseB;                         // 0x10
    s16 m12_entryB;                        // 0x12
    sVec3_FP m14_position;                 // 0x14 — {0x400000, 0, -0x400000}
    sVec3_FP m20_rotation;                 // 0x20 — {0, 0, 0}
    void* m2C_childTask;                   // 0x2C — from FUN_0605620c
    u8  m30;                               // 0x30
    u8  m31_timer;                         // 0x31
    u8  m32_mode;                          // 0x32
    u8  m33;                               // 0x33
    sFieldModelRenderContext m34_modelCtx; // 0x34
    // Saturn size 0x6C
};

// Packed arg struct
struct sA5ExitTriggerArg
{
    s32 m0_memoryAreaIdx;
    s16 m4_modelA;
    s16 m6_poseA;
    s16 m8_entryA;
    s16 mA_modelB;
    s16 mC_poseB;
    s16 mE_entryB;
};

// Static render-context position from Saturn ROM at FLD_A5::06098740
static const sVec3_FP s_exitTriggerModelPosition = {
    fixedPoint(0x401000), fixedPoint(0), fixedPoint(-0x40F000)
};

// 060563d8 — particle effect setup. Depends on A5 particle pool system.
static void a5ExitTrigger_setupParticles(sA5ExitTriggerEntity* /*pThis*/)
{
    Unimplemented();
}

// 06056510 — render context callback. Sets visibility and bumps mode.
static void a5ExitTrigger_contextCallback(sA5ExitTriggerEntity* pThis)
{
    pThis->m34_modelCtx.m18_visibilityFlags |= 1;
    if (pThis->m32_mode == 0)
    {
        pThis->m32_mode++;
    }
}

// 0605620c — create a 0x18-byte vibration child subtask (zeroed init).
static void* a5ExitTrigger_createChildTask(sA5ExitTriggerEntity* pThis)
{
    Unimplemented();
    return nullptr;
}

// 06056580 — alternate update installed when bitField[0x95] & 4
static void a5ExitTrigger_UpdateAlt(sA5ExitTriggerEntity* /*pThis*/)
{
    Unimplemented();
}

// 06056870
sA5ExitTriggerEntity* a5_createExitTriggerEntity_06056870(p_workArea parent, const sA5ExitTriggerArg* arg)
{
    sA5ExitTriggerEntity* pThis = createSubTaskFromFunction<sA5ExitTriggerEntity>(parent, nullptr);
    if (pThis == nullptr)
        return nullptr;

    getMemoryArea(&pThis->m0_memoryArea, arg->m0_memoryAreaIdx);
    pThis->m8_modelA  = arg->m4_modelA;
    pThis->mA_poseA   = arg->m6_poseA;
    pThis->mC_entryA  = arg->m8_entryA;
    pThis->mE_modelB  = arg->mA_modelB;
    pThis->m10_poseB  = arg->mC_poseB;
    pThis->m12_entryB = arg->mE_entryB;

    pThis->m14_position.m0_X = fixedPoint(0x400000);
    pThis->m14_position.m4_Y = 0;
    pThis->m14_position.m8_Z = fixedPoint(-0x400000);
    pThis->m20_rotation.m0_X = 0;
    pThis->m20_rotation.m4_Y = 0;
    pThis->m20_rotation.m8_Z = 0;

    pThis->m30 = 0;
    pThis->m31_timer = 0x3C;
    pThis->m33 = 0;

    setFieldExitConfig_A5(0, nullptr);

    initFieldModelRenderContext(
        &pThis->m34_modelCtx,
        pThis,
        (void*)&a5ExitTrigger_contextCallback,
        const_cast<sVec3_FP*>(&s_exitTriggerModelPosition),
        nullptr,
        2, 0, -1, 0, 6);

    if ((mainGameState.bitField[0x95] & 4) == 0)
    {
        pThis->m32_mode = 0;
    }
    else
    {
        a5ExitTrigger_setupParticles(pThis);
        pThis->m_UpdateMethod = &a5ExitTrigger_UpdateAlt;
        pThis->m32_mode = 5;
    }

    pThis->m2C_childTask = a5ExitTrigger_createChildTask(pThis);

    return pThis;
}

// 06056970 — create the subfield-0 exit trigger entity
void createA5_envObjects_sub0_exits(p_workArea parent)
{
    sA5ExitTriggerArg arg;
    arg.m0_memoryAreaIdx = 2;
    arg.m4_modelA = 4;
    arg.m6_poseA  = 0x538;
    arg.m8_entryA = 0x2AC;
    arg.mA_modelB = 8;
    arg.mC_poseB  = 0x53C;
    arg.mE_entryB = 0x2B0;
    a5_createExitTriggerEntity_06056870(parent, &arg);
}

// 060569ac — create the subfield-7 exit trigger entity
void createA5_envObjects_sub7_exits(p_workArea parent)
{
    sA5ExitTriggerArg arg;
    arg.m0_memoryAreaIdx = 5;
    arg.m4_modelA = 4;
    arg.m6_poseA  = 0x160;
    arg.m8_entryA = 0xC0;
    arg.mA_modelB = 8;
    arg.mC_poseB  = 0x164;
    arg.mE_entryB = 0xC4;
    a5_createExitTriggerEntity_06056870(parent, &arg);
}
