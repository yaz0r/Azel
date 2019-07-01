#include "PDS.h"
#include "twn_ruin.h"
#include "town/town.h"
#include "town/townScript.h"

void updateEdgePosition(sNPC* r4);

struct TWN_RUIN_data : public sSaturnMemoryFile
{
    void init()
    {

    }
};

TWN_RUIN_data* gTWN_RUIN = NULL;

u8 townVDP1Buffer[0x63800];

const char* listOfFilesToLoad[] = {
    "COMMON3.MCB",
    "COMMON3.CGB",
    "RUINMP.MCB",
    "RUINMP.CGB",
    nullptr
};

void townOverlayDelete(townDebugTask2Function* pThis)
{
    FunctionUnimplemented();
}

struct sRuinBackgroundTask : public s_workAreaTemplate<sRuinBackgroundTask>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sRuinBackgroundTask::Init, &sRuinBackgroundTask::Update, &sRuinBackgroundTask::Draw, nullptr };
        return &taskDefinition;
    }

    static void Init(sRuinBackgroundTask* pThis)
    {
        reinitVdp2();
        FunctionUnimplemented();
    }

    static void Update(sRuinBackgroundTask* pThis)
    {
        FunctionUnimplemented();
    }

    static void Draw(sRuinBackgroundTask* pThis)
    {
        FunctionUnimplemented();
    }

    // size 0x9C
};

void startRuinBackgroundTask(p_workArea pThis)
{
    createSubTask<sRuinBackgroundTask>(pThis);
}

void registerNpcs(sSaturnPtr r4_objectTable, sSaturnPtr r5_script, s32 r6)
{
    npcData0.m0_numExtraScriptsIterations = 0;
    npcData0.m5E = -1;
    npcData0.m60 = r4_objectTable;
    npcData0.mFC = 0;
    npcData0.m100 = 0;
    npcData0.m11C_currentStackPointer = npcData0.m120_stack.end();
    npcData0.m164 = 0;
    npcData0.m168 = 0;
    npcData0.m16C_displayStringTask = 0;
    npcData0.m170 = 0;

    for (int i = 0; i < npcData0.m70_npcPointerArray.size(); i++)
    {
        npcData0.m70_npcPointerArray[i] = nullptr;
    }

    townVar0 = nullptr;

    npcData0.mFC |= 0xF;

    npcData0.m104_scriptPtr = r5_script;
    npcData0.m108 = 0;
    npcData0.m10C = 0;

    npcData0.m116 = r6;

    npcData0.mF0 = 0;
    npcData0.mF4 = 0;
}

void applyAnimation(u8* base, u32 offset, std::vector<sPoseData>::iterator& pose)
{
    u8* r13 = base + offset;

    pushCurrentMatrix();
    {
        translateCurrentMatrix(&pose->m0_translation);
        rotateCurrentMatrixZYX(&pose->mC_rotation);
        if (READ_BE_U32(r13))
        {
            addObjectToDrawList(base, READ_BE_U32(r13));
        }
        if (READ_BE_U32(r13 + 4))
        {
            pose++;
            applyAnimation(base, READ_BE_U32(r13 + 4), pose);
        }
    }
    popMatrix();
    if (READ_BE_U32(r13 + 8))
    {
        pose++;
        applyAnimation(base, READ_BE_U32(r13 + 8), pose);
    }

}

void applyAnimation2(u8* base, u32 offset, std::vector<sPoseDataInterpolation>::iterator pose)
{
    u8* r13 = base + offset;

    pushCurrentMatrix();
    {
        translateCurrentMatrix(&pose->m0_translation);
        rotateCurrentMatrixZYX(&pose->mC_rotation);
        if (READ_BE_U32(r13))
        {
            addObjectToDrawList(base, READ_BE_U32(r13));
        }
        if (READ_BE_U32(r13 + 4))
        {
            pose++;
            applyAnimation2(base, READ_BE_U32(r13 + 4), pose);
        }
    }
    popMatrix();
    if (READ_BE_U32(r13 + 8))
    {
        pose++;
        applyAnimation2(base, READ_BE_U32(r13 + 8), pose);
    }

}

void applyEdgeAnimation(s_3dModel* pModel, sVec2_FP* r5)
{
    std::vector<sPoseData>::iterator r14_pose = pModel->m2C_poseData.begin();
    u8* r12 = pModel->m4_pModelFile + READ_BE_U32(pModel->m4_pModelFile + pModel->mC_modelIndexOffset);
    r12 = pModel->m4_pModelFile + READ_BE_U32(r12 + 4);

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

            if (READ_BE_U32(r12))
            {
                addObjectToDrawList(pModel->m4_pModelFile, READ_BE_U32(r12));
            }

            u8* r13 = pModel->m4_pModelFile + READ_BE_U32(r12 + 4);
            pushCurrentMatrix();
            {
                r14_pose++;
                translateCurrentMatrix(&r14_pose->m0_translation);
                rotateCurrentMatrixShiftedZ(r14_pose->mC_rotation[2]);
                rotateCurrentMatrixShiftedY(r14_pose->mC_rotation[1] + MTH_Mul(r5->m_value[1], 0xB333));
                rotateCurrentMatrixShiftedX(r14_pose->mC_rotation[0]);

                if (READ_BE_U32(r13))
                {
                    addObjectToDrawList(pModel->m4_pModelFile, READ_BE_U32(r13));
                }

                if (READ_BE_U32(r13 + 4))
                {
                    r14_pose++;
                    applyAnimation(pModel->m4_pModelFile, READ_BE_U32(r13 + 4), r14_pose);
                }
            }
            popMatrix();

            if (READ_BE_U32(r13 + 8))
            {
                r14_pose++;
                applyAnimation(pModel->m4_pModelFile, READ_BE_U32(r13 + 8), r14_pose);
            }
        }
        popMatrix();

        if (READ_BE_U32(r12 + 8))
        {
            r14_pose++;
            applyAnimation(pModel->m4_pModelFile, READ_BE_U32(r12 + 8), r14_pose);
        }
    }
    popMatrix();
}


