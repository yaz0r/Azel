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
// 1. [CRITICAL] Beam struct field offsets wrong: m80 should be targetIndex (byte), m88 should be targetPosition (ptr)
// 2. [CRITICAL] Missing trail data shifting in states 1 and 2
// 3. [CRITICAL] Missing per-target damage scaling loop (5/6 ratio per target step)
// 4. [CRITICAL] Damage direction computed from wrong source (should use trail data diff)
// 5. [HIGH] Missing VDP1 data ptr init (m4), heap allocations (m58/m5C/m60/m64), sHomingLaserTask_InitSub0
// 6. [HIGH] applyDamageToEnnemy uses wrong position offset
// 7. [HIGH] State 2 completely wrong: missing trail logic and threshold check
// 8. [MEDIUM] Struct m28/m2C/m30 layout wrong (3 separate s32, not sVec3_FP + separate)
// 9. [MEDIUM] m50_cameraAngleZ overlaps m4C_cameraAngle (should be m4C.m4_Y)

//=============================================================================
// Laser Storm
//=============================================================================

struct sLaserStormTargetEntry
{
    sBattleTargetable* m0_pTargetable; // 0x00
    sVec3_FP* m4_pBeamPosition;       // 0x04 -- points to the beam origin position
    s8 m8_targetIndex;                 // 0x08
    // size 0x0C
};

// Laser Storm beam sub-task (size 0x10C, def at 060ad570)
struct sLaserStormBeamTask : public s_workAreaTemplateWithArgWithCopy<sLaserStormBeamTask, sLaserStormTargetEntry*>
{
    // 0x00: m0_fileBundle (inherited)
    // 0x04: m4_vd1Allocation (inherited)
    u8 m8_pad[6];                          // 0x08..0x0D
    sVec3_FP m10_beamTipPosition;          // 0x10 -- interpolated beam tip
    u8 m1C_pad[0x18];                      // 0x1C..0x33
    sVec3_FP m34_beamDirection;            // 0x34 -- direction at contact
    u8 m40_pad[0x18];                      // 0x40..0x57
    s32* m58_trailData1;                   // 0x58 -- allocated trail array 1
    s32* m5C_trailData2;                   // 0x5C -- allocated trail array 2
    s32* m60_segmentPositions;             // 0x60 -- allocated segment position array
    s32* m64_segmentPositions2;            // 0x64 -- allocated segment position array 2
    s32 m68_pad2[2];                       // 0x68
    s16 m6C_approachCounter;               // 0x6C -- countdown for beam approach
    s16 m6E_pad;
    s32 m70_totalApproachSteps;            // 0x70 -- total approach steps (10)
    s16 m74_delayTimer;                    // 0x74 -- delay before beam fires
    s16 m76_pad;
    u8 m78_targetIndex;                    // 0x78
    u8 m79_pad[5];
    s8 m7E_state;                          // 0x7E
    u8 m7F_pad;
    sVec3_FP* m80_pTargetPosition;         // 0x80 -- pointer to target position (from getBattleTargetablePosition)
    sBattleTargetable* m84_pTargetable;    // 0x84 -- target
    sVec3_FP* m8C_pBeamOrigin;             // 0x8C -- pointer to beam origin position
    sVec3_FP* m90_pDragonPosition;         // 0x90 -- pointer to dragon position
    u8 m94_pad[0x5C];                      // 0x94..0xEF: trail rendering data
    u8 mF0_trailStruct[0x1C];             // 0xF0..0x10B: sHomingLaserTask trail data
    // size 0x10C
};

