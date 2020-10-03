#include "PDS.h"
#include "animation.h"
#include "processModel.h"

sAnimationData::sAnimationData(u8* base, u32 offset)
{
    u8* pData = base + offset;
    m0_flags = READ_BE_U16(pData); pData += 2;
    if (m0_flags == 0)
        return;

    m2_numBones = READ_BE_U16(pData); pData += 2;
    m4_numFrames = READ_BE_U16(pData); pData += 2;
    pData += 2; //?
    m8_offsetToTrackHeader = READ_BE_U32(pData); pData += 4;

    for(int boneId = 0; boneId < m2_numBones; boneId++)
    {
        pData = m8_offsetToTrackHeader + base + offset + 0x38 * boneId;
        sTrackHeader perBoneData;
        for (int i = 0; i < 9; i++)
        {
            perBoneData.m0_tracksLength[i] = READ_BE_S16(pData); pData += 2;
        }
        pData += 2; //padding?
        for (int i = 0; i < 9; i++)
        {
            perBoneData.m14_trackDataOffset[i] = READ_BE_U32(pData); pData += 4;
        }

        for (int i = 0; i < 9; i++)
        {
            pData = perBoneData.m14_trackDataOffset[i] + base + offset;
            for (int j = 0; j < perBoneData.m0_tracksLength[i]; j++)
            {
                perBoneData.m14_trackData[i].push_back(READ_BE_S16(pData)); pData += 2;
            }
        }

        m8_trackHeader.push_back(perBoneData);
    }
}

u32 sModelHierarchy::countNumberOfBones()
{
    u32 count = 1;
    if (m4_subNode)
    {
        count += m4_subNode->countNumberOfBones();
    }
    if (m8_nextNode)
    {
        count += m8_nextNode->countNumberOfBones();
    }

    return count;
}

void sModelHierarchy::patchFilePointers(u32 offset)
{
    if (m0_3dModel)
    {
        m0_3dModel->patchFilePointers(offset);
    }
    if (m4_subNode)
    {
        m4_subNode->patchFilePointers(offset);
    }
    if (m8_nextNode)
    {
        m8_nextNode->patchFilePointers(offset);
    }
}

sStaticPoseData::sStaticPoseData(u8* base, u32 offset, u32 numBones)
{
    base += offset;
    for (int i = 0; i < numBones; i++)
    {
        sStaticPoseData::sBonePoseData newPoseData;
        newPoseData.m0_translation[0] = READ_BE_S32(base); base += 4;
        newPoseData.m0_translation[1] = READ_BE_S32(base); base += 4;
        newPoseData.m0_translation[2] = READ_BE_S32(base); base += 4;

        newPoseData.mC_rotation[0] = READ_BE_S32(base); base += 4;
        newPoseData.mC_rotation[1] = READ_BE_S32(base); base += 4;
        newPoseData.mC_rotation[2] = READ_BE_S32(base); base += 4;

        newPoseData.m18_scale[0] = READ_BE_S32(base); base += 4;
        newPoseData.m18_scale[1] = READ_BE_S32(base); base += 4;
        newPoseData.m18_scale[2] = READ_BE_S32(base); base += 4;

        m0_bones.push_back(newPoseData);
    }
}

void copyPosePosition(s_3dModel* pModel)
{
    sStaticPoseData* r5 = pModel->m34_pDefaultPose;

    for (u32 i = 0; i < pModel->m12_numBones; i++)
    {
        pModel->m2C_poseData[i].m0_translation = r5->m0_bones[i].m0_translation;
    }
}

void copyPoseRotation(s_3dModel* pModel)
{
    sStaticPoseData* r5 = pModel->m34_pDefaultPose;

    for (u32 i = 0; i < pModel->m12_numBones; i++)
    {
        pModel->m2C_poseData[i].mC_rotation = r5->m0_bones[i].mC_rotation;
    }
}

void resetPoseScale(s_3dModel* pModel)
{
    std::vector<sPoseData>& pOutputPose = pModel->m2C_poseData;

    for (u32 i = 0; i < pModel->m12_numBones; i++)
    {
        pOutputPose[i].m18_scale[0] = 0x10000;
        pOutputPose[i].m18_scale[1] = 0x10000;
        pOutputPose[i].m18_scale[2] = 0x10000;
    }
}

s32 stepAnimationTrack(sAnimTrackStatus& r4, const std::vector<s16>& r5, u16 maxStep)
{
    assert(r5.size() == maxStep);

    if (r4.delay > 0)
    {
        r4.delay -= 1;
        return r4.value;
    }

    if (r4.currentStep)
    {
        //06022D5A
        u16 r0 = r5[r4.currentStep];
        r4.delay = (r0 & 0xF) - 1;
        r4.value = (s16)(r0 & 0xFFF0);
    }
    else
    {
        //06022D6E
        r4.delay = 0;
        r4.value = r5[0] * 16;
    }

    r4.currentStep++;

    if (r4.currentStep >= maxStep)
    {
        r4.currentStep = 0;
    }

    return r4.value;
}

