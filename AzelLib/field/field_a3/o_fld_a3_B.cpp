#include "PDS.h"
#include "o_fld_a3_0.h"
#include "o_fld_a3_4.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldItemBox.h"

// 06057610
struct fieldA3_B_exitTask : public s_workAreaTemplate<fieldA3_B_exitTask>
{
    static void Update(fieldA3_B_exitTask* pThis)
    {
        const sVec3_FP& pos = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m8_pos;

        if (pos[2] <= -0x44000)
        {
            if (pos[2] < -0x3C8000)
            {
                mainGameState.setBit566(1272);
                exitCutsceneTaskUpdateSub0(2, 1, -1);
                pThis->m_UpdateMethod = nullptr;
            }
        }
        else
        {
            mainGameState.setBit566(1271);
            exitCutsceneTaskUpdateSub0(2, 0, -1);
            pThis->m_UpdateMethod = nullptr;
        }
    }

    //size C
};

// 060577e0
void fieldA3_B_createExitTask(p_workArea workArea)
{
    createSubTaskFromFunction<fieldA3_B_exitTask>(workArea, &fieldA3_B_exitTask::Update);
}

// 0605c620
void fieldA3_B_createItemBoxes()
{
    fieldA3_1_createItemBoxes_Sub1(readItemBoxDefinition({ 0x60923DC, gFLD_A3 }));
    fieldA3_1_createItemBoxes_Sub1(readItemBoxDefinition({ 0x6092424, gFLD_A3 }));
    fieldA3_1_createItemBoxes_Sub1(readItemBoxDefinition({ 0x609246C, gFLD_A3 }));
}

// 06057d9e
void fieldA3_B_startTasks(p_workArea workArea)
{
    fieldA3_B_createExitTask(workArea);
    fieldA3_B_createItemBoxes();
}

// 06054A50
void subfieldA3_B(p_workArea workArea)
{
    soundFunc(100);
    soundFunc(101);

    s_DataTable3* pDataTable3 = readDataTable3({ 0x608EEA4, gFLD_A3 });
    setupField2(pDataTable3, fieldA3_B_startTasks);

    if (getFieldTaskPtr()->m30_fieldEntryPoint == 1)
    {
        sVec3_FP position = readSaturnVec3({ 0x6081BA8, gFLD_A3 });
        sVec3_FP rotation = readSaturnVec3({ 0x6081BD8, gFLD_A3 });
        setupDragonPosition(&position, &rotation);
    }
    else
    {
        sVec3_FP position = readSaturnVec3({ 0x6081BB4, gFLD_A3 });
        sVec3_FP rotation = readSaturnVec3({ 0x6081C08, gFLD_A3 });
        setupDragonPosition(&position, &rotation);
    }

    setupDragonCaveLighting();
    setupCaveCameraConfig();
    createVDP2FieldTask(workArea);
}
