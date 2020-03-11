#include "PDS.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"
#include "battleDragon.h"
#include "battleOverlay_C.h"
#include "LCS.h"
#include "town/town.h" // todo: clean
#include "kernel/vdp1Allocator.h" // todo: clean
#include "BTL_A3/baldor.h" // todo: clean

struct sGunShotRootTask : public s_workAreaTemplateWithCopy<sGunShotRootTask>
{
    s16 mD4_currentSelectedEnemy;
    s16 mD6_shotDelay;
    s16 mD8_numShotFired;
    s8 mDC;

    // size 0xE8
};

struct sGunArg
{
    sBattleTargetable* m0;
    sVec3_FP* m4;
    s16 m8;
    s16 mA;
    s16 mC;
    s8 mE;
};

struct sGunShotTask : public s_workAreaTemplateWithArgWithCopy<sGunShotTask, sGunArg*>
{
    s_LCSTask340Sub::s_LCSTask340Sub_m58 m8;
    sVec3_FP m14;
    s32 m44;
    s32 m58;
    s32 m5C;
    s8 m65;
    sBattleTargetable* m68;
    sVec3_FP* m6C;
    sVec3_FP* m70;
    sVec3_FP m80;
    u16 m90_vdp1Memory;
    sSaturnPtr m94;
    s16 m98;
    s16 m9A;
    s16 m9C;
    // size 0xA0
};

void sGunShotTask_Init(sGunShotTask* pThis, sGunArg* arg)
{
    pThis->m68 = arg->m0;
    if ((pThis->m68 == nullptr) || (pThis->m68->m50 & 0x40001))
    {
        pThis->getTask()->markFinished();
        return;
    }

    pThis->m65 = arg->mE;
    pThis->m90_vdp1Memory = dramAllocatorEnd[0].mC_buffer->m4_vd1Allocation->m4_vdp1Memory;

    if ((pThis->m65 & 1) == 0)
    {
        pThis->m94 = gCurrentBattleOverlay->getSaturnPtr(0x060ae404);
    }
    else
    {
        pThis->m98 = arg->m8;
        s_LCSTask340Sub::Init3Sub3(&pThis->m8, pThis->m90_vdp1Memory, readSaturnEA(gCurrentBattleOverlay->getSaturnPtr(0x060abef4) + pThis->m98 * 4));
    }
    pThis->m70 = arg->m4;
    pThis->m14 = *arg->m4;
    pThis->m6C = getBattleTargetablePosition(*pThis->m68);
    pThis->m80 = *pThis->m6C;

    if ((pThis->m65 & 4) == 0)
    {
        pThis->m58 = 9;
        pThis->m5C = 10;
        pThis->m94 = gCurrentBattleOverlay->getSaturnPtr(0x60AE414);
    }
    else
    {
        pThis->m58 = 0xD;
        pThis->m5C = 0xE;
    }

    pThis->m44 = 0x3000;

    playSoundEffect(9);

    if (pThis->m65 & 4)
    {
        pThis->m9A = arg->mA;
    }
    pThis->m9C = arg->mC;

    FunctionUnimplemented();
}

void sGunShotTask_Update(sGunShotTask* pThis)
{
    FunctionUnimplemented();
}

void sGunShotTask_Draw(sGunShotTask* pThis)
{
    FunctionUnimplemented();
}

void sGunShotRootTask_Update(sGunShotRootTask* pThis)
{
    if (pThis->mDC)
    {
        assert(0);
    }

    if (pThis->mD6_shotDelay-- > 0)
        return;

    if (gBattleManager->m10_battleOverlay->m4_battleEngine->m3A6_numGunShots <= pThis->mD8_numShotFired++)
    {
        pThis->mD8_numShotFired = 0;
        pThis->mD6_shotDelay = 0x10;
        pThis->mDC++;
        return;
    }

    sGunArg args;

    args.m0 = nullptr;
    args.m4 = nullptr;
    args.m8 = 0;
    args.mA = 0;
    args.mC = -1;
    args.mE = 0;

    static const sGunShotTask::TypedTaskDefinition gunShotDefinition =
    {
        &sGunShotTask_Init,
        &sGunShotTask_Update,
        &sGunShotTask_Draw,
        nullptr
    };

    switch(mainGameState.gameStats.mA_weaponType)
    {
    case 0:
        args.m0 = gBattleManager->m10_battleOverlay->mC_targetSystem->m0_enemyTargetables[pThis->mD4_currentSelectedEnemy]->m4_targetable;
        args.m4 = &gBattleManager->m10_battleOverlay->m18_dragon->mF0;
        args.mE = 6;
        createSiblingTaskWithArgWithCopy<sGunShotTask>(gBattleManager->m10_battleOverlay->m4_battleEngine, &args, &gunShotDefinition);
        break;
    default:
        assert(0);
    }

    pThis->mD6_shotDelay = 2;
}

void createGunShotRootTask(s_workAreaCopy* parent)
{
    switch (mainGameState.gameStats.mA_weaponType)
    {
    case 0:
        gBattleManager->m10_battleOverlay->m4_battleEngine->m3A6_numGunShots = 5;
        break;
    default:
        assert(0);
    }

    sGunShotRootTask* pNewTask = createSubTaskFromFunctionWithCopy<sGunShotRootTask>(parent, &sGunShotRootTask_Update);
    if (pNewTask)
    {
        pNewTask->mD4_currentSelectedEnemy = gBattleManager->m10_battleOverlay->m4_battleEngine->m398_currentSelectedEnemy;
        pNewTask->mD6_shotDelay = 0;
        pNewTask->mD8_numShotFired = 0;
    }

    gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished = 1;
}
