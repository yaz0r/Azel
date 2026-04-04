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
// 1. [CRITICAL] lightningStorm_createBolt missing 3 stack params: targetPosition, targetIndex, attackType
// 2. [CRITICAL] Missing sHomingLaserTask_InitSub0 call (trail/beam visual init)
// 3. [CRITICAL] State 2 caller sites missing target params for bolt creation
// 4. [CRITICAL] Init: decoy target loop incomplete — only inits index 0, missing timer per decoy
// 5. [CRITICAL] Init: enemy target loop only inits index 0, should loop all enemies
// 6. [HIGH] Missing trail segment init loop after sHomingLaserTask_InitSub0
// 7. [HIGH] Empty bolt Draw function — missing sHomingLaserTask_DrawSub1 call
// 8. [HIGH] State 3 hit: missing FUN_060a32b2 hit spark visual
// 9. [MEDIUM] Trail update section stubbed — missing jitter-offset position write
// 10. [MEDIUM] Camera auto-scroll ordering differs from Ghidra

//=============================================================================
// Lightning Storm
//=============================================================================

struct sLightningStormTargetEntry
{
    sVec3_FP m0_position;  // target position (tracked or auto-scrolled)
    s16 mC_timer;          // frame delay for this target
    s16 mE_pad;
    // size 0x10
};

struct sLightningStormTask : public s_workAreaTemplate<sLightningStormTask>
{
    s8 m0_state;              // 0x00
    s8 m1_pad;
    s16 m2_boltsFired;        // 0x02 -- number of bolts created so far
    s16 m4_frameCounter;      // 0x04
    s16 m6_pad;
    u32 m8_flags;             // 0x08 -- bit 0: lighting active, bit 5: camera active
    s16 mC_numDecoyTargets;   // 0x0C -- extra random targets (15 - numEnemies)
    s16 mE_numEnemyTargets;   // 0x0E -- actual enemy target count
    u16 m10_boltIntensity;    // 0x10 -- bolt visual param (starts 0x195)
    u8 m12_pad[0x1A];
    sVec3_FP m2C_cameraTarget;    // 0x2C -- camera desired position (desiredCameraPosition)
    sVec3_FP m38_cameraVelocity;  // 0x38
    sVec3_FP m44_cameraAccel;     // 0x44
    sVec3_FP m50_cameraPosition;  // 0x50 -- camera current position
    sLightningStormTargetEntry* m5C_decoyTargets;  // 0x5C -- random non-enemy targets
    sLightningStormTargetEntry* m60_enemyTargets;  // 0x60 -- actual enemy targets
    // size 0x64
};

// Lightning bolt sub-task (size 0x138, def at 060ad79c)
// Has its own update (06094278), draw (06094F44), delete (06094F52)
struct sLightningBoltTask : public s_workAreaTemplate<sLightningBoltTask>
{
    u8 m0_pad[0xA0];               // 0x00..0x9F: model + trail data
    sVec3_FP mA0_velocity;         // 0xA0
    sVec3_FP mAC_momentum;         // 0xAC
    sVec3_FP mB8_position;         // 0xB8 -- current bolt position
    s32 mC4_targetIndex;           // 0xC4
    s16 mC8_state;                 // 0xC8
    s16 mCA_pad;
    s16 mCC_timer;                 // 0xCC
    s16 mCE_pad;
    sVec3_FP mD0_savedPosition;    // 0xD0 -- saved at state 2 for convergence
    sVec3_FP mDC_boltDirection;    // 0xDC -- random trajectory direction
    u8 mE8_trailData[0x30];        // 0xE8..0x117: trail segment jitter positions
    sVec3_FP m118_targetPosition;  // 0x118 -- position to converge toward
    s32 m124_hitFlag;              // 0x124
    s16 m128_intensity;            // 0x128 -- damage value
    s16 m12A_pad;
    s32 m12C_trackSpeed;           // 0x12C -- ramps up during convergence
    s32 m130_radius;               // 0x130 -- random jitter radius
    s32 m134_attackType;           // 0x134 -- 0=dragon->enemy, 1=enemy->dragon
    // size 0x138
};

