#include "PDS.h"

#include <soloud.h>

#define IMGUI_API

#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"

#ifdef _WIN32
#pragma comment(lib, "Opengl32.lib")
#endif

#if defined(__EMSCRIPTEN__) || defined(TARGET_OS_IOS) || defined(TARGET_OS_TV)
static float gVolume = 1.f;
#else
#ifdef SHIPPING_BUILD
static float gVolume = 1.f;
#else
static float gVolume = 0.f;
#endif
#endif

bool useVDP1GL = true;

SoLoud::Soloud gSoloud; // Engine core

extern SDL_Window *gWindow;
extern SDL_GLContext gGlcontext;

GLuint gVdp1PolyFB = 0;
GLuint gVdp1PolyTexture = 0;
GLuint gVdp1PolyDepth = 0;

GLuint gCompositedFB = 0;
GLuint gCompositedTexture = 0;

GLuint gVdp1Texture = 0;
GLuint gNBG0Texture = 0;
GLuint gNBG1Texture = 0;
GLuint gNBG2Texture = 0;
GLuint gNBG3Texture = 0;

#ifdef SHIPPING_BUILD
int frameLimit = 30;
#else
#if defined(PDS_TOOL) && !(defined(__EMSCRIPTEN__) || defined(TARGET_OS_IOS) || defined(TARGET_OS_TV))
int frameLimit = -1;
#else
int frameLimit = 30;
#endif
#endif

#ifdef USE_GL_ES3
const GLchar blit_vs[] =
"#version 300 es\n"
"in vec3 a_position;   \n"
"in vec2 a_texcoord;   \n"
"out  highp vec2 v_texcoord;     \n"
"void main()                  \n"
"{                            \n"
"   gl_Position = vec4(a_position, 1); \n"
"   v_texcoord = (a_position.xy+vec2(1,1))/2.0;; \n"
"} "
;

const GLchar blit_ps[] =
"#version 300 es\n"
"precision highp float;									\n"
"in highp vec2 v_texcoord;								\n"
"uniform sampler2D s_texture;							\n"
"out vec4 fragColor;									\n"
"void main()											\n"
"{														\n"
"	vec4 txcol = texture(s_texture, v_texcoord);		\n"
"   if(txcol.a <= 0.f) discard;\n"
"   fragColor = txcol; \n"
"   fragColor.w = 1.f;								\n"
"}														\n"
;
#elif defined(USE_GL)
const GLchar blit_vs[] =
"#version 330 \n"
"in vec3 a_position;   \n"
"in vec2 a_texcoord;   \n"
"out  highp vec2 v_texcoord;     \n"
"void main()                  \n"
"{                            \n"
"   gl_Position = vec4(a_position, 1); \n"
"   v_texcoord = (a_position.xy+vec2(1,1))/2.0;; \n"
"} "
;

const GLchar blit_ps[] =
"#version 330 \n"
"precision highp float;									\n"
"in highp vec2 v_texcoord;								\n"
"uniform sampler2D s_texture;							\n"
"out vec4 fragColor;									\n"
"void main()											\n"
"{														\n"
"	vec4 txcol = texture2D(s_texture, v_texcoord);		\n"
"   if(txcol.a <= 0) discard;\n"
"   fragColor = txcol; \n"
"   fragColor.w = 1;								\n"
"}														\n"
;
#endif
enum eLayers {
    SPRITE_POLY,
    SPRITE_SOFTWARE,
    NBG0,
    NBG1,
    //NBG2,
    NBG3,

    MAX
};

void bindBackBuffer()
{
#ifndef USE_NULL_RENDERER
#ifdef __IPHONEOS__
    SDL_SysWMinfo wmi;
    SDL_VERSION(&wmi.version);
    SDL_GetWindowWMInfo(gWindow, &wmi);

    glBindFramebuffer(GL_FRAMEBUFFER, wmi.info.uikit.framebuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, wmi.info.uikit.colorbuffer);
#else
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
    int internalResolution[2] = { 1024, 720 };
    SDL_GL_GetDrawableSize(gWindow, &internalResolution[0], &internalResolution[1]);
    glViewport(0, 0, internalResolution[0], internalResolution[1]);
#endif
}

void azelSdl2_Init()
{
#ifndef USE_NULL_RENDERER
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        assert(false);
    }

#ifdef USE_GL_ES3 
    const char* glsl_version = "#version 300 es";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
#endif
    gWindow = SDL_CreateWindow("PDS: Azel", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    assert(gWindow);

    gGlcontext = SDL_GL_CreateContext(gWindow);
    assert(gGlcontext);

#ifdef USE_GL
    gl3wInit();
#endif

    // Setup ImGui binding
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
#if !defined(__EMSCRIPTEN__) && !defined(TARGET_OS_IOS) && !defined(TARGET_OS_TV)
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
#endif
    //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

    printf("glsl_version: %s\n", glsl_version);

    ImGui_ImplOpenGL3_Init(glsl_version);
    ImGui_ImplSDL2_InitForOpenGL(gWindow, gGlcontext);

#ifndef USE_NULL_RENDERER
    // setup vdp1 Poly
    glGenFramebuffers(1, &gVdp1PolyFB);
    glGenTextures(1, &gVdp1PolyTexture);
    glGenRenderbuffers(1, &gVdp1PolyDepth);

    // Composited output
    glGenFramebuffers(1, &gCompositedFB);
    glGenTextures(1, &gCompositedTexture);

    glGenTextures(1, &gVdp1Texture);
    glGenTextures(1, &gNBG0Texture);
    glGenTextures(1, &gNBG1Texture);
    glGenTextures(1, &gNBG2Texture);
    glGenTextures(1, &gNBG3Texture);
#endif
    
#ifdef _DEBUG
    SDL_GL_SetSwapInterval(0);
#endif
    
#endif
}

