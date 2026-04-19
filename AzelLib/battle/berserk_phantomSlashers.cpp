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
#include "menu_dragonMorph.h"
#include "kernel/fileBundle.h"


//=============================================================================
// FIXME LIST (Ghidra comparison audit 2026-04-03)
//
// 1. [CRITICAL] sPhantomSlasherTask_Update (0608156e): Missing end-of-frame
//    termination logic. After damage processing, Ghidra counts projectiles with
//    m4_active==0 and calls markFinished when all are inactive. Also increments
//    m58 each frame and calls markFinished if m58 > 0x14A (330-frame timeout).
//    Both checks are completely absent -- attack task never self-terminates.
//
// 2. [CRITICAL] phantomSlasherProjectile_flyToTarget (06082748) state 0:
//    Missing 4th randomNumber() call. Ghidra uses separate random angles for
//    sinCos and cosCos (4 random calls total), but C++ reuses angle3 for both
//    (only 3 random calls). This desyncs the RNG and changes projectile
//    trajectories.
//
// 3. [HIGH] phantomSlasherProjectile_updateTrails (06083a10): Second arg to
//    setAnimationFrame reads wrong offset. Ghidra uses *(short*)(param_1+0x6E)
//    which is offset 0x16 into m58_mainModel (a model-internal field). C++ reads
//    m1F4_animTimer (offset 0x1F4). Completely wrong source data.
//
// 4. [HIGH] berserk_createPhantomSlashers (06080b5c): Camera angle Z for side
//    quadrants (1/3) with battleType!=6 should use cameraAngleSide.Z (0xFFFF6000
//    = -0xA000) from DAT_060acdd8+8, but C++ always copies cameraAngleDefault.Z
//    (0x1E000). Wrong camera angle during side-quadrant berserk attacks.
//
// 5. [HIGH] berserk_createPhantomSlashers (06080b5c): Missing m5C_currentFireIndex
//    random initialization. Ghidra computes MTH_Mul(randomNumber()>>16, numTargets)
//    and stores to m5C. C++ has TODO comment but no implementation -- projectiles
//    always start targeting enemy index 0 instead of a random enemy.
//
// 6. [HIGH] phantomSlasher_createProjectile (06082540): Ghidra uses
//    createSubTaskWithCopy (size 0x204) but C++ uses createSubTask. Task type
//    mismatch may affect task memory layout or copy-chain behavior.
//
// 7. [MEDIUM] berserk_createPhantomSlashers (06080b5c) level==3: Ghidra sets
//    m5E_numTargets to 1 (bytes 0x5E=0, 0x5F=1 big-endian), but C++ sets it
//    to 0. Affects target-count checks in returnPhase1 and damage loop for
//    onslaught variant.
//
// 8. [MEDIUM] berserk_createPhantomSlashers (06080b5c): Init loop zeroes
//    fields 0x60-0x6F (two 32-bit writes per iteration, 2 iterations with
//    stride 8). C++ only partially zeroes m60_hasCameraData[i*4] as single
//    bytes and redundantly zeroes m64. Fields 0x65-0x67 may retain garbage
//    before being set later.
//
// 9. [LOW] berserk_createPhantomSlashers (06080b5c): Missing debug print calls
//    vdp2DebugPrintSetPosition(0xd,0xc) and vdp2PrintfSmallFont("no camera data")
//    in the no-camera-data path. Debug-only, no gameplay impact.
//
// 10. [LOW] projMainModelIndex/projTrailModelIndex tables: C++ declares 4 entries
//     but projectileCountTable only has 3 entries {5,10,15}. Ghidra data at
//     060acd8c has 4 entries {5,10,15,15}. The clamping logic (level>2?2:level)
//     produces correct results but the table size is inconsistent.
//=============================================================================

//=============================================================================
// Phantom Slashers / Wraith Slashers / Onslaught
//=============================================================================

struct sPhantomSlasherProjectile
{
    s32 m0_state;           // 0=pending, 1=has target
    s32 m4_active;
    s8 m8_damageState;      // 0=not hit, 1=hit, 2=returning
    sVec3_FP m10_position;
    sVec3_FP m1C_targetOffset;
    s32 m28_targetIndex;
    // size 0x2C
};

struct sPhantomSlasherTargetEntry
{
    s16 m0_hitFlag;
    s16 m2;
    sVec3_FP m4_direction;
    // size 0x10
};

struct sPhantomSlasherTask : public s_workAreaTemplate<sPhantomSlasherTask>
{
    sPhantomSlasherProjectile* m0_projectiles;
    sPhantomSlasherTargetEntry* m4_targets;
    sVec3_FP m8_cameraPosition;
    sVec3_FP m14_cameraTarget;
    sVec3_FP m20_lookAtTarget;
    sVec3_FP m2C_cameraRotation;
    sVec3_FP m38_cameraAngle;
    sVec3_FP m44;
    s32 m48_springAccel;
    s32 m4C_springVelocity;
    s32 m50_springPosition;   // interpolation factor 0->0x10000
    s16 m54_numProjectiles;
    s8 m56_state;
    s8 m57_level;             // 0=phantom, 1=wraith, 2=onslaught (3=onslaught variant)
    s16 m58;
    s16 m5A_timer;
    s16 m5C_currentFireIndex;
    s16 m5E_numTargets;
    s8 m60_hasCameraData[4];  // actually s32 at offset 0x60 -- nonzero if overlay has camera data
    s8 m64;
    s8 m65;
    s8 m66;
    s8 m67;
    // size 0x68
};

// BTL_A3::06082482
static void phantomSlasher_initVisual()
{
    vdp2Controls.m4_pendingVdp2Regs->m20_BGON &= ~1;
    vdp2Controls.m_isDirty = 1;
    g_fadeControls.m_4D = 6;
    if ((s8)g_fadeControls.m_4C < 7)
    {
        vdp2Controls.m20_registers[1].m112_CLOFSL = 0x4A;
        vdp2Controls.m20_registers[0].m112_CLOFSL = 0x4A;
    }
    fadePalette(&g_fadeControls.m0_fade0, 0xC210, 0x94E2, 10);
    g_fadeControls.m_4D = 5;
}

// BTL_A3::060824ce
static void phantomSlasher_restoreVisual()
{
    vdp2Controls.m4_pendingVdp2Regs->m20_BGON |= 1;
    vdp2Controls.m_isDirty = 1;
    g_fadeControls.m_4D = 6;
    if ((s8)g_fadeControls.m_4C < 7)
    {
        vdp2Controls.m20_registers[1].m112_CLOFSL = 0x4A;
        vdp2Controls.m20_registers[0].m112_CLOFSL = 0x4A;
    }
    fadePalette(&g_fadeControls.m0_fade0, 0x94E2, 0xC210, 10);
    g_fadeControls.m_4D = 5;
}

// Projectile sub-task (size 0x204, def at 060ace18)
// Note: Saturn code uses createSubTaskWithCopy but parent (PhantomSlasherTask) is not a Copy task,
// so the copy fields are undefined. We use a regular task template here.

struct sProjectileTrailEntry
{
    sVec3_FP m0_position;   // 0x00
    sVec3_FP mC_rotation;   // 0x0C
    s_3dModel m18_model;     // 0x18 (Saturn size 0x4C)
    u8 m64_pad[4];           // padding to stride 0x68
    // size 0x68
};

