#include "PDS.h"
#include "town.h"
#include "townScript.h"
#include "townEdge.h"
#include "townLCS.h"
#include "kernel/animation.h"
#include "kernel/fileBundle.h"
#include "town/ruin/twn_ruin.h"
#include "audio/systemSounds.h"

void updateEdgeSub3(sEdgeTask* pThis);

sEdgeTask* startEdgeTask(sSaturnPtr r4)
{
    return createSubTaskWithArgWithCopy<sEdgeTask>(allocateNPC(currentResTask, readSaturnS32(r4)), r4);
}

void applyAnimation(sModelHierarchy* pNode, std::vector<sPoseData>::iterator& pose)
{
    pushCurrentMatrix();
    {
        translateCurrentMatrix(&pose->m0_translation);
        rotateCurrentMatrixZYX(&pose->mC_rotation);
        if (pNode->m0_3dModel)
        {
            addObjectToDrawList(pNode->m0_3dModel);
        }
        if (pNode->m4_subNode)
        {
            pose++;
            applyAnimation(pNode->m4_subNode, pose);
        }
    }
    popMatrix();
    if (pNode->m8_nextNode)
    {
        pose++;
        applyAnimation(pNode->m8_nextNode, pose);
    }

}

void applyAnimation2(sModelHierarchy* pNode, std::vector<sPoseDataInterpolation>::iterator& pose)
{
    pushCurrentMatrix();
    {
        translateCurrentMatrix(&pose->m0_translation);
        rotateCurrentMatrixZYX(&pose->mC_rotation);
        if (pNode->m0_3dModel)
        {
            addObjectToDrawList(pNode->m0_3dModel);
        }
        if (pNode->m4_subNode)
        {
            pose++;
            applyAnimation2(pNode->m4_subNode, pose);
        }
    }
    popMatrix();
    if (pNode->m8_nextNode)
    {
        pose++;
        applyAnimation2(pNode->m8_nextNode, pose);
    }
}

void applyEdgeAnimation(s_3dModel* pModel, sVec2_FP* r5)
{
    std::vector<sPoseData>::iterator r14_pose = pModel->m2C_poseData.begin();
    sModelHierarchy* r12 = pModel->m4_pModelFile->getModelHierarchy(pModel->mC_modelIndexOffset)->m4_subNode;

    pushCurrentMatrix();
    {
        translateCurrentMatrix(&r14_pose->m0_translation);
        rotateCurrentMatrixZYX(&r14_pose->mC_rotation);

        pushCurrentMatrix();
        {
            r14_pose++;
            translateCurrentMatrix(&r14_pose->m0_translation);
            rotateCurrentMatrixShiftedZ(r14_pose->mC_rotation[2]);
            rotateCurrentMatrixShiftedY(r14_pose->mC_rotation[1] + MTH_Mul(r5->m_value[1], 0x4CCC));
            rotateCurrentMatrixShiftedX(r14_pose->mC_rotation[0]);

            if (r12->m0_3dModel)
            {
                addObjectToDrawList(r12->m0_3dModel);
            }

            sModelHierarchy* r13 = r12->m4_subNode;
            pushCurrentMatrix();
            {
                r14_pose++;
                translateCurrentMatrix(&r14_pose->m0_translation);
                rotateCurrentMatrixShiftedZ(r14_pose->mC_rotation[2]);
                rotateCurrentMatrixShiftedY(r14_pose->mC_rotation[1] + MTH_Mul(r5->m_value[1], 0xB333));
                rotateCurrentMatrixShiftedX(r14_pose->mC_rotation[0]);

                if (r13->m0_3dModel)
                {
                    addObjectToDrawList(r13->m0_3dModel);
                }

                if (r13->m4_subNode)
                {
                    r14_pose++;
                    applyAnimation(r13->m4_subNode, r14_pose);
                }
            }
            popMatrix();

            if (r13->m8_nextNode)
            {
                r14_pose++;
                applyAnimation(r13->m8_nextNode, r14_pose);
            }
        }
        popMatrix();

        if (r12->m8_nextNode)
        {
            r14_pose++;
            applyAnimation(r12->m8_nextNode, r14_pose);
        }
    }
    popMatrix();
}


