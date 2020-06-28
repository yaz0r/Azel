#include "PDS.h"
#include <bx/platform.h>

#ifdef WITH_GL

#define IMGUI_API
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include "renderer_gl.h"

#ifdef _WIN32
#pragma comment(lib, "Opengl32.lib")
#endif

extern "C" {
void* cbSetupMetalLayer(void*);
}

SDL_Window* gWindowGL = nullptr;
SDL_Window* gWindowBGFX = nullptr;
SDL_GLContext gGlcontext = nullptr;
extern const char* gGLSLVersion;
ImGuiContext* imguiGLContext = nullptr;

backend* SDL_ES3_backend::create()
{
    if (init())
    {
        return new SDL_ES3_backend();
    }
    return nullptr;
}

bool SDL_ES3_backend::init()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK) != 0)
    {
        assert(false);
    }

#ifdef USE_GL_ES3 
    const char* glsl_version = "#version 300 es\n";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
    const char* glsl_version = "#version 330\n";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
#endif

    gGLSLVersion = glsl_version;

    u32 flags = 0;
    flags |= SDL_WINDOW_RESIZABLE;
    flags |= SDL_WINDOW_ALLOW_HIGHDPI;

#ifdef __IPHONEOS__
    flags |= SDL_WINDOW_FULLSCREEN;
#endif

    int resolution[2] = { 1280, 814 };
#if (defined(__APPLE__) && (TARGET_OS_OSX))
    resolution[0] = 320;
    resolution[1] = 200;
#endif
    gWindowGL = SDL_CreateWindow("PDS: Azel", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, resolution[0], resolution[1], flags | SDL_WINDOW_OPENGL);
    assert(gWindowGL);

    gGlcontext = SDL_GL_CreateContext(gWindowGL);
    assert(gGlcontext);

    gWindowBGFX = SDL_CreateWindow("BGFX", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, resolution[0], resolution[1], flags);

    SDL_SysWMinfo wmi;
    SDL_VERSION(&wmi.version);
    if (!SDL_GetWindowWMInfo(gWindowBGFX, &wmi)) {
        return false;
    }

    bgfx::Init initparam;
#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
    initparam.platformData.ndt = wmi.info.x11.display;
    initparam.platformData.nwh = (void*)(uintptr_t)wmi.info.x11.window;
#elif BX_PLATFORM_OSX
    initparam.platformData.ndt = NULL;
    initparam.platformData.nwh = cbSetupMetalLayer(wmi.info.cocoa.window);
#elif BX_PLATFORM_WINDOWS
    initparam.platformData.ndt = NULL;
    initparam.platformData.nwh = wmi.info.win.window;
#elif BX_PLATFORM_STEAMLINK
    initparam.platformData.ndt = wmi.info.vivante.display;
    initparam.platformData.nwh = wmi.info.vivante.window;
#endif // BX_PLATFORM_

    bgfx::init(initparam);
    //bgfx::setDebug(BGFX_DEBUG_TEXT | BGFX_DEBUG_STATS);

#ifdef USE_GL
    gl3wInit();
#endif

#ifndef SHIPPING_BUILD
    imguiGLContext = ImGui::CreateContext();
    ImGui::SetCurrentContext(imguiGLContext);

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
#if !defined(__EMSCRIPTEN__) && !defined(TARGET_OS_IOS) && !defined(TARGET_OS_TV)
//    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
#endif
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

    printf("glsl_version: %s\n", gGLSLVersion);

    ImGui_ImplOpenGL3_Init(gGLSLVersion);
    ImGui_ImplSDL2_InitForOpenGL(gWindowGL, gGlcontext);
#endif

    return true;
}

void SDL_ES3_backend::bindBackBuffer()
{
#ifdef __IPHONEOS__
    SDL_SysWMinfo wmi;
    SDL_VERSION(&wmi.version);
    SDL_GetWindowWMInfo(gWindowGL, &wmi);

    glBindFramebuffer(GL_FRAMEBUFFER, wmi.info.uikit.framebuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, wmi.info.uikit.colorbuffer);
#else
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
    int internalResolution[2] = { 1024, 720 };
    SDL_GL_GetDrawableSize(gWindowGL, &internalResolution[0], &internalResolution[1]);
    glViewport(0, 0, internalResolution[0], internalResolution[1]);
}

void SDL_ES3_backend::ImGUI_NewFrame()
{
    if (!isShipping())
    {
        ImGui_ImplOpenGL3_NewFrame();
    }
}

void SDL_ES3_backend::ImGUI_RenderDrawData(ImDrawData* pDrawData)
{
    if (!isShipping())
    {
        ImGui_ImplOpenGL3_RenderDrawData(pDrawData);
    }
}

#endif
