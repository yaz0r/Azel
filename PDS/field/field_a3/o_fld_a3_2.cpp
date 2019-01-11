#include "PDS.h"

void fieldA3_2_startTasks(p_workArea workArea)
{
    create_fieldA3_0_task0(workArea);

    getFieldTaskPtr()->mC->m168 = create_fieldA3_0_task1(workArea, 4, 0x60);

    PDS_unimplemented("fieldA3_2_startTasks");
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

    createFieldPaletteTask(workArea);

    adjustVerticalLimits(-0x54000, 0x76000);

    subfieldA3_1_Sub0();

    //subfieldA3_1_Sub1();

    PDS_unimplemented("subfieldA3_2");
}
