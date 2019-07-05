#include "PDS.h"
#include "animation.h"

void copyPosePosition(s_3dModel* pModel)
{
    u8* r5 = pModel->m34_pDefaultPose;

    for (u32 i = 0; i < pModel->m12_numBones; i++)
    {
        pModel->m2C_poseData[i].m0_translation[0] = READ_BE_U32(r5 + 0);
        pModel->m2C_poseData[i].m0_translation[1] = READ_BE_U32(r5 + 4);
        pModel->m2C_poseData[i].m0_translation[2] = READ_BE_U32(r5 + 8);
        r5 += 0x24;
    }
}

void copyPoseRotation(s_3dModel* pModel)
{
    u8* r5 = pModel->m34_pDefaultPose;

    for (u32 i = 0; i < pModel->m12_numBones; i++)
    {
        pModel->m2C_poseData[i].mC_rotation[0] = READ_BE_U32(r5 + 0xC);
        pModel->m2C_poseData[i].mC_rotation[1] = READ_BE_U32(r5 + 0x10);
        pModel->m2C_poseData[i].mC_rotation[2] = READ_BE_U32(r5 + 0x14);
        r5 += 0x24;
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

void modelMode1_position0(s_3dModel* pDragonStateData1)
{
    std::vector<sPoseData>::iterator r13_pPoseData = pDragonStateData1->m2C_poseData.begin();
    u8* r14 = pDragonStateData1->m30_pCurrentAnimation + READ_BE_U32(pDragonStateData1->m30_pCurrentAnimation + 8);

    if (pDragonStateData1->m10_currentAnimationFrame)
    {
        r13_pPoseData->m0_translation[0] += stepAnimationTrack(r13_pPoseData->m48[0], pDragonStateData1->m30_pCurrentAnimation + READ_BE_S32(r14 + 0x14), READ_BE_S16(r14 + 0));
        r13_pPoseData->m0_translation[1] += stepAnimationTrack(r13_pPoseData->m48[1], pDragonStateData1->m30_pCurrentAnimation + READ_BE_S32(r14 + 0x18), READ_BE_S16(r14 + 2));
        r13_pPoseData->m0_translation[2] += stepAnimationTrack(r13_pPoseData->m48[2], pDragonStateData1->m30_pCurrentAnimation + READ_BE_S32(r14 + 0x1C), READ_BE_S16(r14 + 4));
    }
    else
    {
        if (pDragonStateData1->m8 & 4)
        {
            assert(0);
        }
        else
        {
            r13_pPoseData->m0_translation[0] = READ_BE_S32(pDragonStateData1->m34_pDefaultPose + 0);
            r13_pPoseData->m0_translation[1] = READ_BE_S32(pDragonStateData1->m34_pDefaultPose + 4);
            r13_pPoseData->m0_translation[2] = READ_BE_S32(pDragonStateData1->m34_pDefaultPose + 8);

            stepAnimationTrack(r13_pPoseData->m48[0], pDragonStateData1->m30_pCurrentAnimation + READ_BE_S32(r14 + 0x14), READ_BE_S16(r14 + 0));
            stepAnimationTrack(r13_pPoseData->m48[1], pDragonStateData1->m30_pCurrentAnimation + READ_BE_S32(r14 + 0x18), READ_BE_S16(r14 + 2));
            stepAnimationTrack(r13_pPoseData->m48[2], pDragonStateData1->m30_pCurrentAnimation + READ_BE_S32(r14 + 0x1C), READ_BE_S16(r14 + 4));
        }
    }
}

void modelMode1_position1(s_3dModel* p3dModel)
{
    FunctionUnimplemented();
}

void addAnimationFrame(sVec3_FP* r1, sVec3_FP* r2, s_3dModel* p3dModel)
{
    for (int i = 0; i < p3dModel->m12_numBones; i++)
    {
        *r1 += *r2;

        r1 = (sVec3_FP*)(sizeof(sPoseData) + (u8*)r1);
        r2 = (sVec3_FP*)(sizeof(sPoseData) + (u8*)r2);
    }
}


void modelMode1_rotation(s_3dModel* p3dModel)
{
    std::vector<sPoseData>::iterator r7_pPoseData = p3dModel->m2C_poseData.begin();
    u8* r8 = p3dModel->m30_pCurrentAnimation + READ_BE_U32(p3dModel->m30_pCurrentAnimation + 8);
    u8* r9 = r8 + 6; //skip position to rotations

    if (p3dModel->m10_currentAnimationFrame)
    {
        for (int i = 0; i < p3dModel->m12_numBones; i++)
        {
            r7_pPoseData->mC_rotation[0] += stepAnimationTrack(r7_pPoseData->m48[3], p3dModel->m30_pCurrentAnimation + READ_BE_U32(r8 + 0x20), READ_BE_S16(r9 + 0)) << 12;
            r7_pPoseData->mC_rotation[1] += stepAnimationTrack(r7_pPoseData->m48[4], p3dModel->m30_pCurrentAnimation + READ_BE_U32(r8 + 0x24), READ_BE_S16(r9 + 2)) << 12;
            r7_pPoseData->mC_rotation[2] += stepAnimationTrack(r7_pPoseData->m48[5], p3dModel->m30_pCurrentAnimation + READ_BE_U32(r8 + 0x28), READ_BE_S16(r9 + 4)) << 12;

            r7_pPoseData++;
            r8 += 0x38;
            r9 += 3 * 2 + 0x32;
        }
    }
    else
    {
        for (int i = 0; i < p3dModel->m12_numBones; i++)
        {
            r7_pPoseData->mC_rotation[0] = stepAnimationTrack(r7_pPoseData->m48[3], p3dModel->m30_pCurrentAnimation + READ_BE_U32(r8 + 0x20), READ_BE_S16(r9 + 0)) << 12;
            r7_pPoseData->mC_rotation[1] = stepAnimationTrack(r7_pPoseData->m48[4], p3dModel->m30_pCurrentAnimation + READ_BE_U32(r8 + 0x24), READ_BE_S16(r9 + 2)) << 12;
            r7_pPoseData->mC_rotation[2] = stepAnimationTrack(r7_pPoseData->m48[5], p3dModel->m30_pCurrentAnimation + READ_BE_U32(r8 + 0x28), READ_BE_S16(r9 + 4)) << 12;

            r7_pPoseData++;
            r8 += 0x38;
            r9 += 3 * 2 + 0x32;
        }
    }
}

void modelMode1_scale(s_3dModel* pDragonStateData1)
{
    FunctionUnimplemented();
}

void modelMode5_position0(s_3dModel* pDragonStateData1)
{
    std::vector<sPoseData>::iterator r14_pPoseData = pDragonStateData1->m2C_poseData.begin();
    u8* r13 = pDragonStateData1->m30_pCurrentAnimation + READ_BE_U32(pDragonStateData1->m30_pCurrentAnimation + 8);
    u8* r4 = pDragonStateData1->m34_pDefaultPose;

    if (pDragonStateData1->m10_currentAnimationFrame & 3)
    {
        r14_pPoseData->m0_translation += r14_pPoseData->m24_halfTranslation;
    }
    else
    {
        if (pDragonStateData1->m10_currentAnimationFrame)
        {
            r14_pPoseData->m0_translation += r14_pPoseData->m24_halfTranslation;
        }
        else if (pDragonStateData1->m8 & 4)
        {
            //6022762
            assert(0);
        }
        else
        {
            //6022794
            r14_pPoseData->m0_translation[0] = READ_BE_S32(r4);
            r14_pPoseData->m0_translation[1] = READ_BE_S32(r4 + 4);
            r14_pPoseData->m0_translation[2] = READ_BE_S32(r4 + 8);

            stepAnimationTrack(r14_pPoseData->m48[0], pDragonStateData1->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x14), READ_BE_U16(r13 + 0));
            stepAnimationTrack(r14_pPoseData->m48[1], pDragonStateData1->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x18), READ_BE_U16(r13 + 2));
            stepAnimationTrack(r14_pPoseData->m48[2], pDragonStateData1->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x1C), READ_BE_U16(r13 + 4));
        }

        //60227C2
        s32 r3 = 0;
        if (pDragonStateData1->m30_pCurrentAnimation)
        {
            r3 = READ_BE_S16(pDragonStateData1->m30_pCurrentAnimation + 4);
        }
        r3--;
        if (pDragonStateData1->m10_currentAnimationFrame < r3)
        {
            r14_pPoseData->m24_halfTranslation[0] = stepAnimationTrack(r14_pPoseData->m48[0], pDragonStateData1->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x14), READ_BE_U16(r13 + 0)) / 4;
            r14_pPoseData->m24_halfTranslation[1] = stepAnimationTrack(r14_pPoseData->m48[1], pDragonStateData1->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x18), READ_BE_U16(r13 + 2)) / 4;
            r14_pPoseData->m24_halfTranslation[2] = stepAnimationTrack(r14_pPoseData->m48[2], pDragonStateData1->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x1C), READ_BE_U16(r13 + 4)) / 4;
        }
    }
}

