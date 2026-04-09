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
#include "kernel/fileBundle.h"

// FIXME audit findings:
// 1. [CRITICAL] Update state 2: phantomSlasher_getBaseDamage uses constant 0x50, should use pThis->mCA_speed
// 2. [CRITICAL] Init: "invalid first enemy + overlay camera" path doesn't set mD8_cameraQuadrant
//    (should be (dragonCurrentQuadrant + MTH_Mul(rnd>>16, 3) + 3) & 3)
// 3. [MEDIUM] Draw: missing dragonFieldTaskDrawSub3Sub1() call after lighting restore (projection stack imbalance)
// 4. [LOW] Update state 2: missing debug print block

//=============================================================================
// Cleansing Wave
//=============================================================================

struct sCleansingWaveTargetEntry
{
    s32 m0_timer;          // distance-based frame delay (or 0x4D for invalid)
    sVec3_FP m4_position;  // target position
    // size 0x10
};

struct sCleansingWaveTask : public s_workAreaTemplateWithCopy<sCleansingWaveTask>
{
    // 0x00: m0_fileBundle (inherited)
    // 0x04: m4_vd1Allocation (inherited)
    u8* m8_rawModelData;                 // 0x08
    s_3dModel mC_model;                  // 0x0C (Saturn size 0x4C, ends at 0x58)
    u8 m58_pad[4];                       // 0x58
    sVec3_FP m5C_velocity;               // 0x5C
    sVec3_FP m68_acceleration;           // 0x68
    sVec3_FP m74_cameraTarget;           // 0x74 -- also the spring position
    sVec3_FP m80_rotAccel;               // 0x80
    sVec3_FP m8C_rotVelocity;            // 0x8C
    sVec3_FP m98_rotation;               // 0x98 -- wave rotation angles (28-bit)
    sVec3_FP mA4_cameraPosition;         // 0xA4
    sVec3_FP mB0_cameraAngle;            // 0xB0
    sVec3_FP mBC_wavePosition;           // 0xBC -- wave sweep position
    s16 mC8_state;                       // 0xC8
    s16 mCA_speed;                       // 0xCA -- animation speed, starts 0xDC
    sCleansingWaveTargetEntry* mCC_targets; // 0xCC -- allocated target array
    u16 mD0_frameCounter;                // 0xD0
    u16 mD2_animFrame;                   // 0xD2
    u16 mD4_hitCount;                    // 0xD4
    u16 mD6_loopCount;                   // 0xD6
    s8 mD8_cameraQuadrant;              // 0xD8
    u8 mD9_pad[3];                      // 0xD9
    s32 mDC_hasCameraData;               // 0xDC
    s32 mE0_hasModel;                    // 0xE0
    s32 mE4_hasLighting;                 // 0xE4
    s32 mE8_hasCamera;                   // 0xE8
    // size 0xEC
};

// BTL_A3::0609b4d0
static void sCleansingWaveTask_Delete(sCleansingWaveTask* pThis)
{
    s_battleGrid* pGrid = gBattleManager->m10_battleOverlay->m8_gridTask;

    if (pThis->mE8_hasCamera != 0)
    {
        pGrid->m64_cameraRotationTarget.m8_Z = 0;
        battleEngine_restoreCameraAfterEnemyAttack();
    }
    if (pThis->mE4_hasLighting != 0)
    {
        battleGrid_setupLightInterpolation(10, pGrid->m1CC_lightColor,
            pGrid->m1D8_newLightColor);
        battleGrid_setupLightInterpolation2(10, pGrid->m1E4_lightFalloff0,
            pGrid->m1F0);
        resetProjectVector();
        pThis->mE4_hasLighting = 0;
    }
    gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished = 1;
}

