#include "PDS.h"
#include "battleEngine.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleGrid.h"
#include "battleEngineSub0.h"
#include "battleHud.h"
#include "battleRadar.h"
#include "battleOverlay_C.h"
#include "battleDebug.h"
#include "battleDragon.h"
#include "battleIntro.h"
#include "battleEngineSub1.h"
#include "battleCommandMenu.h"
#include "battleDebug.h"
#include "battleTextDisplay.h"
#include "kernel/receiveItemTask.h"
#include "battleResultScreen.h"
#include "gunShotRootTask.h"
#include "homingLaser.h"
#include "kernel/debug/trace.h"
#include "kernel/cinematicBarsTask.h"
#include "items.h"
#include "audio/systemSounds.h"
#include "battleGenericData.h"
#include "kernel/fade.h"
#include "interpolators/FPInterpolator.h"
#include "interpolators/vec2FPInterpolator.h"

#include "battleEnd.h"
#include "kernel/vdp1Allocator.h"

#include "BTL_A3/BTL_A3.h" // TODO: cleanup
#include "BTL_A3/baldor.h" // TODO: cleanup

// TODO: cleanup
void BattleEnd_deleteSub0();
void battleEngine_updateQuadrantFromAngle(s_battleEngine* pThis, fixedPoint uParm2, s32 r6);
s32 createBattleCommandMenuSub2(s32 param1);

struct sBattleCinematicBars : public s_workAreaTemplate<sBattleCinematicBars>
{
    s_cinematicBarTask* m0_cinematicBarTask;
    s16 m7;
    s8 m22_status;
    // size 0x2C
};

void sBattleCinematicBars_update(sBattleCinematicBars* pThis)
{
    switch (pThis->m22_status)
    {
    case 0:
        pThis->m22_status++;
        cinematicBars_startClosing(pThis->m0_cinematicBarTask, 4);
        break;
    case 1:
        if (pThis->m0_cinematicBarTask->m0_status != s_cinematicBarTask::m1_open)
        {
            return;
        }
        pThis->m7 = 0;
        pThis->m22_status++;
        break;
    case 2:
        if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m80000_hideBattleHUD)
        {
            return;
        }
        pThis->m0_cinematicBarTask->cinematicBars_startOpening(4);
        // TODO: another dummy call here
        pThis->m22_status++;
        break;
    case 3:
        if (pThis->m0_cinematicBarTask->m0_status != s_cinematicBarTask::m0_closed)
        {
            return;
        }
        pThis->m7 = 0;
        pThis->m22_status++;
        break;
    case 4:
        pThis->getTask()->markFinished();
        break;
    default:
        assert(0);
    }
}

void sBattleCinematicBars_delete(sBattleCinematicBars* pThis)
{
    pThis->m0_cinematicBarTask->getTask()->markFinished();
}

void battleCreateCinematicBars(s_battleEngine* pThis)
{
    if (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m80000_hideBattleHUD)
    {
        sBattleCinematicBars* pNewTask = createSubTaskFromFunction<sBattleCinematicBars>(pThis, &sBattleCinematicBars_update);
        pNewTask->m0_cinematicBarTask = createCinematicBarTask(pNewTask);
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m80000_hideBattleHUD = 1;

        //TODO: there is a call to a dummy function here. Look in proto?

        pNewTask->m_DeleteMethod = &sBattleCinematicBars_delete;
    }
}

void battleEngine_FlagQuadrantBitForSafety(u16 uParm1)
{
    if (uParm1 == 0)
    {
        gBattleManager->m10_battleOverlay->m4_battleEngine->m1E0_quadrantBitStatus &= ~0xF;
    }
    else
    {
        gBattleManager->m10_battleOverlay->m4_battleEngine->m1E0_quadrantBitStatus |= uParm1 & 0xF;
    }
}

void battleEngine_FlagQuadrantForSafety(u8 quadrant)
{
    battleEngine_FlagQuadrantBitForSafety(1 << quadrant);
}

void battleEngine_FlagQuadrantBitForDanger(u16 uParm1)
{
    if (uParm1 == 0)
    {
        gBattleManager->m10_battleOverlay->m4_battleEngine->m1E0_quadrantBitStatus &= ~0xF0;
    }
    else
    {
        gBattleManager->m10_battleOverlay->m4_battleEngine->m1E0_quadrantBitStatus |= (uParm1 & 0xF) << 4;
    }
}

void battleEngine_FlagQuadrantForDanger(u8 quadrant)
{
    battleEngine_FlagQuadrantBitForDanger(1 << quadrant);
}

void battleEngine_FlagQuadrantBitForAttack(u16 uParm1)
{
    if (uParm1 == 0)
    {
        gBattleManager->m10_battleOverlay->m4_battleEngine->m1E0_quadrantBitStatus &= ~0xF00;
    }
    else
    {
        gBattleManager->m10_battleOverlay->m4_battleEngine->m1E0_quadrantBitStatus |= (uParm1 & 0xF) << 8;
    }
}

void battleEngine_FlagQuadrantForAttack(u8 quadrant)
{
    battleEngine_FlagQuadrantBitForAttack(1 << quadrant);
}

void initBattleEngineArray()
{
    s_battleEngine* pBattleEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;

    pBattleEngine->m498_numEnemies = 0;
    for (int i = 0; i < 0x80; i++)
    {
        pBattleEngine->m49C_enemies[i].m0_isActive = -1;
        pBattleEngine->m49C_enemies[i].m4_targetable = 0;
        pBattleEngine->m49C_enemies[i].m8_distanceToDragonSquare = -1;
    }
}

void battleEngine_convertBattleQuadrantToBattleDirection(s_battleEngine* pThis)
{
    switch (pThis->m22C_dragonCurrentQuadrant)
    {
    case 0:
        pThis->m440_battleDirectionAngle = 0;
        break;
    case 1:
        pThis->m440_battleDirectionAngle = 0x4000000;
        break;
    case 2:
        pThis->m440_battleDirectionAngle = 0x8000000;
        break;
    case 3:
        pThis->m440_battleDirectionAngle = -0x4000000;
        break;
    default:
        assert(0);
    }
}

void battleEngine_InitSub2Sub0(s_battleEngine* pThis)
{
    s32 var1 = pThis->m440_battleDirectionAngle.normalized();
    if ((var1 < -0x4000000) || (var1 > 0x3ffffff))
    {
        pThis->m46C_dragon2dSpeed[1] = pThis->m45C_perQuadrantDragonSpeed[2];
    }
    else
    {
        pThis->m46C_dragon2dSpeed[1] = pThis->m45C_perQuadrantDragonSpeed[0];
    }

    if (var1 < 1)
    {
        pThis->m46C_dragon2dSpeed[0] = pThis->m45C_perQuadrantDragonSpeed[3];
    }
    else
    {
        pThis->m46C_dragon2dSpeed[0] = pThis->m45C_perQuadrantDragonSpeed[1];
    }
}

void battleEngine_InitSub2(s_battleEngine* pThis)
{
    battleEngine_InitSub2Sub0(pThis);
    pThis->mC_battleCenter = pThis->m234 + pThis->m24C;
    pThis->m6C_dragonIntegrateStep[0] = MTH_Mul(pThis->m46C_dragon2dSpeed[0], getSin(pThis->m440_battleDirectionAngle.getInteger() & 0xFFF));
    pThis->m6C_dragonIntegrateStep[2] = MTH_Mul(pThis->m46C_dragon2dSpeed[1], getCos(pThis->m440_battleDirectionAngle.getInteger() & 0xFFF));

    pThis->m104_dragonPosition[0] = pThis->m234[0] + pThis->m6C_dragonIntegrateStep[0];
    pThis->m104_dragonPosition[2] = pThis->m234[2] + pThis->m6C_dragonIntegrateStep[2];

    sVec3_FP temp;
    computeLookAt(pThis->mC_battleCenter - pThis->m104_dragonPosition, temp);

    pThis->m43C = temp[0];
}

void battleEngine_InitSub3Sub0(s_battleEngine* pThis)
{
    if (pThis->m104_dragonPosition[1] < pThis->m354_dragonAltitudeMinMax[1])
    {
        pThis->m104_dragonPosition[1] = pThis->m354_dragonAltitudeMinMax[1];
    }
    else if (pThis->m104_dragonPosition[1] > pThis->m354_dragonAltitudeMinMax[0])
    {
        pThis->m104_dragonPosition[1] = pThis->m354_dragonAltitudeMinMax[0];
    }
}

void battleEngine_InitSub3(s_battleEngine* pThis)
{
    pThis->m104_dragonPosition[1] = pThis->m364_perQuadrantDragonAltitude[gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant] + pThis->m35C_cameraAltitudeMinMax[1];
    pThis->m270_enemyAltitude[1] = pThis->m374_perQuadrantEnemyAltitude[gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant];

    battleEngine_InitSub3Sub0(pThis);
}

struct sBattlePassiveAbilitiesTask : public s_workAreaTemplate<sBattlePassiveAbilitiesTask>
{
    s8 m0;
    s8 m1;
    s8 m2;
    s8 m3;
    // size 0x4
};

// BTL_A3::06064350
static void battlePassiveAbilities_recoverHP(sBattlePassiveAbilitiesTask* pThis)
{
    if (pThis->m0 == 0 && (pThis->m1 & 2))
    {
        pThis->m1 &= ~2;
        s16 recovered = FP_Div((s32)mainGameState.gameStats.mB8_maxHP << 16, 0x120000).getInteger();
        s16 needed = mainGameState.gameStats.mB8_maxHP - mainGameState.gameStats.m10_currentHP;
        if (recovered > needed)
            recovered = needed;
        mainGameState.gameStats.m10_currentHP += recovered;
    }
}

// BTL_A3::060643a4
static void battlePassiveAbilities_enableCounter(sBattlePassiveAbilitiesTask* pThis)
{
    if (!(pThis->m1 & 8))
    {
        pThis->m1 |= 8;
        if (!(gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 0x20000))
            pThis->m2 = 1;
        gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers |= 0x20000;
    }
}

// BTL_A3::06064404
static void battlePassiveAbilities_triggerCounter(sBattlePassiveAbilitiesTask* pThis)
{
    if ((gBattleManager->m10_battleOverlay->m18_dragon->m1C4 & 0x10) &&
        gBattleManager->m10_battleOverlay->m18_dragon->m1D4_damageTaken > -1 &&
        mainGameState.gameStats.m10_currentHP > 0)
    {
        pThis->m3 = 1;
    }
    if (pThis->m3 == 1 && battleEngine_isBattleIntroFinished())
    {
        if (mainGameState.gameStats.m10_currentHP > 0 &&
            gBattleManager->m10_battleOverlay->m4_battleEngine->m498_numEnemies > 0 &&
            gBattleManager->m10_battleOverlay->mC_targetSystem->m20A_numSelectableEnemies > 0 &&
            (randomNumber() & 1) == 0)
        {
            Unimplemented(); // BTL_A3::06069d74(0x10)
            battleEngine_SetBattleMode(m3_shootEnemeyWithHomingLaser);
            gBattleManager->m10_battleOverlay->m4_battleEngine->m184 = 0;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m390 = -1;
            pThis->m3 = 0;
            return;
        }
        pThis->m3 = 0;
    }
}

// BTL_A3::060644e8
static void battlePassiveAbilities_resetStatus()
{
    s_battleDragon* pDragon = gBattleManager->m10_battleOverlay->m18_dragon;
    pDragon->m1C0_statusModifiers &= ~0x7f;
    *(s16*)((char*)pDragon + 0x1D8) = 0;
    *(s16*)((char*)pDragon + 0x1DA) = 0;
    *(s16*)((char*)pDragon + 0x1DC) = 0;
    *(s16*)((char*)pDragon + 0x1DE) = 0;
    *(s16*)((char*)pDragon + 0x1E0) = 0;
    *(s16*)((char*)pDragon + 0x1E2) = 0;
    s16 val = createBattleCommandMenuSub2(0x3c);
    gBattleManager->m10_battleOverlay->m4_battleEngine->m3B4.m0_max = fixedPoint::fromInteger(val);
}

// BTL_A3::060645e4
static void battlePassiveAbilities_update(sBattlePassiveAbilitiesTask* pThis)
{
    if (mainGameState.gameStats.m1_dragonLevel == 0 || mainGameState.gameStats.m1_dragonLevel == 8)
        return;

    if (gBattleManager->m10_battleOverlay->m4_battleEngine->m3B4.m16_combo != 3)
    {
        pThis->m0 = 0;
        pThis->m1 &= ~(1 | 2 | 4 | 8 | 0x10);
        if (pThis->m2 == 1)
            gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers &= ~0x20000;
        pThis->m2 = 0;
        pThis->m3 = 0;
        return;
    }

    bool bVar2 = true;
    if (battleEngine_isPlayerTurnActive() != 0 && !(gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m400000))
        bVar2 = false;
    if (bVar2)
    {
        s8 prev = pThis->m0++;
        if (prev == 0x1e)
        {
            pThis->m0 = 0;
            pThis->m1 |= 1;
            pThis->m1 |= 2;
        }
    }

    if (mainGameState.gameStats.m1_dragonLevel == 0)
        return;

    s8 dragonLevel = mainGameState.gameStats.m1_dragonLevel;
    if (dragonLevel >= 1 && dragonLevel <= 5)
    {
        switch (gDragonState->m1C_dragonArchetype)
        {
        case 0: battlePassiveAbilities_recoverHP(pThis); break;
        case 1: battlePassiveAbilities_enableCounter(pThis); break;
        case 2: battlePassiveAbilities_triggerCounter(pThis); break;
        case 3: battlePassiveAbilities_resetStatus(); break;
        case 4:
            // BP regen (LAB_06064564 inline)
            if (pThis->m0 == 0 && (pThis->m1 & 1))
            {
                pThis->m1 &= ~1;
                s16 recovered = FP_Div((s32)mainGameState.gameStats.mBA_maxBP << 16, 0x240000).getInteger();
                s16 needed = mainGameState.gameStats.mBA_maxBP - mainGameState.gameStats.m14_currentBP;
                if (recovered > needed) recovered = needed;
                mainGameState.gameStats.m14_currentBP += recovered;
            }
            break;
        }
    }
    else if (dragonLevel == 6)
    {
        battlePassiveAbilities_recoverHP(pThis);
        battlePassiveAbilities_enableCounter(pThis);
        battlePassiveAbilities_triggerCounter(pThis);
        battlePassiveAbilities_resetStatus();
        // BP regen inline (LAB_06064564)
        if (pThis->m0 == 0 && (pThis->m1 & 1))
        {
            pThis->m1 &= ~1;
            s16 recovered = FP_Div((s32)mainGameState.gameStats.mBA_maxBP << 16, 0x240000).getInteger();
            s16 needed = mainGameState.gameStats.mBA_maxBP - mainGameState.gameStats.m14_currentBP;
            if (recovered > needed) recovered = needed;
            mainGameState.gameStats.m14_currentBP += recovered;
        }
    }
    else if (dragonLevel == 7)
    {
        switch (gDragonState->m1C_dragonArchetype)
        {
        case 0: battlePassiveAbilities_recoverHP(pThis); break;
        case 1: battlePassiveAbilities_enableCounter(pThis); break;
        case 2: battlePassiveAbilities_triggerCounter(pThis); break;
        case 3: battlePassiveAbilities_resetStatus(); break;
        case 4:
            if (pThis->m0 == 0 && (pThis->m1 & 1))
            {
                pThis->m1 &= ~1;
                s16 recovered = FP_Div((s32)mainGameState.gameStats.mBA_maxBP << 16, 0x240000).getInteger();
                s16 needed = mainGameState.gameStats.mBA_maxBP - mainGameState.gameStats.m14_currentBP;
                if (recovered > needed) recovered = needed;
                mainGameState.gameStats.m14_currentBP += recovered;
            }
            break;
        }
    }
}

void battleEngine_InitSub5(p_workArea parent)
{
    createSubTaskFromFunction<sBattlePassiveAbilitiesTask>(parent, battlePassiveAbilities_update);
}

void battleEngine_setCurrentCameraPositionPointer(sVec3_FP* pData)
{
    gBattleManager->m10_battleOverlay->m4_battleEngine->m3D4_pCurrentCameraPosition = pData;
    gBattleManager->m10_battleOverlay->m8_gridTask->m1B8_pCameraTranslationSource = pData;
}

void battleEngine_setDesiredCameraPositionPointer(sVec3_FP* pData)
{
    gBattleManager->m10_battleOverlay->m4_battleEngine->m3D8_pDesiredCameraPosition = pData;
}

void battleEngine_resetCameraInterpolationSub0(sVec2_FP& param)
{
    s_battleEngine* pBattleEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    s_battleGrid* pBattleGrid = gBattleManager->m10_battleOverlay->m8_gridTask;

    addTraceLog(*pBattleEngine->m3D8_pDesiredCameraPosition, "3D8");
    addTraceLog(*pBattleEngine->m3D4_pCurrentCameraPosition, "3D4");

    sVec3_FP delta = *pBattleEngine->m3D8_pDesiredCameraPosition - *pBattleEngine->m3D4_pCurrentCameraPosition;
    computeLookAt(delta, param);
    
    pBattleEngine->m1C8 = param[0];
    pBattleEngine->m1CC = param[1];

    pBattleGrid->m64_cameraRotationTarget[0] = param[0];
    pBattleGrid->m64_cameraRotationTarget[1] = param[1];
}

void battleEngine_resetCameraInterpolationSub1(sVec2_FP& param)
{
    addTraceLog(param, "param");

    s_battleGrid* pBattleGrid = gBattleManager->m10_battleOverlay->m8_gridTask;
    pBattleGrid->mB4_cameraRotation[0] = param[0];
    pBattleGrid->mB4_cameraRotation[1] = param[1];
}

void battleEngine_resetCameraInterpolation()
{
    s_battleGrid* pBattleGrid = gBattleManager->m10_battleOverlay->m8_gridTask;
    pBattleGrid->m180_cameraTranslation = *pBattleGrid->m1B8_pCameraTranslationSource;
    addTraceLog(pBattleGrid->m180_cameraTranslation, "cameraTranslation");

    sVec2_FP auStack16;
    battleEngine_resetCameraInterpolationSub0(auStack16);
    battleEngine_resetCameraInterpolationSub1(auStack16);
    pBattleGrid->m18C.zeroize();
    pBattleGrid->mC0_cameraRotationInterpolation.zeroize();
}

struct sBattleIntroScrollTask : public s_workAreaTemplate<sBattleIntroScrollTask>
{
    char m_data[0x144];
    // size: 0x144
};

// BTL_A3::060a28b8
static void battleIntroScroll_update(sBattleIntroScrollTask* pThis)
{
    Unimplemented();
}

// BTL_A3::060a2c72
static void battleIntroScroll_draw(sBattleIntroScrollTask* pThis)
{
    sVec3_FP local;
    if ((s8)((char*)gBattleManager->m10_battleOverlay->m1C_envTask)[0x1c] & 1)
        transformAndAddVecByCurrentMatrix(gBattleManager->m10_battleOverlay->m4_battleEngine->m3D8_pDesiredCameraPosition, &local);
}

static const sBattleIntroScrollTask::TypedTaskDefinition battleIntroScrollTaskDefinition = {
    nullptr,
    battleIntroScroll_update,
    battleIntroScroll_draw,
    nullptr,
};

void battleEngine_InitSub9(p_workArea parent)
{
    createSubTask<sBattleIntroScrollTask>(parent, &battleIntroScrollTaskDefinition);
}

void battleEngine_InitSub11()
{
    //TODO: recheck!
    int iVar1 = gDragonState->mC_dragonType;
    if (((((iVar1 == 1) || (iVar1 == 2)) || (iVar1 == 3)) || ((iVar1 == 4 || (iVar1 == 5)))) ||
        ((iVar1 == 6 || ((iVar1 == 7 || (iVar1 == 8)))))) {
        mainGameState.setBit(52 * 8 + 5);
        mainGameState.setBit(49 * 8 + 5);
        mainGameState.setBit(52 * 8 + 1);
        mainGameState.setBit(52 * 8 + 3);
        mainGameState.setBit(52 * 8 + 2);
    }
    else {
        mainGameState.clearBit(52 * 8 + 5);
        mainGameState.clearBit(49 * 8 + 5);
        mainGameState.clearBit(52 * 8 + 1);
        mainGameState.clearBit(52 * 8 + 3);
        mainGameState.clearBit(52 * 8 + 2);
    }
}

void executeFuncPtr(sSaturnPtr funcPtr, s_battleEngine* pThis)
{
    gCurrentBattleOverlay->invoke(funcPtr, pThis);
}

void battleEngine_RestoreBattleAutoScrollDelta(s_battleEngine* pThis)
{
    pThis->m1A0_battleAutoScrollDelta = pThis->m1AC_battleAutoScrollDeltaBackup;
}

void battleEngine_BackupBattleAutoScrollDelta(s_battleEngine* pThis)
{
    pThis->m1AC_battleAutoScrollDeltaBackup = pThis->m1A0_battleAutoScrollDelta;
}

