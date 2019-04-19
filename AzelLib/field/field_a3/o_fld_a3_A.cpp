#include "PDS.h"
#include "o_fld_a3_0.h"
#include "o_fld_a3_4.h"

void fieldA3_A_createTask0(p_workArea workArea)
{
}

void fieldA3_A_createTask1(p_workArea workArea)
{
}

void fieldA3_A_startTasks(p_workArea workArea)
{
    fieldA3_A_createTask0(workArea);
    fieldA3_A_createTask1(workArea);
}

void subfieldA3_A(p_workArea workArea)
{
    soundFunc(100);
    soundFunc(101);

    s_DataTable3* pDataTable3 = readDataTable3({ 0x608EE80, gFLD_A3 });
    setupField2(pDataTable3, fieldA3_A_startTasks);

    if (getFieldTaskPtr()->m30 == 0)
    {
        setupDragonPosition(&readSaturnVec3({ 0x6081B90, gFLD_A3 }), &readSaturnVec3({ 0x6081C20, gFLD_A3 }));
    }
    else
    {
        setupDragonPosition(&readSaturnVec3({ 0x6081B9C, gFLD_A3 }), &readSaturnVec3({ 0x6081C08, gFLD_A3 }));
    }

    subfieldA3_4_sub0();

    setupFieldCameraConfig_A3_4();

    subfieldA3_4_sub1(workArea);
}
