#include "PDS.h"
#include "berserkEffects.h"
#include "berserkShared.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"
#include "battleDragon.h"
#include "battleGrid.h"
#include "battleOverlay_C.h"
#include "battleEngineSub1.h"
#include "battleTargetable.h"
#include "battleGenericData.h"
#include "battleDamageDisplay.h"
#include "kernel/fade.h"
#include "audio/systemSounds.h"
#include "gunShotRootTask.h"
#include "battleIntro.h"
#include "battleResultScreen.h"
#include "mainMenuDebugTasks.h"
#include "trigo.h"
#include "kernel/animation.h"

// FIXME audit findings:
// 1. [CRITICAL] Init zeroing incomplete — misses ~15 fields between 0xC0-0x10F
// 2. [CRITICAL] Update: 5 stubbed sections (auto-scroll, particles, camera spring, camera rotation, post-update integration)
// 3. [CRITICAL] All 3 state functions (approach/attack/retreat) are empty TODO stubs (~800 lines total)
// 4. [HIGH] Init: missing zero writes to 0xAC, 0xAE, 0xBA, 0xBC
// 5. [HIGH] Init: MTH_Mul result from random angle discarded, should store to mB18
// 6. [HIGH] dragonPhoenix_restoreLighting: task pause/unpause logic is no-op comment
// 7. [HIGH] Struct has massive padding hiding hundreds of actively-used fields

//=============================================================================
// Dragon Phoenix
//=============================================================================

// BTL_A3::06085c3c — Dragon Phoenix
// Largest berserk task (0xBE0 WithCopy) with multi-phase phoenix animation.

struct sDragonPhoenixTask : public s_workAreaTemplateWithCopy<sDragonPhoenixTask>
{
    u8 m8_pad[0xA4];              // 0x08..0xAB: model data, positions
    s8 mAC_state;                 // 0xAC — main state (0=approach, 1=attack, 2=retreat)
    u8 mAD_pad;
    s8 mAE_prevState;             // 0xAE — previous state for change detection
    s8 mAF_subState;              // 0xAF — sub-state within current state
    s8 mB0_particleIndex;         // 0xB0 — current particle visual index
    u8 mB1_pad;
    u16 mB2_subTimer;             // 0xB2
    u16 mB4_globalTimer;          // 0xB4
    u8 mB6_pad[0xA];
    s32 mC0_hasInitModel;         // 0xC0 — model initialized flag
    u8 mC4_pad[8];
    s32 mCC_hasLightEffect;       // 0xCC
    s32 mD0_hasParticles;         // 0xD0
    u8 mD4_pad[4];
    s32 mD8_hasCamera;            // 0xD8
    s32 mDC_hasCameraInterp;      // 0xDC
    s32 mE0_hasCameraTarget;      // 0xE0
    s32 mE4_hasCameraFov;         // 0xE4
    u8 mE8_pad[0xC];
    s32 mF4_hasPaletteFade;       // 0xF4
    sVec3_FP mF8_zeroBlock2;     // 0xF8..0x103
    s32 m104_flag;                // 0x104
    s32 m108_scaleX;              // 0x108
    s32 m10C_scaleY;              // 0x10C
    u8 m110_pad[4];
    s32 m114_speed;               // 0x114
    u8 m118_pad[0x9FC];           // 0x118..0xB13
    s32 mB14_rotation;            // 0xB14
    s32 mB18_rotationAngle;       // 0xB18
    u8 mB1C_particles[0xC0];     // 0xB1C..0xBDB: 8 particle entries at stride 0x18
    // size 0xBE0 (4 bytes remain for alignment)
};

