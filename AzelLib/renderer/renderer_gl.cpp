#include "PDS.h"
#include <bx/platform.h>
#include <backends/imgui_impl_sdl3.h>
#include "imguiBGFX.h"

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
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
    {
        assert(false);
    }

    SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;

#ifdef __IPHONEOS__
    flags |= SDL_WINDOW_FULLSCREEN;
    flags |= SDL_WINDOW_METAL;
#endif

    int resolution[2] = { 1280, 960 };

    gWindowBGFX = SDL_CreateWindow("BGFX", resolution[0], resolution[1], flags);

    SDL_PropertiesID props = SDL_GetWindowProperties(gWindowBGFX);

    bgfx::Init initparam;
#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
    initparam.platformData.ndt = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, NULL);
    initparam.platformData.nwh = (void*)(uintptr_t)SDL_GetNumberProperty(props, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
#elif BX_PLATFORM_OSX
    initparam.platformData.ndt = NULL;
    initparam.platformData.nwh = cbSetupMetalLayer(SDL_GetPointerProperty(props, SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, NULL));
#elif BX_PLATFORM_IOS
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");
    SDL_Renderer* pRenderer = SDL_CreateRenderer(gWindowBGFX, NULL);
    initparam.platformData.ndt = NULL;
    initparam.platformData.nwh = SDL_GetRenderMetalLayer(pRenderer);
    initparam.type = bgfx::RendererType::Metal;
#elif BX_PLATFORM_WINDOWS
    initparam.platformData.ndt = NULL;
    initparam.platformData.nwh = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
#elif BX_PLATFORM_STEAMLINK
    initparam.platformData.ndt = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_VIVANTE_DISPLAY_POINTER, NULL);
    initparam.platformData.nwh = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_VIVANTE_WINDOW_POINTER, NULL);
#endif // BX_PLATFORM_

    //initparam.type = bgfx::RendererType::OpenGL;
    //initparam.type = bgfx::RendererType::Vulkan;
    //initparam.type = bgfx::RendererType::Metal;
    bgfx::init(initparam);

    imguiCreate();

    ImGuiIO& io = ImGui::GetIO();
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    if ((bgfx::getRendererType() == bgfx::RendererType::Direct3D11) || (bgfx::getRendererType() == bgfx::RendererType::Direct3D12)) {
        ImGui_ImplSDL3_InitForD3D(gWindowBGFX);
    }
    else
    if (bgfx::getRendererType() == bgfx::RendererType::Metal) {
        ImGui_ImplSDL3_InitForMetal(gWindowBGFX);
    }
    else
    if (bgfx::getRendererType() == bgfx::RendererType::OpenGL) {
        ImGui_ImplSDL3_InitForOpenGL(gWindowBGFX, SDL_GL_GetCurrentContext());
    }
    else
    if (bgfx::getRendererType() == bgfx::RendererType::Vulkan) {
        ImGui_ImplSDL3_InitForVulkan(gWindowBGFX);
    }
    else {
        ImGui_ImplSDL3_InitForOther(gWindowBGFX);
    }

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

