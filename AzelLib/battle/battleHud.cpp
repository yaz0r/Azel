#include "PDS.h"
#include "battleHud.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "kernel/vdp1Allocator.h"
#include "kernel/graphicalObject.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"
#include "battleDebug.h"
#include "battlePowerGauge.h"
#include "battleCommandMenu.h"
#include "battleHpAndBpDisplay.h"
#include "battleGenericData.h"
#include "battleDragon.h"

void s_battleOverlay_20_update(s_battleOverlay_20* pThis)
{
    pThis->m2C++;
    switch (pThis->m10_currentMode)
    {
    case 0: //hidden
        if (battleEngine_isPlayerTurnActive())
        {
            if (gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode == 2)
                return;
            if (gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode == 0xE)
                return;
        }
        if (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m80000_hideBattleHUD)
        {
            pThis->m30.m4_startValue = 0x470000;
            pThis->m30.mC_targetValue = 0;
            pThis->m30.m18_interpolationLength = 4;
            FPInterpolator_Init(&pThis->m30);

            pThis->m4C.m4_startValue = 0x470000;
            pThis->m4C.mC_targetValue = 0x470000;
            pThis->m4C.m18_interpolationLength = 4;
            FPInterpolator_Init(&pThis->m4C);
            pThis->m10_currentMode = 3;
            pThis->m12_nextMode = 1;
        }
        break;
    case 1: //visible
        if (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m80000_hideBattleHUD)
        {
            if ((gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode == 0xC) && ((gBattleManager->m10_battleOverlay->m20_battleHud->m28_battleCommandMenu->m20 & 0x10) == 0))
            {
                // open command menu, scroll up
                pThis->m30.m4_startValue = 0;
                pThis->m30.mC_targetValue = -0x180000;
                pThis->m30.m18_interpolationLength = 4;
                FPInterpolator_Init(&pThis->m30);

                pThis->m4C.m4_startValue = 0x470000;
                pThis->m4C.mC_targetValue = 0x180000;
                pThis->m4C.m18_interpolationLength = 4;
                FPInterpolator_Init(&pThis->m4C);
                pThis->m10_currentMode = 3;
                pThis->m12_nextMode = 2;
            }
        }
        else
        {
            // start hiding
            pThis->m30.m4_startValue = 0;
            pThis->m30.mC_targetValue = 0x470000;
            pThis->m30.m18_interpolationLength = 4;
            FPInterpolator_Init(&pThis->m30);

            pThis->m4C.m4_startValue = 0x470000;
            pThis->m4C.mC_targetValue = 0x470000;
            pThis->m4C.m18_interpolationLength = 4;
            FPInterpolator_Init(&pThis->m4C);
            pThis->m10_currentMode = 3;
            pThis->m12_nextMode = 0;
        }
        break;
    case 2: // command menu open
        if ((gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode == 0xC) && ((gBattleManager->m10_battleOverlay->m20_battleHud->m28_battleCommandMenu->m20 & 0x10) != 0))
        {
            pThis->m30.m4_startValue = -0x180000;
            pThis->m30.mC_targetValue = 0;
            pThis->m30.m18_interpolationLength = 4;
            FPInterpolator_Init(&pThis->m30);

            pThis->m4C.m4_startValue = -0x180000;
            pThis->m4C.mC_targetValue = 0x470000;
            pThis->m4C.m18_interpolationLength = 4;
            FPInterpolator_Init(&pThis->m4C);
            pThis->m10_currentMode = 3;
            pThis->m12_nextMode = 1;
        }
        else if (gBattleManager->m10_battleOverlay->m20_battleHud->m28_battleCommandMenu == nullptr)
        {
            pThis->m30.m4_startValue = -0x180000;
            pThis->m30.mC_targetValue = 0x470000;
            pThis->m30.m18_interpolationLength = 4;
            FPInterpolator_Init(&pThis->m30);

            pThis->m4C.m4_startValue = -0x180000;
            pThis->m4C.mC_targetValue = 0x470000;
            pThis->m4C.m18_interpolationLength = 4;
            FPInterpolator_Init(&pThis->m4C);
            pThis->m10_currentMode = 3;
            pThis->m12_nextMode = 0;
        }
        break;
    case 3: // scrolling
        if (FPInterpolator_Step(&pThis->m30))
        {
            pThis->m10_currentMode = pThis->m12_nextMode;
        }
        FPInterpolator_Step(&pThis->m4C);
        pThis->m18_part1Y = (pThis->m30.m0_currentValue + 0x8000) >> 0x10;
        pThis->m1C_part2Y = (pThis->m4C.m0_currentValue + 0x8000) >> 0x10;
        break;
    default:
        assert(0);
        break;
    }
}

