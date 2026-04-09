#include "PDS.h"
#include "o_fld_a7.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldRadar.h"
#include "field/fieldVisibilityGrid.h"
#include "audio/soundDriver.h"

// 06054304
void subfieldA7_2(p_workArea workArea)
{
    playPCM(workArea, 0x65);
    loadFileFromFileList(4);

    s_visibilityGridWorkArea* pCameraTask = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;

    // setupField2 with no data table
    setupField2(nullptr, startTasksA7_2);

    pCameraTask->mC.m0_X = 0;
    pCameraTask->mC.m8_Z = 0;

    // Set dragon mF4 callback
    getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->mF4 = &dragonCallback_A7_2;

    fieldRadar_enableAltitudeGauge();
    adjustVerticalLimits(0, 0xB4000);
    adjustHorizontalLimits(fixedPoint(-0x400000), fixedPoint(0x400000), fixedPoint(-0x400000), fixedPoint(0x400000));

    createA7_2_envTask(workArea);

    getFieldTaskPtr()->m8_pSubFieldData->m344_randomBattleTask->m0 = nullBattle;

    startFieldScript(0x12, -1);
}