s32 stepAnimationTrack(sAnimTrackStatus& r4, u8* r5, u16 maxStep)
{
    if (r4.delay > 0)
    {
        r4.delay -= 1;
        return r4.value;
    }

    if (r4.currentStep)
    {
        //06022D5A
        u16 r0 = READ_BE_U16(r5 + r4.currentStep * 2);
        r4.delay = (r0 & 0xF) - 1;
        r4.value = (s16)(r0 & 0xFFF0);
    }
    else
    {
        //06022D6E
        r4.delay = 0;
        r4.value = READ_BE_S16(r5) * 16;
    }

    if (maxStep > r4.currentStep + 1)
    {
        r4.currentStep++;
    }
    else
    {
        r4.currentStep = 0;
    }

    return r4.value;
}

void modelMode1_position0(s_3dModel* p3dModel)
{
    std::vector<sPoseData>::iterator r13_pPoseData = p3dModel->m2C_poseData.begin();
    const sAnimationData::sTrackHeader& r14 = p3dModel->m30_pCurrentAnimation->m8_trackHeader[0];

    if (p3dModel->m10_currentAnimationFrame)
    {
        r13_pPoseData->m0_translation[0] += stepAnimationTrack(r13_pPoseData->m48[0], r14.m14_trackData[0], r14.m0_tracksLength[0]);
        r13_pPoseData->m0_translation[1] += stepAnimationTrack(r13_pPoseData->m48[1], r14.m14_trackData[1], r14.m0_tracksLength[1]);
        r13_pPoseData->m0_translation[2] += stepAnimationTrack(r13_pPoseData->m48[2], r14.m14_trackData[2], r14.m0_tracksLength[2]);
    }
    else
    {
        if (p3dModel->m8 & 4)
        {
            r13_pPoseData->m0_translation[0] = stepAnimationTrack(r13_pPoseData->m48[0], r14.m14_trackData[0], r14.m0_tracksLength[0]);
            r13_pPoseData->m0_translation[1] = stepAnimationTrack(r13_pPoseData->m48[1], r14.m14_trackData[1], r14.m0_tracksLength[1]);
            r13_pPoseData->m0_translation[2] = stepAnimationTrack(r13_pPoseData->m48[2], r14.m14_trackData[2], r14.m0_tracksLength[2]);
        }
        else
        {
            r13_pPoseData->m0_translation = p3dModel->m34_pDefaultPose->m0_bones[0].m0_translation;

            stepAnimationTrack(r13_pPoseData->m48[0], r14.m14_trackData[0], r14.m0_tracksLength[0]);
            stepAnimationTrack(r13_pPoseData->m48[1], r14.m14_trackData[1], r14.m0_tracksLength[1]);
            stepAnimationTrack(r13_pPoseData->m48[2], r14.m14_trackData[2], r14.m0_tracksLength[2]);
        }
    }
}

void modelMode1_position1(s_3dModel* p3dModel)
{
    Unimplemented();
}

#if 0
void addAnimationFrame(sVec3_FP* r1, sVec3_FP* r2, s_3dModel* p3dModel)
{
    assert(0); // Don't use!
    for (int i = 0; i < p3dModel->m12_numBones; i++)
    {
        *r1 += *r2;

        r1 = (sVec3_FP*)(sizeof(sPoseData) + (u8*)r1);
        r2 = (sVec3_FP*)(sizeof(sPoseData) + (u8*)r2);
    }
}
#endif

void modelMode1_rotation(s_3dModel* p3dModel)
{
    std::vector<sAnimationData::sTrackHeader>::const_iterator r8 = p3dModel->m30_pCurrentAnimation->m8_trackHeader.begin();

    if (p3dModel->m10_currentAnimationFrame)
    {
        for (int i = 0; i < p3dModel->m12_numBones; i++)
        {
            sPoseData& r7_poseData = p3dModel->m2C_poseData[i];
            r7_poseData.mC_rotation[0] += stepAnimationTrack(r7_poseData.m48[3], r8->m14_trackData[3], r8->m0_tracksLength[3]) * 0x1000;
            r7_poseData.mC_rotation[1] += stepAnimationTrack(r7_poseData.m48[4], r8->m14_trackData[4], r8->m0_tracksLength[4]) * 0x1000;
            r7_poseData.mC_rotation[2] += stepAnimationTrack(r7_poseData.m48[5], r8->m14_trackData[5], r8->m0_tracksLength[5]) * 0x1000;
            r8++;
        }
    }
    else
    {
        for (int i = 0; i < p3dModel->m12_numBones; i++)
        {
            sPoseData& r7_poseData = p3dModel->m2C_poseData[i];
            r7_poseData.mC_rotation[0] = stepAnimationTrack(r7_poseData.m48[3], r8->m14_trackData[3], r8->m0_tracksLength[3]) * 0x1000;
            r7_poseData.mC_rotation[1] = stepAnimationTrack(r7_poseData.m48[4], r8->m14_trackData[4], r8->m0_tracksLength[4]) * 0x1000;
            r7_poseData.mC_rotation[2] = stepAnimationTrack(r7_poseData.m48[5], r8->m14_trackData[5], r8->m0_tracksLength[5]) * 0x1000;
            r8++;
        }
    }
}

void modelMode1_scale(s_3dModel* pDragonStateData1)
{
    Unimplemented();
}

