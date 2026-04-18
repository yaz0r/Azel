#include "PDS.h"
#include "o_fld_b2.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldItemBox.h"
#include "field/fieldItemBoxDefinition.h"
#include "audio/soundDriver.h"
#include "3dEngine.h"

// 0605fb6c — create item boxes for subfield 2 (underground)
static void createB2ItemBoxes_2()
{
    createFieldItemBox(readItemBoxDefinition(gFLD_B2->getSaturnPtr(0x0608cca0)));
    createFieldItemBox(readItemBoxDefinition(gFLD_B2->getSaturnPtr(0x0608cce8)));
}

// 06059eec — subfield 2 startTasks callback
static void b2_startTasks_2(p_workArea workArea)
{
    createFieldSpecificDataTask_B2(workArea);

    createB2ItemBoxes_2();
    Unimplemented(); // FUN_FLD_B2__060594a0 — create env object (0x40 underground task)
    Unimplemented(); // FUN_FLD_B2__0605e2d2 — create ambient sound task (0x64)

    setSoundDistanceParams(0x200000, 0x6E);
}

// 060545D0 — subfield 2 (underground)
void subfieldB2_2(p_workArea workArea)
{
    s_fieldTaskWorkArea* pFieldTask = getFieldTaskPtr();
    if (pFieldTask->m32_previousSubField == (s16)0xFFFF)
    {
        loadSoundBanks(0x13, 0);
        pFieldTask = getFieldTaskPtr();
        playPCM(pFieldTask->m8_pSubFieldData, 100);
    }

    graphicEngineStatus.m405C.m10_nearClipDistance = 0x3000;
    graphicEngineStatus.m405C.m30_oneOverNearClip = FP_Div(0x10000, graphicEngineStatus.m405C.m10_nearClipDistance);
    graphicEngineStatus.m405C.m14_farClipDistance = 0x400000;
    graphicEngineStatus.m405C.m38_oneOverFarClip = FP_Div(0x8000, graphicEngineStatus.m405C.m14_farClipDistance);
    graphicEngineStatus.m405C.m34_oneOverFarClip256 = graphicEngineStatus.m405C.m38_oneOverFarClip << 8;

    setupB2CameraConfigs(
        gFLD_B2->getSaturnPtr(0x0608a558),
        gFLD_B2->getSaturnPtr(0x0608a5e8),
        &b2_startTasks_2);

    adjustVerticalLimits(0x14000, 0x78000);
    createB2Vdp2Task_2(workArea);
    setupCameraVisibility_B2_underground();
    initDragonParams_B2_underground();
    initDragonSpeed(0);

    {
        sVec3_FP pos = readSaturnVec3(gFLD_B2->getSaturnPtr(0x06083930));
        sVec3_FP rot = readSaturnVec3(gFLD_B2->getSaturnPtr(0x0608393C));
        setupDragonPosition(&pos, &rot);
    }

    if ((mainGameState.bitField[0x76] & 4) == 0)
    {
        startFieldScript(7, 0x5EB);
    }
    else
    {
        startFieldScript(0x12, -1);
    }
}
