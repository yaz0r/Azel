#pragma once

#include "battle/interpolators/FPInterpolator.h"

struct sBattleCommandMenu : public s_workAreaTemplate<sBattleCommandMenu>
{
    s8 m0_selectedBattleCommand;
    s8 m1_numBattleCommands;
    s8 m2_mode;
    s8 m3_itemMenuOpen;
    std::array<s8, 6> m4_enabledBattleCommands;
    s16 mC;
    s16 mE;
    s16 m14;
    s16 m16;
    s16 m18_oldDragonAtk;
    s16 m1A_oldDragonDef;
    s32 m1C;
    u32 m20;
    s32 m24;
    s32 m28;
    struct sSubMenuEntry
    {
        s16 m0_itemIndex;
        s16 m2;
    };
    std::array<sSubMenuEntry, 0x38> m34_itemList;
    std::array<sSubMenuEntry, 0x22> m114_berserkList;
    std::array<sSubMenuEntry, 0x9> m19C_weaponList;
    sFPInterpolator m1C0_scrollInterpolator;
    struct sBattleItemSelectionTask* m1DC_itemSelectionMenuHead;
    struct sBattleItemSelectionTask* m1E0_berserkSelectionMenuHead;
    struct sBattleItemSelectionTask* m1E4_weaponSelectionMenuHead;
    //size 0x1e8
};

void createBattleCommandMenu(p_workArea parent);
