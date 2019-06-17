#include "PDS.h"
#include "fade.h"

sFadeControls g_fadeControls;

u32 convertColorToU32(const sVec3_FP& inColor)
{
    u32 color = (((s32)((inColor[2].m_value >> 4) << 16)) + 0x10) << 10;
    color |= (((s32)((inColor[1].m_value >> 4) << 16)) + 0x10) << 5;
    color |= (((s32)((inColor[0].m_value >> 4) << 16)) + 0x10) << 0;

    return color | 0x8000;
}

s16 unpackColor(s16 inColor)
{
    s32 r4 = (inColor & 0x1F) - 0x10;
    if (r4 > 0)
    {
        s32 r3 = r4;
        r4 <<= 4;
        r4 += r3;
    }
    else
    {
        r4 <<= 4;
    }

    return r4;
}

s32 fadePalette(sFadeControlsChannel* pFadeChannel, u32 from, u32 to, u32 steps)
{
    if (g_fadeControls.m_4D < g_fadeControls.m_4C)
        return 0;

    g_fadeControls.m_4C = g_fadeControls.m_4D;

    pFadeChannel->m0_color[0] = unpackColor(from) << 16;
    pFadeChannel->m18_targetColor[0] = unpackColor(to);
    asyncDivStart_integer(pFadeChannel->m0_color[0] - (((s32)pFadeChannel->m18_targetColor[0]) << 16), steps);
    pFadeChannel->mC_colorStep[0] = asyncDivEnd();

    pFadeChannel->m0_color[1] = unpackColor(from >> 5) << 16;
    pFadeChannel->m18_targetColor[1] = unpackColor(to >> 5);
    asyncDivStart_integer(pFadeChannel->m0_color[1] - (((s32)pFadeChannel->m18_targetColor[1]) << 16), steps);
    pFadeChannel->mC_colorStep[1] = asyncDivEnd();

    pFadeChannel->m0_color[2] = unpackColor(from >> 10) << 16;
    pFadeChannel->m18_targetColor[2] = unpackColor(to >> 10);
    asyncDivStart_integer(pFadeChannel->m0_color[2] - (((s32)pFadeChannel->m18_targetColor[2]) << 16), steps);
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

            vdp2Controls.m20_registers[0].COAR = fadeControl.m0_color[0].getInteger();
            vdp2Controls.m20_registers[0].COAG = fadeControl.m0_color[1].getInteger();
            vdp2Controls.m20_registers[0].COAB = fadeControl.m0_color[2].getInteger();
        }
        else
        {
            vdp2Controls.m20_registers[0].COAR = fadeControl.m18_targetColor[0];
            vdp2Controls.m20_registers[0].COAG = fadeControl.m18_targetColor[1];
            vdp2Controls.m20_registers[0].COAB = fadeControl.m18_targetColor[2];

            vdp2Controls.m20_registers[1].COAR = fadeControl.m18_targetColor[0];
            vdp2Controls.m20_registers[1].COAG = fadeControl.m18_targetColor[1];
            vdp2Controls.m20_registers[1].COAB = fadeControl.m18_targetColor[2];

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

            vdp2Controls.m20_registers[0].COBR = fadeControl.m0_color[0].getInteger();
            vdp2Controls.m20_registers[0].COBG = fadeControl.m0_color[1].getInteger();
            vdp2Controls.m20_registers[0].COBB = fadeControl.m0_color[2].getInteger();
        }
        else
        {
            vdp2Controls.m20_registers[0].COBR = fadeControl.m18_targetColor[0];
            vdp2Controls.m20_registers[0].COBG = fadeControl.m18_targetColor[1];
            vdp2Controls.m20_registers[0].COBB = fadeControl.m18_targetColor[2];

            vdp2Controls.m20_registers[1].COBR = fadeControl.m18_targetColor[0];
            vdp2Controls.m20_registers[1].COBG = fadeControl.m18_targetColor[1];
            vdp2Controls.m20_registers[1].COBB = fadeControl.m18_targetColor[2];

            fadeControl.m0_color = fadeControl.m18_targetColor.toSVec3_FP();
            fadeControl.m20_stopped = 1;
        }
    }
}
