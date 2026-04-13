#include "PDS.h"
#include "o_fld_a5.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldModelRender.h"
#include "field/fieldCutsceneTask.h"
#include "field/exitField.h"
#include "kernel/fileBundle.h"
#include "audio/systemSounds.h"

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
    sVec3_FP m14_position;                 // 0x14
    sVec3_FP m20_rotation;                 // 0x20
    void* m2C_childTask;                   // 0x2C
    s16  m30;                              // 0x30
    u8  m31_timer;                         // 0x31 (unused padding byte between m30 and m32)
    u8  m32_mode;                          // 0x32
    u8  m33;                               // 0x33
    sFieldModelRenderContext m34_modelCtx; // 0x34
    u8  m68_isVisible;                     // 0x68 — set by checkPositionVisibility
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

// Random offset table at FLD_A5::06098ac4 (4 entries)
static const s32 s_randomOffsetTable[] = { 0x600, 0xC00, 0, -0x600 };

// 060563d8 — particle effect setup. Depends on A5 particle pool system.
static void a5ExitTrigger_setupParticles(sA5ExitTriggerEntity* /*pThis*/)
{
    Unimplemented();
}

// 06056324 — spawn particle effect using decor emitter config. Depends on
// the unimplemented particle emitter spawner (06085e46).
static void a5ExitTrigger_spawnEffect_06056324(sA5ExitTriggerEntity* /*pThis*/)
{
    Unimplemented();
}

// 0605642e — spawn directional particle burst near the entity.
static void a5ExitTrigger_spawnBurst_0605642e(sA5ExitTriggerEntity* /*pThis*/)
{
    Unimplemented();
}

// 0605623c — spawn scattered particle effect near the entity.
static void a5ExitTrigger_spawnScatter_0605623c(sA5ExitTriggerEntity* /*pThis*/)
{
    Unimplemented();
}

// 060718ec — isScriptActive (shared field.cpp)

// 0607ce18 — startExitCutsceneForCurrentField (shared exitField.cpp)

// 06056510 — render context callback. Sets visibility and bumps mode.
static void a5ExitTrigger_contextCallback(sA5ExitTriggerEntity* pThis)
{
    pThis->m34_modelCtx.m18_visibilityFlags |= 1;
    if (pThis->m32_mode == 0)
    {
        pThis->m32_mode++;
    }
}

// Vibration child task (Saturn size 0x18)
struct sA5ExitTriggerVibrationTask : public s_workAreaTemplate<sA5ExitTriggerVibrationTask>
{
    sVec3_FP m0_rotationImpulse; // 0x00 — current impulse (written by update)
    s32 m10_intensity;            // 0x10 — set externally to drive vibration
    s32 m14_storedIntensity;      // 0x14 — copied from m10 at phase 0
    s32 m18_phase;                // 0x18 — cycles 0..3
    // size 0x18 (last field is s32 but included in the 0x18)
    // Note: actual Saturn struct packs m10/m14/m18 as the last 3 dwords
};

// 0606a9f2 — addCameraImpulse, moved to shared field.cpp

// 060561b0 — vibration update: cycles through 4 phases applying
// positive/negative/half-positive/half-negative rotation impulse
static void a5ExitTriggerVibration_Update(sA5ExitTriggerVibrationTask* pThis)
{
    if (pThis->m10_intensity != 0)
    {
        s32 phase = pThis->m18_phase;
        if (phase == 0)
            pThis->m0_rotationImpulse.m0_X = fixedPoint(pThis->m10_intensity);
        else if (phase == 1)
            pThis->m0_rotationImpulse.m0_X = fixedPoint(-pThis->m10_intensity);
        else if (phase == 2)
            pThis->m0_rotationImpulse.m0_X = fixedPoint(pThis->m10_intensity >> 1);
        else if (phase == 3)
            pThis->m0_rotationImpulse.m0_X = fixedPoint(-(pThis->m10_intensity >> 1));

        addCameraImpulse(nullptr, &pThis->m0_rotationImpulse);
    }

    pThis->m18_phase = (pThis->m18_phase + 1) & 3;
    if (pThis->m18_phase == 0)
    {
        pThis->m10_intensity = pThis->m14_storedIntensity;
    }
}

// 0605620c — create vibration child subtask
static void* a5ExitTrigger_createChildTask(sA5ExitTriggerEntity* pThis)
{
    sA5ExitTriggerVibrationTask* pChild =
        createSubTaskFromFunction<sA5ExitTriggerVibrationTask>((p_workArea)pThis, &a5ExitTriggerVibration_Update);
    if (pChild)
    {
        pChild->m0_rotationImpulse = {};
        pChild->m10_intensity = 0;
        pChild->m14_storedIntensity = 0;
        pChild->m18_phase = 0;
    }
    return pChild;
}

// 060565C2 — normal draw: renders both model pairs when visible.
static void a5ExitTrigger_Draw(sA5ExitTriggerEntity* pThis)
{
    if (pThis->m68_isVisible == 0)
        return;

    pushCurrentMatrix();
    translateCurrentMatrix(&pThis->m14_position);
    LCSItemBox_DrawType0Sub0(pThis->m0_memoryArea.m0_mainMemoryBundle, pThis->m8_modelA, pThis->mA_poseA);
    LCSItemBox_DrawType0Sub0(pThis->m0_memoryArea.m0_mainMemoryBundle, pThis->mE_modelB, pThis->m10_poseB);
    callGridCellDraw_normalSub2(pThis->m0_memoryArea.m0_mainMemoryBundle, pThis->mC_entryA);
    callGridCellDraw_normalSub2(pThis->m0_memoryArea.m0_mainMemoryBundle, pThis->m12_entryB);
    popMatrix();
}

