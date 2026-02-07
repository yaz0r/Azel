#include "PDS.h"
#include <bx/platform.h>

//#define IMGUI_API
//#include "dear-imgui/examples/imgui_impl_sdl.h"
//#include "dear-imgui/examples/imgui_impl_opengl3.h"

#include "renderer_gl.h"

#ifdef _WIN32
#pragma comment(lib, "Opengl32.lib")
#endif

extern "C" {
void* cbSetupMetalLayer(void*);
}

SDL_Window* gWindowBGFX = nullptr;

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

    u32 flags = 0;
    flags |= SDL_WINDOW_RESIZABLE;
    flags |= SDL_WINDOW_ALLOW_HIGHDPI;

#ifdef __IPHONEOS__
    flags |= SDL_WINDOW_FULLSCREEN;
    flags |= SDL_WINDOW_METAL;
#endif

    int resolution[2] = { 1280, 960 };

    gWindowBGFX = SDL_CreateWindow("BGFX", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, resolution[0], resolution[1], flags);

    SDL_SysWMinfo wmi;
    SDL_VERSION(&wmi.version);
    if (!SDL_GetWindowWMInfo(gWindowBGFX, &wmi)) {
        return false;
    }

    bgfx::Init initparam;
#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
    if (wmi.subsystem == SDL_SYSWM_X11) {
        initparam.platformData.ndt = wmi.info.x11.display;
        initparam.platformData.nwh = (void*)(uintptr_t)wmi.info.x11.window;
    }
#if SDL_VERSION_ATLEAST(2, 0, 17)
    else if (wmi.subsystem == SDL_SYSWM_WAYLAND) {
        initparam.platformData.ndt = wmi.info.wl.display;
        initparam.platformData.nwh = wmi.info.wl.egl_window;
    }
#endif
#elif BX_PLATFORM_OSX
    initparam.platformData.ndt = NULL;
    initparam.platformData.nwh = cbSetupMetalLayer(wmi.info.cocoa.window);
#elif BX_PLATFORM_IOS
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");
    SDL_Renderer* pRenderer = SDL_CreateRenderer(gWindowBGFX, -1, 0);
    initparam.platformData.ndt = NULL;
    initparam.platformData.nwh = SDL_RenderGetMetalLayer(pRenderer);
    initparam.type = bgfx::RendererType::Metal;
#elif BX_PLATFORM_WINDOWS
    initparam.platformData.ndt = NULL;
    initparam.platformData.nwh = wmi.info.win.window;
#elif BX_PLATFORM_STEAMLINK
    initparam.platformData.ndt = wmi.info.vivante.display;
    initparam.platformData.nwh = wmi.info.vivante.window;
#endif // BX_PLATFORM_

    // Use Vulkan for better integer precision and driver support on Linux
    // initparam.type = bgfx::RendererType::OpenGL;
    // initparam.type = bgfx::RendererType::Count; // Auto-detect
    // initparam.type = bgfx::RendererType::Metal;
    initparam.type = bgfx::RendererType::Vulkan;
    bgfx::init(initparam);

    //bgfx::setDebug(BGFX_DEBUG_TEXT | BGFX_DEBUG_STATS);

    return true;
}

void SDL_ES3_backend::bindBackBuffer()
{
}

void SDL_ES3_backend::ImGUI_NewFrame()
{
    if (!isShipping())
    {
        //ImGui_ImplOpenGL3_NewFrame();
    }
}

void SDL_ES3_backend::ImGUI_RenderDrawData(ImDrawData* pDrawData)
{
    if (!isShipping())
    {
        //ImGui_ImplOpenGL3_RenderDrawData(pDrawData);
    }
}

