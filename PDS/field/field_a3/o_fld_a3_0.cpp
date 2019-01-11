#include "PDS.h"

void subfieldA3_0Sub0(s_dragonTaskWorkArea* r4)
{
    PDS_unimplemented("subfieldA3_0Sub0");
}

p_workArea create_fieldA3_0_task0(p_workArea workArea)
{
    s_fieldTaskWorkArea_C* newWorkArea = createSubTaskFromFunction<s_fieldTaskWorkArea_C>(workArea, NULL);
    getFieldTaskPtr()->mC = newWorkArea;
    return newWorkArea;
}

p_workArea create_fieldA3_0_task1(p_workArea workArea, s32 r5, s32 r6)
{
    PDS_unimplemented("create_fieldA3_0_task1");
    return workArea;
}

struct s_fieldA3_0_tutorialTask : public s_workAreaTemplate<s_fieldA3_0_tutorialTask>
{

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

void fieldA3_0_startTasks(p_workArea workArea)
{
    create_fieldA3_0_task0(workArea);

    getFieldTaskPtr()->mC->m168 = create_fieldA3_0_task1(workArea, 2, 0x20);

    PDS_unimplemented("fieldA3_0_startTasks");

    create_fieldA3_0_tutorialTask(workArea);
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
            case 5:
            case 6:
            case 8:
            case 10:
                assert(0);
            default:
                getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript({ 0x6090E84, gFLD_A3 });
                startFieldScript(17, -1);
                break;
            }
        }
    }
    //060543E0
    PDS_unimplemented("subfieldA3_0");

    createFieldPaletteTask(workArea);

    //TODO: more stuff here

    adjustVerticalLimits(-0x54000, 0x76000);
}
