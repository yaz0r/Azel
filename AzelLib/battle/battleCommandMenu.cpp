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
#include "battle/gunShotRootTask.h"
#include "commonOverlay.h"
#include "battleTextDisplay.h"

void fieldPaletteTaskInitSub0Sub0(); // TODO: clean
bool isFlashChipDisabledInBattle();
int isBattleCommandEnabled(sBattleCommandMenu* pThis, int buttonIndex);
bool isCurrentBattleID(s8 battleID, s8 subID = 0);

// 06007378
s16 computeBerserkBPCost(s16 baseCost)
{
    fixedPoint spr = FP_Div((s32)mainGameState.gameStats.mC2_dragonSpr, 200);
    fixedPoint reduction = MTH_Mul(fixedPoint(0x6666), spr);
    fixedPoint result = MTH_Mul(fixedPoint::fromInteger(baseCost), fixedPoint(0x13333) - reduction);
    return (s16)((result.asS32() + 0x8000) >> 16);
}

// Todo: kernel
void BattleCommandMenu_UpdateSub0()
{
    vdp2Controls.m4_pendingVdp2Regs->m26_SFCODE = 0xC000;
    vdp2Controls.m_isDirty = 1;
}

void setBattleFont(int param_1)
{
    s8 fontIndex = (param_1 == 0)
        ? gBattleManager->m10_battleOverlay->m14_textDisplay->m18
        : gBattleManager->m10_battleOverlay->m14_textDisplay->m17;
    setActiveFont(fontIndex);
}

