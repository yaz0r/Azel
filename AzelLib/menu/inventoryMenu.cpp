#include "PDS.h"
#include "inventoryMenu.h"
#include "kernel/menuCursor.h"
#include "items.h"
#include "audio/systemSounds.h"
#include "commonOverlay.h"

void switchVdp2MenuTileToHightlighted(u32 vdp2Offset, u32 r5, u32 r6, u32 r7); // TODO: cleanup
s32 getCurrentGunPower(); // TODO: cleanup
u32 loadDrawSub1Sub0(s32* param1, u32 param2, u32 param3); //TODO: cleanup

struct s_inventoryMenu : s_workAreaTemplate<s_inventoryMenu>
{
    s32 m0;
    s32 m4_currentPageInItemList;
    s32 m8;
    s32 mC;
    s32 m10;
    s32 m1C_selectedCategory;
    s_MenuCursorWorkArea* m20_menuCursorCategory;
    s32 m24_currentSelectedItemInCurrentItemPage;
    s_MenuCursor2* m28_menuCursorItem;
    std::array<s8, 4> m2C_hasCategoryAtLeastOneEntry;
    s32 m30_numItemsInCurrentCategory;
    std::array<eItems, 100> m34_itemsInCurrentCategory;
    // size 0xFC
};

void initVdp2ForInventory(int param_1)
{
    if (param_1 != 0) {
        setVdp2LayerScroll(0, 0, 0);
        setVdp2LayerScroll(1, 0, 0);
        setVdp2LayerScroll(3, 0, 0x100);
        resetVdp2LayersAutoScroll();
        setupVDP2StringRendering(0, 0x22, 0x2c, 0x1c);
        clearVdp2TextArea();
    }
    unpackGraphicsToVDP2(gCommonFile->m_data + 0xed54, getVdp2Vram(0x71000));
    return;
}

const std::array<u8, 3> inventoryMenuTable = {
    1,2,4
};

const std::array<u32, 4> inventoryMenuSpriteTable = { {
    {0x25E71084},
    {0x25E71104},
    {0x25E71184},
    {0x25E71204},
} };

static const s16 sellableItemTable[] = {
     0x5,    0x26,    0x27,    0x28,
     0x6,    0x29,    0x2A,    0x2B,
    0x2C,    0x2D,    0x2E,    0x34,
    0x35,    0x36,    0x37,    0x2F,
     0x1,     0x2,    0x3,    0x4,
    0x30,    0x31,    0x32,    0x33,
     0x7,     0x8,    0x9,    0xA,
     0xB,     0xC,    0x12,    0x13,
    0x14,    0x15,    0x17,    0x18,
    0x19,    0x1A,    0x1B,    0x1C,
    0x1D,    0x1E,    0x1F,    0x20,
    0x21,    0x22,    0x23,    0x24,
    0x25,     0xD,    0xE,    0xF,
    0x10,    0x11,    0x16,    0x38,
    -1
};

static const s16 nonSellableItemTable[] = {
    0x69, 0x6F, 0x70, 0x77,
    0x78, 0x79, 0x7A, 0x7B,
    0x7C, 0x7D, 0x7E, 0x7F,
    0x80, 0x81, 0x82, 0x86,
    0x8A, 0x8B, 0x8D, 0x8E,
    0x8F, 0x90, 0x6D, 0x6E,
    0x66, 0x67, 0x68, 0x72,
    0x73, 0x6A, 0x4E, 0x4F,
    0x50, 0x52, 0x53, 0x54,
    0x51, 0x6B, 0x6C, 0x71,
    0x55, 0x65, 0x74, 0x75,
    0x76, 0x83, 0x84, 0x85,
    0x87, 0x88, 0x89, 0x8C,
    0x63, 0x64, 0x56, 0x57,
    0x58, 0x59, 0x5A, 0x5B,
    0x5C, 0x5D, 0x5E, 0x5F,
    0x60, 0x61,
    -1
};

static const s16 berserkItems[] = {
    0xA0, 0x9F, 0x99, 0x9C,
    0x9E, 0xA1, 0x97, 0x95,
    0x96, 0x98, 0xAA, 0x94,
    0xAB, 0xA5, 0xAC, 0x91,
    0xB1, 0x92, 0xAD, 0x93,
    0xB2, 0x9A, 0xAE, 0xB0,
    0xAF, 0xA4, 0x9B, 0xA2,
    0x9D, 0xA3,
    -1
};

