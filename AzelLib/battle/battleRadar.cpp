#include "PDS.h"
#include "battleRadar.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"
#include "battleDebug.h"
#include "battleDragon.h"
#include "town/town.h" // todo: for npcFileDeleter
#include "battleCommandMenu.h"
#include "kernel/vdp1Allocator.h"
#include "battleGenericData.h"

void battleHud2_Init(battleHud2* pThis)
{
    pThis->m4 = 0;
    pThis->mC = 0;
    pThis->mE_offsetY = 0x47;
}

void battleHud2_Update(battleHud2* pThis)
{
    pThis->m4++;
    switch (pThis->m12_mode)
    {
    case 0: // hidden
        if (BattleEngineSub0_UpdateSub0())
        {
            if (gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode == 2)
                return;
            if (gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode == 0xE)
                return;
        }
        if (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m80000_hideBattleHUD)
        {
            pThis->m20_scrollInterpolator.m4_startValue = 0x470000;
            pThis->m20_scrollInterpolator.mC_targetValue = 0;
            pThis->m20_scrollInterpolator.m18_interpolationLength = 4;
            FPInterpolator_Init(&pThis->m20_scrollInterpolator);

            pThis->m12_mode = 4;
            pThis->m14_nextMode = 1;
        }
        break;
    case 1: // visible
        if (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m80000_hideBattleHUD)
        {
            if ((gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode == 0xC) && ((gBattleManager->m10_battleOverlay->m20_battleHud->m28_battleCommandMenu->m20 & 0x10) == 0))
            {
                pThis->m20_scrollInterpolator.m4_startValue = 0;
                pThis->m20_scrollInterpolator.mC_targetValue = -0x180000;
                pThis->m20_scrollInterpolator.m18_interpolationLength = 4;
                FPInterpolator_Init(&pThis->m20_scrollInterpolator);

                pThis->m12_mode = 4;
                pThis->m14_nextMode = 2;
            }
        }
        else
        {
            pThis->m20_scrollInterpolator.m4_startValue = 0;
            pThis->m20_scrollInterpolator.mC_targetValue = 0x470000;
            pThis->m20_scrollInterpolator.m18_interpolationLength = 4;
            FPInterpolator_Init(&pThis->m20_scrollInterpolator);

            pThis->m12_mode = 4;
            pThis->m14_nextMode = 3;
        }
        break;
    case 2: // command menu open
        if ((gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode == mC_commandMenuOpen) && ((gBattleManager->m10_battleOverlay->m20_battleHud->m28_battleCommandMenu->m20 & 0x10) != 0))
        {
            pThis->m20_scrollInterpolator.m4_startValue = -0x180000;
            pThis->m20_scrollInterpolator.mC_targetValue = 0;
            pThis->m20_scrollInterpolator.m18_interpolationLength = 4;
            FPInterpolator_Init(&pThis->m20_scrollInterpolator);

            pThis->m12_mode = 4;
            pThis->m14_nextMode = 1;
        }
        else if (gBattleManager->m10_battleOverlay->m20_battleHud->m28_battleCommandMenu == nullptr)
        {
            pThis->m20_scrollInterpolator.m4_startValue = -0x180000;
            pThis->m20_scrollInterpolator.mC_targetValue = 0x470000;
            pThis->m20_scrollInterpolator.m18_interpolationLength = 4;
            FPInterpolator_Init(&pThis->m20_scrollInterpolator);

            pThis->m12_mode = 4;
            pThis->m14_nextMode = 3;
        }
        break;
    case 3:
        if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m80000_hideBattleHUD == 0)
        {
            pThis->m20_scrollInterpolator.m4_startValue = 0x470000;
            pThis->m20_scrollInterpolator.mC_targetValue = 0;
            pThis->m20_scrollInterpolator.m18_interpolationLength = 4;
            FPInterpolator_Init(&pThis->m20_scrollInterpolator);

            pThis->m12_mode = 4;
            pThis->m14_nextMode = 1;
        }
        break;
    case 4: // scrolling
        if (FPInterpolator_Step(&pThis->m20_scrollInterpolator))
        {
            pThis->m12_mode = pThis->m14_nextMode;
        }
        pThis->mE_offsetY = fixedPoint::toInteger(pThis->m20_scrollInterpolator.m0_currentValue + 0x8000);
        break;
    default:
        assert(0);
        break;
    }
}