struct sPhantomSlasherProjectileTask : public s_workAreaTemplate<sPhantomSlasherProjectileTask>
{
    u8* m0_rawModelData;                 // 0x00 -- mA9C file data pointer
    u8 m4_pad[4];                        // 0x04
    sVec3_FP m8_velocity;                // 0x08
    sVec3_FP m14_momentum;               // 0x14
    sVec3_FP m20_position;               // 0x20 -- current position
    sVec3_FP m2C_rotAccel;               // 0x2C
    sVec3_FP m38_rotVelocity;            // 0x38
    sVec3_FP m44_rotation;               // 0x44 -- current rotation angles
    s32 m50_homingSpeed;                 // 0x50 -- ramps up to 0x7333
    s32 m54_trackingSpeed;               // 0x54 -- ramps up to 0x2000
    s_3dModel m58_mainModel;             // 0x58 (Saturn size 0x4C, ends at 0xA4)
    u8 mA4_pad[4];                       // 0xA4
    sProjectileTrailEntry mA8_trails[3]; // 0xA8 (3 * 0x68 = 0x138, ends at 0x1E0)
    sPhantomSlasherTask* m1E0_pParent;   // 0x1E0 -- pointer to parent task
    sVec3_FP m1E4_targetPosition;        // 0x1E4
    s16 m1F0_timer;                      // 0x1F0
    s16 m1F2_frameCounter;               // 0x1F2
    u16 m1F4_animTimer;                   // 0x1F4
    u8 m1F6_projectileIndex;             // 0x1F6
    u8 m1F7_subState;                    // 0x1F7
    u8 m1F8_prevDamageState;             // 0x1F8
    u8 m1F9_pad3[3];                     // 0x1F9
    s32 m1FC_targetLost;                 // 0x1FC
    s32 m200_flag;                       // 0x200
    // size 0x204
};

// Model index tables at BTL_A3::060acd9c
static const u16 projMainModelIndex[] = { 0x0004, 0x000C, 0x0014, 0x0014 };   // per level
static const u16 projTrailModelIndex[] = { 0x0008, 0x0010, 0x0018, 0x0018 };   // per level
static const s16 projMainAnimOffset[] = { 0x004C, 0x004C, 0x004C, 0x004C };    // 060acdac
static const s16 projTrailAnimOffset[] = { 0x0050, 0x0050, 0x0050, 0x0050 };   // 060acdb4
static const s16 projMainPoseOffset[] = { 0x0034, 0x003C, 0x0044, 0x0044 };    // 060acdbc + level*4
static const s16 projTrailPoseOffset[] = { 0x0038, 0x0040, 0x0048, 0x0048 };   // 060acdbe + level*4

// Forward declare the draw method set during state 0
static void sPhantomSlasherProjectileTask_Draw(sPhantomSlasherProjectileTask* pThis);

// BTL_A3::06082748 -- projectile fly-to-target state (damageState == 1)
static void phantomSlasherProjectile_flyToTarget(sPhantomSlasherProjectileTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    s_battleDragon* pDragon = gBattleManager->m10_battleOverlay->m18_dragon;
    sPhantomSlasherTask* pParent = pThis->m1E0_pParent;

    sVec3_FP sourcePos;
    if (pParent->m57_level == 3)
    {
        sourcePos = pEngine->mC_battleCenter;
    }
    else
    {
        sourcePos = pDragon->m8_position;
    }

    u8 subState = pThis->m1F7_subState;
    if (subState == 0)
    {
        // Init: set projectile state, compute target offset, random trajectory
        u8 idx = pThis->m1F6_projectileIndex;
        pParent->m0_projectiles[idx].m0_state = 0;

        sVec3_FP targetOffset;
        targetOffset[0] = pParent->m0_projectiles[idx].m1C_targetOffset[0] + sourcePos[0];
        targetOffset[1] = pParent->m0_projectiles[idx].m1C_targetOffset[1] + sourcePos[1];
        targetOffset[2] = pParent->m0_projectiles[idx].m1C_targetOffset[2] + sourcePos[2];

        pThis->m8_velocity = {};
        pThis->m14_momentum = {};
        pThis->m20_position = targetOffset;

        // Random spherical velocity
        u32 rnd1 = randomNumber();
        s32 angle1 = MTH_Mul(rnd1 >> 16, fixedPoint(0x10000000));
        s32 r1 = MTH_Mul(getSin(((u32)angle1 >> 16) & 0xFFF), fixedPoint(0x71c71c));

        u32 rnd2 = randomNumber();
        s32 angle2 = MTH_Mul(rnd2 >> 16, fixedPoint(0x10000000));
        s32 cosR = MTH_Mul(getCos(((u32)angle2 >> 16) & 0xFFF), fixedPoint(0x71c71c));

        u32 rnd3 = randomNumber();
        s32 angle3 = MTH_Mul(rnd3 >> 16, fixedPoint(0x10000000));
        s32 sinCos = MTH_Mul(getSin(((u32)angle3 >> 16) & 0xFFF), cosR);
        s32 cosCos = MTH_Mul(getCos(((u32)angle3 >> 16) & 0xFFF), cosR);

        pThis->m2C_rotAccel = {};
        pThis->m38_rotVelocity = {};
        pThis->m44_rotation = {};

        pThis->m2C_rotAccel[0] += cosCos;
        pThis->m2C_rotAccel[1] += r1;
        pThis->m2C_rotAccel[2] += sinCos;

        // Init main model
        pThis->m0_rawModelData = pEngine->mA9C_berzerkCustomModel;
        u8 level = pParent->m57_level;
        s_fileBundle* pBundle = (s_fileBundle*)pThis->m0_rawModelData;
        s32 initResult = init3DModelRawData(pThis, &pThis->m58_mainModel, 0, pBundle,
            projMainModelIndex[level],
            (sAnimationData*)((u8*)pBundle + projMainAnimOffset[level]),
            (sStaticPoseData*)((u8*)pBundle + projMainPoseOffset[level]),
            nullptr, nullptr);
        if (initResult == 0)
        {
            pThis->getTask()->markFinished();
            return;
        }

        // Init 3 trail models
        for (s32 i = 0; i < 3; i++)
        {
            initResult = init3DModelRawData(pThis, &pThis->mA8_trails[i].m18_model, 0, pBundle,
                projTrailModelIndex[level],
                (sAnimationData*)((u8*)pBundle + projTrailAnimOffset[level]),
                (sStaticPoseData*)((u8*)pBundle + projTrailPoseOffset[level]),
                nullptr, nullptr);
            if (initResult == 0)
            {
                pThis->getTask()->markFinished();
                return;
            }
            stepAnimation(&pThis->mA8_trails[i].m18_model);
        }

        // Set draw method
        pThis->m_DrawMethod = (void(*)(sPhantomSlasherProjectileTask*))&sPhantomSlasherProjectileTask_Draw;
        playSystemSoundEffect(0x11);
        pThis->m1F4_animTimer = 0; // m1F4 timer
        pThis->m1F7_subState++;
    }
    else if (subState == 2)
    {
        // State 2: coasting with rotation update
        pThis->m20_position += pEngine->m1A0_battleAutoScrollDelta;

        pThis->m38_rotVelocity[0] += pThis->m2C_rotAccel[0];
        pThis->m38_rotVelocity[1] += pThis->m2C_rotAccel[1];
        pThis->m38_rotVelocity[2] += pThis->m2C_rotAccel[2];

        // Damping (MTH_Mul with 0 = no damping -- Ghidra shows literal 0)
        sVec3_FP damp;
        damp[0] = MTH_Mul(pThis->m38_rotVelocity[0], fixedPoint(0));
        damp[1] = MTH_Mul(pThis->m38_rotVelocity[1], fixedPoint(0));
        damp[2] = MTH_Mul(pThis->m38_rotVelocity[2], fixedPoint(0));
        pThis->m38_rotVelocity[0] -= damp[0];
        pThis->m38_rotVelocity[1] -= damp[1];
        pThis->m38_rotVelocity[2] -= damp[2];

        pThis->m44_rotation[0] += pThis->m38_rotVelocity[0];
        pThis->m44_rotation[1] += pThis->m38_rotVelocity[1];
        pThis->m44_rotation[2] += pThis->m38_rotVelocity[2];
        pThis->m44_rotation[0] = (s32)pThis->m44_rotation[0] & 0xFFFFFFF;
        pThis->m44_rotation[1] = (s32)pThis->m44_rotation[1] & 0xFFFFFFF;
        pThis->m44_rotation[2] = (s32)pThis->m44_rotation[2] & 0xFFFFFFF;
        pThis->m2C_rotAccel = {};

        goto stepMainAnim;
    }
    else if (subState != 1)
    {
        return;
    }

    // State 1 (and fallthrough from state 0): animate and advance
    {
        u16 animTimer = pThis->m1F4_animTimer;
        pThis->m1F4_animTimer = animTimer + 1;

        pThis->m20_position += pEngine->m1A0_battleAutoScrollDelta;

        pThis->m38_rotVelocity[0] += pThis->m2C_rotAccel[0];
        pThis->m38_rotVelocity[1] += pThis->m2C_rotAccel[1];
        pThis->m38_rotVelocity[2] += pThis->m2C_rotAccel[2];

        sVec3_FP damp;
        damp[0] = MTH_Mul(pThis->m38_rotVelocity[0], fixedPoint(0));
        damp[1] = MTH_Mul(pThis->m38_rotVelocity[1], fixedPoint(0));
        damp[2] = MTH_Mul(pThis->m38_rotVelocity[2], fixedPoint(0));
        pThis->m38_rotVelocity[0] -= damp[0];
        pThis->m38_rotVelocity[1] -= damp[1];
        pThis->m38_rotVelocity[2] -= damp[2];

        pThis->m44_rotation[0] += pThis->m38_rotVelocity[0];
        pThis->m44_rotation[1] += pThis->m38_rotVelocity[1];
        pThis->m44_rotation[2] += pThis->m38_rotVelocity[2];
        pThis->m44_rotation[0] = (s32)pThis->m44_rotation[0] & 0xFFFFFFF;
        pThis->m44_rotation[1] = (s32)pThis->m44_rotation[1] & 0xFFFFFFF;
        pThis->m44_rotation[2] = (s32)pThis->m44_rotation[2] & 0xFFFFFFF;
        pThis->m2C_rotAccel = {};

        // Check if animation complete
        s32 totalFrames = 0;
        if (pThis->m58_mainModel.m30_pCurrentAnimation != nullptr)
        {
            totalFrames = (s32)*(s16*)((u8*)pThis->m58_mainModel.m30_pCurrentAnimation + 4);
        }
        if (totalFrames <= (s32)(u32)pThis->m1F4_animTimer)
        {
            // Re-init animation and mark projectile as hit (m0_state = 1)
            u8 level = pParent->m57_level;
            initAnimation(&pThis->m58_mainModel,
                (sAnimationData*)((u8*)pThis->m0_rawModelData + projTrailAnimOffset[level]));
            pThis->m1F7_subState++;
            pParent->m0_projectiles[pThis->m1F6_projectileIndex].m0_state = 1;
            return;
        }
    }

stepMainAnim:
    stepAnimation(&pThis->m58_mainModel);
}

