#include "PDS.h"
#include "battlePowerGauge.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"
#include "battleDebug.h"
#include "battleDragon.h"
#include "battleHud.h"
#include "kernel/graphicalObject.h"
#include "audio/systemSounds.h"
#include "kernel/vdp1Allocator.h"

struct battlePowerGauge : public s_workAreaTemplate<battlePowerGauge>
{
    s_battleEngine_3B4* m0;
    s8 m8;
    s8 m9;
    s32 mC;
    s32 m10;
    s32 m14;
    s32 m18;
    s32 m20;
    s8 m24;
    s8 m25;
    s8 m26;
    s8 m27_numPowerGauges;
    //size 0x28
};

struct sGaugeIncreaseEffectRoot : public s_workAreaTemplate<sGaugeIncreaseEffectRoot>
{
    s32 m0_step;
    // size 0x8
};

struct sGaugeIncreaseEffect : public s_workAreaTemplate<sGaugeIncreaseEffect>
{
    struct sInitParams
    {
        sVec3_FP* m0;
        s32 m4;
        s32 m8;
        s32 mC;
        s32 m10;
        s16 m14;
        s16 m16;
    };

    // size 0x7C
};

void sGaugeIncreaseEffect_createEffect(sGaugeIncreaseEffectRoot* pThis, sGaugeIncreaseEffect::sInitParams* param_2)
{
    Unimplemented();

    randomNumber();
    randomNumber();
    randomNumber();
}

void sGaugeIncreaseEffectRoot_Update(sGaugeIncreaseEffectRoot* pThis)
{
    if (++pThis->m0_step >= 0x14)
    {
        pThis->getTask()->markFinished();
        return;
    }

    sGaugeIncreaseEffect::sInitParams params;
    params.m0 = &gBattleManager->m10_battleOverlay->m18_dragon->mFC_hotpoints[2];
    params.m4 = randomNumber();
    params.m8 = randomNumber();
    params.m10 = 0xCCC;
    params.m14 = 10;

    switch (gBattleManager->m10_battleOverlay->m4_battleEngine->m3B4.m16_combo)
    {
    case 0: // happens when we gained a gaunge when we just used one
        params.mC = 0;
        params.m16 = 0;
        break;
    case 1:
        params.mC = 0x2000;
        params.m16 = 0;
        break;
    case 2:
        params.mC = 0x3000;
        params.m16 = 1;
        break;
    case 3:
        params.mC = 0x4000;
        params.m16 = 2;
        break;
    default:
        assert(0);
    }

    sGaugeIncreaseEffect_createEffect(pThis, &params);
}