void modelMode5_position0(s_3dModel* pDragonStateData1)
{
    sPoseData& r14_pPoseData = pDragonStateData1->m2C_poseData[0];
    const sAnimationData::sTrackHeader& r13 = pDragonStateData1->m30_pCurrentAnimation->m8_trackHeader[0];
    sStaticPoseData* r4 = pDragonStateData1->m34_pDefaultPose;

    if (pDragonStateData1->m10_currentAnimationFrame & 3)
    {
        r14_pPoseData.m0_translation += r14_pPoseData.m24_halfTranslation;
    }
    else
    {
        if (pDragonStateData1->m10_currentAnimationFrame)
        {
            r14_pPoseData.m0_translation += r14_pPoseData.m24_halfTranslation;
        }
        else if (pDragonStateData1->m8 & 4)
        {
            //6022762
            assert(0);
        }
        else
        {
            //6022794
            r14_pPoseData.m0_translation = r4->m0_bones[0].m0_translation;

            stepAnimationTrack(r14_pPoseData.m48[0], r13.m14_trackData[0], r13.m0_tracksLength[0]);
            stepAnimationTrack(r14_pPoseData.m48[1], r13.m14_trackData[1], r13.m0_tracksLength[1]);
            stepAnimationTrack(r14_pPoseData.m48[2], r13.m14_trackData[2], r13.m0_tracksLength[2]);
        }

        //60227C2
        if (pDragonStateData1->m30_pCurrentAnimation->m4_numFrames - 1 > pDragonStateData1->m10_currentAnimationFrame)
        {
            r14_pPoseData.m24_halfTranslation[0] = stepAnimationTrack(r14_pPoseData.m48[0], r13.m14_trackData[0], r13.m0_tracksLength[0]) / 4;
            r14_pPoseData.m24_halfTranslation[1] = stepAnimationTrack(r14_pPoseData.m48[1], r13.m14_trackData[1], r13.m0_tracksLength[1]) / 4;
            r14_pPoseData.m24_halfTranslation[2] = stepAnimationTrack(r14_pPoseData.m48[2], r13.m14_trackData[2], r13.m0_tracksLength[2]) / 4;
        }
    }
}

void modelMode5_position1(s_3dModel* p3dModel)
{
    std::vector<sPoseData>& pPoseData = p3dModel->m2C_poseData;
    if (p3dModel->m10_currentAnimationFrame & 3)
    {
        for (int i = 0; i < p3dModel->m12_numBones; i++)
        {
            pPoseData[i].m0_translation += pPoseData[i].m24_halfTranslation;
        }
        return;
    }
    if (p3dModel->m10_currentAnimationFrame == 0)
    {
        std::vector<sAnimationData::sTrackHeader>::const_iterator r13 = p3dModel->m30_pCurrentAnimation->m8_trackHeader.begin();
        for (int i = 0; i < p3dModel->m12_numBones; i++)
        {
            pPoseData[i].m0_translation.m_value[0] = stepAnimationTrack(pPoseData[i].m48[0], r13->m14_trackData[0], r13->m0_tracksLength[0]);
            pPoseData[i].m0_translation.m_value[1] = stepAnimationTrack(pPoseData[i].m48[1], r13->m14_trackData[1], r13->m0_tracksLength[1]);
            pPoseData[i].m0_translation.m_value[2] = stepAnimationTrack(pPoseData[i].m48[2], r13->m14_trackData[2], r13->m0_tracksLength[2]);
            r13++;
        }
    }
    else
    {
        for (int i = 0; i < p3dModel->m12_numBones; i++)
        {
            pPoseData[i].m0_translation += pPoseData[i].m24_halfTranslation;
        }
    }

    if (p3dModel->m30_pCurrentAnimation->m4_numFrames - 1 > p3dModel->m10_currentAnimationFrame)
    {
        for (int i = 0; i < p3dModel->m12_numBones; i++)
        {
            const sAnimationData::sTrackHeader& r13 = p3dModel->m30_pCurrentAnimation->m8_trackHeader[i];
            pPoseData[i].m24_halfTranslation.m_value[0] = stepAnimationTrack(pPoseData[i].m48[0], r13.m14_trackData[0], r13.m0_tracksLength[0]) / 4;
            pPoseData[i].m24_halfTranslation.m_value[1] = stepAnimationTrack(pPoseData[i].m48[1], r13.m14_trackData[1], r13.m0_tracksLength[1]) / 4;
            pPoseData[i].m24_halfTranslation.m_value[2] = stepAnimationTrack(pPoseData[i].m48[2], r13.m14_trackData[2], r13.m0_tracksLength[2]) / 4;
        }
    }
}

