#include "PDS.h"
#include "o_fld_a5.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldRadar.h"
#include "audio/soundDriver.h"

// 0605aa74
static void fieldA5_9_startTasks(p_workArea workArea)
{
    createFieldSpecificDataTask_A5(workArea);
    createA5_3dSceneManager(workArea);
    Unimplemented(); // FUN_FLD_A5__0605ec60 — 3D scenery entity (size 0x58, getMemoryArea(7), pos {0x480000, 0x5000, -0xA00000})
    // 0605f00e — init worm color tables for subfield 9
    {
        s_fieldSpecificData_A5* pFieldData = (s_fieldSpecificData_A5*)getFieldTaskPtr()->mC;
        pFieldData->mC_wormColorTable0 = getSaturnPtr(gFLD_A5->getSaturnPtr(0x0609C658));
        pFieldData->m10_wormColorTable1 = getSaturnPtr(gFLD_A5->getSaturnPtr(0x0609C664));
        pFieldData->m14_wormColorState0 = 0;
        pFieldData->m18_wormColorState1 = 0;
    }
    createA5_envObjects_sub4_wormSegments(workArea); // 060570ea — same worm segments as sub4 (data 06098CD0)
    // 06059768 — empty
    createA5_wormObjectTask(workArea);
    createA5_wormDustTask(workArea);
    createA5_exitEntityTask(workArea);
    createA5_decorObjects_night_4();
    createA5_encounterConfig(workArea, 7, 0, 0x081F, 0x200, 0xA0, 0x40, 0x1F);
}

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
        startCutscene(loadCutsceneData(gFLD_A5->getSaturnPtr(0x060899ec)));
    }
    else
    {
        getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript(gFLD_A5->getSaturnPtr(0x060898B0));
    }
    initFieldDragonLight();
    adjustVerticalLimits(0x5000, 0x82000);
    fieldRadar_enableAltitudeGauge();
    initDragonParams_A5_B();
    createA5NightVdp2Task(workArea);
    getFieldTaskPtr()->m8_pSubFieldData->m344_randomBattleTask->m0 = (void(*)())&postBattleSound_A5_nightB;
    fieldRadar_initRandomBattle(0x18);
}