// BTL_A3::0608e7a2 -- beam init (receives target entry as arg)
static void sLaserStormBeamTask_Init(sLaserStormBeamTask* pThis, sLaserStormTargetEntry* pTarget)
{
    pThis->m84_pTargetable = pTarget->m0_pTargetable;
    if (pThis->m84_pTargetable == nullptr ||
        (pTarget->m0_pTargetable->m50_flags & 0x140003) != 0)
    {
        pThis->getTask()->markFinished();
        return;
    }

    pThis->m8C_pBeamOrigin = pTarget->m4_pBeamPosition;
    pThis->m80_pTargetPosition = getBattleTargetablePosition(*pTarget->m0_pTargetable);
    pThis->m78_targetIndex = (u8)pTarget->m8_targetIndex;
    pThis->m90_pDragonPosition = &gBattleManager->m10_battleOverlay->m18_dragon->m8_position;

    // Init beam tip position from beam origin
    pThis->m10_beamTipPosition = *pThis->m8C_pBeamOrigin;

    pThis->m6C_approachCounter = 10;
    pThis->m70_totalApproachSteps = 10;
    pThis->m74_delayTimer = (s16)pTarget->m8_targetIndex * 3 + 0x3C;

    // NOTE: sHomingLaserTask_InitSub0 initializes 3D model for beam visual -- not yet implemented
    pThis->m7E_state = 0;
}

// BTL_A3::0608e8ec -- beam update (3-state: delay, approach+damage, fade)
static void sLaserStormBeamTask_Update(sLaserStormBeamTask* pThis)
{
    switch (pThis->m7E_state)
    {
    case 0:
    {
        // State 0: delay timer before beam fires
        s16 t = pThis->m74_delayTimer;
        pThis->m74_delayTimer = t - 1;
        if (t < 0)
        {
            pThis->m74_delayTimer = 0;
            pThis->m7E_state++;
            // TODO: set draw method to sHomingLaserTask_Draw
            playSystemSoundEffect(8);
        }
        break;
    }
    case 1:
    {
        // State 1: beam approaches target, deals damage on arrival
        if (pThis->m84_pTargetable == nullptr ||
            (pThis->m84_pTargetable->m50_flags & 0x140003) != 0)
        {
            // Target died -- skip to fade
            pThis->m7E_state = 2;
            pThis->m6C_approachCounter = 10;
            break;
        }

        if (pThis->m6C_approachCounter < 1)
        {
            // Beam reached target -- apply damage
            sVec3_FP dir;
            sVec3_FP* targetPos = pThis->m80_pTargetPosition;
            sVec3_FP* beamOrigin = pThis->m8C_pBeamOrigin;
            dir[0] = (*targetPos)[0] - (*beamOrigin)[0];
            dir[1] = (*targetPos)[1] - (*beamOrigin)[1];
            dir[2] = (*targetPos)[2] - (*beamOrigin)[2];

            pThis->m34_beamDirection = dir;

            // Compute damage with per-target scaling
            // NOTE: original uses setDividend for per-target damage reduction
            s16 dmg = computeSprScaledDamage(0xB4); // base damage 0xB4 (=180)
            dmg = computeResistanceDamage(pThis->m84_pTargetable, dmg, 2);
            applyDamageToEnnemy(pThis->m84_pTargetable, dmg, targetPos, 3, dir, fixedPoint(0x2000));

            pThis->m6C_approachCounter = 10;
            pThis->m7E_state++;
        }
        else
        {
            // Interpolate beam tip toward target
            sVec3_FP* targetPos = pThis->m80_pTargetPosition;
            sVec3_FP* beamOrigin = pThis->m8C_pBeamOrigin;
            sVec3_FP diff;
            diff[0] = (*targetPos)[0] - (*beamOrigin)[0];
            diff[1] = (*targetPos)[1] - (*beamOrigin)[1];
            diff[2] = (*targetPos)[2] - (*beamOrigin)[2];

            fixedPoint t = FP_Div(
                (pThis->m70_totalApproachSteps - pThis->m6C_approachCounter) * 0x10000,
                pThis->m70_totalApproachSteps << 16);
            diff[0] = MTH_Mul(t, diff[0]);
            diff[1] = MTH_Mul(t, diff[1]);
            diff[2] = MTH_Mul(t, diff[2]);

            pThis->m10_beamTipPosition[0] = diff[0] + (*beamOrigin)[0];
            pThis->m10_beamTipPosition[1] = diff[1] + (*beamOrigin)[1];
            pThis->m10_beamTipPosition[2] = diff[2] + (*beamOrigin)[2];

            pThis->m6C_approachCounter--;
        }
        break;
    }
    case 2:
    {
        // State 2: beam fades out, then destroy
        s32 counter = pThis->m6C_approachCounter;
        pThis->m6C_approachCounter = (s16)(counter - 1);
        if (counter < 0)
        {
            pThis->getTask()->markFinished();
        }
        break;
    }
    default:
        break;
    }
}

