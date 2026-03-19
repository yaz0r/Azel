#include "PDS.h"
#include "o_fld_c8.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldRadar.h"
#include "field/fieldVisibilityGrid.h"
#include "kernel/fileBundle.h"
#include "kernel/animation.h"
#include "processModel.h"
#include "audio/soundDriver.h"
#include "audio/systemSounds.h"

s32 func3dModelSub0(s_3dModel* r4);
void update3dModelDrawFunctionForVertexAnimation(s_3dModel* r4, u8* pData);

static void fieldC8_0_startTasks(p_workArea workArea) { Unimplemented(); }

// 0605f794 — create VDP2 task
static void createVdp2Task_C8_0(p_workArea workArea)
{
    createC8Vdp2Task(workArea, gFLD_C8->getSaturnPtr(0x060b2d74));
}

// --- Environment object task (0x24 bytes, task definition at 0609cabc) ---

struct s_envObjectC8_arg
{
    sSaturnPtr m0_entryEA;
    s32 m4_areaIndex;
    sSaturnPtr m8_modelFrameDataEA;
    s32 mC_speed;
    s32 m10_groupCount;
};

struct s_envObjectC8 : public s_workAreaTemplateWithArg<s_envObjectC8, s_envObjectC8_arg*>
{
    s_memoryAreaOutput m0_memoryArea;
    sSaturnPtr m8_entryEA;
    sSaturnPtr mC_modelFrameDataEA;
    s32 m10_fraction;
    s32 m14_speed;
    s32 m18_frameIndex;
    s32 m1C_groupCount;
    s32 m20_currentGroup;
    // size 0x24

    static void Init(s_envObjectC8* pThis, s_envObjectC8_arg* pArg);
    static void Update(s_envObjectC8* pThis);
    static void Draw(s_envObjectC8* pThis);

    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &Init, &Update, &Draw, nullptr };
        return &taskDefinition;
    }
};

// 0605b824
void s_envObjectC8::Init(s_envObjectC8* pThis, s_envObjectC8_arg* pArg)
{
    getMemoryArea(&pThis->m0_memoryArea, pArg->m4_areaIndex);
    pThis->m8_entryEA = pArg->m0_entryEA;
    pThis->mC_modelFrameDataEA = pArg->m8_modelFrameDataEA;
    pThis->m10_fraction = 0;
    pThis->m18_frameIndex = 0;
    pThis->m14_speed = pArg->mC_speed;
    pThis->m1C_groupCount = pArg->m10_groupCount;
    pThis->m20_currentGroup = 0;
}

// 0605b854
void s_envObjectC8::Update(s_envObjectC8* pThis)
{
    sSaturnPtr frameArrayEA = readSaturnEA(pThis->mC_modelFrameDataEA);

    s32 fraction = pThis->m10_fraction + pThis->m14_speed;
    pThis->m10_fraction = fraction;

    if (fraction > 0xFFFF)
    {
        pThis->m10_fraction = pThis->m10_fraction - 0x10000;
        s32 frameIndex = pThis->m18_frameIndex + 1;
        pThis->m18_frameIndex = frameIndex;

        if (readSaturnS32(frameArrayEA + pThis->m20_currentGroup * 8) <= frameIndex)
        {
            pThis->m18_frameIndex = 0;
            s32 group = pThis->m20_currentGroup + 1;
            pThis->m20_currentGroup = group;

            if (pThis->m1C_groupCount <= group)
            {
                pThis->m20_currentGroup = 0;
            }
        }
    }
}

// 06078044 — queue model for deferred rendering in visibility grid
static void envObjectC8_QueueModel(s_visibilityGridWorkArea* pGrid, sProcessed3dModel* pModel, fixedPoint param)
{
    s16 count = pGrid->m12E4_numCollisionGeometries + 1;
    pGrid->m12E4_numCollisionGeometries = count;
    if (count < 0x18)
    {
        pGrid->m44->m0_model = pModel;
        pGrid->m44->m34 = param;
        pGrid->m44++;
    }
}

// 0607840a — distance-culled per-node rendering
static void envObjectC8_DrawNode(sProcessed3dModel* pModel, fixedPoint param)
{
    if (pModel != nullptr)
    {
        fixedPoint radius = pModel->m0_radius + fixedPoint(0x8000);
        s_visibilityGridWorkArea* pGrid = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;
        s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;

        fixedPoint dx = pCurrentMatrix->m[0][3] - pDragon->m8_pos.m0_X;
        if ((s32)dx < 0) dx = -dx;
        fixedPoint dy = pCurrentMatrix->m[1][3] - pDragon->m8_pos.m4_Y;
        if ((s32)dy < 0) dy = -dy;
        fixedPoint dz = pCurrentMatrix->m[2][3] - pDragon->m8_pos.m8_Z;
        if ((s32)dz < 0) dz = -dz;

        if ((s32)dx <= (s32)radius && (s32)dy <= (s32)radius && (s32)dz <= (s32)radius)
        {
            copyMatrix(pCurrentMatrix, &pGrid->m44->m4_matrix);
            if (pGrid->m12F2_renderMode == 1)
            {
                Unimplemented(); // debug collision rendering
            }
            envObjectC8_QueueModel(pGrid, pModel, param);
        }
    }
}

// 060785cc — recursive model hierarchy tree walker
static void envObjectC8_DrawHierarchy(sModelHierarchy* pNode, std::vector<sStaticPoseData::sBonePoseData>::const_iterator& pBone, fixedPoint param)
{
    do
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pBone->m0_translation);
        rotateCurrentMatrixZYX(&pBone->mC_rotation);

        if (pNode->m0_3dModel)
        {
            envObjectC8_DrawNode(pNode->m0_3dModel, param);
        }
        if (pNode->m4_subNode)
        {
            pBone++;
            envObjectC8_DrawHierarchy(pNode->m4_subNode, pBone, param);
        }

        popMatrix();

        if (pNode->m8_nextNode == nullptr)
            break;

        pBone++;
        pNode = pNode->m8_nextNode;
    } while (true);
}

