#include "PDS.h"
#include <soloud.h>

#include <examples/imgui_impl_sdl.h>

#include "renderer/renderer.h"
#include "renderer/renderer_gl.h"
#include "renderer/renderer_vk.h"

//#include "../ThirdParty/bgfx.cmake/bgfx/examples/common/imgui/imgui.h"
#include "imguiBGFX.h"
#include "items.h"

extern SDL_Window* gWindowBGFX;

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

int outputResolution[2] = { 1024, 814 };
int internalResolution[2] = { 352, 224 };

bgfx::FrameBufferHandle gBGFXVdp1PolyFB = BGFX_INVALID_HANDLE;
bgfx::TextureHandle vdp1BufferTexture = BGFX_INVALID_HANDLE;
bgfx::TextureHandle vdp1DepthBufferTexture = BGFX_INVALID_HANDLE;

bgfx::FrameBufferHandle gBgfxCompositedFB = BGFX_INVALID_HANDLE;

bgfx::TextureHandle bgfx_vdp1_ram_texture = BGFX_INVALID_HANDLE;
bgfx::TextureHandle bgfx_vdp2_ram_texture = BGFX_INVALID_HANDLE;
bgfx::TextureHandle bgfx_vdp2_cram_texture = BGFX_INVALID_HANDLE;

struct s_NBG_data
{
    bgfx::FrameBufferHandle BGFXFB = BGFX_INVALID_HANDLE;
    bgfx::TextureHandle BGFXTexture = BGFX_INVALID_HANDLE;
    bgfx::TextureHandle bgfx_vdp2_planeDataBuffer = BGFX_INVALID_HANDLE;
    bgfx::ViewId viewId = -1;
    int planeId = -1;

    int m_currentWidth = -1;
    int m_currentHeight = -1;
};

std::array<s_NBG_data, 5> NBG_data;

bgfx::ProgramHandle bgfx_vdp2_program = BGFX_INVALID_HANDLE;

#ifdef SHIPPING_BUILD
int frameLimit = 30;
#else
#if defined(PDS_TOOL) && !(defined(__EMSCRIPTEN__) || defined(TARGET_OS_IOS) || defined(TARGET_OS_TV))
int frameLimit = 30;
#else
int frameLimit = 30;
#endif
#endif

enum eLayers {
    SPRITE_POLY,
    SPRITE_SOFTWARE,
    NBG0,
    NBG1,
    //NBG2,
    NBG3,
    RBG0,

    MAX
};

backend* gBackend = nullptr;

bgfx::ShaderHandle loadBgfxShader(const std::string& filename)
{
    std::vector<u8> memBlob;
    FILE* fHandle = fopen(filename.c_str(), "rb");
    if (fHandle == nullptr)
        return BGFX_INVALID_HANDLE;
    fseek(fHandle, 0, SEEK_END);
    u32 size = ftell(fHandle);
    fseek(fHandle, 0, SEEK_SET);
    memBlob.resize(size);
    fread(&memBlob[0], 1, size, fHandle);
    fclose(fHandle);

    bgfx::ShaderHandle handle = bgfx::createShader(bgfx::copy(&memBlob[0], size));
    bgfx::setName(handle, filename.c_str());

    return handle;
}

bgfx::ProgramHandle loadBgfxProgram(const std::string& VSFile, const std::string& PSFile)
{
    std::string shaderFileExtension = "";
    switch (bgfx::getRendererType())
    {
    case bgfx::RendererType::Direct3D11:
    case bgfx::RendererType::Direct3D12: shaderFileExtension = ".dx11.bin";  break;
    case bgfx::RendererType::OpenGL:     shaderFileExtension = ".glsl.bin";  break;
    case bgfx::RendererType::Metal:      shaderFileExtension = ".metal.bin";  break;
    case bgfx::RendererType::Vulkan:     shaderFileExtension = ".spirv.bin";  break;
    default:
        assert(0);
    }

    bgfx::ShaderHandle vsh = loadBgfxShader(std::string("shaders/generated/") + VSFile + shaderFileExtension);
    bgfx::ShaderHandle psh = loadBgfxShader(std::string("shaders/generated/") + PSFile + shaderFileExtension);
    
    assert(bgfx::isValid(vsh));
    assert(bgfx::isValid(psh));

    bgfx::ProgramHandle ProgramHandle = bgfx::createProgram(vsh, psh, true /* destroy shaders when program is destroyed */);
    assert(bgfx::isValid(ProgramHandle));
    return ProgramHandle;
}

void azelSdl2_Init()
{
#ifdef WITH_VK
    //gBackend = SDL_VK_backend::create();
    //if (gBackend == nullptr)
#endif
    {
        gBackend = SDL_ES3_backend::create();
    }


    for (int i = 0; i < NBG_data.size(); i++)
    {
        NBG_data[i].planeId = i;
        NBG_data[i].viewId = VDP2_viewsStart + i;
        assert(NBG_data[i].viewId <= VDP2_MAX);
    }

    bgfx_vdp1_ram_texture = bgfx::createTexture2D(256, 0x80000 / 256, false, 1, bgfx::TextureFormat::R8U, 0);
    bgfx_vdp2_ram_texture = bgfx::createTexture2D(256, 0x80000 / 256, false, 1, bgfx::TextureFormat::R8U, 0);
    bgfx_vdp2_cram_texture = bgfx::createTexture2D(256, 0x1000 / 256, false, 1, bgfx::TextureFormat::R8U, 0);

    bgfx_vdp2_program = loadBgfxProgram("VDP2_vs", "VDP2_ps");
#ifndef SHIPPING_BUILD
    SDL_GL_SetSwapInterval(0);
#endif

    imguiCreate();
    ImGui_ImplSDL2_InitForD3D(gWindowBGFX);
}

