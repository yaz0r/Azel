#include "PDS.h"
#include "fieldItemBox.h"
#include "kernel/fileBundle.h"
#include "kernel/animation.h"
#include "fieldVisibilityGrid.h"
#include "fieldItemBoxDefinition.h"
#include "fieldItemBox_type0.h"
#include "fieldItemBox_type1.h"
#include "fieldItemBox_type2.h"

sVec3_FP LCSItemBox_Table6[] = {
    {0, 0x20000, 0},
    {0, 0x1C000, 0},
    {0, 0, 0}
};

void(*LCSItemBox_CallbackTable[3])(p_workArea, sLCSTarget*) = {
     &LCSItemBox_Callback0,
     &LCSItemBox_Callback1,
     &LCSItemBox_CallbackSavePoint,
};

static constexpr s_itemBoxType1::FunctionType LCSItemBox_DrawTable[3] = {
    &LCSItemBox_DrawType0,
    &LCSItemBox_DrawType1,
    &LCSItemBox_DrawType2
};

static constexpr s_itemBoxType1::FunctionType LCSItemBox_UpdateTable[3] = {
    &LCSItemBox_UpdateType0,
    &LCSItemBox_UpdateType1,
    &LCSItemBox_UpdateType2
};

s16 LCSItemBox_Table0[3] = {
    2,
    0,
    0
};

s16 LCSItemBox_TableFlags[3] = {
    0,
    0,
    0x20
};

static s16 LCSItemBox_Table3[] = {
    0x14,
    0x1C,
    0x24,
    0x2C,
    0x2C,
    0x2C,
    0x48
};

static s16 LCSItemBox_Table2[] = {
    0x158,
    0x160,
    0x168,
    0x170,
    0x170,
    0x170,
    0x18C
};

// TODO: move to model functions
void itemBoxType1InitSub0Sub(s_3dModel* r4)
{
    if (!(r4->mA_animationFlags & 0x38))
    {
        return;
    }

    r4->m10_currentAnimationFrame = 0;
    int animFlag = r4->m30_pCurrentAnimation->m0_flags & 7;
    if ((animFlag == 1) || (animFlag == 4) || (animFlag == 5))
    {
        for (int i = 0; i < r4->m12_numBones; i++)
        {
            for (int j = 0; j < 9; j++)
            {
                r4->m2C_poseData[i].m48[j].currentStep = 0;
                r4->m2C_poseData[i].m48[j].delay = 0;
                r4->m2C_poseData[i].m48[j].value = 0;
            }
        }
    }
    stepAnimation(r4);
}

//init box in already opened state
// TODO: move to model functions
void itemBoxType1InitSub0(s_3dModel* r4, s32 r5)
{
    if (r4->mA_animationFlags & 0x38)
    {
        s16 type = r4->m30_pCurrentAnimation->m0_flags & 7;
        switch (type)
        {
        case 4:
            itemBoxType1InitSub0Sub(r4);
            for (int i = 0; i < r5; i++)
            {
                stepAnimation(r4);
            }
            break;
        default:
            assert(0);
            break;
        }
    }
}

