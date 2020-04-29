#pragma once

struct sFadeControlsChannel
{
    sVec3_FP m0_color;
    sVec3_FP mC_colorStep;
    sVec3_S16 m18_targetColor;
    s16 m1E_counter;
    u8 m20_stopped;
};

struct sFadeControls
{
    sFadeControlsChannel m0_fade0;

    sFadeControlsChannel m24_fade1;

    u16 m_48;
    u16 m_4A;
    u8 m_4C;
    u8 m_4D;
};

extern sFadeControls g_fadeControls;

u32 convertColorToU32ForFade(const sVec3_FP& inColor);
s32 fadePalette(sFadeControlsChannel* arrayData, u32 from, u32 to, u32 steps);
void updateFadeInterrupt();