void applyEdgeAnimation2(s_3dModel* pModel, sVec2_FP* r5)
{
    std::vector<sPoseDataInterpolation>::iterator r14_pose = pModel->m48_poseDataInterpolation.begin();
    u8* r12 = pModel->m4_pModelFile + READ_BE_U32(pModel->m4_pModelFile + pModel->mC_modelIndexOffset);
    r12 = pModel->m4_pModelFile + READ_BE_U32(r12 + 4);

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

            if (READ_BE_U32(r12))
            {
                addObjectToDrawList(pModel->m4_pModelFile, READ_BE_U32(r12));
            }

            u8* r13 = pModel->m4_pModelFile + READ_BE_U32(r12 + 4);
            pushCurrentMatrix();
            {
                r14_pose++;
                translateCurrentMatrix(&r14_pose->m0_translation);
                rotateCurrentMatrixShiftedZ(r14_pose->mC_rotation[2]);
                rotateCurrentMatrixShiftedY(r14_pose->mC_rotation[1] + MTH_Mul(r5->m_value[1], 0xB333));
                rotateCurrentMatrixShiftedX(r14_pose->mC_rotation[0]);

                if (READ_BE_U32(r13))
                {
                    addObjectToDrawList(pModel->m4_pModelFile, READ_BE_U32(r13));
                }

                if (READ_BE_U32(r13 + 4))
                {
                    r14_pose++;
                    applyAnimation2(pModel->m4_pModelFile, READ_BE_U32(r13 + 4), r14_pose);
                }
            }
            popMatrix();

            if (READ_BE_U32(r13 + 8))
            {
                r14_pose++;
                applyAnimation2(pModel->m4_pModelFile, READ_BE_U32(r13 + 8), r14_pose);
            }
        }
        popMatrix();

        if (READ_BE_U32(r12 + 8))
        {
            r14_pose++;
            applyAnimation2(pModel->m4_pModelFile, READ_BE_U32(r12 + 8), r14_pose);
        }
    }
    popMatrix();

}

