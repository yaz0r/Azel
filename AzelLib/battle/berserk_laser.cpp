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
#include "kernel/graphicalObject.h"
#include "kernel/vdp1Allocator.h"
#include "BTL_A3/BTL_A3_UrchinFormation.h"

struct sLaserBerserkTask : public s_workAreaTemplateWithCopy<sLaserBerserkTask>
{
    s32 m0;
    s_vdp1AllocationNode* m4_vdp1DataPtr;
    std::array<s32, 4> m8_perQuadrantCameraAltitude;
    sVec3_FP m18_laserOrigin;
    sVec3_FP m24_laserEnd;
    sVec3_FP m30_cameraTarget;
    sVec3_FP m3C_cameraVelocity;
    sVec3_FP* m48_pTargetPosition;
    sBattleTargetable* m4C_pTargetable;
    s32 m50_frameCounter;
    s32 m54_timer;
    struct sBeamTrail
    {
        sVec3_FP* m0_pSegments;
        s32 m4_numSegments;
        s32 m8_activeCount;
        s32 mC_growthStep;
        sSaturnPtr m10_config;
        u16 m14_vdp1Param;
        // Saturn size 0x18
    };
    sBeamTrail m58_trails[2];
    sVec3_FP m88_cameraPosition;
    sVec3_FP m94_cameraAngle;
    sVec3_FP mA0_cameraRotation;
    s32 mAC;
    sVec3_FP mB0_dualBeamAngle;
    s32 mBC;
    s32 mC0;
    s32 mC4;
    s32 mC8;
    s32 mCC;
    s32 mD0;
    s16 mD4_numTargets;
    s8 mD6;
    s8 mD7_laserType;         // 0=chain, 1=prism
    s32 mD8;
    s32 mDC;
    s8 mDE_state;
    s8 mDF_dualBeam;
    s16 mE0_numHalfTargets;
    s16 mE2_baseDamage;
    s16 mE4_cameraMode;       // 0=orbit, 1=follow
    s16 mE6;
    // size 0xE8
};

// BTL_A3::06096ef0 -- find next valid enemy target for laser
static s32 laserBerserk_initTarget(sLaserBerserkTask* pThis)
{
    sBattleOverlayTask_C* pTargetSystem = gBattleManager->m10_battleOverlay->mC_targetSystem;

    while (true)
    {
        if (pThis->mD4_numTargets >= pTargetSystem->m20A_numSelectableEnemies)
            return 0; // no more targets

        s_battleEnemy* enemy = pTargetSystem->m0_enemyTargetables[pThis->mD4_numTargets];
        if (enemy->m0_isActive > 0)
        {
            pThis->m4C_pTargetable = enemy->m4_targetable;
            if (enemy->m4_targetable != nullptr &&
                (enemy->m4_targetable->m50_flags & 0x140001) == 0)
            {
                return 1; // valid target found
            }
        }
        pThis->mD4_numTargets++;
    }
}

// BTL_A3::06096d1c — beam tracking spring-damper (interpolates laser origin toward target)
static void laserBerserk_updateBeamTracking(sLaserBerserkTask* pThis, s8 param_2)
{
    s32 springK = 0x3333;
    s32 dampK;

    if (pThis->m54_timer < 0x10)
    {
        dampK = pThis->m54_timer * -0xCCD + 0x10000;
    }
    else
    {
        dampK = springK;
        if (param_2 != 0 && param_2 == 1)
        {
            springK = 0x1999;
            dampK = 0x6666;
        }
    }

    sVec3_FP* pTarget = pThis->m48_pTargetPosition;

    pThis->m3C_cameraVelocity.m0_X += MTH_Mul(pTarget->m0_X - pThis->m18_laserOrigin.m0_X, springK);
    pThis->m3C_cameraVelocity.m0_X -= MTH_Mul(pThis->m3C_cameraVelocity.m0_X, dampK);

    pThis->m3C_cameraVelocity.m4_Y += MTH_Mul(pTarget->m4_Y - pThis->m18_laserOrigin.m4_Y, springK);
    pThis->m3C_cameraVelocity.m4_Y -= MTH_Mul(pThis->m3C_cameraVelocity.m4_Y, dampK);

    pThis->m3C_cameraVelocity.m8_Z += MTH_Mul(pTarget->m8_Z - pThis->m18_laserOrigin.m8_Z, springK);
    pThis->m3C_cameraVelocity.m8_Z -= MTH_Mul(pThis->m3C_cameraVelocity.m8_Z, dampK);

    pThis->m18_laserOrigin += pThis->m3C_cameraVelocity;
}

