#pragma once

struct s_moduleManager : public s_workAreaTemplateWithArg<s_moduleManager, s32>
{
    u32 state; // 0
    p_workArea m8;
    u32 mC;
};

p_workArea createModuleManager(p_workArea pTypelessWorkArea, u32 menuID);

struct s_gameStatus {
    s8 m0_gameMode;
    s8 m1;
    s8 m2;
    s8 m3_loadingSaveFile;
    u16 m4_gameStatus;
    u16 m6_previousGameStatus;
    u16 m8_nextGameStatus;
};

extern s_gameStatus gGameStatus;

extern s_moduleManager* gModuleManager;
void initNewGameState();

void setupSaveParams(s32 fieldIndex, s32 subFieldIndex, s32 savepointIndex);
