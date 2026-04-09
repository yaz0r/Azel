#include "PDS.h"
#include "o_fld_a7.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldRadar.h"
#include "audio/soundDriver.h"

// 06054268
void subfieldA7_1(p_workArea workArea)
{
    playPCM(workArea, 0x6F);

    // setupField2 with data table 3 at 06084204, no visibility
    s_DataTable3* pDT3 = readDataTable3(gFLD_A7->getSaturnPtr(0x06084204));
    setupField2(pDT3, startTasksA7_1);

    // Set dragon mF4 callback
    getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->mF4 = &dragonCallback_A7_1;

    // m360 = null function (060544bc)
    getFieldTaskPtr()->m8_pSubFieldData->pUpdateFunction3 = nullptr;

    // 060544a8 — camera config from 060802cc
    setupFieldCameraConfigs(readCameraConfig(gFLD_A7->getSaturnPtr(0x060802cc)), 1);

    fieldRadar_enableAltitudeGauge();
    adjustVerticalLimits(0x5000, 0x12C000);
    adjustHorizontalLimits(0, 0xC00000, 0xFF400000, 0);

    createA7_1_envTask(workArea);
    initDragonParams_A7_1();

    getFieldTaskPtr()->m8_pSubFieldData->m344_randomBattleTask->m0 = nullBattle;

    // Position/rotation data at 060801cc / 060801d8
    static const sVec3_FP pos = { fixedPoint(0x005FE000), fixedPoint(0x0005E000), fixedPoint(-0x832000) };
    static const sVec3_FP rot = { fixedPoint(0), fixedPoint(0x08000000), fixedPoint(0) };
    setupDragonPosition(&pos, &rot);

    s_fieldTaskWorkArea* pFieldTask = getFieldTaskPtr();
    if (pFieldTask->m32_previousSubField == (s16)0xFFFF)
    {
        startFieldScript(0x1E, -1);
    }
    else
    {
        startFieldScript(0x13, -1);
    }
}