// 06078726 — draw model hierarchy with billboard matrix
static void envObjectC8_DrawWithBillboard(s_fileBundle* pBundle, s16 hierarchyOffset, s16 poseOffset)
{
    sModelHierarchy* pHierarchy = pBundle->getModelHierarchy(hierarchyOffset);
    sStaticPoseData* pPose = pBundle->getStaticPose(poseOffset, pHierarchy->countNumberOfBones());

    sMatrix4x3 savedMatrix;
    copyMatrix(pCurrentMatrix, &savedMatrix);
    pushCurrentMatrix();
    copyToCurrentMatrix(&cameraProperties2.m88_billboardViewMatrix);
    multiplyCurrentMatrix(&savedMatrix);

    std::vector<sStaticPoseData::sBonePoseData>::const_iterator bones = pPose->m0_bones.begin();
    envObjectC8_DrawHierarchy(pHierarchy, bones, fixedPoint(0));

    popMatrix();
}

// 0607877e
static void envObjectC8_DrawSub(s_memoryAreaOutput* pMemArea, s16 param1, s16 param2)
{
    envObjectC8_DrawWithBillboard(pMemArea->m0_mainMemoryBundle, param1, param2);
}

// 0605b898
void s_envObjectC8::Draw(s_envObjectC8* pThis)
{
    sSaturnPtr entryEA = pThis->m8_entryEA;
    sSaturnPtr frameArrayEA = readSaturnEA(pThis->mC_modelFrameDataEA);
    sSaturnPtr frameEntryEA = frameArrayEA + pThis->m20_currentGroup * 8;

    pushCurrentMatrix();
    translateCurrentMatrix(readSaturnVec3(entryEA + 4));

    sVec3_S16_12_4 rot;
    rot[0] = readSaturnS16(entryEA + 0x10);
    rot[1] = readSaturnS16(entryEA + 0x12);
    rot[2] = readSaturnS16(entryEA + 0x14);
    rotateCurrentMatrixZYX_s16(rot);

    LCSItemBox_DrawType0Sub0(pThis->m0_memoryArea.m0_mainMemoryBundle,
        readSaturnS16(frameEntryEA + 4),
        readSaturnS16(frameEntryEA + 6));

    s16 extraParam1 = readSaturnS16(pThis->mC_modelFrameDataEA + 4);
    if (extraParam1 != 0)
    {
        envObjectC8_DrawSub(&pThis->m0_memoryArea, extraParam1, readSaturnS16(pThis->mC_modelFrameDataEA + 6));
    }

    popMatrix();
}

// 0605b8fe — create environment object task from entry data
static void createEnvObjectTask_C8(p_workArea gridCell, s_envObjectC8_arg* pArg)
{
    createSubTaskWithArg<s_envObjectC8>(gridCell, pArg);
}

// 0605b270
static void createEnvObject_C8_type0(p_workArea gridCell, sSaturnPtr entryEA, s32 value)
{
    s_envObjectC8_arg arg;
    arg.m0_entryEA = entryEA;
    arg.m4_areaIndex = value;
    arg.m8_modelFrameDataEA = gFLD_C8->getSaturnPtr(0x0609C52C);
    arg.mC_speed = 0x8000;
    arg.m10_groupCount = 7;
    createEnvObjectTask_C8(gridCell, &arg);
}

// 0605b294
static void createEnvObject_C8_type0b(p_workArea gridCell, sSaturnPtr entryEA, s32 value)
{
    s_envObjectC8_arg arg;
    arg.m0_entryEA = entryEA;
    arg.m4_areaIndex = value;
    arg.m8_modelFrameDataEA = gFLD_C8->getSaturnPtr(0x0609C564);
    arg.mC_speed = 0x8000;
    arg.m10_groupCount = 6;
    createEnvObjectTask_C8(gridCell, &arg);
}

// 0605b2b8
static void createEnvObject_C8_type1(p_workArea gridCell, sSaturnPtr entryEA, s32 value)
{
    s_envObjectC8_arg arg;
    arg.m0_entryEA = entryEA;
    arg.m4_areaIndex = value;
    arg.m8_modelFrameDataEA = gFLD_C8->getSaturnPtr(0x0609C614);
    arg.mC_speed = 0x8000;
    arg.m10_groupCount = 7;
    createEnvObjectTask_C8(gridCell, &arg);
}

// 0605b2dc
static void createEnvObject_C8_type1b(p_workArea gridCell, sSaturnPtr entryEA, s32 value)
{
    s_envObjectC8_arg arg;
    arg.m0_entryEA = entryEA;
    arg.m4_areaIndex = value;
    arg.m8_modelFrameDataEA = gFLD_C8->getSaturnPtr(0x0609C61C);
    arg.mC_speed = 0x8000;
    arg.m10_groupCount = 7;
    createEnvObjectTask_C8(gridCell, &arg);
}

// 0605b300
static void createEnvObject_C8_type2(p_workArea gridCell, sSaturnPtr entryEA, s32 value)
{
    s_envObjectC8_arg arg;
    arg.m0_entryEA = entryEA;
    arg.m4_areaIndex = value;
    arg.m8_modelFrameDataEA = gFLD_C8->getSaturnPtr(0x0609C624);
    arg.mC_speed = 0x8000;
    arg.m10_groupCount = 7;
    createEnvObjectTask_C8(gridCell, &arg);
}

// 0605b324
static void createEnvObject_C8_type2b(p_workArea gridCell, sSaturnPtr entryEA, s32 value)
{
    s_envObjectC8_arg arg;
    arg.m0_entryEA = entryEA;
    arg.m4_areaIndex = value;
    arg.m8_modelFrameDataEA = gFLD_C8->getSaturnPtr(0x0609C65C);
    arg.mC_speed = 0x5555;
    arg.m10_groupCount = 6;
    createEnvObjectTask_C8(gridCell, &arg);
}

