#pragma once

struct sLaserData
{
    s16 m4;
    s16 m6;
    s16 m8;
    s16 mA;
    s16 mC;
    s16 mE;
    s16 m10;
    s16 m12;
    s16 m14;
    std::vector<fixedPoint> m18_vertices;
    std::vector<quadColor> m1C_colors;
    s32 m20_numLaserNodes;
    // size?
};

struct battleOverlay : public sSaturnMemoryFile
{
    virtual sSaturnPtr getBattleEngineInitData() = 0;
    virtual void invoke(sSaturnPtr Func, p_workArea pParent) = 0;
    virtual void invoke(sSaturnPtr Func, p_workArea pParent, u32, u32) = 0;

    // Colors used for gun shots
    static const std::vector<quadColor> m60AE424;
    static const std::vector<quadColor> m60AE42C;
    static const std::vector<quadColor> m60AE434;
    static const std::vector<quadColor> m60AE43C;

    // laser data
    static sLaserData mLaserData;
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
