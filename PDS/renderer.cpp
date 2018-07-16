#include "PDS.h"

#include "GL/gl3w.h"

#define IMGUI_API

#include "imgui_impl_sdl_gl3.h"

#ifdef _WIN32
#pragma comment(lib, "Opengl32.lib")
#endif

SDL_Window *gWindow;
SDL_GLContext gGlcontext;

GLuint gVdp1Texture = 0;
GLuint gNBG0Texture = 0;
GLuint gNBG1Texture = 0;
GLuint gNBG2Texture = 0;
GLuint gNBG3Texture = 0;

void azelSdl2_Init()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        assert(false);
    }

    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    gWindow = SDL_CreateWindow("PDS: Azel", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    assert(gWindow);

    gGlcontext = SDL_GL_CreateContext(gWindow);
    assert(gGlcontext);

    gl3wInit();

    // Setup ImGui binding
    ImGui_ImplSdlGL3_Init(gWindow);

    glGenTextures(1, &gVdp1Texture);
    glGenTextures(1, &gNBG0Texture);
    glGenTextures(1, &gNBG1Texture);
    glGenTextures(1, &gNBG2Texture);
    glGenTextures(1, &gNBG3Texture);
}

bool closeApp = false;

void azelSdl2_StartFrame()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSdlGL3_ProcessEvent(&event);
        if (event.type == SDL_QUIT)
            closeApp = true;
    }

    graphicEngineStatus.m4514.m0[0].m16_pending.field_8 = 0;
    graphicEngineStatus.m4514.m0[0].m16_pending.field_C = 0;

    const Uint8* keyState = SDL_GetKeyboardState(NULL);

    //if (event.type == SDL_KEYDOWN)
    {
        if (keyState[SDL_SCANCODE_RETURN])
        {
            graphicEngineStatus.m4514.m0[0].m16_pending.field_8 |= 8;
        }
        if (keyState[SDL_SCANCODE_Z])
        {
            graphicEngineStatus.m4514.m0[0].m16_pending.field_8 |= 0x4;
        }
        if (keyState[SDL_SCANCODE_X])
        {
            graphicEngineStatus.m4514.m0[0].m16_pending.field_8 |= 0x2;
        }
        if (keyState[SDL_SCANCODE_C])
        {
            graphicEngineStatus.m4514.m0[0].m16_pending.field_8 |= 0x1;
        }
        if (keyState[SDL_SCANCODE_UP])
        {
            graphicEngineStatus.m4514.m0[0].m16_pending.field_8 |= 0x10;
            graphicEngineStatus.m4514.m0[0].m16_pending.field_C |= 0x10;
        }
        if (ImGui::GetIO().KeysDown[SDL_SCANCODE_DOWN])
        {
            graphicEngineStatus.m4514.m0[0].m16_pending.field_8 |= 0x20;
            graphicEngineStatus.m4514.m0[0].m16_pending.field_C |= 0x20;
        }
        if (keyState[SDL_SCANCODE_LEFT])
        {
            graphicEngineStatus.m4514.m0[0].m16_pending.field_8 |= 0x40;
            graphicEngineStatus.m4514.m0[0].m16_pending.field_C |= 0x40;
        }
        if (keyState[SDL_SCANCODE_RIGHT])
        {
            graphicEngineStatus.m4514.m0[0].m16_pending.field_8 |= 0x80;
            graphicEngineStatus.m4514.m0[0].m16_pending.field_C |= 0x80;
        }
    }

    ImGui_ImplSdlGL3_NewFrame(gWindow);
}

ImVec4 clear_color = ImColor(114, 144, 154);

void computePosition(u32 patternX, u32 patternY, u32 patternDimension, u32 cellX, u32 cellY, u32 cellDimension, u32 dotX, u32 dotY, u32 dotDimension, u32* outputX, u32* outputY)
{
    u32 cellStride = dotDimension;
    u32 patternStride = dotDimension * cellDimension;
    u32 pageStride = patternStride * patternDimension;

    *outputX = dotX + cellX * cellStride + patternX * patternStride;
    *outputY = dotY + cellY * cellStride + patternY * patternStride;
}