// --- Static draw entity (0x10 bytes, task definition at 0608b434) ---

struct s_staticDrawC8_arg
{
    sSaturnPtr m0_entryEA;
    s32 m4_areaIndex;
    s16 m8_drawParam1;
    s16 mA_drawParam2;
};

struct s_staticDrawC8 : public s_workAreaTemplateWithArg<s_staticDrawC8, s_staticDrawC8_arg*>
{
    s_memoryAreaOutput m0_memoryArea;
    sSaturnPtr m8_entryEA;
    s16 mC_drawParam1;
    s16 mE_drawParam2;
    // size 0x10

    static void Init(s_staticDrawC8* pThis, s_staticDrawC8_arg* pArg);
    static void Draw(s_staticDrawC8* pThis);

    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &Init, nullptr, &Draw, nullptr };
        return &taskDefinition;
    }
};

// 06055c7c
void s_staticDrawC8::Init(s_staticDrawC8* pThis, s_staticDrawC8_arg* pArg)
{
    getMemoryArea(&pThis->m0_memoryArea, pArg->m4_areaIndex);
    pThis->m8_entryEA = pArg->m0_entryEA;
    pThis->mC_drawParam1 = pArg->m8_drawParam1;
    pThis->mE_drawParam2 = pArg->mA_drawParam2;
}

// 06055c30
void s_staticDrawC8::Draw(s_staticDrawC8* pThis)
{
    sSaturnPtr entryEA = pThis->m8_entryEA;

    pushCurrentMatrix();
    translateCurrentMatrix(readSaturnVec3(entryEA + 4));

    sVec3_S16_12_4 rot;
    rot[0] = readSaturnS16(entryEA + 0x10);
    rot[1] = readSaturnS16(entryEA + 0x12);
    rot[2] = readSaturnS16(entryEA + 0x14);
    rotateCurrentMatrixZYX_s16(rot);

    sSaturnPtr modelDataEA = readSaturnEA(entryEA + 0x1C);
    LCSItemBox_DrawType0Sub0(pThis->m0_memoryArea.m0_mainMemoryBundle,
        readSaturnS16(modelDataEA),
        readSaturnS16(modelDataEA + 2));

    envObjectC8_DrawSub(&pThis->m0_memoryArea, pThis->mC_drawParam1, pThis->mE_drawParam2);

    popMatrix();
}

// 06055cf4
static void createCellEntity_C8_06055cf4(p_workArea gridCell, sSaturnPtr entryEA, s32 value)
{
    s_staticDrawC8_arg arg;
    arg.m0_entryEA = entryEA;
    arg.m4_areaIndex = value;
    arg.m8_drawParam1 = 0x28;
    arg.mA_drawParam2 = 0x5d4;
    createSubTaskWithArg<s_staticDrawC8>(gridCell, &arg);
}

// 06055d14
static void createCellEntity_C8_06055d14(p_workArea gridCell, sSaturnPtr entryEA, s32 value)
{
    s_staticDrawC8_arg arg;
    arg.m0_entryEA = entryEA;
    arg.m4_areaIndex = value;
    arg.m8_drawParam1 = 0x30;
    arg.mA_drawParam2 = 0x5dc;
    createSubTaskWithArg<s_staticDrawC8>(gridCell, &arg);
}

// --- Simple model entity (0xC bytes, task definitions at 060b63d8/060b63e8) ---

struct s_simpleModelC8 : public s_workAreaTemplate<s_simpleModelC8>
{
    s_memoryAreaOutput m0_memoryArea;
    sSaturnPtr m8_entryEA;
    // size 0xC

    static void Draw0(s_simpleModelC8* pThis);
    static void Draw1(s_simpleModelC8* pThis);

    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { nullptr, nullptr, nullptr, nullptr };
        return &taskDefinition;
    }
};

// 0607a2fc
void s_simpleModelC8::Draw0(s_simpleModelC8* pThis)
{
    s_visibilityGridWorkArea* pGrid = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;
    sSaturnPtr entryEA = pThis->m8_entryEA;

    sVec3_FP pos = readSaturnVec3(entryEA + 4);
    if (pGrid->m12FC_isObjectClipped && pGrid->m12FC_isObjectClipped(&pos, graphicEngineStatus.m405C.m14_farClipDistance))
    {
        return;
    }

    pushCurrentMatrix();
    translateCurrentMatrix(pos);

    sVec3_S16_12_4 rot;
    rot[0] = readSaturnS16(entryEA + 0x10);
    rot[1] = readSaturnS16(entryEA + 0x12);
    rot[2] = readSaturnS16(entryEA + 0x14);
    rotateCurrentMatrixZYX_s16(rot);

    sSaturnPtr modelDataEA = readSaturnEA(entryEA + 0x1C);
    LCSItemBox_DrawType0Sub0(pThis->m0_memoryArea.m0_mainMemoryBundle,
        readSaturnS16(modelDataEA),
        readSaturnS16(modelDataEA + 2));

    popMatrix();
}

// 0607a398
void s_simpleModelC8::Draw1(s_simpleModelC8* pThis)
{
    s_visibilityGridWorkArea* pGrid = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;
    sSaturnPtr entryEA = pThis->m8_entryEA;

    sVec3_FP pos = readSaturnVec3(entryEA + 4);
    if (pGrid->m12FC_isObjectClipped && pGrid->m12FC_isObjectClipped(&pos, graphicEngineStatus.m405C.m14_farClipDistance))
    {
        return;
    }

    pushCurrentMatrix();
    translateCurrentMatrix(pos);

    sVec3_S16_12_4 rot;
    rot[0] = readSaturnS16(entryEA + 0x10);
    rot[1] = readSaturnS16(entryEA + 0x12);
    rot[2] = readSaturnS16(entryEA + 0x14);
    rotateCurrentMatrixZYX_s16(rot);

    u32 depthRangeIndex = gridCellDraw_GetDepthRange(pCurrentMatrix->m[2][3]);
    if ((s32)depthRangeIndex <= pGrid->m1300)
    {
        sSaturnPtr modelDataEA = readSaturnEA(entryEA + 0x1C);
        s16 modelOffset = readSaturnS16(modelDataEA);
        addBillBoardToDrawList(pThis->m0_memoryArea.m0_mainMemoryBundle->get3DModel(modelOffset));
    }

    popMatrix();
}

