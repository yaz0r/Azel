#include "PDS.h"
#include "kernel/animation.h"
#include "kernel/fileBundle.h"

void modeDrawFunction6Sub2(sModelHierarchy* pModelData, std::vector<sPoseData>::iterator& pPoseData, std::vector<s_hotpointDefinition>::iterator& r6, std::vector<std::vector<sVec3_FP>>::iterator& r7)
{
    PDS_unimplemented("modeDrawFunction6Sub2");
}

void modeDrawFunction1Sub2(sModelHierarchy* pModelData, std::vector<sMatrix4x3>::iterator& r5, std::vector<s_hotpointDefinition>::iterator& r6, std::vector<std::vector<sVec3_FP>>::iterator& r7)
{
    assert(0);
}

void resetAnimation(s_3dModel* r4)
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

void LCSItemBox_DrawType0Sub0Sub0(sModelHierarchy* pHierarchy, std::vector<sStaticPoseData::sBonePoseData>::const_iterator& r5)
{
    do
    {
        pushCurrentMatrix();

        translateCurrentMatrix(&r5->m0_translation);
        rotateCurrentMatrixZYX(&r5->mC_rotation);

        if (pHierarchy->m0_3dModel)
        {
            addObjectToDrawList(pHierarchy->m0_3dModel);
        }
        if (pHierarchy->m4_subNode)
        {
            r5++;
            LCSItemBox_DrawType0Sub0Sub0(pHierarchy->m4_subNode, r5);
        }

        popMatrix();

        if (pHierarchy->m8_nextNode == nullptr)
            break;

        r5++;
        pHierarchy = pHierarchy->m8_nextNode;

    } while (1);
}

void LCSItemBox_DrawType0Sub0(s_fileBundle* r4, s16 r5, s16 r6)
{
    sModelHierarchy* pHierarchy = r4->getModelHierarchy(r5);
    sStaticPoseData* pStaticPose = r4->getStaticPose(r6, pHierarchy->countNumberOfBones());

    std::vector<sStaticPoseData::sBonePoseData>::const_iterator bones = pStaticPose->m0_bones.begin();
    LCSItemBox_DrawType0Sub0Sub0(pHierarchy, bones);
}

bool model_initHotpointBundle(s_3dModel* pDragonStateData1, sHotpointBundle* unkArg)
{
    pDragonStateData1->m40 = unkArg->getData(pDragonStateData1);

    pDragonStateData1->m44_hotpointData.resize(pDragonStateData1->m12_numBones);

    std::vector<s_hotpointDefinition>::iterator r12 = pDragonStateData1->m40->begin();

    for (u32 i = 0; i < pDragonStateData1->m12_numBones; i++)
    {
        if (r12->m4_count > 0)
        {
            pDragonStateData1->m44_hotpointData[i].resize(r12->m4_count);
        }
        else
        {
            pDragonStateData1->m44_hotpointData[i].resize(0);
        }

        r12++;
    }

    return true;
}

sHotpointBundle* readRiderDefinitionSub(sSaturnPtr ptrEA)
{
    if (ptrEA.isNull())
    {
        return nullptr;
    }

    return new sHotpointBundle(ptrEA);
}
