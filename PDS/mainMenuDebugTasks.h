#pragma once

p_workArea createTownDebugTask(p_workArea);
p_workArea createFieldDebugTask(p_workArea);
p_workArea createBattleDebugTask(p_workArea);
p_workArea createSoundDebugTask(p_workArea);
p_workArea createMovieDebugTask(p_workArea);

p_workArea createNewGameTask(p_workArea pWorkArea);
p_workArea createContinueTask(p_workArea pWorkArea);

u32 dragonFieldTaskInitSub3Sub1(s_dragonStateSubData1* r4, u8* r5);
u32 dragonFieldTaskInitSub3Sub2(s_dragonStateSubData1* r4);

extern u32 vblankData[8];
