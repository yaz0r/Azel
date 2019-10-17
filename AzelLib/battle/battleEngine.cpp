#include "PDS.h"
#include "battleEngine.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleGrid.h"
#include "battleEngineSub0.h"
#include "battleOverlay_20.h"
#include "battleOverlay_C.h"
#include "battleDebug.h"
#include "battleDragon.h"
#include "battleIntro.h"
#include "battleEngineSub1.h"
#include "kernel/debug/trace.h"

#include "BTL_A3/BTL_A3.h"

void fieldPaletteTaskInitSub0Sub2();

void initBattleEngineArray()
{
    s_battleEngine* pBattleEngine = getBattleManager()->m10_battleOverlay->m4_battleEngine;

    pBattleEngine->m498 = 0;
    for (int i = 0; i < 0x7F; i++)
    {
        pBattleEngine->m49C[i].m0 = -1;
        pBattleEngine->m49C[i].m4 = 0;
        pBattleEngine->m49C[i].m8 = -1;
    }
}

void battleEngine_InitSub0(s_battleEngine* pThis)
{
    switch (pThis->m22C_battleDirection)
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
        pThis->m470 = pThis->m45C[2];
    }
    else
    {
        pThis->m470 = pThis->m45C[0];
    }

    if (var1 < 1)
    {
        pThis->m46C = pThis->m45C[3];
    }
    else
    {
        pThis->m46C = pThis->m45C[1];
    }
}

void battleEngine_InitSub2(s_battleEngine* pThis)
{
    battleEngine_InitSub2Sub0(pThis);
    pThis->mC = pThis->m234 + pThis->m24C;
    pThis->m6C[0] = MTH_Mul(pThis->m46C, getSin(pThis->m440_battleDirectionAngle.getInteger() & 0xFFF));
    pThis->m6C[2] = MTH_Mul(pThis->m46C, getCos(pThis->m440_battleDirectionAngle.getInteger() & 0xFFF));

    pThis->m104_dragonStartPosition[0] = pThis->m234[0] + pThis->m6C[0];
    pThis->m104_dragonStartPosition[2] = pThis->m234[2] + pThis->m6C[2];

    sVec3_FP temp;
    generateCameraMatrixSub1(pThis->mC - pThis->m104_dragonStartPosition, temp);

    pThis->m43C = temp[0];
}

void battleEngine_InitSub3Sub0(s_battleEngine* pThis)
{
    if (pThis->m104_dragonStartPosition[1] < pThis->m354[1]) {
        pThis->m104_dragonStartPosition[1] = pThis->m354[1];
    }
    else if (pThis->m354[0] < pThis->m104_dragonStartPosition[1])
    {
        pThis->m104_dragonStartPosition[1] = pThis->m354[0];
    }
}

void battleEngine_InitSub3(s_battleEngine* pThis)
{
    pThis->m104_dragonStartPosition[1] = pThis->m364[getBattleManager()->m10_battleOverlay->m4_battleEngine->m22C_battleDirection] + pThis->m354[3];
    pThis->m270[1] = pThis->m374[getBattleManager()->m10_battleOverlay->m4_battleEngine->m22C_battleDirection];

    battleEngine_InitSub3Sub0(pThis);
}

void battleEngine_InitSub5(p_workArea parent)
{
    FunctionUnimplemented();
}

void battleEngine_InitSub6(sVec3_FP* pData)
{
    getBattleManager()->m10_battleOverlay->m4_battleEngine->m3D4 = pData;
    getBattleManager()->m10_battleOverlay->m8_gridTask->m1B8_pCameraTranslationSource = pData;
}

void battleEngine_InitSub7(sVec3_FP* pData)
{
    getBattleManager()->m10_battleOverlay->m4_battleEngine->m3D8 = pData;
}

void createBattleDisplayCommandHelpTask(p_workArea parent, sSaturnPtr data)
{
    FunctionUnimplemented();
}