// BTL_A3::06096e1a — dual-beam secondary beam angle update
static void laserBerserk_updateDualBeamAngle(sLaserBerserkTask* pThis)
{
    pThis->mB0_dualBeamAngle.m8_Z += fixedPoint(0x31C71C7);
    s32 vx = (s32)pThis->m3C_cameraVelocity.m0_X;
    s32 vy = (s32)pThis->m3C_cameraVelocity.m4_Y;
    s32 vz = (s32)pThis->m3C_cameraVelocity.m8_Z;
    s32 lenSq = MTH_Mul(vx, vx) + MTH_Mul(vy, vy) + MTH_Mul(vz, vz);
    s32 threshold = MTH_Mul(0x800, 0x800);
    if (threshold < lenSq)
    {
        sVec2_FP lookAt;
        computeLookAt(pThis->m3C_cameraVelocity, lookAt);
        pThis->mB0_dualBeamAngle.m0_X = lookAt[0];
        pThis->mB0_dualBeamAngle.m4_Y = lookAt[1];
    }
}

// BTL_A3::0609d57c — shift beam trail segments and insert new head position
static void laserBerserk_shiftTrailSegments(sLaserBerserkTask::sBeamTrail* pTrail, sVec3_FP* pNewPos, sVec3_FP* pAutoScroll)
{
    s32 count = pTrail->m4_numSegments;
    while (count - 1 > pTrail->m8_activeCount)
    {
        pTrail->m0_pSegments[count - 1] = pTrail->m0_pSegments[count - 2];
        pTrail->m0_pSegments[count - 1] += *pAutoScroll;
        count--;
    }

    // Insert new head position at slot 0
    pTrail->m0_pSegments[0] = *pNewPos;

    // Grow active count
    if (pTrail->m4_numSegments - 1 > pTrail->m8_activeCount)
    {
        pTrail->m8_activeCount += pTrail->mC_growthStep;
    }
}

// BTL_A3::0609d4dc — initialize beam trail buffer
static void laserBerserk_initTrailBuffer(sLaserBerserkTask::sBeamTrail* pTrail, p_workArea pParent, sVec3_FP* pInitPos, u16 vdp1Param, sSaturnPtr config)
{
    pTrail->m14_vdp1Param = vdp1Param;
    pTrail->m10_config = config;
    s32 numSegments = readSaturnS32(config + 0x14);
    pTrail->m4_numSegments = numSegments;
    pTrail->m0_pSegments = (sVec3_FP*)allocateHeapForTask(pParent, numSegments * sizeof(sVec3_FP));

    for (s32 i = 0; i < pTrail->m4_numSegments; i++)
    {
        pTrail->m0_pSegments[i] = *pInitPos;
    }

    pTrail->m8_activeCount = 0;
    pTrail->mC_growthStep = 0;
}

