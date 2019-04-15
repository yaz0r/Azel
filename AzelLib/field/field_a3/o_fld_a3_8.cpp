#include "PDS.h"
#include "o_fld_a3_0.h"
#include "o_fld_a3_4.h"

struct fieldA3_8_exitTask : public s_workAreaTemplate<fieldA3_8_exitTask>
{
    static void Update(fieldA3_8_exitTask* pThis)
    {
        if (getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m8_pos[2] > -0x48000)
        {
            mainGameState.setBit566(1269);
            subfieldA3_1Sub0Sub2(0, -1);
            pThis->m_UpdateMethod = NULL;
        }
    }

    s32 m8;
    //size C
};

void fieldA3_8_createExitTask(p_workArea workArea)
{
    createSubTaskFromFunction<fieldA3_8_exitTask>(workArea, &fieldA3_8_exitTask::Update);
}

void fieldA3_8_startTasks(p_workArea workArea)
{
    TaskUnimplemented();
    fieldA3_8_createExitTask(workArea);
}

void subfieldA3_8(p_workArea workArea)
{
    soundFunc(100);
    soundFunc(101);

    loadFileFromFileList(2);

    s_DataTable3* pDataTable3 = readDataTable3({ 0x608EE38, gFLD_A3 });
    setupField2(pDataTable3, fieldA3_8_startTasks);

    setupDragonPosition(&readSaturnVec3({ 0x6081B6C, gFLD_A3 }), &readSaturnVec3({ 0x6081BD8, gFLD_A3 }));

    subfieldA3_4_sub0();

    setupFieldCameraConfig_A3_4();

    subfieldA3_4_sub1(workArea);
}