void modelMode5_rotation(s_3dModel* p3dModel)
{
    std::vector<sPoseData>& pPoseData = p3dModel->m2C_poseData;
    if (p3dModel->m10_currentAnimationFrame & 3)
    {
        for (int i = 0; i < p3dModel->m12_numBones; i++)
        {
            pPoseData[i].mC_rotation += pPoseData[i].m30_halfRotation;
        }
    }
    else
    {
        if (p3dModel->m10_currentAnimationFrame)
        {
            for (int i = 0; i < p3dModel->m12_numBones; i++)
            {
                pPoseData[i].mC_rotation += pPoseData[i].m30_halfRotation;
            }
        }
        else
        {
            std::vector<sAnimationData::sTrackHeader>::const_iterator r13 = p3dModel->m30_pCurrentAnimation->m8_trackHeader.begin();
            for (int i = 0; i < p3dModel->m12_numBones; i++)
            {
                pPoseData[i].mC_rotation.m_value[0] = stepAnimationTrack(pPoseData[i].m48[3], r13->m14_trackData[3], r13->m0_tracksLength[3]) * 4096;
                pPoseData[i].mC_rotation.m_value[1] = stepAnimationTrack(pPoseData[i].m48[4], r13->m14_trackData[4], r13->m0_tracksLength[4]) * 4096;
                pPoseData[i].mC_rotation.m_value[2] = stepAnimationTrack(pPoseData[i].m48[5], r13->m14_trackData[5], r13->m0_tracksLength[5]) * 4096;
                r13++;
            }
        }

        if (p3dModel->m30_pCurrentAnimation->m4_numFrames - 1 > p3dModel->m10_currentAnimationFrame)
        {
            std::vector<sAnimationData::sTrackHeader>::const_iterator r13 = p3dModel->m30_pCurrentAnimation->m8_trackHeader.begin();
            for (int i = 0; i < p3dModel->m12_numBones; i++)
            {
                pPoseData[i].m30_halfRotation.m_value[0] = stepAnimationTrack(pPoseData[i].m48[3], r13->m14_trackData[3], r13->m0_tracksLength[3]) * 1024;
                pPoseData[i].m30_halfRotation.m_value[1] = stepAnimationTrack(pPoseData[i].m48[4], r13->m14_trackData[4], r13->m0_tracksLength[4]) * 1024;
                pPoseData[i].m30_halfRotation.m_value[2] = stepAnimationTrack(pPoseData[i].m48[5], r13->m14_trackData[5], r13->m0_tracksLength[5]) * 1024;
                r13++;
            }
        }

    }
}

// TODO: recheck that
void modelMode5_scale(s_3dModel* p3dModel)
{
    sPoseData& r14_pPoseData = p3dModel->m2C_poseData[0];
    const sAnimationData::sTrackHeader& r13 = p3dModel->m30_pCurrentAnimation->m8_trackHeader[0];
    sStaticPoseData* r4 = p3dModel->m34_pDefaultPose;

    if (p3dModel->m10_currentAnimationFrame & 3)
    {
        r14_pPoseData.m18_scale += r14_pPoseData.m3C_halfScale;
    }
    else
    {
        if (p3dModel->m10_currentAnimationFrame)
        {
            r14_pPoseData.m18_scale += r14_pPoseData.m3C_halfScale;
        }
        else
        {
            stepAnimationTrack(r14_pPoseData.m48[6], r13.m14_trackData[6], r13.m0_tracksLength[6]);
            stepAnimationTrack(r14_pPoseData.m48[7], r13.m14_trackData[7], r13.m0_tracksLength[7]);
            stepAnimationTrack(r14_pPoseData.m48[8], r13.m14_trackData[8], r13.m0_tracksLength[8]);
        }

        if (p3dModel->m30_pCurrentAnimation->m4_numFrames - 1 > p3dModel->m10_currentAnimationFrame)
        {
            r14_pPoseData.m3C_halfScale[0] = stepAnimationTrack(r14_pPoseData.m48[6], r13.m14_trackData[6], r13.m0_tracksLength[6]) / 4;
            r14_pPoseData.m3C_halfScale[1] = stepAnimationTrack(r14_pPoseData.m48[7], r13.m14_trackData[7], r13.m0_tracksLength[7]) / 4;
            r14_pPoseData.m3C_halfScale[2] = stepAnimationTrack(r14_pPoseData.m48[8], r13.m14_trackData[8], r13.m0_tracksLength[8]) / 4;
        }
    }
}

void modelMode4_position0(s_3dModel* p3dModel)
{
    sPoseData& rootPoseData = p3dModel->m2C_poseData[0];
    const sAnimationData::sTrackHeader& r13 = p3dModel->m30_pCurrentAnimation->m8_trackHeader[0];
    sStaticPoseData* r4 = p3dModel->m34_pDefaultPose;

    if (p3dModel->m10_currentAnimationFrame & 1)
    {
        rootPoseData.m0_translation += rootPoseData.m24_halfTranslation;
        return;
    }

    //06022638
    if (p3dModel->m10_currentAnimationFrame)
    {
        rootPoseData.m0_translation += rootPoseData.m24_halfTranslation;
    }
    else
    {
        //0602265A
        if (p3dModel->m8 & 4)
        {
            assert(0);
        }
        else
        {
            rootPoseData.m0_translation = r4->m0_bones[0].m0_translation;

            stepAnimationTrack(rootPoseData.m48[0], r13.m14_trackData[0], r13.m0_tracksLength[0]);
            stepAnimationTrack(rootPoseData.m48[1], r13.m14_trackData[1], r13.m0_tracksLength[1]);
            stepAnimationTrack(rootPoseData.m48[2], r13.m14_trackData[2], r13.m0_tracksLength[2]);
        }
    }

    //60226BA
    if (p3dModel->m30_pCurrentAnimation->m4_numFrames - 1 >= p3dModel->m10_currentAnimationFrame)
    {
        rootPoseData.m24_halfTranslation[0] = stepAnimationTrack(rootPoseData.m48[0], r13.m14_trackData[0], r13.m0_tracksLength[0]) / 2;
        rootPoseData.m24_halfTranslation[1] = stepAnimationTrack(rootPoseData.m48[1], r13.m14_trackData[1], r13.m0_tracksLength[1]) / 2;
        rootPoseData.m24_halfTranslation[2] = stepAnimationTrack(rootPoseData.m48[2], r13.m14_trackData[2], r13.m0_tracksLength[2]) / 2;
    }

}

