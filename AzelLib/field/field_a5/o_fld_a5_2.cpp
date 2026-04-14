#include "PDS.h"
#include "o_fld_a5.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldRadar.h"
#include "audio/soundDriver.h"

// 0605a768
static void fieldA5_2_startTasks(p_workArea workArea)
{
    createFieldSpecificDataTask_A5(workArea);
    createA5_3dSceneManager(workArea);
    createA5_baldorWormBody(workArea);
    createA5_wormObjectTask(workArea);
    createA5_exitEntityTask(workArea);
    createA5_triggerZones_day(workArea);
    createA5_envObjects_sub2_sandfall(workArea);
    createA5_envObjects_sub2_skyEntity(workArea, gFLD_A5->getSaturnPtr(0x06099FF0));
    createA5_envObjects_sub2_weatherTask(workArea);
    initA5_wormColorTables_night();
    createA5_proximityAlert_night(workArea);
    createA5_wormSegments_night(workArea);
    createA5_corridorWorm_2(workArea);
    createA5_decorObjects_night_2();
    createA5_encounterConfig(workArea, 3, 0, 0x081F, 0x200, 0xA0, 0x40, 0x1F);
}

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
        setupDragonPositionAndCamera_A5(gFLD_A5->getSaturnPtr(0x060899F8), gFLD_A5->getSaturnPtr(0x06089A04));
    }
    else
    {
        s16 prev = getFieldTaskPtr()->m32_previousSubField;
        if (prev == 3)
            getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript(gFLD_A5->getSaturnPtr(0x06089818));
        else if (prev == 6)
            getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript(gFLD_A5->getSaturnPtr(0x0608984C));
        else
        {
            mainGameState.bitField[0x59] |= 0x80;
            getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript(gFLD_A5->getSaturnPtr(0x060897E4));
        }
    }
    initFieldDragonLight();
    adjustVerticalLimits(0x14000, 0x82000);
    fieldRadar_enableAltitudeGauge();
    initDragonParams_A5_open();
    createA5Vdp2Task(workArea);
    getFieldTaskPtr()->m8_pSubFieldData->m344_randomBattleTask->m0 = (void(*)())&postBattleSound_A5_nightA;
    fieldRadar_initRandomBattle(0x17);
}