void modelMode5_position1(s_3dModel* p3dModel)
{
    FunctionUnimplemented();
}

void modelMode5_rotation(s_3dModel* p3dModel)
{
    std::vector<sPoseData>& pPoseData = p3dModel->m2C_poseData;
    if (p3dModel->m10_currentAnimationFrame & 3)
    {
        addAnimationFrame(&pPoseData[0].mC_rotation, &pPoseData[0].m30_halfRotation, p3dModel);
        return;
    }

    if (p3dModel->m10_currentAnimationFrame)
    {
        addAnimationFrame(&pPoseData[0].mC_rotation, &pPoseData[0].m30_halfRotation, p3dModel);
    }
    else
    {
        u8* r13 = p3dModel->m30_pCurrentAnimation + READ_BE_U32(p3dModel->m30_pCurrentAnimation + 8);
        for (int i = 0; i < p3dModel->m12_numBones; i++)
        {
            pPoseData[i].mC_rotation.m_value[0] = stepAnimationTrack(pPoseData[i].m48[3], p3dModel->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x20), READ_BE_U16(r13 + 6)) * 4096;
            pPoseData[i].mC_rotation.m_value[1] = stepAnimationTrack(pPoseData[i].m48[4], p3dModel->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x24), READ_BE_U16(r13 + 8)) * 4096;
            pPoseData[i].mC_rotation.m_value[2] = stepAnimationTrack(pPoseData[i].m48[5], p3dModel->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x28), READ_BE_U16(r13 + 10)) * 4096;
            r13 += 0x38;
        }
    }

    if (READ_BE_U16(p3dModel->m30_pCurrentAnimation + 4) - 1 > p3dModel->m10_currentAnimationFrame)
    {
        u8* r13 = p3dModel->m30_pCurrentAnimation + READ_BE_U32(p3dModel->m30_pCurrentAnimation + 8);
        for (int i = 0; i < p3dModel->m12_numBones; i++)
        {
            pPoseData[i].m30_halfRotation.m_value[0] = stepAnimationTrack(pPoseData[i].m48[3], p3dModel->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x20), READ_BE_U16(r13 + 6)) * 1024;
            pPoseData[i].m30_halfRotation.m_value[1] = stepAnimationTrack(pPoseData[i].m48[4], p3dModel->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x24), READ_BE_U16(r13 + 8)) * 1024;
            pPoseData[i].m30_halfRotation.m_value[2] = stepAnimationTrack(pPoseData[i].m48[5], p3dModel->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x28), READ_BE_U16(r13 + 10)) * 1024;
            r13 += 0x38;
        }
    }
}