GLuint getTextureForLayer(eLayers layerIndex)
{
    switch (layerIndex)
    {
    case SPRITE_POLY:
        return gVdp1PolyTexture;
    case SPRITE_SOFTWARE:
        return gVdp1Texture;
    case NBG0:
        return gNBG0Texture;
    case NBG1:
        return gNBG1Texture;
    //case NBG2:
    //    return gNBG2Texture;
    case NBG3:
        return gNBG3Texture;
    default:
        assert(0);
        break;
    }
}

bool isBackgroundEnabled(eLayers layerIndex)
{
    switch (layerIndex)
    {
    case SPRITE_POLY:
        return true;
    case SPRITE_SOFTWARE:
        return true;
    case NBG0:
        return vdp2Controls.m4_pendingVdp2Regs->BGON & 1;
    case NBG1:
        return vdp2Controls.m4_pendingVdp2Regs->BGON & 2;
    //case NBG2:
    //    return vdp2Controls.m_pendingVdp2Regs->BGON & 4;
    case NBG3:
        return vdp2Controls.m4_pendingVdp2Regs->BGON & 8;
    default:
        assert(0);
        break;
    }
}

int getPriorityForLayer(eLayers layerIndex)
{
    switch (layerIndex)
    {
    case SPRITE_POLY:
        return vdp2Controls.m4_pendingVdp2Regs->PRISA & 7;
    case SPRITE_SOFTWARE:
        return vdp2Controls.m4_pendingVdp2Regs->PRISA & 7;
    case NBG0:
        return vdp2Controls.m4_pendingVdp2Regs->PRINA & 7;
    case NBG1:
        return (vdp2Controls.m4_pendingVdp2Regs->PRINA >> 8) & 7;
//    case NBG2:
 //       return vdp2Controls.m_pendingVdp2Regs->PRINB & 7;
    case NBG3:
        return (vdp2Controls.m4_pendingVdp2Regs->PRINB >> 8) & 7;
    default:
        assert(0);
        break;
    }
}

bool closeApp = false;

struct UIState
{
    int mousex;
    int mousey;
    int mousedown;
    int scroll;

    char textinput[32];
};

static UIState gUIState;

void azelSdl2_StartFrame()
{
#ifndef USE_NULL_RENDERER
    checkGL();
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);
        switch (event.type)
        {
        case SDL_MOUSEMOTION:
            // update mouse position
            gUIState.mousex = event.motion.x;
            gUIState.mousey = event.motion.y;
            break;
        case SDL_MOUSEBUTTONDOWN:
            // update button down state if left-clicking
            if (event.button.button == 1)
            {
                gUIState.mousedown = 1;
            }
            if (event.button.button == 4)
            {
                gUIState.scroll = +1;
            }
            if (event.button.button == 5)
            {
                gUIState.scroll = -1;
            }
            break;
        case SDL_MOUSEBUTTONUP:
            // update button down state if left-clicking
            if (event.button.button == 1)
            {
                gUIState.mousedown = 0;
            }
            break;
        case SDL_MOUSEWHEEL:
            if (event.wheel.y > 0)
            {
                gUIState.scroll += 1;
            }
            else if (event.wheel.y < 0)
            {
                gUIState.scroll -= 1;
            }
            break;
        default:
            break;
        }

        if (event.type == SDL_QUIT)
            closeApp = true;
    }
#endif

    gSoloud.setGlobalVolume(gVolume);

    graphicEngineStatus.m4514.m0[0].m16_pending.m6_buttonDown = 0;
    graphicEngineStatus.m4514.m0[0].m16_pending.m8_newButtonDown = 0;
    graphicEngineStatus.m4514.m0[0].m16_pending.mC_newButtonDown2 = 0;

#ifndef USE_NULL_RENDERER
    const Uint8* keyState = SDL_GetKeyboardState(NULL);

    for (int i = 0; i < SDL_NUM_SCANCODES; i++)
    {
        if(keyState[i])
        {
            u16 buttonMask = 0;
            switch (i)
            {
            case SDL_SCANCODE_RETURN:
                buttonMask = 8;
                break;
            case SDL_SCANCODE_Z:
                buttonMask = 4;
                break;
            case SDL_SCANCODE_X:
                buttonMask = 2;
                break;
            case SDL_SCANCODE_C:
                buttonMask = 1;
                break;
            case SDL_SCANCODE_UP:
                buttonMask = 0x10;
                break;
            case SDL_SCANCODE_DOWN:
                buttonMask = 0x20;
                break;
            case SDL_SCANCODE_LEFT:
                buttonMask = 0x40;
                break;
            case SDL_SCANCODE_RIGHT:
                buttonMask = 0x80;
                break;
            default:
                break;
            }

            if (buttonMask)
            {
                graphicEngineStatus.m4514.m0[0].m16_pending.m6_buttonDown |= buttonMask;

                if ((graphicEngineStatus.m4514.m0[0].m0_current.m6_buttonDown & buttonMask) == 0)
                {
                    graphicEngineStatus.m4514.m0[0].m16_pending.m8_newButtonDown |= buttonMask;
                    graphicEngineStatus.m4514.m0[0].m16_pending.mC_newButtonDown2 |= buttonMask;
                }
            }
        }
    }

    checkGL();
    
    ImGui_ImplOpenGL3_NewFrame();
    
    checkGL();
    
    ImGui_ImplSDL2_NewFrame(gWindow);
    ImGui::NewFrame();
    
    checkGL();
