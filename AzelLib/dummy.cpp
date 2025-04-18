#include "PDS.h"
#include "kernel/debug/trace.h"
#include "titleScreen.h"

u32 isInMenu2;

void soundFunc(int) { PDS_unimplemented("soundFunc"); }
#if !defined(SHIPPING_BUILD)
bool keyboardIsKeyDown(u8 keycode) { PDS_unimplemented("keyboardIsKeyDown");  return false; }
#endif

void titleMenuToggleTutorials(struct s_titleMenuEntry* menu1, struct s_titleMenuEntry* menu2)
{
    PDS_unimplemented("titleMenuToggleTutorials");
    menu1->m_isEnabled = true;
    menu2->m_isEnabled = true;
}
void titleScreenDrawSub3(u32) { PDS_unimplemented("titleScreenDrawSub3"); }

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
    r1 = 0x29;
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

    if (isTraceEnabled())
    {
        extern bool delayTrace;
        delayTrace = false;
        readTraceLogU32(r0, "Random");
    }

    return r0;

}
bool readKeyboardToggle(u16 keyIndex)
{
    return false;
}

void closeAllOpenFiles()
{

}