// BTL_A3::0609b2d8
static void sCleansingWaveTask_Draw(sCleansingWaveTask* pThis)
{
    s_battleGrid* pGrid = gBattleManager->m10_battleOverlay->m8_gridTask;
    s_battleDragon* pDragon = gBattleManager->m10_battleOverlay->m18_dragon;

    sVec3_FP lightDir = {};
    if (pThis->mE0_hasModel != 0)
    {
        if (pThis->mE4_hasLighting != 0)
        {
            // Set up custom lighting for the wave model
            pushProjectionStack();
            battleEngine_UpdateSub7Sub1Sub0(&pGrid->m280_lightAngle1, lightDir);
            setupLight(lightDir.m0_X, lightDir.m4_Y, lightDir.m8_Z, 0);
            generateLightFalloffMap(0x101010, 0, 0);
        }

        // Draw model at dragon position
        pushCurrentMatrix();
        translateCurrentMatrix(&pDragon->m8_position);
        pThis->mC_model.m18_drawFunction(&pThis->mC_model);
        popMatrix();

        if (pThis->mE4_hasLighting != 0)
        {
            // Restore lighting from grid values
            u32 lightColorRGB =
                (u32)fpToColorByte(pGrid->m1CC_lightColor[2]) << 16 |
                (u32)fpToColorByte(pGrid->m1CC_lightColor[1]) << 8 |
                (u32)fpToColorByte(pGrid->m1CC_lightColor[0]);
            setupLight(lightDir.m0_X, lightDir.m4_Y, lightDir.m8_Z, lightColorRGB);

            u32 falloff0RGB =
                (u32)fpToColorByte(pGrid->m1E4_lightFalloff0[2]) << 16 |
                (u32)fpToColorByte(pGrid->m1E4_lightFalloff0[1]) << 8 |
                (u32)fpToColorByte(pGrid->m1E4_lightFalloff0[0]);
            u32 falloff1RGB =
                (u32)fpToColorByte(pGrid->m1FC_lightFalloff1.m8_Z) << 16 |
                (u32)fpToColorByte(pGrid->m1FC_lightFalloff1.m4_Y) << 8 |
                (u32)fpToColorByte(pGrid->m1FC_lightFalloff1.m0_X);
            u32 falloff2RGB =
                (u32)fpToColorByte(pGrid->m208_lightFalloff2.m8_Z) << 16 |
                (u32)fpToColorByte(pGrid->m208_lightFalloff2.m4_Y) << 8 |
                (u32)fpToColorByte(pGrid->m208_lightFalloff2.m0_X);
            generateLightFalloffMap(falloff0RGB, falloff1RGB, falloff2RGB);

            // 0601e1c8: dragonFieldTaskDrawSub3Sub1 -- popProjectionStack + Saturn VDP sync
            // The push/pop projection stack manages the Saturn rendering pipeline state.
            // In the bgfx reimplementation, this is handled differently.
            // TODO: implement popProjectionStack when the projection stack system is complete
        }
    }

    if (pThis->mE4_hasLighting != 0)
    {
        // Lens flare at wave position
        sVec3_FP wavePos;
        wavePos.m0_X = pThis->mBC_wavePosition[0];
        wavePos.m4_Y = pThis->mBC_wavePosition[1] + fixedPoint(0x3800);
        wavePos.m8_Z = pThis->mBC_wavePosition[2];

        sVec3_FP screenPos;
        transformAndAddVecByCurrentMatrix(&wavePos, &screenPos);
        dragonFieldTaskDrawSub1Sub1(screenPos.m0_X, screenPos.m4_Y, screenPos.m8_Z, 0x19000);
    }
}