// 0607a414
static void createCellEntity_C8_0607a414(p_workArea gridCell, sSaturnPtr entryEA, s32 value)
{
    sSaturnPtr modelDataEA = readSaturnEA(entryEA + 0x1C);
    s16 selector = readSaturnS16(modelDataEA + 2);

    static const s_simpleModelC8::TypedTaskDefinition taskDef0 = { nullptr, nullptr, &s_simpleModelC8::Draw1, nullptr };
    static const s_simpleModelC8::TypedTaskDefinition taskDef1 = { nullptr, nullptr, &s_simpleModelC8::Draw0, nullptr };

    const s_simpleModelC8::TypedTaskDefinition* pDef = (selector == 0) ? &taskDef0 : &taskDef1;

    s_simpleModelC8* pTask = createSubTask<s_simpleModelC8>(gridCell, pDef);
    if (pTask)
    {
        getMemoryArea(&pTask->m0_memoryArea, value);
        pTask->m8_entryEA = entryEA;
    }
}

// --- Interactive pickup entity (0xA0 bytes, task definition at 0609c074) ---

struct s_interactiveEntityC8_arg
{
    sSaturnPtr m0_entryEA;
    s32 m4_areaIndex;
    sSaturnPtr m8_dataPtr;
    sSaturnPtr mC_tablePtr;
};

struct s_interactiveEntityC8 : public s_workAreaTemplateWithArg<s_interactiveEntityC8, s_interactiveEntityC8_arg*>
{
    s_memoryAreaOutput m0_memoryArea;
    sSaturnPtr m8_entryEA;
    // mC-m8F: collision/LCS sub-structure, s_3dModel at m40, etc.
    u8 m24_flags;
    u8 m9C_state;
    u8 m9D;
    u8 m9F;
    sSaturnPtr m90_dataPtr;
    sSaturnPtr m94_tablePtr;
    fixedPoint m98_depth;
    // size 0xA0

    static void Init(s_interactiveEntityC8* pThis, s_interactiveEntityC8_arg* pArg);
    static void Update(s_interactiveEntityC8* pThis);
    static void Draw(s_interactiveEntityC8* pThis);

    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &Init, &Update, &Draw, nullptr };
        return &taskDefinition;
    }
};

// 0605a270
void s_interactiveEntityC8::Init(s_interactiveEntityC8* pThis, s_interactiveEntityC8_arg* pArg)
{
    Unimplemented(); // complex: game state checks, animation init, sound, LCS setup
}

// 0605a3d2
void s_interactiveEntityC8::Update(s_interactiveEntityC8* pThis)
{
    Unimplemented(); // 5-state animation state machine
}

// 0605a5e8
void s_interactiveEntityC8::Draw(s_interactiveEntityC8* pThis)
{
    Unimplemented(); // clip check, translate/rotate, model draw
}

// 0605a680
static void createInteractiveEntityC8(p_workArea gridCell, s_interactiveEntityC8_arg* pArg)
{
    createSubTaskWithArg<s_interactiveEntityC8>(gridCell, pArg);
}

// 0605a6c0
static void createCellEntity_C8_0605a6c0(p_workArea gridCell, sSaturnPtr entryEA, s32 value)
{
    s_interactiveEntityC8_arg arg;
    arg.m0_entryEA = entryEA;
    arg.m4_areaIndex = value;
    arg.m8_dataPtr = gFLD_C8->getSaturnPtr(0x0609C134);
    arg.mC_tablePtr = gFLD_C8->getSaturnPtr(0x0609C0FC);
    createInteractiveEntityC8(gridCell, &arg);
}

// 0605a6e0
static void createCellEntity_C8_0605a6e0(p_workArea gridCell, sSaturnPtr entryEA, s32 value)
{
    s_interactiveEntityC8_arg arg;
    arg.m0_entryEA = entryEA;
    arg.m4_areaIndex = value;
    arg.m8_dataPtr = gFLD_C8->getSaturnPtr(0x0609C124);
    arg.mC_tablePtr = gFLD_C8->getSaturnPtr(0x0609C104);
    createInteractiveEntityC8(gridCell, &arg);
}

// 0605d6fc — linear search in array of 0x14-byte entries
static s32 searchEntryArray(sSaturnPtr arrayEA, s32 key, s32 count)
{
    for (s32 i = 0; i < count; i++)
    {
        if (readSaturnS32(arrayEA + i * 0x14) == key)
        {
            return i;
        }
    }
    return 0;
}

// 0605a740
static void createCellEntity_C8_0605a740(p_workArea gridCell, sSaturnPtr entryEA, s32 value)
{
    s_interactiveEntityC8_arg arg;
    arg.m0_entryEA = entryEA;
    arg.m4_areaIndex = value;
    arg.m8_dataPtr = gFLD_C8->getSaturnPtr(0x0609C164);
    arg.mC_tablePtr = gFLD_C8->getSaturnPtr(0x0609C11C);
    createInteractiveEntityC8(gridCell, &arg);
}

