#include "PDS.h"
#include "BTL_X0_enemy.h"
#include "BTL_X0_data.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "battle/battleEngine.h"
#include "battle/battleEnemyModels.h"
#include "battle/battleTargetable.h"
#include "battle/battleEnemyLifeMeter.h"
#include "kernel/graphicalObject.h"
#include "kernel/animation.h"
#include "kernel/fileBundle.h"
#include "mainMenuDebugTasks.h"
#include "audio/soundDriver.h"

#include "battle/battleTextDisplay.h"
#include "battle/battleDebug.h"
#include "battle/battleGrid.h"
#include "battle/battleIntro.h"
#include "battle/itemVisualEffect.h"
#include "battle/battleFormationIntroEffect.h"
#include "battle/BTL_A3/BTL_A3_UrchinFormation.h"
#include "audio/systemSounds.h"
#include "battle/battleDragon.h"
#include "battle/gunShotRootTask.h"
#include "battle/battleOverlay_C.h"
#include "battle/battleDamageDisplay.h"

// 06063544
static void BTL_X0_updateBodyAnimation(sBTL_X0_EnemyModel* pThis)
{
    if (pThis->m310_attackActive == 1)
        updateAndInterpolateAnimation(&pThis->m98_models[0]);
    else
        stepAnimation(&pThis->m98_models[0]);
}

// Stubs for command handlers
// 0606335c
static void BTL_X0_cmd_idle(sBTL_X0_EnemyModel* pThis)
{
    if (battleEngine_isPlayerTurnActive() != 0)
        return;

    s32 done = vec2FPInterpolator_Step(&pThis->m1AC_interpolator);
    if (done != 0)
    {
        pThis->m1AC_interpolator.mC_startValue = pThis->m7C_position;

        if (pThis->m30E_flag2 == 1)
        {
            pThis->m1AC_interpolator.m24_targetValue.m0_X = (randomNumber() % 0x14000) - 0xA000;
            pThis->m1AC_interpolator.m24_targetValue.m4_Y = (randomNumber() % 0x14000) - 0xA000;
            if (pThis->m308_variantIndex == 2)
                pThis->m1AC_interpolator.m24_targetValue.m8_Z = (randomNumber() % 0x14000) - 0x19000;
            else
                pThis->m1AC_interpolator.m24_targetValue.m8_Z = (randomNumber() % 0x14000) - 0xA000;
            pThis->m1AC_interpolator.m38_interpolationLength = 0x5A;
        }
        else
        {
            pThis->m1AC_interpolator.m24_targetValue.m0_X = 0;
            pThis->m1AC_interpolator.m24_targetValue.m4_Y = 0;
            pThis->m1AC_interpolator.m24_targetValue.m8_Z = 0;
            pThis->m1AC_interpolator.m38_interpolationLength = 0x3C;
        }

        sSaturnPtr baseTable = g_BTL_X0->getSaturnPtr(0x060b7f18) + (s8)(pThis->m308_variantIndex * 0xC);
        pThis->m1AC_interpolator.m24_targetValue.m0_X += readSaturnS32(baseTable);
        pThis->m1AC_interpolator.m24_targetValue.m4_Y += readSaturnS32(baseTable + 4);
        pThis->m1AC_interpolator.m24_targetValue.m8_Z += readSaturnS32(baseTable + 8);

        vec2FPInterpolator_Init(&pThis->m1AC_interpolator);
    }

    pThis->m7C_position.m0_X = pThis->m1AC_interpolator.m0_currentValue.m0_X;
    pThis->m7C_position.m4_Y = pThis->m1AC_interpolator.m0_currentValue.m4_Y;
    pThis->m7C_position.m8_Z = pThis->m1AC_interpolator.m0_currentValue.m8_Z;
}
void battleEngine_displayAttackName(int param1, int param2, int param3);
s32 createBattleCommandMenuSub2(s32 param1);
static void BTL_X0_playHitReaction(sBTL_X0_EnemyModel* pThis, s8 param1, s32 interpLength);
static void BTL_X0_fireProjectile(sBTL_X0_EnemyModel* pThis, void* hotpointData, s16 param3, s8 animIdx, s8 projectileIdx);
static void BTL_X0_copyTargetablePositions(sBTL_X0_EnemyModel* pThis);
static void BTL_X0_checkTargetableCollisions(sBTL_X0_EnemyModel* pThis);
static s32 BTL_X0_cmd_genericAttack(sBTL_X0_EnemyModel* pThis);
static void BTL_X0_resetEnemyCommandState(sBTL_X0_EnemyModel* pThis);
static void BTL_X0_initCameraForAttack(sBTL_X0_EnemyModel* pThis);
static void BTL_X0_stepCameraRotation(sBTL_X0_EnemyModel* pThis);
static void BTL_X0_restoreCameraAfterAttack();
static void BTL_X0_updateCameraPositions(sBTL_X0_EnemyModel* pThis);
static void BTL_X0_initBodyAnimation(sBTL_X0_EnemyModel* pThis, s8 animIdx);
static void BTL_X0_playBodyAnimation(sBTL_X0_EnemyModel* pThis, s8 animIdx, s32 interpLength);
static void BTL_X0_updateGridInterpolation(sBTL_X0_EnemyModel* pThis, s32 param2, s32 param3);
static u8 BTL_X0_lightColorTable_cache(int idx);
static s32 BTL_X0_isAnimationFinished(sBTL_X0_EnemyModel* pThis);

// 060644a2
static void BTL_X0_cmd_attack2(sBTL_X0_EnemyModel* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    u8 subState = pThis->m307_subState;

    if (subState == 0)
    {
        s8 quadrant = pEngine->m22C_dragonCurrentQuadrant;
        if (quadrant == 0)
        {
            pThis->m305_attackQuadrantBit = 1;
            battleEngine_displayAttackName(0x12, 0x1E, 0);
        }
        else if (quadrant == 2)
        {
            pThis->m305_attackQuadrantBit = 4;
            battleEngine_displayAttackName(1, 0x1E, 0);
        }
        battleEngine_FlagQuadrantBitForAttack((u16)pThis->m305_attackQuadrantBit);
        pThis->m307_subState = 1;
    }
    else if (subState == 1)
    {
        s8 quadrant = pEngine->m22C_dragonCurrentQuadrant;
        if (quadrant == 0)
        {
            battleEngine_initiateEnemyMoveDragon(3, 2);
            pThis->m307_subState = 2;
        }
        else if (quadrant == 2)
        {
            pThis->m307_subState = 3;
        }
    }
    else if (subState == 2)
    {
        if (battleEngine_isBattleIntroFinished())
        {
            pThis->mF0_frameCounter = 0;
            pThis->m307_subState = 4;
            battleEngine_SetBattleMode(eBattleModes::m9);
        }
    }
    else if (subState == 3)
    {
        pThis->mF0_frameCounter++;
        if (pThis->mF0_frameCounter >= 0x3C)
        {
            BTL_X0_playHitReaction(pThis, 1, 5);
            pThis->mF0_frameCounter = 0;
            pThis->m307_subState = 5;
            pThis->mF2_animIndex = (s16)performModulo2(6, randomNumber());
            pThis->mF4_projectileIndex = (s16)(randomNumber() & 7);
        }
    }
    else if (subState == 4)
    {
        if ((pEngine->m188_flags.m2000) == 0)
            return;
        BTL_X0_playHitReaction(pThis, 1, 5);
        pThis->m307_subState = 5;
        pThis->mF0_frameCounter = 0;
        pThis->mF2_animIndex = (s16)performModulo2(6, randomNumber());
        pThis->mF4_projectileIndex = (s16)(randomNumber() & 7);
    }
    else if (subState == 5)
    {
        s16 counter = pThis->mF0_frameCounter;
        pThis->mF0_frameCounter = counter + 1;

        if (counter > 0x34)
        {
            pEngine->m188_flags.m100_attackAnimationFinished = 1;
            pThis->m307_subState = 0;
            pThis->m30A_commandIndex = 0;
        }

        s16 frame = pThis->mF0_frameCounter;
        if (frame == 0x20 || frame == 0x23 || frame == 0x26 ||
            frame == 0x29 || frame == 0x2C || frame == 0x2F)
        {
            s16 projectileType = readSaturnS16(g_BTL_X0->getSaturnPtr(0x060b7f48) + (u8)pThis->m308_variantIndex * 2);
            BTL_X0_fireProjectile(pThis, pThis->m1A8_attackDataBuffer, projectileType,
                (s8)pThis->mF2_animIndex, (s8)pThis->mF4_projectileIndex);
            pThis->mF2_animIndex++;
            pThis->mF4_projectileIndex++;
        }
    }
}
// 06064ae0
static void BTL_X0_startCameraEffect(sBTL_X0_EnemyModel* pThis, u8 param)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    if (pEngine->m3D0 == nullptr)
    {
        pThis->m309_cameraEffectType = param;
        u16 effectIndex = readSaturnU16(g_BTL_X0->getSaturnPtr(0x060b7e8c) + (u8)pThis->m309_cameraEffectType * 2);
        pEngine->m3D0 = createItemVisualEffect(
            dramAllocatorEnd[6].mC_fileBundle, effectIndex,
            &pThis->m1C_lifeMeterPosition, &pThis->m1C_lifeMeterPosition, 1, 1);
    }
}

// 06063ca6
static void BTL_X0_cmd_attack5(sBTL_X0_EnemyModel* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    u8 subState = pThis->m307_subState;

    if (subState == 0)
    {
        if ((pEngine->m188_flags.m2000) == 0)
            return;

        battleEngine_displayAttackName(1, 0x1E, 0);
        pThis->m307_subState++;
        pThis->mF0_frameCounter = 0;

        s8 variant = pThis->m308_variantIndex;
        if (variant == 0 || variant == 1 || variant == 3)
        {
            BTL_X0_playHitReaction(pThis, 1, 5);
            BTL_X0_startCameraEffect(pThis, 4);
        }

        s8 quadrant = pEngine->m22C_dragonCurrentQuadrant;
        if (quadrant == 0)
            pThis->m305_attackQuadrantBit = 1;
        else if (quadrant == 1)
            pThis->m305_attackQuadrantBit = 2;
        else if (quadrant == 2)
            pThis->m305_attackQuadrantBit = 4;
        else if (quadrant == 3)
            pThis->m305_attackQuadrantBit = 8;

        battleEngine_FlagQuadrantBitForAttack((u16)pThis->m305_attackQuadrantBit);

        pThis->mF2_animIndex = (s16)performModulo2(6, randomNumber());
        pThis->mF4_projectileIndex = (s16)(randomNumber() & 7);
        return;
    }

    if (subState != 1)
        return;

    s16 counter = pThis->mF0_frameCounter;
    pThis->mF0_frameCounter = counter + 1;

    if (counter > 0x34)
    {
        pEngine->m188_flags.m100_attackAnimationFinished = 1;
        pThis->m307_subState = 0;
        pThis->m30A_commandIndex = 0;
    }

    if (pThis->mF0_frameCounter == 0x27 && pEngine->m3D0 != nullptr)
    {
        pEngine->m3D0->getTask()->markFinished();
        pEngine->m3D0 = nullptr;
    }

    s8 dragonQuadrant = pEngine->m22C_dragonCurrentQuadrant;
    s16 frame = pThis->mF0_frameCounter;
    s16 projectileType = readSaturnS16(g_BTL_X0->getSaturnPtr(0x060b7f48) + (u8)pThis->m308_variantIndex * 2);

    if (dragonQuadrant == 0 || dragonQuadrant == 2)
    {
        if (frame == 0x20 || frame == 0x23 || frame == 0x26 ||
            frame == 0x29 || frame == 0x2C || frame == 0x2F)
        {
            void* hotpointData = (pThis->m308_variantIndex == 2)
                ? (void*)((u8*)pThis->m1A8_attackDataBuffer + 0xA8)
                : pThis->m1A8_attackDataBuffer;

            BTL_X0_fireProjectile(pThis, hotpointData, projectileType,
                (s8)pThis->mF2_animIndex, (s8)pThis->mF4_projectileIndex);
            pThis->mF2_animIndex++;
            pThis->mF4_projectileIndex++;
        }
    }
    else
    {
        if (frame == 0x20 || frame == 0x26 || frame == 0x2C)
        {
            void* hotpointData = (pThis->m308_variantIndex == 2)
                ? (void*)((u8*)pThis->m1A8_attackDataBuffer + 0xA8)
                : pThis->m1A8_attackDataBuffer;

            BTL_X0_fireProjectile(pThis, hotpointData, projectileType,
                (s8)pThis->mF2_animIndex, (s8)pThis->mF4_projectileIndex);
            pThis->mF2_animIndex++;
            pThis->mF4_projectileIndex++;
        }
    }
}
// Angle lookup tables for projectile direction
static s32 BTL_X0_projectileAngleTable[6];
static s32 BTL_X0_projectileDirectionTable[8];
static bool BTL_X0_projectileTablesLoaded = false;

static void BTL_X0_loadProjectileTables()
{
    if (BTL_X0_projectileTablesLoaded) return;
    sSaturnPtr pAngles = g_BTL_X0->getSaturnPtr(0x060b743c);
    for (int i = 0; i < 6; i++)
        BTL_X0_projectileAngleTable[i] = readSaturnS32(pAngles + i * 4);
    sSaturnPtr pDirs = g_BTL_X0->getSaturnPtr(0x060b7454);
    for (int i = 0; i < 8; i++)
        BTL_X0_projectileDirectionTable[i] = readSaturnS32(pDirs + i * 4);
    BTL_X0_projectileTablesLoaded = true;
}

