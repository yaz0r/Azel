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
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { NULL, NULL, NULL, NULL};
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
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { NULL, NULL, NULL, NULL};
        return &taskDefinition;
    }

    u8* m8_dramAllocation; //0
    u8* m4_vramAllocation;//4
    u8* m8_MCBInDram;//8
};

s_loadDragonWorkArea* loadDragonModel(s_workArea* pWorkArea, e_dragonLevel dragonLevel);
void morphDragon(s_loadDragonWorkArea* pLoadDragonWorkArea, s_3dModel* pDragonStateSubData1, u8* pMCB, const sDragonData3* pDragonData3, s16 cursorX, s16 cursorY);

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

bool init3DModelRawData(s_workArea* pWorkArea, s_3dModel* pDragonStateData1, u32 unkArg0, u8* pDragonModel, u16 modelIndexOffset, u8* pModelData1, u8* pDefaultPose, u8* unkArg2, const s_RiderDefinitionSub* unkArg3);
void initModelDrawFunction(s_3dModel* pDragonStateData1);
u32 createDragonStateSubData1Sub1(s_3dModel* pDragonStateData1, u8* pModelData1);
s32 riderInit(s_3dModel* r4, u8* r5);
void playAnimationGeneric(s_3dModel* r4, u8* r5, s32 r6);
void setNextGameStatus(u32 r4);
u32 performModulo(u32 r0, u32 r1);
void resetTempAllocators();
u16 loadFnt(const char* filename);

struct s_fileEntry
{
    std::string mFileName;
    s32 m0_fileID; // was the file index in the CD, but useless here
    s32 m4_fileSize;
    s32 m8_refcount;
    struct npcFileDeleter* mC_buffer;
    // size 0x10
};

extern std::vector<s_fileEntry> dramAllocatorEnd;

u8* dramAllocate(u32 size);

struct sVdp1Allocation
{
    u8* m4_baseInVdp1Memory;
};
sVdp1Allocation* vdp1Allocate(u32 size);

struct s_exitMenuTaskSub1Task : public s_workAreaTemplateWithArg<s_exitMenuTaskSub1Task, s32>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &s_exitMenuTaskSub1Task::exitMenuTaskSub1TaskInit, &s_exitMenuTaskSub1Task::exitMenuTaskSub1TaskUpdate, &s_exitMenuTaskSub1Task::exitMenuTaskSub1TaskDraw, NULL};
        return &taskDefinition;
    }
    
    static void exitMenuTaskSub1TaskInit(s_exitMenuTaskSub1Task*, s32);
    static void exitMenuTaskSub1TaskUpdate(s_exitMenuTaskSub1Task*);
    static void exitMenuTaskSub1TaskDraw(s_exitMenuTaskSub1Task*);
    
    u32 state; // 0
    p_workArea m8;
    u32 mC;
};

extern s_exitMenuTaskSub1Task* gExitMenuTaskSub1Task;
