#include "PDS.h"
#include <soloud.h>

#include "renderer/renderer.h"
#include "renderer/renderer_gl.h"
#include "renderer/renderer_vk.h"

#define IMGUI_API
#include "imgui_impl_sdl.h"

#include "items.h"

extern SDL_Window* gWindowGL;
extern SDL_GLContext gGlcontext;
extern const char* gGLSLVersion;

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

int internalResolution[2] = { 1024, 720 };

bgfx::FrameBufferHandle gBGFXVdp1PolyFB = BGFX_INVALID_HANDLE;
bgfx::TextureHandle vdp1BufferTexture = BGFX_INVALID_HANDLE;
bgfx::TextureHandle vdp1DepthBufferTexture = BGFX_INVALID_HANDLE;

GLuint gVdp1PolyFB = 0;
GLuint gVdp1PolyTexture = 0;
GLuint gVdp1PolyDepth = 0;

bgfx::FrameBufferHandle gBgfxCompositedFB = BGFX_INVALID_HANDLE;
GLuint gCompositedFB = 0;
GLuint gCompositedTexture = 0;

GLuint gVdp1Texture = 0;

bgfx::TextureHandle bgfx_vdp1_ram_texture = BGFX_INVALID_HANDLE;
bgfx::TextureHandle bgfx_vdp2_ram_texture = BGFX_INVALID_HANDLE;
bgfx::TextureHandle bgfx_vdp2_cram_texture = BGFX_INVALID_HANDLE;

GLuint vdp1_ram_texture = 0;
GLuint vdp2_ram_texture = 0;
GLuint vdp2_cram_texture = 0;

struct s_NBG_data
{
    bgfx::FrameBufferHandle BGFXFB = BGFX_INVALID_HANDLE;
    bgfx::TextureHandle BGFXTexture = BGFX_INVALID_HANDLE;
    bgfx::TextureHandle bgfx_vdp2_planeDataBuffer = BGFX_INVALID_HANDLE;
    bgfx::ViewId viewId = -1;
    int planeId = -1;
    GLuint FB;
    GLuint Texture;

    int m_currentWidth = -1;
    int m_currentHeight = -1;
};

std::array<s_NBG_data, 5> NBG_data;

GLuint gVDP2Program = 0;
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
"	vec4 txcol = texture(s_texture, v_texcoord);		\n"
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
    RBG0,

    MAX
};

backend* gBackend = nullptr;

GLuint compileShader(const char* VS, const char* PS);

bool loadFileToVector(std::vector<char>& outputVector, const std::string& filename)
{
    FILE* fHandle = fopen(filename.c_str(), "rb");
    if (fHandle == nullptr)
        return false;
    fseek(fHandle, 0, SEEK_END);
    u32 size = ftell(fHandle);
    fseek(fHandle, 0, SEEK_SET);
    outputVector.resize(size);
    fread(&outputVector[0], 1, size, fHandle);
    fclose(fHandle);

    outputVector.push_back('\0');
}

