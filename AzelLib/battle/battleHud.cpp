#include "PDS.h"
#include "battleHud.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "kernel/vdp1Allocator.h"
#include "town/town.h" // todo: for npcFileDeleter
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"
#include "battleDebug.h"
#include "battlePowerGauge.h"
#include "battleCommandMenu.h"

void s_battleOverlay_20_updateSub0(s_battleOverlay_20_sub* pData)
{
    pData->m8 = (pData->mC - pData->m4) / 2;
    pData->m10_currentStepValue = 0;
    pData->m14_stepIncrement = FP_Div(0x8000000, pData->m18 << 0x10);
    pData->m0_currentValue = pData->m4;
}

s32 s_battleOverlay_20_updateSub1(s_battleOverlay_20_sub* pData)
{
    if (pData->m10_currentStepValue > 0x7ffffff)
    {
        pData->m10_currentStepValue = 0x8000000;
        pData->m14_stepIncrement = 0;
        pData->m0_currentValue = pData->mC;
        return true;
    }
    else
    {
        pData->m0_currentValue = (pData->m8 + pData->m4) - MTH_Mul(getCos(pData->m10_currentStepValue.getInteger() & 0xFFF), pData->m8);
        pData->m10_currentStepValue += pData->m14_stepIncrement;
        return false;
    }
}

