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
#include "kernel/debug/trace.h"

#include "BTL_A3/BTL_A3.h"

void battleEngine_UpdateSub7Sub0Sub3Sub0(s_battleEngine* pThis, fixedPoint uParm2, s32 r6);
void fieldPaletteTaskInitSub0Sub2();

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

    pBattleEngine->m498 = 0;
    for (int i = 0; i < 0x80; i++)
    {
        pBattleEngine->m49C[i].m0_isActive = -1;
        pBattleEngine->m49C[i].m4 = 0;
        pBattleEngine->m49C[i].m8_distanceToDragonSquare = -1;
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

void battleEngine_InitSub1(s_battleEngine* pThis)
{
    pThis->m1AC = pThis->m1A0;
}

void battleEngine_InitSub2Sub0(s_battleEngine* pThis)
{
    s32 var1 = pThis->m440_battleDirectionAngle.normalized();
    if ((var1 < -0x4000000) || (var1 > 0x3ffffff))
    {
        pThis->m46C_dragon2dSpeed[1] = pThis->m45C_perQuadrantDragonSpeed[1][0];
    }
    else
    {
        pThis->m46C_dragon2dSpeed[1] = pThis->m45C_perQuadrantDragonSpeed[0][0];
    }

    if (var1 < 1)
    {
        pThis->m46C_dragon2dSpeed[0] = pThis->m45C_perQuadrantDragonSpeed[1][1];
    }
    else
    {
        pThis->m46C_dragon2dSpeed[0] = pThis->m45C_perQuadrantDragonSpeed[0][1];
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
    computeVectorAngles(pThis->mC_battleCenter - pThis->m104_dragonPosition, temp);

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

void battleEngine_InitSub5(p_workArea parent)
{
    FunctionUnimplemented();
}

void battleEngine_InitSub6(sVec3_FP* pData)
{
    gBattleManager->m10_battleOverlay->m4_battleEngine->m3D4 = pData;
    gBattleManager->m10_battleOverlay->m8_gridTask->m1B8_pCameraTranslationSource = pData;
}

void battleEngine_InitSub7(sVec3_FP* pData)
{
    gBattleManager->m10_battleOverlay->m4_battleEngine->m3D8 = pData;
}

void createBattleDisplayCommandHelpTask(p_workArea parent, sSaturnPtr data)
{
    FunctionUnimplemented();
}

void battleEngine_InitSub8Sub0(sVec2_FP& param)
{
    s_battleEngine* pBattleEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    s_battleGrid* pBattleGrid = gBattleManager->m10_battleOverlay->m8_gridTask;

    if (isTraceEnabled())
    {
        addTraceLog(*pBattleEngine->m3D8, "3D8");
        addTraceLog(*pBattleEngine->m3D4, "3D4");
    }

    sVec3_FP delta = *pBattleEngine->m3D8 - *pBattleEngine->m3D4;
    computeVectorAngles(delta, param);
    
    pBattleEngine->m1C8 = param[0];
    pBattleEngine->m1CC = param[1];

    pBattleGrid->m64_cameraRotationTarget[0] = param[0];
    pBattleGrid->m64_cameraRotationTarget[1] = param[1];
}

void battleEngine_InitSub8Sub1(sVec2_FP& param)
{
    if (isTraceEnabled())
    {
        addTraceLog(param, "param");
    }
    s_battleGrid* pBattleGrid = gBattleManager->m10_battleOverlay->m8_gridTask;
    pBattleGrid->mB4_cameraRotation[0] = param[0];
    pBattleGrid->mB4_cameraRotation[1] = param[1];
}

void battleEngine_InitSub8()
{
    s_battleGrid* pBattleGrid = gBattleManager->m10_battleOverlay->m8_gridTask;
    pBattleGrid->m180_cameraTranslation = *pBattleGrid->m1B8_pCameraTranslationSource;
    sVec2_FP auStack16;
    battleEngine_InitSub8Sub0(auStack16);
    battleEngine_InitSub8Sub1(auStack16);
    pBattleGrid->m18C.zeroize();
    pBattleGrid->mC0_cameraRotationInterpolation.zeroize();
}

void battleEngine_InitSub9(p_workArea parent)
{
    FunctionUnimplemented();
}

void battleEngine_InitSub11()
{
    int iVar1 = gDragonState->mC_dragonType;
    if (((((iVar1 == 1) || (iVar1 == 2)) || (iVar1 == 3)) || ((iVar1 == 4 || (iVar1 == 5)))) ||
        ((iVar1 == 6 || ((iVar1 == 7 || (iVar1 == 8)))))) {
        mainGameState.consumables[49] |= 4;
        mainGameState.consumables[52] |= 0x74;
    }
    else {
        mainGameState.consumables[49] &= ~4;
        mainGameState.consumables[52] &= ~0x74;
    }
}

void executeFuncPtr(sSaturnPtr funcPtr, s_battleEngine* pThis)
{
    gCurrentBattleOverlay->invoke(funcPtr, pThis);
}

void battleEngine_Init(s_battleEngine* pThis, sSaturnPtr overlayBattleData)
{
    gBattleManager->m10_battleOverlay->m4_battleEngine = pThis;
    pThis->m3A8_overlayBattledata = overlayBattleData;
    pThis->m3B0_subBattleId = gBattleManager->m6_subBattleId;

    sSaturnPtr battleData_4 = readSaturnEA(overlayBattleData + pThis->m3B0_subBattleId * 0x20 + 4);
    pThis->m3AC = readSaturnEA(overlayBattleData + pThis->m3B0_subBattleId * 0x20 );

    createInBattleDebugTask(pThis);
    createBattleDisplayCommandHelpTask(pThis, readSaturnEA(overlayBattleData + pThis->m3B0_subBattleId * 0x20 + 0xC));
    initBattleEngineArray();

    int randomQuadrantOdd = performModulo2(100, randomNumber()) % 0xFF;
    std::array<int, 3> perQuadrantOdds;

    sSaturnPtr pData = readSaturnEA(gCurrentBattleOverlay->getBattleEngineInitData() + gBattleManager->m4 * 4);
    if (pData.isNull())
    {
        perQuadrantOdds[0] = 100;
        perQuadrantOdds[1] = 0;
        perQuadrantOdds[2] = 0;
    }
    else
    {
        perQuadrantOdds[0] = readSaturnS8(pData + gBattleManager->m8 * 0x10 + 4);
        perQuadrantOdds[1] = readSaturnS8(pData + gBattleManager->m8 * 0x10 + 5);
        perQuadrantOdds[2] = readSaturnS8(pData + gBattleManager->m8 * 0x10 + 6);
    }

    if (randomQuadrantOdd < perQuadrantOdds[0])
    {
        pThis->m22C_dragonCurrentQuadrant = 0;
    }
    else if (randomQuadrantOdd < perQuadrantOdds[0] + perQuadrantOdds[1])
    {
        pThis->m22C_dragonCurrentQuadrant = 1;
    }
    else if (randomQuadrantOdd < perQuadrantOdds[0] + perQuadrantOdds[1] + perQuadrantOdds[2])
    {
        pThis->m22C_dragonCurrentQuadrant = 2;
    }
    else
    {
        pThis->m22C_dragonCurrentQuadrant = 3;
    }

    pThis->m22D_dragonPreviousQuadrant = pThis->m22C_dragonCurrentQuadrant;

    if (pData.isNull())
    {
        pThis->m3B4.m16_combo = 0;
    }
    else
    {
        pThis->m3B4.m16_combo = readSaturnS8(pData + gBattleManager->m8 * 0x10 + 0xC);
    }

    pThis->m270_enemyAltitude.zeroize();
    pThis->m258.zeroize();
    pThis->m1A0 = readSaturnVec3(battleData_4 + 0x4C);
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
    for (int i = 0; i < 2; i++)
    {
        pThis->m45C_perQuadrantDragonSpeed[i][0] = readSaturnS32(battleData_4 + 0x1C + i * 8 + 0);
        pThis->m45C_perQuadrantDragonSpeed[i][1] = readSaturnS32(battleData_4 + 0x1C + i * 8 + 4);
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

    pThis->m390[0] = 0;
    pThis->m390[1] = 2;
    pThis->m390[2] = 0;

    pThis->m3A2 = 0;

    pThis->m3B4.m0_max = 0x3C0000;
    pThis->m3B4.m4 = 0x10000;
    pThis->m3B4.m8 = 0;

    initMatrixToIdentity(&pThis->m1F0);

    pThis->m434 = 0xA000;

    createBattleEnvironmentGridTask(pThis);

    pThis->m264 += pThis->m1A0;
    pThis->m234 = pThis->m270_enemyAltitude + pThis->m264;

    pThis->m240 = pThis->m270_enemyAltitude + pThis->m258 + pThis->m264;

    battleEngine_convertBattleQuadrantToBattleDirection(pThis);
    battleEngine_InitSub1(pThis);
    battleEngine_InitSub2(pThis);
    battleEngine_InitSub3(pThis);
    battleEngine_createDragonTask(pThis);
    battleEngine_InitSub5(pThis);
    battleEngine_InitSub6(&gBattleManager->m10_battleOverlay->m8_gridTask->m34_cameraPosition);
    battleEngine_InitSub7(&pThis->mC_battleCenter);
    battleEngine_InitSub8();
    battleEngine_InitSub9(pThis);

    executeFuncPtr(readSaturnEA(overlayBattleData + pThis->m3B0_subBattleId * 0x20 + 8), pThis);

    resetCamera(0, 0, 0x160, 0xe0, 0xb0, 0x70);
    initVDP1Projection(0x1c71c71, 0);
    setupVdp1LocalCoordinatesAndClipping();

    g_fadeControls.m_4D = 6;
    if (g_fadeControls.m_4C < g_fadeControls.m_4D)
    {
        vdp2Controls.m20_registers[0].m112_CLOFSL = 0;
        vdp2Controls.m20_registers[1].m112_CLOFSL = 0;
    }

    fadePalette(&g_fadeControls.m24_fade1, 0xC210, 0xC210, 1);
    fadePalette(&g_fadeControls.m0_fade0, 0xFFFF, 0xFFFF, 1);

    g_fadeControls.m_4D = 5;

    pThis->m474.zeroize();
    pThis->m480.fill(-1);
    pThis->m190.fill(0);

    createBattleEngineSub1(pThis);
    battleEngine_InitSub11();

    if (mainGameState.gameStats.mC_laserPower < 1) {
        mainGameState.gameStats.mC_laserPower = 80;
        mainGameState.gameStats.mE_gunPower = 60;
    }

    for (int i = 0; i < 5; i++)
    {
        pThis->m484[i].fill(-1);
    }
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

            sSaturnPtr pData = readSaturnEA(gCurrentBattleOverlay->getBattleEngineInitData() + gBattleManager->m4 * 4);
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

s32 s_battleDragon_InitSub0()
{
    s8 bVar1 = gBattleManager->m10_battleOverlay->m4_battleEngine->m230;

    if ((((bVar1 != 0) && (bVar1 != 1)) && (bVar1 != 2)) &&
        (((bVar1 != 3 && (bVar1 != 8)) && (bVar1 != 10)))) {
        return 0;
    }
    return 1;
}


void battleEngine_UpdateSub5()
{
    s_battleDragon* pDragon = gBattleManager->m10_battleOverlay->m18_dragon;

    if (s_battleDragon_InitSub0() == 0)
    {
        assert(0);
    }
    else
    {
        pDragon->m74_targetRotation[1] -= pDragon->m74_targetRotation[1].normalized();
    }
}

void battleEngine_UpdateSub6(s_battleEngine* pThis)
{
    pThis->m1A0 = pThis->m1AC;
}

void battleEngine_SetBattleMode(eBattleModes param)
{
    s_battleEngine* pBattleEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    pBattleEngine->m38C_battleMode = param;
    pBattleEngine->m188_flags.m80 = 1;
    pBattleEngine->m188_flags.m100 = 0;
    pBattleEngine->m188_flags.m2000 = 0;
    pBattleEngine->m188_flags.m10 = 1;
    pBattleEngine->m188_flags.m20_battleIntroRunning = 1;
    pBattleEngine->m188_flags.m200_suppressBattleInputs = 1;
    pBattleEngine->m38D_battleIntroStatus = 0;
    pBattleEngine->m384_battleIntroDelay = 0;
    pBattleEngine->m386 = 0;
    pBattleEngine->m184 = 0;
    pBattleEngine->m3E8.zeroize();
    battleEngine_UpdateSub6(pBattleEngine);
}

void battleEngine_SetBattleMode16()
{
    s_battleEngine* pBattleEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    pBattleEngine->m38C_battleMode = eBattleModes::m10;
    pBattleEngine->m188_flags.m800 = 0;
    pBattleEngine->m188_flags.m100 = 0;
    pBattleEngine->m188_flags.m2000 = 0;
    pBattleEngine->m188_flags.m40 = 0;
    pBattleEngine->m188_flags.m400000 = 0;
    pBattleEngine->m188_flags.m80000_hideBattleHUD = 0;
    pBattleEngine->m188_flags.m100000 = 0;

    battleEngine_FlagQuadrantBitForAttack(0);

    pBattleEngine->m390[0] = 0;
    pBattleEngine->m390[2] = 0;

    pBattleEngine->m384_battleIntroDelay = 0;
    pBattleEngine->m386 = 0;
    pBattleEngine->m3E8.zeroize();
    battleEngine_UpdateSub6(pBattleEngine);

    gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2 = 1;
}

void battleEngine_UpdateSub8(s_battleEngine* pThis)
{
    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x16])
    {
        assert(0);
    }

    pThis->m264 += pThis->m1A0;
    pThis->m234 = pThis->m270_enemyAltitude + pThis->m264;
    pThis->m240 = pThis->m270_enemyAltitude + pThis->m258 + pThis->m264;
}

s32 battleEngine_UpdateSub7Sub0Sub0() // 0x20 is indicating battle intro is running
{
    if (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m20_battleIntroRunning)
        return 1;
    return 0;
}

s32 battleEngine_UpdateSub7Sub0Sub2Sub0()
{
    if (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m10)
        return 1;
    return 0;
}

void battleEngine_UpdateSub7Sub1()
{
    s_battleGrid* pGrid = gBattleManager->m10_battleOverlay->m8_gridTask;
    if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m20000)
    {
        assert(0);
        sVec3_FP local_1c;
        //battleEngine_UpdateSub7Sub1Sub0(pGrid->m280_lightAngle1, &local_1c);
        pGrid->m1CC_lightColor = pGrid->m1D8_newLightColor;
    }
}

void battleEngine_UpdateSub7Sub2()
{
    gBattleManager->m10_battleOverlay->m8_gridTask->m1 = 0;

    gBattleManager->m10_battleOverlay->m8_gridTask->m134_desiredCameraPosition = readSaturnVec3(gCurrentBattleOverlay->getSaturnPtr(0x60ac478) + 0x24 * gBattleManager->m10_battleOverlay->m8_gridTask->m1);
    gBattleManager->m10_battleOverlay->m8_gridTask->m140_desiredCameraTarget = readSaturnVec3(gCurrentBattleOverlay->getSaturnPtr(0x60ac484) + 0x24 * gBattleManager->m10_battleOverlay->m8_gridTask->m1);
}

void battleEngine_UpdateSub7Sub3Sub0()
{
    gBattleManager->m10_battleOverlay->m8_gridTask->mB4_cameraRotation[2] = 0;
    gBattleManager->m10_battleOverlay->m8_gridTask->m64_cameraRotationTarget[2] = 0;
}

void battleEngine_UpdateSub7Sub3()
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
        battleEngine_InitSub6(&pGrid->m34_cameraPosition);
        pBattleEngine->m3D8 = &pBattleEngine->mC_battleCenter;
        battleEngine_UpdateSub7Sub3Sub0();
    }
}

