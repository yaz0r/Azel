#include "PDS.h"

// Above Excavation

void fieldA3_1_startTasks_sub1Task_InitSub0(p_workArea, sLCSTarget*)
{
    getFieldTaskPtr()->mC->m9C[0] = 1;
}

void fieldA3_1_startTasks_sub1Task_InitSub1(p_workArea, sLCSTarget*)
{
    getFieldTaskPtr()->mC->m9C[1] = 1;
}

void fieldA3_1_startTasks_sub1Task_InitSub2(p_workArea, sLCSTarget*)
{
    getFieldTaskPtr()->mC->m9C[2] = 1;
}

void(*fieldA3_1_startTasks_sub1Task_InitFunctionTable[])(p_workArea, sLCSTarget*) = {
    fieldA3_1_startTasks_sub1Task_InitSub0,
    fieldA3_1_startTasks_sub1Task_InitSub1,
    fieldA3_1_startTasks_sub1Task_InitSub2
};

// Map exits
struct sfieldA3_1_startTasks_sub1Task : public s_workAreaTemplate<sfieldA3_1_startTasks_sub1Task>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sfieldA3_1_startTasks_sub1Task::Init, &sfieldA3_1_startTasks_sub1Task::Update, NULL, NULL};
        return &taskDefinition;
    }

    static void Init(sfieldA3_1_startTasks_sub1Task* pThis)
    {
        static const sVec3_FP fieldA3_1_startTasks_sub1Task_InitPositionTable[3] = {
            {0x348000, -0x10000, -0x1EE000},
            {0x2EA000, -0x30180, -0x15B53F4},
            {0x428000, -0x10000, -0x1A96000}
        };
        for (int i = 2; i >= 0; i--)
        {
            createLCSTarget(&pThis->m0[i], pThis, fieldA3_1_startTasks_sub1Task_InitFunctionTable[i], &fieldA3_1_startTasks_sub1Task_InitPositionTable[i], NULL, 3, 0, -1, 0, 0);
            getFieldTaskPtr()->mC->m9C[i] = 0;
        }

        pThis->m0[0].m18 |= 1;
    }

    static void Update(sfieldA3_1_startTasks_sub1Task* pThis)
    {
        const sVec3_FP& r4_dragonPos = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m8_pos;

        static const sVec2_FP zoneMin = { 0x280000 , -0x1720000 };
        static const sVec2_FP zoneMax = { 0x3E0000 , -0x1400000 };

        // Conana’s Nest check
        if (mainGameState.getBit(0x91, 4) && (r4_dragonPos[0] >= zoneMin[0]) && (r4_dragonPos[0] < zoneMax[0]) && (r4_dragonPos[2] >= zoneMin[1]) && (r4_dragonPos[2] < zoneMax[1]))
        {
            pThis->m0[1].m18 = 0;
        }
        else
        {
            pThis->m0[1].m18 |= 1;
        }

        for (int i = 2; i >= 0; i--)
        {
            updateLCSTarget(&pThis->m0[i]);
        }
    }

    std::array<sLCSTarget, 3> m0; // stride is 0x34
    // size 9C
};

void fieldA3_1_startTasks_sub1(p_workArea workArea)
{
    createSubTask<sfieldA3_1_startTasks_sub1Task>(workArea);
}

struct sFieldA3_1_fieldIntroTask : public s_workAreaTemplate<sFieldA3_1_fieldIntroTask>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { NULL, NULL, NULL, NULL};
        return &taskDefinition;
    }

    static void Update(sFieldA3_1_fieldIntroTask* pThis)
    {
        if (startFieldScript(14, 1453))
        {
            pThis->getTask()->markFinished();
        }
    }
};

void create_fieldA3_1_fieldIntroTask(p_workArea workArea)
{
    if (!mainGameState.getBit(0xB5, 5))
    {
        createSubTaskFromFunction<sFieldA3_1_fieldIntroTask>(workArea, &sFieldA3_1_fieldIntroTask::Update);
    }
}

static std::array<s16,3> A3_1_exitsVars =
{
    1261,1261,1260
};

static std::array<s8, 3> A3_1_exitsLocations =
{
    7, 8, 9
};

static std::array<sSaturnPtr, 3> A3_1_exitsCutscenes =
{
    {
        {0, gFLD_A3},
        {0x6091190, gFLD_A3},
        {0x60911C4, gFLD_A3},
    }
};

s32 fieldA3_1_checkExitsTaskUpdate2Sub0()
{
    if (getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m48_cutsceneTask == nullptr)
    {
        return true;
    }
    return false;
}

// TODO: move to kernel
void fieldA3_1_checkExitsTaskUpdate2Sub1(s32 r4)
{
    if (getFieldTaskPtr()->m35 && r4)
    {
        setNextGameStatus(r4);
    }
    else
    {
        dispatchTutorialMultiChoiceSub2();
        getFieldTaskPtr()->m3D = r4;
    }
}