#endif
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
    for (int i = 0; i < textureWidth * textureHeight; i++)
    {
        textureOutput[i] = 0x00FF0000;
    }

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

                textureOutput[(textureHeight - 1 - rawOutputY) * textureWidth + rawOutputX] = finalColor;
            }
        }
    }
}

void renderBG0(u32 width, u32 height)
{
    u32 textureWidth = width;
    u32 textureHeight = height;

    if ((vdp2Controls.m4_pendingVdp2Regs[0].TVMD & 0xC0) == 0xC0)
    {
        textureWidth *= 2;
        textureHeight *= 2;
    }

    u32* textureOutput = new u32[textureWidth * textureHeight];

    if(vdp2Controls.m4_pendingVdp2Regs->BGON & 0x1)
    {
        s_layerData planeData;
        planeData.CHSZ = vdp2Controls.m4_pendingVdp2Regs->CHCTLA & 1;
        planeData.CHCN = (vdp2Controls.m4_pendingVdp2Regs->CHCTLA >> 4) & 7;
        planeData.PNB = (vdp2Controls.m4_pendingVdp2Regs->PNCN0 >> 15) & 0x1;
        planeData.CNSM = (vdp2Controls.m4_pendingVdp2Regs->PNCN0 >> 14) & 0x1;
        planeData.CAOS = (vdp2Controls.m4_pendingVdp2Regs->CRAOFA) & 0x7;
        planeData.PLSZ = (vdp2Controls.m4_pendingVdp2Regs->PLSZ) & 3;
        planeData.SCN = (vdp2Controls.m4_pendingVdp2Regs->PNCN0) & 0x1F;
        planeData.scrollX = vdp2Controls.m4_pendingVdp2Regs->SCXN0 >> 16;
        planeData.scrollY = vdp2Controls.m4_pendingVdp2Regs->SCYN0 >> 16;

        u32 pageDimension = (planeData.CHSZ == 0) ? 64 : 32;
        u32 patternSize = (planeData.PNB == 0) ? 4 : 2;

        u32 pageSize = pageDimension * pageDimension * patternSize;

        u32 offset = (vdp2Controls.m4_pendingVdp2Regs->MPOFN & 7) << 6;

        planeData.planeOffsets[0] = (offset + (vdp2Controls.m4_pendingVdp2Regs->MPABN0 & 0x3F)) * pageSize;
        planeData.planeOffsets[1] = (offset + ((vdp2Controls.m4_pendingVdp2Regs->MPABN0 >> 8) & 0x3F)) * pageSize;
        planeData.planeOffsets[2] = (offset + (vdp2Controls.m4_pendingVdp2Regs->MPCDN0 & 0x3F)) * pageSize;
        planeData.planeOffsets[3] = (offset + ((vdp2Controls.m4_pendingVdp2Regs->MPCDN0 >> 8) & 0x3F)) * pageSize;

        renderLayer(planeData, textureWidth, textureHeight, textureOutput);
    }

#ifndef USE_NULL_RENDERER
    glBindTexture(GL_TEXTURE_2D, gNBG0Texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureOutput);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#endif
    
    delete[] textureOutput;

}

