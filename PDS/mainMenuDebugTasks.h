#pragma once

p_workArea createTownDebugTask(p_workArea);
p_workArea createFieldDebugTask(p_workArea);
p_workArea createBattleDebugTask(p_workArea);
p_workArea createSoundDebugTask(p_workArea);
p_workArea createMovieDebugTask(p_workArea);

p_workArea createNewGameTask(p_workArea pWorkArea);
p_workArea createContinueTask(p_workArea pWorkArea);

u32 dragonFieldTaskInitSub3Sub1(s_3dModel* r4, u8* r5);
u32 updateAndInterpolateAnimation(s_3dModel* r4);

p_workArea createMenuTask(p_workArea parentTask);
void stopAllSounds();

extern p_workArea fieldTaskVar0;
extern u32 fieldTaskVar2;

struct s_loadRiderWorkArea : public s_workAreaTemplate<s_loadRiderWorkArea>
{
    static s_taskDefinitionWithArg* getTaskDefinition()
    {
        static s_taskDefinitionWithArg taskDefinition = { NULL, NULL, NULL, NULL, "s_loadRiderWorkArea" };
        return &taskDefinition;
    }
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { NULL, NULL, NULL, NULL, "s_loadRiderWorkArea" };
        return &taskDefinition;
    }

    u8* m0_riderModel; //0
    u32 m4; //4
    s_workArea* m_ParentWorkArea; //8
    u32 m_riderType; //C
    u32 m_modelIndex; // 10
    u32 m_14; //14
    s_3dModel m18_3dModel;//18
};

extern s_loadRiderWorkArea* pRider1State;
extern s_loadRiderWorkArea* pRider2State;

struct s_loadDragonWorkArea : public s_workAreaTemplate<s_loadDragonWorkArea>
{
    static s_taskDefinitionWithArg* getTaskDefinition()
    {
        static s_taskDefinitionWithArg taskDefinition = { NULL, NULL, NULL, NULL, "s_loadDragonWorkArea" };
        return &taskDefinition;
    }
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { NULL, NULL, NULL, NULL, "s_loadDragonWorkArea" };
        return &taskDefinition;
    }

    u8* dramAllocation; //0
    u8* vramAllocation;//4
    u16 MCBOffsetInDram;//8
    u16 CGBOffsetInDram;//A
};

s_loadDragonWorkArea* loadDragonModel(s_workArea* pWorkArea, e_dragonLevel dragonLevel);
void morphDragon(s_loadDragonWorkArea* pLoadDragonWorkArea, s_3dModel* pDragonStateSubData1, u32 unk0, const sDragonData3* pDragonData3, s16 cursorX, s16 cursorY);

struct s_vblankData {
    u32 m0;
    u32 m4;
    u32 m8;
    u32 mC;
    u32 m10;
    u32 m14;
    u32 m18;
    u32 m1C;
};
extern s_vblankData vblankData;

extern u8 gDragonModel[0x16500];

void initModelDrawFunction(s_3dModel* pDragonStateData1);
u32 createDragonStateSubData1Sub1(s_3dModel* pDragonStateData1, u8* pModelData1);
s32 riderInit(s_3dModel* r4, u8* r5);
void playAnimationGeneric(s_3dModel* r4, u8* r5, s32 r6);