struct sfieldA3_1_checkExitsTask : public s_workAreaTemplate<sfieldA3_1_checkExitsTask>
{
    static void Update2(sfieldA3_1_checkExitsTask* pThis)
    {
        if (fieldA3_1_checkExitsTaskUpdate2Sub0())
        {
            if (mainGameState.getBit(0xA, 6))
            {
                dispatchTutorialMultiChoiceSub2();
            }
            else
            {
                fieldA3_1_checkExitsTaskUpdate2Sub1(6);
            }
        }
    }
    static void Update(sfieldA3_1_checkExitsTask* pThis)
    {
        for (int i = 0; i < 3; i++)
        {
            if (getFieldTaskPtr()->mC->m9C[i])
            {
                if (A3_1_exitsVars[i] >= 1000)
                {
                    mainGameState.setBit(A3_1_exitsVars[i] - 566);
                }
                else
                {
                    mainGameState.setBit(A3_1_exitsVars[i]);
                }

                if (i == 2)
                {
                    startCutscene(loadCutsceneData({ 0x6091CC4, gFLD_A3 }, 0x3C));
                    pThis->m_UpdateMethod = &sfieldA3_1_checkExitsTask::Update2;
                }
                else
                {
                    startExitFieldCutscene(pThis, readCameraScript(A3_1_exitsCutscenes[i]), A3_1_exitsLocations[i], i, 0x8000);
                    pThis->m_UpdateMethod = nullptr;
                }
            }
        }
    }
};

void create_fieldA3_1_checkExitsTask(p_workArea workArea)
{
    createSubTaskFromFunction<sfieldA3_1_checkExitsTask>(workArea, &sfieldA3_1_checkExitsTask::Update);
}

void fieldA3_1_startTasks_sub0()
{
    getFieldTaskPtr()->mC->m0 = fixedPoint(0x10000);
}

struct s_itemType0 : public s_workAreaTemplate<s_itemType0>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { NULL, NULL, NULL, NULL };
        return &taskDefinition;
    }

    static void Update(s_itemType0*)
    {
        PDS_unimplemented("s_itemType0::Update");
        //assert(0);
    }

    p_workArea m0;
    s_itemBoxDefinition* m4;
    //size: 8
};

void LCSItemBox_CallabckSavePoint(p_workArea, sLCSTarget*)
{
    assert(0);
}

void LCSItemBox_Callback0(p_workArea r4, sLCSTarget*);
void LCSItemBox_Callback1(p_workArea r4, sLCSTarget*);
void LCSItemBox_CallbackSavePoint(p_workArea r4, sLCSTarget*);

void(*LCSItemBox_CallbackTable[3])(p_workArea, sLCSTarget*) = {
     &LCSItemBox_Callback0,
     &LCSItemBox_Callback1,
     &LCSItemBox_CallabckSavePoint,
};


s16 LCSItemBox_Table0[3] = {
    2,
    0,
    0
};

s16 LCSItemBox_TableFlags[3] = {
    0,
    0,
    0x20
};

static s16 LCSItemBox_Table3[] = {
    0x14,
    0x1C,
    0x24,
    0x2C,
    0x2C,
    0x2C,
    0x48
};

static s16 LCSItemBox_Table2[] = {
    0x158,
    0x160,
    0x168,
    0x170,
    0x170,
    0x170,
    0x18C
};

static sVec3_FP LCSItemBox_Table6[] = {
    {0, 0x20000, 0},
    {0, 0x1C000, 0},
    {0, 0, 0}
};

p_workArea createSavePointParticles()
{
    TaskUnimplemented();
    return nullptr;
}

