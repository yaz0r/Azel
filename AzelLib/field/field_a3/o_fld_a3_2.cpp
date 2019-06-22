#include "PDS.h"
#include "a3_background_layer.h"

void fieldA3_2_exit0(p_workArea, sLCSTarget*)
{
    getFieldTaskPtr()->mC->m9F_A3_2_exits[0] = 1;
}

void fieldA3_2_exit1(p_workArea, sLCSTarget*)
{
    getFieldTaskPtr()->mC->m9F_A3_2_exits[1] = 1;
}

void fieldA3_2_exit2(p_workArea, sLCSTarget*)
{
    getFieldTaskPtr()->mC->m9F_A3_2_exits[2] = 1;
}

void fieldA3_2_exit3(p_workArea, sLCSTarget*)
{
    getFieldTaskPtr()->mC->m9F_A3_2_exits[3] = 1;
}

void(*fieldA3_2_exitFuncTable[])(p_workArea, sLCSTarget*) = {
    fieldA3_2_exit0,
    fieldA3_2_exit1,
    fieldA3_2_exit2,
    fieldA3_2_exit3
};

struct fieldA3_2_exitLCSTask : public s_workAreaTemplate<fieldA3_2_exitLCSTask>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &fieldA3_2_exitLCSTask::Init, &fieldA3_2_exitLCSTask::Update, NULL, NULL };
        return &taskDefinition;
    }

    static void Init(fieldA3_2_exitLCSTask* pThis)
    {
        for (int r12 = 3; r12 >= 0; r12--)
        {
            pThis->mD0[r12][0] = getSin(readSaturnFP(gFLD_A3->getSaturnPtr(0x6081E84 + 4 * r12)).getInteger() & 0xFFF) >> 4;
            pThis->mD0[r12][1] = 0;
            pThis->mD0[r12][2] = getCos(readSaturnFP(gFLD_A3->getSaturnPtr(0x6081E84 + 4 * r12)).getInteger() & 0xFFF) >> 4;

            static sVec3_FP exitLocations[4];
            exitLocations[r12] = readSaturnVec3(gFLD_A3->getSaturnPtr(0x6081E28 + 4 * 3 * r12));
            createLCSTarget(&pThis->m0[r12], pThis, fieldA3_2_exitFuncTable[r12], &exitLocations[r12], &pThis->mD0[r12], 3, 0, -1, 0, 0);

            getFieldTaskPtr()->mC->m9F_A3_2_exits[r12] = 0;
        }

        pThis->m0[1].m18_diableFlags |= 1;
    }

    static void UpdateSub0(fieldA3_2_exitLCSTask* pThis)
    {
        for (int r12 = 3; r12 >= 0; r12--)
        {
            updateLCSTarget(&pThis->m0[r12]);
        }
    }

    static void Update(fieldA3_2_exitLCSTask* pThis)
    {
        if (mainGameState.getBit(0x11, 5))
        {
            pThis->m0[1].m18_diableFlags = 0;
            pThis->m_UpdateMethod = fieldA3_2_exitLCSTask::UpdateSub0;
        }

        UpdateSub0(pThis);
    }

    std::array<sLCSTarget, 4> m0;
    std::array<sVec3_FP, 4> mD0;
    // size 0x100
};

void fieldA3_2_createExitLCSTask(p_workArea workArea)
{
    createSubTask<fieldA3_2_exitLCSTask>(workArea);
}

void fieldA3_2_createTask4(p_workArea workArea)
{
    FunctionUnimplemented();
}

const std::array<s16, 4> A3_2_exitsVars =
{
    1270, 1271, 1272, 1273
};

static const std::array<s8, 4> A3_2_exitsLocations =
{
    0xA, 0xB, 0xB, 0xC
};

struct sfieldA3_2_checkExitsTask : public s_workAreaTemplate<sfieldA3_2_checkExitsTask>
{
    static void Update(sfieldA3_2_checkExitsTask* pThis)
    {
        for (int i = 0; i < 4; i++)
        {
            if (getFieldTaskPtr()->mC->m9F_A3_2_exits[i])
            {
                mainGameState.setBit566(A3_2_exitsVars[i]);

                // Hack: this is in the function to be initialized after gFLD_A3
                static const std::array<sSaturnPtr, 8> A3_2_exitsCutscenes =
                {
                    {
                        {0x609122C, gFLD_A3},
                        {0x6091294, gFLD_A3},
                        {0x60912FC, gFLD_A3},
                        {0x6091364, gFLD_A3},
                    }
                };

                startExitFieldCutscene(pThis, readCameraScript(A3_2_exitsCutscenes[i]), A3_2_exitsLocations[i], i, 0x8000);
                pThis->m_UpdateMethod = NULL;
            }
        }
    }
};

void fieldA3_2_createCheckExitTask(p_workArea workArea)
{
    createSubTaskFromFunction<sfieldA3_2_checkExitsTask>(workArea, &sfieldA3_2_checkExitsTask::Update);
}

void fieldA3_2_createItemBoxes(p_workArea workArea)
{
    fieldA3_1_createItemBoxes_Sub0(workArea, readItemBoxDefinition({ 0x609219C, gFLD_A3 }));
    fieldA3_1_createItemBoxes_Sub1(readItemBoxDefinition({ 0x60921E4, gFLD_A3 }));
    fieldA3_1_createItemBoxes_Sub1(readItemBoxDefinition({ 0x609222C, gFLD_A3 }));
}

void fieldA3_2_startTasks(p_workArea workArea)
{
    fieldA3_0_createTask0(workArea);

    getFieldTaskPtr()->mC->m168 = fieldA3_0_createTask1(workArea, 4, 0x60);

    fieldA3_2_createExitLCSTask(workArea);
    create_fieldA3_backgroundLayer2(workArea);
    fieldA3_2_createTask4(workArea);
    fieldA3_2_createCheckExitTask(workArea);

    PDS_unimplemented("fieldA3_2_startTasks");

    fieldA3_2_createItemBoxes(workArea);
}

void subfieldA3_2Sub0(s_dragonTaskWorkArea*)
{
    PDS_unimplemented("subfieldA3_2Sub0");
}

void subfieldA3_2(p_workArea workArea)
{
    s16 r13 = getFieldTaskPtr()->m30;

    playPCM(workArea, 100);
    playPCM(workArea, 101);

    s_DataTable3* pDataTable3 = readDataTable3({ 0x608BE04, gFLD_A3 });
    std::vector<std::vector<sCameraVisibility>>* pVisibility = readCameraVisbility({ 0x608F8BC, gFLD_A3 }, pDataTable3);
    s_DataTable2* pDataTable2 = readDataTable2({ 0x6088E8C, gFLD_A3 });
    setupField(pDataTable3, pDataTable2, fieldA3_2_startTasks, pVisibility);

    getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->mF4 = subfieldA3_2Sub0;

    switch (getFieldTaskPtr()->m32)
    {
    default:
        getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript({ 0x6091260, gFLD_A3 });
        break;
    }

    startFieldScript(19, -1);

    initFieldDragonLight();

    createFieldPaletteTask(workArea);

    adjustVerticalLimits(-0x54000, 0x76000);

    subfieldA3_1_Sub0();

    //subfieldA3_1_Sub1();

    PDS_unimplemented("subfieldA3_2");
}
