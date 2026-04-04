#pragma once

struct sLaserData;

struct sHomingLaserTrailData
{
    std::vector<sVec3_FP> m0_laserNodePosition;
    s32 m4_numLaserNodes;
    s32 m8;
    s32 mC;
    const sLaserData* m10_laserData;
    u16 m14;
    std::vector<quadColor> m18_color;
};

void sHomingLaserTask_InitSub0(sHomingLaserTrailData* pThis, p_workArea param_2, sVec3_FP* param_3, u16 param_4, const sLaserData* pLaserData);
void sHomingLaserTask_DrawSub1(sHomingLaserTrailData* pThis);

// BTL_A3::0609dc3a — shift trail segments backward, insert new head position
void sHomingLaserTask_shiftTrailSegments(sHomingLaserTrailData* pTrail, sVec3_FP* pNewPos, sVec3_FP* pAutoScroll);

void battleEngine_createHomingLaserRootTask(s_workAreaCopy* pThis, s32 param_2);
void updateDragonStats(int type, sVec3_FP* pOutput);