struct s_itemBoxType1 : public s_workAreaTemplateWithArg<s_itemBoxType1, s_itemBoxDefinition*>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &s_itemBoxType1::Init, NULL, NULL, NULL };
        return &taskDefinition;
    }

    static void Init(s_itemBoxType1* pThis, s_itemBoxDefinition* arg)
    {
        s_itemBoxDefinition* r13 = arg;
        getMemoryArea(&pThis->m0, 0);

        pThis->m3C_pos = r13->m0_pos;
        pThis->m48_boundingMin = r13->mC_boundingMin;
        pThis->m54_boundingMax = r13->m18_boundingMax;
        pThis->m6C_rotation = r13->m24_rotation;
        pThis->m78_scale = r13->m30_scale;
        pThis->m7C = FP_Div(0x10000, r13->m30_scale);
        pThis->m80 = r13->m34;
        pThis->m84 = r13->m38;
        pThis->m8B_LCSType = r13->m41_LCSType;
        pThis->m88_receivedItemId = r13->m3C_receivedItemId;
        pThis->m8A_receivedItemQuantity = r13->m40_receivedItemQuantity;
        pThis->m8C = r13->m42;
        pThis->m86 = r13->m43;
        pThis->m8D = r13->m44;

        createLCSTarget(&pThis->m8_LCSTarget, pThis, LCSItemBox_CallbackTable[r13->m41_LCSType], &pThis->m60, NULL, LCSItemBox_TableFlags[r13->m41_LCSType] | LCSItemBox_Table0[r13->m41_LCSType] | 0x100, r13->m38, r13->m3C_receivedItemId, r13->m40_receivedItemQuantity, r13->m42);

        switch (r13->m41_LCSType)
        {
        case 0:
        {
            u8* pModel = pThis->m0.m0_mainMemory;
            u8* pModelData1 = pModel + READ_BE_U32(pModel + 0x1A0);
            u8* pDefaultPose = pModel + READ_BE_U32(pModel + LCSItemBox_Table2[r13->m42]);

            init3DModelRawData(pThis, &pThis->m98, 0, pModel, LCSItemBox_Table3[r13->m42], pModelData1, pDefaultPose, 0, 0);

            if (pThis->m80 > 0)
            {
                s32 bitIndex;
                if (pThis->m80 < 1000)
                {
                    bitIndex = pThis->m80;
                }
                else
                {
                    bitIndex = pThis->m80 - 566;
                }

                if (mainGameState.getBit(bitIndex))
                {
                    assert(0);
                }
            }
            break;
        }
        case 1:
            if (mainGameState.getBit(pThis->m88_receivedItemId + 243))
            {
                pThis->m94 = 0;
                pThis->mEA_wasRendered = 2;
            }
            else
            {
                pThis->m94 = -0x444444;
            }
        case 2:
            pThis->mEC_savePointParticlesTask = createSavePointParticles();
            break;
        default:
            assert(0);
            break;
        }

        pThis->m_UpdateMethod = LCSItemBox_UpdateTable[r13->m41_LCSType];
        pThis->m_DrawMethod = LCSItemBox_DrawTable[r13->m41_LCSType];
    }

    s8 LCSItemBox_shouldSpin()
    {
        if (cameraProperties2.m0_position[0] < m54_boundingMax[0])
            return false;

        if (cameraProperties2.m0_position[0] > m48_boundingMin[0])
            return false;

        if (cameraProperties2.m0_position[2] < m54_boundingMax[2])
            return false;

        if (cameraProperties2.m0_position[2] > m48_boundingMin[2])
            return false;

        return 1;
    }

    void LCSItemBox_UpdateType0Sub0(s32 r5, s32 r6, fixedPoint r7)
    {
        PDS_unimplemented("LCSItemBox_UpdateType0Sub0");
    }

    static void LCSItemBox_UpdateType0(s_itemBoxType1* pThis)
    {
        if (pThis->LCSItemBox_shouldSpin())
        {
            pThis->m6C_rotation[1] += fixedPoint(0x444444);
            pThis->m8_LCSTarget.m18 = 0;
        }
        else
        {
            pThis->m8_LCSTarget.m18 |= 2;
        }

        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->m3C_pos);
        rotateCurrentMatrixZYX(&pThis->m6C_rotation);
        scaleCurrentMatrixRow0(pThis->m78_scale);
        scaleCurrentMatrixRow1(pThis->m78_scale);
        scaleCurrentMatrixRow2(pThis->m78_scale);

        transformAndAddVecByCurrentMatrix(&LCSItemBox_Table6[pThis->m8B_LCSType], &pThis->m60);

        pThis->LCSItemBox_UpdateType0Sub0(0x58, 0x19C, pThis->m7C);

        popMatrix();


        if (pThis->m8D)
        {
            pThis->m8_LCSTarget.m18 |= 1;
        }

        updateLCSTarget(&pThis->m8_LCSTarget);
    }

    static void LCSItemBox_UpdateType1(s_itemBoxType1* pThis)
    {
        switch (pThis->mEA_wasRendered)
        {
        case 0:
            if (pThis->LCSItemBox_shouldSpin())
            {
                pThis->m90 = (pThis->m94 + pThis->m90) & 0xFFFFFFF;
                pThis->m20 = 0;
            }
            else
            {
                pThis->m20 |= 2;
            }
            break;
        case 1:
            if (((pThis->m90 + pThis->m94) & 0xFFFFFFF) > pThis->m90)
            {
                pThis->m90 = 0;
                pThis->mEA_wasRendered++;
            }
            else
            {
                pThis->m90 = ((pThis->m90 + pThis->m94) & 0xFFFFFFF);
            }
        case 2:
        default:
            break;
        }

        {
            pushCurrentMatrix();
            translateCurrentMatrix(&pThis->m3C_pos);
            rotateCurrentMatrixZYX(&pThis->m6C_rotation);
            scaleCurrentMatrixRow0(pThis->m78_scale);
            scaleCurrentMatrixRow1(pThis->m78_scale);
            scaleCurrentMatrixRow2(pThis->m78_scale);

            transformAndAddVecByCurrentMatrix(&LCSItemBox_Table6[pThis->m8B_LCSType], &pThis->m60);

            {
                pushCurrentMatrix();
                translateCurrentMatrix(READ_BE_Vec3(pThis->m0.m0_mainMemory + 0x190));
                rotateCurrentMatrixZYX(READ_BE_Vec3(pThis->m0.m0_mainMemory + 0x190 + 0xC));

                gridCellDraw_normalSub2(pThis->m0.m0_mainMemory, 0x138, pThis->m7C);

                {
                    pushCurrentMatrix();
                    translateCurrentMatrix(READ_BE_Vec3(pThis->m0.m0_mainMemory + 0x190 + 0x24));
                    rotateCurrentMatrixShiftedZ(READ_BE_S32(pThis->m0.m0_mainMemory + 0x190 + 0x24 + 0x14));
                    rotateCurrentMatrixShiftedY(READ_BE_S32(pThis->m0.m0_mainMemory + 0x190 + 0x24 + 0x10) + pThis->m90);
                    rotateCurrentMatrixShiftedX(READ_BE_S32(pThis->m0.m0_mainMemory + 0x190 + 0x24 + 0xC));

                    gridCellDraw_normalSub2(pThis->m0.m0_mainMemory, 0x13C, pThis->m7C);

                    popMatrix();
                }
                popMatrix();
            }
            popMatrix();
        }

        if ((pThis->m88_receivedItemId == 0) || mainGameState.getBit(pThis->m88_receivedItemId + 243))
        {
            pThis->m20 |= 1;
        }

        updateLCSTarget(&pThis->m8_LCSTarget);
    }

    static void LCSItemBox_UpdateType2(s_itemBoxType1* pThis)
    {
        if (pThis->LCSItemBox_shouldSpin())
        {
            pThis->m20 = 0;
        }
        else
        {
            pThis->m20 |= 2;
        }

        {
            pushCurrentMatrix();
            translateCurrentMatrix(&pThis->m3C_pos);
            rotateCurrentMatrixZYX(&pThis->m6C_rotation);
            scaleCurrentMatrixRow0(pThis->m78_scale);
            scaleCurrentMatrixRow1(pThis->m78_scale);
            scaleCurrentMatrixRow2(pThis->m78_scale);

            transformAndAddVecByCurrentMatrix(&LCSItemBox_Table6[pThis->m8B_LCSType], &pThis->m60);
            pThis->LCSItemBox_UpdateType0Sub0(0x54, 0x198, 0x7C);
            popMatrix();
        }
        
        if (pThis->m8D)
        {
            pThis->m20 |= 1;
        }

        updateLCSTarget(&pThis->m8_LCSTarget);
    }

    static constexpr FunctionType LCSItemBox_UpdateTable[3] = {
        &s_itemBoxType1::LCSItemBox_UpdateType0,
        &s_itemBoxType1::LCSItemBox_UpdateType1,
        &s_itemBoxType1::LCSItemBox_UpdateType2
    };

    static constexpr s16 LCSItemBox_Table4[7] =
    {
        0x150,
        0x15C,
        0x164,
        0x16C,
        0x174,
        0x17C,
        0x184,
    };

    static constexpr s16 LCSItemBox_Table5[7] =
    {
        0xC,
        0x18,
        0x20,
        0x28,
        0x30,
        0x38,
        0x40,
    };

    static void LCSItemBox_DrawType0(s_itemBoxType1* pThis)
    {
        s_visibilityGridWorkArea* pGridTask = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;

        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->m3C_pos);
        rotateCurrentMatrixZYX(&pThis->m6C_rotation);
        scaleCurrentMatrixRow0(pThis->m78_scale);
        scaleCurrentMatrixRow1(pThis->m78_scale);
        scaleCurrentMatrixRow2(pThis->m78_scale);

        u32 depthRangeIndex = gridCellDraw_GetDepthRange(pCurrentMatrix->matrix[11]);

        if (depthRangeIndex <= pGridTask->m1300)
        {
            LCSItemBox_DrawType0Sub0(pThis->m0.m0_mainMemory, LCSItemBox_Table5[pThis->m8C], LCSItemBox_Table4[pThis->m8C]);
        }

        popMatrix();
    }

    static void LCSItemBox_DrawType1(s_itemBoxType1* pThis)
    {
        s_visibilityGridWorkArea* pGridTask = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;

        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->m3C_pos);
        rotateCurrentMatrixZYX(&pThis->m6C_rotation);
        scaleCurrentMatrixRow0(pThis->m78_scale);
        scaleCurrentMatrixRow1(pThis->m78_scale);
        scaleCurrentMatrixRow2(pThis->m78_scale);

        pushCurrentMatrix();

        sVec3_FP translation = READ_BE_Vec3(pThis->m0.m0_mainMemory + 0x148);
        sVec3_FP rotation = READ_BE_Vec3(pThis->m0.m0_mainMemory + 0x148 + 0xC);

        translateCurrentMatrix(&translation);
        rotateCurrentMatrixZYX(&rotation);

        u32 depthRangeIndex = gridCellDraw_GetDepthRange(pCurrentMatrix->matrix[11]);

        if (depthRangeIndex <= pGridTask->m1300)
        {
            addObjectToDrawList(pThis->m0.m0_mainMemory, READ_BE_U32(pThis->m0.m0_mainMemory + 0x5C));

            pushCurrentMatrix();

            sVec3_FP translation2 = READ_BE_Vec3(pThis->m0.m0_mainMemory + 0x148 + 0x24);
            sVec3_FP rotation2 = READ_BE_Vec3(pThis->m0.m0_mainMemory + 0x148 + 0x24 + 0xC);

            translateCurrentMatrix(&translation2);
            rotateCurrentMatrixShiftedZ(rotation2[2]);
            rotateCurrentMatrixShiftedY(rotation2[1] + pThis->m90);
            rotateCurrentMatrixShiftedX(rotation2[0]);

            addObjectToDrawList(pThis->m0.m0_mainMemory, READ_BE_U32(pThis->m0.m0_mainMemory + 0x60));

            popMatrix();
        }

        popMatrix();
        popMatrix();
    }

    static void LCSItemBox_DrawType2(s_itemBoxType1* pThis)
    {
        s_visibilityGridWorkArea* pGridTask = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;

        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->m3C_pos);
        rotateCurrentMatrixZYX(&pThis->m6C_rotation);
        scaleCurrentMatrixRow0(pThis->m78_scale);
        scaleCurrentMatrixRow1(pThis->m78_scale);
        scaleCurrentMatrixRow2(pThis->m78_scale);

        u32 depthRangeIndex = gridCellDraw_GetDepthRange(pCurrentMatrix->matrix[11]);

        if (depthRangeIndex <= pGridTask->m1300)
        {
            LCSItemBox_DrawType0Sub0(pThis->m0.m0_mainMemory, 8, 0x14C);
            pThis->mEA_wasRendered = 1;
        }
        else
        {
            pThis->mEA_wasRendered = 0;
        }

        popMatrix();
    }

    static constexpr FunctionType LCSItemBox_DrawTable[3] = {
        &s_itemBoxType1::LCSItemBox_DrawType0,
        &s_itemBoxType1::LCSItemBox_DrawType1,
        &s_itemBoxType1::LCSItemBox_DrawType2
    };

    static void LCSItemBox_OpenedBoxUpdate(s_itemBoxType1* pThis)
    {
        switch (pThis->mEA_wasRendered)
        {
        case 0:
            pThis->m20 |= 1;
            pThis->mE8 = 0x15;
            pThis->mEA_wasRendered++;
            // fall
        case 1:
            stepAnimation(&pThis->m98);
            pThis->mEA_wasRendered++;
            pThis->mE8--;
            if (pThis->mE8 <= 0)
            {
                pThis->mEA_wasRendered = 3;
            }
            return;
        case 2:
            pThis->mEA_wasRendered--;
            break;
        case 3:
            break;
        default:
            assert(0);
        }
    }

    static void LCSItemBox_OpenedBoxDraw(s_itemBoxType1* pThis)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->m3C_pos);
        rotateCurrentMatrixZYX(&pThis->m6C_rotation);
        scaleCurrentMatrixRow0(pThis->m78_scale);
        scaleCurrentMatrixRow1(pThis->m78_scale);
        scaleCurrentMatrixRow2(pThis->m78_scale);
        pThis->m98.m18_drawFunction(&pThis->m98);
        popMatrix();
    }

    void LCSItemBox_Callback0Sub0()
    {
        PDS_unimplemented("LCSItemBox_Callback0Sub0");
    }

    s_memoryAreaOutput m0;
    sLCSTarget m8_LCSTarget;
    s8 m20;
    s8 m21;
    sVec3_FP m3C_pos;
    sVec3_FP m48_boundingMin;
    sVec3_FP m54_boundingMax;
    sVec3_FP m60;
    sVec3_FP m6C_rotation;
    fixedPoint m78_scale;
    fixedPoint m7C;
    fixedPoint m80;
    s16 m84;
    s16 m86;
    s16 m88_receivedItemId;
    s8 m8A_receivedItemQuantity;
    s8 m8B_LCSType;
    s8 m8C;
    s8 m8D;
    s32 m90;
    s32 m94;
    s_3dModel m98;
    s16 mE8;
    s16 mEA_wasRendered;
    p_workArea mEC_savePointParticlesTask;
    //size: F0
};