void renderBG1(u32 width, u32 height)
{
    u32 textureWidth = width;
    u32 textureHeight = height;

    u32* textureOutput = new u32[textureWidth * textureHeight];

    if (vdp2Controls.m4_pendingVdp2Regs->BGON & 0x2)
    {
        s_layerData planeData;

        planeData.CHSZ = (vdp2Controls.m4_pendingVdp2Regs->CHCTLA >> 8) & 0x1;
        planeData.CHCN = (vdp2Controls.m4_pendingVdp2Regs->CHCTLA >> 12) & 3;
        planeData.PNB = (vdp2Controls.m4_pendingVdp2Regs->PNCN1 >> 15) & 0x1;
        planeData.CNSM = (vdp2Controls.m4_pendingVdp2Regs->PNCN1 >> 14) & 0x1;
        planeData.CAOS = (vdp2Controls.m4_pendingVdp2Regs->CRAOFA >> 4) & 0x7;
        planeData.PLSZ = (vdp2Controls.m4_pendingVdp2Regs->PLSZ >> 2) & 3;
        planeData.SCN = (vdp2Controls.m4_pendingVdp2Regs->PNCN1) & 0x1F;
        planeData.scrollX = vdp2Controls.m4_pendingVdp2Regs->SCXN1 >> 16;
        planeData.scrollY = vdp2Controls.m4_pendingVdp2Regs->SCYN1 >> 16;

        u32 pageDimension = (planeData.CHSZ == 0) ? 64 : 32;
        u32 patternSize = (planeData.PNB == 0) ? 4 : 2;

        u32 pageSize = pageDimension * pageDimension * patternSize;

        u32 offset = ((vdp2Controls.m4_pendingVdp2Regs->MPOFN >> 4) & 7) << 6;

        planeData.planeOffsets[0] = (offset + (vdp2Controls.m4_pendingVdp2Regs->MPABN1 & 0x3F)) * pageSize;
        planeData.planeOffsets[1] = (offset + ((vdp2Controls.m4_pendingVdp2Regs->MPABN1 >> 8) & 0x3F)) * pageSize;
        planeData.planeOffsets[2] = (offset + (vdp2Controls.m4_pendingVdp2Regs->MPCDN1 & 0x3F)) * pageSize;
        planeData.planeOffsets[3] = (offset + ((vdp2Controls.m4_pendingVdp2Regs->MPCDN1 >> 8) & 0x3F)) * pageSize;

        renderLayer(planeData, textureWidth, textureHeight, textureOutput);
    }

#ifndef USE_NULL_RENDERER
    glBindTexture(GL_TEXTURE_2D, gNBG1Texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureOutput);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#endif
    
    delete[] textureOutput;
}

