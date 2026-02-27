#include "PDS.h"
#include "gunShotRootTask.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"
#include "battleDragon.h"
#include "battleOverlay_C.h"
#include "LCS.h"
#include "kernel/graphicalObject.h"
#include "kernel/vdp1Allocator.h" // todo: clean
#include "BTL_A3/baldor.h" // todo: clean
#include "audio/systemSounds.h"
#include "battleGenericData.h"
#include "kernel/rayDisplay.h"
#include "BTL_A3/BTL_A3_data.h"
#include "battle/particleEffect.h"

void CreateDamageSpriteForCurrentBattleOverlay(sVec3_FP* param1, sVec3_FP* param2, s32 param3, s8 param4); // TODO: clean

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
    sAnimatedQuad m8;
    const std::vector<quadColor>* m10_colorSetup;
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
    if ((pThis->m68 == nullptr) || (pThis->m68->m50_flags & 0x40001))
    {
        pThis->getTask()->markFinished();
        return;
    }

    pThis->m65 = arg->mE;
    pThis->m90_vdp1Memory = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory;

    if ((pThis->m65 & 1) == 0)
    {
        pThis->m94 = g_BTL_GenericData->getSaturnPtr(0x060ae404);
    }
    else
    {
        pThis->m98 = arg->m8;
        particleInitSub(&pThis->m8, pThis->m90_vdp1Memory, &g_BTL_GenericData->m_0x60abef4_animatedQuads[pThis->m98]);
    }
    pThis->m70 = arg->m4;
    pThis->m14 = *arg->m4;
    pThis->m6C = getBattleTargetablePosition(*pThis->m68);
    pThis->m80 = *pThis->m6C;

    if ((pThis->m65 & 4) == 0)
    {
        pThis->m58 = 9;
        pThis->m5C = 10;
        pThis->m94 = g_BTL_GenericData->getSaturnPtr(0x60AE414);
    }
    else
    {
        pThis->m58 = 0xD;
        pThis->m5C = 0xE;
    }

    pThis->m44 = 0x3000;

    playSystemSoundEffect(9);

    if (pThis->m65 & 4)
    {
        pThis->m9A = arg->mA;
    }
    pThis->m9C = arg->mC;

    Unimplemented();
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

void increateBP(s32 param_1)
{
    if (mainGameState.gameStats.mBA_maxBP < mainGameState.gameStats.m14_currentBP + param_1)
    {
        mainGameState.gameStats.m14_currentBP = mainGameState.gameStats.mBA_maxBP;
    }
    else
    {
        mainGameState.gameStats.m14_currentBP = mainGameState.gameStats.m14_currentBP + param_1;
    }
}

s32 sGunShotTask_UpdateSub1Sub0(sBattleTargetable* pTargetable, s32 param_2)
{
    if ((((mainGameState.gameStats.mA_weaponType != 0x39) &&
        (mainGameState.gameStats.mA_weaponType != 0x3a)) &&
        (mainGameState.gameStats.mA_weaponType != 0x3b)) &&
        (mainGameState.gameStats.mA_weaponType != 0x3c)) {

        s32 iVar1;

        if (mainGameState.gameStats.mA_weaponType == 0x3d) {
            iVar1 = gBattleManager->m10_battleOverlay->m4_battleEngine->m3A6_numGunShots * 0xc0000;
        }
        else {
            if (mainGameState.gameStats.mA_weaponType != 0x3e) {
                if (mainGameState.gameStats.mA_weaponType != 0x3f) {
                    return param_2;
                }
                if (pTargetable->m60 < 0xb) {
                    return param_2;
                }
                iVar1 = MTH_Mul(fixedPoint::fromInteger(param_2), 0x1b333);
                return fixedPoint::toInteger(iVar1 + 0x8000);
            }
            iVar1 = gBattleManager->m10_battleOverlay->m4_battleEngine->m3A6_numGunShots << 0x14;
        }
        iVar1 = FP_Div(fixedPoint::fromInteger(mainGameState.gameStats.mBA_maxBP), iVar1);
        increateBP(fixedPoint::toInteger(iVar1 + 0x8000));
    }

    return param_2;
}