void battleEngine_Init(s_battleEngine* pThis, sSaturnPtr overlayBattleData)
{
    gBattleManager->m10_battleOverlay->m4_battleEngine = pThis;
    pThis->m3A8_overlayBattledata = overlayBattleData;
    pThis->m3B0_subBattleId = (s8)gBattleManager->m6_subBattleId;

    sSaturnPtr battleData_4 = readSaturnEA(overlayBattleData + pThis->m3B0_subBattleId * 0x20 + 4);
    pThis->m3AC = readSaturnEA(overlayBattleData + pThis->m3B0_subBattleId * 0x20 );

    createInBattleDebugTask(pThis);
    createBattleTextDisplay(pThis, readSaturnEA(overlayBattleData + pThis->m3B0_subBattleId * 0x20 + 0xC));
    initBattleEngineArray();

    // Generate random quadrant (mask to 8 bits, not modulo)
    u32 randomValue = performModulo2(100, randomNumber()) & 0xff;

    sSaturnPtr pData = readSaturnEA(gCurrentBattleOverlay->getEncounterDataTable() + gBattleManager->m4 * 4);

    u8 odds0, odds1, odds2;
    if (pData.isNull())
    {
        odds0 = 100;
        odds1 = 0;
        odds2 = 0;
    }
    else
    {
        // Read quadrant probabilities
        odds0 = (u8)readSaturnS8(pData + gBattleManager->m8 * 0x10 + 4);
        odds1 = (u8)readSaturnS8(pData + gBattleManager->m8 * 0x10 + 5);
        odds2 = (u8)readSaturnS8(pData + gBattleManager->m8 * 0x10 + 6);
    }

    // Select quadrant based on probability accumulation (mask each value before adding)
    if (randomValue < odds0)
    {
        pThis->m22C_dragonCurrentQuadrant = 0;
    }
    else
    {
        u32 accumulated = (odds1 & 0xff) + odds0;
        if (randomValue < accumulated)
        {
            pThis->m22C_dragonCurrentQuadrant = 1;
        }
        else if (randomValue < accumulated + (odds2 & 0xff))
        {
            pThis->m22C_dragonCurrentQuadrant = 2;
        }
        else
        {
            pThis->m22C_dragonCurrentQuadrant = 3;
        }
    }

    pThis->m22D_dragonPreviousQuadrant = pThis->m22C_dragonCurrentQuadrant;

    // Read combo value for current quadrant (m3B4.m16_combo = Saturn offset 0x3CA)
    if (pData.isNull())
    {
        pThis->m3B4.m16_combo = 0;
    }
    else
    {
        // combo - read from data table using current quadrant as part of offset
        pThis->m3B4.m16_combo = readSaturnS8(pData + pThis->m22C_dragonCurrentQuadrant + gBattleManager->m8 * 0x10 + 0xC);
    }

    pThis->m270_enemyAltitude.zeroize();
    pThis->m258.zeroize();
    pThis->m1A0_battleAutoScrollDelta = readSaturnVec3(battleData_4 + 0x4C);
    pThis->m264 = readSaturnVec3(battleData_4 + 0x00);

    pThis->m264[1] = readSaturnFP(battleData_4 + 0x18);

    pThis->m234 = pThis->m264 + pThis->m270_enemyAltitude;

    for (int i = 0; i < 2; i++)
    {
        pThis->m354_dragonAltitudeMinMax[i] = readSaturnS32(battleData_4 + 0xC + i * 4);
    }
    for (int i = 0; i < 2; i++)
    {
        pThis->m35C_cameraAltitudeMinMax[i] = readSaturnS32(battleData_4 + 0x14 + i * 4);
    }

    pThis->m230 = readSaturnS8(battleData_4 + 0x58);
    for (int i = 0; i < 4; i++)
    {
        pThis->m45C_perQuadrantDragonSpeed[i] = readSaturnS32(battleData_4 + 0x1C + i * 4);
    }
    pThis->m24C.zeroize();
    for (int i = 0; i < 4; i++)
    {
        pThis->m374_perQuadrantEnemyAltitude[i] = readSaturnS32(battleData_4 + 0x2C + i * 4);
    }
    for (int i = 0; i < 4; i++)
    {
        pThis->m364_perQuadrantDragonAltitude[i] = readSaturnS32(battleData_4 + 0x3C + i * 4);
    }

    pThis->m1D0 = 0x111111;
    pThis->m1D4 = 0x111111;

    pThis->m390 = 0;
    pThis->m392 = 2;
    pThis->m394 = 0;

    pThis->m3A2_selectedBerserk = eItems::m0_dummy;

    pThis->m3B4.m0_max = 0x3C0000;
    pThis->m3B4.m4 = 0x10000;
    pThis->m3B4.m8 = 0;

    initMatrixToIdentity(&pThis->m1F0);

    pThis->m434 = 0xA000;

    createBattleEnvironmentGridTask(pThis);

    pThis->m264 += pThis->m1A0_battleAutoScrollDelta;
    pThis->m234 = pThis->m270_enemyAltitude + pThis->m264;
    pThis->m240 = pThis->m270_enemyAltitude + pThis->m258 + pThis->m264;

    battleEngine_convertBattleQuadrantToBattleDirection(pThis);
    battleEngine_BackupBattleAutoScrollDelta(pThis);
    battleEngine_InitSub2(pThis);
    battleEngine_InitSub3(pThis);
    battleEngine_createDragonTask(pThis);
    battleEngine_InitSub5(pThis);
    battleEngine_setCurrentCameraPositionPointer(&gBattleManager->m10_battleOverlay->m8_gridTask->m34_cameraPosition);
    battleEngine_setDesiredCameraPositionPointer(&pThis->mC_battleCenter);
    battleEngine_resetCameraInterpolation();
    battleEngine_InitSub9(pThis);

    executeFuncPtr(readSaturnEA(overlayBattleData + pThis->m3B0_subBattleId * 0x20 + 8), pThis);

    resetCamera(0, 0, 352, 224, 352/2, 224/2);
    initVDP1Projection(DEG_80 / 2, 0);
    setupVdp1LocalCoordinatesAndClipping();

    g_fadeControls.m_4D = 6;
    if ((s8)g_fadeControls.m_4C < 7)
    {
        vdp2Controls.m20_registers[1].m112_CLOFSL = 0;
        vdp2Controls.m20_registers[0].m112_CLOFSL = 0;
    }

    fadePalette(&g_fadeControls.m24_fade1, 0xC210, 0xC210, 1);
    fadePalette(&g_fadeControls.m0_fade0, 0xFFFF, 0xFFFF, 1);

    g_fadeControls.m_4D = 5;

    pThis->m474_XPReceivedFromBattle = 0;
    pThis->m478_dyneReceivedFromBattle = 0;
    pThis->m47C_exp = 0;
    pThis->m480.fill(-1);
    pThis->m190.fill(0);

    createBattleEngineSub1(pThis);
    battleEngine_InitSub11();

    if (mainGameState.gameStats.mC_laserPower < 1) {
        mainGameState.gameStats.mC_laserPower = 80;
        mainGameState.gameStats.mE_gunPower = 60;
    }

    pThis->m484.fill(-1);
}

void battleEngine_UpdateSub1Sub0(s32 param_1)
{
    if (param_1 > 100)
    {
        param_1 = 100;
    }
    else if (param_1 < 0)
    {
        param_1 = 0;
    }

    gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m0 = setDividend(gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m2 * 65536, param_1 * 65536, 0x640000);
}

void battleEngine_UpdateSub1(int bitMask)
{
    s_battleEngine* pBattleEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    int iVar7 = 0;
    while (readSaturnU32(pBattleEngine->m3AC + iVar7 * 0xC) != 0)
    {
        if (bitMask & readSaturnU32(pBattleEngine->m3AC + iVar7 * 0xC + 8))
        {
            sSaturnPtr functionPointer = readSaturnEA(pBattleEngine->m3AC + iVar7 * 0xC);
            gCurrentBattleOverlay->invoke(functionPointer, gBattleManager->m10_battleOverlay->m18_dragon, readSaturnU32(pBattleEngine->m3AC + iVar7 * 0xC + 4), iVar7);
            pBattleEngine->m3B1++;

            sSaturnPtr pData = readSaturnEA(gCurrentBattleOverlay->getEncounterDataTable() + gBattleManager->m4 * 4);
            if (readSaturnU32(pData) == 0)
            {
                gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m2 = 150;
                battleEngine_UpdateSub1Sub0(0);
            }
            else
            {
                gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m2 = readSaturnS16(pData + gBattleManager->m8 * 0x10 + 2);
                battleEngine_UpdateSub1Sub0(readSaturnS8(pData + gBattleManager->m8 * 0x10 + 8 + gBattleManager->m10_battleOverlay->m4_battleEngine->m27C_dragonMovementInterpolator1.m50));
            }
        }
        iVar7++;
    }
}

s32 isBattleAutoScroll()
{
    s8 bVar1 = gBattleManager->m10_battleOverlay->m4_battleEngine->m230;

    if ((((bVar1 != 0) && (bVar1 != 1)) && (bVar1 != 2)) &&
        (((bVar1 != 3 && (bVar1 != 8)) && (bVar1 != 10)))) {
        return 0;
    }
    return 1;
}


void battleEngine_snapDragonFacing()
{
    s_battleDragon* pDragon = gBattleManager->m10_battleOverlay->m18_dragon;

    if (isBattleAutoScroll() == 0)
    {
        pDragon->m88 |= 0x20;
        switch (gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant)
        {
        case 0:
            pDragon->m74_targetRotation[1] -= pDragon->m74_targetRotation[1].normalized();
            break;
        case 1:
            pDragon->m74_targetRotation[1] += 0x4000000 - (pDragon->m74_targetRotation[1] & 0xfffffff);
            break;
        case 2:
            pDragon->m74_targetRotation[1] += 0x8000000 - (pDragon->m74_targetRotation[1] & 0xfffffff);
            break;
        case 3:
            pDragon->m74_targetRotation[1] += -0x4000000 - pDragon->m74_targetRotation[1].normalized();
            break;
        default:
            assert(0);
        }
    }
    else
    {
        pDragon->m74_targetRotation[1] -= pDragon->m74_targetRotation[1].normalized();
    }

}

void battleEngine_SetBattleMode(eBattleModes param)
{
    s_battleEngine* pBattleEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    pBattleEngine->m38C_battleMode = param;
    pBattleEngine->m188_flags.m80 = 1;
    pBattleEngine->m188_flags.m100_attackAnimationFinished = 0;
    pBattleEngine->m188_flags.m2000 = 0;
    pBattleEngine->m188_flags.m10 = 1;
    pBattleEngine->m188_flags.m20_battleIntroRunning = 1;
    pBattleEngine->m188_flags.m200_suppressBattleInputs = 1;
    pBattleEngine->m38D_battleSubMode = 0;
    pBattleEngine->m384_battleModeDelay = 0;
    pBattleEngine->m386 = 0;
    pBattleEngine->m184 = 0;
    pBattleEngine->m3E8.zeroize();
    battleEngine_RestoreBattleAutoScrollDelta(pBattleEngine);
}

s32 battleEngine_SetBattleMode16()
{
    s_battleEngine* pBattleEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    pBattleEngine->m38C_battleMode = eBattleModes::m10_position;
    pBattleEngine->m188_flags.m800 = 0;
    pBattleEngine->m188_flags.m100_attackAnimationFinished = 0;
    pBattleEngine->m188_flags.m2000 = 0;
    pBattleEngine->m188_flags.m40 = 0;
    pBattleEngine->m188_flags.m1000000_dragonMoving = 0;
    pBattleEngine->m188_flags.m400000 = 0;
    pBattleEngine->m188_flags.m80000_hideBattleHUD = 0;
    pBattleEngine->m188_flags.m100000 = 0;

    battleEngine_FlagQuadrantBitForAttack(0);

    pBattleEngine->m390 = 0;
    pBattleEngine->m394 = 0;

    pBattleEngine->m384_battleModeDelay = 0;
    pBattleEngine->m386 = 0;
    pBattleEngine->m3E8.zeroize();
    battleEngine_RestoreBattleAutoScrollDelta(pBattleEngine);

    gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2_needToSortEnemiesByDistanceFromDragon = 1;

    return 0;
}

void battleEngine_ApplyBattleAutoScrollDelta(s_battleEngine* pThis)
{
    // Debug mode: keyboard-controlled auto-scroll (0605a054-0605a141)
    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x16])
    {
        // Requires readKeyboardTable1 implementation for full debug keyboard handling
        Unimplemented();
    }

    // Apply auto-scroll delta to position vectors
    pThis->m264 += pThis->m1A0_battleAutoScrollDelta;
    pThis->m234 = pThis->m270_enemyAltitude + pThis->m264;
    pThis->m240 = pThis->m270_enemyAltitude + pThis->m258 + pThis->m264;
}

bool battleEngine_isBattleIntroFinished() // 0x20 is indicating battle intro is running
{
    if (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m20_battleIntroRunning)
        return 1;
    return 0;
}

bool battleEngine_isInputAllowed()
{
    if (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m10)
        return 1;
    return 0;
}

// BTL_A3::0607c32c
void battleEngine_UpdateSub7Sub1Sub0(fixedPoint* param_1, sVec3_FP& out)
{
    sVec3_FP local;
    local[0] = -MTH_Mul_5_6(0x10000, getCos(param_1[0].getInteger() & 0xfff), getSin(param_1[1].getInteger() & 0xfff));
    local[1] = MTH_Mul(0x10000, getSin(param_1[0].getInteger() & 0xfff));
    local[2] = -MTH_Mul_5_6(0x10000, getCos(param_1[0].getInteger() & 0xfff), getCos(param_1[1].getInteger() & 0xfff));
    transformVecByCurrentMatrix(local, out);
}

void battleEngine_UpdateSub7Sub1()
{
    s_battleGrid* pGrid = gBattleManager->m10_battleOverlay->m8_gridTask;
    if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m20000)
    {
        sVec3_FP local_1c;
        battleEngine_UpdateSub7Sub1Sub0(&pGrid->m280_lightAngle1, local_1c);
        pGrid->m1CC_lightColor = pGrid->m1D8_newLightColor;
        setupLight(local_1c[0], local_1c[1], local_1c[2],
            s_RGB8::fromVector(pGrid->m1CC_lightColor).toU32());
        pGrid->m1E4_lightFalloff0 = pGrid->m1F0;
        setupLightColor(s_RGB8::fromVector(pGrid->m1E4_lightFalloff0).toU32());
        resetProjectVector();
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m20000 = 0;
    }
}

void battleEngine_UpdateSub7Sub2()
{
    gBattleManager->m10_battleOverlay->m8_gridTask->m1 = 0;

    gBattleManager->m10_battleOverlay->m8_gridTask->m134_desiredCameraPosition = readSaturnVec3(g_BTL_GenericData->getSaturnPtr(0x60ac478) + 0x24 * gBattleManager->m10_battleOverlay->m8_gridTask->m1);
    gBattleManager->m10_battleOverlay->m8_gridTask->m140_desiredCameraTarget = readSaturnVec3(g_BTL_GenericData->getSaturnPtr(0x60ac484) + 0x24 * gBattleManager->m10_battleOverlay->m8_gridTask->m1);
}

void battleEngine_restoreCameraDefaultSub0()
{
    gBattleManager->m10_battleOverlay->m8_gridTask->mB4_cameraRotation[2] = 0;
    gBattleManager->m10_battleOverlay->m8_gridTask->m64_cameraRotationTarget[2] = 0;
}

void battleEngine_restoreCameraDefault()
{
    s_battleEngine* pBattleEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    s_battleGrid* pGrid = gBattleManager->m10_battleOverlay->m8_gridTask;
    pBattleEngine->m188_flags.m800 = 0;

    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0xB] == 0)
    {
        if (isTraceEnabled())
        {
            addTraceLog(pGrid->m1C, "pGrid->m1C");
            addTraceLog(pGrid->m28, "pGrid->m28");
            addTraceLog(pBattleEngine->m104_dragonPosition, "pBattleEngine->m104_dragonStartPosition");
        }
        pGrid->m34_cameraPosition = pBattleEngine->m104_dragonPosition + pGrid->m1C + pGrid->m28;
        battleEngine_setCurrentCameraPositionPointer(&pGrid->m34_cameraPosition);
        pBattleEngine->m3D8_pDesiredCameraPosition = &pBattleEngine->mC_battleCenter;
        battleEngine_restoreCameraDefaultSub0();
    }
}

struct sDragonDeathTask : public s_workAreaTemplate<sDragonDeathTask>
{
    u8 m0_state = 0;
    s16 m2_x0;
    s16 m4_y0;
    s16 m6_x1;
    s16 m8_y1;
    s16 mA_initial_x0;
    s16 mC_initial_y0;
    s16 mE_initial_x1;
    s16 m10_initial_y1;
    s16 m12_target_x0;
    s16 m14_target_y0;
    s16 m16_target_x1;
    s16 m18_target_y1;
    s32 m1C_transitionDuration;
    s16 m24_currentTick;
    // size 0x28
};

void dragonDeathTask_init(sDragonDeathTask* pThis) {
    pThis->m24_currentTick = 0;

    pThis->m2_x0 = 0;
    pThis->m4_y0 = 0;
    pThis->m6_x1 = 352;
    pThis->m8_y1 = 224;

    pThis->mA_initial_x0 = pThis->m2_x0;
    pThis->mC_initial_y0 = pThis->m4_y0;
    pThis->mE_initial_x1 = pThis->m6_x1;
    pThis->m10_initial_y1 = pThis->m8_y1;

    pThis->m12_target_x0 = 0x60;
    pThis->m14_target_y0 = 0x80;
    pThis->m16_target_x1 = 0x100;
    pThis->m18_target_y1 = 0xd0;

    pThis->m1C_transitionDuration = 30;
    *(u16*)getVdp2Vram(0x2A600) = 0x8000;
    vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA = (vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA & 0xFFF80000) | 0x15300;
    g_fadeControls.m_4D = 6;
    if ((char)g_fadeControls.m_4C < 7) {
        vdp2Controls.m20_registers[1].m112_CLOFSL = 0;
        vdp2Controls.m20_registers[0].m112_CLOFSL = 0;
    }
    fadePalette(&g_fadeControls.m0_fade0, 0x819f, 0x8000, 0xa5);
    g_fadeControls.m_4D = 5;
}

void dragonDeathTask_update(sDragonDeathTask* pThis) {
    switch (pThis->m0_state) {
    case 0:
        pThis->m2_x0 = pThis->mA_initial_x0 + intDivide(pThis->m1C_transitionDuration, (pThis->m12_target_x0 - pThis->mA_initial_x0) * pThis->m24_currentTick);
        pThis->m4_y0 = pThis->mC_initial_y0 + intDivide(pThis->m1C_transitionDuration, (pThis->m14_target_y0 - pThis->mC_initial_y0) * pThis->m24_currentTick);
        pThis->m6_x1 = pThis->mE_initial_x1 + intDivide(pThis->m1C_transitionDuration, (pThis->m16_target_x1 - pThis->mE_initial_x1) * pThis->m24_currentTick);
        pThis->m8_y1 = pThis->m10_initial_y1 + intDivide(pThis->m1C_transitionDuration, (pThis->m18_target_y1 - pThis->m10_initial_y1) * pThis->m24_currentTick);
        resetCamera(pThis->m2_x0, pThis->m4_y0, pThis->m6_x1, pThis->m8_y1, (pThis->m2_x0 + pThis->m6_x1) / 2, (pThis->m4_y0 + pThis->m8_y1) / 2);
        if (++pThis->m24_currentTick > pThis->m1C_transitionDuration) {
            pThis->m24_currentTick = 0;
            pThis->m0_state++;
        }
        setupVdp1LocalCoordinatesAndClipping();
        break;
    case 1:
        if ((++pThis->m24_currentTick > 90) && pThis) {
            pThis->getTask()->markFinished();
        }
        break;
    }
}

void dragonDeathTask_delete(sDragonDeathTask* pThis) {
    resetCamera(0, 0, 0x160, 0xe0, 0xb0, 0x70);
    setupVdp1LocalCoordinatesAndClipping();
    g_fadeControls.m_4D = 6;
    if (g_fadeControls.m_4C < 7) {
        vdp2Controls.m20_registers[1].m112_CLOFSL = 0;
        vdp2Controls.m20_registers[0].m112_CLOFSL = 0;
    }
    fadePalette(&g_fadeControls.m0_fade0, 0x8000, 0x8000, 1);
    g_fadeControls.m_4D = 5;
    BattleEnd_deleteSub0();
}

static const sDragonDeathTask::TypedTaskDefinition dragonDeathTaskDefinition = {
    dragonDeathTask_init,
    dragonDeathTask_update,
    nullptr,
    dragonDeathTask_delete,
};

struct sBattleFormationTransitionTask : public s_workAreaTemplate<sBattleFormationTransitionTask>
{
    s8 m0_state;
    u8 m1, m2, m3;
    u16 m4_newSubBattleId;
    u16 m6;
    sVec2FPInterpolator m8_autoScrollInterpolator;
    // m8 + 0x38 = m40 = m38_interpolationLength; sizeof = 0x3c
    std::array<sFPInterpolator, 4> m44_dragonSpeedInterpolators;
    std::array<sFPInterpolator, 4> mb4_enemyAltInterpolators;
    std::array<sFPInterpolator, 4> m124_dragonAltInterpolators;
    sFPInterpolator m194_dragonYInterpolator;
    sFPInterpolator m1b0_enemyAltYInterpolator;
    // size: 0x1cc
};