void battlePowerGauge_update(battlePowerGauge* pThis)
{
    if ((gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 0x20) == 0)
    {
        pThis->m27_numPowerGauges = 3;
    }
    else
    {
        pThis->m27_numPowerGauges = 1;
        if (pThis->m0->m16_combo > 1)
        {
            pThis->m0->m16_combo = 1;
            pThis->m9 = pThis->m0->m16_combo;
            pThis->m8 = 1;
            pThis->m0->m10_value = pThis->m0->m0_max;
        }
    }

    if (gBattleManager->m10_battleOverlay->m4_battleEngine->m498_numEnemies > 0)
    {
        if (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m8_showingBattleResultScreen)
        {
            if (pThis->m14 != pThis->m0->m0_max)
            {
                pThis->m0->m10_value = MTH_Mul(pThis->m0->mC, pThis->m0->m0_max);
            }
            pThis->m14 = pThis->m0->m0_max;

            if (!BattleEngineSub0_UpdateSub0() || gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m400000)
            {
                if (pThis->m0->m0_max == 0)
                {
                    pThis->m0->m10_value = 0;
                    pThis->m0->mC = 0;
                }
                else
                {
                    pThis->mC = pThis->m0->m10_value;
                    if (!(gBattleManager->m10_battleOverlay->m4_battleEngine->m388 & 0x400) && (pThis->m20 == 0))
                    {
                        pThis->m0->m10_value += pThis->m0->m4;
                    }
                    if (pThis->m24 == pThis->m0->m16_combo)
                    {
                        if (pThis->m0->m0_max > pThis->m0->m10_value)
                        {
                            pThis->m20 = 0;
                            if (pThis->m0->m10_value < 1)
                            {
                                pThis->m0->m10_value = 0;
                            }
                            pThis->m0->mC = FP_Div(pThis->m0->m10_value, pThis->m0->m0_max);
                        }
                        else
                        {
                            pThis->m0->m10_value = pThis->m0->m0_max;
                            if (pThis->m20 == 0)
                            {
                                pThis->m0->m16_combo++;
                                if (pThis->m0->m16_combo < pThis->m27_numPowerGauges) {
                                    pThis->m20 = 0;
                                    pThis->m0->m10_value = 0;
                                    pThis->m0->mC = 0;
                                }
                                else
                                {
                                    pThis->m0->m16_combo = pThis->m27_numPowerGauges;
                                    pThis->m20 = 1;
                                    pThis->m0->mC = 0x10000;
                                }

                                static const sGaugeIncreaseEffectRoot::TypedTaskDefinition taskDefinition =
                                {
                                    nullptr,
                                    sGaugeIncreaseEffectRoot_Update,
                                    nullptr,
                                    nullptr,
                                };
                                createSubTask<sGaugeIncreaseEffectRoot>(pThis, &taskDefinition);

                                if (mainGameState.gameStats.m1_dragonLevel == 8)
                                {
                                    playSystemSoundEffect(0x20);
                                }
                                else
                                {
                                    playSystemSoundEffect(0x19);
                                }
                                pThis->m25 = 1;
                                pThis->m26 = 0;
                            }
                            else
                            {
                                if (pThis->m24 != pThis->m0->m16_combo)
                                {
                                    pThis->m20 = 0;
                                    pThis->m0->m10_value = 0;
                                    pThis->m0->mC = 0;
                                }
                            }
                        }
                    }
                    else
                    {
                        pThis->m0->m14++;
                        if (pThis->m20 != 0)
                        {
                            pThis->m20 = 0;
                            pThis->m0->mC = 0;
                        }
                        pThis->m0->m10_value = MTH_Mul(pThis->m0->mC, pThis->m0->m0_max);
                    }
                    pThis->m24 = pThis->m0->m16_combo;
                    pThis->m10 = pThis->m0->mC;
                }
            }
            else
            {
                if (pThis->m24 != pThis->m0->m16_combo) {
                    pThis->m0->m14++;
                    if (pThis->m20 != 0)
                    {
                        pThis->m20 = 0;
                        pThis->m0->mC = 0;
                    }
                    pThis->m0->m10_value = MTH_Mul(pThis->m0->mC, pThis->m0->m0_max);
                    pThis->m24 = pThis->m0->m16_combo;
                }
            }
        }
        
    }
}

void drawGaugeVdp1(u16 mode, std::array<sVec2_S16, 4>& params, u16 color, fixedPoint depth)
{
    s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
    vdp1WriteEA.m0_CMDCTRL = 0x1004; // command 0
    vdp1WriteEA.m4_CMDPMOD = mode | 0x400; // CMDPMOD
    vdp1WriteEA.m6_CMDCOLR = color; // CMDCOLR
    vdp1WriteEA.mC_CMDXA = params[0][0]; // CMDXA
    vdp1WriteEA.mE_CMDYA = -params[0][1]; // CMDYA
    vdp1WriteEA.m10_CMDXB = params[1][0]; // CMDXB
    vdp1WriteEA.m12_CMDYB = -params[1][1]; // CMDYB
    vdp1WriteEA.m14_CMDXC = params[2][0]; // CMDXC
    vdp1WriteEA.m16_CMDYC = -params[2][1]; // CMDYC
    vdp1WriteEA.m18_CMDXD = params[3][0]; // CMDXD
    vdp1WriteEA.m1A_CMDYD = -params[3][1]; // CMDYD

    fixedPoint computedDepth = depth * graphicEngineStatus.m405C.m38_oneOverFarClip;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = computedDepth.getInteger();
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

    graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
    graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
    graphicEngineStatus.m14_vdp1Context[0].mC += 1;

}

void drawGauge(sVec2_S16& position, s32 maxSize, s32 param3, s32 value, s32 maxValue, s32 param6, u16 color)
{
    if (param3 < 1)
    {
        param3 = 7;
    }

    if (maxValue > 0) {
        fixedPoint scaledValue = performDivision(maxValue, value * maxSize);
        if (scaledValue > maxSize)
        {
            scaledValue = maxSize;
        }
        if (scaledValue > 0)
        {
            std::array<sVec2_S16, 4> gaugeDimensions;
            gaugeDimensions[0][0] = position[0] - 0xB0;
            gaugeDimensions[1][0] = scaledValue + position[0] - 0xB0;
            gaugeDimensions[0][1] = 0x70 - position[1];
            gaugeDimensions[2][1] = (-param3 - position[1]) + 0x70;
            gaugeDimensions[1][1] = gaugeDimensions[0][1];
            gaugeDimensions[2][0] = gaugeDimensions[1][0];
            gaugeDimensions[3][0] = gaugeDimensions[0][0];
            gaugeDimensions[3][1] = gaugeDimensions[2][1];
            drawGaugeVdp1(0xC0, gaugeDimensions, color, 0);
        }
    }
}