// BTL_A3::060968b0
static void sLaserBerserkTask_Update(sLaserBerserkTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    s_battleDragon* pDragon = gBattleManager->m10_battleOverlay->m18_dragon;

    // Camera update (every frame)
    if (pThis->mE4_cameraMode== 0)
    {
        berserkCameraOrbit(&pThis->m88_cameraPosition, (s32)pThis->mA0_cameraRotation[1],
            pThis->m8_perQuadrantCameraAltitude.data());
    }
    else
    {
        battleEngineSub1_UpdateSub2(&pThis->m88_cameraPosition, pDragon->m8_position,
            pThis->m94_cameraAngle, pThis->mA0_cameraRotation);
    }

    pThis->m54_timer++;

    switch (pThis->mDE_state)
    {
    case 0:
    {
        // State 0: init
        battleEngine_resetCameraInterpolation();
        pThis->mDE_state++;
        break;
    }
    case 1:
    {
        // State 1: fire laser at targets, apply damage
        s32 result = laserBerserk_initTarget(pThis);
        if (result == 1)
        {
            pThis->m48_pTargetPosition = getBattleTargetablePosition(*pThis->m4C_pTargetable);
            laserBerserk_updateBeamTracking(pThis, 0);

            sVec3_FP screenPos;
            transformAndAddVecByCurrentMatrix(&pThis->m18_laserOrigin, &screenPos);
            sVec3_FP hitPos;
            s32 hitResult = sGunShotTask_UpdateSub0(&screenPos, &pThis->m4C_pTargetable->m40, &hitPos);

            if (hitResult > 0)
            {
                // FUN_060a32b2 — creates hit spark VDP1 visual sub-task (0x84 bytes, visual only)
                // FUN_060a32b2(pThis, vdp1Data, &sVdp1Quad_060ae454, targetPos, 0x5000, 2, 0x28, 0x060ad80a)

                // Apply SPR-scaled damage to target
                s16 baseDmg = phantomSlasher_getBaseDamage(pThis->mE2_baseDamage);
                s16 damage = sGunShotTask_UpdateSub1Sub2(pThis->m4C_pTargetable, baseDmg, 2);
                applyDamageToEnnemy(pThis->m4C_pTargetable, damage, &hitPos, 3,
                    pThis->m3C_cameraVelocity, fixedPoint(0x2000));

                // Reduce base damage per hit (2/3 decay, min 5)
                if (pThis->mE2_baseDamage > 5)
                {
                    pThis->mE2_baseDamage = (s16)(((s32)pThis->mE2_baseDamage * 2) / 3);
                    if (pThis->mE2_baseDamage < 5)
                        pThis->mE2_baseDamage = 5;
                }

                // Reduce beam velocity (0.8x per hit)
                pThis->m3C_cameraVelocity[0] = MTH_Mul(fixedPoint(0xCCCC), pThis->m3C_cameraVelocity[0]);
                pThis->m3C_cameraVelocity[1] = MTH_Mul(fixedPoint(0xCCCC), pThis->m3C_cameraVelocity[1]);
                pThis->m3C_cameraVelocity[2] = MTH_Mul(fixedPoint(0xCCCC), pThis->m3C_cameraVelocity[2]);

                urchinFormation_createFadeTask(pThis, 0xC210, 0xD318, 1, 0xD318, 0xC210, 0xF);

                pThis->mD4_numTargets++;
            }

            // Rotation sweep
            pThis->mA0_cameraRotation[1] = (s32)pThis->mA0_cameraRotation[1] + pThis->mAC;
            if (pThis->mE4_cameraMode== 0)
            {
                s32 signedRot = signExtend28((s32)pThis->mA0_cameraRotation[1]);
                if (signedRot > -0xE38E38 && signedRot < 0xE38E38)
                {
                    pThis->mAC = -pThis->mAC;
                }
            }
        }
        else
        {
            // No more targets -- transition to fade out
            // Set target position to dragon hotpoint
            pThis->m48_pTargetPosition = &pDragon->mFC_hotpoints[2];
            pThis->mDE_state = 2;
            pThis->m50_frameCounter = 0x2D;
        }
        break;
    }
    case 2:
    {
        // State 2: fade out and finish
        s32 timer = pThis->m50_frameCounter--;
        if (timer < 1)
        {
            battleEngine_restoreCameraAfterEnemyAttack();
            pEngine->m188_flags.m100_attackAnimationFinished = 1;
            pThis->getTask()->markFinished();
        }

        // Update laser origin with velocity + auto-scroll (Ghidra: param_1[6..8])
        pThis->m18_laserOrigin += pThis->m3C_cameraVelocity;
        pThis->m18_laserOrigin.m0_X += pEngine->m1A0_battleAutoScrollDelta.m0_X;
        pThis->m18_laserOrigin.m4_Y += pEngine->m1A0_battleAutoScrollDelta.m4_Y;
        pThis->m18_laserOrigin.m8_Z += pEngine->m1A0_battleAutoScrollDelta.m8_Z;
        break;
    }
    default:
        break;
    }

    // Shift beam trail segments
    laserBerserk_shiftTrailSegments(&pThis->m58_trails[0], &pThis->m18_laserOrigin, &pEngine->m1A0_battleAutoScrollDelta);

    // Dual-beam secondary beam update
    if (pThis->mDF_dualBeam != 0)
    {
        laserBerserk_updateDualBeamAngle(pThis);
        // Secondary beam follows primary with offset via angle in mB0/mB4
        battleEngineSub1_UpdateSub2(
            &pThis->m24_laserEnd, pThis->m18_laserOrigin,
            sVec3_FP(0, 0, fixedPoint(0x3000)),
            pThis->mB0_dualBeamAngle);
        laserBerserk_shiftTrailSegments(&pThis->m58_trails[1], &pThis->m24_laserEnd, &pEngine->m1A0_battleAutoScrollDelta);
    }
}

