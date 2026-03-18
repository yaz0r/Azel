#include "PDS.h"
#include <soloud.h>

#include <backends/imgui_impl_sdl3.h>

#include "renderer/renderer.h"
#include "renderer/renderer_gl.h"
#include "renderer/renderer_vk.h"

//#include "../ThirdParty/bgfx.cmake/bgfx/examples/common/imgui/imgui.h"
#include "imguiBGFX.h"
#include "items.h"
#include "renderer_vdp2.h"
#include "town/townScriptDecompiler.h"
#include "debugWindows.h"

bgfx::ProgramHandle loadBgfxProgram(const std::string& VSFile, const std::string& PSFile);

extern SDL_Window* gWindowBGFX;

#if defined(__EMSCRIPTEN__) || defined(TARGET_OS_IOS) || defined(TARGET_OS_TV)
static float gVolume = 1.f;
#else
#ifdef SHIPPING_BUILD
static float gVolume = 1.f;
#else
static float gVolume = 1.f;
#endif
#endif

bool useVDP1GL = true;
bool useCombinedDualPlane = true;
bool debugFlatPlaneA = false;

SoLoud::Soloud gSoloud; // Engine core

int outputResolution[2] = { 1024, 814 };
int internalResolution[2] = { 352, 224 };

bgfx::FrameBufferHandle gBGFXVdp1PolyFB = BGFX_INVALID_HANDLE;
bgfx::TextureHandle vdp1BufferTexture = BGFX_INVALID_HANDLE;
bgfx::TextureHandle vdp1PriorityTexture = BGFX_INVALID_HANDLE;
bgfx::TextureHandle vdp1DepthBufferTexture = BGFX_INVALID_HANDLE;
bgfx::UniformHandle u_spritePriority = BGFX_INVALID_HANDLE;

bgfx::FrameBufferHandle gBgfxCompositedFB = BGFX_INVALID_HANDLE;

bgfx::TextureHandle bgfx_vdp1_ram_texture = BGFX_INVALID_HANDLE;

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

void azelSdl_Init()
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
    bgfx_vdp2_rbg0_program = loadBgfxProgram("VDP2_vs", "VDP2_RBG0_ps");
    bgfx_vdp2_rbg0_dual_program = loadBgfxProgram("VDP2_vs", "VDP2_RBG0_dual_ps");
#ifndef SHIPPING_BUILD
    SDL_GL_SetSwapInterval(0);
#endif
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
    return BGFX_INVALID_HANDLE;
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
    return false;
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
    return false;
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
void azelSdl_StartFrame()
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
        vdp1PriorityTexture = bgfx::createTexture2D(internalResolution[0], internalResolution[1], false, 0, bgfx::TextureFormat::R8, BGFX_TEXTURE_RT | tsFlags);
        vdp1DepthBufferTexture = bgfx::createTexture2D(internalResolution[0], internalResolution[1], false, 0, bgfx::TextureFormat::D24S8, BGFX_TEXTURE_RT | tsFlags);
        std::array<bgfx::Attachment, 3> vdp1BufferAt;
        vdp1BufferAt[0].init(vdp1BufferTexture);
        vdp1BufferAt[1].init(vdp1PriorityTexture);
        vdp1BufferAt[2].init(vdp1DepthBufferTexture);
        gBGFXVdp1PolyFB = bgfx::createFrameBuffer(3, &vdp1BufferAt[0], true);

        u_spritePriority = bgfx::createUniform("u_spritePriority", bgfx::UniformType::Vec4);
    }

#ifndef USE_NULL_RENDERER
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (!isShipping())
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
        }
        
        switch (event.type)
        {
        case SDL_EVENT_MOUSE_MOTION:
            // update mouse position
            gUIState.mousex = (int)event.motion.x;
            gUIState.mousey = (int)event.motion.y;
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
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
        case SDL_EVENT_MOUSE_BUTTON_UP:
            // update button down state if left-clicking
            if (event.button.button == 1)
            {
                gUIState.mousedown = 0;
            }
            break;
        case SDL_EVENT_MOUSE_WHEEL:
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

        if (event.type == SDL_EVENT_QUIT)
            closeApp = true;
    }