void battleEngine_InitSub8Sub0(sVec2_FP& param)
{
    s_battleEngine* pBattleEngine = getBattleManager()->m10_battleOverlay->m4_battleEngine;
    s_battleGrid* pBattleGrid = getBattleManager()->m10_battleOverlay->m8_gridTask;

    if (isTraceEnabled())
    {
        addTraceLog(*pBattleEngine->m3D8, "3D8");
        addTraceLog(*pBattleEngine->m3D4, "3D4");
    }

    sVec3_FP delta = *pBattleEngine->m3D8 - *pBattleEngine->m3D4;
    generateCameraMatrixSub1(delta, param);
    
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
    s_battleGrid* pBattleGrid = getBattleManager()->m10_battleOverlay->m8_gridTask;
    pBattleGrid->mB4_cameraRotation[0] = param[0];
    pBattleGrid->mB4_cameraRotation[1] = param[1];
}

void battleEngine_InitSub8()
{
    s_battleGrid* pBattleGrid = getBattleManager()->m10_battleOverlay->m8_gridTask;
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
    FunctionUnimplemented();
}

void battleEngine_Init(s_battleEngine* pThis, sSaturnPtr overlayBattleData)
{
    getBattleManager()->m10_battleOverlay->m4_battleEngine = pThis;
    pThis->m3A8_overlayBattledata = overlayBattleData;
    pThis->m3B0_subBattleId = getBattleManager()->m6_subBattleId;

    sSaturnPtr battleData_4 = readSaturnEA(overlayBattleData + pThis->m3B0_subBattleId * 0x20 + 4);
    pThis->m3AC = readSaturnEA(overlayBattleData + pThis->m3B0_subBattleId * 0x20 );

    createInBattleDebugTask(pThis);
    createBattleDisplayCommandHelpTask(pThis, readSaturnEA(overlayBattleData + pThis->m3B0_subBattleId * 0x20 + 0xC));
    initBattleEngineArray();

    int var6 = performModulo2(100, randomNumber()) % 0xFF;
    int cStack60;
    int uVar4;
    int uVar3;

    sSaturnPtr pData = readSaturnEA(gCurrentBattleOverlay->getBattleEngineInitData() + getBattleManager()->m4 * 4);
    if (pData.isNull())
    {
        cStack60 = 0x64;
        uVar4 = 0;
        uVar3 = 0;
    }
    else
    {
        cStack60 = readSaturnS8(pData + getBattleManager()->m8 * 0x10 + 4);
        uVar3 = readSaturnS8(pData + getBattleManager()->m8 * 0x10 + 5);
        uVar4 = readSaturnS8(pData + getBattleManager()->m8 * 0x10 + 6);
    }

    if (var6 < cStack60)
    {
        pThis->m22C_battleDirection = 0;
    }
    else
    {
        uVar3 += cStack60;
        if (var6 < uVar3)
        {
            pThis->m22C_battleDirection = 1;
        }
        else
        {
            if (var6 < uVar3 + uVar4)
            {
                pThis->m22C_battleDirection = 2;
            }
            else
            {
                pThis->m22C_battleDirection = 3;
            }
        }
    }

    pThis->m22D_originalBattleDirection = pThis->m22C_battleDirection;

    if (pData.isNull())
    {
        pThis->m3CA = 0;
    }
    else
    {
        readSaturnS8(pData + getBattleManager()->m8 * 0x10 + 0xC);
    }

    pThis->m270.zeroize();
    pThis->m258.zeroize();
    pThis->m1A0 = readSaturnVec3(battleData_4 + 0x4C);
    pThis->m264 = readSaturnVec3(battleData_4 + 0x00);

    pThis->m264[1] = readSaturnFP(battleData_4 + 0x18);

    pThis->m234 = pThis->m264 + pThis->m270;

    for (int i = 0; i < 4; i++)
    {
        pThis->m354[i] = readSaturnS32(battleData_4 + 0xC + i * 4);
    }
    pThis->m230 = readSaturnS8(battleData_4 + 0x58);
    for (int i = 0; i < 4; i++)
    {
        pThis->m45C[i] = readSaturnS32(battleData_4 + 0x1C + i * 4);
    }
    pThis->m24C.zeroize();
    for (int i = 0; i < 4; i++)
    {
        pThis->m374[i] = readSaturnS32(battleData_4 + 0x2C + i * 4);
    }
    for (int i = 0; i < 4; i++)
    {
        pThis->m364[i] = readSaturnS32(battleData_4 + 0x3C + i * 4);
    }

    pThis->m1D0 = 0x111111;
    pThis->m1D4 = 0x111111;

    pThis->m390[0] = 0;
    pThis->m390[1] = 2;
    pThis->m390[2] = 0;

    pThis->m3A2 = 0;

    pThis->m3B4 = 0x3C0000;
    pThis->m3B8 = 0x10000;
    pThis->m3BC = 0;

    initMatrixToIdentity(&pThis->m1F0);

    pThis->m434 = 0xA000;

    createBattleEnvironmentGridTask(pThis);

    pThis->m264 += pThis->m1A0;
    pThis->m234 = pThis->m270 + pThis->m264;

    pThis->m240 = pThis->m270 + pThis->m258 + pThis->m264;

    battleEngine_InitSub0(pThis);
    battleEngine_InitSub1(pThis);
    battleEngine_InitSub2(pThis);
    battleEngine_InitSub3(pThis);
    battleEngine_createDragonTask(pThis);
    battleEngine_InitSub5(pThis);
    battleEngine_InitSub6(&getBattleManager()->m10_battleOverlay->m8_gridTask->m34_cameraPosition);
    battleEngine_InitSub7(&pThis->mC);
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
        mainGameState.gameStats.mE_shotPower = 60;
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

    getBattleManager()->m10_battleOverlay->m4_battleEngine->m3CC->m0 = setDividend(getBattleManager()->m10_battleOverlay->m4_battleEngine->m3CC->m2 * 65536, param_1 * 65536, 0x640000);
}