static const s16 weaponItems[] = {
    0x3B, 0x3A, 0x3C, 0x3E,
    0x3D, 0x40, 0x41, 0x3F,
    -1
};

static const std::array<const s16*,4> inventoryCatergoryTables = {
    sellableItemTable,
    nonSellableItemTable,
    berserkItems,
    weaponItems
};

u16 inventoryMenuCursorData_[]{
    0x1C, 0x1C,
    0x1C, 0x3C,
    0x1C, 0x5C,
    0x1C, 0x7C,
};

sMainMenuTaskInitData2 inventoryMenuCursorData = {
    &graphicEngineStatus.m40BC_layersConfig[1],
    inventoryMenuCursorData_
};

bool hasInventoryCategoryAtLeastOneEntry(s32 category)
{
    const s16* pTable = inventoryCatergoryTables[category];

    int iVar1;

    do {
        while (true) {
            iVar1 = *(pTable++);
            if (iVar1 < 0) {
                return 0;
            }
            if (0x4c < iVar1) break;
            if (mainGameState.consumables[iVar1] != 0) {
                return 1;
            }
        }
    } while (!mainGameState.getBit(0xF3 + iVar1));
    return 1;
}

static std::vector<std::array<s16, 2>> inventoryMenuCursor2 = {
    {
        {0x54, 0x10},
        {0x54, 0x28},
        {0x54, 0x40},
        {0x54, 0x58},
        {0x54, 0x70},
    }
};

static std::vector<std::array<s16, 2>> inventoryMenuCursor2_pal = {
    {
        {0x54, 0x20},
        {0x54, 0x38},
        {0x54, 0x50},
        {0x54, 0x68},
        {0x54, 0x80},
    }
};

void inventoryMenuTaskInitSub1(s_inventoryMenu* pThis)
{
    pThis->mC = loadFnt("ITEM.FNT");
    pThis->m20_menuCursorCategory = createMenuCursorTask(pThis, &inventoryMenuCursorData);

    if ((VDP2Regs_.m4_TVSTAT & 1) == 0) {
        pThis->m28_menuCursorItem = createMenuCursorTask2(pThis, &inventoryMenuCursor2);
    }
    else {
        pThis->m28_menuCursorItem = createMenuCursorTask2(pThis, &inventoryMenuCursor2_pal);
    }

    pThis->m20_menuCursorCategory->m0_selectedEntry = -1;
    pThis->m28_menuCursorItem->m0 = -1;
}

void inventoryMenuTaskInit(s_inventoryMenu* pThis)
{
    if (graphicEngineStatus.m40AC.m0_menuId == 1) {
        initVdp2ForInventory(0);
        pThis->m0 = 0;
        pThis->m8 = 0x10;
        startVdp2LayerScroll(0, -10, 0, 0x10);
        startVdp2LayerScroll(1, 0, -0x10, 0x10);
    }
    else
    {
        initVdp2ForInventory(1);
        pThis->m0 = 1;
    }

    int iVar3 = -1;
    for (int i=0; i<4; i++)
    {
        if (!hasInventoryCategoryAtLeastOneEntry(i))
        {
            pThis->m2C_hasCategoryAtLeastOneEntry[i] = 0;
        }
        else
        {
            if (iVar3 < 0)
            {
                iVar3 = i;
                switchVdp2MenuTileToHightlighted(inventoryMenuSpriteTable[i], 2, 2, 0x620);
            }
            else
            {
                switchVdp2MenuTileToHightlighted(inventoryMenuSpriteTable[i], 2, 2, 0x660);
            }
            pThis->m2C_hasCategoryAtLeastOneEntry[i] = 1;
        }
    }

    if (iVar3 < 0)
    {
        iVar3 = 0;
    }

    pThis->m1C_selectedCategory = iVar3;
    inventoryMenuTaskInitSub1(pThis);
    pThis->m10 = inventoryMenuTable[mainGameState.readPackedBits(0, 2)];
    pThis->m4_currentPageInItemList = -1;
}

static const std::array<const char*, 4> inventoryMenuText =
{
    "List of items that can be sold",
    "List of items that cannot be sold",
    "Show Berserk List",
    "Customize Your Weapon",
};

