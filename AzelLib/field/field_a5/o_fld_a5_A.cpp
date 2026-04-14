#include "PDS.h"
#include "o_fld_a5.h"
#include "field/field_a3/o_fld_a3.h"
#include "audio/soundDriver.h"

// 0605aad2
static void fieldA5_A_startTasks(p_workArea workArea)
{
    createFieldSpecificDataTask_A5(workArea);
    createA5_corridorWorm_A(workArea);
    createA5_envObject_A();
    createA5_wormObjectTask(workArea);
    createA5_exitEntityTask(workArea);
    static s32 exitParams[] = { 0xE6000, (s32)0xFFFCE000 };
    setFieldExitConfig_A5(3, exitParams);
}

// 06054C68
void subfieldA5_A(p_workArea workArea)
{
    playPCM(workArea, 0x71);
    s_DataTable3* pDT3 = readDataTable3(gFLD_A5->getSaturnPtr(0x0608F0D4));
    setupField2(pDT3, fieldA5_A_startTasks);
    setupDragonPositionAndCamera_A5(gFLD_A5->getSaturnPtr(0x060898E4), gFLD_A5->getSaturnPtr(0x060898F0));
    setupCameraConfig_A5_corridor();
    getFieldTaskPtr()->m8_pSubFieldData->m334->m50E_followModeIndex = 2;
    initDragonParams_A5_corridor();
    createA5CorridorVdp2Task(workArea);
    getFieldTaskPtr()->m8_pSubFieldData->m344_randomBattleTask->m0 = (void(*)())&postBattleSound_A5_corridor;
}