void battleEngine_UpdateSub1(int bitMask)
{
    s_battleEngine* pBattleEngine = getBattleManager()->m10_battleOverlay->m4_battleEngine;
    int iVar7 = 0;
    while (readSaturnU32(pBattleEngine->m3AC + iVar7 * 0xC) != 0)
    {
        if (bitMask & readSaturnU32(pBattleEngine->m3AC + iVar7 * 0xC + 8))
        {
            sSaturnPtr functionPointer = readSaturnEA(pBattleEngine->m3AC + iVar7 * 0xC);
            gCurrentBattleOverlay->invoke(functionPointer, getBattleManager()->m10_battleOverlay->m18_dragon, readSaturnU32(pBattleEngine->m3AC + iVar7 * 0xC + 4), iVar7);
            pBattleEngine->m3B1++;

            sSaturnPtr pData = readSaturnEA(gCurrentBattleOverlay->getBattleEngineInitData() + getBattleManager()->m4 * 4);
            if (readSaturnU32(pData) == 0)
            {
                getBattleManager()->m10_battleOverlay->m4_battleEngine->m3CC->m2 = 150;
                battleEngine_UpdateSub1Sub0(0);
            }
            else
            {
                getBattleManager()->m10_battleOverlay->m4_battleEngine->m3CC->m2 = readSaturnS16(pData + getBattleManager()->m8 * 0x10 + 2);
                battleEngine_UpdateSub1Sub0(readSaturnS8(pData + getBattleManager()->m8 * 0x10 + 8 + getBattleManager()->m10_battleOverlay->m4_battleEngine->m2CC));
            }
        }
        iVar7++;
    }
}

void battleEngine_UpdateSub4(struct npcFileDeleter*)
{
    FunctionUnimplemented();
}

