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
#include "BTL_A3/BTL_A3_UrchinFormation.h"
#include "homingLaser.h"
#include "kernel/graphicalObject.h"
#include "kernel/vdp1Allocator.h"


//=============================================================================
// Berserker Rage
//=============================================================================

struct sBerserkerRageTask : public s_workAreaTemplate<sBerserkerRageTask>
{
    s8 m0_burstCounter;        // 0x00 -- bursts remaining in current volley
    s8 m1_burstDelay;          // 0x01 -- frames between bursts (starts 10, decreases)
    s8 m2_state;               // 0x02
    s8 m3_pad;
    s16 m4_numTargetsRemaining; // 0x04 -- remaining valid targets for random selection
    s16 m6_timer;              // 0x06
    s16 m8_globalTimer;        // 0x08
    s16 mA_pad;
    u32 mC_flags;              // 0x0C -- bit 4: single target mode, bit 5: has camera
    s32 m10_volleyCount;       // 0x10 -- volleys remaining (starts 99)
    sVec3_FP m14_springVel;    // 0x14
    sVec3_FP m20_springAccel;  // 0x20
    sVec3_FP m2C_cameraTarget; // 0x2C -- camera desired position
    sVec3_FP m38_cameraPosition; // 0x38 -- camera current position
    sVec3_FP m44_cameraAngle;  // 0x44 — also used as sVec3_FP for battleEngineSub1_UpdateSub2
    sVec3_FP* m50_pSourcePos;  // 0x50 -- pointer to source position (dragon or battleCenter)
    sVec3_FP* m54_pTargetPos;  // 0x54 -- pointer to target position
    sVec3_FP* m58_pHotpoint;   // 0x58 -- hotpoint for bolt origin
    sVec3_FP m5C_cameraRotation; // 0x5C
    sVec3_FP* m68_targetPositions; // 0x68 -- allocated array of target positions (0xC each)
    s16* m6C_targetIndices;    // 0x6C -- allocated array for random target selection
    s32 m70_attackType;        // 0x70 -- 0=dragon attacks enemies, 1=enemy attacks dragon
    // size 0x74
};

// Base damage table at 060ad76c
static const s16 berserkerRageDamageTable[] = { 0x18, 0x12 }; // [0]=dragon->enemy, [1]=enemy->dragon

// Rage bolt sub-task (size 0xEC, def at 060ad780)
struct sBerserkerRageBoltTask : public s_workAreaTemplate<sBerserkerRageBoltTask>
{
    u8 m0_pad[0x84];               // 0x00..0x83: trail/model data
    sHomingLaserTrailData m84_trailData;
    u16 mA0_targetIndex;           // 0xA0
    s16 mA2_state;                 // 0xA2
    u8 mA4_pad[2];                 // 0xA4
    s16 mA6_damage;                // 0xA6
    s16 mA8_timer;                 // 0xA8
    s16 mAA_pad;
    s32 mAC_continueFlag;          // 0xAC -- nonzero to continue after shield hit
    sVec3_FP mB0_position;         // 0xB0 -- current bolt position
    sVec3_FP mBC_velocity;         // 0xBC -- movement direction
    sVec3_FP mC8_sourcePosition;   // 0xC8 -- initial position (minus autoScroll)
    sVec3_FP* mD4_pTargetPosPtr;   // 0xD4 -- pointer to source position for tracking
    s32 mD8_hitFlag;               // 0xD8
    s32 mDC_attackType;            // 0xDC -- 0=dragon->enemy, 1=enemy->dragon
    s32 mE0_missFlag;              // 0xE0
    s32 mE4_trackSpeed;            // 0xE4
    s32 mE8_trackDecay;            // 0xE8
    // size 0xEC
};