void s_battleOverlay_20_update(s_battleOverlay_20* pThis)
{
    pThis->m2C++;
    switch (pThis->m10_currentMode)
    {
    case 0: //hidden
        if (BattleEngineSub0_UpdateSub0())
        {
            if (gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode == 2)
                return;
            if (gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode == 0xE)
                return;
        }
        if (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m80000_hideBattleHUD)
        {
            pThis->m30.m4 = 0x470000;
            pThis->m30.mC = 0;
            pThis->m30.m18 = 4;
            s_battleOverlay_20_updateSub0(&pThis->m30);

            pThis->m4C.m4 = 0x470000;
            pThis->m4C.mC = 0x470000;
            pThis->m4C.m18 = 4;
            s_battleOverlay_20_updateSub0(&pThis->m4C);
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
                pThis->m30.m4 = 0;
                pThis->m30.mC = -0x180000;
                pThis->m30.m18 = 4;
                s_battleOverlay_20_updateSub0(&pThis->m30);

                pThis->m4C.m4 = 0x470000;
                pThis->m4C.mC = 0x180000;
                pThis->m4C.m18 = 4;
                s_battleOverlay_20_updateSub0(&pThis->m4C);
                pThis->m10_currentMode = 3;
                pThis->m12_nextMode = 2;
            }
        }
        else
        {
            // start hiding
            pThis->m30.m4 = 0;
            pThis->m30.mC = 0x470000;
            pThis->m30.m18 = 4;
            s_battleOverlay_20_updateSub0(&pThis->m30);

            pThis->m4C.m4 = 0x470000;
            pThis->m4C.mC = 0x470000;
            pThis->m4C.m18 = 4;
            s_battleOverlay_20_updateSub0(&pThis->m4C);
            pThis->m10_currentMode = 3;
            pThis->m12_nextMode = 0;
        }
        break;
    case 2: // command menu open
        if ((gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode == 0xC) && ((gBattleManager->m10_battleOverlay->m20_battleHud->m28_battleCommandMenu->m20 & 0x10) != 0))
        {
            pThis->m30.m4 = -0x180000;
            pThis->m30.mC = 0;
            pThis->m30.m18 = 4;
            s_battleOverlay_20_updateSub0(&pThis->m30);

            pThis->m4C.m4 = -0x180000;
            pThis->m4C.mC = 0x470000;
            pThis->m4C.m18 = 4;
            s_battleOverlay_20_updateSub0(&pThis->m4C);
            pThis->m10_currentMode = 3;
            pThis->m12_nextMode = 1;
        }
        else if (gBattleManager->m10_battleOverlay->m20_battleHud->m28_battleCommandMenu == nullptr)
        {
            pThis->m30.m4 = -0x180000;
            pThis->m30.mC = 0x470000;
            pThis->m30.m18 = 4;
            s_battleOverlay_20_updateSub0(&pThis->m30);

            pThis->m4C.m4 = -0x180000;
            pThis->m4C.mC = 0x470000;
            pThis->m4C.m18 = 4;
            s_battleOverlay_20_updateSub0(&pThis->m4C);
            pThis->m10_currentMode = 3;
            pThis->m12_nextMode = 0;
        }
        break;
    case 3: // scrolling
        if (s_battleOverlay_20_updateSub1(&pThis->m30))
        {
            pThis->m10_currentMode = pThis->m12_nextMode;
        }
        s_battleOverlay_20_updateSub1(&pThis->m4C);
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
        u32 vdp1WriteEA = graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        setVdp1VramU16(vdp1WriteEA + 0x00, 0x1001); // command 0
        setVdp1VramU16(vdp1WriteEA + 0x04, 0x88); // CMDPMOD
        setVdp1VramU16(vdp1WriteEA + 0x06, pThis->m14_vdp1Memory + 0x2ebc); // CMDCOLR
        setVdp1VramU16(vdp1WriteEA + 0x08, pThis->m14_vdp1Memory + 0x278); // CMDSRCA
        setVdp1VramU16(vdp1WriteEA + 0x0A, 0x108); // CMDSIZE
        setVdp1VramU16(vdp1WriteEA + 0x0C, pThis->m1A_part2X - 0x82); // CMDXA
        setVdp1VramU16(vdp1WriteEA + 0x0E, -(-0x5C - pThis->m1C_part2Y)); // CMDYA
        setVdp1VramU16(vdp1WriteEA + 0x14, pThis->m1A_part2X + 0x87); // CMDXC
        setVdp1VramU16(vdp1WriteEA + 0x16, -(-0x73 - pThis->m1C_part2Y)); // CMDYC

        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = vdp1WriteEA >> 3;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

        graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
        graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA = vdp1WriteEA + 0x20;
        graphicEngineStatus.m14_vdp1Context[0].mC += 1;
    }

    sSaturnPtr spriteDataTable = gCurrentBattleOverlay->getSaturnPtr(0x60b1a40);
    sSaturnPtr currentSprite = gCurrentBattleOverlay->getSaturnPtr(0x60b19b8);
    for(int i=0; i<4; i++)
    {
        sSaturnPtr spriteData = readSaturnEA(spriteDataTable + 4 * i);

        u32 vdp1WriteEA = graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        setVdp1VramU16(vdp1WriteEA + 0x00, 0x1000); // command 0
        setVdp1VramU16(vdp1WriteEA + 0x04, readSaturnS16(spriteData + 10)); // CMDPMOD
        setVdp1VramU16(vdp1WriteEA + 0x06, readSaturnS16(spriteData + 6) + dramAllocatorEnd[0].mC_buffer->m4_vd1Allocation->m4_vdp1Memory); // CMDCOLR
        setVdp1VramU16(vdp1WriteEA + 0x08, readSaturnS16(spriteData + 2) + dramAllocatorEnd[0].mC_buffer->m4_vd1Allocation->m4_vdp1Memory); // CMDSRCA
        setVdp1VramU16(vdp1WriteEA + 0x0A, readSaturnS16(spriteData + 8)); // CMDSIZE
        setVdp1VramU16(vdp1WriteEA + 0x0C, readSaturnS16(currentSprite) + pThis->m1A_part2X - 0xb0); // CMDXA
        setVdp1VramU16(vdp1WriteEA + 0x0E, -(-pThis->m1C_part2Y - readSaturnS16(currentSprite + 2)) + 0x70); // CMDYA

        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = vdp1WriteEA >> 3;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

        graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
        graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA = vdp1WriteEA + 0x20;
        graphicEngineStatus.m14_vdp1Context[0].mC += 1;

        currentSprite += 2;
    }

    {
        u32 vdp1WriteEA = graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        setVdp1VramU16(vdp1WriteEA + 0x00, 0x1000); // command 0
        setVdp1VramU16(vdp1WriteEA + 0x04, 0x88); // CMDPMOD
        setVdp1VramU16(vdp1WriteEA + 0x06, 0x2ED0 + dramAllocatorEnd[0].mC_buffer->m4_vd1Allocation->m4_vdp1Memory); // CMDCOLR
        setVdp1VramU16(vdp1WriteEA + 0x08, 0x554 + dramAllocatorEnd[0].mC_buffer->m4_vd1Allocation->m4_vdp1Memory); // CMDSRCA
        setVdp1VramU16(vdp1WriteEA + 0x0A, 0x252e); // CMDSIZE
        setVdp1VramU16(vdp1WriteEA + 0x0C, pThis->m16_part1X - 0x8E); // CMDXA
        setVdp1VramU16(vdp1WriteEA + 0x0E, -(-0x33 - pThis->m18_part1Y)); // CMDYA

        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = vdp1WriteEA >> 3;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

        graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
        graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA = vdp1WriteEA + 0x20;
        graphicEngineStatus.m14_vdp1Context[0].mC += 1;
    }

    {
        u32 vdp1WriteEA = graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        setVdp1VramU16(vdp1WriteEA + 0x00, 0x1000); // command 0
        setVdp1VramU16(vdp1WriteEA + 0x04, 0x88); // CMDPMOD
        setVdp1VramU16(vdp1WriteEA + 0x06, 0x2ED0 + dramAllocatorEnd[0].mC_buffer->m4_vd1Allocation->m4_vdp1Memory); // CMDCOLR
        setVdp1VramU16(vdp1WriteEA + 0x08, 0x4d4 + dramAllocatorEnd[0].mC_buffer->m4_vd1Allocation->m4_vdp1Memory); // CMDSRCA
        setVdp1VramU16(vdp1WriteEA + 0x0A, 0x553); // CMDSIZE
        setVdp1VramU16(vdp1WriteEA + 0x0C, pThis->m16_part1X - 0xB0); // CMDXA
        setVdp1VramU16(vdp1WriteEA + 0x0E, -(-0x31 - pThis->m18_part1Y)); // CMDYA

        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = vdp1WriteEA >> 3;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

        graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
        graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA = vdp1WriteEA + 0x20;
        graphicEngineStatus.m14_vdp1Context[0].mC += 1;
    }

}