void applyEdgeAnimation2(s_3dModel* pModel, sVec2_FP* r5)
{
    std::vector<sPoseDataInterpolation>::iterator r14_pose = pModel->m48_poseDataInterpolation.begin();
    sModelHierarchy* r12 = pModel->m4_pModelFile->getModelHierarchy(pModel->mC_modelIndexOffset)->m4_subNode;

    pushCurrentMatrix();
    {
        translateCurrentMatrix(&r14_pose->m0_translation);
        rotateCurrentMatrixZYX(&r14_pose->mC_rotation);

        pushCurrentMatrix();
        {
            r14_pose++;
            translateCurrentMatrix(&r14_pose->m0_translation);
            rotateCurrentMatrixShiftedZ(r14_pose->mC_rotation[2]);
            rotateCurrentMatrixShiftedY(r14_pose->mC_rotation[1] + MTH_Mul(r5->m_value[1], 0x4CCC));
            rotateCurrentMatrixShiftedX(r14_pose->mC_rotation[0]);

            if (r12->m0_3dModel)
            {
                addObjectToDrawList(r12->m0_3dModel);
            }

            sModelHierarchy* r13 = r12->m4_subNode;
            pushCurrentMatrix();
            {
                r14_pose++;
                translateCurrentMatrix(&r14_pose->m0_translation);
                rotateCurrentMatrixShiftedZ(r14_pose->mC_rotation[2]);
                rotateCurrentMatrixShiftedY(r14_pose->mC_rotation[1] + MTH_Mul(r5->m_value[1], 0xB333));
                rotateCurrentMatrixShiftedX(r14_pose->mC_rotation[0]);

                if (r13->m0_3dModel)
                {
                    addObjectToDrawList(r13->m0_3dModel);
                }

                if (r13->m4_subNode)
                {
                    r14_pose++;
                    applyAnimation2(r13->m4_subNode, r14_pose);
                }
            }
            popMatrix();

            if (r13->m8_nextNode)
            {
                r14_pose++;
                applyAnimation2(r13->m8_nextNode, r14_pose);
            }
        }
        popMatrix();

        if (r12->m8_nextNode)
        {
            r14_pose++;
            applyAnimation2(r12->m8_nextNode, r14_pose);
        }
    }
    popMatrix();

}

void EdgeUpdateSub0(sMainLogic_74* r14_pose)
{
    if (resData.m4 >= 0x3F)
        return;

    sResData1C& r5 = resData.m1C[resData.m4++];
    r5.m0_pNext = resData.m8_headOfLinkedList[r14_pose->m2C_collisionSetupIndex];
    r5.m4 = r14_pose;

    resData.m8_headOfLinkedList[r14_pose->m2C_collisionSetupIndex] = &r5;

    sMatrix4x3 var4;
    initMatrixToIdentity(&var4);
    rotateMatrixYXZ(r14_pose->m34_pRotation, &var4);
    transformVec(r14_pose->m20, r14_pose->m8_position, var4);

    r14_pose->m8_position += *r14_pose->m30_pPosition;
}

void stepNPCForward(sNPCE8* pThis)
{
    sMatrix4x3 varC;
    initMatrixToIdentity(&varC);
    rotateMatrixShiftedY(pThis->mC_rotation[1], &varC);
    rotateMatrixShiftedX(pThis->mC_rotation[0], &varC);
    transformVec(pThis->m30_stepTranslation, pThis->m18_stepTranslationInWorld, varC);
    pThis->m0_position += pThis->m18_stepTranslationInWorld;
}