void modelMode4_position1(s_3dModel* p3dModel)
{
    std::vector<sPoseData>& pPoseData = p3dModel->m2C_poseData;
    if (p3dModel->m10_currentAnimationFrame & 1)
    {
        for (int i = 0; i < p3dModel->m12_numBones; i++)
        {
            pPoseData[i].m0_translation += pPoseData[i].m24_halfTranslation;
        }
        return;
    }

    if (p3dModel->m10_currentAnimationFrame)
    {
        for (int i = 0; i < p3dModel->m12_numBones; i++)
        {
            pPoseData[i].m0_translation += pPoseData[i].m24_halfTranslation;
        }
    }
    else
    {
        for (int i = 0; i < p3dModel->m12_numBones; i++)
        {
            assert(pPoseData[i].m48[0].currentStep == 0);
            assert(pPoseData[i].m48[1].currentStep == 0);
            assert(pPoseData[i].m48[2].currentStep == 0);

            const sAnimationData::sTrackHeader& r13 = p3dModel->m30_pCurrentAnimation->m8_trackHeader[i];
            pPoseData[i].m0_translation.m_value[0] = stepAnimationTrack(pPoseData[i].m48[0], r13.m14_trackData[0], r13.m0_tracksLength[0]);
            pPoseData[i].m0_translation.m_value[1] = stepAnimationTrack(pPoseData[i].m48[1], r13.m14_trackData[1], r13.m0_tracksLength[1]);
            pPoseData[i].m0_translation.m_value[2] = stepAnimationTrack(pPoseData[i].m48[2], r13.m14_trackData[2], r13.m0_tracksLength[2]);
        }
    }

    if (p3dModel->m30_pCurrentAnimation->m4_numFrames - 1 > p3dModel->m10_currentAnimationFrame)
    {
        for (int i = 0; i < p3dModel->m12_numBones; i++)
        {
            const sAnimationData::sTrackHeader& r13 = p3dModel->m30_pCurrentAnimation->m8_trackHeader[i];
            pPoseData[i].m24_halfTranslation.m_value[0] = stepAnimationTrack(pPoseData[i].m48[0], r13.m14_trackData[0], r13.m0_tracksLength[0]) / 2;
            pPoseData[i].m24_halfTranslation.m_value[1] = stepAnimationTrack(pPoseData[i].m48[1], r13.m14_trackData[1], r13.m0_tracksLength[1]) / 2;
            pPoseData[i].m24_halfTranslation.m_value[2] = stepAnimationTrack(pPoseData[i].m48[2], r13.m14_trackData[2], r13.m0_tracksLength[2]) / 2;
        }
    }
}

void modelMode4_rotation(s_3dModel* p3dModel)
{
    std::vector<sPoseData>& pPoseData = p3dModel->m2C_poseData;
    if (p3dModel->m10_currentAnimationFrame & 1)
    {
        for (int i = 0; i < p3dModel->m12_numBones; i++)
        {
            pPoseData[i].mC_rotation += pPoseData[i].m30_halfRotation;
        }
        return;
    }

    if (p3dModel->m10_currentAnimationFrame)
    {
        for (int i = 0; i < p3dModel->m12_numBones; i++)
        {
            pPoseData[i].mC_rotation += pPoseData[i].m30_halfRotation;
        }
    }
    else
    {
        for (int i = 0; i < p3dModel->m12_numBones; i++)
        {
            assert(pPoseData[i].m48[3].currentStep == 0);
            assert(pPoseData[i].m48[4].currentStep == 0);
            assert(pPoseData[i].m48[5].currentStep == 0);

            const sAnimationData::sTrackHeader& r13 = p3dModel->m30_pCurrentAnimation->m8_trackHeader[i];
            pPoseData[i].mC_rotation.m_value[0] = stepAnimationTrack(pPoseData[i].m48[3], r13.m14_trackData[3], r13.m0_tracksLength[3]) * 0x1000;
            pPoseData[i].mC_rotation.m_value[1] = stepAnimationTrack(pPoseData[i].m48[4], r13.m14_trackData[4], r13.m0_tracksLength[4]) * 0x1000;
            pPoseData[i].mC_rotation.m_value[2] = stepAnimationTrack(pPoseData[i].m48[5], r13.m14_trackData[5], r13.m0_tracksLength[5]) * 0x1000;
        }
    }

    if (p3dModel->m30_pCurrentAnimation->m4_numFrames - 1 > p3dModel->m10_currentAnimationFrame)
    {
        for (int i = 0; i < p3dModel->m12_numBones; i++)
        {
            const sAnimationData::sTrackHeader& r13 = p3dModel->m30_pCurrentAnimation->m8_trackHeader[i];
            pPoseData[i].m30_halfRotation.m_value[0] = stepAnimationTrack(pPoseData[i].m48[3], r13.m14_trackData[3], r13.m0_tracksLength[3]) * (0x1000 / 2);
            pPoseData[i].m30_halfRotation.m_value[1] = stepAnimationTrack(pPoseData[i].m48[4], r13.m14_trackData[4], r13.m0_tracksLength[4]) * (0x1000 / 2);
            pPoseData[i].m30_halfRotation.m_value[2] = stepAnimationTrack(pPoseData[i].m48[5], r13.m14_trackData[5], r13.m0_tracksLength[5]) * (0x1000 / 2);
        }
    }
}

