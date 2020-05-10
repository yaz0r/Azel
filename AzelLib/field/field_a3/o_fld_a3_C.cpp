#include "PDS.h"
#include "field/field_a3/o_fld_a3.h" //TODO: cleanup

struct fieldA3_C_task0 : public s_workAreaTemplate<fieldA3_C_task0>
{
    static void Update(fieldA3_C_task0* pThis)
    {
        const sVec3_FP& dragonPosition = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m8_pos;

        if (dragonPosition[2] > -0x47999)
        {
            mainGameState.setBit566(1273);
            exitCutsceneTaskUpdateSub0(2, 0, -1);
            pThis->m_UpdateMethod = nullptr;
        }
        else if(dragonPosition[2] < -0x2CB999)
        {
            if(mainGameState.getBit(0xA, 7))
            {
                dispatchTutorialMultiChoiceSub2();
            }
            else
            {
                fieldA3_1_checkExitsTaskUpdate2Sub1(8);
            }
            pThis->m_UpdateMethod = nullptr;
        }
    }

    //size C
};

void fieldA3_C_createTask0(p_workArea workArea)
{
    createSubTaskFromFunction<fieldA3_C_task0>(workArea, &fieldA3_C_task0::Update);
}

struct fieldA3_C_task1 : public s_workAreaTemplate<fieldA3_C_task1>
{
    static void NullUpdate(fieldA3_C_task1* pThis)
    {
        return;
    }

    static void Update(fieldA3_C_task1* pThis)
    {
        if (!mainGameState.getBit(0x11, 7))
        {
            pThis->m0 = 0x2A3333;
            pThis->m4 = 0;
            pThis->m8 = -0x2E9999;

            pThis->m_UpdateMethod = NullUpdate;
        }
    }

    s32 m0;
    s32 m4;
    s32 m8;
    //size C
};

void fieldA3_C_createTask1(p_workArea workArea)
{
    createSubTaskFromFunction<fieldA3_C_task1>(workArea, &fieldA3_C_task1::Update);
}

void fieldA3_C_createItemBox(p_workArea workArea)
{
    fieldA3_1_createItemBoxes_Sub1(readItemBoxDefinition({ 0x60924B4, gFLD_A3 }));
}

void fieldA3_C_startTasks(p_workArea workArea)
{
    fieldA3_C_createTask0(workArea);
    fieldA3_C_createTask1(workArea);
    fieldA3_C_createItemBox(workArea);
}

void subfieldA3_C(p_workArea workArea)
{
    soundFunc(100);
    soundFunc(101);

    s_DataTable3* pDataTable3 = readDataTable3({ 0x608EEC8, gFLD_A3 });
    setupField2(pDataTable3, fieldA3_C_startTasks);

    if (getFieldTaskPtr()->m30 == -1)
    {
        sVec3_FP position = readSaturnVec3({ 0x6081CD0 - 0xC, gFLD_A3 });
        sVec3_FP rotation = readSaturnVec3({ 0x6081CD0, gFLD_A3 });
        setupDragonPosition(&position, &rotation);
    }
    else
    {
        if (getFieldTaskPtr()->m32 == 2)
        {
            sVec3_FP position = readSaturnVec3({ 0x6081BC0, gFLD_A3 });
            sVec3_FP rotation = readSaturnVec3({ 0x6081BD8, gFLD_A3 });
            setupDragonPosition(&position, &rotation);
        }
        else
        {
            sVec3_FP position = readSaturnVec3({ 0x6081BCC, gFLD_A3 });
            sVec3_FP rotation = readSaturnVec3({ 0x6081C08, gFLD_A3 });
            setupDragonPosition(&position, &rotation);
        }
    }

    subfieldA3_4_sub0();

    setupFieldCameraConfig_A3_4();

    subfieldA3_4_sub1(workArea);
}