bgfx::TextureHandle getTextureForLayerBgfx(eLayers layerIndex)
{
    switch (layerIndex)
    {
    case SPRITE_POLY:
        return bgfx::getTexture(gBGFXVdp1PolyFB);
    case SPRITE_SOFTWARE:
        return BGFX_INVALID_HANDLE;
    case NBG0:
        return NBG_data[0].BGFXTexture;
    case NBG1:
        return NBG_data[1].BGFXTexture;
        //case NBG2:
        //    return gNBG2Texture;
    case NBG3:
        return NBG_data[3].BGFXTexture;
    case RBG0:
        return NBG_data[4].BGFXTexture;
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
        return vdp2Controls.m4_pendingVdp2Regs->m20_BGON & 1;
    case NBG1:
        return vdp2Controls.m4_pendingVdp2Regs->m20_BGON & 2;
    //case NBG2:
    //    return vdp2Controls.m_pendingVdp2Regs->BGON & 4;
    case NBG3:
        return vdp2Controls.m4_pendingVdp2Regs->m20_BGON & 8;
    case RBG0:
        return vdp2Controls.m4_pendingVdp2Regs->m20_BGON & 0x10;
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
        return vdp2Controls.m4_pendingVdp2Regs->mF0_PRISA & 7;
    case SPRITE_SOFTWARE:
        return vdp2Controls.m4_pendingVdp2Regs->mF0_PRISA & 7;
    case NBG0:
        return vdp2Controls.m4_pendingVdp2Regs->mF8_PRINA & 7;
    case NBG1:
        return (vdp2Controls.m4_pendingVdp2Regs->mF8_PRINA >> 8) & 7;
//    case NBG2:
 //       return vdp2Controls.m_pendingVdp2Regs->PRINB & 7;
    case NBG3:
        return (vdp2Controls.m4_pendingVdp2Regs->mFA_PRINB >> 8) & 7;
    case RBG0:
        return (vdp2Controls.m4_pendingVdp2Regs->mFC_PRIR) & 7;
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

s8 convertAxis(s16 inputValue)
{
    s16 converted = -inputValue / 256;

    if (abs(converted) < 32)
        converted = 0;

    if (converted > 127)
        return 127;

    if (converted < -128)
        return -128;

    return (s8)converted;
}

static bool         g_MousePressed[3] = { false, false, false };
/*
bool ImGui_ImplSDL2_ProcessEvent(const SDL_Event* event)
{
    ImGuiIO& io = ImGui::GetIO();
    switch (event->type)
    {
    case SDL_MOUSEWHEEL:
    {
        if (event->wheel.x > 0) io.MouseWheelH += 1;
        if (event->wheel.x < 0) io.MouseWheelH -= 1;
        if (event->wheel.y > 0) io.MouseWheel += 1;
        if (event->wheel.y < 0) io.MouseWheel -= 1;
        return true;
    }
    case SDL_MOUSEBUTTONDOWN:
    {
        if (event->button.button == SDL_BUTTON_LEFT) g_MousePressed[0] = true;
        if (event->button.button == SDL_BUTTON_RIGHT) g_MousePressed[1] = true;
        if (event->button.button == SDL_BUTTON_MIDDLE) g_MousePressed[2] = true;
        return true;
    }
    case SDL_TEXTINPUT:
    {
        io.AddInputCharactersUTF8(event->text.text);
        return true;
    }
    case SDL_KEYDOWN:
    case SDL_KEYUP:
    {
        int key = event->key.keysym.scancode;
        IM_ASSERT(key >= 0 && key < IM_ARRAYSIZE(io.KeysDown));
        io.KeysDown[key] = (event->type == SDL_KEYDOWN);
        io.KeyShift = ((SDL_GetModState() & KMOD_SHIFT) != 0);
        io.KeyCtrl = ((SDL_GetModState() & KMOD_CTRL) != 0);
        io.KeyAlt = ((SDL_GetModState() & KMOD_ALT) != 0);
#ifdef _WIN32
        io.KeySuper = false;
#else
        io.KeySuper = ((SDL_GetModState() & KMOD_GUI) != 0);
#endif
        return true;
    }
    }
    return false;
}
*/
void azelSdl2_StartFrame()
{
    int oldResolution[2];
    oldResolution[0] = outputResolution[0];
    oldResolution[1] = outputResolution[1];

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_TV)
    SDL_Metal_GetDrawableSize(gWindowBGFX, &outputResolution[0], &outputResolution[1]);
#else
    SDL_GetWindowSize(gWindowBGFX, &outputResolution[0], &outputResolution[1]);
#endif

    if ((oldResolution[0] != outputResolution[0]) || (oldResolution[1] != outputResolution[1]))
    {
        bgfx::reset(outputResolution[0], outputResolution[1]);

        if (bgfx::isValid(gBGFXVdp1PolyFB))
        {
            bgfx::destroy(gBGFXVdp1PolyFB);
        }

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_TV)
        internalResolution[0] = outputResolution[0];
        internalResolution[1] = outputResolution[1];
#else
        internalResolution[0] = outputResolution[0];
        internalResolution[1] = outputResolution[0] * (3.f / 4.f);
#endif
        // setup vdp1 Poly
        const uint64_t tsFlags = 0
            //| BGFX_SAMPLER_MIN_POINT
            //| BGFX_SAMPLER_MAG_POINT
            //| BGFX_SAMPLER_MIP_POINT
            | BGFX_SAMPLER_U_CLAMP
            | BGFX_SAMPLER_V_CLAMP
            ;

        vdp1BufferTexture = bgfx::createTexture2D(internalResolution[0], internalResolution[1], false, 0, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | tsFlags);
        vdp1DepthBufferTexture = bgfx::createTexture2D(internalResolution[0], internalResolution[1], false, 0, bgfx::TextureFormat::D24S8, BGFX_TEXTURE_RT | tsFlags);
        std::array<bgfx::Attachment, 2> vdp1BufferAt;
        vdp1BufferAt[0].init(vdp1BufferTexture);
        vdp1BufferAt[1].init(vdp1DepthBufferTexture);
        gBGFXVdp1PolyFB = bgfx::createFrameBuffer(2, &vdp1BufferAt[0], true);
    }

#ifndef USE_NULL_RENDERER
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (!isShipping())
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
        }
        
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

    graphicEngineStatus.m4514.m0_inputDevices[0].m16_pending.m6_buttonDown = 0;
    graphicEngineStatus.m4514.m0_inputDevices[0].m16_pending.m8_newButtonDown = 0;
    graphicEngineStatus.m4514.m0_inputDevices[0].m16_pending.mC_newButtonDown2 = 0;

#ifndef USE_NULL_RENDERER

    static SDL_GameController* controller = nullptr;
    if(controller == nullptr)
    {
        for (int i = 0; i < SDL_NumJoysticks(); ++i) {
            if (SDL_IsGameController(i)) {
                controller = SDL_GameControllerOpen(i);
                if (controller) {
                    break;
                }
                else {
                    fprintf(stderr, "Could not open gamecontroller %i: %s\n", i, SDL_GetError());
                }
            }
        }
    }

    if (controller)
    {
        graphicEngineStatus.m4514.m0_inputDevices[0].m16_pending.m0_inputType = 2;

        u16 buttonMask = 0;
        buttonMask |= SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A) << 0;
        buttonMask |= SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_B) << 1;
        buttonMask |= SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_Y) << 2;
        buttonMask |= SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_START) << 3;
        buttonMask |= SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP) << 4;
        buttonMask |= SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN) << 5;
        buttonMask |= SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT) << 6;
        buttonMask |= SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) << 7;

        graphicEngineStatus.m4514.m0_inputDevices[0].m16_pending.m6_buttonDown |= buttonMask;

        if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & buttonMask) == 0)
        {
            graphicEngineStatus.m4514.m0_inputDevices[0].m16_pending.m8_newButtonDown |= buttonMask;
            graphicEngineStatus.m4514.m0_inputDevices[0].m16_pending.mC_newButtonDown2 |= buttonMask;
        }

        // analog
        // need to remap range [-32768 to 32767] to [0 - 255]
        graphicEngineStatus.m4514.m0_inputDevices[0].m16_pending.m2_analogX = convertAxis(SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX));
        graphicEngineStatus.m4514.m0_inputDevices[0].m16_pending.m3_analogY = convertAxis(SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY));
    }
    //else
    {
        graphicEngineStatus.m4514.m0_inputDevices[0].m16_pending.m0_inputType = 1;

        const Uint8* keyState = SDL_GetKeyboardState(NULL);

        for (int i = 0; i < SDL_NUM_SCANCODES; i++)
        {
            if (keyState[i])
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
                    graphicEngineStatus.m4514.m0_inputDevices[0].m16_pending.m6_buttonDown |= buttonMask;

                    if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & buttonMask) == 0)
                    {
                        graphicEngineStatus.m4514.m0_inputDevices[0].m16_pending.m8_newButtonDown |= buttonMask;
                        graphicEngineStatus.m4514.m0_inputDevices[0].m16_pending.mC_newButtonDown2 |= buttonMask;
                    }
                }
            }
        }
    }

    if (!isShipping())
    {
        //gBackend->ImGUI_NewFrame();
            // Pull the input from SDL2 instead
        ImGui_ImplSDL2_NewFrame(gWindowBGFX);
        //imguiBeginFrame(0, 0, 0, 0, outputResolution[0], outputResolution[1], -1);
        imguiBeginFrame(gUIState.mousex, gUIState.mousey, gUIState.mousedown, 0, outputResolution[0], outputResolution[1]);
    }
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
    s_layerData()
    {
        lineScrollEA = 0;
        zoomX = 0x10000;
        zoomY = 0x10000;
    }

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
    s32 outputHeight;

    s32 lineScrollEA;

    s32 zoomX;  // 16.16 fixed point, 0x10000 = 1.0
    s32 zoomY;
};

