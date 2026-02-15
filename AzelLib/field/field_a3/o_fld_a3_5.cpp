#include "PDS.h"
#include "o_fld_a3_0.h"
#include "o_fld_a3_4.h"
#include "field/field_a3/o_fld_a3.h" //TODO: cleanup
#include "field/fieldItemBox.h"

struct fieldA3_5_exitTask : public s_workAreaTemplate<fieldA3_5_exitTask>
{
    static void Update(fieldA3_5_exitTask* pThis)
    {
        if (getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m8_pos[2] >= -0x3E000)
        {
            mainGameState.setBit566(A3_0_exitsVars[2]);
            exitCutsceneTaskUpdateSub0(0, 0, -1);
            pThis->m8 = 0;
        }
        else if (getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m8_pos[0] < 0x48000)
        {
            mainGameState.setBit566(A3_0_exitsVars[3]);
            exitCutsceneTaskUpdateSub0(0, 1, -1);
            pThis->m8 = 0;
        }
    }

    s32 m8;
    //size C
};

void fieldA3_5_createExitTask(p_workArea workArea)
{
    createSubTaskFromFunction<fieldA3_5_exitTask>(workArea, &fieldA3_5_exitTask::Update);
}

void fieldA3_5_createItemBoxes(p_workArea workArea)
{
    fieldA3_1_createItemBoxes_Sub1(readItemBoxDefinition({ 0x609234C, gFLD_A3 }));
}

void fieldA3_5_startTasks(p_workArea workArea)
{
    fieldA3_5_createExitTask(workArea);
    fieldA3_5_createItemBoxes(workArea);
}

void subfieldA3_5(p_workArea workArea)
{
    soundFunc(100);
    soundFunc(101);

    s_DataTable3* pDataTable3 = readDataTable3({ 0x608EDCC, gFLD_A3 });
    setupField2(pDataTable3, fieldA3_5_startTasks);

    if (getFieldTaskPtr()->m30_fieldEntryPoint == 2)
    {
        sVec3_FP position = readSaturnVec3({ 0x6081B24, gFLD_A3 });
        sVec3_FP rotation = readSaturnVec3({ 0x6081BD8, gFLD_A3 });
        setupDragonPosition(&position, &rotation);
    }
    else
    {
        sVec3_FP position = readSaturnVec3({ 0x6081B24 + 0xC, gFLD_A3 });
        sVec3_FP rotation = readSaturnVec3({ 0x6081C14, gFLD_A3 });
        setupDragonPosition(&position, &rotation);
    }

    subfieldA3_4_sub0();

    setupFieldCameraConfig_A3_4();

    createVDP2FieldTask(workArea);
}
