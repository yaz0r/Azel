#include "PDS.h"
#include "battleResultScreenProgressiveText.h"

struct sBattleResultProgressiveText : public s_workAreaTemplate<sBattleResultProgressiveText>
{
    sSaturnPtr m0_configEA;
    s16 m4_currentCounter;
    s16 m6_delayBetweenCharacters;
    s8 mD_positionInString;
    s8 mE_x;
    s8 mF_y;
    s8 m10_paletteId;
    s8 m11;
    s32 m14;
    // size 0x18
};

void battleResultScreenProgressiveText_printCharacter(sBattleResultProgressiveText* pThis)
{
    vdp2PrintStatus.m14_oldPalette = vdp2PrintStatus.m10_palette;
    vdp2PrintStatus.m10_palette = ((pThis->m10_paletteId) & 0xF) << 0xC;
    vdp2DebugPrintSetPosition(pThis->mE_x, pThis->mF_y);
    vdp2PrintfLargeFont("%c", readSaturnS8(readSaturnEA(pThis->m0_configEA + 0xC) + pThis->mD_positionInString));
    pThis->mE_x++;
    vdp2PrintStatus.m10_palette = vdp2PrintStatus.m14_oldPalette;
}

void battleResultScreenProgressiveText_printString(sSaturnPtr config)
{
    vdp2PrintStatus.m14_oldPalette = vdp2PrintStatus.m10_palette;
    vdp2PrintStatus.m10_palette = ((readSaturnS8(config + 8)) & 0xF) << 0xC;
    vdp2DebugPrintSetPosition(readSaturnS32(config + 0), readSaturnS32(config + 4));
    vdp2PrintfLargeFont("%s", readSaturnString(readSaturnEA(config + 0xC)).c_str());
    vdp2PrintStatus.m10_palette = vdp2PrintStatus.m14_oldPalette;
}

void battleResultScreenProgressiveText_update(sBattleResultProgressiveText* pThis)
{
    if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & 6) == 0)
    {
        pThis->m4_currentCounter++;
        if (pThis->m6_delayBetweenCharacters < pThis->m4_currentCounter)
        {
            pThis->m4_currentCounter = 0;
            if (pThis->m11)
            {
                // spawn particles
                Unimplemented();
            }
            battleResultScreenProgressiveText_printCharacter(pThis);
            pThis->mD_positionInString++;

            // if next character is null terminator
            if (readSaturnS8(readSaturnEA(pThis->m0_configEA + 0xC) + pThis->mD_positionInString) == 0)
            {
                if (pThis->m11 > 1)
                {
                    // spawn particles
                    Unimplemented();
                }
                pThis->getTask()->markFinished();
            }
        }
    }
    else
    {
        if (pThis->m14 == 0)
        {
            while (readSaturnS8(readSaturnEA(pThis->m0_configEA + 0xC) + pThis->mD_positionInString))
            {
                battleResultScreenProgressiveText_printCharacter(pThis);
                pThis->mD_positionInString++;
            }
        }
        else
        {
            battleResultScreenProgressiveText_printString(pThis->m0_configEA);
        }
        pThis->getTask()->markFinished();
    }
}

void battleResultScreen_createProgressiveText(p_workArea parent, sSaturnPtr param_2, int param_3, int param_4, int param_5)
{
    sBattleResultProgressiveText* pThis = createSiblingTaskFromFunction<sBattleResultProgressiveText>(parent, battleResultScreenProgressiveText_update);

    pThis->m4_currentCounter = param_3;
    pThis->m6_delayBetweenCharacters = param_3;
    pThis->m0_configEA = param_2;

    pThis->mE_x = readSaturnS8(param_2 + 3);
    pThis->mF_y = readSaturnS8(param_2 + 7);
    pThis->m10_paletteId = readSaturnS32(param_2 + 8);
    pThis->m11 = param_4;
    pThis->m14 = param_5;
}

