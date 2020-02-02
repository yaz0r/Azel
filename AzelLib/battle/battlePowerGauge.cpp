#include "PDS.h"
#include "battlePowerGauge.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"
#include "battleDebug.h"
#include "battleDragon.h"
#include "battleHud.h"
#include "town/town.h" // todo: for npcFileDeleter

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

void battlePowerGauge_update(battlePowerGauge* pThis)
{
    if ((getBattleManager()->m10_battleOverlay->m18_dragon->m1C0 & 0x20) == 0)
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

    if (getBattleManager()->m10_battleOverlay->m4_battleEngine->m498 > 0)
    {
        if (!getBattleManager()->m10_battleOverlay->m4_battleEngine->m188_flags.m8)
        {
            if (pThis->m14 != pThis->m0->m0_max)
            {
                pThis->m0->m10_value = MTH_Mul(pThis->m0->mC, pThis->m0->m0_max);
            }
            pThis->m14 = pThis->m0->m0_max;

            if (!BattleEngineSub0_UpdateSub0() || getBattleManager()->m10_battleOverlay->m4_battleEngine->m188_flags.m400000)
            {
                if (pThis->m0->m0_max == 0)
                {
                    pThis->m0->m10_value = 0;
                    pThis->m0->mC = 0;
                }
                else
                {
                    pThis->mC = pThis->m0->m10_value;
                    if (!(getBattleManager()->m10_battleOverlay->m4_battleEngine->m388 & 0x400) && (pThis->m20 == 0))
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
                                FunctionUnimplemented();
                                //createSubTask()
                                if (mainGameState.gameStats.m1_dragonLevel == 8)
                                {
                                    playSoundEffect(0x20);
                                }
                                else
                                {
                                    playSoundEffect(0x19);
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

void drawGaugeVdp1(u16 mode, s16* params, u16 color, fixedPoint depth)
{
    u32 vdp1WriteEA = graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
    setVdp1VramU16(vdp1WriteEA + 0x00, 0x1004); // command 0
    setVdp1VramU16(vdp1WriteEA + 0x04, mode | 0x400); // CMDPMOD
    setVdp1VramU16(vdp1WriteEA + 0x06, color); // CMDCOLR
    //setVdp1VramU16(vdp1WriteEA + 0x08, params[0]); // CMDSRCA
    //setVdp1VramU16(vdp1WriteEA + 0x0A, -params[1]); // CMDSIZE
    setVdp1VramU16(vdp1WriteEA + 0x0C, params[0]); // CMDXA
    setVdp1VramU16(vdp1WriteEA + 0x0E, -params[1]); // CMDYA
    setVdp1VramU16(vdp1WriteEA + 0x10, params[2]); // CMDXB
    setVdp1VramU16(vdp1WriteEA + 0x12, -params[3]); // CMDYB
    setVdp1VramU16(vdp1WriteEA + 0x14, params[4]); // CMDXC
    setVdp1VramU16(vdp1WriteEA + 0x16, -params[5]); // CMDYC
    setVdp1VramU16(vdp1WriteEA + 0x18, params[6]); // CMDXD
    setVdp1VramU16(vdp1WriteEA + 0x1A, -params[7]); // CMDYD

    fixedPoint computedDepth = depth * graphicEngineStatus.m405C.m38;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = computedDepth.getInteger();
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = vdp1WriteEA >> 3;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

    graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
    graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA = vdp1WriteEA + 0x20;
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
            s16 local_14[8];
            local_14[0] = position[0] - 0xB0;
            local_14[2] = scaledValue + position[0] - 0xB0;
            local_14[1] = 0x70 - position[1];
            local_14[5] = (-param3 - position[1]) + 0x70;
            local_14[3] = local_14[1];
            local_14[4] = local_14[2];
            local_14[6] = local_14[0];
            local_14[7] = local_14[5];
            drawGaugeVdp1(0xC0, local_14, color, 0);
        }
    }
}

void battlePowerGauge_drawSub0(battlePowerGauge* pThis)
{
    FunctionUnimplemented();
}

void battlePowerGauge_draw(battlePowerGauge* pThis)
{
    pThis->m18++;
    bool bVar3 = performModulo(2, pThis->m18) != 0;

    if (getBattleManager()->m10_battleOverlay->m10_inBattleDebug->mFlags[0x16])
    {
        vdp2DebugPrintSetPosition(1, 0x11);
        vdp2PrintfSmallFont("PG VAL:%3d", pThis->m0->m10_value.getInteger());
        vdp2DebugPrintSetPosition(1, 0x12);
        vdp2PrintfSmallFont("PG MAX:%3d", pThis->m0->m0_max.getInteger());
        vdp2DebugPrintSetPosition(1, 0x13);
        vdp2PrintfSmallFont("COMBO :%1d,%1d", pThis->m0->m16_combo);
    }

    if (getBattleManager()->m10_battleOverlay->m4_battleEngine->m188_flags.m8)
    {
        return;
    }

    if (BattleEngineSub0_UpdateSub0())
    {
        if (getBattleManager()->m10_battleOverlay->m4_battleEngine->m38C_battleIntroType == 2)
            return;
        if (getBattleManager()->m10_battleOverlay->m4_battleEngine->m38C_battleIntroType == 0xE)
            return;
    }

    if ((getBattleManager()->m10_battleOverlay->m20->m0 != 0) && (getBattleManager()->m10_battleOverlay->m20->m0 != 1))
        return;

    sVec2_S16 local_34[5];

    local_34[4][0] = 0x22 + getBattleManager()->m10_battleOverlay->m20->m16_part1X;
    local_34[4][1] = 0xA3 + getBattleManager()->m10_battleOverlay->m20->m18_part1Y;
    local_34[2][0] = local_34[4][0] + 0xa5;
    local_34[2][1] = local_34[4][1] + 0x1f;
    local_34[1][0] = local_34[4][0] + 0xcc;
    local_34[1][1] = local_34[4][1] + 0x1f;
    local_34[0][0] = local_34[4][0] + 0xf3;
    local_34[0][1] = local_34[4][1] + 0x1f;

    switch (pThis->m0->m16_combo)
    {
    case 0:
        drawGauge(local_34[2], 0x24, 0, pThis->m0->m10_value, pThis->m0->m0_max, 0, 0xFFE8);
        break;
    case 1:
        drawGauge(local_34[2], 0x24, 0, pThis->m0->m0_max, pThis->m0->m0_max, 0, bVar3 ? 0xFFE8 : 0xFFFF);

        if (pThis->m27_numPowerGauges != 1)
        {
            drawGauge(local_34[1], 0x24, 0, pThis->m0->m10_value, pThis->m0->m0_max, 0, 0xFFE8);
        }
        break;
    case 2:
        drawGauge(local_34[2], 0x24, 0, pThis->m0->m0_max, pThis->m0->m0_max, 0, bVar3 ? 0xFFE8 : 0xFFFF);
        drawGauge(local_34[1], 0x24, 0, pThis->m0->m0_max, pThis->m0->m0_max, 0, bVar3 ? 0xFFE8 : 0xFFFF);
        drawGauge(local_34[0], 0x24, 0, pThis->m0->m10_value, pThis->m0->m0_max, 0, 0xFFE8);
        break;
    case 3: // all filled
        drawGauge(local_34[2], 0x24, 0, pThis->m0->m0_max, pThis->m0->m0_max, 0, bVar3 ? 0xFFE8 : 0xFFFF);
        drawGauge(local_34[1], 0x24, 0, pThis->m0->m0_max, pThis->m0->m0_max, 0, bVar3 ? 0xFFE8 : 0xFFFF);
        drawGauge(local_34[0], 0x24, 0, pThis->m0->m0_max, pThis->m0->m0_max, 0, bVar3 ? 0xFFE8 : 0xFFFF);
        break;
    default:
        assert(0);
        break;
    }

    // Draw HP gauge
    local_34[3][0] = local_34[4][0] + 0x11;
    local_34[3][1] = local_34[4][1] + 0x20;

    int currentHP = mainGameState.gameStats.m10_currentHP;
    int maxHP = mainGameState.gameStats.maxHP;
    if (currentHP > (maxHP / 4))
    {
        if (currentHP > (maxHP / 2))
        {
            drawGauge(local_34[3], 0x28, 3, currentHP, maxHP, 0, 0xA3E8);
        }
        else
        {
            drawGauge(local_34[3], 0x28, 3, currentHP, maxHP, 0, 0xA318);
        }
    }
    else
    {
        drawGauge(local_34[3], 0x28, 3, currentHP, maxHP, 0, 0xA118);
    }

    // Draw BP gauge
    local_34[3][0] = local_34[4][0] + 0x3F;
    local_34[3][1] = local_34[4][1] + 0x20;

    drawGauge(local_34[3], 0x28, 3, mainGameState.gameStats.currentBP, mainGameState.gameStats.maxBP, 0, 0xFE10);

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

    pNewTask->m0 = &getBattleManager()->m10_battleOverlay->m4_battleEngine->m3B4;
    getBattleManager()->m10_battleOverlay->m4_battleEngine->m3B4.m10_value = getBattleManager()->m10_battleOverlay->m4_battleEngine->m3B4.m8;

    pNewTask->m18 = 0;
    pNewTask->mC = pNewTask->m0->m10_value;
    pNewTask->m14 = pNewTask->m0->m0_max;
    pNewTask->m20 = 0;

    pNewTask->m24 = pNewTask->m0->m16_combo;
    pNewTask->m25 = 0;
    pNewTask->m26 = 0;
    pNewTask->m27_numPowerGauges = 3;
}
