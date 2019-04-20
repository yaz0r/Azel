#include "PDS.h"
#include "a3_0_task4.h"

void subfieldA3_0Sub0(s_dragonTaskWorkArea* r4)
{
    PDS_unimplemented("subfieldA3_0Sub0");
}

p_workArea fieldA3_0_createTask0(p_workArea workArea)
{
    s_fieldSpecificData_A3* newWorkArea = createSubTaskFromFunction<s_fieldSpecificData_A3>(workArea, NULL);
    getFieldTaskPtr()->mC = newWorkArea;
    return newWorkArea;
}

p_workArea fieldA3_0_createTask1(p_workArea workArea, s32 r5, s32 r6)
{
    PDS_unimplemented("create_fieldA3_0_task1");
    return workArea;
}

struct s_fieldA3_0_tutorialTask : public s_workAreaTemplate<s_fieldA3_0_tutorialTask>
{
    // size 0x2C
};

void fieldA3_0_tutorialTask_update(s_fieldA3_0_tutorialTask* workArea)
{
    if (startFieldScript(21, -1))
    {
        workArea->getTask()->markFinished();
    }
}

void create_fieldA3_0_tutorialTask(p_workArea workArea)
{
    if ((getFieldTaskPtr()->m2C_currentFieldIndex != 2) || mainGameState.getBit(0xA2, 2))
    {
        createSubTaskFromFunction<s_fieldA3_0_tutorialTask>(workArea, fieldA3_0_tutorialTask_update);
    }
}

void fieldA3_0_createItemBoxes(p_workArea workArea)
{
    fieldA3_1_createItemBoxes_Sub0(workArea, readItemBoxDefinition({ 0x6091DF4, gFLD_A3 }));
    fieldA3_1_createItemBoxes_Sub0(workArea, readItemBoxDefinition({ 0x6091E3C, gFLD_A3 }));
    fieldA3_1_createItemBoxes_Sub1(readItemBoxDefinition({ 0x6091E84, gFLD_A3 }));
    fieldA3_1_createItemBoxes_Sub0(workArea, readItemBoxDefinition({ 0x6091ECC, gFLD_A3 }));
    fieldA3_1_createItemBoxes_Sub0(workArea, readItemBoxDefinition({ 0x6091F14, gFLD_A3 }));
    fieldA3_1_createItemBoxes_Sub1(readItemBoxDefinition({ 0x6091F5C, gFLD_A3 }));
    fieldA3_1_createItemBoxes_Sub1(readItemBoxDefinition({ 0x6091FA4, gFLD_A3 }));
    fieldA3_1_createItemBoxes_Sub1(readItemBoxDefinition({ 0x6091FEC, gFLD_A3 }));
}

const std::array<s16, 8> A3_0_exitsVars =
{
    1262, 1263, 1264, 1265, 1266, 1267, 1268, 1269
};

static const std::array<s8, 8> A3_0_exitsLocations =
{
    4, 4, 5, 5, 6, 6, 10, 8
};

struct sfieldA3_0_checkExitsTask : public s_workAreaTemplate<sfieldA3_0_checkExitsTask>
{
    static void Update(sfieldA3_0_checkExitsTask* pThis)
    {
        for (int i = 0; i < 8; i++)
        {
            if (getFieldTaskPtr()->mC->m94_A3_0_exits[i])
            {
                mainGameState.setBit566(A3_0_exitsVars[i]);

                // Hack: this is in the function to be initialized after gFLD_A3
                static const std::array<sSaturnPtr, 8> A3_0_exitsCutscenes =
                {
                    {
                        {0x6090EB8, gFLD_A3},
                        {0x6090F20, gFLD_A3},
                        {0x6090F88, gFLD_A3},
                        {0x0, gFLD_A3},
                        {0x0, gFLD_A3},
                        {0x60910C0, gFLD_A3},
                        {0x0, gFLD_A3},
                        {0x0, gFLD_A3},
                    }
                };

                if (A3_0_exitsCutscenes[i].m_offset)
                {
                    startExitFieldCutscene(pThis, readCameraScript(A3_0_exitsCutscenes[i]), A3_0_exitsLocations[i], i, 0x8000);
                }
                else
                {
                    //6056A70
                    static const std::array<sSaturnPtr, 8> A3_0_exitsCutscenes2 =
                    {
                        {
                            {0, gFLD_A3},
                            {0, gFLD_A3},
                            {0, gFLD_A3},
                            {0x609176C, gFLD_A3},
                            {0x6091850, gFLD_A3},
                            {0, gFLD_A3},
                            {0x6091A18, gFLD_A3},
                            {0x6091AFC, gFLD_A3},
                        }
                    };

                    startExitFieldCutscene2(pThis, loadCutsceneData(A3_0_exitsCutscenes2[i]), A3_0_exitsLocations[i], i, 0x8000);
                }

                //6056A86
                pThis->m_UpdateMethod = NULL;
            }
        }
    }
};