void initEdgeNPCSub0(sEdgeTask* pThis, s32 r5, sSaturnPtr r6)
{
    s32 r3 = 0;
    if (r5 & 0x80)
    {
        r3 = 0x80;
    }

    pThis->mF |= r3;
    pThis->mD = 0x3F & r5;
    pThis->m18 = r6;
    switch (pThis->mD)
    {
    case 3:
        if (r6.m_file != gTWN_RUIN)
        {
            PDS_Log("Recheck initEdgeNPCSub0 for other towns than ruin", 0);
        }
        else
        {
            assert(r6.m_offset == 0x605B8D4);
        }
        pThis->m14_updateFunction = &updateEdgePosition;
        break;
    default:
        assert(0);
        break;
    }
}

void initEdgeNPCSub1(sEdgeTask* pThis)
{
    pThis->m179 = 0;
    pThis->m178 = 0;
    pThis->m17A = 0;
}

void initEdgeNPC(sEdgeTask* pThis, sSaturnPtr arg)
{
    npcData0.m70_npcPointerArray[readSaturnU8(arg + 0x20)].workArea = pThis;
    npcData0.m70_npcPointerArray[readSaturnU8(arg + 0x20)].pNPC = pThis;
    pThis->mC = 0;
    pThis->m10_InitPtr = arg;
    pThis->m1C = readSaturnS32(arg + 0x28);
    pThis->m30_animationTable = readSaturnEA(arg + 0x2C);
    pThis->mE8.m0_position = readSaturnVec3(arg + 0x8);
    pThis->mE8.mC_rotation = readSaturnVec3(arg + 0x14);

    initEdgeNPCSub0(pThis, readSaturnU8(arg + 0x21), readSaturnEA(arg + 0x30));

    if (pThis->mD == 4)
    {
        pThis->m14E = 1;
    }

    pThis->m84.m30_pPosition = &pThis->mE8.m0_position;
    pThis->m84.m34_pRotation = &pThis->mE8.mC_rotation;
    pThis->m84.m38_pOwner = pThis;
    pThis->m84.m3C_scriptEA = readSaturnEA(arg + 0x38);
    if (u16 offset = readSaturnU16(arg + 0x36))
    {
        pThis->m84.m40 = pThis->m0_fileBundle->getRawFileAtOffset(offset);
    }
    else
    {
        pThis->m84.m40 = 0;
    }

    mainLogicInitSub0(&pThis->m84, readSaturnU8(arg + 0x34));
    mainLogicInitSub1(&pThis->m84, readSaturnVec3(arg + 0x3C), readSaturnVec3(arg + 0x48));
    initEdgeNPCSub1(pThis);
    pThis->m17B = 0;
}

void sEdgeTask::Init(sEdgeTask* pThis, sSaturnPtr arg)
{
    initEdgeNPC(pThis, arg);

    s_fileBundle* pBundle = pThis->m0_fileBundle;
    u32 modelIndex = readSaturnU16(arg + 0x22);
    sStaticPoseData* pStaticPoseData = pBundle->getStaticPose(readSaturnU16(arg + 0x24), pBundle->getModelHierarchy(modelIndex)->countNumberOfBones());

    init3DModelRawData(pThis, &pThis->m34_3dModel, 0x100, pBundle, modelIndex, nullptr, pStaticPoseData, nullptr, nullptr);

    if (readSaturnU8(arg + 0x21) & 0x40)
    {
        assert(0);
    }
}

void updateEdgeSub1(sEdgeTask* pThis)
{
    if (pThis->mE_controlState == 0)
    {
        pThis->mC &= ~2;
    }
}