s32 s_battleDragon_InitSub0()
{
    s8 bVar1 = getBattleManager()->m10_battleOverlay->m4_battleEngine->m230;

    if ((((bVar1 != 0) && (bVar1 != 1)) && (bVar1 != 2)) &&
        (((bVar1 != 3 && (bVar1 != 8)) && (bVar1 != 10)))) {
        return 0;
    }
    return 1;
}


void battleEngine_UpdateSub5()
{
    s_battleDragon* pDragon = getBattleManager()->m10_battleOverlay->m18_dragon;

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

void battleEngine_UpdateSub10(int param)
{
    s_battleEngine* pBattleEngine = getBattleManager()->m10_battleOverlay->m4_battleEngine;
    pBattleEngine->m38C_battleIntroType = param;
    pBattleEngine->m188_flags |= 0x80;
    pBattleEngine->m188_flags &= ~0x100;
    pBattleEngine->m188_flags &= ~0x2000;
    pBattleEngine->m188_flags |= 0x10;
    pBattleEngine->m188_flags |= 0x20;
    pBattleEngine->m188_flags |= 0x200;
    pBattleEngine->m38D_battleIntroStatus = 0;
    pBattleEngine->m384_battleIntroDelay = 0;
    pBattleEngine->m386 = 0;
    pBattleEngine->m184 = 0;
    pBattleEngine->m3E8.zeroize();
    battleEngine_UpdateSub6(pBattleEngine);
}

void battleEngine_UpdateSub8(s_battleEngine* pThis)
{
    if (getBattleManager()->m10_battleOverlay->m10_inBattleDebug->mFlags[0x16])
    {
        assert(0);
    }

    pThis->m264 += pThis->m1A0;
    pThis->m234 = pThis->m270 + pThis->m264;
    pThis->m240 = pThis->m270 + pThis->m258 + pThis->m264;
}

s32 battleEngine_UpdateSub7Sub0Sub0()
{
    if ((getBattleManager()->m10_battleOverlay->m4_battleEngine->m188_flags & 0x20) == 0)
        return 1;
    return 0;
}

s32 battleEngine_UpdateSub7Sub0Sub2Sub0()
{
    if ((getBattleManager()->m10_battleOverlay->m4_battleEngine->m188_flags & 0x10) == 0)
        return 1;
    return 0;
}

void battleEngine_UpdateSub7Sub1()
{
    s_battleGrid* pGrid = getBattleManager()->m10_battleOverlay->m8_gridTask;
    if (getBattleManager()->m10_battleOverlay->m4_battleEngine->m188_flags & 0x20000)
    {
        assert(0);
        sVec3_FP local_1c;
        //battleEngine_UpdateSub7Sub1Sub0(pGrid->m280_lightAngle1, &local_1c);
        pGrid->m1CC_lightColor = pGrid->m1D8_newLightColor;
    }
}

void battleEngine_UpdateSub7Sub2()
{
    getBattleManager()->m10_battleOverlay->m8_gridTask->m1 = 0;

    getBattleManager()->m10_battleOverlay->m8_gridTask->m134_desiredCameraPosition = readSaturnVec3(gCurrentBattleOverlay->getSaturnPtr(0x60ac478) + 0x24 * getBattleManager()->m10_battleOverlay->m8_gridTask->m1);
    getBattleManager()->m10_battleOverlay->m8_gridTask->m140_desiredCameraTarget = readSaturnVec3(gCurrentBattleOverlay->getSaturnPtr(0x60ac484) + 0x24 * getBattleManager()->m10_battleOverlay->m8_gridTask->m1);
}

void battleEngine_UpdateSub7Sub3Sub0()
{
    getBattleManager()->m10_battleOverlay->m8_gridTask->mB4_cameraRotation[2] = 0;
    getBattleManager()->m10_battleOverlay->m8_gridTask->m64_cameraRotationTarget[2] = 0;
}

void battleEngine_UpdateSub7Sub3()
{
    s_battleEngine* pBattleEngine = getBattleManager()->m10_battleOverlay->m4_battleEngine;
    s_battleGrid* pGrid = getBattleManager()->m10_battleOverlay->m8_gridTask;
    pBattleEngine->m188_flags &= ~0x800;

    if (getBattleManager()->m10_battleOverlay->m10_inBattleDebug->mFlags[0xB] == 0)
    {
        if (isTraceEnabled())
        {
            addTraceLog(pGrid->m1C, "pGrid->m1C");
            addTraceLog(pGrid->m28, "pGrid->m28");
            addTraceLog(pBattleEngine->m104_dragonStartPosition, "pBattleEngine->m104_dragonStartPosition");
        }
        pGrid->m34_cameraPosition = pBattleEngine->m104_dragonStartPosition + pGrid->m1C + pGrid->m28;
        battleEngine_InitSub6(&pGrid->m34_cameraPosition);
        pBattleEngine->m3D8 = &pBattleEngine->mC;
        battleEngine_UpdateSub7Sub3Sub0();
    }
}

void battleEngine_UpdateSub7Sub3Sub4(p_workArea parent)
{
    FunctionUnimplemented();
}

s32 battleEngine_UpdateSub7Sub0()
{
    s_battleEngine* pBattleEngine = getBattleManager()->m10_battleOverlay->m4_battleEngine;
    if (pBattleEngine->m3B2 < 1)
    {
        if (battleEngine_UpdateSub7Sub0Sub0() == 0)
            return 0;

        battleEngine_UpdateSub7Sub1();
        pBattleEngine->m1B8 = 0;
        pBattleEngine->m1BC = 0;
        pBattleEngine->m3B2 = 0;

        clearVdp2TextMemory();
        fieldPaletteTaskInitSub0Sub2();

        pBattleEngine->m18C_status = 4;
        pBattleEngine->m188_flags &= ~0x80000;
        if (0 < mainGameState.gameStats.currentHP)
        {
            battleEngine_UpdateSub7Sub2();
            battleEngine_UpdateSub7Sub3();

            if ((getBattleManager()->m10_battleOverlay->m4_battleEngine->m188_flags & 0x10000) == 0)
            {
                getBattleManager()->mE = 0;
            }
            else
            {
                getBattleManager()->mE = 3;
            }

            if ((((pBattleEngine->m230 != 1) && (pBattleEngine->m230 != 3)) && (pBattleEngine->m230 != 5)) && ((pBattleEngine->m230 != 7 && (pBattleEngine->m230 != 8))))
            {
                pBattleEngine->m188_flags |= 8;
                battleEngine_UpdateSub7Sub3Sub4(pBattleEngine);
                return 1;
            }

            if (getBattleManager()->m10_battleOverlay->m4_battleEngine->m188_flags & 0x10000)
            {
                assert(0);
            }
            assert(0);
        }
        assert(0);
    }
    assert(0);
}

void battleEngine_UpdateSub7Sub0Sub1(s_battleEngine* pThis)
{
    s_battleEngine* pBattleEngine = getBattleManager()->m10_battleOverlay->m4_battleEngine;
    if (pBattleEngine->m188_flags & 0x20000)
    {
        assert(0);
    }
}

void battleEngine_UpdateSub7Sub0Sub2(s_battleEngine* pThis)
{
    if (battleEngine_UpdateSub7Sub0Sub2Sub0() != 0)
    {
        assert(0);
    }
}

void battleEngine_UpdateSub9(s_battleEngine* pThis)
{
    sVec2_FP aiStack52;

    if ((getBattleManager()->m10_battleOverlay->m4_battleEngine->m188_flags & 0x200000) == 0)
    {
        battleEngine_InitSub2Sub0(pThis);
        pThis->mC = pThis->m234 + pThis->m24C;
        pThis->m6C[0] = MTH_Mul(pThis->m46C, getSin(pThis->m440_battleDirectionAngle.getInteger()));
        pThis->m6C[2] = MTH_Mul(pThis->m470, getCos(pThis->m440_battleDirectionAngle.getInteger()));
        pThis->m104_dragonStartPosition[0] = pThis->m234[0] + pThis->m6C[0];
        pThis->m104_dragonStartPosition[2] = pThis->m234[2] + pThis->m6C[2];

        if (isTraceEnabled())
        {
            addTraceLog(pThis->mC, "C");
            addTraceLog(pThis->m104_dragonStartPosition, "m104_dragonStartPosition");
        }

        generateCameraMatrixSub1(pThis->mC - pThis->m104_dragonStartPosition, aiStack52);
        pThis->m43C = aiStack52[0];
    }

    if ((getBattleManager()->m10_battleOverlay->m4_battleEngine->m188_flags & 0x800) == 0)
    {
        sVec3_FP iStack88 = pThis->m104_dragonStartPosition + MTH_Mul(0x8000, pThis->mC - pThis->m104_dragonStartPosition);

        sVec2_FP aiStack96;
        // TODO: there is some dead code here?
        generateCameraMatrixSub1(iStack88 - getBattleManager()->m10_battleOverlay->m8_gridTask->m34_cameraPosition, aiStack96);
        pThis->m1C8 = aiStack96[0];
        pThis->m1CC = aiStack96[1];

        getBattleManager()->m10_battleOverlay->m8_gridTask->m64_cameraRotationTarget[0] = aiStack96[0];
        getBattleManager()->m10_battleOverlay->m8_gridTask->m64_cameraRotationTarget[1] = aiStack96[1];
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
                if (pThis->m22C_battleDirection != 0)
                {
                    getBattleManager()->m10_battleOverlay->m18_dragon->m88 |= 0x100;
                    pThis->m22C_battleDirection = pThis->m22D_originalBattleDirection;
                    getBattleManager()->m10_battleOverlay->m4_battleEngine->m188_flags |= 2;
                }
                if (r6 == 0)
                {
                    pThis->m22C_battleDirection = 2;
                }
                else
                {
                    pThis->m22C_battleDirection = 0;
                }
            }
            else
            {
                if (pThis->m22C_battleDirection != 3)
                {
                    getBattleManager()->m10_battleOverlay->m18_dragon->m88 |= 0x100;
                    pThis->m22C_battleDirection = pThis->m22D_originalBattleDirection;
                    getBattleManager()->m10_battleOverlay->m4_battleEngine->m188_flags |= 2;
                }
                if (r6 == 0)
                {
                    pThis->m22C_battleDirection = 1;
                }
                else
                {
                    pThis->m22C_battleDirection = 3;
                }
            }
        }
        else
        {
            if (pThis->m22C_battleDirection != 1)
            {
                getBattleManager()->m10_battleOverlay->m18_dragon->m88 |= 0x100;
                pThis->m22C_battleDirection = pThis->m22D_originalBattleDirection;
                getBattleManager()->m10_battleOverlay->m4_battleEngine->m188_flags |= 2;
            }
            if (r6 == 0)
            {
                pThis->m22C_battleDirection = 3;
            }
            else
            {
                pThis->m22C_battleDirection = 1;
            }

        }
    }
    else
    {
        if (pThis->m22C_battleDirection != 2)
        {
            getBattleManager()->m10_battleOverlay->m18_dragon->m88 |= 0x100;
            pThis->m22C_battleDirection = pThis->m22D_originalBattleDirection;
            getBattleManager()->m10_battleOverlay->m4_battleEngine->m188_flags |= 2;
        }
        if (r6 == 0)
        {
            pThis->m22C_battleDirection = 0;
        }
        else
        {
            pThis->m22C_battleDirection = 2;
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

void updateBattleIntro(s_battleEngine* pThis)
{
    if (BattleEngineSub0_UpdateSub0() == 0)
        return;

    switch (pThis->m38C_battleIntroType)
    {
    case 0xe:
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

                getBattleManager()->m10_battleOverlay->m4_battleEngine->m188_flags &= ~0x20;
            }
            return;
        case 3:
            getBattleManager()->m10_battleOverlay->m4_battleEngine->m188_flags &= ~0x80000;
            return;
        case 4:
            //06068542
            assert(0);
        default:
            assert(0);
            break;
        }
        assert(0);
        break;
    default:
        assert(0);
        break;
    }
}