struct s_layerData
{
    u32 CHSZ;
    u32 CHCN;
    u32 PNB;
    u32 CNSM;
    u32 CAOS;
    u32 PLSZ;
    u32 SCN;

    u32 planeOffsets[4];

    s32 scrollX;
    s32 scrollY;
};

void renderLayer(s_layerData& layerData, u32 textureWidth, u32 textureHeight, u32* textureOutput)
{
    u32 cellDotDimension = 8;
    u32 characterPatternDotDimension = cellDotDimension * ((layerData.CHSZ == 0) ? 1 : 2);
    u32 pageDotDimension = 8 * 64; /* A page is always 64x64 cells, so 512 * 512 dots*/ /*characterPatternDimension * ((planeData.CHSZ == 0) ? 64 : 32);*/
    u32 planeDotWidth = pageDotDimension * ((layerData.PLSZ & 1) ? 2 : 1);
    u32 planeDotHeight = pageDotDimension * ((layerData.PLSZ & 2) ? 2 : 1);
    u32 mapDotWidth = planeDotWidth * 2; // because scrollScreen
    u32 mapDotHeight = planeDotHeight * 2; // because scrollScreen

    u32 pageDimension = (layerData.CHSZ == 0) ? 64 : 32;
    u32 patternSize = (layerData.PNB == 0) ? 4 : 2;
    u32 pageSize = pageDimension * pageDimension * patternSize;

    u32 cellSizeInByte = 8 * 8;
    switch (layerData.CHCN)
    {
        // 4bpp
    case 0:
        cellSizeInByte /= 2;
        break;
        // 8bpp
    case 1:
        cellSizeInByte *= 1;
        break;
    }

    for (u32 rawOutputY = 0; rawOutputY < textureHeight; rawOutputY++)
    {
        for (u32 rawOutputX = 0; rawOutputX < textureWidth; rawOutputX++)
        {
            s32 outputX = rawOutputX + layerData.scrollX;
            s32 outputY = rawOutputY + layerData.scrollY;

            if (outputX < 0)
                continue;
            if (outputY < 0)
                continue;

            u32 planeX = outputX / planeDotWidth;
            u32 planeY = outputY / planeDotHeight;
            u32 dotInPlaneX = outputX % planeDotWidth;
            u32 dotInPlaneY = outputY % planeDotWidth;

            u32 pageX = dotInPlaneX / pageDotDimension;
            u32 pageY = dotInPlaneY / pageDotDimension;
            u32 dotInPageX = dotInPlaneX % pageDotDimension;
            u32 dotInPageY = dotInPlaneY % pageDotDimension;

            u32 characterPatternX = dotInPageX / characterPatternDotDimension;
            u32 characterPatternY = dotInPageY / characterPatternDotDimension;
            u32 dotInCharacterPatternX = dotInPageX % characterPatternDotDimension;
            u32 dotInCharacterPatternY = dotInPageY % characterPatternDotDimension;

            u32 cellX = dotInCharacterPatternX / cellDotDimension;
            u32 cellY = dotInCharacterPatternY / cellDotDimension;
            u32 dotInCellX = dotInCharacterPatternX % cellDotDimension;
            u32 dotInCellY = dotInCharacterPatternY % cellDotDimension;

            u32 planeNumber = planeY * ((layerData.PLSZ & 1) ? 2 : 1) + planeX;
            u32 startOfPlane = layerData.planeOffsets[planeNumber];

            u32 pageNumber = pageY * pageDimension + pageX;
            u32 startOfPage = startOfPlane + pageNumber * pageSize;

            u32 patternNumber = characterPatternY * pageDimension + characterPatternX;
            u32 startOfPattern = startOfPage + patternNumber * patternSize;

            u32 characterNumber;
            u32 paletteNumber;
            u32 characterOffset;

            switch (patternSize)
            {
            case 2:
            {
                u16 patternName = getVdp2VramU16(startOfPattern);
                u16 supplementalCharacterName = layerData.SCN;

                // assuming supplement mode 0 with no data
                paletteNumber = (patternName >> 12) & 0xF;

                switch (layerData.CNSM)
                {
                case 0:
                    switch (layerData.CHSZ)
                    {
                    case 0:
                        characterNumber = patternName & 0x3FF;
                        characterNumber |= (supplementalCharacterName & 0x1F) << 10;
                        break;
                    case 1:
                        characterNumber = (patternName & 0x3FF) << 2;
                        characterNumber |= supplementalCharacterName & 3;
                        characterNumber |= (supplementalCharacterName & 0x1C) << 10;
                        break;
                    }
                    break;
                case 1:
                    switch (layerData.CHSZ)
                    {
                    case 0:
                        characterNumber = patternName & 0xFFF;
                        characterNumber |= (supplementalCharacterName & 0x1C) << 10;
                        break;
                    case 1:
                        characterNumber = (patternName & 0xFFF) << 2;
                        characterNumber |= supplementalCharacterName & 3;
                        characterNumber |= (supplementalCharacterName & 0x10) << 10;
                        break;
                    }
                    break;
                }

                characterOffset = (characterNumber) * 0x20;

                if (characterNumber)
                {
                    characterNumber = characterNumber;
                }
                break;
            }
            case 4:
            {
                u16 data1 = getVdp2VramU16(startOfPattern);
                u16 data2 = getVdp2VramU16(startOfPattern + 2);

                // assuming supplement mode 0 with no data
                characterNumber = data2 & 0x7FFF;
                paletteNumber = data1 & 0x7F;

                characterOffset = characterNumber;
                break;
            }
            default:
                assert(0);
            }

            u32 cellIndex = cellX + cellY * 2;
            u32 cellOffset = characterOffset + cellIndex * cellSizeInByte;

            u8 dotColor = 0;
            u32 paletteOffset = 0;

            switch (layerData.CHCN)
            {
                // 16 colors, 4bits
                case 0:
                {
                    u32 dotOffset = cellOffset + dotInCellY * 4 + dotInCellX / 2;
                    dotColor = getVdp2VramU8(dotOffset);

                    if (dotInCellX & 1)
                    {
                        dotColor &= 0xF;
                    }
                    else
                    {
                        dotColor >>= 4;
                    }
                    break;
                }
                // 256 colors, 8bits
                case 1:
                {
                    u32 dotOffset = cellOffset + dotInCellY * 8 + dotInCellX;
                    dotColor = getVdp2VramU8(dotOffset);
                    break;
                }
                default:
                    assert(0);
            }

            if(dotColor)
            {
                paletteOffset = ((paletteNumber << 4) + dotColor) * 2 + layerData.CAOS * 0x200;
                u16 color = getVdp2CramU16(paletteOffset);
                u32 finalColor = 0xFF000000 | (((color & 0x1F) << 3) | ((color & 0x03E0) << 6) | ((color & 0x7C00) << 9));

                textureOutput[rawOutputY * textureWidth + rawOutputX] = finalColor;
            }
        }
    }
}

