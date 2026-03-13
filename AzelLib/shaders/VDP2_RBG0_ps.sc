#include "bgfx_shader.sh"

#define u32 highp uint
#define s32 highp int

#pragma warning(disable : 3556)
#pragma warning(disable : 4000)

USAMPLER2D(s_VDP2_RAM, 0);
USAMPLER2D(s_VDP2_CRAM, 1);
USAMPLER2D(s_planeConfig, 2);
USAMPLER2D(s_coefficientTable, 3);

#define in_CHSZ readFromPanelConfig(0)
#define in_CHCN readFromPanelConfig(1)
#define in_PNB readFromPanelConfig(2)
#define in_CNSM readFromPanelConfig(3)
#define in_CAOS readFromPanelConfig(4)
#define in_PLSZ readFromPanelConfig(5)
#define in_SCN readFromPanelConfig(6)
#define in_planeOffsets_0 readFromPanelConfig(7)
#define in_planeOffsets_1 readFromPanelConfig(8)
#define in_planeOffsets_2 readFromPanelConfig(9)
#define in_planeOffsets_3 readFromPanelConfig(10)
// indices 11-12 unused (scrollX/scrollY are 0 for RBG0)
#define in_outputHeight readFromPanelConfig(13)

// Rotation parameters (16.16 fixed-point unless noted)
#define in_DXx readFromPanelConfig(14)
#define in_DXy readFromPanelConfig(15)
#define in_DYx readFromPanelConfig(16)
#define in_DYy readFromPanelConfig(17)
#define in_Xst readFromPanelConfig(18)
#define in_Yst readFromPanelConfig(19)
#define in_Mx readFromPanelConfig(20)
#define in_My readFromPanelConfig(21)
#define in_Cx readFromPanelConfig(22)
#define in_Cy readFromPanelConfig(23)
#define in_coeffCount readFromPanelConfig(24)
#define in_scrollMode readFromPanelConfig(25)

// Rotation matrix (16.16 fixed-point)
#define in_A readFromPanelConfig(26)
#define in_B readFromPanelConfig(27)
#define in_D readFromPanelConfig(28)
#define in_E readFromPanelConfig(29)
#define in_Px readFromPanelConfig(30)
#define in_Py readFromPanelConfig(31)

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
};

int readFromPanelConfig(s32 offset)
{
    ivec2 position;
    position.x = offset;
    position.y = 0;
    uvec4 texel = texelFetch(s_planeConfig, position, 0);
    s32 texelValue = int(texel.r);

    return texelValue;
}

int readFromSampler_VDP2_RAM(s32 offset)
{
    ivec2 position;
    s32 texelOffset;
    texelOffset = offset;
    position.y = texelOffset / 256;
    position.x = texelOffset % 256;
    uvec4 texel = texelFetch(s_VDP2_RAM, position, 0);

    s32 texelValue = int(texel.r);

    return texelValue;
}

int readFromSampler_VDP2_CRAM(s32 offset)
{
    ivec2 position;
    s32 texelOffset;
    texelOffset = offset;
    position.y = texelOffset / 256;
    position.x = texelOffset % 256;
    uvec4 texel = texelFetch(s_VDP2_CRAM, position, 0);

    s32 texelValue = int(texel.r);

    return texelValue;
}


int getVdp2VramU8(int offset)
{
    return readFromSampler_VDP2_RAM(offset);
}

int getVdp2VramU16(int offset)
{
    return (getVdp2VramU8(offset) << 8) | getVdp2VramU8(offset + 1);
}

int getVdp2CramU8(int offset)
{
    return readFromSampler_VDP2_CRAM(offset);
}

int getVdp2CramU16(int offset)
{
    return (getVdp2CramU8(offset) << 8) | getVdp2CramU8(offset + 1);
}

