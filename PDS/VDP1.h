#pragma once

extern u16 VDP1_PTMR;
extern u16 VDP1_EWDR;
extern u16 VDP1_EWLR;
extern u16 VDP1_EWRR;

void setVdp1VramU16(u32 EA, u16 value);
void setVdp1VramS16(u32 EA, s16 value);
u16 getVdp1VramU16(u32 EA);
u8 getVdp1VramU8(u32 EA);
u8* getVdp1Pointer(u32 EA);