void renderBG0(u32 width, u32 height)
{
    u32 textureWidth = width;
    u32 textureHeight = height;

    u32* textureOutput = new u32[textureWidth * textureHeight];
    memset(textureOutput, 0x80, textureWidth * textureHeight * 4);

    if(vdp2Controls.m_pendingVdp2Regs->BGON & 0x1)
    {
        s_layerData planeData;
        planeData.CHSZ = vdp2Controls.m_pendingVdp2Regs->CHCTLA & 1;
        planeData.CHCN = (vdp2Controls.m_pendingVdp2Regs->CHCTLA >> 4) & 7;
        planeData.PNB = (vdp2Controls.m_pendingVdp2Regs->PNCN0 >> 15) & 0x1;
        planeData.CNSM = (vdp2Controls.m_pendingVdp2Regs->PNCN0 >> 14) & 0x1;
        planeData.CAOS = (vdp2Controls.m_pendingVdp2Regs->CRAOFA) & 0x7;
        planeData.PLSZ = (vdp2Controls.m_pendingVdp2Regs->PLSZ) & 3;
        planeData.SCN = (vdp2Controls.m_pendingVdp2Regs->PNCN0) & 0x1F;
        planeData.scrollX = vdp2Controls.m_pendingVdp2Regs->SCXN0 >> 16;
        planeData.scrollY = vdp2Controls.m_pendingVdp2Regs->SCYN0 >> 16;

        u32 pageDimension = (planeData.CHSZ == 0) ? 64 : 32;
        u32 patternSize = (planeData.PNB == 0) ? 4 : 2;

        u32 pageSize = pageDimension * pageDimension * patternSize;

        u32 offset = (vdp2Controls.m_pendingVdp2Regs->MPOFN & 7) << 6;

        planeData.planeOffsets[0] = (offset + (vdp2Controls.m_pendingVdp2Regs->MPABN0 & 0x3F)) * pageSize;
        planeData.planeOffsets[1] = (offset + ((vdp2Controls.m_pendingVdp2Regs->MPABN0 >> 8) & 0x3F)) * pageSize;
        planeData.planeOffsets[2] = (offset + (vdp2Controls.m_pendingVdp2Regs->MPCDN0 & 0x3F)) * pageSize;
        planeData.planeOffsets[3] = (offset + ((vdp2Controls.m_pendingVdp2Regs->MPCDN0 >> 8) & 0x3F)) * pageSize;

        renderLayer(planeData, textureWidth, textureHeight, textureOutput);
    }

    glBindTexture(GL_TEXTURE_2D, gNBG0Texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureOutput);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    delete[] textureOutput;

}

