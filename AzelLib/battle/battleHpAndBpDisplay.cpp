#include "PDS.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"
#include "battleHud.h"
#include "kernel/graphicalObject.h"
#include "BTL_A3/BTL_A3_data.h"
#include "kernel/vdp1Allocator.h"

struct sBattleHpOrBpDisplay : s_workAreaTemplateWithCopy<sBattleHpOrBpDisplay>
{
    s16* m0_current;
    s16 m4_currentValue;
    s16* m8_max;
    sVec2_S16* mC_screenPosition;
    u16 m10_vdp1Offset;
    const quadColor* m14;
    u32 m18;
    s8 m1C_isHP;
    // size 0x20
};

const quadColor HpOrBpDisplay_table0 = {
    0xC210, 0xC210, 0xC210, 0xC210
};

const quadColor HpOrBpDisplay_table1 = {
    0x83E0, 0x83E0, 0x83E0, 0x83E0
};

const quadColor HpOrBpDisplay_table2 = {
    0x801F, 0x801F, 0x801F, 0x801F
};

const quadColor HpOrBpDisplay_table3 = {
    0x83FF, 0x83FF, 0x83FF, 0x83FF
};

void sBattleHpOrBpDisplay_update(sBattleHpOrBpDisplay* pThis)
{
    pThis->m18++;
    pThis->m14 = &HpOrBpDisplay_table0;
    if (pThis->m4_currentValue < *pThis->m0_current)
    {
        // increasing
        pThis->m14 = &HpOrBpDisplay_table1;
        if (
            (gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode == '\a') ||
            (gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode == '\b') ||
            (gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode == '\t') ||
            (gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode == '\n'))
        {
            return;
        }

        // 3 different rates
        if (*pThis->m0_current <= pThis->m4_currentValue + 50)
        {
            if (pThis->m4_currentValue + 10 < *pThis->m0_current)
            {
                pThis->m4_currentValue += 10;
            }
            else
            {
                pThis->m4_currentValue++;
            }
        }
        else
        {
            pThis->m4_currentValue += 50;
        }
    }
    else if (pThis->m4_currentValue == *pThis->m0_current)
    {
        // not changing, adjust color based on distance to max
        if (pThis->m1C_isHP)
        {
            if (*pThis->m0_current <= *pThis->m8_max / 4)
            {
                if (performModulo(2, pThis->m18))
                {
                    pThis->m14 = &HpOrBpDisplay_table2;
                }
            }
            else if (*pThis->m8_max / 2 < *pThis->m0_current)
            {
                return;
            }
            else if (performModulo(10, pThis->m18) > 4)
            {
                return;
            }
            pThis->m14 = &HpOrBpDisplay_table3;
        }
    }
    else
    {
        // decreasing
        pThis->m14 = &HpOrBpDisplay_table2;
        if (
            (gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode == '\a') ||
            (gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode == '\b') ||
            (gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode == '\t') ||
            (gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode == '\n'))
        {
            return;
        }

        // 3 different rates
        if (pThis->m4_currentValue - 50  <= *pThis->m0_current)
        {
            if (*pThis->m0_current < pThis->m4_currentValue - 10)
            {
                pThis->m4_currentValue -= 10;
            }
            else
            {
                pThis->m4_currentValue--;
            }
        }
        else
        {
            pThis->m4_currentValue -= 50;
        }
    }
}

void drawNumber(s16 value, sVec2_S16* positon, const quadColor* colors, s32 maxValue)
{
    sVec2_S16 tempPosition = *positon;
    bool bVar3 = false;

    if (maxValue <= value)
    {
        assert(0);
    }
    int count = udivsi3(10, maxValue);

    while(count)
    {
        int digitSpriteIndex = udivsi3(count, value);
        value = performModulo2(count, value);
        count = udivsi3(10, count);
        if (bVar3 || digitSpriteIndex || !value)
        {
            sSaturnPtr digitData = g_BTL_A3->getSaturnPtr(0x60b1e8c + digitSpriteIndex * 0xC);

            s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
            vdp1WriteEA.m0_CMDCTRL = 0x1000; // command 0
            vdp1WriteEA.m4_CMDPMOD = 0x148C; // CMDPMOD
            vdp1WriteEA.m6_CMDCOLR = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + readSaturnU32(digitData + 4); // CMDCOLR
            vdp1WriteEA.m8_CMDSRCA = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + readSaturnU32(digitData + 0); // CMDSRCA
            vdp1WriteEA.mA_CMDSIZE = readSaturnU16(digitData + 8); // CMDSIZE
            vdp1WriteEA.mC_CMDXA = tempPosition[0]; // CMDXA
            vdp1WriteEA.mE_CMDYA = -tempPosition[1]; // CMDYA

            // setup gradient
            *graphicEngineStatus.m14_vdp1Context[0].m10 = *colors;
            vdp1WriteEA.m1C_CMDGRA = graphicEngineStatus.m14_vdp1Context[0].m10 - graphicEngineStatus.m14_vdp1Context[0].m14->begin();
            graphicEngineStatus.m14_vdp1Context[0].m10++;

            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

            graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
            graphicEngineStatus.m14_vdp1Context[0].mC += 1;

            bVar3 = 1;
        }

        tempPosition[0] += 5;
    }
}

void sBattleHpOrBpDisplay_draw(sBattleHpOrBpDisplay* pThis)
{
    if (
        (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m8_showingBattleResultScreen) &&
        ((BattleEngineSub0_UpdateSub0() == 0) || ((gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode != eBattleModes::m2) && (gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode != eBattleModes::mE_battleIntro)))
        )
    {
        int maxValueLength;
        int maxNumDigits;
        if (pThis->m1C_isHP == 0)
        {
            maxValueLength = 1000;
            maxNumDigits = 4;
        }
        else
        {
            maxValueLength = 10000;
            maxNumDigits = 5;
        }

        if (
            (gBattleManager->m10_battleOverlay->m20_battleHud->m0 == 0) ||
            (gBattleManager->m10_battleOverlay->m20_battleHud->m0 == 1) ||
            (gBattleManager->m10_battleOverlay->m20_battleHud->m0 == 2)
            )
        {
            sVec2_S16 position;
            position[0] = gBattleManager->m10_battleOverlay->m20_battleHud->m16_part1X + (*pThis->mC_screenPosition)[0] - 0xB0;
            position[1] = -gBattleManager->m10_battleOverlay->m20_battleHud->m18_part1Y - (*pThis->mC_screenPosition)[1] + 0x70;

            drawNumber(pThis->m4_currentValue, &position, pThis->m14, maxValueLength);
            position[0] += maxNumDigits * 5;
            drawNumber(*pThis->m8_max, &position, &HpOrBpDisplay_table0, maxValueLength);
        }
    }
}

void createBattleHPOrBpDisplayTask(npcFileDeleter* parent, s16* current, s16* max, sVec2_S16* screenPosition, s32 isHP)
{
    static const sBattleHpOrBpDisplay::TypedTaskDefinition definition = {
        nullptr,
        sBattleHpOrBpDisplay_update,
        sBattleHpOrBpDisplay_draw,
        nullptr,
    };

    sBattleHpOrBpDisplay* pNewTask = createSubTaskWithCopy<sBattleHpOrBpDisplay>(parent, &definition);
    pNewTask->m10_vdp1Offset = parent->m4_vd1Allocation->m4_vdp1Memory;
    pNewTask->m0_current = current;
    pNewTask->m4_currentValue = *current;
    pNewTask->m8_max = max;
    pNewTask->mC_screenPosition = screenPosition;
    pNewTask->m1C_isHP = isHP;
    pNewTask->m18 = 0;
}