void displayInventoryCategoryDescription(s_inventoryMenu* pThis)
{
    pThis->m4_currentPageInItemList = -1;
    pThis->m20_menuCursorCategory->m0_selectedEntry = pThis->m1C_selectedCategory;
    pThis->m28_menuCursorItem->m0 = -1;

    setupVDP2StringRendering(10, 0x24, 0x1e, 0xe);
    clearVdp2TextArea();
    setupVDP2StringRendering(3, 0x38, 0x26, 4);
    clearVdp2TextArea();

    if (-1 < pThis->m1C_selectedCategory) {
        vdp2StringContext.m0 = 0;
        drawObjectName(inventoryMenuText[pThis->m1C_selectedCategory]);
    }
}

void drawInventoryStatusOrWeaponStats(s_inventoryMenu* pThis)
{
    vdp2PrintStatus.m10_palette = 0xc000;
    setupVDP2StringRendering(5, 0x35, 0x22, 2);
    clearVdp2TextArea();
    if (pThis->m1C_selectedCategory == 3) {
        vdp2StringContext.m0 = 0;
        drawObjectName("ATCH");
        vdp2StringContext.m4_cursorX = vdp2StringContext.mC_X + 0x18;
        vdp2StringContext.m8_cursorY = vdp2StringContext.m10_Y;
        drawObjectName("POWER");
        vdp2StringContext.m4_cursorX = vdp2StringContext.mC_X + 5;
        vdp2StringContext.m8_cursorY = vdp2StringContext.m10_Y;
        if (mainGameState.gameStats.mA_weaponType == 0)
        {
            drawObjectName("None");
        }
        else {
            vdp2StringContext.m4_cursorX = 1;
            drawObjectName(getObjectListEntry(mainGameState.gameStats.mA_weaponType)->m4_name.c_str());
        }
        vdp2DebugPrintSetPosition(0x22, 0x35);
        vdp2PrintfLargeFont("%4d", getCurrentGunPower());
    }
    else {
        vdp2DebugPrintSetPosition(5, 0x35);
        if (mainGameState.gameStats.m1_dragonLevel < '\b') {
            vdp2PrintfLargeFont("LV %2d", mainGameState.gameStats.m0_level + 1);
        }
        else {
            drawLineLargeFont("FLTR ");
        }
        vdp2DebugPrintSetPosition(0xd, 0x35);
        vdp2PrintfLargeFont("HP %4d/%4d", mainGameState.gameStats.m10_currentHP, mainGameState.gameStats.mB8_maxHP);
        vdp2DebugPrintSetPosition(0x1c, 0x35);
        vdp2PrintfLargeFont("BP %3d/%3d", mainGameState.gameStats.m14_currentBP, mainGameState.gameStats.mBA_maxBP);
    }
}

void useInventoryItem(s_inventoryMenu* pThis)
{
    Unimplemented();
}

s32 inventoryMenuTaskDrawSub3(s32* pParam, int param_2, int param_3)
{
    int local_10;

    if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.mC_newButtonDown2 & 0xc0) == 0) {
        if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & 0xc0) == 0) {
            graphicEngineStatus.m40AC.m6 = 0;
        }
    }
    else {
        local_10 = *pParam;
        if ((((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.mC_newButtonDown2 & 0x40) != 0) && (local_10 = local_10 + -1, param_2 <= local_10)) ||
            (((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.mC_newButtonDown2 & 0x80) != 0 && (local_10 = local_10 + 1, local_10 <= param_3)))) {
            *pParam = local_10;
            playSystemSoundEffect(10);
            graphicEngineStatus.m40AC.m6 = 0;
            return 1;
        }
        if (graphicEngineStatus.m40AC.m6 == 0) {
            playSystemSoundEffect(5);
            graphicEngineStatus.m40AC.m6 = 1;
        }
    }
    return 0;
}