// BTL_A3::06073c8c
static void drawBattleItemDescription(s32 param_1)
{
    s_dragonState* pDragon = gDragonState;
    u8 bVar1 = (u8)getObjectListEntry((eItems)param_1)->m1_type;
    s32 iVar4 = param_1;

    if (bVar1 == 0 || bVar1 == 1 || bVar1 == 2) {
        s16 baseCost = readSaturnS16(gCommonFile->getSaturnPtr(0x20C3F4) + (s32)param_1 * 12 + 2);
        s16 cost = computeBerserkBPCost(baseCost);
        if ((s16)mainGameState.gameStats.m14_currentBP < cost)
            iVar4 = 0x4b;
        switch (param_1) {
        case 0x9a: case 0xae: case 0xaf: case 0xb0: case 0xb2:
            if (gBattleManager->m10_battleOverlay->m4_battleEngine->m3B4.m16_combo != 3)
                iVar4 = 0x4c;
            break;
        default:
            if ((s8)gBattleManager->m10_battleOverlay->m4_battleEngine->m3B4.m16_combo < 2)
                iVar4 = 0x4c;
            break;
        case 0xa6:
            break;
        case 0xaa: case 0xab: case 0xac: case 0xad:
            if (gBattleManager->m10_battleOverlay->m4_battleEngine->m3B4.m16_combo == 0)
                iVar4 = 0x4c;
            break;
        }
    }
    else if ((bVar1 == 3 || bVar1 == 4 || bVar1 == 5) &&
             param_1 == (s32)(s16)(u16)mainGameState.gameStats.mA_weaponType) {
        drawObjectName("Equipping.");
        return;
    }

    s16 sVar5 = (s16)iVar4;
    u32 statusMods = gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers;

    if (param_1 == 4 || param_1 == 0xb1) {
        if (isFlashChipDisabledInBattle()) sVar5 = 0x47;
    }
    else if (param_1 == 5 || param_1 == 0x26 || param_1 == 0x27 || param_1 == 0x28 ||
             param_1 == 0xaa || param_1 == 0xab || param_1 == 0xac) {
        if (mainGameState.gameStats.mB8_maxHP == mainGameState.gameStats.m10_currentHP)
            sVar5 = 0x42;
    }
    else if (param_1 == 6 || param_1 == 0x29 || param_1 == 0x2a || param_1 == 0x2b) {
        if (mainGameState.gameStats.mBA_maxBP == mainGameState.gameStats.m14_currentBP)
            sVar5 = 0x43;
    }
    else if (param_1 == 0x2c) {
        if (mainGameState.gameStats.mB8_maxHP == mainGameState.gameStats.m10_currentHP &&
            mainGameState.gameStats.mBA_maxBP == mainGameState.gameStats.m14_currentBP)
            sVar5 = 0x46;
    }
    else if (param_1 == 0x2d) { if (!(statusMods & 1)) sVar5 = 0x44; }
    else if (param_1 == 0x2e) { if (!(statusMods & 2)) sVar5 = 0x44; }
    else if (param_1 == 0x2f || param_1 == 0xad) { if (!(statusMods & 0x7f)) sVar5 = 0x44; }
    else if (param_1 == 0x30 || param_1 == 0x94 || param_1 == 0xa3 || param_1 == 0xa5) {
        if (statusMods & 0x400) sVar5 = 0x48;
    }
    else if (param_1 == 0x31) { if (statusMods & 0x80) sVar5 = 0x48; }
    else if (param_1 == 0x32) { if (statusMods & 0x100) sVar5 = 0x48; }
    else if (param_1 == 0x33) { if (statusMods & 0x200) sVar5 = 0x48; }
    else if (param_1 == 0x34) { if (!(statusMods & 4)) sVar5 = 0x44; }
    else if (param_1 == 0x35) { if (!(statusMods & 8)) sVar5 = 0x44; }
    else if (param_1 == 0x36) { if (!(statusMods & 0x10)) sVar5 = 0x44; }
    else if (param_1 == 0x37) { if (!(statusMods & 0x20)) sVar5 = 0x44; }
    else if (param_1 == 0x9a) {
        if (statusMods & 0x80) sVar5 = 0x48;
        if (mainGameState.gameStats.m1_dragonLevel != 6 && pDragon->m1C_dragonArchetype != 2)
            sVar5 = 0x4a;
    }
    else if (param_1 == 0xae) {
        if (mainGameState.gameStats.mBA_maxBP == mainGameState.gameStats.m14_currentBP)
            sVar5 = 0x43;
        if (mainGameState.gameStats.m1_dragonLevel != 6 && pDragon->m1C_dragonArchetype != 4)
            sVar5 = 0x4a;
    }
    else if (param_1 == 0xaf) {
        if (statusMods & 0x100) sVar5 = 0x48;
        if (mainGameState.gameStats.m1_dragonLevel != 6 && pDragon->m1C_dragonArchetype != 1)
            sVar5 = 0x4a;
    }
    else if (param_1 == 0xb0) {
        if (statusMods & 0x200) sVar5 = 0x48;
        if (mainGameState.gameStats.m1_dragonLevel != 6 && pDragon->m1C_dragonArchetype != 3)
            sVar5 = 0x4a;
    }
    else if (param_1 == 0xb2) {
        if (mainGameState.gameStats.mB8_maxHP == mainGameState.gameStats.m10_currentHP)
            sVar5 = 0x42;
        if (mainGameState.gameStats.m1_dragonLevel != 6 && pDragon->m1C_dragonArchetype != 0)
            sVar5 = 0x4a;
    }

    if ((bVar1 == 0 || bVar1 == 6) &&
        gBattleManager->m10_battleOverlay->mC_targetSystem->m20A_numSelectableEnemies < 1)
        sVar5 = 0x45;

    drawObjectName(getObjectListEntry((eItems)sVar5)->m8_description.c_str());
}

