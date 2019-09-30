#include "PDS.h"
#include "battleEngine.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleGrid.h"

#include "BTL_A3/BTL_A3.h"

void fieldPaletteTaskInitSub0Sub2();

struct s_battleEngineSub
{
    s8 m0;
    s32 m4;
    s32 m8;
    // size C?
};

struct s_battleEngine : public s_workAreaTemplateWithArgWithCopy<s_battleEngine, sSaturnPtr>
{
    sVec3_FP mC;
    sVec3_FP m18;
    sVec3_FP m6C;
    sVec3_FP m104;
    u32 m188_flags;
    s8 m18C_status;
    std::array<s32, 2> m190;
    sVec3_FP m1A0;
    sVec3_FP m1AC;
    fixedPoint m1D0;
    fixedPoint m1D4;
    int m1E8[2];
    sMatrix4x3 m1F0;
    s8 m22C;
    s8 m22D;
    s8 m230;
    sVec3_FP m234;
    sVec3_FP m240;
    sVec3_FP m24C;
    sVec3_FP m258;
    sVec3_FP m264;
    sVec3_FP m270;
    std::array<s32, 4> m354;
    std::array<s32, 4> m364;
    std::array<s32, 4> m374;
    std::array<s16, 3> m390;
    std::array<s16, 2> m398;
    s16 m3A2;
    std::array<s16, 2> m3A4;
    sSaturnPtr m3A8_overlayBattledata;
    sSaturnPtr m3AC;
    s8 m3B0_subBattleId;
    s8 m3B1;
    s32 m3B4;
    s32 m3B8;
    s32 m3BC;
    s8 m3CA;
    p_workArea m3CC;
    sVec3_FP* m3D8;
    s32 m434;
    s32 m43C;
    fixedPoint m440;
    std::array<fixedPoint, 4> m45C;
    fixedPoint m46C;
    fixedPoint m470;
    sVec3_FP m474;
    std::array<s16, 2> m480;
    std::array<std::array<s16, 2>, 5> m484;
    s16 m498;
    std::array<s_battleEngineSub, 0x7F> m49C;
    // size: 0xaa8
};

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
    switch (pThis->m22C)
    {
    case 0:
        pThis->m440 = 0;
        break;
    case 1:
        pThis->m440 = 0x4000000;
        break;
    case 2:
        pThis->m440 = 0x8000000;
        break;
    case 3:
        pThis->m440 = -0x4000000;
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
    s32 var1 = pThis->m440.normalized();
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
    pThis->m6C[0] = MTH_Mul(pThis->m46C, getSin(pThis->m440.getInteger() & 0xFFF));
    pThis->m6C[2] = MTH_Mul(pThis->m46C, getCos(pThis->m440.getInteger() & 0xFFF));

    pThis->m104[0] = pThis->m234[0] + pThis->m6C[0];
    pThis->m104[2] = pThis->m234[2] + pThis->m6C[2];

    sVec3_FP temp;
    generateCameraMatrixSub1(pThis->mC - pThis->m104, temp);

    pThis->m43C = temp[0];
}

void battleEngine_InitSub3Sub0(s_battleEngine* pThis)
{
    if (pThis->m104[1] < pThis->m354[1]) {
        pThis->m104[1] = pThis->m354[1];
    }
    else if (pThis->m354[0] < pThis->m104[1])
    {
        pThis->m104[1] = pThis->m354[0];
    }
}

void battleEngine_InitSub3(s_battleEngine* pThis)
{
    pThis->m104[1] = pThis->m364[getBattleManager()->m10_battleOverlay->m4_battleEngine->m22C] + pThis->m354[3];
    pThis->m270[1] = pThis->m374[getBattleManager()->m10_battleOverlay->m4_battleEngine->m22C];

    battleEngine_InitSub3Sub0(pThis);
}

void battleEngine_createDragonTask(p_workArea parent)
{
    FunctionUnimplemented();
}

void battleEngine_InitSub5(p_workArea parent)
{
    FunctionUnimplemented();
}

void battleEngine_InitSub6(s32* pData)
{
    FunctionUnimplemented();
}

void battleEngine_InitSub7(sVec3_FP* pData)
{
    getBattleManager()->m10_battleOverlay->m4_battleEngine->m3D8 = pData;
}

void createInBattleDebugTask(p_workArea parent)
{
    FunctionUnimplemented();
}

void createBattleDisplayCommandHelpTask(p_workArea parent, sSaturnPtr data)
{
    FunctionUnimplemented();
}

void createBattleEnvironmentGridTask(p_workArea parent)
{
    FunctionUnimplemented();
}

void battleEngine_InitSub8()
{
    FunctionUnimplemented();
}

void battleEngine_InitSub9(p_workArea parent)
{
    FunctionUnimplemented();
}

void battleEngine_InitSub10(p_workArea parent)
{
    FunctionUnimplemented();
}

void battleEngine_InitSub11()
{
    FunctionUnimplemented();
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

    sSaturnPtr pData = readSaturnEA(g_BTL_A3->getSaturnPtr(0x60AAFA0) + getBattleManager()->m4 * 4);
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
        pThis->m22C = 0;
    }
    else
    {
        uVar3 += cStack60;
        if (var6 < uVar3)
        {
            pThis->m22C = 1;
        }
        else
        {
            if (var6 < uVar3 + uVar4)
            {
                pThis->m22C = 2;
            }
            else
            {
                pThis->m22C = 3;
            }
        }
    }

    pThis->m22D = pThis->m22C;

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
    battleEngine_InitSub6(&getBattleManager()->m10_battleOverlay->m8_gridTask->m34);
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

    battleEngine_InitSub10(pThis);
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

p_workArea battleEngine_UpdateSub0(p_workArea parent)
{
    FunctionUnimplemented();
    return nullptr;
}

void battleEngine_UpdateSub1(int)
{
    FunctionUnimplemented();
}

void battleEngine_UpdateSub2(p_workArea)
{
    FunctionUnimplemented();
}

void battleEngine_UpdateSub3(struct npcFileDeleter*)
{
    FunctionUnimplemented();
}

void battleEngine_UpdateSub4(struct npcFileDeleter*)
{
    FunctionUnimplemented();
}

void battleEngine_UpdateSub5()
{
    FunctionUnimplemented();
}

void battleEngine_UpdateSub10(int)
{
    FunctionUnimplemented();
}

void battleEngine_UpdateSub7(p_workArea)
{
    FunctionUnimplemented();
}

void battleEngine_UpdateSub8(p_workArea)
{
    FunctionUnimplemented();
}

void battleEngine_UpdateSub9(p_workArea)
{
    FunctionUnimplemented();
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

        pThis->m3CC = battleEngine_UpdateSub0(pThis);
        pThis->m3B1 = 0;

        battleEngine_UpdateSub1(1);
        battleEngine_UpdateSub2(pThis);
        battleEngine_UpdateSub3(dramAllocatorEnd[0].mC_buffer);
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
