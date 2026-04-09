#include "PDS.h"
#include "o_fld_a5.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldRadar.h"
#include "audio/soundDriver.h"

// 0605aa10
static void fieldA5_8_startTasks(p_workArea workArea)
{
    createFieldSpecificDataTask_A5(workArea);
    createA5_3dSceneManager(workArea);
    createA5_baldorWormBody(workArea);
    createA5_wormObjectTask(workArea);
    createA5_exitEntityTask(workArea);
    createA5_triggerZones_night(workArea);
    // 0605efe0 — init worm color tables for subfield 8
    {
        s_fieldSpecificData_A5* pFieldData = (s_fieldSpecificData_A5*)getFieldTaskPtr()->mC;
        pFieldData->mC_wormColorTable0 = getSaturnPtr(gFLD_A5->getSaturnPtr(0x0609C640));
        pFieldData->m10_wormColorTable1 = getSaturnPtr(gFLD_A5->getSaturnPtr(0x0609C64C));
        pFieldData->m14_wormColorState0 = 0;
        pFieldData->m18_wormColorState1 = 0;
    }
    createA5_proximityAlert_night(workArea);
    createA5_wormSegments_night(workArea);
    createA5_corridorWorm_2(workArea);
    createA5_decorObjects_night_2();
    createA5_encounterConfig(workArea, 6, 0, 0x081F, 0x200, 0xA0, 0x40, 0x1F);
}

// 06054A88
void subfieldA5_8(p_workArea workArea)
{
    playPCM(workArea, 0x66);
    loadFileFromFileList(1);
    s_DataTable3* pDT3 = readDataTable3(gFLD_A5->getSaturnPtr(0x060959E8));
    setupField2(pDT3, fieldA5_8_startTasks);
    mainGameState.bitField[0xA3] |= 0x20;
    setupCameraConfig_A5_8();
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
    getFieldTaskPtr()->m8_pSubFieldData->m344_randomBattleTask->m0 = (void(*)())&postBattleSound_A5_nightA;
    fieldRadar_initRandomBattle(0x17);
}
