#pragma once

struct sBattleOverlayTask_C : public s_workAreaTemplate<sBattleOverlayTask_C>
{
    std::array<struct s_battleEnemy*, 0x80> m0_enemyTargetables;
    s32 m200_cameraMinAltitude;
    s32 m204_cameraMaxAltitude;
    s16 m20A_numSelectableEnemies;
    //size 0x210
};

void battleEngine_UpdateSub2(p_workArea);
