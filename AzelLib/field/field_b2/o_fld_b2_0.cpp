#include "PDS.h"
#include "o_fld_b2.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldSceneManager.h"
#include "field/fieldItemBox.h"
#include "field/fieldItemBoxDefinition.h"
#include "audio/soundDriver.h"

// 0605fb0c — create item boxes for subfield 0 (desert day)
static void createB2ItemBoxes_0()
{
    createFieldItemBox(readItemBoxDefinition(gFLD_B2->getSaturnPtr(0x0608c940)));
    createFieldItemBox(readItemBoxDefinition(gFLD_B2->getSaturnPtr(0x0608c988)));
    createFieldItemBox(readItemBoxDefinition(gFLD_B2->getSaturnPtr(0x0608c9d0)));
    createFieldItemBox(readItemBoxDefinition(gFLD_B2->getSaturnPtr(0x0608ca18)));
    createFieldItemBox(readItemBoxDefinition(gFLD_B2->getSaturnPtr(0x0608ca60)));
    createFieldItemBox(readItemBoxDefinition(gFLD_B2->getSaturnPtr(0x0608caa8)));
    createFieldItemBox(readItemBoxDefinition(gFLD_B2->getSaturnPtr(0x0608caf0)));
    createFieldItemBox(readItemBoxDefinition(gFLD_B2->getSaturnPtr(0x0608cb38)));
    createFieldItemBox(readItemBoxDefinition(gFLD_B2->getSaturnPtr(0x0608cb80)));
    createFieldItemBox(readItemBoxDefinition(gFLD_B2->getSaturnPtr(0x0608cbc8)));
    createFieldItemBox(readItemBoxDefinition(gFLD_B2->getSaturnPtr(0x0608cc58)));
}

// 06059e34 — subfield 0 startTasks callback
static void b2_startTasks_0(p_workArea workArea)
{
    createFieldSpecificDataTask_B2(workArea);

    s_fieldSpecificData_B2* pFieldData = getFieldSpecificData_B2();
    pFieldData->m3C_sceneManager = createFieldSceneManager(workArea, 2, 0x40);

    Unimplemented(); // FUN_FLD_B2__06060d36 — create 0x8C model render context task (stored at mC->m40)
    pFieldData->mA0_flag = 1;

    Unimplemented(); // FUN_FLD_B2__06056a96 — create item box manager task (0xB8C)
    Unimplemented(); // FUN_FLD_B2__0605e8c8 — create env object (0x190C terrain task)
    Unimplemented(); // FUN_FLD_B2__0605f2a2 — create env object (0x98C scenery task)
    // 0606de6c — encounter config (empty function)
    Unimplemented(); // FUN_FLD_B2__0605d92a — create env object (0x4014 desert ruins task)
    createB2ItemBoxes_0();
    Unimplemented(); // FUN_FLD_B2__0605e2d2 — create ambient sound task (0x64)

    setSoundDistanceParams(0x200000, 0x6E);
}

// 06054280 — subfield 0 (desert)
void subfieldB2_0(p_workArea workArea)
{
    mainGameState.bitField[0xA5] = (mainGameState.bitField[0xA5] & 0x7F) | 0x40;

    loadSoundBanks(0x12, 0);

    s_fieldTaskWorkArea* pFieldTask = getFieldTaskPtr();
    playPCM(pFieldTask->m8_pSubFieldData, 100);
    pFieldTask = getFieldTaskPtr();
    playPCM(pFieldTask->m8_pSubFieldData, 0x65);

    setupB2CameraConfigs(
        gFLD_B2->getSaturnPtr(0x06087da4),
        gFLD_B2->getSaturnPtr(0x06086de8),
        &b2_startTasks_0);

    createB2Vdp2Task_0(workArea);
    setupCameraVisibility_B2_desert();
    enableDragonShadow();
    adjustVerticalLimits(0x5000, 0x136000);
    adjustHorizontalLimits(0x4e0000, 0xb20000, (s32)0xff4e0000, (s32)0xffb20000);
    initDragonParams_B2_desert();

    pFieldTask = getFieldTaskPtr();
    *(void(**)())(pFieldTask->m8_pSubFieldData->m344_randomBattleTask) = &postBattleSound_B2_empty;

    pFieldTask = getFieldTaskPtr();
    if ((s16)pFieldTask->m30_fieldEntryPoint < 0)
    {
        sVec3_FP pos = readSaturnVec3(gFLD_B2->getSaturnPtr(0x06083960));
        sVec3_FP rot = readSaturnVec3(gFLD_B2->getSaturnPtr(0x06083960 + 0xC));
        setupDragonPosition(&pos, &rot);
        return;
    }

    pFieldTask = getFieldTaskPtr();
    s32 prevSubField = (s32)(s16)pFieldTask->m32_previousSubField;

    if (prevSubField == -1)
    {
        if ((mainGameState.bitField[5] & 8) == 0)
        {
            startFieldScript(0xF, -1);
        }
        else
        {
            s32 result = startFieldScript(0xB, 0x5EF);
            if (result == 0)
            {
                startFieldScript(0x10, -1);
                activateCameraScript_B2(gFLD_B2->getSaturnPtr(0x06083f00));
                return;
            }
        }
    }
    else if (prevSubField == 1 || prevSubField == 2)
    {
        startFieldScript(0x10, -1);
        activateCameraScript_B2(gFLD_B2->getSaturnPtr(0x06083f68));
        return;
    }
}
