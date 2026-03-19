#include "PDS.h"
#include "o_fld_a5.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldRadar.h"
#include "audio/soundDriver.h"

static void fieldA5_7_startTasks(p_workArea workArea) { Unimplemented(); }

// 06054964
void subfieldA5_7(p_workArea workArea)
{
    playPCM(workArea, 0x66);
    loadFileFromFileList(1);
    s_DataTable3* pDT3 = readDataTable3(gFLD_A5->getSaturnPtr(0x06094490));
    setupField2(pDT3, fieldA5_7_startTasks);
    mainGameState.bitField[0xA3] |= 0x20;
    setupCameraConfig_A5_7();
    s16 prev = getFieldTaskPtr()->m32_previousSubField;
    switch (prev)
    {
    case 1: getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript(gFLD_A5->getSaturnPtr(0x0608974C)); break;
    case 5: getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript(gFLD_A5->getSaturnPtr(0x06089780)); break;
    case 10: Unimplemented(); break; // startCutscene
    default: getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript(gFLD_A5->getSaturnPtr(0x06089718)); break;
    }
    initFieldDragonLight();
    adjustVerticalLimits(0x5000, 0x82000);
    fieldRadar_enableAltitudeGauge();
    initDragonParams_A5_open();
    createA5Vdp2Task(workArea);
    getFieldTaskPtr()->m8_pSubFieldData->m344_randomBattleTask->m0 = nullBattle;
    Unimplemented(); // random battle init 0x16
}