void renderBG1(u32 width, u32 height)
{
    u32 textureWidth = width;
    u32 textureHeight = height;

    u32* textureOutput = new u32[textureWidth * textureHeight];
    memset(textureOutput, 0x80, textureWidth * textureHeight * 4);

    if (vdp2Controls.m_pendingVdp2Regs->BGON & 0x2)
    {
        s_layerData planeData;

        planeData.CHSZ = (vdp2Controls.m_pendingVdp2Regs->CHCTLA >> 8) & 0x1;
        planeData.CHCN = (vdp2Controls.m_pendingVdp2Regs->CHCTLA >> 12) & 3;
        planeData.PNB = (vdp2Controls.m_pendingVdp2Regs->PNCN1 >> 15) & 0x1;
        planeData.CNSM = (vdp2Controls.m_pendingVdp2Regs->PNCN1 >> 14) & 0x1;
        planeData.CAOS = (vdp2Controls.m_pendingVdp2Regs->CRAOFA >> 4) & 0x7;
        planeData.PLSZ = (vdp2Controls.m_pendingVdp2Regs->PLSZ >> 2) & 3;
        planeData.SCN = (vdp2Controls.m_pendingVdp2Regs->PNCN1) & 0x1F;
        planeData.scrollX = vdp2Controls.m_pendingVdp2Regs->SCXN1 >> 16;
        planeData.scrollY = vdp2Controls.m_pendingVdp2Regs->SCYN1 >> 16;

        u32 pageDimension = (planeData.CHSZ == 0) ? 64 : 32;
        u32 patternSize = (planeData.PNB == 0) ? 4 : 2;

        u32 pageSize = pageDimension * pageDimension * patternSize;

        u32 offset = ((vdp2Controls.m_pendingVdp2Regs->MPOFN >> 4) & 7) << 6;

        planeData.planeOffsets[0] = (offset + (vdp2Controls.m_pendingVdp2Regs->MPABN1 & 0x3F)) * pageSize;
        planeData.planeOffsets[1] = (offset + ((vdp2Controls.m_pendingVdp2Regs->MPABN1 >> 8) & 0x3F)) * pageSize;
        planeData.planeOffsets[2] = (offset + (vdp2Controls.m_pendingVdp2Regs->MPCDN1 & 0x3F)) * pageSize;
        planeData.planeOffsets[3] = (offset + ((vdp2Controls.m_pendingVdp2Regs->MPCDN1 >> 8) & 0x3F)) * pageSize;

        renderLayer(planeData, textureWidth, textureHeight, textureOutput);
    }

    glBindTexture(GL_TEXTURE_2D, gNBG1Texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureOutput);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    delete[] textureOutput;
}