void EdgeUpdateSub0(sMainLogic_74* r14_pose)
{
    if (resData.m4 >= 0x3F)
        return;

    sResData1C& r5 = resData.m1C[resData.m4++];
    r5.m0_pNext = resData.m8_headOfLinkedList[r14_pose->m2C];
    r5.m4 = r14_pose;

    resData.m8_headOfLinkedList[r14_pose->m2C] = &r5;

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

struct sEdgeTask : public s_workAreaTemplateWithArgWithCopy<sEdgeTask, sSaturnPtr>, sNPC
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sEdgeTask::Init, &sEdgeTask::Update, &sEdgeTask::Draw, &sEdgeTask::Delete };
        return &taskDefinition;
    }

    static void initEdgeNPCSub0(sEdgeTask* pThis, s32 r5, sSaturnPtr r6)
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
            assert(r6.m_file == gTWN_RUIN);
            assert(r6.m_offset == 0x605B8D4);
            pThis->m14_updateFunction = &updateEdgePosition;
            break;
        default:
            assert(0);
            break;
        }
    }

    static void initEdgeNPCSub1(sEdgeTask* pThis)
    {
        pThis->m179 = 0;
        pThis->m178 = 0;
        pThis->m17A = 0;
    }

    static void initEdgeNPC(sEdgeTask* pThis, sSaturnPtr arg)
    {
        npcData0.m70_npcPointerArray[readSaturnU8(arg + 0x20)] = pThis;
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
        pThis->m84.m3C = readSaturnS32(arg + 0x38);
        if (u16 offset = readSaturnU16(arg + 0x36))
        {
            pThis->m84.m40 = pThis->m0_dramAllocation + offset;
        }
        else
        {
            pThis->m84.m40 = nullptr;
        }

        mainLogicInitSub0(&pThis->m84, readSaturnU8(arg + 0x34));
        mainLogicInitSub1(&pThis->m84, arg + 0x3C, arg + 0x48);
        initEdgeNPCSub1(pThis);
        pThis->m17B = 0;
    }

    static void Init(sEdgeTask* pThis, sSaturnPtr arg)
    {
        initEdgeNPC(pThis, arg);

        init3DModelRawData(pThis, &pThis->m34_3dModel, 0x100, pThis->m0_dramAllocation, readSaturnU16(arg + 0x22), nullptr, pThis->m0_dramAllocation + READ_BE_U32(pThis->m0_dramAllocation + readSaturnU16(arg + 0x24)), nullptr, nullptr);

        if (readSaturnU8(arg + 0x21) & 0x40)
        {
            assert(0);
        }
    }

    static void updateEdgeSub1(sEdgeTask* pThis)
    {
        if (pThis->mE_controlState == 0)
        {
            pThis->mC &= ~2;
        }
    }

    static void updateEdgeSub2(sEdgeTask* pThis)
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

    static void Update(sEdgeTask* pThis)
    {
        sNPCE8* r12 = &pThis->mE8;

        pThis->mE8.m54_oldPosition = pThis->mE8.m0_position;

        if (pThis->mC)
        {
            //auto walk
            if (!(pThis->mF & 2) && !(pThis->mF & 8))
            {
                pThis->m20[1] = MTH_Mul(pThis->m20[1], 0xB333);
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
            s32 r12 = pThis->m28 + r4;
            pThis->m28 = r12 & 0xFFFF;
            if (r4)
            {
                //0x605A1D0
                if (pThis->m2C_currentAnimation != 1)
                {
                    pThis->m2C_currentAnimation = 1;
                    sSaturnPtr var0 = pThis->m30_animationTable + 4; // walk animation
                    u8* buffer;
                    if (readSaturnU16(var0))
                    {
                        buffer = dramAllocatorEnd[0].mC_buffer->m0_dramAllocation;
                    }
                    else
                    {
                        buffer = pThis->m0_dramAllocation;
                    }

                    playAnimationGeneric(&pThis->m34_3dModel, buffer + READ_BE_U32(readSaturnU16(var0 + 2) + buffer), 5);
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
                    sSaturnPtr var0 = pThis->m30_animationTable; // stand animation
                    u8* buffer;
                    if (readSaturnU16(var0))
                    {
                        buffer = dramAllocatorEnd[0].mC_buffer->m0_dramAllocation;
                    }
                    else
                    {
                        buffer = pThis->m0_dramAllocation;
                    }

                    playAnimationGeneric(&pThis->m34_3dModel, buffer + READ_BE_U32(readSaturnU16(var0 + 2) + buffer), 5);

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
        default:
            assert(0);
            break;
        }

        EdgeUpdateSub0(&pThis->m84);
    }

    static void Draw(sEdgeTask* pThis)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(pThis->mE8.m0_position);
        rotateCurrentMatrixShiftedY(pThis->mE8.mC_rotation[1]);
        rotateCurrentMatrixShiftedX(pThis->mE8.mC_rotation[0]);
        rotateCurrentMatrixShiftedY(0x8000000);

        // draw the shadow
        if (pThis->mF & 0x80)
        {
            addObjectToDrawList(dramAllocatorEnd[0].mC_buffer->m0_dramAllocation, READ_BE_U32(dramAllocatorEnd[0].mC_buffer->m0_dramAllocation + readSaturnU16(pThis->m30_animationTable + 2)));
        }

        if (pThis->m34_3dModel.m48_poseDataInterpolation.size())
        {
            applyEdgeAnimation2(&pThis->m34_3dModel, &pThis->m20);
        }
        else
        {
            applyEdgeAnimation(&pThis->m34_3dModel, &pThis->m20);
        }

        popMatrix();
    }

    static void Delete(sEdgeTask* pThis)
    {
        FunctionUnimplemented();
    }

    s16 m14C_inputFlags;
    s8 m14E;
    s32 m150_inputX;
    s32 m154_inputY;
    s8 m178;
    s8 m179;
    s8 m17A;
    s8 m17B;
    //size 0x17C
};

sEdgeTask* startEdgeTask(sSaturnPtr r4)
{
    return createSiblingTaskWithArgWithCopy<sEdgeTask>(allocateNPC(currentResTask, readSaturnS32(r4)), r4);
}

struct sMainLogic* twnMainLogicTask;

void mainLogicDummy(struct sMainLogic*)
{

}

void mainLogicUpdateSub3();

void mainLogicInitSub2()
{
    initVDP1Projection(0x1C71C71, 0);
}

void drawLcs()
{
    if (npcData0.mFC & 0x10)
    {
        assert(0);
    }
    else
    {
        if (enableDebugTask)
        {
            assert(0);
        }
    }
}

void Imgui_FP(const char* label, fixedPoint* pFP)
{
    float fValue = pFP->toFloat();
    if (ImGui::InputFloat(label, &fValue, 0.01, 0.1))
    {
        pFP->m_value = fValue * 0x10000;
    }
}

void Imgui_Vec3FP(sVec3_FP* pVector)
{
    ImGui::PushItemWidth(100);
    Imgui_FP("x", &pVector->m_value[0]); ImGui::SameLine();
    Imgui_FP("y", &pVector->m_value[1]); ImGui::SameLine();
    Imgui_FP("z", &pVector->m_value[2]);
    ImGui::PopItemWidth();
}

void Imgui_Vec3FP(const char* name, sVec3_FP* pVector)
{
    ImGui::Text(name); ImGui::SameLine();
    ImGui::PushID(name);
    Imgui_Vec3FP(pVector);
    ImGui::PopID();
}

struct sMainLogic : public s_workAreaTemplate<sMainLogic>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sMainLogic::Init, &sMainLogic::Update, &sMainLogic::Draw, NULL };
        return &taskDefinition;
    }

    static void Init(sMainLogic* pThis)
    {
        twnMainLogicTask = pThis;

        if (mainGameState.getBit(0x274, 7))
        {
            pThis->m1 = 1;
        }
        else
        {
            pThis->m1 = 0;
        }
        pThis->m0 = 0;
        pThis->m10 = &mainLogicDummy;

        resetMatrixStack();

        pThis->m74.m30_pPosition = &pThis->m5C_position;
        pThis->m74.m34_pRotation = &pThis->m68_rotation;
        pThis->m74.m38_pOwner = pThis;
        pThis->m74.m3C = 0;
        pThis->m74.m40 = nullptr;

        mainLogicInitSub0(&pThis->m74, 0);
        mainLogicInitSub1(&pThis->m74, gTWN_RUIN->getSaturnPtr(0x605EEE4), gTWN_RUIN->getSaturnPtr(0x605EEF0));

        npcData0.mFC &= ~0x10;

        mainLogicInitSub2();
    }

    // read inputs
    static void mainLogicUpdateSub1(sMainLogic* pThis)
    {
        if (!(npcData0.mFC & 8))
        {
            if (npcData0.mFC & 0x10)
            {
                if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 1)
                {
                    pThis->m4_flags |= 0x4000000;
                }
            }
            else
            {
                //605578C
                if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & graphicEngineStatus.m4514.mD8_buttonConfig[0][2])
                {
                    pThis->m4_flags |= 0x8000000;
                }
            }
        }

        //0x60557A0
        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & graphicEngineStatus.m4514.mD8_buttonConfig[0][13])
        {
            pThis->m4_flags |= 0x20000000;
        }

        s32 r6 = 0;
        s32 r7 = 0;
        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m0_inputType == 2)
        {
            //60557BA
            assert(0);
        }
        else
        {
            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[0][4])
            {
                r6 = 0;
                r7 = 0x10000;
            }
            else if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[0][5])
            {
                r6 = 0;
                r7 = -0x10000;
            }

            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[0][6])
            {
                r6 = 0x10000;
                r7 = 0;
            }
            else if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[0][7])
            {
                r6 = -0x10000;
                r7 = -0;
            }
        }

        //6055820
        pThis->m8_inputX = r6;
        pThis->mC_inputY = r7;
    }

    static void mainLogicUpdateSub2(sMainLogic* pThis)
    {
        if (pThis->m4_flags & 0x4000000)
        {
            assert(0);
        }
        else if (pThis->m4_flags & 0x8000000)
        {
            assert(0);
        }

        //60558AE
        if (pThis->m4_flags & 0x10000000)
        {
            assert(0);
        }

        if (pThis->m4_flags & 0x40000000)
        {
            pThis->m0 ^= 1;
        }
    }

    static void mainLogicUpdateSub4(sMainLogic* pThis)
    {
        if (pThis->m14_EdgeTask)
        {
            sNPCE8* r5 = &pThis->m14_EdgeTask->mE8;
            if (pThis->m118)
            {
                assert(0);
            }
        }
    }

    static void Update(sMainLogic* pThis)
    {
        if (pThis->m14_EdgeTask)
        {
            sEdgeTask* r13 = pThis->m14_EdgeTask;
            mainLogicUpdateSub0(r13->mE8.m0_position[0], r13->mE8.m0_position[2]);
            pThis->m18_position = r13->mE8.m0_position;
            pThis->m18_position[1] += 0x1800;
        }

        mainLogicUpdateSub1(pThis);

        mainLogicUpdateSub2(pThis);

        if (!(npcData0.mFC & 1))
        {
            mainLogicUpdateSub3();
        }

        pThis->m10(pThis);

        // Hack
        {
            static bool forceCameraPosition = false;
            ImGui::Begin("Town");
            {
                ImGui::Checkbox("Force camera position", &forceCameraPosition);
                Imgui_Vec3FP("Camera position", &pThis->m38_cameraPosition);
                Imgui_Vec3FP("Camera target", &pThis->m44_cameraTarget);
            }
            ImGui::End();

            if (forceCameraPosition && pThis->m14_EdgeTask)
            {
                pThis->m38_cameraPosition = pThis->m14_EdgeTask->mE8.m0_position - sVec3_FP(0,0,0x5000);
                pThis->m44_cameraTarget = pThis->m14_EdgeTask->mE8.m0_position;
            }
        }

        pThis->m50_upVector = pThis->m38_cameraPosition;
        pThis->m50_upVector[1] += 0x10000;
        pThis->m4_flags = 0;

        mainLogicUpdateSub4(pThis);
    }

    static void Draw(sMainLogic* pThis)
    {
        sVec3_FP var18 = pThis->m38_cameraPosition + ((pThis->m44_cameraTarget - pThis->m38_cameraPosition) * 16);

        generateCameraMatrix(&cameraProperties2, pThis->m38_cameraPosition, var18, pThis->m50_upVector);

        drawLcs();

        if (enableDebugTask)
        {
            assert(0);
        }
    }

    s8 m0;
    s8 m1;
    s8 m2_cameraFollowMode;
    s32 m4_flags;
    s32 m8_inputX;
    s32 mC_inputY;
    void (*m10)(sMainLogic*);
    sEdgeTask* m14_EdgeTask;
    sVec3_FP m18_position;
    fixedPoint m24_distance;
    fixedPoint m2C;
    fixedPoint m30;
    sVec3_FP m38_cameraPosition;
    sVec3_FP m44_cameraTarget;
    sVec3_FP m50_upVector;
    sVec3_FP m5C_position;
    sVec3_FP m68_rotation;
    sMainLogic_74 m74;
    s32 m118;
    // size 0x320
};