// 0605e59c — creates 0x1E0-byte entity (tower creature/NPC)
static void createCellEntity_C8_0605e59c(p_workArea gridCell, sSaturnPtr entryEA, s32 value)
{
    // 0605d6fc — search for entry type in data table
    sSaturnPtr tableEA = gFLD_C8->getSaturnPtr(0x060B2904);
    s32 entryType = readSaturnS32(entryEA + 0x18);
    s8 index = (s8)searchEntryArray(tableEA, entryType, 3);

    // Build arg struct: {entryEA, value, dataPtr, tablePtr}
    // dataPtr = tableEA + index * 0x14
    // 0605e550 — creates 0x1E0-byte task
    Unimplemented(); // Init/Update/Draw (0605DA3A/0605E32A/0605E448) have bad instruction data
}

// 06055cd4
static void createCellEntity_C8_06055cd4(p_workArea gridCell, sSaturnPtr entryEA, s32 value)
{
    s_staticDrawC8_arg arg;
    arg.m0_entryEA = entryEA;
    arg.m4_areaIndex = value;
    arg.m8_drawParam1 = 0x20;
    arg.mA_drawParam2 = 0x5cc;
    createSubTaskWithArg<s_staticDrawC8>(gridCell, &arg);
}

// 06059a9c — linear search in 0x10-stride entry array
static s32 searchEntryArray16(s32 key, sSaturnPtr arrayEA, s32 count)
{
    for (s32 i = 0; i < count; i++)
    {
        if (readSaturnS32(arrayEA + i * 0x10) == key)
        {
            return i;
        }
    }
    return 0;
}

// --- 0xB4-byte entity (task definition at 0609bf5c) ---

struct s_entityC8_B4_arg
{
    sSaturnPtr m0_entryEA;
    s32 m4_areaIndex;
    sSaturnPtr m8_tablePtr;
    sSaturnPtr mC_dataPtr;
    sSaturnPtr m10_extraPtr;
};

// 06075b84 — LCS/collision sub-structure (0x24 bytes at m60)
struct s_fieldLCSSubStruct
{
    p_workArea m0_owner;
    void* m4_callback;
    sVec3_FP* m8_positionPtr;
    s32 mC;
    s16 m10;
    s16 m12;
    s32 m14;
    u8 m16;
    u8 m17;
    u8 m18;
    u8 m19;
    u8 m1A;
    u8 m1B;
    s32 m1C;
    s32 m20;
    // size 0x24
};

// 06075b84
static void initFieldLCSSubStruct(s_fieldLCSSubStruct* pLCS, p_workArea owner, void* callback,
    sVec3_FP* posPtr, s32 param5, s16 param6, s16 param7, s16 param8, u8 param9, u8 param10)
{
    pLCS->m0_owner = owner;
    pLCS->m4_callback = callback;
    pLCS->m8_positionPtr = posPtr;
    pLCS->mC = param5;
    pLCS->m12 = param7;
    pLCS->m14 = param8;
    if (param8 < 0)
        pLCS->m16 = 0;
    else
        pLCS->m16 = param9;
    pLCS->m10 = param6;
    pLCS->m17 = param10;
    pLCS->m18 = 0;
    pLCS->m19 = 0;
    pLCS->m1A = 0;
    pLCS->m1B = 0;
    pLCS->m1C = 0;
    pLCS->m20 = 0;
}

struct s_entityC8_B4 : public s_workAreaTemplateWithArg<s_entityC8_B4, s_entityC8_B4_arg*>
{
    s_memoryAreaOutput m0_memoryArea;
    sSaturnPtr m8_entryEA;
    s_3dModel mC_3dModel;
    p_workArea m5C_childTask;
    s_fieldLCSSubStruct m60_lcs;
    u8 m78_flags;
    sVec3_FP m94_position;
    sSaturnPtr mA0_tablePtr;
    sSaturnPtr mA4_dataPtr;
    sSaturnPtr mA8_extraPtr;
    fixedPoint mAC_depth;
    s8 mB0_state;
    // size 0xB4

    static void Init(s_entityC8_B4* pThis, s_entityC8_B4_arg* pArg);
    static void Update(s_entityC8_B4* pThis);
    static void Draw(s_entityC8_B4* pThis);

    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &Init, &Update, &Draw, nullptr };
        return &taskDefinition;
    }
};

// 06059c38 — kill child env object task
static void entityC8_B4_killChild(s_entityC8_B4* pThis)
{
    if (pThis->m5C_childTask != nullptr)
    {
        if (pThis->m5C_childTask != nullptr)
        {
            pThis->m5C_childTask->getTask()->m14_flags |= 1;
        }
        pThis->m5C_childTask = nullptr;
    }
}

// 06059d20 — init 3D model in idle/static state
static void entityC8_B4_initIdle(s_entityC8_B4* pThis)
{
    entityC8_B4_killChild(pThis);

    s_fileBundle* pBundle = pThis->m0_memoryArea.m0_mainMemoryBundle;
    sSaturnPtr extraPtr = pThis->mA8_extraPtr;

    u16 modelIndex = readSaturnU16(extraPtr);
    u16 poseOffset = readSaturnU16(extraPtr + 2);
    u16 colorAnimOffset = readSaturnU16(extraPtr + 12);

    sModelHierarchy* pHierarchy = pBundle->getModelHierarchy(modelIndex);
    sStaticPoseData* pPose = pBundle->getStaticPose(poseOffset, pHierarchy->countNumberOfBones());
    u8* pColorAnim = colorAnimOffset ? pBundle->getRawFileAtOffset(colorAnimOffset) : nullptr;

    init3DModelRawData(pThis, &pThis->mC_3dModel, 0, pBundle, modelIndex, nullptr, pPose, pColorAnim, nullptr);
    stepAnimation(&pThis->mC_3dModel);
    func3dModelSub0(&pThis->mC_3dModel);
}

