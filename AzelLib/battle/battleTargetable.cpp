#include "PDS.h"
#include "battleTargetable.h"
#include "battleManager.h"
#include "battleEngine.h"
#include "battleOverlay.h"
#include "battleDragon.h"
#include "battleDebug.h"
#include "kernel/debug/trace.h"

void battleTargetable_updatePosition(sBattleTargetable* pThis)
{
    transformAndAddVec(*pThis->m4_pPosition, pThis->m10_position, cameraProperties2.m28[0]);

    addTraceLog(*pThis->m4_pPosition, "targetablePositionSource");
    addTraceLog(pThis->m10_position, "targetablePosition");
}

sVec3_FP* getBattleTargetablePosition(sBattleTargetable& param1)
{
    return &param1.m10_position;
}

void applyDamageSub(sBattleTargetable& param_1, sVec3_FP& param_2)
{
    param_1.mC = &param_2;
    param_1.m28 = param_2;
    transformAndAddVecByCurrentMatrix(&param_1.m28, &param_1.m1C);
    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x14])
    {
        assert(0);
    }
}

void applyDamageToDragon(sBattleTargetable& pThis, s32 damageValue, sVec3_FP& param_3, s32 param_4, const sVec3_FP& param_5, s32 param_8)
{
    gBattleManager->m10_battleOverlay->m18_dragon->m1D6 += damageValue;
    pThis.m50_flags |= 0x80000;
    pThis.m58 = gBattleManager->m10_battleOverlay->m18_dragon->m1D6;

    applyDamageSub(pThis, param_3);

    pThis.m5E_impactForce = param_4;
    pThis.m54 = 0;
    pThis.m54 = param_8;

    pThis.m34_impactVector = MTH_Mul(FP_Div(0x1000, sqrt_F(MTH_Product3d_FP(param_5, param_5))), param_5);
}

fixedPoint sGunShotTask_UpdateSub2Sub0Sub0(sBattleTargetable* pThis, s16 param_1)
{
    return MTH_Mul(fixedPoint::fromInteger(param_1), setDividend(0x10000, fixedPoint::fromInteger(pThis->m60), 0xA0000)).toInteger();
}

void applyDamageToEnnemy(sBattleTargetable* pThis, s16 param_2, sVec3_FP* param_3, s8 param_4, const sVec3_FP& param_5, fixedPoint param_8)
{
    if (param_4 != 3)
    {
        param_2 = sGunShotTask_UpdateSub2Sub0Sub0(pThis, param_2);
    }

    pThis->m50_flags |= 0x80000;
    pThis->m58 = param_2;
    applyDamageSub(*pThis, *param_3);

    pThis->m5E_impactForce = param_4;
    pThis->m54 = 0;
    pThis->m54 = param_8;

    pThis->m34_impactVector = MTH_Mul(FP_Div(0x1000, sqrt_F(MTH_Product3d_FP(param_5, param_5))), param_5);
}

void deleteTargetable(sBattleTargetable* pThis)
{
    pThis->m50_flags |= 0x40000;
    if (!BattleEngineSub0_UpdateSub0())
    {
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2_needToSortEnemiesByDistanceFromDragon = 1;
    }
}
