#include "PDS.h"
#include "o_fld_a3_0.h"

void subfieldA3_4_sub0()
{
    TaskUnimplemented();
}

void subfieldA3_4_sub1(p_workArea workArea)
{
    TaskUnimplemented();
}

void setupFieldCameraConfig_A3_4()
{
    setupFieldCameraConfigs(readCameraConfig({ 0x60822B4, gFLD_A3 }), 1);
}

struct fieldA3_4_exitTask : public s_workAreaTemplate<fieldA3_4_exitTask>
{
    static void Update(fieldA3_4_exitTask* pThis)
    {
        if (getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m8_pos[0] < 0x40000)
        {
            mainGameState.setBit566(A3_0_exitsVars[0]);
            exitCutsceneTaskUpdateSub0(0, 0, -1);
            pThis->m8 = 0;
        }
        else if (getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m8_pos[2] < -0x1A2000)
        {
            mainGameState.setBit566(A3_0_exitsVars[1]);
            exitCutsceneTaskUpdateSub0(0, 1, -1);
            pThis->m8 = 0;
        }
    }

    s32 m8;
    //size C
};

void fieldA3_4_createExitTask(p_workArea workArea)
{
    createSubTaskFromFunction<fieldA3_4_exitTask>(workArea, &fieldA3_4_exitTask::Update);
}

void fieldA3_4_createItemBoxes(p_workArea workArea)
{
    fieldA3_1_createItemBoxes_Sub1(readItemBoxDefinition({ 0x6092304, gFLD_A3 }));
}

void fieldA3_4_startTasks(p_workArea workArea)
{
    fieldA3_4_createExitTask(workArea);
    fieldA3_4_createItemBoxes(workArea);
}

void subfieldA3_4(p_workArea workArea)
{
    soundFunc(100);
    soundFunc(101);

    s_DataTable3* pDataTable3 = readDataTable3({ 0x608EDA8, gFLD_A3 });
    setupField2(pDataTable3, fieldA3_4_startTasks);

    if (!getFieldTaskPtr()->m30)
    {
        setupDragonPosition(&readSaturnVec3({ 0x6081B0C, gFLD_A3 }), &readSaturnVec3({ 0x6081C20, gFLD_A3 }));
    }
    else
    {
        setupDragonPosition(&readSaturnVec3({ 0x6081B0C + 0xC, gFLD_A3 }), &readSaturnVec3({ 0x6081C08, gFLD_A3 }));
    }

    subfieldA3_4_sub0();

    setupFieldCameraConfig_A3_4();

    subfieldA3_4_sub1(workArea);
}