const std::array<sVec2_FP, 2> cameraParams = {
    {
        {0x2CCC, 0xAAAAAA},
        {0x2CCC, -0x555555},
    }
};

fixedPoint transformByMatrixRow0(const sVec3_FP& r4)
{
    s64 mac = 0;
    mac += (s64)pCurrentMatrix->matrix[0] * (s64)r4[0].asS32();
    mac += (s64)pCurrentMatrix->matrix[1] * (s64)r4[1].asS32();
    mac += (s64)pCurrentMatrix->matrix[2] * (s64)r4[2].asS32();
    return (mac >> 16) + pCurrentMatrix->matrix[3];
}

fixedPoint transformByMatrixRow1(const sVec3_FP& r4)
{
    s64 mac = 0;
    mac += (s64)pCurrentMatrix->matrix[4] * (s64)r4[0].asS32();
    mac += (s64)pCurrentMatrix->matrix[5] * (s64)r4[1].asS32();
    mac += (s64)pCurrentMatrix->matrix[6] * (s64)r4[2].asS32();
    return (mac >> 16) + pCurrentMatrix->matrix[7];
}

fixedPoint transformByMatrixRow2(const sVec3_FP& r4)
{
    s64 mac = 0;
    mac += (s64)pCurrentMatrix->matrix[8] * (s64)r4[0].asS32();
    mac += (s64)pCurrentMatrix->matrix[9] * (s64)r4[1].asS32();
    mac += (s64)pCurrentMatrix->matrix[10] * (s64)r4[2].asS32();
    return (mac >> 16) + pCurrentMatrix->matrix[11];
}

