#include "PDS.h"
#include "renderer/renderer_gl.h"
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
    bgfx::TextureHandle m_handle;
};

std::unordered_map<u64, s_cachedTexture> textureCache;

#define SAT2YAB1(alpha,temp)      (alpha << 24 | (temp & 0x1F) << 3 | (temp & 0x3E0) << 6 | (temp & 0x7C00) << 9)

std::unordered_map<u8*, u32> modelCharacterMap;

void registerModelAndCharacter(u8* model, u32 character)
{
    assert(character >= 0x25C00000);
    assert(character <= 0x25C7FFFF);
    modelCharacterMap[model] = character;
}

u32* decodeVdp1Quad(s_quad quad, u16& textureWidth, u16& textureHeight)
{
    u8* characterData = NULL;
    u32 paletteOffset = 0;
    auto characterSearch = modelCharacterMap.find(quad.model);
    if (characterSearch != modelCharacterMap.end())
    {
        characterData = getVdp1Pointer(characterSearch->second);
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
        u32 colorOffset = (VDP2Regs_.mE6_CRAOFB & 0x70) << 4;
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
        u32 colorOffset = (VDP2Regs_.mE6_CRAOFB & 0x70) << 4;
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
        u32 colorLut = quad.CMDCOLR;
        u32 colorOffset = (VDP2Regs_.mE6_CRAOFB & 0x70) << 4;
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
                    u32 paletteOffset = character | colorLut;//((paletteNumber << 4) + dotColor) * 2 + layerData.CAOS * 0x200;
                    u16 color = getVdp2CramU16(paletteOffset * 2);
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

bgfx::TextureHandle getTextureForQuadBGFX(s_quad& quad)
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

        return cachedEntry.m_handle;
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

    const uint64_t tsFlags = 0
        | BGFX_SAMPLER_U_CLAMP
        | BGFX_SAMPLER_V_CLAMP
        ;

    newTexture.m_handle = bgfx::createTexture2D(textureWidth, textureHeight, false, 1, bgfx::TextureFormat::RGBA8, tsFlags, bgfx::copy(textureOutput, textureWidth * textureHeight * 4));

    textureCache[textureHash] = newTexture;

    return newTexture.m_handle;
}

#endif