// BTL_A3::06074170
void drawUsedItemName(s32 param_1, s32 param_2)
{
    if (gBattleManager->m10_battleOverlay->m20_battleHud->m18_part1Y != -0x18)
        return;
    setupVDP2StringRendering(6, 0x17, 0x20, 2);
    if (param_1 == -1) {
        setBattleFont(1);
        clearVdp2TextArea();
        return;
    }
    setBattleFont(0);
    vdp2StringContext.m1C = vdp2StringContext.m0;
    vdp2StringContext.m0 = 0;
    if ((param_2 & 1) == 0)
        drawObjectName(getObjectListEntry((eItems)param_1)->m8_description.c_str());
    else
        drawBattleItemDescription(param_1);
    vdp2StringContext.m0 = vdp2StringContext.m1C;
    vdp2ClearRemainingTextArea();
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


// BTL_A3::06073ac8
void BattleCommandMenu_DisplayCommandString(sBattleCommandMenu* pThis)
{
    if (gBattleManager->m10_battleOverlay->m20_battleHud->m18_part1Y != -0x18)
        return;
    setupVDP2StringRendering(6, 0x17, 0x20, 2);
    setBattleFont(1);
    vdp2StringContext.m1C = vdp2StringContext.m0;
    vdp2StringContext.m0 = 0;

    s16 sVar3 = 0x1c; // default: form change (poisoned)
    s8 cmd = pThis->m0_selectedBattleCommand;
    s8 combo = (s8)gBattleManager->m10_battleOverlay->m4_battleEngine->m3B4.m16_combo;

    switch (cmd) {
    case 0: // gun
        sVar3 = (combo < 1) ? 0x1d : 0x15;
        break;
    case 1: // homing laser
        if (!(gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 1)) {
            sVar3 = (combo < 1) ? 0x1d : 0x16;
        }
        break;
    case 2: // items
        if (isBattleCommandEnabled(pThis, cmd) < 1)
            sVar3 = 0x19;
        else
            sVar3 = (combo < 1) ? 0x1d : 0x11;
        break;
    case 3: // berserk
        if (mainGameState.gameStats.m1_dragonLevel == 8 || isBattleCommandEnabled(pThis, cmd) < 1) {
            sVar3 = 0x1a;
        } else if (!(gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 1)) {
            sVar3 = (combo < 1) ? 0x1d : 0x12;
        }
        break;
    case 4: // weapons
        if (isBattleCommandEnabled(pThis, cmd) < 1)
            sVar3 = 0x1b;
        else
            sVar3 = (combo < 1) ? 0x1d : 0x13;
        break;
    case 5: // form change
        if (mainGameState.gameStats.m1_dragonLevel == 0 || mainGameState.gameStats.m1_dragonLevel == 6)
            sVar3 = 0x17;
        else if (mainGameState.gameStats.m1_dragonLevel == 8)
            sVar3 = 0x18;
        else if (!(gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 1))
            sVar3 = (combo < 1) ? 0x1d : 0x14;
        break;
    }

    sSaturnPtr strPtr = readSaturnEA(g_BTL_GenericData->getSaturnPtr(0x60ab658) + (s32)sVar3 * 4);
    drawObjectName(readSaturnString(strPtr).c_str());
    vdp2StringContext.m0 = vdp2StringContext.m1C;
    vdp2ClearRemainingTextArea();
    pThis->m20 |= 1;
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
    if (!isCurrentBattleID(0, 8) && !isCurrentBattleID(0, 9) &&
        !isCurrentBattleID(0x10, 0) && !isCurrentBattleID(0x12, 0) &&
        !isCurrentBattleID(0x11, 0) && !isCurrentBattleID(0x13, 0) &&
        !isCurrentBattleID(4, 0) && !isCurrentBattleID(5, 4) &&
        !isCurrentBattleID(5) && !isCurrentBattleID(5, 8) &&
        !isCurrentBattleID(0x14, 0) && !isCurrentBattleID(6, 0xc) &&
        !isCurrentBattleID(0x16, 0) && !isCurrentBattleID(7, 5) &&
        !isCurrentBattleID(7, 6) && !isCurrentBattleID(0x17, 0) &&
        !isCurrentBattleID(8, 1) && !isCurrentBattleID(8, 2) &&
        !isCurrentBattleID(8, 3) && !isCurrentBattleID(8, 4) &&
        !isCurrentBattleID(8, 5) && !isCurrentBattleID(8) &&
        !isCurrentBattleID(9, 0) && !isCurrentBattleID(9, 4) &&
        !isCurrentBattleID(0x18, 0) && !isCurrentBattleID(10, 0) &&
        !isCurrentBattleID(10, 2) && !isCurrentBattleID(10, 5) &&
        !isCurrentBattleID(10, 7) && !isCurrentBattleID(10) &&
        !isCurrentBattleID(0x19, 0) && !isCurrentBattleID(0xb, 0) &&
        !isCurrentBattleID(0xb, 1) && !isCurrentBattleID(0xb, 2) &&
        !isCurrentBattleID(0xb, 3) && !isCurrentBattleID(0xb, 4) &&
        !isCurrentBattleID(0xb, 5) && !isCurrentBattleID(0xb, 6) &&
        !isCurrentBattleID(0xb, 7) && !isCurrentBattleID(0x1a, 0) &&
        !isCurrentBattleID(6, 0xd) && !isCurrentBattleID(0xc, 6) &&
        !isCurrentBattleID(0xc, 7) && !isCurrentBattleID(0xe, 0) &&
        !isCurrentBattleID(0xe, 1) && !isCurrentBattleID(0xe, 2) &&
        !isCurrentBattleID(0xd, 0) && !isCurrentBattleID(0xd, 1) &&
        !isCurrentBattleID(0xd, 2) && !isCurrentBattleID(0xd, 3) &&
        !isCurrentBattleID(0xd, 4) && !isCurrentBattleID(0xd, 5))
    {
        return false;
    }
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
    case (eItems)0x26: case (eItems)0x27: case (eItems)0x28:
        if (mainGameState.gameStats.m10_currentHP == mainGameState.gameStats.mB8_maxHP)
        {
            itemDisabled = true;
        }
        break;
    case eItems::m6_berserkMicro:
    case (eItems)0x29: case (eItems)0x2a: case (eItems)0x2b:
        if (mainGameState.gameStats.m14_currentBP == mainGameState.gameStats.mBA_maxBP)
        {
            itemDisabled = true;
        }
        break;
    case (eItems)0x2c:
        if (mainGameState.gameStats.m10_currentHP == mainGameState.gameStats.mB8_maxHP &&
            mainGameState.gameStats.m14_currentBP == mainGameState.gameStats.mBA_maxBP)
            itemDisabled = true;
        break;
    case (eItems)0x2d:
        if (!(gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 1)) itemDisabled = true;
        break;
    case (eItems)0x2e:
        if (!(gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 2)) itemDisabled = true;
        break;
    case (eItems)0x2f:
        if (!(gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 0x7f)) itemDisabled = true;
        break;
    case (eItems)0x30:
        if (gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 0x400) itemDisabled = true;
        break;
    case (eItems)0x31:
        if (gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 0x80) itemDisabled = true;
        break;
    case (eItems)0x32:
        if (gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 0x100) itemDisabled = true;
        break;
    case (eItems)0x33:
        if (gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 0x200) itemDisabled = true;
        break;
    case (eItems)0x34:
        if (!(gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 4)) itemDisabled = true;
        break;
    case (eItems)0x35:
        if (!(gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 8)) itemDisabled = true;
        break;
    case (eItems)0x36:
        if (!(gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 0x10)) itemDisabled = true;
        break;
    case (eItems)0x37:
        if (!(gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 0x20)) itemDisabled = true;
        break;
    default:
        break;
    }

    pEntry->m0_itemIndex = index;
    if (!itemDisabled)
    {
        pEntry->m2 = 0;
    }
}