void modelMode0_position(s_3dModel* p3dModel)
{
    std::vector<sPoseData>& pPoseData = p3dModel->m2C_poseData;
    for (int i = 0; i < p3dModel->m12_numBones; i++)
    {
        const sAnimationData::sTrackHeader& r6 = p3dModel->m30_pCurrentAnimation->m8_trackHeader[i];
        pPoseData[i].m0_translation.m_value[0] = r6.m14_trackData[0][p3dModel->m10_currentAnimationFrame] * 0x10;
        pPoseData[i].m0_translation.m_value[1] = r6.m14_trackData[1][p3dModel->m10_currentAnimationFrame] * 0x10;
        pPoseData[i].m0_translation.m_value[2] = r6.m14_trackData[2][p3dModel->m10_currentAnimationFrame] * 0x10;
    }
}

void modelMode0_rotation(s_3dModel* p3dModel)
{
    std::vector<sPoseData>& pPoseData = p3dModel->m2C_poseData;
    for (int i = 0; i < p3dModel->m12_numBones; i++)
    {
        const sAnimationData::sTrackHeader& r6 = p3dModel->m30_pCurrentAnimation->m8_trackHeader[i];
        pPoseData[i].mC_rotation.m_value[0] = r6.m14_trackData[3][p3dModel->m10_currentAnimationFrame] * 0x10000;
        pPoseData[i].mC_rotation.m_value[1] = r6.m14_trackData[4][p3dModel->m10_currentAnimationFrame] * 0x10000;
        pPoseData[i].mC_rotation.m_value[2] = r6.m14_trackData[5][p3dModel->m10_currentAnimationFrame] * 0x10000;
    }
}

void modelMode0_scale(s_3dModel*)
{
    PDS_unimplemented("modelMode0_scale");
}


void modelMode4_scale(s_3dModel*)
{
    PDS_unimplemented("modelMode4_scale");
}

u32 createDragonStateSubData1Sub1Sub1(s_3dModel* p3dModel, sAnimationData* pModelData)
{
    u16 flags = pModelData->m0_flags;

    switch (flags & 7)
    {
    case 0: // quantized but full frame
        p3dModel->m20_positionUpdateFunction = modelMode0_position;
        p3dModel->m24_rotationUpdateFunction = modelMode0_rotation;
        p3dModel->m28_scaleUpdateFunction = modelMode0_scale;
        return 1;
    case 1:
        if (p3dModel->mA_animationFlags & 0x100)
        {
            p3dModel->m20_positionUpdateFunction = modelMode1_position0;
        }
        else
        {
            p3dModel->m20_positionUpdateFunction = modelMode1_position1;
        }
        p3dModel->m24_rotationUpdateFunction = modelMode1_rotation;
        p3dModel->m28_scaleUpdateFunction = modelMode1_scale;

        // TODO: recheck this
        for (u32 i = 0; i < p3dModel->m12_numBones; i++)
        {
            for (u32 j = 0; j < 9; j++)
            {
                p3dModel->m2C_poseData[i].m48[j].currentStep = 0;
                p3dModel->m2C_poseData[i].m48[j].delay = 0;
                p3dModel->m2C_poseData[i].m48[j].value = 0;
            }
        }
        break;
    case 4: // every other frames
        if (p3dModel->mA_animationFlags & 0x100)
        {
            p3dModel->m20_positionUpdateFunction = modelMode4_position0;
        }
        else
        {
            p3dModel->m20_positionUpdateFunction = modelMode4_position1;
        }
        p3dModel->m24_rotationUpdateFunction = modelMode4_rotation;
        p3dModel->m28_scaleUpdateFunction = modelMode4_scale;

        for (u32 i = 0; i < p3dModel->m12_numBones; i++)
        {
            for (u32 j = 0; j < 9; j++)
            {
                p3dModel->m2C_poseData[i].m48[j].currentStep = 0;
                p3dModel->m2C_poseData[i].m48[j].delay = 0;
                p3dModel->m2C_poseData[i].m48[j].value = 0;
            }
        }
        break;
    case 5: // every 4 frames
        if (p3dModel->mA_animationFlags & 0x100)
        {
            p3dModel->m20_positionUpdateFunction = modelMode5_position0;
        }
        else
        {
            p3dModel->m20_positionUpdateFunction = modelMode5_position1;
        }
        p3dModel->m24_rotationUpdateFunction = modelMode5_rotation;
        p3dModel->m28_scaleUpdateFunction = modelMode5_scale;

        for (u32 i = 0; i < p3dModel->m12_numBones; i++)
        {
            for (u32 j = 0; j < 9; j++)
            {
                p3dModel->m2C_poseData[i].m48[j].currentStep = 0;
                p3dModel->m2C_poseData[i].m48[j].delay = 0;
                p3dModel->m2C_poseData[i].m48[j].value = 0;
            }
        }
        break;
    default:
        assert(0);
    }

    return 1;
}