void renderBG2()
{
    /*
    u32 textureWidth = 1024;
    u32 textureHeight = 1024;

    u32* textureOutput = new u32[textureWidth * textureHeight];

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

    if (vdp2Controls.m4_pendingVdp2Regs->BGON & 0x8)
    {
        s_layerData planeData;

        planeData.CHSZ = (vdp2Controls.m4_pendingVdp2Regs->CHCTLB >> 4) & 0x1;
        planeData.CHCN = (vdp2Controls.m4_pendingVdp2Regs->CHCTLB >> 5) & 0x1;
        planeData.PNB = (vdp2Controls.m4_pendingVdp2Regs->PNCN3 >> 15) & 0x1;
        planeData.CNSM = (vdp2Controls.m4_pendingVdp2Regs->PNCN3 >> 14) & 0x1;
        planeData.CAOS = (vdp2Controls.m4_pendingVdp2Regs->CRAOFA >> 12) & 0x7;
        planeData.PLSZ = (vdp2Controls.m4_pendingVdp2Regs->PLSZ >> 6) & 3;
        planeData.SCN = (vdp2Controls.m4_pendingVdp2Regs->PNCN3) & 0x1F;
        planeData.scrollX = vdp2Controls.m4_pendingVdp2Regs->SCXN3;
        planeData.scrollY = vdp2Controls.m4_pendingVdp2Regs->SCYN3;

        u32 pageDimension = (planeData.CHSZ == 0) ? 64 : 32;
        u32 patternSize = (planeData.PNB == 0) ? 4 : 2;

        u32 pageSize = pageDimension * pageDimension * patternSize;

        u32 offset = ((vdp2Controls.m4_pendingVdp2Regs->MPOFN >> 12) & 7) << 6;

        planeData.planeOffsets[0] = (offset + (vdp2Controls.m4_pendingVdp2Regs->MPABN3 & 0x3F)) * pageSize;
        planeData.planeOffsets[1] = (offset + ((vdp2Controls.m4_pendingVdp2Regs->MPABN3 >> 8) & 0x3F)) * pageSize;
        planeData.planeOffsets[2] = (offset + (vdp2Controls.m4_pendingVdp2Regs->MPCDN3 & 0x3F)) * pageSize;
        planeData.planeOffsets[3] = (offset + ((vdp2Controls.m4_pendingVdp2Regs->MPCDN3 >> 8) & 0x3F)) * pageSize;

        renderLayer(planeData, textureWidth, textureHeight, textureOutput);
    }

#ifndef USE_NULL_RENDERER
    glBindTexture(GL_TEXTURE_2D, gNBG3Texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureOutput);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#endif
    
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
        int colorMode = (CMDPMOD >> 3) & 0x7;
        u32 characterAddress = ((u32)CMDSRCA) << 3;
        u32 colorBank = ((u32)CMDCOLR) << 1;
        s32 X = CMDXA + localCoordiantesX;
        s32 Y = CMDYA + localCoordiantesY;
        s32 Width = ((CMDSIZE >> 8) & 0x3F) * 8;
        s32 Height = CMDSIZE & 0xFF;

        switch(colorMode)
        {
        case 0:
            case 1:
            {
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

                            if (character)
                            {
                                u32 paletteOffset = colorBank + 2 * character;//((paletteNumber << 4) + dotColor) * 2 + layerData.CAOS * 0x200;
                                u16 color = getVdp2CramU16(paletteOffset);
                                u32 finalColor = 0xFF000000 | (((color & 0x1F) << 3) | ((color & 0x03E0) << 6) | ((color & 0x7C00) << 9));

                                vdp1TextureOutput[(vdp1TextureHeight - 1 - currentY) * vdp1TextureWidth + currentX] = finalColor;
                            }

                            counter++;
                        }
                    }
                }
                break;
            }
            case 4:
            {
                for (int currentY = Y; currentY < Y + Height; currentY++)
                {
                    for (int currentX = X; currentX < X + Width; currentX++)
                    {
                        if ((currentX >= 0) && (currentX < vdp1TextureWidth) && (currentY >= 0) && (currentY < vdp1TextureHeight))
                        {
                            u8 character = getVdp1VramU8(0x25C00000 + characterAddress);
                            characterAddress++;

                            if (character)
                            {
                                u32 paletteOffset = colorBank + 2 * character;//((paletteNumber << 4) + dotColor) * 2 + layerData.CAOS * 0x200;
                                u16 color = getVdp2CramU16(paletteOffset);
                                u32 finalColor = 0xFF000000 | (((color & 0x1F) << 3) | ((color & 0x03E0) << 6) | ((color & 0x7C00) << 9));

                                vdp1TextureOutput[(vdp1TextureHeight - 1 - currentY) * vdp1TextureWidth + currentX] = finalColor;
                            }
                        }
                    }
                }
                break;
            }
            default:
                assert(0);
        }
    }
}

void ScaledSpriteDraw(u32 vdp1EA)
{
    u16 CMDPMOD = getVdp1VramU16(vdp1EA + 4);
    u16 CMDCOLR = getVdp1VramU16(vdp1EA + 6);
    u16 CMDSRCA = getVdp1VramU16(vdp1EA + 8);
    u16 CMDSIZE = getVdp1VramU16(vdp1EA + 0xA);
    s16 CMDXA = getVdp1VramS16(vdp1EA + 0xC);
    s16 CMDYA = getVdp1VramS16(vdp1EA + 0xE);
    u16 CMDGRDA = getVdp1VramU16(vdp1EA + 0x1C);

    if (CMDSRCA)
    {
        u32 characterAddress = ((u32)CMDSRCA) << 3;
        u32 colorBank = ((u32)CMDCOLR) * 8;
        s32 X0 = CMDXA + localCoordiantesX;
        s32 Y0 = CMDYA + localCoordiantesY;
        s32 Width = ((CMDSIZE >> 8) & 0x3F) * 8;
        s32 Height = CMDSIZE & 0xFF;

        s32 X1;
        s32 Y1;

        switch ((getVdp1VramU16(vdp1EA + 0) >> 8) & 0xF)
        {
        case 0:
            X1 = getVdp1VramS16(vdp1EA + 0x14) + localCoordiantesX + 1;
            Y1 = getVdp1VramS16(vdp1EA + 0x16) + localCoordiantesY + 1;
            break;
        default:
            assert(0);
            break;
        }

        int counter = 0;

        for (int currentY = Y0; currentY < Y0 + Height; currentY++)
        {
            for (int currentX = X0; currentX < X0 + Width; currentX++)
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

                    if (character)
                    {
                        u32 paletteOffset = colorBank + 2 * character;
                        u16 color = getVdp1VramU16(0x25C00000 + paletteOffset);
                        color = READ_BE_U16(&color);
                        assert(color & 0x8000);
                        u32 finalColor = 0xFF000000 | (((color & 0x1F) << 3) | ((color & 0x03E0) << 6) | ((color & 0x7C00) << 9));

                        vdp1TextureOutput[(vdp1TextureHeight - 1 - currentY) * vdp1TextureWidth + currentX] = finalColor;
                    }

                    counter++;
                }
            }
        }
    }
}

void drawLine_old(s16 XA, s16 YA, s16 XB, s16 YB, u32 color)
{
    if (YA == YB)
    {
        if(XA <= XB)
        {
            for (int X = XA; X < XB; X++)
            {
                vdp1TextureOutput[(vdp1TextureHeight - 1 - YA) * vdp1TextureWidth + X] = color;
            }
        }
        else
        {
            for (int X = XB; X < XA; X++)
            {
                vdp1TextureOutput[(vdp1TextureHeight - 1 - YA) * vdp1TextureWidth + X] = color;
            }
        }
    }
    else if (XA == XB)
    {
        if(YA <= YB)
        {
            for (int Y = YA; Y < YB; Y++)
            {
                vdp1TextureOutput[(vdp1TextureHeight - 1 - Y) * vdp1TextureWidth + XA] = color;
            }
        }
        else
        {
            for (int Y = YB; Y < YA; Y++)
            {
                vdp1TextureOutput[(vdp1TextureHeight - 1 - Y) * vdp1TextureWidth + XA] = color;
            }
        }
    }
    else
    {
        assert(0);
    }
}

void image_set(int x, int y, u32 color)
{
    if (x >= 0 && y >= 0 && x < vdp1TextureWidth && y < vdp1TextureHeight)
    {
        vdp1TextureOutput[(vdp1TextureHeight - y - 1) * vdp1TextureWidth + x] = color;
    }
}

void drawLine(int x0, int y0, int x1, int y1, u32 color) {
    bool steep = false;
    if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    int dx = x1 - x0;
    int dy = y1 - y0;
    float derror = std::abs(dy / float(dx));
    float error = 0;
    int y = y0;
    for (int x = x0; x <= x1; x++) {
        if (steep) {
            image_set(y, x, color);
        }
        else {
            image_set(x, y, color);
        }
        error += derror;
        if (error > .5) {
            y += (y1 > y0 ? 1 : -1);
            error -= 1.;
        }
    }
}

void PolyLineDraw(u32 vdp1EA)
{
    u16 CMDPMOD = getVdp1VramU16(vdp1EA + 4);
    u16 CMDCOLR = getVdp1VramU16(vdp1EA + 6);
    s16 CMDXA = getVdp1VramS16(vdp1EA + 0xC);
    s16 CMDYA = getVdp1VramS16(vdp1EA + 0xE);
    s16 CMDXB = getVdp1VramS16(vdp1EA + 0x10);
    s16 CMDYB = getVdp1VramS16(vdp1EA + 0x12);
    s16 CMDXC = getVdp1VramS16(vdp1EA + 0x14);
    s16 CMDYC = getVdp1VramS16(vdp1EA + 0x16);
    s16 CMDXD = getVdp1VramS16(vdp1EA + 0x18);
    s16 CMDYD = getVdp1VramS16(vdp1EA + 0x1A);
    u16 CMDGRDA = getVdp1VramU16(vdp1EA + 0x1C);

    u32 finalColor;
    if (CMDCOLR & 0x8000)
    {
        finalColor = 0xFF000000 | (((CMDCOLR & 0x1F) << 3) | ((CMDCOLR & 0x03E0) << 6) | ((CMDCOLR & 0x7C00) << 9));
    }
    else
    {
        finalColor = 0xFF0000FF;
    }

    drawLine(CMDXA + localCoordiantesX, CMDYA + localCoordiantesY, CMDXB + localCoordiantesX, CMDYB + localCoordiantesY, finalColor);
    drawLine(CMDXB + localCoordiantesX, CMDYB + localCoordiantesY, CMDXC + localCoordiantesX, CMDYC + localCoordiantesY, finalColor);
    drawLine(CMDXC + localCoordiantesX, CMDYC + localCoordiantesY, CMDXD + localCoordiantesX, CMDYD + localCoordiantesY, finalColor);
    drawLine(CMDXD + localCoordiantesX, CMDYD + localCoordiantesY, CMDXA + localCoordiantesX, CMDYA + localCoordiantesY, finalColor);
}

void renderVdp1ToGL(u32 width, u32 height)
{
    u32 vdp1EA = 0x25C00000;

    while (1)
    {
        u16 CMDCTRL = getVdp1VramU16(vdp1EA);
        u16 CMDLINK = getVdp1VramU16(vdp1EA + 2);

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
            NormalSpriteDrawGL(vdp1EA);
            break;
        case 1:
            ScaledSpriteDrawGL(vdp1EA);
            break;
        case 2:
            // distorted sprite draw
            PolyLineDrawGL(vdp1EA);
            break;
        case 4:
            // draw polygon
            break;
        case 5:
            PolyLineDrawGL(vdp1EA);
            break;
        case 8:
            // user clipping coordinates
            break;
        case 9:
            // system clipping coordinates
            break;
        case 0xA:
            SetLocalCoordinates(vdp1EA);
            break;
        default:
            assert(0);
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
}

void renderVdp1()
{
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
        case 1:
            ScaledSpriteDraw(vdp1EA);
            break;
        case 2:
            // distorted sprite draw
            PolyLineDraw(vdp1EA);
            break;
        case 4:
            // draw polygon
            break;
        case 5:
            PolyLineDraw(vdp1EA);
            break;
        case 8:
            // user clipping coordinates
            break;
        case 9:
            // system clipping coordinates
            break;
        case 0xA:
            SetLocalCoordinates(vdp1EA);
            break;
        default:
            assert(0);
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
}

#ifndef USE_NULL_RENDERER
void renderTexturedQuad(GLuint sourceTexture)
{
    static GLuint quad_VertexArrayID;
    static GLuint shaderProgram = 0;
    static GLuint vshader = 0;
    static GLuint fshader = 0;
    static GLuint quad_vertexbuffer = 0;
    static GLuint texID;

    static bool initialized = false;
    if (!initialized)
    {
        static const GLfloat g_quad_vertex_buffer_data[] = {
            -1.0f, -1.0f, 0.0f,
            1.0f, -1.0f, 0.0f,
            -1.0f,  1.0f, 0.0f,
            -1.0f,  1.0f, 0.0f,
            1.0f, -1.0f, 0.0f,
            1.0f,  1.0f, 0.0f,
        };

        glGenVertexArrays(1, &quad_VertexArrayID);
        glBindVertexArray(quad_VertexArrayID);
        glGenBuffers(1, &quad_vertexbuffer);

        vshader = glCreateShader(GL_VERTEX_SHADER);
        {
            volatile int compiled = 0;
            const GLchar* pYglprg_normal_v[] = { blit_vs, NULL };
            glShaderSource(vshader, 1, pYglprg_normal_v, NULL);
            glCompileShader(vshader);
            glGetShaderiv(vshader, GL_COMPILE_STATUS, (int*)& compiled);
            if (compiled == GL_FALSE)
            {
                GLint maxLength = 0;
                glGetShaderiv(vshader, GL_INFO_LOG_LENGTH, &maxLength);

                // The maxLength includes the NULL character
                std::vector<GLchar> errorLog(maxLength);
                glGetShaderInfoLog(vshader, maxLength, &maxLength, &errorLog[0]);
            }
            while (!compiled);
        }

        fshader = glCreateShader(GL_FRAGMENT_SHADER);
        {
            volatile int compiled = 0;
            const GLchar* pYglprg_normal_f[] = { blit_ps, NULL };
            glShaderSource(fshader, 1, pYglprg_normal_f, NULL);
            glCompileShader(fshader);
            glGetShaderiv(fshader, GL_COMPILE_STATUS, (int*)& compiled);
            if (compiled == GL_FALSE)
            {
                GLint maxLength = 0;
                glGetShaderiv(fshader, GL_INFO_LOG_LENGTH, &maxLength);

                // The maxLength includes the NULL character
                std::vector<GLchar> errorLog(maxLength);
                glGetShaderInfoLog(fshader, maxLength, &maxLength, &errorLog[0]);
                PDS_unimplemented(errorLog.data());
                assert(compiled);
            }
            while (!compiled);
        }

        shaderProgram = glCreateProgram();
        {
            volatile int linked = 0;
            glAttachShader(shaderProgram, vshader);
            glAttachShader(shaderProgram, fshader);
            glLinkProgram(shaderProgram);
            glGetProgramiv(shaderProgram, GL_LINK_STATUS, (int*)& linked);
            assert(linked == 1);
            while (!linked);
        }

        glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);

        texID = glGetUniformLocation(shaderProgram, "s_texture");
        assert(texID >= 0);

        initialized = true;
    }


    glUseProgram(shaderProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sourceTexture);
    // Set our "renderedTexture" sampler to user Texture Unit 0
    glUniform1i(texID, 0);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
    glVertexAttribPointer(
        0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
    );

    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, 6); // From index 0 to 3 -> 1 triangle

    glDisableVertexAttribArray(0);
}
#endif

bool azelSdl2_EndFrame()
{
    u32 outputResolutionWidth = 0;
    u32 outputResolutionHeight = 0;

    u32 LSMD = (vdp2Controls.m4_pendingVdp2Regs->TVMD >> 6) & 3;
    u32 VRESO = (vdp2Controls.m4_pendingVdp2Regs->TVMD >> 4) & 3;
    u32 HRESO = (vdp2Controls.m4_pendingVdp2Regs->TVMD) & 7;

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

    {
        vdp1TextureWidth = outputResolutionWidth;
        vdp1TextureHeight = outputResolutionHeight;
        vdp1TextureOutput = new u32[vdp1TextureWidth * vdp1TextureHeight];
        memset(vdp1TextureOutput, 0x00, vdp1TextureWidth * vdp1TextureHeight * 4);
    }

    if (!useVDP1GL)
    {
        renderVdp1();
    }
    
    {
#ifndef USE_NULL_RENDERER
        glBindTexture(GL_TEXTURE_2D, gVdp1Texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, vdp1TextureWidth, vdp1TextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, vdp1TextureOutput);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#endif
        delete[] vdp1TextureOutput;
        vdp1TextureOutput = NULL;
    }

    renderBG0(vdp2ResolutionWidth, vdp2ResolutionHeight);
    renderBG1(vdp2ResolutionWidth, vdp2ResolutionHeight);
    //renderBG2(vdp2ResolutionWidth, vdp2ResolutionHeight);
    renderBG3(vdp2ResolutionWidth, vdp2ResolutionHeight);

    if(ImGui::Begin("VDP"))
    {
        ImGui::Image((ImTextureID)gNBG0Texture, ImVec2(vdp2ResolutionWidth, vdp2ResolutionHeight), ImVec2(0, 1), ImVec2(1, 0)); ImGui::SameLine();
        ImGui::Image((ImTextureID)gNBG1Texture, ImVec2(vdp2ResolutionWidth, vdp2ResolutionHeight), ImVec2(0, 1), ImVec2(1, 0));
        ImGui::Image((ImTextureID)gNBG2Texture, ImVec2(vdp2ResolutionWidth, vdp2ResolutionHeight), ImVec2(0, 1), ImVec2(1, 0)); ImGui::SameLine();
        ImGui::Image((ImTextureID)gNBG3Texture, ImVec2(vdp2ResolutionWidth, vdp2ResolutionHeight), ImVec2(0, 1), ImVec2(1, 0)); 

        ImGui::Image((ImTextureID)gVdp1Texture, ImVec2(vdp2ResolutionWidth, vdp2ResolutionHeight), ImVec2(0, 1), ImVec2(1, 0)); ImGui::SameLine();
        ImGui::Image((ImTextureID)gVdp1PolyTexture, ImVec2(vdp2ResolutionWidth, vdp2ResolutionHeight), ImVec2(0, 1), ImVec2(1, 0));
    }
    ImGui::End();

    DebugTasks();

    checkGL();
    
    static int internalResolution[2] = { 1024, 720 };
    
#ifndef USE_NULL_RENDERER
    SDL_GL_GetDrawableSize(gWindow, &internalResolution[0], &internalResolution[1]);
#if (defined(__APPLE__) && TARGET_OS_SIMULATOR)
    internalResolution[0] /= 8;
    internalResolution[1] /= 8;
#endif
    
    glViewport(0, 0, internalResolution[0], internalResolution[1]);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);

    glClearDepthf(1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
#endif
    
    checkGL();

    ImGui::Begin("Config");
    {
        ImGui::InputInt2("Internal Resolution", internalResolution);
    }
    ImGui::End();

    // render VDP1 frame buffer
    if(1)
    {
#ifndef USE_NULL_RENDERER
        checkGL();
        glBindFramebuffer(GL_FRAMEBUFFER, gVdp1PolyFB);
        glBindTexture(GL_TEXTURE_2D, gVdp1PolyTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, internalResolution[0], internalResolution[1], 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        checkGL();
        
        glBindRenderbuffer(GL_RENDERBUFFER, gVdp1PolyDepth);

        checkGL();
        
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, internalResolution[0], internalResolution[1]);

        checkGL();
        
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gVdp1PolyDepth);

        checkGL();
        
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gVdp1PolyTexture, 0);

        checkGL();
        
        GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };

        glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

        checkGL();
        
        assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

        glViewport(0, 0, internalResolution[0], internalResolution[1]);

        glClearColor(0, 0, 0, 0);
        glClearDepthf(1.f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (useVDP1GL)
        {
            renderVdp1ToGL(internalResolution[0], internalResolution[1]);
        }

        glFrontFace(GL_CW);
        glCullFace(GL_BACK);
        glEnable(GL_CULL_FACE);
        checkGL();
#endif
        flushObjectsToDrawList();

#ifndef USE_NULL_RENDERER
        glDisable(GL_CULL_FACE);
#endif
    }
    
    //Compose
#ifndef USE_NULL_RENDERER
    {
        glBindFramebuffer(GL_FRAMEBUFFER, gCompositedFB);
        glBindTexture(GL_TEXTURE_2D, gCompositedTexture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, internalResolution[0], internalResolution[1], 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gCompositedTexture, 0);

        GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

        assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

        glViewport(0, 0, internalResolution[0], internalResolution[1]);

        glClearColor(0, 0, 0, 0);
        glClearDepthf(0.f);

        glClear(GL_COLOR_BUFFER_BIT);

        for (int priorityIndex = 0; priorityIndex <= 7; priorityIndex++)
        {
            for (eLayers layerIndex = SPRITE_POLY; layerIndex < eLayers::MAX; layerIndex = (eLayers)(layerIndex + 1))
            {
                if (isBackgroundEnabled(layerIndex) && (getPriorityForLayer(layerIndex) == priorityIndex))
                {
                    renderTexturedQuad(getTextureForLayer(layerIndex));
                }
            }
            
        }
    }
#endif

    bindBackBuffer();

    ImGui::Begin("Final Composition");
    {
        ImVec2 textureSize = ImGui::GetWindowSize();
        textureSize.y = textureSize.x * (224.f / 352.f);
        ImGui::Image((ImTextureID)gCompositedTexture, textureSize, ImVec2(0, 1), ImVec2(1, 0)); ImGui::SameLine();
    }
    ImGui::End();

    if (ImGui::BeginMainMenuBar())
    {
        ImGui::Text(" %.2f FPS (%.2f ms)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);

        if (ImGui::BeginMenu("Framerate"))
        {
            bool unlimited = true;
            if (ImGui::MenuItem("Unlimited", NULL, frameLimit == -1)) frameLimit = -1;
            if (ImGui::MenuItem("30", NULL, frameLimit == 30)) frameLimit = 30;
            if (ImGui::MenuItem("5", NULL, frameLimit == 5)) frameLimit = 5;
            ImGui::EndMenu();
        }

        ImGui::PushItemWidth(100);
        ImGui::SliderFloat("Volume", &gVolume, 0, 1);
        ImGui::PopItemWidth();

        ImGui::EndMainMenuBar();
    }

    checkGL();
    
    PDS_Logger.Draw("Logs");

    ImGui::Render();
    
    checkGL();
    
    static bool bImguiEnabled = false;

#ifndef USE_NULL_RENDERER
    if (ImGui::GetIO().KeysDown[SDL_SCANCODE_GRAVE] && (ImGui::GetIO().KeysDownDuration[SDL_SCANCODE_GRAVE] == 0.f))
    {
        bImguiEnabled = !bImguiEnabled;
    }
    if (bImguiEnabled)
    {
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
    else
    {
        renderTexturedQuad(gCompositedTexture);
    }
    
    checkGL();
    // Update and Render additional Platform Windows
    ImGuiIO& io = ImGui::GetIO();
#if 0
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        SDL_GL_MakeCurrent(gWindow, gGlcontext);
    }
#endif

    glFlush();
    checkGL();
    {
        static Uint64 last_time = SDL_GetPerformanceCounter();
        Uint64 now = SDL_GetPerformanceCounter();

        float freq = SDL_GetPerformanceFrequency();
        float secs = (now - last_time) / freq;
        float timeToWait = ((1.f/frameLimit) - secs) * 1000;
        //timeToWait = 0;
        if (timeToWait > 0)
        {
            SDL_Delay(timeToWait);
        }

        SDL_GL_SwapWindow(gWindow);

        last_time = SDL_GetPerformanceCounter();
    }
    
    checkGL();
#endif
    return !closeApp;
}

