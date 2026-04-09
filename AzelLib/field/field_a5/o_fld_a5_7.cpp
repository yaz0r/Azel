#include "PDS.h"
#include "o_fld_a5.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldRadar.h"
#include "audio/soundDriver.h"

// 0605a9ac
static void fieldA5_7_startTasks(p_workArea workArea)
{
    createFieldSpecificDataTask_A5(workArea);
    createA5_3dSceneManager(workArea);
    createA5_proximityAlert_day(workArea);
    createA5_wormSegments_day(workArea);
    // 0605971c — empty
    createA5_wormObjectTask(workArea);
    createA5_wormDustTask(workArea);
    createA5_exitEntityTask(workArea);
    Unimplemented(); // FUN_FLD_A5__060569ac — exit trigger entity via FUN_06056870 with {5,4,0x160,0xC0,8,0x164,0xC4}
    createA5_decorObjects_day();
    createA5_fieldEventCheck(workArea);
    createA5_encounterConfig(workArea, 5, 0, 0x081F, 0x200, 0xA0, 0x40, 0x1F);
}

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
    case 10: startCutscene(loadCutsceneData(gFLD_A5->getSaturnPtr(0x06098814))); break;
    default: getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript(gFLD_A5->getSaturnPtr(0x06089718)); break;
    }
    initFieldDragonLight();
    adjustVerticalLimits(0x5000, 0x82000);
    fieldRadar_enableAltitudeGauge();
    initDragonParams_A5_B();
    createA5NightVdp2Task(workArea);
    getFieldTaskPtr()->m8_pSubFieldData->m344_randomBattleTask->m0 = (void(*)())&postBattleSound_A5_day;
    fieldRadar_initRandomBattle(0x16);
}