void updateEdgeSub2(sEdgeTask* pThis)
{
    sNPCE8* r13 = &pThis->mE8;
    if (pThis->mF & 0x2)
    {
        //605ACC6
        fixedPoint r4 = MTH_Mul(0x2D82D8, pThis->m1C);
        fixedPoint r6 = -r4;

        if (pThis->mF & 0x4)
        {
            assert(0);
        }
        else
        {
            assert(0);
        }
    }
    else
    {
        //0605AE18
        if (pThis->mF & 1)
        {
            fixedPoint r8 = FP_Pow2(r13->m30_stepTranslation[2]);
            if (distanceSquareBetween2Points(r13->m3C_targetPosition, r13->m0_position) <= r8)
            {
                // reached destination
                r13->m0_position = r13->m3C_targetPosition;
                pThis->mF &= ~1;
            }
            else
            {
                fixedPoint r4_angleDifferenceToTarget = atan2_FP(r13->m0_position[0] - r13->m3C_targetPosition[0], r13->m0_position[2] - r13->m3C_targetPosition[2]) - r13->mC_rotation[1];
                r4_angleDifferenceToTarget = r4_angleDifferenceToTarget.normalized();

                if (r4_angleDifferenceToTarget >= 0)
                {
                    if (r4_angleDifferenceToTarget > 0x2D82D8)
                        r4_angleDifferenceToTarget = 0x2D82D8;
                }
                else
                {
                    if (r4_angleDifferenceToTarget < -0x2D82D8)
                        r4_angleDifferenceToTarget = -0x2D82D8;
                }

                r13->mC_rotation[1] += r4_angleDifferenceToTarget;
                stepNPCForward(&pThis->mE8);
            }
        }
        else
        {
            //605AECC
            pThis->mC &= ~4;
        }
    }
}

void sEdgeTask::Update(sEdgeTask* pThis)
{
    sNPCE8* r12 = &pThis->mE8;

    pThis->mE8.m54_oldPosition = pThis->mE8.m0_position;

    if (pThis->mC)
    {
        //auto walk
        if (!(pThis->mF & 2) && !(pThis->mF & 8))
        {
            pThis->m20_lookAtAngle[1] = MTH_Mul(pThis->m20_lookAtAngle[1], 0xB333);
        }

        if (pThis->mC & 2)
        {
            //0x0605A000
            updateEdgeSub1(pThis);
        }

        if (pThis->mC & 4)
        {
            //0x0605A00A
            updateEdgeSub2(pThis);
        }
    }
    else
    {
        pThis->m14_updateFunction(pThis);
    }

    //605A01E
    switch (pThis->mE_controlState)
    {
    case 0:
        if (pThis->m17A)
        {
            //605A07C
            assert(0);
        }
        break;
    case 1:
    {
        sVec3_FP var0 = r12->m0_position - r12->m54_oldPosition;
        var0 *= var0;
        s32 r4 = sqrt_I(var0[0] + var0[1] + var0[2]) * 0x1E1;
        s32 r12 = pThis->m28_animationLeftOver + r4;
        pThis->m28_animationLeftOver = r12 & 0xFFFF;
        if (r4)
        {
            //0x605A1D0
            if (pThis->m2C_currentAnimation != 1)
            {
                pThis->m2C_currentAnimation = 1;
                sSaturnPtr var0 = pThis->m30_animationTable + 4 * pThis->m2C_currentAnimation; // walk animation

                u32 offset = readSaturnU16(var0 + 2);
                sAnimationData* buffer;
                if (readSaturnU16(var0))
                {
                    buffer = dramAllocatorEnd[0].mC_fileBundle->m0_fileBundle->getAnimation(offset);
                }
                else
                {
                    buffer = pThis->m0_fileBundle->getAnimation(offset);
                }

                playAnimationGeneric(&pThis->m34_3dModel, buffer, 5);
            }
            r12 >>= 16;
        }
        else
        {
            //0x605A206
            if (pThis->m2C_currentAnimation)
            {
                //0x605A20C
                pThis->m2C_currentAnimation = 0;
                sSaturnPtr var0 = pThis->m30_animationTable + 4 * pThis->m2C_currentAnimation; // stand animation

                u32 offset = readSaturnU16(var0 + 2);
                sAnimationData* buffer;
                if (readSaturnU16(var0))
                {
                    buffer = dramAllocatorEnd[0].mC_fileBundle->m0_fileBundle->getAnimation(offset);
                }
                else
                {
                    buffer = pThis->m0_fileBundle->getAnimation(offset);
                }

                playAnimationGeneric(&pThis->m34_3dModel, buffer, 5);

            }

            r12 = 1;
        }

        if (r12)
        {
            do
            {
                stepAnimation(&pThis->m34_3dModel);
            } while (--r12);
        }

        interpolateAnimation(&pThis->m34_3dModel);

        break;
    }
    case 4:
        updateEdgeSub3(pThis);
        break;
    default:
        assert(0);
        break;
    }

    EdgeUpdateSub0(&pThis->m84);
}

