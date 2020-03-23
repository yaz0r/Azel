#include "PDS.h"
#include "battleCommandMenu.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleDebug.h"
#include "battleHud.h"
#include "battleEngine.h"
#include "battleOverlay_C.h"
#include "battleDragon.h"
#include "mainMenuDebugTasks.h" // TODO: clean
#include "town/town.h" // TODO: clean
#include "kernel/vdp1Allocator.h" // TODO: clean

void fieldPaletteTaskInitSub0Sub0(); // TODO: clean

// Todo: kernel
void BattleCommandMenu_UpdateSub0()
{
    vdp2Controls.m4_pendingVdp2Regs->m26_SFCODE = 0xC000;
    vdp2Controls.m_isDirty = 1;
}

void createBattleCommandMenuSub0(sBattleCommandMenu* pThis, char param2)
{
    switch (param2)
    {
    case 0:
        pThis->m1C0_scrollInterpolator.m4_startValue = -0x5A0000;
        pThis->m1C0_scrollInterpolator.mC_targetValue = 0;
        pThis->m1C0_scrollInterpolator.m18_interpolationLength = 4;
        break;
    case 1:
        pThis->m1C0_scrollInterpolator.m4_startValue = 0;
        pThis->m1C0_scrollInterpolator.mC_targetValue = -0x5A0000;
        pThis->m1C0_scrollInterpolator.m18_interpolationLength = 8;
        break;
    default:
        assert(0);
        break;
    }

    FPInterpolator_Init(&pThis->m1C0_scrollInterpolator);

    pThis->m14 = (pThis->m1C0_scrollInterpolator.m0_currentValue + 0x8000) >> 0x10;
}


void BattleCommandMenu_DisplayCommandString(sBattleCommandMenu* pThis)
{
    FunctionUnimplemented();
}

void createBattleCommandMenuSub1(sBattleCommandMenu* pThis, int param2)
{
    if (param2 == 0)
    {
        playSoundEffect(5);
        BattleCommandMenu_DisplayCommandString(pThis);
    }
}

s32 createBattleCommandMenuSub2(s32 param1)
{
    int iVar1;

    if (((gBattleManager->m10_battleOverlay->m18_dragon->m1C0) & 4) == 0) {
        if (((gBattleManager->m10_battleOverlay->m18_dragon->m1C0) & 0x200) == 0) {
            iVar1 = 0x10000;
        }
        else {
            iVar1 = 0xcccc;
        }
        iVar1 = MTH_Mul(iVar1, param1);
    }
    else {
        iVar1 = MTH_Mul(0x20000, param1);
    }
    return iVar1;
}

s32 BattleCommandMenu_CountSubMenuEntries(sBattleCommandMenu* pThis)
{
    FunctionUnimplemented();
    return 1;
}