// BTL_A3::06082d94 -- projectile return to target (damageState == 2, subState < 2)
static void phantomSlasherProjectile_returnPhase1(sPhantomSlasherProjectileTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    sPhantomSlasherTask* pParent = pThis->m1E0_pParent;
    sBattleOverlayTask_C* pTargetSystem = gBattleManager->m10_battleOverlay->mC_targetSystem;

    pThis->m20_position += pEngine->m1A0_battleAutoScrollDelta;
    stepAnimation(&pThis->m58_mainModel);

    s16 targetIdx = (s16)pParent->m0_projectiles[pThis->m1F6_projectileIndex].m28_targetIndex;
    u8 subState = pThis->m1F7_subState;

    if (subState == 0)
    {
        // Init homing: compute random offset from position, set speed ramps
        pThis->m50_homingSpeed = 0x7AE;
        pThis->m54_trackingSpeed = 0x3D7;

        u32 rnd = randomNumber();
        s32 angle = MTH_Mul(rnd >> 16, fixedPoint(0x10000000));
        u32 angleIdx = ((u32)angle >> 16) & 0xFFF;

        s8 quadrant = pEngine->m22C_dragonCurrentQuadrant;
        sVec3_FP offset = {};
        if (quadrant == 0 || quadrant == 2)
        {
            offset[0] = MTH_Mul(getCos(angleIdx), fixedPoint(0x6000));
            offset[1] = MTH_Mul(getSin(angleIdx), fixedPoint(0x6000));
        }
        else // quadrant 1 or 3
        {
            offset[2] = MTH_Mul(getCos(angleIdx), fixedPoint(0x6000));
            offset[1] = MTH_Mul(getSin(angleIdx), fixedPoint(0x6000));
        }

        pThis->m8_velocity[0] += offset[0];
        pThis->m8_velocity[1] += offset[1];
        pThis->m8_velocity[2] += offset[2];

        // Set target position (look-at from parent)
        if (pParent->m57_level == 3)
        {
            pThis->m1E4_targetPosition = pParent->m20_lookAtTarget;
        }
        else if (pParent->m5E_numTargets == 0 ||
            pTargetSystem->m0_enemyTargetables[targetIdx] == nullptr ||
            pTargetSystem->m0_enemyTargetables[targetIdx]->m4_targetable == nullptr ||
            (pTargetSystem->m0_enemyTargetables[targetIdx]->m4_targetable->m50_flags & 0x140003) != 0 ||
            pTargetSystem->m0_enemyTargetables[targetIdx]->m0_isActive < 1)
        {
            pThis->m1E4_targetPosition = pParent->m20_lookAtTarget;
        }

        // Init trail positions
        for (s32 i = 0; i < 3; i++)
        {
            pThis->mA8_trails[i].m0_position = pThis->m20_position;
            pThis->mA8_trails[i].mC_rotation = pThis->m44_rotation;
            pThis->mA8_trails[i].m0_position -= pEngine->m1A0_battleAutoScrollDelta;
        }

        pThis->m200_flag = 1;
        pThis->m1F7_subState++;
        playSystemSoundEffect(0x14);
        pThis->m1F0_timer = 0x4B;
    }
    else if (subState != 1)
    {
        return;
    }

    // State 1: homing toward target
    s16 timer = pThis->m1F0_timer;
    pThis->m1F0_timer = timer - 1;
    if (timer < 0)
    {
        pThis->m1F7_subState++;
        return;
    }

    // Update target position (track live enemy or use auto-scroll)
    if (pParent->m57_level == 3)
    {
        pThis->m1E4_targetPosition = pParent->m20_lookAtTarget;
    }
    else
    {
        if (pParent->m5E_numTargets == 0 ||
            pTargetSystem->m0_enemyTargetables[targetIdx] == nullptr ||
            pTargetSystem->m0_enemyTargetables[targetIdx]->m4_targetable == nullptr ||
            (pTargetSystem->m0_enemyTargetables[targetIdx]->m4_targetable->m50_flags & 0x140003) != 0 ||
            pTargetSystem->m0_enemyTargetables[targetIdx]->m0_isActive < 1)
        {
            pThis->m1FC_targetLost = 1;
        }

        if (pThis->m1FC_targetLost == 0)
        {
            sVec3_FP* enemyPos = getBattleTargetablePosition(*pTargetSystem->m0_enemyTargetables[targetIdx]->m4_targetable);
            pThis->m1E4_targetPosition = *enemyPos;
        }
        else
        {
            pThis->m1E4_targetPosition += pEngine->m1A0_battleAutoScrollDelta;
        }
    }

    // Ramp up speeds
    if (pThis->m50_homingSpeed < 0x7333)
        pThis->m50_homingSpeed += 0x1AE;
    if (pThis->m54_trackingSpeed < 0x2000)
        pThis->m54_trackingSpeed += 0x70;

    // Homing: accelerate toward target
    sVec3_FP toTarget;
    toTarget[0] = MTH_Mul(pThis->m1E4_targetPosition[0] - pThis->m20_position[0], fixedPoint(pThis->m54_trackingSpeed));
    toTarget[1] = MTH_Mul(pThis->m1E4_targetPosition[1] - pThis->m20_position[1], fixedPoint(pThis->m54_trackingSpeed));
    toTarget[2] = MTH_Mul(pThis->m1E4_targetPosition[2] - pThis->m20_position[2], fixedPoint(pThis->m54_trackingSpeed));

    pThis->m8_velocity[0] += toTarget[0];
    pThis->m8_velocity[1] += toTarget[1];
    pThis->m8_velocity[2] += toTarget[2];

    pThis->m14_momentum[0] += pThis->m8_velocity[0];
    pThis->m14_momentum[1] += pThis->m8_velocity[1];
    pThis->m14_momentum[2] += pThis->m8_velocity[2];

    // Damping
    sVec3_FP damp;
    damp[0] = MTH_Mul(pThis->m14_momentum[0], fixedPoint(pThis->m50_homingSpeed));
    damp[1] = MTH_Mul(pThis->m14_momentum[1], fixedPoint(pThis->m50_homingSpeed));
    damp[2] = MTH_Mul(pThis->m14_momentum[2], fixedPoint(pThis->m50_homingSpeed));

    pThis->m14_momentum[0] -= damp[0];
    pThis->m14_momentum[1] -= damp[1];
    pThis->m14_momentum[2] -= damp[2];

    pThis->m20_position[0] += pThis->m14_momentum[0];
    pThis->m20_position[1] += pThis->m14_momentum[1];
    pThis->m20_position[2] += pThis->m14_momentum[2];

    pThis->m8_velocity = {};

    // Rotation: atan2-based orientation tracking
    s8 quadrant = pEngine->m22C_dragonCurrentQuadrant;
    s32 targetRotX = 0, targetRotY = 0, targetRotZ = 0;
    if (quadrant == 0 || quadrant == 2)
    {
        targetRotZ = atan2_FP((s32)pThis->m14_momentum[1], (s32)pThis->m14_momentum[0]);
    }
    else
    {
        targetRotX = atan2_FP((s32)pThis->m14_momentum[1], (s32)pThis->m14_momentum[2]);
    }

    // Spring-damped rotation tracking
    s32 dRotX = signExtend28(targetRotX - (s32)pThis->m44_rotation[0]);
    s32 dRotY = signExtend28(targetRotY - (s32)pThis->m44_rotation[1]);
    s32 dRotZ = signExtend28(targetRotZ - (s32)pThis->m44_rotation[2]);

    pThis->m2C_rotAccel[0] += MTH_Mul(fixedPoint(dRotX), fixedPoint(0x14CCC));
    pThis->m2C_rotAccel[1] += MTH_Mul(fixedPoint(dRotY), fixedPoint(0x14CCC));
    pThis->m2C_rotAccel[2] += MTH_Mul(fixedPoint(dRotZ), fixedPoint(0x14CCC));

    pThis->m38_rotVelocity[0] += pThis->m2C_rotAccel[0];
    pThis->m38_rotVelocity[1] += pThis->m2C_rotAccel[1];
    pThis->m38_rotVelocity[2] += pThis->m2C_rotAccel[2];

    damp[0] = MTH_Mul(pThis->m38_rotVelocity[0], fixedPoint(0xB333));
    damp[1] = MTH_Mul(pThis->m38_rotVelocity[1], fixedPoint(0xB333));
    damp[2] = MTH_Mul(pThis->m38_rotVelocity[2], fixedPoint(0xB333));

    pThis->m38_rotVelocity[0] -= damp[0];
    pThis->m38_rotVelocity[1] -= damp[1];
    pThis->m38_rotVelocity[2] -= damp[2];

    pThis->m44_rotation[0] += pThis->m38_rotVelocity[0];
    pThis->m44_rotation[1] += pThis->m38_rotVelocity[1];
    pThis->m44_rotation[2] += pThis->m38_rotVelocity[2];
    pThis->m44_rotation[0] = (s32)pThis->m44_rotation[0] & 0xFFFFFFF;
    pThis->m44_rotation[1] = (s32)pThis->m44_rotation[1] & 0xFFFFFFF;
    pThis->m44_rotation[2] = (s32)pThis->m44_rotation[2] & 0xFFFFFFF;
    pThis->m2C_rotAccel = {};

    // Collision check: distance squared to target
    sVec3_FP diff;
    diff[0] = pThis->m20_position[0] - pThis->m1E4_targetPosition[0];
    diff[1] = pThis->m20_position[1] - pThis->m1E4_targetPosition[1];
    diff[2] = pThis->m20_position[2] - pThis->m1E4_targetPosition[2];

    s32 distSq = MTH_Mul(diff[0], diff[0]) + MTH_Mul(diff[1], diff[1]) + MTH_Mul(diff[2], diff[2]);
    s32 threshold = MTH_Mul(fixedPoint(0x5000), fixedPoint(0x5000));

    if (distSq < threshold)
    {
        // Hit! Increment hit counter on parent target entry
        pParent->m4_targets[targetIdx].m0_hitFlag++;
        // Store velocity direction for damage application
        pParent->m4_targets[targetIdx].m4_direction = pThis->m14_momentum;

        berserk_createHitFlashFade(pThis, 0x4A, 0x94E2, 0xE7F0, 5, 0xE7F0, 0x94E2, 5);
        pThis->m1F7_subState++;
    }
}