void LCSItemBox_Callback1(p_workArea r4, sLCSTarget*)
{
    s_itemBoxType1* pThis = (s_itemBoxType1*)r4;
    pThis->mEA_wasRendered++;
}

void LCSItemBox_Callback0(p_workArea r4, sLCSTarget*)
{
    s_itemBoxType1* pThis = (s_itemBoxType1*)r4;
    if (pThis->m21 & 0x20)
        return;

    s32 bitIndex;
    if (pThis->m80 < 1000)
    {
        bitIndex = pThis->m80;
    }
    else
    {
        bitIndex = pThis->m80 - 566;
    }
    mainGameState.setBit(bitIndex);

    pThis->LCSItemBox_Callback0Sub0();
    playSoundEffect(0x17);

    pThis->m_UpdateMethod = &s_itemBoxType1::LCSItemBox_OpenedBoxUpdate;
    pThis->m_DrawMethod = &s_itemBoxType1::LCSItemBox_OpenedBoxDraw;
}

p_workArea findParentGridCellTaskForItem(s_itemBoxDefinition* r14)
{
    s_visibilityGridWorkArea* r4 = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;
    s_DataTable3* r5 = r4->m30;

    // is object in the grid?
    if (r5 &&
        r4->m3C_cellRenderingTasks &&
        (r14->m0_pos[0] >= 0) &&
        (r14->m0_pos[2] <= 0) &&
        (r14->m0_pos[0] < (r4->m20_cellDimensions[0] * r5->m10_gridSize[0])) &&
        (r14->m0_pos[2] >= -(r4->m20_cellDimensions[1] * r5->m10_gridSize[1])))
    {
        s32 gridX = performDivision(r4->m20_cellDimensions[0], r14->m0_pos[0]);
        s32 gridY = performDivision(r4->m20_cellDimensions[1], -r14->m0_pos[2]);

        return r4->m3C_cellRenderingTasks[(r5->m10_gridSize[0] * gridY) + gridX];
    }
    else
    {
        return getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1->m38;
    }
}

