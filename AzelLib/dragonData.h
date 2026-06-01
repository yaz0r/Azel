#pragma once

extern s_fileBundle* gDragonModel;
void loadDragon(s_workArea* pWorkArea);
void loadDragonDataFromCommon();

// 0600c324 — interpolate dragon stats/color from the level stat table by cursor X/Y
void updateDragonStats(int type, sVec3_FP* pOutput);