void renderLayerGPU(s_layerData& layerData, u32 textureWidth, u32 textureHeight, s_NBG_data& NBGData)
{
    // BGFX update texture size if needed, or recreate if switching from software mode
    if ((NBGData.m_currentWidth != textureWidth) || (NBGData.m_currentHeight != textureHeight) || !isValid(NBGData.BGFXFB))
    {
        if (isValid(NBGData.BGFXFB))
        {
            bgfx::destroy(NBGData.BGFXFB);
        }
        
        if (isValid(NBGData.BGFXTexture))
        {
            bgfx::destroy(NBGData.BGFXTexture);
        }
        

        const uint64_t tsFlags = 0
            //| BGFX_SAMPLER_MIN_POINT
            //| BGFX_SAMPLER_MAG_POINT
            //| BGFX_SAMPLER_MIP_POINT
            | BGFX_SAMPLER_U_CLAMP
            | BGFX_SAMPLER_V_CLAMP
            ;

        NBGData.BGFXFB = bgfx::createFrameBuffer(textureWidth, textureHeight, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | tsFlags);
        NBGData.BGFXTexture = bgfx::getTexture(NBGData.BGFXFB);

        NBGData.bgfx_vdp2_planeDataBuffer = bgfx::createTexture2D(20, 1, false, 1, bgfx::TextureFormat::RGBA8);

        NBGData.m_currentWidth = textureWidth;
        NBGData.m_currentHeight = textureHeight;
    }

    bgfx::setViewFrameBuffer(NBGData.viewId, NBGData.BGFXFB);
    {
        // BGFX version
        static bgfx::VertexBufferHandle quad_vertexbuffer = BGFX_INVALID_HANDLE;
        static bgfx::IndexBufferHandle quad_indexbuffer = BGFX_INVALID_HANDLE;
        static bgfx::UniformHandle texID_VDP2_RAM = BGFX_INVALID_HANDLE;
        static bgfx::UniformHandle texID_VDP2_CRAM = BGFX_INVALID_HANDLE;
        static bgfx::VertexLayout ms_layout;
        static bgfx::UniformHandle planeDataBuffer = BGFX_INVALID_HANDLE;
        static bgfx::UniformHandle u_CHSZ = BGFX_INVALID_HANDLE;
        static bgfx::UniformHandle u_CHCN = BGFX_INVALID_HANDLE;
        static bgfx::UniformHandle u_PNB = BGFX_INVALID_HANDLE;
        static bgfx::UniformHandle u_CNSM = BGFX_INVALID_HANDLE;
        static bgfx::UniformHandle u_CAOS = BGFX_INVALID_HANDLE;
        static bgfx::UniformHandle u_PLSZ = BGFX_INVALID_HANDLE;
        static bgfx::UniformHandle u_SCN = BGFX_INVALID_HANDLE;
        static bgfx::UniformHandle u_planeOffsets = BGFX_INVALID_HANDLE;
        static bgfx::UniformHandle u_scrollX = BGFX_INVALID_HANDLE;
        static bgfx::UniformHandle u_scrollY = BGFX_INVALID_HANDLE;
        static bgfx::UniformHandle u_lineScrollEA = BGFX_INVALID_HANDLE;
        static bgfx::UniformHandle u_outputHeight = BGFX_INVALID_HANDLE;

        static bool initialized = false;
        if (!initialized)
        {
            ms_layout
                .begin()
                .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
                .end();

            static const float g_quad_vertex_buffer_data[] = {
                -1.0f, -1.0f, 0.0f,
                1.0f, -1.0f, 0.0f,
                -1.0f,  1.0f, 0.0f,
                -1.0f,  1.0f, 0.0f,
                1.0f, -1.0f, 0.0f,
                1.0f,  1.0f, 0.0f,
            };

            static const short int g_quad_index_buffer_data[] = {
                0,1,2,3,4,5
            };

            quad_vertexbuffer = bgfx::createVertexBuffer(bgfx::copy(g_quad_vertex_buffer_data, sizeof(g_quad_vertex_buffer_data)), ms_layout);
            quad_indexbuffer = bgfx::createIndexBuffer(bgfx::copy(g_quad_index_buffer_data, sizeof(g_quad_index_buffer_data)));

            texID_VDP2_RAM = bgfx::createUniform("s_VDP2_RAM", bgfx::UniformType::Sampler);
            texID_VDP2_CRAM = bgfx::createUniform("s_VDP2_CRAM", bgfx::UniformType::Sampler);
            planeDataBuffer = bgfx::createUniform("s_planeConfig", bgfx::UniformType::Sampler);
            u_CHSZ = bgfx::createUniform("CHSZ", bgfx::UniformType::Vec4);
            u_CHCN = bgfx::createUniform("CHCN", bgfx::UniformType::Vec4);
            u_PNB = bgfx::createUniform("PNB", bgfx::UniformType::Vec4);
            u_CNSM = bgfx::createUniform("CNSM", bgfx::UniformType::Vec4);
            u_CAOS = bgfx::createUniform("CAOS", bgfx::UniformType::Vec4);
            u_PLSZ = bgfx::createUniform("PLSZ", bgfx::UniformType::Vec4);
            u_SCN = bgfx::createUniform("SCN", bgfx::UniformType::Vec4);
            u_planeOffsets = bgfx::createUniform("planeOffsets", bgfx::UniformType::Vec4, 1);
            u_scrollX = bgfx::createUniform("scrollX", bgfx::UniformType::Vec4);
            u_scrollY = bgfx::createUniform("scrollY", bgfx::UniformType::Vec4);
            u_lineScrollEA = bgfx::createUniform("lineScrollEA", bgfx::UniformType::Vec4);
            u_outputHeight = bgfx::createUniform("outputHeight", bgfx::UniformType::Vec4);

            initialized = true;
        }

        bgfx::setState(0
            | BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_A
        );
        
        bgfx::setViewRect(NBGData.viewId, 0, 0, textureWidth, textureHeight);
        bgfx::setViewClear(NBGData.viewId, BGFX_CLEAR_COLOR);

        bgfx::updateTexture2D(NBGData.bgfx_vdp2_planeDataBuffer, 0, 0, 0, 0, sizeof(layerData)/4, 1, bgfx::copy(&layerData, sizeof(layerData)));

        const float v_CHSZ[4] = { (float)layerData.CHSZ, 0.0f, 0.0f, 0.0f };
        const float v_CHCN[4] = { (float)layerData.CHCN, 0.0f, 0.0f, 0.0f };
        const float v_PNB[4]  = { (float)layerData.PNB,  0.0f, 0.0f, 0.0f };
        const float v_CNSM[4] = { (float)layerData.CNSM, 0.0f, 0.0f, 0.0f };
        const float v_CAOS[4] = { (float)layerData.CAOS, 0.0f, 0.0f, 0.0f };
        const float v_PLSZ[4] = { (float)layerData.PLSZ, 0.0f, 0.0f, 0.0f };
        const float v_SCN[4]  = { (float)layerData.SCN,  0.0f, 0.0f, 0.0f };
        const float v_planeOffsets[4] = {
            (float)layerData.planeOffsets[0],
            (float)layerData.planeOffsets[1],
            (float)layerData.planeOffsets[2],
            (float)layerData.planeOffsets[3],
        };
        const float v_scrollX[4] = { (float)layerData.scrollX, 0.0f, 0.0f, 0.0f };
        const float v_scrollY[4] = { (float)layerData.scrollY, 0.0f, 0.0f, 0.0f };
        const float v_lineScrollEA[4] = { (float)layerData.lineScrollEA, 0.0f, 0.0f, 0.0f };
        const float v_outputHeight[4] = { (float)textureHeight, 0.0f, 0.0f, 0.0f };

        bgfx::setUniform(u_CHSZ, v_CHSZ);
        bgfx::setUniform(u_CHCN, v_CHCN);
        bgfx::setUniform(u_PNB, v_PNB);
        bgfx::setUniform(u_CNSM, v_CNSM);
        bgfx::setUniform(u_CAOS, v_CAOS);
        bgfx::setUniform(u_PLSZ, v_PLSZ);
        bgfx::setUniform(u_SCN, v_SCN);
        bgfx::setUniform(u_planeOffsets, v_planeOffsets);
        bgfx::setUniform(u_scrollX, v_scrollX);
        bgfx::setUniform(u_scrollY, v_scrollY);
        bgfx::setUniform(u_lineScrollEA, v_lineScrollEA);
        bgfx::setUniform(u_outputHeight, v_outputHeight);

        bgfx::setTexture(0, texID_VDP2_RAM, bgfx_vdp2_ram_texture);
        bgfx::setTexture(1, texID_VDP2_CRAM, bgfx_vdp2_cram_texture);
        bgfx::setTexture(2, planeDataBuffer, NBGData.bgfx_vdp2_planeDataBuffer);

        bgfx::setVertexBuffer(0, quad_vertexbuffer);
        bgfx::setIndexBuffer(quad_indexbuffer);
        bgfx::submit(NBGData.viewId, bgfx_vdp2_program);

    }
    
    gBackend->bindBackBuffer();
}