// BTL_A3::060603d0
static void battleFormationTransition_init(sBattleFormationTransitionTask* pThis)
{
    s_battleEngine* psVar7 = gBattleManager->m10_battleOverlay->m4_battleEngine;

    initBattleEngineArray();
    resetNBG1Map();

    // Read next subBattleId from current battle data entry
    sSaturnPtr battleDataEntry = readSaturnEA(psVar7->m3A8_overlayBattledata + (s8)psVar7->m3B0_subBattleId * 0x20 + 4);
    pThis->m4_newSubBattleId = readSaturnU8(battleDataEntry + 0x59);
    psVar7->m3B0_subBattleId = (s8)pThis->m4_newSubBattleId;

    sSaturnPtr nextBattleData = readSaturnEA(psVar7->m3A8_overlayBattledata + (s16)pThis->m4_newSubBattleId * 0x20 + 4);

    // Initialize per-quadrant interpolators: copy current values as start, next battle data as target
    for (int i = 0; i < 4; i++)
    {
        pThis->m44_dragonSpeedInterpolators[i].m4_startValue  = (s32)psVar7->m45C_perQuadrantDragonSpeed[i];
        pThis->mb4_enemyAltInterpolators[i].m4_startValue     = (s32)psVar7->m374_perQuadrantEnemyAltitude[i];
        pThis->m124_dragonAltInterpolators[i].m4_startValue   = (s32)psVar7->m364_perQuadrantDragonAltitude[i];

        pThis->m44_dragonSpeedInterpolators[i].mC_targetValue  = readSaturnS32(nextBattleData + 0x1c + i * 4);
        pThis->mb4_enemyAltInterpolators[i].mC_targetValue     = readSaturnS32(nextBattleData + 0x2c + i * 4);
        pThis->m124_dragonAltInterpolators[i].mC_targetValue   = readSaturnS32(nextBattleData + 0x3c + i * 4);
    }

    // Store auto-scroll delta as start/target for vec2FPInterpolator
    pThis->m8_autoScrollInterpolator.mC_startValue  = psVar7->m1A0_battleAutoScrollDelta;
    pThis->m8_autoScrollInterpolator.m24_targetValue = readSaturnVec3(nextBattleData + 0x4c);

    // Store dragon Y delta and enemy alt as start values for Y interpolators
    pThis->m194_dragonYInterpolator.m4_startValue =
        (s32)psVar7->m104_dragonPosition.m4_Y - (s32)gBattleManager->m10_battleOverlay->mC_targetSystem->m204_cameraMaxAltitude;
    pThis->m1b0_enemyAltYInterpolator.m4_startValue = (s32)psVar7->m270_enemyAltitude.m4_Y;

    // Target values for Y interpolators: per-quadrant altitude at current quadrant
    pThis->m194_dragonYInterpolator.mC_targetValue =
        pThis->m124_dragonAltInterpolators[(s8)psVar7->m22C_dragonCurrentQuadrant].mC_targetValue;
    pThis->m1b0_enemyAltYInterpolator.mC_targetValue =
        pThis->mb4_enemyAltInterpolators[(s8)psVar7->m22C_dragonCurrentQuadrant].mC_targetValue;

    // Set interpolation lengths and initialize all interpolators
    pThis->m8_autoScrollInterpolator.m38_interpolationLength = 0x54;
    vec2FPInterpolator_Init(&pThis->m8_autoScrollInterpolator);

    for (int i = 0; i < 4; i++)
    {
        pThis->m44_dragonSpeedInterpolators[i].m18_interpolationLength  = 0x54;
        pThis->mb4_enemyAltInterpolators[i].m18_interpolationLength     = 0x54;
        pThis->m124_dragonAltInterpolators[i].m18_interpolationLength   = 0x54;

        FPInterpolator_Init(&pThis->m44_dragonSpeedInterpolators[i]);
        FPInterpolator_Init(&pThis->mb4_enemyAltInterpolators[i]);
        FPInterpolator_Init(&pThis->m124_dragonAltInterpolators[i]);
    }

    pThis->m194_dragonYInterpolator.m18_interpolationLength  = 0x54;
    pThis->m1b0_enemyAltYInterpolator.m18_interpolationLength = 0x54;
    FPInterpolator_Init(&pThis->m194_dragonYInterpolator);
    FPInterpolator_Init(&pThis->m1b0_enemyAltYInterpolator);

    // Update battle engine altitude bounds from next battle data
    psVar7->m354_dragonAltitudeMinMax[0] = readSaturnS32(nextBattleData + 0xc);
    psVar7->m354_dragonAltitudeMinMax[1] = readSaturnS32(nextBattleData + 0x10);
    psVar7->m35C_cameraAltitudeMinMax[0] = readSaturnS32(nextBattleData + 0x14);
    psVar7->m35C_cameraAltitudeMinMax[1] = readSaturnS32(nextBattleData + 0x18);
    psVar7->m230 = readSaturnS8(nextBattleData + 0x58);

    psVar7->m3AC = readSaturnEA(psVar7->m3A8_overlayBattledata + (s8)psVar7->m3B0_subBattleId * 0x20);

    battleEngine_UpdateSub1(1);

    if ((s8)g_fadeControls.m_4C < 7)
    {
        vdp2Controls.m20_registers[1].m112_CLOFSL = 0;
        vdp2Controls.m20_registers[0].m112_CLOFSL = 0;
    }

    fadePalette(&g_fadeControls.m0_fade0, convertColorToU32ForFade(g_fadeControls.m0_fade0.m0_color), 0xffff, 0xa8);
    playSystemSoundEffect(0x10);
}

// BTL_A3::0606073c
static void battleFormationTransition_update(sBattleFormationTransitionTask* pThis)
{
    s_battleEngine* psVar6 = gBattleManager->m10_battleOverlay->m4_battleEngine;

    if (pThis->m0_state == 0)
    {
        s32 done = vec2FPInterpolator_Step(&pThis->m8_autoScrollInterpolator);
        if (done)
        {
            if (fileInfoStruct.m2C_allocatedHead != nullptr)
                return;
            psVar6->m18C_status = 1;
            pThis->m0_state++;
            battleEngine_SetBattleMode(mE_battleIntro);
        }

        // Write interpolated auto-scroll delta back to engine
        psVar6->m1A0_battleAutoScrollDelta = pThis->m8_autoScrollInterpolator.m0_currentValue;
        psVar6->m1AC_battleAutoScrollDeltaBackup = psVar6->m1A0_battleAutoScrollDelta;

        // Step per-quadrant interpolators and write results back
        for (int i = 0; i < 4; i++)
        {
            FPInterpolator_Step(&pThis->m44_dragonSpeedInterpolators[i]);
            FPInterpolator_Step(&pThis->mb4_enemyAltInterpolators[i]);
            FPInterpolator_Step(&pThis->m124_dragonAltInterpolators[i]);

            psVar6->m45C_perQuadrantDragonSpeed[i]    = pThis->m44_dragonSpeedInterpolators[i].m0_currentValue;
            psVar6->m374_perQuadrantEnemyAltitude[i]  = (s32)pThis->mb4_enemyAltInterpolators[i].m0_currentValue;
            psVar6->m364_perQuadrantDragonAltitude[i] = (s32)pThis->m124_dragonAltInterpolators[i].m0_currentValue;
        }

        FPInterpolator_Step(&pThis->m194_dragonYInterpolator);
        psVar6->m104_dragonPosition.m4_Y =
            pThis->m194_dragonYInterpolator.m0_currentValue +
            fixedPoint::fromS32((s32)gBattleManager->m10_battleOverlay->mC_targetSystem->m204_cameraMaxAltitude);

        FPInterpolator_Step(&pThis->m1b0_enemyAltYInterpolator);
        psVar6->m270_enemyAltitude.m4_Y = pThis->m1b0_enemyAltYInterpolator.m0_currentValue;
    }
    else if (pThis->m0_state == 1)
    {
        pThis->getTask()->markFinished();
    }
}

static const sBattleFormationTransitionTask::TypedTaskDefinition battleFormationTransitionTaskDefinition = {
    battleFormationTransition_init,
    battleFormationTransition_update,
    nullptr,
    nullptr,
};

// BTL_A3::0607a8ce
static void battleEngine_enableAttackCamera()
{
    s_battleEngine* pBattleEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    pBattleEngine->m188_flags.m800 = 1;
}

// BTL_A3::0607a900
static void sEnemyAttackCamera_updateSub0(int param_1)
{
    s_battleGrid* pGrid = gBattleManager->m10_battleOverlay->m8_gridTask;
    if (param_1 > 2)
        param_1 = 0;
    pGrid->m1 = (s8)param_1;
    s8 offset = pGrid->m1 * 0x24;
    pGrid->m134_desiredCameraPosition = readSaturnVec3(g_BTL_GenericData->getSaturnPtr(0x060ac478) + offset);
    pGrid->m140_desiredCameraTarget = readSaturnVec3(g_BTL_GenericData->getSaturnPtr(0x060ac478) + offset + 0xC);
}

// BTL_A3::0607b5cc
static void battleGrid_initSub0(int param_1)
{
    s_battleGrid* pGrid = gBattleManager->m10_battleOverlay->m8_gridTask;
    pGrid->m218_halfFov = param_1 >> 1;
    pGrid->m1C8_flags |= 0x20;
}

struct sBattleVictoryCameraTask : public s_workAreaTemplate<sBattleVictoryCameraTask>
{
    sVec3_FP m0;      // enemy/target camera position (from per-quadrant data table)
    sVec3_FP mC;      // second position (m10 = 0x5000)
    sVec3_FP m18;     // third position
    s16 m24_counter;
    u8 m26_state;
    u8 m27;
    // size: 0x28
};

// BTL_A3::06061534
static void battleVictoryCameraTask_update(sBattleVictoryCameraTask* pThis)
{
    s_battleEngine* pBattleEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    s_battleGrid* pGrid = gBattleManager->m10_battleOverlay->m8_gridTask;

    switch (pThis->m26_state)
    {
    case 0:
        pThis->m24_counter++;
        if (pThis->m24_counter == 0x19)
        {
            g_fadeControls.m_4D = 6;
            if ((char)g_fadeControls.m_4C < 7)
            {
                vdp2Controls.m20_registers[1].m112_CLOFSL = 0;
                vdp2Controls.m20_registers[0].m112_CLOFSL = 0;
            }
            fadePalette(&g_fadeControls.m0_fade0, 0xc210, 0xeb5a, 0x28);
            g_fadeControls.m_4D = 5;
            pThis->m26_state++;
            pThis->m24_counter = 0;
            if (pBattleEngine->m3D0 != nullptr)
                pBattleEngine->m3D0->clearDeleteMethod();
        }
        break;

    case 1:
        pThis->m24_counter++;
        if (pThis->m24_counter == 0x26)
        {
            battleGrid_initSub0(0x238e38e);
            if (pBattleEngine->m3D0 != nullptr)
            {
                pBattleEngine->m3D0->getTask()->markFinished();
                pBattleEngine->m3D0 = nullptr;
            }
            battleEngine_enableAttackCamera();
            sEnemyAttackCamera_updateSub0(1);
            battleEngineSub1_UpdateSub2(&pBattleEngine->m418, pBattleEngine->m104_dragonPosition, pThis->m0, pThis->m18);
            battleEngineSub1_UpdateSub2(&pBattleEngine->m424, gBattleManager->m10_battleOverlay->m18_dragon->m8_position, pThis->mC, pThis->m18);
            battleEngine_setCurrentCameraPositionPointer(&pBattleEngine->m418);
            battleEngine_setDesiredCameraPositionPointer(&pBattleEngine->m424);
            sVec2_FP sStack_28;
            battleEngine_resetCameraInterpolationSub0(sStack_28);
            battleEngine_resetCameraInterpolationSub1(sStack_28);
            pGrid->m18C = {};
            pGrid->mC0_cameraRotationInterpolation = {};
            pThis->m26_state++;
            pThis->m24_counter = 0;
            if (gBattleManager->m10_battleOverlay->m1C_envTask != nullptr)
            {
                p_workArea envChild = *(p_workArea*)((char*)gBattleManager->m10_battleOverlay->m1C_envTask + 0x58);
                if (envChild != nullptr)
                    envChild->getTask()->markPaused();
            }
        }
        break;

    case 2:
        pThis->m24_counter++;
        if (pThis->m24_counter == 10)
            pThis->m26_state++;
        pBattleEngine->m164.m4_Y += fixedPoint::fromS32(0x999);
        {
            sVec2_FP sStack_28;
            battleEngine_resetCameraInterpolationSub0(sStack_28);
            battleEngine_resetCameraInterpolationSub1(sStack_28);
        }
        pGrid->m18C = {};
        pGrid->mC0_cameraRotationInterpolation = {};
        battleEngineSub1_UpdateSub2(&pBattleEngine->m418, pBattleEngine->m104_dragonPosition, pThis->m0, pThis->m18);
        battleEngineSub1_UpdateSub2(&pBattleEngine->m424, gBattleManager->m10_battleOverlay->m18_dragon->m8_position, pThis->mC, pThis->m18);
        {
            s8 q = pBattleEngine->m22C_dragonCurrentQuadrant;
            pThis->m0.m0_X += fixedPoint::fromS32(readSaturnS32(g_BTL_GenericData->getSaturnPtr(0x060a96d8 + q * 12)) >> 3);
            pThis->m0.m8_Z += fixedPoint::fromS32(readSaturnS32(g_BTL_GenericData->getSaturnPtr(0x060a96e0 + q * 12)) >> 3);
        }
        break;

    case 3:
        pThis->m24_counter++;
        if (pThis->m24_counter == 0x3c)
            createBattleEndTask(pThis, 1);
        pBattleEngine->m164.m4_Y += fixedPoint::fromS32(0x999);
        {
            sVec2_FP sStack_28;
            battleEngine_resetCameraInterpolationSub0(sStack_28);
            battleEngine_resetCameraInterpolationSub1(sStack_28);
        }
        pGrid->m18C = {};
        pGrid->mC0_cameraRotationInterpolation = {};
        battleEngineSub1_UpdateSub2(&pBattleEngine->m418, pBattleEngine->m104_dragonPosition, pThis->m0, pThis->m18);
        battleEngineSub1_UpdateSub2(&pBattleEngine->m424, gBattleManager->m10_battleOverlay->m18_dragon->m8_position, pThis->mC, pThis->m18);
        graphicEngineStatus.m5_isTildeDown = 1;
        {
            s8 q = pBattleEngine->m22C_dragonCurrentQuadrant;
            pThis->m0.m0_X += fixedPoint::fromS32(readSaturnS32(g_BTL_GenericData->getSaturnPtr(0x060a96d8 + q * 12)) >> 3);
            pThis->m0.m8_Z += fixedPoint::fromS32(readSaturnS32(g_BTL_GenericData->getSaturnPtr(0x060a96e0 + q * 12)) >> 3);
        }
        break;

    default:
        break;
    }
}

// BTL_A3::06061a24
static void battleVictoryCamera_create(p_workArea parent)
{
    sBattleVictoryCameraTask* pThis = createSiblingTaskFromFunction<sBattleVictoryCameraTask>(parent, battleVictoryCameraTask_update);

    s8 q = gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant;
    pThis->m0.m0_X = readSaturnFP(g_BTL_GenericData->getSaturnPtr(0x060a96d8 + q * 12));
    pThis->m0.m4_Y = readSaturnFP(g_BTL_GenericData->getSaturnPtr(0x060a96dc + q * 12));
    pThis->m0.m8_Z = readSaturnFP(g_BTL_GenericData->getSaturnPtr(0x060a96e0 + q * 12));
    pThis->mC = {};
    pThis->mC.m4_Y = fixedPoint::fromS32(0x5000);
    pThis->m18 = {};
    pThis->m24_counter = 0;
    pThis->m26_state = 0;
    playSystemSoundEffect(0x10);
}

s32 battleEngine_checkBattleCompletion()
{
    s_battleEngine* pBattleEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    if (pBattleEngine->m3B2_numBattleFormationRunning < 1)
    {
        if (battleEngine_isBattleIntroFinished() == 0)
            return 0;

        battleEngine_UpdateSub7Sub1();
        pBattleEngine->m1B8_dragonPitch = 0;
        pBattleEngine->m1BC_dragonYaw = 0;
        pBattleEngine->m3B2_numBattleFormationRunning = 0;

        clearVdp2TextMemory();
        resetNBG1Map();

        pBattleEngine->m18C_status = 4;
        pBattleEngine->m188_flags.m80000_hideBattleHUD = 0;

        if (mainGameState.gameStats.m10_currentHP > 0)
        {
            battleEngine_UpdateSub7Sub2();
            battleEngine_restoreCameraDefault();

            if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m10000 == 0)
            {
                gBattleManager->mE = 0;
            }
            else
            {
                gBattleManager->mE = 3;
            }

            u8 battleFormation = pBattleEngine->m230;
            if ((((battleFormation != 1) && (battleFormation != 3)) && (battleFormation != 5)) && ((battleFormation != 7 && (battleFormation != 8))))
            {
                // Battle won - not in special formation: show result screen
                pBattleEngine->m188_flags.m8_showingBattleResultScreen = 1;
                createBattleResultScreen(pBattleEngine);
                return 1;
            }

            // Special formation - check m10000 flag for alternative path
            if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m10000 != 0)
            {
                // m10000 flag set - create result screen with mE=0
                pBattleEngine->m188_flags.m8_showingBattleResultScreen = 1;
                createBattleResultScreen(pBattleEngine);
                gBattleManager->mE = 0;
                return 1;
            }

            // m10000 not set - transition to next sub-battle formation
            createSubTask<sBattleFormationTransitionTask>(pBattleEngine, &battleFormationTransitionTaskDefinition);
            return 1;
        }

        // Dragon HP <= 0 while formation running - dragon death path
        pBattleEngine->m188_flags.m8_showingBattleResultScreen = 1;
        createSubTask<sDragonDeathTask>(pBattleEngine, &dragonDeathTaskDefinition);
        gBattleManager->mE = 2;
        return 1;
    }

    // Check dragon death path (HP dropped mid-battle)
    if (mainGameState.gameStats.m10_currentHP < 1)
    {
        if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x17] != 0) {
            pBattleEngine->m3B2_numBattleFormationRunning = 0;
            return 0;
        }
        if (battleEngine_isBattleIntroFinished() != 0) {
            pBattleEngine->m188_flags.m8_showingBattleResultScreen = 1;
            pBattleEngine->m18C_status = 4;
            pBattleEngine->m1B8_dragonPitch = 0;
            pBattleEngine->m1BC_dragonYaw = 0;
            pBattleEngine->m3B2_numBattleFormationRunning = 0;
            clearVdp2TextMemory();
            resetNBG1Map();
            createSubTask<sDragonDeathTask>(pBattleEngine, &dragonDeathTaskDefinition);
            gBattleManager->mE = 2;
            return 1;
        }
    }
    else if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m4 != 0)
    {
        // Flag m4 set - call battleVictoryCamera_create and perform state transition
        battleVictoryCamera_create(pBattleEngine);
        pBattleEngine->m188_flags.m8_showingBattleResultScreen = 1;
        pBattleEngine->m18C_status = 2;
        pBattleEngine->m1B8_dragonPitch = 0;
        pBattleEngine->m1BC_dragonYaw = 0;
        pBattleEngine->m3B2_numBattleFormationRunning = 0;
        gBattleManager->mE = 1;
        return 1;
    }

    pBattleEngine->m3B2_numBattleFormationRunning = 0;
    return 0;
}

#ifndef SHIPPING_BUILD
void addTraceLog(s_battleEngine::s_27C* pThis, const char* name)
{
    addTraceLog(pThis->m0_computedValue, "m0_computedValue");
    addTraceLog(pThis->mC_initialValue, "mC_initialValue");
    addTraceLog(pThis->m18_middle, "m18_middle");
    addTraceLog(pThis->m24_targetValue, "m24_targetValue");
    addTraceLog(pThis->m30, "m30");
    addTraceLog(pThis->m3C, "m3C");
    addTraceLog(pThis->m48, "m48");
    addTraceLog(pThis->m50, "m50");
    addTraceLog(pThis->m54, "m54");
    addTraceLog(pThis->m60_currentStep, "m60_currentStep");
    addTraceLog(pThis->m64_stepSize, "m64_stepSize");
    addTraceLog(pThis->m68_rate, "m68_rate");
}
#endif

void battleEngine_UpdateSub7Sub0Sub1Sub1(s_battleEngine::s_27C* pThis)
{
    pThis->m48 = (pThis->m54 - pThis->m3C) / 2;
    pThis->m60_currentStep = 0;
    pThis->m64_stepSize = FP_Div(0x8000000, fixedPoint::fromInteger(pThis->m68_rate));

    if (isTraceEnabled())
    {
        addTraceLog(pThis, "interpolator");
    }
}

void battleEngine_UpdateSub7Sub0Sub1Sub2(s_battleEngine::s_27C* pThis)
{
    pThis->m18_middle = (pThis->m24_targetValue - pThis->mC_initialValue) / 2;
    pThis->m60_currentStep = 0;
    pThis->m64_stepSize = FP_Div(0x8000000, fixedPoint::fromInteger(pThis->m68_rate));
    pThis->m0_computedValue = pThis->mC_initialValue;

    if (isTraceEnabled())
    {
        addTraceLog(pThis, "interpolator");
    }
}

bool updateInterpolator(s_battleEngine::s_27C* pThis)
{
    bool result;
    if (pThis->m60_currentStep > 0x7ffffff)
    {
        pThis->m60_currentStep = 0x8000000;
        pThis->m64_stepSize = 0;
        pThis->m0_computedValue = pThis->m24_targetValue;
        result = true;
    }
    else
    {
        pThis->m0_computedValue = pThis->m18_middle + pThis->mC_initialValue - MTH_Mul(getCos(pThis->m60_currentStep.getInteger()), pThis->m18_middle);
        pThis->m60_currentStep += pThis->m64_stepSize;
        result = false;
    }

    if (isTraceEnabled())
    {
        addTraceLog(pThis, "interpolator");
    }

    return result;
}

void stepInterpolator(s_battleEngine::s_27C* pThis)
{
    pThis->m30 = pThis->m48 + pThis->m3C - MTH_Mul(getCos(pThis->m60_currentStep.getInteger()), pThis->m48);

    if (isTraceEnabled())
    {
        addTraceLog(pThis, "interpolator");
    }
}

void triggerIfBattleModeA(s_battleEngine* pThis)
{
    if (gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode != eBattleModes::mA)
    {
        battleEngine_enableAttackCamera();
        sEnemyAttackCamera_updateSub0(2);
    }
}

void monsterMoveDragonSub0(s_battleEngine* pThis)
{
    fixedPoint uVar1 = pThis->m448.normalized();

    if ((uVar1 < -0x4000000) || (uVar1 > 0x3ffffff))
    {
        pThis->m46C_dragon2dSpeed[1] = pThis->m45C_perQuadrantDragonSpeed[0];
    }
    else
    {
        pThis->m46C_dragon2dSpeed[1] = pThis->m45C_perQuadrantDragonSpeed[2];
    }

    if (uVar1 < 1)
    {
        pThis->m46C_dragon2dSpeed[0] = pThis->m45C_perQuadrantDragonSpeed[1];
    }
    else
    {
        pThis->m46C_dragon2dSpeed[0] = pThis->m45C_perQuadrantDragonSpeed[3];
    }
}

