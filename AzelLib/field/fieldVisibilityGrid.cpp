#include "PDS.h"
#include "fieldVisibilityGrid.h"
#include "field/fieldCamera.h"

#ifdef PDS_TOOL
extern bool bMakeEverythingVisible;
#endif

u32 gridCellDraw_GetDepthRange(fixedPoint r4)
{
    s_visibilityGridWorkArea* r5 = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;
    u32 rangeIndex = 0;

    while (r4 > (*r5->m2C_depthRangeTable)[rangeIndex])
    {
        rangeIndex++;
    }

    return rangeIndex;
}

// 0606fbe8
s32 checkPositionVisibility(const sVec3_FP* r4, s32 r5)
{
#ifdef PDS_TOOL
    if (bMakeEverythingVisible)
    {
        return 0;
    }
#endif
    s_visibilityGridWorkArea* r13 = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;
    sVec3_FP var18 = cameraProperties2.m0_position;

    {
        fixedPoint r3 = (*r4)[0] - var18[0];
        if (r3 < 0)
        {
            r3 = var18[0] - (*r4)[0];
        }
        if (r3 > r5)
            return 1;
    }

    {
        fixedPoint r2 = (*r4)[1] - var18[1];
        if (r2 < 0)
        {
            r2 = var18[1] - (*r4)[1];
        }
        if (r2 > r5)
            return 1;
    }

    {
        fixedPoint r2 = (*r4)[2] - var18[2];
        if (r2 < 0)
        {
            r2 = var18[2] - (*r4)[2];
        }
        if (r2 > r5)
            return 1;
    }

    {
        sMatrix4x3* r5 = getFieldCameraMatrix();
        sVec3_FP varC;
        varC[0] = r5->m[0][2];
        varC[1] = r5->m[1][2];
        varC[2] = r5->m[2][2];

        var18[0] -= varC[0] * 32;
        var18[1] -= varC[1] * 32;
        var18[2] -= varC[2] * 32;

        sVec3_FP var0;

        var0[0] = (*r4)[0] - var18[0];
        var0[1] = (*r4)[1] - var18[1];
        var0[2] = (*r4)[2] - var18[2];

        if (dot3_FP(&r13->m12AC, &var0) <= 0)
            return 1;
        if (dot3_FP(&r13->m12B8, &var0) <= 0)
            return 1;
        if (dot3_FP(&r13->m12C4, &var0) <= 0)
            return 1;
        if (dot3_FP(&r13->m12D0, &var0) <= 0)
            return 1;

        return 0;
    }

    return 1;
}

// 0606fd28
s32 checkPositionVisibilityAgainstFarPlane(sVec3_FP* r4)
{
    return checkPositionVisibility(r4, graphicEngineStatus.m405C.m14_farClipDistance);
}