// BTL_A3::06093760 -- rage bolt update (3-state: init, converge+hit, fade)
static void sBerserkerRageBoltTask_Update(sBerserkerRageBoltTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    s_battleDragon* pDragon = gBattleManager->m10_battleOverlay->m18_dragon;
    sBattleOverlayTask_C* pTargetSystem = gBattleManager->m10_battleOverlay->mC_targetSystem;

    switch (pThis->mA2_state)
    {
    case 0:
    {
        // State 0: init random trajectory
        pThis->mB0_position = pThis->mC8_sourcePosition;
        pThis->mBC_velocity = randomSphericalOffset(0x3000);

        // Bias upward if too low
        if ((s32)pThis->mBC_velocity[1] < -0x2000)
            pThis->mBC_velocity[1] += fixedPoint(0x5000);

        // Adjust Z velocity based on quadrant
        s8 quadrant = pEngine->m22C_dragonCurrentQuadrant;
        if (quadrant == 0)
            pThis->mBC_velocity[2] = fixedPoint(0xFFFFE000);
        else if (quadrant == 2)
            pThis->mBC_velocity[2] = fixedPoint(0x2000);

        pThis->mA2_state++;
        break;
    }
    case 1:
    {
        pThis->mB0_position += pEngine->m1A0_battleAutoScrollDelta;
        pThis->mB0_position += pThis->mBC_velocity;

        sVec3_FP diff;
        diff[0] = pThis->mC8_sourcePosition[0] - pThis->mB0_position[0];
        diff[1] = pThis->mC8_sourcePosition[1] - pThis->mB0_position[1];
        diff[2] = pThis->mC8_sourcePosition[2] - pThis->mB0_position[2];

        if (pThis->mE4_trackSpeed > 0xFFFF && pThis->mD8_hitFlag == 0)
        {
            pThis->mE4_trackSpeed = 0x10000;
            pThis->mD8_hitFlag = 1;

            if ((pDragon->m1C0_statusModifiers & 0x400) != 0 &&
                pThis->mDC_attackType == 1 && pThis->mE0_missFlag == 0)
            {
                if (pThis->mA6_damage != 0)
                {
                    applyDamageToDragon(pDragon->m8C, pThis->mA6_damage, pThis->mC8_sourcePosition, 2,
                        diff, 0x1000);
                }
                if (pThis->mAC_continueFlag == 0)
                {
                    pThis->getTask()->markFinished();
                }
                else
                {
                    // Shield continue: decelerate toward target, advance to state 2
                    pThis->mE4_trackSpeed -= pThis->mE8_trackDecay;
                    sVec3_FP homing;
                    homing[0] = MTH_Mul((*pThis->mD4_pTargetPosPtr)[0] - pThis->mB0_position[0], pThis->mE4_trackSpeed);
                    homing[1] = MTH_Mul((*pThis->mD4_pTargetPosPtr)[1] - pThis->mB0_position[1], pThis->mE4_trackSpeed);
                    homing[2] = MTH_Mul((*pThis->mD4_pTargetPosPtr)[2] - pThis->mB0_position[2], pThis->mE4_trackSpeed);
                    pThis->mC8_sourcePosition[0] = pThis->mB0_position[0] + homing[0];
                    pThis->mC8_sourcePosition[1] = pThis->mB0_position[1] + homing[1];
                    pThis->mC8_sourcePosition[2] = pThis->mB0_position[2] + homing[2];
                    pThis->mA2_state++;
                }
                goto trailShift;
            }

            if (pThis->mA6_damage != 0)
            {
                if (pThis->mDC_attackType == 0)
                {
                    s_battleEnemy* enemy = pTargetSystem->m0_enemyTargetables[pThis->mA0_targetIndex];
                    if (enemy->m4_targetable == nullptr ||
                        (enemy->m4_targetable->m50_flags & 0x140003) != 0 ||
                        enemy->m0_isActive < 1)
                    {
                        pThis->mE0_missFlag = 1;
                    }
                    else
                    {
                        pThis->mA6_damage = phantomSlasher_getBaseDamage(pThis->mA6_damage);
                        pThis->mA6_damage = sGunShotTask_UpdateSub1Sub2(enemy->m4_targetable, pThis->mA6_damage, 2);
                        applyDamageToEnnemy(enemy->m4_targetable, pThis->mA6_damage, &pThis->mC8_sourcePosition, 3,
                            diff, fixedPoint(0x2000));
                        pThis->mE0_missFlag = 0;
                    }
                }
                else if (pThis->mDC_attackType == 1)
                {
                    applyDamageToDragon(pDragon->m8C, pThis->mA6_damage, pThis->mC8_sourcePosition, 2,
                        diff, 0x1000);
                    pThis->mE0_missFlag = 0;
                }
            }

            if (pThis->mE0_missFlag == 0)
            {
                pThis->getTask()->markFinished();
                goto trailShift;
            }
        }

        // Homing: update sourcePosition to interpolate toward target
        {
            sVec3_FP homing;
            homing[0] = MTH_Mul((*pThis->mD4_pTargetPosPtr)[0] - pThis->mB0_position[0], pThis->mE4_trackSpeed);
            homing[1] = MTH_Mul((*pThis->mD4_pTargetPosPtr)[1] - pThis->mB0_position[1], pThis->mE4_trackSpeed);
            homing[2] = MTH_Mul((*pThis->mD4_pTargetPosPtr)[2] - pThis->mB0_position[2], pThis->mE4_trackSpeed);
            pThis->mC8_sourcePosition[0] = pThis->mB0_position[0] + homing[0];
            pThis->mC8_sourcePosition[1] = pThis->mB0_position[1] + homing[1];
            pThis->mC8_sourcePosition[2] = pThis->mB0_position[2] + homing[2];
        }
        // Ramp tracking: decay accelerates, speed increases
        pThis->mE8_trackDecay += 0x147;
        pThis->mE4_trackSpeed += pThis->mE8_trackDecay;
        // Timer countdown
        pThis->mA8_timer--;
        if (pThis->mA8_timer < 0)
        {
            pThis->getTask()->markFinished();
        }
        break;
    }
    case 2:
    {
        // State 2: decelerate after shield hit
        pThis->mB0_position += pEngine->m1A0_battleAutoScrollDelta;
        pThis->mB0_position += pThis->mBC_velocity;
        pThis->mE4_trackSpeed -= pThis->mE8_trackDecay;
        {
            sVec3_FP homing;
            homing[0] = MTH_Mul((*pThis->mD4_pTargetPosPtr)[0] - pThis->mB0_position[0], pThis->mE4_trackSpeed);
            homing[1] = MTH_Mul((*pThis->mD4_pTargetPosPtr)[1] - pThis->mB0_position[1], pThis->mE4_trackSpeed);
            homing[2] = MTH_Mul((*pThis->mD4_pTargetPosPtr)[2] - pThis->mB0_position[2], pThis->mE4_trackSpeed);
            pThis->mC8_sourcePosition[0] = pThis->mB0_position[0] + homing[0];
            pThis->mC8_sourcePosition[1] = pThis->mB0_position[1] + homing[1];
            pThis->mC8_sourcePosition[2] = pThis->mB0_position[2] + homing[2];
        }
        pThis->mA8_timer--;
        if (pThis->mA8_timer < 0)
        {
            pThis->getTask()->markFinished();
        }
        break;
    }
    default:
        break;
    }

trailShift:
    sHomingLaserTask_shiftTrailSegments(&pThis->m84_trailData, &pThis->mC8_sourcePosition,
        &pEngine->m1A0_battleAutoScrollDelta);
}