void battlePowerGauge_drawSub0(battlePowerGauge* pThis)
{
    {
        s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        vdp1WriteEA.m0_CMDCTRL = 0x1000; // command 0
        vdp1WriteEA.m4_CMDPMOD = 0x8C; // CMDPMOD
        vdp1WriteEA.m6_CMDCOLR = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2ECC; // CMDCOLR
        vdp1WriteEA.m8_CMDSRCA = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2B8;
        vdp1WriteEA.mA_CMDSIZE = 0xB05;
        vdp1WriteEA.mC_CMDXA = gBattleManager->m10_battleOverlay->m20_battleHud->m16_part1X - 0x7E; // CMDXA
        vdp1WriteEA.mE_CMDYA = -(-0x52 - gBattleManager->m10_battleOverlay->m20_battleHud->m18_part1Y); // CMDYA

        // setup gradient
        (*graphicEngineStatus.m14_vdp1Context[0].m10)[0] = 0xC210;
        (*graphicEngineStatus.m14_vdp1Context[0].m10)[1] = 0xC210;
        (*graphicEngineStatus.m14_vdp1Context[0].m10)[2] = 0xC210;
        (*graphicEngineStatus.m14_vdp1Context[0].m10)[3] = 0xC210;
        vdp1WriteEA.m1C_CMDGRA = graphicEngineStatus.m14_vdp1Context[0].m10 - graphicEngineStatus.m14_vdp1Context[0].m14->begin();
        graphicEngineStatus.m14_vdp1Context[0].m10++;


        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

        graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
        graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
        graphicEngineStatus.m14_vdp1Context[0].mC += 1;
    }

    u16 color = 0xc210;
    if (pThis->m25 == 1)
    {
        if (performModulo2(2, pThis->m18))
        {
            color = 0xFE10;
        }
        if (++pThis->m26 > 0xF)
        {
            pThis->m25 = 0;
        }
    }

    {
        s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        vdp1WriteEA.m0_CMDCTRL = 0x1000; // command 0
        vdp1WriteEA.m4_CMDPMOD = 0x8C; // CMDPMOD
        vdp1WriteEA.m6_CMDCOLR = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2ECC; // CMDCOLR
        vdp1WriteEA.m8_CMDSRCA = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x27C;
        vdp1WriteEA.mA_CMDSIZE = 0xF08;
        vdp1WriteEA.mC_CMDXA = gBattleManager->m10_battleOverlay->m20_battleHud->m16_part1X + 0x13; // CMDXA
        vdp1WriteEA.mE_CMDYA = -(-0x52 - gBattleManager->m10_battleOverlay->m20_battleHud->m18_part1Y); // CMDYA

        // setup gradient
        (*graphicEngineStatus.m14_vdp1Context[0].m10)[0] = color;
        (*graphicEngineStatus.m14_vdp1Context[0].m10)[1] = color;
        (*graphicEngineStatus.m14_vdp1Context[0].m10)[2] = color;
        (*graphicEngineStatus.m14_vdp1Context[0].m10)[3] = color;
        vdp1WriteEA.m1C_CMDGRA = graphicEngineStatus.m14_vdp1Context[0].m10 - graphicEngineStatus.m14_vdp1Context[0].m14->begin();
        graphicEngineStatus.m14_vdp1Context[0].m10++;


        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

        graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
        graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
        graphicEngineStatus.m14_vdp1Context[0].mC += 1;
    }
}

