#include "PDS.h"
#include "fieldItemBox.h"
#include "fieldItemBox_type1.h"
#include "fieldVisibilityGrid.h"
#include "kernel/animation.h"
#include "kernel/fileBundle.h"

void LCSItemBox_Callback1(p_workArea r4, sLCSTarget*)
{
    s_itemBoxType1* pThis = (s_itemBoxType1*)r4;
    pThis->mEA_wasRendered++;
}

void LCSItemBox_DrawType1(s_itemBoxType1* pThis)
{
    s_visibilityGridWorkArea* pGridTask = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;

    pushCurrentMatrix();
    translateCurrentMatrix(&pThis->m3C_pos);
    rotateCurrentMatrixZYX(&pThis->m6C_rotation);
    scaleCurrentMatrixRow0(pThis->m78_scale);
    scaleCurrentMatrixRow1(pThis->m78_scale);
    scaleCurrentMatrixRow2(pThis->m78_scale);

    pushCurrentMatrix();

    sStaticPoseData* pPose = pThis->m0.m0_mainMemoryBundle->getStaticPose(0x148, 2);

    sVec3_FP translation = pPose->m0_bones[0].m0_translation;
    sVec3_FP rotation = pPose->m0_bones[0].mC_rotation;

    translateCurrentMatrix(&translation);
    rotateCurrentMatrixZYX(&rotation);

    u32 depthRangeIndex = gridCellDraw_GetDepthRange(pCurrentMatrix->matrix[11]);

    if (depthRangeIndex <= pGridTask->m1300)
    {
        addObjectToDrawList(pThis->m0.m0_mainMemoryBundle->get3DModel(0x5C));

        pushCurrentMatrix();

        sVec3_FP translation2 = pPose->m0_bones[1].m0_translation;
        sVec3_FP rotation2 = pPose->m0_bones[1].mC_rotation;

        translateCurrentMatrix(&translation2);
        rotateCurrentMatrixShiftedZ(rotation2[2]);
        rotateCurrentMatrixShiftedY(rotation2[1] + pThis->m90);
        rotateCurrentMatrixShiftedX(rotation2[0]);

        addObjectToDrawList(pThis->m0.m0_mainMemoryBundle->get3DModel(0x60));

        popMatrix();
    }

    popMatrix();
    popMatrix();
}

void LCSItemBox_UpdateType1(s_itemBoxType1* pThis)
{
    switch (pThis->mEA_wasRendered)
    {
    case 0:
        if (LCSItemBox_shouldSpin(pThis))
        {
            pThis->m90 = (pThis->m94 + pThis->m90) & 0xFFFFFFF;
            pThis->m20 = 0;
        }
        else
        {
            pThis->m20 |= 2;
        }
        break;
    case 1:
        if (((pThis->m90 + pThis->m94) & 0xFFFFFFF) > pThis->m90)
        {
            pThis->m90 = 0;
            pThis->mEA_wasRendered++;
        }
        else
        {
            pThis->m90 = ((pThis->m90 + pThis->m94) & 0xFFFFFFF);
        }
    case 2:
    default:
        break;
    }

    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->m3C_pos);
        rotateCurrentMatrixZYX(&pThis->m6C_rotation);
        scaleCurrentMatrixRow0(pThis->m78_scale);
        scaleCurrentMatrixRow1(pThis->m78_scale);
        scaleCurrentMatrixRow2(pThis->m78_scale);

        transformAndAddVecByCurrentMatrix(&LCSItemBox_Table6[pThis->m8B_LCSType], &pThis->m60);

        sStaticPoseData* pPose = pThis->m0.m0_mainMemoryBundle->getStaticPose(0x190, 2);

        {
            pushCurrentMatrix();
            translateCurrentMatrix(pPose->m0_bones[0].m0_translation);
            rotateCurrentMatrixZYX(pPose->m0_bones[0].mC_rotation);

            gridCellDraw_normalSub2(pThis, 0x138, pThis->m7C);

            {
                pushCurrentMatrix();
                translateCurrentMatrix(pPose->m0_bones[1].m0_translation);
                rotateCurrentMatrixShiftedZ(pPose->m0_bones[1].mC_rotation[2]);
                rotateCurrentMatrixShiftedY(pPose->m0_bones[1].mC_rotation[1] + pThis->m90);
                rotateCurrentMatrixShiftedX(pPose->m0_bones[1].mC_rotation[0]);

                gridCellDraw_normalSub2(pThis, 0x13C, pThis->m7C);

                popMatrix();
            }
            popMatrix();
        }
        popMatrix();
    }

    if ((pThis->m88_receivedItemId == 0) || mainGameState.getBit(pThis->m88_receivedItemId + 243))
    {
        pThis->m20 |= 1;
    }

    updateLCSTarget(&pThis->m8_LCSTarget);
}
