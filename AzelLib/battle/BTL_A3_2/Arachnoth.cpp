#include "PDS.h"
#include "Arachnoth.h"
#include "battle/battleManager.h"
#include "battle/battleEngine.h"
#include "battle/battleEnemyLifeMeter.h"
#include "battle/battleTextDisplay.h"
#include "BTL_A3_2_data.h"
#include "kernel/fileBundle.h"
#include "kernel/animation.h"
#include "kernel/graphicalObject.h"
#include "ArachnothSubPart.h"

// https://www.youtube.com/watch?v=Txks9hG21qs&feature=youtu.be&t=3130

struct sArachnothFormation : public s_workAreaTemplateWithCopy<sArachnothFormation>
{
    sArachnothSubModel m8;
    sArachnothSubModel m98;
    sArachnothSubModel m128;
    sArachnothSubModel* m1B8;
    s_3dModel m1BC_3dModel; // -> 0x208
    sVec3_FP m20C;
    sVec3_FP m218;
    sVec3_FP m224_translation;
    sVec3_FP m230;
    s32 m23C;
    s32 m240;
    sVec3_FP m254;
    sVec3_FP m260;
    sVec3_FP m26C_rotation;
    s32 m278;
    sVec3_FP m27C;
    sVec3_FP m288;
    s16 m298_life;
    sEnemyLifeMeterTask* m29C_lifeMeter;
    s32 m2A0;
    s32 m2A4;
    s32 m2A8;
    s32 m2AC;
    s32 m2B0;
    s32 m2B4;
    std::array<fixedPoint, 6> m314;
    std::array<s32, 4> m344;
    //size 0x34C
};

void arachnothFormation_update(sArachnothFormation* pThis)
{
    s32 damageTaken = arachnothSubPartGetDamage(&pThis->m8) + arachnothSubPartGetDamage(&pThis->m98);
    if (damageTaken > 0)
    {
        assert(0);
    }

    if (pThis->m2A0)
    {
        pThis->m2A0--;
    }

    if (pThis->m8.m6C || pThis->m98.m6C)
    {
        pThis->m2A4 = 1;
    }

    if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m1000 && (pThis->m2A4 == 1))
    {
        assert(0);
    }

    if (pThis->m1B8->m74 == 1)
    {
        int var5 = stepAnimation(&pThis->m1B8->m8_model);
        if (var5 == pThis->m1B8->m7C)
        {
            pThis->m1B8->m74 = 0;
        }
    }

    if (pThis->m98.m74 == 1)
    {
        int var5 = stepAnimation(&pThis->m98.m8_model);
        if (var5 == pThis->m98.m7C)
        {
            pThis->m98.m74 = 0;
        }
    }

    if (pThis->m2B4 > 0)
    {
        pThis->m2B4--;
        if (pThis->m2B4 == 0)
        {
            assert(0);
        }
    }

    gBattleManager->m10_battleOverlay->m4_battleEngine->m3B2_numBattleFormationRunning++;

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

    pushCurrentMatrix();
    translateCurrentMatrix(pThis->m224_translation);
    rotateCurrentMatrixYXZ(pThis->m26C_rotation);
    pThis->m98.m8_model.m18_drawFunction(&pThis->m98.m8_model);
    popMatrix();

}

void arachnothUpdateQuadrants(sArachnothFormation* pThis)
{
    FunctionUnimplemented();
}

void createArachnothFormation(s_workAreaCopy* pParent, u32 arg0, u32 arg1)
{
    sArachnothFormation::TypedTaskDefinition definition = {
        nullptr,
        arachnothFormation_update,
        arachnothFormation_draw,
        nullptr,
    };

    sArachnothFormation* pThis = createSubTaskWithCopy<sArachnothFormation>(pParent, &definition);
    pThis->m344.fill(0);
    allocateNPC(pThis, 8);
    pThis->m0_fileBundle = dramAllocatorEnd[8].mC_fileBundle->m0_fileBundle;

    arachnothCreateSubModel(&pThis->m8, pThis, dramAllocatorEnd[8].mC_fileBundle, arg1, g_BTL_A3_2->getSaturnPtr(0x60A9578));
    arachnothInitSubModelAnimation(&pThis->m8, 0, -1);
    arachnothInitSubModelFunctions(&pThis->m8, 0, arachnothSubModelFunction0, arachnothSubModelFunction1, arachnothSubModelFunction2);

    arachnothCreateSubModel2(&pThis->m98, pThis, dramAllocatorEnd[8].mC_fileBundle, arg1, g_BTL_A3_2->getSaturnPtr(0x60A9584), g_BTL_A3_2->getSaturnPtr(0x60A9538), g_BTL_A3_2->getSaturnPtr(0x60A9540));
    arachnothInitSubModelAnimation(&pThis->m98, 0, -1);
    arachnothInitSubModelFunctions(&pThis->m98, 0, arachnothSubModelFunction0, arachnothSubModelFunction1, arachnothSubModelFunction2);

    arachnothCreateSubModel(&pThis->m128, pThis, dramAllocatorEnd[8].mC_fileBundle, arg1, g_BTL_A3_2->getSaturnPtr(0x60A9590));
    arachnothInitSubModelAnimation(&pThis->m128, 0, -1);
    arachnothInitSubModelFunctions(&pThis->m128, 0, arachnothSubModelFunction0, arachnothSubModelFunction1, arachnothSubModelFunction2);

    sModelHierarchy* pHierarchy = pThis->m0_fileBundle->getModelHierarchy(0x18);
    sStaticPoseData* pStaticPose = pThis->m0_fileBundle->getStaticPose(0x1CC, pHierarchy->countNumberOfBones());
    init3DModelRawData(pThis, &pThis->m1BC_3dModel, 0, pThis->m0_fileBundle, 0x18, nullptr, pStaticPose, nullptr, nullptr);

    pThis->m230[0] = 0x201000;
    pThis->m230[1] = 0x13000;
    pThis->m230[2] = -0x1D2000;

    pThis->m20C.zeroize();
    pThis->m218.zeroize();

    pThis->m224_translation = pThis->m230;

    pThis->m254.zeroize();
    pThis->m260.zeroize();
    pThis->m26C_rotation.zeroize();

    pThis->m26C_rotation[1] = 0x8000000;
    
    pThis->m288.zeroize();

    pThis->m298_life = 2500;

    pThis->m29C_lifeMeter = createEnemyLifeMeterTask(&pThis->m224_translation, 0, &pThis->m298_life, 7);

    pThis->m2A0 = 0;
    pThis->m2A4 = 0;
    pThis->m2A8 = 0;

    pThis->m1B8 = &pThis->m8;

    pThis->m2AC = 0;
    pThis->m2B0 = 0;
    pThis->m2B4 = 0;
    pThis->m27C.zeroize();
    pThis->m27C[1] = 0x8000000;

    pThis->m314.fill(0);

    pThis->m278 = 2;
    pThis->m23C = 1;
    pThis->m240 = 0x201000;

    createArachnothFormationSub0(&pThis->m8, pThis->m278);
    createArachnothFormationSub0(&pThis->m98, pThis->m278);
    arachnothUpdateQuadrants(pThis);
    gBattleManager->m10_battleOverlay->m4_battleEngine->m22F_battleRadarLockIcon = 1;
    displayFormationName(0, 1, 11);
}
