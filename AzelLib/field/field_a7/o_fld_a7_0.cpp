#include "PDS.h"
#include "o_fld_a7.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldRadar.h"
#include "audio/soundDriver.h"

// 060540e8
void subfieldA7_0(p_workArea workArea)
{
    playPCM(workArea, 100);
    playPCM(workArea, 0x6C);

    s_fieldTaskWorkArea* pFieldTask = getFieldTaskPtr();
    if (pFieldTask->m2C_currentFieldIndex == 0x16)
    {
        // Position/rotation data at 060801e4 / 060801f0
        static const sVec3_FP pos_0x16 = { fixedPoint(0x005FF000), fixedPoint(0x0002F000), fixedPoint(-0x254000) };
        static const sVec3_FP rot_0x16 = { fixedPoint(0), fixedPoint(0x005B05B0), fixedPoint(0) };
        setupDragonPosition(&pos_0x16, &rot_0x16);
    }
    else
    {
        pFieldTask = getFieldTaskPtr();
        if (pFieldTask->m32_previousSubField == (s16)0xFFFF)
        {
            // Position/rotation data at 060801b4 / 060801c0
            static const sVec3_FP pos_default = { fixedPoint(0x00600000), fixedPoint(0x000A0000), fixedPoint(-0x1CC000) };
            static const sVec3_FP rot_default = { fixedPoint(0), fixedPoint(0), fixedPoint(0) };
            setupDragonPosition(&pos_default, &rot_default);
        }
        else
        {
            // Camera script from 060800d0
            getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript(gFLD_A7->getSaturnPtr(0x060800d0));
        }
        startFieldScript(0x1D, -1);
    }

    // setupField with data table 3 at 06083584 and data table 2 at 060824e8
    s_DataTable3* pDT3 = readDataTable3(gFLD_A7->getSaturnPtr(0x06083584));
    s_DataTable2* pDT2 = readDataTable2(gFLD_A7->getSaturnPtr(0x060824e8));
    setupField(pDT3, pDT2, startTasksA7_0, nullptr);

    // 06072fc8 — enable dragon ground shadow
    enableDragonShadow();

    // 060544a0 — camera config from 06080274
    setupFieldCameraConfigs(readCameraConfig(gFLD_A7->getSaturnPtr(0x06080274)), 1);

    fieldRadar_enableAltitudeGauge();
    adjustVerticalLimits(0x5000, 0x136000);
    adjustHorizontalLimits(0, 0xC00000, 0xFF400000, 0);

    createA7_0_envTask(workArea);

    pFieldTask = getFieldTaskPtr();
    if (pFieldTask->m2C_currentFieldIndex == 0x16)
    {
        initDragonParams_A7_2();
    }
    else
    {
        initDragonParams_A7_0();
    }

    getFieldTaskPtr()->m8_pSubFieldData->m344_randomBattleTask->m0 = nullBattle;

    fieldRadar_initDestinations(0x1C);
}
