#include "PDS.h"
#include "battleResultScreenProgressiveTextNumber.h"

struct sBattleResultProgressiveTextNumber : public s_workAreaTemplate<sBattleResultProgressiveTextNumber>
{
    s32 m0_value;
    s8 m4;
    s8 m5;
    s8 m6;
    sSaturnPtr m8_config;
    // size 0xC
};

// compute screen position for integer display
void battleResultScreen_updateSub8(int value, sVec2_S32* output)
{
    int local_c[2];
    local_c[0] = value;
    local_c[1] = 1;

    while (local_c[0] >= 10)
    {
        local_c[0] = performDivision(10, local_c[0]);
        local_c[1]++;
    }

    (*output)[0] = local_c[0];
    (*output)[1] = local_c[1];
}

void battleResultScreenProgressiveText_printNumber(const std::array<s32, 4>& data, int value)
{
    vdp2PrintStatus.m14_oldPalette = vdp2PrintStatus.m10_palette;
    vdp2PrintStatus.m10_palette = (data[2] & 0xF) << 0xC;
    vdp2DebugPrintSetPosition(data[0], data[1]);

    switch (data[3])
    {
    case 1:
        vdp2PrintfLargeFont("+%2d", value);
        break;
    case 2:
        vdp2PrintfLargeFont("+%3d", value);
        break;
    case 3:
        vdp2PrintfLargeFont("%2d", value);
        break;
    case 8:
        vdp2PrintfLargeFont("%8d", value);
        break;
    default:
        assert(0);
    }

    vdp2PrintStatus.m10_palette = vdp2PrintStatus.m14_oldPalette;
}

void battleResultScreenProgressiveTextNumber_update(sBattleResultProgressiveTextNumber* pThis)
{
    Unimplemented();
    /*if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & 6) == 0)
    {

    }
    else*/
    {
        std::array<s32, 4> data;
        data[0] = readSaturnS32(pThis->m8_config + 0);
        data[1] = readSaturnS32(pThis->m8_config + 4);
        data[2] = readSaturnS32(pThis->m8_config + 8);
        data[3] = readSaturnS32(pThis->m8_config + 12);

        battleResultScreenProgressiveText_printNumber(data, pThis->m0_value);
        pThis->getTask()->markFinished();
    }
}

void battleResultScreen_createProgressiveTextNumber(p_workArea parent, sSaturnPtr config, int value)
{
    sBattleResultProgressiveTextNumber* pThis = createSiblingTaskFromFunction<sBattleResultProgressiveTextNumber>(parent, battleResultScreenProgressiveTextNumber_update);
    pThis->m8_config = config;
    
    sVec2_S32 size;
    battleResultScreen_updateSub8(value, &size);
    pThis->m6 = (readSaturnS8(config + 2) + readSaturnS8(config + 0xF)) - size[1];
    pThis->m5 = 2;
    pThis->m4 = readSaturnS8(config + 2) + readSaturnS8(config + 0xF);
    pThis->m0_value = value;
}