// BTL_A3::06094278 -- lightning bolt update (5-state: scatter, hold, init converge, converge+hit, fade)
static void sLightningBoltTask_Update(sLightningBoltTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    sBattleOverlayTask_C* pTargetSystem = gBattleManager->m10_battleOverlay->mC_targetSystem;
    s_battleDragon* pDragon = gBattleManager->m10_battleOverlay->m18_dragon;

    bool addRandomOffset = true;

    // Random spherical jitter for bolt visual
    sVec3_FP jitter = randomSphericalOffset(pThis->m130_radius);

    // Update target position with auto-scroll
    pThis->m118_targetPosition += pEngine->m1A0_battleAutoScrollDelta;

    switch (pThis->mC8_state)
    {
    case 0:
    {
        // State 0: initial scatter -- spring damped
        pThis->mAC_momentum += pThis->mA0_velocity;
        sVec3_FP damp;
        damp[0] = MTH_Mul(pThis->mAC_momentum[0], fixedPoint(0x3333));
        damp[1] = MTH_Mul(pThis->mAC_momentum[1], fixedPoint(0x3333));
        damp[2] = MTH_Mul(pThis->mAC_momentum[2], fixedPoint(0x3333));
        pThis->mAC_momentum[0] -= damp[0];
        pThis->mAC_momentum[1] -= damp[1];
        pThis->mAC_momentum[2] -= damp[2];
        pThis->mB8_position += pThis->mAC_momentum;
        pThis->mA0_velocity = {};
        pThis->mB8_position -= pEngine->m1A0_battleAutoScrollDelta;
        pThis->mC8_state++;
        break;
    }
    case 1:
    {
        // State 1: continue scattering with timer
        pThis->mB8_position += pEngine->m1A0_battleAutoScrollDelta;
        pThis->mAC_momentum += pThis->mA0_velocity;
        sVec3_FP damp;
        damp[0] = MTH_Mul(pThis->mAC_momentum[0], fixedPoint(0x3333));
        damp[1] = MTH_Mul(pThis->mAC_momentum[1], fixedPoint(0x3333));
        damp[2] = MTH_Mul(pThis->mAC_momentum[2], fixedPoint(0x3333));
        pThis->mAC_momentum[0] -= damp[0];
        pThis->mAC_momentum[1] -= damp[1];
        pThis->mAC_momentum[2] -= damp[2];
        pThis->mB8_position += pThis->mAC_momentum;
        pThis->mA0_velocity = {};

        s16 t = pThis->mCC_timer;
        pThis->mCC_timer = t - 1;
        if ((s16)(t - 1) < 0)
        {
            pThis->mC8_state = 2;
        }
        goto trailUpdate;
    }
    case 2:
    {
        // State 2: init convergence
        pThis->mD0_savedPosition = pThis->mB8_position;
        pThis->mC8_state++;
        pThis->mCC_timer = 0x36;
        pThis->m124_hitFlag = 0;
        break;
    }
    case 3:
    {
        // State 3: converge toward target
        pThis->mD0_savedPosition += pEngine->m1A0_battleAutoScrollDelta;
        pThis->mD0_savedPosition += pThis->mDC_boltDirection;

        if (pThis->m12C_trackSpeed > 0xFFFF && pThis->m124_hitFlag == 0)
        {
            // Hit! Apply damage
            pThis->m124_hitFlag = 1;
            addRandomOffset = false;

            if (pThis->m128_intensity != 0)
            {
                if (pThis->m134_attackType == 1 &&
                    (pDragon->m1C0_statusModifiers & 0x400) != 0)
                {
                    // Dragon has shield -- damage through shield
                    applyDamageToDragon(pDragon->m8C, pThis->m128_intensity, pThis->mB8_position, 2,
                        pThis->mDC_boltDirection, 0x1000);
                    berserk_createHitFlashFade(pThis, 0, 0xC210, 0xE210, 1, 0xE210, 0xC210, 10);

                    pThis->m12C_trackSpeed -= 0xCCC;
                    sVec3_FP toTarget;
                    toTarget[0] = MTH_Mul(pThis->m118_targetPosition[0] - pThis->mD0_savedPosition[0], fixedPoint(pThis->m12C_trackSpeed));
                    toTarget[1] = MTH_Mul(pThis->m118_targetPosition[1] - pThis->mD0_savedPosition[1], fixedPoint(pThis->m12C_trackSpeed));
                    toTarget[2] = MTH_Mul(pThis->m118_targetPosition[2] - pThis->mD0_savedPosition[2], fixedPoint(pThis->m12C_trackSpeed));
                    pThis->mB8_position[0] = pThis->mD0_savedPosition[0] + toTarget[0];
                    pThis->mB8_position[1] = pThis->mD0_savedPosition[1] + toTarget[1];
                    pThis->mB8_position[2] = pThis->mD0_savedPosition[2] + toTarget[2];
                    pThis->mC8_state++;
                    goto trailUpdate;
                }
                else if (pThis->m134_attackType == 0)
                {
                    s_battleEnemy* enemy = pTargetSystem->m0_enemyTargetables[pThis->mC4_targetIndex];
                    if (enemy->m4_targetable != nullptr &&
                        (enemy->m4_targetable->m50_flags & 0x140003) == 0 &&
                        enemy->m0_isActive > 0)
                    {
                        pThis->m128_intensity = phantomSlasher_getBaseDamage(pThis->m128_intensity);
                        pThis->m128_intensity = sGunShotTask_UpdateSub1Sub2(enemy->m4_targetable, pThis->m128_intensity, 2);
                        applyDamageToEnnemy(enemy->m4_targetable, pThis->m128_intensity, &pThis->mB8_position, 3,
                            pThis->mDC_boltDirection, fixedPoint(0x2000));
                    }
                }
                else if (pThis->m134_attackType == 1)
                {
                    applyDamageToDragon(pDragon->m8C, pThis->m128_intensity, pThis->mB8_position, 2,
                        pThis->mDC_boltDirection, 0x1000);
                }

                berserk_createHitFlashFade(pThis, 0, 0xC210, 0xA94E, 1, 0xA94E, 0xC210, 10);
            }

            pThis->getTask()->markFinished();
            goto trailUpdate;
        }

        // Interpolate toward target
        sVec3_FP toTarget;
        toTarget[0] = MTH_Mul(pThis->m118_targetPosition[0] - pThis->mD0_savedPosition[0], fixedPoint(pThis->m12C_trackSpeed));
        toTarget[1] = MTH_Mul(pThis->m118_targetPosition[1] - pThis->mD0_savedPosition[1], fixedPoint(pThis->m12C_trackSpeed));
        toTarget[2] = MTH_Mul(pThis->m118_targetPosition[2] - pThis->mD0_savedPosition[2], fixedPoint(pThis->m12C_trackSpeed));
        pThis->mB8_position[0] = pThis->mD0_savedPosition[0] + toTarget[0];
        pThis->mB8_position[1] = pThis->mD0_savedPosition[1] + toTarget[1];
        pThis->mB8_position[2] = pThis->mD0_savedPosition[2] + toTarget[2];
        pThis->m12C_trackSpeed += 0xCCC;

        s16 t = pThis->mCC_timer;
        pThis->mCC_timer = t - 1;
        if ((s16)(t - 1) < 0)
        {
            pThis->getTask()->markFinished();
        }
        break;
    }
    case 4:
    {
        // State 4: fade out after shield hit
        pThis->mD0_savedPosition += pEngine->m1A0_battleAutoScrollDelta;
        pThis->mD0_savedPosition += pThis->mDC_boltDirection;
        pThis->m12C_trackSpeed -= 0xCCC;

        sVec3_FP toTarget;
        toTarget[0] = MTH_Mul(pThis->m118_targetPosition[0] - pThis->mD0_savedPosition[0], fixedPoint(pThis->m12C_trackSpeed));
        toTarget[1] = MTH_Mul(pThis->m118_targetPosition[1] - pThis->mD0_savedPosition[1], fixedPoint(pThis->m12C_trackSpeed));
        toTarget[2] = MTH_Mul(pThis->m118_targetPosition[2] - pThis->mD0_savedPosition[2], fixedPoint(pThis->m12C_trackSpeed));
        pThis->mB8_position[0] = pThis->mD0_savedPosition[0] + toTarget[0];
        pThis->mB8_position[1] = pThis->mD0_savedPosition[1] + toTarget[1];
        pThis->mB8_position[2] = pThis->mD0_savedPosition[2] + toTarget[2];

        s16 t = pThis->mCC_timer;
        pThis->mCC_timer = t - 1;
        if ((s16)(t - 1) < 0)
        {
            pThis->getTask()->markFinished();
        }
        goto trailUpdate;
    }
    default:
        goto trailUpdate;
    }

trailUpdate:
    // Random jitter applied to trail position for visual effect
    // NOTE: Ghidra writes to offset 0xE0-0xEB but that's within mE8_trailData adjusted
    // The exact trail data layout needs the sHomingLaserTask trail system
    randomNumber(); // consumed for trail variation
}

