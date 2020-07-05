#include "PDS.h"
#include "battleResultScreen.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"
#include "battleGrid.h"
#include "battleOverlay_C.h"
#include "battleDragon.h"
#include "battleDebug.h"
#include "battleResultScreenProgressiveText.h"
#include "battleResultScreenProgressiveText2.h"
#include "battleResultScreenProgressiveTextNumber.h"
#include "battleEnd.h"
#include "audio/systemSounds.h"
#include "BTL_A3/BTL_A3_data.h"

enum eBattleRating : s8
{
    m0_narrowEscape,
    m1_closeCall,
    m2_goodFight,
    m3_greatFight,
    m4_excellent
};

struct sBattleResultScreen : public s_workAreaTemplate<sBattleResultScreen>
{
    s16 m4;
    s8 m72;
    s8 m74;
    s8 m7A;
    eBattleRating m80_battleRating;
    // size 0x84
};

void battleResultScreen_updateSub0(s8 param1, s8 param2, s8 param3, sVec3_FP* param4)
{
    (*param4)[0] = fixedPoint::fromInteger(param1);
    (*param4)[1] = fixedPoint::fromInteger(param2);
    (*param4)[2] = fixedPoint::fromInteger(param3);
}

void battleGrid_setupLightInterpolation(s8 param1, sVec3_FP param2, sVec3_FP param3)
{
    gBattleManager->m10_battleOverlay->m8_gridTask->m1C8_flags |= 4;
    gBattleManager->m10_battleOverlay->m8_gridTask->m21C_lightInterpolateCurrentValue = param2;
    gBattleManager->m10_battleOverlay->m8_gridTask->m234_lightInterpolateFinalValue = param3;
    gBattleManager->m10_battleOverlay->m8_gridTask->m264_lightInterpolateNumSteps = param1;
    gBattleManager->m10_battleOverlay->m8_gridTask->m24C_lightInterpolateStep = FP_Div(param3 - param2, fixedPoint::fromInteger(param1));
}

void battleGrid_setupLightInterpolation2(s8 param1, sVec3_FP param2, sVec3_FP param3)
{
    gBattleManager->m10_battleOverlay->m8_gridTask->m1C8_flags |= 8;
    gBattleManager->m10_battleOverlay->m8_gridTask->m228_lightInterpolate2CurrentValue = param2;
    gBattleManager->m10_battleOverlay->m8_gridTask->m240_lightInterpolate2FinalValue = param3;
    gBattleManager->m10_battleOverlay->m8_gridTask->m266_lightInterpolate2NumSteps = param1;
    gBattleManager->m10_battleOverlay->m8_gridTask->m258_lightInterpolate2Step = FP_Div(param3 - param2, fixedPoint::fromInteger(param1));
}

eBattleRating battleResultScreen_computeBattleRating(sBattleResultScreen* pThis)
{
    FunctionUnimplemented();
    return eBattleRating::m4_excellent;
}

static const std::array<s8, 5> globalRatingModyfier = {
    -5,
    -5,
    0,
    3,
    5,
};

static const std::array<fixedPoint, 5> ratingXPModifyer = {
    0xCCCC,
    0x10000,
    0x11999,
    0x13333,
    0x18000,
};

static const std::array<fixedPoint, DR_LEVEL_MAX> resultScreenCameraPositionPerDragonLevel = {
    0x8000,
    0xA000,
    0xA000,
    0xA000,
    0xA000,
    0xA000,
    0x8000,
    0xA000,
    0xA000
};

void battleResultScreen_updateSub5(int XPReceived)
{
    FunctionUnimplemented();
}

void battleResultScreen_printItemReceived(eBattleRating)
{
    FunctionUnimplemented();
}

s32 battleResultScreen_updateDragonAndRiderSub0()
{
    FunctionUnimplemented();
    return 0;
}

s32 battleResultScreen_updateDragonAndRiderSub1()
{
    FunctionUnimplemented();
    return 0;
}

void battleResultScreen_updateDragonAndRider(eBattleRating rating)
{
    if (!battleResultScreen_updateDragonAndRiderSub0())
    {
        switch (rating)
        {
        case eBattleRating::m0_narrowEscape:
        case eBattleRating::m1_closeCall:
            gBattleManager->m10_battleOverlay->m18_dragon->m88 |= 0x800;
            break;
        case eBattleRating::m2_goodFight:
            gBattleManager->m10_battleOverlay->m18_dragon->m88 |= 0x400;
            break;
        case eBattleRating::m3_greatFight:
        case eBattleRating::m4_excellent:
            gBattleManager->m10_battleOverlay->m18_dragon->m88 |= 0x200;
            break;
        default:
            assert(0);
        }
    }

    if (!battleResultScreen_updateDragonAndRiderSub1())
    {
        // erf?
        if (pRider2State->mC_riderType == 2)
        {
            gBattleManager->m10_battleOverlay->m18_dragon->m88 |= 0x4000;
        }
        else
        {
            gBattleManager->m10_battleOverlay->m18_dragon->m88 |= 0x4000;
        }
    }
}