fixedPoint MulVec2(const sVec2_FP& r4, const sVec2_FP& r5)
{
    s64 mac = 0;
    mac += (s64)r4.m_value[0] * (s64)r5.m_value[0];
    mac += (s64)r4.m_value[1] * (s64)r5.m_value[1];
    
    return fixedPoint(mac >> 16);
}

void updateCameraTarget(sMainLogic* r4, const sVec3_FP& r14_pose)
{
    // project the point to screen

    sVec3_FP var8;
    var8[0] = transformByMatrixRow0(r14_pose);
    var8[1] = transformByMatrixRow1(r14_pose);
    var8[2] = transformByMatrixRow2(r14_pose);

    sVec2_FP var0;
    var0[0] = FP_Div(var8[0], var8[2]);
    var0[1] = FP_Div(var8[1], var8[2]);

    if (var0[0] < 0)
    {
        var0[0] = -var0[0];
    }

    if (var0[1] < 0)
    {
        var0[1] = -var0[1];
    }

    // If the projected target point is already at the center of the screen
    if ((MTH_Mul(graphicEngineStatus.m405C.m18, var0[0]) == 0) && (MTH_Mul(graphicEngineStatus.m405C.m18, var0[1]) == 0))
    {
        r4->m44_cameraTarget = r14_pose;
        return;
    }

    // If not, interpolate
    if (var0[0] > 0x800000)
        var0[0] = 0x800000;

    if (var0[1] > 0x800000)
        var0[1] = 0x800000;

    fixedPoint r13 = MulVec2(var0, var0) / 2;

    if (r13 > 0xB333)
        r13 = 0xB333;

    if (r13 < 0xCCC)
        r13 = 0xCCC;

    r13 = 0x10000 - r13;

    r4->m44_cameraTarget[0] = r14_pose[0] + MTH_Mul(r4->m44_cameraTarget[0] - r14_pose[0], r13);
    r4->m44_cameraTarget[1] = r14_pose[1] + MTH_Mul(r4->m44_cameraTarget[1] - r14_pose[1], r13);
    r4->m44_cameraTarget[2] = r14_pose[2] + MTH_Mul(r4->m44_cameraTarget[2] - r14_pose[2], r13);
}

void mainLogicUpdateSub5(sMainLogic* r4)
{
    sMatrix4x3 varC;

    const sNPCE8& r13 = r4->m14_EdgeTask->mE8;

    initMatrixToIdentity(&varC);
    rotateMatrixShiftedY(r13.m24_stepRotation[1] + r13.mC_rotation[1], &varC);
    rotateMatrixShiftedX(r13.m24_stepRotation[0] + r13.mC_rotation[0], &varC);
    scaleMatrixRow2(-r4->m24_distance / 2, &varC);

    sVec3_FP var0;
    var0[0] = varC.matrix[2] + r4->m18_position[0];
    var0[1] = varC.matrix[6] + r4->m18_position[1];
    var0[2] = varC.matrix[10] + r4->m18_position[2];

    updateCameraTarget(r4, var0);
}

void cameraFollowMode0Bis(sMainLogic* r14_pose)
{
    sEdgeTask* r4_edge = r14_pose->m14_EdgeTask;
    sNPCE8* r13_npcData = &r4_edge->mE8;

    if (r4_edge->mC & 4)
    {
        r14_pose->m2C = 0;
        r14_pose->m30 = 0;
        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[0][11])
        {
            r14_pose->m30 = 0x4000000;
            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[0][15])
            {
                r14_pose->m30 = 0x8000000;
            }
        }
        else
        {
            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[0][15])
            {
                r14_pose->m30 = 0xC000000;
            }
        }
    }

    //6055E90
    const sVec2_FP& cameraParam = cameraParams[r14_pose->m1];
    if ((MTH_Mul(0x151EB, cameraParam[0]) < r14_pose->m24_distance) && !(r14_pose->m4_flags &0x10000))
    {
        if (!(r4_edge->mC & 4))
        {
            //06055EBE
            (*r14_pose->m74.m30_pPosition)[0] += r14_pose->m74.m58_collisionSolveTranslation[0];
        }

        (*r14_pose->m74.m30_pPosition)[1] += r14_pose->m74.m58_collisionSolveTranslation[1];
        (*r14_pose->m74.m30_pPosition)[2] += r14_pose->m74.m58_collisionSolveTranslation[2];
    }

    //6055EE4
    sVec3_FP var4 = r14_pose->m18_position - r14_pose->m5C_position;
    r14_pose->m24_distance = sqrt_F(MTH_Product3d_FP(var4, var4));

    generateCameraMatrixSub1(var4, r14_pose->m68_rotation);

    s32 r4 = atan2_FP(0x174, r14_pose->m24_distance);
    if (r4 > 0x1555555)
    {
        r4 = 1555555;
    }

    fixedPoint r5 = cameraParam[1] - r14_pose->m68_rotation[0];
    fixedPoint r6 = r5.normalized();

    r5 = -r4;
    if (r6 > r4)
    {
        r6 = r4;
    }

    if (r6 < r5)
    {
        r6 = r5;
    }

    r14_pose->m68_rotation[0] += r6;

    fixedPoint r3 = r14_pose->m68_rotation[0];
    if (r3 > 0x13E93E9)
    {
        r14_pose->m68_rotation[0] = 0x13E93E9;
    }
    if (r3 < -0x13E93E9)
    {
        r14_pose->m68_rotation[0] = -0x13E93E9;
    }

    r6 = r13_npcData->mC_rotation[1] + r14_pose->m30 - r14_pose->m68_rotation[1];
    r6 = r6.normalized();

    if(((r6 < 0x71C71C7) && (r6 > -0x71C71C7)) || (r13_npcData->m54_oldPosition == r13_npcData->m0_position))
    {
        // 6055FFA
        if (r6 > r4)
            r6 = r4;
        if (r6 < r5)
            r6 = r5;

        r14_pose->m68_rotation[1] += r6;
    }

    // 605600E
    r4 = MTH_Mul(cameraParam[0] - r14_pose->m24_distance, 0x3333);
    if (r4 > 0x599)
        r4 = 0x599;
    if (r4 < -0x599)
        r4 = -0x599;

    r14_pose->m24_distance += r4;

    fixedPoint r13;
    if (r14_pose->m24_distance >= cameraParam[0])
    {
        r13 = 0xCCCC;
    }
    else
    {
        r13 = setDividend(r14_pose->m24_distance - 0x1000, 0xCCCC, cameraParam[0] - 0x1000);
        if (r13 < 0)
            r13 = 0;
    }

    //6056078
    sMatrix4x3 var10;
    initMatrixToIdentity(&var10);
    rotateMatrixShiftedY(r14_pose->m68_rotation[1], &var10);
    rotateMatrixShiftedX(r14_pose->m68_rotation[0], &var10);
    scaleMatrixRow2(r14_pose->m24_distance, &var10);

    r14_pose->m5C_position[0] = var10.matrix[2] + r14_pose->m18_position[0];
    r14_pose->m5C_position[1] = var10.matrix[6] + r14_pose->m18_position[1];
    r14_pose->m5C_position[2] = var10.matrix[10] + r14_pose->m18_position[2];

    r14_pose->m38_cameraPosition[0] = r14_pose->m5C_position[0] + MTH_Mul(r14_pose->m38_cameraPosition[0] - r14_pose->m5C_position[0], r13);
    r14_pose->m38_cameraPosition[1] = r14_pose->m5C_position[1] + MTH_Mul(r14_pose->m38_cameraPosition[1] - r14_pose->m5C_position[1], r13);
    r14_pose->m38_cameraPosition[2] = r14_pose->m5C_position[2] + MTH_Mul(r14_pose->m38_cameraPosition[2] - r14_pose->m5C_position[2], r13);

    EdgeUpdateSub0(&r14_pose->m74);
    mainLogicUpdateSub5(r14_pose);
}