void battleRadar_UpdateDragonPosition(battleHud2* pThis)
{
    sVec3_FP local_20 = gBattleManager->m10_battleOverlay->m4_battleEngine->mC_battleCenter - gBattleManager->m10_battleOverlay->m18_dragon->m8_position;
    sVec2_FP local_28;

    computeVectorAngles(local_20, local_28);

    pThis->m16_dragonIconPosition[0] = MTH_Mul(0x140000, getSin(local_28[1].getInteger())).getInteger() - 7;
    pThis->m16_dragonIconPosition[1] = -pThis->mE_offsetY - MTH_Mul(0x110000, getCos(local_28[1].getInteger())).getInteger() - 0x3A;
    pThis->m1A = -0x13;
    pThis->m1C = -0x31 - pThis->mE_offsetY;
}

// This draw the radar ring
void battleHud2_DrawSub1(battleHud2* pThis)
{
    s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
    vdp1WriteEA.m0_CMDCTRL = 0x1000; // command 0
    vdp1WriteEA.m4_CMDPMOD = 0x88; // CMDPMOD
    vdp1WriteEA.m6_CMDCOLR = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2EE0; // CMDCOLR
    vdp1WriteEA.m8_CMDSRCA = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0xb18; // CMDSRCA
    vdp1WriteEA.mA_CMDSIZE = 0x523; // CMDSIZE
    vdp1WriteEA.mC_CMDXA = pThis->m1A; // CMDXA
    vdp1WriteEA.mE_CMDYA = -(pThis->m1C); // CMDYA

    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

    graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
    graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
    graphicEngineStatus.m14_vdp1Context[0].mC += 1;
}

