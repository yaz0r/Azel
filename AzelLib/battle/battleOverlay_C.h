#pragma once

struct sBattleOverlayTask_C : public s_workAreaTemplate<sBattleOverlayTask_C>
{
    std::array<struct s_battleEngineSub*, 0x80> m0;
    s32 m200;
    s32 m204;

    //size 0x210
};

void battleEngine_UpdateSub2(p_workArea);
