#pragma once

struct sBattleManager : public s_workAreaTemplateWithArg<sBattleManager, s8>
{
    s16 m0_status;
    s16 m2_currentBattleOverlayId;
    s16 m4; // D
    s16 m6_subBattleId;
    s16 m8; // E
    s16 mA_pendingBattleOverlayId;
    s8 mC;
    s8 mD;
    s8 mE;
    // usually battle overlay, can also be battle debug list
    union {
        p_workArea m10_subTask_debugList;
        struct sBattleOverlayTask* m10_battleOverlay;
    };

    //size m14
};

extern sBattleManager* gBattleManager;
sBattleManager* createBattleManager(p_workArea parentTask, s8 arg);
void startDebugBattle(s32 battleOverlayId);
void  loadBattleOverlaySub0(sBattleManager* pThis);
