#pragma once

struct sBattleItemSelectionTask : public s_workAreaTemplate<sBattleItemSelectionTask>
{
    s8 m0_state;
    s8 m1;
    s8 m2;
    s8 m3_currentPageIndex;
    s8 m4;
    s8 m5_currentSelectionInPage;
    s8 m6_numEntriesInPage;
    s8 m7;
    s8 m8;
    s32* mC_selectedEntry;
    sBattleItemSelectionTask** m10_previousMenu;
    s16 m14_listX;
    s16 m16_listY;
    s16 m1A_listWidth;
    s16 m1C_listHeight;
    s8 m20_previousListHeight;
    std::vector<sBattleCommandMenu::sSubMenuEntry>* m24;
    // size 0x2C
};

sBattleItemSelectionTask* createBattleItemSelectionTask(p_workArea parent, sBattleItemSelectionTask** previousMenu, s32*, s32, std::vector<sBattleCommandMenu::sSubMenuEntry>&);
