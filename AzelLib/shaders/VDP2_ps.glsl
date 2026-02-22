precision highp float;
precision highp int;
in highp vec2 pixelCoordinate;
out vec4 fragColor;

#define u32 highp uint
#define s32 highp int

uniform highp usampler2D s_VDP2_RAM;
uniform highp usampler2D s_VDP2_CRAM;

uniform s32 CHSZ;
uniform s32 CHCN;
uniform s32 PNB;
uniform s32 CNSM;
uniform s32 CAOS;
uniform s32 PLSZ;
uniform s32 SCN;
uniform s32 planeOffsets[4];
uniform s32 scrollX;
uniform s32 scrollY;
uniform s32 lineScrollEA;

uniform int outputHeight;

struct s_layerData
{
    s32 CHSZ;
    s32 CHCN;
    s32 PNB;
    s32 CNSM;
    s32 CAOS;
    s32 PLSZ;
    s32 SCN;

    s32 planeOffsets[4];

    s32 scrollX;
    s32 scrollY;

    s32 lineScrollEA;
};

int readFromSampler(highp usampler2D sampler, s32 offset)
{
    ivec2 position;
    s32 texelOffset;
    texelOffset = offset;
    position.y = texelOffset / 256;
    position.x = texelOffset % 256;
    uvec4 texel = texelFetch(sampler, position, 0);

    s32 texelValue = int(texel.r);

    return texelValue;
}

int getVdp2VramU8(int offset)
{
    return readFromSampler(s_VDP2_RAM, offset);
}

int getVdp2VramU16(int offset)
{
    return (getVdp2VramU8(offset) << 8) | getVdp2VramU8(offset + 1);
}

int getVdp2VramU32(int offset)
{
    return (getVdp2VramU8(offset) << 24) | (getVdp2VramU8(offset + 1) << 16) | (getVdp2VramU8(offset + 2) << 8) | (getVdp2VramU8(offset + 3));
}

int getVdp2CramU8(int offset)
{
    return readFromSampler(s_VDP2_CRAM, offset);
}

int getVdp2CramU16(int offset)
{
    return (getVdp2CramU8(offset) << 8) | getVdp2CramU8(offset + 1);
}