void battleResultScreen_checkLevelUp(sBattleResultScreen* pThis)
{
    FunctionUnimplemented();
}

bool isCurrentBattleID(s8 param1, s8 param2)
{
    if ((gBattleManager->m2_currentBattleOverlayId == param1) && (gBattleManager->m6_subBattleId == param2))
    {
        return true;
    }
    return false;
}

void battleResultScreen_updateSub17(sBattleResultScreen* pThis)
{
    if (!isCurrentBattleID(0x1A, 0) && !isCurrentBattleID(0xe, 2) && !isCurrentBattleID(0x16, 0) && !isCurrentBattleID(4, 0) && !isCurrentBattleID(0x11, 0))
    {
        createBattleEndTask(pThis, 0);
        battleEngine_UpdateSub7Sub2();
        return;
    }
    assert(0);
    FunctionUnimplemented();
}

void battleResultScreen_update(sBattleResultScreen* pThis)
{
    s_battleGrid* pGrid = gBattleManager->m10_battleOverlay->m8_gridTask;
    s_battleEngine* pBattleEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;

    switch (pThis->m72)
    {
    case 0:
        g_fadeControls.m_4D = 6;
        if (g_fadeControls.m_4C < g_fadeControls.m_4D)
        {
            vdp2Controls.m20_registers[1].m112_CLOFSL = 0;
            vdp2Controls.m20_registers[0].m112_CLOFSL = 0;
        }
        fadePalette(&g_fadeControls.m0_fade0, 0xC210, 0xC210, 1);
        fadePalette(&g_fadeControls.m24_fade1, 0xC210, 0xC210, 1);

        g_fadeControls.m_4D = 6;
        if (g_fadeControls.m_4C < g_fadeControls.m_4D)
        {
            vdp2Controls.m20_registers[1].m112_CLOFSL = 0;
            vdp2Controls.m20_registers[0].m112_CLOFSL = 0;
        }

        fadePalette(&g_fadeControls.m0_fade0, 0xC210, 0xa94a, 0x1E);

        g_fadeControls.m_4D = 5;

        battleResultScreen_updateSub0(-0x1f, -0x1f, -0x1f, &pGrid->m1FC_lightFalloff1);
        battleResultScreen_updateSub0(-0x1f, -0x1f, -0x1f, &pGrid->m208_lightFalloff2);

        {
            sVec3_FP local_40;
            battleResultScreen_updateSub0(0, 0, 0, &local_40);
            battleGrid_setupLightInterpolation(0x3c, pGrid->m1CC_lightColor, local_40);

            battleResultScreen_updateSub0(0, 0, 0, &local_40);
            battleGrid_setupLightInterpolation2(0x3c, pGrid->m1E4_lightFalloff0, local_40);
        }

        pThis->m80_battleRating = battleResultScreen_computeBattleRating(pThis);

        // print the battle rating (grey version, split in 2 strings)
        battleResultScreen_createProgressiveText(pThis, readSaturnEA(g_BTL_A3->getSaturnPtr(0x60ab944) + pThis->m80_battleRating * 4), 0, 0, 0);
        battleResultScreen_createProgressiveText(pThis, readSaturnEA(g_BTL_A3->getSaturnPtr(0x60ab944) + pThis->m80_battleRating * 4) + 0x10, 0, 0, 0);

        playSystemSoundEffect(0x11);

        gBattleManager->m10_battleOverlay->m18_dragon->m88 |= 0x100;
        gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant = 0;
        gBattleManager->m10_battleOverlay->m4_battleEngine->m270_enemyAltitude[1] = pBattleEngine->m104_dragonPosition[1] - gBattleManager->m10_battleOverlay->mC_targetSystem->m204_cameraMaxAltitude;

        pThis->m4 = 0x2D;

        pThis->m72++;

        gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers = 0;

        if (mainGameState.gameStats.m1_dragonLevel == 8)
        {
            gBattleManager->m10_battleOverlay->m4_battleEngine->m474_XPReceivedFromBattle = 0;
        }
        else
        {
            mainGameState.gameStats.m7C_overallRating += globalRatingModyfier[pThis->m80_battleRating];
        }

        pBattleEngine->m474_XPReceivedFromBattle = fixedPoint::toInteger(MTH_Mul(ratingXPModifyer[pThis->m80_battleRating], fixedPoint::fromInteger(pBattleEngine->m474_XPReceivedFromBattle)));

        if (mainGameState.gameStats.m20_XP + pBattleEngine->m474_XPReceivedFromBattle < 100000000)
        {
            mainGameState.gameStats.m20_XP += pBattleEngine->m474_XPReceivedFromBattle;
        }
        else
        {
            mainGameState.gameStats.m20_XP = 99999999;
        }

        if (mainGameState.gameStats.m38_dyne + pBattleEngine->m478_dyneReceivedFromBattle < 10000000)
        {
            mainGameState.gameStats.m38_dyne += pBattleEngine->m478_dyneReceivedFromBattle;
        }
        else
        {
            mainGameState.gameStats.m38_dyne = 9999999;
        }

        battleResultScreen_updateSub5(pBattleEngine->m474_XPReceivedFromBattle);

        pBattleEngine->m440_battleDirectionAngle = 0x8000000;
        battleEngine_InitSub2(pBattleEngine);

        pGrid->m134_desiredCameraPosition[2] = resultScreenCameraPositionPerDragonLevel[mainGameState.gameStats.m1_dragonLevel];
        pGrid->m1BC_cameraRotationStep[0] = -0x1555555;
        pGrid->m1BC_cameraRotationStep[2] = 0xaaaaaa;
        break;
    case 1:
        if (pThis->m4 == 0x27)
        {
            // print the battle rating (over grey version)
            battleResultScreen_createProgressiveText(pThis, readSaturnEA(g_BTL_A3->getSaturnPtr(0x60ab930) + pThis->m80_battleRating * 4), 0, pThis->m80_battleRating, 0);
        }

        if (--pThis->m4 < 0)
        {
            // display Dyne count
            battleResultScreen_createProgressiveText2(pThis, g_BTL_A3->getSaturnPtr(0x60ab9F0));
            battleResultScreen_createProgressiveTextNumber(pThis, g_BTL_A3->getSaturnPtr(0x60abB90), gBattleManager->m10_battleOverlay->m4_battleEngine->m478_dyneReceivedFromBattle);

            sVec2_S32 local_30;
            battleResultScreen_updateSub8(gBattleManager->m10_battleOverlay->m4_battleEngine->m478_dyneReceivedFromBattle, &local_30);

            sVec2_S16 screenPosition;
            screenPosition[0] = local_30[0];
            screenPosition[1] = local_30[1];

            if (mainGameState.gameStats.m1_dragonLevel != 8)
            {
                // display XP gain
                battleResultScreen_createProgressiveText2(pThis, g_BTL_A3->getSaturnPtr(0x60ab9E0));
                battleResultScreen_createProgressiveTextNumber(pThis, g_BTL_A3->getSaturnPtr(0x60abB80), gBattleManager->m10_battleOverlay->m4_battleEngine->m474_XPReceivedFromBattle);

                sVec2_S32 local_30;
                battleResultScreen_updateSub8(gBattleManager->m10_battleOverlay->m4_battleEngine->m474_XPReceivedFromBattle, &local_30);

                if (screenPosition[0] < local_30[0])
                {
                    sVec2_S32 local_30;
                    battleResultScreen_updateSub8(gBattleManager->m10_battleOverlay->m4_battleEngine->m474_XPReceivedFromBattle, &local_30);
                    screenPosition[1] = local_30[1];
                }
                else
                {
                    screenPosition[1] = screenPosition[0];
                }
            }

            battleResultScreen_printItemReceived(pThis->m80_battleRating);
            battleResultScreen_updateDragonAndRider(pThis->m80_battleRating);

            pThis->m72++;
            pThis->m4 = screenPosition[1] * 6;
            if (pThis->m4 < 30)
            {
                pThis->m4 = 30;
            }
        }
        break;
    case 2:
        if (--pThis->m4 < 0)
        {
            if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x16])
            {
                battleResultScreenProgressiveText_printNumber(std::array<s32, 4>{0x14, 0x18, 0xC, 0x8}, mainGameState.gameStats.m20_XP);
                battleResultScreenProgressiveText_printNumber(std::array<s32, 4>{0x14, 0x1A, 0xC, 0x8}, mainGameState.gameStats.m38_dyne);
            }

            pThis->m7A = 0;
            battleResultScreen_checkLevelUp(pThis);
            if (!gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x1E] && pThis->m74 == 0)
            {
                pThis->m72 = 6; // skip after levelup
                pThis->m4 = 0x5A;
            }
            else
            {
                assert(0);
            }
        }
        break;
    case 6:
        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & 6)
        {
            battleResultScreen_updateSub17(pThis);
            pThis->m72++;
        }
        if (--pThis->m4 < 0)
        {
            battleResultScreen_updateSub17(pThis);
            pThis->m72++;
        }
        break;
    case 7:
        pThis->getTask()->markFinished();
        break;
    default:
        assert(0);
        break;
    }

    pGrid->m1BC_cameraRotationStep[1] += 0x5B05B;
    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x17])
    {
        assert(0);
    }
}

void createBattleResultScreen(p_workArea parent)
{
    sBattleResultScreen::TypedTaskDefinition definition =
    {
        nullptr,
        battleResultScreen_update,
        nullptr,
        nullptr
    };

    createSiblingTask<sBattleResultScreen>(parent, &definition);
}