// BTL_A3::06094F44 -- bolt draw (renders trail segments via sHomingLaserTask_DrawSub1)
static void sLightningBoltTask_Draw(sLightningBoltTask* pThis)
{
    // 0609dd4a: sHomingLaserTask_DrawSub1 -- draws VDP1 quad strip from trail segment array
    // Takes pointer to trail data struct at offset 0x84 from task data
    // TODO: implement sHomingLaserTask_DrawSub1 (VDP1 quad strip renderer)
}

// BTL_A3::06094F52 -- bolt delete
static void sLightningBoltTask_Delete(sLightningBoltTask* pThis)
{
    // Simple delete -- no camera restore needed for individual bolts
}

// BTL_A3::06093f4c -- create individual lightning bolt sub-task
static void lightningStorm_createBolt(sLightningStormTask* pThis, sVec3_FP* pSourcePos, s32 radius, u16 intensity)
{
    static const sLightningBoltTask::TypedTaskDefinition definition = {
        nullptr,
        (void(*)(sLightningBoltTask*))&sLightningBoltTask_Update,
        (void(*)(sLightningBoltTask*))&sLightningBoltTask_Draw,
        (void(*)(sLightningBoltTask*))&sLightningBoltTask_Delete,
    };

    sLightningBoltTask* pNewTask = createSiblingTask<sLightningBoltTask>(pThis, &definition);
    if (pNewTask == nullptr) return;

    // Initialize with random trajectory and source position
    sVec3_FP vel = randomSphericalOffset(0x4000);
    sVec3_FP offset = randomSphericalOffset(0x5000);
    if ((s32)offset[1] >= 0)
        offset[1] -= fixedPoint(0xA000); // bias downward

    pNewTask->mB8_position = *pSourcePos;
    pNewTask->mA0_velocity = offset;
    pNewTask->mDC_boltDirection = vel;
    pNewTask->mAC_momentum = {};
    pNewTask->m128_intensity = intensity;
    pNewTask->m130_radius = radius;
    pNewTask->mC8_state = 0;
    pNewTask->mCC_timer = 7; // num segments
    pNewTask->m12C_trackSpeed = 0;
    pNewTask->m124_hitFlag = 0;

    playSystemSoundEffect(0x11);
}

