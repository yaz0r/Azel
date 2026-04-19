#pragma once

// Per-subBattle entry in the overlay battle data table
// Saturn layout: 8 pointers at 4 bytes each = 0x20 bytes per entry
// Indexed by subBattleId: overlayBattleData + subBattleId * 0x20
struct sOverlayBattleEntry
{
    sSaturnPtr m0_formationTable;       // +0x00: {funcPtr, arg, flags}[] terminated by funcPtr==0
    sSaturnPtr m4_positionData;         // +0x04: camera/position/speed data (0x5A bytes)
    sSaturnPtr m8_mapFunction;          // +0x08: dispatched via executeFuncPtr → invoke(func, parent)
    sSaturnPtr mC_textDisplayData;      // +0x0C: passed to createBattleTextDisplay
    sSaturnPtr m10;                     // +0x10:
    sSaturnPtr m14;                     // +0x14:
    sSaturnPtr m18;                     // +0x18:
    sSaturnPtr m1C_cameraData;          // +0x1C: berserk camera offsets

    static sOverlayBattleEntry read(sSaturnPtr src)
    {
        sOverlayBattleEntry e;
        e.m0_formationTable = readSaturnEA(src + 0x00);
        e.m4_positionData = readSaturnEA(src + 0x04);
        e.m8_mapFunction = readSaturnEA(src + 0x08);
        e.mC_textDisplayData = readSaturnEA(src + 0x0C);
        e.m10 = readSaturnEA(src + 0x10);
        e.m14 = readSaturnEA(src + 0x14);
        e.m18 = readSaturnEA(src + 0x18);
        e.m1C_cameraData = readSaturnEA(src + 0x1C);
        return e;
    }
    // Saturn size 0x20
};

struct battleOverlay : public sSaturnMemoryFile
{
    battleOverlay(const char* fileName) : sSaturnMemoryFile(fileName)
    {

    }

    virtual sSaturnPtr getEncounterDataTable() = 0;
    virtual void invoke(sSaturnPtr Func, s_workAreaCopy* pParent) = 0;
    virtual void invoke(sSaturnPtr Func, s_workAreaCopy* pParent, u32, u32) = 0;
    virtual p_workArea invokeCreateEffect(sSaturnPtr Func, s_workAreaCopy* pParent) = 0;
};

extern battleOverlay* gCurrentBattleOverlay;

struct sBattleOverlayTask : public s_workAreaTemplate<sBattleOverlayTask>
{
    s16 m0;
    s8 m2_numLoadedFnt;
    s8 m3;
    struct s_battleEngine* m4_battleEngine;
    struct s_battleGrid* m8_gridTask;
    struct sBattleOverlayTask_C* mC_targetSystem;
    struct s_battleDebug* m10_inBattleDebug;
    struct sBattleTextDisplayTask* m14_textDisplay;
    struct s_battleDragon* m18_dragon;
    struct s_BTL_A3_Env* m1C_envTask;
    struct s_battleOverlay_20* m20_battleHud;
    s32 m24;
    //size 0x28
};

p_workArea createBattleOverlayTask(struct sBattleManager* pParent);
p_workArea loadBattleOverlay(s32 battleId, s32 subBattleId);