// 0605df00
void sBTL_X0_Projectile::Update(sBTL_X0_Projectile* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;

    if (pThis->mEC_state != 0)
    {
        pThis->m20_velocity.m0_X = (s32)pThis->m20_velocity.m0_X + (s32)pThis->m2C_acceleration.m0_X;
        pThis->m20_velocity.m4_Y = (s32)pThis->m20_velocity.m4_Y + (s32)pThis->m2C_acceleration.m4_Y;
        pThis->m20_velocity.m8_Z = (s32)pThis->m20_velocity.m8_Z + (s32)pThis->m2C_acceleration.m8_Z;
        pThis->m8_position.m0_X = (s32)pThis->m8_position.m0_X + (s32)pThis->m20_velocity.m0_X;
        pThis->m8_position.m4_Y = (s32)pThis->m8_position.m4_Y + (s32)pThis->m20_velocity.m4_Y;
        pThis->m8_position.m8_Z = (s32)pThis->m8_position.m8_Z + (s32)pThis->m20_velocity.m8_Z;
        pThis->m8_position.m0_X = (s32)pThis->m8_position.m0_X + (s32)pEngine->m1A0_battleAutoScrollDelta.m0_X;
        pThis->m8_position.m4_Y = (s32)pThis->m8_position.m4_Y + (s32)pEngine->m1A0_battleAutoScrollDelta.m4_Y;
        pThis->m8_position.m8_Z = (s32)pThis->m8_position.m8_Z + (s32)pEngine->m1A0_battleAutoScrollDelta.m8_Z;
    }

    if (pThis->mEC_state == 0)
    {
        pThis->mEC_state++;
    }
    else if (pThis->mEC_state == 1)
    {
        sVec3_FP screenPos;
        transformAndAddVecByCurrentMatrix(&pThis->m8_position, &screenPos);
        sVec3_FP hitResult;
        s32 hit = sGunShotTask_UpdateSub0(&screenPos,
            (sVec3_FP*)((u8*)pThis->m8C_pDragon + 0x40), &hitResult);

        if (hit < 1)
        {
            s16 timer = pThis->mE6_timer2 - 1;
            pThis->mE6_timer2 = timer;
            if (timer >= 0)
                return;
        }

        if (pThis->mF0_damageType == 0)
        {
            s_battleDragon* pDragon = gBattleManager->m10_battleOverlay->m18_dragon;
            pDragon->m88 |= 8;
            playSystemSoundEffect(0xD);
        }
        else
        {
            s_battleDragon* pDragon = gBattleManager->m10_battleOverlay->m18_dragon;
            applyDamageToDragon(pDragon->m8C, pThis->mF0_damageType, hitResult, 2, pThis->m20_velocity, 0x1000);
        }

        s_battleDragon* pDragon = gBattleManager->m10_battleOverlay->m18_dragon;
        if ((pDragon->m1C0_statusModifiers & 0x400) == 0)
        {
            pThis->m2C_acceleration = {};
            urchinFormation_createFadeTask((p_workArea)pThis, 0xC210, 0xA94E, 1, 0xA94E, 0xC210, 10);
            pThis->mEC_state++;
        }
        else
        {
            urchinFormation_createFadeTask((p_workArea)pThis, 0xC210, 0xE210, 1, 0xE210, 0xC210, 10);
            pThis->m20_velocity.m0_X = MTH_Mul(pThis->m20_velocity.m0_X, fixedPoint((s32)0xFFFF8000));
            pThis->m20_velocity.m4_Y = MTH_Mul(pThis->m20_velocity.m4_Y, fixedPoint((s32)0xFFFF8000));
            pThis->m20_velocity.m8_Z = MTH_Mul(pThis->m20_velocity.m8_Z, fixedPoint((s32)0xFFFF8000));
            pThis->m2C_acceleration.m0_X = MTH_Mul(pThis->m2C_acceleration.m0_X, fixedPoint((s32)0xFFFF8000));
            pThis->m2C_acceleration.m4_Y = MTH_Mul(pThis->m2C_acceleration.m4_Y, fixedPoint((s32)0xFFFF8000));
            pThis->m2C_acceleration.m8_Z = MTH_Mul(pThis->m2C_acceleration.m8_Z, fixedPoint((s32)0xFFFF8000));
            pThis->mEC_state = 3;
        }
    }
    else if (pThis->mEC_state == 2 || pThis->mEC_state == 3)
    {
        s16 timer = pThis->mE4_timer1 - 1;
        pThis->mE4_timer1 = timer;
        if (timer < 0)
            pThis->getTask()->markFinished();
    }
}

// 0605e148
void sBTL_X0_Projectile::Draw(sBTL_X0_Projectile* pThis)
{
    Unimplemented(); // trail rendering via FUN_060ab11e
}

// 0605dce4
static void BTL_X0_fireProjectile(sBTL_X0_EnemyModel* pThis, void* hotpointData, s16 param3, s8 animIdx, s8 projectileIdx)
{
    BTL_X0_loadProjectileTables();

    static const sBTL_X0_Projectile::TypedTaskDefinition def = {
        nullptr, &sBTL_X0_Projectile::Update, &sBTL_X0_Projectile::Draw, nullptr
    };
    sBTL_X0_Projectile* pProj = createSubTaskWithCopy<sBTL_X0_Projectile>((s_workAreaCopy*)pThis, &def);
    if (pProj == nullptr)
        return;

    pProj->m4_fileBundle = pThis->m0_fileBundle;
    sVec3_FP* pSrc = (sVec3_FP*)hotpointData;
    pProj->m8_position = *pSrc;

    s_battleDragon* pDragon = gBattleManager->m10_battleOverlay->m18_dragon;
    pProj->m90_pDragonPosition2 = &pDragon->m8_position;
    pProj->m8C_pDragon = pDragon;
    pProj->mE4_timer1 = 10;
    pProj->mE0_value = 0x1000;
    pProj->mF0_damageType = param3;
    pProj->mEC_state = 0;
    pProj->mE6_timer2 = 10;

    s32 baseYaw = 0x8000000;
    s32 pitchIdx = performModulo2(6, (s32)animIdx);
    s32 pitch = BTL_X0_projectileAngleTable[pitchIdx];
    s32 dirIdx = performModulo2(8, (s32)projectileIdx);
    s32 yaw = baseYaw + BTL_X0_projectileDirectionTable[dirIdx];

    u16 pitchAngle = (u16)((u32)pitch >> 16);
    u16 yawAngle = (u16)((u32)yaw >> 16);

    fixedPoint cosPitch = getCos((u32)pitchAngle & 0xFFF);
    fixedPoint sinPitch = getSin((u32)pitchAngle & 0xFFF);
    fixedPoint cosYaw = getCos((u32)yawAngle & 0xFFF);
    fixedPoint sinYaw = getSin((u32)yawAngle & 0xFFF);

    pProj->m20_velocity.m0_X = MTH_Mul(MTH_Mul(cosPitch, sinYaw), fixedPoint(0x8000));
    pProj->m20_velocity.m4_Y = MTH_Mul(fixedPoint(-(s32)sinPitch), fixedPoint(0x8000));
    pProj->m20_velocity.m8_Z = MTH_Mul(MTH_Mul(cosPitch, cosYaw), fixedPoint(0x8000));
    pProj->m2C_acceleration = {};
}

// 06063f76
static void BTL_X0_cmd_attackA(sBTL_X0_EnemyModel* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    u8 subState = pThis->m307_subState;

    if (subState == 0)
    {
        if ((pEngine->m188_flags.m2000) == 0)
            return;

        battleEngine_displayAttackName(1, 0x1E, 0);
        pThis->m307_subState++;
        pThis->mF0_frameCounter = 0;
        if (pThis->m308_variantIndex != 2)
        {
            BTL_X0_playHitReaction(pThis, 1, 5);
        }
        pThis->mF2_animIndex = (s16)performModulo2(6, randomNumber());
        pThis->mF4_projectileIndex = (s16)(randomNumber() & 7);
        return;
    }

    if (subState != 1)
        return;

    s16 counter = pThis->mF0_frameCounter;
    pThis->mF0_frameCounter = counter + 1;

    if (counter >= 0x35)
    {
        s8 quadrant = pEngine->m22C_dragonCurrentQuadrant;
        if (quadrant == 0)
            pThis->m305_attackQuadrantBit = 1;
        else if (quadrant == 1)
            pThis->m305_attackQuadrantBit = 2;
        else if (quadrant == 2)
            pThis->m305_attackQuadrantBit = 4;
        else if (quadrant == 3)
            pThis->m305_attackQuadrantBit = 8;

        battleEngine_FlagQuadrantBitForAttack((u16)pThis->m305_attackQuadrantBit);
        pEngine->m188_flags.m100_attackAnimationFinished = 1;
        pThis->m307_subState = 0;
        pThis->m30A_commandIndex = 0;
    }

    s8 dragonQuadrant = pEngine->m22C_dragonCurrentQuadrant;
    s16 frame = pThis->mF0_frameCounter;

    if (dragonQuadrant == 0 || dragonQuadrant == 2)
    {
        if (frame == 0x20 || frame == 0x23 || frame == 0x26 ||
            frame == 0x29 || frame == 0x2C || frame == 0x2F)
        {
            void* hotpointData;
            if (pThis->m308_variantIndex == 2)
                hotpointData = (void*)((u8*)pThis->m1A8_attackDataBuffer + 0xA8);
            else
                hotpointData = pThis->m1A8_attackDataBuffer;

            BTL_X0_fireProjectile(pThis, hotpointData, 0,
                (s8)pThis->mF2_animIndex, (s8)pThis->mF4_projectileIndex);
            pThis->mF2_animIndex++;
            pThis->mF4_projectileIndex++;
        }
    }
    else
    {
        if (frame == 0x20 || frame == 0x26 || frame == 0x2C)
        {
            void* hotpointData;
            if (pThis->m308_variantIndex == 2)
                hotpointData = (void*)((u8*)pThis->m1A8_attackDataBuffer + 0xA8);
            else
                hotpointData = pThis->m1A8_attackDataBuffer;

            BTL_X0_fireProjectile(pThis, hotpointData, 0,
                (s8)pThis->mF2_animIndex, (s8)pThis->mF4_projectileIndex);
            pThis->mF2_animIndex++;
            pThis->mF4_projectileIndex++;
        }
    }
}
// 0606326e
static s32 BTL_X0_cmd_genericAttack(sBTL_X0_EnemyModel* pThis)
{
    if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished)
    {
        pThis->m307_subState = 0;
        pThis->m30A_commandIndex = 0;
        return 1;
    }
    return 0;
}

// 06064900
static void BTL_X0_restoreCameraAfterAttack()
{
    s_battleGrid* pGrid = gBattleManager->m10_battleOverlay->m8_gridTask;
    battleEngine_restoreCameraAfterEnemyAttack();
    battleEngine_resetBattleCameraPreset();
    pGrid->mE4_currentCameraReferenceCenter = pGrid->m134_desiredCameraPosition;
    pGrid->mF0_currentCameraReferenceForward = pGrid->m140_desiredCameraTarget;
    pGrid->m108_deltaCameraPosition = {};
    pGrid->m114_deltaCameraTarget = {};
}

// 060647ae
static void BTL_X0_initCameraForAttack(sBTL_X0_EnemyModel* pThis)
{
    s_battleGrid* pGrid = gBattleManager->m10_battleOverlay->m8_gridTask;
    battleEngine_enableAttackCamera();
    sEnemyAttackCamera_updateSub0(1);
    pGrid->mE4_currentCameraReferenceCenter = pGrid->m134_desiredCameraPosition;
    pGrid->mF0_currentCameraReferenceForward = pGrid->m140_desiredCameraTarget;
    pGrid->m108_deltaCameraPosition = {};
    pGrid->m114_deltaCameraTarget = {};
    pThis->m290_cameraAngle = {};
    pThis->m2CC_cameraAngle2 = {};
    pThis->m2A8_cameraRotation = {};
    pThis->m2E4_cameraRotation2 = {};
    pThis->m2B4_cameraRotVelocity = {};
    pThis->m2F0_cameraRotVelocity2 = {};
}

// 060648a8
static void BTL_X0_stepCameraRotation(sBTL_X0_EnemyModel* pThis)
{
    pThis->m2A8_cameraRotation.m0_X = (s32)pThis->m2A8_cameraRotation.m0_X + (s32)pThis->m2B4_cameraRotVelocity.m0_X;
    pThis->m2A8_cameraRotation.m4_Y = (s32)pThis->m2A8_cameraRotation.m4_Y + (s32)pThis->m2B4_cameraRotVelocity.m4_Y;
    pThis->m2A8_cameraRotation.m8_Z = (s32)pThis->m2A8_cameraRotation.m8_Z + (s32)pThis->m2B4_cameraRotVelocity.m8_Z;
    pThis->m2E4_cameraRotation2.m0_X = (s32)pThis->m2E4_cameraRotation2.m0_X + (s32)pThis->m2F0_cameraRotVelocity2.m0_X;
    pThis->m2E4_cameraRotation2.m4_Y = (s32)pThis->m2E4_cameraRotation2.m4_Y + (s32)pThis->m2F0_cameraRotVelocity2.m4_Y;
    pThis->m2E4_cameraRotation2.m8_Z = (s32)pThis->m2E4_cameraRotation2.m8_Z + (s32)pThis->m2F0_cameraRotVelocity2.m8_Z;
}