void modelMode5_scale(s_3dModel* pDragonStateData1)
{
    FunctionUnimplemented();
}

void modelMode4_position0(s_3dModel* pDragonStateData1)
{
    std::vector<sPoseData>::iterator pPoseData = pDragonStateData1->m2C_poseData.begin();
    u8* r13 = pDragonStateData1->m30_pCurrentAnimation + READ_BE_U32(pDragonStateData1->m30_pCurrentAnimation + 8);
    u8* r4 = pDragonStateData1->m34_pDefaultPose;

    if (pDragonStateData1->m10_currentAnimationFrame & 1)
    {
        pPoseData->m0_translation += pPoseData->m24_halfTranslation;
        return;
    }

    //06022638
    if (pDragonStateData1->m10_currentAnimationFrame)
    {
        pPoseData->m0_translation += pPoseData->m24_halfTranslation;

        s16 r3;
        if (READ_BE_U32(pDragonStateData1->m30_pCurrentAnimation))
        {
            r3 = READ_BE_S16(pDragonStateData1->m30_pCurrentAnimation + 4);
        }
        else
        {
            r3 = 0;
        }

        r3--;
        if (pDragonStateData1->m10_currentAnimationFrame >= r3)
        {
            return;
        }

        pPoseData->m24_halfTranslation[0] = stepAnimationTrack(pPoseData->m48[0], pDragonStateData1->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x14), READ_BE_U16(r13 + 0)) / 2;
        pPoseData->m24_halfTranslation[1] = stepAnimationTrack(pPoseData->m48[1], pDragonStateData1->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x18), READ_BE_U16(r13 + 2)) / 2;
        pPoseData->m24_halfTranslation[2] = stepAnimationTrack(pPoseData->m48[2], pDragonStateData1->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x1C), READ_BE_U16(r13 + 4)) / 2;
    }
    else
    {
        //0602265A
        if (pDragonStateData1->m8 & 4)
        {
            assert(0);
        }
        else
        {
            pPoseData->m0_translation[0] = READ_BE_S32(r4);
            pPoseData->m0_translation[1] = READ_BE_S32(r4 + 4);
            pPoseData->m0_translation[2] = READ_BE_S32(r4 + 8);

            stepAnimationTrack(pPoseData->m48[0], pDragonStateData1->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x14), READ_BE_U16(r13 + 0));
            stepAnimationTrack(pPoseData->m48[1], pDragonStateData1->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x18), READ_BE_U16(r13 + 2));
            stepAnimationTrack(pPoseData->m48[2], pDragonStateData1->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x1C), READ_BE_U16(r13 + 4));
        }
    }

    //60226BA
    s16 r3;
    if (READ_BE_U32(pDragonStateData1->m30_pCurrentAnimation))
    {
        r3 = READ_BE_S16(pDragonStateData1->m30_pCurrentAnimation + 4);
    }
    else
    {
        r3 = 0;
    }

    r3--;
    if (r3 >= pDragonStateData1->m10_currentAnimationFrame)
    {
        pPoseData->m24_halfTranslation[0] = stepAnimationTrack(pPoseData->m48[0], pDragonStateData1->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x14), READ_BE_U16(r13 + 0)) / 2;
        pPoseData->m24_halfTranslation[1] = stepAnimationTrack(pPoseData->m48[1], pDragonStateData1->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x18), READ_BE_U16(r13 + 2)) / 2;
        pPoseData->m24_halfTranslation[2] = stepAnimationTrack(pPoseData->m48[2], pDragonStateData1->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x1C), READ_BE_U16(r13 + 4)) / 2;
    }

}

