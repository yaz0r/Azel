#include "PDS.h"
#include "fade.h"

sFadeControls g_fadeControls;

u32 convertColorToU32ForFade(const sVec3_FP& inColor)
{
    u32 color = ((inColor[0].toInteger() >> 4) + 0x10) << 10;
    color |= ((inColor[1].toInteger() >> 4) + 0x10) << 5;
    color |= ((inColor[2].toInteger() >> 4) + 0x10);
    color |= 0x8000;
    return color;
}

s16 unpackColor(s16 inColor)
{
    s32 r4 = (inColor & 0x1F) - 0x10;
    if (r4 > 0)
    {
        r4 *= 9;
    }
    else
    {
        r4 *= 8;
    }

    return r4;
}

s32 fadePalette(sFadeControlsChannel* pFadeChannel, u32 from, u32 to, u32 steps)
{
    if (g_fadeControls.m_4D < g_fadeControls.m_4C)
        return 0;

    g_fadeControls.m_4C = g_fadeControls.m_4D;

    pFadeChannel->m0_color[0].setFromInteger(unpackColor(from));
    pFadeChannel->m18_targetColor[0] = unpackColor(to);
    asyncDivStart_integer(pFadeChannel->m0_color[0] - pFadeChannel->m18_targetColor[0].toFP32(), steps);
    pFadeChannel->mC_colorStep[0] = asyncDivEnd();

    pFadeChannel->m0_color[1].setFromInteger(unpackColor(from >> 5));
    pFadeChannel->m18_targetColor[1] = unpackColor(to >> 5);
    asyncDivStart_integer(pFadeChannel->m0_color[1] - pFadeChannel->m18_targetColor[1].toFP32(), steps);
    pFadeChannel->mC_colorStep[1] = asyncDivEnd();

    pFadeChannel->m0_color[2].setFromInteger(unpackColor(from >> 10));
    pFadeChannel->m18_targetColor[2] = unpackColor(to >> 10);
    asyncDivStart_integer(pFadeChannel->m0_color[2] - pFadeChannel->m18_targetColor[2].toFP32(), steps);
    pFadeChannel->mC_colorStep[2] = asyncDivEnd();

    pFadeChannel->m1E_counter = steps;
    pFadeChannel->m20_stopped = 0;
    return 1;
}

void updateFadeInterrupt()
{
    if (!g_fadeControls.m0_fade0.m20_stopped)
    {
        sFadeControlsChannel& fadeControl = g_fadeControls.m0_fade0;
        if (fadeControl.m1E_counter--)
        {
            fadeControl.m0_color += fadeControl.mC_colorStep;

            vdp2Controls.m20_registers[0].m114_COAR = fadeControl.m0_color[0].getInteger();
            vdp2Controls.m20_registers[0].m116_COAG = fadeControl.m0_color[1].getInteger();
            vdp2Controls.m20_registers[0].m118_COAB = fadeControl.m0_color[2].getInteger();
        }
        else
        {
            vdp2Controls.m20_registers[0].m114_COAR = fadeControl.m18_targetColor[0];
            vdp2Controls.m20_registers[0].m116_COAG = fadeControl.m18_targetColor[1];
            vdp2Controls.m20_registers[0].m118_COAB = fadeControl.m18_targetColor[2];

            vdp2Controls.m20_registers[1].m114_COAR = fadeControl.m18_targetColor[0];
            vdp2Controls.m20_registers[1].m116_COAG = fadeControl.m18_targetColor[1];
            vdp2Controls.m20_registers[1].m118_COAB = fadeControl.m18_targetColor[2];

            fadeControl.m0_color = fadeControl.m18_targetColor.toSVec3_FP();
            fadeControl.m20_stopped = 1;
        }
    }

    if (!g_fadeControls.m24_fade1.m20_stopped)
    {
        sFadeControlsChannel& fadeControl = g_fadeControls.m24_fade1;
        if (fadeControl.m1E_counter--)
        {
            fadeControl.m0_color += fadeControl.mC_colorStep;

            vdp2Controls.m20_registers[0].m11A_COBR = fadeControl.m0_color[0].getInteger();
            vdp2Controls.m20_registers[0].m11C_COBG = fadeControl.m0_color[1].getInteger();
            vdp2Controls.m20_registers[0].m11E_COBB = fadeControl.m0_color[2].getInteger();
        }
        else
        {
            vdp2Controls.m20_registers[0].m11A_COBR = fadeControl.m18_targetColor[0];
            vdp2Controls.m20_registers[0].m11C_COBG = fadeControl.m18_targetColor[1];
            vdp2Controls.m20_registers[0].m11E_COBB = fadeControl.m18_targetColor[2];

            vdp2Controls.m20_registers[1].m11A_COBR = fadeControl.m18_targetColor[0];
            vdp2Controls.m20_registers[1].m11C_COBG = fadeControl.m18_targetColor[1];
            vdp2Controls.m20_registers[1].m11E_COBB = fadeControl.m18_targetColor[2];

            fadeControl.m0_color = fadeControl.m18_targetColor.toSVec3_FP();
            fadeControl.m20_stopped = 1;
        }
    }
}