// Helper: compute camera positions from enemy camera params into pEngine->m418/m424
static void BTL_X0_updateCameraPositions(sBTL_X0_EnemyModel* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    sVec3_FP angle1, angle2, rot1, rot2;
    angle1.m0_X = pThis->m290_cameraAngle.m0_X;
    angle1.m4_Y = pThis->m290_cameraAngle.m4_Y;
    angle1.m8_Z = pThis->m290_cameraAngle.m8_Z;
    rot1.m0_X = pThis->m2A8_cameraRotation.m0_X;
    rot1.m4_Y = pThis->m2A8_cameraRotation.m4_Y;
    rot1.m8_Z = pThis->m2A8_cameraRotation.m8_Z;
    angle2.m0_X = pThis->m2CC_cameraAngle2.m0_X;
    angle2.m4_Y = pThis->m2CC_cameraAngle2.m4_Y;
    angle2.m8_Z = pThis->m2CC_cameraAngle2.m8_Z;
    rot2.m0_X = pThis->m2E4_cameraRotation2.m0_X;
    rot2.m4_Y = pThis->m2E4_cameraRotation2.m4_Y;
    rot2.m8_Z = pThis->m2E4_cameraRotation2.m8_Z;
    battleEngineSub1_UpdateSub2(&pEngine->m418, pEngine->m104_dragonPosition, angle1, rot1);
    battleEngineSub1_UpdateSub2(&pEngine->m424,
        gBattleManager->m10_battleOverlay->m18_dragon->m8_position, angle2, rot2);
}

// 06055d58
static void BTL_X0_initBodyAnimation(sBTL_X0_EnemyModel* pThis, s8 animIdx)
{
    pThis->m30B_subCommand = animIdx;
    pThis->m310_attackActive = 0;
    s16 animOffset = readSaturnS16(g_BTL_X0->getSaturnPtr(0x060b6cd8) + (u8)pThis->m30B_subCommand * 2);
    sAnimationData* pAnim = pThis->m0_fileBundle->getAnimation(animOffset);
    initAnimation(&pThis->m98_models[0], pAnim);
}

// 06055d8a
static void BTL_X0_playBodyAnimation(sBTL_X0_EnemyModel* pThis, s8 animIdx, s32 interpLength)
{
    pThis->m30B_subCommand = animIdx;
    pThis->m310_attackActive = 1;
    s16 animOffset = readSaturnS16(g_BTL_X0->getSaturnPtr(0x060b6cd8) + (u8)pThis->m30B_subCommand * 2);
    sAnimationData* pAnim = pThis->m0_fileBundle->getAnimation(animOffset);
    playAnimationGeneric(&pThis->m98_models[0], pAnim, interpLength);
}

static void BTL_X0_resetEnemyCommandState(sBTL_X0_EnemyModel* pThis)
{
    pThis->m307_subState = 0;
    pThis->m30A_commandIndex = 0;
}

// 0605e184
static void BTL_X0_beamAttackEffect(sBTL_X0_EnemyModel* pThis, void* p1, void* p2, void* p3)
{
    Unimplemented();
}

// 0605f200 — quadrant to base rotation
static s32 BTL_X0_quadrantToRotation(s8 quadrant)
{
    if (quadrant == 1) return 0x4000000;
    if (quadrant == 2) return 0x8000000;
    if (quadrant == 3) return 0xC000000;
    return 0;
}

// 0605e8b0
void sBTL_X0_MultiBeamTask::Update(sBTL_X0_MultiBeamTask* pThis)
{
    Unimplemented(); // complex beam spawning state machine with camera, fade, and damage
}

// 0605f1d8
void sBTL_X0_MultiBeamTask::Delete(sBTL_X0_MultiBeamTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    if (pThis->m48_cameraActive != 0)
        battleEngine_restoreCameraAfterEnemyAttack();
    pEngine->m188_flags.m100_attackAnimationFinished = 1;
}

// 0605e690
static void BTL_X0_multiBeamAttackEffect(sBTL_X0_EnemyModel* pThis, void* p1, void* p2, void* p3,
    s_3dModel* pModel0, s_3dModel* pModel2, u8 quadrant)
{
    static const sBTL_X0_MultiBeamTask::TypedTaskDefinition def = {
        nullptr, &sBTL_X0_MultiBeamTask::Update, nullptr, &sBTL_X0_MultiBeamTask::Delete
    };
    sBTL_X0_MultiBeamTask* pTask = createSubTask<sBTL_X0_MultiBeamTask>((p_workArea)pThis, &def);
    if (pTask == nullptr)
        return;

    pTask->m0_hotpoint0 = p1;
    pTask->m4_hotpoint1 = p2;
    pTask->m8_hotpoint2 = p3;
    pTask->m5E_frameCounter = 0;
    pTask->m5B_phase = 0;
    pTask->m58_state0 = 0;
    pTask->m59_variantIndex = pThis->m308_variantIndex;
    pTask->m5C_fireRate = 10;
    pTask->m64_pModel0 = pModel0;
    pTask->m68_pModel2 = pModel2;
    pTask->m5A_quadrant = (s8)quadrant;
    pTask->m48_cameraActive = 0;

    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    s_battleDragon* pDragon = gBattleManager->m10_battleOverlay->m18_dragon;

    if (quadrant == (u8)pEngine->m22C_dragonCurrentQuadrant)
        pTask->m6C_cameraFlag = 0;
    else
        pTask->m6C_cameraFlag = 1;

    urchinFormation_createFadeTask((p_workArea)pTask, 0xC210, 0xF39C, 2, 0xE210, 0xC210, 0xC);

    if (pTask->m6C_cameraFlag == 1)
    {
        pTask->m24_pad0 = {};
        pTask->m30_pad1 = {};
        pTask->m3C_dragonPos = pDragon->m8_position;
        pTask->m48_cameraActive = 1;
        pTask->m4C_targetRotation = BTL_X0_quadrantToRotation(pEngine->m22C_dragonCurrentQuadrant) + 0x2000000;
        pTask->m54_currentRotation = 0;

        if ((randomNumber() & 1) == 0)
        {
            pTask->m50_rotVelocity = fixedPoint(0x0DA740);
            if ((s32)pTask->m54_currentRotation < pTask->m4C_targetRotation)
                pTask->m54_currentRotation += 0x10000000;
        }
        else
        {
            pTask->m50_rotVelocity = fixedPoint((s32)0xFF2588C0);
            if (pTask->m4C_targetRotation < (s32)pTask->m54_currentRotation)
                pTask->m54_currentRotation -= 0x10000000;
        }

        sVec3_FP angle = {};
        angle.m8_Z = fixedPoint(pTask->m54_currentRotation);
        sVec3_FP rot = {};
        battleEngineSub1_UpdateSub2(&pTask->m18_cameraPosition, pDragon->m8_position, angle, rot);
        battleEngine_enableAttackCamera();
        battleEngine_setCurrentCameraPositionPointer(&pTask->m18_cameraPosition);
        battleEngine_setDesiredCameraPositionPointer(&pTask->m3C_dragonPos);
        battleEngine_resetCameraInterpolation();
    }
}

// 0605f230
static void BTL_X0_laserAttackEffect(sBTL_X0_EnemyModel* pThis, void* attackData)
{
    Unimplemented();
}

// 0605667c
static void BTL_X0_updateGridInterpolation(sBTL_X0_EnemyModel* pThis, s32 param2, s32 param3)
{
    pThis->m318_animInterp = pThis->m318_animInterp + pThis->m31C_animInterpSpeed;
    if ((s32)pThis->m318_animInterp > 0x10000)
        pThis->m318_animInterp = 0x10000;

    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    sBattleOverlayTask_C* pTargetSys = gBattleManager->m10_battleOverlay->mC_targetSystem;

    sSaturnPtr speedTable = g_BTL_X0->getSaturnPtr(0x060b6ce4);
    sSaturnPtr altTable = g_BTL_X0->getSaturnPtr(0x060b6d14);

    for (s32 q = 3; q >= 0; q--)
    {
        s32 speedFrom = readSaturnS32(speedTable + (param2 * 0x10 + q * 4));
        s32 speedTo = readSaturnS32(speedTable + (param3 * 0x10 + q * 4));
        pEngine->m45C_perQuadrantDragonSpeed[q] = speedFrom + MTH_Mul(pThis->m318_animInterp, fixedPoint(speedTo - speedFrom));

        s32 altFrom = readSaturnS32(altTable + (param2 * 0x10 + q * 4));
        s32 altTo = readSaturnS32(altTable + (param3 * 0x10 + q * 4));
        pEngine->m364_perQuadrantDragonAltitude[q] = altFrom + (s32)MTH_Mul(pThis->m318_animInterp, fixedPoint(altTo - altFrom));

        if (q == (s32)(u8)pEngine->m22C_dragonCurrentQuadrant)
        {
            pEngine->m104_dragonPosition.m4_Y = pEngine->m364_perQuadrantDragonAltitude[q] + pTargetSys->m204_cameraMaxAltitude;
        }
    }
}

// 060649e4
static void BTL_X0_spawnDeathParticle()
{
    Unimplemented();
}

// 060641c2
static void BTL_X0_cmd_attack7(sBTL_X0_EnemyModel* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    u8 subState = pThis->m307_subState;

    if (subState == 0)
    {
        if ((pEngine->m188_flags.m2000) == 0)
            return;

        battleEngine_displayAttackName(0x17, 0x1E, 0);
        void* attackData = pThis->m1A8_attackDataBuffer;
        BTL_X0_beamAttackEffect(pThis, attackData,
            (void*)((u8*)attackData + 0xC), (void*)((u8*)attackData + 0x18));
        pThis->m307_subState++;
        pThis->m305_attackQuadrantBit = 0xF;
        battleEngine_FlagQuadrantBitForAttack((u16)pThis->m305_attackQuadrantBit);
        BTL_X0_playHitReaction(pThis, 2, 10);
        pThis->mF0_frameCounter = 0;
    }
    else if (subState == 1)
    {
        s16 counter = pThis->mF0_frameCounter;
        pThis->mF0_frameCounter = counter + 1;
        if ((s16)(counter + 1) == 0x48)
            playSystemSoundEffect(0x12);

        if (BTL_X0_cmd_genericAttack(pThis) != 0)
        {
            pThis->mF0_frameCounter = 0;
            pThis->m304_state = 0;
            pThis->m306_dangerQuadrant = 0;
        }
    }
}

// 0606426e
static void BTL_X0_cmd_attack8(sBTL_X0_EnemyModel* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    u8 subState = pThis->m307_subState;

    if (subState == 0)
    {
        if ((pEngine->m188_flags.m2000) == 0)
            return;

        BTL_X0_startCameraEffect(pThis, 1);
        pThis->m30D_flag1 = 1;
        initAnimation(&pThis->m98_models[1], pThis->m0_fileBundle->getAnimation(0x228));
        pThis->m307_subState++;
        pThis->mF0_frameCounter = 0;
    }
    else if (subState == 1)
    {
        s16 counter = pThis->mF0_frameCounter;
        pThis->mF0_frameCounter = counter + 1;
        if ((s16)(counter + 1) == 0x3F)
        {
            formationCreateIntroEffectImpl((sFormationTaskBase*)pThis,
                g_BTL_X0->getSaturnPtr(0x060b7ec0), 0);
        }

        if (BTL_X0_isAnimationFinished(pThis) != 0)
        {
            if (pEngine->m3D0 != nullptr)
            {
                pEngine->m3D0->getTask()->markFinished();
                pEngine->m3D0 = nullptr;
            }
            pThis->m30D_flag1 = 0;
            pEngine->m188_flags.m100_attackAnimationFinished = 1;
            pThis->m307_subState++;
            pThis->mF0_frameCounter = 0;
        }

        if (pEngine->m3D0 == nullptr)
            pThis->m30D_flag1 = 0;
    }
    else if (subState == 2)
    {
        if (BTL_X0_cmd_genericAttack(pThis) != 0)
        {
            pThis->m304_state = 0xB;
            pThis->m306_dangerQuadrant = 4;
        }
    }
}

// 060643ba
static void BTL_X0_cmd_attack9(sBTL_X0_EnemyModel* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    u8 subState = pThis->m307_subState;

    if (subState == 0)
    {
        if ((pEngine->m188_flags.m2000) == 0)
            return;

        battleEngine_displayAttackName(0x14, 0x1E, 0);
        u8 quadrant = (u8)pEngine->m22C_dragonCurrentQuadrant;
        if (pEngine->m22C_dragonCurrentQuadrant == 2)
            quadrant = 0;

        void* attackData = pThis->m1A8_attackDataBuffer;
        BTL_X0_multiBeamAttackEffect(pThis,
            (void*)((u8*)attackData + 0x24),
            (void*)((u8*)attackData + 0x30),
            (void*)((u8*)attackData + 0x3C),
            &pThis->m98_models[0], &pThis->m98_models[2], quadrant);
        pThis->m307_subState++;
        pThis->m305_attackQuadrantBit = 0xB;
        battleEngine_FlagQuadrantBitForAttack((u16)pThis->m305_attackQuadrantBit);
    }
    else if (subState == 1)
    {
        if (BTL_X0_cmd_genericAttack(pThis) != 0)
        {
            pThis->m304_state = 5;
            pThis->m306_dangerQuadrant = 0;
        }
    }
}