p_workArea fieldA3_1_createItemBoxes_Sub1(s_itemBoxDefinition* r4)
{
    return createSubTaskWithArg<s_itemBoxType1>(findParentGridCellTaskForItem(r4), r4);
}

s_itemType0* fieldA3_1_createItemBoxes_Sub0(p_workArea workArea, s_itemBoxDefinition* r5)
{
    s_itemType0* r0 = createSubTaskFromFunction<s_itemType0>(workArea, &s_itemType0::Update);
    if (r0)
    {
        r0->m0 = fieldA3_1_createItemBoxes_Sub1(r5);
        r0->m4 = r5;
    }

    return r0;
}

void fieldA3_1_createItemBoxes(p_workArea workArea)
{
    fieldA3_1_createItemBoxes_Sub0(workArea, readItemBoxDefinition({ 0x6092034, gFLD_A3 }));
    fieldA3_1_createItemBoxes_Sub0(workArea, readItemBoxDefinition({ 0x609207C, gFLD_A3 }));

    fieldA3_1_createItemBoxes_Sub1(readItemBoxDefinition({ 0x60920C4, gFLD_A3 }));
    fieldA3_1_createItemBoxes_Sub1(readItemBoxDefinition({ 0x6092154, gFLD_A3 }));
    fieldA3_1_createItemBoxes_Sub1(readItemBoxDefinition({ 0x609210C, gFLD_A3 }));
}

