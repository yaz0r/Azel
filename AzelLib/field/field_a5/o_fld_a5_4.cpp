#include "PDS.h"
#include "o_fld_a5.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldRadar.h"
#include "audio/soundDriver.h"

static void fieldA5_4_startTasks(p_workArea workArea) { Unimplemented(); }

// 06054744
void subfieldA5_4(p_workArea workArea)
{
    playPCM(workArea, 0x66);
    loadFileFromFileList(1);
    s_DataTable3* pDT3 = readDataTable3(gFLD_A5->getSaturnPtr(0x0609396C));
    setupField2(pDT3, fieldA5_4_startTasks);
    mainGameState.bitField[0xA3] &= 0xDF;
    setupCameraConfig_A5_4();
    if (getFieldTaskPtr()->m30_fieldEntryPoint < 0)
    {
        getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript(gFLD_A5->getSaturnPtr(0x06089A04));
    }
    else
    {
        getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript(gFLD_A5->getSaturnPtr(0x060898B0));
        mainGameState.bitField[0x59] |= 0x10;
    }
    initFieldDragonLight();
    adjustVerticalLimits(0x5000, 0x82000);
    fieldRadar_enableAltitudeGauge();
    initDragonParams_A5_open();
    createA5Vdp2Task(workArea);
    getFieldTaskPtr()->m8_pSubFieldData->m344_randomBattleTask->m0 = nullBattle;
    Unimplemented(); // random battle init 0x18
}