void renderBG2()
{
    /*
    u32 textureWidth = 1024;
    u32 textureHeight = 1024;

    u32* textureOutput = new u32[textureWidth * textureHeight];
    memset(textureOutput, 0x80, textureWidth * textureHeight * 4);

    s_planeData planeData;

    planeData.CHSZ = vdp2Controls.m_pendingVdp2Regs->CHCTLB & 1;
    planeData.PNB = vdp2Controls.m_pendingVdp2Regs->PNCN2 & 0x8000;

    planeData.pageDimension = (planeData.CHSZ == 0) ? 64 : 32;
    planeData.patternSize = (planeData.PNB == 0) ? 4 : 2;

    planeData.CAOS = (vdp2Controls.m_pendingVdp2Regs->CRAOFA >> 8) & 0x7;

    planeData.pageSize = planeData.pageDimension * planeData.pageDimension * planeData.patternSize;

    planeData.planeOffsets[0] = (vdp2Controls.m_pendingVdp2Regs->MPABN2 & 0x3F) * planeData.pageSize;
    planeData.planeOffsets[1] = ((vdp2Controls.m_pendingVdp2Regs->MPABN2 >> 8) & 0x3F) * planeData.pageSize;
    planeData.planeOffsets[2] = (vdp2Controls.m_pendingVdp2Regs->MPCDN2 & 0x3F) * planeData.pageSize;
    planeData.planeOffsets[3] = ((vdp2Controls.m_pendingVdp2Regs->MPCDN2 >> 8) & 0x3F) * planeData.pageSize;

    renderPlane(planeData, textureWidth, textureHeight, textureOutput);

    glBindTexture(GL_TEXTURE_2D, gNBG2Texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureOutput);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    delete[] textureOutput;
    */
}

void renderBG3(u32 width, u32 height)
{
    u32 textureWidth = width;
    u32 textureHeight = height;

    u32* textureOutput = new u32[textureWidth * textureHeight];
    memset(textureOutput, 0x80, textureWidth * textureHeight * 4);

    if (vdp2Controls.m_pendingVdp2Regs->BGON & 0x8)
    {
        s_layerData planeData;

        planeData.CHSZ = (vdp2Controls.m_pendingVdp2Regs->CHCTLB >> 4) & 0x1;
        planeData.CHCN = (vdp2Controls.m_pendingVdp2Regs->CHCTLB >> 5) & 0x1;
        planeData.PNB = (vdp2Controls.m_pendingVdp2Regs->PNCN3 >> 15) & 0x1;
        planeData.CNSM = (vdp2Controls.m_pendingVdp2Regs->PNCN3 >> 14) & 0x1;
        planeData.CAOS = (vdp2Controls.m_pendingVdp2Regs->CRAOFA >> 12) & 0x7;
        planeData.PLSZ = (vdp2Controls.m_pendingVdp2Regs->PLSZ >> 6) & 3;
        planeData.SCN = (vdp2Controls.m_pendingVdp2Regs->PNCN3) & 0x1F;
        planeData.scrollX = vdp2Controls.m_pendingVdp2Regs->SCXN3;
        planeData.scrollY = vdp2Controls.m_pendingVdp2Regs->SCYN3;

        u32 pageDimension = (planeData.CHSZ == 0) ? 64 : 32;
        u32 patternSize = (planeData.PNB == 0) ? 4 : 2;

        u32 pageSize = pageDimension * pageDimension * patternSize;

        u32 offset = ((vdp2Controls.m_pendingVdp2Regs->MPOFN >> 12) & 7) << 6;

        planeData.planeOffsets[0] = (offset + (vdp2Controls.m_pendingVdp2Regs->MPABN3 & 0x3F)) * pageSize;
        planeData.planeOffsets[1] = (offset + ((vdp2Controls.m_pendingVdp2Regs->MPABN3 >> 8) & 0x3F)) * pageSize;
        planeData.planeOffsets[2] = (offset + (vdp2Controls.m_pendingVdp2Regs->MPCDN3 & 0x3F)) * pageSize;
        planeData.planeOffsets[3] = (offset + ((vdp2Controls.m_pendingVdp2Regs->MPCDN3 >> 8) & 0x3F)) * pageSize;

        renderLayer(planeData, textureWidth, textureHeight, textureOutput);
    }

    glBindTexture(GL_TEXTURE_2D, gNBG3Texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureOutput);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    delete[] textureOutput;
}

