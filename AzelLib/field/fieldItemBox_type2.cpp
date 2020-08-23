#include "PDS.h"
#include "fieldItemBox.h"
#include "fieldItemBox_type2.h"
#include "fieldVisibilityGrid.h"

p_workArea createSavePointParticles()
{
    FunctionUnimplemented();
    return nullptr;
}

void LCSItemBox_UpdateType2(s_itemBoxType1* pThis)
{
    if (LCSItemBox_shouldSpin(pThis))
    {
        pThis->m20 = 0;
    }
    else
    {
        pThis->m20 |= 2;
    }

    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->m3C_pos);
        rotateCurrentMatrixZYX(&pThis->m6C_rotation);
        scaleCurrentMatrixRow0(pThis->m78_scale);
        scaleCurrentMatrixRow1(pThis->m78_scale);
        scaleCurrentMatrixRow2(pThis->m78_scale);

        transformAndAddVecByCurrentMatrix(&LCSItemBox_Table6[pThis->m8B_LCSType], &pThis->m60);
        LCSItemBox_UpdateType0Sub0(pThis, 0x54, 0x198, 0x7C);
        popMatrix();
    }

    if (pThis->m8D)
    {
        pThis->m20 |= 1;
    }

    updateLCSTarget(&pThis->m8_LCSTarget);
}

void LCSItemBox_DrawType2(s_itemBoxType1* pThis)
{
    s_visibilityGridWorkArea* pGridTask = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;

    pushCurrentMatrix();
    translateCurrentMatrix(&pThis->m3C_pos);
    rotateCurrentMatrixZYX(&pThis->m6C_rotation);
    scaleCurrentMatrixRow0(pThis->m78_scale);
    scaleCurrentMatrixRow1(pThis->m78_scale);
    scaleCurrentMatrixRow2(pThis->m78_scale);

    u32 depthRangeIndex = gridCellDraw_GetDepthRange(pCurrentMatrix->matrix[11]);

    if (depthRangeIndex <= pGridTask->m1300)
    {
        LCSItemBox_DrawType0Sub0(pThis->m0.m0_mainMemoryBundle, 8, 0x14C);
        pThis->mEA_wasRendered = 1;
    }
    else
    {
        pThis->mEA_wasRendered = 0;
    }

    popMatrix();
}
