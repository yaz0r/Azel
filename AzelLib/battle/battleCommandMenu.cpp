#include "PDS.h"
#include "battleCommandMenu.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleDebug.h"
#include "battleHud.h"
#include "battleEngine.h"
#include "mainMenuDebugTasks.h" // TODO: clean
#include "town/town.h" // TODO: clean
#include "kernel/vdp1Allocator.h" // TODO: clean

// Todo: kernel
void BattleCommandMenu_UpdateSub0()
{
    vdp2Controls.m4_pendingVdp2Regs->m26_SFCODE = 0xC000;
    vdp2Controls.m_isDirty = 1;
}

int BattleCommandMenu_Interpolate(sBattleCommandMenu::s1C0* pData)
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

void BattleCommandMenu_Update(sBattleCommandMenu* pThis)
{
    pThis->m1C++;
    switch(pThis->m2_mode)
    {
    case 0:
        pThis->m2_mode = 1;
        break;
    case 1:
        if (BattleCommandMenu_Interpolate(&pThis->m1C0))
        {
            pThis->m2_mode = 2;
        }
        pThis->m14 = (pThis->m1C0.m0_currentValue + 0x8000) >> 0x10;
        BattleCommandMenu_UpdateSub0();
        break;
    case 2:
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m4000 = 1;

        if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.mC_newButtonDown2 & 0x10) == 0)
        {
            if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.mC_newButtonDown2 & 0x20) && ((gBattleManager->m10_battleOverlay->m4_battleEngine->m388 & 0x10) == 0))
            {
                playSoundEffect(2);
                do 
                {
                    pThis->m0++;
                    if (pThis->m0 >= pThis->m1)
                    {
                            pThis->m0 -= pThis->m1;
                    }
                } while (pThis->m4[pThis->m0]);
            }
        }
        else if ((gBattleManager->m10_battleOverlay->m4_battleEngine->m388 & 8) == 0)
        {
            playSoundEffect(2);
            pThis->m0--;
            if (pThis->m0 < 0)
            {
                pThis->m0 += pThis->m1;
            }
            while (pThis->m4[pThis->m0])
            {
                pThis->m0--;
            }
        }
        assert(0);
        break;
    default:
        assert(0);
    }

    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x19])
    {
        assert(0);
    }
}

void BattleCommandMenu_DrawButton(sBattleCommandMenu* pThis, int buttonIndex)
{
    FunctionUnimplemented();
}