GLuint compileShaderFromFiles(const std::string& VSFile, const std::string& PSFile)
{
    std::vector<char> VSSource;
    std::vector<char> PSSource;

    loadFileToVector(VSSource, std::string("shaders/") + VSFile);
    loadFileToVector(PSSource, std::string("shaders/") + PSFile);

    return compileShader(&VSSource[0], &PSSource[0]);
}

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
    case bgfx::RendererType::Metal:     shaderFileExtension = ".metal.bin";  break;
    default:
        assert(0);
    }

    bgfx::ShaderHandle vsh = loadBgfxShader(std::string("shaders/generated/") + VSFile + shaderFileExtension);
    bgfx::ShaderHandle psh = loadBgfxShader(std::string("shaders/generated/") + PSFile + shaderFileExtension);
    
    /*
    std::array<bgfx::UniformHandle, 32> uniforms;
    int numUniforms = bgfx::getShaderUniforms(psh, &uniforms[0], 32);
    for (int i = 0; i < numUniforms; i++)
    {
        bgfx::UniformInfo info;
        bgfx::getUniformInfo(uniforms[i], info);
    }
    */
    return bgfx::createProgram(vsh, psh, true /* destroy shaders when program is destroyed */);
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
    

    // setup vdp1 Poly
    const uint64_t tsFlags = 0
        | BGFX_SAMPLER_MIN_POINT
        | BGFX_SAMPLER_MAG_POINT
        | BGFX_SAMPLER_MIP_POINT
        | BGFX_SAMPLER_U_CLAMP
        | BGFX_SAMPLER_V_CLAMP
        ;

    vdp1BufferTexture = bgfx::createTexture2D(internalResolution[0], internalResolution[1], false, 0, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | tsFlags);
    vdp1DepthBufferTexture = bgfx::createTexture2D(internalResolution[0], internalResolution[1], false, 0, bgfx::TextureFormat::D24S8, BGFX_TEXTURE_RT | tsFlags);
    std::array<bgfx::Attachment,2> vdp1BufferAt;
    vdp1BufferAt[0].init(vdp1BufferTexture);
    vdp1BufferAt[1].init(vdp1DepthBufferTexture);
    gBGFXVdp1PolyFB = bgfx::createFrameBuffer(2, &vdp1BufferAt[0], false);
    glGenFramebuffers(1, &gVdp1PolyFB);
    glGenTextures(1, &gVdp1PolyTexture);
    glGenRenderbuffers(1, &gVdp1PolyDepth);

    // Composited output
    glGenFramebuffers(1, &gCompositedFB);
    glGenTextures(1, &gCompositedTexture);

    for (int i = 0; i < NBG_data.size(); i++)
    {
        NBG_data[i].planeId = i;
        NBG_data[i].viewId = VDP2_viewsStart + i;
        assert(NBG_data[i].viewId <= VDP2_MAX);
        glGenTextures(1, &NBG_data[i].Texture);
        glGenFramebuffers(1, &NBG_data[i].FB);
    }
    glGenTextures(1, &gVdp1Texture);

    glGenTextures(1, &vdp1_ram_texture);
    glGenTextures(1, &vdp2_ram_texture);
    glGenTextures(1, &vdp2_cram_texture);

    bgfx_vdp1_ram_texture = bgfx::createTexture2D(256, 0x80000 / 256, false, 1, bgfx::TextureFormat::R8U, 0);
    bgfx_vdp2_ram_texture = bgfx::createTexture2D(256, 0x80000 / 256, false, 1, bgfx::TextureFormat::R8U, 0);
    bgfx_vdp2_cram_texture = bgfx::createTexture2D(256, 0x1000 / 256, false, 1, bgfx::TextureFormat::R8U, 0);

    gVDP2Program = compileShaderFromFiles("VDP2_vs.glsl", "VDP2_ps.glsl");
    bgfx_vdp2_program = loadBgfxProgram("VDP2_vs", "VDP2_ps");
#ifndef SHIPPING_BUILD
    SDL_GL_SetSwapInterval(0);
#endif

    checkGL();
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

