#pragma once

#include "PDS.h"

enum bgfxViews
{
    VDP2_viewsStart = 1,
    VDP2_NGB0 = VDP2_viewsStart,
    VDP2_NGB1,
    VDP2_NGB2,
    VDP2_NGB3,
    VDP2_NGB4,
    VDP2_MAX = VDP2_NGB4,

    CompositeView,
    vdp1_gpuView,
};

void azelSdl2_Init();
void azelSdl2_StartFrame();
bool azelSdl2_EndFrame();
