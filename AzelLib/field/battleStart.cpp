#include "PDS.h"
#include "battleStart.h"
#include "audio/soundDriver.h"
#include "audio/systemSounds.h"
#include "field/field_a3/o_fld_a3.h"

void battleLoading_Init(sBattleLoadingTask* pThis)
{
    getFieldTaskPtr()->m8_pSubFieldData->m344_randomBattleTask->m5 = 1;
    pThis->m0_enemyId = 0; // TODO: figure that out. The original code expects an argument, but never pass one
    battleLoading_InitSub0();
    playSystemSoundEffect(0x10); // play "enter battle" sound

    getFieldTaskPtr()->m4_overlayTaskData->getTask()->markPaused(); // pause field

    if (g_fadeControls.m_4C < g_fadeControls.m_4D)
    {
        vdp2Controls.m20_registers[0].m112_CLOFSL = 8;
        vdp2Controls.m20_registers[1].m112_CLOFSL = 8;
    }

    fadePalette(&g_fadeControls.m0_fade0, convertColorToU32ForFade(g_fadeControls.m0_fade0.m0_color), 0xFC1F, 0x1E);
}

void battleLoading_Draw(sBattleLoadingTask* pThis)
{
    switch (pThis->m8_status)
    {
    case 0:
        pThis->m8_status++;
        break;
    case 1:
        if (readSaturnS8(gFLD_A3->getSaturnPtr(0x6093888) + getFieldTaskPtr()->m2C_currentFieldIndex * 2) > -1)
        {
            loadSoundBanks(readSaturnS8(gFLD_A3->getSaturnPtr(0x6093888) + getFieldTaskPtr()->m2C_currentFieldIndex * 2), 0);
        }
        pThis->m8_status++;
        break;
    case 2:
        if (isSoundLoadingFinished())
        {
            if (readSaturnS8(gFLD_A3->getSaturnPtr(0x6093888) + getFieldTaskPtr()->m2C_currentFieldIndex * 2) > -1)
            {
                playPCM(pThis, readSaturnS8(gFLD_A3->getSaturnPtr(0x6093888) + getFieldTaskPtr()->m2C_currentFieldIndex * 2));
            }
            pThis->m8_status++;
        }
        break;
    case 3:
        if (readKeyboardToggle(0xDA))
        {
            assert(0);
        }
        if (readKeyboardToggle(0xF6))
        {
            pThis->m8_status++;
        }
        break;
    case 4:
        Unimplemented();
        pThis->m8_status++;
        break;
    case 5:
        if (isSoundLoadingFinished())
        {
            pThis->getTask()->markFinished();
        }
        break;
    default:
        assert(0);
    }

    Unimplemented();
    //debugSound(0);
    vdp2DebugPrintSetPosition(3, 0x19);
    vdp2PrintfLargeFont("ENEMY:%2d ", pThis->m0_enemyId);
}

void battleLoading_Delete(sBattleLoadingTask* pThis)
{
    getFieldTaskPtr()->m4_overlayTaskData->getTask()->clearPaused();
    getFieldTaskPtr()->m8_pSubFieldData->m344_randomBattleTask->m5 = 2;
    if (g_fadeControls.m_4C <= g_fadeControls.m_4D) {
        vdp2Controls.m20_registers[1].m112_CLOFSL = 8;
        vdp2Controls.m20_registers[0].m112_CLOFSL = 8;
    }
    fadePalette(&g_fadeControls.m0_fade0, convertColorToU32ForFade(g_fadeControls.m0_fade0.m0_color), g_fadeControls.m_48, 0x1e);
    vdp2DebugPrintSetPosition(3, 0x19);
    clearVdp2TextLargeFont();
}

const sBattleLoadingTask::TypedTaskDefinition battleStartTaskDefinition =
{
    battleLoading_Init,
    nullptr,
    battleLoading_Draw,
    battleLoading_Delete,
};