// BTL_A3::06083606 -- projectile fade-out after hit (damageState == 2, subState >= 2)
static void phantomSlasherProjectile_returnPhase2(sPhantomSlasherProjectileTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;

    pThis->m20_position += pEngine->m1A0_battleAutoScrollDelta;
    stepAnimation(&pThis->m58_mainModel);

    u8 subState = pThis->m1F7_subState;
    if (subState == 2)
    {
        pThis->m1F0_timer = 0x2D;
        pThis->m1F7_subState++;
    }
    else if (subState != 3)
    {
        return;
    }

    // Rotation tracking (same as phase 1)
    s8 quadrant = pEngine->m22C_dragonCurrentQuadrant;
    s32 targetRotX = 0, targetRotY = 0, targetRotZ = 0;
    if (quadrant == 0 || quadrant == 2)
    {
        targetRotZ = atan2_FP((s32)pThis->m14_momentum[1], (s32)pThis->m14_momentum[0]);
    }
    else
    {
        targetRotX = atan2_FP((s32)pThis->m14_momentum[1], (s32)pThis->m14_momentum[2]);
    }

    s32 dRotX = signExtend28(targetRotX - (s32)pThis->m44_rotation[0]);
    s32 dRotY = signExtend28(targetRotY - (s32)pThis->m44_rotation[1]);
    s32 dRotZ = signExtend28(targetRotZ - (s32)pThis->m44_rotation[2]);

    pThis->m2C_rotAccel[0] += MTH_Mul(fixedPoint(dRotX), fixedPoint(0x14CCC));
    pThis->m2C_rotAccel[1] += MTH_Mul(fixedPoint(dRotY), fixedPoint(0x14CCC));
    pThis->m2C_rotAccel[2] += MTH_Mul(fixedPoint(dRotZ), fixedPoint(0x14CCC));

    pThis->m38_rotVelocity[0] += pThis->m2C_rotAccel[0];
    pThis->m38_rotVelocity[1] += pThis->m2C_rotAccel[1];
    pThis->m38_rotVelocity[2] += pThis->m2C_rotAccel[2];

    sVec3_FP damp;
    damp[0] = MTH_Mul(pThis->m38_rotVelocity[0], fixedPoint(0xB333));
    damp[1] = MTH_Mul(pThis->m38_rotVelocity[1], fixedPoint(0xB333));
    damp[2] = MTH_Mul(pThis->m38_rotVelocity[2], fixedPoint(0xB333));

    pThis->m38_rotVelocity[0] -= damp[0];
    pThis->m38_rotVelocity[1] -= damp[1];
    pThis->m38_rotVelocity[2] -= damp[2];

    pThis->m44_rotation[0] += pThis->m38_rotVelocity[0];
    pThis->m44_rotation[1] += pThis->m38_rotVelocity[1];
    pThis->m44_rotation[2] += pThis->m38_rotVelocity[2];
    pThis->m44_rotation[0] = (s32)pThis->m44_rotation[0] & 0xFFFFFFF;
    pThis->m44_rotation[1] = (s32)pThis->m44_rotation[1] & 0xFFFFFFF;
    pThis->m44_rotation[2] = (s32)pThis->m44_rotation[2] & 0xFFFFFFF;
    pThis->m2C_rotAccel = {};

    // Momentum decay
    pThis->m14_momentum[0] += pThis->m8_velocity[0];
    pThis->m14_momentum[1] += pThis->m8_velocity[1];
    pThis->m14_momentum[2] += pThis->m8_velocity[2];

    sVec3_FP momDamp;
    momDamp[0] = MTH_Mul(pThis->m14_momentum[0], fixedPoint(0));
    momDamp[1] = MTH_Mul(pThis->m14_momentum[1], fixedPoint(0));
    momDamp[2] = MTH_Mul(pThis->m14_momentum[2], fixedPoint(0));

    pThis->m14_momentum[0] -= momDamp[0];
    pThis->m14_momentum[1] -= momDamp[1];
    pThis->m14_momentum[2] -= momDamp[2];

    pThis->m20_position[0] += pThis->m14_momentum[0];
    pThis->m20_position[1] += pThis->m14_momentum[1];
    pThis->m20_position[2] += pThis->m14_momentum[2];
    pThis->m8_velocity = {};

    // Timer countdown -- destroy task when done
    s16 timer = pThis->m1F0_timer - 1;
    pThis->m1F0_timer = timer;
    if (timer < 0)
    {
        pThis->getTask()->markFinished();
    }
}

