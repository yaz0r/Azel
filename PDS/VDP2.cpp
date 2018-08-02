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
    return vdp2Ram + offset;
}

u8* getVdp2Cram(u32 offset)
{
    return vdp2CRam + offset;
}

u8 getVdp2VramU8(u32 offset)
{
    return *(u8*)getVdp2Vram(offset);
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

    vdp2PrintStatus.X = x;
    vdp2PrintStatus.Y = y;
}

int drawStringLargeFont(const char* text)
{
    int r12 = 0;
    s32 r3 = vdp2PrintStatus.X;
    s32 r6 = vdp2PrintStatus.Y;

    u32 pOutput = vdp2TextMemoryOffset + 2*(r3 + r6 * 64);

    s32 r0 = vdp2PrintStatus.palette;

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

int drawStringSmallFont(const char* text)
{
    s32 r3 = vdp2PrintStatus.X;
    s32 r6 = vdp2PrintStatus.Y;

    u32 pOutput = vdp2TextMemoryOffset + 2 * (r3 + r6 * 64);

    s32 r0 = vdp2PrintStatus.palette;
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
    u32 pOutputOffset = vdp2TextMemoryOffset + (vdp2PrintStatus.X + vdp2PrintStatus.Y * 64) * 2;

    while (getVdp2VramU16(pOutputOffset))
    {
        setVdp2VramU16(pOutputOffset,0);
        setVdp2VramU16(pOutputOffset + 0x80, 0);
        pOutputOffset+=2;
    }
}

void clearVdp2TextSmallFont()
{
    u32 pOutputOffset = vdp2TextMemoryOffset + (vdp2PrintStatus.X + vdp2PrintStatus.Y * 64) * 2;

    while (getVdp2VramU16(pOutputOffset))
    {
        setVdp2VramU16(pOutputOffset, 0);
        pOutputOffset+=2;
    }
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

    loadFile("ASCII.CGZ", fontFile, 0);

    addToMemoryLayout(fontFile, 1);

    unpackGraphicsToVDP2(fontFile, VDP2_VRamStart);

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

    memcpy(&VDP2Regs_.CYCA0, &vdp2Controls.m_pendingVdp2Regs->CYCA0, sizeof(s_VDP2Regs) - (intptr_t)(&((s_VDP2Regs*)NULL)->CYCA0));
}

void resetVdp2RegsCopies()
{
    resetVdp2RegsCopy(&vdp2Controls.m_registers[0]);
    resetVdp2RegsCopy(&vdp2Controls.m_registers[1]);

    updateVDP2Regs();
}

void setupVdp2TextLayerColor()
{
    menuUnk0.m_4C = 0;
    menuUnk0.m_4D = 5;
    menuUnk0.m_4A = 0xC210;
    menuUnk0.m_48 = 0xC210;

    vdp2Controls.m_registers[0].N1COEN = 0x7F;
    vdp2Controls.m_registers[1].N1COEN = 0x7F;

    if (menuUnk0.m_4D >= menuUnk0.m_4C) //?
    {
        vdp2Controls.m_registers[0].N1COSL = 0;
        vdp2Controls.m_registers[1].N1COSL = 0;
    }

    menuUnk0.m_field0.m_field20 = 1;
    menuUnk0.m_field24.m_field20 = 1;

    menuUnk0.m_field0.m_field0[0] = 0;
    menuUnk0.m_field0.m_field0[1] = 0;
    menuUnk0.m_field0.m_field0[2] = 0;

    menuUnk0.m_field24.m_field0[0] = 0;
    menuUnk0.m_field24.m_field0[1] = 0;
    menuUnk0.m_field24.m_field0[2] = 0;

    VDP2Regs_.COAR = 0;
    VDP2Regs_.COAG = 0;
    VDP2Regs_.COAB = 0;

    VDP2Regs_.COBR = 0;
    VDP2Regs_.COBG = 0;
    VDP2Regs_.COBB = 0;

    vdp2Controls.m_registers[0].COAR = 0;
    vdp2Controls.m_registers[0].COAG = 0;
    vdp2Controls.m_registers[0].COAB = 0;
    vdp2Controls.m_registers[0].COBR = 0;
    vdp2Controls.m_registers[0].COBG = 0;
    vdp2Controls.m_registers[0].COBB = 0;

    vdp2Controls.m_registers[1].COAR = 0;
    vdp2Controls.m_registers[1].COAG = 0;
    vdp2Controls.m_registers[1].COAB = 0;
    vdp2Controls.m_registers[1].COBR = 0;
    vdp2Controls.m_registers[1].COBG = 0;
    vdp2Controls.m_registers[1].COBB = 0;
}

sLayerConfig textLayerVdp2Setup[] = {
    CHCN, 0,
    CHSZ, 0,
    PNB, 1,
    CNSM, 1,
    PLSZ, 0,
    CAOS, 7,
    END
};

void setupNBG0(sLayerConfig* setup)
{
    while (eVdp2LayerConfig command = setup->m_configType)
    {
        u32 arg = setup->m_value;

        setup++;

        switch (command)
        {
        case CHCN:
            vdp2Controls.m_pendingVdp2Regs->CHCTLA = (vdp2Controls.m_pendingVdp2Regs->CHCTLA & 0xFF8F) | (arg << 4);
            break;
        case CHSZ:
            vdp2Controls.m_pendingVdp2Regs->CHCTLA = (vdp2Controls.m_pendingVdp2Regs->CHCTLA & 0xFFFE) | (arg << 0);
            break;
        case PNB:
            vdp2Controls.m_pendingVdp2Regs->PNCN0 = (vdp2Controls.m_pendingVdp2Regs->PNCN0 & 0x7FFF) | (arg << 15);
            break;
        case CNSM:
            vdp2Controls.m_pendingVdp2Regs->PNCN0 = (vdp2Controls.m_pendingVdp2Regs->PNCN0 & 0xBFFF) | (arg << 14);
            break;
        case SCN:
            vdp2Controls.m_pendingVdp2Regs->PNCN0 = (vdp2Controls.m_pendingVdp2Regs->PNCN0 & 0xFFE0) | (arg << 0);
            break;
        case PLSZ:
            vdp2Controls.m_pendingVdp2Regs->PLSZ = (vdp2Controls.m_pendingVdp2Regs->PLSZ & 0xFFFC) | (arg << 0);
            break;
        case CAOS:
            vdp2Controls.m_pendingVdp2Regs->CRAOFA = (vdp2Controls.m_pendingVdp2Regs->CRAOFA & 0xFFF8) | (arg << 0);
            break;
        default:
            assert(false);
            break;
        }
    };

    // force BG layer 0 on
    vdp2Controls.m_pendingVdp2Regs->BGON = (vdp2Controls.m_pendingVdp2Regs->BGON & 0xFFFE) | 0x1;

    vdp2Controls.m_isDirty = true;
}

void setupNBG1(sLayerConfig* setup)
{
    while (eVdp2LayerConfig command = setup->m_configType)
    {
        u32 arg = setup->m_value;

        setup++;

        switch (command)
        {
        case CHCN:
            vdp2Controls.m_pendingVdp2Regs->CHCTLA = (vdp2Controls.m_pendingVdp2Regs->CHCTLA & 0x8FFF) | (arg << 12);
            break;
        case CHSZ:
            vdp2Controls.m_pendingVdp2Regs->CHCTLA = (vdp2Controls.m_pendingVdp2Regs->CHCTLA & 0xFEFF) | (arg << 8);
            break;
        case PNB:
            vdp2Controls.m_pendingVdp2Regs->PNCN1 = (vdp2Controls.m_pendingVdp2Regs->PNCN1 & 0x7FFF) | (arg << 15);
            break;
        case CNSM:
            vdp2Controls.m_pendingVdp2Regs->PNCN1 = (vdp2Controls.m_pendingVdp2Regs->PNCN1 & 0xBFFF) | (arg << 14);
            break;
        case SCN:
            vdp2Controls.m_pendingVdp2Regs->PNCN1 = (vdp2Controls.m_pendingVdp2Regs->PNCN1 & 0xFFE0) | (arg << 0);
            break;
        case PLSZ:
            vdp2Controls.m_pendingVdp2Regs->PLSZ = (vdp2Controls.m_pendingVdp2Regs->PLSZ & 0xFFF3) | (arg << 2);
            break;
        case CAOS:
            vdp2Controls.m_pendingVdp2Regs->CRAOFA = (vdp2Controls.m_pendingVdp2Regs->CRAOFA & 0xFF8F) | (arg << 4);
            break;
        default:
            assert(false);
            break;
        }
    };

    // force BG layer 1 on
    vdp2Controls.m_pendingVdp2Regs->BGON = (vdp2Controls.m_pendingVdp2Regs->BGON & 0xFFFD) | 0x2;

    vdp2Controls.m_isDirty = true;
}

void setupNBG2(sLayerConfig* setup)
{
    while (eVdp2LayerConfig command = setup->m_configType)
    {
        u32 arg = setup->m_value;

        setup++;

        switch (command)
        {
        case CHCN:
            vdp2Controls.m_pendingVdp2Regs->CHCTLB = (vdp2Controls.m_pendingVdp2Regs->CHCTLB & 0xFFFD) | (arg << 1);
            break;
        case CHSZ:
            vdp2Controls.m_pendingVdp2Regs->CHCTLB = (vdp2Controls.m_pendingVdp2Regs->CHCTLB & 0xFFFE) | (arg << 0);
            break;
        case PNB:
            vdp2Controls.m_pendingVdp2Regs->PNCN2 = (vdp2Controls.m_pendingVdp2Regs->PNCN2 & 0x7FFF) | (arg << 15);
            break;
        case CNSM:
            vdp2Controls.m_pendingVdp2Regs->PNCN2 = (vdp2Controls.m_pendingVdp2Regs->PNCN2 & 0xBFFF) | (arg << 14);
            break;
        case SCN:
            vdp2Controls.m_pendingVdp2Regs->PNCN2 = (vdp2Controls.m_pendingVdp2Regs->PNCN2 & 0xFFE0) | (arg << 0);
            break;
        case PLSZ:
            vdp2Controls.m_pendingVdp2Regs->PLSZ = (vdp2Controls.m_pendingVdp2Regs->PLSZ & 0xFFCF) | (arg << 4);
            break;
        case W0E:
            vdp2Controls.m_pendingVdp2Regs->WCTLB = (vdp2Controls.m_pendingVdp2Regs->WCTLB & 0xFFFD) | (arg << 1);
            break;
        case W0A:
            vdp2Controls.m_pendingVdp2Regs->WCTLB = (vdp2Controls.m_pendingVdp2Regs->WCTLB & 0xFFFE) | (arg << 0);
            break;
        case CAOS:
            vdp2Controls.m_pendingVdp2Regs->CRAOFA = (vdp2Controls.m_pendingVdp2Regs->CRAOFA & 0xF8FF) | (arg << 8);
            break;
        case CCEN:
            vdp2Controls.m_pendingVdp2Regs->CCCTL = (vdp2Controls.m_pendingVdp2Regs->CCCTL & 0xFFCF) | (arg << 2);
            break;
        default:
            assert(false);
            break;
        }
    };

    // force BG layer 2 on
    vdp2Controls.m_pendingVdp2Regs->BGON = (vdp2Controls.m_pendingVdp2Regs->BGON & 0xFFCF) | 0x4;

    vdp2Controls.m_isDirty = true;
}

void setupNBG3(sLayerConfig* setup)
{
    while (eVdp2LayerConfig command = setup->m_configType)
    {
        u32 arg = setup->m_value;

        setup++;

        switch (command)
        {
        case CHCN:
            vdp2Controls.m_pendingVdp2Regs->CHCTLB = (vdp2Controls.m_pendingVdp2Regs->CHCTLB & 0xFFDF) | (arg << 5);
            break;
        case CHSZ:
            vdp2Controls.m_pendingVdp2Regs->CHCTLB = (vdp2Controls.m_pendingVdp2Regs->CHCTLB & 0xFFEF) | (arg << 4);
            break;
        case PNB:
            vdp2Controls.m_pendingVdp2Regs->PNCN3 = (vdp2Controls.m_pendingVdp2Regs->PNCN3 & 0x7FFF) | (arg << 15);
            break;
        case CNSM:
            vdp2Controls.m_pendingVdp2Regs->PNCN3 = (vdp2Controls.m_pendingVdp2Regs->PNCN3 & 0xBFFF) | (arg << 14);
            break;
        case PLSZ:
            vdp2Controls.m_pendingVdp2Regs->PLSZ = (vdp2Controls.m_pendingVdp2Regs->PLSZ & 0xFF3F) | (arg << 6);
            break;
        case CAOS:
            vdp2Controls.m_pendingVdp2Regs->CRAOFA = (vdp2Controls.m_pendingVdp2Regs->CRAOFA & 0x8FFF) | (arg << 12);
            break;
        default:
            assert(false);
            break;
        }

        //unnecessary as this is done at function exit anyway
        //vdp2Controls.m_isDirty = true;
    };

    // force BG layer 3 on
    vdp2Controls.m_pendingVdp2Regs->BGON = (vdp2Controls.m_pendingVdp2Regs->BGON & 0xFFF7) | 0x8;

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
        characterSize = vdp2Controls.m_pendingVdp2Regs->CHCTLA & 1;
        patternNameDataSize = vdp2Controls.m_pendingVdp2Regs->PNCN0 & 0x8000;
        break;
    case 1:
        characterSize = vdp2Controls.m_pendingVdp2Regs->CHCTLA & 0x100;
        patternNameDataSize = vdp2Controls.m_pendingVdp2Regs->PNCN1 & 0x8000;
        break;
    case 2:
        characterSize = vdp2Controls.m_pendingVdp2Regs->CHCTLB & 0x1;
        patternNameDataSize = vdp2Controls.m_pendingVdp2Regs->PNCN2 & 0x8000;
        break;
    case 3:
        characterSize = vdp2Controls.m_pendingVdp2Regs->CHCTLB & 0x10;
        patternNameDataSize = vdp2Controls.m_pendingVdp2Regs->PNCN3 & 0x8000;
        break;
    default:
        assert(0);
        break;
    }

    u32 shitValue;
    if (patternNameDataSize)
    {
        // 1 word
        if (characterSize)
        {
            // 2x2
            shitValue = 11; // 0x800
        }
        else
        {
            // 1x1
            shitValue = 13; // 0x2000
        }
    }
    else
    {
        // 2 words
        if (characterSize)
        {
            // 2x2
            shitValue = 12; // 0x1000
        }
        else
        {
            // 1x1
            shitValue = 14; // 0x4000
        }
    }

    u32 planeAOffset = planeA;// - 0x25E00000;
    u32 planeBOffset = planeB;// - 0x25E00000;
    u32 planeCOffset = planeC;// - 0x25E00000;
    u32 planeDOffset = planeD;// - 0x25E00000;

    u32 mapOffset = (rotateRightR0ByR1(planeAOffset, shitValue + 6)) & 7;

    switch (layer)
    {
    case 0:
        vdp2Controls.m_pendingVdp2Regs->MPOFN = (vdp2Controls.m_pendingVdp2Regs->MPOFN & 0xFFF0) | (mapOffset);
        vdp2Controls.m_pendingVdp2Regs->MPABN0 = ((rotateRightR0ByR1(planeBOffset, shitValue) & 0x3F) << 8) | rotateRightR0ByR1(planeAOffset, shitValue);
        vdp2Controls.m_pendingVdp2Regs->MPCDN0 = ((rotateRightR0ByR1(planeDOffset, shitValue) & 0x3F) << 8) | rotateRightR0ByR1(planeCOffset, shitValue);
        break;
    case 1:
        vdp2Controls.m_pendingVdp2Regs->MPOFN = (vdp2Controls.m_pendingVdp2Regs->MPOFN & 0xFF0F) | (mapOffset << 4);
        vdp2Controls.m_pendingVdp2Regs->MPABN1 = ((rotateRightR0ByR1(planeBOffset, shitValue) & 0x3F) << 8) | rotateRightR0ByR1(planeAOffset, shitValue);
        vdp2Controls.m_pendingVdp2Regs->MPCDN1 = ((rotateRightR0ByR1(planeDOffset, shitValue) & 0x3F) << 8) | rotateRightR0ByR1(planeCOffset, shitValue);
        break;
    case 2:
        vdp2Controls.m_pendingVdp2Regs->MPOFN = (vdp2Controls.m_pendingVdp2Regs->MPOFN & 0xF0FF) | (mapOffset << 8);
        vdp2Controls.m_pendingVdp2Regs->MPABN2 = ((rotateRightR0ByR1(planeBOffset, shitValue) & 0x3F) << 8) | rotateRightR0ByR1(planeAOffset, shitValue);
        vdp2Controls.m_pendingVdp2Regs->MPCDN2 = ((rotateRightR0ByR1(planeDOffset, shitValue) & 0x3F) << 8) | rotateRightR0ByR1(planeCOffset, shitValue);
        break;
    case 3:
        vdp2Controls.m_pendingVdp2Regs->MPOFN = (vdp2Controls.m_pendingVdp2Regs->MPOFN & 0x0FFF) | (mapOffset << 12);
        vdp2Controls.m_pendingVdp2Regs->MPABN3 = ((rotateRightR0ByR1(planeBOffset, shitValue) & 0x3F) << 8) | rotateRightR0ByR1(planeAOffset, shitValue);
        vdp2Controls.m_pendingVdp2Regs->MPCDN3 = ((rotateRightR0ByR1(planeDOffset, shitValue) & 0x3F) << 8) | rotateRightR0ByR1(planeCOffset, shitValue);
        break;
    default:
        assert(0);
    }
}

