#pragma once

#include "PDS.h"

enum BGFXViewIndices
{
    view_vdp1Poly = 0,
};

void azelSdl2_Init();
void azelSdl2_StartFrame();
bool azelSdl2_EndFrame();