void battleEngine_enemyForcedMoveUpdate(s_battleEngine* pThis)
{
    switch (pThis->m38D_battleSubMode)
    {
    case 0:
        battleCreateCinematicBars(pThis);
        pThis->m38D_battleSubMode++;
        gBattleManager->m10_battleOverlay->m8_gridTask->m1BC_cameraRotationStep[0] = 0;
        gBattleManager->m10_battleOverlay->m8_gridTask->m1BC_cameraRotationStep[1] = 0;
        break;
    case 1:
        if (pThis->m384_battleModeDelay++ >= 5)
        {
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2000 = 1;
            pThis->m384_battleModeDelay = 0;
            pThis->m38D_battleSubMode++;
        }
        break;
    case 2:
    {
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m200000 = 1;
        pThis->m38D_battleSubMode++;
        int dStack72 = 0;
        int iVar3 = 3;
        int stackx40_1 = 0;
        fixedPoint dVar7 = pThis->m164[0];
        fixedPoint dVar8 = pThis->m164[2];
        switch (gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant)
        {
        case 0:
            dStack72 = 0x8000000;
            switch (pThis->m22E_dragonMoveDirection)
            {
            case 1:
                pThis->m270_enemyAltitude[0] += pThis->m45C_perQuadrantDragonSpeed[3];
                pThis->m270_enemyAltitude[2] += pThis->m45C_perQuadrantDragonSpeed[0];
                pThis->m258[0] = -pThis->m45C_perQuadrantDragonSpeed[3];
                pThis->m258[2] = 0;
                stackx40_1 = 0x4000000;
                break;
            case 2:
                pThis->m270_enemyAltitude[0] -= pThis->m45C_perQuadrantDragonSpeed[1];
                pThis->m270_enemyAltitude[2] += pThis->m45C_perQuadrantDragonSpeed[0];
                pThis->m258[0] = -pThis->m45C_perQuadrantDragonSpeed[1];
                pThis->m258[2] = 0;
                stackx40_1 = 0xc000000;
                break;
            case 3:
            case 4:
                pThis->m270_enemyAltitude[2] += pThis->m45C_perQuadrantDragonSpeed[0] + pThis->m45C_perQuadrantDragonSpeed[2];
                pThis->m258[0] = 0;
                pThis->m258[2] = -pThis->m45C_perQuadrantDragonSpeed[2];
                break;
            case 5:
                pThis->m270_enemyAltitude[2] += pThis->m45C_perQuadrantDragonSpeed[0] + pThis->m45C_perQuadrantDragonSpeed[2];
                pThis->m258[0] = 0;
                pThis->m258[2] = -pThis->m45C_perQuadrantDragonSpeed[2];
                stackx40_1 = 0x10000000;
                iVar3 = 2;
                break;
            default:
                assert(0);
            }
            break;
        case 2:
            //:0606cdc6
            switch (pThis->m22E_dragonMoveDirection)
            {
            case 3:
                (pThis->m270_enemyAltitude)[2] = (pThis->m270_enemyAltitude)[2] - (pThis->m45C_perQuadrantDragonSpeed[2] + pThis->m45C_perQuadrantDragonSpeed[0]);
                (pThis->m258)[0] = 0;
                (pThis->m258)[2] = pThis->m45C_perQuadrantDragonSpeed[0];
                stackx40_1 = 0;
                iVar3 = 2;
                break;
            default:
                assert(0);
            }
            break;
        default:
            assert(0);
        }

        pThis->m2E8_dragonMovementInterpolator2.m3C = sVec3_FP(0, dStack72, 0);
        pThis->m2E8_dragonMovementInterpolator2.m54 = sVec3_FP(0, stackx40_1, 0);

        battleEngine_UpdateSub7Sub0Sub1Sub1(&pThis->m2E8_dragonMovementInterpolator2);

        pThis->m240 = pThis->m270_enemyAltitude + pThis->m258 + pThis->m264;

        pThis->m27C_dragonMovementInterpolator1.mC_initialValue = pThis->m104_dragonPosition - pThis->m240;
        pThis->m27C_dragonMovementInterpolator1.mC_initialValue[1] = pThis->m104_dragonPosition[1] - pThis->m35C_cameraAltitudeMinMax[1];
        pThis->m27C_dragonMovementInterpolator1.m24_targetValue = sVec3_FP(0, pThis->m364_perQuadrantDragonAltitude[iVar3], 0);
        battleEngine_UpdateSub7Sub0Sub1Sub2(&pThis->m27C_dragonMovementInterpolator1);

        pThis->m2E8_dragonMovementInterpolator2.mC_initialValue = sVec3_FP(dVar7, pThis->m270_enemyAltitude[1], dVar8);
        pThis->m2E8_dragonMovementInterpolator2.m24_targetValue = sVec3_FP(0, pThis->m374_perQuadrantEnemyAltitude[iVar3], 0);
        battleEngine_UpdateSub7Sub0Sub1Sub2(&pThis->m2E8_dragonMovementInterpolator2);

        triggerIfBattleModeA(pThis);
        break;
    }

    case 3:
        if (updateInterpolator(&pThis->m2E8_dragonMovementInterpolator2))
        {
            battleEngine_updateQuadrantFromAngle(pThis, pThis->m448, 1);
            battleEngine_convertBattleQuadrantToBattleDirection(pThis);
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m200000 = 0;
            pThis->m394 = 0;
            pThis->m398_currentSelectedEnemy = 0;
            pThis->m384_battleModeDelay = 0;
            if (gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode == eBattleModes::mA)
            {
                pThis->m38D_battleSubMode++;
            }
            else
            {
                pThis->m38D_battleSubMode = 0x8;
                battleEngine_UpdateSub7Sub2();
                gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m4000000 = 1;
                gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m20_battleIntroRunning = 0;
                gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m400000 = 0;
            }
        }
        else
        {
            stepInterpolator(&pThis->m2E8_dragonMovementInterpolator2);
            pThis->m444 = pThis->m2E8_dragonMovementInterpolator2.m30[0];
            pThis->m448 = pThis->m2E8_dragonMovementInterpolator2.m30[1];
            monsterMoveDragonSub0(pThis);

            if (pThis->m22E_dragonMoveDirection == 3)
            {
                switch (gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant)
                {
                case 0:
                case 2:
                    pThis->m46C_dragon2dSpeed[0] = 0;
                    break;
                case 1:
                case 3:
                    pThis->m46C_dragon2dSpeed[1] = 0;
                    break;
                default:
                    assert(0);
                }
            }

            pThis->m6C_dragonIntegrateStep[0] = MTH_Mul(pThis->m46C_dragon2dSpeed[0], getSin(pThis->m448.getInteger()));
            pThis->m6C_dragonIntegrateStep[2] = MTH_Mul(pThis->m46C_dragon2dSpeed[1], getCos(pThis->m448.getInteger()));

            pThis->mC_battleCenter[0] = pThis->m240[0] + pThis->m6C_dragonIntegrateStep[0];
            pThis->mC_battleCenter[2] = pThis->m240[2] + pThis->m6C_dragonIntegrateStep[2];

            pThis->m270_enemyAltitude[1] = pThis->m2E8_dragonMovementInterpolator2.m0_computedValue[1];

            pThis->m164[0] = pThis->m2E8_dragonMovementInterpolator2.m0_computedValue[0];
            pThis->m164[2] = pThis->m2E8_dragonMovementInterpolator2.m0_computedValue[2];

            updateInterpolator(&pThis->m27C_dragonMovementInterpolator1);

            pThis->m104_dragonPosition = pThis->m27C_dragonMovementInterpolator1.m0_computedValue + pThis->m240;
            pThis->m104_dragonPosition[1] = pThis->m27C_dragonMovementInterpolator1.m0_computedValue[1] + pThis->m35C_cameraAltitudeMinMax[1];

            if (gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode != eBattleModes::mA)
            {
                battleEngineSub1_UpdateSub2(&pThis->m400, gBattleManager->m10_battleOverlay->m8_gridTask->m34_cameraPosition, pThis->m3E8, pThis->m3DC);
            }
        }
        break;
    }
}

void battleEngine_playerOrbitUpdate(s_battleEngine* pThis)
{
    switch (pThis->m38D_battleSubMode)
    {
    case 0: // init dragon move
    {
        pThis->m38D_battleSubMode++;

        battleEngine_convertBattleQuadrantToBattleDirection(pThis);

        sVec3_FP stackx4;
        sVec3_FP stackx10;
        sVec3_FP stackx34;
        sVec3_FP stackx40;

        stackx4.zeroize();
        stackx10.zeroize();
        stackx34.zeroize();
        stackx40.zeroize();

        sVec3_FP stackx1C_dragonTargetRotation = gBattleManager->m10_battleOverlay->m18_dragon->m74_targetRotation;
        sVec3_FP stackx28 = stackx1C_dragonTargetRotation;

        stackx10 = pThis->m60;

        char pcVar15;

        switch (pThis->m22C_dragonCurrentQuadrant)
        {
        case 0:
            switch (pThis->m22E_dragonMoveDirection)
            {
            case 1:
                stackx40[1] = 0x4000000;
                stackx28[1] += 0x4000000;
                pcVar15 = 1;
                break;
            case 2:
                stackx40[1] = -0x4000000;
                stackx28[1] -= 0x4000000;
                pcVar15 = 3;
                break;
            case 3:
                stackx40[1] = 0x8000000;
                stackx28[1] += 0x8000000;
                pcVar15 = 2;
                break;
            default:
                assert(0);
            }
            break;
        case 1:
            switch (pThis->m22E_dragonMoveDirection)
            {
            case 1:
                stackx40[1] = 0x8000000;
                stackx28[1] += 0x4000000;
                pcVar15 = 2;
                break;
            case 2:
                stackx40[1] = 0;
                stackx28[1] -= 0x4000000;
                pcVar15 = 0;
                break;
            case 3:
                stackx40[1] = -0x4000000;
                stackx28[1] += 0x8000000;
                pcVar15 = 3;
                break;
            default:
                assert(0);
            }
            break;
        case 2:
            switch (pThis->m22E_dragonMoveDirection)
            {
            case 1:
                stackx40[1] = 0xC000000;
                stackx28[1] += 0x4000000;
                pcVar15 = 3;
                break;
            case 2:
                stackx40[1] = 0x4000000;
                stackx28[1] -= 0x4000000;
                pcVar15 = 1;
                break;
            case 3:
                stackx40[1] = 0;
                stackx28[1] += 0x8000000;
                pcVar15 = 0;
                break;
            default:
                assert(0);
            }
            break;
        case 3:
            switch (pThis->m22E_dragonMoveDirection)
            {
            case 1:
                stackx40[1] = 0;
                stackx28[1] += 0x4000000;
                pcVar15 = 0;
                break;
            case 2:
                stackx40[1] = -0x8000000;
                stackx28[1] -= 0x4000000;
                pcVar15 = 2;
                break;
            case 3:
                stackx40[1] = 0x4000000;
                stackx28[1] += 0x8000000;
                pcVar15 = 1;
                break;
            default:
                assert(0);
            }
            break;
        default:
            assert(0);
        }

        pThis->m27C_dragonMovementInterpolator1.m3C[0] = pThis->m43C;
        pThis->m27C_dragonMovementInterpolator1.m3C[1] = pThis->m440_battleDirectionAngle;
        pThis->m27C_dragonMovementInterpolator1.m3C[2] = 0;

        pThis->m27C_dragonMovementInterpolator1.m54[0] = 0;
        pThis->m27C_dragonMovementInterpolator1.m54[1] = stackx40[1];
        pThis->m27C_dragonMovementInterpolator1.m54[2] = 0;

        battleEngine_UpdateSub7Sub0Sub1Sub1(&pThis->m27C_dragonMovementInterpolator1);

        pThis->m2E8_dragonMovementInterpolator2.m3C = stackx1C_dragonTargetRotation;
        pThis->m2E8_dragonMovementInterpolator2.m54 = stackx28;

        battleEngine_UpdateSub7Sub0Sub1Sub1(&pThis->m2E8_dragonMovementInterpolator2);

        pThis->m27C_dragonMovementInterpolator1.mC_initialValue.zeroize();
        pThis->m27C_dragonMovementInterpolator1.m24_targetValue.zeroize();

        pThis->m27C_dragonMovementInterpolator1.mC_initialValue[0] = pThis->m164[0];
        pThis->m27C_dragonMovementInterpolator1.mC_initialValue[1] = pThis->m104_dragonPosition[1] - pThis->m35C_cameraAltitudeMinMax[1];
        pThis->m27C_dragonMovementInterpolator1.mC_initialValue[2] = pThis->m164[2];

        pThis->m27C_dragonMovementInterpolator1.m24_targetValue[0] = 0;
        pThis->m27C_dragonMovementInterpolator1.m24_targetValue[1] = pThis->m364_perQuadrantDragonAltitude[pcVar15];
        pThis->m27C_dragonMovementInterpolator1.m24_targetValue[2] = 0;

        battleEngine_UpdateSub7Sub0Sub1Sub2(&pThis->m27C_dragonMovementInterpolator1);

        pThis->m2E8_dragonMovementInterpolator2.mC_initialValue.zeroize();
        pThis->m2E8_dragonMovementInterpolator2.m24_targetValue.zeroize();

        pThis->m2E8_dragonMovementInterpolator2.mC_initialValue[1] = pThis->m270_enemyAltitude[1];
        pThis->m2E8_dragonMovementInterpolator2.m24_targetValue[1] = pThis->m374_perQuadrantEnemyAltitude[pcVar15];

        battleEngine_UpdateSub7Sub0Sub1Sub2(&pThis->m2E8_dragonMovementInterpolator2);

        battleEngine_enableAttackCamera();
    }
    break;
    case 1:
        // dragon moving
        if (updateInterpolator(&pThis->m2E8_dragonMovementInterpolator2))
        {
            battleEngine_updateQuadrantFromAngle(pThis, pThis->m440_battleDirectionAngle, 0);
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m800 = 1;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m4000000 = 1;
            pThis->m38D_battleSubMode++; // go to state 2 (finished)
            pThis->m384_battleModeDelay = 0;
            pThis->m394 = 0;
            pThis->m398_currentSelectedEnemy = 0;
        }

        stepInterpolator(&pThis->m27C_dragonMovementInterpolator1);
        pThis->m43C = pThis->m27C_dragonMovementInterpolator1.m30[0];
        pThis->m440_battleDirectionAngle = pThis->m27C_dragonMovementInterpolator1.m30[1];

        stepInterpolator(&pThis->m2E8_dragonMovementInterpolator2);
        gBattleManager->m10_battleOverlay->m18_dragon->m74_targetRotation[1] = pThis->m2E8_dragonMovementInterpolator2.m30[1];

        if (pThis->m22E_dragonMoveDirection == 1)
        {
            gBattleManager->m10_battleOverlay->m8_gridTask->mB4_cameraRotation[2] = MTH_Mul(getSin(pThis->m2E8_dragonMovementInterpolator2.m60_currentStep.toInteger()), 0xaaaaaa);
        }
        else if (pThis->m22E_dragonMoveDirection == 2)
        {
            gBattleManager->m10_battleOverlay->m8_gridTask->mB4_cameraRotation[2] = MTH_Mul(getSin(pThis->m2E8_dragonMovementInterpolator2.m60_currentStep.toInteger()), -0xaaaaaa);
        }
        else
        {
            // direction 3 (jump 2 quadrants): no camera tilt, direct position update
            gBattleManager->m10_battleOverlay->m8_gridTask->m64_cameraRotationTarget[2] = gBattleManager->m10_battleOverlay->m8_gridTask->mB4_cameraRotation[2];
            pThis->m270_enemyAltitude[1] = pThis->m2E8_dragonMovementInterpolator2.m0_computedValue[1];
            updateInterpolator(&pThis->m27C_dragonMovementInterpolator1);
            pThis->m104_dragonPosition[1] = pThis->m27C_dragonMovementInterpolator1.m0_computedValue[1] + pThis->m35C_cameraAltitudeMinMax[1];
            pThis->m164[0] = pThis->m27C_dragonMovementInterpolator1.m0_computedValue[0];
            pThis->m164[2] = pThis->m27C_dragonMovementInterpolator1.m0_computedValue[2];
            break;
        }
        gBattleManager->m10_battleOverlay->m8_gridTask->m64_cameraRotationTarget[2] = gBattleManager->m10_battleOverlay->m8_gridTask->mB4_cameraRotation[2];

        pThis->m270_enemyAltitude[1] = pThis->m2E8_dragonMovementInterpolator2.m0_computedValue[1];

        updateInterpolator(&pThis->m27C_dragonMovementInterpolator1);

        pThis->m104_dragonPosition[1] = pThis->m27C_dragonMovementInterpolator1.m0_computedValue[1] + pThis->m35C_cameraAltitudeMinMax[1];

        pThis->m164[0] = pThis->m27C_dragonMovementInterpolator1.m0_computedValue[0];
        pThis->m164[2] = pThis->m27C_dragonMovementInterpolator1.m0_computedValue[2];
        break;
    case 2: // dragon move is over
        if (pThis->m384_battleModeDelay++ < 5)
            return;

        battleEngine_SetBattleMode16();
        break;
    default:
        assert(0);
    }
}

void battleEngine_processMovementMode(s_battleEngine* pThis)
{
    if (!battleEngine_isPlayerTurnActive()) {
        gBattleManager->m10_battleOverlay->m4_battleEngine->m190[0]++;
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m200_suppressBattleInputs = 0;
    }

    switch (pThis->m38C_battleMode)
    {
    case eBattleModes::m6_dragonMoving:
        gBattleManager->m10_battleOverlay->m4_battleEngine->m190[0]++;
        battleEngine_playerOrbitUpdate(pThis);
        break;
    case eBattleModes::mB_enemyMovingDragon:
        gBattleManager->m10_battleOverlay->m4_battleEngine->m190[0]++;
        // fall through
    case eBattleModes::mA:
        battleEngine_enemyForcedMoveUpdate(pThis);
        break;
    default:
        return;
    }
}

void battleEngine_staticBattleOrbitUpdate(s_battleEngine* pThis)
{
    if ((battleEngine_isInputAllowed() != 0) && !gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m400000 && gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x1F])
    {
        assert(0);
    }
}

void battleEngine_computeDragonPosition(s_battleEngine* pThis)
{
    sVec2_FP aiStack52;

    if (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m200000)
    {
        battleEngine_InitSub2Sub0(pThis);
        pThis->mC_battleCenter = pThis->m234 + pThis->m24C;
        pThis->m6C_dragonIntegrateStep[0] = MTH_Mul(pThis->m46C_dragon2dSpeed[0], getSin(pThis->m440_battleDirectionAngle.getInteger()));
        pThis->m6C_dragonIntegrateStep[2] = MTH_Mul(pThis->m46C_dragon2dSpeed[1], getCos(pThis->m440_battleDirectionAngle.getInteger()));
        pThis->m104_dragonPosition[0] = pThis->m234[0] + pThis->m6C_dragonIntegrateStep[0];
        pThis->m104_dragonPosition[2] = pThis->m234[2] + pThis->m6C_dragonIntegrateStep[2];

        if (isTraceEnabled())
        {
            addTraceLog(pThis->m46C_dragon2dSpeed, "m46C_dragon2dSpeed");
            addTraceLog(pThis->m440_battleDirectionAngle, "m440_battleDirectionAngle");
            addTraceLog(pThis->m234, "234");
            addTraceLog(pThis->m6C_dragonIntegrateStep, "m6C_dragonIntegrateStep");
            addTraceLog(pThis->mC_battleCenter, "C");
            addTraceLog(pThis->m104_dragonPosition, "m104_dragonStartPosition");
        }

        computeLookAt(pThis->mC_battleCenter - pThis->m104_dragonPosition, aiStack52);
        pThis->m43C = aiStack52[0];
    }

    if (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m800)
    {
        sVec3_FP iStack88 = pThis->m104_dragonPosition + MTH_Mul(0x8000, pThis->mC_battleCenter - pThis->m104_dragonPosition);

        sVec2_FP aiStack96;
        // TODO: there is some dead code here?
        computeLookAt(iStack88 - gBattleManager->m10_battleOverlay->m8_gridTask->m34_cameraPosition, aiStack96);
        pThis->m1C8 = aiStack96[0];
        pThis->m1CC = aiStack96[1];

        gBattleManager->m10_battleOverlay->m8_gridTask->m64_cameraRotationTarget[0] = aiStack96[0];
        gBattleManager->m10_battleOverlay->m8_gridTask->m64_cameraRotationTarget[1] = aiStack96[1];
    }
    else
    {
        battleEngine_resetCameraInterpolationSub0(aiStack52);
    }
    battleEngine_InitSub3Sub0(pThis);
}

