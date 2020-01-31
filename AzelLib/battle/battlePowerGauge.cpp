#include "PDS.h"
#include "battlePowerGauge.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"
#include "battleDebug.h"
#include "battleDragon.h"
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
    s8 m27;
    //size 0x28
};

void battlePowerGauge_update(battlePowerGauge* pThis)
{
    if ((getBattleManager()->m10_battleOverlay->m18_dragon->m1C0 & 0x20) == 0)
    {
        pThis->m27 = 3;
    }
    else
    {
        pThis->m27 = 1;
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
                                if (pThis->m0->m16_combo < pThis->m27) {
                                    pThis->m20 = 0;
                                    pThis->m0->m10_value = 0;
                                    pThis->m0->mC = 0;
                                }
                                else
                                {
                                    pThis->m0->m16_combo = pThis->m27;
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

void battlePowerGauge_draw(battlePowerGauge* pThis)
{
    FunctionUnimplemented();
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
    pNewTask->m27 = 3;
}