// 060565ca
static void BTL_X0_cmd_attackC(sBTL_X0_EnemyModel* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    u8 subState = pThis->m307_subState;

    if (subState == 0)
    {
        if ((pEngine->m188_flags.m2000) == 0)
            return;

        battleEngine_displayAttackName(0x29, 0x1E, 0);
        BTL_X0_laserAttackEffect(pThis, pThis->m1A8_attackDataBuffer);
        pThis->m307_subState++;
        pThis->m305_attackQuadrantBit = 4;
        battleEngine_FlagQuadrantBitForAttack((u16)pThis->m305_attackQuadrantBit);
        pThis->m14_flags |= 2;
        pThis->m30F_flag3 = 2;
    }
    else if (subState == 1)
    {
        if (BTL_X0_cmd_genericAttack(pThis) != 0)
        {
            pThis->m304_state = 0;
            pThis->m306_dangerQuadrant = 0xF;
        }
    }
}

// 06056a32
static void BTL_X0_cmd_attackE(sBTL_X0_EnemyModel* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    u8 subState = pThis->m307_subState;

    if (subState == 0)
    {
        if ((pEngine->m188_flags.m2000) == 0)
            return;

        battleEngine_displayAttackName(0x36, 0x1E, 0);
        pThis->m307_subState++;
        BTL_X0_initBodyAnimation(pThis, 3);

        s32 totalFrames = 0;
        if (pThis->m98_models[0].m30_pCurrentAnimation != nullptr)
            totalFrames = (s32)pThis->m98_models[0].m30_pCurrentAnimation->m4_numFrames;
        pThis->m31C_animInterpSpeed = FP_Div(0x10000, fixedPoint(totalFrames << 16));
        pThis->m318_animInterp = 0;
    }
    else if (subState == 1)
    {
        BTL_X0_updateGridInterpolation(pThis, 0, 1);
        if (BTL_X0_isAnimationFinished(pThis) != 0)
        {
            BTL_X0_playBodyAnimation(pThis, 1, 0x1E);
            pEngine->m188_flags.m100_attackAnimationFinished = 1;
            pThis->m307_subState++;
            pThis->m14_flags &= ~2;
            pThis->m30F_flag3 = 3;
        }
    }
    else if (subState == 2)
    {
        if (BTL_X0_cmd_genericAttack(pThis) != 0)
        {
            pThis->m304_state = 0;
            pThis->m306_dangerQuadrant = 0xF;
        }
    }
}

// 06056b40
static void BTL_X0_cmd_attackF(sBTL_X0_EnemyModel* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    u8 subState = pThis->m307_subState;

    if (subState == 0)
    {
        if ((pEngine->m188_flags.m2000) == 0)
            return;

        battleEngine_displayAttackName(0x37, 0x1E, 0);
        pThis->m307_subState++;
        BTL_X0_playBodyAnimation(pThis, 2, 0);

        s32 totalFrames = 0;
        if (pThis->m98_models[0].m30_pCurrentAnimation != nullptr)
            totalFrames = (s32)pThis->m98_models[0].m30_pCurrentAnimation->m4_numFrames;
        pThis->m31C_animInterpSpeed = FP_Div(0x10000, fixedPoint(totalFrames << 16));
        pThis->m318_animInterp = 0;
    }
    else if (subState == 1)
    {
        BTL_X0_updateGridInterpolation(pThis, 1, 0);
        if (BTL_X0_isAnimationFinished(pThis) != 0)
        {
            BTL_X0_initBodyAnimation(pThis, 0);
            pThis->m307_subState++;
            pEngine->m188_flags.m100_attackAnimationFinished = 1;
            pThis->m14_flags &= ~4;
            pThis->m30F_flag3 = 1;
        }
    }
    else if (subState == 2)
    {
        if (BTL_X0_cmd_genericAttack(pThis) != 0)
        {
            pThis->m304_state = 0;
            pThis->m306_dangerQuadrant = 4;
        }
    }
}

// 06056c4c
static void BTL_X0_cmd_attack10(sBTL_X0_EnemyModel* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    u8 subState = pThis->m307_subState;

    if (subState == 0)
    {
        if ((pEngine->m188_flags.m2000) == 0)
            return;

        battleEngine_displayAttackName(0x2B, 0x1E, 0);
        playPCM(pThis, 9);
        pThis->mF0_frameCounter = 0x6F;
        pThis->m307_subState++;
        BTL_X0_initCameraForAttack(pThis);
        pThis->m2A8_cameraRotation.m4_Y = fixedPoint((s32)0xFF555556);
        pThis->m2B4_cameraRotVelocity.m4_Y = fixedPoint(0x3C131);
        pThis->m290_cameraAngle.m8_Z = fixedPoint((s32)0xFFFE8000);
        BTL_X0_updateCameraPositions(pThis);
        battleEngine_setCurrentCameraPositionPointer(&pEngine->m418);
        battleEngine_setDesiredCameraPositionPointer(&pEngine->m424);
        battleEngine_resetCameraInterpolation();
    }
    else if (subState == 1)
    {
        BTL_X0_stepCameraRotation(pThis);
        BTL_X0_updateCameraPositions(pThis);
        s16 counter = pThis->mF0_frameCounter - 1;
        pThis->mF0_frameCounter = counter;
        if (counter < 0)
        {
            pThis->m307_subState++;
            BTL_X0_restoreCameraAfterAttack();
        }
    }
    else if (subState == 2)
    {
        urchinFormation_createFadeTask((p_workArea)pThis, 0xC210, 0xB94E, 2, 0xB94E, 0xC210, 0x3C);
        {
            s_battleDragon* pDragon = gBattleManager->m10_battleOverlay->m18_dragon;
            pDragon->m1C0_statusModifiers &= 0xFFE0B87F;
            pDragon->m1E6_attackBuffTimer = 0;
            pDragon->m1E8_defenseBuffTimer = 0;
            pDragon->m1EA_agilityBuffTimer = 0;
            pDragon->m1EC_shieldDuration = 0;
            pDragon->m1F4 = 0;
            pDragon->m1F8 = 0;
            pDragon->m1FA = 0;
            pDragon->m1FC = 0;
            pDragon->m1FE = 0;
            pDragon->m200 = 0;
            pEngine->m3B4.m0_max = createBattleCommandMenuSub2(0x3C) << 16;
        }
        pEngine->m188_flags.m100_attackAnimationFinished = 1;
        pThis->m307_subState++;
    }
    else if (subState == 3)
    {
        BTL_X0_cmd_genericAttack(pThis);
    }
}

// 06063630
static void BTL_X0_cmd_death(sBTL_X0_EnemyModel* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    u8 subState = pThis->m307_subState;

    if (subState == 0)
    {
        if ((pEngine->m188_flags.m2000) == 0)
            return;

        pThis->m307_subState++;
        BTL_X0_initCameraForAttack(pThis);
        pThis->m2A8_cameraRotation.m4_Y = fixedPoint((s32)0xFF555556);
        pThis->m2B4_cameraRotVelocity.m4_Y = fixedPoint(0x5B05B);

        s8 quadrant = pEngine->m22C_dragonCurrentQuadrant;
        if (quadrant == 0)
            pThis->m290_cameraAngle.m8_Z = fixedPoint(0xF000);
        else if (quadrant == 1)
            pThis->m290_cameraAngle.m0_X = fixedPoint(0xF000);
        else if (quadrant == 2)
            pThis->m290_cameraAngle.m8_Z = fixedPoint((s32)0xFFFF1000);
        else if (quadrant == 3)
            pThis->m290_cameraAngle.m0_X = fixedPoint((s32)0xFFFF1000);

        BTL_X0_updateCameraPositions(pThis);
        pEngine->m418.m0_X = (s32)pEngine->m418.m0_X + (s32)pEngine->m1A0_battleAutoScrollDelta.m0_X;
        pEngine->m418.m4_Y = (s32)pEngine->m418.m4_Y + (s32)pEngine->m1A0_battleAutoScrollDelta.m4_Y;
        pEngine->m418.m8_Z = (s32)pEngine->m418.m8_Z + (s32)pEngine->m1A0_battleAutoScrollDelta.m8_Z;
        pEngine->m424.m0_X = (s32)pEngine->m424.m0_X + (s32)pEngine->m1A0_battleAutoScrollDelta.m0_X;
        pEngine->m424.m4_Y = (s32)pEngine->m424.m4_Y + (s32)pEngine->m1A0_battleAutoScrollDelta.m4_Y;
        pEngine->m424.m8_Z = (s32)pEngine->m424.m8_Z + (s32)pEngine->m1A0_battleAutoScrollDelta.m8_Z;
        battleEngine_setCurrentCameraPositionPointer(&pEngine->m418);
        battleEngine_setDesiredCameraPositionPointer(&pEngine->m424);
        battleEngine_resetCameraInterpolation();
        BTL_X0_playHitReaction(pThis, 5, 10);
        pThis->mF0_frameCounter = 0x1E;
    }
    else if (subState == 1)
    {
        BTL_X0_stepCameraRotation(pThis);
        BTL_X0_updateCameraPositions(pThis);
        s16 counter = pThis->mF0_frameCounter - 1;
        pThis->mF0_frameCounter = counter;
        if (counter < 0)
        {
            playSystemSoundEffect(0x68);
            pThis->m40_fallAcceleration.m8_Z = fixedPoint((s32)0xFFFFFD56);
            pThis->m34_fallVelocity.m4_Y = fixedPoint(0x2AA);
            pThis->m307_subState++;
            pThis->mF0_frameCounter = 0;
        }
    }
    else if (subState == 2)
    {
        BTL_X0_stepCameraRotation(pThis);
        BTL_X0_updateCameraPositions(pThis);

        fixedPoint prevVelZ = pThis->m34_fallVelocity.m8_Z;
        pThis->m34_fallVelocity.m8_Z = prevVelZ + pThis->m40_fallAcceleration.m8_Z;
        pThis->m7C_position.m8_Z = (s32)pThis->m7C_position.m8_Z + (s32)prevVelZ + (s32)pThis->m40_fallAcceleration.m8_Z;
        pThis->m7C_position.m4_Y = (s32)pThis->m7C_position.m4_Y + (s32)pThis->m34_fallVelocity.m4_Y;

        if ((s32)pThis->m7C_position.m8_Z < -0xFFFFF)
        {
            urchinFormation_createFadeTask((p_workArea)pThis, 0xC210, 0xFFFF, 4, 0xFFFF, 0xC210, 0xB4);
            pThis->m40_fallAcceleration.m8_Z = 0;
            pThis->m34_fallVelocity.m0_X = pEngine->m1A0_battleAutoScrollDelta.m0_X;
            pThis->m34_fallVelocity.m4_Y = pEngine->m1A0_battleAutoScrollDelta.m4_Y;
            pThis->m34_fallVelocity.m8_Z = pEngine->m1A0_battleAutoScrollDelta.m8_Z;
            pThis->m307_subState++;
            pThis->mF0_frameCounter = 0x3C;
        }

        battleEngine_resetCameraInterpolation();
    }
    else if (subState == 3)
    {
        BTL_X0_stepCameraRotation(pThis);
        BTL_X0_updateCameraPositions(pThis);
        battleEngine_resetCameraInterpolation();
        s16 counter = pThis->mF0_frameCounter - 1;
        pThis->mF0_frameCounter = counter;
        if (counter < 0)
        {
            pThis->m307_subState++;
            pEngine->m188_flags.m100_attackAnimationFinished = 1;
            BTL_X0_restoreCameraAfterAttack();
        }
    }
    else if (subState == 4)
    {
        BTL_X0_cmd_genericAttack(pThis);
    }
}

// 06063bb6
static void BTL_X0_cmd_deathSub(sBTL_X0_EnemyModel* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    u8 subState = pThis->m307_subState;

    if (subState == 0)
    {
        if ((pEngine->m188_flags.m2000) == 0)
            return;

        BTL_X0_startCameraEffect(pThis, 3);
        pThis->m307_subState++;
        pThis->m30D_flag1 = 1;
        initAnimation(&pThis->m98_models[1], pThis->m0_fileBundle->getAnimation(0x224));
    }
    else if (subState == 1)
    {
        if (BTL_X0_cmd_genericAttack(pThis) != 0)
        {
            if (pEngine->m3D0 != nullptr)
            {
                pEngine->m3D0->getTask()->markFinished();
                pEngine->m3D0 = nullptr;
            }
            pThis->m30D_flag1 = 0;
        }
        if (pEngine->m3D0 == nullptr)
            pThis->m30D_flag1 = 0;
    }
}

// 06056fae
static void BTL_X0_cmd_attack18(sBTL_X0_EnemyModel* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    u8 subState = pThis->m307_subState;

    if (subState == 0)
    {
        if ((pEngine->m188_flags.m2000) == 0)
            return;

        Unimplemented(); // complex camera interpolator setup, body animation, grid camera
        pThis->m307_subState++;
        pThis->mF0_frameCounter = 0;
    }
    else if (subState == 1)
    {
        Unimplemented(); // interpolator stepping, camera update, fade/particles
    }
    else if (subState == 2)
    {
        battleGrid_setCameraFov(0x238E38E);
        pThis->mF0_frameCounter = 0;
        BTL_X0_restoreCameraAfterAttack();
        BTL_X0_resetEnemyCommandState(pThis);
    }
}

