#include "PDS.h"
#include "Arachnoth.h"
#include "kernel/fileBundle.h"
#include "kernel/animation.h"
#include "kernel/graphicalObject.h"

// https://www.youtube.com/watch?v=Txks9hG21qs&feature=youtu.be&t=3130

struct sArachnothSubModel
{
    //size 0x90
};

struct sArachnothFormation : public s_workAreaTemplateWithCopy<sArachnothFormation>
{
    sArachnothSubModel m8;
    sArachnothSubModel m98;
    sArachnothSubModel m128;
    std::array<s32,4> m344;
    s_3dModel m1BC_3dModel; // -> 0x208
    sVec3_FP m224_translation;
    sVec3_FP m230;
    sVec3_FP m26C_rotation;
    //size 0x34C
};

void arachnothFormation_update(sArachnothFormation* pThis)
{
    FunctionUnimplemented();
}

void arachnothFormation_draw(sArachnothFormation* pThis)
{
    FunctionUnimplemented();

    pushCurrentMatrix();
    translateCurrentMatrix(pThis->m224_translation);
    rotateCurrentMatrixYXZ(pThis->m26C_rotation);
    //pThis->m1BC_3dModel.m1C_addToDisplayListFunction(pThis->m1BC_3dModel.m4_pModelFile->getModelHierarchy(0x18)->m0_3dModel);
    pThis->m1BC_3dModel.m18_drawFunction(&pThis->m1BC_3dModel);
    popMatrix();

    FunctionUnimplemented();
}

void arachnothCreateSubModel(sArachnothSubModel* pThis, p_workArea pParent, s_fileBundle* param_3, s8 param_4, void* param_5)
{
    FunctionUnimplemented();
}

void arachnothCreateSubModel2(sArachnothSubModel* pThis, p_workArea pParent, s_fileBundle* param_3, s8 param_4, void* param_5, void* param_6, void* param_7)
{
    FunctionUnimplemented();
}

void arachnothInitSubModelAnimation(sArachnothSubModel* pThis, s32, s32)
{
    FunctionUnimplemented();
}

void arachnothInitSubModelFunctions(sArachnothSubModel* pThis, s32, void (*param_2)(sArachnothSubModel*, s32), void (*param_3)(sArachnothSubModel*, s32), void (*param_4)(sArachnothSubModel*, s32))
{
    FunctionUnimplemented();
}

void arachnothSubModelFunction0(sArachnothSubModel* pThis, s32) { FunctionUnimplemented(); }
void arachnothSubModelFunction1(sArachnothSubModel* pThis, s32) { FunctionUnimplemented(); }
void arachnothSubModelFunction2(sArachnothSubModel* pThis, s32) { FunctionUnimplemented(); }

u8* pArachnothSubModelData1 = nullptr;
u8* pArachnothSubModelData2 = nullptr;
u8* pArachnothSubModelData3 = nullptr;
u8* pArachnothSubModelData4 = nullptr;
u8* pArachnothSubModelData5 = nullptr;

void createArachnothFormation(s_workAreaCopy* pParent, u32 arg0, u32 arg1)
{
    sArachnothFormation::TypedTaskDefinition definition = {
        nullptr,
        arachnothFormation_update,
        arachnothFormation_draw,
        nullptr,
    };

    sArachnothFormation* pNewTask = createSubTaskWithCopy<sArachnothFormation>(pParent, &definition);
    pNewTask->m344.fill(0);
    allocateNPC(pNewTask, 8);
    pNewTask->m0_fileBundle = dramAllocatorEnd[8].mC_fileBundle->m0_fileBundle;

    arachnothCreateSubModel(&pNewTask->m8, pNewTask, pNewTask->m0_fileBundle, arg1, &pArachnothSubModelData1);
    arachnothInitSubModelAnimation(&pNewTask->m8, 0, -1);
    arachnothInitSubModelFunctions(&pNewTask->m8, 0, arachnothSubModelFunction0, arachnothSubModelFunction1, arachnothSubModelFunction2);

    arachnothCreateSubModel2(&pNewTask->m98, pNewTask, pNewTask->m0_fileBundle, arg1, &pArachnothSubModelData2, &pArachnothSubModelData3, &pArachnothSubModelData4);
    arachnothInitSubModelAnimation(&pNewTask->m98, 0, -1);
    arachnothInitSubModelFunctions(&pNewTask->m98, 0, arachnothSubModelFunction0, arachnothSubModelFunction1, arachnothSubModelFunction2);

    arachnothCreateSubModel(&pNewTask->m128, pNewTask, pNewTask->m0_fileBundle, arg1, &pArachnothSubModelData5);
    arachnothInitSubModelAnimation(&pNewTask->m128, 0, -1);
    arachnothInitSubModelFunctions(&pNewTask->m128, 0, arachnothSubModelFunction0, arachnothSubModelFunction1, arachnothSubModelFunction2);

    sModelHierarchy* pHierarchy = pNewTask->m0_fileBundle->getModelHierarchy(0x18);
    sStaticPoseData* pStaticPose = pNewTask->m0_fileBundle->getStaticPose(0x1CC, pHierarchy->countNumberOfBones());
    init3DModelRawData(pNewTask, &pNewTask->m1BC_3dModel, 0, pNewTask->m0_fileBundle, 0x18, nullptr, pStaticPose, nullptr, nullptr);

    pNewTask->m230[0] = 0x201000;
    pNewTask->m230[1] = 0x13000;
    pNewTask->m230[2] = -0x1D2000;

    FunctionUnimplemented();

    pNewTask->m224_translation = pNewTask->m230;

    FunctionUnimplemented();
}