// 06056580 — alt draw: renders only the second model pair (post-discovery).
static void a5ExitTrigger_DrawAlt(sA5ExitTriggerEntity* pThis)
{
    if (pThis->m68_isVisible == 0)
        return;

    pushCurrentMatrix();
    translateCurrentMatrix(&pThis->m14_position);
    LCSItemBox_DrawType0Sub0(pThis->m0_memoryArea.m0_mainMemoryBundle, pThis->mE_modelB, pThis->m10_poseB);
    callGridCellDraw_normalSub2(pThis->m0_memoryArea.m0_mainMemoryBundle, pThis->m12_entryB);
    popMatrix();
}

// 0605661C — update: 8-state machine driving discovery sequence.
static void a5ExitTrigger_Update(sA5ExitTriggerEntity* pThis)
{
    u8 mode = pThis->m32_mode;

    if (mode == 0)
    {
        // Idle: check if render context flagged both visibility and proximity
        if ((pThis->m34_modelCtx.m18_visibilityFlags & 1) && (pThis->m34_modelCtx.m18_visibilityFlags & 4))
        {
            if ((mainGameState.bitField[0xB] & 0x80) == 0)
                startFieldScript(0xD, 0x5C2);
            else
                startFieldScript(0x11, 0x5C6);
        }
    }
    else if (mode == 1)
    {
        // Randomize position offsets, count down timer
        u32 r = randomNumber();
        pThis->m14_position.m0_X = fixedPoint(s_randomOffsetTable[r & 3] + 0x400000);
        r = randomNumber();
        pThis->m14_position.m8_Z = fixedPoint(s_randomOffsetTable[r & 3] + -0x400000);
        r = randomNumber();
        pThis->m20_rotation.m8_Z = fixedPoint(s_randomOffsetTable[r & 3] << 4);

        pThis->m30--;
        if (pThis->m30 > 0)
        {
            if (pThis->m30 < 0x29)
            {
                // Set child task vibration intensity
                *(s32*)((u8*)pThis->m2C_childTask + 0x10) = 0x1B4E81;
            }
        }
        else
        {
            *(s32*)((u8*)pThis->m2C_childTask + 0x10) = 0;
            pThis->m30 = 0x32;
            pThis->m32_mode++;
        }
    }
    else if (mode == 2)
    {
        // Count down, spawn burst every 4th frame
        pThis->m30--;
        if (pThis->m30 > 0)
        {
            if ((pThis->m30 & 3) == 0)
            {
                a5ExitTrigger_spawnBurst_0605642e(pThis);
            }
        }
        else
        {
            pThis->m32_mode++;
        }
    }
    else if (mode == 3)
    {
        // Start field script, spawn effect, install alt draw, advance
        if ((mainGameState.bitField[0xB] & 0x80) == 0)
            startFieldScript(0xE, -1);
        else
            startFieldScript(0x12, -1);

        pThis->m14_position.m0_X = fixedPoint(0x400000);
        pThis->m14_position.m8_Z = fixedPoint(-0x400000);
        a5ExitTrigger_spawnEffect_06056324(pThis);
        pThis->m30 = 0x14;
        pThis->m32_mode++;
        // Patch draw to alt version (Saturn writes to task header at -0xC)
        pThis->m_DrawMethod = &a5ExitTrigger_DrawAlt;
    }
    else if (mode == 4)
    {
        // Count down, spawn scatter each frame
        pThis->m30--;
        if (pThis->m30 > 0)
        {
            a5ExitTrigger_spawnScatter_0605623c(pThis);
        }
        else
        {
            pThis->m32_mode++;
        }
    }
    else if (mode == 5)
    {
        // Wait for field script system to become idle
        if (isScriptActive() == 0)
        {
            pThis->m34_modelCtx.m4_initCallback = (void*)0x0605652e; // placeholder — contextCallback variant
            pThis->m34_modelCtx.m10_flags = 3;
            pThis->m34_modelCtx.m18_visibilityFlags = 0;
            mainGameState.bitField[0x95] |= 4;
            pThis->m32_mode++;
        }
    }
    else if (mode == 7)
    {
        // Trigger the exit cutscene
        mainGameState.bitField[0x58] |= 4;
        startExitCutsceneForCurrentField(gFLD_A5->getSaturnPtr(0x060988E8), 10, 0, -0x8000);
        pThis->m32_mode++;
    }
    // mode 6 and 8+: no-op (state 6 just advances via contextCallback)

    // Visibility check against far clip
    bool visible = checkPositionVisibilityAgainstFarPlane(&pThis->m14_position);
    pThis->m68_isVisible = visible ? 0 : 1;

    if (!visible)
    {
        if (pThis->m33 == 0)
        {
            setFieldExitConfig_A5(2, nullptr);
            pThis->m33 = 1;
        }
    }
    else
    {
        if (pThis->m33 != 0)
        {
            setFieldExitConfig_A5(0, nullptr);
            pThis->m33 = 0;
        }
    }

    updateFieldModelRenderContext(&pThis->m34_modelCtx);
}

// 06056870
sA5ExitTriggerEntity* a5_createExitTriggerEntity_06056870(p_workArea parent, const sA5ExitTriggerArg* arg)
{
    static sA5ExitTriggerEntity::TypedTaskDefinition td = {
        nullptr, &a5ExitTrigger_Update, &a5ExitTrigger_Draw, nullptr
    };
    sA5ExitTriggerEntity* pThis = createSubTask<sA5ExitTriggerEntity>(parent, &td);
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
    pThis->m68_isVisible = 0;

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
        // Patch draw to alt version (Saturn writes to task header at -0xC)
        pThis->m_DrawMethod = &a5ExitTrigger_DrawAlt;
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
