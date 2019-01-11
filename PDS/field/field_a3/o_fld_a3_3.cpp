#include "PDS.h"

void fieldA3_3_createItemBoxes(p_workArea workArea)
{
    fieldA3_1_createItemBoxes_Sub1(readItemBoxDefinition({ 0x6092274, gFLD_A3 }));
    fieldA3_1_createItemBoxes_Sub1(readItemBoxDefinition({ 0x60922BC, gFLD_A3 }));
}

void fieldA3_3_startTasks(p_workArea workArea)
{
    create_fieldA3_0_task0(workArea);

    PDS_unimplemented("fieldA3_3_startTasks");

    fieldA3_3_createItemBoxes(workArea);
}

void subfieldA3_3(p_workArea workArea)
{
    playPCM(workArea, 100);
    playPCM(workArea, 101);

    s_DataTable3* pDataTable3 = readDataTable3({ 0x608C054, gFLD_A3 });
    std::vector<std::vector<sCameraVisibility>>* pVisibility = readCameraVisbility({ 0x608F9E8, gFLD_A3 }, pDataTable3);
    setupField3(pDataTable3, fieldA3_3_startTasks, pVisibility);

    getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript({ 0x6091400, gFLD_A3 });

    PDS_unimplemented("subfieldA3_3");

    createFieldPaletteTask(workArea);

    adjustVerticalLimits(-0x5C000, 0x76000);

    subfieldA3_1_Sub0();

    PDS_unimplemented("subfieldA3_3");

    startFieldScript(20, -1);


}