u32* vdp1TextureOutput;
u32 vdp1TextureWidth;
u32 vdp1TextureHeight;

s16 localCoordiantesX;
s16 localCoordiantesY;

void SetLocalCoordinates(u32 vdp1EA)
{
    u16 CMDXA = getVdp1VramU16(vdp1EA + 0xC);
    u16 CMDYA = getVdp1VramU16(vdp1EA + 0xE);

    localCoordiantesX = CMDXA;
    localCoordiantesY = CMDYA;
}

void NormalSpriteDraw(u32 vdp1EA)
{
    u16 CMDPMOD = getVdp1VramU16(vdp1EA + 4);
    u16 CMDCOLR = getVdp1VramU16(vdp1EA + 6);
    u16 CMDSRCA = getVdp1VramU16(vdp1EA + 8);
    u16 CMDSIZE = getVdp1VramU16(vdp1EA + 0xA);
    s16 CMDXA = getVdp1VramU16(vdp1EA + 0xC);
    s16 CMDYA = getVdp1VramU16(vdp1EA + 0xE);
    u16 CMDGRDA = getVdp1VramU16(vdp1EA + 0x1C);

    if (CMDSRCA)
    {
        u32 characterAddress = ((u32)CMDSRCA) << 3;
        u32 colorBank = ((u32)CMDCOLR) << 1;
        s32 X = CMDXA + localCoordiantesX;
        s32 Y = CMDYA + localCoordiantesY;
        s32 Width = ((CMDSIZE >> 8) & 0x3F) * 8;
        s32 Height = CMDSIZE & 0xFF;

        int counter = 0;

        for (int currentY = Y; currentY < Y + Height; currentY++)
        {
            for (int currentX = X; currentX < X + Width; currentX++)
            {
                if ((currentX >= 0) && (currentX < vdp1TextureWidth) && (currentY >= 0) && (currentY < vdp1TextureHeight))
                {
                    u8 character = getVdp1VramU8(0x25C00000 + characterAddress);

                    if (counter & 1)
                    {
                        characterAddress++;
                    }
                    else
                    {
                        character >>= 4;
                    }
                    character &= 0xF;

                    if(character)
                    {
                        u32 paletteOffset = colorBank + 2 * character;//((paletteNumber << 4) + dotColor) * 2 + layerData.CAOS * 0x200;
                        u16 color = getVdp2CramU16(paletteOffset);
                        u32 finalColor = 0xFF000000 | (((color & 0x1F) << 3) | ((color & 0x03E0) << 6) | ((color & 0x7C00) << 9));

                        vdp1TextureOutput[currentY * vdp1TextureWidth + currentX] = finalColor;
                    }

                    counter++;
                }
            }
        }
    }
}