// BTL_A3::06096ca0
static void sLaserBerserkTask_Draw(sLaserBerserkTask* pThis)
{
    // Ghidra: transformAndAddVecByCurrentMatrix(*(sVec3_FP**)(param_1+0x58), ...)
    // m58 is the trail struct; *(s32*)m58 = pointer to trail data; first entry = beam source pos
    sVec3_FP screenPos;
    sVec3_FP* pBeamSource = (sVec3_FP*)(pThis->m58_trails[0].m0_pSegments);
    if (pBeamSource)
    {
        transformAndAddVecByCurrentMatrix(pBeamSource, &screenPos);
    }
    else
    {
        transformAndAddVecByCurrentMatrix(&pThis->m18_laserOrigin, &screenPos);
    }
    dragonFieldTaskDrawSub1Sub1(screenPos.m0_X, screenPos.m4_Y, screenPos.m8_Z, 0xA000);

    // TODO: renderBeamTrailSegments(&m58_trails[0], m50, 0x1E)
    // renderBeamTrailSegments(&pThis->m58_trails[0], pThis->m50_frameCounter, 0x1E);

    // TODO: dual-beam trail render
    // if (pThis->mDF_dualBeam != 0)
    //     renderBeamTrailSegments(&pThis->m58_trails[1], pThis->m50_frameCounter, 0x1E);
}