void battleHud2_DrawSub2(battleHud2* pThis)
{
    // this draws the radar quadrant

    u16 radarStatus_danger = gBattleManager->m10_battleOverlay->m4_battleEngine->m1E0_quadrantBitStatus;
    u16 radarStatus_safe = gBattleManager->m10_battleOverlay->m4_battleEngine->m1E0_quadrantBitStatus >> 4;

    if (radarStatus_danger & 1)
    {
        //bottom
        int outputColorIndex = graphicEngineStatus.m14_vdp1Context[0].m10 - graphicEngineStatus.m14_vdp1Context[0].m14->begin();
        quadColor& outputColor = *(graphicEngineStatus.m14_vdp1Context[0].m10++);

        s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        vdp1WriteEA.m0_CMDCTRL = 0x1000; // command 0
        vdp1WriteEA.m4_CMDPMOD = 0x8C; // CMDPMOD
        vdp1WriteEA.m6_CMDCOLR = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2EE4; // CMDCOLR
        vdp1WriteEA.m8_CMDSRCA = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0xC78; // CMDSRCA
        vdp1WriteEA.mA_CMDSIZE = 0x523; // CMDSIZE
        vdp1WriteEA.mC_CMDXA   = pThis->m1A; // CMDXA
        vdp1WriteEA.mE_CMDYA   = -(pThis->m1C); // CMDYA

        outputColor[0] = readSaturnU16(g_BTL_GenericData->getSaturnPtr(0x060b1d94) + 2 * performModulo(0xf, pThis->m4));
        outputColor[1] = readSaturnU16(g_BTL_GenericData->getSaturnPtr(0x060b1d94) + 2 * (performModulo(0xf, pThis->m4) + 1));
        outputColor[2] = readSaturnU16(g_BTL_GenericData->getSaturnPtr(0x060b1d94) + 2 * (performModulo(0xf, pThis->m4) + 2));
        outputColor[3] = readSaturnU16(g_BTL_GenericData->getSaturnPtr(0x060b1d94) + 2 * (performModulo(0xf, pThis->m4) + 3));

        vdp1WriteEA.m1C_CMDGRA = outputColorIndex;

        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

        graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
        graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
        graphicEngineStatus.m14_vdp1Context[0].mC += 1;
    }

    if (radarStatus_danger & 2)
    {
        //right
        int outputColorIndex = graphicEngineStatus.m14_vdp1Context[0].m10 - graphicEngineStatus.m14_vdp1Context[0].m14->begin();
        quadColor& outputColor = *(graphicEngineStatus.m14_vdp1Context[0].m10++);

        s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        vdp1WriteEA.m0_CMDCTRL = 0x1000; // command 0
        vdp1WriteEA.m4_CMDPMOD = 0x8C; // CMDPMOD
        vdp1WriteEA.m6_CMDCOLR = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2EE4; // CMDCOLR
        vdp1WriteEA.m8_CMDSRCA = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0xBC8; // CMDSRCA
        vdp1WriteEA.mA_CMDSIZE = 0x523; // CMDSIZE
        vdp1WriteEA.mC_CMDXA = pThis->m1A; // CMDXA
        vdp1WriteEA.mE_CMDYA = -(pThis->m1C); // CMDYA

        outputColor[0] = readSaturnU16(g_BTL_GenericData->getSaturnPtr(0x060b1d94) + 2 * performModulo(0xf, pThis->m4));
        outputColor[1] = readSaturnU16(g_BTL_GenericData->getSaturnPtr(0x060b1d94) + 2 * (performModulo(0xf, pThis->m4) + 1));
        outputColor[2] = readSaturnU16(g_BTL_GenericData->getSaturnPtr(0x060b1d94) + 2 * (performModulo(0xf, pThis->m4) + 2));
        outputColor[3] = readSaturnU16(g_BTL_GenericData->getSaturnPtr(0x060b1d94) + 2 * (performModulo(0xf, pThis->m4) + 3));

        vdp1WriteEA.m1C_CMDGRA = outputColorIndex;

        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

        graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
        graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
        graphicEngineStatus.m14_vdp1Context[0].mC += 1;
    }

    if (radarStatus_danger & 4)
    {
        //top
        int outputColorIndex = graphicEngineStatus.m14_vdp1Context[0].m10 - graphicEngineStatus.m14_vdp1Context[0].m14->begin();
        quadColor& outputColor = *(graphicEngineStatus.m14_vdp1Context[0].m10++);

        s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        vdp1WriteEA.m0_CMDCTRL = 0x1000; // command 0
        vdp1WriteEA.m4_CMDPMOD = 0x8C; // CMDPMOD
        vdp1WriteEA.m6_CMDCOLR = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2EE4; // CMDCOLR
        vdp1WriteEA.m8_CMDSRCA = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0xC20; // CMDSRCA
        vdp1WriteEA.mA_CMDSIZE = 0x523; // CMDSIZE
        vdp1WriteEA.mC_CMDXA = pThis->m1A; // CMDXA
        vdp1WriteEA.mE_CMDYA = -(pThis->m1C); // CMDYA

        outputColor[0] = readSaturnU16(g_BTL_GenericData->getSaturnPtr(0x060b1d94) + 2 * performModulo(0xf, pThis->m4));
        outputColor[1] = readSaturnU16(g_BTL_GenericData->getSaturnPtr(0x060b1d94) + 2 * (performModulo(0xf, pThis->m4) + 1));
        outputColor[2] = readSaturnU16(g_BTL_GenericData->getSaturnPtr(0x060b1d94) + 2 * (performModulo(0xf, pThis->m4) + 2));
        outputColor[3] = readSaturnU16(g_BTL_GenericData->getSaturnPtr(0x060b1d94) + 2 * (performModulo(0xf, pThis->m4) + 3));

        vdp1WriteEA.m1C_CMDGRA = outputColorIndex;

        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

        graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
        graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
        graphicEngineStatus.m14_vdp1Context[0].mC += 1;
    }

    if (radarStatus_danger & 8)
    {
        //left
        int outputColorIndex = graphicEngineStatus.m14_vdp1Context[0].m10 - graphicEngineStatus.m14_vdp1Context[0].m14->begin();
        quadColor& outputColor = *(graphicEngineStatus.m14_vdp1Context[0].m10++);

        s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        vdp1WriteEA.m0_CMDCTRL = 0x1000; // command 0
        vdp1WriteEA.m4_CMDPMOD = 0x8C; // CMDPMOD
        vdp1WriteEA.m6_CMDCOLR = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2EE4; // CMDCOLR
        vdp1WriteEA.m8_CMDSRCA = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0xB70; // CMDSRCA
        vdp1WriteEA.mA_CMDSIZE = 0x523; // CMDSIZE
        vdp1WriteEA.mC_CMDXA = pThis->m1A; // CMDXA
        vdp1WriteEA.mE_CMDYA = -(pThis->m1C); // CMDYA

        outputColor[0] = readSaturnU16(g_BTL_GenericData->getSaturnPtr(0x060b1d94) + 2 * performModulo(0xf, pThis->m4));
        outputColor[1] = readSaturnU16(g_BTL_GenericData->getSaturnPtr(0x060b1d94) + 2 * (performModulo(0xf, pThis->m4) + 1));
        outputColor[2] = readSaturnU16(g_BTL_GenericData->getSaturnPtr(0x060b1d94) + 2 * (performModulo(0xf, pThis->m4) + 2));
        outputColor[3] = readSaturnU16(g_BTL_GenericData->getSaturnPtr(0x060b1d94) + 2 * (performModulo(0xf, pThis->m4) + 3));

        vdp1WriteEA.m1C_CMDGRA = outputColorIndex;

        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

        graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
        graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
        graphicEngineStatus.m14_vdp1Context[0].mC += 1;
    }

    if (radarStatus_safe & 1)
    {
        //bottom
        int outputColorIndex = graphicEngineStatus.m14_vdp1Context[0].m10 - graphicEngineStatus.m14_vdp1Context[0].m14->begin();
        quadColor& outputColor = *(graphicEngineStatus.m14_vdp1Context[0].m10++);

        s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        vdp1WriteEA.m0_CMDCTRL = 0x1000; // command 0
        vdp1WriteEA.m4_CMDPMOD = 0x8C; // CMDPMOD
        vdp1WriteEA.m6_CMDCOLR = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2EE4; // CMDCOLR
        vdp1WriteEA.m8_CMDSRCA = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0xC78; // CMDSRCA
        vdp1WriteEA.mA_CMDSIZE = 0x523; // CMDSIZE
        vdp1WriteEA.mC_CMDXA = pThis->m1A; // CMDXA
        vdp1WriteEA.mE_CMDYA = -(pThis->m1C); // CMDYA

        outputColor[0] = readSaturnU16(g_BTL_GenericData->getSaturnPtr(0x060b1e0c) + 2 * performModulo(0xf, pThis->m4));
        outputColor[1] = readSaturnU16(g_BTL_GenericData->getSaturnPtr(0x060b1e0c) + 2 * (performModulo(0xf, pThis->m4) + 1));
        outputColor[2] = readSaturnU16(g_BTL_GenericData->getSaturnPtr(0x060b1e0c) + 2 * (performModulo(0xf, pThis->m4) + 2));
        outputColor[3] = readSaturnU16(g_BTL_GenericData->getSaturnPtr(0x060b1e0c) + 2 * (performModulo(0xf, pThis->m4) + 3));

        vdp1WriteEA.m1C_CMDGRA = outputColorIndex;

        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

        graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
        graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
        graphicEngineStatus.m14_vdp1Context[0].mC += 1;
    }

    if (radarStatus_safe & 2)
    {
        //right
        int outputColorIndex = graphicEngineStatus.m14_vdp1Context[0].m10 - graphicEngineStatus.m14_vdp1Context[0].m14->begin();
        quadColor& outputColor = *(graphicEngineStatus.m14_vdp1Context[0].m10++);

        s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        vdp1WriteEA.m0_CMDCTRL = 0x1000; // command 0
        vdp1WriteEA.m4_CMDPMOD = 0x8C; // CMDPMOD
        vdp1WriteEA.m6_CMDCOLR = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2EE4; // CMDCOLR
        vdp1WriteEA.m8_CMDSRCA = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0xBC8; // CMDSRCA
        vdp1WriteEA.mA_CMDSIZE = 0x523; // CMDSIZE
        vdp1WriteEA.mC_CMDXA = pThis->m1A; // CMDXA
        vdp1WriteEA.mE_CMDYA = -(pThis->m1C); // CMDYA

        outputColor[0] = readSaturnU16(g_BTL_GenericData->getSaturnPtr(0x060b1e0c) + 2 * performModulo(0xf, pThis->m4));
        outputColor[1] = readSaturnU16(g_BTL_GenericData->getSaturnPtr(0x060b1e0c) + 2 * (performModulo(0xf, pThis->m4) + 1));
        outputColor[2] = readSaturnU16(g_BTL_GenericData->getSaturnPtr(0x060b1e0c) + 2 * (performModulo(0xf, pThis->m4) + 2));
        outputColor[3] = readSaturnU16(g_BTL_GenericData->getSaturnPtr(0x060b1e0c) + 2 * (performModulo(0xf, pThis->m4) + 3));

        vdp1WriteEA.m1C_CMDGRA = outputColorIndex;

        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

        graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
        graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
        graphicEngineStatus.m14_vdp1Context[0].mC += 1;
    }

    if (radarStatus_safe & 4)
    {
        //top
        int outputColorIndex = graphicEngineStatus.m14_vdp1Context[0].m10 - graphicEngineStatus.m14_vdp1Context[0].m14->begin();
        quadColor& outputColor = *(graphicEngineStatus.m14_vdp1Context[0].m10++);

        s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        vdp1WriteEA.m0_CMDCTRL = 0x00, 0x1000; // command 0
        vdp1WriteEA.m4_CMDPMOD = 0x04, 0x8C; // CMDPMOD
        vdp1WriteEA.m6_CMDCOLR = 0x06, dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2EE4; // CMDCOLR
        vdp1WriteEA.m8_CMDSRCA = 0x08, dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0xC20; // CMDSRCA
        vdp1WriteEA.mA_CMDSIZE = 0x0A, 0x523; // CMDSIZE
        vdp1WriteEA.mC_CMDXA = 0x0C, pThis->m1A; // CMDXA
        vdp1WriteEA.mE_CMDYA = 0x0E, -(pThis->m1C); // CMDYA

        outputColor[0] = readSaturnU16(gCurrentBattleOverlay->getSaturnPtr(0x060b1e0c) + 2 * performModulo(0xf, pThis->m4));
        outputColor[1] = readSaturnU16(gCurrentBattleOverlay->getSaturnPtr(0x060b1e0c) + 2 * (performModulo(0xf, pThis->m4) + 1));
        outputColor[2] = readSaturnU16(gCurrentBattleOverlay->getSaturnPtr(0x060b1e0c) + 2 * (performModulo(0xf, pThis->m4) + 2));
        outputColor[3] = readSaturnU16(gCurrentBattleOverlay->getSaturnPtr(0x060b1e0c) + 2 * (performModulo(0xf, pThis->m4) + 3));

        vdp1WriteEA.m1C_CMDGRA = outputColorIndex;

        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

        graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
        graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
        graphicEngineStatus.m14_vdp1Context[0].mC += 1;
    }

    if (radarStatus_safe & 8)
    {
        //left
        int outputColorIndex = graphicEngineStatus.m14_vdp1Context[0].m10 - graphicEngineStatus.m14_vdp1Context[0].m14->begin();
        quadColor& outputColor = *(graphicEngineStatus.m14_vdp1Context[0].m10++);

        s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        vdp1WriteEA.m0_CMDCTRL = 0x1000; // command 0
        vdp1WriteEA.m4_CMDPMOD = 0x8C; // CMDPMOD
        vdp1WriteEA.m6_CMDCOLR = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2EE4; // CMDCOLR
        vdp1WriteEA.m8_CMDSRCA = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0xB70; // CMDSRCA
        vdp1WriteEA.mA_CMDSIZE = 0x523; // CMDSIZE
        vdp1WriteEA.mC_CMDXA = pThis->m1A; // CMDXA
        vdp1WriteEA.mE_CMDYA = -(pThis->m1C); // CMDYA

        outputColor[0] = readSaturnU16(g_BTL_GenericData->getSaturnPtr(0x060b1e0c) + 2 * performModulo(0xf, pThis->m4));
        outputColor[1] = readSaturnU16(g_BTL_GenericData->getSaturnPtr(0x060b1e0c) + 2 * (performModulo(0xf, pThis->m4) + 1));
        outputColor[2] = readSaturnU16(g_BTL_GenericData->getSaturnPtr(0x060b1e0c) + 2 * (performModulo(0xf, pThis->m4) + 2));
        outputColor[3] = readSaturnU16(g_BTL_GenericData->getSaturnPtr(0x060b1e0c) + 2 * (performModulo(0xf, pThis->m4) + 3));

        vdp1WriteEA.m1C_CMDGRA = outputColorIndex;

        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

        graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
        graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
        graphicEngineStatus.m14_vdp1Context[0].mC += 1;
    }

    u8 radarLockIcon = gBattleManager->m10_battleOverlay->m4_battleEngine->m22F_battleRadarLockIcon;
    if (radarLockIcon & 1)
    {
        //bottom
        s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        vdp1WriteEA.m0_CMDCTRL = 0x1000; // command 0
        vdp1WriteEA.m4_CMDPMOD = 0x88; // CMDPMOD
        vdp1WriteEA.m6_CMDCOLR = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2EE8; // CMDCOLR
        vdp1WriteEA.m8_CMDSRCA = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0xC78; // CMDSRCA
        vdp1WriteEA.mA_CMDSIZE = 0x523; // CMDSIZE
        vdp1WriteEA.mC_CMDXA = pThis->m1A; // CMDXA
        vdp1WriteEA.mE_CMDYA = -(pThis->m1C); // CMDYA

        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

        graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
        graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
        graphicEngineStatus.m14_vdp1Context[0].mC += 1;
    }

    if (radarLockIcon & 2)
    {
        //right
        s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        vdp1WriteEA.m0_CMDCTRL = 0x1000; // command 0
        vdp1WriteEA.m4_CMDPMOD = 0x88; // CMDPMOD
        vdp1WriteEA.m6_CMDCOLR = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2EE8; // CMDCOLR
        vdp1WriteEA.m8_CMDSRCA = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0xBC8; // CMDSRCA
        vdp1WriteEA.mA_CMDSIZE = 0x523; // CMDSIZE
        vdp1WriteEA.mC_CMDXA = pThis->m1A; // CMDXA
        vdp1WriteEA.mE_CMDYA = -(pThis->m1C); // CMDYA

        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

        graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
        graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
        graphicEngineStatus.m14_vdp1Context[0].mC += 1;
    }

    if (radarLockIcon & 4)
    {
        //top
        s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        vdp1WriteEA.m0_CMDCTRL = 0x1000; // command 0
        vdp1WriteEA.m4_CMDPMOD = 0x88; // CMDPMOD
        vdp1WriteEA.m6_CMDCOLR = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2EE8; // CMDCOLR
        vdp1WriteEA.m8_CMDSRCA = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0xC20; // CMDSRCA
        vdp1WriteEA.mA_CMDSIZE = 0x523; // CMDSIZE
        vdp1WriteEA.mC_CMDXA = pThis->m1A; // CMDXA
        vdp1WriteEA.mE_CMDYA = -(pThis->m1C); // CMDYA

        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

        graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
        graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
        graphicEngineStatus.m14_vdp1Context[0].mC += 1;
    }

    if (radarLockIcon & 8)
    {
        s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        vdp1WriteEA.m0_CMDCTRL = 0x1000; // command 0
        vdp1WriteEA.m4_CMDPMOD = 0x88; // CMDPMOD
        vdp1WriteEA.m6_CMDCOLR = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2EE8; // CMDCOLR
        vdp1WriteEA.m8_CMDSRCA = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0xB70; // CMDSRCA
        vdp1WriteEA.mA_CMDSIZE = 0x523; // CMDSIZE
        vdp1WriteEA.mC_CMDXA = pThis->m1A; // CMDXA
        vdp1WriteEA.mE_CMDYA = -(pThis->m1C); // CMDYA

        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

        graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
        graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
        graphicEngineStatus.m14_vdp1Context[0].mC += 1;
    }

    FunctionUnimplemented();
}