// BTL_A3::06083a10 -- update trail positions (runs after damageState 2 handlers)
static void phantomSlasherProjectile_updateTrails(sPhantomSlasherProjectileTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;

    for (u32 i = 0; i < 3; i++)
    {
        if (((u32)pThis->m1F2_frameCounter & 3) == i)
        {
            // Trail catches up to current position
            pThis->mA8_trails[i].m0_position = pThis->m20_position;
            pThis->mA8_trails[i].mC_rotation = pThis->m44_rotation;
            setAnimationFrame(&pThis->mA8_trails[i].m18_model, (s32)pThis->m1F4_animTimer);
        }
        else
        {
            // Trail drifts with auto-scroll
            pThis->mA8_trails[i].m0_position += pEngine->m1A0_battleAutoScrollDelta;
        }
    }
}

// BTL_A3::06082634 -- draw method for projectile (set dynamically in state 0)
static void sPhantomSlasherProjectileTask_Draw(sPhantomSlasherProjectileTask* pThis)
{
    // Draw main model
    pushCurrentMatrix();
    translateCurrentMatrix(&pThis->m20_position);
    rotateCurrentMatrixYXZ(&pThis->m44_rotation);
    pThis->m58_mainModel.m18_drawFunction(&pThis->m58_mainModel);
    popMatrix();

    // Draw trail models (only after return phase starts)
    if (pThis->m200_flag != 0)
    {
        for (s32 i = 0; i < 3; i++)
        {
            pushCurrentMatrix();
            translateCurrentMatrix(&pThis->mA8_trails[i].m0_position);
            rotateCurrentMatrixYXZ(&pThis->mA8_trails[i].mC_rotation);
            pThis->mA8_trails[i].m18_model.m18_drawFunction(&pThis->mA8_trails[i].m18_model);
            popMatrix();
        }
    }
}

// BTL_A3::06082578
static void sPhantomSlasherProjectileTask_Update(sPhantomSlasherProjectileTask* pThis)
{
    sPhantomSlasherTask* pParent = pThis->m1E0_pParent;
    s8 damageState = pParent->m0_projectiles[(u32)pThis->m1F6_projectileIndex].m8_damageState;
    if (damageState != 0)
    {
        if (damageState == 1)
        {
            if (pThis->m1F8_prevDamageState != 1)
            {
                pThis->m1F7_subState = 0;
            }
            phantomSlasherProjectile_flyToTarget(pThis);
        }
        else if (damageState == 2)
        {
            if (pThis->m1F8_prevDamageState != 2)
            {
                pThis->m1F7_subState = 0;
            }
            if (pThis->m1F7_subState < 2)
            {
                phantomSlasherProjectile_returnPhase1(pThis);
            }
            else
            {
                phantomSlasherProjectile_returnPhase2(pThis);
            }
            phantomSlasherProjectile_updateTrails(pThis);
        }
    }
    pThis->m1F8_prevDamageState = pParent->m0_projectiles[(u32)pThis->m1F6_projectileIndex].m8_damageState;
    pThis->m1F2_frameCounter++;
}