void renderVdp1(u32 width, u32 height)
{
    vdp1TextureWidth = width;
    vdp1TextureHeight = height;
    vdp1TextureOutput = new u32[vdp1TextureWidth * vdp1TextureHeight];
    memset(vdp1TextureOutput, 0x80, vdp1TextureWidth * vdp1TextureHeight * 4);

    u32 vdp1EA = 0x25C00000;

    while (1)
    {
        u16 CMDCTRL = getVdp1VramU16(vdp1EA);
        u16 CMDLINK = getVdp1VramU16(vdp1EA+2);

        u16 END = CMDCTRL >> 15;
        u16 JP = (CMDCTRL >> 12) & 7;
        u16 ZP = (CMDCTRL >> 8) & 0xF;
        u16 DIR = (CMDCTRL >> 4) & 3;
        u16 COMM = CMDCTRL & 0xF;

        if (END)
        {
            break;
        }

        switch (COMM)
        {
        case 0:
            NormalSpriteDraw(vdp1EA);
            break;
        case 0xA:
            SetLocalCoordinates(vdp1EA);
            break;
        default:
            break;
        }

        switch (JP)
        {
        case 0:
            vdp1EA += 0x20;
            break;
        case 1:
            vdp1EA = 0x25C00000 + (CMDLINK << 3);
            break;
        default:
            assert(0);
        }
        
    }

    glBindTexture(GL_TEXTURE_2D, gVdp1Texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, vdp1TextureWidth, vdp1TextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, vdp1TextureOutput);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    delete[] vdp1TextureOutput;
}

bool azelSdl2_EndFrame()
{
    u32 outputResolutionWidth = 0;
    u32 outputResolutionHeight = 0;

    u32 LSMD = (vdp2Controls.m_pendingVdp2Regs->TVMD >> 6) & 3;
    u32 VRESO = (vdp2Controls.m_pendingVdp2Regs->TVMD >> 4) & 3;
    u32 HRESO = (vdp2Controls.m_pendingVdp2Regs->TVMD) & 7;

    switch (VRESO)
    {
    case 0:
        outputResolutionHeight = 224;
        break;
    default:
        assert(0);
        break;
    }

    switch (HRESO)
    {
    case 1:
        outputResolutionWidth = 352;
        break;
    default:
        assert(0);
        break;
    }

    u32 vdp2ResolutionWidth = outputResolutionWidth;
    u32 vdp2ResolutionHeight = outputResolutionHeight;

    renderVdp1(outputResolutionWidth, outputResolutionHeight);

    renderBG0(vdp2ResolutionWidth, vdp2ResolutionHeight);
    renderBG1(vdp2ResolutionWidth, vdp2ResolutionHeight);
    //renderBG2(vdp2ResolutionWidth, vdp2ResolutionHeight);
    renderBG3(vdp2ResolutionWidth, vdp2ResolutionHeight);

    if(ImGui::Begin(""))
    {
        ImGui::Image((ImTextureID)gNBG0Texture, ImVec2(vdp2ResolutionWidth, vdp2ResolutionHeight)); ImGui::SameLine();
        ImGui::Image((ImTextureID)gNBG1Texture, ImVec2(vdp2ResolutionWidth, vdp2ResolutionHeight));
        ImGui::Image((ImTextureID)gNBG2Texture, ImVec2(vdp2ResolutionWidth, vdp2ResolutionHeight)); ImGui::SameLine();
        ImGui::Image((ImTextureID)gNBG3Texture, ImVec2(vdp2ResolutionWidth, vdp2ResolutionHeight));

        ImGui::Image((ImTextureID)gVdp1Texture, ImVec2(vdp2ResolutionWidth, vdp2ResolutionHeight));
    }
    ImGui::End();

    DebugTasks();

    glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    flushObjectsToDrawList();

    ImGui::Render();
    SDL_GL_SwapWindow(gWindow);

    return !closeApp;
}