void updateEdgeSub3Sub0(sEdgeTask* pThis)
{
    if ((pThis->m14E-- == 0) || (pThis->m2C_currentAnimation < 5) || (pThis->m2C_currentAnimation > 8))
    {
        sSaturnPtr r13 = gTWN_RUIN->getSaturnPtr(0x60604B0);
        s16 r12;
        if ((readSaturnS16(r13 + 4) != pThis->m2C_currentAnimation) || (npcData0.mFC & 0x11))
        {
            //0605C1FE
            r12 = readSaturnS16(r13 + 4);
            pThis->m14E = readSaturnS16(r13 + 2) + performModulo2(readSaturnU16(r13 + 2), randomNumber() & 0x7FFFFFFF);
        }
        else
        {
            //0605C24C
            u32 r4 = performModulo2(readSaturnU16(r13 + 6), randomNumber() & 0x7FFFFFFF);
            sSaturnPtr r4Bis;
            if (r4 <= readSaturnU8(r13 + 8))
            {
                r4Bis = r13 + 8;
            }
            else if (r4 <= readSaturnU8(r13 + 0xC))
            {
                r4Bis = r13 + 0xC;
            }
            else
            {
                r4Bis = r13 + 0x10;
            }

            //605C27E
            r12 = readSaturnS16(r4Bis + 2);
            pThis->m14E = readSaturnU8(r4Bis + 1);
        }

        //0605C286
        if (pThis->m2C_currentAnimation != r12)
        {
            s32 r6 = 0xA;
            if (pThis->m2C_currentAnimation < 5)
            {
                r6 = 5;
            }

            pThis->m2C_currentAnimation = r12;
            sSaturnPtr var0 = pThis->m30_animationTable + pThis->m2C_currentAnimation * 4;

            u32 offset = readSaturnU16(var0 + 2);
            sAnimationData* buffer;
            if (readSaturnU16(var0))
            {
                buffer = dramAllocatorEnd[0].mC_fileBundle->m0_fileBundle->getAnimation(offset);
            }
            else
            {
                buffer = pThis->m0_fileBundle->getAnimation(offset);
            }

            playAnimationGeneric(&pThis->m34_3dModel, buffer, r6);
        }
    }

    //605C2C6
    updateAndInterpolateAnimation(&pThis->m34_3dModel);
}

