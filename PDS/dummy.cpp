#include "PDS.h"

u32 isInMenu2;

sMenuUnk0 menuUnk0;

void playSoundEffect(int) { PDS_unimplemented("playSoundEffect"); }
bool keyboardIsKeyDown(u8 keycode) { PDS_unimplemented("keyboardIsKeyDown");  return false; }
void titleMenuToggleTutorials(struct s_titleMenuEntry* menu1, struct s_titleMenuEntry* menu2) { PDS_unimplemented("titleMenuToggleTutorials"); }
u32 titleScreenDrawSub1(sMenuUnk0* arrayData) { PDS_unimplemented("titleScreenDrawSub1"); return 0; }
void fadePalette(sMenuUnk0Sub* arrayData, u32 from, u32 to, u32 steps) { PDS_unimplemented("fadePalette"); }
void titleScreenDrawSub3(u32) { PDS_unimplemented("titleScreenDrawSub3"); }
void playMusic(u8 musicNumber, u8 unk1) { PDS_unimplemented("playMusic"); }
bool soundFunc1() { PDS_unimplemented("soundFunc1"); return 0; }

void playPCM(p_workArea, u32) { PDS_unimplemented("playPCM"); }

u32 RNG_seed = 0;
void DEBUG_setRandomSeed(u32 seed)
{
    RNG_seed = seed;
}

void initRandomSeed(u32 init)
{
    RNG_seed = init;
    randomNumber();
}

u32 randomNumber()
{
    u32 r0 = RNG_seed;
    u32 r1 = (u16)r0;
    if (r1 == 0)
    {
        r0 = 0x2A6D365B;
    }
    else
    {
        r1 = 0x29;
    }

    r1 *= r0;
    r0 >>= 16;
    u32 r2 = (u16)r1;
    r1 >>= 16;
    r1 += r2;
    r1 <<= 16;
    r0 |= r1;
    r0 = ((r0 & 0xFFFF) << 16) | ((r0 >> 16) & 0xFFFF);
    r1 |= r2;
    RNG_seed = r1;
    return r0;

}
bool readKeyboardToggle(u8 keyIndex) { return false; }

