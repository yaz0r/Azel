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

struct fieldA3_8_generatorTask : public s_workAreaTemplate<fieldA3_8_generatorTask>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { NULL, &fieldA3_8_generatorTask::Update, &fieldA3_8_generatorTask::Draw, NULL };
        return &taskDefinition;
    }

    void Update(fieldA3_8_generatorTask* pThis)
    {
        TaskUnimplemented();
    }

    void Draw(fieldA3_8_generatorTask* pThis)
    {
        TaskUnimplemented();
    }

    s_memoryAreaOutput m0;
    sVec3_FP m8;
    // size: 0xA0
};

void fieldA3_8_create_generatorTask(p_workArea workArea)
{
    fieldA3_8_generatorTask* pNewTask = createSubTask<fieldA3_8_generatorTask>(workArea);

    getMemoryArea(&pNewTask->m0, 2);
    pNewTask->m8 = readSaturnVec3(gFLD_A3->getSaturnPtr(0x608EEEC));

    TaskUnimplemented();
}

//TODO: Really not sure what the point of this is. Maybe check the locations on the map to see what it maps to?
struct fieldA3_8_task2 : public s_workAreaTemplate<fieldA3_8_task2>
{
    static void Update(fieldA3_8_task2* pThis)
    {
        if (mainGameState.getBit(0xA2, 3))
        {
            pThis->m0 = 0x72000;
            pThis->m4 = 0;
            pThis->m8 = -0x2A000;
        }
        else
        {
            pThis->m0 = 0x25000;
            pThis->m4 = 0;
            pThis->m8 = -0x20A000;
        }
    }

    s32 m0;
    s32 m4;
    s32 m8;
    //size C
};

void fieldA3_8_create_task2(p_workArea workArea)
{
    createSubTaskFromFunction<fieldA3_8_task2>(workArea, &fieldA3_8_task2::Update);
}

void fieldA3_8_startTasks(p_workArea workArea)
{
    fieldA3_8_create_generatorTask(workArea);
    fieldA3_8_createExitTask(workArea);
    fieldA3_8_create_task2(workArea);
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
