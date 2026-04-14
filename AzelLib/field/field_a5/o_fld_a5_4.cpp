#include "PDS.h"
#include "o_fld_a5.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldRadar.h"
#include "audio/soundDriver.h"

// 0605a816
static void fieldA5_4_startTasks(p_workArea workArea)
{
    createFieldSpecificDataTask_A5(workArea);
    createA5_3dSceneManager(workArea);
    createA5_envObjects_sub4_particle(workArea);
    createA5_envObjects_sub4_light(workArea);
    initA5_wormColorTables_day();
    createA5_envObjects_sub4_wormSegments(workArea);
    // 06059768 — empty
    createA5_wormObjectTask(workArea);
    createA5_wormDustTask(workArea);
    createA5_exitEntityTask(workArea);
    createA5_decorObjects_night_4();
    createA5_encounterConfig(workArea, 4, 0, 0x081F, 0x200, 0xA0, 0x40, 0x1F);
}

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
        setupDragonPositionAndCamera_A5(gFLD_A5->getSaturnPtr(0x06089A10), gFLD_A5->getSaturnPtr(0x06089A1C));
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
    getFieldTaskPtr()->m8_pSubFieldData->m344_randomBattleTask->m0 = (void(*)())&postBattleSound_A5_nightB;
    fieldRadar_initRandomBattle(0x18);
}