void modelMode4_position1(s_3dModel* p3dModel)
{
    std::vector<sPoseData>& pPoseData = p3dModel->m2C_poseData;
    if (p3dModel->m10_currentAnimationFrame & 1)
    {
        addAnimationFrame(&pPoseData[0].m0_translation, &pPoseData[0].m24_halfTranslation, p3dModel);
        return;
    }

    if (p3dModel->m10_currentAnimationFrame)
    {
        addAnimationFrame(&pPoseData[0].m0_translation, &pPoseData[0].m24_halfTranslation, p3dModel);

        if (READ_BE_U16(p3dModel->m30_pCurrentAnimation + 4) - 1 > p3dModel->m10_currentAnimationFrame)
        {
            u8* r13 = p3dModel->m30_pCurrentAnimation + READ_BE_U32(p3dModel->m30_pCurrentAnimation + 8);
            for (int i = 0; i < p3dModel->m12_numBones; i++)
            {
                pPoseData[i].m24_halfTranslation.m_value[0] = stepAnimationTrack(pPoseData[i].m48[0], p3dModel->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x14), READ_BE_U16(r13 + 0));
                pPoseData[i].m24_halfTranslation.m_value[1] = stepAnimationTrack(pPoseData[i].m48[1], p3dModel->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x18), READ_BE_U16(r13 + 2));
                pPoseData[i].m24_halfTranslation.m_value[2] = stepAnimationTrack(pPoseData[i].m48[2], p3dModel->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x1C), READ_BE_U16(r13 + 4));
                r13 += 0x38;
            }
        }
    }
    else
    {
        u8* r13 = p3dModel->m30_pCurrentAnimation + READ_BE_U32(p3dModel->m30_pCurrentAnimation + 8);
        for (int i = 0; i < p3dModel->m12_numBones; i++)
        {
            /*assert(pPoseData[i].m48[0].currentStep == 0);
            assert(pPoseData[i].m48[1].currentStep == 0);
            assert(pPoseData[i].m48[2].currentStep == 0);*/

            pPoseData[i].m24_halfTranslation.m_value[0] = stepAnimationTrack(pPoseData[i].m48[0], p3dModel->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x14), READ_BE_U16(r13 + 0)) >> 1;
            pPoseData[i].m24_halfTranslation.m_value[1] = stepAnimationTrack(pPoseData[i].m48[1], p3dModel->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x18), READ_BE_U16(r13 + 2)) >> 1;
            pPoseData[i].m24_halfTranslation.m_value[2] = stepAnimationTrack(pPoseData[i].m48[2], p3dModel->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x1C), READ_BE_U16(r13 + 4)) >> 1;
            r13 += 0x38;
        }
    }
}

