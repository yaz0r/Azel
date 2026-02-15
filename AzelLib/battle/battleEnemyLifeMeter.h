#pragma once

struct sEnemyLifeMeterTask : public s_workAreaTemplate<sEnemyLifeMeterTask>
{
    sVec3_FP* m0;
    sVec3_FP m4_lastSafeProjectedPosition;
    sVec3_FP m10;
    fixedPoint m1C;
    fixedPoint m20;
    s16* m24;
    s16 m28;
    s16 m2A;
    s16 m2C_entryIndex;
    s16 m2E_width;
    s8 m30;
    u8 m31;
    // size 0x34
};

sEnemyLifeMeterTask* createEnemyLifeMeterTask(sVec3_FP* arg0, s32 arg1, s16* arg2, s16 arg3);