void fieldA3_1_task4_init(s_fieldA3_1_task4* pThis)
{
    sSaturnPtr r6 = gFLD_A3->getSaturnPtr(0x06081E04);

    if (mainGameState.getBit(0xA2, 3))
    {
        pThis->m0 = readSaturnS32(r6);
        pThis->m4 = readSaturnS32(r6 + 4);
    }
    else
    {
        r6 += 0x18;
        pThis->m0 = readSaturnS32(r6);
        pThis->m4 = readSaturnS32(r6 + 4);
    }

    pThis->m8 = readSaturnS32(r6 + 8);
}

void fieldA3_1_task4_update(s_fieldA3_1_task4* pThis)
{
    return; // but why?
}

static const s_fieldA3_1_task4::TypedTaskDefinition fieldA3_1_task4_definition = {
    fieldA3_1_task4_init,
    fieldA3_1_task4_update,
    nullptr,
    nullptr
};

void create_fieldA3_1_task4(p_workArea workArea)
{
    createSubTask<s_fieldA3_1_task4>(workArea, &fieldA3_1_task4_definition);
}

// This is the LCS target used to activate the birds to access Conana's nest
struct fieldA3_1_startTasks_subTask : public s_workAreaTemplateWithArg<fieldA3_1_startTasks_subTask, sSaturnPtr>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &fieldA3_1_startTasks_subTask::Init, &fieldA3_1_startTasks_subTask::Update, &fieldA3_1_startTasks_subTask::Draw, NULL };
        return &taskDefinition;
    }

    static void Init(fieldA3_1_startTasks_subTask* pThis, sSaturnPtr arg)
    {
        getMemoryArea(&pThis->m0_memoryArea, 3);

        u8* p3dModelRawData = pThis->m0_memoryArea.m0_mainMemory + READ_BE_U32(pThis->m0_memoryArea.m0_mainMemory);
        init3DModelRawData(pThis, &pThis->m9C_3dModel, 0, p3dModelRawData, 4, pThis->m0_memoryArea.m0_mainMemory + READ_BE_U32(pThis->m0_memoryArea.m0_mainMemory + 0x324), pThis->m0_memoryArea.m0_mainMemory + READ_BE_U32(pThis->m0_memoryArea.m0_mainMemory + 0x2A0), 0, nullptr);
        stepAnimation(&pThis->m9C_3dModel);

        pThis->m8_translation = readSaturnVec3(arg);
        pThis->m54 = 0x67B4;

        TaskUnimplemented();
    }

    static void Update(fieldA3_1_startTasks_subTask* pThis)
    {
        TaskUnimplemented();
    }

    static void Draw(fieldA3_1_startTasks_subTask* pThis)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->m8_translation);
        rotateCurrentMatrixZYX(&pThis->m14_rotation);
        addBillBoardToDrawList(pThis->m0_memoryArea.m0_mainMemory, READ_BE_U32(pThis->m0_memoryArea.m0_mainMemory + 0xA0)); // draw a bird
        popMatrix();
    }

    s_memoryAreaOutput m0_memoryArea;
    sVec3_FP m8_translation;
    sVec3_FP m14_rotation;
    s32 m54;
    s_3dModel m9C_3dModel;
    // size F0
};

