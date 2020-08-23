#include "PDS.h"
#include "fieldItemBox.h"
#include "fieldItemBox_type0.h"
#include "kernel/animation.h"
#include "audio/systemSounds.h"
#include "fieldVisibilityGrid.h"

void LCSItemBox_UpdateType0(s_itemBoxType1* pThis)
{
    if (LCSItemBox_shouldSpin(pThis))
    {
        pThis->m6C_rotation[1] += fixedPoint(0x444444);
        pThis->m8_LCSTarget.m18_diableFlags = 0;
    }
    else
    {
        pThis->m8_LCSTarget.m18_diableFlags |= 2;
    }

    pushCurrentMatrix();
    translateCurrentMatrix(&pThis->m3C_pos);
    rotateCurrentMatrixZYX(&pThis->m6C_rotation);
    scaleCurrentMatrixRow0(pThis->m78_scale);
    scaleCurrentMatrixRow1(pThis->m78_scale);
    scaleCurrentMatrixRow2(pThis->m78_scale);

    transformAndAddVecByCurrentMatrix(&LCSItemBox_Table6[pThis->m8B_LCSType], &pThis->m60);

    LCSItemBox_UpdateType0Sub0(pThis, 0x58, 0x19C, pThis->m7C);

    popMatrix();

    if (pThis->m8D)
    {
        pThis->m8_LCSTarget.m18_diableFlags |= 1;
    }

    updateLCSTarget(&pThis->m8_LCSTarget);
}

static void LCSItemBox_OpenedBoxUpdate(s_itemBoxType1* pThis)
{
    switch (pThis->mEA_wasRendered)
    {
    case 0:
        pThis->m20 |= 1;
        pThis->mE8 = 0x15;
        pThis->mEA_wasRendered++;
        // fall
    case 1:
        stepAnimation(&pThis->m98_3dModel);
        pThis->mEA_wasRendered++;
        pThis->mE8--;
        if (pThis->mE8 <= 0)
        {
            pThis->mEA_wasRendered = 3;
        }
        return;
    case 2:
        pThis->mEA_wasRendered--;
        break;
    case 3:
        break;
    default:
        assert(0);
    }
}

void LCSItemBox_OpenedBoxDraw(s_itemBoxType1* pThis)
{
    pushCurrentMatrix();
    translateCurrentMatrix(&pThis->m3C_pos);
    rotateCurrentMatrixZYX(&pThis->m6C_rotation);
    scaleCurrentMatrixRow0(pThis->m78_scale);
    scaleCurrentMatrixRow1(pThis->m78_scale);
    scaleCurrentMatrixRow2(pThis->m78_scale);
    pThis->m98_3dModel.m18_drawFunction(&pThis->m98_3dModel);
    popMatrix();
}

void LCSItemBox_Callback0Sub0(s_itemBoxType1* pThis)
{
    PDS_unimplemented("LCSItemBox_Callback0Sub0");
}

void LCSItemBox_Callback0(p_workArea r4, sLCSTarget*)
{
    s_itemBoxType1* pThis = (s_itemBoxType1*)r4;
    if (pThis->m21 & 0x20)
        return;

    mainGameState.setBit566(pThis->m80_bitIndex);

    LCSItemBox_Callback0Sub0(pThis);
    playSystemSoundEffect(0x17);

    pThis->m_UpdateMethod = &LCSItemBox_OpenedBoxUpdate;
    pThis->m_DrawMethod = &LCSItemBox_OpenedBoxDraw;
}

static constexpr s16 LCSItemBox_Table4[7] =
{
    0x150,
    0x15C,
    0x164,
    0x16C,
    0x174,
    0x17C,
    0x184,
};

static constexpr s16 LCSItemBox_Table5[7] =
{
    0xC,
    0x18,
    0x20,
    0x28,
    0x30,
    0x38,
    0x40,
};

void LCSItemBox_DrawType0(s_itemBoxType1* pThis)
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
        LCSItemBox_DrawType0Sub0(pThis->m0.m0_mainMemoryBundle, LCSItemBox_Table5[pThis->m8C], LCSItemBox_Table4[pThis->m8C]);
    }

    popMatrix();
}

