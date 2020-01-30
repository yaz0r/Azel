#pragma once

#include "battleHud.h"

struct battleHud2 : public s_workAreaTemplate<battleHud2>
{
    u32 m0_vdp1Memory;
    u32 m4;
    u16 mC;
    u16 mE_offsetY;
    u16 m12_mode;
    u16 m14_nextMode;
    sVec2_S16 m16_dragonIconPosition;
    s16 m1A;
    s16 m1C;
    s_battleOverlay_20_sub m20;
    //size 0x3C
};

void battleEngine_CreateHud2(struct npcFileDeleter*);
