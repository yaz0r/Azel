#include "PDS.h"
#include "o_fld_a5.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldRadar.h"
#include "audio/soundDriver.h"

static void fieldA5_0_startTasks(p_workArea workArea)
{
    Unimplemented();
}

// 060558e8
static void setupFieldCameraConfig_A5_0()
{
    setupFieldCameraConfigs(readCameraConfig(gFLD_A5->getSaturnPtr(0x06089C34)), 1);
}

// 06054188
static void initDragonSpeed_A5()
{
    s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    pDragon->mC8_normalLightColor = { 0x16, 0x16, 0x13 };
    pDragon->mCB_falloffColor0 = { 6, 6, 8 };
    pDragon->mCE_falloffColor1 = { 0xF, 0xF, 0xB };
    pDragon->mD1_falloffColor2 = { 0xF, 0xF, 0xB };
    pDragon->mD4 = { 0x10, 0x10, 0x10 };
    pDragon->mC0_lightRotationAroundDragon = fixedPoint(0xC000000);
    pDragon->mC4 = 0;
}

// 060543a2
void subfieldA5_0(p_workArea workArea)
{
    playPCM(workArea, 0x66);
    loadFileFromFileList(1);

    s_DataTable3* pDataTable3 = readDataTable3(gFLD_A5->getSaturnPtr(0x0608E24C));
    setupField2(pDataTable3, fieldA5_0_startTasks);
    // TODO: A5-specific visibility processing from data at 0x0608E33C

    mainGameState.bitField[0xA3] &= 0xDF;

    setupFieldCameraConfig_A5_0();

    s16 prevSubField = getFieldTaskPtr()->m32_previousSubField;
    if (prevSubField == 1)
    {
        getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript(gFLD_A5->getSaturnPtr(0x0608974C));
    }
    else if (prevSubField == 5)
    {
        getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript(gFLD_A5->getSaturnPtr(0x06089780));
    }
    else if (prevSubField == 10)
    {
        Unimplemented(); // startCutscene
    }
    else
    {
        getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript(gFLD_A5->getSaturnPtr(0x06089718));
    }

    initFieldDragonLight();
    adjustVerticalLimits(0x5000, 0x82000);
    fieldRadar_enableAltitudeGauge();
    initDragonSpeed_A5();
    Unimplemented(); // TODO: A5-specific VDP2 palette task (FUN_FLD_A5__0605d492)

    getFieldTaskPtr()->m8_pSubFieldData->m344_randomBattleTask->m0 = nullBattle;

    Unimplemented(); // FUN_FLD_A5__0607a9a0(0x16) — random battle init with script 0x16
}
