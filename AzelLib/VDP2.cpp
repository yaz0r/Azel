#include "PDS.h"

u8 vdp2Ram[0x80000];
u8* VDP2_VRamStart = vdp2Ram;

u8 vdp2CRam[0x1000];
u8* VDP2_CRamStart = vdp2CRam;

u32 vdp2TextMemoryOffset = 0x6000;
//u16 vdp2TextMemory = (u16*)&vdp2Ram[0x6000];
u8* vdp2FontPalettes = VDP2_CRamStart + 0xE00;
u8* vdp2Palette = VDP2_CRamStart + 0xC00;
u8* MENU_SCB = (u8*)&vdp2Ram[0x71C00];

s_Vdp2PrintStatus vdp2PrintStatus;
s_VDP2Regs VDP2Regs_;

sVdp2StringContext vdp2StringContext;
sVdp2Controls vdp2Controls;

u8* getVdp2Vram(u32 offset)
{
    if (offset >= 0x25E00000)
    {
        return getVdp2Vram(offset - 0x25E00000);
    }
    return vdp2Ram + (offset & 0x1FFFFF);
}

u32 getVdp2VramOffset(u8* ptr)
{
    intptr_t offset = ptr - vdp2Ram;
    assert(offset >= 0);
    assert(offset < 0x80000);

    return (u32)offset + 0x25E00000;
}

u8* getVdp2Cram(u32 offset)
{
    return vdp2CRam + (offset & 0xFFF);
}

u8 getVdp2VramU8(u32 offset)
{
    return *(u8*)getVdp2Vram(offset);
}

u32 getVdp2VramU32(u32 offset)
{
    return (*(u8*)getVdp2Vram(offset) << 24) | (*(u8*)getVdp2Vram(offset + 1) << 16) | (*(u8*)getVdp2Vram(offset + 2) << 8) | (*(u8*)getVdp2Vram(offset + 3) << 0);
}

u16 getVdp2VramU16(u32 offset)
{
    u32 high = *(u8*)getVdp2Vram(offset);
    u32 low = *(u8*)getVdp2Vram(offset+1);

    return (high << 8) | low;
}

void setVdp2VramU16(u32 offset, u16 value)
{
    *(u8*)getVdp2Vram(offset) = (value >> 8) & 0xFF;
    *(u8*)getVdp2Vram(offset+1) = value & 0xFF;
}

void setVdp2VramU8(u32 offset, u8 value)
{
    *(u8*)getVdp2Vram(offset) = value;
}

void setVdp2VramU32(u32 offset, u32 value)
{
    *(u8*)getVdp2Vram(offset) = (value >> 24) & 0xFF;
    *(u8*)getVdp2Vram(offset + 1) = (value >> 16) & 0xFF;
    *(u8*)getVdp2Vram(offset + 2) = (value >> 8) & 0xFF;
    *(u8*)getVdp2Vram(offset + 3) = value & 0xFF;
}

u16 getVdp2CramU16(u32 offset)
{
    u32 high = *(u8*)getVdp2Cram(offset);
    u32 low = *(u8*)getVdp2Cram(offset + 1);

    return (high << 8) | low;
}

void vdp2DebugPrintSetPosition(s32 x, s32 y)
{
    if (x < 0)
        x += 44;
    if (y < 0)
        y += 28;

    vdp2PrintStatus.m0_X = x;
    vdp2PrintStatus.m4_Y = y;
}

int drawStringLargeFont(const char* text)
{
    int r12 = 0;
    s32 r3 = vdp2PrintStatus.m0_X;
    s32 r6 = vdp2PrintStatus.m4_Y;

    u32 pOutput = vdp2TextMemoryOffset + 2*(r3 + r6 * 64);

    s32 r0 = vdp2PrintStatus.m10_palette;

    u16 r8 = 0xDE;
    u16 r9 = r8 - 0x3D; // ?
    u8 r11 = 0x80;

    u32 r7 = r0 + 0x63;

    while (char currentChar = *(text++))
    {
        u32 r14 = pOutput + 0x80;

        if (currentChar >= r9)
        {
            assert(0);
        }
        else
        {
            if (currentChar >= 0x20)
            {
                currentChar -= 0x20;

                u16 finalValue = r7 + (u16)currentChar * 2;

                setVdp2VramU16(pOutput, finalValue);
                setVdp2VramU16(r14, finalValue + 1);
            }
            else
            {
                assert(0);
            }
        }

        pOutput+=2;
        r12++;
    }

    return 0;
}

void vdp2DebugPrintNewLine(const std::string& text)
{
    drawLineSmallFont(text.c_str());
    vdp2PrintStatus.m0_X = 0;
    vdp2PrintStatus.m4_Y++;

    if (vdp2PrintStatus.m4_Y > 63)
    {
        vdp2PrintStatus.m4_Y = 0;
    }
}

int drawStringSmallFont(const char* text)
{
    s32 r3 = vdp2PrintStatus.m0_X;
    s32 r6 = vdp2PrintStatus.m4_Y;

    u32 pOutput = vdp2TextMemoryOffset + 2 * (r3 + r6 * 64);

    s32 r0 = vdp2PrintStatus.m10_palette;
    s32 r1 = r0 + 3;

    int count = 0;

    while (char currentChar = *(text++))
    {
        currentChar -= 0x20;
        if (currentChar < 0x60)
        {
            u16 finalValue = r1 + (u16)currentChar;

            setVdp2VramU16(pOutput, finalValue);
            pOutput += 2;
            count++;
        }
    }

    return count;
}


void clearVdp2TextLargeFont()
{
    u32 pOutputOffset = vdp2TextMemoryOffset + (vdp2PrintStatus.m0_X + vdp2PrintStatus.m4_Y * 64) * 2;

    while (getVdp2VramU16(pOutputOffset))
    {
        setVdp2VramU16(pOutputOffset,0);
        setVdp2VramU16(pOutputOffset + 0x80, 0);
        pOutputOffset+=2;
    }
}

void clearVdp2TextSmallFont()
{
    u32 pOutputOffset = vdp2TextMemoryOffset + (vdp2PrintStatus.m0_X + vdp2PrintStatus.m4_Y * 64) * 2;

    while (getVdp2VramU16(pOutputOffset))
    {
        setVdp2VramU16(pOutputOffset, 0);
        pOutputOffset+=2;
    }
}