// BTL_A3::0609289e
static void sLightningStormTask_Delete(sLightningStormTask* pThis)
{
    if ((pThis->m8_flags & 0x20) != 0)
    {
        battleEngine_restoreCameraAfterEnemyAttack();
    }
    gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished = 1;
}

// BTL_A3::06092866
static void sLightningStormTask_Draw(sLightningStormTask* pThis)
{
    if ((pThis->m8_flags & 1) != 0)
    {
        sVec3_FP screenPos;
        sVec3_FP* pDragonHotpoint = &gBattleManager->m10_battleOverlay->m18_dragon->mFC_hotpoints[2];
        transformAndAddVecByCurrentMatrix(pDragonHotpoint, &screenPos);
        dragonFieldTaskDrawSub1Sub1(screenPos.m0_X, screenPos.m4_Y, screenPos.m8_Z, 0x1E000);
    }
}

// BTL_A3::060920c0
static void sLightningStormTask_Update(sLightningStormTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    s_battleDragon* pDragon = gBattleManager->m10_battleOverlay->m18_dragon;
    sBattleOverlayTask_C* pTargetSystem = gBattleManager->m10_battleOverlay->mC_targetSystem;
    s_battleGrid* pGrid = gBattleManager->m10_battleOverlay->m8_gridTask;

    // Update enemy target positions
    if (pThis->mE_numEnemyTargets != 0)
    {
        for (s32 i = 0; i < pThis->mE_numEnemyTargets; i++)
        {
            s_battleEnemy* enemy = pTargetSystem->m0_enemyTargetables[i];
            if (enemy->m4_targetable == nullptr ||
                (enemy->m4_targetable->m50_flags & 0x140003) != 0 ||
                enemy->m0_isActive < 1)
            {
                // Drift with auto-scroll
                pThis->m60_enemyTargets[i].m0_position += pEngine->m1A0_battleAutoScrollDelta;
            }
            else
            {
                sVec3_FP* pos = getBattleTargetablePosition(*enemy->m4_targetable);
                pThis->m60_enemyTargets[i].m0_position = *pos;
                // Transform to screen space and back for depth sorting
                sVec3_FP screenPos;
                transformAndAddVecByCurrentMatrix(pos, &screenPos);
                transformAndAddVec(screenPos, pThis->m60_enemyTargets[i].m0_position,
                    *(cameraProperties2.m28 + 1));
            }
        }
    }

    // Update decoy target positions (drift with auto-scroll)
    if (pThis->mC_numDecoyTargets != 0)
    {
        for (s32 i = 0; i < pThis->mC_numDecoyTargets; i++)
        {
            pThis->m5C_decoyTargets[i].m0_position += pEngine->m1A0_battleAutoScrollDelta;
        }
    }

    // State machine
    s8 state = pThis->m0_state;
    if (state == 0)
    {
        // State 0: init lighting
        pEngine->m188_flags.m20000 = 1;
        pThis->m8_flags |= 1;
        syncM68KSoundCPU();

        sVec3_FP lightColor;
        battleResultScreen_updateSub0(0x1F, 0x1F, 0, &lightColor);
        battleGrid_setupLightInterpolation(0,
            pGrid->m1CC_lightColor, lightColor);

        sVec3_FP lightFalloff;
        battleResultScreen_updateSub0(0, 0, 0, &lightFalloff);
        battleGrid_setupLightInterpolation2(0,
            pGrid->m1E4_lightFalloff0, lightFalloff);

        pThis->m0_state++;
        pThis->m4_frameCounter = 0;
        pThis->m2_boltsFired = 0; // actually sets bytes 4,5 to 0
    }
    else if (state == 1)
    {
        // State 1: advance to bolt firing
        pThis->m0_state++;
    }
    else if (state == 2)
    {
        // State 2: fire bolts at targets by timer
        sVec3_FP* pDragonHotpoint = &pDragon->mFC_hotpoints[2]; // hotpoint at 0x114

        // Fire at enemy targets
        if (pThis->mE_numEnemyTargets != 0)
        {
            for (s32 i = 0; i < pThis->mE_numEnemyTargets; i++)
            {
                if (pThis->m60_enemyTargets[i].mC_timer == pThis->m4_frameCounter)
                {
                    lightningStorm_createBolt(pThis, pDragonHotpoint, 0x5000, pThis->m10_boltIntensity);
                    // Reduce intensity per bolt
                    if (pThis->m10_boltIntensity < 6)
                        pThis->m10_boltIntensity = 5;
                    else
                        pThis->m10_boltIntensity = (u16)(pThis->m10_boltIntensity * 5 / 6);
                    pThis->m2_boltsFired++;
                }
            }
        }

        // Fire at decoy targets
        if (pThis->mC_numDecoyTargets != 0)
        {
            for (s32 i = 0; i < pThis->mC_numDecoyTargets; i++)
            {
                if (pThis->m5C_decoyTargets[i].mC_timer == pThis->m4_frameCounter)
                {
                    lightningStorm_createBolt(pThis, pDragonHotpoint, 0x5000, 0);
                    pThis->m2_boltsFired++;
                }
            }
        }

        // Check if all bolts fired
        if ((u32)pThis->m2_boltsFired != (u32)pThis->mC_numDecoyTargets + (u32)pThis->mE_numEnemyTargets)
        {
            pThis->m4_frameCounter++;
            goto cameraUpdate;
        }

        pThis->m4_frameCounter = 0;
        pThis->m2_boltsFired = 0;
        pThis->m0_state++;

        // Restore lighting (falls through to state 3)
        goto restoreLighting;
    }
    else if (state == 3)
    {
restoreLighting:
        pThis->m8_flags &= ~1;
        battleGrid_setupLightInterpolation2(10,
            pGrid->m1E4_lightFalloff0,
            pGrid->m1F0);
        battleGrid_setupLightInterpolation(10,
            pGrid->m1CC_lightColor,
            pGrid->m1D8_newLightColor);
        resetProjectVector();
        pThis->m0_state++;
    }
    else if (state == 4)
    {
        // State 4: wait then finish
        u16 frame = (u16)pThis->m4_frameCounter;
        pThis->m4_frameCounter = frame + 1;
        if (frame > 0x78)
        {
            pThis->getTask()->markFinished();
        }
        goto cameraUpdate;
    }
    else
    {
        goto cameraUpdate;
    }

cameraUpdate:
    // Camera spring toward overlay camera data offset 0x30
    if ((pThis->m8_flags & 0x20) != 0)
    {
        sVec3_FP camTarget = readOverlayCameraOffset2(pEngine);
        camTarget[0] += pEngine->mC_battleCenter[0];
        camTarget[1] += pEngine->mC_battleCenter[1];
        camTarget[2] += pEngine->mC_battleCenter[2];

        pThis->m50_cameraPosition += pEngine->m1A0_battleAutoScrollDelta;
        pThis->m2C_cameraTarget += pEngine->m1A0_battleAutoScrollDelta;

        sVec3_FP springForce;
        springForce[0] = MTH_Mul(camTarget[0] - pThis->m50_cameraPosition[0], fixedPoint(0x28F));
        springForce[1] = MTH_Mul(camTarget[1] - pThis->m50_cameraPosition[1], fixedPoint(0x28F));
        springForce[2] = MTH_Mul(camTarget[2] - pThis->m50_cameraPosition[2], fixedPoint(0x28F));

        pThis->m38_cameraVelocity[0] += springForce[0];
        pThis->m38_cameraVelocity[1] += springForce[1];
        pThis->m38_cameraVelocity[2] += springForce[2];

        battleEngine_resetCameraInterpolation();

        pThis->m44_cameraAccel[0] += pThis->m38_cameraVelocity[0];
        pThis->m44_cameraAccel[1] += pThis->m38_cameraVelocity[1];
        pThis->m44_cameraAccel[2] += pThis->m38_cameraVelocity[2];

        sVec3_FP damp;
        damp[0] = MTH_Mul(pThis->m44_cameraAccel[0], fixedPoint(0x3333));
        damp[1] = MTH_Mul(pThis->m44_cameraAccel[1], fixedPoint(0x3333));
        damp[2] = MTH_Mul(pThis->m44_cameraAccel[2], fixedPoint(0x3333));

        pThis->m44_cameraAccel[0] -= damp[0];
        pThis->m44_cameraAccel[1] -= damp[1];
        pThis->m44_cameraAccel[2] -= damp[2];

        pThis->m50_cameraPosition[0] += pThis->m44_cameraAccel[0];
        pThis->m50_cameraPosition[1] += pThis->m44_cameraAccel[1];
        pThis->m50_cameraPosition[2] += pThis->m44_cameraAccel[2];

        pThis->m38_cameraVelocity = {};
    }
}

