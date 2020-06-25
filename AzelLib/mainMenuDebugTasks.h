#pragma once

p_workArea createTownDebugTask(p_workArea);
p_workArea createFieldDebugTask(p_workArea);
p_workArea createSoundDebugTask(p_workArea);
p_workArea createMovieDebugTask(p_workArea);

p_workArea createNewGameTask(p_workArea pWorkArea);
p_workArea createContinueTask(p_workArea pWorkArea);

u32 updateAndInterpolateAnimation(s_3dModel* r4);

p_workArea createMenuTask(p_workArea parentTask);

extern p_workArea fieldTaskVar0;
extern u32 fieldTaskVar2;

struct s_loadRiderWorkArea : public s_workAreaTemplate<s_loadRiderWorkArea>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { NULL, NULL, NULL, NULL};
        return &taskDefinition;
    }

    struct s_fileBundle* m0_riderBundle;
    u32 m4; //4
    s_workArea* m_ParentWorkArea; //8
    u32 mC_riderType; //C
    u32 m10_modelIndex; // 10
    u32 m14_weaponModelIndex; //14
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
    u32 m0_frameReadyToPresent;
    u32 m4_wasFrameOutDisplayed;
    u32 m8;
    u32 mC_numFramesPresented;
    u32 m10_numVsyncSinceLastPresent;
    u32 m14_numVsyncPerFrame;
    u32 m18;
    u32 m1C_callback;
};
extern s_vblankData vblankData;

bool init3DModelRawData(s_workArea* pWorkArea, s_3dModel* pDragonStateData1, u32 unkArg0, s_fileBundle* pDragonBundle, u16 modelIndexOffset, struct sAnimationData* pAnimationData, struct sStaticPoseData* pDefaultPose, u8* colorAnim, sHotpointBundle* unkArg3);
void initModelDrawFunction(s_3dModel* pDragonStateData1);
u32 createDragonStateSubData1Sub1(s_3dModel* pDragonStateData1, struct sAnimationData* pAnimation);
s32 riderInit(s_3dModel* r4, struct sAnimationData* pAnimation);
void playAnimationGeneric(s_3dModel* pModel, struct sAnimationData* pAnimation, s32 interpolationLength);
s32 setNextGameStatus(s32 r4);
u32 performModulo(u32 r0, u32 r1);
u32 performModulo2(u32 r0, u32 r1);
void resetTempAllocators();
u16 loadFnt(const char* filename);
void clearVdp2Menu();

struct s_fileEntry
{
    std::string mFileName;
    s32 m0_fileID; // was the file index in the CD, but useless here
    s32 m4_fileSize;
    s32 m8_refcount;
    struct npcFileDeleter* mC_fileBundle;
    // size 0x10
};

extern std::vector<s_fileEntry> dramAllocatorEnd;

u8* dramAllocate(u32 size);
p_workArea createFieldTask(p_workArea pTypelessWorkArea, u32 arg);
void updateDragonStatsFromLevel();

void freeRamResource();
p_workArea loadField(p_workArea r4, s32 r5);
void setOpenMenu7();

void updateDragonIfCursorChanged(u32 level);
void loadRiderIfChanged(u32 rider);
void loadRider2IfChanged(u32 rider);