void battleEngine_UpdateSub7Sub0Sub5(s_battleEngine* pThis)
{
    if ((pThis->m388 & 4) == 0)
    {
        if (getBattleManager()->m10_battleOverlay->m4_battleEngine->m498 &&
            battleEngine_UpdateSub7Sub0Sub2Sub0() &&
            ((getBattleManager()->m10_battleOverlay->m4_battleEngine->m188_flags & 0x80000) == 0)
            ) {
            assert(0);
        }
    }
}

void battleEngine_UpdateSub7Sub0Sub6(s_battleEngine* pThis)
{
    if ((getBattleManager()->m10_battleOverlay->m4_battleEngine->m188_flags & 0x4000) && (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m10 & 1))
    {
        getBattleManager()->m10_battleOverlay->m4_battleEngine->m188_flags &= ~0x4000;
    }
    if (((pThis->m388 & 2) == 0) &&
        (getBattleManager()->m10_battleOverlay->m4_battleEngine->m498) &&
        (battleEngine_UpdateSub7Sub0Sub2Sub0())
        )
    {
        assert(0);
    }
}

void battleEngine_UpdateSub7Sub0Sub7(s_battleEngine* pThis)
{
    if ((pThis->m388 & 1) == 0)
    {
        if ((0 < getBattleManager()->m10_battleOverlay->m4_battleEngine->m498) &&
            (battleEngine_UpdateSub7Sub0Sub2Sub0() != 0)) {
            if ((pThis->m3CA < '\x01') ||
                (((graphicEngineStatus.m4514.mD8_buttonConfig[2][2] & graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown) == 0 ||
                ((getBattleManager()->m10_battleOverlay->mC->m20A) < 1))))
            {
                if (graphicEngineStatus.m4514.mD8_buttonConfig[2][2] & graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown)
                {
                    playSoundEffect(5);
                }
            }
            else
            {
                getBattleManager()->m10_battleOverlay->m4_battleEngine->m188_flags &= ~0x40;
                battleEngine_UpdateSub10(0);
                pThis->m184 = 0;
            }
        }
    }
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

    if (getBattleManager()->m10_battleOverlay->m4_battleEngine->m498 < 1)
        return;

    FunctionUnimplemented();
    //assert(0);
}

