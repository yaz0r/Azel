#pragma once
p_workArea initLoadSavegameScreen(p_workArea parent, s32);
p_workArea createLoadTask(p_workArea workArea);
p_workArea createSaveTask(p_workArea workArea);

void applyLayerDisplayConfig(const std::vector<std::array<s32, 2>>& r4); // todo: clean