void mainLogicUpdateSub3()
{
    if (twnMainLogicTask->m14_EdgeTask == nullptr)
        return;

    s32 r5_inLcsMode = npcData0.mFC & 0x10;

    switch (twnMainLogicTask->m2_cameraFollowMode)
    {
    case 0:
        if (r5_inLcsMode)
        {
            assert(0);
        }
        else
        {
            twnMainLogicTask->m10 = &cameraFollowMode0Bis;
        }
        break;
    default:
        assert(0);
        break;
    }
}


p_workArea startMainLogic(p_workArea pParent)
{
    return createSubTask<sMainLogic>(pParent);
}

p_workArea startCameraTask(p_workArea)
{
    return NULL;
}

s32* twnVar1;
s32 twnVar2 = 0x7FFFFFFF;

p_workArea overlayStart_TWN_RUIN(p_workArea pUntypedThis, u32 arg)
{
    // load data
    if (gTWN_RUIN == NULL)
    {
        FILE* fHandle = fopen("TWN_RUIN.PRG", "rb");
        assert(fHandle);

        fseek(fHandle, 0, SEEK_END);
        u32 fileSize = ftell(fHandle);

        fseek(fHandle, 0, SEEK_SET);
        u8* fileData = new u8[fileSize];
        fread(fileData, fileSize, 1, fHandle);
        fclose(fHandle);

        gTWN_RUIN = new TWN_RUIN_data();
        gTWN_RUIN->m_name = "TWN_RUIN.PRG";
        gTWN_RUIN->m_data = fileData;
        gTWN_RUIN->m_dataSize = fileSize;
        gTWN_RUIN->m_base = 0x6054000;

        gTWN_RUIN->init();
    }
    townDebugTask2Function* pThis = static_cast<townDebugTask2Function*>(pUntypedThis);

    pThis->m_DeleteMethod = &townOverlayDelete;

    playMusic(-1, 0);
    playMusic(59, 0);

    playPCM(pThis, 100);

    loadFnt("EVTRUIN.FNT");

    graphicEngineStatus.m405C.m10 = 0x800;
    graphicEngineStatus.m405C.m30 = FP_Div(0x10000, graphicEngineStatus.m405C.m10);

    graphicEngineStatus.m405C.m14_farClipDistance = 0xF000;
    graphicEngineStatus.m405C.m38 = FP_Div(0x8000, graphicEngineStatus.m405C.m14_farClipDistance);

    graphicEngineStatus.m405C.m34 = graphicEngineStatus.m405C.m38 << 8;

    initDramAllocator(pThis, townBuffer, sizeof(townBuffer), listOfFilesToLoad);

    allocateVramList(pThis, townVDP1Buffer, sizeof(townVDP1Buffer));

    registerNpcs(gTWN_RUIN->getSaturnPtr(0x605E984), gTWN_RUIN->getSaturnPtr(0x06054398), arg);

    startScriptTask(pThis);

    startRuinBackgroundTask(pThis);

    sEdgeTask* pEdgeTask = startEdgeTask(gTWN_RUIN->getSaturnPtr(0x605E990));

    npcData0.m160_pEdgePosition = &pEdgeTask->m84.m8_position;

    startMainLogic(pThis);

    twnMainLogicTask->m14_EdgeTask = pEdgeTask;

    startCameraTask(pThis);

    twnVar1 = &twnVar2;

    return pThis;
}

s32 TwnFadeOut(s32 arg0)
{
    fadePalette(&g_fadeControls.m0_fade0, convertColorToU32(g_fadeControls.m0_fade0.m0_color), 0x8000, arg0);
    fadePalette(&g_fadeControls.m24_fade1, convertColorToU32(g_fadeControls.m24_fade1.m0_color), 0x8000, arg0);
    graphicEngineStatus.m40AC.m1_isMenuAllowed = 0;
    return 0;
}