static void itemBoxType1Init(s_itemBoxType1* pThis, s_itemBoxDefinition* arg)
{
    s_itemBoxDefinition* r13 = arg;
    getMemoryArea(&pThis->m0, 0);

    pThis->m3C_pos = r13->m0_pos;
    pThis->m48_boundingMin = r13->mC_boundingMin;
    pThis->m54_boundingMax = r13->m18_boundingMax;
    pThis->m6C_rotation = r13->m24_rotation;
    pThis->m78_scale = r13->m30_scale;
    pThis->m7C = FP_Div(0x10000, r13->m30_scale);
    pThis->m80_bitIndex = r13->m34_bitIndex;
    pThis->m84_savePointIndex = r13->m38;
    pThis->m8B_LCSType = r13->m41_LCSType;
    pThis->m88_receivedItemId = r13->m3C_receivedItemId;
    pThis->m8A_receivedItemQuantity = r13->m40_receivedItemQuantity;
    pThis->m8C = r13->m42;
    pThis->m86 = r13->m43;
    pThis->m8D = r13->m44;

    createLCSTarget(&pThis->m8_LCSTarget, pThis, LCSItemBox_CallbackTable[r13->m41_LCSType], &pThis->m60, NULL, LCSItemBox_TableFlags[r13->m41_LCSType] | LCSItemBox_Table0[r13->m41_LCSType] | 0x100, r13->m38, r13->m3C_receivedItemId, r13->m40_receivedItemQuantity, r13->m42);

    switch (r13->m41_LCSType)
    {
    case 0:
    {
        s_fileBundle* pBundle = pThis->m0.m0_mainMemoryBundle;
        sAnimationData* pAnimation = pBundle->getAnimation(0x1A0);
        sStaticPoseData* pDefaultPose = pBundle->getStaticPose(LCSItemBox_Table2[r13->m42], pAnimation->m2_numBones);

        init3DModelRawData(pThis, &pThis->m98_3dModel, 0, pBundle, LCSItemBox_Table3[r13->m42], pAnimation, pDefaultPose, 0, 0);

        if (pThis->m80_bitIndex > 0)
        {
            if (mainGameState.getBit566(pThis->m80_bitIndex))
            {
                pThis->m_DrawMethod = LCSItemBox_OpenedBoxDraw;
                pThis->mEA_wasRendered = 3;

                itemBoxType1InitSub0(&pThis->m98_3dModel, 20);
                return;
            }
        }
        break;
    }
    case 1:
        if (mainGameState.getBit(pThis->m88_receivedItemId + 243))
        {
            pThis->m94 = 0;
            pThis->mEA_wasRendered = 2;
        }
        else
        {
            pThis->m94 = -0x444444;
        }
    case 2:
        pThis->mEC_savePointParticlesTask = createSavePointParticles();
        break;
    default:
        assert(0);
        break;
    }

    pThis->m_UpdateMethod = LCSItemBox_UpdateTable[r13->m41_LCSType];
    pThis->m_DrawMethod = LCSItemBox_DrawTable[r13->m41_LCSType];
}

s8 LCSItemBox_shouldSpin(s_itemBoxType1* pThis)
{
    if (cameraProperties2.m0_position[0] < pThis->m54_boundingMax[0])
        return false;

    if (cameraProperties2.m0_position[0] > pThis->m48_boundingMin[0])
        return false;

    if (cameraProperties2.m0_position[2] < pThis->m54_boundingMax[2])
        return false;

    if (cameraProperties2.m0_position[2] > pThis->m48_boundingMin[2])
        return false;

    return 1;
}

void LCSItemBox_UpdateType0Sub0(s_itemBoxType1* pThis, s32 r5, s32 r6, fixedPoint r7)
{
    PDS_unimplemented("LCSItemBox_UpdateType0Sub0");
}

p_workArea findParentGridCellTaskForItem(s_itemBoxDefinition* r14)
{
    s_visibilityGridWorkArea* r4 = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;
    s_DataTable3* r5 = r4->m30;

    // is object in the grid?
    if (r5 &&
        r4->m3C_cellRenderingTasks &&
        (r14->m0_pos[0] >= 0) &&
        (r14->m0_pos[2] <= 0) &&
        (r14->m0_pos[0] < (r4->m20_cellDimensions[0] * r5->m10_gridSize[0])) &&
        (r14->m0_pos[2] >= -(r4->m20_cellDimensions[1] * r5->m10_gridSize[1])))
    {
        s32 gridX = performDivision(r4->m20_cellDimensions[0], r14->m0_pos[0]);
        s32 gridY = performDivision(r4->m20_cellDimensions[1], -r14->m0_pos[2]);

        return r4->m3C_cellRenderingTasks[(r5->m10_gridSize[0] * gridY) + gridX];
    }
    else
    {
        return getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1->m38;
    }
}

p_workArea fieldA3_1_createItemBoxes_Sub1(s_itemBoxDefinition* r4)
{
    static const s_itemBoxType1::TypedTaskDefinition definition = {
        &itemBoxType1Init,
        nullptr,
        nullptr,
        nullptr,
    };
    return createSubTaskWithArg<s_itemBoxType1>(findParentGridCellTaskForItem(r4), r4, &definition);
}

struct s_itemType0 : public s_workAreaTemplate<s_itemType0>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { NULL, NULL, NULL, NULL };
        return &taskDefinition;
    }

    static void Update(s_itemType0*)
    {
        PDS_unimplemented("s_itemType0::Update");
        //assert(0);
    }

    p_workArea m0;
    s_itemBoxDefinition* m4;
    //size: 8
};

s_itemType0* fieldA3_1_createItemBoxes_Sub0(p_workArea workArea, s_itemBoxDefinition* r5)
{
    s_itemType0* r0 = createSubTaskFromFunction<s_itemType0>(workArea, &s_itemType0::Update);
    if (r0)
    {
        r0->m0 = fieldA3_1_createItemBoxes_Sub1(r5);
        r0->m4 = r5;
    }

    return r0;
}