// 06059d74 — activate entity (start animation)
static void entityC8_B4_activate(s_entityC8_B4* pThis)
{
    entityC8_B4_killChild(pThis);

    s_fileBundle* pBundle = pThis->m0_memoryArea.m0_mainMemoryBundle;
    sSaturnPtr extraPtr = pThis->mA8_extraPtr;

    u16 modelIndex = readSaturnU16(extraPtr + 4);
    u16 animOffset = readSaturnU16(extraPtr + 6);
    u16 poseOffset = readSaturnU16(extraPtr + 8);
    u16 colorAnimOffset = readSaturnU16(extraPtr + 10);

    sModelHierarchy* pHierarchy = pBundle->getModelHierarchy(modelIndex);
    u32 numBones = pHierarchy->countNumberOfBones();
    sAnimationData* pAnim = animOffset ? pBundle->getAnimation(animOffset) : nullptr;
    sStaticPoseData* pPose = pBundle->getStaticPose(poseOffset, numBones);
    u8* pColorAnim = colorAnimOffset ? pBundle->getRawFileAtOffset(colorAnimOffset) : nullptr;

    init3DModelRawData(pThis, &pThis->mC_3dModel, 0, pBundle, modelIndex, pAnim, pPose, pColorAnim, nullptr);
    stepAnimation(&pThis->mC_3dModel);
    func3dModelSub0(&pThis->mC_3dModel);
    pThis->m78_flags |= 1;
}

// 06059c88 — normal mode init (creates child env object + LCS collision)
static void entityC8_B4_initNormal(s_entityC8_B4* pThis, s_entityC8_B4_arg* pArg)
{
    entityC8_B4_killChild(pThis);

    // Create child animated env object
    s_envObjectC8_arg envArg;
    envArg.m0_entryEA = pArg->m0_entryEA;
    envArg.m4_areaIndex = pArg->m4_areaIndex;
    envArg.m8_modelFrameDataEA = pThis->mA0_tablePtr;
    envArg.mC_speed = 0x8000;
    envArg.m10_groupCount = 6;
    pThis->m5C_childTask = createSubTaskWithArg<s_envObjectC8>(pThis, &envArg);

    // Compute LCS position from entry transform + offset vector
    static const sVec3_FP offsetVec = { fixedPoint(0), fixedPoint(0), fixedPoint(0x93333) };
    sMatrix4x3 mat;
    initMatrixToIdentity(&mat);
    translateMatrix(readSaturnVec3(pThis->m8_entryEA + 4), &mat);

    sVec3_S16_12_4 rot;
    rot[0] = readSaturnS16(pThis->m8_entryEA + 0x10);
    rot[1] = readSaturnS16(pThis->m8_entryEA + 0x12);
    rot[2] = readSaturnS16(pThis->m8_entryEA + 0x14);
    rotateMatrixZYX_s16(rot, &mat);
    transformAndAddVec(offsetVec, pThis->m94_position, mat);

    // Init LCS sub-structure
    initFieldLCSSubStruct(&pThis->m60_lcs, pThis, nullptr /*DAT_06059ae0*/, &pThis->m94_position,
        0, 0, 0, -1, -1, 0);
    pThis->m78_flags = 0;
}

// 06059f60 — alternate update (idle animation only)
static void entityC8_B4_updateAlt(s_entityC8_B4* pThis)
{
    func3dModelSub0(&pThis->mC_3dModel);
}

// 06059e14
void s_entityC8_B4::Init(s_entityC8_B4* pThis, s_entityC8_B4_arg* pArg)
{
    getMemoryArea(&pThis->m0_memoryArea, pArg->m4_areaIndex);
    pThis->m8_entryEA = pArg->m0_entryEA;
    pThis->mA0_tablePtr = pArg->m8_tablePtr;
    pThis->mA4_dataPtr = pArg->mC_dataPtr;
    pThis->mA8_extraPtr = pArg->m10_extraPtr;
    pThis->m5C_childTask = nullptr;

    bool isAlternateMode = (mainGameState.bitField[0xA6] & 8) != 0;
    if (!isAlternateMode)
    {
        entityC8_B4_initNormal(pThis, pArg);
    }
    else
    {
        entityC8_B4_initIdle(pThis);
        pThis->m_UpdateMethod = (void(*)(s_entityC8_B4*))entityC8_B4_updateAlt;
    }
    pThis->mB0_state = isAlternateMode ? 1 : 0;
}

// 06059e70
void s_entityC8_B4::Update(s_entityC8_B4* pThis)
{
    // 0605b6b6/06075bd8 — LCS collision check/update (depends on field-specific zone data)
    // TODO: implement when field zone system (field_C + 0xDC/0xE0) is understood

    switch (pThis->mB0_state)
    {
    case 0:
        if ((mainGameState.bitField[0xA6] & 8) != 0)
        {
            entityC8_B4_activate(pThis);
            playSystemSoundEffect(0x66);
            pThis->mB0_state = 2;
        }
        break;
    case 1:
        pThis->m_UpdateMethod = (void(*)(s_entityC8_B4*))entityC8_B4_updateAlt;
        break;
    case 2:
    {
        s16 frame = stepAnimation(&pThis->mC_3dModel);
        if (frame == 0x24)
        {
            pThis->mB0_state = 3;
        }
        break;
    }
    case 3:
    {
        stepAnimation(&pThis->mC_3dModel);
        s16 result = func3dModelSub0(&pThis->mC_3dModel);
        if (result == 0)
        {
            u16 vertAnimOffset = readSaturnU16(pThis->mA8_extraPtr + 12);
            update3dModelDrawFunctionForVertexAnimation(
                &pThis->mC_3dModel,
                pThis->m0_memoryArea.m0_mainMemoryBundle->getRawFileAtOffset(vertAnimOffset));
            pThis->mB0_state = 4;
        }
        break;
    }
    case 4:
    {
        func3dModelSub0(&pThis->mC_3dModel);
        s16 frame = stepAnimation(&pThis->mC_3dModel);
        if (frame == 0)
        {
            entityC8_B4_initIdle(pThis);
            pThis->mB0_state = 1;
        }
        break;
    }
    }
}