s32 TwnFadeIn(s32 arg0)
{
    FunctionUnimplemented();
    return 0;
}

s32 hasLoadingCompleted()
{
    FunctionUnimplemented();
    return 1;
}

void scriptFunction_6057058_sub0Sub0()
{
    sNPCE8* r13 = &twnMainLogicTask->m14_EdgeTask->mE8;
    twnMainLogicTask->m18_position = r13->m0_position;

    sMatrix4x3 var0;
    initMatrixToIdentity(&var0);
    translateMatrix(twnMainLogicTask->m18_position, &var0);
    rotateMatrixShiftedY(r13->mC_rotation[1], &var0);
    rotateMatrixShiftedX(r13->mC_rotation[0], &var0);

    twnMainLogicTask->m5C_position[0] = var0.matrix[3] + MTH_Mul(var0.matrix[2], 0x199);
    twnMainLogicTask->m5C_position[1] = var0.matrix[7] + MTH_Mul(var0.matrix[6], 0x199);
    twnMainLogicTask->m5C_position[2] = var0.matrix[11] + MTH_Mul(var0.matrix[10], 0x199);

    twnMainLogicTask->m38_cameraPosition = twnMainLogicTask->m5C_position;
    twnMainLogicTask->m50_upVector = twnMainLogicTask->m5C_position;
    twnMainLogicTask->m50_upVector[1] += 0x10000;

    twnMainLogicTask->m44_cameraTarget[0] = var0.matrix[3] + MTH_Mul(var0.matrix[2], -0x1000);
    twnMainLogicTask->m44_cameraTarget[1] = var0.matrix[7] + MTH_Mul(var0.matrix[6], -0x1000);
    twnMainLogicTask->m44_cameraTarget[2] = var0.matrix[11] + MTH_Mul(var0.matrix[10], -0x1000);
}

void scriptFunction_6057058_sub0()
{
    twnMainLogicTask->m_DrawMethod = sMainLogic::Draw;
    scriptFunction_6057058_sub0Sub0();
    twnMainLogicTask->m2_cameraFollowMode = 0;
    mainLogicUpdateSub3();
}

void setupNPCWalkInZDirection(s32 r4_npcIndex, s32 r5_zDirection, s32 r6_distance)
{
    sNPC* r13 = getNpcDataByIndex(r4_npcIndex);
    sNPCE8* r14 = &r13->mE8;
    r14->m30_stepTranslation[0] = 0;
    r14->m30_stepTranslation[1] = 0;
    r14->m30_stepTranslation[2] = -r5_zDirection;

    sMatrix4x3 var10;
    initMatrixToIdentity(&var10);
    rotateMatrixShiftedY(r14->mC_rotation[1], &var10);
    rotateMatrixShiftedX(r14->mC_rotation[0], &var10);
    sVec3_FP var4;
    transformVec(r14->m30_stepTranslation, var4, var10);

    r14->m3C_targetPosition[0] = r14->m0_position[0] + var4[0] * r6_distance;
    r14->m3C_targetPosition[1] = r14->m0_position[1] + var4[1] * r6_distance;
    r14->m3C_targetPosition[2] = r14->m0_position[2] + var4[2] * r6_distance;

    r14->m48_targetRotation = r14->mC_rotation;

    r13->mF &= ~0x6;
    r13->mF |= 1;
    r13->mC |= 4;
}

s32 scriptFunction_6057058()
{
    scriptFunction_6057058_sub0();
    setupNPCWalkInZDirection(0, 227, 36);

    return 0;
}

s32 TWN_RUIN_ExecuteNative(sSaturnPtr ptr)
{
    assert(ptr.m_file == gTWN_RUIN);

    switch (ptr.m_offset)
    {
    case 0x6057570: //hasLoadingCompleted
        return hasLoadingCompleted();
    case 0x6057058:
        return scriptFunction_6057058();
        break;
    default:
        assert(0);
    }
    return 0;
}

s32 TWN_RUIN_ExecuteNative(sSaturnPtr ptr, s32 arg0)
{
    assert(ptr.m_file == gTWN_RUIN);

    switch (ptr.m_offset)
    {
    case 0x605C83C:
        return TwnFadeOut(arg0);
    case 0x0605c7c4:
        return TwnFadeIn(arg0);
    default:
        assert(0);
    }
    return 0;
}

s32 TWN_RUIN_ExecuteNative(sSaturnPtr ptr, s32 arg0, s32 arg1)
{
    assert(ptr.m_file == gTWN_RUIN);

    switch (ptr.m_offset)
    {
    case 0x605B320:
        getNpcDataByIndex(arg0)->mE_controlState = arg1;
        return 0;
    case 0x605C55C:
        PDS_Logger[eLogCategories::log_unimlemented].AddLog("Unimplemented TWN_RUIN native function: 0x%08X\n", ptr.m_offset);
        break;
    default:
        assert(0);
    }
    return 0;
}

void updateEdgeControls(sEdgeTask* r4)
{
    s32 r5 = 0;
    s32 r6 = 0;
    if (npcData0.mFC & 2)
    {
        r4->m14C_inputFlags |= 0xC0;
    }
    else
    {
        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m0_inputType == 2)
        {
            //0605BC5E
            r5 = 512 * graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m2_analogX;
            r6 = 512 * graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m3_analogY;
        }
        else
        {
            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & 0x10)
            {
                r6 = 0x10000;
            }
            else if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & 0x20)
            {
                r6 = -0x10000;
            }

            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & 0x40)
            {
                r5 = 0x10000;
            }
            else if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & 0x80)
            {
                r5 = -0x10000;
            }
        }
    }

    r4->m150_inputX = r5;
    r4->m154_inputY = r6;

    if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[0][0])
    {
        r4->m14C_inputFlags |= 2;
    }

    if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & graphicEngineStatus.m4514.mD8_buttonConfig[0][1])
    {
        r4->m14C_inputFlags |= 1;
    }
}