u32 characterMap1[0x80];
u16 characterMap2[0x1000];

void resetCharacterMap1()
{
    for (int i = 0; i < 0x80; i++)
    {
        characterMap1[i] = 0;
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
    memset(getVdp2Vram(vdp2TextMemoryOffset), 0, 0x10 * 0x1000);
    resetCharacterMaps();
}

void initVdp2TextLayer()
{
    setupNBG3(textLayerVdp2Setup);

    initLayerMap(3, vdp2TextMemoryOffset, vdp2TextMemoryOffset, vdp2TextMemoryOffset, vdp2TextMemoryOffset);

    clearVdp2TextMemory();

    vdp2DebugPrintSetPosition(0, 0);

    vdp2PrintStatus.oldX = vdp2PrintStatus.X;
    vdp2PrintStatus.oldY = vdp2PrintStatus.Y;
    vdp2PrintStatus.palette = 0xC000;
    vdp2PrintStatus.oldPalette = 0xC000;
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

void initVDP2()
{
    vdp2Controls.m_0 = 0;
    vdp2Controls.m_pendingVdp2Regs = &vdp2Controls.m_registers[0];

    resetVdp2RegsCopies();
    clearVdp2VRam(0, 0x80000);
    clearVdp2CRam(0, 0x1000);

    //initVdp2Var1();
    //initVdp2Var2();

    setupVdp2TextLayerColor();
    loadFont();
    initVdp2TextLayer();
    setFontDefaultColors();
}

void reinitVdp2()
{
    vdp2Controls.m_0 = 0;
    vdp2Controls.m_pendingVdp2Regs = &vdp2Controls.m_registers[0];

    resetVdp2RegsCopies();

    //initVdp2Var1();
    //initVdp2Var2();

    initVdp2TextLayer();
    setFontDefaultColors();
}

u8 incrementVar;
void updateVDP2CoordinatesIncrement(u32 unk0, u32 unk1)
{

}
void updateVDP2CoordinatesIncrement2(u32 unk0, u32 unk1)
{

}

int drawLineLargeFont(const char* text)
{
    int result = drawStringLargeFont(text);

    vdp2PrintStatus.X = 0;
    vdp2PrintStatus.Y += 2;
    if (vdp2PrintStatus.Y > 0x3F)
    {
        vdp2PrintStatus.Y = 0;
    }

    return result;
}

int  drawLineSmallFont(const char* string)
{
    int result = drawStringSmallFont(string);

    vdp2PrintStatus.X = 0;
    vdp2PrintStatus.Y++;

    if (vdp2PrintStatus.Y > 0x3F)
    {
        vdp2PrintStatus.Y = 0;
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
    unimplemented("clearVdp2TextMemoryRect");
}

s32 setActiveFont(u16 r4)
{
    sVdp2StringControl* r14 = &vdp2StringControlBuffer;
    u16 fontIndex = pVdp2StringControl->f0_index;
    if (fontIndex != r4)
    {
        do 
        {
            if (r14->f0_index == r4)
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

    pVdp2StringControl->f0_index = 0;
    pVdp2StringControl->field_4 = 0;
    pVdp2StringControl->field_8 = 0;
    pVdp2StringControl->field_C = 0;
    pVdp2StringControl->field_10 = 0;
    pVdp2StringControl->field_14 = 0;
    pVdp2StringControl->field_15 = 0;
    pVdp2StringControl->pPrevious = 0;
    pVdp2StringControl->pNext = 0;
}

struct {
    u32 m0;
    u32 m4; // 4
    u32 m8; // 8
    u32 field_C; // C
}vdp2StringContext2;

void resetVdp2StringContext2()
{
    vdp2StringContext2.m0 = 0;
    vdp2StringContext2.m4 = 0;
    vdp2StringContext2.m8 = 0;
    vdp2StringContext2.field_C = 0;
}

void resetVdp2StringContext()
{
    vdp2StringContext.field_0 = 0;
    vdp2StringContext.cursorX = 0;
    vdp2StringContext.cursorY = 0;
    vdp2StringContext.X = 0;
    vdp2StringContext.Y = 0;
    vdp2StringContext.Width = 44;
    vdp2StringContext.Height = 30;
    vdp2StringContext.field_1C = 0;
    vdp2StringContext.field_20 = 0;
    vdp2StringContext.field_24 = 0;
    vdp2StringContext.field_28 = 0;
    vdp2StringContext.field_2C = 0;
    vdp2StringContext.field_30 = 0;
    vdp2StringContext.field_34 = 0;
    vdp2StringContext.field_38 = 0;
    vdp2StringContext.field_3C = 0x8421;
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

sVdp2StringControl* var_60525E4;

void loadCharacterToVdp2(s16 index, s16 offset)
{
    u8* vdp2FontBase = getVdp2Vram(0x8000);

    s16 r5 = offset - 0x8000;
    u16* r6 = pVdp2StringControl->field_C;

    if (pVdp2StringControl->field_14 == 4)
    {
        assert(0);
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
        unimplemented("loadCharacterToVdp2 because crash in release");
//        loadCharacterToVdp2(r14, r14 + 0x8000);
    } while (r14);
}

s32 resetVdp2StringsSub1(u16* pData)
{
    sVdp2StringControl* pOld = getEndOfvdp2StringControlBufferList();

    sVdp2StringControl* pNew = static_cast<sVdp2StringControl*>(allocateHeap(sizeof(sVdp2StringControl)));
    assert(pNew);

    pNew->f0_index = pOld->f0_index + 1;
    pNew->field_4 = pData[0];
    pNew->field_8 = pData;
    pNew->field_C = 0;
    pNew->field_10 = 0;
    pNew->pPrevious = pOld;
    pNew->pNext = NULL;
    pNew->field_14 = pData[1];

    bool r12 = false;

    switch (pNew->field_14)
    {
    case 4:
        pNew->field_C = pData + 0x10 / 2;
        break;
    case 5:
        pNew->field_C = pData + 0x10 / 2;
        var_60525E4 = pNew;
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

    return pVdp2StringControl->f0_index;
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

    vdp2StringContext.cursorX = x;
    vdp2StringContext.cursorY = y;
    vdp2StringContext.X = x;
    vdp2StringContext.Y = y;
    vdp2StringContext.Width = width;
    vdp2StringContext.Height = height;
}

void VDP2DrawString(const char*)
{
    assert(0);
}