// 06059f6c
void s_entityC8_B4::Draw(s_entityC8_B4* pThis)
{
    if (pThis->mB0_state == 0)
        return;

    pushCurrentMatrix();
    translateCurrentMatrix(readSaturnVec3(pThis->m8_entryEA + 4));

    sSaturnPtr extraPtr = pThis->mA8_extraPtr;

    sVec3_S16_12_4 rot;
    rot[0] = readSaturnS16(pThis->m8_entryEA + 0x10);
    rot[1] = readSaturnS16(pThis->m8_entryEA + 0x12);
    rot[2] = readSaturnS16(pThis->m8_entryEA + 0x14);
    rotateCurrentMatrixZYX_s16(rot);

    pThis->mAC_depth = pCurrentMatrix->m[2][3];
    pThis->mC_3dModel.m18_drawFunction(&pThis->mC_3dModel);
    envObjectC8_DrawSub(&pThis->m0_memoryArea,
        readSaturnS16(extraPtr + 0xE),
        readSaturnS16(extraPtr + 0x10));

    popMatrix();
}

// 06059fc8
static void createEntityC8_B4(p_workArea gridCell, s_entityC8_B4_arg* pArg)
{
    createSubTaskWithArg<s_entityC8_B4>(gridCell, pArg);
}

// 0605a008
static void createCellEntity_C8_0605a008(p_workArea gridCell, sSaturnPtr entryEA, s32 value)
{
    s32 entryType = readSaturnS32(entryEA + 0x18);
    s32 index = searchEntryArray16(entryType, gFLD_C8->getSaturnPtr(0x0609BFB8), 2);

    s_entityC8_B4_arg arg;
    arg.m0_entryEA = entryEA;
    arg.m4_areaIndex = value;
    arg.m8_tablePtr = gFLD_C8->getSaturnPtr(0x0609BF9C);
    arg.mC_dataPtr = gFLD_C8->getSaturnPtr(0x0609BFB8) + index * 0x10;
    arg.m10_extraPtr = gFLD_C8->getSaturnPtr(0x0609BFA4);
    createEntityC8_B4(gridCell, &arg);
}

// 0605a700
static void createCellEntity_C8_0605a700(p_workArea gridCell, sSaturnPtr entryEA, s32 value)
{
    s_interactiveEntityC8_arg arg;
    arg.m0_entryEA = entryEA;
    arg.m4_areaIndex = value;
    arg.m8_dataPtr = gFLD_C8->getSaturnPtr(0x0609C154);
    arg.mC_tablePtr = gFLD_C8->getSaturnPtr(0x0609C10C);
    createInteractiveEntityC8(gridCell, &arg);
}

// 0605a720
static void createCellEntity_C8_0605a720(p_workArea gridCell, sSaturnPtr entryEA, s32 value)
{
    s_interactiveEntityC8_arg arg;
    arg.m0_entryEA = entryEA;
    arg.m4_areaIndex = value;
    arg.m8_dataPtr = gFLD_C8->getSaturnPtr(0x0609C144);
    arg.mC_tablePtr = gFLD_C8->getSaturnPtr(0x0609C114);
    createInteractiveEntityC8(gridCell, &arg);
}

// 06079dfe — dispatch per-cell entity creation from function pointer list
static void dispatchCellEntities_C8(p_workArea gridCell, sSaturnPtr cellDataEA, s32 value)
{
    sSaturnPtr entry = cellDataEA;
    while (true)
    {
        u32 funcAddr = readSaturnU32(entry);
        if (funcAddr == 0) break;

        switch (funcAddr)
        {
        case 0x0605B270: createEnvObject_C8_type0(gridCell, entry, value); break;
        case 0x0605B294: createEnvObject_C8_type0b(gridCell, entry, value); break;
        case 0x0605B2B8: createEnvObject_C8_type1(gridCell, entry, value); break;
        case 0x0605B2DC: createEnvObject_C8_type1b(gridCell, entry, value); break;
        case 0x0605B300: createEnvObject_C8_type2(gridCell, entry, value); break;
        case 0x0605B324: createEnvObject_C8_type2b(gridCell, entry, value); break;
        case 0x06055CD4: createCellEntity_C8_06055cd4(gridCell, entry, value); break;
        case 0x06055CF4: createCellEntity_C8_06055cf4(gridCell, entry, value); break;
        case 0x06055D14: createCellEntity_C8_06055d14(gridCell, entry, value); break;
        case 0x0605A008: createCellEntity_C8_0605a008(gridCell, entry, value); break;
        case 0x0605A6C0: createCellEntity_C8_0605a6c0(gridCell, entry, value); break;
        case 0x0605A6E0: createCellEntity_C8_0605a6e0(gridCell, entry, value); break;
        case 0x0605A700: createCellEntity_C8_0605a700(gridCell, entry, value); break;
        case 0x0605A720: createCellEntity_C8_0605a720(gridCell, entry, value); break;
        case 0x0605A740: createCellEntity_C8_0605a740(gridCell, entry, value); break;
        case 0x0605E59C: createCellEntity_C8_0605e59c(gridCell, entry, value); break;
        case 0x0607A414: createCellEntity_C8_0607a414(gridCell, entry, value); break;
        default:
            PDS_Log("C8: unknown cell entity func 0x%08X", funcAddr);
            break;
        }

        entry = entry + 0x20; // stride 32 bytes
    }
}

// 06079e26 — setup field with data tables and per-cell visibility registration
static void setupFieldC8(sSaturnPtr dataTable3EA, sSaturnPtr visibilityEA, void(*createFunc)(p_workArea))
{
    s_DataTable3* pDT3 = readDataTable3(dataTable3EA);
    setupField2(pDT3, createFunc);

    // Per-cell visibility: visibilityEA points to {ptrArray, value, width, height}
    // Iterates width*height cells, calling per-cell function pointer lists
    if (!visibilityEA.isNull())
    {
        s32 width = readSaturnS32(visibilityEA + 8);
        s32 height = readSaturnS32(visibilityEA + 12);
        sSaturnPtr ptrArray = readSaturnEA(visibilityEA);
        s32 value = readSaturnS32(visibilityEA + 4);

        s_visibilityGridWorkArea* pGrid = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;
        s32 count = width * height;
        for (s32 i = 0; i < count; i++)
        {
            sSaturnPtr cellDataEA = readSaturnEA(ptrArray + i * 4);
            if (!cellDataEA.isNull())
            {
                // 06079dfe — iterate function pointer list for this cell
                dispatchCellEntities_C8(pGrid->m3C_cellRenderingTasks[i], cellDataEA, value);
            }
        }
    }
}

