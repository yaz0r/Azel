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

void menuGraphicsTaskDrawSub2Sub1(u16 r4);
p_workArea createMenuTask(p_workArea parentTask);

struct s_loadDragonWorkArea : public s_workArea
{
    u8* dramAllocation; //0
    u8* vramAllocation;//4
    u16 MCBOffsetInDram;//8
    u16 CGBOffsetInDram;//A
};

s_loadDragonWorkArea* loadDragonModel(s_workArea* pWorkArea, e_dragonLevel dragonLevel);
void morphDragon(s_loadDragonWorkArea* pLoadDragonWorkArea, s_3dModel* pDragonStateSubData1, u32 unk0, const sDragonData3* pDragonData3, s16 cursorX, s16 cursorY);

struct s_vblankData {
    u32 field_0;
    u32 field_4;
    u32 field_8;
    u32 field_C;
    u32 field_10;
    u32 field_14;
    u32 field_18;
    u32 field_1C;
};
extern s_vblankData vblankData;

extern u8 gDragonModel[0x16500];