u32 createDragonStateSubData1Sub1(s_3dModel* pDragonStateData1, sAnimationData* pAnimation)
{
    pDragonStateData1->m30_pCurrentAnimation = pAnimation;
    pDragonStateData1->m10_currentAnimationFrame = 0;

    u16 flags = READ_BE_U16(pAnimation);

    if ((flags & 8) || (pDragonStateData1->mA_animationFlags & 0x100))
    {
        copyPosePosition(pDragonStateData1);
    }

    if (flags & 0x10)
    {
        copyPoseRotation(pDragonStateData1);
    }

    if (flags & 0x20)
    {
        resetPoseScale(pDragonStateData1);
    }

    return createDragonStateSubData1Sub1Sub1(pDragonStateData1, pAnimation);
}

u32 dragonFieldTaskInitSub3Sub1Sub1(s_3dModel* pModel, sAnimationData* pAnimation)
{
    pModel->m30_pCurrentAnimation = pAnimation;
    pModel->m10_currentAnimationFrame = 0;

    u16 flags = pAnimation->m0_flags;

    if ((flags & 8) || (pModel->mA_animationFlags & 0x100))
    {
        pModel->m2C_poseData[0].m0_translation = pModel->m34_pDefaultPose->m0_bones[0].m0_translation;
    }

    if (flags & 0x10)
    {
        copyPoseRotation(pModel);
    }

    if (flags & 0x20)
    {
        resetPoseScale(pModel);
    }

    return createDragonStateSubData1Sub1Sub1(pModel, pAnimation);
}


u32 setupModelAnimation(s_3dModel* pModel, sAnimationData* pAnimation)
{
    if (pAnimation == NULL)
    {
        if (pModel->mA_animationFlags & 0x10)
        {
            copyPoseRotation(pModel);
        }
        if (pModel->mA_animationFlags & 0x20)
        {
            resetPoseScale(pModel);
        }

        pModel->mA_animationFlags &= ~0x38;
        pModel->m30_pCurrentAnimation = nullptr;
        return 1;
    }
    else
    {
        if (pModel->m30_pCurrentAnimation == NULL)
        {
            pModel->mA_animationFlags |= pAnimation->m0_flags;
            initModelDrawFunction(pModel);
            return dragonFieldTaskInitSub3Sub1Sub1(pModel, pAnimation);
        }
        else
        {
            if (pModel->m30_pCurrentAnimation->m0_flags != pAnimation->m0_flags)
            {
                pModel->mA_animationFlags &= ~0x38;
                pModel->mA_animationFlags |= pAnimation->m0_flags;
                initModelDrawFunction(pModel);
                return dragonFieldTaskInitSub3Sub1Sub1(pModel, pAnimation);
            }
            // 6021728
            pModel->m30_pCurrentAnimation = pAnimation;
            pModel->m10_currentAnimationFrame = 0;

            u16 r0 = pModel->m30_pCurrentAnimation->m0_flags & 7;
            if ((r0 == 1) || (r0 == 4) || (r0 == 5))
            {
                for (int i = 0; i < pModel->m12_numBones; i++)
                {
                    for (int j = 0; j < 9; j++)
                    {
                        pModel->m2C_poseData[i].m48[j].currentStep = 0;
                        pModel->m2C_poseData[i].m48[j].delay = 0;
                        pModel->m2C_poseData[i].m48[j].value = 0;
                    }
                }
            }

            return 1;
        }
        assert(0);
    }
    assert(0);
}

u32 stepAnimation(s_3dModel* p3DModel)
{
    if ((p3DModel->mA_animationFlags & 0x38) == 0)
    {
        return 0;
    }

    if (p3DModel->m30_pCurrentAnimation->m0_flags & 8)
    {
        p3DModel->m20_positionUpdateFunction(p3DModel);
    }

    if (p3DModel->m30_pCurrentAnimation->m0_flags & 0x10)
    {
        p3DModel->m24_rotationUpdateFunction(p3DModel);
    }

    if (p3DModel->m30_pCurrentAnimation->m0_flags & 0x20)
    {
        p3DModel->m28_scaleUpdateFunction(p3DModel);
    }

    p3DModel->m16_previousAnimationFrame = p3DModel->m10_currentAnimationFrame;
    p3DModel->m10_currentAnimationFrame++;

    // animation reset
    if (p3DModel->m10_currentAnimationFrame >= p3DModel->m30_pCurrentAnimation->m4_numFrames)
    {
        p3DModel->m10_currentAnimationFrame = 0;
    }

    return p3DModel->m16_previousAnimationFrame;
}

