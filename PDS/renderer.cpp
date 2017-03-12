#include "PDS.h"

#include "GL/gl3w.h"

#define IMGUI_API

#include "imgui_impl_sdl_gl3.h"

#ifdef _WIN32
#pragma comment(lib, "Opengl32.lib")
#endif

SDL_Window *gWindow;
SDL_GLContext gGlcontext;

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

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
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

struct s_planeData
{
    u32 CHSZ;
    u32 CHCN;
    u32 PNB;
    u32 CNSM;
    u32 CAOS;
    u32 PLSZ;

    u32 planeOffsets[4];
};

void renderPlane2(s_planeData& planeData, u32 textureWidth, u32 textureHeight, u32* textureOutput)
{
    u32 cellDotDimension = 8;
    u32 characterPatternDotDimension = cellDotDimension * ((planeData.CHSZ == 0) ? 1 : 2);
    u32 pageDotDimension = 8 * 64; /* A page is always 64x64 cells, so 512 * 512 dots*/ /*characterPatternDimension * ((planeData.CHSZ == 0) ? 64 : 32);*/
    u32 planeDotWidth = pageDotDimension * ((planeData.PLSZ & 1) ? 2 : 1);
    u32 planeDotHeight = pageDotDimension * ((planeData.PLSZ & 2) ? 2 : 1);
    u32 mapDotWidth = planeDotWidth * 2; // because scrollScreen
    u32 mapDotHeight = planeDotHeight * 2; // because scrollScreen

    u32 pageDimension = (planeData.CHSZ == 0) ? 64 : 32;
    u32 patternSize = (planeData.PNB == 0) ? 4 : 2;
    u32 pageSize = pageDimension * pageDimension * patternSize;

    for (u32 outputY = 0; outputY < textureHeight; outputY++)
    {
        for (u32 outputX = 0; outputX < textureWidth; outputX++)
        {
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

            u32 planeNumber = planeY * ((planeData.PLSZ & 1) ? 2 : 1) + planeX;
            u32 startOfPlane = planeData.planeOffsets[planeNumber];

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

                // assuming supplement mode 0 with no data
                
                paletteNumber = (patternName >> 12) & 0xF;

                switch (planeData.CNSM)
                {
                case 0:
                    characterNumber = patternName & 0x3FF;
                    break;
                case 1:
                    characterNumber = patternName & 0xFFF;
                    break;
                }

                if (planeData.CHSZ == 1)
                {
                    characterNumber <<= 2;
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

            u32 cellOffset = characterOffset + (cellY * 8 + cellX) * 8;

            u8 dotColor = 0;
            u32 paletteOffset = 0;

            switch (planeData.CHCN)
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

                    paletteOffset = ((paletteNumber << 4) + dotColor) * 2 + planeData.CAOS * 0x200;
                    break;
                }
                // 256 colors, 8bits
                case 1:
                {
                    u32 dotOffset = cellOffset + dotInCellY * 8 + dotInCellX;
                    dotColor = getVdp2VramU8(dotOffset);
                    paletteOffset = ((paletteNumber << 4) + dotColor) * 2 + planeData.CAOS * 0x200;
                    break;
                }
                default:
                    assert(0);
            }


            u16 color = getVdp2CramU16(paletteOffset);
            u32 finalColor = 0xFF000000 | (((color & 0x1F) << 3) | ((color & 0x03E0) << 6) | ((color & 0x7C00) << 9));

            textureOutput[outputY * textureWidth + outputX] = finalColor;
        }
    }
}

