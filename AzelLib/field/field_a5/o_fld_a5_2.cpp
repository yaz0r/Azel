#include "PDS.h"
#include "o_fld_a5.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldRadar.h"
#include "audio/soundDriver.h"

static void fieldA5_2_startTasks(p_workArea workArea) { Unimplemented(); }

// 06054588
void subfieldA5_2(p_workArea workArea)
{
    playPCM(workArea, 0x66);
    loadFileFromFileList(1);
    s_DataTable3* pDT3 = readDataTable3(gFLD_A5->getSaturnPtr(0x06091490));
    setupField2(pDT3, fieldA5_2_startTasks);
    mainGameState.bitField[0xA3] &= 0xDF;
    setupCameraConfig_A5_2();
    s16 entryPoint = getFieldTaskPtr()->m30_fieldEntryPoint;
    if (entryPoint < 0)
    {
        getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript(gFLD_A5->getSaturnPtr(0x060899EC));
    }
    else
    {
        s16 prev = getFieldTaskPtr()->m32_previousSubField;
        if (prev == 3)
            getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript(gFLD_A5->getSaturnPtr(0x06089818));
        else if (prev == 6)
            getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript(gFLD_A5->getSaturnPtr(0x0608984C));
        else
            getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript(gFLD_A5->getSaturnPtr(0x060897E4));
    }
    initFieldDragonLight();
    adjustVerticalLimits(0x5000, 0x82000);
    fieldRadar_enableAltitudeGauge();
    initDragonParams_A5_open();
    createA5Vdp2Task(workArea);
    getFieldTaskPtr()->m8_pSubFieldData->m344_randomBattleTask->m0 = nullBattle;
    Unimplemented(); // random battle init
}
