#include "PDS.h"
#include "o_fld_a5.h"
#include "field/field_a3/o_fld_a3.h"
#include "audio/soundDriver.h"

static void fieldA5_6_startTasks(p_workArea workArea) { Unimplemented(); }

// 0605489C
void subfieldA5_6(p_workArea workArea)
{
    playPCM(workArea, 0x71);
    s_DataTable3* pDT3 = readDataTable3(gFLD_A5->getSaturnPtr(0x0608F0B0));
    setupField2(pDT3, fieldA5_6_startTasks);
    setupCameraConfig_A5_corridor();
    getFieldTaskPtr()->m8_pSubFieldData->m334->m50E = 2;
    {
        sSaturnPtr posData = gFLD_A5->getSaturnPtr(0x060898F0);
        sVec3_FP pos = { readSaturnS32(posData), readSaturnS32(posData + 4), readSaturnS32(posData + 8) };
        sVec3_FP rot = { readSaturnS32(posData + 0xC), readSaturnS32(posData + 0x10), readSaturnS32(posData + 0x14) };
        setupDragonPosition(&pos, &rot);
    }
    initDragonParams_A5_corridor();
    createA5Vdp2Task(workArea);
    getFieldTaskPtr()->m8_pSubFieldData->m344_randomBattleTask->m0 = nullBattle;
}