void modelMode4_rotation(s_3dModel* p3dModel)
{
    std::vector<sPoseData>& pPoseData = p3dModel->m2C_poseData;
    if (p3dModel->m10_currentAnimationFrame & 1)
    {
        addAnimationFrame(&pPoseData[0].mC_rotation, &pPoseData[0].m30_halfRotation, p3dModel);
        return;
    }

    if (p3dModel->m10_currentAnimationFrame)
    {
        addAnimationFrame(&pPoseData[0].mC_rotation, &pPoseData[0].m30_halfRotation, p3dModel);
    }
    else
    {
        u8* r13 = p3dModel->m30_pCurrentAnimation + READ_BE_U32(p3dModel->m30_pCurrentAnimation + 8);
        for (int i = 0; i < p3dModel->m12_numBones; i++)
        {
            assert(pPoseData[i].m48[3].currentStep == 0);
            assert(pPoseData[i].m48[4].currentStep == 0);
            assert(pPoseData[i].m48[5].currentStep == 0);

            pPoseData[i].mC_rotation.m_value[0] = stepAnimationTrack(pPoseData[i].m48[3], p3dModel->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x20), READ_BE_U16(r13 + 6)) << 12;
            pPoseData[i].mC_rotation.m_value[1] = stepAnimationTrack(pPoseData[i].m48[4], p3dModel->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x24), READ_BE_U16(r13 + 8)) << 12;
            pPoseData[i].mC_rotation.m_value[2] = stepAnimationTrack(pPoseData[i].m48[5], p3dModel->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x28), READ_BE_U16(r13 + 10)) << 12;
            r13 += 0x38;
        }
    }

    if (READ_BE_U16(p3dModel->m30_pCurrentAnimation + 4) - 1 > p3dModel->m10_currentAnimationFrame)
    {
        u8* r13 = p3dModel->m30_pCurrentAnimation + READ_BE_U32(p3dModel->m30_pCurrentAnimation + 8);
        for (int i = 0; i < p3dModel->m12_numBones; i++)
        {
            pPoseData[i].m30_halfRotation.m_value[0] = stepAnimationTrack(pPoseData[i].m48[3], p3dModel->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x20), READ_BE_U16(r13 + 6)) << 11;
            pPoseData[i].m30_halfRotation.m_value[1] = stepAnimationTrack(pPoseData[i].m48[4], p3dModel->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x24), READ_BE_U16(r13 + 8)) << 11;
            pPoseData[i].m30_halfRotation.m_value[2] = stepAnimationTrack(pPoseData[i].m48[5], p3dModel->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x28), READ_BE_U16(r13 + 10)) << 11;
            r13 += 0x38;
        }
    }
}

void modelMode0_position(s_3dModel*)
{
    PDS_unimplemented("modelMode0_position");
}

void modelMode0_rotation(s_3dModel* p3dModel)
{
    std::vector<sPoseData>& pPoseData = p3dModel->m2C_poseData;
    u16 r0 = p3dModel->m12_numBones;
    u8* r6 = p3dModel->m30_pCurrentAnimation + READ_BE_U32(p3dModel->m30_pCurrentAnimation + 8);

    for (int i = 0; i < p3dModel->m12_numBones; i++)
    {
        pPoseData[i].mC_rotation.m_value[0] = READ_BE_S16(p3dModel->m10_currentAnimationFrame * 2 + p3dModel->m30_pCurrentAnimation + READ_BE_U32(r6 + 0x20)) << 16;
        pPoseData[i].mC_rotation.m_value[1] = READ_BE_S16(p3dModel->m10_currentAnimationFrame * 2 + p3dModel->m30_pCurrentAnimation + READ_BE_U32(r6 + 0x24)) << 16;
        pPoseData[i].mC_rotation.m_value[2] = READ_BE_S16(p3dModel->m10_currentAnimationFrame * 2 + p3dModel->m30_pCurrentAnimation + READ_BE_U32(r6 + 0x28)) << 16;

        r6 += 0x38;
    }
}

