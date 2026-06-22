#pragma once

#include "MailboxConnection.h"
#include "interception.h"

#include <cstdint>

class fixedPoint;
struct sVec3_FP;
struct sMatrix4x3;

extern bool enableValidation;

void validationInit();
void validationShutdown();

void enableValidationHooks();

// Removed during a town load, so the guest can run the original's async multi-frame load
void setFrameSyncBreakpointsEnabled(bool enabled);

void validateAssert(bool result);

enum validationContext {
    VCTX_Base = 0,
    VCTX_Town,
    VCTX_Math,
    VCTX_Count,
};

bool isValidationContextEnabled(validationContext context);
void enableValidationContext(validationContext context);
void disableValidationContext(validationContext context);

void validate(u32 base, const u8 &value);
void validate(u32 base, const s8 &value);
void validate(u32 base, const u16 &value);
void validate(u32 base, const s16 &value);
void validate(u32 base, const u32 &value);
void validate(u32 base, const s32 &value);

void validate(u32 base, const fixedPoint &value);
void validate(u32 base, const sVec3_FP &value);
void validate(u32 base, const sMatrix4x3 &value);

void validateRegister(u32 regIndex, u32 value);

#define DECLARE_HOOK(name, saturnBreakpoint, returnType, ...)                                                          \
    returnType name(__VA_ARGS__);                                                                                      \
    returnType name##_detour(__VA_ARGS__);                                                                             \
    interceptor<returnType, __VA_ARGS__> name##_intercept(name, name##_detour, saturnBreakpoint);

#define DECLARE_HOOK_VOID(name, saturnBreakpoint, returnType)                                                          \
    returnType name();                                                                                                 \
    returnType name##_detour();                                                                                        \
    interceptor<returnType> name##_intercept(name, name##_detour, saturnBreakpoint);