void battleHud2_DrawSub3(battleHud2* pThis)
{
    // this draws the triangle cursor in the middle of the radar
    int outputColorIndex = graphicEngineStatus.m14_vdp1Context[0].m10 - graphicEngineStatus.m14_vdp1Context[0].m14->begin();
    quadColor& outputColor = *(graphicEngineStatus.m14_vdp1Context[0].m10++);

    s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
    vdp1WriteEA.m0_CMDCTRL = 0x1000; // command 0
    vdp1WriteEA.m4_CMDPMOD = 0x8C; // CMDPMOD
    vdp1WriteEA.m6_CMDCOLR = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2ED8; // CMDCOLR
    vdp1WriteEA.m8_CMDSRCA = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0xB14; // CMDSRCA
    vdp1WriteEA.mA_CMDSIZE = 0x108; // CMDSIZE
    vdp1WriteEA.mC_CMDXA = pThis->m1A + 0x10; // CMDXA
    vdp1WriteEA.mE_CMDYA = -(pThis->m1C - 0xD); // CMDYA

    outputColor[0] = readSaturnU16(g_BTL_GenericData->getSaturnPtr(0x060b1d2c) + 2 * performModulo(0xd, pThis->m4));
    outputColor[1] = readSaturnU16(g_BTL_GenericData->getSaturnPtr(0x060b1d2c) + 2 * (performModulo(0xd, pThis->m4) + 1));
    outputColor[2] = readSaturnU16(g_BTL_GenericData->getSaturnPtr(0x060b1d2c) + 2 * (performModulo(0xd, pThis->m4) + 2));
    outputColor[3] = readSaturnU16(g_BTL_GenericData->getSaturnPtr(0x060b1d2c) + 2 * (performModulo(0xd, pThis->m4) + 3));

    vdp1WriteEA.m1C_CMDGRA = outputColorIndex;

    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

    graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
    graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
    graphicEngineStatus.m14_vdp1Context[0].mC += 1;
}

