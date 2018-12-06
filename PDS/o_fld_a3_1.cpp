#include "PDS.h"

// Above Excavation

void fieldA3_1_startTasks_sub1Task_InitSub0()
{
    getFieldTaskPtr()->mC->m9C[0] = 1;
}

void fieldA3_1_startTasks_sub1Task_InitSub1()
{
    getFieldTaskPtr()->mC->m9C[1] = 1;
}

void fieldA3_1_startTasks_sub1Task_InitSub2()
{
    getFieldTaskPtr()->mC->m9C[2] = 1;
}

void(*fieldA3_1_startTasks_sub1Task_InitFunctionTable[])() = {
    fieldA3_1_startTasks_sub1Task_InitSub0,
    fieldA3_1_startTasks_sub1Task_InitSub1,
    fieldA3_1_startTasks_sub1Task_InitSub2
};

// Map exits
struct sfieldA3_1_startTasks_sub1Task : public s_workAreaTemplate<sfieldA3_1_startTasks_sub1Task>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sfieldA3_1_startTasks_sub1Task::Init, &sfieldA3_1_startTasks_sub1Task::Update, NULL, NULL, "afieldA3_1_startTasks_sub1Task" };
        return &taskDefinition;
    }

    void Init(void*) override
    {
        static const sVec3_FP fieldA3_1_startTasks_sub1Task_InitPositionTable[3] = {
            {0x348000, -0x10000, -0x1EE000},
            {0x2EA000, -0x30180, -0x15B53F4},
            {0x428000, -0x10000, -0x1A96000}
        };
        for (int i = 2; i >= 0; i--)
        {
            createLCSTarget(&m0[i], this, fieldA3_1_startTasks_sub1Task_InitFunctionTable[i], &fieldA3_1_startTasks_sub1Task_InitPositionTable[i], NULL, 3, 0, -1, 0, 0);
            getFieldTaskPtr()->mC->m9C[i] = 0;
        }

        m0[0].m18 |= 1;
    }

    void Update() override
    {
        const sVec3_FP& r4_dragonPos = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m8_pos;

        static const sVec2_FP zoneMin = { 0x280000 , -0x1720000 };
        static const sVec2_FP zoneMax = { 0x3E0000 , -0x1400000 };

        // Conana’s Nest check
        if (mainGameState.getBit(0x91, 4) && (r4_dragonPos[0] >= zoneMin[0]) && (r4_dragonPos[0] < zoneMax[0]) && (r4_dragonPos[2] >= zoneMin[1]) && (r4_dragonPos[2] < zoneMax[1]))
        {
            m0[1].m18 = 0;
        }
        else
        {
            m0[1].m18 |= 1;
        }

        for (int i = 2; i >= 0; i--)
        {
            updateLCSTarget(&m0[i]);
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
        static TypedTaskDefinition taskDefinition = { NULL, NULL, NULL, NULL, "sFieldA3_1_fieldIntroTask" };
        return &taskDefinition;
    }

    void Update() override
    {
        if (startFieldScript(14, 1453))
        {
            getTask()->markFinished();
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

void fieldA3_1_startTasks_sub0()
{
    getFieldTaskPtr()->mC->m0 = fixedPoint(0x10000);
}

struct s_itemType0 : public s_workAreaTemplate<s_itemType0>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { NULL, NULL, NULL, NULL, "s_itemType0" };
        return &taskDefinition;
    }

    void Update()
    {
        PDS_unimplemented("s_itemType0::Update");
        //assert(0);
    }

    p_workArea m0;
    s_itemBoxDefinition* m4;
    //size: 8
};

void LCSItemBox_Callback0()
{
    assert(0);
}

void LCSItemBox_Callback1()
{
    assert(0);
}

void LCSItemBox_CallabckSavePoint()
{
    assert(0);
}

void(*LCSItemBox_CallbackTable[3])() = {
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

struct s_itemBoxType1 : public s_workAreaTemplate<s_itemBoxType1>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &s_itemBoxType1::Init, NULL, NULL, NULL, "s_itemBoxType1" };
        return &taskDefinition;
    }

    void Init(void* arg)
    {
        s_itemBoxDefinition* r13 = (s_itemBoxDefinition*)arg;
        getMemoryArea(&m0, 0);

        m3C = r13->m0_pos;
        m48 = r13->mC;
        m54 = r13->m18;
        m60 = r13->m24;
        m7C = FP_Div(0x10000, r13->m30);
        m80 = r13->m34;
        m84 = r13->m38;
        m8B = r13->m41_LCSType;
        m88 = r13->m3C;
        m8A = r13->m40;
        m8C = r13->m42;
        m86 = r13->m43;
        m8D = r13->m44;

        createLCSTarget(&m8_LCSTarget, this, LCSItemBox_CallbackTable[r13->m41_LCSType], &m60, NULL, LCSItemBox_TableFlags[r13->m41_LCSType] | LCSItemBox_Table0[r13->m41_LCSType] | 0x100, r13->m38, r13->m3C, r13->m40, r13->m42);

        switch (r13->m41_LCSType)
        {
        case 0:
        {
            u8* pModel = m0.m0_mainMemory;
            u8* pModelData1 = pModel + READ_BE_U32(pModel + 0x1A0);
            u8* pDefaultPose = pModel + READ_BE_U32(pModel + LCSItemBox_Table2[r13->m42]);

            init3DModelRawData(this, &m98, 0, pModel, LCSItemBox_Table3[r13->m42], pModelData1, pDefaultPose, 0, 0);

            if (m80 > 0)
            {
                s32 bitIndex;
                if (m80 < 1000)
                {
                    bitIndex = m80;
                }
                else
                {
                    bitIndex = m80 - 566;
                }

                if (mainGameState.getBit(bitIndex))
                {
                    assert(0);
                }
            }
            break;
        }
        case 1:
        case 2:
            break;
        default:
            assert(0);
            break;
        }

        m_UpdateMethod = LCSItemBox_UpdateTable[r13->m41_LCSType];
        m_DrawMethod = LCSItemBox_DrawTable[r13->m41_LCSType];
    }

    s8 LCSItemBox_UpdateType0Sub1()
    {
        PDS_unimplemented("LCSItemBox_UpdateType0Sub1");
        return 1;
    }

    void LCSItemBox_UpdateType0Sub0(s32 r5, s32 r6, fixedPoint r7)
    {
        PDS_unimplemented("LCSItemBox_UpdateType0Sub0");
    }

    void LCSItemBox_UpdateType0()
    {
        if (LCSItemBox_UpdateType0Sub1())
        {
            m6C[1] += fixedPoint(0x444444);
            m8_LCSTarget.m18 = 0;
        }
        else
        {
            m8_LCSTarget.m18 |= 2;
        }

        pushCurrentMatrix();
        translateCurrentMatrix(&m3C);
        rotateCurrentMatrixZYX(&m6C);
        scaleCurrentMatrixRow0(m78);
        scaleCurrentMatrixRow1(m78);
        scaleCurrentMatrixRow2(m78);

        transformAndAddVecByCurrentMatrix(&LCSItemBox_Table6[m8B], &m60);

        LCSItemBox_UpdateType0Sub0(0x58, 0x19C, m7C);

        popMatrix();


        if (m8D)
        {
            m8_LCSTarget.m18 |= 1;
        }

        updateLCSTarget(&m8_LCSTarget);
    }

    void LCSItemBox_UpdateType1()
    {
        PDS_unimplemented("LCSItemBox_UpdateType1");
        //assert(0);
    }

    void LCSItemBox_UpdateType2()
    {
        PDS_unimplemented("LCSItemBox_UpdateType2");
        //assert(0);
    }

    const FunctionType LCSItemBox_UpdateTable[3] = {
        &s_itemBoxType1::LCSItemBox_UpdateType0,
        &s_itemBoxType1::LCSItemBox_UpdateType1,
        &s_itemBoxType1::LCSItemBox_UpdateType2
    };

    const s16 LCSItemBox_Table4[7] =
    {
        0x150,
        0x15C,
        0x164,
        0x16C,
        0x174,
        0x17C,
        0x184,
    };

    const s16 LCSItemBox_Table5[7] =
    {
        0xC,
        0x18,
        0x20,
        0x28,
        0x30,
        0x38,
        0x40,
    };

    void LCSItemBox_DrawType0()
    {
        s_visibilityGridWorkArea* pGridTask = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;

        pushCurrentMatrix();
        translateCurrentMatrix(&m3C);
        rotateCurrentMatrixZYX(&m6C);
        scaleCurrentMatrixRow0(m78);
        scaleCurrentMatrixRow1(m78);
        scaleCurrentMatrixRow2(m78);

        u32 depthRangeIndex = gridCellDraw_GetDepthRange(pCurrentMatrix->matrix[11]);

        if (depthRangeIndex <= pGridTask->m1300)
        {
            LCSItemBox_DrawType0Sub0(m0.m0_mainMemory, LCSItemBox_Table5[m8C], LCSItemBox_Table4[m8C]);
        }

        popMatrix();
    }

    void LCSItemBox_DrawType1()
    {
        s_visibilityGridWorkArea* pGridTask = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;

        pushCurrentMatrix();
        translateCurrentMatrix(&m3C);
        rotateCurrentMatrixZYX(&m6C);
        scaleCurrentMatrixRow0(m78);
        scaleCurrentMatrixRow1(m78);
        scaleCurrentMatrixRow2(m78);

        pushCurrentMatrix();

        sVec3_FP translation;
        translation[0] = READ_BE_S32(m0.m0_mainMemory + 0x148);
        translation[1] = READ_BE_S32(m0.m0_mainMemory + 0x148 + 4);
        translation[2] = READ_BE_S32(m0.m0_mainMemory + 0x148 + 8);

        sVec3_FP rotation;
        rotation[0] = READ_BE_S32(m0.m0_mainMemory + 0x148 + 0xC);
        rotation[1] = READ_BE_S32(m0.m0_mainMemory + 0x148 + 0x10);
        rotation[2] = READ_BE_S32(m0.m0_mainMemory + 0x148 + 0x14);

        translateCurrentMatrix(&translation);
        rotateCurrentMatrixZYX(&rotation);

        u32 depthRangeIndex = gridCellDraw_GetDepthRange(pCurrentMatrix->matrix[11]);

        if (depthRangeIndex <= pGridTask->m1300)
        {
            addObjectToDrawList(m0.m0_mainMemory, READ_BE_U32(m0.m0_mainMemory + 0x5C));

            pushCurrentMatrix();

            sVec3_FP translation2;
            translation2[0] = READ_BE_S32(m0.m0_mainMemory + 0x148 + 0x24);
            translation2[1] = READ_BE_S32(m0.m0_mainMemory + 0x148 + 0x24 + 4);
            translation2[2] = READ_BE_S32(m0.m0_mainMemory + 0x148 + 0x24 + 8);

            sVec3_FP rotation2;
            rotation2[0] = READ_BE_S32(m0.m0_mainMemory + 0x148 + 0x24 + 0xC);
            rotation2[1] = READ_BE_S32(m0.m0_mainMemory + 0x148 + 0x24 + 0x10);
            rotation2[2] = READ_BE_S32(m0.m0_mainMemory + 0x148 + 0x24 + 0x14);

            translateCurrentMatrix(&translation2);
            rotateCurrentMatrixShiftedZ(rotation2[2]);
            rotateCurrentMatrixShiftedY(rotation2[1]);
            rotateCurrentMatrixShiftedX(rotation2[0]);

            addObjectToDrawList(m0.m0_mainMemory, READ_BE_U32(m0.m0_mainMemory + 0x60));

            popMatrix();
        }

        popMatrix();
        popMatrix();
    }

    void LCSItemBox_DrawType2()
    {
        s_visibilityGridWorkArea* pGridTask = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;

        pushCurrentMatrix();
        translateCurrentMatrix(&m3C);
        rotateCurrentMatrixZYX(&m6C);
        scaleCurrentMatrixRow0(m78);
        scaleCurrentMatrixRow1(m78);
        scaleCurrentMatrixRow2(m78);

        u32 depthRangeIndex = gridCellDraw_GetDepthRange(pCurrentMatrix->matrix[11]);

        if (depthRangeIndex <= pGridTask->m1300)
        {
            LCSItemBox_DrawType0Sub0(m0.m0_mainMemory, 8, 0x14C);
            mEA_wasRendered = 1;
        }
        else
        {
            mEA_wasRendered = 0;
        }

        popMatrix();
    }

    const FunctionType LCSItemBox_DrawTable[3] = {
        &s_itemBoxType1::LCSItemBox_DrawType0,
        &s_itemBoxType1::LCSItemBox_DrawType1,
        &s_itemBoxType1::LCSItemBox_DrawType2
    };

    s_memoryAreaOutput m0;
    sLCSTarget m8_LCSTarget;
    sVec3_FP m3C;
    sVec3_FP m48;
    sVec3_FP m54;
    sVec3_FP m60;
    sVec3_FP m6C;
    s32 m78;
    fixedPoint m7C;
    fixedPoint m80;
    s16 m84;
    s16 m86;
    s16 m88;
    s8 m8A;
    s8 m8B;
    s8 m8C;
    s8 m8D;
    s_3dModel m98;
    s16 mEA_wasRendered;
    //size: F0
};

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

