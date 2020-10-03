#include "PDS.h"
#include "kernel/animation.h"
#include "kernel/fileBundle.h"
#include "a3_background_layer.h"
#include "items.h"
#include "audio/soundDriver.h"
#include "audio/systemSounds.h"
#include "field/field_a3/o_fld_a3.h" //TODO: cleanup
#include "field/fieldRadar.h"
#include "field/exitField.h"
#include "field/fieldItemBox.h"

// Above Excavation

void fieldA3_1_startTasks_sub1Task_InitSub0(p_workArea, sLCSTarget*)
{
    getFieldTaskPtr()->mC->m9C_A3_1_exits[0] = 1;
}

void fieldA3_1_startTasks_sub1Task_InitSub1(p_workArea, sLCSTarget*)
{
    getFieldTaskPtr()->mC->m9C_A3_1_exits[1] = 1;
}

void fieldA3_1_startTasks_sub1Task_InitSub2(p_workArea, sLCSTarget*)
{
    getFieldTaskPtr()->mC->m9C_A3_1_exits[2] = 1;
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
            createLCSTarget(&pThis->m0[i], pThis, fieldA3_1_startTasks_sub1Task_InitFunctionTable[i], &fieldA3_1_startTasks_sub1Task_InitPositionTable[i], NULL, 3, 0, eItems::mMinusOne, 0, 0);
            getFieldTaskPtr()->mC->m9C_A3_1_exits[i] = 0;
        }

        pThis->m0[0].m18_diableFlags |= 1;
    }

    static void Update(sfieldA3_1_startTasks_sub1Task* pThis)
    {
        const sVec3_FP& r4_dragonPos = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m8_pos;

        static const sVec2_FP zoneMin = { 0x280000 , -0x1720000 };
        static const sVec2_FP zoneMax = { 0x3E0000 , -0x1400000 };

        // Conana’s Nest check
        if (mainGameState.getBit(0x91 * 8 + 4) && (r4_dragonPos[0] >= zoneMin[0]) && (r4_dragonPos[0] < zoneMax[0]) && (r4_dragonPos[2] >= zoneMin[1]) && (r4_dragonPos[2] < zoneMax[1]))
        {
            pThis->m0[1].m18_diableFlags = 0;
        }
        else
        {
            pThis->m0[1].m18_diableFlags |= 1;
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
    if (!mainGameState.getBit(0xB5 * 8 + 5))
    {
        createSubTaskFromFunction<sFieldA3_1_fieldIntroTask>(workArea, &sFieldA3_1_fieldIntroTask::Update);
    }
}

static const std::array<s16,3> A3_1_exitsVars =
{
    1261,1261,1260
};

static const std::array<s8, 3> A3_1_exitsLocations =
{
    7, 8, 9
};

static const std::array<sSaturnPtr, 3> A3_1_exitsCutscenes =
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
            if (mainGameState.getBit(0xA * 8 + 6))
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
            if (getFieldTaskPtr()->mC->m9C_A3_1_exits[i])
            {
                mainGameState.setBit566(A3_1_exitsVars[i]);

                if (i == 2)
                {
                    startCutscene(loadCutsceneData({ 0x6091CC4, gFLD_A3 }));
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

    if (mainGameState.getBit(0xA2 * 8 + 3))
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

        s_fileBundle* pBundle = pThis->m0_memoryArea.m0_mainMemoryBundle;
        sAnimationData* pAnimation = pBundle->getAnimation(0x324);
        sStaticPoseData* pPose = pBundle->getStaticPose(0x2A0, pAnimation->m2_numBones);
        init3DModelRawData(pThis, &pThis->m9C_3dModel, 0, pBundle, 4, pAnimation, pPose, 0, nullptr);
        stepAnimation(&pThis->m9C_3dModel);

        pThis->m8_translation = readSaturnVec3(arg);
        pThis->m54 = 0x67B4;

        Unimplemented();
    }

    static void Update(fieldA3_1_startTasks_subTask* pThis)
    {
        Unimplemented();
    }

    static void Draw(fieldA3_1_startTasks_subTask* pThis)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->m8_translation);
        rotateCurrentMatrixZYX(&pThis->m14_rotation);
        addBillBoardToDrawList(pThis->m0_memoryArea.m0_mainMemoryBundle->get3DModel(0xA0)); // draw a bird
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

        s_fileBundle* pBundle = pThis->m0_memoryArea.m0_mainMemoryBundle;
        sAnimationData* pAnimation = pBundle->getAnimation(0x324);
        sStaticPoseData* pPose = pBundle->getStaticPose(0x2A0, pAnimation->m2_numBones);

        init3DModelRawData(pThis, &pThis->m78_3dModel, 0, pBundle, 4, pAnimation, pPose, 0, nullptr);
        stepAnimation(&pThis->m78_3dModel);

        pThis->m8_translation = readSaturnVec3(arg);
        Unimplemented();
    }

    static void Update(fieldA3_1_startTasks_subTask2* pThis)
    {
        Unimplemented();
    }

    static void Draw(fieldA3_1_startTasks_subTask2* pThis)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->m8_translation);
        rotateCurrentMatrixZYX(&pThis->m14_rotation);
        addBillBoardToDrawList(pThis->m0_memoryArea.m0_mainMemoryBundle->get3DModel(0x9C));
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

        s_fileBundle* pBundle = pThis->m0_memoryArea.m0_mainMemoryBundle;
        sAnimationData* pAnimation = pBundle->getAnimation(0x324);
        sStaticPoseData* pPose = pBundle->getStaticPose(0x2A0, pAnimation->m2_numBones);

        init3DModelRawData(pThis, &pThis->m3C_3dModel, 0, pBundle, 4, pAnimation, pPose, 0, nullptr);
        stepAnimation(&pThis->m3C_3dModel);

        pThis->m8_translation = readSaturnVec3(arg);
        pThis->m2C = readSaturnVec3(arg + 0xC);
        pThis->m20_rotation = readSaturnVec3(arg + 0xC * 2);
    }

    static void Update(fieldA3_1_startTasks_subTask3* pThis)
    {
        Unimplemented();
    }

    static void Draw(fieldA3_1_startTasks_subTask3* pThis)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->m8_translation);
        rotateCurrentMatrixZYX(&pThis->m20_rotation);
        addBillBoardToDrawList(pThis->m0_memoryArea.m0_mainMemoryBundle->get3DModel(0xA0)); // draw a bird
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
    Unimplemented();
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
    fieldA3_0_createTask0(workArea);

    fieldA3_1_startTasks_sub1(workArea);

    create_fieldA3_backgroundLayer(workArea);

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

    fieldTaskPtr->m8_pSubFieldData->m338_pDragonTask->mF4 = subfieldA3_1Sub0;

    {
        sVec3_FP position = { 0x448000, 0x69000, -0x1932000 };
        sVec3_FP rotation = { 0x0, 0x0, 0x0 };
        setupDragonPosition(&position, &rotation);
    }

    if (fieldTaskPtr->m30_fieldEntryPoint != -1)
    {
        //6054472
        switch (fieldTaskPtr->m32_previousSubField)
        {
        case 8:
            fieldTaskPtr->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript({ 0x60911f8, gFLD_A3 }); // coming back from A3_0
            break;
        case 9:
            startCutscene(loadCutsceneData({ 0x6091be0, gFLD_A3 })); // coming back from nest
            break;
        default:
            // play above excavation intro already?
            if (mainGameState.getBit(0xA * 8 + 6))
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
    initFieldDragonLight();

    createFieldPaletteTask(workArea);

    setupFieldCameraConfig_A3_1();

    adjustVerticalLimits(-0x58000, 0x76000);

    fieldRadar_enableAltitudeGauge();

    getFieldTaskPtr()->m8_pSubFieldData->m344_randomBattleTask->m0 = nullBattle;

    startFieldScript(18, -1);

    //TODO: more stuff here
}

