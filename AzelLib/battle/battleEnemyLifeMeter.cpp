#include "PDS.h"
#include "kernel/graphicalObject.h"
#include "kernel/vdp1Allocator.h"
#include "battleEnemyLifeMeter.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleDragon.h"
#include "battleEngine.h"
#include "battleDebug.h"
#include "commonOverlay.h"

void drawGaugeVdp1(u16 mode, std::array<sVec2_S16, 4>& params, u16 color, fixedPoint depth); // TODO: cleanup
void drawLifeMeterBorder(s32* param);

void createEnemyLifeMeterTask_update(sEnemyLifeMeterTask* pThis)
{
    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x1D])
    {
        vdp2DebugPrintSetPosition(3, performModulo(10, pThis->m2C_entryIndex) + 10);
        vdp2PrintfSmallFont("no %d hp (%4d,%4d,%d) %d ", pThis->m2C_entryIndex, *pThis->m24, pThis->m1C.toInteger(), pThis->m20.toInteger(), performModulo2(10, randomNumber()));
    }

    switch (pThis->m30)
    {
    case 0:
        if (pThis->m20 > 0)
        {
            pThis->m1C += pThis->m20;
            if (fixedPoint::fromInteger(*pThis->m24) <= pThis->m1C)
            {
                pThis->m1C = fixedPoint::fromInteger(*pThis->m24);
                pThis->m20 = 0;
            }
        }
        else if (pThis->m20 < 0)
        {
            pThis->m1C += pThis->m20;
            if (pThis->m1C <= fixedPoint::fromInteger(*pThis->m24))
            {
                pThis->m1C = fixedPoint::fromInteger(*pThis->m24);
                pThis->m20 = 0;
            }
        }

        if (pThis->m31 & 2)
        {
            pThis->m20 = FP_Div(fixedPoint::fromInteger(*pThis->m24) - pThis->m1C, fixedPoint::fromInteger(24));
            pThis->m2A = 30;
            pThis->m31 &= ~2;
            if (*pThis->m24 < 1)
            {
                pThis->m30 = 1;
            }
        }

        if (pThis->m31 & 8)
        {
            fixedPoint temp = FP_Div(- pThis->m1C, fixedPoint::fromInteger(24));
            if (temp < pThis->m20)
            {
                pThis->m20 = temp;
            }
            pThis->m30 = 1;
            if (pThis->m0)
            {
                pThis->m10 = *pThis->m0;
                pThis->m0 = nullptr;
            }
        }

        if (pThis->m2A > -1)
        {
            if (--pThis->m2A < 1)
            {
                pThis->m31 &= ~0xF0;
            }
        }
        break;
    case 1:
        if (pThis->m20 < 0)
        {
            pThis->m1C += pThis->m20;
            if (pThis->m1C < 1)
            {
                pThis->m1C = 0;
                pThis->m20 = 0;
            }
        }
        else if (pThis->m20 > -1)
        {
            pThis->m1C = 0;
            pThis->m20 = 0;
        }

        if (!(pThis->m31 & 4))
        {
            if (pThis->m31 & 8)
            {
                if (pThis->m0)
                {
                    pThis->m10 = *pThis->m0;
                    pThis->m0 = nullptr;
                }
                if (pThis->m1C < fixedPoint::fromInteger(500))
                {
                    pThis->m2E_width -= 3;
                    if (pThis->m2E_width < 2)
                    {
                        pThis->m2E_width = 1;
                        pThis->getTask()->markFinished();
                    }
                }
            }
        }
        else
        {
            if (pThis->m20 == 0)
            {
                pThis->m30 = 0;
                pThis->m31 &= 0xD;
                pThis->m2A = 0;
            }
        }
        break;
    default:
        assert(0);
    }

    if ((pThis->m2C_entryIndex > -1) && (pThis->m31 & 1))
    {
        gBattleManager->m10_battleOverlay->m4_battleEngine->m474_XPReceivedFromBattle += readSaturnS16(gCommonFile->getSaturnPtr(0x201798 + pThis->m2C_entryIndex * 8));
        gBattleManager->m10_battleOverlay->m4_battleEngine->m478_dyneReceivedFromBattle += readSaturnS16(gCommonFile->getSaturnPtr(0x20179A + pThis->m2C_entryIndex * 8));
        pThis->m31 &= ~1;
    }
}

const std::array<u16, 10> enemyLifeGaugeColors = {
    0x801F, 0x825F, 0x83FF, 0x83F2,
    0x82C0, 0xD3E0, 0xFE60, 0xFC80,
    0xFC4F, 0xECD7
};

