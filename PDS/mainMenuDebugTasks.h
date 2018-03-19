#pragma once

p_workArea createTownDebugTask(p_workArea);
p_workArea createFieldDebugTask(p_workArea);
p_workArea createBattleDebugTask(p_workArea);
p_workArea createSoundDebugTask(p_workArea);
p_workArea createMovieDebugTask(p_workArea);

p_workArea createNewGameTask(p_workArea pWorkArea);
p_workArea createContinueTask(p_workArea pWorkArea);

u32 dragonFieldTaskInitSub3Sub1(s_3dModel* r4, u8* r5);
u32 dragonFieldTaskInitSub3Sub2(s_3dModel* r4);

struct s_loadDragonWorkArea : public s_workArea
{
    u8* dramAllocation; //0
    u8* vramAllocation;//4
    u16 MCBOffsetInDram;//8
    u16 CGBOffsetInDram;//A
};

s_loadDragonWorkArea* loadDragonModel(s_workArea* pWorkArea, e_dragonLevel dragonLevel);

extern u32 vblankData[8];

extern u8 gDragonModel[0x16500];
