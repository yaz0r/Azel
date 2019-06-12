#include "PDS.h"

#include "3dEngine_textureCache.h"

#include <unordered_map>

#if !defined(USE_NULL_RENDERER)

struct s_cachedTexture
{
    u16 CMDCTRL;
    u16 CMDPMOD;
    u16 CMDCOLR;
    u16 CMDSRCA;
    u16 CMDSIZE;
    GLuint textureHandle;
};

std::unordered_map<u64, s_cachedTexture> textureCache;

#define SAT2YAB1(alpha,temp)      (alpha << 24 | (temp & 0x1F) << 3 | (temp & 0x3E0) << 6 | (temp & 0x7C00) << 9)

std::unordered_map<u8*, u8*> modelCharacterMap;

void registerModelAndCharacter(u8* model, u8* character)
{
    modelCharacterMap[model] = character;
}

u32* decodeVdp1Quad(s_quad quad, u16& textureWidth, u16& textureHeight)
{
    u8* characterData = NULL;
    u32 paletteOffset = 0;
    auto characterSearch = modelCharacterMap.find(quad.model);
    if (characterSearch != modelCharacterMap.end())
    {
        characterData = characterSearch->second;
    }
    else
    {
        characterData = getVdp1Pointer(0x25C00000);
    }

    u32 textureAddress = ((unsigned int)quad.CMDSRCA) << 3;
    textureWidth = (quad.CMDSIZE & 0x3F00) >> 5;
    textureHeight = quad.CMDSIZE & 0xFF;

    int colorMode = (quad.CMDPMOD >> 3) & 0x7;

    u8 SPD = ((quad.CMDPMOD & 0x40) != 0);
    u8 END = ((quad.CMDPMOD & 0x80) != 0);
    u8 MSB = ((quad.CMDPMOD & 0x8000) != 0);
    u32 alpha = 0xFF;

    int ednmode;
    int endcnt = 0;

    if ((quad.CMDPMOD & 0x20) == 0)
        ednmode = 1;
    else
        ednmode = 0;

    unsigned char* textureOutput = new unsigned char[textureWidth * textureHeight * 4];
    u32* texture = (u32*)textureOutput;

    switch (colorMode)
    {
    case 1:
    {
        // 4 bpp LUT mode
        u16 temp;
        u32 colorLut = quad.CMDCOLR * 8;
        u32 colorOffset = (VDP2Regs_.CRAOFB & 0x70) << 4;
        u16 i;
        int charAddr = textureAddress;

        for (i = 0; i < textureHeight; i++)
        {
            u16 j;
            j = 0;
            int endcnt = 0;
            u32 dot = 0;
            while (j < textureWidth)
            {
                for (int k = 0; k < 2; k++)
                {
                    dot = *(characterData + charAddr);

                    if (k == 0)
                    {
                        dot >>= 4;
                    }
                    else
                    {
                        dot &= 0xF;
                    }

                    if (ednmode && endcnt >= 2)
                    {
                        *texture++ = 0x00;
                    }
                    else if (((dot) == 0) && !SPD)
                    {
                        *texture++ = 0;
                    }
                    else if (((dot) == 0x0F) && !END) // 6. Commandtable end code
                    {
                        *texture++ = 0x0;
                        endcnt++;

                    }
                    else {
                        u32 paletteOffset = colorLut + 2 * dot;//((paletteNumber << 4) + dotColor) * 2 + layerData.CAOS * 0x200;
                        //temp = getVdp1VramU16(0x25C00000 + (((dot) * 2 + colorLut) & 0x7FFFF));
                        temp = READ_BE_U16(characterData + (dot) * 2 + colorLut);

                        if (temp & 0x8000)
                        {
                            if (MSB) *texture++ = (alpha << 24);
                            else *texture++ = SAT2YAB1(alpha, temp);
                        }
                        else if (temp != 0x0000)
                        {
                            //Vdp1ProcessSpritePixel(Vdp2Regs->SPCTL & 0xF, &temp, &shadow, &priority, &colorcl);
                            //if( shadow != 0 ) 
                            {
                                *texture++ = 0x00;
                            }
                            /*else
                            {
                            priority = ((u8 *)&Vdp2Regs->PRISA)[priority]&0x7;
                            colorcl =  ((u8 *)&Vdp2Regs->CCRSA)[colorcl]&0x1F;
                            alpha = 0xF8;
                            if( ((Vdp2Regs->CCCTL >> 6) & 0x01) == 0x01  )
                            {
                            switch( (Vdp2Regs->SPCTL>>12)&0x03 )
                            {
                            case 0:
                            if( priority <= ((Vdp2Regs->SPCTL>>8)&0x07) )
                            alpha = 0xF8-((colorcl<<3)&0xF8);
                            break;
                            case 1:
                            if( priority == ((Vdp2Regs->SPCTL>>8)&0x07) )
                            alpha = 0xF8-((colorcl<<3)&0xF8);
                            break;
                            case 2:
                            if( priority >= ((Vdp2Regs->SPCTL>>8)&0x07) )
                            alpha = 0xF8-((colorcl<<3)&0xF8);
                            break;
                            case 3:
                            //if( priority <= (Vdp2Regs->SPCTL>>8)&0x07 )
                            //   alpha = 0xF8-((colorcl<<3)&0xF8);
                            break;
                            }
                            }
                            alpha |= priority;
                            if( MSB ) *texture++ = (alpha<<24);
                            //else *texture++ = Vdp2ColorRamGetColor(temp+colorOffset, alpha);
                            }*/
                        }
                        else
                        {
                            *texture++ = 0x0;
                        }
                    }
                }
                j += 2;
                charAddr += 1;
            }
        }
        break;
    }
    case 4:
    {
        // 8 bpp(256 color) Bank mode
        u32 colorLut = quad.CMDCOLR * 2;
        u32 colorOffset = (VDP2Regs_.CRAOFB & 0x70) << 4;
        assert(colorOffset == 0);
        for (int currentY = 0; currentY < textureHeight; currentY++)
        {
            for (int currentX = 0; currentX < textureWidth; currentX++)
            {
                u8 character = getVdp1VramU8(0x25C00000 + textureAddress + currentX + currentY * textureWidth);

                if (character)
                {
                    u32 paletteOffset = colorLut + 2 * character;//((paletteNumber << 4) + dotColor) * 2 + layerData.CAOS * 0x200;
                    u16 color = getVdp2CramU16(paletteOffset);
                    u32 finalColor = 0xFF000000 | (((color & 0x1F) << 3) | ((color & 0x03E0) << 6) | ((color & 0x7C00) << 9));

                    *texture++ = finalColor;
                }
                else
                {
                    *texture++ = 0;
                }
            }
        }
        break;
    }
    case 0:
    {
        // 4 bpp Bank mode
        u32 colorLut = quad.CMDCOLR * 2;
        u32 colorOffset = (VDP2Regs_.CRAOFB & 0x70) << 4;
        assert(colorOffset == 0);

        for (int currentY = 0; currentY < textureHeight; currentY++)
        {
            for (int currentX = 0; currentX < textureWidth; currentX++)
            {
                u8 character = getVdp1VramU8(0x25C00000 + textureAddress + (currentX + currentY * textureWidth) / 2);

                if (currentX & 1)
                {
                    character &= 0xF;
                }
                else
                {
                    character = (character >> 4) & 0xF;
                }

                if (character)
                {
                    u32 paletteOffset = colorLut + 2 * character;//((paletteNumber << 4) + dotColor) * 2 + layerData.CAOS * 0x200;
                    u16 color = getVdp2CramU16(paletteOffset);
                    u32 finalColor = 0xFF000000 | (((color & 0x1F) << 3) | ((color & 0x03E0) << 6) | ((color & 0x7C00) << 9));

                    *texture++ = finalColor;
                }
                else
                {
                    *texture++ = 0;
                }
            }
        }
        break;
    }
    default:
        //assert(0);
        break;
    }
    return (u32*)textureOutput;
}