void battleHud_drawStatusString(s_battleOverlay_20* pThis)
{
    FunctionUnimplemented();
    /*
    {
        int outputColorIndex = graphicEngineStatus.m14_vdp1Context[0].m10 - graphicEngineStatus.m14_vdp1Context[0].m14->begin();
        sPerQuadDynamicColor& outputColor = *(graphicEngineStatus.m14_vdp1Context[0].m10++);

        u32 vdp1WriteEA = graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        setVdp1VramU16(vdp1WriteEA + 0x00, 0x1000); // command 0
        setVdp1VramU16(vdp1WriteEA + 0x04, 0x88); // CMDPMOD
        setVdp1VramU16(vdp1WriteEA + 0x06, dramAllocatorEnd[0].mC_buffer->m4_vd1Allocation->m4_vdp1Memory + 0x2EB8); // CMDCOLR
        setVdp1VramU16(vdp1WriteEA + 0x08, dramAllocatorEnd[0].mC_buffer->m4_vd1Allocation->m4_vdp1Memory + 0xAE4); // CMDSRCA
        setVdp1VramU16(vdp1WriteEA + 0x0A, 0x40A); // CMDSIZE
        setVdp1VramU16(vdp1WriteEA + 0x0C, pThis->m16_part1X + 0x50); // CMDXA
        setVdp1VramU16(vdp1WriteEA + 0x0E, -(pThis->m18_part1Y - 0x43)); // CMDYA

        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = vdp1WriteEA >> 3;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

        graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
        graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA = vdp1WriteEA + 0x20;
        graphicEngineStatus.m14_vdp1Context[0].mC += 1;
    }
    */
}


void s_battleOverlay_20_draw(s_battleOverlay_20* pThis)
{
    if (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m8)
    {
        if (BattleEngineSub0_UpdateSub0())
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

void s_battleOverlay_20_delete(s_battleOverlay_20*)
{
    FunctionUnimplemented();
}

void createBattleHPOrBpDisplayTask(npcFileDeleter* parent, u16*, u16*, u16*, s32)
{
    FunctionUnimplemented();
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
    pNewTask->m1E = 0x31;
    pNewTask->m20 = 0xb3;
    pNewTask->m22 = 0x66;
    pNewTask->m24 = 0xb3;

    createBattleHPOrBpDisplayTask(parent, &mainGameState.gameStats.m10_currentHP, &mainGameState.gameStats.maxHP, &pNewTask->m1E, 1);
    createBattleHPOrBpDisplayTask(parent, &mainGameState.gameStats.currentBP, &mainGameState.gameStats.maxBP, &pNewTask->m22, 0);
    createPowerGaugeTask(parent, &pNewTask->m16_part1X, 0);

    pNewTask->m0 = 0;
    pNewTask->m28_battleCommandMenu = nullptr;
    pNewTask->m10_currentMode = 0;
}
