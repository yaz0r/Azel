#include "PDS.h"
#include "o_fld_a5.h"
#include "field/field_a3/o_fld_a3.h"
#include "audio/soundDriver.h"

// 0605a7e0
static void fieldA5_3_startTasks(p_workArea workArea)
{
    createFieldSpecificDataTask_A5(workArea);
    createA5_corridorWorm_3(workArea);
    createA5_wormObjectTask(workArea);
    createA5_exitEntityTask(workArea);
    static s32 exitParams[] = { 0xFA000, (s32)0xFFF4C000 };
    setFieldExitConfig_A5(3, exitParams);
}

// 060546CC
void subfieldA5_3(p_workArea workArea)
{
    playPCM(workArea, 0x71);
    s_DataTable3* pDT3 = readDataTable3(gFLD_A5->getSaturnPtr(0x0608F068));
    setupField2(pDT3, fieldA5_3_startTasks);
    setupCameraConfig_A5_corridor();
    getFieldTaskPtr()->m8_pSubFieldData->m334->m50E_followModeIndex = 2;
    s16 prev = getFieldTaskPtr()->m32_previousSubField;
    sSaturnPtr rotEA;
    if (prev == 4 || prev == 9)
        rotEA = gFLD_A5->getSaturnPtr(0x060898A4);
    else
        rotEA = gFLD_A5->getSaturnPtr(0x0608988C);
    setupDragonPositionAndCamera_A5(rotEA - 0xC, rotEA);
    initDragonParams_A5_corridor();
    createA5CorridorVdp2Task(workArea);
    getFieldTaskPtr()->m8_pSubFieldData->m344_randomBattleTask->m0 = (void(*)())&postBattleSound_A5_corridor;
}