vec4 sampleLayer(int rawOutputX, int rawOutputY, s_layerData layerData)
{
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

    // Wrap coordinates into the map
    outputX = outputX % mapDotWidth;
    outputY = outputY % mapDotHeight;
    if (outputX < 0)
        outputX += mapDotWidth;
    if (outputY < 0)
        outputY += mapDotHeight;

    int planeX = outputX / planeDotWidth;
    int planeY = outputY / planeDotHeight;
    int dotInPlaneX = outputX % planeDotWidth;
    int dotInPlaneY = outputY % planeDotWidth;

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

    int planeNumber = planeY * (((layerData.PLSZ & 1) != 0) ? 2 : 1) + planeX;
    int startOfPlane = layerData.planeOffsets[planeNumber];

    int pageNumber = pageY * pageDimension + pageX;
    int startOfPage = startOfPlane + pageNumber * pageSize;

    int patternNumber = characterPatternY * pageDimension + characterPatternX;
    int startOfPattern = startOfPage + patternNumber * patternSize;

    int characterNumber = 0;
    int paletteNumber = 0;
    int characterOffset = 0;

    if(patternSize == 2)
    {
        int patternName = getVdp2VramU16(startOfPattern);
        int supplementalCharacterName = layerData.SCN;

        // assuming supplement mode 0 with no data
        if(layerData.CHCN == 0)
        {
            paletteNumber = (patternName >> 12) & 0xF;
        }
        else
        {
            paletteNumber = (patternName >> 8) & 0x70;
        }

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
    inputLayerData.CHSZ = in_CHSZ;
    inputLayerData.CHCN = in_CHCN;
    inputLayerData.PNB = in_PNB;
    inputLayerData.CNSM = in_CNSM;
    inputLayerData.CAOS = in_CAOS;
    inputLayerData.PLSZ = in_PLSZ;
    inputLayerData.SCN = in_SCN;
    inputLayerData.planeOffsets[0] = in_planeOffsets_0;
    inputLayerData.planeOffsets[1] = in_planeOffsets_1;
    inputLayerData.planeOffsets[2] = in_planeOffsets_2;
    inputLayerData.planeOffsets[3] = in_planeOffsets_3;
    inputLayerData.scrollX = 0;
    inputLayerData.scrollY = 0;

    int screenX = int(gl_FragCoord.x);
    int screenY = int(gl_FragCoord.y);

    // Coefficient table lookup: index by scanline (Y) or column (X)
    int coeffIdx = (in_scrollMode == 0) ? screenY : screenX;
    if (coeffIdx < 0 || coeffIdx >= in_coeffCount) {
        gl_FragColor = vec4(0,0,0,0);
        return;
    }
    u32 rawKA = texelFetch(s_coefficientTable, ivec2(coeffIdx % 256, coeffIdx / 256), 0).r;
    int KA = int(rawKA);

    // VDP2 rotation formula (16.16 fixed-point)
    // Coefficient scales displacement from center, not the starting position
    int dispX = in_DXx * (screenX - in_Cx) + in_DXy * (screenY - in_Cy);
    int dispY = in_DYx * (screenX - in_Cx) + in_DYy * (screenY - in_Cy);

    float fKA = float(KA) / 65536.0;
    int Xp = in_Xst + int(float(dispX) * fKA);
    int Yp = in_Yst + int(float(dispY) * fKA);

    // Apply rotation matrix A/B/D/E around pivot (Px, Py)
    int dx = Xp - in_Px * 65536;
    int dy = Yp - in_Py * 65536;
    float fA = float(in_A) / 65536.0;
    float fB = float(in_B) / 65536.0;
    float fD = float(in_D) / 65536.0;
    float fE = float(in_E) / 65536.0;
    int mapX = int(fA * float(dx) + fB * float(dy)) + in_Px * 65536 + in_Mx;
    int mapY = int(fD * float(dx) + fE * float(dy)) + in_Py * 65536 + in_My;

    // Convert 16.16 fixed-point to pixel coordinates and sample tile
    gl_FragColor = sampleLayer(mapX >> 16, mapY >> 16, inputLayerData);
}