void inventoryPrintEntry(s32 param_1, eItems param_2)
{
    displayObjectIcon(0, 0xC, param_1 + 0x24, getObjectIcon(param_2));
    drawObjectName(getObjectListEntry(param_2)->m4_name.c_str());
    vdp2StringContext.m4_cursorX = vdp2StringContext.mC_X + 0x16;
    vdp2StringContext.m8_cursorY = vdp2StringContext.m18_Height;
    if (-1 < param_1) {
        vdp2StringContext.m8_cursorY = 0;
    }
    vdp2StringContext.m8_cursorY += vdp2StringContext.m10_Y + param_1;

    printVdp2String2("\x80\x81"); // X sign asa  wide 2x2 character

    int quantity;
    if (param_2 < 0x4D)
    {
        quantity = mainGameState.consumables[param_2] & 0xFF;
    }
    else
    {
        quantity = mainGameState.getBit(0xF3 + param_2);
    }

    printVdp2Number2(quantity & 0xFF, 2);
}

void inventoryPrintBerserk(s32, eItems, s32)
{
    Unimplemented();
}

void inventoryPrintWeapon(s32 param_1, eItems param_2, s32 param_3)
{
    if (param_2 == mainGameState.gameStats.mA_weaponType)
    {
        vdp2StringContext.m0 = 1;
    }
    else
    {
        vdp2StringContext.m0 = 0;
    }
    inventoryPrintEntry(param_1, param_2);
}

void inventoryPrintItem(s32 param_1, eItems param_2, s32 param_3)
{
    vdp2StringContext.m0 = 9;
    if ((param_3 & getObjectListEntry(param_2)->m0_flags) != 0) {
        if ((char)mainGameState.gameStats.m1_dragonLevel < '\b') {
            vdp2StringContext.m0 = 0;
        }
        else {
            if ((((param_2 != 6) && (param_2 != 0x29)) && (param_2 != 0x2a)) && (param_2 != 0x2b)) {
                vdp2StringContext.m0 = 0;
            }
        }
    }

    inventoryPrintEntry(param_1, param_2);
}

void inventoryMenuTaskDrawSub1(s_inventoryMenu* pThis)
{
    void (*printFuntion)(s32, eItems, s32) = nullptr;
    switch (pThis->m1C_selectedCategory)
    {
    case 2:
        printFuntion = inventoryPrintBerserk;
        break;
    case 3:
        printFuntion = inventoryPrintWeapon;
        break;
    default:
        printFuntion = inventoryPrintItem;
        break;
    }

    setupVDP2StringRendering(10, 0x24, 0x1e, 0xe);
    clearVdp2TextArea();
    for (int i=0; i<5; i++)
    {
        if (i < pThis->m30_numItemsInCurrentCategory) {
            vdp2StringContext.m4_cursorX = vdp2StringContext.mC_X + 4;
            vdp2StringContext.m8_cursorY = vdp2StringContext.m18_Height;
            if (-1 < i) {
                vdp2StringContext.m8_cursorY = 0;
            }
            vdp2StringContext.m8_cursorY += vdp2StringContext.m10_Y + i * 3;
            printFuntion(i*3, pThis->m34_itemsInCurrentCategory[5 * pThis->m4_currentPageInItemList + i], pThis->m10);
        }

    }
}

void inventoryMenuTaskDrawSub2(s_inventoryMenu* pThis, s32 param_2)
{
    Unimplemented();
}

void inventoryMenuTaskDrawSub0(s_inventoryMenu* pThis, s32 categoryId)
{
    std::array<eItems, 100>::iterator psVar4 = pThis->m34_itemsInCurrentCategory.begin();
    std::array<eItems, 100>::iterator psVar4End = pThis->m34_itemsInCurrentCategory.end();
    const s16* currentCategoryItemList = inventoryCatergoryTables[categoryId];

    switch (categoryId)
    {
    case 2: // berserks
        // TODO: Missing implementation for berserk skills category
        // Original game likely had special handling for dragon berserk abilities
        Unimplemented();
        break;
    default:
        while (true)
        {
            int iVar1 = *(currentCategoryItemList++);
            if (iVar1 < 0) {
                break;
            }
            // Bounds check to prevent array overflow
            if (psVar4 >= psVar4End) {
                break;
            }
            int itemCount;
            if (iVar1 < 0x4D) {
                itemCount = mainGameState.consumables[iVar1];
            }
            else {
                itemCount = mainGameState.getBit(0xF3 + iVar1);
            }

            if (itemCount)
            {
                *psVar4++ = (eItems)iVar1;
            }
        }
    }

    pThis->m30_numItemsInCurrentCategory = psVar4 - pThis->m34_itemsInCurrentCategory.begin();
}