// BTL_A3::0608272e
static void sPhantomSlasherProjectileTask_Delete(sPhantomSlasherProjectileTask* pThis)
{
    pThis->m1E0_pParent->m0_projectiles[(u32)pThis->m1F6_projectileIndex].m4_active = 0;
}

// BTL_A3::06082540 -- create individual projectile sub-task
static s32 phantomSlasher_createProjectile(sPhantomSlasherTask* pThis, u32 index)
{
    static const sPhantomSlasherProjectileTask::TypedTaskDefinition definition = {
        nullptr,
        (void(*)(sPhantomSlasherProjectileTask*))&sPhantomSlasherProjectileTask_Update,
        nullptr,
        (void(*)(sPhantomSlasherProjectileTask*))&sPhantomSlasherProjectileTask_Delete,
    };

    sPhantomSlasherProjectileTask* pNewTask = createSubTask<sPhantomSlasherProjectileTask>(pThis, &definition);
    if (pNewTask == nullptr)
    {
        return 0;
    }
    pNewTask->m1F6_projectileIndex = (u8)index;
    pNewTask->m1E0_pParent = pThis;
    pNewTask->m200_flag = 0;
    pNewTask->m1F2_frameCounter = 0;
    return 1;
}

// BTL_A3::0608156e
static void sPhantomSlasherTask_Update(sPhantomSlasherTask* pThis);

// BTL_A3::06082440
static void sPhantomSlasherTask_Delete(sPhantomSlasherTask* pThis)
{
    if (*(s32*)pThis->m60_hasCameraData != 0)
    {
        battleEngine_restoreCameraAfterEnemyAttack();
    }
    if (pThis->m64 != 0)
    {
        phantomSlasher_restoreVisual();
    }
    gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished = 1;
}

// BTL_A3::06080b5c
void berserk_createPhantomSlashers(s_battleEngine* pThis, u8 level)
{
    static const sPhantomSlasherTask::TypedTaskDefinition definition = {
        nullptr,
        &sPhantomSlasherTask_Update,
        nullptr,
        &sPhantomSlasherTask_Delete,
    };

    static const s16 projectileCountTable[] = { 5, 10, 15 }; // BTL_A3::060acd8c

    sPhantomSlasherTask* pNewTask = createSubTask<sPhantomSlasherTask>(pThis, &definition);
    if (pNewTask == nullptr)
    {
        pThis->m188_flags.m100_attackAnimationFinished = 1;
        return;
    }

    // Zero target tracking
    for (s32 i = 0; i < 2; i++)
    {
        pNewTask->m60_hasCameraData[i * 4] = 0;
        pNewTask->m64 = 0; // m64[i*4] in the original
    }

    s_battleDragon* pDragon = gBattleManager->m10_battleOverlay->m18_dragon;
    sVec3_FP sourcePos;

    if (level == 3) // onslaught targeting player
    {
        pNewTask->m5E_numTargets = 0;
        pNewTask->m67 = 1;
        sourcePos = pThis->mC_battleCenter;
    }
    else
    {
        pNewTask->m5E_numTargets = gBattleManager->m10_battleOverlay->mC_targetSystem->m20A_numSelectableEnemies;
        sourcePos = pDragon->m8_position;
    }

    // Zero velocity
    pNewTask->m44 = {};

    // Projectile count
    pNewTask->m54_numProjectiles = projectileCountTable[level > 2 ? 2 : level];

    // Allocate projectile array
    pNewTask->m0_projectiles = (sPhantomSlasherProjectile*)allocateHeapForTask(
        pNewTask, pNewTask->m54_numProjectiles * sizeof(sPhantomSlasherProjectile));
    if (pNewTask->m0_projectiles == nullptr)
    {
        pNewTask->getTask()->markFinished();
        return;
    }

    // Allocate target array if needed
    if (pNewTask->m5E_numTargets > 0)
    {
        pNewTask->m4_targets = (sPhantomSlasherTargetEntry*)allocateHeapForTask(
            pNewTask, pNewTask->m5E_numTargets * sizeof(sPhantomSlasherTargetEntry));
        if (pNewTask->m4_targets == nullptr)
        {
            pNewTask->getTask()->markFinished();
            return;
        }
        for (s32 i = 0; i < pNewTask->m5E_numTargets; i++)
        {
            pNewTask->m4_targets[i].m0_hitFlag = 0;
        }

        // Randomize initial target
        u32 rnd = randomNumber();
        // TODO: m5C_currentFireIndex = MTH_Mul(rnd >> 16, numTargets) -- random starting target
    }

    // Init first projectile targeting
    if (pNewTask->m54_numProjectiles != 0)
    {
        sBattleOverlayTask_C* pTargetSystem = gBattleManager->m10_battleOverlay->mC_targetSystem;

        if (level == 3)
        {
            // Onslaught: target dragon
            pNewTask->m0_projectiles[0].m4_active = 1;
            pNewTask->m0_projectiles[0].m0_state = 0;
            pNewTask->m0_projectiles[0].m10_position = pDragon->m8_position;
        }
        else
        {
            s32 targetIdx = pNewTask->m0_projectiles[0].m28_targetIndex;
            s_battleEnemy* enemy = pTargetSystem->m0_enemyTargetables[targetIdx];
            if (enemy->m4_targetable == nullptr ||
                (enemy->m4_targetable->m50_flags & 0x140003) != 0 ||
                enemy->m0_isActive < 1)
            {
                // No valid target
                pNewTask->m0_projectiles[0].m4_active = 0;
                pNewTask->m0_projectiles[0].m0_state = 0;
                pNewTask->m0_projectiles[0].m10_position = pThis->mC_battleCenter;
            }
            else
            {
                pNewTask->m0_projectiles[0].m4_active = 1;
                pNewTask->m0_projectiles[0].m0_state = 0;
                if (pNewTask->m5E_numTargets > 0)
                {
                    sVec3_FP* enemyPos = getBattleTargetablePosition(*enemy->m4_targetable);
                    pNewTask->m0_projectiles[0].m10_position = *enemyPos;
                }
                else
                {
                    pNewTask->m0_projectiles[0].m10_position = pThis->mC_battleCenter;
                }
            }
        }
        randomNumber(); // consumed for initial angle
    }

    // Camera rotation based on quadrant
    static const sVec3_FP cameraAngleDefault(0, 0x3000, 0x1E000); // BTL_A3::060acdcc
    static const sVec3_FP cameraAngleSide(0, 0x3000, 0);          // BTL_A3::060acdd8

    if (level == 3)
    {
        pNewTask->m38_cameraAngle = cameraAngleDefault;
    }
    else
    {
        s8 quadrant = pThis->m22C_dragonCurrentQuadrant;
        if (quadrant == 0 || quadrant == 2)
        {
            pNewTask->m38_cameraAngle = cameraAngleDefault;
        }
        else
        {
            s8 battleType = getBattleTypeId();
            if (battleType == 6)
                pNewTask->m38_cameraAngle = sVec3_FP(0, 0x3000, 0);
            else
                pNewTask->m38_cameraAngle = sVec3_FP(0, 0x3000, 0);
            pNewTask->m38_cameraAngle.m8_Z = cameraAngleDefault.m8_Z; // TODO: use correct table entry
        }
    }

    pNewTask->m2C_cameraRotation = {};
    s8 quadrant = pThis->m22C_dragonCurrentQuadrant;
    switch (quadrant)
    {
    case 0: pNewTask->m2C_cameraRotation.m4_Y = 0x38e38e; break;
    case 1: pNewTask->m2C_cameraRotation.m4_Y = 0x438e38e; break;
    case 2: pNewTask->m2C_cameraRotation.m4_Y = 0x8aaaaaa; break;
    case 3: pNewTask->m2C_cameraRotation.m4_Y = 0xc38e38e; break;
    }

    // Camera interpolation
    battleEngineSub1_UpdateSub2(&pNewTask->m8_cameraPosition, sourcePos,
        pNewTask->m38_cameraAngle, pNewTask->m2C_cameraRotation);
    pNewTask->m14_cameraTarget = sourcePos;

    // Check overlay camera data
    sSaturnPtr cameraData = getOverlayCameraData(pThis);
    if (cameraData.m_offset == 0)
    {
        pNewTask->m56_state = 0;
        pNewTask->m57_level = level;
        pNewTask->m58 = 0;
        phantomSlasher_initVisual();
        pNewTask->m64 = 0;
        pNewTask->m65 = 0;
        pNewTask->m66 = 0;
        pNewTask->m67 = 1;
        return;
    }

    // Has camera data -- compute initial camera distance squared
    sVec3_FP camOffset;
    if (level == 3)
    {
        s8 invQuadrant = (3 - pThis->m22C_dragonCurrentQuadrant);
        camOffset = readOverlayCameraOffset(pThis, invQuadrant);
    }
    else
    {
        camOffset = readOverlayCameraOffset(pThis, pThis->m22C_dragonCurrentQuadrant);
    }
    // m60 = distance squared (nonzero = has camera data)
    *(s32*)pNewTask->m60_hasCameraData = MTH_Mul(camOffset[0], camOffset[0]); // simplified

    pNewTask->m56_state = 0;
    pNewTask->m57_level = level;
    pNewTask->m58 = 0;
    phantomSlasher_initVisual();
    pNewTask->m64 = 0;
    pNewTask->m65 = 0;
    pNewTask->m66 = 0;
    pNewTask->m67 = 1;
}