void battleEngine_updateQuadrantFromAngle(s_battleEngine* pThis, fixedPoint uParm2, s32 r6)
{
    uParm2 = uParm2.normalized();

    if (((int)uParm2 < -0x2000000) || (0x1ffffff < (int)uParm2)) {
        if (((int)uParm2 < -0x6000000) || (-0x2000001 < (int)uParm2)) {
            if (((int)uParm2 < 0x2000000) || (0x6000000 <= (int)uParm2))
            {
                if (pThis->m22C_dragonCurrentQuadrant != 0)
                {
                    gBattleManager->m10_battleOverlay->m18_dragon->m88 |= 0x100;
                    pThis->m22D_dragonPreviousQuadrant = pThis->m22C_dragonCurrentQuadrant;
                    gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2_needToSortEnemiesByDistanceFromDragon = 1;
                }
                if (r6 == 0)
                {
                    pThis->m22C_dragonCurrentQuadrant = 2;
                }
                else
                {
                    pThis->m22C_dragonCurrentQuadrant = 0;
                }
            }
            else
            {
                if (pThis->m22C_dragonCurrentQuadrant != 3)
                {
                    gBattleManager->m10_battleOverlay->m18_dragon->m88 |= 0x100;
                    pThis->m22D_dragonPreviousQuadrant = pThis->m22C_dragonCurrentQuadrant;
                    gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2_needToSortEnemiesByDistanceFromDragon = 1;
                }
                if (r6 == 0)
                {
                    pThis->m22C_dragonCurrentQuadrant = 1;
                }
                else
                {
                    pThis->m22C_dragonCurrentQuadrant = 3;
                }
            }
        }
        else
        {
            if (pThis->m22C_dragonCurrentQuadrant != 1)
            {
                gBattleManager->m10_battleOverlay->m18_dragon->m88 |= 0x100;
                pThis->m22D_dragonPreviousQuadrant = pThis->m22C_dragonCurrentQuadrant;
                gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2_needToSortEnemiesByDistanceFromDragon = 1;
            }
            if (r6 == 0)
            {
                pThis->m22C_dragonCurrentQuadrant = 3;
            }
            else
            {
                pThis->m22C_dragonCurrentQuadrant = 1;
            }

        }
    }
    else
    {
        if (pThis->m22C_dragonCurrentQuadrant != 2)
        {
            gBattleManager->m10_battleOverlay->m18_dragon->m88 |= 0x100;
            pThis->m22D_dragonPreviousQuadrant = pThis->m22C_dragonCurrentQuadrant;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2_needToSortEnemiesByDistanceFromDragon = 1;
        }
        if (r6 == 0)
        {
            pThis->m22C_dragonCurrentQuadrant = 0;
        }
        else
        {
            pThis->m22C_dragonCurrentQuadrant = 2;
        }
    }
}

void battleEngine_detectQuadrantFromAngle(s_battleEngine* pThis)
{
    if (battleEngine_isInputAllowed())
    {
        battleEngine_updateQuadrantFromAngle(pThis, pThis->m440_battleDirectionAngle, 0);
    }
}

struct sEnemyAttackCamera : public s_workAreaTemplate<sEnemyAttackCamera>
{
    s8 m1_cameraIndex;
    sVec3_FP m4;
    sVec3_FP m10_rotation1;
    sVec3_FP m1C_rotation1Step;
    sVec3_FP m28;
    sVec3_FP m34;
    sVec3_FP m40_rotation2;
    sVec3_FP m4C_rotation2Step;
    s16 m58;

    // size: 5C
};

void attackCamera1(sVec3_FP* pOutput)
{
    s_battleEngine* pBattleEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;

    pOutput->zeroize();

    int randomValue = randomNumber() & 1;

    switch (pBattleEngine->m22C_dragonCurrentQuadrant)
    {
    case 0:
        (*pOutput)[2] = pBattleEngine->m45C_perQuadrantDragonSpeed[pBattleEngine->m22C_dragonCurrentQuadrant] / 2;
        if (randomValue == 0)
        {
            (*pOutput)[0] = pBattleEngine->m45C_perQuadrantDragonSpeed[pBattleEngine->m22C_dragonCurrentQuadrant] / 2;
        }
        else
        {
            (*pOutput)[0] = -pBattleEngine->m45C_perQuadrantDragonSpeed[pBattleEngine->m22C_dragonCurrentQuadrant] / 2;
        }
        break;
    case 1:
        (*pOutput)[0] = pBattleEngine->m45C_perQuadrantDragonSpeed[pBattleEngine->m22C_dragonCurrentQuadrant] / 2;
        if (randomValue == 0)
        {
            (*pOutput)[2] = pBattleEngine->m45C_perQuadrantDragonSpeed[pBattleEngine->m22C_dragonCurrentQuadrant] / 2;
        }
        else
        {
            (*pOutput)[2] = -pBattleEngine->m45C_perQuadrantDragonSpeed[pBattleEngine->m22C_dragonCurrentQuadrant] / 2;
        }
        break;
    case 2:
        (*pOutput)[2] = -pBattleEngine->m45C_perQuadrantDragonSpeed[pBattleEngine->m22C_dragonCurrentQuadrant] / 2;
        if (randomValue == 0)
        {
            (*pOutput)[0] = pBattleEngine->m45C_perQuadrantDragonSpeed[pBattleEngine->m22C_dragonCurrentQuadrant] / 2;
        }
        else
        {
            (*pOutput)[0] = -pBattleEngine->m45C_perQuadrantDragonSpeed[pBattleEngine->m22C_dragonCurrentQuadrant] / 2;
        }
        break;
    case 3:
        (*pOutput)[0] = -pBattleEngine->m45C_perQuadrantDragonSpeed[pBattleEngine->m22C_dragonCurrentQuadrant] / 2;
        if (randomValue == 0)
        {
            (*pOutput)[2] = pBattleEngine->m45C_perQuadrantDragonSpeed[pBattleEngine->m22C_dragonCurrentQuadrant] / 2;
        }
        else
        {
            (*pOutput)[2] = -pBattleEngine->m45C_perQuadrantDragonSpeed[pBattleEngine->m22C_dragonCurrentQuadrant] / 2;
        }
        break;
    default:
        assert(0);
    }
}

void attackCamera2(sVec3_FP* pOutput)
{
    s_battleEngine* pBattleEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;

    pOutput->zeroize();

    int randomValue = randomNumber() & 1;

    switch (pBattleEngine->m22C_dragonCurrentQuadrant)
    {
    case 0:
        (*pOutput)[2] = -pBattleEngine->m45C_perQuadrantDragonSpeed[pBattleEngine->m22C_dragonCurrentQuadrant];
        if (randomValue == 0)
        {
            (*pOutput)[0] = pBattleEngine->m45C_perQuadrantDragonSpeed[pBattleEngine->m22C_dragonCurrentQuadrant];
        }
        else
        {
            (*pOutput)[0] = -pBattleEngine->m45C_perQuadrantDragonSpeed[pBattleEngine->m22C_dragonCurrentQuadrant];
        }
        break;
    case 1:
        (*pOutput)[0] = -pBattleEngine->m45C_perQuadrantDragonSpeed[pBattleEngine->m22C_dragonCurrentQuadrant];
        if (randomValue == 0)
        {
            (*pOutput)[2] = pBattleEngine->m45C_perQuadrantDragonSpeed[pBattleEngine->m22C_dragonCurrentQuadrant];
        }
        else
        {
            (*pOutput)[2] = -pBattleEngine->m45C_perQuadrantDragonSpeed[pBattleEngine->m22C_dragonCurrentQuadrant];
        }
        break;
    case 2:
        (*pOutput)[2] = pBattleEngine->m45C_perQuadrantDragonSpeed[pBattleEngine->m22C_dragonCurrentQuadrant];
        if (randomValue == 0)
        {
            (*pOutput)[0] = pBattleEngine->m45C_perQuadrantDragonSpeed[pBattleEngine->m22C_dragonCurrentQuadrant];
        }
        else
        {
            (*pOutput)[0] = -pBattleEngine->m45C_perQuadrantDragonSpeed[pBattleEngine->m22C_dragonCurrentQuadrant];
        }
        break;
    case 3:
        (*pOutput)[0] = pBattleEngine->m45C_perQuadrantDragonSpeed[pBattleEngine->m22C_dragonCurrentQuadrant];
        if (randomValue == 0)
        {
            (*pOutput)[2] = pBattleEngine->m45C_perQuadrantDragonSpeed[pBattleEngine->m22C_dragonCurrentQuadrant];
        }
        else
        {
            (*pOutput)[2] = -pBattleEngine->m45C_perQuadrantDragonSpeed[pBattleEngine->m22C_dragonCurrentQuadrant];
        }
        break;
    default:
        assert(0);
    }
}

void attackCamera4(sVec3_FP* pOutput)
{
    s_battleEngine* pBattleEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;

    pOutput->zeroize();

    switch (pBattleEngine->m22C_dragonCurrentQuadrant)
    {
    case 0:
        (*pOutput)[2] = -pBattleEngine->m45C_perQuadrantDragonSpeed[2] - pBattleEngine->m45C_perQuadrantDragonSpeed[pBattleEngine->m22C_dragonCurrentQuadrant];
        break;
    case 1:
        (*pOutput)[0] = -pBattleEngine->m45C_perQuadrantDragonSpeed[3] - pBattleEngine->m45C_perQuadrantDragonSpeed[pBattleEngine->m22C_dragonCurrentQuadrant];
        break;
    case 2:
        (*pOutput)[2] = pBattleEngine->m45C_perQuadrantDragonSpeed[0] + pBattleEngine->m45C_perQuadrantDragonSpeed[pBattleEngine->m22C_dragonCurrentQuadrant];
        break;
    case 3:
        (*pOutput)[0] = pBattleEngine->m45C_perQuadrantDragonSpeed[1] + pBattleEngine->m45C_perQuadrantDragonSpeed[pBattleEngine->m22C_dragonCurrentQuadrant];
        break;
    default:
        assert(0);
    }
}

void attackCamera3(sVec3_FP* pOutput, s32 param2)
{
    s_battleEngine* pBattleEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;

    switch (param2)
    {
    case 0:
        pOutput->zeroize();
        *pOutput = MTH_Mul(0x8000, pBattleEngine->mC_battleCenter - gBattleManager->m10_battleOverlay->m18_dragon->m8_position);
        break;
    case 1:
        *pOutput = pBattleEngine->mC_battleCenter - gBattleManager->m10_battleOverlay->m18_dragon->m8_position;
        break;
    case 2:
        pOutput->zeroize();
        break;
    default:
        assert(0);
    }
}

void attackCamera8(sVec3_FP* pOutput, s32 param2)
{
    s_battleEngine* pBattleEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;

    switch (param2)
    {
    case 1:
        (*pOutput)[1] = gBattleManager->m10_battleOverlay->m18_dragon->m8_position[1] - pBattleEngine->mC_battleCenter[1];
        break;
    case 2:
        (*pOutput)[1] = MTH_Mul(0x8000, gBattleManager->m10_battleOverlay->m18_dragon->m8_position[1] - pBattleEngine->m35C_cameraAltitudeMinMax[1]);
        break;
    case 3:
        (*pOutput)[1] = MTH_Mul(-0x8000, gBattleManager->m10_battleOverlay->m18_dragon->m8_position[1] - pBattleEngine->m35C_cameraAltitudeMinMax[1]);
        break;
    case 4:
    case 5:
        (*pOutput)[1] = pBattleEngine->m35C_cameraAltitudeMinMax[1] - (gBattleManager->m10_battleOverlay->m18_dragon->m8_position[1] + 0x5000);
        break;
    case 6:
    case 7:
    case 8:
        (*pOutput)[1] = 0;
        break;
    default:
        assert(0);
    }
}

void attackCamera5(sEnemyAttackCamera* pThis)
{
    s_battleEngine* pBattleEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    switch (pBattleEngine->m22C_dragonCurrentQuadrant)
    {
    case 0:
        pThis->m4[2] = MTH_Mul(0xC000, pBattleEngine->m45C_perQuadrantDragonSpeed[pBattleEngine->m22C_dragonCurrentQuadrant]);
        break;
    case 1:
        pThis->m4[0] = MTH_Mul(0xC000, pBattleEngine->m45C_perQuadrantDragonSpeed[pBattleEngine->m22C_dragonCurrentQuadrant]);
        break;
    case 2:
        pThis->m4[2] = -MTH_Mul(0xC000, pBattleEngine->m45C_perQuadrantDragonSpeed[pBattleEngine->m22C_dragonCurrentQuadrant]);
        break;
    case 3:
        pThis->m4[0] = -MTH_Mul(0xC000, pBattleEngine->m45C_perQuadrantDragonSpeed[pBattleEngine->m22C_dragonCurrentQuadrant]);
        break;
    default:
        assert(0);
    }

    if ((randomNumber() & 1) == 0)
    {
        pThis->m1C_rotation1Step[1] = 0xb60b6;
    }
    else
    {
        pThis->m1C_rotation1Step[1] = -0xb60b6;
    }

    switch (performModulo2(3, randomNumber()))
    {
    case 0:
        pThis->m1C_rotation1Step[0] = 0x2D82D;
        break;
    case 1:
        pThis->m1C_rotation1Step[0] = -0x2D82D;
        break;
    case 2:
        pThis->m1C_rotation1Step[0] = 0;
        break;
    default:
        assert(0);
        break;
    }

    pThis->m10_rotation1[0] = MTH_Mul(-0x1E0000, pThis->m1C_rotation1Step[0]);
    pThis->m10_rotation1[1] = MTH_Mul(-0x1E0000, pThis->m1C_rotation1Step[1]);
}

void attackCamera7(sEnemyAttackCamera* pThis)
{
    if ((randomNumber() & 1) == 0)
    {
        gBattleManager->m10_battleOverlay->m8_gridTask->m64_cameraRotationTarget[2] = -0x71c71c;
        pThis->m10_rotation1[1] = -0x5555555;
        pThis->m1C_rotation1Step[1] = 0xb60b6;
    }
    else
    {
        gBattleManager->m10_battleOverlay->m8_gridTask->m64_cameraRotationTarget[2] = 0x71c71c;
        pThis->m10_rotation1[1] = 0x5555555;
        pThis->m1C_rotation1Step[1] = -0xb60b6;
    }

    if ((gBattleManager->m10_battleOverlay->m4_battleEngine->mC_battleCenter[1] - gBattleManager->m10_battleOverlay->m18_dragon->m8_position[1]) < 0)
    {
        pThis->m10_rotation1[0] = 0x555555;
        pThis->m1C_rotation1Step[0] = -0x2d82d;
    }
    else
    {
        pThis->m10_rotation1[0] = -0x555555;
        pThis->m1C_rotation1Step[0] = 0x2d82d;
    }

    pThis->m4.zeroize();

    int randomValue = randomNumber() & 1;

    switch (gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant)
    {
    case 0:
        if (randomValue == 0)
        {
            pThis->m4[2] = MTH_Mul(0xC000, gBattleManager->m10_battleOverlay->m4_battleEngine->m45C_perQuadrantDragonSpeed[3]);
        }
        else
        {
            pThis->m4[2] = MTH_Mul(0xC000, gBattleManager->m10_battleOverlay->m4_battleEngine->m45C_perQuadrantDragonSpeed[1]);
        }
        break;
    case 1:
        if (randomValue == 0)
        {
            pThis->m4[0] = MTH_Mul(0xC000, gBattleManager->m10_battleOverlay->m4_battleEngine->m45C_perQuadrantDragonSpeed[0]);
        }
        else
        {
            pThis->m4[0] = MTH_Mul(0xC000, gBattleManager->m10_battleOverlay->m4_battleEngine->m45C_perQuadrantDragonSpeed[2]);
        }
        break;
    case 2:
        if (randomValue == 0)
        {
            pThis->m4[2] = -MTH_Mul(0xC000, gBattleManager->m10_battleOverlay->m4_battleEngine->m45C_perQuadrantDragonSpeed[1]);
        }
        else
        {
            pThis->m4[2] = -MTH_Mul(0xC000, gBattleManager->m10_battleOverlay->m4_battleEngine->m45C_perQuadrantDragonSpeed[3]);
        }
        break;
    case 3:
        if (randomValue == 0)
        {
            pThis->m4[0] = -MTH_Mul(0xC000, gBattleManager->m10_battleOverlay->m4_battleEngine->m45C_perQuadrantDragonSpeed[2]);
        }
        else
        {
            pThis->m4[0] = -MTH_Mul(0xC000, gBattleManager->m10_battleOverlay->m4_battleEngine->m45C_perQuadrantDragonSpeed[0]);
        }
        break;
    default:
        assert(0);
        break;
    }
}

void attackCamera6(sEnemyAttackCamera* pThis)
{
    if ((randomNumber() & 1) == 0)
    {
        gBattleManager->m10_battleOverlay->m8_gridTask->m64_cameraRotationTarget[2] = -0xAAAAAA;
        pThis->m10_rotation1[1] = -0xAAAAAA;
        pThis->m1C_rotation1Step[1] = 0xb60b6;
    }
    else
    {
        gBattleManager->m10_battleOverlay->m8_gridTask->m64_cameraRotationTarget[2] = 0xAAAAAA;
        pThis->m10_rotation1[1] = 0xAAAAAA;
        pThis->m1C_rotation1Step[1] = -0xb60b6;
    }

    if ((gBattleManager->m10_battleOverlay->m4_battleEngine->mC_battleCenter[1] - gBattleManager->m10_battleOverlay->m18_dragon->m8_position[1]) < 0)
    {
        pThis->m10_rotation1[0] = 0x38E38E;
        pThis->m1C_rotation1Step[0] = -0x5b05b;
    }
    else
    {
        pThis->m10_rotation1[0] = -0x38E38E;
        pThis->m1C_rotation1Step[0] = 0x5b05b;
    }

    pThis->m4.zeroize();

    switch (gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant)
    {
    case 0:
        pThis->m4[2] = 0xC000;
        break;
    case 1:
        pThis->m4[0] = 0xC000;
        break;
    case 2:
        pThis->m4[2] = -0xC000;
        break;
    case 3:
        pThis->m4[0] = -0xC000;
        break;
    default:
        assert(0);
        break;
    }
}

void sEnemyAttackCamera_init(sEnemyAttackCamera* pThis)
{
    battleEngine_enableAttackCamera();
    pThis->m4.zeroize();
    pThis->m34.zeroize();
    pThis->m10_rotation1.zeroize();
    pThis->m40_rotation2.zeroize();
    pThis->m1C_rotation1Step.zeroize();
    pThis->m4C_rotation2Step.zeroize();
    pThis->m1_cameraIndex = gBattleManager->m10_battleOverlay->m4_battleEngine->m433_attackCameraIndex;
    pThis->m58 = 0;

    switch (pThis->m1_cameraIndex)
    {
    case 0:
        battleEngine_restoreCameraDefault();
        battleEngine_UpdateSub7Sub2();
        pThis->getTask()->markFinished();
        return;
    case 1:
        attackCamera1(&pThis->m4);
        attackCamera3(&pThis->m34, 0);
        break;
    case 2:
    case 3:
    case 5:
        attackCamera2(&pThis->m4);
        attackCamera3(&pThis->m34, 0);
    case 4:
        attackCamera4(&pThis->m4);
        attackCamera3(&pThis->m34, 0);
        break;
    case 6:
        attackCamera5(pThis);
        attackCamera3(&pThis->m34, 2);
        break;
    case 7:
        attackCamera7(pThis);
        attackCamera3(&pThis->m28, 1);
        break;
    case 8:
        attackCamera6(pThis);
        attackCamera3(&pThis->m28, 1);
        break;
    default:
        assert(0);
    }

    attackCamera8(&pThis->m4, pThis->m1_cameraIndex);

}

void sEnemyAttackCamera_updateSub1(int param_1)
{
    s_battleGrid* pGrid = gBattleManager->m10_battleOverlay->m8_gridTask;

    switch (param_1)
    {
    case 0:
        pGrid->mE4_currentCameraReferenceCenter = MTH_Mul(0x8000, pGrid->mE4_currentCameraReferenceCenter - pGrid->m134_desiredCameraPosition);
        pGrid->mF0_currentCameraReferenceForward = MTH_Mul(0x8000, pGrid->mF0_currentCameraReferenceForward - pGrid->m140_desiredCameraTarget);
        break;
    case 1:
        pGrid->mE4_currentCameraReferenceCenter = MTH_Mul(0x8000, pGrid->m134_desiredCameraPosition - pGrid->mE4_currentCameraReferenceCenter);
        pGrid->mF0_currentCameraReferenceForward = MTH_Mul(0x8000, pGrid->m140_desiredCameraTarget - pGrid->mF0_currentCameraReferenceForward);
        break;
    default:
        assert(0);
        break;
    }

    pGrid->m108_deltaCameraPosition.zeroize();
    pGrid->m114_deltaCameraTarget.zeroize();
}

void battleEngineSub1_UpdateSub2(sVec3_FP* pOutput, const sVec3_FP& translation, const sVec3_FP& inputVector, const sVec3_FP& rotation)
{
    sVec3_FP temp;

    pushCurrentMatrix();
    translateCurrentMatrix(translation);
    rotateCurrentMatrixYXZ(&rotation);
    transformAndAddVecByCurrentMatrix(&inputVector, &temp);
    transformAndAddVec(temp, *pOutput, cameraProperties2.m28[0]);
    popMatrix();

    if (isTraceEnabled())
    {
        addTraceLog(translation, "battleEngineSub1_UpdateSub2 translation");
        addTraceLog(inputVector, "battleEngineSub1_UpdateSub2 inputVector");
        addTraceLog(rotation, "battleEngineSub1_UpdateSub2 rotation");
        addTraceLog(*pOutput, "battleEngineSub1_UpdateSub2 output");
    }
}

void battleEngine_restoreCameraAfterEnemyAttack()
{
    gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m800 = 0;
    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0xB] == 0)
    {
        gBattleManager->m10_battleOverlay->m8_gridTask->m34_cameraPosition =
            gBattleManager->m10_battleOverlay->m4_battleEngine->m104_dragonPosition
            + gBattleManager->m10_battleOverlay->m8_gridTask->m1C
            + gBattleManager->m10_battleOverlay->m8_gridTask->m28;

        battleEngine_setCurrentCameraPositionPointer(&gBattleManager->m10_battleOverlay->m8_gridTask->m34_cameraPosition);
        gBattleManager->m10_battleOverlay->m4_battleEngine->m3D8_pDesiredCameraPosition = &gBattleManager->m10_battleOverlay->m4_battleEngine->mC_battleCenter;
        battleEngine_resetCameraInterpolation();
        battleEngine_restoreCameraDefaultSub0();
    }
}