void battleEngine_Update(s_battleEngine* pThis)
{
    switch (pThis->m18C_status)
    {
    case 0:
        pThis->m18[0] = 0;
        pThis->m18[1] = 0x8000000;
        pThis->m18[2] = 0;
        pThis->m18C_status++;

        pThis->m1E8[0] = 0x2000;
        pThis->m1E8[1] = 0x1000;
        pThis->m398[0] = 0;
        pThis->m398[1] = 0;
        pThis->m3A4[0] = 2;
        pThis->m3A4[1] = 0x10;

        pThis->m188_flags |= 0x8000;

        fieldPaletteTaskInitSub0Sub2();

        pThis->m3CC = createBattleEngineSub0(pThis);
        pThis->m3B1 = 0;

        battleEngine_UpdateSub1(1);
        battleEngine_UpdateSub2(pThis);
        createBattleOverlay_task20(dramAllocatorEnd[0].mC_buffer);
        battleEngine_UpdateSub4(dramAllocatorEnd[0].mC_buffer);
        battleEngine_UpdateSub5();

        battleEngine_UpdateSub10(0xE);
        return;
    case 1:
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
    FunctionUnimplemented();
}

void battleEngine_Delete(s_battleEngine* pThis)
{
    FunctionUnimplemented();
}

s32 BattleEngineSub0_UpdateSub0()
{
    if ((getBattleManager()->m10_battleOverlay->m4_battleEngine->m188_flags & 0x80) != 0)
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