struct fieldA3_1_startTasks_subTask2 : public s_workAreaTemplateWithArg<fieldA3_1_startTasks_subTask2, sSaturnPtr>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &fieldA3_1_startTasks_subTask2::Init, &fieldA3_1_startTasks_subTask2::Update, &fieldA3_1_startTasks_subTask2::Draw, NULL };
        return &taskDefinition;
    }

    static void Init(fieldA3_1_startTasks_subTask2* pThis, sSaturnPtr arg)
    {
        getMemoryArea(&pThis->m0_memoryArea, 3);

        u8* p3dModelRawData = pThis->m0_memoryArea.m0_mainMemory + READ_BE_U32(pThis->m0_memoryArea.m0_mainMemory);
        init3DModelRawData(pThis, &pThis->m78_3dModel, 0, p3dModelRawData, 4, pThis->m0_memoryArea.m0_mainMemory + READ_BE_U32(pThis->m0_memoryArea.m0_mainMemory + 0x324), pThis->m0_memoryArea.m0_mainMemory + READ_BE_U32(pThis->m0_memoryArea.m0_mainMemory + 0x2A0), 0, nullptr);
        stepAnimation(&pThis->m78_3dModel);

        pThis->m8_translation = readSaturnVec3(arg);
        TaskUnimplemented();
    }

    static void Update(fieldA3_1_startTasks_subTask2* pThis)
    {
        TaskUnimplemented();
    }

    static void Draw(fieldA3_1_startTasks_subTask2* pThis)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->m8_translation);
        rotateCurrentMatrixZYX(&pThis->m14_rotation);
        addBillBoardToDrawList(pThis->m0_memoryArea.m0_mainMemory, READ_BE_U32(pThis->m0_memoryArea.m0_mainMemory + 0x9C));
        popMatrix();
    }

    s_memoryAreaOutput m0_memoryArea;
    sVec3_FP m8_translation;
    sVec3_FP m14_rotation;
    s_3dModel m78_3dModel;
    // size CC
};

struct fieldA3_1_startTasks_subTask3 : public s_workAreaTemplateWithArg<fieldA3_1_startTasks_subTask3, sSaturnPtr>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &fieldA3_1_startTasks_subTask3::Init, &fieldA3_1_startTasks_subTask3::Update, &fieldA3_1_startTasks_subTask3::Draw, NULL };
        return &taskDefinition;
    }

    static void Init(fieldA3_1_startTasks_subTask3* pThis, sSaturnPtr arg)
    {
        getMemoryArea(&pThis->m0_memoryArea, 3);

        u8* p3dModelRawData = pThis->m0_memoryArea.m0_mainMemory + READ_BE_U32(pThis->m0_memoryArea.m0_mainMemory);
        init3DModelRawData(pThis, &pThis->m3C_3dModel, 0, p3dModelRawData, 4, pThis->m0_memoryArea.m0_mainMemory + READ_BE_U32(pThis->m0_memoryArea.m0_mainMemory + 0x324), pThis->m0_memoryArea.m0_mainMemory + READ_BE_U32(pThis->m0_memoryArea.m0_mainMemory + 0x2A0), 0, nullptr);
        stepAnimation(&pThis->m3C_3dModel);

        pThis->m8_translation = readSaturnVec3(arg);
        pThis->m2C = readSaturnVec3(arg + 0xC);
        pThis->m20_rotation = readSaturnVec3(arg + 0xC * 2);
    }

    static void Update(fieldA3_1_startTasks_subTask3* pThis)
    {
        TaskUnimplemented();
    }

    static void Draw(fieldA3_1_startTasks_subTask3* pThis)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->m8_translation);
        rotateCurrentMatrixZYX(&pThis->m20_rotation);
        addBillBoardToDrawList(pThis->m0_memoryArea.m0_mainMemory, READ_BE_U32(pThis->m0_memoryArea.m0_mainMemory + 0xA0)); // draw a bird
        popMatrix();
    }

    s_memoryAreaOutput m0_memoryArea;
    sVec3_FP m8_translation;
    sVec3_FP m20_rotation;
    sVec3_FP m2C;
    s_3dModel m3C_3dModel;
    // size 8C
};

struct sfieldA3_1_startTasks_sub3_func_task : public s_workAreaTemplate<sfieldA3_1_startTasks_sub3_func_task>
{
    fieldA3_1_startTasks_subTask* m0;
    // size 4
};

