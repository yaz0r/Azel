#include "PDS.h"
#include "urchin.h"
#include "BTL_A3_UrchinFormation.h"
#include "kernel/fileBundle.h"
#include "kernel/animation.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "battle/battleEngine.h"
#include "battle/battleDebug.h"
#include "battle/battleTargetable.h"
#include "town/town.h" //TODO: cleanup

struct sUrchin : public s_workAreaTemplateWithArgWithCopy<sUrchin, sUrchinFormationDataSub*>
{
    sVec3_FP m8;
    sVec3_FP m14;
    sVec3_FP m20;
    sVec3_FP m38;
    sVec3_FP m44;
    s_3dModel m5C_model;
    s8 mAE;
    s8 mAF;
    s8 mB0;
    s16 mB6_numTargetables;
    std::vector<sBattleTargetable> mC0_targetable;
    std::vector<sVec3_FP> mC4_position;
    sUrchinFormationDataSub* mCC;
    const sBTL_A3_UrchinFormation_18* mD0;
    //size 0xDC
};

void Baldor_initSub0Sub1(p_workArea pThis, s_3dModel* pModel, s16* param3, std::vector<sBattleTargetable>& param4, std::vector<sVec3_FP>& param5); // TODO: cleanup

void Urchin_init(sUrchin* pThis, sUrchinFormationDataSub* pConfig)
{
    pThis->mCC = pConfig;

    sModelHierarchy* pHierarchy = pThis->m0_dramAllocation->getModelHierarchy(pConfig->m8);
    sStaticPoseData* pStaticPose = pThis->m0_dramAllocation->getStaticPose(pConfig->mA, pHierarchy->countNumberOfBones());

    init3DModelRawData(pThis, &pThis->m5C_model, 0, pThis->m0_dramAllocation, pConfig->m8, nullptr, pStaticPose, nullptr, pConfig->mC);
    Baldor_initSub0Sub1(pThis, &pThis->m5C_model, &pThis->mB6_numTargetables, pThis->mC0_targetable, pThis->mC4_position);
    FunctionUnimplemented();
}

void Urchin_update(sUrchin* pThis)
{
    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x1B])
    {
        assert(0);
    }

    pThis->m8 = gBattleManager->m10_battleOverlay->m4_battleEngine->mC_battleCenter + pThis->m14;

    FunctionUnimplemented();
}

void Urchin_draw(sUrchin* pThis)
{
    pushCurrentMatrix();
    translateCurrentMatrix(pThis->m8);
    translateCurrentMatrix(pThis->m38);
    if (pThis->mB0 & 2)
    {
        assert(0);
    }
    pThis->m5C_model.m18_drawFunction(&pThis->m5C_model);
    if (pThis->mB0 & 2)
    {
        assert(0);
    }
    popMatrix();
}

void Urchin_delete(sUrchin* pThis)
{
    FunctionUnimplemented();
}

void createUrchin(sUrchinFormationDataSub* pConfig, sBTL_A3_UrchinFormation_18& param2, int param3, int param4)
{
    static const sUrchin::TypedTaskDefinition definition = {
        Urchin_init,
        Urchin_update,
        Urchin_draw,
        Urchin_delete,
    };

    sUrchin* pNewTask = createSiblingTaskWithArgWithCopy<sUrchin>(dramAllocatorEnd[pConfig->m1].mC_buffer, pConfig, &definition);

    pNewTask->mD0 = &param2;
    pNewTask->mAE = param3;
    pNewTask->mAF = param4;

    sBTL_A3_UrchinFormation_18_14* iVar4 = &param2.m14[pNewTask->mAE];
    iVar4->m18 |= param4;

    pNewTask->m14 = iVar4->mC;
    pNewTask->m20 = iVar4->mC;

    switch (param2.mD[pNewTask->mAF])
    {
    case 0:
        pNewTask->m38[1] = 0;
        break;
    case 1:
        pNewTask->m38[1] = 0x4000000;
        break;
    case 2:
        pNewTask->m38[1] = 0x8000000;
        break;
    case 3:
        pNewTask->m38[1] = 0xC000000;
        break;
    default:
        assert(0);
    }

    pNewTask->m44 = pNewTask->m38;

    iVar4->m18 &= ~4;
}
