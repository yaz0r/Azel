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
    sVec3_FP m20_transformedVector;
    sVec3_FP m2C;
    s32 m44;
    s32 m58;
    s32 m5C;
    s8 m64;
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

void sGunShotTask_UpdateSub0Sub0(sVec3_FP* param_1, sVec3_FP* param_2, sVec3_FP* param_3)
{
    sVec3_FP sum = *param_1 + *param_2;
    *param_3 = sum + sum / 2;
}

s32 sGunShotTask_UpdateSub0(sVec3_FP* param1, sVec3_FP* param2, sVec3_FP* param3)
{
    sVec3_FP local_20 = *param2 - *param1;

    fixedPoint distance = (*param1)[0] + (*param2)[0];

    if (MTH_Mul(distance, distance) < dot3_FP(&local_20, &local_20))
    {
        return -1;
    }
    else
    {
        sVec3_FP sStack44;
        sGunShotTask_UpdateSub0Sub0(param1, param2, &sStack44);
        transformAndAddVec(sStack44, *param3, cameraProperties2.m28[0]);
        return 1;
    }
}

s32 sGunShotTask_UpdateSub1Sub0(sBattleTargetable* pTargetable, s32 param_2)
{
    if ((((mainGameState.gameStats.mA_weaponType != 0x39) &&
        (mainGameState.gameStats.mA_weaponType != 0x3a)) &&
        (mainGameState.gameStats.mA_weaponType != 0x3b)) &&
        (mainGameState.gameStats.mA_weaponType != 0x3c)) {
        assert(0);
    }

    return param_2;
}

s32 sGunShotTask_UpdateSub1Sub2(sBattleTargetable* pTargetable, s16 param_2, s32 param_3)
{
    s32 iVar2 = 0;

    switch (param_3)
    {
    case 0:
        if (!(pTargetable->m50 & 0x1000))
        {
            iVar2 = 0x10000;
            if (pTargetable->m50 & 0x800)
            {
                iVar2 = 0x8000;
            }
        }
        break;
    default:
        assert(0);
        break;
    }

    return fixedPoint::toInteger(MTH_Mul(fixedPoint::fromInteger(param_2), iVar2) + 0x8000);
}

struct sWeaponData
{
    s16 m0_weaponType;
    s16 m2;
    fixedPoint m4;
};

const std::array<sWeaponData, 9> weaponTable = {
    {
        {0x3B, 0, 0x13333},
        {0x3A, 0, 0x18000},
        {0x3C, 0, 0x20000},
        {0x3E, 0, 0x10000},
        {0x3D, 0, 0x10000},
        {0x40, 0, 0xB333},
        {0x41, 0, 0x1E666},
        {0x3F, 0, 0x10000},
        {-1, 0, 0x10000},
    }
};

s32 getCurrentGunPower()
{
    for (int i=0; i<9; i++)
    {
        if ((mainGameState.gameStats.mA_weaponType == weaponTable[i].m0_weaponType))
        {
            return fixedPoint::toInteger(MTH_Mul(weaponTable[i].m4, fixedPoint::fromInteger(mainGameState.gameStats.mE_gunPower)) + 0x8000);
        }
    }

    return mainGameState.gameStats.mE_gunPower;
}

s32 sGunShotTask_UpdateSub1(sGunShotTask* pThis)
{
    fixedPoint ratio = FP_Div(fixedPoint::fromInteger(getCurrentGunPower()), fixedPoint::fromInteger(gBattleManager->m10_battleOverlay->m4_battleEngine->m3A6_numGunShots));
    return sGunShotTask_UpdateSub1Sub2(pThis->m68, sGunShotTask_UpdateSub1Sub0(pThis->m68, fixedPoint::toInteger(ratio + 0x8000)), 0);
}

void sGunShotTask_UpdateSub2(sGunShotTask* pThis, s32 param_2, sVec3_FP* param_3)
{
    FunctionUnimplemented();
}

