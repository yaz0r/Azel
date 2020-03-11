#pragma once

struct battleOverlay : public sSaturnMemoryFile
{
    virtual sSaturnPtr getBattleEngineInitData() = 0;
    virtual void invoke(sSaturnPtr Func, p_workArea pParent) = 0;
    virtual void invoke(sSaturnPtr Func, p_workArea pParent, u32, u32) = 0;
};

extern battleOverlay* gCurrentBattleOverlay;

struct sBattleOverlayTask : public s_workAreaTemplate<sBattleOverlayTask>
{
    s16 m0;
    s8 m2;
    s8 m3;
    struct s_battleEngine* m4_battleEngine;
    struct s_battleGrid* m8_gridTask;
    struct sBattleOverlayTask_C* mC_targetSystem;
    struct s_battleDebug* m10_inBattleDebug;
    void* m14;
    struct s_battleDragon* m18_dragon;
    p_workArea m1C_envTask;
    struct s_battleOverlay_20* m20_battleHud;
    s32 m24;
    //size 0x28
};

p_workArea createBattleOverlayTask(struct sBattleManager* pParent);