// 06072e9c
void addBerserkToActiveList(sBattleCommandMenu::sSubMenuEntry* pEntry, eItems index)
{
    s16 baseCost = readSaturnS16(gCommonFile->getSaturnPtr(0x20C3F4) + (s32)index * 12 + 2);
    s16 cost = computeBerserkBPCost(baseCost);
    bool bVar1 = mainGameState.gameStats.m14_currentBP < cost;

    s_dragonState* pDragon = gDragonState;

    if (gBattleManager->m10_battleOverlay->m4_battleEngine->m3B4.m16_combo < 2)
    {
        if (index == (eItems)0xaa || index == (eItems)0xab || index == (eItems)0xac)
        {
            bVar1 = gBattleManager->m10_battleOverlay->m4_battleEngine->m3B4.m16_combo == 0 || bVar1;
            if (mainGameState.gameStats.mB8_maxHP == mainGameState.gameStats.m10_currentHP)
                bVar1 = true;
        }
        else if (index == (eItems)0xad)
        {
            bVar1 = gBattleManager->m10_battleOverlay->m4_battleEngine->m3B4.m16_combo == 0 || bVar1;
            if ((gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 0x7f) == 0)
                bVar1 = true;
        }
        else
        {
            bVar1 = true;
        }
    }

    switch (index)
    {
    case (eItems)0x94:
    case (eItems)0xa3:
    case (eItems)0xa5:
        if (gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 0x400)
            bVar1 = true;
        break;
    default:
        if (gBattleManager->m10_battleOverlay->mC_targetSystem->m20A_numSelectableEnemies == 0)
            bVar1 = true;
        break;
    case (eItems)0x9a:
        if (mainGameState.gameStats.m1_dragonLevel != 6 && pDragon->m1C_dragonArchetype != 2)
            bVar1 = true;
        if (gBattleManager->m10_battleOverlay->m4_battleEngine->m3B4.m16_combo != 3)
            bVar1 = true;
        if (gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 0x80)
            bVar1 = true;
        break;
    case (eItems)0xaa:
    case (eItems)0xab:
    case (eItems)0xac:
        if (mainGameState.gameStats.mB8_maxHP == mainGameState.gameStats.m10_currentHP)
            bVar1 = true;
        break;
    case (eItems)0xad:
        if ((gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 0x7f) == 0)
            bVar1 = true;
        break;
    case (eItems)0xae:
        if (mainGameState.gameStats.m1_dragonLevel != 6 && pDragon->m1C_dragonArchetype != 4)
            bVar1 = true;
        if (gBattleManager->m10_battleOverlay->m4_battleEngine->m3B4.m16_combo != 3)
            bVar1 = true;
        if (mainGameState.gameStats.mBA_maxBP == mainGameState.gameStats.m14_currentBP)
            bVar1 = true;
        break;
    case (eItems)0xaf:
        if (mainGameState.gameStats.m1_dragonLevel != 6 && pDragon->m1C_dragonArchetype != 1)
            bVar1 = true;
        if (gBattleManager->m10_battleOverlay->m4_battleEngine->m3B4.m16_combo != 3)
            bVar1 = true;
        if (gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 0x100)
            bVar1 = true;
        break;
    case (eItems)0xb0:
        if (mainGameState.gameStats.m1_dragonLevel != 6 && pDragon->m1C_dragonArchetype != 3)
            bVar1 = true;
        if (gBattleManager->m10_battleOverlay->m4_battleEngine->m3B4.m16_combo != 3)
            bVar1 = true;
        if (gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 0x200)
            bVar1 = true;
        break;
    case (eItems)0xb1:
        if (isFlashChipDisabledInBattle())
            bVar1 = true;
        break;
    case (eItems)0xb2:
        if (mainGameState.gameStats.m1_dragonLevel != 6 && pDragon->m1C_dragonArchetype != 0)
            bVar1 = true;
        if (gBattleManager->m10_battleOverlay->m4_battleEngine->m3B4.m16_combo != 3)
            bVar1 = true;
        if (mainGameState.gameStats.mB8_maxHP == mainGameState.gameStats.m10_currentHP)
            bVar1 = true;
        break;
    }

    pEntry->m0_itemIndex = index;
    if (!bVar1)
        pEntry->m2 = 0;
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
    case 4: // weapons
        for (int i = 0; i < 9; i++)
        {
            pThis->m19C_weaponList[i].m0_itemIndex = eItems::m0_dummy;
            pThis->m19C_weaponList[i].m2 = 1;
        }
        for (int i = 0; i < 9 && weaponTable[i].m0_weaponType > 0; i++)
        {
            eItems wType = (eItems)weaponTable[i].m0_weaponType;
            const sObjectListEntry* pObject = getObjectListEntry(wType);
            if (pObject->m0_flags & 2)
            {
                if (mainGameState.getItemCount(wType))
                {
                    u8 type = pObject->m1_type;
                    if (type == 3 || type == 4 || type == 5)
                    {
                        pThis->m19C_weaponList[totalEntryCount].m0_itemIndex = wType;
                        pThis->m19C_weaponList[totalEntryCount].m2 = (mainGameState.gameStats.mA_weaponType == wType) ? 2 : 0;
                        totalEntryCount++;
                    }
                }
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
    int count = 0;
    if (buttonIndex == 2) // items
    {
        for (int i = 0; i < 0x38; i++)
        {
            if (mainGameState.getItemCount((eItems)i))
            {
                u8 type = getObjectListEntry((eItems)i)->m1_type;
                if (type == 6 || type == 7 || type == 8)
                    count++;
            }
        }
    }
    else if (buttonIndex == 3) // berserks
    {
        for (int classType = 0; classType < 6; classType++)
        {
            sSaturnPtr classBerserksTable = readSaturnEA(g_BTL_GenericData->getSaturnPtr(0x60ac094) + classType * 4);
            for (int j = 0; j < 5; j++)
            {
                s16 berserkId = readSaturnS16(classBerserksTable + j * 8 + 6);
                if (berserkId && mainGameState.getItemCount((eItems)berserkId))
                    count++;
            }
        }
    }
    else if (buttonIndex == 4) // weapons
    {
        for (int i = 0x39; i < 0x42; i++)
        {
            if (mainGameState.getItemCount((eItems)i))
            {
                u8 type = getObjectListEntry((eItems)i)->m1_type;
                if (type == 3 || type == 4 || type == 5)
                    count++;
            }
        }
    }
    return count;
}

// BTL_A3::060742a8
void drawBattleItemMenuSelectedItem(sBattleCommandMenu* pThis, std::vector<sBattleCommandMenu::sSubMenuEntry>& list, sBattleItemSelectionTask* pMenu)
{
    if (gBattleManager->m10_battleOverlay->m20_battleHud->m18_part1Y != -0x18)
        return;

    if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 0xf0) == 0) {
        sBattleCommandMenu::sSubMenuEntry* pEntry = &list[pMenu->m3_currentPageIndex * 5 + pMenu->m5_currentSelectionInPage];
        if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 6) == 0) {
            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 1)
                pThis->m20 &= ~0x20u;
        }
        else if ((pEntry->m2 & 1) == 0)
            pThis->m20 &= ~0x20u;
        else
            pThis->m20 |= 0x20;
        drawUsedItemName((s32)pEntry->m0_itemIndex, (pThis->m20 & 0x20) != 0);
    }
    else {
        pThis->m20 &= ~0x20u;
    }
}

