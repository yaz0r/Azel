#pragma once

struct sBattleOverlayTask : public s_workAreaTemplate<sBattleOverlayTask>
{
    s16 m0;
    s8 m2;
    s8 m3;
    struct s_battleEngine* m4_battleEngine;
    struct s_battleGrid* m8_gridTask;
    p_workArea m18_dragon;
    s32 m24;
    //size 0x28
};

p_workArea createBattleOverlayTask(struct sBattleManager* pParent);