// BTL_A3::06096530
s32 berserk_createLaser(s_battleEngine* pThis, u8 type)
{
    static const sLaserBerserkTask::TypedTaskDefinition definition = {
        nullptr,
        sLaserBerserkTask_Update,
        sLaserBerserkTask_Draw,
        nullptr,
    };

    s_battleDragon* pDragon = gBattleManager->m10_battleOverlay->m18_dragon;

    sLaserBerserkTask* pNewTask = createSubTaskWithCopy<sLaserBerserkTask>(pThis, &definition);
    if (pNewTask == nullptr)
    {
        pThis->m188_flags.m100_attackAnimationFinished = 1;
        return 0;
    }

    // Store VDP1 data pointer
    pNewTask->m4_vdp1DataPtr = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation;

    pNewTask->mD4_numTargets = 0;
    u32 rnd0 = randomNumber();
    pNewTask->mD6 = (s8)(rnd0 % 3);
    pNewTask->mE0_numHalfTargets = gBattleManager->m10_battleOverlay->mC_targetSystem->m20A_numSelectableEnemies >> 1;

    // Init target (before remaining init, per Ghidra)
    s32 initResult = laserBerserk_initTarget(pNewTask);
    if (initResult != 1)
    {
        pThis->m188_flags.m100_attackAnimationFinished = 1;
        pNewTask->getTask()->markFinished();
        return 0;
    }

    pNewTask->mD7_laserType = type;
    pNewTask->mDF_dualBeam = (type == 0) ? 0 : 1;
    pNewTask->m18_laserOrigin = pDragon->mFC_hotpoints[2];

    pNewTask->m48_pTargetPosition = getBattleTargetablePosition(*pNewTask->m4C_pTargetable);
    pNewTask->m50_frameCounter = 0x1E;
    pNewTask->mC8 = 0x3000;
    pNewTask->m54_timer = 0;
    pNewTask->mB0_dualBeamAngle = {};

    pNewTask->m24_laserEnd = pNewTask->m18_laserOrigin;
    pNewTask->m24_laserEnd.m4_Y += fixedPoint(0x3000);

    static const s16 laserBaseDamage[] = { 0x78, 0x10E }; // BTL_A3::060ad804 — chain=120, prism=270
    pNewTask->mE2_baseDamage = laserBaseDamage[type];

    // Init per-quadrant camera altitude
    initPerQuadrantCameraAltitude(pNewTask->m8_perQuadrantCameraAltitude.data());

    // Camera setup
    pNewTask->mA0_cameraRotation = {};
    pNewTask->mAC = 0;
    pNewTask->mE4_cameraMode= 0;
    pNewTask->mDE_state = 0;

    s8 battleType = getBattleTypeId();
    switch (battleType)
    {
    case 0:
        pNewTask->mE4_cameraMode = 1;
        pNewTask->m94_cameraAngle = {};
        pNewTask->m94_cameraAngle.m4_Y = fixedPoint(0xA000);
        pNewTask->m94_cameraAngle.m8_Z = fixedPoint(0x14000);
        break;

    case 5:
        pNewTask->mAC = -0x76543;
        pNewTask->mA0_cameraRotation.m4_Y = -0x11C71C7;
        break;

    default:
        break;
    }

    if (battleType != 5)
    {
        if (type == 0)
            pNewTask->mAC = -0xB60B6;
        else
            pNewTask->mAC = -0x147AE1;

        u32 rnd = randomNumber();
        pNewTask->mA0_cameraRotation.m4_Y = (s32)0xFEAAAAAB - MTH_Mul(rnd >> 16, 0xB1C71C7);
    }

    // Shared camera + lighting + trail init (runs for all battle types)
    if (pNewTask->mE4_cameraMode == 0)
    {
        berserkCameraOrbit(&pNewTask->m88_cameraPosition, (s32)pNewTask->mA0_cameraRotation[1],
            pNewTask->m8_perQuadrantCameraAltitude.data());
    }
    else
    {
        battleEngineSub1_UpdateSub2(&pNewTask->m88_cameraPosition, pDragon->m8_position,
            pNewTask->m94_cameraAngle, pNewTask->mA0_cameraRotation);
    }
    battleEngine_enableAttackCamera();
    battleEngine_setCurrentCameraPositionPointer(&pNewTask->m88_cameraPosition);
    battleEngine_setDesiredCameraPositionPointer(&pNewTask->m18_laserOrigin);
    battleEngine_resetCameraInterpolation();

    pThis->m188_flags.m20000 = 1;
    syncM68KSoundCPU();

    sVec3_FP lightColor;
    battleResultScreen_updateSub0(0x1F, 0x1F, -0x1F, &lightColor);
    s_battleGrid* pGrid = gBattleManager->m10_battleOverlay->m8_gridTask;
    battleGrid_setupLightInterpolation(0, pGrid->m1CC_lightColor, lightColor);
    sVec3_FP lightFalloff;
    battleResultScreen_updateSub0(0x08, 0x08, 0x0A, &lightFalloff);
    battleGrid_setupLightInterpolation2(0, pGrid->m1E4_lightFalloff0, lightFalloff);

    u16 vdp1Param = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory;
    sSaturnPtr trailConfig1 = g_BTL_GenericData->getSaturnPtr(0x060adaa0);
    laserBerserk_initTrailBuffer(&pNewTask->m58_trails[0], pNewTask, &pNewTask->m18_laserOrigin, vdp1Param, trailConfig1);
    s32 result = 0xDF;
    if (pNewTask->mD7_laserType != 0)
    {
        sSaturnPtr trailConfig2 = g_BTL_GenericData->getSaturnPtr(0x060adb84);
        laserBerserk_initTrailBuffer(&pNewTask->m58_trails[1], pNewTask, &pNewTask->m24_laserEnd, vdp1Param, trailConfig2);
        result = 0;
    }
    return result;
}