// BTL_A3::06093F1A -- rage bolt draw (renders trail segments via sHomingLaserTask_DrawSub1)
static void sBerserkerRageBoltTask_Draw(sBerserkerRageBoltTask* pThis)
{
    sHomingLaserTask_DrawSub1(&pThis->m84_trailData);
}

// BTL_A3::06093F28 -- rage bolt delete
static void sBerserkerRageBoltTask_Delete(sBerserkerRageBoltTask* pThis)
{
    // Bolt cleanup
}

// BTL_A3::06093618
static void berserkerRage_createBolt(sBerserkerRageTask* pThis, sVec3_FP* pSource, sVec3_FP* pTarget, s16 damage, s16 targetIndex, s32 continueFlag, s32 laserDataType, s32 missFlag, s32 attackType)
{
    static const sBerserkerRageBoltTask::TypedTaskDefinition definition = {
        nullptr,
        &sBerserkerRageBoltTask_Update,
        &sBerserkerRageBoltTask_Draw,
        &sBerserkerRageBoltTask_Delete,
    };

    sBerserkerRageBoltTask* pNewTask = createSiblingTask<sBerserkerRageBoltTask>(pThis, &definition);
    if (pNewTask == nullptr) return;

    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;

    pNewTask->mA8_timer = 0x33;
    pNewTask->mA2_state = 0;
    pNewTask->mA6_damage = damage;
    pNewTask->mA0_targetIndex = targetIndex;
    pNewTask->mD8_hitFlag = 0;
    pNewTask->mE4_trackSpeed = 0;
    pNewTask->mE8_trackDecay = 0;
    pNewTask->mAC_continueFlag = continueFlag;
    pNewTask->mDC_attackType = attackType;
    pNewTask->mE0_missFlag = missFlag;
    pNewTask->mD4_pTargetPosPtr = pTarget;

    // Store source position minus auto-scroll
    pNewTask->mC8_sourcePosition[0] = (*pSource)[0] - pEngine->m1A0_battleAutoScrollDelta[0];
    pNewTask->mC8_sourcePosition[1] = (*pSource)[1] - pEngine->m1A0_battleAutoScrollDelta[1];
    pNewTask->mC8_sourcePosition[2] = (*pSource)[2] - pEngine->m1A0_battleAutoScrollDelta[2];

    const sLaserData* pLaserData = (laserDataType == 0)
        ? &BTL_GenericData::mBoltLaserData0
        : &BTL_GenericData::mBoltLaserData1;
    u16 vdp1Param = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory;
    sHomingLaserTask_InitSub0(&pNewTask->m84_trailData, pNewTask, &pNewTask->mC8_sourcePosition, vdp1Param, pLaserData);

    playSystemSoundEffect(8);
}

