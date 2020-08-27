#include "PDS.h"
#include "battleCommandMenu.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleDebug.h"
#include "battleHud.h"
#include "battleEngine.h"
#include "battleOverlay_C.h"
#include "battleDragon.h"
#include "battleItemSelection.h"
#include "items.h"
#include "mainMenuDebugTasks.h" // TODO: clean
#include "kernel/graphicalObject.h"
#include "kernel/vdp1Allocator.h" // TODO: clean
#include "audio/systemSounds.h"
#include "battle/battleGenericData.h"

void fieldPaletteTaskInitSub0Sub0(); // TODO: clean

// Todo: kernel
void BattleCommandMenu_UpdateSub0()
{
    vdp2Controls.m4_pendingVdp2Regs->m26_SFCODE = 0xC000;
    vdp2Controls.m_isDirty = 1;
}

void setBattleFont(int param_1)
{
    FunctionUnimplemented();
/*    if (param_1 == 0)
    {
        setActiveFont(gBattleManager->m10_battleOverlay->m14->m18);
    }
    else
    {
        setActiveFont(gBattleManager->m10_battleOverlay->m14->m17);
    }
    */
}

void drawUsedItemName(s32, s32)
{
    FunctionUnimplemented();
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
        playSystemSoundEffect(5);
        BattleCommandMenu_DisplayCommandString(pThis);
    }
}