void fieldA3_0_createCheckExitTask(p_workArea workArea)
{
    createSubTaskFromFunction<sfieldA3_0_checkExitsTask>(workArea, &sfieldA3_0_checkExitsTask::Update);
}

void fieldA3_0_exitFunction_0(p_workArea, sLCSTarget*)
{
    getFieldTaskPtr()->mC->m94_A3_0_exits[0] = 1;
}

void fieldA3_0_exitFunction_1(p_workArea, sLCSTarget*)
{
    getFieldTaskPtr()->mC->m94_A3_0_exits[1] = 1;
}

void fieldA3_0_exitFunction_2(p_workArea, sLCSTarget*)
{
    getFieldTaskPtr()->mC->m94_A3_0_exits[2] = 1;
}

void fieldA3_0_exitFunction_3(p_workArea, sLCSTarget*)
{
    getFieldTaskPtr()->mC->m94_A3_0_exits[3] = 1;
}

void fieldA3_0_exitFunction_4(p_workArea, sLCSTarget*)
{
    getFieldTaskPtr()->mC->m94_A3_0_exits[4] = 1;
}

void fieldA3_0_exitFunction_5(p_workArea, sLCSTarget*)
{
    getFieldTaskPtr()->mC->m94_A3_0_exits[5] = 1;
}

void fieldA3_0_exitFunction_6(p_workArea, sLCSTarget*)
{
    getFieldTaskPtr()->mC->m94_A3_0_exits[6] = 1;
}

void fieldA3_0_exitFunction_7(p_workArea, sLCSTarget*)
{
    getFieldTaskPtr()->mC->m94_A3_0_exits[7] = 1;
}

void(*fieldA3_0_exitFunctionTable[])(p_workArea, sLCSTarget*) = {
    fieldA3_0_exitFunction_0,
    fieldA3_0_exitFunction_1,
    fieldA3_0_exitFunction_2,
    fieldA3_0_exitFunction_3,
    fieldA3_0_exitFunction_4,
    fieldA3_0_exitFunction_5,
    fieldA3_0_exitFunction_6,
    fieldA3_0_exitFunction_7,
};

struct sfieldA3_0_createExitTask : public s_workAreaTemplate<sfieldA3_0_createExitTask>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sfieldA3_0_createExitTask::Init, &sfieldA3_0_createExitTask::Update, NULL, NULL };
        return &taskDefinition;
    }

    static void Init(sfieldA3_0_createExitTask* pThis)
    {
        static sVec3_FP fieldA3_0_exitPosition[8];

        for (s32 r12 = 7; r12 >= 0; r12--)
        {
            fixedPoint rotation = readSaturnFP({ 0x6081E64 + 4 * r12, gFLD_A3 });
            pThis->m1A0_rotation[r12].m_value[0] = getSin(rotation.getInteger() & 0xFFF) >> 4;
            pThis->m1A0_rotation[r12].m_value[1] = 0;
            pThis->m1A0_rotation[r12].m_value[2] = getCos(rotation.getInteger() & 0xFFF) >> 4;

            fieldA3_0_exitPosition[r12] = readSaturnVec3({ 0x6081DA4 + 4 * 3 * r12, gFLD_A3 });

            createLCSTarget(&pThis->m0_LCSTargets[r12], pThis, fieldA3_0_exitFunctionTable[r12], &fieldA3_0_exitPosition[r12], &pThis->m1A0_rotation[r12], 3, 0, -1, 0, 0);

            getFieldTaskPtr()->mC->m94_A3_0_exits[r12] = 0;
        }
    }

    static void Update(sfieldA3_0_createExitTask* pThis)
    {
        for (int r12 = 7; r12 >= 0; r12--)
        {
            updateLCSTarget(&pThis->m0_LCSTargets[r12]);
        }
    }

    std::array<sLCSTarget, 8> m0_LCSTargets; // stride is 0x34
    std::array<sVec3_FP, 8> m1A0_rotation;
    // size 0x200
};