// 06057534
static void BTL_X0_cmd_attack19(sBTL_X0_EnemyModel* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    u8 subState = pThis->m307_subState;

    if (subState == 0)
    {
        if ((pEngine->m188_flags.m2000) == 0)
            return;

        pThis->m1AC_interpolator.mC_startValue = pThis->m7C_position;
        pThis->m1AC_interpolator.m24_targetValue.m0_X = 0;
        pThis->m1AC_interpolator.m24_targetValue.m4_Y = 0;
        pThis->m1AC_interpolator.m24_targetValue.m8_Z = 0;
        pThis->m1AC_interpolator.m38_interpolationLength = 0x3C;
        vec2FPInterpolator_Init(&pThis->m1AC_interpolator);
        pThis->m307_subState++;
        pThis->mF0_frameCounter = 0;
        pThis->mF2_animIndex = (s16)performModulo2(6, randomNumber());
        pThis->mF4_projectileIndex = (s16)(randomNumber() & 7);
    }
    else if (subState == 1)
    {
        s16 counter = pThis->mF0_frameCounter;
        pThis->mF0_frameCounter = counter + 1;

        if (counter == 10 || counter == 0xD || counter == 0x10 ||
            counter == 0x13 || counter == 0x16)
        {
            BTL_X0_fireProjectile(pThis, pThis->m1A8_attackDataBuffer, 0,
                (s8)pThis->mF2_animIndex, (s8)pThis->mF4_projectileIndex);
            pThis->mF2_animIndex++;
            pThis->mF4_projectileIndex++;
        }

        s32 done = vec2FPInterpolator_Step(&pThis->m1AC_interpolator);
        if (done != 0)
        {
            pEngine->m188_flags.m100_attackAnimationFinished = 1;
            BTL_X0_resetEnemyCommandState(pThis);
            pThis->mF0_frameCounter = 0;
            pThis->mF2_animIndex = 0;
            pThis->mF4_projectileIndex = 0;
            playSystemSoundEffect(0x72);
        }

        pThis->m7C_position.m0_X = pThis->m1AC_interpolator.m0_currentValue.m0_X;
        pThis->m7C_position.m4_Y = pThis->m1AC_interpolator.m0_currentValue.m4_Y;
        pThis->m7C_position.m8_Z = pThis->m1AC_interpolator.m0_currentValue.m8_Z;
    }
}

// 060576da
static void BTL_X0_cmd_attack1A(sBTL_X0_EnemyModel* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    u8 subState = pThis->m307_subState;

    if (subState == 0)
    {
        if ((pEngine->m188_flags.m2000) == 0)
            return;

        pEngine->m230 = 4;
        pThis->m307_subState++;

        // Set up position interpolator for rotation approach
        pThis->m1AC_interpolator.mC_startValue.m0_X = pThis->m28_rotation.m4_Y;
        pThis->m1AC_interpolator.mC_startValue.m4_Y = 0;
        pThis->m1AC_interpolator.mC_startValue.m8_Z = pEngine->m1A0_battleAutoScrollDelta.m8_Z;

        u8 quadrant = (u8)pEngine->m22C_dragonCurrentQuadrant;
        pThis->m1AC_interpolator.m24_targetValue.m0_X =
            fixedPoint(readSaturnS32(g_BTL_X0->getSaturnPtr(0x060b6dbc) + quadrant * 4));
        pThis->m1AC_interpolator.m24_targetValue.m4_Y = 0;
        pThis->m1AC_interpolator.m24_targetValue.m8_Z = 0;
        pThis->m1AC_interpolator.m38_interpolationLength = 0x1E;
        vec2FPInterpolator_Init(&pThis->m1AC_interpolator);

        BTL_X0_initCameraForAttack(pThis);
        pThis->m290_cameraAngle.m8_Z = fixedPoint(0x1E000);
        s32 targetRot = readSaturnS32(g_BTL_X0->getSaturnPtr(0x060b6dbc) + quadrant * 4);
        pThis->m2A8_cameraRotation.m4_Y = fixedPoint(targetRot + 0x8000000);
        pThis->m2B4_cameraRotVelocity.m4_Y = FP_Div(0x8000000, fixedPoint(0x780000));
        BTL_X0_updateCameraPositions(pThis);
        battleEngine_setCurrentCameraPositionPointer(&pEngine->m418);
        battleEngine_setDesiredCameraPositionPointer(&pEngine->m424);
        battleEngine_resetCameraInterpolation();
    }
    else if (subState == 1)
    {
        BTL_X0_stepCameraRotation(pThis);
        BTL_X0_updateCameraPositions(pThis);
        s32 done = vec2FPInterpolator_Step(&pThis->m1AC_interpolator);
        if (done != 0)
        {
            pThis->m307_subState++;
            pThis->mF0_frameCounter = 0x5A;
        }
        pThis->m28_rotation.m4_Y = pThis->m1AC_interpolator.m0_currentValue.m0_X;
        pEngine->m1A0_battleAutoScrollDelta.m8_Z = (s32)pThis->m1AC_interpolator.m0_currentValue.m8_Z;
        pEngine->m1AC_battleAutoScrollDeltaBackup.m8_Z = pEngine->m1A0_battleAutoScrollDelta.m8_Z;
    }
    else if (subState == 2)
    {
        BTL_X0_stepCameraRotation(pThis);
        BTL_X0_updateCameraPositions(pThis);
        s16 counter = pThis->mF0_frameCounter - 1;
        pThis->mF0_frameCounter = counter;
        if (counter < 0)
        {
            pThis->m307_subState++;
            pThis->mF0_frameCounter = 0;
            pThis->m40_fallAcceleration.m4_Y = fixedPoint((s32)0xFFFFFFD4);
            pThis->m34_fallVelocity.m4_Y = 0;
            pThis->m34_fallVelocity.m8_Z = fixedPoint((s32)0xFFFFF556);
            pThis->m58_fallRotVelocity.m0_X = fixedPoint(0xB60B6);
            pThis->m58_fallRotVelocity.m8_Z = fixedPoint(0x2D82D8);
            playSystemSoundEffect(0x6B);
        }
    }
    else if (subState == 3)
    {
        BTL_X0_stepCameraRotation(pThis);
        BTL_X0_updateCameraPositions(pThis);

        pThis->m34_fallVelocity.m0_X = (s32)pThis->m34_fallVelocity.m0_X + (s32)pThis->m40_fallAcceleration.m0_X;
        pThis->m34_fallVelocity.m4_Y = (s32)pThis->m34_fallVelocity.m4_Y + (s32)pThis->m40_fallAcceleration.m4_Y;
        pThis->m34_fallVelocity.m8_Z = (s32)pThis->m34_fallVelocity.m8_Z + (s32)pThis->m40_fallAcceleration.m8_Z;
        pThis->m7C_position.m0_X = (s32)pThis->m7C_position.m0_X + (s32)pThis->m34_fallVelocity.m0_X;
        pThis->m7C_position.m4_Y = (s32)pThis->m7C_position.m4_Y + (s32)pThis->m34_fallVelocity.m4_Y;
        pThis->m7C_position.m8_Z = (s32)pThis->m7C_position.m8_Z + (s32)pThis->m34_fallVelocity.m8_Z;
        pThis->m28_rotation.m0_X = (s32)pThis->m28_rotation.m0_X + (s32)pThis->m58_fallRotVelocity.m0_X;
        pThis->m28_rotation.m4_Y = (s32)pThis->m28_rotation.m4_Y + (s32)pThis->m58_fallRotVelocity.m4_Y;
        pThis->m28_rotation.m8_Z = (s32)pThis->m28_rotation.m8_Z + (s32)pThis->m58_fallRotVelocity.m8_Z;

        if ((s32)pThis->m28_rotation.m0_X > 0x4000000)
        {
            pThis->m28_rotation.m0_X = 0x4000000;
            pEngine->m230 = 0;
        }

        if ((s32)pThis->m7C_position.m4_Y > -0xBE000)
        {
            BTL_X0_checkTargetableCollisions(pThis);
        }

        if ((s32)pThis->m7C_position.m4_Y < -0xC7FFF)
        {
            urchinFormation_createFadeTask((p_workArea)pThis, 0xC210, 0xFBDE, 4, 0xFBDE, 0xC210, 0x3C);
            pEngine->m188_flags.m100_attackAnimationFinished = 1;
            BTL_X0_restoreCameraAfterAttack();
            BTL_X0_resetEnemyCommandState(pThis);
            pThis->mF0_frameCounter = 0;
            pThis->mF2_animIndex = 0;
            pThis->mF4_projectileIndex = 0;
            playSystemSoundEffect(0x72);
        }
    }
}

// 060630cc
static void BTL_X0_updateIdlePosition(sBTL_X0_EnemyModel* pThis)
{
    switch (pThis->m30A_commandIndex)
    {
    case 0:
        BTL_X0_cmd_idle(pThis);
        break;
    case 1:
    case 3:
    case 4:
        break;
    case 2:
        BTL_X0_cmd_attack2(pThis);
        break;
    case 5:
        BTL_X0_cmd_attack5(pThis);
        break;
    case 6:
    case 0x14:
        pThis->m304_state = 0xF;
        pThis->m307_subState = 0;
        pThis->m30A_commandIndex = 0;
        break;
    case 7:
        BTL_X0_cmd_attack7(pThis);
        break;
    case 8:
        BTL_X0_cmd_attack8(pThis);
        break;
    case 9:
        BTL_X0_cmd_attack9(pThis);
        break;
    case 10:
        BTL_X0_cmd_attackA(pThis);
        break;
    case 0xB:
        pThis->m304_state = 4;
        pThis->m306_dangerQuadrant = 0;
        pThis->m307_subState = 0;
        pThis->m30A_commandIndex = 0;
        break;
    case 0xC:
        BTL_X0_cmd_attackC(pThis);
        break;
    case 0xD:
        pThis->m304_state = 0;
        pThis->m306_dangerQuadrant = 4;
        pThis->m307_subState = 0;
        pThis->m30A_commandIndex = 0;
        break;
    case 0xE:
        BTL_X0_cmd_attackE(pThis);
        break;
    case 0xF:
        BTL_X0_cmd_attackF(pThis);
        break;
    case 0x10:
        BTL_X0_cmd_attack10(pThis);
        break;
    case 0x11:
    case 0x12:
    case 0x13:
    case 0x15:
        BTL_X0_cmd_genericAttack(pThis);
        break;
    case 0x16:
        BTL_X0_cmd_death(pThis);
        break;
    case 0x17:
        BTL_X0_cmd_deathSub(pThis);
        break;
    case 0x18:
        BTL_X0_cmd_attack18(pThis);
        break;
    case 0x19:
        BTL_X0_cmd_attack19(pThis);
        break;
    case 0x1A:
        BTL_X0_cmd_attack1A(pThis);
        break;
    }
}

// 060632ac
static s32 BTL_X0_isAnimationFinished(sBTL_X0_EnemyModel* pThis)
{
    s_3dModel* pModel = &pThis->m98_models[0];
    s32 totalFrames = 0;
    if (pModel->m30_pCurrentAnimation != nullptr)
    {
        totalFrames = (s32)pModel->m30_pCurrentAnimation->m4_numFrames;
    }
    if ((s32)pModel->m16_previousAnimationFrame >= totalFrames - 1)
        return 1;
    return 0;
}

// 060634d4
static void BTL_X0_initDeathAnimation(sBTL_X0_EnemyModel* pThis, s8 param)
{
    pThis->m30B_subCommand = param;
    pThis->m310_attackActive = 0;
    s16 animOffset = readSaturnS16(g_BTL_X0->getSaturnPtr(0x060b7e7c) + (u8)pThis->m30B_subCommand * 2);
    sAnimationData* pAnim = pThis->m0_fileBundle->getAnimation(animOffset);
    initAnimation(&pThis->m98_models[0], pAnim);
    pThis->m311_animPending = 1;
}

// 0606350c
static void BTL_X0_playHitReaction(sBTL_X0_EnemyModel* pThis, s8 param1, s32 interpLength)
{
    pThis->m30B_subCommand = param1;
    pThis->m310_attackActive = 1;
    s16 animOffset = readSaturnS16(g_BTL_X0->getSaturnPtr(0x060b7e7c) + (u8)pThis->m30B_subCommand * 2);
    sAnimationData* pAnim = pThis->m0_fileBundle->getAnimation(animOffset);
    playAnimationGeneric(&pThis->m98_models[0], pAnim, interpLength);
    pThis->m311_animPending = 1;
}

