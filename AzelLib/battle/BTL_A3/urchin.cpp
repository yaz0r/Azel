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

void Baldor_updateSub1(sVec3_FP* pCurrent, sVec3_FP* pDelta, sVec3_FP* pTarget, s32 pDeltaFactor, s32 pDistanceToTargetFactor, s8 translationOrRotation); // TODO: cleanup

struct sUrchin : public s_workAreaTemplateWithArgWithCopy<sUrchin, sGenericFormationPerTypeData*>
{
    sVec3_FP m8;
    sVec3_FP m14_positionCurrent;
    sVec3_FP m20_positionTarget;
    sVec3_FP m2C_positionDelta;
    sVec3_FP m38_rotationCurrent;
    sVec3_FP m44_rotationTarget;
    sVec3_FP m50_rotationDelta;
    s_3dModel m5C_model;
    s8 mAC;
    s8 mAD;
    s8 mAE;
    s8 mAF;
    s8 mB0;
    s8 mB4;
    s16 mB6_numTargetables;
    std::vector<sBattleTargetable> mC0_targetable;
    std::vector<sVec3_FP> mC4_position;
    p_workArea mC8;
    sGenericFormationPerTypeData* mCC;
    sBTL_A3_UrchinFormation_18* mD0;
    p_workArea mD4;
    p_workArea mD8;
    //size 0xDC
};

void Baldor_initSub0Sub1(p_workArea pThis, s_3dModel* pModel, s16* param3, std::vector<sBattleTargetable>& param4, std::vector<sVec3_FP>& param5); // TODO: cleanup
p_workArea createBaldorSubTask0(sVec3_FP* arg0, s32 arg1, s8* arg2, s8 arg3); // TODO: cleanup

bool updateUrchinAnimationSequence(sUrchin* pThis, u16 param_2, int param_3, int param_4, int param_5)
{
    if (param_3)
    {
        int numFramesInAnimation;
        if (pThis->m5C_model.m30_pCurrentAnimation == nullptr)
        {
            numFramesInAnimation = 0;
        }
        else
        {
            numFramesInAnimation = pThis->m5C_model.m30_pCurrentAnimation->m4_numFrames;
        }
        if (pThis->m5C_model.m16_previousAnimationFrame < numFramesInAnimation - 1)
        {
            return false;
        }
    }

    if (param_2 == 0)
    {
        if (param_4)
        {
            riderInit(&pThis->m5C_model, nullptr);
        }
    }
    else
    {
        riderInit(&pThis->m5C_model, pThis->m0_dramAllocation->getAnimation(param_2));
        if (param_5)
        {
            int numFrameToSkip = randomNumber() & 0x1F;
            for (int i = 0; i < numFrameToSkip; i++)
            {
                stepAnimation(&pThis->m5C_model);
            }
        }
    }
}

void Urchin_init(sUrchin* pThis, sGenericFormationPerTypeData* pConfig)
{
    pThis->mCC = pConfig;

    sModelHierarchy* pHierarchy = pThis->m0_dramAllocation->getModelHierarchy(pConfig->m8);
    sStaticPoseData* pStaticPose = pThis->m0_dramAllocation->getStaticPose(pConfig->mA, pHierarchy->countNumberOfBones());

    init3DModelRawData(pThis, &pThis->m5C_model, 0, pThis->m0_dramAllocation, pConfig->m8, nullptr, pStaticPose, nullptr, pConfig->mC);
    Baldor_initSub0Sub1(pThis, &pThis->m5C_model, &pThis->mB6_numTargetables, pThis->mC0_targetable, pThis->mC4_position);
    updateUrchinAnimationSequence(pThis, readSaturnS16(pConfig->m1C + 0x1C), 0, 1, 1);
    pThis->mC8 = createBaldorSubTask0(&pThis->m8, 0, &pThis->mB4, pConfig->m0);

    if (!readSaturnEA(pConfig->m1C).isNull())
    {
        assert(0);
    }

    if ((gBattleManager->m4 == 8) && (gBattleManager->m6_subBattleId == 4)) {
        gBattleManager->m10_battleOverlay->m4_battleEngine->m22F_battleRadarLockIcon =  0xB;
    }
}

void Urchin_update(sUrchin* pThis)
{
    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x1B])
    {
        assert(0);
    }

    pThis->m8 = gBattleManager->m10_battleOverlay->m4_battleEngine->mC_battleCenter + pThis->m14_positionCurrent;
    pThis->mD0->m14[pThis->mAE].m0 = pThis->m8;

    if ((pThis->mAC == 6) && (pThis->mAD == 6))
    {
        assert(0);
    }

    if ((pThis->mD0->m14[pThis->mAE].m18 & 4) == 0)
    {
        if (pThis->mCC->m2)
        {
            pThis->mD0->m10 = 1;
        }

        if (pThis->mD0->m12 & 1)
        {
            assert(0);
        }

        if (pThis->mD0->m12 & 6)
        {
            assert(0);
        }

        stepAnimation(&pThis->m5C_model);

        if (pThis->mD8)
        {
            assert(0);
        }

        if (pThis->mD4)
        {
            assert(0);
        }

        switch (pThis->mAC)
        {
        case 0:
            Baldor_updateSub1(&pThis->m14_positionCurrent, &pThis->m2C_positionDelta, &pThis->m20_positionTarget, 0x1999, 0x28F, 0);

            switch (gBattleManager->m4)
            {
            case 5:
                assert(0);
            case 8:
                assert(0);
            default:
                Baldor_updateSub1(&pThis->m38_rotationCurrent, &pThis->m50_rotationDelta, &pThis->m44_rotationTarget, 0x1999, 0x28F, 1);
                break;
            }

            break;
        default:
            assert(0);
        }

        FunctionUnimplemented();
    }
}

void Urchin_draw(sUrchin* pThis)
{
    pushCurrentMatrix();
    translateCurrentMatrix(pThis->m8);
    rotateCurrentMatrixYXZ(pThis->m38_rotationCurrent);
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

void createUrchin(sGenericFormationPerTypeData* pConfig, sBTL_A3_UrchinFormation_18& param2, int param3, int param4)
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

    pNewTask->m14_positionCurrent = iVar4->mC;
    pNewTask->m20_positionTarget = iVar4->mC;

    switch (param2.mD[pNewTask->mAF])
    {
    case 0:
        pNewTask->m38_rotationCurrent[1] = 0;
        break;
    case 1:
        pNewTask->m38_rotationCurrent[1] = 0x4000000;
        break;
    case 2:
        pNewTask->m38_rotationCurrent[1] = 0x8000000;
        break;
    case 3:
        pNewTask->m38_rotationCurrent[1] = 0xC000000;
        break;
    default:
        assert(0);
    }

    pNewTask->m44_rotationTarget = pNewTask->m38_rotationCurrent;

    iVar4->m18 &= ~4;
}
