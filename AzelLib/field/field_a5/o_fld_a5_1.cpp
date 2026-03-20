#include "PDS.h"
#include "o_fld_a5.h"
#include "field/field_a3/o_fld_a3.h"
#include "audio/soundDriver.h"

static void fieldA5_1_startTasks(p_workArea workArea) { Unimplemented(); }

// 060544AC
void subfieldA5_1(p_workArea workArea)
{
    playPCM(workArea, 0x71);
    s_DataTable3* pDT3 = readDataTable3(gFLD_A5->getSaturnPtr(0x0608F044));
    setupField2(pDT3, fieldA5_1_startTasks);
    setupCameraConfig_A5_corridor();
    getFieldTaskPtr()->m8_pSubFieldData->m334->m50E_followModeIndex = 2;
    // setupDragonPosition with position + rotation from overlay data
    s16 prev = getFieldTaskPtr()->m32_previousSubField;
    sSaturnPtr posData;
    if (prev == 2 || prev == 8)
        posData = gFLD_A5->getSaturnPtr(0x060897CC);
    else
        posData = gFLD_A5->getSaturnPtr(0x060897B4);
    {
        sVec3_FP pos = { readSaturnS32(posData), readSaturnS32(posData + 4), readSaturnS32(posData + 8) };
        sVec3_FP rot = { readSaturnS32(posData + 0xC), readSaturnS32(posData + 0x10), readSaturnS32(posData + 0x14) };
        setupDragonPosition(&pos, &rot);
    }
    initDragonParams_A5_corridor();
    createA5Vdp2Task(workArea);
    getFieldTaskPtr()->m8_pSubFieldData->m344_randomBattleTask->m0 = nullBattle;
}