void sGunShotTask_Update(sGunShotTask* pThis)
{
    if (!(pThis->m65 & 2))
    {
        pThis->m80 += gBattleManager->m10_battleOverlay->m4_battleEngine->m1A0;
    }
    else
    {
        pThis->m80 = *pThis->m6C;
    }

    sVec3_FP local_54 = pThis->m14 + gBattleManager->m10_battleOverlay->m4_battleEngine->m1A0;

    switch (pThis->m64)
    {
    case 0:
        if ((!(pThis->m65 & 4) && ((pThis->m68 != nullptr) && (pThis->m68->m50 & 0x40001))) || ((pThis->m68 != nullptr) && (pThis->m68->m50 & 0x140001)))
        {
            pThis->m64 = 1;
            pThis->m14 = pThis->m2C + local_54;
            pThis->m58 += 2;
        }
        else
        {
            if (pThis->m58 > 0)
            {
                pThis->m58--;
                sVec3_FP local_2c = pThis->m80 - *pThis->m70;
                local_2c = MTH_Mul(FP_Div((pThis->m5C - pThis->m58) * 0x10000, pThis->m5C * 0x10000), local_2c);
                pThis->m14 = local_2c + *pThis->m70;
                pThis->m2C = pThis->m14 - local_54;
                if (!(pThis->m65 & 2))
                {
                    sVec3_FP local_3c;
                    sVec3_FP sStack72;
                    transformAndAddVecByCurrentMatrix(&pThis->m14, &local_3c);
                    if (sGunShotTask_UpdateSub0(&local_3c, &pThis->m68->m40, &sStack72) > 0)
                    {
                        sGunShotTask_UpdateSub2(pThis, sGunShotTask_UpdateSub1(pThis), &sStack72);
                    }
                }
            }
            else
            {
                if (!(pThis->m65 & 2))
                {
                    sVec3_FP local_3c;
                    sVec3_FP sStack72;
                    transformAndAddVecByCurrentMatrix(&pThis->m14, &local_3c);
                    if (sGunShotTask_UpdateSub0(&local_3c, &pThis->m68->m40, &sStack72) > 0)
                    {
                        sGunShotTask_UpdateSub2(pThis, sGunShotTask_UpdateSub1(pThis), &sStack72);
                    }
                    else
                    {
                        pThis->m64 = 3;
                        pThis->m14 = local_54 + pThis->m2C;
                        pThis->m58 += 2;
                    }
                }
                else
                {
                    pThis->m14 = pThis->m80;
                    pThis->m58 = 0;

                    sVec3_FP sStack72;
                    transformAndAddVec(*pThis->m68->m4, sStack72, cameraProperties2.m28[0]);
                    sGunShotTask_UpdateSub2(pThis, sGunShotTask_UpdateSub1(pThis), &sStack72);
                }
            }
        }
        break;
    default:
        assert(0);
    }

    if (!(pThis->m65 & 1))
    {
        sVec2_FP sStack116;
        computeVectorAngles(pThis->m2C, sStack116);
        sVec3_FP local_6c = sVec3_FP(0, 0, 0x8000);

        pushCurrentMatrix();
        translateCurrentMatrix(pThis->m14);
        rotateCurrentMatrixShiftedY(sStack116[1]);
        rotateCurrentMatrixShiftedX(sStack116[0]);
        sVec3_FP sStack128;
        transformAndAddVecByCurrentMatrix(&local_6c, &sStack128);
        transformAndAddVec(sStack128, pThis->m20_transformedVector, cameraProperties2.m28[0]);
        popMatrix();
    }
    else
    {
        assert(0);
    }

    if (pThis->m65 & 8)
    {
        assert(0);
    }
}

void sGunShotTask_Draw(sGunShotTask* pThis)
{
    FunctionUnimplemented();
}

void sGunShotRootTask_Update(sGunShotRootTask* pThis)
{
    if (pThis->mDC)
    {
        if (pThis->mDC != 1)
        {
            return;
        }

        if (--pThis->mD6_shotDelay > -1)
        {
            return;
        }

        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished = 1;

        pThis->getTask()->markFinished();
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