void battleEngine_UpdateSub7Sub3Sub4(p_workArea parent)
{
    FunctionUnimplemented();
}

s32 battleEngine_UpdateSub7Sub0()
{
    s_battleEngine* pBattleEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    if (pBattleEngine->m3B2_numBattleFormationRunning < 1)
    {
        if (battleEngine_UpdateSub7Sub0Sub0() == 0)
            return 0;

        battleEngine_UpdateSub7Sub1();
        pBattleEngine->m1B8_dragonPitch = 0;
        pBattleEngine->m1BC_dragonYaw = 0;
        pBattleEngine->m3B2_numBattleFormationRunning = 0;

        clearVdp2TextMemory();
        fieldPaletteTaskInitSub0Sub2();

        pBattleEngine->m18C_status = 4;
        pBattleEngine->m188_flags.m80000_hideBattleHUD = 0;
        if (0 < mainGameState.gameStats.m10_currentHP)
        {
            battleEngine_UpdateSub7Sub2();
            battleEngine_UpdateSub7Sub3();

            if (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m10000)
            {
                gBattleManager->mE = 0;
            }
            else
            {
                gBattleManager->mE = 3;
            }

            if ((((pBattleEngine->m230 != 1) && (pBattleEngine->m230 != 3)) && (pBattleEngine->m230 != 5)) && ((pBattleEngine->m230 != 7 && (pBattleEngine->m230 != 8))))
            {
                pBattleEngine->m188_flags.m8 = 1;
                battleEngine_UpdateSub7Sub3Sub4(pBattleEngine);
                return 1;
            }

            if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m10000)
            {
                assert(0);
            }
            assert(0);
        }
        assert(0);
    }

    if (mainGameState.gameStats.m10_currentHP < 1)
    {
        assert(0);
    }
    else
    {
        if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m4)
        {
            assert(0);
        }
    }

    pBattleEngine->m3B2_numBattleFormationRunning = 0;
    return 0;
}

