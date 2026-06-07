#pragma once

#include "MailboxConnection.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <cassert>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

template <class ReturnType, class... Args>
class interceptor {
public:
    using functionSignature = ReturnType (*)(Args...);

    interceptor(functionSignature originalFunction, functionSignature interceptedFunction,
                u32 saturnBreakpoint = 0)
        : m_originalFunction(originalFunction)
        , m_interceptedFunction(interceptedFunction)
        , m_saturnBreakpoint(saturnBreakpoint) {}

    // Follows the 0xE9 relay jump MSVC inserts under incremental linking
    static u8 *unpackFunction(functionSignature fn) {
#ifdef _WIN32
        u8 *ptr = reinterpret_cast<u8 *>(fn);
        if (*ptr == 0xE9) {
            s32 offset = *reinterpret_cast<s32 *>(ptr + 1);
            ptr += offset + 5;
        }
        return ptr;
#else
        (void)fn;
        assert(0);
        return nullptr;
#endif
    }

    void enable() {
        if (m_enabled) {
            return;
        }
#ifdef _WIN32
        u8 *originalFunctionPtr = unpackFunction(m_originalFunction);
        u8 *detourFunctionPtr = unpackFunction(m_interceptedFunction);

        DWORD oldFlags;
        VirtualProtect(originalFunctionPtr, m_savedBytes.size(), PAGE_EXECUTE_READWRITE, &oldFlags);

        std::memcpy(m_savedBytes.data(), originalFunctionPtr, m_savedBytes.size());
        // FF 25 00000000 = jmp qword ptr [rip+0]; the 8-byte absolute target follows immediately.
        const std::array<u8, 6> jmp = {0xFF, 0x25, 0x00, 0x00, 0x00, 0x00};
        std::memcpy(originalFunctionPtr, jmp.data(), jmp.size());
        std::memcpy(originalFunctionPtr + jmp.size(), &detourFunctionPtr, sizeof(u8 *));

        VirtualProtect(originalFunctionPtr, m_savedBytes.size(), oldFlags, &oldFlags);
#else
        assert(0);
#endif
        m_enabled = true;

        // Arm once: callUndetoured re-enters enable() on every detour invocation
        if (m_saturnBreakpoint != 0 && !m_breakpointArmed && g_validationConnection != nullptr) {
            g_validationConnection->setBreakpoint(m_saturnBreakpoint);
            m_breakpointArmed = true;
        }
    }

    void disable() {
        if (!m_enabled) {
            return;
        }
#ifdef _WIN32
        u8 *originalFunctionPtr = unpackFunction(m_originalFunction);

        DWORD oldFlags;
        VirtualProtect(originalFunctionPtr, m_savedBytes.size(), PAGE_EXECUTE_READWRITE, &oldFlags);
        std::memcpy(originalFunctionPtr, m_savedBytes.data(), m_savedBytes.size());
        VirtualProtect(originalFunctionPtr, m_savedBytes.size(), oldFlags, &oldFlags);
#else
        assert(0);
#endif
        m_enabled = false;
    }

    ReturnType callUndetoured(Args... args) {
        assert(m_enabled);
        if constexpr (std::is_void_v<ReturnType>) {
            disable();
            m_originalFunction(std::forward<Args>(args)...);
            enable();
        } else {
            disable();
            ReturnType returnValue = m_originalFunction(std::forward<Args>(args)...);
            enable();
            return returnValue;
        }
    }

    bool m_enabled = false;

private:
    std::array<u8, 16> m_savedBytes{};
    functionSignature m_originalFunction;
    functionSignature m_interceptedFunction;
    u32 m_saturnBreakpoint = 0; // guest PC to arm in Ymir on enable(); 0 = none
    bool m_breakpointArmed = false;
};
