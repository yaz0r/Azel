#pragma once

struct sTrailRenderer
{
    sVec3_FP* m0_positions;
    s32 m4_numPositions;
    s32 m8_visibleCount;
    s32 mC_growthRate;
    sSaturnPtr m10_renderData;
    u16 m14_vdp1Base;
};

void trailRenderer_init(sTrailRenderer* pTrail, p_workArea task, sVec3_FP* initialPosition, u16 vdp1Base, sSaturnPtr renderData);
void trailRenderer_update(sTrailRenderer* pTrail, sVec3_FP* currentPosition, sVec3_FP* cameraMovement);
void trailRenderer_draw(sTrailRenderer* pTrail);