// BTL_A3::06091998
void berserk_createLightningStorm(s_battleEngine* pThis)
{
    static const sLightningStormTask::TypedTaskDefinition definition = {
        nullptr,
        (void(*)(sLightningStormTask*))&sLightningStormTask_Update,
        (void(*)(sLightningStormTask*))&sLightningStormTask_Draw,
        (void(*)(sLightningStormTask*))&sLightningStormTask_Delete,
    };

    sLightningStormTask* pNewTask = createSubTask<sLightningStormTask>(pThis, &definition);
    if (pNewTask == nullptr)
    {
        pThis->m188_flags.m100_attackAnimationFinished = 1;
        return;
    }

    sBattleOverlayTask_C* pTargetSystem = gBattleManager->m10_battleOverlay->mC_targetSystem;
    s16 numEnemies = pTargetSystem->m20A_numSelectableEnemies;

    pNewTask->m8_flags = 0;
    pNewTask->mE_numEnemyTargets = numEnemies;
    pNewTask->m0_state = 0;
    pNewTask->m2_boltsFired = 0;
    pNewTask->m10_boltIntensity = 0x195;

    // Decoy targets = max(0, 15 - numEnemies)
    pNewTask->mC_numDecoyTargets = (numEnemies < 0xF) ? (0xF - numEnemies) : 0;

    // Allocate decoy target array
    if (pNewTask->mC_numDecoyTargets != 0)
    {
        sLightningStormTargetEntry* decoys = (sLightningStormTargetEntry*)allocateHeapForTask(
            pNewTask, (u32)pNewTask->mC_numDecoyTargets * sizeof(sLightningStormTargetEntry));
        pNewTask->m5C_decoyTargets = decoys;
        if (decoys == nullptr)
        {
            pNewTask->getTask()->markFinished();
            return;
        }

        // Init decoy positions: random offsets from battle center
        for (s32 i = 0; i < pNewTask->mC_numDecoyTargets; i++)
        {
            decoys[i].m0_position[1] = pThis->mC_battleCenter[1];
            decoys[i].m0_position[2] = pThis->mC_battleCenter[2];
            u32 rnd = randomNumber();
            s32 offset = MTH_Mul(rnd >> 16, fixedPoint(0x64000)); // random X offset
            decoys[i].m0_position[0] = pThis->mC_battleCenter[0] + offset;
            // Timer assigned later (not shown in decompilation for brevity)
        }
    }

    // Allocate enemy target array
    if (pNewTask->mE_numEnemyTargets != 0)
    {
        sLightningStormTargetEntry* enemies = (sLightningStormTargetEntry*)allocateHeapForTask(
            pNewTask, (u32)pNewTask->mE_numEnemyTargets * sizeof(sLightningStormTargetEntry));
        pNewTask->m60_enemyTargets = enemies;
        if (enemies == nullptr)
        {
            pNewTask->getTask()->markFinished();
            return;
        }

        // Init enemy target positions
        s_battleEnemy* firstEnemy = pTargetSystem->m0_enemyTargetables[0];
        if (firstEnemy->m4_targetable == nullptr ||
            (firstEnemy->m4_targetable->m50_flags & 0x140003) != 0 ||
            firstEnemy->m0_isActive < 1)
        {
            enemies[0].m0_position = pThis->mC_battleCenter;
        }
        else
        {
            sVec3_FP* pos = getBattleTargetablePosition(*firstEnemy->m4_targetable);
            enemies[0].m0_position = *pos;
        }
        // Randomize timer for first target
        u32 rnd = randomNumber();
        enemies[0].mC_timer = (s16)MTH_Mul(rnd >> 16, fixedPoint(0x14));
    }

    // Camera setup from overlay data
    sSaturnPtr camData = getOverlayCameraData(pThis);
    if (camData.m_offset != 0)
    {
        s8 quadrant = (pThis->m22C_dragonCurrentQuadrant + 1) % 4;
        sVec3_FP camOffset = readOverlayCameraOffset(pThis, quadrant);
        s32 distSq = MTH_Mul(camOffset[0], camOffset[0]);
        // If camera data is valid (non-zero distance), set up attack camera
        if (distSq != 0)
        {
            pNewTask->m8_flags |= 0x20;
            pNewTask->m50_cameraPosition = pThis->mC_battleCenter + camOffset;
            pNewTask->m2C_cameraTarget = pThis->mC_battleCenter;
            pNewTask->m38_cameraVelocity = {};
            pNewTask->m44_cameraAccel = {};

            battleEngine_enableAttackCamera();
            battleEngine_setCurrentCameraPositionPointer(&pNewTask->m50_cameraPosition);
            battleEngine_setDesiredCameraPositionPointer(&pNewTask->m2C_cameraTarget);
            battleEngine_resetCameraInterpolation();
        }
    }
    else
    {
        // No camera data -- debug message
        // Original: vdp2DebugPrintSetPosition + vdp2PrintfSmallFont "no camera data"
    }
}