void sEnemyAttackCamera_update(sEnemyAttackCamera* pThis)
{
    pThis->m58++;
    if (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m1000)
    {
        pThis->m10_rotation1 += pThis->m1C_rotation1Step;
        pThis->m40_rotation2 += pThis->m4C_rotation2Step;
        switch (pThis->m1_cameraIndex)
        {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            battleEngineSub1_UpdateSub2(&gBattleManager->m10_battleOverlay->m4_battleEngine->m418, gBattleManager->m10_battleOverlay->m18_dragon->m8_position, pThis->m4, pThis->m10_rotation1);
            battleEngineSub1_UpdateSub2(&gBattleManager->m10_battleOverlay->m4_battleEngine->m424, gBattleManager->m10_battleOverlay->m18_dragon->m8_position, pThis->m34, pThis->m40_rotation2);
            break;
        case 7:
            if (pThis->m58 < 0x1E)
            {
                pThis->m34 = pThis->m28;
            }
            else
            {
                if (pThis->m58 < 0x5A)
                {
                    pThis->m34 = MTH_Mul(FP_Div(0x54 - pThis->m58, 0x3C), pThis->m28);
                }
                else
                {
                    pThis->m34.zeroize();
                }
            }

            if (pThis->m10_rotation1[0] > 0x1555555)
            {
                pThis->m1C_rotation1Step[0] = 0;
            }

            if (pThis->m10_rotation1[0] < -0x1555555)
            {
                pThis->m1C_rotation1Step[0] = 0;
            }

            battleEngineSub1_UpdateSub2(&gBattleManager->m10_battleOverlay->m4_battleEngine->m418, gBattleManager->m10_battleOverlay->m4_battleEngine->mC_battleCenter, pThis->m4, pThis->m10_rotation1);
            battleEngineSub1_UpdateSub2(&gBattleManager->m10_battleOverlay->m4_battleEngine->m424, gBattleManager->m10_battleOverlay->m18_dragon->m8_position, pThis->m34, pThis->m40_rotation2);
            break;
        case 8:
            if (pThis->m58 < 0x1E)
            {
                pThis->m34 = pThis->m28;
            }
            else
            {
                if (pThis->m58 < 0x5A)
                {
                    pThis->m34 = MTH_Mul(FP_Div(0x54 - pThis->m58, 0x3C), pThis->m28);
                }
                else
                {
                    pThis->m34.zeroize();
                }
            }

            if (pThis->m10_rotation1[0] > 0x1555555)
            {
                pThis->m1C_rotation1Step[0] = 0;
            }

            if (pThis->m10_rotation1[0] < -0x1555555)
            {
                pThis->m1C_rotation1Step[0] = 0;
            }

            battleEngineSub1_UpdateSub2(&gBattleManager->m10_battleOverlay->m4_battleEngine->m418, gBattleManager->m10_battleOverlay->m18_dragon->m8_position, pThis->m4, pThis->m10_rotation1);
            battleEngineSub1_UpdateSub2(&gBattleManager->m10_battleOverlay->m4_battleEngine->m424, gBattleManager->m10_battleOverlay->m18_dragon->m8_position, pThis->m34, pThis->m40_rotation2);
            break;
        default:
            assert(0);
            break;
        }

        if (pThis->m58 == 1)
        {
            battleEngine_setCurrentCameraPositionPointer(&gBattleManager->m10_battleOverlay->m4_battleEngine->m418);
            battleEngine_setDesiredCameraPositionPointer(&gBattleManager->m10_battleOverlay->m4_battleEngine->m424);
            sEnemyAttackCamera_updateSub0(1);
            sEnemyAttackCamera_updateSub1(0);
            battleEngine_resetCameraInterpolation();
        }
    }
    else
    {
        battleEngine_UpdateSub7Sub2();
        sEnemyAttackCamera_updateSub1(1);
        battleEngine_restoreCameraAfterEnemyAttack();
        pThis->getTask()->markFinished();
    }
}

void battleEngine_updateBattleMode_8_playAttackCamera(s_battleEngine* pThis)
{
    switch (pThis->m38D_battleSubMode)
    {
    case 0:
        battleCreateCinematicBars(pThis);
        pThis->m38D_battleSubMode++;

        {
            static const sEnemyAttackCamera::TypedTaskDefinition definition = {
                 &sEnemyAttackCamera_init,
                 &sEnemyAttackCamera_update,
                 nullptr,
                 nullptr,
            };

            createSubTask<sEnemyAttackCamera>(pThis, &definition);
        }
        return;
    case 1:
        if (++pThis->m384_battleModeDelay > 4)
        {
            pThis->m384_battleModeDelay = 0;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2000 = 1;
            pThis->m38D_battleSubMode++;
        }
        break;
    case 2:
        if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished)
        {
            pThis->m38D_battleSubMode++;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m40 = 1;
        }
        break;
    case 3:
        if (++pThis->m384_battleModeDelay > 0xF)
        {
            pThis->m384_battleModeDelay = 0;
            pThis->m38D_battleSubMode++;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m1000 = 1;
        }
        break;
    case 4:
        if (++pThis->m384_battleModeDelay > 0xF)
        {
            pThis->m384_battleModeDelay = 0;
            pThis->m38D_battleSubMode++;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m1000 = 0;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m20_battleIntroRunning = 0;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2_needToSortEnemiesByDistanceFromDragon = 1;
        }
        break;
    case 5:
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m10 = 0;
        pThis->m38D_battleSubMode++;
        break;
    case 6:
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m80000_hideBattleHUD = 0;
        pThis->m38D_battleSubMode++;
        break;
    case 7:
        if (pThis->m384_battleModeDelay++ < 5)
            break;
        battleEngine_SetBattleMode16();
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m4000 = 0;
        break;
    default:
        assert(0);
        break;
    }
}

void battleEngine_updateBattleMode_E_battleIntro(s_battleEngine* pThis)
{
    switch (pThis->m38D_battleSubMode)
    {
    case 0:
        battleCreateCinematicBars(pThis);
        pThis->m3D0 = 0;
        pThis->m38D_battleSubMode++;
        return;
    case 1:
        if (4 < pThis->m384_battleModeDelay++)
        {
            pThis->m384_battleModeDelay = 0;
            pThis->m38D_battleSubMode++;
            pThis->m3D0 = createBattleIntroTask(pThis);

            g_fadeControls.m_4D = 6;
            if (g_fadeControls.m_4C < g_fadeControls.m_4D)
            {
                vdp2Controls.m20_registers[0].m112_CLOFSL = 0;
                vdp2Controls.m20_registers[1].m112_CLOFSL = 0;
            }

            fadePalette(&g_fadeControls.m0_fade0, 0xFFFF, 0xC210, 0x5A);
            g_fadeControls.m_4D = 5;
        }
        return;
    case 2:
        if (pThis->m3D0 == nullptr)
        {
            pThis->m384_battleModeDelay = 0;
            pThis->m38D_battleSubMode++;

            g_fadeControls.m_4D = 6;
            if (g_fadeControls.m_4C < g_fadeControls.m_4D)
            {
                vdp2Controls.m20_registers[0].m112_CLOFSL = 0;
                vdp2Controls.m20_registers[1].m112_CLOFSL = 0;
            }

            fadePalette(&g_fadeControls.m24_fade1, 0xC210, 0xC210, 1);
            fadePalette(&g_fadeControls.m0_fade0, 0xC210, 0xC210, 1);
            g_fadeControls.m_4D = 5;

            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m20_battleIntroRunning = 0;
        }
        return;
    case 3:
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m80000_hideBattleHUD = 0;
        pThis->m38D_battleSubMode++;
        return;
    case 4:
        if (pThis->m384_battleModeDelay++ > 4)
        {
            battleEngine_SetBattleMode16();
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m4000 = 0;
        }
        return;
    default:
        assert(0);
        break;
    }
}

void battleEngine_updateBattleMode_7_sub0()
{
    s_battleEngine* pBattleEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;

    if (pBattleEngine->m432 & 4)
    {
        battleEngineSub1_UpdateSub2(&pBattleEngine->m3F4_cameraPositionWhileShooting, *pBattleEngine->m3D8_pDesiredCameraPosition, pBattleEngine->m3E8, pBattleEngine->m3DC);
        return;
    }
    if ((pBattleEngine->m432 & 1) == 0)
    {
        if ((pBattleEngine->m432 & 2) == 0)
        {
            pBattleEngine->m3DC[1] += 0xb60b6;
            battleEngineSub1_UpdateSub2(&pBattleEngine->m3F4_cameraPositionWhileShooting, *pBattleEngine->m3D8_pDesiredCameraPosition, pBattleEngine->m3E8, pBattleEngine->m3DC);
            return;
        }
    }
    else
    {
        if (pBattleEngine->m430 > 0)
        {
            pBattleEngine->m430--;
            pBattleEngine->m3DC[1] += 0xb60b6;
            battleEngineSub1_UpdateSub2(&pBattleEngine->m3F4_cameraPositionWhileShooting, *pBattleEngine->m3D8_pDesiredCameraPosition, pBattleEngine->m3E8, pBattleEngine->m3DC);
            return;
        }
    }

    battleEngine_enableAttackCamera();
    pBattleEngine->m3E8.zeroize();
    pBattleEngine->m3DC.zeroize();

    pBattleEngine->m3E8[2] = 0x1000;

    static const std::array<s32, 4> table = {
        0x0,     0x4000000,     0x8000000,     0xC000000
    };

    pBattleEngine->m3DC[1] = table[pBattleEngine->m22C_dragonCurrentQuadrant];

    battleEngineSub1_UpdateSub2(&pBattleEngine->m3F4_cameraPositionWhileShooting, gBattleManager->m10_battleOverlay->m18_dragon->m8_position, pBattleEngine->m3E8, pBattleEngine->m3DC);
    battleEngine_setCurrentCameraPositionPointer(&pBattleEngine->m3F4_cameraPositionWhileShooting);
    battleEngine_setDesiredCameraPositionPointer(&gBattleManager->m10_battleOverlay->m18_dragon->m8_position);
    battleEngine_resetCameraInterpolation();
    pBattleEngine->m432 = 4;
    pBattleEngine->m430 = 0;
}

void battleEngine_updateBattleMode_7(s_battleEngine* pThis)
{
    switch (pThis->m38D_battleSubMode)
    {
    case 0:
        battleCreateCinematicBars(pThis);
        pThis->m38D_battleSubMode++;
        battleEngine_updateBattleMode_7_sub0();
        break;
    case 1:
        battleEngine_updateBattleMode_7_sub0();
        if (++pThis->m384_battleModeDelay > 4)
        {
            pThis->m384_battleModeDelay = 0;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2000 = 1;
            pThis->m38D_battleSubMode++;
        }
        break;
    case 2:
        battleEngine_updateBattleMode_7_sub0();
        if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished)
        {
            pThis->m38D_battleSubMode++;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m40 = 1;
        }
        break;
    case 3:
        battleEngine_updateBattleMode_7_sub0();
        if (++pThis->m384_battleModeDelay > 0xf)
        {
            pThis->m384_battleModeDelay = 0;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2000000 = 1;
            pThis->m38D_battleSubMode++;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m1000 = 1;
        }
        break;
    case 4:
        if (++pThis->m384_battleModeDelay > 0xf)
        {
            pThis->m384_battleModeDelay = 0;
            pThis->m38D_battleSubMode++;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m1000 = 0;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m20_battleIntroRunning = 0;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2_needToSortEnemiesByDistanceFromDragon = 1;
        }
        break;
    case 5:
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m10 = 0;
        pThis->m38D_battleSubMode++;
        break;
    case 6:
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m80000_hideBattleHUD = 0;
        pThis->m38D_battleSubMode++;
        break;
    case 7:
        if (++pThis->m384_battleModeDelay > 5)
        {
            battleEngine_SetBattleMode16();
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m4000 = 0;
        }
        break;
    default:
        assert(0);
    }
}

void increaseStatsCount(int statIndex)
{
    switch (statIndex)
    {
    case 1:
        mainGameState.gameStats.m5C_gunShotFired++;
        break;
    case 2:
        mainGameState.gameStats.m60_homingLaserFired++;
        break;
    default:
        assert(0);
    }
}

void battleEngine_updateBattleMode_0_shootEnemyWithGun(s_battleEngine* pThis)
{
    switch (pThis->m38D_battleSubMode)
    {
    case 0:
        battleEngine_enableAttackCamera();
        pThis->m40C_gunTarget = pThis->mC_battleCenter;
        battleEngine_setDesiredCameraPositionPointer(&pThis->m40C_gunTarget);
        if (mainGameState.gameStats.mA_weaponType == 0x40)
        {
            pThis->m38D_battleSubMode = 2;
        }
        else
        {
            pThis->m38D_battleSubMode = 1;
        }
        gBattleManager->m10_battleOverlay->m8_gridTask->m134_desiredCameraPosition[1] = 0x5000;
        gBattleManager->m10_battleOverlay->m8_gridTask->m134_desiredCameraPosition[2] = 0;
        gBattleManager->m10_battleOverlay->m8_gridTask->m140_desiredCameraTarget[1] = 0;
        playSystemSoundEffect(7);
        pThis->m188_flags.m2_needToSortEnemiesByDistanceFromDragon = 1;
        break;
    case 1: // select enemy phase
        if (gBattleManager->m10_battleOverlay->mC_targetSystem->m20A_numSelectableEnemies > 0)
        {
            pThis->m39C_maxSelectableEnemies = gBattleManager->m10_battleOverlay->mC_targetSystem->m20A_numSelectableEnemies;
            if (gBattleManager->m10_battleOverlay->mC_targetSystem->m20A_numSelectableEnemies - 1 < pThis->m398_currentSelectedEnemy)
            {
                pThis->m398_currentSelectedEnemy = gBattleManager->m10_battleOverlay->mC_targetSystem->m20A_numSelectableEnemies - 1;
            }

            // rotate selected enemy
            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.mC_newButtonDown2 & 0x80)
            {
                pThis->m398_currentSelectedEnemy++;
                if (pThis->m39C_maxSelectableEnemies <= pThis->m398_currentSelectedEnemy)
                {
                    pThis->m398_currentSelectedEnemy -= pThis->m39C_maxSelectableEnemies;
                }
                playSystemSoundEffect(2);
            }
            else if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.mC_newButtonDown2 & 0x40)
            {
                pThis->m398_currentSelectedEnemy--;
                if (pThis->m398_currentSelectedEnemy < 0)
                {
                    pThis->m398_currentSelectedEnemy += pThis->m39C_maxSelectableEnemies;
                }
                playSystemSoundEffect(2);
            }

            // mark all enemies are deselected
            int iVar2 = gBattleManager->m10_battleOverlay->mC_targetSystem->m20A_numSelectableEnemies - 1;
            while (iVar2 > -1)
            {
                sBattleTargetable* pEntry = gBattleManager->m10_battleOverlay->mC_targetSystem->m0_enemyTargetables[iVar2]->m4_targetable;
                pEntry->m50_flags &= ~0x200000;
                iVar2--;
            }
            
            // flag current enemy selected
            gBattleManager->m10_battleOverlay->mC_targetSystem->m0_enemyTargetables[pThis->m398_currentSelectedEnemy]->m4_targetable->m50_flags |= 0x200000;

            pThis->m40C_gunTarget = *getBattleTargetablePosition(*gBattleManager->m10_battleOverlay->mC_targetSystem->m0_enemyTargetables[pThis->m398_currentSelectedEnemy]->m4_targetable) + gBattleManager->m10_battleOverlay->m4_battleEngine->m1A0_battleAutoScrollDelta;
            gBattleManager->m10_battleOverlay->m8_gridTask->m140_desiredCameraTarget[2] = -vecDistance(*pThis->m3D4_pCurrentCameraPosition, *pThis->m3D8_pDesiredCameraPosition);

            if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 6) == 0)
            {
                if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 1) == 0) {
                    return;
                }
                if (pThis->m388 & 0x100)
                    return;

                // validate enemy selection
                for (int i = 0; i < gBattleManager->m10_battleOverlay->mC_targetSystem->m20A_numSelectableEnemies; i++)
                {
                    gBattleManager->m10_battleOverlay->mC_targetSystem->m0_enemyTargetables[i]->m4_targetable->m50_flags &= ~0x200000;
                }

                battleEngine_UpdateSub7Sub2();
                gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m4000000 = 1;
                battleEngine_SetBattleMode16();
                playSystemSoundEffect(1);
                gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m4000 = 1;
                return;
            }

            // cancel out of enemy selection
            if (pThis->m388 & 0x80)
            {
                return;
            }

            pThis->m38D_battleSubMode = 3;
            pThis->m384_battleModeDelay = 0;
            break;
        }
    case 3: // consume energy bar
        if (mainGameState.gameStats.mA_weaponType == 0x40)
        {
            pThis->m40C_gunTarget = pThis->mC_battleCenter;
        }
        else
        {
            pThis->m40C_gunTarget = *getBattleTargetablePosition(*gBattleManager->m10_battleOverlay->mC_targetSystem->m0_enemyTargetables[pThis->m398_currentSelectedEnemy]->m4_targetable) + gBattleManager->m10_battleOverlay->m4_battleEngine->m1A0_battleAutoScrollDelta;
        }
        gBattleManager->m10_battleOverlay->m8_gridTask->m140_desiredCameraTarget[2] = -vecDistance(*pThis->m3D4_pCurrentCameraPosition, *pThis->m3D8_pDesiredCameraPosition);

        battleCreateCinematicBars(pThis);
        pThis->m38D_battleSubMode++;
        if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x15] == 0)
        {
            pThis->m3B4.m16_combo--; // consume one energy bar
        }

        increaseStatsCount(1);
        break;
    case 4: // delay before shot (4 frames)
        if (mainGameState.gameStats.mA_weaponType == 0x40)
        {
            pThis->m40C_gunTarget = pThis->mC_battleCenter;
        }
        else
        {
            pThis->m40C_gunTarget = *getBattleTargetablePosition(*gBattleManager->m10_battleOverlay->mC_targetSystem->m0_enemyTargetables[pThis->m398_currentSelectedEnemy]->m4_targetable) + gBattleManager->m10_battleOverlay->m4_battleEngine->m1A0_battleAutoScrollDelta;
        }
        gBattleManager->m10_battleOverlay->m8_gridTask->m140_desiredCameraTarget[2] = -vecDistance(*pThis->m3D4_pCurrentCameraPosition, *pThis->m3D8_pDesiredCameraPosition);
        if (pThis->m384_battleModeDelay++ > 4)
        {
            pThis->m384_battleModeDelay = 0;
            pThis->m38D_battleSubMode++;
            pThis->m40C_gunTarget = *getBattleTargetablePosition(*gBattleManager->m10_battleOverlay->mC_targetSystem->m0_enemyTargetables[pThis->m398_currentSelectedEnemy]->m4_targetable) + gBattleManager->m10_battleOverlay->m4_battleEngine->m1A0_battleAutoScrollDelta;
            pThis->m3E8 = pThis->m40C_gunTarget - pThis->mC_battleCenter;
            battleEngine_setDesiredCameraPositionPointer(&pThis->m40C_gunTarget);
        }
        break;
    case 5: // init shot proper
        pThis->m40C_gunTarget = pThis->mC_battleCenter + pThis->m3E8;
        createGunShotRootTask(pThis);
        pThis->m384_battleModeDelay = 0;
        pThis->m38D_battleSubMode++;
        break;
    case 6: // wait for shot end
        pThis->m40C_gunTarget = pThis->mC_battleCenter + pThis->m3E8;
        if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished)
        {
            battleEngine_UpdateSub7Sub2();
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m4000000 = 1;
            gBattleManager->m10_battleOverlay->m8_gridTask->m1BC_cameraRotationStep.zeroize();
            pThis->m384_battleModeDelay = 0;
            pThis->m38D_battleSubMode++;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m1000 = 1;
        }
        break;
    case 7: // post-shot delay
    {
        s16 cnt = pThis->m384_battleModeDelay++;
        if (cnt > 0xf)
        {
            pThis->m384_battleModeDelay = 0;
            pThis->m38D_battleSubMode++;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m1000 = 0;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m20_battleIntroRunning = 0;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2_needToSortEnemiesByDistanceFromDragon = 1;
        }
        break;
    }
    case 8:
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m10 = 0;
        pThis->m38D_battleSubMode++;
        break;
    case 9:
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m80000_hideBattleHUD = 0;
        pThis->m38D_battleSubMode++;
        break;
    case 10:
        if (pThis->m384_battleModeDelay++ > 4)
        {
            battleEngine_SetBattleMode16();
        }
        break;
    default:
        assert(0);
    }
}

struct sItemResourceTask : public s_workAreaTemplateWithCopy<sItemResourceTask>
{
    char m_data[0x128];
    // size: 0x128
};

// BTL_A3::06091940
static void itemResourceTask_draw(sItemResourceTask* pThis)
{
    // no-op
}

// BTL_A3::06091944
static void itemResourceTask_delete(sItemResourceTask* pThis)
{
    dramFree((u8*)gBattleManager->m10_battleOverlay->m4_battleEngine->mA9C);
    vdp1Free((u8*)gBattleManager->m10_battleOverlay->m4_battleEngine->mAA0);
}

static const sItemResourceTask::TypedTaskDefinition itemResourceTaskDefinition = {
    nullptr,
    nullptr,
    itemResourceTask_draw,
    itemResourceTask_delete,
};

// BTL_A3::06067698
static void useItemApplyEffect(s_battleEngine* pThis)
{
    Unimplemented();
}

// BTL_A3::06064272
static void battleEngine_restoreCameraPostItem()
{
    s_battleGrid* pGrid = gBattleManager->m10_battleOverlay->m8_gridTask;
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    pGrid->mE4_currentCameraReferenceCenter = pGrid->m134_desiredCameraPosition;
    pGrid->mF0_currentCameraReferenceForward = pGrid->m140_desiredCameraTarget;
    pGrid->m108_deltaCameraPosition = {};
    pGrid->m114_deltaCameraTarget = {};
    if (pEngine->m3D0 != nullptr)
    {
        pEngine->m3D0->getTask()->markFinished();
        pEngine->m3D0 = nullptr;
    }
}

// BTL_A3::06069dc0
static void battleEngine_showReceiveItemText(eItems itemId)
{
    sBattleTextDisplayTask* pDisplay = gBattleManager->m10_battleOverlay->m14_textDisplay;
    if (pDisplay != nullptr)
    {
        pDisplay->m14 = (s16)0xffe2;
        createReceiveItemTask(pDisplay, (s_receivedItemTask**)&pDisplay->m8, (s32)pDisplay->m14, itemId, 0);
    }
}

