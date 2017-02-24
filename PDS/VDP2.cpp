#include "PDS.h"

u8 vdp2Ram[0x100000];
u16* vdp2TextMemory = (u16*)&vdp2Ram[0x6000];
s_Vdp2PrintStatus vdp2DebugPrintPosition;
s_VDP2Regs VDP2Regs_;

void vdp2DebugPrintSetPosition(s32 x, s32 y)
{
    if (x < 0)
        x += 44;
    if (y < 0)
        y += 28;

    vdp2DebugPrintPosition.X = x;
    vdp2DebugPrintPosition.Y = y;
}

int renderVdp2String(char* text)
{
    int r12 = 0;
    s32 r3 = vdp2DebugPrintPosition.X;
    s32 r6 = vdp2DebugPrintPosition.Y;

    u16* pOutput = vdp2TextMemory + (r3 + r6 * 64);

    s32 r0 = vdp2DebugPrintPosition.field_10;

    u16 r8 = 0xDE;
    u16 r9 = r8 - 0x3D; // ?
    u8 r11 = 0x80;

    while (char currentChar = *(text++))
    {
        u16* r14 = pOutput + 0x40;

        if (currentChar >= r9)
        {
            assert(0);
        }
        else
        {
            if (currentChar >= 0x20)
            {
                currentChar -= 0x20;

                u16 finalValue = 0x63 + (u16)currentChar * 2;

                *pOutput = finalValue;
                *r14 = finalValue + 1;
            }
            else
            {
                assert(0);
            }
        }

        pOutput++;
        r12++;
    }

    return 0;
}

void clearVdp2Text()
{
    u16* pOutput = vdp2TextMemory + (vdp2DebugPrintPosition.X + vdp2DebugPrintPosition.Y * 64);

    while (*pOutput)
    {
        *pOutput = 0;
        *(pOutput + 0x40) = 0;

        pOutput++;
    }
}