void printBerserkAttackClass(int pageIndex)
{
    vdp2PrintStatus.m14_oldPalette = vdp2PrintStatus.m10_palette;
    vdp2PrintStatus.m10_palette = 0xc000;
    vdp2DebugPrintSetPosition(6, 3);
    sSaturnPtr strPtr = readSaturnEA(g_BTL_GenericData->getSaturnPtr(0x060ac0ac) + pageIndex * 4);
    vdp2PrintfLargeFont("%s", readSaturnString(strPtr).c_str());
    vdp2PrintStatus.m10_palette = vdp2PrintStatus.m14_oldPalette;
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
                resetNBG1Map();
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
            resetNBG1Map();
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
                        resetNBG1Map();
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
        FPInterpolator_Step(&pThis->m1C0_scrollInterpolator);
        pThis->m14 = fixedPoint::toInteger(pThis->m1C0_scrollInterpolator.m0_currentValue + 0x8000);
        if (pThis->m3_itemMenuOpen == 0)
        {
            setBattleFont(0);
            createBattleItemSelectionTask(pThis, &pThis->m1E4_weaponSelectionMenuHead, &pThis->m30_selectedWeapon, pThis->m24, pThis->m19C_weaponList);
            pThis->m20 &= ~0x20;
            pThis->m3_itemMenuOpen = 1;
        }
        else if (pThis->m3_itemMenuOpen)
        {
            if (pThis->m1E4_weaponSelectionMenuHead)
            {
                drawBattleItemMenuSelectedItem(pThis, pThis->m19C_weaponList, pThis->m1E4_weaponSelectionMenuHead);
            }
            else
            {
                if (pThis->m30_selectedWeapon < 0)
                {
                    createBattleCommandMenuSub0(pThis, 0);
                    pThis->m2_mode = 1;
                }
                else
                {
                    if (mainGameState.gameStats.mA_weaponType == pThis->m19C_weaponList[pThis->m30_selectedWeapon].m0_itemIndex)
                        mainGameState.gameStats.mA_weaponType = (eItems)0;
                    else
                        mainGameState.gameStats.mA_weaponType = pThis->m19C_weaponList[pThis->m30_selectedWeapon].m0_itemIndex;
                    pThis->m2_mode = 7;
                    pThis->m20 |= 0x10;
                    if (!gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x15])
                        gBattleManager->m10_battleOverlay->m4_battleEngine->m3B4.m16_combo--;
                    playSystemSoundEffect(7);
                }
                pThis->m20 &= ~0x8;
                pThis->m20 &= ~0x20;
                drawUsedItemName(-1, 0);
            }
        }
        break;
    case 6: // form change
    {
        s32 atkDiff = pThis->m18_oldDragonAtk - mainGameState.gameStats.mBE_dragonAtt;
        if (atkDiff < 0)
            atkDiff = mainGameState.gameStats.mBE_dragonAtt - pThis->m18_oldDragonAtk;
        s32 defDiff = 0;
        if (atkDiff < 3)
        {
            defDiff = pThis->m1A_oldDragonDef - mainGameState.gameStats.mBC_dragonDef;
            if (defDiff < 0)
                defDiff = mainGameState.gameStats.mBC_dragonDef - pThis->m1A_oldDragonDef;
        }
        if (atkDiff >= 3 || defDiff >= 3)
        {
            if (!gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x15])
                gBattleManager->m10_battleOverlay->m4_battleEngine->m3B4.m16_combo--;
            pThis->m2_mode = 7;
            pThis->m20 = (pThis->m20 & ~1u) | 0x10;
        }
        else
        {
            pThis->m2_mode = 2;
        }
        break;
    }
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
        vdp2PrintStatus.m14_oldPalette = vdp2PrintStatus.m10_palette;
        vdp2PrintStatus.m10_palette = 0xc000;
        vdp2DebugPrintSetPosition(10, 0xe);
        vdp2PrintfSmallFont("PHASE[%d:%d] CURSOR[%d:%d]", (int)pThis->m2_mode, (int)pThis->m3_itemMenuOpen);
        vdp2PrintStatus.m10_palette = vdp2PrintStatus.m14_oldPalette;
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
    mainGameState.setPackedBits(0x1043, 4, pThis->m0_selectedBattleCommand);
    gBattleManager->m10_battleOverlay->m20_battleHud->m28_battleCommandMenu = nullptr;
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