// BTL_A3::060620b0
void drawLifeMeterBorder(s32* param)
{
    s_vdp1Context& ctx = graphicEngineStatus.m14_vdp1Context[0];
    u16 vdp1Base = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory;
    s16 depth = (s16)((graphicEngineStatus.m405C.m38_oneOverFarClip.m_value << 12) >> 16);

    s16 x     = (s16)param[0];
    s16 y     = (s16)param[1];
    s16 xRight = (s16)param[2];
    s16 yTop  = (s16)param[3];

    // Command 0: left border indicator
    {
        s_vdp1Command& cmd = ctx.m0_currentVdp1WriteEA[0];
        cmd.m0_CMDCTRL = 0x1001;
        cmd.m4_CMDPMOD = 0x88;
        cmd.m6_CMDCOLR = vdp1Base + 0x2ed8;
        cmd.m8_CMDSRCA = vdp1Base + 0xaf8;
        cmd.mA_CMDSIZE = 0x108;
        cmd.mC_CMDXA = x - 3;
        cmd.mE_CMDYA = -y;
        cmd.m14_CMDXC = x + 3;
        cmd.m16_CMDYC = -yTop;
        ctx.m20_pCurrentVdp1Packet->m4_bucketTypes = depth;
        ctx.m20_pCurrentVdp1Packet->m6_vdp1EA = &ctx.m0_currentVdp1WriteEA[0];
        ctx.m20_pCurrentVdp1Packet++;
        ctx.m1C += 1;
        ctx.mC += 1;
    }

    // Command 1: right border indicator
    {
        s_vdp1Command& cmd = ctx.m0_currentVdp1WriteEA[1];
        cmd.m0_CMDCTRL = 0x1001;
        cmd.m4_CMDPMOD = 0x88;
        cmd.m6_CMDCOLR = vdp1Base + 0x2ed8;
        cmd.m8_CMDSRCA = vdp1Base + 0xb00;
        cmd.mA_CMDSIZE = 0x108;
        cmd.mC_CMDXA = xRight - 3;
        cmd.mE_CMDYA = -y;
        cmd.m14_CMDXC = xRight + 3;
        cmd.m16_CMDYC = -yTop;
        ctx.m20_pCurrentVdp1Packet->m4_bucketTypes = depth;
        ctx.m20_pCurrentVdp1Packet->m6_vdp1EA = &ctx.m0_currentVdp1WriteEA[1];
        ctx.m20_pCurrentVdp1Packet++;
        ctx.m1C += 1;
        ctx.mC += 1;
    }

    // Command 2: full-width border
    {
        s_vdp1Command& cmd = ctx.m0_currentVdp1WriteEA[2];
        cmd.m0_CMDCTRL = 0x1001;
        cmd.m4_CMDPMOD = 0x88;
        cmd.m6_CMDCOLR = vdp1Base + 0x2ed8;
        cmd.m8_CMDSRCA = vdp1Base + 0xafc;
        cmd.mA_CMDSIZE = 0x108;
        cmd.mC_CMDXA = x;
        cmd.mE_CMDYA = -y;
        cmd.m14_CMDXC = xRight;
        cmd.m16_CMDYC = -yTop;
        ctx.m20_pCurrentVdp1Packet->m4_bucketTypes = depth;
        ctx.m20_pCurrentVdp1Packet->m6_vdp1EA = &ctx.m0_currentVdp1WriteEA[2];
        ctx.m20_pCurrentVdp1Packet++;
        ctx.m1C += 1;
        ctx.m0_currentVdp1WriteEA += 3;
        ctx.mC += 1;
    }
}