s32 createBattleCommandMenuSub2(s32 param1)
{
    int iVar1;

    if (((gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers) & 4) == 0) {
        if (((gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers) & 0x200) == 0) {
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

bool isFlashChipDisabledInBattle()
{
    FunctionUnimplemented();

    return true;
}

void addObjectToList(sBattleCommandMenu::sSubMenuEntry* pEntry, eItems index)
{
    bool itemDisabled = false;
    switch (index)
    {
    case eItems::m1_blastChip:
    case eItems::m2_dualBlastChip:
    case eItems::m3_triBlastChip:
        if (gBattleManager->m10_battleOverlay->mC_targetSystem->m20A_numSelectableEnemies < 1)
        {
            itemDisabled = true;
        }
        break;
    case eItems::m4_flashChip:
        if (isFlashChipDisabledInBattle())
        {
            itemDisabled = true;
        }
        break;
    case eItems::m5_elixirMinor:
        if (mainGameState.gameStats.m10_currentHP == mainGameState.gameStats.mB8_maxHP)
        {
            itemDisabled = true;
        }
        break;
    case eItems::m6_berserkMicro:
        if (mainGameState.gameStats.m14_currentBP == mainGameState.gameStats.mBA_maxBP)
        {
            itemDisabled = true;
        }
    break;
    default:
        assert(0);
    }

    pEntry->m0_itemIndex = index;
    if (!itemDisabled)
    {
        pEntry->m2 = 0;
    }
}

void addBerserkToActiveList(sBattleCommandMenu::sSubMenuEntry* pEntry, eItems index)
{
    FunctionUnimplemented();

    bool itemDisabled = false;
    pEntry->m0_itemIndex = index;
    if (!itemDisabled)
    {
        pEntry->m2 = 0;
    }
}

s32 BattleCommandMenu_PopulateSubMenu(sBattleCommandMenu* pThis)
{
    int totalEntryCount = 0;
    switch (pThis->m0_selectedBattleCommand)
    {
    case 2: // items
        for (int i=0; i<0x38; i++)
        {
            pThis->m34_itemList[i].m0_itemIndex = eItems::m0_dummy;
            pThis->m34_itemList[i].m2 = 1;
        }
        for (int i = 0; i < 0x38; i++)
        {
            eItems itemEntry = (eItems)i;

            const sObjectListEntry* pObject = getObjectListEntry(itemEntry);
            if (pObject->m0_flags & 2)
            {
                if ((pObject->m1_type == 6) || (pObject->m1_type == 7) || (pObject->m1_type == 8))
                {
                    if (mainGameState.getItemCount(itemEntry))
                    {
                        addObjectToList(&pThis->m34_itemList[totalEntryCount++], itemEntry);
                    }
                }
            }
        }
        break;
    case 3: // berserks
        for (int i=0; i<0x22; i++)
        {
            pThis->m114_berserkList[i].m0_itemIndex = eItems::m0_dummy;
            pThis->m114_berserkList[i].m2 = 0;
        }
        // for each class types
        for (int classType = 0; classType < 6; classType++)
        {
            sSaturnPtr classBerserksTable = readSaturnEA(g_BTL_GenericData->getSaturnPtr(0x60ac094) + classType * 4);

            for (int classBerserkIndex = 0; classBerserkIndex < 5; classBerserkIndex++)
            {
                eItems berserkId = (eItems)readSaturnS16(classBerserksTable + classBerserkIndex * 8 + 6);
                if (berserkId)
                {
                    if (mainGameState.getItemCount(berserkId) == 0)
                    {
                        pThis->m114_berserkList[classType * 5 + classBerserkIndex].m0_itemIndex = eItems::mA6_unlearned;
                        pThis->m114_berserkList[classType * 5 + classBerserkIndex].m2 = 1;
                    }
                    else
                    {
                        addBerserkToActiveList(&pThis->m114_berserkList[classType * 5 + classBerserkIndex], berserkId);
                    }
                }
                totalEntryCount++;
            }
        }
        break;
    default:
        assert(0);
    }

    return totalEntryCount;
}

int isBattleCommandEnabled(sBattleCommandMenu* pThis, int buttonIndex)
{
    FunctionUnimplemented();
    return 1;
}

void drawBattleItemMenuSelectedItem(sBattleCommandMenu* pThis, std::vector<sBattleCommandMenu::sSubMenuEntry>& p2, sBattleItemSelectionTask* pMenu)
{
    FunctionUnimplemented();
}

void printBerserkAttackClass(int pageIndex)
{
    FunctionUnimplemented();
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
                playSystemSoundEffect(2);
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
            playSystemSoundEffect(2);
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
                ResetNBG1Map();
                setupVDP2StringRendering(6, 0x17, 0x20, 2);
                pThis->m20 &= ~1;
                pThis->m20 |= 0x10;
                clearVdp2TextArea();
                pThis->m2_mode = 7;
                createBattleCommandMenuSub0(pThis, 1);
                playSystemSoundEffect(4);
            }
        }
        else if ((gBattleManager->m10_battleOverlay->m4_battleEngine->m388 & 0x80U) == 0)
        {
            //select current entry
            ResetNBG1Map();
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
                    pThis->m3_itemMenuOpen = 0;
                }
                break;
            case 1: // homing laser
                uVar9 = 0;
                if (((0 < gBattleManager->m10_battleOverlay->m4_battleEngine->m498_numEnemies) &&
                    (0 < gBattleManager->m10_battleOverlay->m4_battleEngine->m3B4.m16_combo)) &&
                    ((((gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers) & 1) == 0 &&
                    (0 < (gBattleManager->m10_battleOverlay->mC_targetSystem->m20A_numSelectableEnemies))))) {
                    pThis->m20 |= 0x14;
                    clearVdp2TextArea();
                    uVar9 = 1;
                    pThis->m2_mode = 7;
                    pThis->m3_itemMenuOpen = 0;
                }
                break;
            case 2: // items
                uVar9 = 0;
                if ('\0' < (char)gBattleManager->m10_battleOverlay->m4_battleEngine->m3B4.m16_combo) {
                    pThis->m24 = BattleCommandMenu_PopulateSubMenu(pThis);
                    int iVar8;
                    if ((0 < pThis->m24) && (pThis->m1DC_itemSelectionMenuHead == nullptr)) {
                        pThis->m20 |= 8;
                        pThis->m2_mode = 3;
                        clearVdp2TextArea();
                        pThis->m3_itemMenuOpen = 0;
                        uVar9 = 1;
                    }
                }
                break;
            case 3: // berserk
                uVar9 = 0;
                if (('\0' < (char)gBattleManager->m10_battleOverlay->m4_battleEngine->m3B4.m16_combo) &&
                    (((gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers) & 1) == 0)) {
                    pThis->m24 = BattleCommandMenu_PopulateSubMenu(pThis);
                    int iVar8;
                    if ((0 < pThis->m24) &&
                        ((isBattleCommandEnabled(pThis, pThis->m0_selectedBattleCommand) > 0 &&
                        (pThis->m1E0_berserkSelectionMenuHead == nullptr)))) {
                        pThis->m20 |= 8;
                        pThis->m2_mode = 4;
                        clearVdp2TextArea();
                        pThis->m3_itemMenuOpen = 0;
                        uVar9 = 1;
                    }
                }
                break;
            case 4: // change weapon?
                uVar9 = 0;
                if ('\0' < (char)gBattleManager->m10_battleOverlay->m4_battleEngine->m3B4.m16_combo) {
                    pThis->m24 = BattleCommandMenu_PopulateSubMenu(pThis);
                    int iVar8;
                    if ((0 < pThis->m24) && (pThis->m1E4_weaponSelectionMenuHead == nullptr)) {
                        pThis->m20 |= 8;
                        pThis->m2_mode = 5;
                        clearVdp2TextArea();
                        pThis->m3_itemMenuOpen = 0;
                        uVar9 = 1;
                    }
                }
                break;
            case 5: // change form?
                uVar9 = 0;
                if (('\0' < (char)gBattleManager->m10_battleOverlay->m4_battleEngine->m3B4.m16_combo) &&
                    (((gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers) & 1) == 0)) {
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
                        ResetNBG1Map();
                        setupVDP2StringRendering(6, 0x17, 0x20, 2);
                        clearVdp2TextArea();
                        pThis->m2_mode = 6;
                        pThis->m3_itemMenuOpen = 0;
                        playSystemSoundEffect(4);
                        uVar9 = 1;
                        pThis->m18_oldDragonAtk = mainGameState.gameStats.mBE_dragonAtt;
                        pThis->m1A_oldDragonDef = mainGameState.gameStats.mBC_dragonDef;
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
        FPInterpolator_Step(&pThis->m1C0_scrollInterpolator);
        pThis->m14 = fixedPoint::toInteger(pThis->m1C0_scrollInterpolator.m0_currentValue + 0x8000);
        if (pThis->m3_itemMenuOpen == 0)
        {
            setBattleFont(0);
            createBattleItemSelectionTask(pThis, &pThis->m1DC_itemSelectionMenuHead, &pThis->m28_selectedItem, pThis->m24, pThis->m34_itemList);
            pThis->m20 &= ~0x20;
            pThis->m3_itemMenuOpen = 1;
        }
        else if (pThis->m3_itemMenuOpen)
        {
            if (pThis->m1DC_itemSelectionMenuHead)
            {
                drawBattleItemMenuSelectedItem(pThis, pThis->m34_itemList, pThis->m1DC_itemSelectionMenuHead);
            }
            else
            {
                if (pThis->m28_selectedItem < 0)
                {
                    createBattleCommandMenuSub0(pThis, 0);
                    pThis->m2_mode = 1;
                }
                else
                {
                    gBattleManager->m10_battleOverlay->m4_battleEngine->m39E_selectedItem = pThis->m34_itemList[pThis->m28_selectedItem].m0_itemIndex;
                    battleEngine_SetBattleMode(m1_useItem);
                    fieldPaletteTaskInitSub0Sub0();
                    pThis->getTask()->markFinished();
                }

                pThis->m20 &= ~0x8;
                pThis->m20 &= ~0x20;
                drawUsedItemName(-1, 0);
            }
        }
        break;
    case 4: // berserk selection
        FPInterpolator_Step(&pThis->m1C0_scrollInterpolator);
        pThis->m14 = fixedPoint::toInteger(pThis->m1C0_scrollInterpolator.m0_currentValue + 0x8000);
        if (pThis->m3_itemMenuOpen == 0)
        {
            setBattleFont(0);
            createBattleItemSelectionTask(pThis, &pThis->m1E0_berserkSelectionMenuHead, &pThis->m2C_selectedBerserk, pThis->m24, pThis->m114_berserkList);
            pThis->m20 &= ~0x20;
            pThis->m3_itemMenuOpen = 1;
        }
        else if (pThis->m3_itemMenuOpen)
        {
            if (pThis->m1E0_berserkSelectionMenuHead)
            {
                drawBattleItemMenuSelectedItem(pThis, pThis->m114_berserkList, pThis->m1E0_berserkSelectionMenuHead);
                printBerserkAttackClass(pThis->m1E0_berserkSelectionMenuHead->m3_currentPageIndex);
            }
            else
            {
                if (pThis->m2C_selectedBerserk < 0)
                {
                    createBattleCommandMenuSub0(pThis, 0);
                    pThis->m2_mode = 1;
                }
                else
                {
                    gBattleManager->m10_battleOverlay->m4_battleEngine->m3A2_selectedBerserk = pThis->m114_berserkList[pThis->m2C_selectedBerserk].m0_itemIndex;
                    battleEngine_SetBattleMode(m4_useBerserk);
                    fieldPaletteTaskInitSub0Sub0();
                    pThis->getTask()->markFinished();
                }

                pThis->m20 &= ~0x8;
                pThis->m20 &= ~0x20;
                drawUsedItemName(-1, 0);
                printBerserkAttackClass(6);
            }
        }
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
        pThis->m14 = fixedPoint::toInteger(pThis->m1C0_scrollInterpolator.m0_currentValue + 0x8000);
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
    int buttonWidth = pThis->mC + pThis->m14 + readSaturnS16(g_BTL_GenericData->getSaturnPtr(0x60ac154) + buttonIndex * 4) - 0xB0;
    int buttonHeight = (-readSaturnS16(g_BTL_GenericData->getSaturnPtr(0x60ac154) + buttonIndex * 4 + 2) - pThis->mE) + 0x70;

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
        pSpriteData = readSaturnEA(g_BTL_GenericData->getSaturnPtr(0x60b1bec) + 4 * buttonIndex);
    break;
    case 1:
        if(pThis->m0_selectedBattleCommand == buttonIndex)
        {
            pSpriteData = readSaturnEA(g_BTL_GenericData->getSaturnPtr(0x60b1bbc) + 4 * buttonIndex);
        }
        else
        {
            pSpriteData = readSaturnEA(g_BTL_GenericData->getSaturnPtr(0x60b1bd4) + 4 * buttonIndex);
        }
        break;
    case 2:
        pSpriteData = readSaturnEA(g_BTL_GenericData->getSaturnPtr(0x60b1c04) + 4 * buttonIndex);
        break;
    default:
        assert(0);
        break;
    }

    s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
    vdp1WriteEA.m0_CMDCTRL = 0x1000;
    vdp1WriteEA.m4_CMDPMOD = readSaturnS16(pSpriteData + 0xA);
    vdp1WriteEA.m6_CMDCOLR = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + readSaturnS16(pSpriteData + 0x6);
    vdp1WriteEA.m8_CMDSRCA = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + readSaturnS16(pSpriteData + 0x2);
    vdp1WriteEA.mA_CMDSIZE = readSaturnS16(pSpriteData + 0x8);
    vdp1WriteEA.mC_CMDXA = buttonWidth;
    vdp1WriteEA.mE_CMDYA = -buttonHeight;

    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

    graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
    graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
    graphicEngineStatus.m14_vdp1Context[0].mC += 1;
}

void BattleCommandMenu_Draw(sBattleCommandMenu* pThis)
{
    if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m4000 && (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.mA & 1))
    {
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m4000 = 0;
    }

    if (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m8_showingBattleResultScreen)
    {
        // main command menu background
        {
            s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
            vdp1WriteEA.m0_CMDCTRL = 0x1000;
            vdp1WriteEA.m4_CMDPMOD = 0x88;
            vdp1WriteEA.m6_CMDCOLR = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2ed0;
            vdp1WriteEA.m8_CMDSRCA = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x314;
            vdp1WriteEA.mA_CMDSIZE = 0x77F;
            vdp1WriteEA.mC_CMDXA = pThis->m14 - 0x97;
            vdp1WriteEA.mE_CMDYA = -0x64;

            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

            graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
            graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
            graphicEngineStatus.m14_vdp1Context[0].mC += 1;
        }

        // top left menu graphics
        {
            s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
            vdp1WriteEA.m0_CMDCTRL = 0x1000;
            vdp1WriteEA.m4_CMDPMOD = 0x88;
            vdp1WriteEA.m6_CMDCOLR = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2ed0;
            vdp1WriteEA.m8_CMDSRCA = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2D4;
            vdp1WriteEA.mA_CMDSIZE = 0x420;
            vdp1WriteEA.mC_CMDXA = pThis->m14 - 0xB0;
            vdp1WriteEA.mE_CMDYA = -(0x6E - pThis->m16);

            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

            graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
            graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
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
            // cursor shadow
            s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
            vdp1WriteEA.m0_CMDCTRL = 0x1000;
            vdp1WriteEA.m4_CMDPMOD = 0x88;
            vdp1WriteEA.m6_CMDCOLR = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2ECC;
            vdp1WriteEA.m8_CMDSRCA = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x218;
            vdp1WriteEA.mA_CMDSIZE = 0x418;
            vdp1WriteEA.mC_CMDXA = pThis->mC + readSaturnS16(g_BTL_GenericData->getSaturnPtr(0x60AC154) + pThis->m0_selectedBattleCommand * 4) - 0xB0;
            vdp1WriteEA.mE_CMDYA = -((-readSaturnS16(g_BTL_GenericData->getSaturnPtr(0x60AC156) + pThis->m0_selectedBattleCommand * 4) - pThis->mE) + 0x70);

            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

            graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
            graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
            graphicEngineStatus.m14_vdp1Context[0].mC += 1;
        }
        if (performModulo(0x14, pThis->m1C) < 10)
        {
            // blinking cursor
            s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
            vdp1WriteEA.m0_CMDCTRL = 0x1000;
            vdp1WriteEA.m4_CMDPMOD = 0x88;
            vdp1WriteEA.m6_CMDCOLR = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2ECC;
            vdp1WriteEA.m8_CMDSRCA = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x248;
            vdp1WriteEA.mA_CMDSIZE = 0x418;
            vdp1WriteEA.mC_CMDXA = pThis->mC + readSaturnS16(g_BTL_GenericData->getSaturnPtr(0x60AC154) + pThis->m0_selectedBattleCommand * 4) - 0xB0;
            vdp1WriteEA.mE_CMDYA = -((-readSaturnS16(g_BTL_GenericData->getSaturnPtr(0x60AC156) + pThis->m0_selectedBattleCommand * 4) - pThis->mE) + 0x70);

            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

            graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
            graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
            graphicEngineStatus.m14_vdp1Context[0].mC += 1;
        }
        break;
        case 3:
            // cursor shadow
            {
                s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
                vdp1WriteEA.m0_CMDCTRL = 0x1000;
                vdp1WriteEA.m4_CMDPMOD = 0x88;
                vdp1WriteEA.m6_CMDCOLR = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2ECC;
                vdp1WriteEA.m8_CMDSRCA = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x218;
                vdp1WriteEA.mA_CMDSIZE = 0x418;
                vdp1WriteEA.mC_CMDXA = pThis->mC + readSaturnS16(g_BTL_GenericData->getSaturnPtr(0x60AC154) + pThis->m0_selectedBattleCommand * 4) - 0xB0;
                vdp1WriteEA.mE_CMDYA = -((-readSaturnS16(g_BTL_GenericData->getSaturnPtr(0x60AC156) + pThis->m0_selectedBattleCommand * 4) - pThis->mE) + 0x70);

                graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
                graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
                graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

                graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
                graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
                graphicEngineStatus.m14_vdp1Context[0].mC += 1;
            }

            // cursor
            {
                s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
                vdp1WriteEA.m0_CMDCTRL = 0x1000;
                vdp1WriteEA.m4_CMDPMOD = 0x88;
                vdp1WriteEA.m6_CMDCOLR = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2ECC;
                vdp1WriteEA.m8_CMDSRCA = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x248;
                vdp1WriteEA.mA_CMDSIZE = 0x418;
                vdp1WriteEA.mC_CMDXA = pThis->mC + readSaturnS16(g_BTL_GenericData->getSaturnPtr(0x60AC154) + pThis->m0_selectedBattleCommand * 4) - 0xB0;
                vdp1WriteEA.mE_CMDYA = -((-readSaturnS16(g_BTL_GenericData->getSaturnPtr(0x60AC156) + pThis->m0_selectedBattleCommand * 4) - pThis->mE) + 0x70);

                graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
                graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
                graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

                graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
                graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
                graphicEngineStatus.m14_vdp1Context[0].mC += 1;

            }
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

    // allocate those as they used to be static
    pThis->m34_itemList.resize(0x38);
    pThis->m114_berserkList.resize(0x22);
    pThis->m19C_weaponList.resize(0x9);

    gBattleManager->m10_battleOverlay->m20_battleHud->m28_battleCommandMenu = pThis;
    pThis->mC = 0x19;
    pThis->mE = 0xC;
    playSystemSoundEffect(3);

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

    pThis->m1DC_itemSelectionMenuHead = nullptr;
    pThis->m1E0_berserkSelectionMenuHead = nullptr;
    pThis->m1E4_weaponSelectionMenuHead = nullptr;
    pThis->m4_enabledBattleCommands.fill(0);
    pThis->m1_numBattleCommands = 6;
}
