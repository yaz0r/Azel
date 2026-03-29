#include "PDS.h"
#include "o_fld_a3_0.h"
#include "o_fld_a3_4.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldItemBox.h"

// 06057212
struct fieldA3_6_exitTask : public s_workAreaTemplate<fieldA3_6_exitTask>
{
    static void Update(fieldA3_6_exitTask* pThis)
    {
        const sVec3_FP& pos = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m8_pos;

        if (pos[0] > 0x128000)
        {
            if (pos[2] <= -0x28E000)
            {
                mainGameState.setBit566(A3_0_exitsVars[5]);
                exitCutsceneTaskUpdateSub0(0, 1, -1);
            }
            else
            {
                mainGameState.setBit566(A3_0_exitsVars[4]);
                exitCutsceneTaskUpdateSub0(0, 0, -1);
            }
            pThis->m_UpdateMethod = nullptr;
        }
    }

    //size C
};

// 060577b8
void fieldA3_6_createExitTask(p_workArea workArea)
{
    createSubTaskFromFunction<fieldA3_6_exitTask>(workArea, &fieldA3_6_exitTask::Update);
}

// 0605c618
void fieldA3_6_createItemBoxes()
{
    fieldA3_1_createItemBoxes_Sub1(readItemBoxDefinition({ 0x6092394, gFLD_A3 }));
}

// 06057d3c
void fieldA3_6_startTasks(p_workArea workArea)
{
    fieldA3_6_createExitTask(workArea);
    fieldA3_6_createItemBoxes();
}

// 0605483c
void subfieldA3_6(p_workArea workArea)
{
    soundFunc(100);
    soundFunc(101);

    s_DataTable3* pDataTable3 = readDataTable3({ 0x608EDF0, gFLD_A3 });
    setupField2(pDataTable3, fieldA3_6_startTasks);

    if (getFieldTaskPtr()->m30_fieldEntryPoint == 4)
    {
        sVec3_FP position = readSaturnVec3({ 0x6081B3C, gFLD_A3 });
        sVec3_FP rotation = readSaturnVec3({ 0x6081BE4, gFLD_A3 });
        setupDragonPosition(&position, &rotation);
    }
    else
    {
        sVec3_FP position = readSaturnVec3({ 0x6081B48, gFLD_A3 });
        sVec3_FP rotation = readSaturnVec3({ 0x6081BFC, gFLD_A3 });
        setupDragonPosition(&position, &rotation);
    }

    setupDragonCaveLighting();
    setupCaveCameraConfig();
    createVDP2FieldTask(workArea);
}