#endif

    gSoloud.setGlobalVolume(gVolume);

    graphicEngineStatus.m4514.m0_inputDevices[0].m16_pending.m6_buttonDown = 0;
    graphicEngineStatus.m4514.m0_inputDevices[0].m16_pending.m8_newButtonDown = 0;
    graphicEngineStatus.m4514.m0_inputDevices[0].m16_pending.mC_newButtonDown2 = 0;

#ifndef USE_NULL_RENDERER

    static SDL_Gamepad* controller = nullptr;
    if(controller == nullptr)
    {
        int numGamepads = 0;
        SDL_JoystickID* gamepads = SDL_GetGamepads(&numGamepads);
        if (gamepads)
        {
            for (int i = 0; i < numGamepads; ++i) {
                controller = SDL_OpenGamepad(gamepads[i]);
                if (controller) {
                    break;
                }
                else {
                    fprintf(stderr, "Could not open gamepad %i: %s\n", i, SDL_GetError());
                }
            }
            SDL_free(gamepads);
        }
    }

    if (controller)
    {
        graphicEngineStatus.m4514.m0_inputDevices[0].m16_pending.m0_inputType = 2;

        u16 buttonMask = 0;
        buttonMask |= SDL_GetGamepadButton(controller, SDL_GAMEPAD_BUTTON_SOUTH) << 0;
        buttonMask |= SDL_GetGamepadButton(controller, SDL_GAMEPAD_BUTTON_EAST) << 1;
        buttonMask |= SDL_GetGamepadButton(controller, SDL_GAMEPAD_BUTTON_NORTH) << 2;
        buttonMask |= SDL_GetGamepadButton(controller, SDL_GAMEPAD_BUTTON_START) << 3;
        buttonMask |= SDL_GetGamepadButton(controller, SDL_GAMEPAD_BUTTON_DPAD_UP) << 4;
        buttonMask |= SDL_GetGamepadButton(controller, SDL_GAMEPAD_BUTTON_DPAD_DOWN) << 5;
        buttonMask |= SDL_GetGamepadButton(controller, SDL_GAMEPAD_BUTTON_DPAD_LEFT) << 6;
        buttonMask |= SDL_GetGamepadButton(controller, SDL_GAMEPAD_BUTTON_DPAD_RIGHT) << 7;

        graphicEngineStatus.m4514.m0_inputDevices[0].m16_pending.m6_buttonDown |= buttonMask;

        if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & buttonMask) == 0)
        {
            graphicEngineStatus.m4514.m0_inputDevices[0].m16_pending.m8_newButtonDown |= buttonMask;
            graphicEngineStatus.m4514.m0_inputDevices[0].m16_pending.mC_newButtonDown2 |= buttonMask;
        }

        // analog
        // need to remap range [-32768 to 32767] to [0 - 255]
        graphicEngineStatus.m4514.m0_inputDevices[0].m16_pending.m2_analogX = convertAxis(SDL_GetGamepadAxis(controller, SDL_GAMEPAD_AXIS_LEFTX));
        graphicEngineStatus.m4514.m0_inputDevices[0].m16_pending.m3_analogY = convertAxis(SDL_GetGamepadAxis(controller, SDL_GAMEPAD_AXIS_LEFTY));
    }
    //else
    {
        graphicEngineStatus.m4514.m0_inputDevices[0].m16_pending.m0_inputType = 1;

        const bool* keyState = SDL_GetKeyboardState(NULL);

        for (int i = 0; i < SDL_SCANCODE_COUNT; i++)
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
        ImGui_ImplSDL3_NewFrame();
        //imguiBeginFrame(0, 0, 0, 0, outputResolution[0], outputResolution[1], -1);
        imguiBeginFrame(gUIState.mousex, gUIState.mousey, gUIState.mousedown, 0, outputResolution[0], outputResolution[1]);
    }
#endif
}

ImVec4 clear_color = ImColor(114, 144, 154);

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