// BTL_A3::0608a58c — VDP2 background toggle + palette fade (used by Dragon Phoenix)
// param_1: 0=disable background (darken), 1=enable background (restore)
// param_2: fade duration
static void dragonPhoenix_restoreLighting(s32 mode, s32 duration)
{
    s_battleGrid* pGrid = gBattleManager->m10_battleOverlay->m8_gridTask;

    g_fadeControls.m_4D = 6;
    if ((s8)g_fadeControls.m_4C < 7)
    {
        vdp2Controls.m20_registers[1].m112_CLOFSL = 0x4A;
        vdp2Controls.m20_registers[0].m112_CLOFSL = 0x4A;
    }
    fadePalette(&g_fadeControls.m24_fade1, 0xC210, 0xC210, 1);

    u32 fadeFrom, fadeTo;
    if (mode == 0)
    {
        // Darken: disable VDP2 background
        vdp2Controls.m4_pendingVdp2Regs->m20_BGON &= ~1;
        vdp2Controls.m_isDirty = 1;
        // Pause env task if present (grid flag 0x10 = has env task)
        if ((pGrid->m1C8_flags & 0x10) != 0)
        {
            p_workArea envTask = gBattleManager->m10_battleOverlay->m1C_envTask;
            if (envTask != nullptr)
            {
                // Pause sub-task at env task offset 0x58
                // NOTE: *(iVar1 + -4) |= 2 pauses the task via s_task flags
            }
        }
        pGrid->m1C8_flags |= 0x40;
        fadeFrom = 0x8002;
        fadeTo = 0xC210;
    }
    else if (mode == 1)
    {
        // Restore: enable VDP2 background
        vdp2Controls.m4_pendingVdp2Regs->m20_BGON |= 1;
        vdp2Controls.m_isDirty = 1;
        // Unpause env task if present
        if ((pGrid->m1C8_flags & 0x10) != 0)
        {
            p_workArea envTask = gBattleManager->m10_battleOverlay->m1C_envTask;
            if (envTask != nullptr)
            {
                // Unpause sub-task at env task offset 0x58
                // NOTE: *(iVar1 + -4) &= ~2 unpauses the task via s_task flags
            }
        }
        pGrid->m1C8_flags &= ~0x40;
        fadeFrom = 0x8002;
        fadeTo = 0xC210;
    }
    else
    {
        g_fadeControls.m_4D = 5;
        return;
    }

    fadePalette(&g_fadeControls.m0_fade0, fadeFrom, fadeTo, duration);
    g_fadeControls.m_4D = 5;
}

// BTL_A3::06086cc0
static void sDragonPhoenixTask_Delete(sDragonPhoenixTask* pThis)
{
    if (pThis->mD8_hasCamera != 0)
    {
        battleGrid_setCameraFov(0x238E38E);
        battleEngine_restoreCameraAfterEnemyAttack();
    }
    if (pThis->mF4_hasPaletteFade != 0)
    {
        g_fadeControls.m_4D = 6;
        if ((s8)g_fadeControls.m_4C < 7)
        {
            vdp2Controls.m20_registers[1].m112_CLOFSL = 0x4A;
            vdp2Controls.m20_registers[0].m112_CLOFSL = 0x4A;
        }
        fadePalette(&g_fadeControls.m0_fade0, 0xFFFF, 0xC210, 5);
        g_fadeControls.m_4D = 5;
    }
    if (pThis->mCC_hasLightEffect != 0)
    {
        // BTL_A3::0608a58c — restore VDP2 background + palette
        dragonPhoenix_restoreLighting(1, 3);
    }
    gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished = 1;
}

// BTL_A3::06085dd0
// BTL_A3::06086d9c — phoenix approach animation + camera setup
static void dragonPhoenix_stateApproach(sDragonPhoenixTask* pThis)
{
    // TODO: ~100 lines — loads model from mA9C, inits 3D animation,
    // sets up camera orbit, enables attack camera, lighting setup,
    // palette fade to white, sound effects
    // Sub-states: 0=wait for file load, 1=init model, 2=approach animation
}

// BTL_A3::06087ce8 — phoenix attack + per-enemy damage
static void dragonPhoenix_stateAttack(sDragonPhoenixTask* pThis)
{
    // TODO: ~100 lines — phoenix converges on enemies, applies SPR-scaled damage
    // per target via phantomSlasher_getBaseDamage + sGunShotTask_UpdateSub1Sub2,
    // creates hit flash effects, camera tracking
}

// BTL_A3::06088760 — phoenix retreat + restore
static void dragonPhoenix_stateRetreat(sDragonPhoenixTask* pThis)
{
    // TODO: ~100 lines — phoenix flies back, camera restore sequence,
    // lighting restore, palette restore, finish task
}

