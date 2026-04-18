#include "PDS.h"
#include "o_fld_b2.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldSceneManager.h"
#include "field/fieldItemBox.h"
#include "field/fieldItemBoxDefinition.h"
#include "audio/soundDriver.h"
#include "3dEngine.h"

// 0605fb5a — create item boxes for subfield 1 (oasis)
static void createB2ItemBoxes_1()
{
    createFieldItemBox(readItemBoxDefinition(gFLD_B2->getSaturnPtr(0x0608cd30)));
    createFieldItemBox(readItemBoxDefinition(gFLD_B2->getSaturnPtr(0x0608cd78)));
}

// 06059eaa — subfield 1 startTasks callback
static void b2_startTasks_1(p_workArea workArea)
{
    createFieldSpecificDataTask_B2(workArea);

    s_fieldSpecificData_B2* pFieldData = getFieldSpecificData_B2();
    pFieldData->m3C_sceneManager = createFieldSceneManager(workArea, 3, 0x40);

    Unimplemented(); // FUN_FLD_B2__060559ea — create env object (0x12C oasis plants task)
    createB2ItemBoxes_1();
    Unimplemented(); // FUN_FLD_B2__0605e2d2 — create ambient sound task (0x64)

    setSoundDistanceParams(0x200000, 0x6E);
}

// 06054412 — subfield 1 (oasis)
void subfieldB2_1(p_workArea workArea)
{
    s_fieldTaskWorkArea* pFieldTask = getFieldTaskPtr();
    s16 prevSubField = pFieldTask->m32_previousSubField;

    if (prevSubField == (s16)0xFFFF || prevSubField == 0 || prevSubField == 3)
    {
        loadSoundBanks(0x13, 0);
        pFieldTask = getFieldTaskPtr();
        playPCM(pFieldTask->m8_pSubFieldData, 100);
    }

    pFieldTask = getFieldTaskPtr();
    playPCM(pFieldTask->m8_pSubFieldData, 0x6C);

    mainGameState.bitField[0xA4] = mainGameState.bitField[0xA4] & 0xFE;

    initDragonParams_B2_oasis();

    pFieldTask = getFieldTaskPtr();
    if (prevSubField == 0 || prevSubField != 2)
    {
        sVec3_FP pos = readSaturnVec3(gFLD_B2->getSaturnPtr(0x06083900));
        sVec3_FP rot = readSaturnVec3(gFLD_B2->getSaturnPtr(0x0608390C));
        setupDragonPosition(&pos, &rot);

        if ((mainGameState.bitField[0x29] & 0x80) == 0)
        {
            s32 packedVal = readPackedBits(mainGameState.bitField, 0x588, 3);
            if (packedVal > 3)
            {
                mainGameState.bitField[0xA4] = mainGameState.bitField[0xA4] | 1;
                initDragonParams_B2_oasisStorm();
            }
        }
    }
    else
    {
        sVec3_FP pos = readSaturnVec3(gFLD_B2->getSaturnPtr(0x06083918));
        sVec3_FP rot = readSaturnVec3(gFLD_B2->getSaturnPtr(0x06083924));
        setupDragonPosition(&pos, &rot);
    }

    graphicEngineStatus.m405C.m10_nearClipDistance = 0x3000;
    graphicEngineStatus.m405C.m30_oneOverNearClip = FP_Div(0x10000, graphicEngineStatus.m405C.m10_nearClipDistance);
    graphicEngineStatus.m405C.m14_farClipDistance = 0x300000;
    graphicEngineStatus.m405C.m38_oneOverFarClip = FP_Div(0x8000, graphicEngineStatus.m405C.m14_farClipDistance);
    graphicEngineStatus.m405C.m34_oneOverFarClip256 = graphicEngineStatus.m405C.m38_oneOverFarClip << 8;

    setupB2CameraConfigs(
        gFLD_B2->getSaturnPtr(0x0608a138),
        gFLD_B2->getSaturnPtr(0x06087fb0),
        &b2_startTasks_1);

    createB2Vdp2Task_1(workArea);
    // 06056e84 is empty
    startFieldScript(0x11, -1);

    // 0x360 in subFieldData — debug making mode function (no member in C++ struct)
    // pFieldTask->m8_pSubFieldData->m360 = &b2_debugMakingMode;
}
