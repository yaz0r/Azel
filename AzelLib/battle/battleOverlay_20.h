#pragma once

struct s_battleOverlay_20 : public s_workAreaTemplate<s_battleOverlay_20>
{
    s8 m0;
    s16 m10;
    u16 m14_vdp1Memory;
    u16 m16;
    u16 m18;
    u16 m1A;
    u16 m1C;
    u16 m1E;
    u16 m20;
    u16 m22;
    u16 m24;
    s32 m28;
    u32 m2C;
    //size 0x68
};

void createBattleOverlay_task20(struct npcFileDeleter*);