struct sIconDef
{
    u16 m0_CMDSRCA;
    u16 m2_CMDSIZE;
    u16 m4_width;
    u16 m6_height;
    u16 m8_command;
};

void drawMenuIcon(const sIconDef* pDef, s16 X, s16 Y, u16 color)
{
    s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
    vdp1WriteEA.m0_CMDCTRL = pDef->m8_command;
    vdp1WriteEA.m4_CMDPMOD = 0x80;
    vdp1WriteEA.m6_CMDCOLR = color;
    vdp1WriteEA.m8_CMDSRCA = pDef->m0_CMDSRCA;
    vdp1WriteEA.mA_CMDSIZE = pDef->m2_CMDSIZE;
    vdp1WriteEA.mC_CMDXA = pDef->m4_width + X - 0xB0;
    vdp1WriteEA.mE_CMDYA = pDef->m6_height + Y - 0x70;

    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

    graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
    graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
    graphicEngineStatus.m14_vdp1Context[0].mC += 1;
}

void inventoryMenuTaskDraw(s_inventoryMenu* pThis)
{
    switch (pThis->m0)
    {
    case 0:
        if (pThis->m8--)
        {
            return;
        }
        displayInventoryCategoryDescription(pThis);
        pThis->m0++;
        break;
    case 1:
        drawInventoryStatusOrWeaponStats(pThis);
        if (pThis->m4_currentPageInItemList < 0)
        {
            // currently manipulating the category list
            if (!(graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 1))
            {
                if (readKeyboardToggle(0xC3))
                {
                    assert(0);
                }
                if (pThis->m1C_selectedCategory > -1)
                {
                    if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 0x86) == 0)
                    {
                        if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 0x30) != 0) {
                            playSystemSoundEffect(10);
                            if (pThis->m2C_hasCategoryAtLeastOneEntry[pThis->m1C_selectedCategory] == 1) {
                                switchVdp2MenuTileToHightlighted(inventoryMenuSpriteTable[pThis->m1C_selectedCategory], 2, 2, 0x660);
                            }

                            int uVar7;
                            if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 0x10) == 0) {
                                uVar7 = pThis->m1C_selectedCategory + 1;
                                if (3 < uVar7) {
                                    uVar7 = 0;
                                }
                            }
                            else {
                                uVar7 = pThis->m1C_selectedCategory - 1;
                                if ((int)uVar7 < 0) {
                                    uVar7 = 3;
                                }
                            }

                            if (pThis->m2C_hasCategoryAtLeastOneEntry[pThis->m1C_selectedCategory] == 1) {
                                switchVdp2MenuTileToHightlighted(inventoryMenuSpriteTable[uVar7], 2, 2, 0x620);
                            }

                            pThis->m1C_selectedCategory = uVar7;
                            pThis->m20_menuCursorCategory->m0_selectedEntry = uVar7;
                            setupVDP2StringRendering(3, 0x38, 0x26, 4);
                            clearVdp2TextArea();
                            if (-1 < pThis->m1C_selectedCategory) {
                                vdp2StringContext.m0 = 0;
                                drawObjectName(inventoryMenuText[pThis->m1C_selectedCategory]);
                            }
                        }
                    }
                    else
                    {
                        if (pThis->m2C_hasCategoryAtLeastOneEntry[pThis->m1C_selectedCategory])
                        {
                            playSystemSoundEffect(0);
                            pThis->m4_currentPageInItemList = 0;
                            pThis->m20_menuCursorCategory->m0_selectedEntry = -1;
                            pThis->m24_currentSelectedItemInCurrentItemPage = 0;
                            pThis->m28_menuCursorItem->m0 = 0;
                            inventoryMenuTaskDrawSub0(pThis, pThis->m1C_selectedCategory);
                            inventoryMenuTaskDrawSub1(pThis);
                            inventoryMenuTaskDrawSub2(pThis, pThis->m34_itemsInCurrentCategory[0]);
                        }
                        else
                        {
                            playSystemSoundEffect(5);
                        }
                        
                    }
                }
            }
            else
            {
                pThis->m0++;
            }
        }
        else
        {
            // manipulating the item list inside current category
            if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 1) ||
                ((pThis->m4_currentPageInItemList == 0 && ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.mC_newButtonDown2 & 0x40) != 0))))
            {
                // go back to category menu
                playSystemSoundEffect(1);
                displayInventoryCategoryDescription(pThis);
            }
            else
            if (!(((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 6) != 0) && (useInventoryItem(pThis), pThis->m4_currentPageInItemList < 0)))
            {
                int iVar2 = inventoryMenuTaskDrawSub3(&pThis->m4_currentPageInItemList, 0, performDivision(5, pThis->m30_numItemsInCurrentCategory - 1));
                if (iVar2)
                {
                    inventoryMenuTaskDrawSub1(pThis);
                }

                int iVar1 = pThis->m30_numItemsInCurrentCategory + pThis->m4_currentPageInItemList * -5 + -1;
                if (4 < iVar1) {
                    iVar1 = 4;
                }
                if (iVar1 < pThis->m24_currentSelectedItemInCurrentItemPage) {
                    pThis->m24_currentSelectedItemInCurrentItemPage = iVar1;
                }

                if ((loadDrawSub1Sub0(&pThis->m24_currentSelectedItemInCurrentItemPage, 0, iVar1) != 0) || (iVar2 != 0)) {
                    inventoryMenuTaskDrawSub2(pThis, pThis->m34_itemsInCurrentCategory[5 * pThis->m4_currentPageInItemList + pThis->m24_currentSelectedItemInCurrentItemPage]);
                }

                pThis->m28_menuCursorItem->m0 = pThis->m24_currentSelectedItemInCurrentItemPage;

                // draw the left/right icons for page change
                int maxPages = performDivision(5, pThis->m30_numItemsInCurrentCategory - 1);
                if (pThis->m4_currentPageInItemList > 0)
                {
                    const sIconDef* pIconDef = nullptr;

                    if (!(VDP2Regs_.m4_TVSTAT & 1))
                    {
                        static const sIconDef icon = {
                            0x20D0,
                            0x108,
                            0x48,
                            0x40,
                            0x0,
                        };
                        pIconDef = &icon;
                    }
                    else
                    {
                        static const sIconDef icon = {
                            0x20D0,
                            0x108,
                            0x48,
                            0x50,
                            0x0,
                        };
                        pIconDef = &icon;
                    }

                    drawMenuIcon(pIconDef, 0, 0, 0x760);
                }
                if (pThis->m4_currentPageInItemList < maxPages)
                {
                    const sIconDef* pIconDef = nullptr;

                    if (!(VDP2Regs_.m4_TVSTAT & 1))
                    {
                        static const sIconDef icon = {
                            0x20D0,
                            0x108,
                            0x140,
                            0x40,
                            0x10,
                        };
                        pIconDef = &icon;
                    }
                    else
                    {
                        static const sIconDef icon = {
                            0x20D0,
                            0x108,
                            0x140,
                            0x50,
                            0x10,
                        };
                        pIconDef = &icon;
                    }

                    drawMenuIcon(pIconDef, 0, 0, 0x760);
                }
            }
        }
        break;
    case 2: // close menu
        playSystemSoundEffect(1);
        setupVDP2StringRendering(0, 0x22, 0x2c, 0x1c);
        clearVdp2TextArea();
        pThis->m20_menuCursorCategory->m0_selectedEntry = -1;
        pThis->m28_menuCursorItem->m0 = -1;
        pThis->m8 = 0x10;
        startVdp2LayerScroll(0, 10, 0, 0x10);
        startVdp2LayerScroll(1, 0, 0x10, 0x10);
        pThis->m0++;
        [[fallthrough]];
    case 3: // wait for menu close
        if (--pThis->m8 == 0)
        {
            pThis->getTask()->markFinished();
        }
        break;
    default:
        assert(0);
    }
}

void inventoryMenuTaskDelete(s_inventoryMenu* pThis)
{
    Unimplemented();
}

p_workArea createInventoryMenuTask(p_workArea parent)
{
    static const s_inventoryMenu::TypedTaskDefinition definition = {
        inventoryMenuTaskInit,
        nullptr,
        inventoryMenuTaskDraw,
        inventoryMenuTaskDelete,
    };

    return createSubTask<s_inventoryMenu>(parent, &definition);
}