struct sLaserStormTask : public s_workAreaTemplateWithCopy<sLaserStormTask>
{
    // 0x00: m0_fileBundle (inherited)
    // 0x04: m4_vd1Allocation (inherited)
    sLaserStormTargetEntry* m8_targets; // 0x08 -- allocated target array
    u8 mC_pad[0x10];                   // 0x0C..0x1B
    sVec3_FP m1C_beamPosition;         // 0x1C -- computed beam origin
    sVec3_FP m28_positionOffset;       // 0x28 -- offset from m234
    sVec3_FP m2C_altitudeOffset;       // 0x2C -- Y offset
    sVec3_FP m40_cameraPosition;       // 0x40
    sVec3_FP m4C_cameraAngle;          // 0x4C
    s32 m50_cameraAngleZ;              // 0x50 -- 0xA000
    u8 m54_pad[4];
    sVec3_FP m58_cameraRotation;       // 0x58
    s16 m64_timer1;                    // 0x64
    s16 m66_totalDuration;             // 0x66
    u8 m68_pad[4];
    s16 m6C_state;                     // 0x6C
    s16 m6E_numTargets;                // 0x6E
    // size 0x70
};

// BTL_A3::0608ed20
static void sLaserStormTask_Update(sLaserStormTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    s_battleDragon* pDragon = gBattleManager->m10_battleOverlay->m18_dragon;
    sBattleOverlayTask_C* pTargetSystem = gBattleManager->m10_battleOverlay->mC_targetSystem;

    s16 state = pThis->m6C_state;
    if (state == 0)
    {
        // State 0: init -- compute beam height, create beam sub-tasks for each enemy
        if (pThis->m6E_numTargets < 0)
            pThis->m6E_numTargets = 1;

        // Compute Y position for beam -- max of dragon altitude or enemy altitude + offsets
        s32 dragonAltY = ((s32)pDragon->m8_position[1] - (s32)pEngine->m234[1]) + 0x1E000;
        s32 enemyAltY = (s32)pEngine->m234[1] + 0x64000;
        s32 beamY = (dragonAltY > enemyAltY) ? dragonAltY : enemyAltY;
        pThis->m2C_altitudeOffset = sVec3_FP(0, fixedPoint(beamY), 0);

        // Compute beam position = m234 + offsets
        pThis->m1C_beamPosition[0] = pEngine->m234[0] + pThis->m28_positionOffset[0];
        pThis->m1C_beamPosition[1] = pEngine->m234[1] + (s32)pThis->m2C_altitudeOffset[1];
        pThis->m1C_beamPosition[2] = pEngine->m234[2] + pThis->m28_positionOffset[2];

        // Create beam sub-tasks for each valid target
        for (s32 i = 0; i < pThis->m6E_numTargets; i++)
        {
            if ((pTargetSystem->m0_enemyTargetables[i]->m4_targetable->m50_flags & 0x40000) == 0)
            {
                pThis->m8_targets[i].m0_pTargetable = pTargetSystem->m0_enemyTargetables[i]->m4_targetable;
                pThis->m8_targets[i].m4_pBeamPosition = &pThis->m1C_beamPosition;
                pThis->m8_targets[i].m8_targetIndex = (s8)i;
                // Create beam sub-task (0x10C WithCopy, def at 060ad570)
                static const sLaserStormBeamTask::TypedTaskDefinition beamDef = {
                    &sLaserStormBeamTask_Init,
                    &sLaserStormBeamTask_Update,
                    nullptr,
                    nullptr,
                };
                createSubTaskWithArgWithCopy<sLaserStormBeamTask>(pThis, &pThis->m8_targets[i], &beamDef);
            }
        }

        // Camera setup
        battleEngine_enableAttackCamera();
        pThis->m50_cameraAngleZ = 0xA000;
        battleEngineSub1_UpdateSub2(&pThis->m40_cameraPosition, pDragon->m8_position,
            pThis->m4C_cameraAngle, pThis->m58_cameraRotation);
        battleEngine_setCurrentCameraPositionPointer(&pThis->m40_cameraPosition);
        battleEngine_setDesiredCameraPositionPointer(&pThis->m1C_beamPosition);

        pThis->m6C_state++;
        s16 totalDuration = pThis->m6E_numTargets * 3 + 0x46;
        pThis->m66_totalDuration = totalDuration;
        pThis->m64_timer1 = totalDuration;

        // 3x FUN_060a3416 -- creates visual particle ring sub-tasks at beam origin
        // with radii 0x5000, 0xA000, 0xF000 (purely visual, no gameplay impact)
        // TODO: implement FUN_060a3416 particle ring creator
    }
    else if (state == 1)
    {
        // State 1: wait for beams to complete
        battleEngineSub1_UpdateSub2(&pThis->m40_cameraPosition, pDragon->m8_position,
            pThis->m4C_cameraAngle, pThis->m58_cameraRotation);

        s16 timer = pThis->m64_timer1;
        pThis->m64_timer1 = timer - 1;
        if (timer < 0)
        {
            pThis->m6C_state++;
            pThis->m64_timer1 = 0x2D;
        }
        else
        {
            // Switch camera target partway through
            if ((s32)pThis->m64_timer1 == pThis->m66_totalDuration - 0x3C)
            {
                battleEngine_setDesiredCameraPositionPointer(&pEngine->m234);
            }
            // Update beam position
            pThis->m1C_beamPosition[0] = pEngine->m234[0] + pThis->m28_positionOffset[0];
            pThis->m1C_beamPosition[1] = pEngine->m234[1] + (s32)pThis->m2C_altitudeOffset[1];
            pThis->m1C_beamPosition[2] = pEngine->m234[2] + pThis->m28_positionOffset[2];
        }
    }
    else if (state == 2)
    {
        // State 2: fade out and finish
        battleEngineSub1_UpdateSub2(&pThis->m40_cameraPosition, pDragon->m8_position,
            pThis->m4C_cameraAngle, pThis->m58_cameraRotation);

        s16 timer = pThis->m64_timer1 - 1;
        pThis->m64_timer1 = timer;
        if (timer < 0)
        {
            battleEngine_restoreCameraDefault();
            pEngine->m188_flags.m100_attackAnimationFinished = 1;
            pThis->getTask()->markFinished();
        }
    }
}

// BTL_A3::0608f100
void berserk_createLaserStorm(s_battleEngine* pThis)
{
    static const sLaserStormTask::TypedTaskDefinition definition = {
        nullptr,
        &sLaserStormTask_Update,
        nullptr,
        nullptr, // no delete -- update handles finish + m100
    };

    sLaserStormTask* pNewTask = createSubTaskWithCopy<sLaserStormTask>(pThis, &definition);
    if (pNewTask == nullptr)
    {
        pThis->m188_flags.m100_attackAnimationFinished = 1;
        return;
    }

    s32 numEnemies = battleEngine_getNumActiveEnemies();
    pNewTask->m6E_numTargets = (s16)numEnemies;

    // Allocate target array
    sLaserStormTargetEntry* targets = (sLaserStormTargetEntry*)allocateHeapForTask(
        pNewTask, (s16)pNewTask->m6E_numTargets * sizeof(sLaserStormTargetEntry));
    pNewTask->m8_targets = targets;
}
