#include "PDS.h"
#include "battleEngineSub1.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"
#include "battleDragon.h"
#include "battleGrid.h"

struct sBattleEngineSub1 : public s_workAreaTemplate<sBattleEngineSub1>
{
    s8 m0;
    s8 m1;
    s8 m2;
    s16 m4;
    s16 m6;
    fixedPoint m14;
    fixedPoint m18;
    fixedPoint m2C;
    fixedPoint m30;
    sVec3_FP m80;
    //size 0x8C;
};

void battleEngineSub1_Update(sBattleEngineSub1* pThis)
{
    if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m8)
    {
        gBattleManager->m10_battleOverlay->m8_gridTask->m1C.zeroize();
        return;
    }

    sVec2_FP temp;
    computeVectorAngles(gBattleManager->m10_battleOverlay->m4_battleEngine->mC_battleCenter - gBattleManager->m10_battleOverlay->m18_dragon->m8_position, temp);

    if (BattleEngineSub0_UpdateSub0())
    {
        switch (gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode)
        {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 0xb:
        case 0xd:
        case 0xe:
            pThis->m0 = 1;
            gBattleManager->m10_battleOverlay->m8_gridTask->m1BC_cameraRotationStep[0] = 0;
            gBattleManager->m10_battleOverlay->m8_gridTask->m1BC_cameraRotationStep[1] = 0;
            gBattleManager->m10_battleOverlay->m8_gridTask->m1C.zeroize();
            pThis->m80.zeroize();
            pThis->m6 = 0x3C;
            return;
        case 0xC:
        case 0xf:
            break;
        default:
            return;
        }
    }

    switch (pThis->m0)
    {
    case 0:
        pThis->m0 = 1;
        pThis->m4 = 0;
        pThis->m6 = 0x3C;
        break;
    case 1:
        do 
        {
            pThis->m1 = randomNumber() & 7;
        } while (pThis->m1 == pThis->m2);
        pThis->m2 = pThis->m1;
        pThis->m14 = gBattleManager->m10_battleOverlay->m8_gridTask->m1BC_cameraRotationStep[0];
        pThis->m18 = gBattleManager->m10_battleOverlay->m8_gridTask->m1BC_cameraRotationStep[1];
        pThis->m2C = readSaturnS32(gCurrentBattleOverlay->getSaturnPtr(0x60ab120) + pThis->m1 * 8 + 0);
        pThis->m30 = readSaturnS32(gCurrentBattleOverlay->getSaturnPtr(0x60ab120) + pThis->m1 * 8 + 4);
        FunctionUnimplemented();
        pThis->m0++;
        break;
    case 2:
        FunctionUnimplemented();
        break;
    default:
        assert(0);
        break;
    }
}

void battleEngineSub1_Draw(sBattleEngineSub1* pThis)
{
    FunctionUnimplemented();
}

void createBattleEngineSub1(p_workArea parent)
{
    static const sBattleEngineSub1::TypedTaskDefinition definition = {
        nullptr,
        &battleEngineSub1_Update,
        &battleEngineSub1_Draw,
        nullptr,
    };

    sBattleEngineSub1* pNewTask = createSubTask<sBattleEngineSub1>(parent, &definition);
    pNewTask->m6 = 0x3C;
    pNewTask->m1 = randomNumber() & 7;
    pNewTask->m2 = randomNumber() & 7;
}