void battleEngine_UpdateSub7Sub0Sub1Sub1(s_battleEngine::s_27C* pThis)
{
    pThis->m48 = (pThis->m54 - pThis->m3C) / 2;
    pThis->m60_currentStep = 0;
    pThis->m64_stepSize = FP_Div(0x8000000, pThis->m68_rate << 16);
}

void battleEngine_UpdateSub7Sub0Sub1Sub2(s_battleEngine::s_27C* pThis)
{
    pThis->m18_middle = (pThis->m24_targetValue - pThis->mC_initialValue) / 2;
    pThis->m60_currentStep = 0;
    pThis->m64_stepSize = FP_Div(0x8000000, pThis->m68_rate << 16);
    pThis->m0_computedValue = pThis->mC_initialValue;
}

bool updateInterpolator(s_battleEngine::s_27C* pThis)
{
    if (pThis->m60_currentStep > 0x7ffffff)
    {
        pThis->m60_currentStep = 0x8000000;
        pThis->m64_stepSize = 0;
        pThis->m0_computedValue = pThis->m24_targetValue;
        return true;
    }
    else
    {
        pThis->m0_computedValue = pThis->m18_middle + pThis->mC_initialValue - MTH_Mul(getCos(pThis->m60_currentStep.getInteger()), pThis->m18_middle);
        pThis->m60_currentStep += pThis->m64_stepSize;
        return false;
    }
}

