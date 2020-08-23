#include "PDS.h"
#include "fieldVisibilityGrid.h"

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