static void findAndLoadTownFile(const char* filename, void* dstDram, u16 vdp1Offset)
{
    Unimplemented();
}

static void useItemSub0(s8 param1, u32 param2)
{
    Unimplemented();
}

// 0601e0b4
static void FUN_0601e0b4()
{
    Unimplemented();
}

// BTL_A3::06066c54
static void FUN_BTL_A3__06066c54(s_battleEngine* pThis)
{
    Unimplemented();
}

// BTL_A3::06091720
p_workArea loadItemResources(s_battleEngine* pThis, eItems itemId)
{
    sItemResourceTask* psVar1 = createSubTaskWithCopy<sItemResourceTask>(pThis, &itemResourceTaskDefinition);
    if (psVar1 == nullptr)
        return nullptr;

    pThis->mA9C = nullptr;
    pThis->mAA0 = nullptr;

    s16 category;
    s16 itemId16 = (s16)itemId;

    if (itemId16 == 1 || itemId16 == 0xa1)
    {
        // dragonY > envHeight+0x23000 → category 0, else category 1
        if (*(s32*)((u8*)gBattleManager->m10_battleOverlay->mC_targetSystem + 0x204) + 0x23000 <
            pThis->mC_battleCenter.m4_Y)
            category = 0;
        else
            category = 1;
    }
    else if (itemId16 == 2 || itemId16 == 3 || itemId16 == 0x95 || itemId16 == 0x96)
    {
        if (pThis->mC_battleCenter.m4_Y <=
            *(s32*)((u8*)gBattleManager->m10_battleOverlay->mC_targetSystem + 0x204) + 0x23000)
            category = 3;
        else
            category = 2;
    }
    else if (itemId16 == 0x2d || itemId16 == 0x2e || itemId16 == 0x2f ||
             itemId16 == 0x34 || itemId16 == 0x35 || itemId16 == 0x36 ||
             itemId16 == 0x37 || itemId16 == 0xad)
    {
        category = 5;
    }
    else if (itemId16 == 0x91 || itemId16 == 0x92 || itemId16 == 0x93)
    {
        category = 4;
    }
    else if (itemId16 == 0x97)
    {
        category = 9;
    }
    else if (itemId16 == 0x98)
    {
        category = 10;
    }
    else if (itemId16 == 0x9b)
    {
        category = 7;
    }
    else if (itemId16 == 0xa2)
    {
        category = 8;
    }
    else
    {
        // unsupported item — mark task finished, return null
        if (psVar1 != nullptr)
            psVar1->getTask()->markFinished();
        return nullptr;
    }

    // Write category into psVar1 work area at offset 0xEA (= task offset 0x1d*8+2 in Ghidra notation)
    *(s16*)((char*)psVar1 + 0xEA) = category;

    // Allocate dram and vdp1 buffers
    pThis->mA9C = (p_workArea)dramAllocate(0xe000);
    pThis->mAA0 = (p_workArea)vdp1Allocate(0xe000);

    Unimplemented(); // findAndLoadTownFile calls (category-indexed MCB/CGB file table at 060ad5e8)

    return psVar1;
}

void battleEngine_updateBattleMode_1_useItem(s_battleEngine* pThis)
{
    switch (pThis->m38D_battleSubMode)
    {
    case 0:
        battleCreateCinematicBars(pThis);
        pThis->m38D_battleSubMode++;
        pThis->mAA4 = loadItemResources(pThis, pThis->m39E_selectedItem);
        break;
    case 1:
    {
        s16 cnt = pThis->m384_battleModeDelay++;
        if (cnt < 5)
            break;
        pThis->m384_battleModeDelay = 0;
        gBattleManager->m10_battleOverlay->m18_dragon->m88 |= 0x2000;
        u8 itemCategory = getObjectListEntry(pThis->m39E_selectedItem)->m1_type;
        if (itemCategory == 6)
        {
            pThis->m38D_battleSubMode++;
        }
        else if (itemCategory == 7 || itemCategory == 8)
        {
            pThis->m38D_battleSubMode++;
            u32 rnd = randomNumber();
            useItemSub0('\x01', (rnd & 1) == 0 ? 2 : 1);
        }
        else
        {
            resetProjectVector();
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m20000 = 0;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m80000_hideBattleHUD = 0;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m4000000 = 1;
            pThis->m38D_battleSubMode = 8;
        }
        battleEngine_showReceiveItemText(pThis->m39E_selectedItem);
        break;
    }
    case 2:
        if (pThis->m390++ < 0x1f)
            break;
        useItemApplyEffect(pThis);
        pThis->m390 = 0;
        pThis->m38D_battleSubMode++;
        break;
    case 3:
        if (!(gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished))
            break;
        pThis->m390 = 0;
        pThis->m38D_battleSubMode++;
        break;
    case 4:
        if (pThis->m390++ < 0xb)
            break;
        pThis->m390 = 0;
        pThis->m38D_battleSubMode++;
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m1000 = 1;
        if (pThis->mAA4 != nullptr)
            pThis->mAA4->getTask()->markFinished();
        pThis->mAA4 = nullptr;
        break;
    case 5:
        if (pThis->m390++ < 0x29)
            break;
        resetProjectVector();
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m200000 = 0;
        battleEngine_restoreCameraPostItem();
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2000000 = 1;
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m1000 = 0;
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m20_battleIntroRunning = 0;
        pThis->m390 = 0;
        pThis->m38D_battleSubMode++;
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2_needToSortEnemiesByDistanceFromDragon = 1;
        break;
    case 6:
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m10 = 0;
        pThis->m38D_battleSubMode++;
        break;
    case 7:
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m80000_hideBattleHUD = 0;
        pThis->m38D_battleSubMode++;
        break;
    case 8:
        if (pThis->m390++ < 5)
            break;
        battleEngine_SetBattleMode16();
        break;
    default:
        assert(0);
    }
}

// BTL_A3::06067180
void battleEngine_updateBattleMode_4_useBerserk(s_battleEngine* pThis)
{
    s_battleDragon* pDragon = gBattleManager->m10_battleOverlay->m18_dragon;
    s_battleGrid* pGrid = gBattleManager->m10_battleOverlay->m8_gridTask;
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    switch (pThis->m38D_battleSubMode)
    {
    case 0:
        battleCreateCinematicBars(pThis);
        pThis->mAA4 = loadItemResources(pThis, pThis->m3A2_selectedBerserk);
        if ((pThis->m3A2_selectedBerserk == 0x99) || (pThis->m3A2_selectedBerserk == 0xA0))
            pEngine->m188_flags.m1 = 1;
        pThis->m38D_battleSubMode++;
        break;
    case 1:
        if (pThis->m384_battleModeDelay++ < 5)
            break;
        {
            s8 bVar1 = getObjectListEntry(pThis->m3A2_selectedBerserk)->m1_type;
            if (bVar1 == 0) {
                pDragon->m88 |= 2;
                useItemSub0(0, 4);
            } else {
                if (bVar1 != 1 && bVar1 != 2) {
                    pEngine->m188_flags.m80000_hideBattleHUD = 0;
                    pThis->m384_battleModeDelay = 0;
                    pThis->m38D_battleSubMode = 9;
                    break;
                }
                pDragon->m88 |= 4;
                useItemSub0(1, 4);
            }
            pThis->m384_battleModeDelay = 0;
            pThis->m38D_battleSubMode++;
            battleEngine_showReceiveItemText(pThis->m3A2_selectedBerserk);
        }
        break;
    case 2:
        {
            s8 bVar1 = getObjectListEntry(pThis->m3A2_selectedBerserk)->m1_type;
            if (bVar1 == 0) {
                if (pDragon->m1CC_currentAnimation != 0x10)
                    return;
                pEngine->m188_flags.m20000 = 1;
                FUN_0601e0b4();
                sVec3_FP sVar3;
                battleResultScreen_updateSub0('n', 'n', 'n', &sVar3);
                battleGrid_setupLightInterpolation(0x26, pGrid->m1CC_lightColor, sVar3);
                sVec3_FP sVar4;
                battleResultScreen_updateSub0('\b', '\b', '\b', &sVar4);
                battleGrid_setupLightInterpolation2(0x26, pGrid->m1E4_lightFalloff0, sVar4);
                sVec3_FP local_24;
                transformAndAddVecByCurrentMatrix(&pDragon->mFC_hotpoints[2], &local_24);
                return;
            } else if (bVar1 == 1 || bVar1 == 2) {
                if (pDragon->m1CC_currentAnimation != 0x11)
                    return;
                pThis->m384_battleModeDelay = 0;
                pThis->m38D_battleSubMode++;
                return;
            }
            pEngine->m188_flags.m80000_hideBattleHUD = 0;
            pThis->m384_battleModeDelay = 0;
            pThis->m38D_battleSubMode = 9;
        }
        break;
    case 3:
        FUN_BTL_A3__06066c54(pThis);
        if (pEngine->m188_flags.m20000) {
            sVec3_FP local_24;
            transformAndAddVecByCurrentMatrix(&pDragon->mFC_hotpoints[2], &local_24);
            return;
        }
        break;
    case 4:
        if (pEngine->m188_flags.m100_attackAnimationFinished) {
            if (pEngine->m188_flags.m20000) {
                battleGrid_setupLightInterpolation(10, pGrid->m1CC_lightColor, pGrid->m1D8_newLightColor);
                battleGrid_setupLightInterpolation2(10, pGrid->m1E4_lightFalloff0, pGrid->m1F0);
            }
            pThis->m38D_battleSubMode++;
        }
        break;
    case 5:
        if (pThis->m384_battleModeDelay++ < 10)
            break;
        pThis->m394 = 0;
        battleEngine_UpdateSub7Sub1();
        battleEngine_restoreCameraPostItem();
        pThis->m384_battleModeDelay = 0;
        pThis->m38D_battleSubMode++;
        pEngine->m188_flags.m1000 = 1;
        if (pThis->mAA4 != nullptr) {
            pThis->mAA4->getTask()->markFinished();
            pThis->mAA4 = nullptr;
        }
        break;
    case 6:
        if (pThis->m384_battleModeDelay++ < 15)
            break;
        pThis->m384_battleModeDelay = 0;
        pThis->m38D_battleSubMode++;
        pEngine->m188_flags.m1000 = 0;
        pEngine->m188_flags.m20_battleIntroRunning = 0;
        pEngine->m188_flags.m2_needToSortEnemiesByDistanceFromDragon = 1;
        break;
    case 7:
        pEngine->m188_flags.m10 = 0;
        pThis->m38D_battleSubMode++;
        break;
    case 8:
        pEngine->m188_flags.m80000_hideBattleHUD = 0;
        pThis->m38D_battleSubMode++;
        break;
    case 9:
        if (pThis->m384_battleModeDelay++ < 4)
            break;
        battleEngine_SetBattleMode16();
        break;
    default:
        assert(0);
    }
}

void battleEngine_updateBattleMode_3_shootEnemyWithHomingLaserSub0(s_battleEngine* pThis, sVec3_FP* param_2)
{
    param_2->zeroize();

    switch (gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant)
    {
    case 0:
        (*param_2)[2] = -pThis->m45C_perQuadrantDragonSpeed[gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant] / 4;
        (*param_2)[0] = pThis->m45C_perQuadrantDragonSpeed[gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant] / 4;
        break;
    case 1:
        (*param_2)[0] = pThis->m45C_perQuadrantDragonSpeed[gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant] / 4;
        (*param_2)[2] = -pThis->m45C_perQuadrantDragonSpeed[gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant] / 4;
        break;
    case 2:
        (*param_2)[2] = pThis->m45C_perQuadrantDragonSpeed[gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant] / 4;
        (*param_2)[0] = -pThis->m45C_perQuadrantDragonSpeed[gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant] / 4;
        break;
    case 3:
        (*param_2)[0] = -pThis->m45C_perQuadrantDragonSpeed[gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant] / 4;
        (*param_2)[2] = -pThis->m45C_perQuadrantDragonSpeed[gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant] / 4;
        break;
    default:
        assert(0);
    }
}

void battleEngine_updateBattleMode_3_shootEnemyWithHomingLaser(s_battleEngine* pThis)
{
    switch (pThis->m38D_battleSubMode)
    {
    case 0:
        pThis->m38D_battleSubMode++;
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m1 = 1;
        if (pThis->m390 == -1)
        {
            pThis->m386 = 1;
            pThis->m390 = 0;
        }
        else
        {
            pThis->m386 = 0;
        }
        pThis->m384_battleModeDelay = 0;
        break;
    case 1:
        pThis->m396 = readSaturnS16(g_BTL_GenericData->getSaturnPtr(0x060ad4e4) + gDragonState->mC_dragonType * 2);

        if (pThis->m396 < gBattleManager->m10_battleOverlay->mC_targetSystem->m20A_numSelectableEnemies) {
            pThis->m394 = pThis->m396;
        }
        else {
            pThis->m394 = gBattleManager->m10_battleOverlay->mC_targetSystem->m20A_numSelectableEnemies;
        }

        if (pThis->m390++ >= pThis->m392)
        {
            pThis->m390 = 0;
            if (pThis->m394 <= pThis->m384_battleModeDelay)
            {
                if (pThis->m394 > 0)
                {
                    battleCreateCinematicBars(pThis);
                    pThis->m384_battleModeDelay = 0;
                    pThis->m38D_battleSubMode++;
                    if ((gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x15] == 0) && (pThis->m386 != 1))
                    {
                        pThis->m3B4.m16_combo--;
                    }
                    pThis->m386 = 0;
                    increaseStatsCount(2);
                }
                else
                {
                    gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m4000000 = 1;
                    battleEngine_SetBattleMode16();
                }
            }
            else
            {
                // make enemy targetables selected one by one
                if (pThis->m3A0_LaserType == 0)
                {
                    gBattleManager->m10_battleOverlay->mC_targetSystem->m0_enemyTargetables[pThis->m384_battleModeDelay]->m4_targetable->m50_flags |= 0x20000;
                    gBattleManager->m10_battleOverlay->mC_targetSystem->m0_enemyTargetables[pThis->m384_battleModeDelay]->m4_targetable->m50_flags &= ~0x10000;
                }
                else
                {
                    gBattleManager->m10_battleOverlay->mC_targetSystem->m0_enemyTargetables[0]->m4_targetable->m50_flags |= 0x20000;
                    gBattleManager->m10_battleOverlay->mC_targetSystem->m0_enemyTargetables[0]->m4_targetable->m50_flags &= ~0x10000;
                }
                pThis->m384_battleModeDelay++;
            }
        }
        break;
    case 2:
        if (pThis->m384_battleModeDelay++ > 4)
        {
            pThis->m384_battleModeDelay = 0;
            pThis->m38D_battleSubMode++;
            battleEngine_enableAttackCamera();

            gBattleManager->m10_battleOverlay->m8_gridTask->m134_desiredCameraPosition[2] = 0;
            gBattleManager->m10_battleOverlay->m8_gridTask->mE4_currentCameraReferenceCenter[2] = 0;
            gBattleManager->m10_battleOverlay->m8_gridTask->m140_desiredCameraTarget[2] = -0xF000;
            gBattleManager->m10_battleOverlay->m8_gridTask->m108_deltaCameraPosition[2] = 0;

            pThis->m3E8.zeroize();
            pThis->m3DC.zeroize();
            pThis->m3F4_cameraPositionWhileShooting.zeroize();

            battleEngine_updateBattleMode_3_shootEnemyWithHomingLaserSub0(pThis, &pThis->m3E8);
            battleEngineSub1_UpdateSub2(&pThis->m3F4_cameraPositionWhileShooting, gBattleManager->m10_battleOverlay->m18_dragon->m8_position, pThis->m3E8, pThis->m3DC);
            battleEngine_setCurrentCameraPositionPointer(&pThis->m3F4_cameraPositionWhileShooting);
            battleEngine_setDesiredCameraPositionPointer(&gBattleManager->m10_battleOverlay->m18_dragon->m8_position);
            battleEngine_resetCameraInterpolation();
        }
        break;
    case 3:
        battleEngineSub1_UpdateSub2(&pThis->m3F4_cameraPositionWhileShooting, gBattleManager->m10_battleOverlay->m18_dragon->m8_position, pThis->m3E8, pThis->m3DC);
        if (pThis->m384_battleModeDelay++ > 5)
        {
            gBattleManager->m10_battleOverlay->m18_dragon->m88 |= 1;
            pThis->m384_battleModeDelay = 0;
            pThis->m38D_battleSubMode++;
        }
        break;
    case 4:
        battleEngineSub1_UpdateSub2(&pThis->m3F4_cameraPositionWhileShooting, gBattleManager->m10_battleOverlay->m18_dragon->m8_position, pThis->m3E8, pThis->m3DC);
        if (gDragonState->mC_dragonType == 8)
        {
            if (pThis->m384_battleModeDelay++ == 3)
            {
                battleEngine_createHomingLaserRootTask(pThis, pThis->m394);
                pThis->m384_battleModeDelay = 0;
                pThis->m38D_battleSubMode++;
            }
        }
        else
        {
            if (pThis->m384_battleModeDelay++ == 5)
            {
                battleEngine_createHomingLaserRootTask(pThis, pThis->m394);
                pThis->m384_battleModeDelay = 0;
                pThis->m38D_battleSubMode++;
            }
        }
        break;
    case 5:
        battleEngineSub1_UpdateSub2(&pThis->m3F4_cameraPositionWhileShooting, gBattleManager->m10_battleOverlay->m18_dragon->m8_position, pThis->m3E8, pThis->m3DC);
        if (pThis->m384_battleModeDelay++ == 2)
        {
            battleEngine_setDesiredCameraPositionPointer(&pThis->mC_battleCenter);
        }
        if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished)
        {
            pThis->m394 = 0;
            battleEngine_UpdateSub7Sub2();
            gBattleManager->m10_battleOverlay->m8_gridTask->mE4_currentCameraReferenceCenter = gBattleManager->m10_battleOverlay->m8_gridTask->m134_desiredCameraPosition;
            gBattleManager->m10_battleOverlay->m8_gridTask->mF0_currentCameraReferenceForward = gBattleManager->m10_battleOverlay->m8_gridTask->m140_desiredCameraTarget;
            gBattleManager->m10_battleOverlay->m8_gridTask->m108_deltaCameraPosition.zeroize();
            gBattleManager->m10_battleOverlay->m8_gridTask->m114_deltaCameraTarget.zeroize();

            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2000000 = 1;
            pThis->m384_battleModeDelay = 0;

            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m1000 = 1;
            pThis->m38D_battleSubMode++;
        }
        break;
    case 6:
        if (pThis->m384_battleModeDelay++ > 0xF)
        {
            pThis->m384_battleModeDelay = 0;
            pThis->m38D_battleSubMode++;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m1000 = 0;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m20_battleIntroRunning = 0;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2_needToSortEnemiesByDistanceFromDragon = 1;
        }
        break;
    case 7:
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m10 = 0;
        pThis->m38D_battleSubMode++;
        break;
    case 8:
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m80000_hideBattleHUD = 0;
        pThis->m38D_battleSubMode++;
        break;
    case 9:
        if (pThis->m384_battleModeDelay++ > 4)
        {
            battleEngine_SetBattleMode16();
        }
        break;
    default:
        assert(0);
    }
}

static void battleEngine_updateBattleMode_9(s_battleEngine* pThis); // forward decl

void updateBattleIntro(s_battleEngine* pThis)
{
    if (battleEngine_isPlayerTurnActive() == 0)
        return;

    switch (pThis->m38C_battleMode)
    {
    case eBattleModes::m0_shootEnemyWithGun:
        battleEngine_updateBattleMode_0_shootEnemyWithGun(pThis);
        break;
    case eBattleModes::m1_useItem:
        battleEngine_updateBattleMode_1_useItem(pThis);
        break;
    case eBattleModes::m3_shootEnemeyWithHomingLaser:
        battleEngine_updateBattleMode_3_shootEnemyWithHomingLaser(pThis);
        break;
    case eBattleModes::m4_useBerserk:
        battleEngine_updateBattleMode_4_useBerserk(pThis);
        break;
    case eBattleModes::m6_dragonMoving:
        break;
    case eBattleModes::m7:
        battleEngine_updateBattleMode_7(pThis);
        break;
    case eBattleModes::m8_playAttackCamera:
        battleEngine_updateBattleMode_8_playAttackCamera(pThis);
        break;
    case eBattleModes::mA:
        break;
    case eBattleModes::mB_enemyMovingDragon:
        break;
    case eBattleModes::mC_commandMenuOpen:
        break;
    case eBattleModes::mE_battleIntro:
        battleEngine_updateBattleMode_E_battleIntro(pThis);
        break;
    case eBattleModes::mF_tutorialTextInput:
        // 0605d5a8 — tutorial text input mode: wait for attack-finished flag, then return to position mode
        if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished)
        {
            battleEngine_SetBattleMode16();
        }
        break;
    case eBattleModes::m10_position:
        if (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m10)
        {
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m80 = 0;
        }
        else if (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m20_battleIntroRunning)
        {
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m10 = 0;
        }
        else
        {
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m20_battleIntroRunning = 0;
        }
        return;
    case (eBattleModes)9: // enemy-moving-dragon camera sequence
        battleEngine_updateBattleMode_9(pThis);
        break;
    default:
        assert(0);
        break;
    }
}

