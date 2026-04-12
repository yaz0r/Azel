#include "PDS.h"
#include "fieldItemBox.h"
#include "kernel/fileBundle.h"
#include "kernel/animation.h"
#include "processModel.h"
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
            resetAnimation(r4);
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
    pThis->m7C_invScale= FP_Div(0x10000, r13->m30_scale);
    pThis->m80_bitIndex = r13->m34_bitIndex;
    pThis->m84_modelIdx = r13->m38;
    pThis->m8B_LCSType = r13->m41_LCSType;
    pThis->m88_poseIdx = r13->m3C_receivedItemId;
    pThis->m8A_param9 = r13->m40_receivedItemQuantity;
    pThis->m8C_param10= r13->m42;
    pThis->m86 = r13->m43;
    pThis->m8D_visibilityFlag = r13->m44;

    createLCSTarget(&pThis->m8_LCSTarget, pThis, LCSItemBox_CallbackTable[r13->m41_LCSType], &pThis->m60_renderPosition, NULL, LCSItemBox_TableFlags[r13->m41_LCSType] | LCSItemBox_Table0[r13->m41_LCSType] | 0x100, r13->m38, r13->m3C_receivedItemId, r13->m40_receivedItemQuantity, r13->m42);

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
                pThis->mEA_state = 3;

                itemBoxType1InitSub0(&pThis->m98_3dModel, 20);
                return;
            }
        }
        break;
    }
    case 1:
        if (mainGameState.getBit(pThis->m88_poseIdx + 243))
        {
            pThis->m94 = 0;
            pThis->mEA_state = 2;
        }
        else
        {
            pThis->m94 = -0x444444;
        }
    case 2:
        pThis->mEC_savePointParticlesTask = createSavePointParticles(pThis);
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

// 0607730a — per-node draw: cull against dragon, copy matrix, queue entry.
// Assumes caller already switched to camera space.
static void LCSItemBox_UpdateType0Sub0_drawNode(sProcessed3dModel* pModel, fixedPoint scale)
{
    if (pModel == nullptr)
        return;

    s32 boundingRadius = pModel->m0_radius.m_value + 0x8000;
    s_visibilityGridWorkArea* pGrid = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;
    s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;

    s32 dx = pCurrentMatrix->m[0][3].m_value - pDragon->m8_pos.m0_X.m_value;
    if (dx < 0) dx = -dx;
    s32 dy = pCurrentMatrix->m[1][3].m_value - pDragon->m8_pos.m4_Y.m_value;
    if (dy < 0) dy = -dy;
    s32 dz = pCurrentMatrix->m[2][3].m_value - pDragon->m8_pos.m8_Z.m_value;
    if (dz < 0) dz = -dz;

    if (dx <= boundingRadius && dy <= boundingRadius && dz <= boundingRadius)
    {
        copyMatrix(pCurrentMatrix, &pGrid->m44->m4_matrix);
        allocateLCSEntry(pGrid, pModel, scale);
    }
}

// 060774cc — recursive model hierarchy walker.
static void LCSItemBox_UpdateType0Sub0_drawHierarchy(sModelHierarchy* pHierarchy,
    std::vector<sStaticPoseData::sBonePoseData>::const_iterator& pBone, fixedPoint scale)
{
    while (true)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pBone->m0_translation);
        rotateCurrentMatrixZYX(&pBone->mC_rotation);

        if (pHierarchy->m0_3dModel != nullptr)
        {
            LCSItemBox_UpdateType0Sub0_drawNode(pHierarchy->m0_3dModel, scale);
        }

        if (pHierarchy->m4_subNode != nullptr)
        {
            pBone++;
            LCSItemBox_UpdateType0Sub0_drawHierarchy(pHierarchy->m4_subNode, pBone, scale);
        }

        popMatrix();

        if (pHierarchy->m8_nextNode == nullptr)
            break;

        pBone++;
        pHierarchy = pHierarchy->m8_nextNode;
    }
}

// 0606fa32 / 06077626 — deferred draw for model hierarchies. Switches to
// camera space, multiplies the current world matrix, and recursively walks
// the hierarchy submitting each visible node to the collision/LCS queue.
void LCSItemBox_UpdateType0Sub0(s_fileBundle* pBundle, s32 r5, s32 r6, fixedPoint r7)
{
    sModelHierarchy* pHierarchy = pBundle->getModelHierarchy(r5);
    sStaticPoseData* pPose = pBundle->getStaticPose(r6, pHierarchy->countNumberOfBones());

    sMatrix4x3 savedWorldMatrix;
    copyMatrix(pCurrentMatrix, &savedWorldMatrix);

    pushCurrentMatrix();
    copyToCurrentMatrix(&cameraProperties2.m28[0]);
    multiplyCurrentMatrix(&savedWorldMatrix);

    auto boneIter = pPose->m0_bones.cbegin();
    LCSItemBox_UpdateType0Sub0_drawHierarchy(pHierarchy, boneIter, r7);

    popMatrix();
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
        s32 gridX = intDivide(r4->m20_cellDimensions[0], r14->m0_pos[0]);
        s32 gridY = intDivide(r4->m20_cellDimensions[1], -r14->m0_pos[2]);

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
