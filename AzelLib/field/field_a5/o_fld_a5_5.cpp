#include "PDS.h"
#include "o_fld_a5.h"
#include "field/field_a3/o_fld_a3.h"
#include "audio/soundDriver.h"

// 0605a87a
static void fieldA5_5_startTasks(p_workArea workArea)
{
    createFieldSpecificDataTask_A5(workArea);
    createA5_corridorWorm_5(workArea);
    createA5_envObject_5();
    createA5_wormObjectTask(workArea);
    createA5_exitEntityTask(workArea);
    static s32 exitParams[] = { 0x64000, (s32)0xFFF4C000 };
    setFieldExitConfig_A5(3, exitParams);
}

// 06054840
void subfieldA5_5(p_workArea workArea)
{
    playPCM(workArea, 0x71);
    s_DataTable3* pDT3 = readDataTable3(gFLD_A5->getSaturnPtr(0x0608F08C));
    setupField2(pDT3, fieldA5_5_startTasks);
    setupCameraConfig_A5_corridor();
    getFieldTaskPtr()->m8_pSubFieldData->m334->m50E_followModeIndex = 2;
    setupDragonPositionAndCamera_A5(gFLD_A5->getSaturnPtr(0x060898E4), gFLD_A5->getSaturnPtr(0x060898F0));
    initDragonParams_A5_corridor();
    createA5CorridorVdp2Task(workArea);
    getFieldTaskPtr()->m8_pSubFieldData->m344_randomBattleTask->m0 = (void(*)())&postBattleSound_A5_corridor;
}