void s_battleOverlay_20_drawSub0(s_battleOverlay_20* pThis)
{
    {
        s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        vdp1WriteEA.m0_CMDCTRL = 0x1001; // command 0
        vdp1WriteEA.m4_CMDPMOD = 0x88; // CMDPMOD
        vdp1WriteEA.m6_CMDCOLR = pThis->m14_vdp1Memory + 0x2ebc; // CMDCOLR
        vdp1WriteEA.m8_CMDSRCA = pThis->m14_vdp1Memory + 0x278; // CMDSRCA
        vdp1WriteEA.mA_CMDSIZE = 0x108; // CMDSIZE
        vdp1WriteEA.mC_CMDXA = pThis->m1A_part2X - 0x82; // CMDXA
        vdp1WriteEA.mE_CMDYA = -(-0x5C - pThis->m1C_part2Y); // CMDYA
        vdp1WriteEA.m14_CMDXC = pThis->m1A_part2X + 0x87; // CMDXC
        vdp1WriteEA.m16_CMDYC = -(-0x73 - pThis->m1C_part2Y); // CMDYC

        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

        graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
        graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
        graphicEngineStatus.m14_vdp1Context[0].mC += 1;
    }

    sSaturnPtr spriteDataTable = g_BTL_GenericData->getSaturnPtr(0x60b1a40);
    sSaturnPtr currentSprite = g_BTL_GenericData->getSaturnPtr(0x60b19b8);
    for(int i=0; i<4; i++)
    {
        sSaturnPtr spriteData = readSaturnEA(spriteDataTable + 4 * i);

        s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        vdp1WriteEA.m0_CMDCTRL = 0x1000; // command 0
        vdp1WriteEA.m4_CMDPMOD = readSaturnS16(spriteData + 10); // CMDPMOD
        vdp1WriteEA.m6_CMDCOLR = readSaturnS16(spriteData + 6) + dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory; // CMDCOLR
        vdp1WriteEA.m8_CMDSRCA = readSaturnS16(spriteData + 2) + dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory; // CMDSRCA
        vdp1WriteEA.mA_CMDSIZE = readSaturnS16(spriteData + 8); // CMDSIZE
        vdp1WriteEA.mC_CMDXA = readSaturnS16(currentSprite) + pThis->m1A_part2X - 0xb0; // CMDXA
        vdp1WriteEA.mE_CMDYA = -(-pThis->m1C_part2Y - readSaturnS16(currentSprite + 2)) + 0x70; // CMDYA

        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

        graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
        graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
        graphicEngineStatus.m14_vdp1Context[0].mC += 1;

        currentSprite += 2;
    }

    {
        s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        vdp1WriteEA.m0_CMDCTRL = 0x1000; // command 0
        vdp1WriteEA.m4_CMDPMOD = 0x88; // CMDPMOD
        vdp1WriteEA.m6_CMDCOLR = 0x2ED0 + dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory; // CMDCOLR
        vdp1WriteEA.m8_CMDSRCA = 0x554 + dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory; // CMDSRCA
        vdp1WriteEA.mA_CMDSIZE = 0x252e; // CMDSIZE
        vdp1WriteEA.mC_CMDXA = pThis->m16_part1X - 0x8E; // CMDXA
        vdp1WriteEA.mE_CMDYA = -(-0x33 - pThis->m18_part1Y); // CMDYA

        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

        graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
        graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
        graphicEngineStatus.m14_vdp1Context[0].mC += 1;
    }

    {
        s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        vdp1WriteEA.m0_CMDCTRL = 0x1000; // command 0
        vdp1WriteEA.m4_CMDPMOD = 0x88; // CMDPMOD
        vdp1WriteEA.m6_CMDCOLR = 0x2ED0 + dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory; // CMDCOLR
        vdp1WriteEA.m8_CMDSRCA = 0x4d4 + dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory; // CMDSRCA
        vdp1WriteEA.mA_CMDSIZE = 0x533; // CMDSIZE
        vdp1WriteEA.mC_CMDXA = pThis->m16_part1X - 0xB0; // CMDXA
        vdp1WriteEA.mE_CMDYA = -(-0x31 - pThis->m18_part1Y); // CMDYA

        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

        graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
        graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
        graphicEngineStatus.m14_vdp1Context[0].mC += 1;
    }

}

