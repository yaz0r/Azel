#include "PDS.h"

u8 vdp2Ram[0x80000];
u8* VDP2_VRamStart = vdp2Ram;

u8 vdp2CRam[0x1000];
u8* VDP2_CRamStart = vdp2CRam;

u16* vdp2TextMemory = (u16*)&vdp2Ram[0x6000];

s_Vdp2PrintStatus vdp2DebugPrintPosition;
s_VDP2Regs VDP2Regs_;

struct sVdp2Controls
{
    u32 m_0;
    s_VDP2Regs* m_pendingVdp2Regs;
    u32 m_8;
    u32 m_C;
    u32 m_10;
    u32 m_14;
    u32 m_18;
    u32 m_1C;

    // 0x20
    s_VDP2Regs m_registers[2];
} vdp2Controls;

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

u32 getFileSize(const char* fileName)
{
    FILE* fHandle = fopen(fileName, "rb");
    assert(fHandle);

    fseek(fHandle, 0, SEEK_END);
    u32 fileSize = ftell(fHandle);

    fclose(fHandle);

    return fileSize;
}

void loadFile(const char* fileName, u32 size, u8* destination, u32 unk0)
{
    FILE* fHandle = fopen(fileName, "rb");
    assert(fHandle);

    fread(destination, size, 1, fHandle);

    fclose(fHandle);
}

void addToVDP2MemoryLayout(u8* pointer, u32 unk0)
{
}

void unpackGraphicsToVDP2(u8* compressedData, u8* destination)
{
    u8 r6 = *(compressedData++);
    u8 r7 = 9;

    do
    {
        if ((--r7) == 0)
        {
            r6 = *(compressedData++);
            r7 = 8;
        }

        u8 bit = r6 & 1;
        r6 >>= 1;

        if (bit)
        {
            *(destination++) = *(compressedData++);
        }
        else
        {
            if ((--r7) == 0)
            {
                r6 = *(compressedData++);
                r7 = 8;
            }

            u8 bit2 = r6 & 1;
            r6 >>= 1;

            if (bit2)
            {
                u8 r0 = *(compressedData++);
                u8 r1 = *(compressedData++);

                s16 composite = (((s16)r1) << 8) | r0;

                if (composite)
                {
                    composite >>= 3;
                    r0 &= 7;

                    if (r0)
                    {
                        r0 += 2;
                        u8* source = destination + composite;

                        for (int i = 0; i < (int)r0; i++)
                        {
                            *(destination++) = *(source++);
                        }
                    }
                    else
                    {
                        u8 r0 = *(compressedData++);
                        u8* source = destination + composite;

                        for (int i = 0; i < (int)r0 + 1; i++)
                        {
                            *(destination++) = *(source++);
                        }
                    }
                }
                else
                {
                    // end of stream
                    return;
                }
            }
            else
            {
                u8 r0 = 0;
                if ((--r7) == 0)
                {
                    r6 = *(compressedData++);
                    r7 = 8;
                }

                u8 bit3 = r6 & 1;
                r6 >>= 1;

                r0 |= bit3;

                if ((--r7) == 0)
                {
                    r6 = *(compressedData++);
                    r7 = 8;
                }

                u8 bit4 = r6 & 1;
                r6 >>= 1;

                r0 <<= 1;
                r0 |= bit4;

                s16 r2 = *(compressedData++);
                r2 |= 0xFF00;

                r0 += 2;
                u8* source = destination + r2;

                for (int i = 0; i < (int)r0; i++)
                {
                    *(destination++) = *(source++);
                }
            }
        }
    } while (1);
}

void loadFont()
{
    u32 fileSize = getFileSize("ASCII.CGZ");
    assert(fileSize);

    u8* fontFile = new u8[fileSize];
    assert(fontFile);

    loadFile("ASCII.CGZ", fileSize, fontFile, 0);

    addToVDP2MemoryLayout(fontFile, 1);

    unpackGraphicsToVDP2(fontFile, VDP2_VRamStart);

    delete[] fontFile;
}

void clearVdp2VRam(u32 offset, u32 size)
{
    u8* output = VDP2_VRamStart + offset;
    for (int i = 0; i < size; i++)
    {
        *(output++) = 0;
    }
}

void clearVdp2CRam(u32 offset, u32 size)
{
    u8* output = VDP2_CRamStart + offset;
    for (int i = 0; i < size; i++)
    {
        *(output++) = 0;
    }
}

void resetVdp2RegsCopy(s_VDP2Regs* pRegisters)
{
    u16 tvStat = VDP2Regs_.TVSTAT;
    u16 tvMode = VDP2Regs_.TVMD;

    memset(pRegisters, 0, sizeof(s_VDP2Regs));

    pRegisters->TVMD = ((tvStat & 1) << 5) | (0x8000 & tvMode) | 1;
    pRegisters->CYCA0 = 0xFFFF;
    pRegisters->CYCA1 = 0xFFFF;
    pRegisters->CYCB0 = 0xFFFF;
    pRegisters->CYCB1 = 0xFFFF;
    pRegisters->ZMXIN0 = 1;
    pRegisters->ZMYIN0 = 1;
    pRegisters->ZMXIN1 = 1;
    pRegisters->ZMYIN1 = 1;
    pRegisters->SPCTL = 0x20;
}

void updateVDP2Regs()
{
    VDP2Regs_.TVMD = vdp2Controls.m_pendingVdp2Regs->TVMD;
    VDP2Regs_.EXTEN = vdp2Controls.m_pendingVdp2Regs->EXTEN;
    // skip TVSTAT
    VDP2Regs_.VRSIZE = vdp2Controls.m_pendingVdp2Regs->VRSIZE;
    // skip HCNT
    // skip VCNT
    // skip padding
    VDP2Regs_.RAMCTL = vdp2Controls.m_pendingVdp2Regs->RAMCTL;

    memcpy(&VDP2Regs_.CYCA0, &vdp2Controls.m_pendingVdp2Regs->CYCA0, sizeof(s_VDP2Regs) - (u32)(&((s_VDP2Regs*)NULL)->CYCA0));
}

void resetVdp2RegsCopies()
{
    resetVdp2RegsCopy(&vdp2Controls.m_registers[0]);
    resetVdp2RegsCopy(&vdp2Controls.m_registers[1]);

    updateVDP2Regs();
}

void initVDP2()
{
    vdp2Controls.m_0 = 0;
    vdp2Controls.m_pendingVdp2Regs = &vdp2Controls.m_registers[0];

    resetVdp2RegsCopies();
    clearVdp2VRam(0, 0x80000);
    clearVdp2CRam(0, 0x1000);

    //initVdp2Var1();
    //initVdp2Var2();

    //setupVdp2TextLayerColor();
    loadFont();
    //initVdp2TextLayer();
    //setupVdp2TextLayerCharacterMaps();
}