// BTL_A3::060935e0
static void sBerserkerRageTask_Delete(sBerserkerRageTask* pThis)
{
    if ((pThis->mC_flags & 0x20) != 0)
    {
        battleEngine_restoreCameraAfterEnemyAttack();
    }
    gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished = 1;
}

// BTL_A3::06092d44
static void sBerserkerRageTask_Update(sBerserkerRageTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    s_battleDragon* pDragon = gBattleManager->m10_battleOverlay->m18_dragon;
    sBattleOverlayTask_C* pTargetSystem = gBattleManager->m10_battleOverlay->mC_targetSystem;
    s_battleGrid* pGrid = gBattleManager->m10_battleOverlay->m8_gridTask;

    // Determine number of targets based on attack type
    s16 numTargets;
    if (pThis->m70_attackType == 0)
        numTargets = pTargetSystem->m20A_numSelectableEnemies;
    else if (pThis->m70_attackType == 1)
        numTargets = 1;
    else
        numTargets = 0;

    // Update target positions
    if (numTargets == 0)
    {
        // Single fixed target -- drift with auto-scroll
        pThis->m68_targetPositions[0] += pEngine->m1A0_battleAutoScrollDelta;
    }
    else
    {
        for (s32 i = 0; i < numTargets; i++)
        {
            if (pThis->m70_attackType == 0)
            {
                s_battleEnemy* enemy = pTargetSystem->m0_enemyTargetables[i];
                if (enemy->m4_targetable == nullptr ||
                    (enemy->m4_targetable->m50_flags & 0x140003) != 0)
                {
                    // Invalid -- drift
                    sVec3_FP* pos = &pThis->m68_targetPositions[i];
                    *pos += pEngine->m1A0_battleAutoScrollDelta;
                }
                else
                {
                    // Track live position
                    sVec3_FP* pos = &pThis->m68_targetPositions[i];
                    sVec3_FP* enemyPos = getBattleTargetablePosition(*enemy->m4_targetable);
                    *pos = *enemyPos;
                }
            }
            else
            {
                // Enemy attacking dragon -- track dragon position
                sVec3_FP* pos = &pThis->m68_targetPositions[i];
                *pos = pDragon->m8_position;
            }
        }
    }

    // Global timer
    pThis->m8_globalTimer++;

    // State machine
    switch (pThis->m2_state)
    {
    case 0:
    {
        u16 timer = (u16)pThis->m6_timer;
        pThis->m6_timer = timer + 1;
        if (timer >= 0x10)
        {
            if (pThis->m70_attackType == 0)
            {
                vdp2Controls.m4_pendingVdp2Regs->m20_BGON &= ~1;
                vdp2Controls.m_isDirty = 1;
            }
            g_fadeControls.m_4D = 6;
            if ((s8)g_fadeControls.m_4C < 7)
            {
                vdp2Controls.m20_registers[1].m112_CLOFSL = 0x4A;
                vdp2Controls.m20_registers[0].m112_CLOFSL = 0x4A;
            }
            fadePalette(&g_fadeControls.m0_fade0, 0xC210, 0xA108, 0x5A);
            g_fadeControls.m_4D = 5;

            sVec3_FP lightFalloff;
            battleResultScreen_updateSub0(0, 0, 0, &lightFalloff);
            battleGrid_setupLightInterpolation2(2, pGrid->m1E4_lightFalloff0, lightFalloff);

            pThis->m6_timer = 0;
            pThis->m2_state++;
        }
        break;
    }
    case 1:
    {
        pThis->m6_timer++;

        s8 burstCounter = pThis->m0_burstCounter;
        pThis->m0_burstCounter = burstCounter - 1;
        if ((s8)(burstCounter - 1) == 0)
        {
            s32 volleyCount = pThis->m10_volleyCount;
            pThis->m10_volleyCount = volleyCount - 1;
            if (volleyCount < 0)
            {
                pThis->m2_state++;
                break;
            }

            pThis->m0_burstCounter = pThis->m1_burstDelay;
            if ((u8)pThis->m1_burstDelay > 2)
                pThis->m1_burstDelay--;

            if ((pThis->mC_flags & 0x10) == 0)
            {
                if (pThis->m70_attackType == 0)
                {
                    // Fisher-Yates random target selection
                    bool exhausted = false;
                    while (true)
                    {
                        u32 rnd = randomNumber();
                        s32 idx = MTH_Mul(rnd >> 16, (u32)pThis->m4_numTargetsRemaining);

                        s16 targetIdx = pThis->m6C_targetIndices[idx];
                        s_battleEnemy* enemy = pTargetSystem->m0_enemyTargetables[targetIdx];
                        if (enemy->m4_targetable != nullptr &&
                            (enemy->m4_targetable->m50_flags & 0x140003) == 0)
                        {
                            s32 baseDmg = berserkerRageDamageTable[0];
                            berserkerRage_createBolt(pThis, &pDragon->mFC_hotpoints[2],
                                &pThis->m68_targetPositions[targetIdx], baseDmg, targetIdx, 0, 0, 0, 0);
                            break;
                        }

                        pThis->m4_numTargetsRemaining--;
                        if (pThis->m4_numTargetsRemaining == 0)
                        {
                            exhausted = true;
                            break;
                        }
                        s16 lastIdx = pThis->m6C_targetIndices[pThis->m4_numTargetsRemaining];
                        pThis->m6C_targetIndices[pThis->m4_numTargetsRemaining] = pThis->m6C_targetIndices[idx];
                        pThis->m6C_targetIndices[idx] = lastIdx;
                    }
                    if (exhausted)
                    {
                        pThis->m2_state++;
                        break;
                    }
                }
                else
                {
                    s32 baseDmg;
                    if ((pDragon->m1C0_statusModifiers & 0x400) == 0)
                    {
                        baseDmg = berserkerRageDamageTable[pThis->m70_attackType];
                    }
                    else
                    {
                        if ((pThis->m10_volleyCount & 7) == 1 || pThis->m10_volleyCount > 0x59)
                            baseDmg = berserkerRageDamageTable[pThis->m70_attackType] * 5;
                        else
                            baseDmg = 0;
                    }
                    berserkerRage_createBolt(pThis, pThis->m58_pHotpoint,
                        &pDragon->m8_position, baseDmg, 0, 0, pThis->m70_attackType, 0, pThis->m70_attackType);
                }
            }
            else
            {
                s32 baseDmg = berserkerRageDamageTable[pThis->m70_attackType];
                berserkerRage_createBolt(pThis, &pDragon->mFC_hotpoints[2],
                    &pThis->m68_targetPositions[0], baseDmg, 0, 0, 0, 0, 0);
            }
        }

        if (pThis->m8_globalTimer > 0x1C2)
        {
            pThis->m2_state++;
        }
        break;
    }
    case 2:
    {
        if (pThis->m70_attackType == 0)
        {
            vdp2Controls.m4_pendingVdp2Regs->m20_BGON |= 1;
            vdp2Controls.m_isDirty = 1;
        }
        g_fadeControls.m_4D = 6;
        if ((s8)g_fadeControls.m_4C < 7)
        {
            vdp2Controls.m20_registers[1].m112_CLOFSL = 0x4A;
            vdp2Controls.m20_registers[0].m112_CLOFSL = 0x4A;
        }
        fadePalette(&g_fadeControls.m0_fade0, 0xA108, 0xC210, 0xC);
        g_fadeControls.m_4D = 5;

        battleGrid_setupLightInterpolation2(10, pGrid->m1E4_lightFalloff0, pGrid->m1F0);

        pThis->m6_timer = 0;
        pThis->m2_state++;
        [[fallthrough]];
    }
    case 3:
    {
        if (pThis->m6_timer++ > 0x3C)
        {
            pThis->getTask()->markFinished();
        }
        break;
    }
    default:
        break;
    }

    // Camera update
    // Camera spring toward overlay offset 0x30 + targetPos
    if ((pThis->mC_flags & 0x20) != 0)
    {
        sVec3_FP camOffset = readOverlayCameraOffset2(pEngine);
        sVec3_FP targetBase = *pThis->m54_pTargetPos;

        pThis->m2C_cameraTarget += pEngine->m1A0_battleAutoScrollDelta;

        sVec3_FP springTarget;
        springTarget[0] = (camOffset[0] + targetBase[0]) - pThis->m2C_cameraTarget[0];
        springTarget[1] = (camOffset[1] + targetBase[1]) - pThis->m2C_cameraTarget[1];
        springTarget[2] = (camOffset[2] + targetBase[2]) - pThis->m2C_cameraTarget[2];

        springTarget[0] = MTH_Mul(springTarget[0], fixedPoint(0xA3));
        springTarget[1] = MTH_Mul(springTarget[1], fixedPoint(0xA3));
        springTarget[2] = MTH_Mul(springTarget[2], fixedPoint(0xA3));

        pThis->m14_springVel[0] += springTarget[0];
        pThis->m14_springVel[1] += springTarget[1];
        pThis->m14_springVel[2] += springTarget[2];

        pThis->m5C_cameraRotation[1] -= fixedPoint(0x5B05B);
        battleEngineSub1_UpdateSub2(&pThis->m38_cameraPosition,
            *pThis->m50_pSourcePos, pThis->m44_cameraAngle, pThis->m5C_cameraRotation);

        pThis->m20_springAccel[0] += pThis->m14_springVel[0];
        pThis->m20_springAccel[1] += pThis->m14_springVel[1];
        pThis->m20_springAccel[2] += pThis->m14_springVel[2];

        sVec3_FP damp;
        damp[0] = MTH_Mul(pThis->m20_springAccel[0], fixedPoint(0x1999));
        damp[1] = MTH_Mul(pThis->m20_springAccel[1], fixedPoint(0x1999));
        damp[2] = MTH_Mul(pThis->m20_springAccel[2], fixedPoint(0x1999));

        pThis->m20_springAccel[0] -= damp[0];
        pThis->m20_springAccel[1] -= damp[1];
        pThis->m20_springAccel[2] -= damp[2];

        pThis->m2C_cameraTarget[0] += pThis->m20_springAccel[0];
        pThis->m2C_cameraTarget[1] += pThis->m20_springAccel[1];
        pThis->m2C_cameraTarget[2] += pThis->m20_springAccel[2];

        pThis->m14_springVel = {};
    }
}

