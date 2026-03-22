#include "PDS.h"
#include "battleTrail.h"
#include "battleManager.h"
#include "battleEngine.h"
#include "kernel/graphicalObject.h"
#include "kernel/rayDisplay.h"

// 060a0b48
void trailRenderer_init(sTrailRenderer* pTrail, p_workArea task, sVec3_FP* initialPosition, u16 vdp1Base, sSaturnPtr renderData)
{
    pTrail->m14_vdp1Base = vdp1Base;
    pTrail->m10_renderData = renderData;
    pTrail->m4_numPositions = readSaturnS32(renderData + 0x14);
    pTrail->m0_positions = new sVec3_FP[pTrail->m4_numPositions]; // TODO: should use allocateHeapForTask

    for (s32 i = 0; i < pTrail->m4_numPositions; i++)
    {
        pTrail->m0_positions[i] = *initialPosition;
    }

    pTrail->m8_visibleCount = 0;
    pTrail->mC_growthRate = 0;
}

// 060a0be8
void trailRenderer_update(sTrailRenderer* pTrail, sVec3_FP* currentPosition, sVec3_FP* cameraMovement)
{
    // Shift positions: each position takes the previous one's value + camera movement
    for (s32 i = pTrail->m4_numPositions - 1; i > pTrail->m8_visibleCount; i--)
    {
        pTrail->m0_positions[i] = pTrail->m0_positions[i - 1] + *cameraMovement;
    }

    // Set the newest position
    pTrail->m0_positions[0] = *currentPosition;

    // Grow visible count
    if (pTrail->m8_visibleCount <= pTrail->m4_numPositions - 1)
    {
        pTrail->m8_visibleCount += pTrail->mC_growthRate;
    }
}

// 060a0eaa
void trailRenderer_draw(sTrailRenderer* pTrail)
{
    if (!pTrail->m0_positions || pTrail->m4_numPositions < 2) return;

    // Read sprite data from the render data
    u16 characterAddress = pTrail->m14_vdp1Base + readSaturnU16(pTrail->m10_renderData + 4);
    s16 characterSize = readSaturnS16(pTrail->m10_renderData + 6);
    u16 characterColor = pTrail->m14_vdp1Base + readSaturnU16(pTrail->m10_renderData + 8);

    // Gouraud colors at offset 0x18 in render data
    quadColor trailGouraud;
    trailGouraud[0] = readSaturnU16(pTrail->m10_renderData + 0x18);
    trailGouraud[1] = readSaturnU16(pTrail->m10_renderData + 0x1A);
    trailGouraud[2] = readSaturnU16(pTrail->m10_renderData + 0x1C);
    trailGouraud[3] = readSaturnU16(pTrail->m10_renderData + 0x1E);

    // Width tapers from thick (oldest) to thin (newest)
    s32 baseWidth = 0x800;

    for (s32 i = pTrail->m4_numPositions - 1; i > pTrail->m8_visibleCount; i--)
    {
        std::array<sVec3_FP, 2> line;
        line[0] = pTrail->m0_positions[i];
        line[1] = pTrail->m0_positions[i - 1];

        s32 width = baseWidth * i / pTrail->m4_numPositions;
        if (width < 0x100) width = 0x100;

        displayRaySegment(line, width, characterAddress, characterSize, characterColor, &trailGouraud, 8);
    }
}