s32 sGunShotTask_UpdateSub1Sub2(sBattleTargetable* pTargetable, s16 param_2, s32 param_3)
{
    s32 iVar2 = 0;

    switch (param_3)
    {
    case 0:
        if (!(pTargetable->m50_flags & 0x1000))
        {
            iVar2 = 0x10000;
            if (pTargetable->m50_flags & 0x800)
            {
                iVar2 = 0x8000;
            }
        }
        break;
    case 1:
        if (!(pTargetable->m50_flags & 0x400))
        {
            iVar2 = 0x10000;
            if (pTargetable->m50_flags & 0x200)
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
        if (mainGameState.gameStats.mA_weaponType == weaponTable[i].m0_weaponType)
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

void createGunAndLaserDamage3dModel(s_workAreaCopy* pParent, sVec3_FP*, sVec3_FP*, fixedPoint); // TODO: cleanup

void sGunShotTask_UpdateSub2(sGunShotTask* pThis, s32 param_2, sVec3_FP* param_3)
{
    applyDamageToEnnemy(pThis->m68, param_2, param_3, 1, pThis->m2C, 0x800);
    pThis->m68->m50_flags &= ~0x200000;
    if (pThis->m68->m60 && !(pThis->m68->m50_flags & 0x1000))
    {
        if (pThis->m9C > -1)
        {
            Unimplemented();// createGunDamageSpriteTask
        }
        pThis->getTask()->markFinished();
    }
    else
    {
        sVec2_FP asStack84;
        computeLookAt(pThis->m2C, asStack84);

        sVec3_FP dStack96;
        dStack96[0] = asStack84[0];
        dStack96[1] = asStack84[1];
        dStack96[2] = randomNumber();
        createGunAndLaserDamage3dModel(pThis, &pThis->m14, &dStack96, 0x4CCC);

        pThis->m64 = 2;
        pThis->m58 = 10;
        playSystemSoundEffect(0x16);
    }
}

void Baldor_updateSub0Sub2Sub2(sVec3_FP* param_1, sVec3_FP* param_2, int param_3, u8 param_4) {
    int entryToUse;
    switch (param_4) {
    case 0:
        if (gBattleManager->m2_currentBattleOverlayId == 1) {
            entryToUse = 3;
        }
        else {
            entryToUse = 2;
        }
        break;
    case 1:
        if (gBattleManager->m2_currentBattleOverlayId == 1) {
            entryToUse = 5;
        }
        else {
            entryToUse = 4;
        }
        break;
    case 2:
        if (gBattleManager->m2_currentBattleOverlayId == 1) {
            entryToUse = 8;
        }
        else {
            entryToUse = 7;
        }
        break;
    default:
        assert(0);
    }

    createParticleEffect(dramAllocatorEnd[0].mC_fileBundle, &g_BTL_GenericData->m_0x60abef4_animatedQuads[entryToUse], param_1, param_2, nullptr, param_3, 0, 0);
}

void sGunShotTask_Update(sGunShotTask* pThis)
{
    if (!(pThis->m65 & 2))
    {
        pThis->m80 += gBattleManager->m10_battleOverlay->m4_battleEngine->m1A0_battleAutoScrollDelta;
    }
    else
    {
        pThis->m80 = *pThis->m6C;
    }

    sVec3_FP local_54 = pThis->m14 + gBattleManager->m10_battleOverlay->m4_battleEngine->m1A0_battleAutoScrollDelta;

    switch (pThis->m64)
    {
    case 0:
        if ((!(pThis->m65 & 4) && ((pThis->m68 != nullptr) && (pThis->m68->m50_flags & 0x40001))) || ((pThis->m68 != nullptr) && (pThis->m68->m50_flags & 0x140001)))
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
                    transformAndAddVec(*pThis->m68->m4_pPosition, sStack72, cameraProperties2.m28[0]);
                    sGunShotTask_UpdateSub2(pThis, sGunShotTask_UpdateSub1(pThis), &sStack72);
                }
            }
        }
        break;
    case 1: //TODO how does this differ from case 3?
        if (--pThis->m58 > -1) {
            pThis->m14 += pThis->m2C + gBattleManager->m10_battleOverlay->m4_battleEngine->m1A0_battleAutoScrollDelta;
            if (pThis->m14[1] <= gBattleManager->m10_battleOverlay->mC_targetSystem->m204_cameraMaxAltitude)
            {
                pThis->m_DrawMethod = nullptr;
                Baldor_updateSub0Sub2Sub2(&pThis->m14, 0, 0x10000, 1);
                pThis->m68->m50_flags &= ~0x200000;
                pThis->getTask()->markFinished();
            }
        }
        else {
            pThis->getTask()->markFinished();
            pThis->m68->m50_flags &= ~0x200000;
            CreateDamageSpriteForCurrentBattleOverlay(&pThis->m14, 0, 0x10000, 1);
        }
        break;
    case 2:
        pThis->m2C = MTH_Mul(-0x8000, pThis->m2C);
        pThis->m2C[0] += MTH_Mul(randomNumber() & 0xC000, pThis->m2C[0]);
        pThis->m2C[1] += MTH_Mul(randomNumber() & 0xC000, pThis->m2C[1]);

    case 3: //TODO how does this differ from case 1?
        if (--pThis->m58 > -1)
        {
            pThis->m14 += pThis->m2C + gBattleManager->m10_battleOverlay->m4_battleEngine->m1A0_battleAutoScrollDelta;
            if (pThis->m14[1] <= gBattleManager->m10_battleOverlay->mC_targetSystem->m204_cameraMaxAltitude)
            {
                pThis->m_DrawMethod = nullptr;
                Baldor_updateSub0Sub2Sub2(&pThis->m14, 0, 0x10000, 1);
                pThis->m68->m50_flags &= ~0x200000;
                pThis->getTask()->markFinished();
            }
        }
        else
        {
            pThis->getTask()->markFinished();
            pThis->m68->m50_flags &= ~0x200000;
            CreateDamageSpriteForCurrentBattleOverlay(&pThis->m14, 0, 0x10000, 1);
        }
        break;
    default:
        assert(0);
    }

    if (!(pThis->m65 & 1))
    {
        sVec2_FP sStack116;
        computeLookAt(pThis->m2C, sStack116);
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

void sGunShotTask_SetupSpriteData(sGunShotTask* pThis)
{
    switch (mainGameState.gameStats.mA_weaponType)
    {
    default:
    case 0x39:
        pThis->m10_colorSetup = &BTL_GenericData::m60AE424;
        break;
    case 0x3B:
        pThis->m10_colorSetup = &BTL_GenericData::m60AE42C;
        break;
    case 0x3F:
        pThis->m10_colorSetup = &BTL_GenericData::m60AE43C;
        break;
    case 0x40:
        pThis->m10_colorSetup = &BTL_GenericData::m60AE434;
        break;
    }
}

void sGunShotTask_Draw(sGunShotTask* pThis)
{
    if (!(pThis->m65 & 1))
    {
        std::array<sVec3_FP, 2> local_24;
        local_24[0] = pThis->m14;
        local_24[1] = pThis->m20_transformedVector;

        sGunShotTask_SetupSpriteData(pThis);

        if (!(pThis->m65 & 4))
        {
            assert(0);
        }
        else
        {
            displayRaySegment(
                local_24,
                readSaturnS32(pThis->m94 + 0xC),
                readSaturnU16(pThis->m94 + 0x4) + pThis->m90_vdp1Memory,
                readSaturnS16(pThis->m94 + 0x6),
                readSaturnU16(pThis->m94 + 0x8) + pThis->m90_vdp1Memory,
                &(*pThis->m10_colorSetup)[0],
                8
            );
        }
    }
    else
    {
        assert(0);
    }
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
        createSubTaskWithArgWithCopy<sGunShotTask>(gBattleManager->m10_battleOverlay->m4_battleEngine, &args, &gunShotDefinition);
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