GLuint getTextureForLayer(eLayers layerIndex)
{
    switch (layerIndex)
    {
    case SPRITE_POLY:
        return gVdp1PolyTexture;
    case SPRITE_SOFTWARE:
        return gVdp1Texture;
    case NBG0:
        return NBG_data[0].Texture;
    case NBG1:
        return NBG_data[1].Texture;
    //case NBG2:
    //    return gNBG2Texture;
    case NBG3:
        return NBG_data[3].Texture;
    case RBG0:
        return NBG_data[4].Texture;
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

void azelSdl2_StartFrame()
{
#ifndef USE_NULL_RENDERER
    checkGL();
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
    else
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

    checkGL();

    if (!isShipping())
    {
        gBackend->ImGUI_NewFrame();
        ImGui_ImplSDL2_NewFrame(gWindowGL);
        ImGui::NewFrame();
    }
    
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
    s_layerData()
    {
        lineScrollEA = 0;
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

    s32 lineScrollEA;
};

void renderLayerGPU(s_layerData& layerData, u32 textureWidth, u32 textureHeight, s_NBG_data& NBGData)
{
    // BGFX update texture size if needed
    if ((NBGData.m_currentWidth != textureWidth) || (NBGData.m_currentHeight != textureHeight))
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
            | BGFX_SAMPLER_MIN_POINT
            | BGFX_SAMPLER_MAG_POINT
            | BGFX_SAMPLER_MIP_POINT
            | BGFX_SAMPLER_U_CLAMP
            | BGFX_SAMPLER_V_CLAMP
            ;

        NBGData.BGFXFB = bgfx::createFrameBuffer(textureWidth, textureHeight, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | tsFlags);
        NBGData.BGFXTexture = bgfx::getTexture(NBGData.BGFXFB);

        NBGData.bgfx_vdp2_planeDataBuffer = bgfx::createTexture2D(16, 16, 0, 0, bgfx::TextureFormat::R32U);

        bgfx::setViewFrameBuffer(NBGData.viewId, NBGData.BGFXFB);

        NBGData.m_currentWidth = textureWidth;
        NBGData.m_currentHeight = textureHeight;
    }

    {
        static GLuint quad_VertexArrayID;
        static GLuint quad_vertexbuffer = 0;
        static GLint texID_VDP2_RAM = 0;
        static GLint texID_VDP2_CRAM = 0;

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

            glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);

            texID_VDP2_RAM = glGetUniformLocation(gVDP2Program, "s_VDP2_RAM");
            texID_VDP2_CRAM = glGetUniformLocation(gVDP2Program, "s_VDP2_CRAM");
            initialized = true;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, NBGData.FB);
        glBindTexture(GL_TEXTURE_2D, NBGData.Texture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, NBGData.Texture, 0);

        GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

        assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);


        glViewport(0, 0, textureWidth, textureHeight);

        glClearColor(1, 0, 0, 1);
        glClearDepthf(0.f);

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(gVDP2Program);

        if(texID_VDP2_RAM > -1)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, vdp2_ram_texture);
            glUniform1i(glGetUniformLocation(gVDP2Program, "s_VDP2_RAM"), 0);
        }

        if (texID_VDP2_CRAM > -1)
        {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, vdp2_cram_texture);
            glUniform1i(glGetUniformLocation(gVDP2Program, "s_VDP2_CRAM"), 1);
        }

        glUniform1i(glGetUniformLocation(gVDP2Program, "CHSZ"), layerData.CHSZ);
        glUniform1i(glGetUniformLocation(gVDP2Program, "CHCN"), layerData.CHCN);
        glUniform1i(glGetUniformLocation(gVDP2Program, "PNB"), layerData.PNB);
        glUniform1i(glGetUniformLocation(gVDP2Program, "CNSM"), layerData.CNSM);
        glUniform1i(glGetUniformLocation(gVDP2Program, "CAOS"), layerData.CAOS);
        glUniform1i(glGetUniformLocation(gVDP2Program, "PLSZ"), layerData.PLSZ);
        glUniform1i(glGetUniformLocation(gVDP2Program, "SCN"), layerData.SCN);
        glUniform1i(glGetUniformLocation(gVDP2Program, "planeOffsets[0]"), layerData.planeOffsets[0]);
        glUniform1i(glGetUniformLocation(gVDP2Program, "planeOffsets[1]"), layerData.planeOffsets[1]);
        glUniform1i(glGetUniformLocation(gVDP2Program, "planeOffsets[2]"), layerData.planeOffsets[2]);
        glUniform1i(glGetUniformLocation(gVDP2Program, "planeOffsets[3]"), layerData.planeOffsets[3]);
        glUniform1i(glGetUniformLocation(gVDP2Program, "scrollX"), layerData.scrollX);
        glUniform1i(glGetUniformLocation(gVDP2Program, "scrollY"), layerData.scrollY);
        glUniform1i(glGetUniformLocation(gVDP2Program, "outputHeight"), textureHeight);
        glUniform1i(glGetUniformLocation(gVDP2Program, "lineScrollEA"), layerData.lineScrollEA);

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
        checkGL();
        glDrawArrays(GL_TRIANGLES, 0, 6); // From index 0 to 3 -> 1 triangle

        glDisableVertexAttribArray(0);
    }

    {
        // BGFX version
        static bgfx::VertexBufferHandle quad_vertexbuffer = BGFX_INVALID_HANDLE;
        static bgfx::IndexBufferHandle quad_indexbuffer = BGFX_INVALID_HANDLE;
        static bgfx::UniformHandle texID_VDP2_RAM = BGFX_INVALID_HANDLE;
        static bgfx::UniformHandle texID_VDP2_CRAM = BGFX_INVALID_HANDLE;
        static bgfx::VertexLayout ms_layout;
        static bgfx::UniformHandle planeDataBuffer = BGFX_INVALID_HANDLE;

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

            initialized = true;
        }

        bgfx::setState(0
            | BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_A
        );
        
        bgfx::setViewRect(NBGData.viewId, 0, 0, textureWidth, textureHeight);
        bgfx::setViewClear(NBGData.viewId, BGFX_CLEAR_COLOR);

        bgfx::updateTexture2D(NBGData.bgfx_vdp2_planeDataBuffer, 0, 0, 0, 0, sizeof(layerData)/4, 1, bgfx::copy(&layerData, sizeof(layerData)));

        bgfx::setTexture(0, texID_VDP2_RAM, bgfx_vdp2_ram_texture);
        bgfx::setTexture(1, texID_VDP2_CRAM, bgfx_vdp2_cram_texture);
        bgfx::setTexture(2, planeDataBuffer, NBGData.bgfx_vdp2_planeDataBuffer);

        bgfx::setVertexBuffer(0, quad_vertexbuffer);
        bgfx::setIndexBuffer(quad_indexbuffer);
        bgfx::submit(NBGData.viewId, bgfx_vdp2_program);

    }
    
    gBackend->bindBackBuffer();
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

    if ((vdp2Controls.m4_pendingVdp2Regs[0].m0_TVMD & 0xC0) == 0xC0)
    {
        textureWidth *= 2;
        textureHeight *= 2;
    }

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
            u32* textureOutput = new u32[textureWidth * textureHeight];
            renderLayer(planeData, textureWidth, textureHeight, textureOutput);