int computeSpritePriority(u16 cmdcolr)
{
    u16 spctl = vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL;
    u16 spriteType = spctl & 0xF;

    int prRegister = 0;
    switch (spriteType)
    {
    case 0: case 1:
        // 1 PR bit at bit 15
        prRegister = (cmdcolr >> 15) & 1;
        break;
    case 2:
        // 1 PR bit at bit 14
        prRegister = (cmdcolr >> 14) & 1;
        break;
    default:
        // Types 3+ use 2 PR bits at bits 14-15
        prRegister = (cmdcolr >> 14) & 3;
        break;
    }

    switch (prRegister)
    {
    case 0: return vdp2Controls.m4_pendingVdp2Regs->mF0_PRISA & 7;
    case 1: return (vdp2Controls.m4_pendingVdp2Regs->mF0_PRISA >> 8) & 7;
    case 2: return vdp2Controls.m4_pendingVdp2Regs->mF2_PRISB & 7;
    case 3: return (vdp2Controls.m4_pendingVdp2Regs->mF2_PRISB >> 8) & 7;
    default: return vdp2Controls.m4_pendingVdp2Regs->mF0_PRISA & 7;
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

void renderVdp1WithPriority(bgfx::ViewId outputView, int priorityLevel)
{
    if (!bgfx::isValid(vdp1BufferTexture) || !bgfx::isValid(vdp1PriorityTexture))
    {
        return;
    }

    static bgfx::ProgramHandle program = BGFX_INVALID_HANDLE;
    static bgfx::VertexBufferHandle quad_vertexbuffer = BGFX_INVALID_HANDLE;
    static bgfx::IndexBufferHandle quad_indexbuffer = BGFX_INVALID_HANDLE;
    static bgfx::UniformHandle inputTexture = BGFX_INVALID_HANDLE;
    static bgfx::UniformHandle prioritySampler = BGFX_INVALID_HANDLE;
    static bgfx::UniformHandle u_priorityLevel = BGFX_INVALID_HANDLE;
    static bgfx::VertexLayout ms_layout;

    static bool initialized = false;
    if (!initialized)
    {
        program = loadBgfxProgram("VDP2_blit_vs", "VDP2_blit_vdp1_ps");

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
        prioritySampler = bgfx::createUniform("s_priority", bgfx::UniformType::Sampler);
        u_priorityLevel = bgfx::createUniform("u_priorityLevel", bgfx::UniformType::Vec4);

        initialized = true;
    }

    bgfx::setState(0
        | BGFX_STATE_WRITE_RGB
        | BGFX_STATE_WRITE_A
        | BGFX_STATE_DEPTH_TEST_ALWAYS
    );

    bgfx::setViewRect(outputView, 0, 0, internalResolution[0], internalResolution[1]);

    float priorityVec[4] = { (float)priorityLevel, 0, 0, 0 };
    bgfx::setUniform(u_priorityLevel, priorityVec);

    bgfx::setTexture(0, inputTexture, vdp1BufferTexture);
    bgfx::setTexture(1, prioritySampler, vdp1PriorityTexture);

    bgfx::setVertexBuffer(0, quad_vertexbuffer);
    bgfx::setIndexBuffer(quad_indexbuffer);
    bgfx::submit(outputView, program);
}

bool azelSdl_EndFrame()
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
        if (gDebugWindows.vdp)
        {
            if (ImGui::Begin("VDP", &gDebugWindows.vdp))
            {
                ImGui::PushID("BG0");
                ImGui::Text("NBG0"); ImGui::SameLine(); ImGui::Checkbox("GPU", &BG0_GPU);
                ImGui::Image(ImGui::toId(NBG_data[0].BGFXTexture, 0, 0), ImVec2(vdp2ResolutionWidth, vdp2ResolutionHeight), ImVec2(0, 1), ImVec2(1, 0));
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
                ImGui::SameLine(); ImGui::Checkbox("Combined dual", &useCombinedDualPlane);
                ImGui::SameLine(); ImGui::Checkbox("Flat A", &debugFlatPlaneA);
                ImGui::Image(ImGui::toId(NBG_data[4].BGFXTexture, 0, 0), ImVec2(vdp2ResolutionWidth, vdp2ResolutionHeight), ImVec2(0, 1), ImVec2(1, 0));
                ImGui::PopID();

                ImGui::Text("VDP1 poly");
                ImGui::Image(ImGui::toId(bgfx::getTexture(gBGFXVdp1PolyFB), 0, 0), ImVec2(vdp2ResolutionWidth, vdp2ResolutionHeight), ImVec2(0, 1), ImVec2(1, 0));
            }
            ImGui::End();
        }

        if (gDebugWindows.tasks)
        {
            PrintDebugTasksHierarchy();
            PrintDebugTasksInfo();
        }

        if (gDebugWindows.config)
        {
            if (ImGui::Begin("Config", &gDebugWindows.config))
            {
                ImGui::InputInt2("Internal Resolution", internalResolution);
            }
            ImGui::End();
        }
    }

    // render VDP1 frame buffer
    if(1)
    {
        bgfx::setViewName(vdp1_gpuView, "Vdp1");
        bgfx::setViewFrameBuffer(vdp1_gpuView, gBGFXVdp1PolyFB);
        bgfx::setViewRect(vdp1_gpuView, 0, 0, internalResolution[0], internalResolution[1]);

        bgfx::setViewClear(vdp1_gpuView, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0);

        float scaleX = internalResolution[0] / 352.f;
        float scaleY = internalResolution[1] / 224.f;
        bgfx::setViewScissor(vdp1_gpuView, cameraProperties.x0* scaleX, cameraProperties.y0* scaleY, cameraProperties.x1* scaleX - cameraProperties.x0 * scaleX, cameraProperties.y1* scaleY - cameraProperties.y0 * scaleY);

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
            // VDP1 sprites first at this priority
            renderVdp1WithPriority(CompositeView, priorityIndex);
            // VDP2 scroll layers on top at this priority
            for (eLayers layerIndex = NBG0; layerIndex < eLayers::MAX; layerIndex = (eLayers)(layerIndex + 1))
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
        if (gDebugWindows.finalComposition)
        {
            if (ImGui::Begin("Final Composition", &gDebugWindows.finalComposition))
            {
                ImVec2 textureSize = ImGui::GetWindowSize();
                textureSize.y = textureSize.x * (224.f / 352.f);
                //ImGui::Image((ImTextureID)gCompositedTexture, textureSize, ImVec2(0, 1), ImVec2(1, 0)); ImGui::SameLine();
            }
            ImGui::End();
        }

        if (ImGui::BeginMainMenuBar())
        {
            ImGui::Text(" %.2f FPS (%.2f ms)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);

            if (ImGui::BeginMenu("Framerate"))
            {
                if (ImGui::MenuItem("Unlimited", NULL, frameLimit == -1)) frameLimit = -1;
                if (ImGui::MenuItem("30", NULL, frameLimit == 30)) frameLimit = 30;
                if (ImGui::MenuItem("5", NULL, frameLimit == 5)) frameLimit = 5;
                ImGui::EndMenu();
            }

            ImGui::PushItemWidth(100);
            ImGui::SliderFloat("Volume", &gVolume, 0, 1);
            ImGui::PopItemWidth();

            drawDebugMenu();

            extern bool bTraceEnabled;
            ImGui::Checkbox("Trace", &bTraceEnabled);

            ImGui::EndMainMenuBar();
        }

        drawScriptDecompilerWindow();

        if (gDebugWindows.inventory)
        {
            ImGui::Begin("Inventory", &gDebugWindows.inventory);

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
        if (gDebugWindows.defaultLog)
            PDS_Logger[eLogCategories::log_default].Draw("Default log", &gDebugWindows.defaultLog);
        if (gDebugWindows.taskLog)
            PDS_Logger[eLogCategories::log_task].Draw("Task log", &gDebugWindows.taskLog);
        if (gDebugWindows.unimplementedLog)
            PDS_Logger[eLogCategories::log_unimlemented].Draw("Unimplemented log", &gDebugWindows.unimplementedLog);
        if (gDebugWindows.soundM68k)
            PDS_Logger[eLogCategories::log_m68k].Draw("Sound m68k", &gDebugWindows.soundM68k);
        if (gDebugWindows.warningLog)
            PDS_Logger[eLogCategories::log_warning].Draw("Warning log", &gDebugWindows.warningLog);
#endif
    }
    

    static bool bImguiEnabled = false;

#ifndef USE_NULL_RENDERER
#ifndef SHIPPING_BUILD
    if (ImGui::IsKeyPressed(ImGuiKey_GraveAccent))
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
        if (!isShipping())
        {
            ImGui::Render();
        }
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

