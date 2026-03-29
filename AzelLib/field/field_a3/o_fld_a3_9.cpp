#include "PDS.h"
#include "o_fld_a3_0.h"
#include "o_fld_a3_4.h"
#include "field/field_a3/o_fld_a3.h"

// 060573f8
struct fieldA3_9_exitTask : public s_workAreaTemplate<fieldA3_9_exitTask>
{
    static void Update(fieldA3_9_exitTask* pThis)
    {
        s_dragonTaskWorkArea* pDragonTask = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
        const sVec3_FP& pos = pDragonTask->m8_pos;

        if (pos[2] <= -0x60000)
        {
            if (pos[2] >= -0x1A4000)
            {
                goto end;
            }
            if (fileInfoStruct.m2C_allocatedHead != NULL)
            {
                initDragonMovementMode();
                goto end;
            }
            mainGameState.setBit566(1274);
            subfieldA3_1Sub0Sub2(3, -1);
        }
        else
        {
            if (fileInfoStruct.m2C_allocatedHead != NULL)
            {
                initDragonMovementMode();
                goto end;
            }
            mainGameState.setBit566(1261);
            subfieldA3_1Sub0Sub2(1, -1);
        }
        pThis->m_UpdateMethod = nullptr;
    end:
        fieldA3_A_task0Sub0();
    }

    //size C
};

// 060577d0
void fieldA3_9_createExitTask(p_workArea workArea)
{
    createSubTaskFromFunction<fieldA3_9_exitTask>(workArea, &fieldA3_9_exitTask::Update);
}

// 06057d82
void fieldA3_9_startTasks(p_workArea workArea)
{
    fieldA3_9_createExitTask(workArea);
}

// 06054926
void subfieldA3_9(p_workArea workArea)
{
    soundFunc(100);
    soundFunc(101);

    s_DataTable3* pDataTable3 = readDataTable3({ 0x608EE5C, gFLD_A3 });
    setupField2(pDataTable3, fieldA3_9_startTasks);

    if (getFieldTaskPtr()->m32_previousSubField == 3)
    {
        sVec3_FP position = readSaturnVec3({ 0x6081B84, gFLD_A3 });
        sVec3_FP rotation = readSaturnVec3({ 0x6081C08, gFLD_A3 });
        setupDragonPosition(&position, &rotation);
    }
    else
    {
        sVec3_FP position = readSaturnVec3({ 0x6081B78, gFLD_A3 });
        sVec3_FP rotation = readSaturnVec3({ 0x6081BD8, gFLD_A3 });
        setupDragonPosition(&position, &rotation);
    }

    setupDragonCaveLighting();
    setupCaveCameraConfig();
    createVDP2FieldTask(workArea);
}
