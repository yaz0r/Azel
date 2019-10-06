#include "PDS.h"
#include "battleDragon.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"
#include "kernel/fileBundle.h"

s32 s_battleDragon_InitSub0()
{
    s8 bVar1 = getBattleManager()->m10_battleOverlay->m4_battleEngine->m230;

    if ((((bVar1 != 0) && (bVar1 != 1)) && (bVar1 != 2)) &&
        (((bVar1 != 3 && (bVar1 != 8)) && (bVar1 != 10)))) {
        return 0;
    }
    return 1;
}

void s_battleDragon_InitSub4Sub0()
{
    updateAndInterpolateAnimation(&gDragonState->m28_dragon3dModel);
    updateAnimationMatrices(&gDragonState->m78_animData, &gDragonState->m28_dragon3dModel);
}

void s_battleDragon_InitSub4(int arg0, int arg1)
{
    playAnimation(&gDragonState->m28_dragon3dModel, gDragonState->m0_pDragonModelBundle->getAnimation(gDragonState->m20_dragonAnimOffsets[arg0]), arg1);
    s_battleDragon_InitSub4Sub0();
}

void s_battleDragon_InitSub5Sub0(s_battleDragon_8C* pThis)
{
    transformAndAddVec(*pThis->m4, pThis->m10, cameraProperties2.m28[0]);
}

void s_battleDragon_InitSub5(s_battleDragon_8C* pThis, s_battleDragon* param2, sVec3_FP* param3, s32 param4, s32 param5)
{
    pThis->m0 = param2;
    pThis->m4 = param3;
    pThis->m4C = param4;
    pThis->m40 = *param3;
    s_battleDragon_InitSub5Sub0(pThis);
    pThis->m8 = &pThis->m10;
    pThis->mC = 0;
    pThis->m50 = param5;
    pThis->m58 = 0;
    pThis->m5A = 0;
    pThis->m5E = 0;
    pThis->m5F = 0;
    pThis->m5C = 0;
    pThis->m54 = 0;
    pThis->m60 = 10;
}

void s_battleDragon_InitSub3(s_loadRiderWorkArea* pRider, s16 param2, s32 param3)
{
    playAnimationGeneric(&pRider->m18_3dModel, pRider1State->m0_riderBundle->getAnimation(param2), param3);
    updateAndInterpolateAnimation(&pRider->m18_3dModel);
}

static void s_battleDragon_Init(s_battleDragon* pThis)
{
    getBattleManager()->m10_battleOverlay->m18_dragon = pThis;

    pThis->m8_position = getBattleManager()->m10_battleOverlay->m4_battleEngine->m104_dragonStartPosition;
    resetMatrixStack();
    pThis->m1CE = 0;

    if (s_battleDragon_InitSub0() == 0)
    {
        assert(0);
    }
    else
    {
        s_battleDragon_InitSub3(pRider1State, readSaturnS16(gCurrentBattleOverlay->getSaturnPtr(0x60ae368) + getBattleManager()->m10_battleOverlay->m4_battleEngine->m22C * 2), 0);
        s_battleDragon_InitSub3(pRider2State, readSaturnS16(gCurrentBattleOverlay->getSaturnPtr(0x60ae370) + getBattleManager()->m10_battleOverlay->m4_battleEngine->m22C * 2), 0);
        pThis->m1CC = 2;
    }

    s_battleDragon_InitSub4(pThis->m1CC, 0);
    s_battleDragon_InitSub5(&pThis->m8C, pThis, &pThis->m1A4, 0x1000, 0xf0000001);
    pThis->m1C8 = 0x111111;

    if ((mainGameState.gameStats.m1_dragonLevel == 6) && (pThis->m24C == 0))
    {
        assert(0);
    }
    else
    {
        pThis->m24C = 0;
    }

    FunctionUnimplemented();
}

static void s_battleDragon_Update(s_battleDragon* pThis)
{
    FunctionUnimplemented();
}

static void s_battleDragon_Draw(s_battleDragon* pThis)
{
    FunctionUnimplemented();
}

static void s_battleDragon_Delete(s_battleDragon* pThis)
{
    FunctionUnimplemented();
}

void battleEngine_createDragonTask(s_workAreaCopy* parent)
{
    static const s_battleDragon::TypedTaskDefinition definition = {
        &s_battleDragon_Init,
        &s_battleDragon_Update,
        &s_battleDragon_Draw,
        &s_battleDragon_Delete,
    };
    createSubTaskWithCopy<s_battleDragon>(parent, &definition);
}
