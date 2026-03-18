#pragma once

#include "PDS.h"

enum bgfxViews
{
    VDP2_viewsStart = 1,
    VDP2_NGB0 = VDP2_viewsStart,
    VDP2_NGB1,
    VDP2_NGB2,
    VDP2_NGB3,
    VDP2_NGB4,          // RBG0 plane B (or single-plane RBG0)
    VDP2_RBG0_PlaneA,   // RBG0 plane A overlay (dual-plane mode)
    VDP2_MAX = VDP2_RBG0_PlaneA,

    vdp1_gpuView,
    CompositeView,
};

void azelSdl_Init();
void azelSdl_StartFrame();
bool azelSdl_EndFrame();

int computeSpritePriority(u16 cmdcolr, u16 cmdpmod);