// BTL_A3::0608156e
static void sPhantomSlasherTask_Update(sPhantomSlasherTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    s_battleDragon* pDragon = gBattleManager->m10_battleOverlay->m18_dragon;
    sBattleOverlayTask_C* pTargetSystem = gBattleManager->m10_battleOverlay->mC_targetSystem;

    sVec3_FP sourcePos, targetPos;
    if (pThis->m57_level == 3)
    {
        sourcePos = pEngine->mC_battleCenter;
        targetPos = pDragon->m8_position;
    }
    else
    {
        sourcePos = pDragon->m8_position;
        targetPos = pEngine->mC_battleCenter;
    }

    s8 state = pThis->m56_state;

    if (state == 0)
    {
        // State 0: init projectiles, camera setup, load models
        battleEngineSub1_UpdateSub2(&pThis->m8_cameraPosition, sourcePos,
            pThis->m38_cameraAngle, pThis->m2C_cameraRotation);
        pThis->m14_cameraTarget = sourcePos;

        // Wait for file loading to complete
        if (fileInfoStruct.m2C_allocatedHead != nullptr)
            goto updateEnd;

        // Create projectile sub-tasks
        for (u32 i = 0; i < (u32)pThis->m54_numProjectiles; i++)
        {
            if (pThis->m0_projectiles[i].m4_active == 1)
            {
                if (!phantomSlasher_createProjectile(pThis, i))
                {
                    pThis->getTask()->markFinished();
                    return;
                }
            }
        }

        // Find first active projectile
        pThis->m5C_currentFireIndex = 0;
        for (s32 i = 0; i < pThis->m54_numProjectiles; i++)
        {
            if (pThis->m0_projectiles[pThis->m5C_currentFireIndex].m0_state != 0)
                break;
            pThis->m5C_currentFireIndex++;
        }

        if (pThis->m5C_currentFireIndex == pThis->m54_numProjectiles)
        {
            pThis->getTask()->markFinished();
            return;
        }

        // Set camera target to first projectile's target position
        pThis->m14_cameraTarget[0] = sourcePos[0] + pThis->m0_projectiles[pThis->m5C_currentFireIndex].m1C_targetOffset[0];
        pThis->m14_cameraTarget[1] = sourcePos[1] + pThis->m0_projectiles[pThis->m5C_currentFireIndex].m1C_targetOffset[1];
        pThis->m14_cameraTarget[2] = sourcePos[2] + pThis->m0_projectiles[pThis->m5C_currentFireIndex].m1C_targetOffset[2];

        pThis->m5A_timer = 8;
        pThis->m56_state++;
    }

    if (state == 0 || state == 1)
    {
        // State 1: fire projectiles one by one
        // Camera interpolation
        battleEngineSub1_UpdateSub2(&pThis->m8_cameraPosition, sourcePos,
            pThis->m38_cameraAngle, pThis->m2C_cameraRotation);

        // Add auto-scroll delta
        pThis->m14_cameraTarget += pEngine->m1A0_battleAutoScrollDelta;

        // Fire next projectile when timer expires
        if (pThis->m5C_currentFireIndex < pThis->m54_numProjectiles)
        {
            pThis->m5A_timer--;
            if (pThis->m5A_timer < 0)
            {
                if (pThis->m0_projectiles[pThis->m5C_currentFireIndex].m4_active == 1)
                {
                    pThis->m0_projectiles[pThis->m5C_currentFireIndex].m8_damageState = 1;
                    pThis->m14_cameraTarget[0] = sourcePos[0] + pThis->m0_projectiles[pThis->m5C_currentFireIndex].m1C_targetOffset[0];
                    pThis->m14_cameraTarget[1] = sourcePos[1] + pThis->m0_projectiles[pThis->m5C_currentFireIndex].m1C_targetOffset[1];
                    pThis->m14_cameraTarget[2] = sourcePos[2] + pThis->m0_projectiles[pThis->m5C_currentFireIndex].m1C_targetOffset[2];
                }
                pThis->m5C_currentFireIndex++;
                pThis->m5A_timer = 8;
            }
        }

        // Count completed projectiles
        u32 doneCount = 0;
        for (s32 i = 0; i < pThis->m54_numProjectiles; i++)
        {
            if (pThis->m0_projectiles[i].m0_state == 1 || pThis->m0_projectiles[i].m4_active == 0)
                doneCount++;
        }

        // All projectiles done -> advance to state 2
        if (doneCount == (u32)pThis->m54_numProjectiles)
        {
            pThis->m5A_timer = 0x16;
            pThis->m56_state = 2;
        }
    }
    else if (state == 2)
    {
        // State 2: zoom out, update auto-scroll
        pThis->m14_cameraTarget += pEngine->m1A0_battleAutoScrollDelta;

        battleEngineSub1_UpdateSub2(&pThis->m8_cameraPosition, sourcePos,
            pThis->m38_cameraAngle, pThis->m2C_cameraRotation);

        pThis->m5A_timer--;
        if (pThis->m5A_timer < 0)
        {
            pThis->m5A_timer = 9;
            pThis->m2C_cameraRotation[1] -= 0xAAAAAA;

            if (*(s32*)pThis->m60_hasCameraData != 0)
            {
                pThis->m14_cameraTarget = sourcePos;
                sVec3_FP camOffset = readOverlayCameraOffset(pEngine, pEngine->m22C_dragonCurrentQuadrant);
                pThis->m8_cameraPosition = targetPos + camOffset;
                battleEngine_resetCameraInterpolation();
            }
            pThis->m56_state++;
        }
    }
    else if (state == 3)
    {
        // State 3: camera transition with spring damper
        if (*(s32*)pThis->m60_hasCameraData != 0)
        {
            pThis->m14_cameraTarget += pEngine->m1A0_battleAutoScrollDelta;
            pThis->m8_cameraPosition += pEngine->m1A0_battleAutoScrollDelta;
            battleEngine_resetCameraInterpolation();
        }

        pThis->m5A_timer--;
        if (pThis->m5A_timer < 0)
        {
            pThis->m56_state++;
            pThis->m5C_currentFireIndex = 0;
            pThis->m5A_timer = 3;
        }
    }
    else if (state == 4)
    {
        // State 4: camera lerp to final position + trigger damage
        if (pThis->m5C_currentFireIndex < pThis->m54_numProjectiles)
        {
            pThis->m5A_timer--;
            if (pThis->m5A_timer < 0)
            {
                if (pThis->m0_projectiles[pThis->m5C_currentFireIndex].m4_active == 1)
                {
                    pThis->m0_projectiles[pThis->m5C_currentFireIndex].m8_damageState = 2;
                }
                pThis->m5C_currentFireIndex++;
                pThis->m5A_timer = 3;
            }
        }

        if (*(s32*)pThis->m60_hasCameraData != 0)
        {
            // Spring-damper camera interpolation
            pThis->m48_springAccel += MTH_Mul(0x10000 - pThis->m50_springPosition, 0xA3D);
            pThis->m4C_springVelocity += pThis->m48_springAccel;
            pThis->m4C_springVelocity -= MTH_Mul(pThis->m4C_springVelocity, 0x6666);
            pThis->m50_springPosition += pThis->m4C_springVelocity;
            pThis->m48_springAccel = 0;

            sVec3_FP camOffset2 = readOverlayCameraOffset2(pEngine);
            sVec3_FP camDelta = (targetPos + camOffset2) - sourcePos;
            camDelta[0] = MTH_Mul(camDelta[0], pThis->m50_springPosition);
            camDelta[1] = MTH_Mul(camDelta[1], pThis->m50_springPosition);
            camDelta[2] = MTH_Mul(camDelta[2], pThis->m50_springPosition);
            pThis->m14_cameraTarget = sourcePos + camDelta;

            sVec3_FP camOffset = readOverlayCameraOffset(pEngine, pEngine->m22C_dragonCurrentQuadrant);
            sVec3_FP camDelta2 = sourcePos - (targetPos + camOffset);
            camDelta2[0] = MTH_Mul(camDelta2[0], pThis->m50_springPosition >> 1);
            camDelta2[1] = MTH_Mul(camDelta2[1], pThis->m50_springPosition >> 1);
            camDelta2[2] = MTH_Mul(camDelta2[2], pThis->m50_springPosition >> 1);
            pThis->m8_cameraPosition = (targetPos + camOffset) + camDelta2;
            battleEngine_resetCameraInterpolation();
        }

        // Update look-at target for each projectile
        for (u32 i = 0; i < (u32)pThis->m54_numProjectiles; i++)
        {
            if (pThis->m5E_numTargets == 0)
            {
                pThis->m20_lookAtTarget = targetPos;
            }
            else if (pThis->m0_projectiles[i].m4_active == 1)
            {
                if (pThis->m57_level == 3)
                {
                    pThis->m20_lookAtTarget[0] = targetPos[0];
                    pThis->m20_lookAtTarget[1] = targetPos[1];
                }
                else
                {
                    s32 targetIdx = i * 4;
                    s_battleEnemy* enemy = pTargetSystem->m0_enemyTargetables[targetIdx / 4];
                    if (enemy->m4_targetable != nullptr &&
                        (enemy->m4_targetable->m50_flags & 0x140003) == 0 &&
                        enemy->m0_isActive > 0)
                    {
                        sVec3_FP* pos = getBattleTargetablePosition(*enemy->m4_targetable);
                        pThis->m20_lookAtTarget = *pos;
                        break;
                    }
                    if (i != (u32)pThis->m54_numProjectiles - 1)
                        continue;
                    pThis->m20_lookAtTarget[0] = targetPos[0];
                    pThis->m20_lookAtTarget[1] = targetPos[1];
                }
                pThis->m20_lookAtTarget[2] = targetPos[2];
            }
        }
    }

updateEnd:
    // Process target hit tracking (runs every frame for all states)
    if (pThis->m5E_numTargets != 0)
    {
        static const s16 damageTable[] = { 0x48, 0x51, 0x7A, 0x52 }; // BTL_A3::060acd94

        for (s32 i = 0; i < pThis->m5E_numTargets; i++)
        {
            if (pThis->m4_targets[i].m0_hitFlag != 0)
            {
                if (pThis->m57_level == 3)
                {
                    // Damage to dragon (onslaught hits player)
                    s16 dmg = damageTable[(u8)pThis->m57_level] * pThis->m4_targets[i].m0_hitFlag;
                    applyDamageToDragon(
                        gBattleManager->m10_battleOverlay->m18_dragon->m8C,
                        dmg,
                        gBattleManager->m10_battleOverlay->m18_dragon->m8_position,
                        2,
                        pThis->m4_targets[i].m4_direction,
                        0x1000);
                }
                else
                {
                    // Damage to enemy
                    s_battleEnemy* enemy = pTargetSystem->m0_enemyTargetables[i];
                    if (enemy->m4_targetable != nullptr &&
                        (enemy->m4_targetable->m50_flags & 0x140003) == 0 &&
                        enemy->m0_isActive > 0)
                    {
                        s16 baseDmg = (s16)(damageTable[(u8)pThis->m57_level] * pThis->m4_targets[i].m0_hitFlag);
                        s16 scaledDmg = phantomSlasher_getBaseDamage(baseDmg);
                        s16 damage = computeResistanceDamage(
                            enemy->m4_targetable, scaledDmg, 2);
                        sVec3_FP* enemyPos = getBattleTargetablePosition(*enemy->m4_targetable);
                        applyDamageToEnnemy(
                            enemy->m4_targetable, damage, enemyPos, 3,
                            pThis->m4_targets[i].m4_direction, 0x2000);
                    }
                }
            }
            pThis->m4_targets[i].m0_hitFlag = 0;
        }
    }

    // FIXME 1: end-of-frame termination checks (was completely missing)
    // Count inactive projectiles — terminate when all are done
    u32 inactiveCount = 0;
    for (s32 i = 0; i < pThis->m54_numProjectiles; i++)
    {
        if (pThis->m0_projectiles[i].m4_active == 0)
            inactiveCount++;
    }
    if (inactiveCount == (u32)pThis->m54_numProjectiles)
    {
        pThis->getTask()->markFinished();
        return;
    }

    // 330-frame safety timeout
    pThis->m58++;
    if (pThis->m58 > 0x14A)
    {
        pThis->getTask()->markFinished();
    }
}