void updateEdgePositionSub1(sEdgeTask* r4)
{
    if (!(r4->m14C_inputFlags & 0x40))
    {
        switch (twnMainLogicTask->m0)
        {
        case 0:
            if (r4->m14C_inputFlags & 2)
            {
                r4->mE8.m24_stepRotation[1] = MTH_Mul(0x555555, r4->m150_inputX);
            }
            else
            {
                r4->mE8.m24_stepRotation[1] = MTH_Mul(0x38E38E, r4->m150_inputX);
            }
            break;
        default:
            assert(0);
            break;
        }
    }
    else
    {
        r4->mE8.m24_stepRotation[1] = 0;
    }

    //605BDAE
    if (r4->m14C_inputFlags & 0x80)
    {
        r4->mE8.m30_stepTranslation[0] = 0;
        r4->mE8.m30_stepTranslation[2] = 0;
    }
    else
    {
        s32 r10;
        if (r4->m14C_inputFlags & 0x2)
        {
            r10 = -0x212;
        }
        else
        {
            r10 = -0x109;
        }

        //0605BDCA
        switch (twnMainLogicTask->m0)
        {
        case 0:
            r4->mE8.m30_stepTranslation[2] = MTH_Mul(r4->m154_inputY, r10);
            r4->mE8.m30_stepTranslation[0] = 0;
            break;
        default:
            assert(0);
            break;
        }
    }

    //605BE7E
    r4->m14C_inputFlags &= 1;
    if (r4->m14C_inputFlags)
    {
        assert(0);
    }
}

void updateEdgePositionSub2(sNPCE8* r4)
{
    r4->mC_rotation[1] += r4->m24_stepRotation[1];
}

void updateEdgePositionSub3(sEdgeTask* r4)
{
    if (currentResTask->m8)
    {
        //605BEEA
        assert(0);
    }
    else
    {
        //0605C018
        if (npcData0.mFC & 0x10)
        {
            //605C026
            assert(0);
        }
    }

    //605C174
    fixedPoint r13 = r4->mE8.m24_stepRotation[1];
    if (r13 > 0x1C71C71)
    {
        r13 = 0x1C71C71;
    }
    if (r13 < -0x1C71C71)
    {
        r13 = -0x1C71C71;
    }

    if (r13)
    {
        r4->m20[1] += MTH_Mul(r13 - r4->m20[1], 0xB333);
    }
    else
    {
        r4->m20[1] += MTH_Mul(r13 - r4->m20[1], 0x8000);
    }

    r4->m20[0] = MTH_Mul(r4->m20[0], 0xB333);
}

void updateEdgePosition(sNPC* r4)
{
    sEdgeTask* r12 = static_cast<sEdgeTask*>(r4);
    sNPCE8* r13 = &r12->mE8;
    sMainLogic_74* r14 = &r12->m84;

    updateEdgeControls(r12);
    updateEdgePositionSub1(r12);

    *r4->m84.m30_pPosition += r4->m84.m58_collisionSolveTranslation;
    if (r4->m84.m44 & 4)
    {
        if (r14->m4C[1] < 0xB504)
        {
            r12->mF |= 0x80;
        }
        else
        {
            r12->mF &= ~0x80;
            if (r13->m30_stepTranslation[1] < 0)
            {
                r13->m30_stepTranslation[1] = 0;
            }
        }

        //605B948
        if (r12->mE_controlState != 4)
        {
            r12->mE_controlState = 4;
        }
    }
    else
    {
        //605B954
        if ((r13->m30_stepTranslation[1] < -0x199) || (r13->m30_stepTranslation[1] > 0))
        {
            r12->mF |= 0x80;
            if (r12->m2C_currentAnimation != 4)
            {
                r12->m2C_currentAnimation = 4;
                u8* buffer;
                if (readSaturnU16(r12->m30_animationTable + 0x10) == 0)
                {
                    buffer = dramAllocatorEnd[0].mC_buffer->m0_dramAllocation;
                }
                else
                {
                    buffer = r12->m0_dramAllocation;
                }

                // play falling animation
                playAnimationGeneric(&r12->m34_3dModel, buffer + READ_BE_U32(buffer + readSaturnU16(r12->m30_animationTable + 0x10 + 2)), 5);
            }
        }
    }
    //605B9AA
    static bool gGravity = true;
    ImGui::Begin("Town");
    ImGui::Checkbox("Gravity", &gGravity);
    ImGui::End();

    if (gGravity)
    {
        r13->m30_stepTranslation[1] += -0x56;
    }

    if (r13->m30_stepTranslation[1] < -0x800)
    {
        r13->m30_stepTranslation[1] = -0x800;
    }

    updateEdgePositionSub2(r13);

    sMatrix4x3 var10;
    initMatrixToIdentity(&var10);
    rotateMatrixShiftedY(r13->mC_rotation[1], &var10);
    rotateMatrixShiftedX(r13->mC_rotation[0], &var10);

    if (r14->m44 & 4)
    {
        //0605B9F0
        assert(0);
    }

    //0605BB48
    if (r14->m44 & 4)
    {
        //605BB50
        assert(0);
    }

    //605BBD6
    transformVec(r13->m30_stepTranslation, r13->m18_stepTranslationInWorld, var10);

    r13->m0_position += r13->m18_stepTranslationInWorld;

    updateEdgePositionSub3(r12);

    r12->m14C_inputFlags = 0;
}
