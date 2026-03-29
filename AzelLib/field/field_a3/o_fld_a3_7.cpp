#include "PDS.h"
#include "o_fld_a3_0.h"
#include "o_fld_a3_4.h"
#include "field/field_a3/o_fld_a3.h"

// 060572e8
struct fieldA3_7_exitTask : public s_workAreaTemplate<fieldA3_7_exitTask>
{
    static void Update(fieldA3_7_exitTask* pThis)
    {
        s_dragonTaskWorkArea* pDragonTask = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
        const sVec3_FP& pos = pDragonTask->m8_pos;

        if (pos[2] <= -0x40000)
        {
            if (pos[2] >= -0x2C4000)
            {
                goto end;
            }
            if (fileInfoStruct.m2C_allocatedHead != NULL)
            {
                initDragonMovementMode();
                goto end;
            }
            subfieldA3_1Sub0Sub2(1, -1);
        }
        else
        {
            if (fileInfoStruct.m2C_allocatedHead != NULL)
            {
                initDragonMovementMode();
                goto end;
            }
            subfieldA3_1Sub0Sub2(0, -1);
        }
        pThis->m_UpdateMethod = nullptr;
    end:
        fieldA3_A_task0Sub0();
    }

    //size C
};

// 060577c0
void fieldA3_7_createExitTask(p_workArea workArea)
{
    createSubTaskFromFunction<fieldA3_7_exitTask>(workArea, &fieldA3_7_exitTask::Update);
}

// 060579fc
struct fieldA3_7_locationTask : public s_workAreaTemplate<fieldA3_7_locationTask>
{
    static void Init(fieldA3_7_locationTask* pThis)
    {
        if (!mainGameState.getBit(0xA2 * 8 + 3))
        {
            pThis->m0 = 0x228000;
            pThis->m4 = 0;
            pThis->m8 = -0x2E2000;
        }
        else
        {
            pThis->m0 = 0x68000;
            pThis->m4 = 0;
            pThis->m8 = -0x22000;
        }
    }

    static void Update(fieldA3_7_locationTask* pThis)
    {
        return;
    }

    s32 m0;
    s32 m4;
    s32 m8;
    // size C
};

static const fieldA3_7_locationTask::TypedTaskDefinition fieldA3_7_locationTask_definition = {
    fieldA3_7_locationTask::Init,
    fieldA3_7_locationTask::Update,
    nullptr,
    nullptr
};

// 06057abe
void fieldA3_7_createLocationTask(p_workArea workArea)
{
    createSubTask<fieldA3_7_locationTask>(workArea, &fieldA3_7_locationTask_definition);
}

// 06057d52
void fieldA3_7_startTasks(p_workArea workArea)
{
    fieldA3_7_createExitTask(workArea);
    fieldA3_7_createLocationTask(workArea);
}

// 06054896
void subfieldA3_7(p_workArea workArea)
{
    soundFunc(100);
    soundFunc(101);

    s_DataTable3* pDataTable3 = readDataTable3({ 0x608EE14, gFLD_A3 });
    setupField2(pDataTable3, fieldA3_7_startTasks);

    if (getFieldTaskPtr()->m32_previousSubField == 0)
    {
        sVec3_FP position = readSaturnVec3({ 0x6081B54, gFLD_A3 });
        sVec3_FP rotation = readSaturnVec3({ 0x6081BD8, gFLD_A3 });
        setupDragonPosition(&position, &rotation);
    }
    else
    {
        sVec3_FP position = readSaturnVec3({ 0x6081B60, gFLD_A3 });
        sVec3_FP rotation = readSaturnVec3({ 0x6081C08, gFLD_A3 });
        setupDragonPosition(&position, &rotation);
    }

    setupDragonCaveLighting();
    setupCaveCameraConfig();
    createVDP2FieldTask(workArea);
}