void battlePowerGauge_draw(battlePowerGauge* pThis)
{
    pThis->m18++;
    bool bVar3 = performModulo(2, pThis->m18) != 0;

    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x16])
    {
        vdp2DebugPrintSetPosition(1, 0x11);
        vdp2PrintfSmallFont("PG VAL:%3d", pThis->m0->m10_value.getInteger());
        vdp2DebugPrintSetPosition(1, 0x12);
        vdp2PrintfSmallFont("PG MAX:%3d", pThis->m0->m0_max.getInteger());
        vdp2DebugPrintSetPosition(1, 0x13);
        vdp2PrintfSmallFont("COMBO :%1d,%1d", pThis->m0->m16_combo);
    }

    if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m8_showingBattleResultScreen)
    {
        return;
    }

    if (BattleEngineSub0_UpdateSub0())
    {
        if (gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode == 2)
            return;
        if (gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode == 0xE)
            return;
    }

    if ((gBattleManager->m10_battleOverlay->m20_battleHud->m0 != 0) && (gBattleManager->m10_battleOverlay->m20_battleHud->m0 != 1))
        return;

    sVec2_S16 UIStart;

    UIStart[0] = 0x22 + gBattleManager->m10_battleOverlay->m20_battleHud->m16_part1X;
    UIStart[1] = 0xA3 + gBattleManager->m10_battleOverlay->m20_battleHud->m18_part1Y;

    sVec2_S16 gauge0;
    sVec2_S16 gauge1;
    sVec2_S16 gauge2;
    gauge0[0] = UIStart[0] + 0xa5;
    gauge0[1] = UIStart[1] + 0x1f;
    gauge1[0] = UIStart[0] + 0xcc;
    gauge1[1] = UIStart[1] + 0x1f;
    gauge2[0] = UIStart[0] + 0xf3;
    gauge2[1] = UIStart[1] + 0x1f;

    switch (pThis->m0->m16_combo)
    {
    case 0:
        drawGauge(gauge0, 0x24, 0, pThis->m0->m10_value, pThis->m0->m0_max, 0, 0xFFE8);
        break;
    case 1:
        drawGauge(gauge0, 0x24, 0, pThis->m0->m0_max, pThis->m0->m0_max, 0, bVar3 ? 0xFFE8 : 0xFFFF);

        if (pThis->m27_numPowerGauges != 1)
        {
            drawGauge(gauge1, 0x24, 0, pThis->m0->m10_value, pThis->m0->m0_max, 0, 0xFFE8);
        }
        break;
    case 2:
        drawGauge(gauge0, 0x24, 0, pThis->m0->m0_max, pThis->m0->m0_max, 0, bVar3 ? 0xFFE8 : 0xFFFF);
        drawGauge(gauge1, 0x24, 0, pThis->m0->m0_max, pThis->m0->m0_max, 0, bVar3 ? 0xFFE8 : 0xFFFF);
        drawGauge(gauge2, 0x24, 0, pThis->m0->m10_value, pThis->m0->m0_max, 0, 0xFFE8);
        break;
    case 3: // all filled
        drawGauge(gauge0, 0x24, 0, pThis->m0->m0_max, pThis->m0->m0_max, 0, bVar3 ? 0xFFE8 : 0xFFFF);
        drawGauge(gauge1, 0x24, 0, pThis->m0->m0_max, pThis->m0->m0_max, 0, bVar3 ? 0xFFE8 : 0xFFFF);
        drawGauge(gauge2, 0x24, 0, pThis->m0->m0_max, pThis->m0->m0_max, 0, bVar3 ? 0xFFE8 : 0xFFFF);
        break;
    default:
        assert(0);
        break;
    }

    // Draw HP gauge
    sVec2_S16 tempCoordinates;
    tempCoordinates[0] = UIStart[0] + 0x11;
    tempCoordinates[1] = UIStart[1] + 0x20;

    int currentHP = mainGameState.gameStats.m10_currentHP;
    int maxHP = mainGameState.gameStats.mB8_maxHP;
    if (currentHP > (maxHP / 4))
    {
        if (currentHP > (maxHP / 2))
        {
            drawGauge(tempCoordinates, 0x28, 3, currentHP, maxHP, 0, 0xA3E8);
        }
        else
        {
            drawGauge(tempCoordinates, 0x28, 3, currentHP, maxHP, 0, 0xA318);
        }
    }
    else
    {
        drawGauge(tempCoordinates, 0x28, 3, currentHP, maxHP, 0, 0xA118);
    }

    // Draw BP gauge
    tempCoordinates[0] = UIStart[0] + 0x3F;
    tempCoordinates[1] = UIStart[1] + 0x20;

    drawGauge(tempCoordinates, 0x28, 3, mainGameState.gameStats.m14_currentBP, mainGameState.gameStats.mBA_maxBP, 0, 0xFE10);

    battlePowerGauge_drawSub0(pThis);
}


void createPowerGaugeTask(npcFileDeleter* parent, u16*, s32)
{
    static const battlePowerGauge::TypedTaskDefinition definition = {
        nullptr,
        &battlePowerGauge_update,
        &battlePowerGauge_draw,
        nullptr,
    };

    battlePowerGauge* pNewTask = createSubTask<battlePowerGauge>(parent, &definition);

    pNewTask->m0 = &gBattleManager->m10_battleOverlay->m4_battleEngine->m3B4;
    gBattleManager->m10_battleOverlay->m4_battleEngine->m3B4.m10_value = gBattleManager->m10_battleOverlay->m4_battleEngine->m3B4.m8;

    pNewTask->m18 = 0;
    pNewTask->mC = pNewTask->m0->m10_value;
    pNewTask->m14 = pNewTask->m0->m0_max;
    pNewTask->m20 = 0;

    pNewTask->m24 = pNewTask->m0->m16_combo;
    pNewTask->m25 = 0;
    pNewTask->m26 = 0;
    pNewTask->m27_numPowerGauges = 3;
}
