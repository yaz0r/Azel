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

void validate(std::uint32_t base, const std::uint8_t &value);
void validate(std::uint32_t base, const std::int8_t &value);
void validate(std::uint32_t base, const std::uint16_t &value);
void validate(std::uint32_t base, const std::int16_t &value);
void validate(std::uint32_t base, const std::uint32_t &value);
void validate(std::uint32_t base, const std::int32_t &value);

void validate(std::uint32_t base, const fixedPoint &value);
void validate(std::uint32_t base, const sVec3_FP &value);
void validate(std::uint32_t base, const sMatrix4x3 &value);

void validateRegister(std::uint32_t regIndex, std::uint32_t value);

#define DECLARE_HOOK(name, saturnBreakpoint, returnType, ...)                                                          \
    returnType name(__VA_ARGS__);                                                                                      \
    returnType name##_detour(__VA_ARGS__);                                                                             \
    interceptor<returnType, __VA_ARGS__> name##_intercept(name, name##_detour, saturnBreakpoint);

#define DECLARE_HOOK_VOID(name, saturnBreakpoint, returnType)                                                          \
    returnType name();                                                                                                 \
    returnType name##_detour();                                                                                        \
    interceptor<returnType> name##_intercept(name, name##_detour, saturnBreakpoint);