void fieldA3_1_startTasks(p_workArea workArea)
{
    create_fieldA3_0_task0(workArea);

    fieldA3_1_startTasks_sub1(workArea);

    //        create_fieldA3_0_task3(workArea);

    create_fieldA3_1_fieldIntroTask(workArea);

    PDS_unimplemented("fieldA3_1_startTasks");

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
        if (getFieldTaskPtr()->mC->m130 != 1)
            return;
        subfieldA3_1Sub0Sub0();
        r4->m108++;
    case 1:
        subfieldA3_1Sub0Sub1();
        if (getFieldTaskPtr()->mC->m130 != 2)
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
                startCutscene(loadCutsceneData({ 0x6091688, gFLD_A3 }));
            }
            else
            {
                startCutscene(loadCutsceneData({ 0x60915A4, gFLD_A3 }));
            }
            break;
        }
    }
    //6054526
    PDS_unimplemented("subfieldA3_1");

    createFieldPaletteTask(workArea);

    setupFieldCameraConfig_A3_1();

    adjustVerticalLimits(-0x58000, 0x76000);

    subfieldA3_1_Sub0();

    getFieldTaskPtr()->m8_pSubFieldData->m344_randomBattleTask->m0 = nullBattle;

    startFieldScript(18, -1);

    //TODO: more stuff here
}