// BTL_A3::060928c4
void berserk_createBerserkerRage(s_battleEngine* pThis, s32 attackType, sVec3_FP* pHotpoint)
{
    static const sBerserkerRageTask::TypedTaskDefinition definition = {
        nullptr,
        &sBerserkerRageTask_Update,
        nullptr,
        &sBerserkerRageTask_Delete,
    };

    sBerserkerRageTask* pNewTask = createSubTask<sBerserkerRageTask>(pThis, &definition);
    if (pNewTask == nullptr)
    {
        pThis->m188_flags.m100_attackAnimationFinished = 1;
        return;
    }

    s_battleDragon* pDragon = gBattleManager->m10_battleOverlay->m18_dragon;
    sBattleOverlayTask_C* pTargetSystem = gBattleManager->m10_battleOverlay->mC_targetSystem;

    pNewTask->m70_attackType = attackType;
    pNewTask->m58_pHotpoint = pHotpoint;

    s16 numTargets;
    if (pNewTask->m70_attackType == 0)
    {
        pNewTask->m50_pSourcePos = &pDragon->m8_position;
        pNewTask->m54_pTargetPos = &pThis->mC_battleCenter;
        numTargets = pTargetSystem->m20A_numSelectableEnemies;
    }
    else
    {
        pNewTask->m50_pSourcePos = &pThis->mC_battleCenter;
        pNewTask->m54_pTargetPos = &pDragon->m8_position;
        numTargets = 1;
    }

    pNewTask->m0_burstCounter = 1;
    pNewTask->m1_burstDelay = 10;
    pNewTask->m10_volleyCount = 99;
    pNewTask->m2_state = 0;
    pNewTask->m6_timer = 0;
    pNewTask->m8_globalTimer = 0;
    pNewTask->mC_flags = 0;

    if (numTargets == 0)
    {
        // Single target mode -- allocate single position
        sVec3_FP* pos = (sVec3_FP*)allocateHeapForTask(pNewTask, sizeof(sVec3_FP));
        pNewTask->m68_targetPositions = (sVec3_FP*)pos;
        if (pos == nullptr)
        {
            pNewTask->getTask()->markFinished();
            return;
        }
        *pos = *pNewTask->m54_pTargetPos;
        pNewTask->mC_flags |= 0x10;
    }
    else
    {
        // Multi-target mode
        sVec3_FP* positions = (sVec3_FP*)allocateHeapForTask(pNewTask, numTargets * sizeof(sVec3_FP));
        s16* indices = (s16*)allocateHeapForTask(pNewTask, numTargets * sizeof(s16));
        pNewTask->m68_targetPositions = positions;
        pNewTask->m6C_targetIndices = indices;
        if (positions == nullptr || indices == nullptr)
        {
            pNewTask->getTask()->markFinished();
            return;
        }

        pNewTask->m4_numTargetsRemaining = numTargets;
        for (s32 i = 0; i < numTargets; i++)
        {
            if (pNewTask->m70_attackType == 0)
            {
                sVec3_FP* enemyPos = getBattleTargetablePosition(
                    *pTargetSystem->m0_enemyTargetables[i]->m4_targetable);
                positions[i] = *enemyPos;
            }
            else
            {
                positions[i] = *pNewTask->m54_pTargetPos;
            }
            indices[i] = (s16)i;
        }
        pNewTask->mC_flags &= ~0x10;
    }

    // Flash effect
    urchinFormation_createFadeTask(pNewTask, 0xC210, 0xF39C, 2, 0xF39C, 0xC210, 0xC);

    // Camera setup from overlay data
    sSaturnPtr camData = getOverlayCameraData(pThis);
    if (camData.m_offset == 0)
    {
        // No camera data
        // Original: vdp2DebugPrintSetPosition + vdp2PrintfSmallFont "no camera data"
    }
    else
    {
        // Setup camera rotation based on quadrant
        pNewTask->m5C_cameraRotation = {};
        s8 quadrant = pThis->m22C_dragonCurrentQuadrant;
        if (quadrant == 0)
            pNewTask->m5C_cameraRotation[1] = fixedPoint(0x40000);
        else if (quadrant == 1)
            pNewTask->m5C_cameraRotation[1] = fixedPoint(0x80000);
        else if (quadrant == 2)
            pNewTask->m5C_cameraRotation[1] = fixedPoint(0xC0000);
        else if (quadrant == 3)
            pNewTask->m5C_cameraRotation[1] = 0;

        pNewTask->m44_cameraAngle = {};
        pNewTask->m44_cameraAngle.m8_Z = fixedPoint(0xA000);

        // Y offset based on height comparison
        sVec3_FP camOffset2 = readOverlayCameraOffset2(pThis);
        if ((s32)(*pNewTask->m50_pSourcePos)[1] <
            (s32)(camOffset2[1] + (*pNewTask->m54_pTargetPos)[1]))
        {
            s32 minY = pTargetSystem->m204_cameraMaxAltitude;
            if ((s32)(*pNewTask->m50_pSourcePos)[1] - 0x2800 < minY)
                pNewTask->m44_cameraAngle.m4_Y = 0;
            else
                pNewTask->m44_cameraAngle.m4_Y = fixedPoint(0xFFFFD800);
        }
        else
        {
            pNewTask->m44_cameraAngle.m4_Y = fixedPoint(0x2800);
        }

        // Negate for enemy attack
        if (pNewTask->m70_attackType == 1)
        {
            pNewTask->m44_cameraAngle.m0_X = -pNewTask->m44_cameraAngle.m0_X;
            pNewTask->m44_cameraAngle.m4_Y = -pNewTask->m44_cameraAngle.m4_Y;
            pNewTask->m44_cameraAngle.m8_Z = -pNewTask->m44_cameraAngle.m8_Z;
        }

        // Init camera position
        battleEngineSub1_UpdateSub2(&pNewTask->m38_cameraPosition,
            *pNewTask->m50_pSourcePos, pNewTask->m44_cameraAngle, pNewTask->m5C_cameraRotation);

        pNewTask->m14_springVel = {};
        pNewTask->m20_springAccel = {};
        pNewTask->m2C_cameraTarget = *pNewTask->m50_pSourcePos;

        pNewTask->mC_flags |= 0x20;
        battleEngine_enableAttackCamera();
        battleEngine_setCurrentCameraPositionPointer(&pNewTask->m38_cameraPosition);
        battleEngine_setDesiredCameraPositionPointer(&pNewTask->m2C_cameraTarget);
        battleEngine_resetCameraInterpolation();
    }
}