void stepInterpolator(s_battleEngine::s_27C* pThis)
{
    pThis->m30 = pThis->m48 + pThis->m3C - -MTH_Mul(getCos(pThis->m60_currentStep.getInteger()), pThis->m48);
}

void battleEngine_UpdateSub7Sub0Sub1(s_battleEngine* pThis)
{
    if (!BattleEngineSub0_UpdateSub0()) {
        gBattleManager->m10_battleOverlay->m4_battleEngine->m190[0]++;
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m200_suppressBattleInputs = 0;
    }

    switch (pThis->m38C_battleMode)
    {
    case eBattleModes::m6_dragonMoving:
        break;
    case eBattleModes::mA:
        assert(0);
        break;
    case eBattleModes::mB_enemyMovingDragon:
        assert(0);
        break;

    default:
        return;
    }

    gBattleManager->m10_battleOverlay->m4_battleEngine->m190[0]++;

    switch (pThis->m38D_battleIntroStatus)
    {
    case 0: // init dragon move
        {
            pThis->m38D_battleIntroStatus++;

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
                    stackx28[1] += 0x4000000;
                    pcVar15 = 3;
                    break;
                default:
                    assert(0);
                }
            case 2:
                switch (pThis->m22E_dragonMoveDirection)
                {
                case 1:
                    stackx40[1] = 0xC000000;
                    stackx28[1] += 0x4000000;
                    pcVar15 = 3;
                    break;
                case 2:
                    stackx40[1] = 0;
                    stackx28[1] -= 0x4000000;
                    pcVar15 = 0;
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

            createBattleIntroTaskSub0();
        }
        break;
    case 1:
        // dragon moving
        if (updateInterpolator(&pThis->m2E8_dragonMovementInterpolator2))
        {
            battleEngine_UpdateSub7Sub0Sub3Sub0(pThis, pThis->m440_battleDirectionAngle, 0);
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m800 = 1;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m4000000 = 1;
            pThis->m38D_battleIntroStatus++; // go to state 2 (finished)
            pThis->m384_battleIntroDelay = 0;
            pThis->m398[0] = 0;
            pThis->m390[2] = 0;
        }

        stepInterpolator(&pThis->m27C_dragonMovementInterpolator1);
        pThis->m43C = pThis->m27C_dragonMovementInterpolator1.m30[0];
        pThis->m440_battleDirectionAngle = pThis->m27C_dragonMovementInterpolator1.m30[1];

        stepInterpolator(&pThis->m2E8_dragonMovementInterpolator2);
        gBattleManager->m10_battleOverlay->m18_dragon->m74_targetRotation[1] = pThis->m2E8_dragonMovementInterpolator2.m30[1];

        if (pThis->m22E_dragonMoveDirection == 1)
        {
            gBattleManager->m10_battleOverlay->m8_gridTask->mB4_cameraRotation[1] = MTH_Mul(getSin(pThis->m2E8_dragonMovementInterpolator2.m60_currentStep), 0xaaaaaa);
        }
        else
        {
            gBattleManager->m10_battleOverlay->m8_gridTask->mB4_cameraRotation[1] = MTH_Mul(getSin(pThis->m2E8_dragonMovementInterpolator2.m60_currentStep), -0xaaaaaa);
        }
        gBattleManager->m10_battleOverlay->m8_gridTask->m64_cameraRotationTarget[1] = gBattleManager->m10_battleOverlay->m8_gridTask->mB4_cameraRotation[1];

        pThis->m270_enemyAltitude[1] = pThis->m2E8_dragonMovementInterpolator2.m0_computedValue[1];

        updateInterpolator(&pThis->m27C_dragonMovementInterpolator1);

        pThis->m104_dragonPosition[1] = pThis->m27C_dragonMovementInterpolator1.m0_computedValue[1] + pThis->m35C_cameraAltitudeMinMax[1];

        pThis->m164[0] = pThis->m27C_dragonMovementInterpolator1.m0_computedValue[0];
        pThis->m164[2] = pThis->m27C_dragonMovementInterpolator1.m0_computedValue[2];
        break;
    case 2: // dragon move is over
        if (pThis->m384_battleIntroDelay++ < 5)
            return;

        battleEngine_SetBattleMode16();
        break;
    default:
        assert(0);
    }
}