// 06062f8e
static void BTL_X0_initEnemyTargetables(sBTL_X0_EnemyModel* pThis)
{
    pThis->m28_rotation.m4_Y = 0x8000000;

    s_3dModel& model0 = pThis->m98_models[0];
    if (model0.m40 == nullptr)
        return;

    std::vector<s_hotpointDefinition>& defs = *model0.m40;

    pThis->mF8_targetableCount = 0;
    for (u32 i = 0; i < model0.m12_numBones && i < (u32)defs.size(); i++)
    {
        pThis->mF8_targetableCount += (s16)defs[i].m4_count;
    }

    pThis->m1A0_targetableArray = allocateHeapForTask(pThis, pThis->mF8_targetableCount * sizeof(sBattleTargetable));
    pThis->m1A4_targetablePositionData = allocateHeapForTask(pThis, pThis->mF8_targetableCount * sizeof(sVec3_FP));

    s32 targetIdx = 0;
    for (u32 bone = 0; bone < model0.m12_numBones && bone < (u32)defs.size(); bone++)
    {
        if (bone < model0.m44_hotpointData.size() && !model0.m44_hotpointData[bone].empty())
        {
            for (u32 hp = 0; hp < defs[bone].m4_count && hp < (u32)defs[bone].m0.size(); hp++)
            {
                s_hotpoinEntry& entry = defs[bone].m0[hp];
                sBattleTargetable* pTargetable = (sBattleTargetable*)((u8*)pThis->m1A0_targetableArray + targetIdx * sizeof(sBattleTargetable));
                sVec3_FP* pPosition = (sVec3_FP*)((u8*)pThis->m1A4_targetablePositionData + targetIdx * sizeof(sVec3_FP));
                initTargetable(pTargetable, (s_workAreaCopy*)pThis, pPosition, entry.m10, (u32)entry.m0, 0, 0, 10);
                targetIdx++;
            }
        }
    }

    BTL_X0_copyTargetablePositions(pThis);

    pThis->m7C_position.m0_X = 0;
    pThis->m7C_position.m4_Y = 0;
    pThis->m7C_position.m8_Z = 0;
}

// 060632ce
static void BTL_X0_checkCollisionAndDamage(sBTL_X0_EnemyModel* pThis)
{
    if (pThis->m30B_subCommand == 0)
    {
        if ((s32)pThis->m98_models[0].m16_previousAnimationFrame == 0x23)
        {
            playSystemSoundEffect(0x68);
        }
    }
}

// 06062a2e
static void BTL_X0_updateTargetableQuadrants(sBTL_X0_EnemyModel* pThis, s32 param)
{
    if ((pThis->m14_flags & 0x10000000) == 0)
    {
        s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
        u8 quadrantValue = BTL_X0_lightColorTable_cache(
            (u8)pEngine->m22C_dragonCurrentQuadrant + param * 4);
        for (s32 i = 0; i < pThis->mF8_targetableCount; i++)
        {
            sBattleTargetable* pTargetable = (sBattleTargetable*)((u8*)pThis->m1A0_targetableArray + i * sizeof(sBattleTargetable));
            pTargetable->m60 = quadrantValue;
        }
    }
}

// 06063566 — transforms 7 selected hotpoints of model[0] by cameraProperties2.m28[0]
// and writes them into the 0x54-byte buffer at m1A8_attackDataBuffer.
static void BTL_X0_transformTargetablePositions(sBTL_X0_EnemyModel* pThis)
{
    if (pThis->m1A8_attackDataBuffer == nullptr || pThis->m98_models.empty())
        return;

    s_3dModel& model0 = pThis->m98_models[0];
    sVec3_FP* dst = (sVec3_FP*)pThis->m1A8_attackDataBuffer;
    const sMatrix4x3& cam = cameraProperties2.m28[0];

    auto get = [&](u32 bone, u32 hp) -> const sVec3_FP* {
        if (bone < model0.m44_hotpointData.size() && hp < model0.m44_hotpointData[bone].size())
            return &model0.m44_hotpointData[bone][hp];
        return nullptr;
    };

    // Ghidra: src offset encodes bone (byte/4) and hotpoint (byte/12 within bone's vec3 array).
    struct Entry { u32 bone; u32 hp; };
    static const Entry entries[7] = {
        {10, 5}, // (m44 + 0x28) + 0x3c → m44[10], hotpoint 5
        {26, 0}, // *(sVec3_FP**)(m44 + 0x68) → m44[26], hotpoint 0
        {33, 0}, // *(sVec3_FP**)(m44 + 0x84) → m44[33], hotpoint 0
        {10, 1}, // (m44 + 0x28) + 0x0c → m44[10], hotpoint 1
        {10, 2}, // + 0x18 → hotpoint 2
        {10, 3}, // + 0x24 → hotpoint 3
        {10, 4}, // + 0x30 → hotpoint 4
    };
    for (int i = 0; i < 7; i++)
    {
        const sVec3_FP* src = get(entries[i].bone, entries[i].hp);
        if (src)
            transformAndAddVec(*src, dst[i], cam);
    }
}

// 06062966 — copies raw hotpoint positions of model[0] into the m1A4 targetable-position buffer,
// in order (skipping bones with no hotpoint pointer). Iterates model[0].m12_numBones.
static void BTL_X0_copyTargetablePositions(sBTL_X0_EnemyModel* pThis)
{
    if (pThis->m1A4_targetablePositionData == nullptr || pThis->m98_models.empty())
        return;

    s_3dModel& model0 = pThis->m98_models[0];
    if (model0.m40 == nullptr)
        return;

    sVec3_FP* dst = (sVec3_FP*)pThis->m1A4_targetablePositionData;
    s32 dstIdx = 0;
    std::vector<s_hotpointDefinition>& defs = *model0.m40;

    for (u32 bone = 0; bone < model0.m12_numBones && bone < defs.size() && bone < model0.m44_hotpointData.size(); bone++)
    {
        if (model0.m44_hotpointData[bone].empty())
            continue;
        s32 count = (s32)defs[bone].m4_count;
        for (s32 h = 0; h < count && h < (s32)model0.m44_hotpointData[bone].size(); h++)
        {
            dst[dstIdx++] = model0.m44_hotpointData[bone][h];
        }
    }
}

// 06064a86
static void BTL_X0_checkTargetableCollisions(sBTL_X0_EnemyModel* pThis)
{
    Unimplemented();
}

// 06064714
static void BTL_X0_enemyDebugDraw(sBTL_X0_EnemyModel* pThis)
{
    Unimplemented();
}

void setupConditionalLightColor(int param_1);
void clearLightColor();

// Light-color lookup table at BTL_X0::060b7e98 — indexed by quadrant + flag3*4
static u8 BTL_X0_lightColorTable_cache(int idx)
{
    static u8 cached[32] = {0};
    static bool loaded = false;
    if (!loaded)
    {
        sSaturnPtr base = g_BTL_X0->getSaturnPtr(0x060b7e98);
        for (int i = 0; i < 32; i++)
            cached[i] = (u8)readSaturnU8(base + i);
        loaded = true;
    }
    return cached[idx];
}

// 06062a8e
static void BTL_X0_processDamage(sBTL_X0_EnemyModel* pThis, s16 param)
{
    if ((pThis->m14_flags & 0x10000000) != 0)
        return;

    if (readKeyboardToggle(0x81) != 0)
    {
        pThis->mEE_hpCurrent = 0;
        pThis->m14_flags |= 0x80000000;
        pThis->m302_damageValue = pThis->mEC_hpMax;
        createDamageDisplayTask((s_workAreaCopy*)pThis, pThis->m302_damageValue, &pThis->m1C_lifeMeterPosition, 0);
    }
    else if (readKeyboardToggle(0x89) != 0)
    {
        pThis->mEE_hpCurrent = 1000;
        playSystemSoundEffect(0x6a);
        ((sEnemyLifeMeterTask*)pThis->m10_lifeMeterTask)->m31 |= 2;
    }

    if ((pThis->m14_flags & 0x80000000) == 0)
    {
        for (s32 i = 0; i < pThis->mF8_targetableCount; i++)
        {
            sBattleTargetable* pTargetable = (sBattleTargetable*)((u8*)pThis->m1A0_targetableArray + i * sizeof(sBattleTargetable));
            if ((pTargetable->m50_flags & 0x80000) != 0)
            {
                pTargetable->m50_flags &= ~0x80000;
                pTargetable->m50_flags &= ~0x20000;
                return;
            }
        }
    }

    if ((pThis->m14_flags & 0x80000000) == 0)
    {
        s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
        if (pEngine->m188_flags.m1000 &&
            (pThis->m14_flags & 0x40000000) != 0 &&
            pThis->m302_damageValue >= 0)
        {
            createDamageDisplayTask((s_workAreaCopy*)pThis, pThis->m302_damageValue, &pThis->m1C_lifeMeterPosition, 0);
            pThis->m302_damageValue = 0;
            pThis->m14_flags &= ~0x40000000;
        }
    }
    else
    {
        pThis->m14_flags |= 0x10000000;
        ((sEnemyLifeMeterTask*)pThis->m10_lifeMeterTask)->m31 |= 1;
        ((sEnemyLifeMeterTask*)pThis->m10_lifeMeterTask)->m31 |= 8;
        for (s32 i = 0; i < pThis->mF8_targetableCount; i++)
        {
            sBattleTargetable* pTargetable = (sBattleTargetable*)((u8*)pThis->m1A0_targetableArray + i * sizeof(sBattleTargetable));
            pTargetable->m50_flags |= 0x100000;
        }
    }
}

// 0606259c
static void BTL_X0_enemyModel_Update(sBTL_X0_EnemyModel* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    pThis->m1C_lifeMeterPosition.m0_X = pEngine->mC_battleCenter.m0_X + pThis->m7C_position.m0_X;
    pThis->m1C_lifeMeterPosition.m4_Y = pEngine->mC_battleCenter.m4_Y + pThis->m7C_position.m4_Y;
    pThis->m1C_lifeMeterPosition.m8_Z = pEngine->mC_battleCenter.m8_Z + pThis->m7C_position.m8_Z;

    if (pThis->mFC_idleState == 0)
    {
        pThis->m1AC_interpolator.mC_startValue = pThis->m7C_position;
        pThis->m1AC_interpolator.m24_targetValue.m0_X = (randomNumber() % 0x14000) - 0xA000;
        pThis->m1AC_interpolator.m24_targetValue.m4_Y = (randomNumber() % 0x14000) - 0xA000;
        pThis->m1AC_interpolator.m24_targetValue.m8_Z = (randomNumber() % 0x14000) - 0xA000;
        pThis->m1AC_interpolator.m38_interpolationLength = 0x5A;
        vec2FPInterpolator_Init(&pThis->m1AC_interpolator);
        pThis->mFC_idleState++;
    }
    else if (pThis->mFC_idleState == 1)
    {
        BTL_X0_updateIdlePosition(pThis);
        if (pThis->m311_animPending == 0)
        {
            if (BTL_X0_isAnimationFinished(pThis))
            {
                if (pThis->m30A_commandIndex == 0x16 || pThis->m30A_commandIndex == 0x18)
                {
                    BTL_X0_initDeathAnimation(pThis, 3);
                }
                else if (pThis->m30B_subCommand == 3)
                {
                    BTL_X0_playHitReaction(pThis, 0, 5);
                }
                else
                {
                    s8 animIdx = (randomNumber() & 1) ? 3 : 0;
                    BTL_X0_playHitReaction(pThis, animIdx, 5);
                }
            }
        }
        else
        {
            pThis->m311_animPending = 0;
        }
    }

    if ((pThis->m14_flags & 1) == 0)
    {
        BTL_X0_updateBodyAnimation(pThis);
        if (pThis->m98_models.size() > 1)
            stepAnimation(&pThis->m98_models[1]);
        if (pThis->m98_models.size() > 2)
            stepAnimation(&pThis->m98_models[2]);
    }

    BTL_X0_checkCollisionAndDamage(pThis);
    s32 flag3 = (s32)pThis->m30F_flag3;
    BTL_X0_updateTargetableQuadrants(pThis, flag3);
    BTL_X0_processDamage(pThis, (s16)flag3);

    battleEngine_FlagQuadrantBitForSafety(0);
    battleEngine_FlagQuadrantBitForSafety((u16)(u8)pThis->m304_state);
    battleEngine_FlagQuadrantBitForDanger(0);
    battleEngine_FlagQuadrantBitForDanger((u16)(u8)pThis->m306_dangerQuadrant);
}

// 0606279a
static void BTL_X0_enemyModel_Draw(sBTL_X0_EnemyModel* pThis)
{
    // Debug keyboard toggle: swap between model[0] and model[1]
    if (readKeyboardToggle(0xC5) != 0)
    {
        pThis->m30D_flag1 = (pThis->m30D_flag1 == 0) ? 1 : 0;
    }

    // Conditional light color — table indexed by dragonCurrentQuadrant + m30F_flag3 * 4
    if ((pThis->m14_flags & 0x800000) != 0)
    {
        s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
        int idx = (u8)pEngine->m22C_dragonCurrentQuadrant + (s8)pThis->m30F_flag3 * 4;
        setupConditionalLightColor((u32)BTL_X0_lightColorTable_cache(idx));
    }

    pushCurrentMatrix();
    translateCurrentMatrix(&pThis->m1C_lifeMeterPosition);
    scaleCurrentMatrixRow0(pThis->m26C_scale.m0_X);
    scaleCurrentMatrixRow1(pThis->m26C_scale.m4_Y);
    scaleCurrentMatrixRow2(pThis->m26C_scale.m8_Z);
    rotateCurrentMatrixYXZ(pThis->m28_rotation);

    {
        // flag1 selects model[0] or model[1] for the primary draw
        s_3dModel* pPrimary = (pThis->m30D_flag1 == 0) ? &pThis->m98_models[0] : &pThis->m98_models[1];
        if (pPrimary->m18_drawFunction)
            pPrimary->m18_drawFunction(pPrimary);
    }
    popMatrix();

    // Second pass: draw Azel (m98_models[2]) at world-space position computed from
    // Atolm's head attach point (model[0].m44_hotpointData[10][0]) — Ghidra m44 + 0x28 = bone 10.
    if (pThis->m30D_flag1 == 0 && pThis->m98_models.size() > 2 && pThis->m98_models[2].m18_drawFunction)
    {
        sVec3_FP local;
        auto& hp = pThis->m98_models[0].m44_hotpointData;
        if (hp.size() > 10 && !hp[10].empty())
            transformAndAddVec(hp[10][0], local, cameraProperties2.m28[0]);
        else
            local = pThis->m1C_lifeMeterPosition;

        pushCurrentMatrix();
        translateCurrentMatrix(&local);
        scaleCurrentMatrixRow0(pThis->m26C_scale.m0_X);
        scaleCurrentMatrixRow1(pThis->m26C_scale.m4_Y);
        scaleCurrentMatrixRow2(pThis->m26C_scale.m8_Z);
        rotateCurrentMatrixYXZ(pThis->m28_rotation);
        pThis->m98_models[2].m18_drawFunction(&pThis->m98_models[2]);
        popMatrix();
    }

    // Clear conditional light color (also clears the flag bit)
    if ((pThis->m14_flags & 0x800000) != 0)
    {
        pThis->m14_flags &= ~0x800000;
        clearLightColor();
    }

    BTL_X0_transformTargetablePositions(pThis);
    BTL_X0_copyTargetablePositions(pThis);

    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x19] != 0)
    {
        BTL_X0_enemyDebugDraw(pThis);
    }
}