int isBattleCommandEnabled(sBattleCommandMenu* pThis, int buttonIndex)
{
    FunctionUnimplemented();
    return 1;
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
        if (FPInterpolator_Step(&pThis->m1C0_scrollInterpolator))
        {
            pThis->m2_mode = 2;
        }
        pThis->m14 = (pThis->m1C0_scrollInterpolator.m0_currentValue + 0x8000) >> 0x10;
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
                    pThis->m0_selectedBattleCommand++;
                    if (pThis->m0_selectedBattleCommand >= pThis->m1_numBattleCommands)
                    {
                            pThis->m0_selectedBattleCommand -= pThis->m1_numBattleCommands;
                    }
                } while (pThis->m4_enabledBattleCommands[pThis->m0_selectedBattleCommand]);
            }
        }
        else if ((gBattleManager->m10_battleOverlay->m4_battleEngine->m388 & 8) == 0)
        {
            playSoundEffect(2);
            pThis->m0_selectedBattleCommand--;
            if (pThis->m0_selectedBattleCommand < 0)
            {
                pThis->m0_selectedBattleCommand += pThis->m1_numBattleCommands;
            }
            while (pThis->m4_enabledBattleCommands[pThis->m0_selectedBattleCommand])
            {
                pThis->m0_selectedBattleCommand--;
            }
        }
        BattleCommandMenu_DisplayCommandString(pThis);
        if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 6) == 0)
        {
            if (((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 1) != 0) &&
                ((gBattleManager->m10_battleOverlay->m4_battleEngine->m388 & 0x100U) == 0))
            {
                // cancel out of menu
                fieldPaletteTaskInitSub0Sub2();
                setupVDP2StringRendering(6, 0x17, 0x20, 2);
                pThis->m20 &= ~1;
                pThis->m20 |= 0x10;
                clearVdp2TextArea();
                pThis->m2_mode = 7;
                createBattleCommandMenuSub0(pThis, 1);
                playSoundEffect(4);
            }
        }
        else if ((gBattleManager->m10_battleOverlay->m4_battleEngine->m388 & 0x80U) == 0)
        {
            //select current entry
            fieldPaletteTaskInitSub0Sub2();
            setupVDP2StringRendering(6, 0x17, 0x20, 2);
            pThis->m20 &= ~1;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m4000 = 0;
            createBattleCommandMenuSub0(pThis, 1);
            int uVar9;
            switch (pThis->m0_selectedBattleCommand)
            {
            case 0: // gun attack
                uVar9 = 0;
                if ((0 < gBattleManager->m10_battleOverlay->m4_battleEngine->m498_numEnemies)
                    && (0 < gBattleManager->m10_battleOverlay->m4_battleEngine->m3B4.m16_combo)
                    && (0 < gBattleManager->m10_battleOverlay->mC_targetSystem->m20A_numSelectableEnemies))
                {
                    pThis->m20 |= 0x12;
                    clearVdp2TextArea();
                    uVar9 = 1;
                    pThis->m2_mode = 7;
                    pThis->m3 = 0;
                }
                break;
            case 1: // homing laser
                uVar9 = 0;
                if (((0 < gBattleManager->m10_battleOverlay->m4_battleEngine->m498_numEnemies) &&
                    (0 < gBattleManager->m10_battleOverlay->m4_battleEngine->m3B4.m16_combo)) &&
                    ((((gBattleManager->m10_battleOverlay->m18_dragon->m1C0) & 1) == 0 &&
                    (0 < (gBattleManager->m10_battleOverlay->mC_targetSystem->m20A_numSelectableEnemies))))) {
                    pThis->m20 |= 0x14;
                    clearVdp2TextArea();
                    uVar9 = 1;
                    pThis->m2_mode = 7;
                    pThis->m3 = 0;
                }
                break;
            case 2: // items
                uVar9 = 0;
                if ('\0' < (char)gBattleManager->m10_battleOverlay->m4_battleEngine->m3B4.m16_combo) {
                    pThis->m24 = BattleCommandMenu_CountSubMenuEntries(pThis);
                    int iVar8;
                    if ((0 < pThis->m24) && (pThis->m1DC[0] == 0)) {
                        pThis->m20 |= 8;
                        pThis->m2_mode = 3;
                        clearVdp2TextArea();
                        pThis->m3 = 0;
                        uVar9 = 1;
                    }
                }
                break;
            case 3: // berserk
                uVar9 = 0;
                if (('\0' < (char)gBattleManager->m10_battleOverlay->m4_battleEngine->m3B4.m16_combo) &&
                    (((gBattleManager->m10_battleOverlay->m18_dragon->m1C0) & 1) == 0)) {
                    pThis->m24 = BattleCommandMenu_CountSubMenuEntries(pThis);
                    int iVar8;
                    if ((0 < pThis->m24) &&
                        ((isBattleCommandEnabled(pThis, pThis->m0_selectedBattleCommand) > 0 &&
                        (pThis->m1DC[1] == 0)))) {
                        pThis->m20 |= 8;
                        pThis->m2_mode = 4;
                        clearVdp2TextArea();
                        pThis->m3 = 0;
                        uVar9 = 1;
                    }
                }
                break;
            case 4: // change weapon?
                uVar9 = 0;
                if ('\0' < (char)gBattleManager->m10_battleOverlay->m4_battleEngine->m3B4.m16_combo) {
                    pThis->m24 = BattleCommandMenu_CountSubMenuEntries(pThis);
                    int iVar8;
                    if ((0 < pThis->m24) && (pThis->m1DC[2] == 0)) {
                        pThis->m20 |= 8;
                        pThis->m2_mode = 5;
                        clearVdp2TextArea();
                        pThis->m3 = 0;
                        uVar9 = 1;
                    }
                }
                break;
            case 5: // change form?
                uVar9 = 0;
                if (('\0' < (char)gBattleManager->m10_battleOverlay->m4_battleEngine->m3B4.m16_combo) &&
                    (((gBattleManager->m10_battleOverlay->m18_dragon->m1C0) & 1) == 0)) {
                    int bVar2 = 0;
                    if (gBattleManager->mC == 0)
                    {
                        bVar2 = 1;
                    }
                    else if ((gDragonState->mC_dragonType != 0) && (gDragonState->mC_dragonType != 8))
                    {
                        bVar2 = 1;
                    }

                    if (bVar2)
                    {
                        fieldPaletteTaskInitSub0Sub2();
                        setupVDP2StringRendering(6, 0x17, 0x20, 2);
                        clearVdp2TextArea();
                        pThis->m2_mode = 6;
                        pThis->m3 = 0;
                        playSoundEffect(4);
                        uVar9 = 1;
                        pThis->m18_oldDragonAtk = mainGameState.gameStats.dragonAtt;
                        pThis->m1A_oldDragonDef = mainGameState.gameStats.dragonDef;
                        graphicEngineStatus.m40AC.m0_menuId = 3;
                    }
                }
                break;
            default:
                assert(0);
                break;
            }

            createBattleCommandMenuSub1(pThis, uVar9);
        }
        break;
    case 3: // item selection
        assert(0);
        break;
    case 4: // berserk selection
        assert(0);
        break;
    case 5: // weapon change
        assert(0);
        break;
    case 6: // form change
        assert(0);
        break;
    case 7: // used for gun attack and homing laser
        if (FPInterpolator_Step(&pThis->m1C0_scrollInterpolator))
        {
            gBattleManager->m10_battleOverlay->m4_battleEngine->m3B4.m0_max = createBattleCommandMenuSub2(0x3C) << 0x10;
            battleEngine_SetBattleMode16();
            if ((pThis->m20 & 2) == 0)
            {
                if (pThis->m20 & 4)
                {
                    battleEngine_SetBattleMode(eBattleModes::m3_shootEnemeyWithHomingLaser);
                    gBattleManager->m10_battleOverlay->m4_battleEngine->m184 = 0;
                    gBattleManager->m10_battleOverlay->m4_battleEngine->m390 = 0;
                }
            }
            else
            {
                battleEngine_SetBattleMode(eBattleModes::m0_shootEnemyWithGun);
                gBattleManager->m10_battleOverlay->m4_battleEngine->m3A7 = 0;
                gBattleManager->m10_battleOverlay->m4_battleEngine->m184 = 0;
            }
            fieldPaletteTaskInitSub0Sub0();
            pThis->getTask()->markFinished();
        }
        pThis->m14 = (pThis->m1C0_scrollInterpolator.m0_currentValue + 0x8000) >> 16;
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
    int state = 0;
    int buttonWidth = pThis->mC + pThis->m14 + readSaturnS16(gCurrentBattleOverlay->getSaturnPtr(0x60ac154) + buttonIndex * 4) - 0xB0;
    int buttonHeight = (-readSaturnS16(gCurrentBattleOverlay->getSaturnPtr(0x60ac154) + buttonIndex * 4 + 2) - pThis->mE) + 0x70;

    switch (buttonIndex)
    {
    case 0:
    case 1:
        if (gBattleManager->m10_battleOverlay->m4_battleEngine->m3B4.m16_combo > 0)
        {
            state = 1;
        }
        break;
    case 2:
        if (isBattleCommandEnabled(pThis, 2) < 1)
        {
            state = 2;
        }
        else if (gBattleManager->m10_battleOverlay->m4_battleEngine->m3B4.m16_combo > 0)
        {
            state = 1;
        }
        break;
    case 3:
        if (mainGameState.gameStats.m1_dragonLevel == 8)
        {
            state = 2;
            pThis->m4_enabledBattleCommands[3] = 1;
        }
        else if (isBattleCommandEnabled(pThis, 3) < 1)
        {
            state = 0;
        }
        else if (gBattleManager->m10_battleOverlay->m4_battleEngine->m3B4.m16_combo > 0)
        {
            state = 1;
        }
        break;
    case 4:
        if (isBattleCommandEnabled(pThis, 4) < 1)
        {
            state = 2;
            pThis->m4_enabledBattleCommands[4] = 1;
        }
        else if (gBattleManager->m10_battleOverlay->m4_battleEngine->m3B4.m16_combo > 0)
        {
            state = 1;
        }
        break;
    case 5:
        if ((mainGameState.gameStats.m1_dragonLevel == 0) || (mainGameState.gameStats.m1_dragonLevel == 6) || (mainGameState.gameStats.m1_dragonLevel == 8))
        {
            state = 2;
            pThis->m4_enabledBattleCommands[5] = 1;
        }
        else if(gBattleManager->m10_battleOverlay->m4_battleEngine->m3B4.m16_combo > 0)
        {
            state = 1;
        }
        break;
    }

    sSaturnPtr pSpriteData;

    switch (state)
    {
    case 0:
        pSpriteData = readSaturnEA(gCurrentBattleOverlay->getSaturnPtr(0x60b1bec) + 4 * buttonIndex);
    break;
    case 1:
        if(pThis->m0_selectedBattleCommand == buttonIndex)
        {
            pSpriteData = readSaturnEA(gCurrentBattleOverlay->getSaturnPtr(0x60b1bbc) + 4 * buttonIndex);
        }
        else
        {
            pSpriteData = readSaturnEA(gCurrentBattleOverlay->getSaturnPtr(0x60b1bd4) + 4 * buttonIndex);
        }
        break;
    case 2:
        pSpriteData = readSaturnEA(gCurrentBattleOverlay->getSaturnPtr(0x60b1c04) + 4 * buttonIndex);
        break;
    default:
        assert(0);
        break;
    }

    u32 vdp1WriteEA = graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
    setVdp1VramU16(vdp1WriteEA + 0x00, 0x1000); // command 0
    setVdp1VramU16(vdp1WriteEA + 0x04, readSaturnS16(pSpriteData + 0xA)); // CMDPMOD
    setVdp1VramU16(vdp1WriteEA + 0x06, dramAllocatorEnd[0].mC_buffer->m4_vd1Allocation->m4_vdp1Memory + readSaturnS16(pSpriteData + 0x6)); // CMDCOLR
    setVdp1VramU16(vdp1WriteEA + 0x08, dramAllocatorEnd[0].mC_buffer->m4_vd1Allocation->m4_vdp1Memory + readSaturnS16(pSpriteData + 0x2)); // CMDSRCA
    setVdp1VramU16(vdp1WriteEA + 0x0A, readSaturnS16(pSpriteData + 0x8)); // CMDSIZE
    setVdp1VramU16(vdp1WriteEA + 0x0C, buttonWidth); // CMDXA
    setVdp1VramU16(vdp1WriteEA + 0x0E, -buttonHeight); // CMDYA

    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = vdp1WriteEA >> 3;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

    graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
    graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA = vdp1WriteEA + 0x20;
    graphicEngineStatus.m14_vdp1Context[0].mC += 1;
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
            setVdp1VramU16(vdp1WriteEA + 0x0C, pThis->mC + readSaturnS16(gCurrentBattleOverlay->getSaturnPtr(0x60AC154) + pThis->m0_selectedBattleCommand * 4) - 0xB0); // CMDXA
            setVdp1VramU16(vdp1WriteEA + 0x0E, -((-readSaturnS16(gCurrentBattleOverlay->getSaturnPtr(0x60AC156) + pThis->m0_selectedBattleCommand * 4) - pThis->mE) + 0x70)); // CMDYA
            
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
            setVdp1VramU16(vdp1WriteEA + 0x0C, pThis->mC + readSaturnS16(gCurrentBattleOverlay->getSaturnPtr(0x60AC154) + pThis->m0_selectedBattleCommand * 4) - 0xB0); // CMDXA
            setVdp1VramU16(vdp1WriteEA + 0x0E, -((-readSaturnS16(gCurrentBattleOverlay->getSaturnPtr(0x60AC156) + pThis->m0_selectedBattleCommand * 4) - pThis->mE) + 0x70)); // CMDYA

            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = vdp1WriteEA >> 3;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

            graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
            graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA = vdp1WriteEA + 0x20;
            graphicEngineStatus.m14_vdp1Context[0].mC += 1;
        }
        break;
        case 3:
            assert(0);
            break;
        default:
            break;
        }
    }
}

void BattleCommandMenu_Delete(sBattleCommandMenu* pThis)
{
    FunctionUnimplemented();
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

    pThis->m0_selectedBattleCommand = mainGameState.readPackedBits(0x1043, 4);

    if ((mainGameState.gameStats.m1_dragonLevel == 6) && (pThis->m0_selectedBattleCommand == 5))
    {
        pThis->m0_selectedBattleCommand = 0;
    }
    else if ((mainGameState.gameStats.m1_dragonLevel == 6) && ((pThis->m0_selectedBattleCommand == 5) || (pThis->m0_selectedBattleCommand == 3)))
    {
        pThis->m0_selectedBattleCommand = 0;
    }

    createBattleCommandMenuSub0(pThis, 0);

    pThis->m1DC.zeroize();
    pThis->m4_enabledBattleCommands.fill(0);
    pThis->m1_numBattleCommands = 6;
}