void renderPlane(s_planeData& planeData, u32 textureWidth, u32 textureHeight, u32* textureOutput)
{
    u32 characterPatternSizeVH = (planeData.CHSZ == 0) ? 1 : 2;
    u32 pageDimension = (planeData.CHSZ == 0) ? 64 : 32;
    u32 patternSize = (planeData.PNB == 0) ? 4 : 2;

    u32 characterSize = planeData.CHSZ;
    u32 patternNameDataSize = planeData.PNB;

    u32 planeOffsets[4] =
    {
        planeData.planeOffsets[0],
        planeData.planeOffsets[1],
        planeData.planeOffsets[2],
        planeData.planeOffsets[3]
    };

    u32* currentOutput = textureOutput;

    u32 planeOffset = planeOffsets[0];
    if (planeOffset)
    {
        int numPages = 1;

        u32 currentMemoryOffset = planeOffset;

        for (int pageIdx = 0; pageIdx < numPages; pageIdx++)
        {
            u32 pageOutputX = 0;
            u32 pageOutputY = 0;
            u32 pageOutputStart = 0;

            for (int patternY = 0; patternY < pageDimension; patternY++)
            {
                for (int patternX = 0; patternX < pageDimension; patternX++)
                {
                    u32 characterNumber;
                    u32 paletteNumber;
                    u32 characterOffset;

                    switch (patternSize)
                    {
                    case 2:
                    {
                        u16 patternName = getVdp2VramU16(currentMemoryOffset);
                        currentMemoryOffset += 2;

                        // assuming supplement mode 0 with no data
                        characterNumber = patternName & 0x3FF;
                        paletteNumber = (patternName >> 12) & 0xF;

                        characterOffset = (characterNumber << 1) * 0x10;

                        if (patternName)
                        {
                            patternName = patternName;
                        }
                        break;
                    }
                    case 4:
                    {
                        u16 data1 = getVdp2VramU16(currentMemoryOffset);
                        u16 data2 = getVdp2VramU16(currentMemoryOffset + 2);
                        currentMemoryOffset += 4;

                        // assuming supplement mode 0 with no data
                        characterNumber = data2 & 0x7FFF;
                        paletteNumber = data1 & 0x7F;

                        characterOffset = characterNumber;
                        break;
                    }
                    default:
                        assert(0);
                    }

                    // character is 2X2
                    u32 currentCellOffset = characterOffset;
                    for (int cellY = 0; cellY < characterPatternSizeVH; cellY++)
                    {
                        for (int cellX = 0; cellX < characterPatternSizeVH; cellX++)
                        {
                            u32 dotStart = currentCellOffset;
                            for (int dotX = 0; dotX < 8; dotX++)
                            {
                                for (int dotY = 0; dotY < 8; dotY++)
                                {
                                    // assume 4bpp
                                    u32 dotOffset = dotStart + dotY * 4 + dotX / 2;
                                    u8 dotColor = getVdp2VramU8(dotOffset);

                                    if (dotX & 1)
                                    {
                                        dotColor &= 0xF;
                                    }
                                    else
                                    {
                                        dotColor >>= 4;
                                    }

                                    u32 paletteOffset = ((paletteNumber << 4) + dotColor)*2 + planeData.CAOS * 0x200;

                                    u16 color = getVdp2CramU16(paletteOffset);

                                    //*(currentOutput++) = //(((tmp & 0x1F) << 3) | ((tmp & 0x03E0) << 6) | ((tmp & 0x7C00) << 9)) | ((tmp & 0x8000) << 16);

                                    u32 finalColor = (((color & 0x1F) << 3) | ((color & 0x03E0) << 6) | ((color & 0x7C00) << 9));

                                    u32 outputX = 0;
                                    u32 outputY = 0;

                                    computePosition(patternX, patternY, pageDimension, cellX, cellY, characterPatternSizeVH, dotX, dotY, 8, &outputX, &outputY);
                                    if((outputY < textureHeight) && (outputX < textureWidth))
                                        currentOutput[outputY * textureWidth + outputX] = finalColor | 0xFF000000;
                                }
                            }
                            currentCellOffset += 32; // also assume 4bpp
                        }
                    }
                }
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

    s_planeData planeData;

    planeData.CHSZ = vdp2Controls.m_pendingVdp2Regs->CHCTLA & 1;
    planeData.CHCN = (vdp2Controls.m_pendingVdp2Regs->CHCTLA >> 4) & 7;
    planeData.PNB = (vdp2Controls.m_pendingVdp2Regs->PNCN0 >> 15) & 0x1;
    planeData.CNSM = (vdp2Controls.m_pendingVdp2Regs->PNCN0 >> 14) & 0x1;
    planeData.CAOS = (vdp2Controls.m_pendingVdp2Regs->CRAOFA) & 0x7;
    planeData.PLSZ = (vdp2Controls.m_pendingVdp2Regs->PLSZ) & 3;

    u32 pageDimension = (planeData.CHSZ == 0) ? 64 : 32;
    u32 patternSize = (planeData.PNB == 0) ? 4 : 2;

    u32 pageSize = pageDimension * pageDimension * patternSize;

    planeData.planeOffsets[0] = (vdp2Controls.m_pendingVdp2Regs->MPABN0 & 0x3F) * pageSize;
    planeData.planeOffsets[1] = ((vdp2Controls.m_pendingVdp2Regs->MPABN0>>8) & 0x3F) * pageSize;
    planeData.planeOffsets[2] = (vdp2Controls.m_pendingVdp2Regs->MPCDN0 & 0x3F) * pageSize;
    planeData.planeOffsets[3] = ((vdp2Controls.m_pendingVdp2Regs->MPCDN0 >> 8) & 0x3F) * pageSize;

    renderPlane2(planeData, textureWidth, textureHeight, textureOutput);

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

    s_planeData planeData;

    planeData.CHSZ = (vdp2Controls.m_pendingVdp2Regs->CHCTLA >> 8) & 0x1;
    planeData.CHCN = (vdp2Controls.m_pendingVdp2Regs->CHCTLA >> 12) & 3;
    planeData.PNB = (vdp2Controls.m_pendingVdp2Regs->PNCN1 >> 15) & 0x1;
    planeData.CNSM = (vdp2Controls.m_pendingVdp2Regs->PNCN1 >> 14) & 0x1;
    planeData.CAOS = (vdp2Controls.m_pendingVdp2Regs->CRAOFA >> 4) & 0x7;
    planeData.PLSZ = (vdp2Controls.m_pendingVdp2Regs->PLSZ >> 2) & 3;


    u32 pageDimension = (planeData.CHSZ == 0) ? 64 : 32;
    u32 patternSize = (planeData.PNB == 0) ? 4 : 2;

    u32 pageSize = pageDimension * pageDimension * patternSize;

    planeData.planeOffsets[0] = (vdp2Controls.m_pendingVdp2Regs->MPABN1 & 0x3F) * pageSize;
    planeData.planeOffsets[1] = ((vdp2Controls.m_pendingVdp2Regs->MPABN1 >> 8) & 0x3F) * pageSize;
    planeData.planeOffsets[2] = (vdp2Controls.m_pendingVdp2Regs->MPCDN1 & 0x3F) * pageSize;
    planeData.planeOffsets[3] = ((vdp2Controls.m_pendingVdp2Regs->MPCDN1 >> 8) & 0x3F) * pageSize;

    renderPlane2(planeData, textureWidth, textureHeight, textureOutput);

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

    s_planeData planeData;

    planeData.CHSZ = (vdp2Controls.m_pendingVdp2Regs->CHCTLB >> 4) & 0x1;
    planeData.CHCN = (vdp2Controls.m_pendingVdp2Regs->CHCTLB >> 5) & 0x1;
    planeData.PNB = (vdp2Controls.m_pendingVdp2Regs->PNCN3 >> 15) & 0x1;
    planeData.CNSM = (vdp2Controls.m_pendingVdp2Regs->PNCN3 >> 14) & 0x1;
    planeData.CAOS = (vdp2Controls.m_pendingVdp2Regs->CRAOFA >> 12) & 0x7;
    planeData.PLSZ = (vdp2Controls.m_pendingVdp2Regs->PLSZ >> 6) & 3;


    u32 pageDimension = (planeData.CHSZ == 0) ? 64 : 32;
    u32 patternSize = (planeData.PNB == 0) ? 4 : 2;

    u32 pageSize = pageDimension * pageDimension * patternSize;

    planeData.planeOffsets[0] = (vdp2Controls.m_pendingVdp2Regs->MPABN3 & 0x3F) * pageSize;
    planeData.planeOffsets[1] = ((vdp2Controls.m_pendingVdp2Regs->MPABN3 >> 8) & 0x3F) * pageSize;
    planeData.planeOffsets[2] = (vdp2Controls.m_pendingVdp2Regs->MPCDN3 & 0x3F) * pageSize;
    planeData.planeOffsets[3] = ((vdp2Controls.m_pendingVdp2Regs->MPCDN3 >> 8) & 0x3F) * pageSize;

    renderPlane2(planeData, textureWidth, textureHeight, textureOutput);

    glBindTexture(GL_TEXTURE_2D, gNBG3Texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureOutput);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    delete[] textureOutput;
}

bool azelSdl2_EndFrame()
{
    u32 vdp2ResolutionWidth = 704;
    u32 vdp2ResolutionHeight = 448;


    renderBG0(vdp2ResolutionWidth, vdp2ResolutionHeight);
    ImGui::Begin("NBG0");
    {
        ImGui::Image((ImTextureID)gNBG0Texture, ImVec2(vdp2ResolutionWidth, vdp2ResolutionHeight));
    }
    ImGui::End();

    renderBG1(vdp2ResolutionWidth, vdp2ResolutionHeight);
    ImGui::Begin("NBG1");
    {
        ImGui::Image((ImTextureID)gNBG1Texture, ImVec2(vdp2ResolutionWidth, vdp2ResolutionHeight));
    }
    ImGui::End();
    /*
    renderBG2();
    ImGui::Begin("NBG2");
    {
        ImGui::Image((ImTextureID)gNBG2Texture, ImVec2(1024, 1024));
    }
    ImGui::End();
    
    renderBG3(vdp2ResolutionWidth, vdp2ResolutionHeight);
    ImGui::Begin("NBG3");
    {
        ImGui::Image((ImTextureID)gNBG3Texture, ImVec2(vdp2ResolutionWidth, vdp2ResolutionHeight));
    }
    ImGui::End();
   */
    glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui::Render();
    SDL_GL_SwapWindow(gWindow);

    return !closeApp;
}

