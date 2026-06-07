#include "PDS.h"

#include "mainMenuDebugTasks.h"

#include "validation/validation.h"
#include "validation/validationHooks_town.h"

#include "movie/movie.h"

#include "titleScreen.h"

#include <cstdio>

namespace {
constexpr std::uint32_t kUpdateInputsReturn = 0x0602392a;
constexpr std::uint32_t kUpdateInputsAddr = 0x060238ba;
} // namespace

// 06012e48
DECLARE_HOOK_VOID(updateInputs, kUpdateInputsReturn, void)
void updateInputs_detour() {

    g_validationConnection->executeUntilAddress(kUpdateInputsAddr);

    updateInputs_intercept.callUndetoured();

    g_validationConnection->executeUntilAddress(kUpdateInputsReturn);

    constexpr std::uint32_t kInputDevice0 = 0x60501B8;
    graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m0_inputType = g_validationConnection->readU8(kInputDevice0 + 0);
    graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown = g_validationConnection->readU16(kInputDevice0 + 6);
    graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown = g_validationConnection->readU16(kInputDevice0 + 8);
    graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.mA = g_validationConnection->readU16(kInputDevice0 + 0xA);
    graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.mC_newButtonDown2 = g_validationConnection->readU16(kInputDevice0 + 0xC);

    DEBUG_setRandomSeed(g_validationConnection->readU32(0x604b02c));
}

DECLARE_HOOK_VOID(runTasks, 0x0602fa00, void)
void runTasks_detour() {
    g_validationConnection->executeUntilAddress(0x0602fa00);
    runTasks_intercept.callUndetoured();
    g_validationConnection->executeUntilAddress(0x6006060);
}

void s_movieMainWorkArea_Init_detour(s_movieMainWorkArea* pThis, s32 movieIndex);
interceptor<void, s_movieMainWorkArea*, s32> s_movieMainWorkArea_Init_intercept(
    &s_movieMainWorkArea::Init, s_movieMainWorkArea_Init_detour, 0x06054010);
void s_movieMainWorkArea_Init_detour(s_movieMainWorkArea* pThis, s32 movieIndex) {
    g_validationConnection->executeUntilAddress(0x06054010);
    s_movieMainWorkArea_Init_intercept.callUndetoured(pThis, movieIndex);
}

void s_movieMainWorkArea_Draw_detour(s_movieMainWorkArea* pThis);
interceptor<void, s_movieMainWorkArea*> s_movieMainWorkArea_Draw_intercept(
    &s_movieMainWorkArea::Draw, s_movieMainWorkArea_Draw_detour, 0x060540fa);
void s_movieMainWorkArea_Draw_detour(s_movieMainWorkArea* pThis) {
    g_validationConnection->executeUntilAddress(0x060540fa);
    s_movieMainWorkArea_Draw_intercept.callUndetoured(pThis);
}

void s_titleScreenWorkArea_Draw_detour(s_titleScreenWorkArea* pWorkArea);
interceptor<void, s_titleScreenWorkArea*> s_titleScreenWorkArea_Draw_intercept(
    &s_titleScreenWorkArea::Draw, s_titleScreenWorkArea_Draw_detour, 0x06029bf0);

void s_titleScreenWorkArea_Draw_detour(s_titleScreenWorkArea* pWorkArea) {
    g_validationConnection->executeUntilAddress(0x06029bf0);
    s_titleScreenWorkArea_Draw_intercept.callUndetoured(pWorkArea);
}

void enableValidationHooks() {
    updateInputs_intercept.enable();
    //runTasks_intercept.enable();
    //s_movieMainWorkArea_Init_intercept.enable();
    //s_movieMainWorkArea_Draw_intercept.enable();
    //s_titleScreenWorkArea_Draw_intercept.enable();
    enableTownHooks();
}
