#pragma once

struct battleOverlay : public sSaturnMemoryFile
{
    virtual sSaturnPtr getBattleEngineInitData() = 0;
    virtual void invoke(sSaturnPtr Func, struct s_battleDragon*, u32, u32) = 0;
};

extern battleOverlay* gCurrentBattleOverlay;

struct sBattleOverlayTask_C
{
    s32 m200;
    s32 m204;
};

struct sBattleOverlayTask : public s_workAreaTemplate<sBattleOverlayTask>
{
    s16 m0;
    s8 m2;
    s8 m3;
    struct s_battleEngine* m4_battleEngine;
    struct s_battleGrid* m8_gridTask;
    struct sBattleOverlayTask_C* mC;
    struct s_battleDebug* m10_inBattleDebug;
    struct s_battleDragon* m18_dragon;
    struct s_battleOverlay_20* m20;
    s32 m24;
    //size 0x28
};

p_workArea createBattleOverlayTask(struct sBattleManager* pParent);