// BTL_A3::0609a54c
static void sCleansingWaveTask_Update(sCleansingWaveTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    s_battleDragon* pDragon = gBattleManager->m10_battleOverlay->m18_dragon;
    s_battleGrid* pGrid = gBattleManager->m10_battleOverlay->m8_gridTask;
    sBattleOverlayTask_C* pTargetSystem = gBattleManager->m10_battleOverlay->mC_targetSystem;
    s16 numTargets = pTargetSystem->m20A_numSelectableEnemies;

    // --- Section 1: Update target positions every frame ---
    for (s32 i = 0; i < numTargets; i++)
    {
        s_battleEnemy* enemy = pTargetSystem->m0_enemyTargetables[i];
        if (enemy->m4_targetable == nullptr ||
            (enemy->m4_targetable->m50_flags & 0x140003) != 0 ||
            enemy->m0_isActive < 1)
        {
            // Invalid target -- drift with auto-scroll
            pThis->mCC_targets[i].m4_position += pEngine->m1A0_battleAutoScrollDelta;
        }
        else
        {
            // Valid target -- track live position
            sVec3_FP* enemyPos = getBattleTargetablePosition(*enemy->m4_targetable);
            pThis->mCC_targets[i].m4_position = *enemyPos;
        }
    }

    // --- Section 2: Spring physics for camera target toward battle center ---
    pThis->m74_cameraTarget += pEngine->m1A0_battleAutoScrollDelta;

    sVec3_FP accel;
    accel[0] = MTH_Mul(pEngine->mC_battleCenter[0] - pThis->m74_cameraTarget[0], fixedPoint(0x28F));
    accel[1] = MTH_Mul(pEngine->mC_battleCenter[1] - pThis->m74_cameraTarget[1], fixedPoint(0x28F));
    accel[2] = MTH_Mul(pEngine->mC_battleCenter[2] - pThis->m74_cameraTarget[2], fixedPoint(0x28F));

    pThis->m5C_velocity[0] += accel[0];
    pThis->m5C_velocity[1] += accel[1];
    pThis->m5C_velocity[2] += accel[2];

    pThis->m68_acceleration[0] += pThis->m5C_velocity[0];
    pThis->m68_acceleration[1] += pThis->m5C_velocity[1];
    pThis->m68_acceleration[2] += pThis->m5C_velocity[2];

    sVec3_FP damp;
    damp[0] = MTH_Mul(pThis->m68_acceleration[0], fixedPoint(0x3333));
    damp[1] = MTH_Mul(pThis->m68_acceleration[1], fixedPoint(0x3333));
    damp[2] = MTH_Mul(pThis->m68_acceleration[2], fixedPoint(0x3333));

    pThis->m68_acceleration[0] -= damp[0];
    pThis->m68_acceleration[1] -= damp[1];
    pThis->m68_acceleration[2] -= damp[2];

    pThis->m74_cameraTarget[0] += pThis->m68_acceleration[0];
    pThis->m74_cameraTarget[1] += pThis->m68_acceleration[1];
    pThis->m74_cameraTarget[2] += pThis->m68_acceleration[2];

    pThis->m5C_velocity = {};

    // --- Section 3: Rotation spring physics (spin toward 0x1555555 on Z) ---
    s32 rotDelta = signExtend28(0x1555555 - (s32)pThis->m98_rotation[2]);
    pThis->m80_rotAccel[2] += MTH_Mul(fixedPoint(rotDelta), fixedPoint(0x170A));

    pThis->m8C_rotVelocity[0] += pThis->m80_rotAccel[0];
    pThis->m8C_rotVelocity[1] += pThis->m80_rotAccel[1];
    pThis->m8C_rotVelocity[2] += pThis->m80_rotAccel[2];

    damp[0] = MTH_Mul(pThis->m8C_rotVelocity[0], fixedPoint(0x9999));
    damp[1] = MTH_Mul(pThis->m8C_rotVelocity[1], fixedPoint(0x9999));
    damp[2] = MTH_Mul(pThis->m8C_rotVelocity[2], fixedPoint(0x9999));

    pThis->m8C_rotVelocity[0] -= damp[0];
    pThis->m8C_rotVelocity[1] -= damp[1];
    pThis->m8C_rotVelocity[2] -= damp[2];

    pThis->m98_rotation[0] += pThis->m8C_rotVelocity[0];
    pThis->m98_rotation[1] += pThis->m8C_rotVelocity[1];
    pThis->m98_rotation[2] += pThis->m8C_rotVelocity[2];
    pThis->m98_rotation[0] = (s32)pThis->m98_rotation[0] & 0xFFFFFFF;
    pThis->m98_rotation[1] = (s32)pThis->m98_rotation[1] & 0xFFFFFFF;
    pThis->m98_rotation[2] = (s32)pThis->m98_rotation[2] & 0xFFFFFFF;
    pThis->m80_rotAccel = {};

    // --- Section 4: Camera update ---
    if (pThis->mDC_hasCameraData == 0)
    {
        // Computed camera: add Y rotation offset, use battleEngineSub1_UpdateSub2
        pThis->m98_rotation[1] = (s32)pThis->m98_rotation[1] + 0xB60B6;
        sVec3_FP cameraAngle;
        cameraAngle[0] = pThis->m98_rotation[0];
        cameraAngle[1] = pThis->m98_rotation[1];
        cameraAngle[2] = 0;
        battleEngineSub1_UpdateSub2(&pThis->mA4_cameraPosition, pDragon->m8_position,
            pThis->mB0_cameraAngle, cameraAngle);

        // Clamp camera Y above target system's minimum
        s32 minY = pTargetSystem->m204_cameraMaxAltitude;
        if ((s32)pThis->mA4_cameraPosition[1] <= minY)
        {
            pThis->mA4_cameraPosition[1] = fixedPoint(minY + 0x1000);
        }
    }
    else
    {
        // Overlay camera data
        sVec3_FP camOffset = readOverlayCameraOffset(pEngine, pThis->mD8_cameraQuadrant);
        pThis->mA4_cameraPosition[0] = (s32)camOffset[0] + pEngine->mC_battleCenter[0];
        pThis->mA4_cameraPosition[1] = (s32)camOffset[1] + pEngine->mC_battleCenter[1];
        pThis->mA4_cameraPosition[2] = (s32)camOffset[2] + pEngine->mC_battleCenter[2];
    }

    // --- Section 5: State machine ---
    s16 state = pThis->mC8_state;
    if (state == 0)
    {
        // State 0: wait for file loading, init model, camera, lighting
        if (fileInfoStruct.m2C_allocatedHead != nullptr)
            goto animationUpdate;

        // Init model from mA9C file data
        pThis->m8_rawModelData = pEngine->mA9C_berzerkCustomModel;
        s_fileBundle* pBundle = (s_fileBundle*)pThis->m8_rawModelData;
        u32 poseFileOffset = pBundle->getRawFileOffset(0x4C);
        s32 initResult = init3DModelRawData(pThis, &pThis->mC_model, 0, pBundle, 4,
            pBundle->getAnimation(0x50),
            pBundle->getStaticPose(poseFileOffset, pBundle->getModelHierarchy(4)->countNumberOfBones()),
            nullptr, nullptr);
        if (initResult == 0)
        {
            pThis->getTask()->markFinished();
            return;
        }

        // Camera setup
        battleEngine_enableAttackCamera();
        battleEngine_setCurrentCameraPositionPointer(&pThis->mA4_cameraPosition);
        battleEngine_setDesiredCameraPositionPointer(&pThis->m74_cameraTarget);
        battleEngine_resetCameraInterpolation();
        pThis->mE8_hasCamera = 1;
        pEngine->m188_flags.m20000 = 1;
        syncM68KSoundCPU();

        // Lighting setup
        sVec3_FP lightColor;
        battleResultScreen_updateSub0(0, 0x1F, 0x0F, &lightColor);
        battleGrid_setupLightInterpolation(0, pGrid->m1CC_lightColor, lightColor);

        sVec3_FP lightFalloff;
        battleResultScreen_updateSub0(5, 0x0A, 0x0A, &lightFalloff);
        battleGrid_setupLightInterpolation2(10, pGrid->m1E4_lightFalloff0, lightFalloff);

        pThis->mE4_hasLighting = 1;
        pThis->mE0_hasModel = 1;
        pThis->mC8_state++;
        pThis->mD0_frameCounter = 0;

        // Fall through to state 0/1 shared code
        goto state0_1_shared;
    }
    else if (state == 1)
    {
state0_1_shared:
        // State 0/1 shared: wait 0x33 frames, then advance and set wave position
        u16 frame = pThis->mD0_frameCounter;
        pThis->mD0_frameCounter = frame + 1;
        if (frame < 0x33)
            goto animationUpdate;

        pThis->mC8_state++;
        pThis->mD0_frameCounter = 0;
        pThis->mBC_wavePosition = pDragon->m8_position;
    }
    else if (state == 2)
    {
        // Handled below (wave sweep code)
    }
    else if (state == 3)
    {
        // State 3: wait for animation to finish
        if (pThis->mE0_hasModel == 0)
        {
            pThis->mC8_state++;
            pThis->mD0_frameCounter = 0;
            if (pThis->mE4_hasLighting != 0)
            {
                battleGrid_setupLightInterpolation(10, pGrid->m1CC_lightColor,
                    pGrid->m1D8_newLightColor);
                battleGrid_setupLightInterpolation2(10,
                    pGrid->m1E4_lightFalloff0,
                    pGrid->m1F0);
                resetProjectVector();
                pThis->mE4_hasLighting = 0;
            }
        }
        goto animationUpdate;
    }
    else if (state == 4)
    {
        // State 4: fade out, restore camera, finish
        s16 timer = (s16)pThis->mD0_frameCounter;
        pThis->mD0_frameCounter = timer + 1;
        if (timer == 0xF)
        {
            pGrid->m64_cameraRotationTarget.m8_Z = 0;
            battleEngine_restoreCameraAfterEnemyAttack();
            pThis->mE8_hasCamera = 0;
        }
        else if (timer == 0x1E)
        {
            pThis->getTask()->markFinished();
        }
        goto animationUpdate;
    }
    else
    {
        goto animationUpdate;
    }

    // --- State 2: Wave sweep (also entered after state 0/1 transition) ---
    {
        // Move wave position toward battle center
        s32 stepX = ((s32)pEngine->mC_battleCenter[0] - (s32)pDragon->m8_position[0]) >> 4;
        s32 stepZ = ((s32)pEngine->mC_battleCenter[2] - (s32)pDragon->m8_position[2]) >> 4;

        pThis->mBC_wavePosition[0] += fixedPoint(stepX);
        // Y doesn't move
        pThis->mBC_wavePosition[2] += fixedPoint(stepZ);
        pThis->mBC_wavePosition += pEngine->m1A0_battleAutoScrollDelta;

        // Check targets -- damage when frame counter matches target timer
        for (s32 i = 0; i < numTargets; i++)
        {
            if ((u32)pThis->mD0_frameCounter == (u32)pThis->mCC_targets[i].m0_timer)
            {
                pThis->mD4_hitCount++;

                s_battleEnemy* enemy = pTargetSystem->m0_enemyTargetables[i];
                if (enemy->m4_targetable != nullptr &&
                    (enemy->m4_targetable->m50_flags & 0x140003) == 0 &&
                    enemy->m0_isActive > 0)
                {
                    // Compute damage direction
                    sVec3_FP dir;
                    dir[0] = pThis->mCC_targets[i].m4_position[0] - pDragon->m8_position[0];
                    dir[1] = pThis->mCC_targets[i].m4_position[1] - pDragon->m8_position[1];
                    dir[2] = pThis->mCC_targets[i].m4_position[2] - pDragon->m8_position[2];

                    // FUN_0608d7f0 -- SPR-scaled damage. Base is mCA_speed (decays with time)
                    s16 baseDmg = phantomSlasher_getBaseDamage(pThis->mCA_speed);
                    s16 damage = sGunShotTask_UpdateSub1Sub2(enemy->m4_targetable, baseDmg, 2);
                    applyDamageToEnnemy(enemy->m4_targetable, damage,
                        &pThis->mCC_targets[i].m4_position, 3, dir, fixedPoint(0x2000));

                    berserk_createHitFlashFade(pThis, 0, 0xC210, 0xCA97, 5, 0xCA97, 0xC210, 5);

                    randomNumber(); // consumed
                    MTH_Mul(randomNumber() >> 16, fixedPoint(0x1C71C72)); // consumed
                    return; // return after first hit per frame
                }
            }
        }

        // No target hit this frame -- advance counter
        pThis->mD0_frameCounter++;
        pGrid->m64_cameraRotationTarget.m8_Z = pThis->m98_rotation[2];

        // Update speed: mCA_speed = 0xDC - frameCounter*2, clamped to 0x96
        pThis->mCA_speed = (s16)(0xDC - (s16)pThis->mD0_frameCounter * 2);
        if (pThis->mCA_speed < 0x96)
            pThis->mCA_speed = 0x96;

        // Check completion: all targets hit OR frame counter > 0x4B
        if ((u32)pThis->mD4_hitCount == (u32)(s32)numTargets || pThis->mD0_frameCounter > 0x4B)
        {
            pThis->mD6_loopCount = 3;
            pThis->mC8_state++;
        }
    }

animationUpdate:
    // --- Section 6: Animation update (runs every frame when model active) ---
    if (pThis->mE0_hasModel != 0)
    {
        s32 totalFrames = 0;
        if (pThis->mC_model.m30_pCurrentAnimation != nullptr)
        {
            totalFrames = (s32)*(s16*)((u8*)pThis->mC_model.m30_pCurrentAnimation + 4);
        }

        u16 animFrame = pThis->mD2_animFrame + 1;
        pThis->mD2_animFrame = animFrame;

        if ((s32)(u32)animFrame < totalFrames)
        {
            stepAnimation(&pThis->mC_model);
            pGrid->m64_cameraRotationTarget.m8_Z = pThis->m98_rotation[2];
            if (pThis->mD2_animFrame == 0x14)
            {
                playSystemSoundEffect(0x11);
            }
        }
        else
        {
            // Animation loop complete
            pThis->mD2_animFrame = 0;
            u16 loopCount = pThis->mD6_loopCount;
            pThis->mD6_loopCount = loopCount + 1;
            if (loopCount > 3)
            {
                pThis->mE0_hasModel = 0;
                pThis->m_DrawMethod = nullptr;
            }
        }
    }
}