// 06062930
static void BTL_X0_enemyModel_Delete(sBTL_X0_EnemyModel* pThis)
{
    for (u32 i = 0; i < pThis->m98_models.size(); i++)
    {
        s_3dModel* pModel = &pThis->m98_models[i];
        if (pModel->m54_targetables != nullptr)
        {
            for (s32 j = 0; j < pModel->m52_targetableCount; j++)
            {
                deleteTargetable(&pModel->m54_targetables[j]);
            }
        }
    }
}

// 060593b8
static sBTL_X0_AttackSubPart* BTL_X0_createAttackSubPart(sBTL_X0_AttackSubTask* pParent, u8 partIndex)
{
    sBTL_X0_AttackSubPart* pPart = createSubTask<sBTL_X0_AttackSubPart>((p_workArea)pParent);
    if (pPart == nullptr)
        return nullptr;

    pPart->m0_fileBundle = pParent->m0_parentEnemy->m0_fileBundle;
    pPart->m4_vd1Allocation = pParent->m0_parentEnemy->m4_vd1Allocation;

    pPart->mFC_state = 0;
    pPart->m154_parentAttackTask = pParent;
    pPart->m300_partIndex = partIndex;
    pPart->m208_value = 0;
    pPart->m304_state = 0;
    pPart->m301_flag = 0;
    pPart->m29C_value = 0x28F;
    pPart->m2A0_value = 0x28F;
    pPart->m14_flags |= 0x80000000;
    pPart->m14_flags |= 0x10000000;

    return pPart;
}

// 06059444
void sBTL_X0_AttackSubPart::Update(sBTL_X0_AttackSubPart* pThis)
{
    Unimplemented();
}

// 06059b12
void sBTL_X0_AttackSubPart::Draw(sBTL_X0_AttackSubPart* pThis)
{
    Unimplemented();
}

// 06059b8e
void sBTL_X0_AttackSubPart::Delete(sBTL_X0_AttackSubPart* pThis)
{
}

static const sVec3_FP BTL_X0_attackPositions[6] = {
    { fixedPoint(0x00000000), fixedPoint(0x0001E000), fixedPoint(0x00000000) },
    { fixedPoint(0x0001E000), fixedPoint(0x00000000), fixedPoint(0x00000000) },
    { fixedPoint(0x00000000), fixedPoint(0x0001E000), fixedPoint((s32)0xFFFE2000) },
    { fixedPoint(0x00000000), fixedPoint(0x00000000), fixedPoint((s32)0xFFFE2000) },
    { fixedPoint(0x00000000), fixedPoint((s32)0xFFFE2000), fixedPoint(0x00000000) },
    { fixedPoint((s32)0xFFFE2000), fixedPoint(0x00000000), fixedPoint(0x00000000) },
};

// 06057e04
void sBTL_X0_AttackSubTask::Update(sBTL_X0_AttackSubTask* pThis)
{
    sBTL_X0_EnemyModel* pEnemy = pThis->m0_parentEnemy;
    s8 currentCmd = pEnemy->m30A_commandIndex;

    if (currentCmd != pThis->m6B_lastCommand)
    {
        if (currentCmd != 0x13 && pThis->mB8_pass == 0)
        {
            // no-op: skip command dispatch unless it's 0x13 or pass > 0
        }
        else if (currentCmd == 0x09)
        {
            for (int i = 0; i < 4; i++)
                pThis->m5E_partCmd[i] = 6;
        }
        else if (currentCmd == 0x11)
        {
            u32 rng = randomNumber();
            pThis->m6A_targetPart = ((rng & 1) == 0) ? 3 : 0;
            if (pThis->m70_partAlive[pThis->m6A_targetPart] == 0)
                pThis->m6A_targetPart = (pThis->m6A_targetPart == 3) ? 0 : 3;

            for (int i = 0; i < 4; i++)
            {
                if (pThis->m70_partAlive[i] == 1)
                    pThis->m5E_partCmd[i] = (pThis->m6A_targetPart == i) ? 4 : 3;
                else
                    pThis->m5E_partCmd[i] = 8;
            }
        }
        else if (currentCmd == 0x12)
        {
            u32 rng = randomNumber();
            pThis->m6A_targetPart = ((rng & 1) == 0) ? 1 : 2;
            if (pThis->m70_partAlive[pThis->m6A_targetPart] == 0)
                pThis->m6A_targetPart = (pThis->m6A_targetPart == 1) ? 2 : 1;

            for (int i = 0; i < 4; i++)
            {
                if (pThis->m70_partAlive[i] == 1)
                    pThis->m5E_partCmd[i] = (pThis->m6A_targetPart == i) ? 4 : 3;
                else
                    pThis->m5E_partCmd[i] = 8;
            }
        }
        else if (currentCmd == 0x13)
        {
            if (pThis->mB8_pass == 0)
            {
                for (int i = 0; i < 4; i++)
                    pThis->m5E_partCmd[i] = 1;
                pThis->m6A_targetPart = (u8)MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(4)).getInteger();
                pThis->mB8_pass = 1;
            }
            else
            {
                s32 deadCount = 0;
                for (int i = 0; i < 4; i++)
                {
                    if (pThis->m70_partAlive[i] == 1)
                    {
                        pThis->m5E_partCmd[i] = 3;
                        pThis->m90_partComplete[i] = 0;
                    }
                    else
                    {
                        if (deadCount == 0 || (randomNumber() & 1) == 0)
                            pThis->m6A_targetPart = (u8)i;
                        pThis->m5E_partCmd[i] = 9;
                        deadCount++;
                    }
                }
            }
        }
        else if (currentCmd == 0x15)
        {
            pThis->m6A_targetPart = (u8)MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(4)).getInteger();

            fixedPoint randA = MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x10000000));
            fixedPoint randB = MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x10000000));
            pThis->m54_rotY = MTH_Mul(getSin(randB.getInteger()), randA);

            fixedPoint randC = MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x10000000));
            fixedPoint randD = MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x10000000));
            fixedPoint cosD_C = MTH_Mul(getCos(randD.getInteger()), randC);

            fixedPoint randE = MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x10000000));
            pThis->m58_rotZ = MTH_Mul(getSin(randE.getInteger()), cosD_C);

            fixedPoint randF = MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x10000000));
            pThis->m50_rotX = MTH_Mul(getCos(randF.getInteger()), cosD_C);

            for (int i = 0; i < 6; i++)
            {
                sMatrix4x3 mat;
                initMatrixToIdentity(&mat);
                rotateMatrixShiftedY(pThis->m54_rotY, &mat);
                rotateMatrixShiftedX(pThis->m50_rotX, &mat);
                rotateMatrixShiftedZ(pThis->m58_rotZ, &mat);
                transformAndAddVec(BTL_X0_attackPositions[i], pThis->m8_positions[i], mat);
            }

            // Fisher-Yates shuffle of partOrder
            for (int pair = 0; pair < 2; pair++)
            {
                s32 remaining0 = 4 - pair * 2;
                s32 idx0 = MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(remaining0)).getInteger();
                u8 tmp0 = pThis->m66_partOrder[idx0];
                pThis->m66_partOrder[idx0] = pThis->m66_partOrder[3 - pair * 2];
                pThis->m66_partOrder[3 - pair * 2] = tmp0;
                pThis->m5E_partCmd[pair * 2] = 7;

                s32 remaining1 = remaining0 - 1;
                s32 idx1 = MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(remaining1)).getInteger();
                u8 tmp1 = pThis->m66_partOrder[idx1];
                pThis->m66_partOrder[idx1] = pThis->m66_partOrder[3 - pair * 2 - 1];
                pThis->m66_partOrder[3 - pair * 2 - 1] = tmp1;
                pThis->m5E_partCmd[pair * 2 + 1] = 7;
            }

            pThis->mB0_value = MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(2)).getInteger();
        }
        else
        {
            // Default: reset m90 and set part cmds based on alive state
            for (int i = 0; i < 4; i++)
                pThis->m90_partComplete[i] = 1;

            for (int i = 0; i < 4; i++)
                pThis->m5E_partCmd[i] = (pThis->m70_partAlive[i] == 1) ? 2 : 8;
        }
    }

    pThis->m6B_lastCommand = (u8)pEnemy->m30A_commandIndex;

    // Variant 3: timer-based part visibility cycling
    if (pThis->m6C_variant == 3)
    {
        if (battleEngine_isPlayerTurnActive() == 0)
        {
            if (pThis->m5C_timer < 300)
                pThis->m5C_timer++;
            else
                pThis->m5C_timer = 0;
        }

        if (pThis->m5C_timer < 0x4B || pThis->m5C_timer > 0xE0)
        {
            pThis->mA0_partState[1] = 0;
            pThis->mA0_partState[2] = 0;
        }
        else
        {
            pThis->mA0_partState[1] = 1;
            pThis->mA0_partState[2] = 1;
        }

        if (pThis->m5C_timer < 0x96)
        {
            pThis->mA0_partState[3] = 1;
            pThis->mA0_partState[0] = 1;
        }
        else
        {
            pThis->mA0_partState[3] = 0;
            pThis->mA0_partState[0] = 0;
        }

        if (pThis->m5C_timer == 0x95 || pThis->m5C_timer == 0xE0)
            pThis->mB4_trigger = 1;
        else
            pThis->mB4_trigger = 0;
    }

    // Check if all parts completed for command 0x13
    s32 completedCount = 0;
    if (pEnemy->m30A_commandIndex == 0x13)
    {
        for (int i = 0; i < 4; i++)
        {
            if (pThis->m90_partComplete[i] == 0)
                completedCount++;
        }
    }
    if (completedCount == 4)
    {
        s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
        pEngine->m188_flags.m100_attackAnimationFinished = 1;
    }

    // Check for per-part error signals
    for (int i = 0; i < 4; i++)
    {
        if (pThis->m80_partError[i] != 0)
        {
            pThis->getTask()->markFinished();
        }
    }
}

// 060586d0
void sBTL_X0_AttackSubTask::Delete(sBTL_X0_AttackSubTask* pThis)
{
}

// 06057d30
static p_workArea BTL_X0_createAttackSubTask(sBTL_X0_EnemyModel* pThis, void* attackDataBuffer)
{
    sBTL_X0_AttackSubTask* pTask = createSubTask<sBTL_X0_AttackSubTask>((p_workArea)pThis);
    if (pTask == nullptr)
        return nullptr;

    pTask->m0_parentEnemy = pThis;
    pTask->m6C_variant = (u8)pThis->m308_variantIndex;
    pTask->m6B_lastCommand = 0;
    pTask->m4_attackDataBuffer = attackDataBuffer;
    pTask->mB8_pass = 0;
    pTask->m5C_timer = 0;

    for (int i = 0; i < 4; i++)
    {
        pTask->m66_partOrder[i] = (u8)i;
        pTask->m5E_partCmd[i] = 0;
        pTask->m70_partAlive[i] = 0;
        pTask->m80_partError[i] = 0;
        pTask->m90_partComplete[i] = 1;
        pTask->mA0_partState[i] = 0;
    }

    for (int i = 0; i < 4; i++)
    {
        sBTL_X0_AttackSubPart* pPart = BTL_X0_createAttackSubPart(pTask, (u8)i);
        if (pPart == nullptr)
        {
            pTask->getTask()->markFinished();
            return pTask;
        }
    }

    return pTask;
}

