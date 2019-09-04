#include "PDS.h"
#include "menuCursor.h"
#include "menuSprite.h"

void s_MenuCursorWorkArea::menuCursorTaskInit(s_MenuCursorWorkArea* pThis, sMainMenuTaskInitData2* pMenuData)
{
    pThis->m4 = pMenuData->m0;
    pThis->m8 = pMenuData->m4;
}

void s_MenuCursorWorkArea::menuCursorTaskUpdate(s_MenuCursorWorkArea* pWorkArea)
{
    if (--pWorkArea->mC < 0)
    {
        pWorkArea->mC = 40;
    }
}

s_menuSprite cursorSpriteDef0 = { 0x2080, 0x520, 0, 0 };
s_menuSprite cursorSpriteDef1 = { 0x2030, 0x520, 0, 0 };

void s_MenuCursorWorkArea::menuCursorTaskDraw(s_MenuCursorWorkArea* pWorkArea)
{
    if (pWorkArea->m0_selectedEntry < 0)
        return;

    s32 X = pWorkArea->m8[pWorkArea->m0_selectedEntry * 2 + 0] - pWorkArea->m4->scrollX;
    s32 Y = pWorkArea->m8[pWorkArea->m0_selectedEntry * 2 + 1] - pWorkArea->m4->scrollY;

    if (pWorkArea->mC > 20)
    {
        drawMenuSprite(&cursorSpriteDef0, X, Y, 0x760);
    }

    drawMenuSprite(&cursorSpriteDef1, X, Y, 0x760);
}

s_MenuCursorWorkArea* createMenuCursorTask(p_workArea pWorkArea, sMainMenuTaskInitData2* r5)
{
    return createSubTaskWithArg<s_MenuCursorWorkArea, sMainMenuTaskInitData2*>(pWorkArea, r5);
}

void s_MenuCursor2::Init(s_MenuCursor2* pThis, std::vector<std::array<s16, 2>>* config)
{
    pThis->m4 = config;
    pThis->mC = 2;
}

void s_MenuCursor2::Draw(s_MenuCursor2* pThis)
{
    if (pThis->m0 < 0)
        return;

    s32 iVar1 = pThis->mC - vblankData.m14;
    s32 uVar3 = pThis->m8;
    if (iVar1 < 1)
    {
        uVar3++;
        iVar1 += 2;
        if (uVar3 > 7)
        {
            uVar3 = 0;
        }
        pThis->m8 = uVar3;
    }
    pThis->mC = iVar1;

    static const std::vector<s_menuSprite> spriteDef = { {
        {0x2000, 0x10B, 0, 3},
        {0x2008, 0x10A, 0, 4},
        {0x2010, 0x108, 0, 5},
        {0x2014, 0x107, 0, 6},
        {0x2018, 0x106, 0, 7},
        {0x201C, 0x108, 0, 6},
        {0x2020, 0x10A, 0, 4},
        {0x2028, 0x10B, 0, 3},
    } };

    drawMenuSprite(&spriteDef[uVar3], (*pThis->m4)[pThis->m0][0], (*pThis->m4)[pThis->m0][1], 0x7F0);
}

s_MenuCursor2* createMenuCursorTask2(p_workArea pWorkArea, std::vector<std::array<s16, 2>>* r5)
{
    return createSubTaskWithArg<s_MenuCursor2, std::vector<std::array<s16, 2>>*>(pWorkArea, r5);
}