extern const quadColor HpOrBpDisplay_table0;
extern const quadColor HpOrBpDisplay_table2;

// 0607242e
void battleHud_drawStatusString(s_battleOverlay_20* pThis)
{
    s32 spriteEntryIndex = 0;
    u32 activeCount = 0;
    bool blinkPhase = performModulo(0x1E, pThis->m2C) > 0xE;
    bool shouldBlink = false;
    bool criticalHP = false;
    const quadColor* colorTable = &HpOrBpDisplay_table0;

    s_battleDragon* pDragon = gBattleManager->m10_battleOverlay->m18_dragon;
    u32 statusBits = pDragon->m1C0_statusModifiers;

    for (s32 i = 5; i >= 0; i--)
    {
        if (statusBits & (1 << i))
        {
            spriteEntryIndex = i + 1;
            pThis->m1_statusActive[i] = 1;
            activeCount++;
        }
        else
        {
            pThis->m1_statusActive[i] = 0;
        }
    }

    if (activeCount != 0)
    {
        if (activeCount == 1)
        {
            pThis->mD_statusCycleIndex = (u8)(spriteEntryIndex - 1);
            u32 statusTimerIndex = pThis->mD_statusCycleIndex * 2;
            shouldBlink = blinkPhase;
            s16 statusTimer = *(s16*)((u8*)pDragon + 0x1D8 + statusTimerIndex);
            if (statusTimer > 0x78)
                criticalHP = true;
        }
        else
        {
            while (pThis->m1_statusActive[pThis->mD_statusCycleIndex] == 0)
            {
                pThis->mD_statusCycleIndex++;
                if (pThis->mD_statusCycleIndex > 5)
                    pThis->mD_statusCycleIndex -= 6;
            }
            if (pThis->m1_statusActive[pThis->mD_statusCycleIndex] == 1)
            {
                pThis->mE_statusDisplayIndex = pThis->mD_statusCycleIndex;
                s32 nextIdx = pThis->mD_statusCycleIndex + 1;
                s32 framePhase = performModulo(0x1E, pThis->m2C);
                if (framePhase == 0x1D)
                {
                    pThis->mD_statusCycleIndex = (u8)nextIdx;
                    if (pThis->mD_statusCycleIndex > 5)
                        pThis->mD_statusCycleIndex -= 6;
                }
            }
            u32 statusTimerIndex = pThis->mE_statusDisplayIndex * 2;
            shouldBlink = blinkPhase;
            s16 statusTimer = *(s16*)((u8*)pDragon + 0x1D8 + statusTimerIndex);
            if (statusTimer > 0x78)
                criticalHP = true;
            spriteEntryIndex = pThis->mE_statusDisplayIndex + 1;
        }
    }

    if (shouldBlink)
        return;

    if (criticalHP && performModulo(2, pThis->m2C) != 0)
        colorTable = &HpOrBpDisplay_table2;

    sSaturnPtr spriteTablePtr = g_BTL_GenericData->getSaturnPtr(0x060b1aa4);
    sSaturnPtr spriteDescPtr = readSaturnEA(spriteTablePtr + spriteEntryIndex * 4);

    s16 cmdsrca_offset = readSaturnS16(spriteDescPtr + 2);
    s16 cmdcolr_offset = readSaturnS16(spriteDescPtr + 6);
    u16 cmdsize = readSaturnU16(spriteDescPtr + 8);
    u16 cmdpmod = readSaturnU16(spriteDescPtr + 10) | 4;

    u16 vdp1Base = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory;

    s32 outputColorIndex = (s32)(graphicEngineStatus.m14_vdp1Context[0].m10 - graphicEngineStatus.m14_vdp1Context[0].m14->begin());
    quadColor& outputColor = *(graphicEngineStatus.m14_vdp1Context[0].m10++);

    s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
    vdp1WriteEA.m0_CMDCTRL = 0x1000;
    vdp1WriteEA.m4_CMDPMOD = cmdpmod;
    vdp1WriteEA.m6_CMDCOLR = vdp1Base + cmdcolr_offset;
    vdp1WriteEA.m8_CMDSRCA = vdp1Base + cmdsrca_offset;
    vdp1WriteEA.mA_CMDSIZE = cmdsize;
    vdp1WriteEA.mC_CMDXA = pThis->m16_part1X + 0x50;
    vdp1WriteEA.mE_CMDYA = -((s16)pThis->m18_part1Y - 0x43);

    outputColor[0] = (*colorTable)[0];
    outputColor[1] = (*colorTable)[1];
    outputColor[2] = (*colorTable)[2];
    outputColor[3] = (*colorTable)[3];

    vdp1WriteEA.m1C_CMDGRA = outputColorIndex;

    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

    graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
    graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
    graphicEngineStatus.m14_vdp1Context[0].mC += 1;
}


