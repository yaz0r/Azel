#include "PDS.h"

#include "mainMenuDebugTasks.h"
#include "inputRecorder.h"

#include "validation/validation.h"
#include "validation/validationHooks_town.h"
#include "validation/validationHooks_math.h"

#include "movie/movie.h"

#include "titleScreen.h"

#include <cstdio>

constexpr u32 kUpdateInputsReturn = 0x0602392a;
constexpr u32 kUpdateInputsAddr = 0x060238ba;
constexpr u32 kInputDevice0 = 0x60501B8; // emu m0_inputDevices[0].m0_current

// Saturn offsets: analog X/Y are at +2/+3, unlike the packed C++ struct
static void pushInputToEmu() {
    if (g_validationConnection == nullptr)
        return;
    const auto& cur = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current;
    g_validationConnection->writeU8(kInputDevice0 + 0x0, cur.m0_inputType);
    g_validationConnection->writeU8(kInputDevice0 + 0x2, (u8)cur.m2_analogX);
    g_validationConnection->writeU8(kInputDevice0 + 0x3, (u8)cur.m3_analogY);
    g_validationConnection->writeU8(kInputDevice0 + 0x4, (u8)cur.m4);
    g_validationConnection->writeU8(kInputDevice0 + 0x5, (u8)cur.m5);
    g_validationConnection->writeU16(kInputDevice0 + 0x6, cur.m6_buttonDown);
    g_validationConnection->writeU16(kInputDevice0 + 0x8, cur.m8_newButtonDown);
    g_validationConnection->writeU16(kInputDevice0 + 0xA, cur.mA);
    g_validationConnection->writeU16(kInputDevice0 + 0xC, cur.mC_newButtonDown2);
    g_validationConnection->writeU16(kInputDevice0 + 0xE, cur.mE);
    g_validationConnection->writeU16(kInputDevice0 + 0x10, cur.m10);
    g_validationConnection->writeU16(kInputDevice0 + 0x12, cur.m12);
    g_validationConnection->writeU16(kInputDevice0 + 0x14, cur.m14);
}

// Fire once per main-loop frame; removed during a town load
void setFrameSyncBreakpointsEnabled(bool enabled) {
    if (g_validationConnection == nullptr) {
        return;
    }
    if (enabled) {
        g_validationConnection->setBreakpoint(kUpdateInputsAddr);
        g_validationConnection->setBreakpoint(kUpdateInputsReturn);
    } else {
        g_validationConnection->removeBreakpoint(kUpdateInputsAddr);
        g_validationConnection->removeBreakpoint(kUpdateInputsReturn);
    }
}

// 06012e48
DECLARE_HOOK_VOID(updateInputs, kUpdateInputsReturn, void)
void updateInputs_detour() {

    g_validationConnection->executeUntilAddress(kUpdateInputsAddr);

    updateInputs_intercept.callUndetoured();

    g_validationConnection->executeUntilAddress(kUpdateInputsReturn);

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
    gOnInputFinalized = &pushInputToEmu;
    enableTownHooks();
    enableMathHooks();

    //runTasks_intercept.enable();
    //s_movieMainWorkArea_Init_intercept.enable();
    //s_movieMainWorkArea_Draw_intercept.enable();
    //s_titleScreenWorkArea_Draw_intercept.enable();
}
