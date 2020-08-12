#include "PDS.h"
#include "o_fld_a3_0.h"
#include "o_fld_a3_4.h"
#include "field/field_a3/o_fld_a3.h" //TODO: cleanup

void fieldA3_A_task0Sub0()
{
    FunctionUnimplemented();
}

struct fieldA3_A_task0 : public s_workAreaTemplate<fieldA3_A_task0>
{
    static void Update(fieldA3_A_task0* pThis)
    {
        const sVec3_FP& dragonPosition = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m8_pos;

        if (dragonPosition[0] < 0x56000)
        {
            if(fileInfoStruct.m2C_allocatedHead == NULL)
            {
                mainGameState.setBit566(1268);
                subfieldA3_1Sub0Sub2(0, -1);
                pThis->m_UpdateMethod = nullptr;
            }
            else
            {
                subfieldA3_1Sub0Sub0();
            }
        }
        else if(dragonPosition[2] < -0x216000)
        {
            if (fileInfoStruct.m2C_allocatedHead == NULL)
            {
                mainGameState.setBit566(1270);
                subfieldA3_1Sub0Sub2(2, -1);
                pThis->m_UpdateMethod = nullptr;
            }
            else
            {
                subfieldA3_1Sub0Sub0();
            }
        }

        //60575CA
        fieldA3_A_task0Sub0();
    }

    //size C
};


void fieldA3_A_createTask0(p_workArea workArea)
{
    createSubTaskFromFunction<fieldA3_A_task0>(workArea, &fieldA3_A_task0::Update);
}

//TODO: Really not sure what the point of this is. Maybe check the locations on the map to see what it maps to?
struct fieldA3_A_task1 : public s_workAreaTemplate<fieldA3_A_task1>
{
    //aliased to fieldA3_1_task4_update
    static void fieldA3_A_task1_update(fieldA3_A_task1* pThis)
    {
        return;
    }

    static void Update(fieldA3_A_task1* pThis)
    {
        if (mainGameState.getBit(0x11, 7))
        {
            pThis->getTask()->markFinished();
        }
        else
        {
            pThis->m0 = 0x198000;
            pThis->m4 = 0;
            pThis->m8 = -0x234000;

            pThis->m_UpdateMethod = fieldA3_A_task1_update;
        }
    }

    s32 m0;
    s32 m4;
    s32 m8;
    //size C
};

void fieldA3_A_createTask1(p_workArea workArea)
{
    createSubTaskFromFunction<fieldA3_A_task1>(workArea, &fieldA3_A_task1::Update);
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

    if (getFieldTaskPtr()->m32 == 0)
    {
        sVec3_FP position = readSaturnVec3({ 0x6081B90, gFLD_A3 });
        sVec3_FP rotation = readSaturnVec3({ 0x6081C20, gFLD_A3 });
        setupDragonPosition(&position, &rotation);
    }
    else
    {
        sVec3_FP position = readSaturnVec3({ 0x6081B9C, gFLD_A3 });
        sVec3_FP rotation = readSaturnVec3({ 0x6081C08, gFLD_A3 });
        setupDragonPosition(&position, &rotation);
    }

    subfieldA3_4_sub0();

    setupFieldCameraConfig_A3_4();

    createVDP2FieldTask(workArea);
}