// BTL_A3::060a3a68 -- cross-product side check
static s32 computeRelativeSide(sVec3_FP* pDragonPos, sVec3_FP& battleCenter, sVec3_FP* pEnemyPos)
{
    s32 cross = MTH_Mul(battleCenter.m8_Z - (*pDragonPos)[2], (*pEnemyPos)[0] - (*pDragonPos)[0])
              + MTH_Mul(-(battleCenter.m0_X - (*pDragonPos)[0]), (*pEnemyPos)[2] - (*pDragonPos)[2]);
    if (cross > 0)
        return 0;
    return 1;
}

// BTL_A3::06099f40
void berserk_createCleansingWave(s_battleEngine* pThis)
{
    static const sCleansingWaveTask::TypedTaskDefinition definition = {
        nullptr,
        (void(*)(sCleansingWaveTask*))&sCleansingWaveTask_Update,
        (void(*)(sCleansingWaveTask*))&sCleansingWaveTask_Draw,
        (void(*)(sCleansingWaveTask*))&sCleansingWaveTask_Delete,
    };

    sCleansingWaveTask* pNewTask = createSubTaskWithCopy<sCleansingWaveTask>(pThis, &definition);
    if (pNewTask == nullptr)
    {
        pThis->m188_flags.m100_attackAnimationFinished = 1;
        return;
    }

    // Zero the state flags
    pNewTask->mE0_hasModel = 0;
    pNewTask->mE4_hasLighting = 0;
    pNewTask->mE8_hasCamera = 0;

    s_battleDragon* pDragon = gBattleManager->m10_battleOverlay->m18_dragon;
    sBattleOverlayTask_C* pTargetSystem = gBattleManager->m10_battleOverlay->mC_targetSystem;
    s16 numTargets = pTargetSystem->m20A_numSelectableEnemies;

    if (numTargets < 1)
    {
        // No targets -- finish immediately
        pNewTask->getTask()->markFinished();
        return;
    }

    // Allocate target array (0x10 bytes per target)
    sCleansingWaveTargetEntry* targets = (sCleansingWaveTargetEntry*)allocateHeapForTask(
        pNewTask, numTargets * sizeof(sCleansingWaveTargetEntry));
    pNewTask->mCC_targets = targets;
    if (targets == nullptr)
    {
        pNewTask->getTask()->markFinished();
        return;
    }

    // Zero physics state
    pNewTask->m5C_velocity = {};
    pNewTask->m68_acceleration = {};
    pNewTask->m80_rotAccel = {};
    pNewTask->m8C_rotVelocity = {};
    pNewTask->m98_rotation = {};

    // Init target entries -- compute distance-based timer for each
    for (s32 i = 0; i < numTargets; i++)
    {
        s_battleEnemy* enemy = pTargetSystem->m0_enemyTargetables[i];
        if (enemy->m4_targetable == nullptr ||
            (enemy->m4_targetable->m50_flags & 0x140003) != 0 ||
            enemy->m0_isActive < 1)
        {
            // Invalid target -- use battle center with max timer
            targets[i].m0_timer = 0x4D;
            targets[i].m4_position = pThis->mC_battleCenter;
        }
        else
        {
            // Compute distance to dragon along the appropriate axis
            sVec3_FP* enemyPos = getBattleTargetablePosition(*enemy->m4_targetable);
            s32 dist = 0;
            s8 quadrant = pThis->m22C_dragonCurrentQuadrant;
            if (quadrant == 0 || quadrant == 2)
            {
                // Compare Z axis
                s32 dz = (s32)(*enemyPos)[2] - (s32)pDragon->m8_position[2];
                dist = (dz < 0) ? -dz : dz;
            }
            else // quadrant 1 or 3
            {
                // Compare X axis
                s32 dx = (s32)(*enemyPos)[0] - (s32)pDragon->m8_position[0];
                dist = (dx < 0) ? -dx : dx;
            }

            targets[i].m4_position = *enemyPos;
            targets[i].m0_timer = dist >> 0xD;
            if (targets[i].m0_timer > 0x4B)
                targets[i].m0_timer = 0x4B;
        }
    }

    // Init state
    pNewTask->mC8_state = 0;

    // Check if we should use overlay camera data
    s8 battleType = getBattleTypeId();
    bool useOverlayCamera = false;
    if (gBattleManager->m4 == 0 &&
        (gBattleManager->m8 == 0 || gBattleManager->m8 == 1 ||
         gBattleManager->m8 == 2 || gBattleManager->m8 == 6 || gBattleManager->m8 == 7))
    {
        useOverlayCamera = true;
    }

    if (((battleType == 5 || battleType == 6 || useOverlayCamera ||
         (randomNumber() & 3) == 0) &&
        battleType != 1 && gBattleManager->m4 != 7) &&
        getOverlayCameraData(pThis).m_offset != 0)
    {
        // Use overlay camera data
        pNewTask->mDC_hasCameraData = 1;

        // Determine camera quadrant
        s_battleEnemy* firstEnemy = pTargetSystem->m0_enemyTargetables[0];
        if (battleType == 3 &&
            firstEnemy->m4_targetable != nullptr &&
            (firstEnemy->m4_targetable->m50_flags & 0x140003) == 0 &&
            firstEnemy->m0_isActive >= 1)
        {
            sVec3_FP* enemyPos = getBattleTargetablePosition(*firstEnemy->m4_targetable);
            s32 side = computeRelativeSide(&pDragon->m8_position, pThis->mC_battleCenter, enemyPos);
            if (side == 0)
                pNewTask->mD8_cameraQuadrant = (pThis->m22C_dragonCurrentQuadrant + 3) & 3;
            else
                pNewTask->mD8_cameraQuadrant = (pThis->m22C_dragonCurrentQuadrant + 1) & 3;
        }
        else
        {
            u32 rnd = randomNumber();
            MTH_Mul(rnd >> 16, fixedPoint(3)); // random quadrant offset (result not stored -- matches Ghidra)
        }
    }
    else
    {
        // No overlay camera -- compute look-at based camera
        pNewTask->mDC_hasCameraData = 0;

        sVec3_FP dir;
        dir[0] = pThis->mC_battleCenter[0] - pDragon->m8_position[0];
        dir[1] = pThis->mC_battleCenter[1] - pDragon->m8_position[1];
        dir[2] = pThis->mC_battleCenter[2] - pDragon->m8_position[2];

        sVec2_FP lookAt;
        computeLookAt(dir, lookAt);

        pNewTask->m98_rotation[0] = lookAt[0];
        pNewTask->m98_rotation[1] = lookAt[1];
        pNewTask->m98_rotation[2] = 0;
        pNewTask->m98_rotation[1] = (s32)pNewTask->m98_rotation[1] + (s32)fixedPoint(0xFE000000);

        pNewTask->mB0_cameraAngle[0] = 0;
        if ((s32)pDragon->m8_position[1] < (s32)pThis->mC_battleCenter[1])
            pNewTask->mB0_cameraAngle[1] = fixedPoint(0xFFFFE000);
        else
            pNewTask->mB0_cameraAngle[1] = fixedPoint(0x8000);
        pNewTask->mB0_cameraAngle[2] = fixedPoint(0xA000);
    }

    // Init position from dragon
    pNewTask->m74_cameraTarget = pDragon->m8_position;

    // Init animation/timing
    pNewTask->mCA_speed = 0xDC;
    pNewTask->mD0_frameCounter = 0;
    pNewTask->mD4_hitCount = 0;
    pNewTask->mD6_loopCount = 0;
}
