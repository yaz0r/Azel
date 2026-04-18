#include "PDS.h"
#include "o_fld_b2.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldSceneManager.h"
#include "field/fieldItemBox.h"
#include "field/fieldItemBoxDefinition.h"
#include "audio/soundDriver.h"

// 0605fb7e — create item boxes for subfield 3 (desert night)
static void createB2ItemBoxes_3()
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
    createFieldItemBox(readItemBoxDefinition(gFLD_B2->getSaturnPtr(0x0608cc10)));
}

// 06059f5c — subfield 3 startTasks callback
static void b2_startTasks_3(p_workArea workArea)
{
    createFieldSpecificDataTask_B2(workArea);

    s_fieldSpecificData_B2* pFieldData = getFieldSpecificData_B2();
    pFieldData->m3C_sceneManager = createFieldSceneManager(workArea, 2, 0x40);
    pFieldData->mA0_flag = 1;

    Unimplemented(); // FUN_FLD_B2__06056a96 — create item box manager task (0xB8C)
    Unimplemented(); // FUN_FLD_B2__0605e8c8 — create env object (0x190C terrain task)
    Unimplemented(); // FUN_FLD_B2__0605f2a2 — create env object (0x98C scenery task)
    // 0606de6c — encounter config (empty function)
    createB2ItemBoxes_3();
    Unimplemented(); // FUN_FLD_B2__0605e2d2 — create ambient sound task (0x64)

    setSoundDistanceParams(0x200000, 0x6E);
}

// 06054700 — subfield 3 (return desert)
void subfieldB2_3(p_workArea workArea)
{
    mainGameState.bitField[0xA5] = mainGameState.bitField[0xA5] & 0x3F;

    s_fieldTaskWorkArea* pFieldTask = getFieldTaskPtr();
    if (pFieldTask->m2C_currentFieldIndex == 6)
    {
        pFieldTask = getFieldTaskPtr();
        if (pFieldTask->m32_previousSubField == (s16)0xFFFF)
        {
            s32 result = startFieldScript(0, 0x5E4);
            if (result != 0)
            {
                sVec3_FP pos = readSaturnVec3(gFLD_B2->getSaturnPtr(0x06083948));
                sVec3_FP rot = readSaturnVec3(gFLD_B2->getSaturnPtr(0x06083954));
                setupDragonPosition(&pos, &rot);
                mainGameState.bitField[0xA5] = mainGameState.bitField[0xA5] | 0x80;
                mainGameState.bitField[0xF] = (mainGameState.bitField[0xF] & 0x7F) | 0x40;
            }
        }
    }

    loadSoundBanks(0x11, 0);

    pFieldTask = getFieldTaskPtr();
    if (pFieldTask->m32_previousSubField == (s16)0xFFFF)
    {
        s32 sndResult = findSound(100);
        if ((s8)sndResult < 0)
        {
            pFieldTask = getFieldTaskPtr();
            playPCM(pFieldTask->m8_pSubFieldData, 100);
        }
    }
    else
    {
        pFieldTask = getFieldTaskPtr();
        playPCM(pFieldTask->m8_pSubFieldData, 100);
    }

    setupB2CameraConfigs(
        gFLD_B2->getSaturnPtr(0x06087da4),
        gFLD_B2->getSaturnPtr(0x06086de8),
        &b2_startTasks_3);

    createB2Vdp2Task_3(workArea);
    setupCameraVisibility_B2_desert();
    enableDragonShadow();
    adjustVerticalLimits(0x5000, 0x10e000);
    adjustHorizontalLimits(0x4e0000, 0xb20000, (s32)0xff4e0000, (s32)0xffb20000);
    initDragonParams_B2_desert();

    pFieldTask = getFieldTaskPtr();
    if ((s16)pFieldTask->m30_fieldEntryPoint < 0)
    {
        sVec3_FP pos = readSaturnVec3(gFLD_B2->getSaturnPtr(0x06083960));
        sVec3_FP rot = readSaturnVec3(gFLD_B2->getSaturnPtr(0x06083960 + 0xC));
        setupDragonPosition(&pos, &rot);
    }
    else
    {
        pFieldTask = getFieldTaskPtr();
        s16 prevSubField = pFieldTask->m32_previousSubField;

        if (prevSubField == (s16)0xFFFF)
        {
            if ((mainGameState.bitField[0xA5] & 0x80) == 0)
            {
                sVec3_FP pos = readSaturnVec3(gFLD_B2->getSaturnPtr(0x06083948));
                sVec3_FP rot = readSaturnVec3(gFLD_B2->getSaturnPtr(0x06083954));
                setupDragonPosition(&pos, &rot);
                startFieldScript(0x10, -1);
                activateCameraScript_B2(gFLD_B2->getSaturnPtr(0x06083f00));
            }
        }
        else if (prevSubField == 1 || prevSubField == 2)
        {
            startFieldScript(0x10, -1);
            activateCameraScript_B2(gFLD_B2->getSaturnPtr(0x06083f68));
        }
    }

    pFieldTask = getFieldTaskPtr();
    *(void(**)())(pFieldTask->m8_pSubFieldData->m344_randomBattleTask) = &postBattleSound_B2_empty;
}