#ifndef USE_NULL_RENDERER
            glBindTexture(GL_TEXTURE_2D, NBG_data[0].Texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureOutput);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#endif
            delete[] textureOutput;
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
            u32* textureOutput = new u32[textureWidth * textureHeight];
            renderLayer(planeData, textureWidth, textureHeight, textureOutput);
#ifndef USE_NULL_RENDERER
            glBindTexture(GL_TEXTURE_2D, NBG_data[1].Texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureOutput);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#endif

            delete[] textureOutput;
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
#ifndef USE_NULL_RENDERER
            glBindTexture(GL_TEXTURE_2D, NBG_data[3].Texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureOutput);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#endif
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
            u32* textureOutput = new u32[textureWidth * textureHeight];
            renderLayer(planeData, textureWidth, textureHeight, textureOutput);
#ifndef USE_NULL_RENDERER
            glBindTexture(GL_TEXTURE_2D, NBG_data[4].Texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureOutput);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#endif
            delete[] textureOutput;
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
    ZoneScopedN("renderVdp1ToGL");

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
            PolyDrawGL(vdp1EA);
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
    );

    bgfx::setViewRect(outputView, 0, 0, internalResolution[0], internalResolution[1]);
    bgfx::setViewClear(outputView, BGFX_CLEAR_COLOR);

    bgfx::setTexture(0, inputTexture, sourceTexture);

    bgfx::setVertexBuffer(0, quad_vertexbuffer);
    bgfx::setIndexBuffer(quad_indexbuffer);
    bgfx::submit(outputView, program);
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
    case 1:
        outputResolutionWidth = 352;
        break;
    case 3:
        // TODO: fix
        //outputResolutionWidth = 704; //this should be correct, but breaks the title screen
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
        vdp1TextureOutput.resize(vdp1TextureWidth * vdp1TextureHeight);
        memset(&vdp1TextureOutput[0], 0x00, vdp1TextureWidth * vdp1TextureHeight * 4);
    }

    if (!useVDP1GL)
    {
        renderVdp1();
    }
    
    {
#ifndef USE_NULL_RENDERER
        glBindTexture(GL_TEXTURE_2D, gVdp1Texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, vdp1TextureWidth, vdp1TextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &vdp1TextureOutput[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#endif
    }

    // update VDP buffers
    {
        glBindTexture(GL_TEXTURE_2D, vdp1_ram_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, 256, 0x80000 / 256, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, getVdp1Pointer(0x25C00000));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, vdp2_ram_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, 256, 0x80000 / 256, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, getVdp2Vram(0));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, vdp2_cram_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, 256, 0x1000 / 256, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, getVdp2Cram(0));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
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
            ImGui::Image((ImTextureID)NBG_data[0].Texture, ImVec2(vdp2ResolutionWidth, vdp2ResolutionHeight), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::PopID();

            ImGui::PushID("BG1");
            ImGui::Text("NBG1"); ImGui::SameLine(); ImGui::Checkbox("GPU", &BG1_GPU);
            ImGui::Image((ImTextureID)NBG_data[1].Texture, ImVec2(vdp2ResolutionWidth, vdp2ResolutionHeight), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::PopID();

            ImGui::PushID("BG2");
            ImGui::Text("NBG2"); ImGui::SameLine(); ImGui::Checkbox("GPU", &BG2_GPU);
            ImGui::Image((ImTextureID)NBG_data[2].Texture, ImVec2(vdp2ResolutionWidth, vdp2ResolutionHeight), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::PopID();

            ImGui::PushID("BG3");
            ImGui::Text("NBG3"); ImGui::SameLine(); ImGui::Checkbox("GPU", &BG3_GPU);
            ImGui::Image((ImTextureID)NBG_data[3].Texture, ImVec2(vdp2ResolutionWidth, vdp2ResolutionHeight), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::PopID();

            ImGui::PushID("RBG0");
            ImGui::Text("RBG0"); ImGui::SameLine(); ImGui::Checkbox("GPU", &RBG0_GPU);
            ImGui::Image((ImTextureID)NBG_data[4].Texture, ImVec2(vdp2ResolutionWidth, vdp2ResolutionHeight), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::PopID();

            ImGui::Text("VDP1");
            ImGui::Image((ImTextureID)gVdp1Texture, ImVec2(vdp2ResolutionWidth, vdp2ResolutionHeight), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::Text("VDP1 poly");
            ImGui::Image((ImTextureID)gVdp1PolyTexture, ImVec2(vdp2ResolutionWidth, vdp2ResolutionHeight), ImVec2(0, 1), ImVec2(1, 0));
        }
        ImGui::End();

        PrintDebugTasksHierarchy();
        PrintDebugTasksInfo();
    }


    checkGL();
   
    
#ifndef USE_NULL_RENDERER
    SDL_GL_GetDrawableSize(gWindowGL, &internalResolution[0], &internalResolution[1]);
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
#ifndef USE_NULL_RENDERER

        bgfx::setViewFrameBuffer(vdp1_gpuView, gBGFXVdp1PolyFB);
        bgfx::setViewRect(vdp1_gpuView, 0, 0, internalResolution[0], internalResolution[1]);

        bgfx::setViewClear(vdp1_gpuView, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0);

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

        glFrontFace(GL_CW);
        glCullFace(GL_BACK);
        glEnable(GL_CULL_FACE);
        checkGL();
#endif
        flushObjectsToDrawList();

#ifndef USE_NULL_RENDERER
        glDisable(GL_CULL_FACE);
#endif

        if (useVDP1GL)
        {
            renderVdp1ToGL(internalResolution[0], internalResolution[1]);
        }
    }
    
    //Compose
#ifndef USE_NULL_RENDERER
    {
        ZoneScopedN("Compose");

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

        //get clear color of back screen
        u32 backscreenColorAddress = (vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA & 0x7FFFF) * 2;
        u16 backScreenColor = *(u16*)getVdp2Vram(backscreenColorAddress);

        float R = ((backScreenColor & 0x1F) << 3) >> 0;
        float G = ((backScreenColor & 0x03E0) << 6) >> 8;
        float B = ((backScreenColor & 0x7C00) << 9) >> 16;

        glClearColor(R / 0xFF, G / 0xFF, B / 0xFF, 0x0);
        glClearDepthf(0.f);

        glClear(GL_COLOR_BUFFER_BIT);

        for (int priorityIndex = 0; priorityIndex <= 7; priorityIndex++)
        {
            for (eLayers layerIndex = SPRITE_POLY; layerIndex < eLayers::MAX; layerIndex = (eLayers)(layerIndex + 1))
            {
                if (isBackgroundEnabled(layerIndex) && (getPriorityForLayer(layerIndex) == priorityIndex))
                {
                    renderTexturedQuad(getTextureForLayer(layerIndex));
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
            ImGui::Image((ImTextureID)gCompositedTexture, textureSize, ImVec2(0, 1), ImVec2(1, 0)); ImGui::SameLine();
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

    checkGL();

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
        ImGui::Render();
    }
    
    
    checkGL();
    
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
        gBackend->ImGUI_RenderDrawData(ImGui::GetDrawData());
    }
    else
    {
        renderTexturedQuad(gCompositedTexture);
        //renderTexturedQuadBgfx(0, bgfx::getTexture(gBgfxCompositedFB));
    }
    
    checkGL();

    // Update and Render additional Platform Windows
#ifndef SHIPPING_BUILD
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        SDL_GL_MakeCurrent(gWindowGL, gGlcontext);
    }
#endif

    bgfx::setViewRect(0, 0, 0, uint16_t(200), uint16_t(200));
    bgfx::touch(0);

    bgfx::dbgTextClear();
    bgfx::dbgTextPrintf(0, 1, 0x0f, "Color can be changed with ANSI \x1b[9;me\x1b[10;ms\x1b[11;mc\x1b[12;ma\x1b[13;mp\x1b[14;me\x1b[0m code too.");


    bgfx::frame();

    glFlush();
    checkGL();
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

        SDL_GL_SwapWindow(gWindowGL);

        last_time = SDL_GetPerformanceCounter();
    }
    
    checkGL();
#endif

    FrameMark;

    return !closeApp;
}