void renderLayer(s_layerData& layerData, u32 textureWidth, u32 textureHeight, u32* textureOutput);

void renderLayerSoftware(s_layerData& layerData, u32 textureWidth, u32 textureHeight, s_NBG_data& NBGData)
{
    if ((NBGData.m_currentWidth != textureWidth) || (NBGData.m_currentHeight != textureHeight) || isValid(NBGData.BGFXFB))
    {
        if (isValid(NBGData.BGFXFB))
        {
            bgfx::destroy(NBGData.BGFXFB);
            NBGData.BGFXFB = BGFX_INVALID_HANDLE;
            NBGData.BGFXTexture = BGFX_INVALID_HANDLE;
        }

        const uint64_t tsFlags = 0
            | BGFX_SAMPLER_U_CLAMP
            | BGFX_SAMPLER_V_CLAMP
            ;

        NBGData.BGFXTexture = bgfx::createTexture2D(textureWidth, textureHeight, false, 1, bgfx::TextureFormat::BGRA8, tsFlags);

        NBGData.m_currentWidth = textureWidth;
        NBGData.m_currentHeight = textureHeight;
    }

    u32* textureOutput = new u32[textureWidth * textureHeight];
    renderLayer(layerData, textureWidth, textureHeight, textureOutput);

    bgfx::updateTexture2D(NBGData.BGFXTexture, 0, 0, 0, 0, textureWidth, textureHeight, bgfx::copy(textureOutput, textureWidth * textureHeight * 4));

    delete[] textureOutput;
}

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
            s32 outputX = rawOutputX;
            s32 outputY = rawOutputY;

            // Apply coordinate increment (zoom)
            outputX = (s32)((s64)outputX * layerData.zoomX >> 16);
            outputY = (s32)((s64)outputY * layerData.zoomY >> 16);

            if (layerData.lineScrollEA)
            {
                outputY = getVdp2VramU32(layerData.lineScrollEA + rawOutputY * 4) >> 16;
            }

            outputX += layerData.scrollX;
            outputY += layerData.scrollY;

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

                switch (layerData.CHCN)
                {
                case 0:
                    // assuming supplement mode 0 with no data
                    paletteNumber = (patternName & 0xF000) >> 12;
                    break;
                case 1:
                    paletteNumber = (patternName & 0x7000) >> 8;
                    break;
                default:
                    assert(0);
                }

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