GLuint getTextureForQuad(s_quad& quad)
{
    u64 textureHash = (quad.CMDCTRL | ((u64)quad.CMDPMOD << 16) | ((u64)quad.CMDCOLR << 32) | ((u64)quad.CMDSRCA << 48)) ^ quad.CMDSIZE;

    auto search = textureCache.find(textureHash);
    if (search != textureCache.end())
    {
        s_cachedTexture& cachedEntry = search->second;
        assert((quad.CMDCTRL == cachedEntry.CMDCTRL)
            && (quad.CMDPMOD == cachedEntry.CMDPMOD)
            && (quad.CMDCOLR == cachedEntry.CMDCOLR)
            && (quad.CMDSRCA == cachedEntry.CMDSRCA)
            && (quad.CMDSIZE == cachedEntry.CMDSIZE)
        );

        return cachedEntry.textureHandle;
    }

    s_cachedTexture newTexture;

    newTexture.CMDCTRL = quad.CMDCTRL;
    newTexture.CMDPMOD = quad.CMDPMOD;
    newTexture.CMDCOLR = quad.CMDCOLR;
    newTexture.CMDSRCA = quad.CMDSRCA;
    newTexture.CMDSIZE = quad.CMDSIZE;

    u16 textureWidth;
    u16 textureHeight;
    u32* textureOutput = decodeVdp1Quad(quad, textureWidth, textureHeight);

    GLuint textureHandle;
    glGenTextures(1, &textureHandle);

    newTexture.textureHandle = textureHandle;

#if 0
    AddString(outstring, "Texture address = %08X\r\n", ((unsigned int)cmd.CMDSRCA) << 3);
    AddString(outstring, "Texture width = %d, height = %d\r\n", (cmd.CMDSIZE & 0x3F00) >> 5, cmd.CMDSIZE & 0xFF);
    AddString(outstring, "Texture read direction: ");

    switch ((CMDCTRL >> 4) & 0x3)
    {
    case 0:
        AddString(outstring, "Normal\r\n");
        break;
    case 1:
        AddString(outstring, "Reversed horizontal\r\n");
        break;
    case 2:
        AddString(outstring, "Reversed vertical\r\n");
        break;
    case 3:
        AddString(outstring, "Reversed horizontal and vertical\r\n");
        break;
    default: break;
    }

    // Only draw commands use CMDPMOD
    if (!(cmd.CMDCTRL & 0x0008))
    {
        if (cmd.CMDPMOD & 0x8000)
        {
            AddString(outstring, "MSB set\r\n");
        }

        if (cmd.CMDPMOD & 0x1000)
        {
            AddString(outstring, "High Speed Shrink Enabled\r\n");
        }

        if (!(cmd.CMDPMOD & 0x0800))
        {
            AddString(outstring, "Pre-clipping Enabled\r\n");
        }

        if (cmd.CMDPMOD & 0x0400)
        {
            AddString(outstring, "User Clipping Enabled\r\n");
            AddString(outstring, "Clipping Mode = %d\r\n", (cmd.CMDPMOD >> 9) & 0x1);
        }

        if (cmd.CMDPMOD & 0x0100)
        {
            AddString(outstring, "Mesh Enabled\r\n");
        }

        if (!(cmd.CMDPMOD & 0x0080))
        {
            AddString(outstring, "End Code Enabled\r\n");
        }

        if (!(cmd.CMDPMOD & 0x0040))
        {
            AddString(outstring, "Transparent Pixel Enabled\r\n");
        }

        AddString(outstring, "Color mode: ");

        switch ((cmd.CMDPMOD >> 3) & 0x7)
        {
        case 0:
            AddString(outstring, "4 BPP(16 color bank)\r\n");
            AddString(outstring, "Color bank: %08X\r\n", (cmd.CMDCOLR << 3));
            break;
        case 1:
            AddString(outstring, "4 BPP(16 color LUT)\r\n");
            AddString(outstring, "Color lookup table: %08X\r\n", (cmd.CMDCOLR << 3));
            break;
        case 2:
            AddString(outstring, "8 BPP(64 color bank)\r\n");
            AddString(outstring, "Color bank: %08X\r\n", (cmd.CMDCOLR << 3));
            break;
        case 3:
            AddString(outstring, "8 BPP(128 color bank)\r\n");
            AddString(outstring, "Color bank: %08X\r\n", (cmd.CMDCOLR << 3));
            break;
        case 4:
            AddString(outstring, "8 BPP(256 color bank)\r\n");
            AddString(outstring, "Color bank: %08X\r\n", (cmd.CMDCOLR << 3));
            break;
        case 5:
            AddString(outstring, "15 BPP(RGB)\r\n");

            // Only non-textured commands
            if (cmd.CMDCTRL & 0x0004)
            {
                AddString(outstring, "Non-textured color: %04X\r\n", cmd.CMDCOLR);
            }
            break;
        default: break;
        }

        AddString(outstring, "Color Calc. mode: ");

        switch (cmd.CMDPMOD & 0x7)
        {
        case 0:
            AddString(outstring, "Replace\r\n");
            break;
        case 1:
            AddString(outstring, "Cannot overwrite/Shadow\r\n");
            break;
        case 2:
            AddString(outstring, "Half-luminance\r\n");
            break;
        case 3:
            AddString(outstring, "Replace/Half-transparent\r\n");
            break;
        case 4:
            AddString(outstring, "Gouraud Shading\r\n");
            AddString(outstring, "Gouraud Shading Table = %08X\r\n", ((unsigned int)cmd.CMDGRDA) << 3);
            break;
        case 6:
            AddString(outstring, "Gouraud Shading + Half-luminance\r\n");
            AddString(outstring, "Gouraud Shading Table = %08X\r\n", ((unsigned int)cmd.CMDGRDA) << 3);
            break;
        case 7:
            AddString(outstring, "Gouraud Shading/Gouraud Shading + Half-transparent\r\n");
            AddString(outstring, "Gouraud Shading Table = %08X\r\n", ((unsigned int)cmd.CMDGRDA) << 3);
            break;
        default: break;
        }
    }
#endif

    glBindTexture(GL_TEXTURE_2D, textureHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureOutput);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    textureCache[textureHash] = newTexture;

    return textureHandle;
}

#endif
