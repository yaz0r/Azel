#pragma once

struct sBattleItemSelectionTask;

sBattleItemSelectionTask* createBattleItemSelectionTask(p_workArea parent, sBattleItemSelectionTask** previousMenu, s32*, s32, std::array<sBattleCommandMenu::sSubMenuEntry, 0x38>&);
