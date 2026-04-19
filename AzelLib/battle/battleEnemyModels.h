#pragma once

s32 createBattleEnemyModels(s_workArea* pOwner, s_fileBundle* pBundle, u8& modelCount, std::vector<s_3dModel>& models, sSaturnPtr dataTable, u8 count);
void initBattleEnemyTargetables(s_workArea* pEnemy, u8 modelCount, std::vector<s_3dModel>& models);