void unpackGraphicsToVDP2(u8* compressedData, u8* destination)
{
    u8* pCurrentDestination = destination;
    u8 r6 = *(compressedData++);
    u8 r7 = 9;

    do
    {
        if (pCurrentDestination >= destination + 0x1C78)
        {
            destination = destination;
        }

        if ((--r7) == 0)
        {
            r6 = *(compressedData++);
            r7 = 8;
        }

        u8 bit = r6 & 1;
        r6 >>= 1;

        if (bit)
        {
            *(pCurrentDestination++) = *(compressedData++);
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

                    composite |= 0xFFFFE000;

                    if (r0)
                    {
                        r0 += 2;

                        u8* source = pCurrentDestination + composite;

                        for (int i = 0; i < (int)r0; i++)
                        {
                            *(pCurrentDestination++) = *(source++);
                        }
                    }
                    else
                    {
                        u8 r0 = *(compressedData++);

                        u8* source = pCurrentDestination + composite;

                        for (int i = 0; i < (int)r0 + 1; i++)
                        {
                            *(pCurrentDestination++) = *(source++);
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
                u8* source = pCurrentDestination + r2;

                for (int i = 0; i < (int)r0; i++)
                {
                    *(pCurrentDestination++) = *(source++);
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

    loadFile("ASCII.CGZ", fontFile, 0);

    addToMemoryLayout(fontFile, 1);

    unpackGraphicsToVDP2(fontFile, VDP2_VRamStart);

    if(0)
    {
        FILE* fOut = fopen("outputFont.bin", "wb+");
        fwrite(VDP2_VRamStart, 1, 0x5E80, fOut);
        fclose(fOut);
    }

    delete[] fontFile;
}

void clearVdp2VRam(u32 offset, u32 size)
{
    u8* output = VDP2_VRamStart + offset;
    for (u32 i = 0; i < size; i++)
    {
        *(output++) = 0;
    }
}

void clearVdp2CRam(u32 offset, u32 size)
{
    u8* output = VDP2_CRamStart + offset;
    for (u32 i = 0; i < size; i++)
    {
        *(output++) = 0;
    }
}

void resetVdp2RegsCopy(s_VDP2Regs* pRegisters)
{
    u16 tvStat = VDP2Regs_.m4_TVSTAT;
    u16 tvMode = VDP2Regs_.m0_TVMD;

    memset(pRegisters, 0, sizeof(s_VDP2Regs));

    pRegisters->m0_TVMD = ((tvStat & 1) << 5) | (0x8000 & tvMode) | 1;
    pRegisters->m10_CYCA0 = 0xFFFF;
    pRegisters->m14_CYCA1 = 0xFFFF;
    pRegisters->m18_CYCB0 = 0xFFFF;
    pRegisters->m1C_CYCB1 = 0xFFFF;
    pRegisters->m78_ZMXN0 = 0x10000;
    pRegisters->m88_ZMXN1 = 0x10000;
    pRegisters->mE0_SPCTL = 0x20;
}

void updateVDP2Regs()
{
    VDP2Regs_.m0_TVMD = vdp2Controls.m4_pendingVdp2Regs->m0_TVMD;
    VDP2Regs_.m2_EXTEN = vdp2Controls.m4_pendingVdp2Regs->m2_EXTEN;
    // skip TVSTAT
    VDP2Regs_.m6_VRSIZE = vdp2Controls.m4_pendingVdp2Regs->m6_VRSIZE;
    // skip HCNT
    // skip VCNT
    // skip padding
    VDP2Regs_.mE_RAMCTL = vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL;

    memcpy(&VDP2Regs_.m10_CYCA0, &vdp2Controls.m4_pendingVdp2Regs->m10_CYCA0, sizeof(s_VDP2Regs) - (intptr_t)(&((s_VDP2Regs*)NULL)->m10_CYCA0));
}

void resetVdp2RegsCopies()
{
    resetVdp2RegsCopy(&vdp2Controls.m20_registers[0]);
    resetVdp2RegsCopy(&vdp2Controls.m20_registers[1]);

    updateVDP2Regs();
}

void setupVdp2TextLayerColor()
{
    g_fadeControls.m_4C = 0;
    g_fadeControls.m_4D = 5;
    g_fadeControls.m_4A = 0xC210;
    g_fadeControls.m_48 = 0xC210;

    vdp2Controls.m20_registers[0].m110_CLOFEN = 0x7F;
    vdp2Controls.m4_pendingVdp2Regs->m110_CLOFEN = 0x7F;

    if (g_fadeControls.m_4D >= g_fadeControls.m_4C) //?
    {
        vdp2Controls.m20_registers[0].m112_CLOFSL = 0;
        vdp2Controls.m4_pendingVdp2Regs->m112_CLOFSL = 0;
    }

    g_fadeControls.m0_fade0.m20_stopped = 1;
    g_fadeControls.m24_fade1.m20_stopped = 1;

    g_fadeControls.m0_fade0.m0_color[0] = 0;
    g_fadeControls.m0_fade0.m0_color[1] = 0;
    g_fadeControls.m0_fade0.m0_color[2] = 0;

    g_fadeControls.m24_fade1.m0_color[0] = 0;
    g_fadeControls.m24_fade1.m0_color[1] = 0;
    g_fadeControls.m24_fade1.m0_color[2] = 0;

    VDP2Regs_.m114_COAR = 0;
    VDP2Regs_.m116_COAG = 0;
    VDP2Regs_.m118_COAB = 0;

    VDP2Regs_.m11A_COBR = 0;
    VDP2Regs_.m11C_COBG = 0;
    VDP2Regs_.m11E_COBB = 0;

    vdp2Controls.m20_registers[0].m114_COAR = 0;
    vdp2Controls.m20_registers[0].m116_COAG = 0;
    vdp2Controls.m20_registers[0].m118_COAB = 0;
    vdp2Controls.m20_registers[0].m11A_COBR = 0;
    vdp2Controls.m20_registers[0].m11C_COBG = 0;
    vdp2Controls.m20_registers[0].m11E_COBB = 0;

    vdp2Controls.m4_pendingVdp2Regs->m114_COAR = 0;
    vdp2Controls.m4_pendingVdp2Regs->m116_COAG = 0;
    vdp2Controls.m4_pendingVdp2Regs->m118_COAB = 0;
    vdp2Controls.m4_pendingVdp2Regs->m11A_COBR = 0;
    vdp2Controls.m4_pendingVdp2Regs->m11C_COBG = 0;
    vdp2Controls.m4_pendingVdp2Regs->m11E_COBB = 0;
}

sLayerConfig textLayerVdp2Setup[] = {
    m2_CHCN, 0,
    m5_CHSZ, 0,
    m6_PNB, 1,
    m7_CNSM, 1,
    m12_PLSZ, 0,
    m40_CAOS, 7,
    m0_END
};

void setupNBG0(const sLayerConfig* setup)
{
    while (eVdp2LayerConfig command = setup->m_configType)
    {
        u32 arg = setup->m_value;

        setup++;

        switch (command)
        {
        case m2_CHCN:
            vdp2Controls.m4_pendingVdp2Regs->m28_CHCTLA = (vdp2Controls.m4_pendingVdp2Regs->m28_CHCTLA & 0xFF8F) | (arg << 4);
            break;
        case m5_CHSZ:
            vdp2Controls.m4_pendingVdp2Regs->m28_CHCTLA = (vdp2Controls.m4_pendingVdp2Regs->m28_CHCTLA & 0xFFFE) | (arg << 0);
            break;
        case m6_PNB:
            vdp2Controls.m4_pendingVdp2Regs->m30_PNCN0 = (vdp2Controls.m4_pendingVdp2Regs->m30_PNCN0 & 0x7FFF) | (arg << 15);
            break;
        case m7_CNSM:
            vdp2Controls.m4_pendingVdp2Regs->m30_PNCN0 = (vdp2Controls.m4_pendingVdp2Regs->m30_PNCN0 & 0xBFFF) | (arg << 14);
            break;
        case m11_SCN:
            vdp2Controls.m4_pendingVdp2Regs->m30_PNCN0 = (vdp2Controls.m4_pendingVdp2Regs->m30_PNCN0 & 0xFFE0) | (arg << 0);
            break;
        case m12_PLSZ:
            vdp2Controls.m4_pendingVdp2Regs->m3A_PLSZ = (vdp2Controls.m4_pendingVdp2Regs->m3A_PLSZ & 0xFFFC) | (arg << 0);
            break;
        case m40_CAOS:
            vdp2Controls.m4_pendingVdp2Regs->mE4_CRAOFA = (vdp2Controls.m4_pendingVdp2Regs->mE4_CRAOFA & 0xFFF8) | (arg << 0);
            break;
        default:
            assert(false);
            break;
        }
    };

    // force BG layer 0 on
    vdp2Controls.m4_pendingVdp2Regs->m20_BGON = (vdp2Controls.m4_pendingVdp2Regs->m20_BGON & 0xFFFE) | 0x1;

    vdp2Controls.m_isDirty = true;
}

void setupNBG1(const sLayerConfig* setup)
{
    while (eVdp2LayerConfig command = setup->m_configType)
    {
        u32 arg = setup->m_value;

        setup++;

        switch (command)
        {
        case m2_CHCN:
            vdp2Controls.m4_pendingVdp2Regs->m28_CHCTLA = (vdp2Controls.m4_pendingVdp2Regs->m28_CHCTLA & 0x8FFF) | (arg << 12);
            break;
        case m5_CHSZ:
            vdp2Controls.m4_pendingVdp2Regs->m28_CHCTLA = (vdp2Controls.m4_pendingVdp2Regs->m28_CHCTLA & 0xFEFF) | (arg << 8);
            break;
        case m6_PNB:
            vdp2Controls.m4_pendingVdp2Regs->m32_PNCN1 = (vdp2Controls.m4_pendingVdp2Regs->m32_PNCN1 & 0x7FFF) | (arg << 15);
            break;
        case m7_CNSM:
            vdp2Controls.m4_pendingVdp2Regs->m32_PNCN1 = (vdp2Controls.m4_pendingVdp2Regs->m32_PNCN1 & ~0x4000) | (arg << 14);
            break;
        case m9_SCC:
            vdp2Controls.m4_pendingVdp2Regs->m32_PNCN1 = (vdp2Controls.m4_pendingVdp2Regs->m32_PNCN1 & ~0x100) | (arg << 8);
            break;
        case m11_SCN:
            vdp2Controls.m4_pendingVdp2Regs->m32_PNCN1 = (vdp2Controls.m4_pendingVdp2Regs->m32_PNCN1 & 0xFFE0) | (arg << 0);
            break;
        case m12_PLSZ:
            vdp2Controls.m4_pendingVdp2Regs->m3A_PLSZ = (vdp2Controls.m4_pendingVdp2Regs->m3A_PLSZ & 0xFFF3) | (arg << 2);
            break;
        case m21_LCSY:
            vdp2Controls.m4_pendingVdp2Regs->m9A_SCRCTL = (vdp2Controls.m4_pendingVdp2Regs->m9A_SCRCTL & ~0x400) | (arg << 10);
            break;
        case m40_CAOS:
            vdp2Controls.m4_pendingVdp2Regs->mE4_CRAOFA = (vdp2Controls.m4_pendingVdp2Regs->mE4_CRAOFA & 0xFF8F) | (arg << 4);
            break;
        case m41:
            vdp2Controls.m4_pendingVdp2Regs->mE8_LNCLEN = (vdp2Controls.m4_pendingVdp2Regs->mE8_LNCLEN & ~0x2) | (arg << 1);
            break;
        case m44_CCEN:
            vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL = (vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL & ~0x2) | (arg << 1);
            break;
        case m46_SCCM:
            vdp2Controls.m4_pendingVdp2Regs->mEE_SFCCMD = (vdp2Controls.m4_pendingVdp2Regs->mEE_SFCCMD & ~0xC) | (arg << 2);
            break;
        case m45_COEN:
            vdp2Controls.m4_pendingVdp2Regs->m110_CLOFEN = (vdp2Controls.m4_pendingVdp2Regs->m110_CLOFEN & ~0x2) | (arg << 1);
            break;
        default:
            assert(false);
            break;
        }
    };

    // force BG layer 1 on
    vdp2Controls.m4_pendingVdp2Regs->m20_BGON = (vdp2Controls.m4_pendingVdp2Regs->m20_BGON & 0xFFFD) | 0x2;

    vdp2Controls.m_isDirty = true;
}

void setupNBG2(const sLayerConfig* setup)
{
    while (eVdp2LayerConfig command = setup->m_configType)
    {
        u32 arg = setup->m_value;

        setup++;

        switch (command)
        {
        case m2_CHCN:
            vdp2Controls.m4_pendingVdp2Regs->m2A_CHCTLB = (vdp2Controls.m4_pendingVdp2Regs->m2A_CHCTLB & 0xFFFD) | (arg << 1);
            break;
        case m5_CHSZ:
            vdp2Controls.m4_pendingVdp2Regs->m2A_CHCTLB = (vdp2Controls.m4_pendingVdp2Regs->m2A_CHCTLB & 0xFFFE) | (arg << 0);
            break;
        case m6_PNB:
            vdp2Controls.m4_pendingVdp2Regs->m34_PNCN2 = (vdp2Controls.m4_pendingVdp2Regs->m34_PNCN2 & 0x7FFF) | (arg << 15);
            break;
        case m7_CNSM:
            vdp2Controls.m4_pendingVdp2Regs->m34_PNCN2 = (vdp2Controls.m4_pendingVdp2Regs->m34_PNCN2 & 0xBFFF) | (arg << 14);
            break;
        case m11_SCN:
            vdp2Controls.m4_pendingVdp2Regs->m34_PNCN2 = (vdp2Controls.m4_pendingVdp2Regs->m34_PNCN2 & 0xFFE0) | (arg << 0);
            break;
        case m12_PLSZ:
            vdp2Controls.m4_pendingVdp2Regs->m3A_PLSZ = (vdp2Controls.m4_pendingVdp2Regs->m3A_PLSZ & 0xFFCF) | (arg << 4);
            break;
        case m34_W0E:
            vdp2Controls.m4_pendingVdp2Regs->mD2_WCTLB = (vdp2Controls.m4_pendingVdp2Regs->mD2_WCTLB & 0xFFFD) | (arg << 1);
            break;
        case m37_W0A:
            vdp2Controls.m4_pendingVdp2Regs->mD2_WCTLB = (vdp2Controls.m4_pendingVdp2Regs->mD2_WCTLB & 0xFFFE) | (arg << 0);
            break;
        case m40_CAOS:
            vdp2Controls.m4_pendingVdp2Regs->mE4_CRAOFA = (vdp2Controls.m4_pendingVdp2Regs->mE4_CRAOFA & 0xF8FF) | (arg << 8);
            break;
        case m44_CCEN:
            vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL = (vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL & 0xFFCF) | (arg << 2);
            break;
        default:
            assert(false);
            break;
        }
    };

    // force BG layer 2 on
    vdp2Controls.m4_pendingVdp2Regs->m20_BGON = (vdp2Controls.m4_pendingVdp2Regs->m20_BGON & 0xFFCF) | 0x4;

    vdp2Controls.m_isDirty = true;
}

void setupNBG3(const sLayerConfig* setup)
{
    while (eVdp2LayerConfig command = setup->m_configType)
    {
        u32 arg = setup->m_value;

        setup++;

        switch (command)
        {
        case m2_CHCN:
            vdp2Controls.m4_pendingVdp2Regs->m2A_CHCTLB = (vdp2Controls.m4_pendingVdp2Regs->m2A_CHCTLB & 0xFFDF) | (arg << 5);
            break;
        case m5_CHSZ:
            vdp2Controls.m4_pendingVdp2Regs->m2A_CHCTLB = (vdp2Controls.m4_pendingVdp2Regs->m2A_CHCTLB & 0xFFEF) | (arg << 4);
            break;
        case m6_PNB:
            vdp2Controls.m4_pendingVdp2Regs->m36_PNCN3 = (vdp2Controls.m4_pendingVdp2Regs->m36_PNCN3 & 0x7FFF) | (arg << 15);
            break;
        case m7_CNSM:
            vdp2Controls.m4_pendingVdp2Regs->m36_PNCN3 = (vdp2Controls.m4_pendingVdp2Regs->m36_PNCN3 & 0xBFFF) | (arg << 14);
            break;
        case m12_PLSZ:
            vdp2Controls.m4_pendingVdp2Regs->m3A_PLSZ = (vdp2Controls.m4_pendingVdp2Regs->m3A_PLSZ & 0xFF3F) | (arg << 6);
            break;
        case m40_CAOS:
            vdp2Controls.m4_pendingVdp2Regs->mE4_CRAOFA = (vdp2Controls.m4_pendingVdp2Regs->mE4_CRAOFA & 0x8FFF) | (arg << 12);
            break;
        default:
            assert(false);
            break;
        }

        //unnecessary as this is done at function exit anyway
        //vdp2Controls.m_isDirty = true;
    };

    // force BG layer 3 on
    vdp2Controls.m4_pendingVdp2Regs->m20_BGON = (vdp2Controls.m4_pendingVdp2Regs->m20_BGON & 0xFFF7) | 0x8;

    vdp2Controls.m_isDirty = true;
}

void setupRGB0(const sLayerConfig* setup)
{
    while (eVdp2LayerConfig command = setup->m_configType)
    {
        u32 arg = setup->m_value;

        setup++;

        switch (command)
        {
        case m2_CHCN:
            vdp2Controls.m4_pendingVdp2Regs->m2A_CHCTLB = (vdp2Controls.m4_pendingVdp2Regs->m2A_CHCTLB & 0x8fff) | (arg << 0xC);
            break;
        case m5_CHSZ:
            vdp2Controls.m4_pendingVdp2Regs->m2A_CHCTLB = (vdp2Controls.m4_pendingVdp2Regs->m2A_CHCTLB & 0xFEFF) | (arg << 8);
            break;
        case m6_PNB:
            vdp2Controls.m4_pendingVdp2Regs->m38_PNCR = (vdp2Controls.m4_pendingVdp2Regs->m38_PNCR & 0x7FFF) | (arg << 15);
            break;
        case m7_CNSM:
            vdp2Controls.m4_pendingVdp2Regs->m38_PNCR = (vdp2Controls.m4_pendingVdp2Regs->m38_PNCR & 0xBFFF) | (arg << 14);
            break;
        case m11_SCN:
            vdp2Controls.m4_pendingVdp2Regs->m38_PNCR = (vdp2Controls.m4_pendingVdp2Regs->m38_PNCR & 0xFFE0) | (arg << 0);
            break;
        case m27_RPMD:
            vdp2Controls.m4_pendingVdp2Regs->mB0_RPMD = (vdp2Controls.m4_pendingVdp2Regs->mB0_RPMD & 0xFFFC) | arg;
            break;
        case m34_W0E:
            vdp2Controls.m4_pendingVdp2Regs->mD4_WCTLC = (vdp2Controls.m4_pendingVdp2Regs->mD4_WCTLC & 0xFFFD) | (arg << 1);
            break;
        case m37_W0A:
            vdp2Controls.m4_pendingVdp2Regs->mD4_WCTLC = (vdp2Controls.m4_pendingVdp2Regs->mD4_WCTLC & 0xFFFE) | (arg << 0);
            break;
        default:
            assert(false);
            break;
        }
    };

    // force BG layer RGB0 on
    vdp2Controls.m4_pendingVdp2Regs->m20_BGON = (vdp2Controls.m4_pendingVdp2Regs->m20_BGON & 0xFFEF) | 0x10;

    vdp2Controls.m_isDirty = true;
}

void setupRotationParams(const sLayerConfig* setup)
{
    while (eVdp2LayerConfig command = setup->m_configType)
    {
        u32 arg = setup->m_value;

        setup++;

        switch (command)
        {
        case m31_RxKTE:
            vdp2Controls.m4_pendingVdp2Regs->mB4_KTCTL = (vdp2Controls.m4_pendingVdp2Regs->mB4_KTCTL & 0xFFFE) | (arg & 1);
            break;
        default:
            assert(false);
            break;
        }
    };

    vdp2Controls.m_isDirty = true;
}

void setupRotationParams2(const sLayerConfig* setup)
{
    while (eVdp2LayerConfig command = setup->m_configType)
    {
        u32 arg = setup->m_value;

        setup++;

        switch (command)
        {
        default:
            assert(false);
            break;
        }
    };

    vdp2Controls.m_isDirty = true;
}

u32 rotateRightR0ByR1(u32 r0, u32 r1)
{
    return r0 >> r1;
}

void initLayerMap(u32 layer, u32 planeA, u32 planeB, u32 planeC, u32 planeD)
{
    u32 characterSize = 0;
    u32 patternNameDataSize = 0;

    switch (layer)
    {
    case 0:
        characterSize = vdp2Controls.m4_pendingVdp2Regs->m28_CHCTLA & 1;
        patternNameDataSize = vdp2Controls.m4_pendingVdp2Regs->m30_PNCN0 & 0x8000;
        break;
    case 1:
        characterSize = vdp2Controls.m4_pendingVdp2Regs->m28_CHCTLA & 0x100;
        patternNameDataSize = vdp2Controls.m4_pendingVdp2Regs->m32_PNCN1 & 0x8000;
        break;
    case 2:
        characterSize = vdp2Controls.m4_pendingVdp2Regs->m2A_CHCTLB & 0x1;
        patternNameDataSize = vdp2Controls.m4_pendingVdp2Regs->m34_PNCN2 & 0x8000;
        break;
    case 3:
        characterSize = vdp2Controls.m4_pendingVdp2Regs->m2A_CHCTLB & 0x10;
        patternNameDataSize = vdp2Controls.m4_pendingVdp2Regs->m36_PNCN3 & 0x8000;
        break;
    default:
        assert(0);
        break;
    }

    u32 shiftValue;
    if (patternNameDataSize)
    {
        // 1 word
        if (characterSize)
        {
            // 2x2
            shiftValue = 11; // 0x800
        }
        else
        {
            // 1x1
            shiftValue = 13; // 0x2000
        }
    }
    else
    {
        // 2 words
        if (characterSize)
        {
            // 2x2
            shiftValue = 12; // 0x1000
        }
        else
        {
            // 1x1
            shiftValue = 14; // 0x4000
        }
    }

    u32 planeAOffset = planeA;// - 0x25E00000;
    u32 planeBOffset = planeB;// - 0x25E00000;
    u32 planeCOffset = planeC;// - 0x25E00000;
    u32 planeDOffset = planeD;// - 0x25E00000;

    u32 mapOffset = (rotateRightR0ByR1(planeAOffset, shiftValue + 6)) & 7;

    switch (layer)
    {
    case 0:
        vdp2Controls.m4_pendingVdp2Regs->m3C_MPOFN = (vdp2Controls.m4_pendingVdp2Regs->m3C_MPOFN & 0xFFF0) | (mapOffset);
        vdp2Controls.m4_pendingVdp2Regs->m40_MPABN0 = ((rotateRightR0ByR1(planeBOffset, shiftValue) & 0x3F) << 8) | rotateRightR0ByR1(planeAOffset, shiftValue);
        vdp2Controls.m4_pendingVdp2Regs->m42_MPCDN0 = ((rotateRightR0ByR1(planeDOffset, shiftValue) & 0x3F) << 8) | rotateRightR0ByR1(planeCOffset, shiftValue);
        break;
    case 1:
        vdp2Controls.m4_pendingVdp2Regs->m3C_MPOFN = (vdp2Controls.m4_pendingVdp2Regs->m3C_MPOFN & 0xFF0F) | (mapOffset << 4);
        vdp2Controls.m4_pendingVdp2Regs->m44_MPABN1 = ((rotateRightR0ByR1(planeBOffset, shiftValue) & 0x3F) << 8) | rotateRightR0ByR1(planeAOffset, shiftValue);
        vdp2Controls.m4_pendingVdp2Regs->m46_MPCDN1 = ((rotateRightR0ByR1(planeDOffset, shiftValue) & 0x3F) << 8) | rotateRightR0ByR1(planeCOffset, shiftValue);
        break;
    case 2:
        vdp2Controls.m4_pendingVdp2Regs->m3C_MPOFN = (vdp2Controls.m4_pendingVdp2Regs->m3C_MPOFN & 0xF0FF) | (mapOffset << 8);
        vdp2Controls.m4_pendingVdp2Regs->m48_MPABN2 = ((rotateRightR0ByR1(planeBOffset, shiftValue) & 0x3F) << 8) | rotateRightR0ByR1(planeAOffset, shiftValue);
        vdp2Controls.m4_pendingVdp2Regs->m4A_MPCDN2 = ((rotateRightR0ByR1(planeDOffset, shiftValue) & 0x3F) << 8) | rotateRightR0ByR1(planeCOffset, shiftValue);
        break;
    case 3:
        vdp2Controls.m4_pendingVdp2Regs->m3C_MPOFN = (vdp2Controls.m4_pendingVdp2Regs->m3C_MPOFN & 0x0FFF) | (mapOffset << 12);
        vdp2Controls.m4_pendingVdp2Regs->m4C_MPABN3 = ((rotateRightR0ByR1(planeBOffset, shiftValue) & 0x3F) << 8) | rotateRightR0ByR1(planeAOffset, shiftValue);
        vdp2Controls.m4_pendingVdp2Regs->m4E_MPCDN3 = ((rotateRightR0ByR1(planeDOffset, shiftValue) & 0x3F) << 8) | rotateRightR0ByR1(planeCOffset, shiftValue);
        break;
    default:
        assert(0);
    }
}

struct sCharacterMap1Entry
{
    u16 m0_value;
    u16 m2_useCount;
};
std::array<sCharacterMap1Entry, 0x80> characterMap1;
u16 characterMap2[0x1000];

void resetCharacterMap1()
{
    for (int i = 0; i < 0x80; i++)
    {
        characterMap1[i].m0_value = 0;
        characterMap1[i].m2_useCount = 0;
    }
}

void resetCharacterMaps()
{
    resetCharacterMap1();

    for (int i = 0; i < 0x1000; i++)
    {
        characterMap2[i] = 0;
    }
}

void clearVdp2TextMemory()
{
    memset(getVdp2Vram(vdp2TextMemoryOffset), 0, 0x10 * 0x100);
    resetCharacterMaps();
}

void initVdp2TextLayer()
{
    setupNBG3(textLayerVdp2Setup);

    initLayerMap(3, vdp2TextMemoryOffset, vdp2TextMemoryOffset, vdp2TextMemoryOffset, vdp2TextMemoryOffset);

    clearVdp2TextMemory();

    vdp2DebugPrintSetPosition(0, 0);

    vdp2PrintStatus.m8_oldX = vdp2PrintStatus.m0_X;
    vdp2PrintStatus.mC_oldY = vdp2PrintStatus.m4_Y;
    vdp2PrintStatus.m10_palette = 0xC000;
    vdp2PrintStatus.m14_oldPalette = 0xC000;
}

// 16 palettes of 16 u16
u8 defaultFontPalettes[16 * 2 * 16] = {
    0x14, 0x02, 0x08, 0x42, 0x0C, 0x84, 0x18, 0xE7, 0x25, 0x4A, 0x31, 0xAD, 0x3E, 0x31, 0x56, 0xF7, 0x2D, 0x60, 0x63, 0x00, 0x01, 0xD5, 0x02, 0xFF, 0x16, 0x80, 0x1F, 0xA0, 0x08, 0x4D, 0x0C, 0x7B,
    0x25, 0x43, 0x04, 0x21, 0x0C, 0x63, 0x14, 0xA5, 0x1C, 0xE7, 0x21, 0x08, 0x25, 0x29, 0x29, 0x4A, 0x2D, 0x6B, 0x35, 0xAD, 0x42, 0x10, 0x4A, 0x52, 0x08, 0x21, 0x14, 0x22, 0x20, 0x23, 0x30, 0x44,
    0x25, 0x43, 0x04, 0x21, 0x0C, 0x63, 0x14, 0xA5, 0x1C, 0xE7, 0x21, 0x08, 0x25, 0x29, 0x29, 0x4A, 0x2D, 0x6B, 0x35, 0xAD, 0x42, 0x10, 0x4A, 0x52, 0x04, 0x21, 0x08, 0x42, 0x0C, 0x63, 0x10, 0x84,
    0x25, 0x43, 0x04, 0x21, 0x0C, 0x63, 0x14, 0xA5, 0x1C, 0xE7, 0x21, 0x08, 0x25, 0x29, 0x29, 0x4A, 0x2D, 0x6B, 0x35, 0xAD, 0x42, 0x10, 0x4A, 0x52, 0x04, 0x22, 0x04, 0x23, 0x08, 0x44, 0x0C, 0x65,
    0x25, 0x43, 0x04, 0x21, 0x0C, 0x63, 0x14, 0xA5, 0x1C, 0xE7, 0x21, 0x08, 0x25, 0x29, 0x29, 0x4A, 0x2D, 0x6B, 0x35, 0xAD, 0x42, 0x10, 0x4A, 0x52, 0x04, 0x41, 0x04, 0x61, 0x08, 0x82, 0x0C, 0xA3,
    0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x80, 0x00, 0xEB, 0x5A, 0x80, 0x40, 0x80, 0x60, 0x88, 0xC0, 0x91, 0x00, 0x9D, 0xA0, 0xAA, 0x20, 0xB7, 0x00, 0x80, 0x00, 0xFD, 0x22, 0xFF, 0x03, 0x81, 0xC0, 0x82, 0xA3, 0x83, 0xE7, 0x83, 0xF7,
    0xA4, 0xA3, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF1, 0x7C, 0xED, 0x1B, 0xE4, 0xFA, 0xD8, 0x97, 0xD0, 0x54, 0xC8, 0x72, 0xA8, 0x6A, 0x84, 0x21,
    0xA4, 0xA3, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEB, 0x47, 0xDE, 0xE4, 0xD6, 0xA5, 0xCE, 0x64, 0xC6, 0x23, 0xB1, 0x83, 0x9C, 0xE3, 0x84, 0x21,
    0xA4, 0xA3, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xA3, 0x68, 0x9F, 0x27, 0x9A, 0xE6, 0x96, 0xA5, 0x92, 0x64, 0x8D, 0xA3, 0x88, 0xE2, 0x84, 0x21,
    0xA4, 0xA3, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF2, 0x10, 0xED, 0xCE, 0xE1, 0x8C, 0xE1, 0x6B, 0xE0, 0xE7, 0xB4, 0xE7, 0xA0, 0xA5, 0x84, 0x21,
    0xA4, 0xA3, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xB1, 0x9C, 0xB1, 0x9C, 0xA5, 0x39, 0x9C, 0xF6, 0x98, 0xD4, 0x94, 0xB2, 0x90, 0x89, 0x84, 0x21,
    0xA4, 0xA3, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEF, 0x7B, 0xDE, 0xF7, 0xCE, 0x73, 0xC6, 0x31, 0xB5, 0xAD, 0xA1, 0x08, 0x90, 0x84, 0x84, 0x21,
    0xA4, 0xA3, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x87, 0x5A, 0x82, 0xF9, 0x82, 0xD8, 0x82, 0x54, 0x81, 0xD0, 0x81, 0x8C, 0x84, 0xE7, 0x84, 0x21,
    0xA4, 0xA3, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xB1, 0x8C, 0xA9, 0x4A, 0xA5, 0x29, 0x9C, 0xE7, 0x98, 0xC6, 0x94, 0xA5, 0x90, 0x84, 0x84, 0x21,
    0x00, 0x00, 0x08, 0x42, 0x63, 0x5A, 0x10, 0x84, 0x4A, 0x94, 0x42, 0x10, 0x31, 0x8C, 0x56, 0xD6, 0x35, 0xAD, 0x29, 0x4A, 0x21, 0x08, 0x54, 0x47, 0x01, 0xC0, 0x02, 0xA3, 0x03, 0xE7, 0x03, 0xF7,
};

void asyncDmaCopy(void* source, void* target, u32 size, u32 unk)
{
    memcpy(target, source, size);
}

void asyncDmaCopy(sSaturnPtr EA, void* target, u32 size, u32 unk)
{
    for (int i = 0; i < size; i++)
    {
        *((u8*)target+i) = readSaturnU8(EA + i);
    }
}

void copyFontToVdp2()
{
    asyncDmaCopy(defaultFontPalettes, vdp2FontPalettes, 0x200, 0);
}

void setFontDefaultColor(u32 paletteIndex, u16 color0, u16 color1)
{

}

void setFontDefaultColors()
{
    setFontDefaultColor(7, 0xF03C, 0x8421);
    setFontDefaultColor(8, 0xEB47, 0x8421);
    setFontDefaultColor(9, 0xA368, 0x8421);
    setFontDefaultColor(10, 0xF210, 0x8421);
    setFontDefaultColor(11, 0xB19C, 0x8421);
    setFontDefaultColor(12, 0xEF7B, 0x8421);
    setFontDefaultColor(13, 0x875A, 0x8421);
    setFontDefaultColor(14, 0xB18C, 0x8421);

    copyFontToVdp2();
}

std::array<sVdpVar1, 14> vdpVar1;

void initVdp2Var1()
{
    for (int i=0; i<14; i++)
    {
        vdpVar1[i].mF_isPending = 0;
    }
}

sVdpVar1* vdpVar2;
sVdpVar1* vdpVar3;

void initVdp2Var2()
{
    vdpVar2 = &vdpVar1[0];
    vdpVar3 = &vdpVar1[0];
    vdpVar1[0].m10_nextTransfert = nullptr;
}

void initVDP2()
{
    vdp2Controls.m0_doubleBufferIndex = 0;
    vdp2Controls.m4_pendingVdp2Regs = &vdp2Controls.m20_registers[0];

    resetVdp2RegsCopies();
    clearVdp2VRam(0, 0x80000);
    clearVdp2CRam(0, 0x1000);

    initVdp2Var1();
    initVdp2Var2();

    setupVdp2TextLayerColor();
    loadFont();
    initVdp2TextLayer();
    setFontDefaultColors();
}

void reinitVdp2()
{
    vdp2Controls.m0_doubleBufferIndex = 0;
    vdp2Controls.m4_pendingVdp2Regs = &vdp2Controls.m20_registers[0];

    resetVdp2RegsCopies();

    initVdp2Var1();
    initVdp2Var2();

    initVdp2TextLayer();
    setFontDefaultColors();
}

void updateVDP2CoordinatesIncrement(u32 unk0, u32 unk1)
{
    switch (pauseEngine[4])
    {
    case 0:
        vdp2Controls.m20_registers[0].m78_ZMXN0 = unk0;
        vdp2Controls.m4_pendingVdp2Regs->m78_ZMXN0 = unk0;
        vdp2Controls.m20_registers[0].m7C_ZMYN0 = unk1;
        vdp2Controls.m4_pendingVdp2Regs->m7C_ZMYN0 = unk1;
        break;
    case 1:
        vdp2Controls.m20_registers[0].m88_ZMXN1 = unk0;
        vdp2Controls.m4_pendingVdp2Regs->m88_ZMXN1 = unk0;
        vdp2Controls.m20_registers[0].m8C_ZMYN1 = unk1;
        vdp2Controls.m4_pendingVdp2Regs->m8C_ZMYN1 = unk1;
        break;
    default:
        assert(0);
        break;
    }
}

void setupVDP2CoordinatesIncrement2(u32 unk0, u32 unk1)
{
    switch (pauseEngine[4])
    {
    case 0:
        vdp2Controls.m20_registers[0].m70_SCXN0 = unk0;
        vdp2Controls.m4_pendingVdp2Regs->m70_SCXN0 = unk0;
        vdp2Controls.m20_registers[0].m74_SCYN0 = unk1;
        vdp2Controls.m4_pendingVdp2Regs->m74_SCYN0 = unk1;
        break;
    case 1:
        vdp2Controls.m20_registers[0].m80_SCXN1 = unk0;
        vdp2Controls.m4_pendingVdp2Regs->m80_SCXN1 = unk0;
        vdp2Controls.m20_registers[0].m84_SCYN1 = unk1;
        vdp2Controls.m4_pendingVdp2Regs->m84_SCYN1 = unk1;
        break;
    case 2:
        vdp2Controls.m20_registers[0].m90_SCXN2 = unk0;
        vdp2Controls.m4_pendingVdp2Regs->m90_SCXN2 = unk0;
        vdp2Controls.m20_registers[0].m92_SCYN2 = unk1;
        vdp2Controls.m4_pendingVdp2Regs->m92_SCYN2 = unk1;
        break;
    case 3:
        vdp2Controls.m20_registers[0].m94_SCXN3 = unk0;
        vdp2Controls.m4_pendingVdp2Regs->m94_SCXN3 = unk0;
        vdp2Controls.m20_registers[0].m96_SCYN3 = unk1;
        vdp2Controls.m4_pendingVdp2Regs->m96_SCYN3 = unk1;
        break;
    default:
        assert(0);
        break;
    }
}

int drawLineLargeFont(const char* text)
{
    int result = drawStringLargeFont(text);

    vdp2PrintStatus.m0_X = 0;
    vdp2PrintStatus.m4_Y += 2;
    if (vdp2PrintStatus.m4_Y > 0x3F)
    {
        vdp2PrintStatus.m4_Y = 0;
    }

    return result;
}

int  drawLineSmallFont(const char* string)
{
    int result = drawStringSmallFont(string);

    vdp2PrintStatus.m0_X = 0;
    vdp2PrintStatus.m4_Y++;

    if (vdp2PrintStatus.m4_Y > 0x3F)
    {
        vdp2PrintStatus.m4_Y = 0;
    }

    return result;
}

void clearVdp2StringFieldDebugList()
{
    clearVdp2TextSmallFont();

    for (int i = 3; i < 19; i++)
    {
        vdp2DebugPrintSetPosition(18, i);
        clearVdp2TextSmallFont();
    }
}

void vdp2PrintfSmallFont(const char* format, ...)
{
    const u32 stringSize = 128;
    char buffer[stringSize];
    int length;

    va_list args;

    va_start(args, format);
    {
        length = vsnprintf(buffer, stringSize, format, args);
    }
    va_end(args);

    assert(length < stringSize);

    drawLineSmallFont(buffer);
}

void vdp2PrintfLargeFont(const char* format, ...)
{
    const u32 stringSize = 128;
    char buffer[stringSize];
    int length;

    va_list args;

    va_start(args, format);
    {
        length = vsnprintf(buffer, stringSize, format, args);
    }
    va_end(args);

    assert(length < stringSize);

    drawLineLargeFont(buffer);
}

sVdp2StringControl vdp2StringControlBuffer;
sVdp2StringControl* pVdp2StringControl = NULL;

void clearVdp2TextMemoryRect(s32 r4, s32 r5, s32 r6, s32 r7)
{
    PDS_unimplemented("clearVdp2TextMemoryRect");
}

s32 setActiveFont(u16 r4)
{
    sVdp2StringControl* r14 = &vdp2StringControlBuffer;
    u16 fontIndex = pVdp2StringControl->m0_index;
    if (fontIndex != r4)
    {
        do 
        {
            if (r14->m0_index == r4)
            {
                resetCharacterMaps();

                clearVdp2TextMemoryRect(0, 0, 44, 32);

                pVdp2StringControl = r14;
                break;
            }

            r14 = r14->pNext;
        } while (r14);
    }

    if (r14)
    {
        return fontIndex;
    }
    return -1;
}

void initVdp2StringControl()
{
    pVdp2StringControl = &vdp2StringControlBuffer;

    pVdp2StringControl->m0_index = 0;
    pVdp2StringControl->m4 = 0;
    pVdp2StringControl->m8 = 0;
    pVdp2StringControl->mC = 0;
    pVdp2StringControl->m10 = 0;
    pVdp2StringControl->m14 = 0;
    pVdp2StringControl->m15 = 0;
    pVdp2StringControl->pPrevious = 0;
    pVdp2StringControl->pNext = 0;
}

struct {
    u32 m0;
    u32 m4; // 4
    u32 m8; // 8
    u32 mC; // C
}vdp2StringContext2;

void resetVdp2StringContext2()
{
    vdp2StringContext2.m0 = 0;
    vdp2StringContext2.m4 = 0;
    vdp2StringContext2.m8 = 0;
    vdp2StringContext2.mC = 0;
}

void resetVdp2StringContext()
{
    vdp2StringContext.m0 = 0;
    vdp2StringContext.m4_cursorX = 0;
    vdp2StringContext.m8_cursorY = 0;
    vdp2StringContext.mC_X = 0;
    vdp2StringContext.m10_Y = 0;
    vdp2StringContext.m14_Width = 44;
    vdp2StringContext.m18_Height = 30;
    vdp2StringContext.m1C = 0;
    vdp2StringContext.m20 = 0;
    vdp2StringContext.m24 = 0;
    vdp2StringContext.m28 = 0;
    vdp2StringContext.m2C = 0;
    vdp2StringContext.m30 = 0;
    vdp2StringContext.m34 = 0;
    vdp2StringContext.m38 = 0;
    vdp2StringContext.m3C = 0x8421;
}

sVdp2StringControl* getEndOfvdp2StringControlBufferList()
{
    sVdp2StringControl* r4 = &vdp2StringControlBuffer;
    while (r4->pNext)
    {
        r4 = r4->pNext;
    }

    return r4;
}

sVdp2StringControl* extendedSprites;

void loadCharacterToVdp2(s16 index, s16 offset)
{
    u8* vdp2FontBase = getVdp2Vram(0x8000);

    s16 r5 = offset - 0x8000;
    u16* r6 = pVdp2StringControl->mC;

    if (pVdp2StringControl->m14 == 4)
    {
        if (r5 < 0x100)
        {
            r6 = extendedSprites->mC;
        }
        else
        {
            r5 -= 0x100;
        }
    }
    else
    {
        offset &= 0xFFFF;
    }

    u16* r11 = r6 + (r5 << 4);
    u8* r14 = vdp2FontBase + (index << 7);
    
    u8 var_28[8];
    u8* var_2C = var_28 + 7;

    for (int counter = 2; counter > 0; counter--)
    {
        for (int r10 = 8; r10 > 0; r10--)
        {
            u16 r13 = *r11;
            u16 r6 = (r13 << 1) | (r13 >> 1);

            u16 r2 = r11[-1];
            u16 r7 = r11[-1];
            u16 r3 = r11[0];

            r7 |= r3;
            r7 &= r6;

            r2 |= r3;
            r2 |= r7 >> 1;

            r6 |= r2;

            {
                u8* r8 = var_2C;
                for (int r15 = 8; r15 > 0; r15--)
                {
                    u32 r9 = r13 & 3;
                    *r8 = ((r9 << 3) | r9) & 0x11;
                    r13 >>= 2;
                    r8--;
                }
            }

            {
                u8* r13 = var_28;
                {
                    u8 r3 = *(r13++);
                    u32 r9 = (r7 >> 10) & 0x30;
                    u32 r8 = (r6 >> 9) & 0x60;
                    r14[0] = ((r8 + (r8 >> 3)) & 0x44) + ((r9 + (r9 >> 3)) & 0x22) + r3;
                }
                {
                    u8 r3 = *(r13++);
                    u32 r8 = (r7 >> 8) & 0x30;
                    u32 r9 = (r6 >> 7) & 0x60;
                    r14[1] = ((r9 + (r9 >> 3)) & 0x44) + ((r8 + (r8 >> 3)) & 0x22) + r3;
                }
                {
                    u8 r3 = *(r13++);
                    u32 r9 = (r7 >> 6) & 0x30;
                    u32 r8 = (r6 >> 5) & 0x60;
                    r14[2] = ((r8 + (r8 >> 3)) & 0x44) + ((r9 + (r9 >> 3)) & 0x22) + r3;
                }
                {
                    u8 r3 = *(r13++);
                    u32 r9 = (r7 >> 4) & 0x30;
                    u32 r8 = (r6 >> 3) & 0x60;
                    r14[3] = ((r8 + (r8 >> 3)) & 0x44) + ((r9 + (r9 >> 3)) & 0x22) + r3;
                }
                {
                    u8 r3 = *(r13++);
                    u32 r9 = (r7 >> 2) & 0x30;
                    u32 r8 = (r6 >> 1) & 0x60;
                    r14[0x20] = ((r8 + (r8 >> 3)) & 0x44) + ((r9 + (r9 >> 3)) & 0x22) + r3;
                }
                {
                    u8 r3 = *(r13++);
                    u32 r9 = (r7 >> 1) & 0x30;
                    u32 r8 = (r6 << 1) & 0x60;
                    r14[0x21] = ((r8 + (r8 >> 3)) & 0x44) + ((r9 + (r9 >> 3)) & 0x22) + r3;
                }
                {
                    u8 r3 = *(r13++);
                    u32 r9 = (r7 << 2) & 0x30;
                    u32 r8 = (r6 << 3) & 0x60;
                    r14[0x22] = ((r8 + (r8 >> 3)) & 0x44) + ((r9 + (r9 >> 3)) & 0x22) + r3;
                }
                {
                    u8 r3 = *(r13++);
                    u32 r9 = (r7 << 4) & 0x30;
                    u32 r8 = (r6 << 5) & 0x60;
                    r14[0x23] = ((r8 + (r8 >> 3)) & 0x44) + ((r9 + (r9 >> 3)) & 0x22) + r3;
                }
                r14 += 4;
            }
        }

        r14 += 0x20;
    }
    

}

void resetVdp2StringsSub1Sub1()
{
    s16 r14 = 0x80;

    do 
    {
        r14--;
        loadCharacterToVdp2(r14, r14 + 0x8000);
    } while (r14);
}

s32 resetVdp2StringsSub1(u16* pData)
{
    sVdp2StringControl* pOld = getEndOfvdp2StringControlBufferList();

    sVdp2StringControl* pNew = static_cast<sVdp2StringControl*>(allocateHeap(sizeof(sVdp2StringControl)));
    assert(pNew);

    pNew->m0_index = pOld->m0_index + 1;
    pNew->m4 = pData[0];
    pNew->m8 = pData;
    pNew->mC = 0;
    pNew->m10 = 0;
    pNew->pPrevious = pOld;
    pNew->pNext = NULL;
    pNew->m14 = pData[1];

    bool r12 = false;

    switch (pNew->m14)
    {
    case 4:
        pNew->mC = pData + 0x10 / 2;
        break;
    case 5:
        pNew->mC = pData + 0x10 / 2;
        extendedSprites = pNew;
        r12 = true;
        break;
    default:
        break;
    }

    pVdp2StringControl = pNew;
    resetCharacterMaps();

    if (r12)
    {
        resetVdp2StringsSub1Sub1();
    }

    return pVdp2StringControl->m0_index;
}

void resetVdp2Strings()
{
    initVdp2StringControl();

    resetVdp2StringContext2();

    resetVdp2StringContext();

    resetCharacterMaps();

    resetVdp2StringsSub1(resetVdp2StringsData);
}

void setupVDP2StringRendering(s32 x, s32 y, s32 width, s32 height)
{
    if (x < 0)
    {
        x = 0;
    }

    if (x > 63)
    {
        x = 63;
    }

    if (y < 0)
    {
        y = 0;
    }

    if (y > 63)
    {
        y = 63;
    }

    if (x + width > 63)
    {
        width = 63 - x;
    }

    if (y + height > 63)
    {
        height = 63 - y;
    }

    vdp2StringContext.m4_cursorX = x;
    vdp2StringContext.m8_cursorY = y;
    vdp2StringContext.mC_X = x;
    vdp2StringContext.m10_Y = y;
    vdp2StringContext.m14_Width = width;
    vdp2StringContext.m18_Height = height;
}

void VDP2DrawString(const char* string)
{
    s_stringStatusQuery query;
    getVdp2StringContext(string, &query);
    
    query.m2C |= 1;

    printVdp2String(&query);

    moveVdp2TextCursor(&query);
}

void getVdp2StringContext(const char* string, s_stringStatusQuery* vars)
{
    vars->m0_cursorX = vdp2StringContext.m4_cursorX;
    vars->m4_cursorY = vdp2StringContext.m8_cursorY;
    vars->m10_windowX1 = vdp2StringContext.mC_X;
    vars->m14_windowY1 = vdp2StringContext.m10_Y;
    vars->m8_windowWidth = vdp2StringContext.m14_Width;
    vars->mC_windowHeight = vdp2StringContext.m18_Height;
    vars->m18_windowX2 = vdp2StringContext.mC_X + vdp2StringContext.m14_Width;
    vars->m1C_windowY2 = vdp2StringContext.m10_Y + vdp2StringContext.m18_Height;
    vars->m20_string = string;
    vars->m24_vdp2MemoryOffset = vdp2TextMemoryOffset + ((vdp2StringContext.m8_cursorY * 0x40) + vdp2StringContext.m4_cursorX) * 2;
    vars->m28 = vdp2StringContext.m0;
    vars->m2C = vdp2StringContext.m38;
}

void moveVdp2TextCursor(s_stringStatusQuery* vars)
{
    vdp2StringContext.m4_cursorX = vars->m0_cursorX;
    vdp2StringContext.m8_cursorY = vars->m4_cursorY;
}

u32 printVdp2StringTable[10] = {
    12, 13, 7, 8, 9, 10, 11, 13, 7, 14
};

void printVdp2StringNewLine(s_stringStatusQuery* vars)
{
    PDS_unimplemented("printVdp2StringNewLine");
}

void printVdp2StringSub2(s32 r4)
{
    Unimplemented();
}

//
void printVdp2StringSub1(s_stringStatusQuery* vars)
{
    if (vars->m2C & 1)
    {
        int stringLength = computeStringLength(vars->m20_string, vars->m8_windowWidth);
        if (stringLength < vars->m8_windowWidth)
        {
            vars->m0_cursorX += (vars->m8_windowWidth - stringLength) / 2;
        }
    }
}

void printVdp2String(s_stringStatusQuery* vars)
{
    u32 r11 = (printVdp2StringTable[vars->m28] << 12) + 0x63;
    printVdp2StringSub1(vars);
    vars->m24_vdp2MemoryOffset = vdp2TextMemoryOffset + (((vars->m4_cursorY * 0x40) + vars->m0_cursorX)) * 2;

    while (u8 r4 = *(vars->m20_string++))
    {
        switch (r4)
        {
        case 0xA:
            printVdp2StringNewLine(vars);
            break;
        case '%':
        {
            s32 r4 = *(vars->m20_string++);
            r4 &= 0xFF;
            switch (r4)
            {
            case 0x61:
                printVdp2StringSub2(*(vars->m20_string++) - 0x30);
                break;
            default:
                assert(0);
                break;
            }
        }
        default:
            setVdp2VramU16(vars->m24_vdp2MemoryOffset, r11 + (r4 - 0x20) * 2);
            setVdp2VramU16(vars->m24_vdp2MemoryOffset + 0x80, r11 + (r4 - 0x20) * 2 + 1);
            vars->m24_vdp2MemoryOffset += 2;
            vars->m0_cursorX++;
            break;
        }

        if (vars->m8_windowWidth)
        {
            if (vars->m18_windowX2 - vars->m0_cursorX < 1)
            {
                printVdp2StringNewLine(vars);
            }
        }

        if (vars->mC_windowHeight)
        {
            if (vars->m1C_windowY2 - vars->m4_cursorY < 1)
            {
                break;
            }
        }
    }

    vars->m20_string--;
}

s32 computeStringLength(sSaturnPtr pString, s32 r5)
{
    return computeStringLength(readSaturnString(pString).c_str(), r5);
}

s32 computeStringLength(const char* pString, s32 r5)
{
    s32 r14 = 1;
    while (r5 > r14)
    {
        u8 r4 = *pString;
        pString = pString + 1;
        r4 &= 0xFF;
        switch (r4)
        {
        case 0:
        case 0xA:
            return r14;
        case 0x25:
            assert(0);
            break;
        default:
            break;
        }
        r14++;
    };
    return r14;
}

void clearBlueBox(s32 x, s32 y, s32 width, s32 height)
{
    x /= 2;
    width /= 2;
    height += y + 1;
    height /= 2;
    height -= y / 2;
    y /= 2;

    u32 r2 = 0x5800 + ((y * 0x20) + x) * 2;

    for (int r7 = 0; r7 < height; r7++)
    {
        u32 r4 = r2;
        for (int j = 0; j < width; j++)
        {
            setVdp2VramU16(r4, 0);
            r4 += 2;
        }
        r2 += 0x40;
    }
}

void drawBlueBox(s32 x, s32 y, s32 width, s32 height, u32 param5)
{
    width /= 2;
    int iVar6 = (((int)y >> 1) * 0x20 + (x >> 1)) * 2;
    u32 puVar8 = 0x25E05800 + iVar6;

    // Draw the top
    {
        u16 sVar3;
        if ((y & 1) == 0)
        {
            sVar3 = 0x79;
        }
        else
        {
            sVar3 = 0x82;
        }

        sVar3 += param5;

        // draw opening sprite
        setVdp2VramU16(puVar8, sVar3);
        u32 psVar2 = 0x25E05800 + iVar6 + 2;
        iVar6 = 2;
        // draw span
        if (2 < width)
        {
            do
            {
                iVar6++;
                setVdp2VramU16(psVar2, sVar3 + 1);
                psVar2 += 2;
            } while (iVar6 < width);
        }
        // draw closing sprite
        setVdp2VramU16(psVar2, sVar3 + 2);
    }

    // Draw the middle
    {
        iVar6 = 2;
        while (iVar6 < ((int)(height + y + 1) >> 1) - ((int)y >> 1))
        {
            u32 puVar7 = puVar8 + 0x40;
            int iVar4 = 2;
            setVdp2VramU16(puVar7, param5 + 0x7C); // opening sprite
            u32 psVar2 = puVar8 + 0x42;
            if (width > 2)
            {
                do {
                    setVdp2VramU16(psVar2, param5 + 0x7D); // draw span
                    iVar4++;
                    psVar2 += 2;
                } while (iVar4 < width);
            }
            setVdp2VramU16(psVar2, param5 + 0x7E); // draw closing sprite
            iVar6++;
            puVar8 = puVar7;
        }
    }
    puVar8 += 0x40;

    // Draw the bottom
    {
        u16 sVar3;
        if ((height + y & 1) == 0)
        {
            sVar3 = 0x7F;
        }
        else
        {
            sVar3 = 0x85;
        }

        sVar3 += param5;

        // draw opening sprite
        setVdp2VramU16(puVar8, sVar3);
        u32 psVar2 = puVar8 + 2;
        iVar6 = 2;
        // draw span
        if (2 < width)
        {
            do
            {
                iVar6++;
                setVdp2VramU16(psVar2, sVar3 + 1);
                psVar2 += 2;
            } while (iVar6 < width);
        }
        // draw closing sprite
        setVdp2VramU16(psVar2, sVar3 + 2);
    }
}

void displayObjectIcon(s32 r4, s32 r5_x, s32 r6_y, s32 r7_iconId)
{
    s32 offset = vdp2TextMemoryOffset + ((r6_y * 0x40) + r5_x) * 2;
    s32 glyph = r7_iconId * 4 + 0x220;
    setVdp2VramU16(offset, glyph);
    setVdp2VramU16(offset + 2, glyph + 1);
    setVdp2VramU16(offset + 0x80, glyph + 2);
    setVdp2VramU16(offset + 0x82, glyph + 3);
}

void flushPengingVDP2Transfers()
{
    sVdpVar1* pTransfert = vdpVar2;
    while (pTransfert = pTransfert->m10_nextTransfert, pTransfert != nullptr)
    {
        pTransfert->mF_isPending = 0;
        if (pTransfert->mE_isDoubleBuffered == 0)
        {
            memcpy_dma(pTransfert->m0_source[0], pTransfert->m8_destination, pTransfert->mC_size * 0x10);
        }
        else
        {
            memcpy_dma(pTransfert->m0_source[vdp2Controls.m0_doubleBufferIndex], pTransfert->m8_destination, pTransfert->mC_size * 0x10);
        }
    }
}

void interruptVDP2Update()
{
    //vdp2Controls.m0_doubleBufferIndex ^= 1;
    vdp2Controls.m4_pendingVdp2Regs = &vdp2Controls.m20_registers[vdp2Controls.m0_doubleBufferIndex];
    updateVDP2Regs();
    if (vdp2Controls.m_isDirty)
    {
        //dmaVDP2Regs();
        vdp2Controls.m_isDirty = 0;
    }
    flushPengingVDP2Transfers();
    initVdp2Var2();
}

void printVdp2Number3(s_stringStatusQuery* vars, int value, int length)
{
    Unimplemented();
}

void printVdp2Number2(int value, int length)
{
    s_stringStatusQuery vars;
    getVdp2StringContext(nullptr, &vars);
    printVdp2Number3(&vars, value, length);
    moveVdp2TextCursor(&vars);
}

int loadExtendedCharacter(u16 character)
{
    if (character < 0x8080)
    {
        return character + 0x8000;
    }

    // already loaded?
    int iVar3 = 0x80;
    std::array<sCharacterMap1Entry, 0x80>::iterator pdVar5 = characterMap1.begin();
    int iVar1 = iVar3;
    if (true) {
        do {
            if (pdVar5->m0_value == character)
            {
                pdVar5->m2_useCount++;
                return iVar1;
            }
            iVar1 = iVar1 + 1;
            pdVar5 = pdVar5 + 1;
        } while (pdVar5 < characterMap1.end());
    }

    // now loaded, find a slot to load
    int iVar2 = 0;
    iVar1 = iVar3;
    do {
        if (characterMap1[iVar2].m2_useCount == 0) {
            loadCharacterToVdp2(iVar3, character);
            characterMap1[iVar2].m0_value = character;
            characterMap1[iVar2].m2_useCount = 1;
            return iVar1;
        }
        iVar2 = iVar2 + 1;
        iVar1 = iVar1 + 1;
        iVar3 = iVar3 + 1;
    } while (iVar2 < 0x80);
    
    return 0;
}

void clearVdp2TextAreaSub1Sub1(u16 r4)
{
    if (r4 >= 0x80)
    {
        characterMap1[r4 - 0x80].m2_useCount--;
    }
}

u32 clearVdp2TextAreaSub1(u16 r4, s32 x, s32 y)
{
    if (r4 == 0)
    {
        setVdp2VramU16(vdp2TextMemoryOffset + (y * 64 + x) * 2, 0);
    }

    u16 var0 = characterMap2[(y << 6) + x];
    u16 r13 = var0;
    if (r13 == r4)
    {
        return 0;
    }
    if (r13 == 0)
    {
        return 2;
    }

    u16* var_14 = characterMap2 + ((x - (r13 & 1)) << 7);
    y -= (2 & r13) >> 1;
    var0 = (y << 1);
    *(var_14 + (var0 >> 1)) = 0;

    u16* var_10 = characterMap2 + ((x + 1) << 7);
    *(var_10 + (var0 >> 1)) = 0;
    *(var_14 + (y + 1)) = 0;

    *(var_10 + (y + 1)) = 0;

    setVdp2VramU16(vdp2TextMemoryOffset + ((y << 6) + x) * 2, 0);
    setVdp2VramU16(vdp2TextMemoryOffset + ((y << 6) + x) * 2 + 2, 0);
    setVdp2VramU16(vdp2TextMemoryOffset + ((y << 6) + x) * 2 + 0x80, 0);
    setVdp2VramU16(vdp2TextMemoryOffset + ((y << 6) + x) * 2 + 0x82, 0);

    clearVdp2TextAreaSub1Sub1(((r13 & 0x7FF) - 0x400) >> 2);

    return 1;
}

s32 clearExtendedCharacter(u32 param_1, s_stringStatusQuery* vars)
{
    if (clearVdp2TextAreaSub1(param_1, vars->m0_cursorX, vars->m4_cursorY))
    {
        clearVdp2TextAreaSub1(param_1, vars->m0_cursorX + 1, vars->m4_cursorY);
        clearVdp2TextAreaSub1(param_1, vars->m0_cursorX, vars->m4_cursorY + 1);
        clearVdp2TextAreaSub1(param_1, vars->m0_cursorX + 1, vars->m4_cursorY + 1);
        return 1;
    }
    return 0;
}


void printVdp2String3(s_stringStatusQuery* vars)
{
    u32 r11 = (printVdp2StringTable[vars->m28] *0x1000) + 0x400;
    printVdp2StringSub1(vars);
    vars->m24_vdp2MemoryOffset = vdp2TextMemoryOffset + (((vars->m4_cursorY * 0x40) + vars->m0_cursorX)) * 2;
    do 
    {
        int character = (u8)*(vars->m20_string++);
        if (character == 0)
        {
            return;
        }
        if (character < 0x10)
        {
            assert(0);
        }
        else
        {
            if (character > 0x7F)
            {
                character = (character * 0x100) + (u8) * (vars->m20_string++);
            }

            int characterSprite = loadExtendedCharacter(character) * 4 + r11;
            if (!clearExtendedCharacter(characterSprite, vars))
            {
                clearVdp2TextAreaSub1Sub1(character);
                vars->m24_vdp2MemoryOffset += 4;
                vars->m0_cursorX++;
            }
            else
            {
                setVdp2VramU16(vars->m24_vdp2MemoryOffset + 0x00, characterSprite + 0);
                setVdp2VramU16(vars->m24_vdp2MemoryOffset + 0x02, characterSprite + 1);
                setVdp2VramU16(vars->m24_vdp2MemoryOffset + 0x80, characterSprite + 2);
                setVdp2VramU16(vars->m24_vdp2MemoryOffset + 0x82, characterSprite + 3);

                characterMap2[vars->m0_cursorX * 0x40 + vars->m4_cursorY] = characterSprite + 0;
                characterMap2[(vars->m0_cursorX + 1) * 0x40 + vars->m4_cursorY] = characterSprite + 1;
                characterMap2[vars->m0_cursorX * 0x40 + vars->m4_cursorY] = characterSprite + 2;
                characterMap2[(vars->m0_cursorX + 1) * 0x40 + vars->m4_cursorY + 1] = characterSprite + 3;

                vars->m24_vdp2MemoryOffset += 4;
                vars->m0_cursorX += 2;
            }
        }

        if ((vars->m8_windowWidth != 0) && (vars->m18_windowX2 - vars->m0_cursorX < 2)) {
            assert(0);
        }
    } while ((vars->mC_windowHeight == 0) || (1 < vars->m1C_windowY2 - vars->m4_cursorY));
    vars->m20_string--;
}

void printVdp2String2(const char* string)
{
    s_stringStatusQuery vars;
    getVdp2StringContext(string, &vars);
    printVdp2String3(&vars);
    moveVdp2TextCursor(&vars);
}