void renderBG0(u32 width, u32 height, bool bGPU)
{
    u32 textureWidth = width;
    u32 textureHeight = height;

    if(vdp2Controls.m4_pendingVdp2Regs->m20_BGON & 0x1)
    {
        s_layerData planeData;
        planeData.CHSZ = vdp2Controls.m4_pendingVdp2Regs->m28_CHCTLA & 1;
        planeData.CHCN = (vdp2Controls.m4_pendingVdp2Regs->m28_CHCTLA >> 4) & 7;
        planeData.PNB = (vdp2Controls.m4_pendingVdp2Regs->m30_PNCN0 >> 15) & 0x1;
        planeData.CNSM = (vdp2Controls.m4_pendingVdp2Regs->m30_PNCN0 >> 14) & 0x1;
        planeData.CAOS = (vdp2Controls.m4_pendingVdp2Regs->mE4_CRAOFA) & 0x7;
        planeData.PLSZ = (vdp2Controls.m4_pendingVdp2Regs->m3A_PLSZ) & 3;
        planeData.SCN = (vdp2Controls.m4_pendingVdp2Regs->m30_PNCN0) & 0x1F;
        planeData.scrollX = vdp2Controls.m4_pendingVdp2Regs->m70_SCXN0 >> 16;
        planeData.scrollY = vdp2Controls.m4_pendingVdp2Regs->m74_SCYN0 >> 16;
        planeData.outputHeight = textureHeight;
        planeData.zoomX = vdp2Controls.m4_pendingVdp2Regs->m78_ZMXN0;
        planeData.zoomY = vdp2Controls.m4_pendingVdp2Regs->m7C_ZMYN0;

        u32 pageDimension = (planeData.CHSZ == 0) ? 64 : 32;
        u32 patternSize = (planeData.PNB == 0) ? 4 : 2;

        u32 pageSize = pageDimension * pageDimension * patternSize;

        u32 offset = (vdp2Controls.m4_pendingVdp2Regs->m3C_MPOFN & 7) << 6;

        planeData.planeOffsets[0] = (offset + (vdp2Controls.m4_pendingVdp2Regs->m40_MPABN0 & 0x3F)) * pageSize;
        planeData.planeOffsets[1] = (offset + ((vdp2Controls.m4_pendingVdp2Regs->m40_MPABN0 >> 8) & 0x3F)) * pageSize;
        planeData.planeOffsets[2] = (offset + (vdp2Controls.m4_pendingVdp2Regs->m42_MPCDN0 & 0x3F)) * pageSize;
        planeData.planeOffsets[3] = (offset + ((vdp2Controls.m4_pendingVdp2Regs->m42_MPCDN0 >> 8) & 0x3F)) * pageSize;

        if (!bGPU)
        {
            renderLayerSoftware(planeData, textureWidth, textureHeight, NBG_data[0]);
        }
        else
        {
            renderLayerGPU(planeData, textureWidth, textureHeight, NBG_data[0]);
        }
    }
}