void battleEngine_UpdateSub7Sub0Sub2(s_battleEngine* pThis)
{
    if ((battleEngine_UpdateSub7Sub0Sub2Sub0() != 0) && !gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m400000 && gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x1F])
    {
        assert(0);
    }
}

void battleEngine_UpdateSub9(s_battleEngine* pThis)
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
            addTraceLog(pThis->mC_battleCenter, "C");
            addTraceLog(pThis->m104_dragonPosition, "m104_dragonStartPosition");
        }

        computeVectorAngles(pThis->mC_battleCenter - pThis->m104_dragonPosition, aiStack52);
        pThis->m43C = aiStack52[0];
    }

    if (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m800)
    {
        sVec3_FP iStack88 = pThis->m104_dragonPosition + MTH_Mul(0x8000, pThis->mC_battleCenter - pThis->m104_dragonPosition);

        sVec2_FP aiStack96;
        // TODO: there is some dead code here?
        computeVectorAngles(iStack88 - gBattleManager->m10_battleOverlay->m8_gridTask->m34_cameraPosition, aiStack96);
        pThis->m1C8 = aiStack96[0];
        pThis->m1CC = aiStack96[1];

        gBattleManager->m10_battleOverlay->m8_gridTask->m64_cameraRotationTarget[0] = aiStack96[0];
        gBattleManager->m10_battleOverlay->m8_gridTask->m64_cameraRotationTarget[1] = aiStack96[1];
    }
    else
    {
        battleEngine_InitSub8Sub0(aiStack52);
    }
    battleEngine_InitSub3Sub0(pThis);
}