void createEnemyLifeMeterTask_draw(sEnemyLifeMeterTask* pThis)
{
    if (pThis->m2A > -1)
    {
        u8 clipFlags = 0;

        sVec3_FP* psVar2;
        if (pThis->m0 == nullptr)
        {
            psVar2 = &pThis->m10;
        }
        else
        {
            psVar2 = pThis->m0;
        }

        // TODO: refactorize this (and other places) as a project to screen function
        sVec3_FP projectedPosition;
        transformAndAddVecByCurrentMatrix(psVar2, &projectedPosition);
        fixedPoint oneOverZ = FP_Div(0x10000, projectedPosition[2]);
        projectedPosition[0] = MTH_Mul_5_6(graphicEngineStatus.m405C.m18_widthScale, projectedPosition[0], oneOverZ);
        projectedPosition[1] = MTH_Mul_5_6(graphicEngineStatus.m405C.m1C_heightScale, projectedPosition[1], oneOverZ);

        if (projectedPosition[0] < -165)
        {
            projectedPosition[0] = -166;
            clipFlags |= 0x20;
        }
        else if (projectedPosition[0] > 90)
        {
            projectedPosition[0] = 91;
            clipFlags |= 0x10;
        }

        if (projectedPosition[1] < -69)
        {
            projectedPosition[1] = -70;
            clipFlags |= 0x40;
        }
        else if (projectedPosition[1] > 91)
        {
            projectedPosition[1] = 92;
            clipFlags |= 0x80;
        }

        u8 uVar3 = (pThis->m31 & 0xF0) | clipFlags;
        if (((uVar3 & 0x30) == 0x30) || ((uVar3 & 0xC0) == 0xC0))
        {
            projectedPosition = pThis->m4_lastSafeProjectedPosition;
        }
        else
        {
            pThis->m4_lastSafeProjectedPosition = projectedPosition;
            pThis->m31 &= 0xF;
            pThis->m31 |= clipFlags;
        }

        int tempValue = pThis->m1C.toInteger();
        int numGaugeCount = 0;
        while (tempValue > 500)
        {
            tempValue -= 500;
            numGaugeCount++;
        }

        if ((tempValue != 500) && numGaugeCount)
        {
            // draw the background gauge
            std::array<sVec2_S16, 4> gaugeDimensions;
            gaugeDimensions[0][0] = projectedPosition[0];
            gaugeDimensions[1][0] = projectedPosition[0] + pThis->m2E_width;
            gaugeDimensions[0][1] = projectedPosition[1];
            gaugeDimensions[2][1] = projectedPosition[1] - 8;
            gaugeDimensions[1][1] = gaugeDimensions[0][1];
            gaugeDimensions[2][0] = gaugeDimensions[1][0];
            gaugeDimensions[3][0] = gaugeDimensions[0][0];
            gaugeDimensions[3][1] = gaugeDimensions[2][1];

            int colorIndex = std::min<int>(numGaugeCount - 1, 8);

            drawGaugeVdp1(0xC0, gaugeDimensions, enemyLifeGaugeColors[colorIndex], 0x1000);
        }

        if (tempValue > 0)
        {
            // draw the current gauge
            std::array<sVec2_S16, 4> gaugeDimensions;
            s16 scaledWidth = (s16)fixedPoint::toInteger(FP_Div(fixedPoint::fromInteger(tempValue), fixedPoint::fromInteger(500)) * 50);
            if (pThis->m2E_width < scaledWidth)
            {
                scaledWidth = pThis->m2E_width;
            }
            gaugeDimensions[0][0] = projectedPosition[0];
            gaugeDimensions[1][0] = gaugeDimensions[0][0] + scaledWidth;
            gaugeDimensions[0][1] = projectedPosition[1];
            gaugeDimensions[2][1] = projectedPosition[1] - 8;
            gaugeDimensions[1][1] = gaugeDimensions[0][1];
            gaugeDimensions[2][0] = gaugeDimensions[1][0];
            gaugeDimensions[3][0] = gaugeDimensions[0][0];
            gaugeDimensions[3][1] = gaugeDimensions[2][1];

            int colorIndex = std::min<int>(numGaugeCount, 9);

            drawGaugeVdp1(0xC0, gaugeDimensions, enemyLifeGaugeColors[colorIndex], 0x1000);
        }

        s32 borderParams[4] = {
            projectedPosition[0].m_value,
            projectedPosition[1].m_value,
            (s32)pThis->m2E_width + projectedPosition[0].m_value,
            projectedPosition[1].m_value - 8
        };
        drawLifeMeterBorder(borderParams);
    }
}

// BTL_A3::06062096
void createEnemyLifeMeterTask_delete(sEnemyLifeMeterTask* pThis)
{
}

sEnemyLifeMeterTask* createEnemyLifeMeterTask(sVec3_FP* arg0, s32 arg1, s16* arg2, s16 arg3)
{
    static const sEnemyLifeMeterTask::TypedTaskDefinition definition = {
        nullptr,
        createEnemyLifeMeterTask_update,
        createEnemyLifeMeterTask_draw,
        createEnemyLifeMeterTask_delete,
    };

    sEnemyLifeMeterTask* pNewTask = createSubTask<sEnemyLifeMeterTask>(gBattleManager->m10_battleOverlay->m18_dragon, &definition);

    pNewTask->m0 = arg0;
    pNewTask->m24 = arg2;
    pNewTask->m2C_entryIndex = arg3;

    if (arg3 < 0)
    {
        pNewTask->m28 = arg1;
    }
    else
    {
        pNewTask->m28 = readSaturnS16(gCommonFile->getSaturnPtr(0x20179c + arg3 * 4 * 2));
        *arg2 = pNewTask->m28;
    }

    pNewTask->m1C = fixedPoint::fromInteger(*arg2);

    if (pNewTask->m28 < 500)
    {
        pNewTask->m2E_width = fixedPoint::toInteger(FP_Div(fixedPoint::fromInteger(pNewTask->m28), fixedPoint::fromInteger(500)) * 50);
    }
    else
    {
        pNewTask->m2E_width = 50;
    }

    for (int i=0; i<9; i++)
    {
        if (gBattleManager->m10_battleOverlay->m4_battleEngine->m484[i] == pNewTask->m2C_entryIndex)
        {
            break;
        }

        if (gBattleManager->m10_battleOverlay->m4_battleEngine->m484[i] == -1)
        {
            gBattleManager->m10_battleOverlay->m4_battleEngine->m484[i] = pNewTask->m2C_entryIndex;
            break;
        }
    }

    return pNewTask;
}