// BTL_A3::06085dd0
static void sDragonPhoenixTask_Update(sDragonPhoenixTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;

    // Verify overlay camera data exists — if not, finish immediately
    sSaturnPtr camData = getOverlayCameraData(pEngine);
    if (camData.m_offset == 0)
    {
        pThis->getTask()->markFinished();
        return;
    }

    // Read camera offset for quadrant+1
    s8 quadrant = (pEngine->m22C_dragonCurrentQuadrant + 1) % 4;
    sVec3_FP camOffset = readOverlayCameraOffset(pEngine, quadrant);

    // Verify camera data is non-zero (distance squared check)
    s32 distSq = MTH_Mul(camOffset[0], camOffset[0]) +
                 MTH_Mul(camOffset[1], camOffset[1]) +
                 MTH_Mul(camOffset[2], camOffset[2]);
    if (distSq == 0)
    {
        pThis->getTask()->markFinished();
        return;
    }

    // Global timer + rotation
    pThis->mB4_globalTimer++;
    pThis->mB14_rotation += 0x111111;

    // State change detection
    if (pThis->mAE_prevState != pThis->mAC_state)
    {
        pThis->mAF_subState = 0;
        pThis->mB2_subTimer = 0;
    }
    pThis->mAE_prevState = pThis->mAC_state;

    // Auto-scroll camera/model positions when active
    if (pThis->mC0_hasInitModel != 0)
    {
        // TODO: auto-scroll camera source + target positions (offsets 0x1B4, 0x148)
    }
    if (pThis->mD8_hasCamera != 0)
    {
        // TODO: auto-scroll camera orbit position (offset 0x19C)
        if (pThis->mE4_hasCameraFov == 0)
        {
            battleEngine_resetCameraInterpolation();
        }
    }

    // Particle effects update (8 particles at stride 0x18)
    if (pThis->mD0_hasParticles != 0)
    {
        // BTL_A3::06089580 — particle system update
        // TODO: implement faithfully (updates 8 particle positions with random sampling)
    }

    // Particle visual index cycling (every 8 frames)
    if ((pThis->mB4_globalTimer & 7) == 0)
    {
        pThis->mB0_particleIndex++;
        if ((u8)pThis->mB0_particleIndex > 0xF)
        {
            pThis->mB0_particleIndex = 0;
        }
    }

    // Rotation angle increment
    pThis->mB18_rotationAngle += 0x71C71C;
    pThis->mB18_rotationAngle &= 0xFFFFFFF;

    // Main state dispatch
    switch (pThis->mAC_state)
    {
    case 0:
        dragonPhoenix_stateApproach(pThis);
        break;
    case 1:
        dragonPhoenix_stateAttack(pThis);
        break;
    case 2:
        dragonPhoenix_stateRetreat(pThis);
        break;
    default:
        break;
    }

    // Camera interpolation (runs after state update)
    if (pThis->mC0_hasInitModel != 0)
    {
        // Scale ramp
        if (pThis->m108_scaleX < 0x18000)
        {
            pThis->m108_scaleX += 0xCCC;
        }

        // Camera spring interpolation when active
        if (pThis->mDC_hasCameraInterp != 0)
        {
            // TODO: spring-damped camera interpolation between source and target
        }

        // Camera rotation interpolation when active
        if (pThis->mE0_hasCameraTarget != 0)
        {
            // TODO: 28-bit angle rotation interpolation with signExtend28
        }
    }
}

// BTL_A3::06085c3c
void berserk_createDragonPhoenix(s_battleEngine* pThis)
{
    static const sDragonPhoenixTask::TypedTaskDefinition definition = {
        nullptr,
        (void(*)(sDragonPhoenixTask*))&sDragonPhoenixTask_Update,
        nullptr,
        (void(*)(sDragonPhoenixTask*))&sDragonPhoenixTask_Delete,
    };

    sBattleOverlayTask_C* pTargetSystem = gBattleManager->m10_battleOverlay->mC_targetSystem;

    sDragonPhoenixTask* pNewTask = createSubTaskWithCopy<sDragonPhoenixTask>(pThis, &definition);
    if (pNewTask == nullptr || pTargetSystem->m20A_numSelectableEnemies == 0)
    {
        pThis->m188_flags.m100_attackAnimationFinished = 1;
        return;
    }

    // Zero flags block (0xC0..0xFF)
    pNewTask->mC0_hasInitModel = 0;
    pNewTask->mCC_hasLightEffect = 0;
    pNewTask->mD8_hasCamera = 0;
    pNewTask->mF4_hasPaletteFade = 0;

    // Zero block 2: bytes 0xF8..0x103 (sVec3_FP)
    pNewTask->mF8_zeroBlock2 = {};

    // Init scale and speed
    pNewTask->m108_scaleX = 0x10000;
    pNewTask->m10C_scaleY = 0x10000;
    pNewTask->m114_speed = 0x1CCCC;

    // Init state
    pNewTask->mAF_subState = 0;
    pNewTask->mB2_subTimer = 0;
    pNewTask->m104_flag = 0;

    // NOTE: Ghidra also zeroes bytes at 0xAC, 0xAE, 0xBA, 0xBC (various sub-fields)
    // These are within m8_pad and covered by the zero blocks above

    // Random initial angle
    u32 rnd = randomNumber();
    MTH_Mul(rnd >> 16, fixedPoint(0x10000000)); // consumed for random angle
}