vec4 sampleLayer(int rawOutputX, int rawOutputY, s_layerData layerData)
{
    if(layerData.lineScrollEA != 0)
    {
        int offsetValue = getVdp2VramU32(layerData.lineScrollEA + rawOutputY * 4);
        rawOutputY = offsetValue >> 16;
    }

    int cellDotDimension = 8;
    int characterPatternDotDimension = cellDotDimension * ((layerData.CHSZ == 0) ? 1 : 2);
    int pageDotDimension = 8 * 64; /* A page is always 64x64 cells, so 512 * 512 dots*/ /*characterPatternDimension * ((planeData.CHSZ == 0) ? 64 : 32);*/
    int planeDotWidth = pageDotDimension * (((layerData.PLSZ & 1) != 0) ? 2 : 1);
    int planeDotHeight = pageDotDimension * (((layerData.PLSZ & 2) != 0) ? 2 : 1);
    int mapDotWidth = planeDotWidth * 2; // because scrollScreen
    int mapDotHeight = planeDotHeight * 2; // because scrollScreen

    int pageDimension = (layerData.CHSZ == 0) ? 64 : 32;
    int patternSize = (layerData.PNB == 0) ? 4 : 2;
    int pageSize = pageDimension * pageDimension * patternSize;
    
    int cellSizeInByte = 8 * 8;
    if(layerData.CHCN == 0)
        cellSizeInByte /= 2;
    else if(layerData.CHCN == 1)
        cellSizeInByte *= 1;

    s32 outputX = rawOutputX + layerData.scrollX;
    s32 outputY = rawOutputY + layerData.scrollY;

    if (outputX < 0)
        return vec4(1,0,0,1);
    if (outputY < 0)
        return vec4(1,0,0,1);

    int planeX = outputX / planeDotWidth;
    int planeY = outputY / planeDotHeight;
    int dotInPlaneX = outputX % planeDotWidth;
    int dotInPlaneY = outputY % planeDotHeight;

    int pageX = dotInPlaneX / pageDotDimension;
    int pageY = dotInPlaneY / pageDotDimension;
    int dotInPageX = dotInPlaneX % pageDotDimension;
    int dotInPageY = dotInPlaneY % pageDotDimension;

    int characterPatternX = dotInPageX / characterPatternDotDimension;
    int characterPatternY = dotInPageY / characterPatternDotDimension;
    int dotInCharacterPatternX = dotInPageX % characterPatternDotDimension;
    int dotInCharacterPatternY = dotInPageY % characterPatternDotDimension;

    int cellX = dotInCharacterPatternX / cellDotDimension;
    int cellY = dotInCharacterPatternY / cellDotDimension;
    int dotInCellX = dotInCharacterPatternX % cellDotDimension;
    int dotInCellY = dotInCharacterPatternY % cellDotDimension;

    int planeNumber = planeY * 2 + planeX;
    int startOfPlane = layerData.planeOffsets[planeNumber];

    int pagesPerRow = ((layerData.PLSZ & 1) != 0) ? 2 : 1;
    int pageNumber = pageY * pagesPerRow + pageX;
    int startOfPage = startOfPlane + pageNumber * pageSize;

    int patternNumber = characterPatternY * pageDimension + characterPatternX;
    int startOfPattern = startOfPage + patternNumber * patternSize;

    int characterNumber;
    int paletteNumber;
    int characterOffset;

    if(patternSize == 2)
    {
        int patternName = getVdp2VramU16(startOfPattern);
        int supplementalCharacterName = layerData.SCN;

        // assuming supplement mode 0 with no data
        paletteNumber = (patternName >> 12) & 0xF;

        if(layerData.CNSM == 0)
        {
            if(layerData.CHSZ == 0)
            {
                characterNumber = patternName & 0x3FF;
                characterNumber |= (supplementalCharacterName & 0x1F) << 10;
            }
            if(layerData.CHSZ == 1)
            {
                characterNumber = (patternName & 0x3FF) << 2;
                characterNumber |= supplementalCharacterName & 3;
                characterNumber |= (supplementalCharacterName & 0x1C) << 10;
            }
        }
        if(layerData.CNSM == 1)
        {
            if(layerData.CHSZ == 0)
            {
                characterNumber = patternName & 0xFFF;
                characterNumber |= (supplementalCharacterName & 0x1C) << 10;
            }
            if(layerData.CHSZ == 1)
            {
                characterNumber = (patternName & 0xFFF) << 2;
                characterNumber |= supplementalCharacterName & 3;
                characterNumber |= (supplementalCharacterName & 0x10) << 10;
            }
        }

        characterOffset = (characterNumber) * 0x20;
    }
    else if(patternSize == 4)
    {
        int data1 = getVdp2VramU16(startOfPattern);
        int data2 = getVdp2VramU16(startOfPattern + 2);

        // assuming supplement mode 0 with no data
        characterNumber = data2 & 0x7FFF;
        paletteNumber = data1 & 0x7F;

        characterOffset = characterNumber;
    }
    else
    {
        return vec4(1,0,0,1);
    }

    int cellIndex = cellX + cellY * 2;
    int cellOffset = characterOffset + cellIndex * cellSizeInByte;

    int dotColor = 0;
    int paletteOffset = 0;

    {
        // 16 colors, 4bits
        if(layerData.CHCN == 0)
        {
            int dotOffset = cellOffset + dotInCellY * 4 + dotInCellX / 2;
            dotColor = getVdp2VramU8(dotOffset);

            if ((dotInCellX & 1) != 0)
            {
                dotColor &= 0xF;
            }
            else
            {
                dotColor >>= 4;
            }
        }
        // 256 colors, 8bits
        else if(layerData.CHCN == 1)
        {
            int dotOffset = cellOffset + dotInCellY * 8 + dotInCellX;
            dotColor = getVdp2VramU8(dotOffset);
        }
        else
            return vec4(1,0,0,1);
    }

    if(dotColor != 0)
    {
        paletteOffset = ((paletteNumber << 4) + dotColor) * 2 + layerData.CAOS * 0x200;
        int color = getVdp2CramU16(paletteOffset);
        int finalColor = 0xFF000000 | (((color & 0x1F) << 3) | ((color & 0x03E0) << 6) | ((color & 0x7C00) << 9));

        float R = float(finalColor & 0xFF) / 255.f;
        float G = float((finalColor>>8) & 0xFF) / 255.f;
        float B = float((finalColor>>16) & 0xFF) / 255.f;

        return vec4(R, G, B, 1.f);
    }

    return vec4(0,0,0,0);
}

void main()
{
    s_layerData inputLayerData;
    inputLayerData.CHSZ = CHSZ;
    inputLayerData.CHCN = CHCN;
    inputLayerData.PNB = PNB;
    inputLayerData.CNSM = CNSM;
    inputLayerData.CAOS = CAOS;
    inputLayerData.PLSZ = PLSZ;
    inputLayerData.SCN = SCN;
    inputLayerData.planeOffsets = planeOffsets;
    inputLayerData.scrollX = scrollX;
    inputLayerData.scrollY = scrollY;
    inputLayerData.lineScrollEA = lineScrollEA;

    fragColor = sampleLayer(int(gl_FragCoord.x), outputHeight - int(gl_FragCoord.y), inputLayerData);
}
