#include "PDS.h"
#include "o_fld_a5.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldRadar.h"
#include "audio/soundDriver.h"

static void fieldA5_9_startTasks(p_workArea workArea) { Unimplemented(); }

// 06054BBC
void subfieldA5_9(p_workArea workArea)
{
    playPCM(workArea, 0x66);
    loadFileFromFileList(1);
    s_DataTable3* pDT3 = readDataTable3(gFLD_A5->getSaturnPtr(0x06096D70));
    setupField2(pDT3, fieldA5_9_startTasks);
    mainGameState.bitField[0xA3] |= 0x20;
    setupCameraConfig_A5_9();
    s16 entryPoint = getFieldTaskPtr()->m30_fieldEntryPoint;
    if (entryPoint == -1)
    {
        getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript(gFLD_A5->getSaturnPtr(0x06089A04));
    }
    else if (entryPoint == 8)
    {
        Unimplemented(); // startCutscene
    }
    else
    {
        getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript(gFLD_A5->getSaturnPtr(0x060898B0));
    }
    initFieldDragonLight();
    adjustVerticalLimits(0x5000, 0x82000);
    fieldRadar_enableAltitudeGauge();
    initDragonParams_A5_open();
    Unimplemented(); // A5-specific VDP2 task (0605DCC4) — different from standard createA5Vdp2Task
    getFieldTaskPtr()->m8_pSubFieldData->m344_randomBattleTask->m0 = nullBattle;
    Unimplemented(); // random battle init 0x18
}