void modelMode0_scale(s_3dModel*)
{
    PDS_unimplemented("modelMode0_scale");
}


void modelMode4_scale(s_3dModel*)
{
    assert(0);
}

u32 createDragonStateSubData1Sub1Sub1(s_3dModel* p3dModel, u8* pModelData)
{
    u16 flags = READ_BE_U16(pModelData);

    switch (flags & 7)
    {
    case 0:
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
    case 4:
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
    case 5:
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

u32 createDragonStateSubData1Sub1(s_3dModel* pDragonStateData1, u8* pModelData1)
{
    pDragonStateData1->m30_pCurrentAnimation = pModelData1;
    pDragonStateData1->m10_currentAnimationFrame = 0;

    u16 flags = READ_BE_U16(pModelData1);

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

    return createDragonStateSubData1Sub1Sub1(pDragonStateData1, pModelData1);
}

u32 dragonFieldTaskInitSub3Sub1Sub1(s_3dModel* pModel, u8* pAnimation)
{
    pModel->m30_pCurrentAnimation = pAnimation;
    pModel->m10_currentAnimationFrame = 0;

    u16 flags = READ_BE_U16(pAnimation);

    if ((flags & 8) || (pModel->mA_animationFlags & 0x100))
    {
        pModel->m2C_poseData[0].m0_translation[0] = READ_BE_U32(pModel->m34_pDefaultPose + 0);
        pModel->m2C_poseData[0].m0_translation[1] = READ_BE_U32(pModel->m34_pDefaultPose + 4);
        pModel->m2C_poseData[0].m0_translation[2] = READ_BE_U32(pModel->m34_pDefaultPose + 8);
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


u32 setupModelAnimation(s_3dModel* pModel, u8* r5)
{
    if (r5 == NULL)
    {
        assert(0);
    }
    else
    {
        if (pModel->m30_pCurrentAnimation == NULL)
        {
            assert(0);
        }
        else
        {
            if (READ_BE_U16(pModel->m30_pCurrentAnimation) != READ_BE_U16(r5))
            {
                pModel->mA_animationFlags &= ~0x38;
                pModel->mA_animationFlags |= READ_BE_U16(r5);
                initModelDrawFunction(pModel);
                return dragonFieldTaskInitSub3Sub1Sub1(pModel, r5);
            }
            // 6021728
            pModel->m30_pCurrentAnimation = r5;
            pModel->m10_currentAnimationFrame = 0;

            u16 r0 = READ_BE_U16(r5) & 7;
            if ((r0 != 1) && (r0 != 4) && (r0 != 5))
            {
                return 1;
            }

            for (int i = 0; i < pModel->m12_numBones; i++)
            {
                for (int j = 0; j < 9; j++)
                {
                    pModel->m2C_poseData[i].m48[j].currentStep = 0;
                    pModel->m2C_poseData[i].m48[j].delay = 0;
                    pModel->m2C_poseData[i].m48[j].value = 0;
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

    if (READ_BE_U16(p3DModel->m30_pCurrentAnimation) & 8)
    {
        p3DModel->m20_positionUpdateFunction(p3DModel);
    }

    if (READ_BE_U16(p3DModel->m30_pCurrentAnimation) & 0x10)
    {
        p3DModel->m24_rotationUpdateFunction(p3DModel);
    }

    if (READ_BE_U16(p3DModel->m30_pCurrentAnimation) & 0x20)
    {
        p3DModel->m28_scaleUpdateFunction(p3DModel);
    }

    p3DModel->m16 = p3DModel->m10_currentAnimationFrame;
    p3DModel->m10_currentAnimationFrame++;

    // animation reset
    if (READ_BE_U16(p3DModel->m30_pCurrentAnimation + 4) <= p3DModel->m10_currentAnimationFrame)
    {
        p3DModel->m10_currentAnimationFrame = 0;
    }

    return p3DModel->m16;
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
            std::vector<sPoseData>::iterator r12 = p3dModel->m2C_poseData.begin();
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
    if ((interpolationLength > 0) && (pModel->m38 == 0))
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

void playAnimation(s_3dModel* pModel, u8* pAnimation, u32 r6)
{
    if (setupPoseInterpolation(pModel, r6) && ((pModel->mA_animationFlags & 0x200) == 0))
    {
        assert(0);
    }
    else
    {
        setupModelAnimation(pModel, pAnimation);
    }
}
