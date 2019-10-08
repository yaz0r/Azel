#include "PDS.h"

#ifdef WITH_VK

#include "renderer_vk.h"

#include "SDL_vulkan.h"
#include "vulkan/vulkan.hpp"

#define IMGUI_API
#include "imgui_impl_sdl.h"
#include "imgui_impl_vulkan.h"

SDL_Window* gWindowVulkan = nullptr;
VkInstance gVkInstance = nullptr;

backend* SDL_VK_backend::create()
{
    if (init())
    {
        return new SDL_VK_backend();
    }
    return nullptr;
}

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

bool checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

bool SDL_VK_backend::init()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK) != 0)
    {
        assert(false);
    }

    u32 flags = 0;
    flags |= SDL_WINDOW_VULKAN;
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
    gWindowVulkan = SDL_CreateWindow("PDS: Azel Vulkan", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, resolution[0], resolution[1], flags);
    assert(gWindowVulkan);

    bool enableValidationLayers = checkValidationLayerSupport();

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }

    
    vkCreateInstance(&createInfo, nullptr, &gVkInstance);


    return true;
}

void SDL_VK_backend::bindBackBuffer()
{
    assert(0);
}

void SDL_VK_backend::ImGUI_NewFrame()
{
}

void SDL_VK_backend::ImGUI_RenderDrawData(ImDrawData* pDrawData)
{
    ImGui_ImplVulkan_RenderDrawData(pDrawData, nullptr);
}

#endif