// 0606237c
p_workArea BTL_X0_createEnemyModels(s_workArea* pFormation, s8 variant)
{
    static const sBTL_X0_EnemyModel::TypedTaskDefinition def = {
        nullptr,
        &BTL_X0_enemyModel_Update,
        &BTL_X0_enemyModel_Draw,
        &BTL_X0_enemyModel_Delete,
    };
    sBTL_X0_EnemyModel* pThis = createSubTaskWithCopy<sBTL_X0_EnemyModel>((s_workAreaCopy*)pFormation, &def);
    if (pThis == nullptr)
        return nullptr;

    s_fileBundle* pBundle = dramAllocatorEnd[6].mC_fileBundle->m0_fileBundle;
    pThis->m0_fileBundle = pBundle;
    pThis->m8_parentFormation = pFormation;

    pThis->m98_models.resize(3);

    // Model 0: Atolm body — uses hotpoint bundle at 060b7d20 (per-bone targeting points, including bone 10 = head attach)
    sModelHierarchy* pHierarchy0 = pBundle->getModelHierarchy(4);
    u32 numBones0 = pHierarchy0->countNumberOfBones();
    {
        sHotpointBundle* pHotspots = nullptr;
        sSaturnPtr hotspotEA = g_BTL_X0->getSaturnPtr(0x060b7d20);
        if (!hotspotEA.isNull())
        {
            pThis->m98_models[0].m_hotpointBundles.reserve(numBones0);
            for (u32 b = 0; b < numBones0; b++)
                pThis->m98_models[0].m_hotpointBundles.emplace_back(hotspotEA + b * 8);
            pHotspots = pThis->m98_models[0].m_hotpointBundles.data();
        }
        init3DModelRawData(pThis, &pThis->m98_models[0], 0, pBundle, 4,
            pBundle->getAnimation(0x20C), pBundle->getStaticPose(0x1F4, numBones0), nullptr, pHotspots);
    }
    stepAnimation(&pThis->m98_models[0]);

    // Model 1: Atolm debug/alternate — swapped in for model[0] when m30D_flag1 is toggled (debug key 0xC5)
    sModelHierarchy* pHierarchy1 = pBundle->getModelHierarchy(0xC);
    u32 numBones1 = pHierarchy1->countNumberOfBones();
    init3DModelRawData(pThis, &pThis->m98_models[1], 0, pBundle, 0xC,
        pBundle->getAnimation(0x224), pBundle->getStaticPose(0x1FC, numBones1), nullptr, nullptr);
    stepAnimation(&pThis->m98_models[1]);

    // Model 2: Azel (rider) — drawn at world-space position derived from Atolm's head hotpoint
    sModelHierarchy* pHierarchy2 = pBundle->getModelHierarchy(8);
    u32 numBones2 = pHierarchy2->countNumberOfBones();
    init3DModelRawData(pThis, &pThis->m98_models[2], 0, pBundle, 8,
        pBundle->getAnimation(0x22C), pBundle->getStaticPose(0x1F8, numBones2), nullptr, nullptr);
    stepAnimation(&pThis->m98_models[2]);

    // Allocate 0x54-byte attack-data buffer and zero-init
    pThis->m1A8_attackDataBuffer = allocateHeapForTask(pThis, 0x54);
    if (pThis->m1A8_attackDataBuffer)
        memset(pThis->m1A8_attackDataBuffer, 0, 0x54);

    if (variant == 0)
    {
        pThis->mEC_hpMax = 3000;
        pThis->mEE_hpCurrent = 3000;
        pThis->m10_lifeMeterTask = createEnemyLifeMeterTask(&pThis->m1C_lifeMeterPosition, 0, &pThis->mEE_hpCurrent, 0x24);
        pThis->m308_variantIndex = 0;
        displayFormationName(0, 0, 0xB);
        pThis->m304_state = 5;
    }
    else
    {
        if (variant == 1)
        {
            pThis->mEC_hpMax = 6000;
            pThis->mEE_hpCurrent = 6000;
            pThis->m10_lifeMeterTask = createEnemyLifeMeterTask(&pThis->m1C_lifeMeterPosition, 0, &pThis->mEE_hpCurrent, 0x33);
            pThis->m308_variantIndex = 1;
            displayFormationName(0, 0, 0xB);
        }
        else if (variant == 3)
        {
            pThis->mEC_hpMax = 8000;
            pThis->mEE_hpCurrent = 8000;
            pThis->m10_lifeMeterTask = createEnemyLifeMeterTask(&pThis->m1C_lifeMeterPosition, 0, &pThis->mEE_hpCurrent, 0x60);
            pThis->m308_variantIndex = 3;
        }
        // Variants 1 and 3 create an attack sub-task; variant 0 and other values skip it
        if (variant == 1 || variant == 3)
        {
            pThis->m314_attackSubTask = BTL_X0_createAttackSubTask(pThis, pThis->m1A8_attackDataBuffer);
            pThis->m304_state = 0xF;
        }
    }

    pThis->m26C_scale.m0_X = 0x10000;
    pThis->m26C_scale.m4_Y = 0x10000;
    pThis->m26C_scale.m8_Z = 0x10000;
    pThis->m30C_flag0 = 0;
    pThis->m30D_flag1 = 0;
    pThis->m30E_flag2 = 1;
    pThis->m30F_flag3 = 0;
    pThis->m310_attackActive = 0;

    BTL_X0_initEnemyTargetables(pThis);

    return pThis;
}

// 06055f0c
static void BTL_X0_enemyModel2_updateTargetableQuadrants(sBTL_X0_EnemyModel* pThis, s32 param)
{
    if ((pThis->m14_flags & 0x10000000) != 0)
        return;

    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    s32 offset = param * 4;
    u8 quadrantValue = BTL_X0_lightColorTable_cache((u8)pEngine->m22C_dragonCurrentQuadrant + offset);
    for (s32 i = 0; i < pThis->mF8_targetableCount; i++)
    {
        sBattleTargetable* pTargetable = (sBattleTargetable*)((u8*)pThis->m1A0_targetableArray + i * sizeof(sBattleTargetable));
        pTargetable->m60 = quadrantValue;
    }

    sBattleTargetable* tBase = (sBattleTargetable*)pThis->m1A0_targetableArray;
    sSaturnPtr table0 = g_BTL_X0->getSaturnPtr(0x060b6d54);
    tBase[0].m60 = readSaturnU8(table0 + (u8)pEngine->m22C_dragonCurrentQuadrant + offset);

    sSaturnPtr table1 = g_BTL_X0->getSaturnPtr(0x060b6d64);
    tBase[19].m60 = readSaturnU8(table1 + (u8)pEngine->m22C_dragonCurrentQuadrant + offset);

    sSaturnPtr table2 = g_BTL_X0->getSaturnPtr(0x060b6d74);
    tBase[8].m60 = readSaturnU8(table2 + (u8)pEngine->m22C_dragonCurrentQuadrant + offset);
}

// 06055918
static void BTL_X0_enemyModel2_Update(sBTL_X0_EnemyModel* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    pThis->m1C_lifeMeterPosition.m0_X = pEngine->mC_battleCenter.m0_X + pThis->m7C_position.m0_X;
    pThis->m1C_lifeMeterPosition.m4_Y = pEngine->mC_battleCenter.m4_Y + pThis->m7C_position.m4_Y;
    pThis->m1C_lifeMeterPosition.m8_Z = pEngine->mC_battleCenter.m8_Z + pThis->m7C_position.m8_Z;

    if (pThis->mFC_idleState == 0)
    {
        BTL_X0_initEnemyTargetables(pThis);

        pThis->m1AC_interpolator.mC_startValue.m0_X = 0;
        pThis->m1AC_interpolator.mC_startValue.m4_Y = 0;
        pThis->m1AC_interpolator.mC_startValue.m8_Z = 0xF000;
        pThis->m7C_position.m0_X = 0;
        pThis->m7C_position.m4_Y = 0;
        pThis->m7C_position.m8_Z = 0xF000;

        pThis->m1AC_interpolator.m24_targetValue.m0_X = (randomNumber() % 0x14000) - 0xA000;
        pThis->m1AC_interpolator.m24_targetValue.m4_Y = (randomNumber() % 0x14000) - 0xA000;
        pThis->m1AC_interpolator.m24_targetValue.m8_Z = (randomNumber() % 0x14000) - 0x19000;
        pThis->m1AC_interpolator.m38_interpolationLength = 0x5A;
        vec2FPInterpolator_Init(&pThis->m1AC_interpolator);
        pThis->mFC_idleState++;
    }
    else if (pThis->mFC_idleState == 1)
    {
        BTL_X0_updateIdlePosition(pThis);
    }

    if ((pThis->m14_flags & 1) == 0)
        BTL_X0_updateBodyAnimation(pThis);

    BTL_X0_checkCollisionAndDamage(pThis);
    s32 flag3 = (s32)pThis->m30F_flag3;
    BTL_X0_enemyModel2_updateTargetableQuadrants(pThis, flag3);
    BTL_X0_processDamage(pThis, (s16)flag3);

    battleEngine_FlagQuadrantBitForSafety(0);
    battleEngine_FlagQuadrantBitForSafety((u16)(u8)pThis->m304_state);
    battleEngine_FlagQuadrantBitForDanger(0);
    battleEngine_FlagQuadrantBitForDanger((u16)(u8)pThis->m306_dangerQuadrant);
}

// 06055be8
static void BTL_X0_enemyModel2_Draw(sBTL_X0_EnemyModel* pThis)
{
    pushCurrentMatrix();
    translateCurrentMatrix(&pThis->m1C_lifeMeterPosition);
    scaleCurrentMatrixRow0(pThis->m26C_scale.m0_X);
    scaleCurrentMatrixRow1(pThis->m26C_scale.m4_Y);
    scaleCurrentMatrixRow2(pThis->m26C_scale.m8_Z);

    if (pThis->m98_models.size() > 0 && pThis->m98_models[0].m18_drawFunction)
    {
        pThis->m98_models[0].m18_drawFunction(&pThis->m98_models[0]);
    }
    popMatrix();

    if (pThis->m98_models.size() > 1 && pThis->m98_models[1].m18_drawFunction)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->m1C_lifeMeterPosition);
        scaleCurrentMatrixRow0(pThis->m26C_scale.m0_X);
        scaleCurrentMatrixRow1(pThis->m26C_scale.m4_Y);
        scaleCurrentMatrixRow2(pThis->m26C_scale.m8_Z);
        pThis->m98_models[1].m18_drawFunction(&pThis->m98_models[1]);
        popMatrix();
    }
}

// 06055750
p_workArea BTL_X0_createEnemyModels2(s_workArea* pFormation)
{
    static const sBTL_X0_EnemyModel::TypedTaskDefinition def = {
        nullptr,
        &BTL_X0_enemyModel2_Update,
        &BTL_X0_enemyModel2_Draw,
        &BTL_X0_enemyModel_Delete,
    };
    sBTL_X0_EnemyModel* pThis = createSubTaskWithCopy<sBTL_X0_EnemyModel>((s_workAreaCopy*)pFormation, &def);
    if (pThis == nullptr)
        return nullptr;

    s_fileBundle* pBundle = dramAllocatorEnd[8].mC_fileBundle->m0_fileBundle;
    pThis->m0_fileBundle = pBundle;
    pThis->m8_parentFormation = pFormation;
    pThis->m30B_subCommand = 0;    // byte at 0x30B
    pThis->m310_attackActive = 0;  // byte at 0x310

    pThis->m98_models.resize(2);

    // Variant 2 uses a different creature (2-model setup) — body with hotpoints + secondary sub-model
    sModelHierarchy* pHierarchy0 = pBundle->getModelHierarchy(4);
    u32 numBones0 = pHierarchy0->countNumberOfBones();
    {
        sSaturnPtr hotspotEA = g_BTL_X0->getSaturnPtr(0x060b6be0);
        pThis->m98_models[0].m_hotpointBundles.reserve(numBones0);
        for (u32 b = 0; b < numBones0; b++)
            pThis->m98_models[0].m_hotpointBundles.emplace_back(hotspotEA + b * 8);

        // Model 0 animation selected from short table at 060b6cd8 indexed by m308_variantIndex (0 at this point)
        u16 anim0Offset = readSaturnU16(g_BTL_X0->getSaturnPtr(0x060b6cd8) + (s8)pThis->m308_variantIndex * 2);
        init3DModelRawData(pThis, &pThis->m98_models[0], 0, pBundle, 4,
            pBundle->getAnimation(anim0Offset), pBundle->getStaticPose(0xD8, numBones0), nullptr,
            pThis->m98_models[0].m_hotpointBundles.data());
    }
    stepAnimation(&pThis->m98_models[0]);

    // Model 1: secondary sub-model for this variant (unused by the flag1 swap in variant-2 draw)
    sModelHierarchy* pHierarchy1 = pBundle->getModelHierarchy(8);
    u32 numBones1 = pHierarchy1->countNumberOfBones();
    init3DModelRawData(pThis, &pThis->m98_models[1], 0, pBundle, 8,
        pBundle->getAnimation(0xF4), pBundle->getStaticPose(0xDC, numBones1), nullptr, nullptr);
    stepAnimation(&pThis->m98_models[1]);

    BTL_X0_updateTargetableQuadrants(pThis, 1);

    pThis->mEC_hpMax = 0x1964;
    pThis->mEE_hpCurrent = 0x1964;
    pThis->m10_lifeMeterTask = createEnemyLifeMeterTask(&pThis->m1C_lifeMeterPosition, 0, &pThis->mEE_hpCurrent, 0x5F);
    pThis->m308_variantIndex = 2;

    pThis->m26C_scale.m0_X = 0x10000;
    pThis->m26C_scale.m4_Y = 0x10000;
    pThis->m26C_scale.m8_Z = 0x10000;

    displayFormationName(0x1F, 0, 0xB);

    // Allocate 0xB4-byte attack-data buffer and zero-init
    pThis->m1A8_attackDataBuffer = allocateHeapForTask(pThis, 0xB4);
    if (pThis->m1A8_attackDataBuffer)
        memset(pThis->m1A8_attackDataBuffer, 0, 0xB4);

    pThis->m304_state = 0;
    pThis->m306_dangerQuadrant = 4;

    pThis->m30F_flag3 = 1;

    playPCM(pThis, 0x6C);

    return pThis;
}