// 06055bb4 — camera config
static void setupCameraConfig_C8_0()
{
    setupFieldCameraConfigs(readCameraConfig(gFLD_C8->getSaturnPtr(0x0608AF64)), 1);
}

// 06054210 — clip distances and overlay init
static void initClipAndOverlay_C8(p_workArea workArea)
{
    graphicEngineStatus.m405C.m10_nearClipDistance = 0x3000;
    graphicEngineStatus.m405C.m30_oneOverNearClip = FP_Div(0x10000, graphicEngineStatus.m405C.m10_nearClipDistance);
    graphicEngineStatus.m405C.m14_farClipDistance = 0x200000;
    graphicEngineStatus.m405C.m38_oneOverFarClip = FP_Div(0x8000, graphicEngineStatus.m405C.m14_farClipDistance);
    graphicEngineStatus.m405C.m34_oneOverFarClip256 = graphicEngineStatus.m405C.m38_oneOverFarClip << 8;
    Unimplemented(); // FUN_FLD_C8__06078a26 — sets m12FC clipping function
    getFieldTaskPtr()->m8_pSubFieldData->m334->m50E = 2;
    fieldRadar_setEncounterDistance(fixedPoint(0x12C000));
}

// 06057f1a — set field-specific data flags
static void setFieldDataFlags_C8()
{
    s_fieldSpecificData_C8* pFieldData = (s_fieldSpecificData_C8*)getFieldTaskPtr()->mC;
    if (pFieldData)
    {
        pFieldData->m25 = 0;
        pFieldData->m27 = 1;
    }
}

// 0605cbec — empty function
static void emptyFunc_C8() {}

// 0605e678 — push dragon away from tower center if too close
static void dragonTowerPushback(s_dragonTaskWorkArea* pDragon, sSaturnPtr towerCenterEA, fixedPoint minDistance)
{
    sVec3_FP towerCenter;
    towerCenter.m0_X = readSaturnS32(towerCenterEA);
    towerCenter.m4_Y = pDragon->m8_pos.m4_Y;
    towerCenter.m8_Z = readSaturnS32(towerCenterEA + 8);

    fixedPoint dist = vecDistance(pDragon->m8_pos, towerCenter);
    fixedPoint pushDist = dist - minDistance;

    fixedPoint pushX, pushZ;
    if ((s32)pushDist < 0)
    {
        pushX = 0;
        pushZ = 0;
    }
    else
    {
        fixedPoint diffX = pDragon->m8_pos.m0_X - towerCenter.m0_X;
        fixedPoint diffZ = pDragon->m8_pos.m8_Z - towerCenter.m8_Z;
        pushX = performDivision(dist, MTH_Mul(pushDist, diffX));
        pushZ = performDivision(dist, MTH_Mul(pushDist, diffZ));
    }

    pDragon->m160_deltaTranslation[0] = pDragon->m160_deltaTranslation[0] - pushX;
    // m164 unchanged
    pDragon->m160_deltaTranslation[2] = pDragon->m160_deltaTranslation[2] - pushZ;
    pDragon->m3C = pDragon->m20_angle;
}

// 0605e7be — tower-specific dragon update
static void dragonUpdateC8(s_dragonTaskWorkArea* pDragon)
{
    dragonTowerPushback(pDragon, gFLD_C8->getSaturnPtr(0x060B29B4), 0xA1000);
}

// 0607d684 — setupDragonPosition from Saturn data
static void setupDragonPositionFromSaturn(sSaturnPtr posEA, sSaturnPtr rotEA)
{
    sVec3_FP pos = { readSaturnS32(posEA), readSaturnS32(posEA + 4), readSaturnS32(posEA + 8) };
    sVec3_FP rot = { readSaturnS32(rotEA), readSaturnS32(rotEA + 4), readSaturnS32(rotEA + 8) };
    setupDragonPosition(&pos, &rot);
}

// 060542e6 — cutscene start based on entry point
static void startCutsceneFromEntryPoint_C8(sSaturnPtr data)
{
    Unimplemented();
}

// 0607baa0 — random battle init
static void initRandomBattle_C8(s32 scriptIndex)
{
    Unimplemented();
}

// 06054460
void subfieldC8_0(p_workArea workArea)
{
    createVdp2Task_C8_0(workArea);
    setupFieldC8(gFLD_C8->getSaturnPtr(0x0608FDE4), gFLD_C8->getSaturnPtr(0x0608F414), fieldC8_0_startTasks);
    adjustVerticalLimits(0x6C000, 0xF91000);
    setupCameraConfig_C8_0();
    initClipAndOverlay_C8(workArea);
    setFieldDataFlags_C8();
    emptyFunc_C8();
    getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->mF4 = dragonUpdateC8;
    setupDragonPositionFromSaturn(gFLD_C8->getSaturnPtr(0x0608A868), gFLD_C8->getSaturnPtr(0x0608A874));

    if (getFieldTaskPtr()->m30_fieldEntryPoint < 0)
    {
        setupDragonPositionFromSaturn(gFLD_C8->getSaturnPtr(0x0608AA00), gFLD_C8->getSaturnPtr(0x0608AA0C));
    }
    else
    {
        startCutsceneFromEntryPoint_C8(gFLD_C8->getSaturnPtr(0x0608A880));
    }

    initRandomBattle_C8(0x32);
}