void battleEngine_UpdateSub7Sub0Sub3Sub0(s_battleEngine* pThis, fixedPoint uParm2, s32 r6)
{
    uParm2 = uParm2.normalized();

    if (((int)uParm2 < -0x2000000) || (0x1ffffff < (int)uParm2)) {
        if (((int)uParm2 < -0x6000000) || (-0x2000001 < (int)uParm2)) {
            if (((int)uParm2 < 0x2000000) || (0x6000000 <= (int)uParm2))
            {
                if (pThis->m22C_dragonCurrentQuadrant != 0)
                {
                    gBattleManager->m10_battleOverlay->m18_dragon->m88 |= 0x100;
                    pThis->m22C_dragonCurrentQuadrant = pThis->m22D_dragonPreviousQuadrant;
                    gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2 = 1;
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
                    pThis->m22C_dragonCurrentQuadrant = pThis->m22D_dragonPreviousQuadrant;
                    gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2 = 1;
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
                pThis->m22C_dragonCurrentQuadrant = pThis->m22D_dragonPreviousQuadrant;
                gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2 = 1;
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
            pThis->m22C_dragonCurrentQuadrant = pThis->m22D_dragonPreviousQuadrant;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2 = 1;
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

void battleEngine_UpdateSub7Sub0Sub3(s_battleEngine* pThis)
{
    if (battleEngine_UpdateSub7Sub0Sub2Sub0())
    {
        battleEngine_UpdateSub7Sub0Sub3Sub0(pThis, pThis->m440_battleDirectionAngle, 0);
    }
}

void battleCreateCinematicBars(s_battleEngine* pThis)
{
    FunctionUnimplemented();
}

struct sEnemyAttackCamera : public s_workAreaTemplate<sEnemyAttackCamera>
{
    // size: 5C
};

void sEnemyAttackCamera_init(sEnemyAttackCamera* pThis)
{
    FunctionUnimplemented();
}

void sEnemyAttackCamera_update(sEnemyAttackCamera* pThis)
{
    FunctionUnimplemented();
}

void battleEngine_updateBattleMode_8_playAttackCamera(s_battleEngine* pThis)
{
    switch (pThis->m38D_battleIntroStatus)
    {
    case 0:
        battleCreateCinematicBars(pThis);
        pThis->m38D_battleIntroStatus++;

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
        if (++pThis->m384_battleIntroDelay > 4)
        {
            pThis->m384_battleIntroDelay = 0;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2000 = 1;
            pThis->m38D_battleIntroStatus++;
        }
        break;
    case 2:
        if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100)
        {
            pThis->m38D_battleIntroStatus++;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m40 = 1;
        }
        break;
    case 3:
        if (++pThis->m384_battleIntroDelay > 0xF)
        {
            pThis->m384_battleIntroDelay = 0;
            pThis->m38D_battleIntroStatus++;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m1000 = 1;
        }
        break;
    case 4:
        if (++pThis->m384_battleIntroDelay > 0xF)
        {
            pThis->m384_battleIntroDelay = 0;
            pThis->m38D_battleIntroStatus++;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m1000 = 0;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m20_battleIntroRunning = 0;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2 = 1;
        }
        break;
    case 5:
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m10 = 0;
        pThis->m38D_battleIntroStatus++;
        break;
    case 6:
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m80000_hideBattleHUD = 0;
        pThis->m38D_battleIntroStatus++;
        break;
    case 7:
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
    switch (pThis->m38D_battleIntroStatus)
    {
    case 0:
        battleCreateCinematicBars(pThis);
        pThis->m3D0 = 0;
        pThis->m38D_battleIntroStatus++;
        return;
    case 1:
        if (4 < pThis->m384_battleIntroDelay++)
        {
            pThis->m384_battleIntroDelay = 0;
            pThis->m38D_battleIntroStatus++;
            pThis->m3D0 = createBattleIntroTask(pThis);

            g_fadeControls.m_4D = 6;
            if (g_fadeControls.m_4C < g_fadeControls.m_4D)
            {
                vdp2Controls.m20_registers[0].m112_CLOFSL = 0;
                vdp2Controls.m20_registers[1].m112_CLOFSL = 0;
            }

            fadePalette(&g_fadeControls.m0_fade0, 0xC210, 0xC210, 1);
            g_fadeControls.m_4D = 5;
        }
        return;
    case 2:
        if (pThis->m3D0 == nullptr)
        {
            pThis->m384_battleIntroDelay = 0;
            pThis->m38D_battleIntroStatus++;

            g_fadeControls.m_4D = 6;
            if (g_fadeControls.m_4C < g_fadeControls.m_4D)
            {
                vdp2Controls.m20_registers[0].m112_CLOFSL = 0;
                vdp2Controls.m20_registers[1].m112_CLOFSL = 0;
            }

            fadePalette(&g_fadeControls.m0_fade0, 0xC210, 0xC210, 1);
            g_fadeControls.m_4D = 5;

            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m20_battleIntroRunning = 0;
        }
        return;
    case 3:
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m80000_hideBattleHUD = 0;
        pThis->m38D_battleIntroStatus++;
        return;
    case 4:
        if (pThis->m384_battleIntroDelay++ > 4)
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

void updateBattleIntro(s_battleEngine* pThis)
{
    if (BattleEngineSub0_UpdateSub0() == 0)
        return;

    switch (pThis->m38C_battleMode)
    {
    case eBattleModes::m6_dragonMoving:
        break;
    case eBattleModes::m8_playAttackCamera:
        battleEngine_updateBattleMode_8_playAttackCamera(pThis);
        break;
    case eBattleModes::mE_battleIntro:
        battleEngine_updateBattleMode_E_battleIntro(pThis);
        break;
    case eBattleModes::m10:
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
    default:
        assert(0);
        break;
    }
}

void battleEngine_UpdateSub7Sub0Sub5(s_battleEngine* pThis)
{
    if ((pThis->m388 & 4) == 0)
    {
        if (gBattleManager->m10_battleOverlay->m4_battleEngine->m498 &&
            battleEngine_UpdateSub7Sub0Sub2Sub0() &&
            (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m80000_hideBattleHUD)
            ) {
            if ((pThis->m3B4.m16_combo > 0) && (graphicEngineStatus.m4514.mD8_buttonConfig[2][1] & graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown))
            {
                assert(0);
            }
            if ((pThis->m388 & 0x200) == 0)
            {
                if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & 8)
                {
                    assert(0);
                }
                if (graphicEngineStatus.m4514.mD8_buttonConfig[2][1] & graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown)
                {
                    playSoundEffect(5);
                }
            }
        }
    }
}

void battleEngine_UpdateSub7Sub0Sub6(s_battleEngine* pThis)
{
    if ((gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m4000) && (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m10 & 1))
    {
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m4000 = 0;
    }
    if (((pThis->m388 & 2) == 0) &&
        (gBattleManager->m10_battleOverlay->m4_battleEngine->m498) &&
        (battleEngine_UpdateSub7Sub0Sub2Sub0())
        )
    {
        if ((pThis->m3B4.m16_combo > 0) && !gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m4000 && (graphicEngineStatus.m4514.mD8_buttonConfig[2][0] & graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown))
        {
            assert(0);
        }
        if (graphicEngineStatus.m4514.mD8_buttonConfig[2][0] & graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown)
        {
            playSoundEffect(5);
        }
    }
}

void battleEngine_UpdateSub7Sub0Sub7(s_battleEngine* pThis)
{
    if ((pThis->m388 & 1) == 0)
    {
        if ((0 < gBattleManager->m10_battleOverlay->m4_battleEngine->m498) &&
            (battleEngine_UpdateSub7Sub0Sub2Sub0() != 0)) {
            if ((pThis->m3B4.m16_combo < '\x01') ||
                (((graphicEngineStatus.m4514.mD8_buttonConfig[2][2] & graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown) == 0 ||
                ((gBattleManager->m10_battleOverlay->mC->m20A_numActiveEntries) < 1))))
            {
                if (graphicEngineStatus.m4514.mD8_buttonConfig[2][2] & graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown)
                {
                    playSoundEffect(5);
                }
            }
            else
            {
                gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m40 = 0;
                battleEngine_SetBattleMode(eBattleModes::m0);
                pThis->m184 = 0;
            }
        }
    }
}

s32 battleEngine_UpdateSub7Sub4()
{
    if (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m200_suppressBattleInputs)
        return 1;
    return 0;
}

s32 getDragonAgilityForMove(s16 param)
{
    return MTH_Mul(0x10000, MTH_Mul(param * 0x10000, 0x18000 - MTH_Mul(0x10000, FP_Div(mainGameState.gameStats.dragonAgl, 200)))+ 0x8000).getInteger();
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

void battleEngine_UpdateSub7(s_battleEngine* pThis)
{
    battleEngine_UpdateSub8(pThis);
    if (battleEngine_UpdateSub7Sub0() == 1)
        return;
    battleEngine_UpdateSub7Sub0Sub1(pThis);
    battleEngine_UpdateSub7Sub0Sub2(pThis);
    battleEngine_UpdateSub9(pThis);
    battleEngine_UpdateSub7Sub0Sub3(pThis);
    updateBattleIntro(pThis);
    battleEngine_UpdateSub7Sub0Sub5(pThis);
    battleEngine_UpdateSub7Sub0Sub6(pThis);
    battleEngine_UpdateSub7Sub0Sub7(pThis);

    if (gBattleManager->m10_battleOverlay->m4_battleEngine->m498 < 1)
        return;

    if (!battleEngine_UpdateSub7Sub4())
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

                if ((gBattleManager->m10_battleOverlay->m18_dragon->m1C0 & 8) == 0)
                {
                    assert(readSaturnS8(gCurrentBattleOverlay->getSaturnPtr(0x60A9281) + gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant * 4) == quadrantRotationTable[gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant][1]);

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

                if ((gBattleManager->m10_battleOverlay->m18_dragon->m1C0 & 8) == 0)
                {
                    assert(readSaturnS8(gCurrentBattleOverlay->getSaturnPtr(0x60A9282) + gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant * 4) == quadrantRotationTable[gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant][2]);

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
                (((gBattleManager->m10_battleOverlay->m18_dragon->m1C0 & 8) != 0) | (bVar1)) &&
                (((graphicEngineStatus.m4514.mD8_buttonConfig[2][7] | graphicEngineStatus.m4514.mD8_buttonConfig[2][6]) & graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown) != 0)
                )
            {
                playSoundEffect(5);
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
                    pThis->m1B8_dragonPitch = 0x1000;
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
            assert(0); 
        default:
            assert(0);
        }
    }
    else
    {
        assert(0);
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
        pThis->m398[0] = 0;
        pThis->m398[1] = 0;
        pThis->m3A4_prelockMode[0] = 2;
        pThis->m3A4_prelockMode[1] = 0x10;

        pThis->m188_flags.m8000 = 1;

        fieldPaletteTaskInitSub0Sub2();

        pThis->m3CC = createBattleEngineSub0(pThis);
        pThis->m3B1 = 0;

        battleEngine_UpdateSub1(1);
        battleEngine_UpdateSub2(pThis);
        battleEngine_CreateHud1(dramAllocatorEnd[0].mC_buffer);
        battleEngine_CreateRadar(dramAllocatorEnd[0].mC_buffer);
        battleEngine_UpdateSub5();

        battleEngine_SetBattleMode(eBattleModes::mE_battleIntro);
        return;
    case 1: // running
        battleEngine_UpdateSub7(pThis);
        battleEngine_UpdateSub5();
        return;
    case 2:
        battleEngine_UpdateSub8(pThis);
        battleEngine_UpdateSub9(pThis);
        battleEngine_UpdateSub5();
        return;
    case 3:
        return;
    case 4:
        battleEngine_UpdateSub8(pThis);
        battleEngine_UpdateSub9(pThis);
        battleEngine_UpdateSub5();
        return;
    default:
        assert(0);
        break;
    }
}

void battleEngine_Draw(s_battleEngine* pThis)
{
    // seems to be all debug input related
    FunctionUnimplemented();
}

void battleEngine_Delete(s_battleEngine* pThis)
{
    FunctionUnimplemented();
}

s32 BattleEngineSub0_UpdateSub0()
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
    return createSiblingTaskWithArg<s_battleEngine>(parent, battleData, &definition);
}
