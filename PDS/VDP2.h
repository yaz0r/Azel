#pragma once

struct s_Vdp2PrintStatus
{
    s32 X;
    s32 Y;
    u16 field_10;
};
extern s_Vdp2PrintStatus vdp2DebugPrintPosition;

struct s_VDP2Regs
{
    u32 TVSTAT;
};
extern s_VDP2Regs VDP2Regs_;

void vdp2DebugPrintSetPosition(s32 x, s32 y);
void clearVdp2Text();
int renderVdp2String(char* text);