void updateEdgeSub3(sEdgeTask* pThis)
{
    sNPCE8* r13 = &pThis->mE8;

    sVec3_FP delta = r13->m0_position - r13->m54_oldPosition;
    sVec3_FP deltaSquare = delta * delta;

    s32 r4 = sqrt_I(deltaSquare[0] + deltaSquare[1] + deltaSquare[2]) * 0x1E1;
    s32 animCounter = (pThis->m28_animationLeftOver + r4);
    pThis->m28_animationLeftOver = animCounter & 0xFFFF;

    if (r4 > 0x666)
    {
        //605C33C
        switch (pThis->m2C_currentAnimation)
        {
        case 2:
            if (r4 < 0x28000)
            {
                //0605C34C
                pThis->m2C_currentAnimation = 1; // run to walk
                sSaturnPtr var0 = pThis->m30_animationTable + 4 * pThis->m2C_currentAnimation;

                u32 offset = readSaturnU16(var0 + 2);
                sAnimationData* buffer;
                if (readSaturnU16(var0))
                {
                    buffer = dramAllocatorEnd[0].mC_fileBundle->m0_fileBundle->getAnimation(offset);
                }
                else
                {
                    buffer = pThis->m0_fileBundle->getAnimation(offset);
                }

                playAnimationGeneric(&pThis->m34_3dModel, buffer, 5);
            }
            break;
        case 1:
            if (r4 > 0x30000)
            {
                //0605C3AE
                pThis->m2C_currentAnimation = 2; // run animation
                sSaturnPtr var0 = pThis->m30_animationTable + 4 * pThis->m2C_currentAnimation;

                u32 offset = readSaturnU16(var0 + 2);
                sAnimationData* buffer;
                if (readSaturnU16(var0))
                {
                    buffer = dramAllocatorEnd[0].mC_fileBundle->m0_fileBundle->getAnimation(offset);
                }
                else
                {
                    buffer = pThis->m0_fileBundle->getAnimation(offset);
                }

                playAnimationGeneric(&pThis->m34_3dModel, buffer, 5);
                break;

            }
            break;
        default:
            //0x605C3DA
            {
                pThis->m2C_currentAnimation = 1; // walk animation
                sSaturnPtr var0 = pThis->m30_animationTable + 4 * pThis->m2C_currentAnimation;

                u32 offset = readSaturnU16(var0 + 2);
                sAnimationData* buffer;
                if (readSaturnU16(var0))
                {
                    buffer = dramAllocatorEnd[0].mC_fileBundle->m0_fileBundle->getAnimation(offset);
                }
                else
                {
                    buffer = pThis->m0_fileBundle->getAnimation(offset);
                }

                playAnimationGeneric(&pThis->m34_3dModel, buffer, 5);
                break;
            }
        }

        //0605C402
        animCounter >>= 16;
        if (animCounter)
        {
            if (pThis->m2C_currentAnimation == 1)
            {
                do
                {
                    s32 frameIndex = stepAnimation(&pThis->m34_3dModel);
                    if ((frameIndex == 8) || (frameIndex == 0x1B))
                    {
                        playSystemSoundEffect(0x22);
                    }
                } while (--animCounter);
            }
            else
            {
                do
                {
                    s32 frameIndex = stepAnimation(&pThis->m34_3dModel);
                    if ((frameIndex == 0xB) || (frameIndex == 0x2B))
                    {
                        playSystemSoundEffect(0x23);
                    }
                } while (--animCounter);
            }
        }
        interpolateAnimation(&pThis->m34_3dModel);
        pThis->m14E = 0;
    }
    else
    {
        //605C45E
        updateEdgeSub3Sub0(pThis);
    }
}

void sEdgeTask::Draw(sEdgeTask* pThis)
{
    pushCurrentMatrix();
    translateCurrentMatrix(pThis->mE8.m0_position);
    rotateCurrentMatrixShiftedY(pThis->mE8.mC_rotation[1]);
    rotateCurrentMatrixShiftedX(pThis->mE8.mC_rotation[0]);
    rotateCurrentMatrixShiftedY(0x8000000);

    // draw the shadow
    if ((pThis->mF & 0x80) == 0)
    {
        addObjectToDrawList(dramAllocatorEnd[0].mC_fileBundle->m0_fileBundle->get3DModel(readSaturnU16(pThis->m30_animationTable + 2)));
    }

    if (pThis->m34_3dModel.m48_poseDataInterpolation.size())
    {
        applyEdgeAnimation2(&pThis->m34_3dModel, &pThis->m20_lookAtAngle);
    }
    else
    {
        applyEdgeAnimation(&pThis->m34_3dModel, &pThis->m20_lookAtAngle);
    }

    popMatrix();
}

void sEdgeTask::Delete(sEdgeTask* pThis)
{
    FunctionUnimplemented();
}