void interpolateAnimation(s_3dModel* p3dModel)
{
    if (p3dModel->m48_poseDataInterpolation.size())
    {
        p3dModel->m4C_interpolationStep++;
        if (p3dModel->m4E_interpolationLength < p3dModel->m4C_interpolationStep)
        {
            //TODO: freeVdp1Block(pDragonStateData1->m0_pOwnerTask, pDragonStateData1->m48)
            p3dModel->m48_poseDataInterpolation.resize(0);
            initModelDrawFunction(p3dModel);
        }
        else
        {
            s32 r9 = p3dModel->m12_numBones;
            std::vector<sPoseData>::const_iterator r12 = p3dModel->m2C_poseData.begin();
            std::vector<sPoseDataInterpolation>::iterator r14 = p3dModel->m48_poseDataInterpolation.begin();
            fixedPoint r11 = FP_Div(p3dModel->m4C_interpolationStep, p3dModel->m4E_interpolationLength);
            do {
                r14->m0_translation[0] = MTH_Mul(r12->m0_translation[0] - r14->m24_halfTranslation[0], r11) + r14->m24_halfTranslation[0];
                r14->m0_translation[1] = MTH_Mul(r12->m0_translation[1] - r14->m24_halfTranslation[1], r11) + r14->m24_halfTranslation[1];
                r14->m0_translation[2] = MTH_Mul(r12->m0_translation[2] - r14->m24_halfTranslation[2], r11) + r14->m24_halfTranslation[2];

                r14->mC_rotation[0] = MTH_Mul(fixedPoint(r12->mC_rotation[0] - r14->m30_halfRotation[0]).normalized(), r11) + r14->m30_halfRotation[0];
                r14->mC_rotation[1] = MTH_Mul(fixedPoint(r12->mC_rotation[1] - r14->m30_halfRotation[1]).normalized(), r11) + r14->m30_halfRotation[1];
                r14->mC_rotation[2] = MTH_Mul(fixedPoint(r12->mC_rotation[2] - r14->m30_halfRotation[2]).normalized(), r11) + r14->m30_halfRotation[2];

                r14->m18_scale[0] = MTH_Mul(r12->m18_scale[0] - r14->m3C_halfScale[0], r11) + r14->m3C_halfScale[0];
                r14->m18_scale[1] = MTH_Mul(r12->m18_scale[1] - r14->m3C_halfScale[1], r11) + r14->m3C_halfScale[1];
                r14->m18_scale[2] = MTH_Mul(r12->m18_scale[2] - r14->m3C_halfScale[2], r11) + r14->m3C_halfScale[2];

                r12++;
                r14++;

            } while (--r9);
        }
    }
}

u32 updateAndInterpolateAnimation(s_3dModel* pModel)
{
    u32 r0 = stepAnimation(pModel);
    interpolateAnimation(pModel);

    return r0;
}

u32 setupPoseInterpolation(s_3dModel* pModel, u32 interpolationLength)
{
    if ((interpolationLength > 0) && (pModel->m38_pColorAnim == 0))
    {
        if (pModel->m48_poseDataInterpolation.size())
        {
            for (int i = 0; i < pModel->m12_numBones; i++)
            {
                pModel->m48_poseDataInterpolation[i].m24_halfTranslation = pModel->m2C_poseData[i].m0_translation;
                pModel->m48_poseDataInterpolation[i].m30_halfRotation = pModel->m2C_poseData[i].mC_rotation;
                pModel->m48_poseDataInterpolation[i].m3C_halfScale = pModel->m2C_poseData[i].m18_scale;
            }
        }
        else
        {
            pModel->m48_poseDataInterpolation.resize(pModel->m12_numBones);
            if (pModel->m48_poseDataInterpolation.size() == 0)
            {
                pModel->m4C_interpolationStep = 0;
                pModel->m4E_interpolationLength = 0;
                return 0;
            }

            for (int i = 0; i < pModel->m12_numBones; i++)
            {
                pModel->m48_poseDataInterpolation[i].m24_halfTranslation = pModel->m2C_poseData[i].m0_translation;
                pModel->m48_poseDataInterpolation[i].m0_translation = pModel->m2C_poseData[i].m0_translation;
                pModel->m48_poseDataInterpolation[i].m30_halfRotation = pModel->m2C_poseData[i].mC_rotation;
                pModel->m48_poseDataInterpolation[i].mC_rotation = pModel->m2C_poseData[i].mC_rotation;
                pModel->m48_poseDataInterpolation[i].m3C_halfScale = pModel->m2C_poseData[i].m18_scale;
                pModel->m48_poseDataInterpolation[i].m18_scale = pModel->m2C_poseData[i].m18_scale;
            }
        }

        pModel->m4C_interpolationStep = 0;
        pModel->m4E_interpolationLength = interpolationLength;
        return 1;
    }
    else
    {
        pModel->m4C_interpolationStep = 0;
        pModel->m4E_interpolationLength = 0;
        return 0;
    }
}

void playAnimation(s_3dModel* pModel, sAnimationData* pAnimation, u32 interpolationLength)
{
    if (setupPoseInterpolation(pModel, interpolationLength) && ((pModel->mA_animationFlags & 0x200) == 0))
    {
        assert(0);
    }
    else
    {
        setupModelAnimation(pModel, pAnimation);
    }
}
