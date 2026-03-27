#include "PDS.h"
#include "battleStart.h"
#include "audio/soundDriver.h"
#include "audio/systemSounds.h"
#include "field/field_a3/o_fld_a3.h"

// 060684cc
void battleLoading_Init(sBattleLoadingTask* pThis, s32 enemyId)
{
    getFieldTaskPtr()->m8_pSubFieldData->m344_randomBattleTask->m5 = 1;
    pThis->m0_enemyId = enemyId;
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

// 0606853a
void battleLoading_Draw(sBattleLoadingTask* pThis)
{
    switch (pThis->m8_status)
    {
    case 0:
        pThis->m8_status++;
        break;
    case 1:
    {
        s8 soundBank = readSaturnS8(gFLD_A3->getSaturnPtr(0x6093888) + getFieldTaskPtr()->m2C_currentFieldIndex * 2);
        if (soundBank > -1)
        {
            loadSoundBanks(soundBank, 0);
        }
        pThis->m8_status++;
        break;
    }
    case 2:
        if (isSoundLoadingFinished())
        {
            s8 soundBank = readSaturnS8(gFLD_A3->getSaturnPtr(0x6093889) + getFieldTaskPtr()->m2C_currentFieldIndex * 2);
            if (soundBank > -1)
            {
                playPCM(pThis, soundBank);
            }
            pThis->m8_status++;
        }
        break;
    case 3:
        if (readKeyboardToggle(0xDA)) // debug key — set game bit and advance
        {
            if (pThis->m4 >= 0)
            {
                u32 bitIndex = pThis->m4;
                u32 byteIndex = bitIndex;
                if (bitIndex > 999)
                {
                    byteIndex = bitIndex - 0x236;
                }
                if (bitIndex > 999)
                {
                    bitIndex = bitIndex - 0x236;
                }
                mainGameState.bitField[byteIndex >> 3] |= bitMasks[bitIndex & 7];
            }
            pThis->m8_status++;
        }
        if (readKeyboardToggle(0xF6)) // debug key — skip and advance
        {
            pThis->m8_status++;
        }
        break;
    case 4:
        popSoundSequence(1);
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

    // debugSound(); // debug sound status display (not needed for gameplay)
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
