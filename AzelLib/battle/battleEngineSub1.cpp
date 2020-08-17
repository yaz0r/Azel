#include "PDS.h"
#include "battleEngineSub1.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"
#include "battleDragon.h"
#include "battleGrid.h"
#include "kernel/debug/trace.h"
#include "battle/interpolators/vec2FPInterpolator.h"
#include "battleGenericData.h"

struct sBattleEngineSub1 : public s_workAreaTemplate<sBattleEngineSub1>
{
    s8 m0;
    s8 m1;
    s8 m2;
    s16 m4;
    s16 m6_interpolationLength;
    sVec2FPInterpolator m8;
    sVec2FPInterpolator m44;
    sVec3_FP m80;
    //size 0x8C;
};

void battleEngineSub1_Update(sBattleEngineSub1* pThis)
{
    if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m8_showingBattleResultScreen)
    {
        gBattleManager->m10_battleOverlay->m8_gridTask->m1C.zeroize();
        return;
    }

    sVec2_FP temp;
    computeVectorAngles(gBattleManager->m10_battleOverlay->m4_battleEngine->mC_battleCenter - gBattleManager->m10_battleOverlay->m18_dragon->m8_position, temp);

    sVec3_FP local_50;
    local_50[0] = 0;
    local_50[1] = temp[1];
    local_50[2] = 0;

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
            pThis->m6_interpolationLength = 0x3C;
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
        pThis->m6_interpolationLength = 0x3C;
        break;
    case 1:
        do 
        {
            pThis->m1 = randomNumber() & 7;
        } while (pThis->m1 == pThis->m2);
        pThis->m2 = pThis->m1;
        pThis->m8.mC_startValue[0] = gBattleManager->m10_battleOverlay->m8_gridTask->m1BC_cameraRotationStep[0];
        pThis->m8.mC_startValue[1] = gBattleManager->m10_battleOverlay->m8_gridTask->m1BC_cameraRotationStep[1];
        pThis->m8.m24_targetValue[0] = readSaturnS32(g_BTL_GenericData->getSaturnPtr(0x60ab120) + pThis->m1 * 8 + 0);
        pThis->m8.m24_targetValue[1] = readSaturnS32(g_BTL_GenericData->getSaturnPtr(0x60ab120) + pThis->m1 * 8 + 4);
        pThis->m8.m38_interpolationLength = pThis->m6_interpolationLength;
        vec2FPInterpolator_Init(&pThis->m8);
        pThis->m44.mC_startValue = pThis->m80;
        pThis->m44.m24_targetValue = readSaturnVec3(g_BTL_GenericData->getSaturnPtr(0x60AB160) + pThis->m1 * 0xC);
        pThis->m44.m38_interpolationLength = pThis->m6_interpolationLength;
        vec2FPInterpolator_Init(&pThis->m44);
        pThis->m0++;
        break;
    case 2:
        {
            vec2FPInterpolator_Step(&pThis->m8);
            gBattleManager->m10_battleOverlay->m8_gridTask->m1BC_cameraRotationStep[0] = pThis->m8.m0_currentValue[0];
            gBattleManager->m10_battleOverlay->m8_gridTask->m1BC_cameraRotationStep[1] = pThis->m8.m0_currentValue[1];

            if (vec2FPInterpolator_Step(&pThis->m44))
            {
                pThis->m0++;
            }
            pThis->m80 = pThis->m44.m0_currentValue;

            sVec3_FP local_2c;
            battleEngineSub1_UpdateSub2(&local_2c, gBattleManager->m10_battleOverlay->m18_dragon->m8_position, pThis->m44.m0_currentValue, local_50);

            if (isTraceEnabled())
            {
                addTraceLog(local_2c, "local_2c");
                addTraceLog(gBattleManager->m10_battleOverlay->m18_dragon->m8_position, "m8_position");
            }

            gBattleManager->m10_battleOverlay->m8_gridTask->m1C = local_2c - gBattleManager->m10_battleOverlay->m18_dragon->m8_position;
        }
        break;
    case 3:
        pThis->m0 = 1;
        pThis->m4 = 0;
        pThis->m6_interpolationLength = 0x78;
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
    pNewTask->m6_interpolationLength = 0x3C;
    pNewTask->m1 = randomNumber() & 7;
    pNewTask->m2 = randomNumber() & 7;
}