void fieldA3_1_startTasks_sub3_func(sfieldA3_1_startTasks_sub3_func_task* pThis)
{
    /*
    getFieldTaskPtr()->mC->mC0[7][0] = pThis->m0;
    getFieldTaskPtr()->mC->mC0[7][1] = pThis->m0;
    getFieldTaskPtr()->mC->mC0[7][2] = pThis->m0;

    getFieldTaskPtr()->mC->mC0[7][0] = pThis->m0;
    getFieldTaskPtr()->mC->mC0[7][1] = pThis->m0;
    getFieldTaskPtr()->mC->mC0[7][2] = pThis->m0;
    */
    TaskUnimplemented();
}

void fieldA3_1_startTasks_sub3(p_workArea workArea)
{
    sfieldA3_1_startTasks_sub3_func_task* r14 = createSubTaskFromFunction<sfieldA3_1_startTasks_sub3_func_task>(workArea, &fieldA3_1_startTasks_sub3_func);

    r14->m0 = createSubTaskWithArg<fieldA3_1_startTasks_subTask>(r14, gFLD_A3->getSaturnPtr(0x6090778));

    for (int i = 0; i < 7; i++)
    {
        createSubTaskWithArg<fieldA3_1_startTasks_subTask2>(r14, gFLD_A3->getSaturnPtr(0x60906D0 + i * 0xC));
    }

    for (int i = 0; i < 14; i++)
    {
        createSubTaskWithArg<fieldA3_1_startTasks_subTask3>(r14, gFLD_A3->getSaturnPtr(0x6090B74 + i * 0xC));
    }

    getFieldTaskPtr()->mC->m130_conanaNestCutsceneTrigger = 0; 
}

void fieldA3_1_startTasks(p_workArea workArea)
{
    create_fieldA3_0_task0(workArea);

    fieldA3_1_startTasks_sub1(workArea);

    //        create_fieldA3_0_task3(workArea);

    create_fieldA3_1_fieldIntroTask(workArea);
    create_fieldA3_1_checkExitsTask(workArea);

    create_fieldA3_1_task4(workArea);

    fieldA3_1_startTasks_sub3(workArea);

    fieldA3_1_createItemBoxes(workArea);

    fieldA3_1_startTasks_sub0();
}

void subfieldA3_1Sub0Sub1()
{
    PDS_unimplemented("subfieldA3_1Sub0Sub1");
}

void subfieldA3_1Sub0(s_dragonTaskWorkArea* r4)
{
    switch (r4->m108)
    {
    case 0:
        if (getFieldTaskPtr()->mC->m130_conanaNestCutsceneTrigger != 1)
            return;
        subfieldA3_1Sub0Sub0();
        r4->m108++;
    case 1:
        subfieldA3_1Sub0Sub1();
        if (getFieldTaskPtr()->mC->m130_conanaNestCutsceneTrigger != 2)
            return;
        subfieldA3_1Sub0Sub2(4, 0x8000);
        r4->m108++;
    case 2:
        return;
    default:
        assert(0);
        break;
    }
}

void setupFieldCameraConfig_A3_1()
{
    setupFieldCameraConfigs(readCameraConfig({ 0x6081F9C, gFLD_A3 }), 1);
}

void subfieldA3_1(p_workArea workArea)
{
    playPCM(workArea, 100);
    playPCM(workArea, 101);

    s_DataTable3* pDataTable3 = readDataTable3({ 0x608838C, gFLD_A3 });
    std::vector<std::vector<sCameraVisibility>>* pVisibility = readCameraVisbility({ 0x608F628, gFLD_A3 }, pDataTable3);
    s_DataTable2* pDataTable2 = readDataTable2({ 0x60866D0, gFLD_A3 });
    setupField(pDataTable3, pDataTable2, fieldA3_1_startTasks, pVisibility);

    getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->mF4 = subfieldA3_1Sub0;

    {
        sVec3_FP position = { 0x0, 0x0, 0x0 };
        sVec3_FP rotation = { 0x154000, 0x0, -0x18E000 };
        setupDragonPosition(&position, &rotation);
    }

    if (getFieldTaskPtr()->m30 != -1)
    {
        //6054472
        switch (getFieldTaskPtr()->m32)
        {
        case 8:
            assert(0);
        case 9:
            assert(0);
        default:
            if (mainGameState.getBit(0xA, 6))
            {
                startCutscene(loadCutsceneData({ 0x6091688, gFLD_A3 }, 0x3C));
            }
            else
            {
                startCutscene(loadCutsceneData({ 0x60915A4, gFLD_A3 }, 0x3C));
            }
            break;
        }
    }
    //6054526
    initFieldDragonLight();

    createFieldPaletteTask(workArea);

    setupFieldCameraConfig_A3_1();

    adjustVerticalLimits(-0x58000, 0x76000);

    subfieldA3_1_Sub0();

    getFieldTaskPtr()->m8_pSubFieldData->m344_randomBattleTask->m0 = nullBattle;

    startFieldScript(18, -1);

    //TODO: more stuff here
}