void battleHud2_DrawSub4(battleHud2* pThis)
{
    // this draw the dragon icon on radar
    s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
    vdp1WriteEA.m0_CMDCTRL = 0x1000; // command 0
    vdp1WriteEA.m4_CMDPMOD = 0x88; // CMDPMOD
    vdp1WriteEA.m6_CMDCOLR = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2EDC; // CMDCOLR
    vdp1WriteEA.m8_CMDSRCA = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0xB04; // CMDSRCA
    vdp1WriteEA.mA_CMDSIZE = 0x210; // CMDSIZE
    vdp1WriteEA.mC_CMDXA = pThis->m16_dragonIconPosition[0]; // CMDXA
    vdp1WriteEA.mE_CMDYA = -pThis->m16_dragonIconPosition[1]; // CMDYA

    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

    graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
    graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
    graphicEngineStatus.m14_vdp1Context[0].mC += 1;
}


void battleHud2_Draw(battleHud2* pThis)
{
    if (gBattleManager->m10_battleOverlay->m4_battleEngine->m3A4_prelockMode[1] & 0x10) {
        if (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m8_showingBattleResultScreen)
        {
            if (BattleEngineSub0_UpdateSub0())
            {
                if (gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode == 2)
                    return;
                if (gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode == 0xE)
                    return;
            }

            battleRadar_UpdateDragonPosition(pThis);
            battleHud2_DrawSub1(pThis);
            battleHud2_DrawSub2(pThis);
            battleHud2_DrawSub3(pThis);
            battleHud2_DrawSub4(pThis);

            if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x18])
            {
                assert(0);
            }
        }
        
    }
}

void battleEngine_CreateRadar(npcFileDeleter* parent)
{
    static const battleHud2::TypedTaskDefinition definition = {
        &battleHud2_Init,
        &battleHud2_Update,
        &battleHud2_Draw,
        nullptr,
    };

    battleHud2* pNewTask = createSubTask<battleHud2>(parent, &definition);
    pNewTask->m0_vdp1Memory = parent->m4_vd1Allocation->m4_vdp1Memory;
}