void renderBG1(u32 width, u32 height, bool bGPU)
{
    u32 textureWidth = width;
    u32 textureHeight = height;

    if (vdp2Controls.m4_pendingVdp2Regs->m20_BGON & 0x2)
    {
        s_layerData planeData;

        planeData.CHSZ = (vdp2Controls.m4_pendingVdp2Regs->m28_CHCTLA >> 8) & 0x1;
        planeData.CHCN = (vdp2Controls.m4_pendingVdp2Regs->m28_CHCTLA >> 12) & 3;
        planeData.PNB = (vdp2Controls.m4_pendingVdp2Regs->m32_PNCN1 >> 15) & 0x1;
        planeData.CNSM = (vdp2Controls.m4_pendingVdp2Regs->m32_PNCN1 >> 14) & 0x1;
        planeData.CAOS = (vdp2Controls.m4_pendingVdp2Regs->mE4_CRAOFA >> 4) & 0x7;
        planeData.PLSZ = (vdp2Controls.m4_pendingVdp2Regs->m3A_PLSZ >> 2) & 3;
        planeData.SCN = (vdp2Controls.m4_pendingVdp2Regs->m32_PNCN1) & 0x1F;
        planeData.scrollX = vdp2Controls.m4_pendingVdp2Regs->m80_SCXN1 >> 16;
        planeData.scrollY = vdp2Controls.m4_pendingVdp2Regs->m84_SCYN1 >> 16;

        u32 pageDimension = (planeData.CHSZ == 0) ? 64 : 32;
        u32 patternSize = (planeData.PNB == 0) ? 4 : 2;

        u32 pageSize = pageDimension * pageDimension * patternSize;

        u32 offset = ((vdp2Controls.m4_pendingVdp2Regs->m3C_MPOFN >> 4) & 7) << 6;

        planeData.planeOffsets[0] = (offset + (vdp2Controls.m4_pendingVdp2Regs->m44_MPABN1 & 0x3F)) * pageSize;
        planeData.planeOffsets[1] = (offset + ((vdp2Controls.m4_pendingVdp2Regs->m44_MPABN1 >> 8) & 0x3F)) * pageSize;
        planeData.planeOffsets[2] = (offset + (vdp2Controls.m4_pendingVdp2Regs->m46_MPCDN1 & 0x3F)) * pageSize;
        planeData.planeOffsets[3] = (offset + ((vdp2Controls.m4_pendingVdp2Regs->m46_MPCDN1 >> 8) & 0x3F)) * pageSize;

        if (vdp2Controls.m4_pendingVdp2Regs->m9A_SCRCTL & 0x400)
        {
            planeData.lineScrollEA = getVdp2VramOffset(vdp2Controls.m4_pendingVdp2Regs->mA4_LSTA1) - 0x25E00000;
        }

        if (!bGPU)
        {
            renderLayerSoftware(planeData, textureWidth, textureHeight, NBG_data[1]);
        }
        else
        {
            renderLayerGPU(planeData, textureWidth, textureHeight, NBG_data[1]);
        }
    }
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

void renderBG3(u32 width, u32 height, bool bGPU)
{
    u32 textureWidth = width;
    u32 textureHeight = height;

    if (vdp2Controls.m4_pendingVdp2Regs->m20_BGON & 0x8)
    {
        s_layerData planeData;

        planeData.CHSZ = (vdp2Controls.m4_pendingVdp2Regs->m2A_CHCTLB >> 4) & 0x1;
        planeData.CHCN = (vdp2Controls.m4_pendingVdp2Regs->m2A_CHCTLB >> 5) & 0x1;
        planeData.PNB = (vdp2Controls.m4_pendingVdp2Regs->m36_PNCN3 >> 15) & 0x1;
        planeData.CNSM = (vdp2Controls.m4_pendingVdp2Regs->m36_PNCN3 >> 14) & 0x1;
        planeData.CAOS = (vdp2Controls.m4_pendingVdp2Regs->mE4_CRAOFA >> 12) & 0x7;
        planeData.PLSZ = (vdp2Controls.m4_pendingVdp2Regs->m3A_PLSZ >> 6) & 3;
        planeData.SCN = (vdp2Controls.m4_pendingVdp2Regs->m36_PNCN3) & 0x1F;
        planeData.scrollX = vdp2Controls.m4_pendingVdp2Regs->m94_SCXN3;
        planeData.scrollY = vdp2Controls.m4_pendingVdp2Regs->m96_SCYN3;

        u32 pageDimension = (planeData.CHSZ == 0) ? 64 : 32;
        u32 patternSize = (planeData.PNB == 0) ? 4 : 2;

        u32 pageSize = pageDimension * pageDimension * patternSize;

        u32 offset = ((vdp2Controls.m4_pendingVdp2Regs->m3C_MPOFN >> 12) & 7) << 6;

        planeData.planeOffsets[0] = (offset + (vdp2Controls.m4_pendingVdp2Regs->m4C_MPABN3 & 0x3F)) * pageSize;
        planeData.planeOffsets[1] = (offset + ((vdp2Controls.m4_pendingVdp2Regs->m4C_MPABN3 >> 8) & 0x3F)) * pageSize;
        planeData.planeOffsets[2] = (offset + (vdp2Controls.m4_pendingVdp2Regs->m4E_MPCDN3 & 0x3F)) * pageSize;
        planeData.planeOffsets[3] = (offset + ((vdp2Controls.m4_pendingVdp2Regs->m4E_MPCDN3 >> 8) & 0x3F)) * pageSize;

        if (!bGPU)
        {
            u32* textureOutput = new u32[textureWidth * textureHeight];
            renderLayer(planeData, textureWidth, textureHeight, textureOutput);

            assert(0); // need to update the texture in bgfx

            delete[] textureOutput;
        }
        else
        {
            renderLayerGPU(planeData, textureWidth, textureHeight, NBG_data[3]);
        }
    }
    
}

void renderRBG0(u32 width, u32 height, bool bGPU)
{
    u32 textureWidth = width;
    u32 textureHeight = height;

    if (vdp2Controls.m4_pendingVdp2Regs->m20_BGON & 0x10)
    {
        s_layerData planeData;

        planeData.CHSZ = (vdp2Controls.m4_pendingVdp2Regs->m2A_CHCTLB >> 8) & 0x1;
        planeData.CHCN = (vdp2Controls.m4_pendingVdp2Regs->m2A_CHCTLB >> 12) & 0x7;
        planeData.PNB = (vdp2Controls.m4_pendingVdp2Regs->m38_PNCR >> 15) & 0x1;
        planeData.CNSM = (vdp2Controls.m4_pendingVdp2Regs->m38_PNCR >> 14) & 0x1;
        planeData.CAOS = (vdp2Controls.m4_pendingVdp2Regs->mE6_CRAOFB >> 0) & 0x7;
        planeData.PLSZ = (vdp2Controls.m4_pendingVdp2Regs->m3A_PLSZ >> 8) & 3;
        planeData.SCN = (vdp2Controls.m4_pendingVdp2Regs->m38_PNCR) & 0x1F;
        planeData.scrollX = 0;
        planeData.scrollY = 0;
        planeData.outputHeight = textureHeight;

        u32 pageDimension = (planeData.CHSZ == 0) ? 64 : 32;
        u32 patternSize = (planeData.PNB == 0) ? 4 : 2;

        u32 pageSize = pageDimension * pageDimension * patternSize;

        u32 offset = ((vdp2Controls.m4_pendingVdp2Regs->m3E_MPOFR >> 0) & 7) << 6;

        planeData.planeOffsets[0] = (offset + (vdp2Controls.m4_pendingVdp2Regs->m50_MPABRA & 0x3F)) * pageSize;
        planeData.planeOffsets[1] = (offset + ((vdp2Controls.m4_pendingVdp2Regs->m50_MPABRA >> 8) & 0x3F)) * pageSize;
        planeData.planeOffsets[2] = (offset + (vdp2Controls.m4_pendingVdp2Regs->m52_MPCDRA & 0x3F)) * pageSize;
        planeData.planeOffsets[3] = (offset + ((vdp2Controls.m4_pendingVdp2Regs->m52_MPCDRA >> 8) & 0x3F)) * pageSize;

        if (!bGPU)
        {
            renderLayerSoftware(planeData, textureWidth, textureHeight, NBG_data[4]);
        }
        else
        {
            renderLayerGPU(planeData, textureWidth, textureHeight, NBG_data[4]);
        }
    }

}

std::vector<u32> vdp1TextureOutput;
u32 vdp1TextureWidth;
u32 vdp1TextureHeight;

s16 localCoordiantesX;
s16 localCoordiantesY;

void SetLocalCoordinates(s_vdp1Command* vdp1EA)
{
    localCoordiantesX = vdp1EA->mC_CMDXA;
    localCoordiantesY = vdp1EA->mE_CMDYA;
}

void NormalSpriteDraw(s_vdp1Command* vdp1EA)
{
    u16 CMDCTRL = vdp1EA->m0_CMDCTRL;
    u16 CMDPMOD = vdp1EA->m4_CMDPMOD;
    u16 CMDCOLR = vdp1EA->m6_CMDCOLR;
    u16 CMDSRCA = vdp1EA->m8_CMDSRCA;
    u16 CMDSIZE = vdp1EA->mA_CMDSIZE;
    s16 CMDXA = vdp1EA->mC_CMDXA;
    s16 CMDYA = vdp1EA->mE_CMDYA;
    u16 CMDGRDA = vdp1EA->m1C_CMDGRA;

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

void ScaledSpriteDraw(s_vdp1Command* vdp1EA)
{
    u16 CMDCTRL = vdp1EA->m0_CMDCTRL;
    u16 CMDPMOD = vdp1EA->m4_CMDPMOD;
    u16 CMDCOLR = vdp1EA->m6_CMDCOLR;
    u16 CMDSRCA = vdp1EA->m8_CMDSRCA;
    u16 CMDSIZE = vdp1EA->mA_CMDSIZE;
    s16 CMDXA = vdp1EA->mC_CMDXA;
    s16 CMDYA = vdp1EA->mE_CMDYA;
    u16 CMDGRDA = vdp1EA->m1C_CMDGRA;

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

        switch ((CMDCTRL >> 8) & 0xF)
        {
        case 0:
            X1 = vdp1EA->m14_CMDXC + localCoordiantesX + 1;
            Y1 = vdp1EA->m16_CMDYC + localCoordiantesY + 1;
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

void PolyLineDraw(s_vdp1Command* vdp1EA)
{
    u16 CMDPMOD = vdp1EA->m4_CMDPMOD;
    u16 CMDCOLR = vdp1EA->m6_CMDCOLR;
    s16 CMDXA = vdp1EA->mC_CMDXA;
    s16 CMDYA = vdp1EA->mE_CMDYA;
    s16 CMDXB = vdp1EA->m10_CMDXB;
    s16 CMDYB = vdp1EA->m12_CMDYB;
    s16 CMDXC = vdp1EA->m14_CMDXC;
    s16 CMDYC = vdp1EA->m16_CMDYC;
    s16 CMDXD = vdp1EA->m18_CMDXD;
    s16 CMDYD = vdp1EA->m1A_CMDYD;
    u16 CMDGRDA = vdp1EA->m1C_CMDGRA;

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
    ZoneScopedN("renderVdp1ToGL");

    s_vdp1Command* vdp1EA = &mainContextVdp1[0][0];

    while (1)
    {
        s_vdp1Command& command = *vdp1EA;
        u16 CMDCTRL = command.m0_CMDCTRL;
        s_vdp1Command* CMDLINK = command.m2_CMDLINK;

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
            NormalSpriteDrawGL(&command);
            break;
        case 1:
            ScaledSpriteDrawGL(&command);
            break;
        case 2:
            // distorted sprite draw
            PolyLineDrawGL(&command);
            break;
        case 4:
            // draw polygon
            PolyDrawGL(&command);
            break;
        case 5:
            PolyLineDrawGL(&command);
            break;
        case 8:
            // user clipping coordinates
            break;
        case 9:
            // system clipping coordinates
            break;
        case 0xA:
            SetLocalCoordinates(&command);
            break;
        default:
            assert(0);
            break;
        }

        switch (JP)
        {
        case 0:
            vdp1EA++;
            break;
        case 1:
            vdp1EA = vdp1EA->m2_CMDLINK;
            break;
        default:
            assert(0);
        }

    }
}

void renderVdp1()
{
    s_vdp1Command* vdp1EA = &mainContextVdp1[0][0];

    while (1)
    {
        s_vdp1Command& command = *vdp1EA;
        u16 CMDCTRL = command.m0_CMDCTRL;
        s_vdp1Command* CMDLINK = command.m2_CMDLINK;

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
            NormalSpriteDraw(&command);
            break;
        case 1:
            ScaledSpriteDraw(&command);
            break;
        case 2:
            // distorted sprite draw
            PolyLineDraw(&command);
            break;
        case 4:
            // draw polygon
            break;
        case 5:
            PolyLineDraw(&command);
            break;
        case 8:
            // user clipping coordinates
            break;
        case 9:
            // system clipping coordinates
            break;
        case 0xA:
            SetLocalCoordinates(&command);
            break;
        default:
            assert(0);
            break;
        }

        switch (JP)
        {
        case 0:
            vdp1EA++;
            break;
        case 1:
            vdp1EA = vdp1EA->m2_CMDLINK;
            break;
        default:
            assert(0);
        }
        
    }
}

void renderTexturedQuadBgfx(bgfx::ViewId outputView, bgfx::TextureHandle sourceTexture)
{
    if (!bgfx::isValid(sourceTexture))
    {
        return;
    }
    static bgfx::ProgramHandle program = BGFX_INVALID_HANDLE;
    static bgfx::VertexBufferHandle quad_vertexbuffer = BGFX_INVALID_HANDLE;
    static bgfx::IndexBufferHandle quad_indexbuffer = BGFX_INVALID_HANDLE;
    static bgfx::UniformHandle inputTexture = BGFX_INVALID_HANDLE;
    static bgfx::VertexLayout ms_layout;

    static bool initialized = false;
    if (!initialized)
    {
        program = loadBgfxProgram("VDP2_blit_vs", "VDP2_blit_ps");

        ms_layout
            .begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .end();

        static const float g_quad_vertex_buffer_data[] = {
            -1.0f, -1.0f, 0.0f,
            1.0f, -1.0f, 0.0f,
            -1.0f,  1.0f, 0.0f,
            -1.0f,  1.0f, 0.0f,
            1.0f, -1.0f, 0.0f,
            1.0f,  1.0f, 0.0f,
        };

        static const short int g_quad_index_buffer_data[] = {
            0,1,2,3,4,5
        };

        quad_vertexbuffer = bgfx::createVertexBuffer(bgfx::copy(g_quad_vertex_buffer_data, sizeof(g_quad_vertex_buffer_data)), ms_layout);
        quad_indexbuffer = bgfx::createIndexBuffer(bgfx::copy(g_quad_index_buffer_data, sizeof(g_quad_index_buffer_data)));

        inputTexture = bgfx::createUniform("s_texture", bgfx::UniformType::Sampler);

        initialized = true;
    }

    bgfx::setState(0
        | BGFX_STATE_WRITE_RGB
        | BGFX_STATE_WRITE_A
        | BGFX_STATE_DEPTH_TEST_ALWAYS
        | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
    );

    bgfx::setViewRect(outputView, 0, 0, internalResolution[0], internalResolution[1]);

    bgfx::setTexture(0, inputTexture, sourceTexture);

    bgfx::setVertexBuffer(0, quad_vertexbuffer);
    bgfx::setIndexBuffer(quad_indexbuffer);
    bgfx::submit(outputView, program);
}

bool azelSdl2_EndFrame()
{
    u32 outputResolutionWidth = 0;
    u32 outputResolutionHeight = 0;

    u32 LSMD = (vdp2Controls.m4_pendingVdp2Regs->m0_TVMD >> 6) & 3;
    u32 VRESO = (vdp2Controls.m4_pendingVdp2Regs->m0_TVMD >> 4) & 3;
    u32 HRESO = (vdp2Controls.m4_pendingVdp2Regs->m0_TVMD) & 7;

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
    case 0:
    case 4:
        outputResolutionWidth = 320;
        break;
    case 1:
    case 5:
        outputResolutionWidth = 352;
        break;
    case 2:
    case 6:
        outputResolutionWidth = 320;
        break;
    case 3:
    case 7:
        outputResolutionWidth = 352;
        break;
    default:
        assert(0);
        break;
    }

    u32 vdp2ResolutionWidth = (HRESO == 2 || HRESO == 6) ? 640 : (HRESO == 3 || HRESO == 7) ? 704 : outputResolutionWidth;
    u32 vdp2ResolutionHeight = (LSMD >= 2) ? (outputResolutionHeight * 2) : outputResolutionHeight;

    {
        vdp1TextureWidth = outputResolutionWidth;
        vdp1TextureHeight = outputResolutionHeight;
        vdp1TextureOutput.resize(vdp1TextureWidth * vdp1TextureHeight);
        memset(&vdp1TextureOutput[0], 0x00, vdp1TextureWidth * vdp1TextureHeight * 4);
    }

    if (!useVDP1GL)
    {
        renderVdp1();
    }

    {
        bgfx::updateTexture2D(bgfx_vdp1_ram_texture, 0, 0, 0, 0, 256, 0x80000 / 256, bgfx::makeRef(getVdp1Pointer(0x25C00000), 0x80000));
        bgfx::updateTexture2D(bgfx_vdp2_ram_texture, 0, 0, 0, 0, 256, 0x80000 / 256, bgfx::makeRef(getVdp2Vram(0), 0x80000));
        bgfx::updateTexture2D(bgfx_vdp2_cram_texture, 0, 0, 0, 0, 256, 0x1000 / 256, bgfx::makeRef(getVdp2Cram(0), 0x1000));
    }

    static bool BG0_GPU = true;
    static bool BG1_GPU = true;
    static bool BG2_GPU = true;
    static bool BG3_GPU = true;
    static bool RBG0_GPU = false;

    renderBG0(vdp2ResolutionWidth, vdp2ResolutionHeight, BG0_GPU);
    renderBG1(vdp2ResolutionWidth, vdp2ResolutionHeight, BG1_GPU);
    //renderBG2(vdp2ResolutionWidth, vdp2ResolutionHeight);
    renderBG3(vdp2ResolutionWidth, vdp2ResolutionHeight, BG3_GPU);
    renderRBG0(vdp2ResolutionWidth, vdp2ResolutionHeight, RBG0_GPU);

    if(!isShipping())
    {
        if(ImGui::Begin("VDP"))
        {
            ImGui::PushID("BG0");
            ImGui::Text("NBG0"); ImGui::SameLine(); ImGui::Checkbox("GPU", &BG0_GPU);
            ImGui::Image(ImGui::toId(NBG_data[0].BGFXTexture,0,0), ImVec2(vdp2ResolutionWidth, vdp2ResolutionHeight), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::PopID();

            ImGui::PushID("BG1");
            ImGui::Text("NBG1"); ImGui::SameLine(); ImGui::Checkbox("GPU", &BG1_GPU);
            ImGui::Image(ImGui::toId(NBG_data[1].BGFXTexture, 0, 0), ImVec2(vdp2ResolutionWidth, vdp2ResolutionHeight), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::PopID();

            ImGui::PushID("BG2");
            ImGui::Text("NBG2"); ImGui::SameLine(); ImGui::Checkbox("GPU", &BG2_GPU);
            ImGui::Image(ImGui::toId(NBG_data[2].BGFXTexture, 0, 0), ImVec2(vdp2ResolutionWidth, vdp2ResolutionHeight), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::PopID();

            ImGui::PushID("BG3");
            ImGui::Text("NBG3"); ImGui::SameLine(); ImGui::Checkbox("GPU", &BG3_GPU);
            ImGui::Image(ImGui::toId(NBG_data[3].BGFXTexture, 0, 0), ImVec2(vdp2ResolutionWidth, vdp2ResolutionHeight), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::PopID();

            ImGui::PushID("RBG0");
            ImGui::Text("RBG0"); ImGui::SameLine(); ImGui::Checkbox("GPU", &RBG0_GPU);
            ImGui::Image(ImGui::toId(NBG_data[4].BGFXTexture, 0, 0), ImVec2(vdp2ResolutionWidth, vdp2ResolutionHeight), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::PopID();

            //ImGui::Text("VDP1");
            //ImGui::Image((ImTextureID)gVdp1Texture, ImVec2(vdp2ResolutionWidth, vdp2ResolutionHeight), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::Text("VDP1 poly");
            ImGui::Image(ImGui::toId(bgfx::getTexture(gBGFXVdp1PolyFB), 0, 0), ImVec2(vdp2ResolutionWidth, vdp2ResolutionHeight), ImVec2(0, 1), ImVec2(1, 0));
        }
        ImGui::End();

        PrintDebugTasksHierarchy();
        PrintDebugTasksInfo();
    }

    if(!isShipping())
    {
        if (ImGui::Begin("Config"))
        {
            ImGui::InputInt2("Internal Resolution", internalResolution);
        }
        ImGui::End();
    }

    // render VDP1 frame buffer
    if(1)
    {
        bgfx::setViewFrameBuffer(vdp1_gpuView, gBGFXVdp1PolyFB);
        bgfx::setViewRect(vdp1_gpuView, 0, 0, internalResolution[0], internalResolution[1]);

        bgfx::setViewClear(vdp1_gpuView, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0);

        bgfx::setViewName(vdp1_gpuView, "Vdp1");
        bgfx::setViewMode(vdp1_gpuView, bgfx::ViewMode::Sequential);

        flushObjectsToDrawList();

        if (useVDP1GL)
        {
            renderVdp1ToGL(internalResolution[0], internalResolution[1]);
        }
    }
    
    //Compose
#ifndef USE_NULL_RENDERER
    {
        ZoneScopedN("Compose");

        //get clear color of back screen
        u32 backscreenColorAddress = (vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA & 0x7FFFF) * 2;
        u16 backScreenColor = *(u16*)getVdp2Vram(backscreenColorAddress);

        float R = ((backScreenColor & 0x1F) << 3) >> 0;
        float G = ((backScreenColor & 0x03E0) << 6) >> 8;
        float B = ((backScreenColor & 0x7C00) << 9) >> 16;

        // TODOL set the bgfx back color to this color

        for (int priorityIndex = 0; priorityIndex <= 7; priorityIndex++)
        {
            for (eLayers layerIndex = SPRITE_POLY; layerIndex < eLayers::MAX; layerIndex = (eLayers)(layerIndex + 1))
            {
                if (isBackgroundEnabled(layerIndex) && (getPriorityForLayer(layerIndex) == priorityIndex))
                {
                    renderTexturedQuadBgfx(CompositeView, getTextureForLayerBgfx(layerIndex));
                }
            }
            
        }
    }
#endif

    gBackend->bindBackBuffer();

    if (!isShipping())
    {
        ImGui::Begin("Final Composition");
        {
            ImVec2 textureSize = ImGui::GetWindowSize();
            textureSize.y = textureSize.x * (224.f / 352.f);
            //ImGui::Image((ImTextureID)gCompositedTexture, textureSize, ImVec2(0, 1), ImVec2(1, 0)); ImGui::SameLine();
        }
        ImGui::End();

        static bool bInventoryOpen = false;

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

            if (ImGui::BeginMenu("GameState"))
            {
                ImGui::MenuItem("Inventory", NULL, &bInventoryOpen);
                ImGui::EndMenu();
            }

            extern bool bTraceEnabled;
            ImGui::Checkbox("Trace", &bTraceEnabled);

            ImGui::EndMainMenuBar();
        }

        if (bInventoryOpen)
        {
            ImGui::Begin("Inventory");

            ImGui::Columns(4);
            for (int i = 0; i < 0xB0; i++)
            {
                const sObjectListEntry* pObject = getObjectListEntry((eItems)i);

                ImGui::PushID(i);

                ImGui::Text("0x%04X", i); ImGui::NextColumn();
                ImGui::Text(pObject->m4_name.c_str()); ImGui::NextColumn();
                ImGui::Text(pObject->m8_description.c_str()); ImGui::NextColumn();

                if (i < 0x4D)
                {
                    int owned = mainGameState.consumables[i];
                    if (ImGui::InputInt("owned", &owned))
                    {
                        mainGameState.consumables[i] = owned;
                    }
                }
                else
                {
                    bool owned = mainGameState.getBit(0xF3 + i);
                    if (ImGui::Checkbox("owned", &owned))
                    {
                        if (owned)
                        {
                            mainGameState.setBit(0xF3 + i);
                        }
                        else
                        {
                            mainGameState.clearBit(0xF3 + i);
                        }

                    }
                }
                ImGui::NextColumn();
                ImGui::Separator();

                ImGui::PopID();
            }
            ImGui::Columns(1);
            ImGui::End();
        }
    }

    if(!isShipping())
    {
#if !defined(SHIPPING_BUILD)
        for (int i = 0; i < eLogCategories::log_max; i++)
        {
            switch (i)
            {
            case eLogCategories::log_default:
                PDS_Logger[i].Draw("Default log");
                break;
            case eLogCategories::log_task:
                PDS_Logger[i].Draw("Task log");
                break;
            case eLogCategories::log_unimlemented:
                PDS_Logger[i].Draw("Unimplemented log");
                break;
            case eLogCategories::log_m68k:
                PDS_Logger[i].Draw("Sound m68k");
                break;
            case eLogCategories::log_warning:
                PDS_Logger[i].Draw("Warning log");
                break;
            default:
                assert(0);
                break;
            }
        }
#endif
    }
    

    static bool bImguiEnabled = false;

#ifndef USE_NULL_RENDERER
#ifndef SHIPPING_BUILD
    if (ImGui::GetIO().KeysDown[SDL_SCANCODE_GRAVE] && (ImGui::GetIO().KeysDownDuration[SDL_SCANCODE_GRAVE] == 0.f))
    {
        bImguiEnabled = !bImguiEnabled;
    }
#endif

    if (bImguiEnabled)
    {
        imguiEndFrame();
        //gBackend->ImGUI_RenderDrawData(ImGui::GetDrawData());
        //imguiDestroy()
    }
    else
    {
        ImGui::Render();
        //renderTexturedQuadBgfx(0, bgfx::getTexture(gBgfxCompositedFB));
    }
    
    bgfx::frame();

    {
        ZoneScopedN("WaitNextFrame");

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

        last_time = SDL_GetPerformanceCounter();
    }
#endif

    FrameMark;

    return !closeApp;
}