void BattleCommandMenu_Draw(sBattleCommandMenu* pThis)
{
    if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m4000 && (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.mA & 1))
    {
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m4000 = 0;
    }

    if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m8 == 0)
    {
        // main command menu background
        {
            u32 vdp1WriteEA = graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
            setVdp1VramU16(vdp1WriteEA + 0x00, 0x1000); // command 0
            setVdp1VramU16(vdp1WriteEA + 0x04, 0x88); // CMDPMOD
            setVdp1VramU16(vdp1WriteEA + 0x06, dramAllocatorEnd[0].mC_buffer->m4_vd1Allocation->m4_vdp1Memory + 0x2ed0); // CMDCOLR
            setVdp1VramU16(vdp1WriteEA + 0x08, dramAllocatorEnd[0].mC_buffer->m4_vd1Allocation->m4_vdp1Memory + 0x314); // CMDSRCA
            setVdp1VramU16(vdp1WriteEA + 0x0A, 0x77F); // CMDSIZE
            setVdp1VramU16(vdp1WriteEA + 0x0C, pThis->m14 - 0x97); // CMDXA
            setVdp1VramU16(vdp1WriteEA + 0x0E, -0x64); // CMDYA

            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = vdp1WriteEA >> 3;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

            graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
            graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA = vdp1WriteEA + 0x20;
            graphicEngineStatus.m14_vdp1Context[0].mC += 1;
        }

        // top left menu graphics
        {
            u32 vdp1WriteEA = graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
            setVdp1VramU16(vdp1WriteEA + 0x00, 0x1000); // command 0
            setVdp1VramU16(vdp1WriteEA + 0x04, 0x88); // CMDPMOD
            setVdp1VramU16(vdp1WriteEA + 0x06, dramAllocatorEnd[0].mC_buffer->m4_vd1Allocation->m4_vdp1Memory + 0x2ed0); // CMDCOLR
            setVdp1VramU16(vdp1WriteEA + 0x08, dramAllocatorEnd[0].mC_buffer->m4_vd1Allocation->m4_vdp1Memory + 0x2D4); // CMDSRCA
            setVdp1VramU16(vdp1WriteEA + 0x0A, 0x420); // CMDSIZE
            setVdp1VramU16(vdp1WriteEA + 0x0C, pThis->m14 - 0xB0); // CMDXA
            setVdp1VramU16(vdp1WriteEA + 0x0E, -(0x6E - pThis->m16)); // CMDYA

            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = vdp1WriteEA >> 3;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

            graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
            graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA = vdp1WriteEA + 0x20;
            graphicEngineStatus.m14_vdp1Context[0].mC += 1;
        }

        BattleCommandMenu_DrawButton(pThis, 0);
        BattleCommandMenu_DrawButton(pThis, 1);
        BattleCommandMenu_DrawButton(pThis, 2);
        BattleCommandMenu_DrawButton(pThis, 3);
        BattleCommandMenu_DrawButton(pThis, 4);
        BattleCommandMenu_DrawButton(pThis, 5);

        switch (pThis->m2_mode)
        {
        case 1:
            break;
        case 2:
        {
            // cursor
            u32 vdp1WriteEA = graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
            setVdp1VramU16(vdp1WriteEA + 0x00, 0x1000); // command 0
            setVdp1VramU16(vdp1WriteEA + 0x04, 0x88); // CMDPMOD
            setVdp1VramU16(vdp1WriteEA + 0x06, dramAllocatorEnd[0].mC_buffer->m4_vd1Allocation->m4_vdp1Memory + 0x2ECC); // CMDCOLR
            setVdp1VramU16(vdp1WriteEA + 0x08, dramAllocatorEnd[0].mC_buffer->m4_vd1Allocation->m4_vdp1Memory + 0x218); // CMDSRCA
            setVdp1VramU16(vdp1WriteEA + 0x0A, 0x418); // CMDSIZE
            setVdp1VramU16(vdp1WriteEA + 0x0C, pThis->mC + readSaturnS16(gCurrentBattleOverlay->getSaturnPtr(0x60AC154) + pThis->m0 * 4) - 0xB0); // CMDXA
            setVdp1VramU16(vdp1WriteEA + 0x0E, -((-readSaturnS16(gCurrentBattleOverlay->getSaturnPtr(0x60AC156) + pThis->m0 * 4) - pThis->mE) + 0x70)); // CMDYA
            
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = vdp1WriteEA >> 3;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

            graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
            graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA = vdp1WriteEA + 0x20;
            graphicEngineStatus.m14_vdp1Context[0].mC += 1;
        }
        if (performModulo(0x14, pThis->m1C) < 10)
        {
            // blinking cursor
            u32 vdp1WriteEA = graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
            setVdp1VramU16(vdp1WriteEA + 0x00, 0x1000); // command 0
            setVdp1VramU16(vdp1WriteEA + 0x04, 0x88); // CMDPMOD
            setVdp1VramU16(vdp1WriteEA + 0x06, dramAllocatorEnd[0].mC_buffer->m4_vd1Allocation->m4_vdp1Memory + 0x2ECC); // CMDCOLR
            setVdp1VramU16(vdp1WriteEA + 0x08, dramAllocatorEnd[0].mC_buffer->m4_vd1Allocation->m4_vdp1Memory + 0x248); // CMDSRCA
            setVdp1VramU16(vdp1WriteEA + 0x0A, 0x418); // CMDSIZE
            setVdp1VramU16(vdp1WriteEA + 0x0C, pThis->mC + readSaturnS16(gCurrentBattleOverlay->getSaturnPtr(0x60AC154) + pThis->m0 * 4) - 0xB0); // CMDXA
            setVdp1VramU16(vdp1WriteEA + 0x0E, -((-readSaturnS16(gCurrentBattleOverlay->getSaturnPtr(0x60AC156) + pThis->m0 * 4) - pThis->mE) + 0x70)); // CMDYA

            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = vdp1WriteEA >> 3;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

            graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
            graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA = vdp1WriteEA + 0x20;
            graphicEngineStatus.m14_vdp1Context[0].mC += 1;
        }
        break;
        default:
            assert(0);
        }
    }
}

void BattleCommandMenu_Delete(sBattleCommandMenu* pThis)
{
    FunctionUnimplemented();
}

void createBattleCommandMenuSub0Sub0(sBattleCommandMenu::s1C0* pThis)
{
    pThis->m8 = (pThis->mC - pThis->m4) / 2;
    pThis->m10_currentStepValue = 0;
    pThis->m14_stepIncrement = FP_Div(0x8000000, ((int)pThis->m18) << 16);
    pThis->m0_currentValue = pThis->m4;
}

void createBattleCommandMenuSub0(sBattleCommandMenu* pThis, char param2)
{
    switch (param2)
    {
    case 0:
        pThis->m1C0.m4 = -0x5A0000;
        pThis->m1C0.mC = 0;
        pThis->m1C0.m18 = 4;
        break;
    case 1:
        pThis->m1C0.m4 = 0;
        pThis->m1C0.mC = -0x5A0000;
        pThis->m1C0.m18 = 8;
        break;
    default:
        assert(0);
        break;
    }

    createBattleCommandMenuSub0Sub0(&pThis->m1C0);

    pThis->m14 = (pThis->m1C0.m0_currentValue + 0x8000) >> 0x10;
}

void createBattleCommandMenu(p_workArea parent)
{
    static const sBattleCommandMenu::TypedTaskDefinition definition = {
        nullptr,
        &BattleCommandMenu_Update,
        &BattleCommandMenu_Draw,
        &BattleCommandMenu_Delete,
    };

    sBattleCommandMenu* pThis = createSubTask<sBattleCommandMenu>(parent, &definition);
    gBattleManager->m10_battleOverlay->m20_battleHud->m28_battleCommandMenu = pThis;
    pThis->mC = 0x19;
    pThis->mE = 0xC;
    playSoundEffect(3);

    pThis->m0 = mainGameState.readPackedBits(0x1043, 4);

    if ((mainGameState.gameStats.m1_dragonLevel == 6) && (pThis->m0 == 5))
    {
        pThis->m0 = 0;
    }
    else if ((mainGameState.gameStats.m1_dragonLevel == 6) && ((pThis->m0 == 5) || (pThis->m0 == 3)))
    {
        pThis->m0 = 0;
    }

    createBattleCommandMenuSub0(pThis, 0);

    pThis->m1DC.zeroize();
    pThis->m4.fill(0);
    pThis->m1 = 6;
}
