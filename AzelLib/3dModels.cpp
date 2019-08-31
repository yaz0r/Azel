#include "PDS.h"
#include "kernel/animation.h"
#include "kernel/fileBundle.h"

void modeDrawFunction6Sub2(sModelHierarchy* pModelData, std::vector<sPoseData>::iterator& pPoseData, const s_RiderDefinitionSub*& r6, std::vector<std::vector<sVec3_FP>>::iterator& r7)
{
    PDS_unimplemented("modeDrawFunction6Sub2");
}

void modeDrawFunction1Sub2(sModelHierarchy* pModelData, std::vector<sMatrix4x3>::iterator& r5, const s_RiderDefinitionSub*& r6, std::vector<std::vector<sVec3_FP>>::iterator& r7)
{
    assert(0);
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

    LCSItemBox_DrawType0Sub0Sub0(pHierarchy, pStaticPose->m0_bones.begin());
}
