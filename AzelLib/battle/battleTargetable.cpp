#include "PDS.h"
#include "battleTargetable.h"
#include "kernel/debug/trace.h"

void battleTargetable_updatePosition(sBattleTargetable* pThis)
{
    transformAndAddVec(*pThis->m4_pPosition, pThis->m10_position, cameraProperties2.m28[0]);

    addTraceLog(*pThis->m4_pPosition, "targetablePositionSource");
    addTraceLog(pThis->m10_position, "targetablePosition");
}

sVec3_FP* getBattleTargetablePosition(sBattleTargetable& param1)
{
    return &param1.m10_position;
}