// 0606a67e
static void battleEngine_updateBattleMode_9(s_battleEngine* pThis)
{
    switch (pThis->m38D_battleSubMode)
    {
    case 0:
        battleCreateCinematicBars(pThis);
        pThis->m38D_battleSubMode++;
        break;
    case 1:
        if (pThis->m384_battleModeDelay++ > 4)
        {
            pThis->m384_battleModeDelay = 0;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2000 = 1;
            pThis->m38D_battleSubMode++;
        }
        break;
    case 2:
        if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished)
        {
            pThis->m38D_battleSubMode++;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m40 = 1;
        }
        break;
    case 3:
        if (pThis->m384_battleModeDelay++ > 0xF)
        {
            pThis->m384_battleModeDelay = 0;
            pThis->m38D_battleSubMode++;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m1000 = 1;
        }
        break;
    case 4:
        if (pThis->m384_battleModeDelay++ > 0xF)
        {
            pThis->m384_battleModeDelay = 0;
            pThis->m38D_battleSubMode++;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m1000 = 0;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m20_battleIntroRunning = 0;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2_needToSortEnemiesByDistanceFromDragon = 1;
        }
        break;
    case 5:
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m10 = 0;
        pThis->m38D_battleSubMode++;
        break;
    case 6:
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m80000_hideBattleHUD = 0;
        pThis->m38D_battleSubMode++;
        break;
    case 7:
        if (pThis->m384_battleModeDelay++ > 4)
        {
            battleEngine_SetBattleMode16();
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m4000 = 0;
        }
        break;
    }
}

void battleEngine_processCommandMenuInput(s_battleEngine* pThis)
{
    if ((pThis->m388 & 4) == 0)
    {
        if (gBattleManager->m10_battleOverlay->m4_battleEngine->m498_numEnemies &&
            battleEngine_isInputAllowed() &&
            (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m80000_hideBattleHUD)
            ) {
            if ((pThis->m3B4.m16_combo > 0) && (graphicEngineStatus.m4514.mD8_buttonConfig[2][1] & graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown))
            {
                gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m40 = 0;
                battleEngine_SetBattleMode(mC_commandMenuOpen);
                createBattleCommandMenu(gBattleManager->m10_battleOverlay->m20_battleHud);
            } else if ((pThis->m388 & 0x200) == 0)
            {
                if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & 8)
                {
                    battleEngine_SetBattleMode(mC_commandMenuOpen);
                    createBattleCommandMenu(gBattleManager->m10_battleOverlay->m20_battleHud);
                }
                if (graphicEngineStatus.m4514.mD8_buttonConfig[2][1] & graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown)
                {
                    playSystemSoundEffect(5);
                }
            }
        }
    }
}

void battleEngine_processLaserInput(s_battleEngine* pThis)
{
    if ((gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m4000) && (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m10 & 1))
    {
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m4000 = 0;
    }
    if (((pThis->m388 & 2) == 0) &&
        (gBattleManager->m10_battleOverlay->m4_battleEngine->m498_numEnemies) &&
        (battleEngine_isInputAllowed())
        )
    {
        if ((pThis->m3B4.m16_combo > 0) && !gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m4000 && (graphicEngineStatus.m4514.mD8_buttonConfig[2][0] & graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown))
        {
            if (gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 1)
            {
                playSystemSoundEffect(5);
            }
            else
            {
                if (gBattleManager->m10_battleOverlay->mC_targetSystem->m20A_numSelectableEnemies > 0)
                {
                    gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m40 = 0;
                    battleEngine_SetBattleMode(eBattleModes::m3_shootEnemeyWithHomingLaser);
                    pThis->m184 = 0;
                    pThis->m390 = 0;
                }
            }
        }
        else if (graphicEngineStatus.m4514.mD8_buttonConfig[2][0] & graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown)
        {
            playSystemSoundEffect(5);
        }
    }
}

void battleEngine_processGunInput(s_battleEngine* pThis)
{
    if ((pThis->m388 & 1) == 0)
    {
        if ((0 < gBattleManager->m10_battleOverlay->m4_battleEngine->m498_numEnemies) &&
            (battleEngine_isInputAllowed() != 0)) {
            if ((pThis->m3B4.m16_combo < 1) ||
                (((graphicEngineStatus.m4514.mD8_buttonConfig[2][2] & graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown) == 0 ||
                ((gBattleManager->m10_battleOverlay->mC_targetSystem->m20A_numSelectableEnemies) < 1))))
            {
                if (graphicEngineStatus.m4514.mD8_buttonConfig[2][2] & graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown)
                {
                    playSystemSoundEffect(5);
                }
            }
            else
            {
                gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m40 = 0;
                battleEngine_SetBattleMode(eBattleModes::m0_shootEnemyWithGun);
                pThis->m184 = 0;
            }
        }
    }
}

s32 battleEngine_isInputNotSuppressed()
{
    if (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m200_suppressBattleInputs)
        return 1;
    return 0;
}

s32 getDragonAgilityForMove(s16 param)
{
    return MTH_Mul(0x10000, MTH_Mul(param * 0x10000, 0x18000 - MTH_Mul(0x10000, FP_Div(mainGameState.gameStats.mC0_dragonAgl, 200)))+ 0x8000).getInteger();
}

void initiateDragonBattleMove(int param1, s16 param2)
{
    battleEngine_SetBattleMode(eBattleModes::m6_dragonMoving);
    gBattleManager->m10_battleOverlay->m4_battleEngine->m22E_dragonMoveDirection = param1;

    s32 dragonAgilityForMove = getDragonAgilityForMove(param2);
    gBattleManager->m10_battleOverlay->m4_battleEngine->m27C_dragonMovementInterpolator1.m68_rate = dragonAgilityForMove;
    gBattleManager->m10_battleOverlay->m4_battleEngine->m2E8_dragonMovementInterpolator2.m68_rate = dragonAgilityForMove;

    gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m1000000_dragonMoving = 1;
}

void battleEngine_MainUpdate(s_battleEngine* pThis)
{
    battleEngine_ApplyBattleAutoScrollDelta(pThis);
    if (battleEngine_checkBattleCompletion() == 1)
        return;
    pThis->m47C_exp = pThis->m47C_exp + 1;
    battleEngine_processMovementMode(pThis);
    battleEngine_staticBattleOrbitUpdate(pThis);
    battleEngine_computeDragonPosition(pThis);
    battleEngine_detectQuadrantFromAngle(pThis);
    updateBattleIntro(pThis);
    battleEngine_processCommandMenuInput(pThis);
    battleEngine_processLaserInput(pThis);
    battleEngine_processGunInput(pThis);

    if (gBattleManager->m10_battleOverlay->m4_battleEngine->m498_numEnemies < 1)
        return;

    if (!battleEngine_isInputNotSuppressed())
    {
        pThis->m1BC_dragonYaw = 0;
        pThis->m1B8_dragonPitch = 0;
        return;
    }

    bool bVar1 = 1;

    static const std::array<std::array<int, 4>, 4> quadrantRotationTable = {
    {
        { 0, 1, 3, 2 },
        { 1, 2, 0, 3 },
        { 2, 3, 1, 0 },
        { 3, 0, 2, 1 },
    }
    };

    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x1F] == 0) {
        switch (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m0_inputType)
        {
        case 1: // digital
            if (graphicEngineStatus.m4514.mD8_buttonConfig[2][7] & graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown)
            {
                if (pThis->m388 & 0x40)
                {
                    return;
                }

                if ((gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 8) == 0)
                {
                    assert(readSaturnS8(g_BTL_GenericData->getSaturnPtr(0x60A9281) + gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant * 4) == quadrantRotationTable[gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant][1]);

                    int uVar4 = shiftLeft32(1, quadrantRotationTable[gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant][1]);
                    if ((uVar4 & gBattleManager->m10_battleOverlay->m4_battleEngine->m22F_battleRadarLockIcon) == 0)
                    {
                        initiateDragonBattleMove(1, 0x1E);
                        bVar1 = 0;
                    }
                }
                else
                {
                    bVar1 = 0;
                }
            }
            else if (graphicEngineStatus.m4514.mD8_buttonConfig[2][6] & graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown)
            {
                if (pThis->m388 & 0x20)
                {
                    return;
                }

                if ((gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 8) == 0)
                {
                    assert(readSaturnS8(g_BTL_GenericData->getSaturnPtr(0x60A9282) + gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant * 4) == quadrantRotationTable[gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant][2]);

                    int uVar4 = shiftLeft32(1, quadrantRotationTable[gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant][2]);
                    if ((uVar4 & gBattleManager->m10_battleOverlay->m4_battleEngine->m22F_battleRadarLockIcon) == 0)
                    {
                        initiateDragonBattleMove(2, 0x1E);
                        bVar1 = 0;
                    }
                }
                else
                {
                    bVar1 = 0;
                }
            }
            // 6059b1c
            if (
                (((gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 8) != 0) | (bVar1)) &&
                (((graphicEngineStatus.m4514.mD8_buttonConfig[2][7] | graphicEngineStatus.m4514.mD8_buttonConfig[2][6]) & graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown) != 0)
                )
            {
                playSystemSoundEffect(5);
                return;
            }

            if(graphicEngineStatus.m4514.mD8_buttonConfig[2][4] & graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown)
            {
                if (pThis->m1B8_dragonPitch + 0x51E > 0xFFFF) {
                    if (pThis->m1B8_dragonPitch - 0x1478 > 0x10000)
                    {
                        pThis->m1B8_dragonPitch -= 0x1478;
                        return;
                    }
                    pThis->m1B8_dragonPitch = 0x10000;
                    return;
                }
                pThis->m1B8_dragonPitch += 0x51E;
                return;
            }
            else if (graphicEngineStatus.m4514.mD8_buttonConfig[2][5] & graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown)
            {
                if (pThis->m1B8_dragonPitch - 0x51E < -0xFFFF)
                {
                    if (pThis->m1B8_dragonPitch + 0x1478 > -0x10001)
                    {
                        pThis->m1B8_dragonPitch = -0x10000;
                        return;
                    }
                    pThis->m1B8_dragonPitch += 0x1478;
                    return;
                }
                pThis->m1B8_dragonPitch -= 0x51E;
                return;
            }
            else
            {
                if (pThis->m1B8_dragonPitch - 0x1333 < 1)
                {
                    if (pThis->m1B8_dragonPitch + 0x1333 > -1)
                    {
                        pThis->m1B8_dragonPitch = 0;
                        return;
                    }
                    pThis->m1B8_dragonPitch += 0x1333;
                    return;
                }
                pThis->m1B8_dragonPitch -= 0x1333;
                return;
            }
            break;
        case 2: // analog
        {
            // Analog stick directly controls pitch/yaw
            pThis->m1BC_dragonYaw = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m2_analogX * -0x200;
            pThis->m1B8_dragonPitch = (s32)graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m3_analogY << 8;

            // Same orbit rotation logic as digital
            if (graphicEngineStatus.m4514.mD8_buttonConfig[2][7] & graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown)
            {
                if (pThis->m388 & 0x40)
                    return;

                if ((gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 8) == 0)
                {
                    int uVar4 = shiftLeft32(1, quadrantRotationTable[gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant][1]);
                    if ((uVar4 & gBattleManager->m10_battleOverlay->m4_battleEngine->m22F_battleRadarLockIcon) == 0)
                    {
                        initiateDragonBattleMove(1, 0x1E);
                        bVar1 = 0;
                    }
                }
                else
                {
                    bVar1 = 0;
                }
            }
            else if (graphicEngineStatus.m4514.mD8_buttonConfig[2][6] & graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown)
            {
                if (pThis->m388 & 0x20)
                    return;

                if ((gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 8) == 0)
                {
                    int uVar4 = shiftLeft32(1, quadrantRotationTable[gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant][2]);
                    if ((uVar4 & gBattleManager->m10_battleOverlay->m4_battleEngine->m22F_battleRadarLockIcon) == 0)
                    {
                        initiateDragonBattleMove(2, 0x1E);
                        bVar1 = 0;
                    }
                }
                else
                {
                    bVar1 = 0;
                }
            }

            if (
                (((gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 8) != 0) | (bVar1)) &&
                (((graphicEngineStatus.m4514.mD8_buttonConfig[2][7] | graphicEngineStatus.m4514.mD8_buttonConfig[2][6]) & graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown) != 0)
                )
            {
                playSystemSoundEffect(5);
                return;
            }
            break;
        }
        default:
            assert(0);
        }
    }
    else
    {
        // Debug mode input handling - alternate input configuration for development
        // 06059d48 (digital) and 06059fc4 (analog)

        // Check input type
        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m0_inputType != 1) // not digital
        {
            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m0_inputType != 2) // not analog either
                return;

            // Analog input in debug mode
            pThis->m1B8_dragonPitch = (s32)graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m3_analogY << 8;
            pThis->m1BC_dragonYaw = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m2_analogX * -0x100;

            // Check control flags (0x78 = 0x08 | 0x10 | 0x20 | 0x40)
            if ((pThis->m388 & 0x78) == 0)
            {
                // No special control flags set
                if ((gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 8) == 0)
                    return;

                // Dragon has special status - zero out controls and check for button press
                pThis->m1B8_dragonPitch = 0;
                pThis->m1BC_dragonYaw = 0;

                if (((graphicEngineStatus.m4514.mD8_buttonConfig[2][4] |
                      graphicEngineStatus.m4514.mD8_buttonConfig[2][5] |
                      graphicEngineStatus.m4514.mD8_buttonConfig[2][7] |
                      graphicEngineStatus.m4514.mD8_buttonConfig[2][6]) &
                     graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown) == 0)
                {
                    return;
                }
                playSystemSoundEffect(5);
                return;
            }

            // Control flags are set - clear pitch, handle yaw with more complex logic
            pThis->m1B8_dragonPitch = 0;
            // Yaw adjustment will be handled in the yaw-specific section below
        }
        else
        {
            // Digital input in debug mode

            // Pitch adjustment (buttons 4 and 5)
            if ((graphicEngineStatus.m4514.mD8_buttonConfig[2][4] &
                 graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown) == 0)
            {
                // Button 4 not pressed, check button 5 (down)
                if ((graphicEngineStatus.m4514.mD8_buttonConfig[2][5] &
                     graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown) == 0)
                {
                    // Neither button 4 nor 5 pressed
                    if ((pThis->m388 & 0x78) != 0)
                    {
                        // Control flag is set, zero pitch
                        pThis->m1B8_dragonPitch = 0;
                        pThis->m1BC_dragonYaw = 0;
                        return;
                    }

                    // Auto-center pitch when idle
                    if (pThis->m1B8_dragonPitch - 0x1333 < 1)
                    {
                        if (-1 < pThis->m1B8_dragonPitch + 0x1333)
                        {
                            pThis->m1B8_dragonPitch = 0;
                        }
                        else
                        {
                            pThis->m1B8_dragonPitch += 0x1333;
                        }
                    }
                    else
                    {
                        pThis->m1B8_dragonPitch -= 0x1333;
                    }
                }
                else
                {
                    // Button 5 (down) is pressed
                    if ((pThis->m388 & 0x10) != 0)
                    {
                        pThis->m1B8_dragonPitch = 0;
                        return;
                    }

                    if (pThis->m1B8_dragonPitch - 0x51E < -0xFFFF)
                    {
                        if (-0x10001 < pThis->m1B8_dragonPitch + 0x1478)
                        {
                            pThis->m1B8_dragonPitch = -0x10000;
                        }
                        else
                        {
                            pThis->m1B8_dragonPitch += 0x1478;
                        }
                    }
                    else
                    {
                        pThis->m1B8_dragonPitch -= 0x51E;
                    }
                }
            }
            else
            {
                // Button 4 (up) is pressed
                if ((pThis->m388 & 0x08) != 0)
                {
                    pThis->m1B8_dragonPitch = 0;
                    return;
                }

                if (pThis->m1B8_dragonPitch + 0x51E < 0x10000)
                {
                    pThis->m1B8_dragonPitch += 0x51E;
                }
                else
                {
                    if (pThis->m1B8_dragonPitch - 0x1478 < 0x10001)
                    {
                        pThis->m1B8_dragonPitch = 0x10000;
                    }
                    else
                    {
                        pThis->m1B8_dragonPitch -= 0x1478;
                    }
                }
            }
        }

        // Yaw adjustment (buttons 7 and 6) - applies to both digital and analog debug modes
        if ((graphicEngineStatus.m4514.mD8_buttonConfig[2][7] &
             graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown) == 0)
        {
            if ((graphicEngineStatus.m4514.mD8_buttonConfig[2][6] &
                 graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown) == 0)
            {
                // Neither button 7 nor 6 pressed
                if ((pThis->m388 & 0x78) != 0)
                {
                    pThis->m1B8_dragonPitch = 0;
                    pThis->m1BC_dragonYaw = 0;
                    return;
                }

                // Auto-center yaw when idle
                if (pThis->m1BC_dragonYaw - 0x1333 < 1)
                {
                    if (-1 < pThis->m1BC_dragonYaw + 0x1333)
                    {
                        pThis->m1BC_dragonYaw = 0;
                    }
                    else
                    {
                        pThis->m1BC_dragonYaw += 0x1333;
                    }
                }
                else
                {
                    pThis->m1BC_dragonYaw -= 0x1333;
                }
            }
            else
            {
                // Button 6 (right) is pressed
                if ((pThis->m388 & 0x20) != 0)
                {
                    pThis->m1BC_dragonYaw = 0;
                    return;
                }

                if (pThis->m1BC_dragonYaw < 0)
                {
                    pThis->m1BC_dragonYaw = 0;
                }
                else
                {
                    if (pThis->m1BC_dragonYaw - 0x51E < -0xFFFF)
                    {
                        if (-0x10001 < pThis->m1BC_dragonYaw + 0x1478)
                        {
                            pThis->m1BC_dragonYaw = -0x10000;
                        }
                        else
                        {
                            pThis->m1BC_dragonYaw += 0x1478;
                        }
                    }
                    else
                    {
                        pThis->m1BC_dragonYaw -= 0x51E;
                    }
                }
            }
        }
        else
        {
            // Button 7 (left) is pressed
            if ((pThis->m388 & 0x40) != 0)
            {
                pThis->m1BC_dragonYaw = 0;
                return;
            }

            if (pThis->m1BC_dragonYaw < 0)
            {
                pThis->m1BC_dragonYaw = 0;
            }
            else if (pThis->m1BC_dragonYaw + 0x51E < 0x10000)
            {
                pThis->m1BC_dragonYaw += 0x51E;
            }
            else
            {
                if (pThis->m1BC_dragonYaw - 0x1478 < 0x10001)
                {
                    pThis->m1BC_dragonYaw = 0x10000;
                }
                else
                {
                    pThis->m1BC_dragonYaw -= 0x1478;
                }
            }
        }

        // Final check: if dragon has special status, zero controls and check for button
        if ((gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 8) == 0)
            return;

        pThis->m1B8_dragonPitch = 0;
        pThis->m1BC_dragonYaw = 0;

        if (((graphicEngineStatus.m4514.mD8_buttonConfig[2][4] |
              graphicEngineStatus.m4514.mD8_buttonConfig[2][5] |
              graphicEngineStatus.m4514.mD8_buttonConfig[2][7] |
              graphicEngineStatus.m4514.mD8_buttonConfig[2][6]) &
             graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown) == 0)
        {
            return;
        }
        playSystemSoundEffect(5);
    }
}

void battleEngine_Update(s_battleEngine* pThis)
{
    switch (pThis->m18C_status)
    {
    case 0: // init
        pThis->m18[0] = 0;
        pThis->m18[1] = 0x8000000;
        pThis->m18[2] = 0;
        pThis->m18C_status++;

        pThis->m1E8[0] = 0x2000;
        pThis->m1E8[1] = 0x1000;
        pThis->m398_currentSelectedEnemy = 0;
        pThis->m39C_maxSelectableEnemies = 0;
        pThis->m3A4_prelockMode[0] = 2;
        pThis->m3A4_prelockMode[1] = 0x10;

        pThis->m188_flags.m8000 = 1;

        resetNBG1Map();

        pThis->m3CC = createBattleEngineSub0(pThis);
        pThis->m3B1 = 0;

        battleEngine_UpdateSub1(1);
        battleEngine_UpdateSub2(pThis);
        battleEngine_CreateHud1(dramAllocatorEnd[0].mC_fileBundle);
        battleEngine_CreateRadar(dramAllocatorEnd[0].mC_fileBundle);
        battleEngine_snapDragonFacing();

        battleEngine_SetBattleMode(eBattleModes::mE_battleIntro);
        return;
    case 1: // running
        battleEngine_MainUpdate(pThis);
        battleEngine_snapDragonFacing();
        return;
    case 2:
        battleEngine_ApplyBattleAutoScrollDelta(pThis);
        battleEngine_computeDragonPosition(pThis);
        battleEngine_snapDragonFacing();
        return;
    case 3:
        return;
    case 4:
        battleEngine_ApplyBattleAutoScrollDelta(pThis);
        battleEngine_computeDragonPosition(pThis);
        battleEngine_snapDragonFacing();
        return;
    default:
        assert(0);
        break;
    }
}

void battleEngine_Draw(s_battleEngine* pThis)
{
    // seems to be all debug input related
    Unimplemented();
}

void battleEngine_postBattleUpdateHPBP()
{
    if (mainGameState.gameStats.m10_currentHP < 1) {
        mainGameState.gameStats.m10_currentHP = mainGameState.gameStats.mB8_maxHP;
        mainGameState.gameStats.m14_currentBP = mainGameState.gameStats.mBA_maxBP;
    }
    if (10000 < mainGameState.gameStats.mB8_maxHP) {
        mainGameState.gameStats.mB8_maxHP = 9999;
    }
    if (1000 < mainGameState.gameStats.mBA_maxBP) {
        mainGameState.gameStats.mBA_maxBP = 999;
    }
}

void battleEngine_Delete(s_battleEngine* pThis)
{
    battleEngine_postBattleUpdateHPBP();
}

s32 battleEngine_isPlayerTurnActive()
{
    if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m80 != 0)
    {
        return 1;
    }
    return 0;
}

p_workArea createBattleEngineTask(p_workArea parent, sSaturnPtr battleData)
{
    static const s_battleEngine::TypedTaskDefinition definition = {
        &battleEngine_Init,
        &battleEngine_Update,
        &battleEngine_Draw,
        &battleEngine_Delete,
    };
    //TODO this should have been createSubTaskWithArgWithCopy, but parent class doesn't have a copy, so it's probably a bug
    return createSubTaskWithArg<s_battleEngine>(parent, battleData, &definition);
}