void s_battleOverlay_20_draw(s_battleOverlay_20* pThis)
{
    if (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m8_showingBattleResultScreen)
    {
        if (battleEngine_isPlayerTurnActive())
        {
            if (gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode == 2)
                return;
            if (gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode == 0xE)
                return;
        }

        s_battleOverlay_20_drawSub0(pThis);
        battleHud_drawStatusString(pThis);

        if(gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x21] == 0)
        {
            pThis->m0 = 0;
        }
        else
        {
            pThis->m0 = 1;
        }
    }
}

// BTL_A3::06072806
void s_battleOverlay_20_delete(s_battleOverlay_20*)
{
}

void battleEngine_CreateHud1(npcFileDeleter* parent)
{
    static const s_battleOverlay_20::TypedTaskDefinition definition = {
        nullptr,
        &s_battleOverlay_20_update,
        &s_battleOverlay_20_draw,
        &s_battleOverlay_20_delete,
    };

    s_battleOverlay_20* pNewTask = createSubTask<s_battleOverlay_20>(parent, &definition);

    gBattleManager->m10_battleOverlay->m20_battleHud = pNewTask;

    pNewTask->m14_vdp1Memory = parent->m4_vd1Allocation->m4_vdp1Memory;
    pNewTask->m16_part1X = 0;
    pNewTask->m18_part1Y = 0x47;
    pNewTask->m1A_part2X = 0;
    pNewTask->m1C_part2Y = 0x47;
    pNewTask->m1E[0] = 0x31;
    pNewTask->m1E[1] = 0xb3;
    pNewTask->m22[0] = 0x66;
    pNewTask->m22[1] = 0xb3;

    createBattleHPOrBpDisplayTask(parent, &mainGameState.gameStats.m10_currentHP, &mainGameState.gameStats.mB8_maxHP, &pNewTask->m1E, 1);
    createBattleHPOrBpDisplayTask(parent, &mainGameState.gameStats.m14_currentBP, &mainGameState.gameStats.mBA_maxBP, &pNewTask->m22, 0);
    createPowerGaugeTask(parent, &pNewTask->m16_part1X, 0);

    pNewTask->m0 = 0;
    pNewTask->m28_battleCommandMenu = nullptr;
    pNewTask->m10_currentMode = 0;
}
