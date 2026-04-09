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
    sSaturnPtr posData;
    if (prev == 4 || prev == 9)
        posData = gFLD_A5->getSaturnPtr(0x06089898);
    else
        posData = gFLD_A5->getSaturnPtr(0x06089880);
    {
        sVec3_FP pos = { readSaturnS32(posData), readSaturnS32(posData + 4), readSaturnS32(posData + 8) };
        sVec3_FP rot = { readSaturnS32(posData + 0xC), readSaturnS32(posData + 0x10), readSaturnS32(posData + 0x14) };
        setupDragonPosition(&pos, &rot);
    }
    initDragonParams_A5_corridor();
    createA5Vdp2Task(workArea);
    getFieldTaskPtr()->m8_pSubFieldData->m344_randomBattleTask->m0 = nullBattle;
}