void create_fieldA3_0_exitTask(p_workArea workArea)
{
    createSubTask<sfieldA3_0_createExitTask>(workArea);
}

void fieldA3_0_startTasks(p_workArea workArea)
{
    fieldA3_0_createTask0(workArea);

    getFieldTaskPtr()->mC->m168 = fieldA3_0_createTask1(workArea, 2, 0x20);

    create_fieldA3_0_exitTask(workArea);

    PDS_unimplemented("fieldA3_0_startTasks");

    create_fieldA3_0_task4(workArea);

    fieldA3_0_createCheckExitTask(workArea);

    fieldA3_0_createItemBoxes(workArea);

    create_fieldA3_0_tutorialTask(workArea);
}

void setupFieldCameraConfig_A3_0()
{
    setupFieldCameraConfigs(readCameraConfig({ 0x6081F44, gFLD_A3 }), 1);
}

void subfieldA3_0(p_workArea workArea)
{
    s16 r13 = getFieldTaskPtr()->m30;

    playPCM(workArea, 100);
    playPCM(workArea, 101);

    s_DataTable3* pDataTable3 = readDataTable3({ 0x6085AA4, gFLD_A3 });
    std::vector<std::vector<sCameraVisibility>>* pVisibility = readCameraVisbility({ 0x608F410, gFLD_A3 }, pDataTable3);
    s_DataTable2* pDataTable2 = readDataTable2({ 0x6083D3C, gFLD_A3 });
    setupField(pDataTable3, pDataTable2, fieldA3_0_startTasks, pVisibility);

    getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->mF4 = subfieldA3_0Sub0;

    {
        sVec3_FP position = { 0x28A000, 0x32000, 0x1294000 };
        sVec3_FP rotation = { 0x0, 0x4000000, 0x0 };
        setupDragonPosition(&position, &rotation);
    }

    if (r13 != -1)
    {
        if (getFieldTaskPtr()->m2C_currentFieldIndex == 21)
        {
            {
                sVec3_FP position = { 0x2EF000, 0x3C000, -0x2D1000 };
                sVec3_FP rotation = { 0x0, 0x16C16C, 0x0 };
                setupDragonPosition(&position, &rotation);
            }

            graphicEngineStatus.m405C.m14_farClipDistance = 0x2AE000;
            graphicEngineStatus.m405C.m38 = FP_Div(0x8000, 0x2AE000);

            graphicEngineStatus.m405C.m34 = graphicEngineStatus.m405C.m38 << 8;
        }
        else
        {
            //060542E4
            switch (getFieldTaskPtr()->m32)
            {
            case 4:
                if (getFieldTaskPtr()->m30)
                {
                    getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript({ 0x6090F54, gFLD_A3 });
                }
                else
                {
                    getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript({ 0x6090EEC, gFLD_A3 });
                }
                break;
            case 5:
                if (getFieldTaskPtr()->m30)
                {
                    getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript({ 0x6091024, gFLD_A3 });
                }
                else
                {
                    getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript({ 0x6090FBC, gFLD_A3 });
                }
                break;
            case 6:
                if (getFieldTaskPtr()->m30)
                {
                    startCutscene(loadCutsceneData({ 0x6091934, gFLD_A3 }));
                }
                else
                {
                    getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript({ 0x609108C, gFLD_A3 });
                }
                break;
            case 8:
                getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript({ 0x609115C, gFLD_A3 });
                break;
            case 10:
                getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript({ 0x6091128, gFLD_A3 });
                startFieldScript(17, -1);
                break;
            default:
                getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript({ 0x6090E84, gFLD_A3 });
                startFieldScript(17, -1);
                break;
            }
        }
    }
    //060543E0
    initFieldDragonLight();

    createFieldPaletteTask(workArea);

    setupFieldCameraConfig_A3_0();

    adjustVerticalLimits(-0x5A000, 0x76000);

    subfieldA3_1_Sub0();

    getFieldTaskPtr()->m8_pSubFieldData->m344_randomBattleTask->m0 = nullBattle;

    TaskUnimplemented();
    //subfieldA3_1_Sub1();
}
