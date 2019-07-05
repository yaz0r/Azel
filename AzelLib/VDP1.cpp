#include "PDS.h"

u8 VDP1Vram[0x80000];

void setVdp1VramU16(u32 EA, u16 value)
{
    u32 offset = EA - 0x25C00000;
    *(u16*)(VDP1Vram + offset) = value;
}

void setVdp1VramS16(u32 EA, s16 value)
{
    u32 offset = EA - 0x25C00000;
    *(s16*)(VDP1Vram + offset) = value;
}

u16 getVdp1VramU16(u32 EA)
{
    u32 offset = EA - 0x25C00000;
    return *(u16*)(VDP1Vram + offset);
}

s16 getVdp1VramS16(u32 EA)
{
    u32 offset = EA - 0x25C00000;
    return *(s16*)(VDP1Vram + offset);
}

u8 getVdp1VramU8(u32 EA)
{
    u32 offset = EA - 0x25C00000;
    return *(u8*)(VDP1Vram + offset);
}

u8* getVdp1Pointer(u32 EA)
{
    u32 offset = EA - 0x25C00000;
    assert(offset < 0x7FFFF);
    return VDP1Vram + offset;
}

u16 VDP1_PTMR = 0;
u16 VDP1_EWDR = 0;
u16 VDP1_EWLR = 0;
u16 VDP1_EWRR = 0;

