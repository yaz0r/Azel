#pragma once

#include "MailboxConnection.h" // g_validationConnection, setBreakpoint (armed by enable())

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

    // saturnBreakpoint: the guest PC this hook's detour drives the emulator to (its executeUntilAddress target). When
    // non-zero, enable() arms it in Ymir up front (once) so out-of-order guest execution is caught from the first frame.
    interceptor(functionSignature originalFunction, functionSignature interceptedFunction,
                std::uint32_t saturnBreakpoint = 0)
        : m_originalFunction(originalFunction)
        , m_interceptedFunction(interceptedFunction)
        , m_saturnBreakpoint(saturnBreakpoint) {}

    // Resolves the real entry point, transparently following the 0xE9 relay jump MSVC inserts under
    // Edit-and-Continue / incremental linking.
    static std::uint8_t *unpackFunction(functionSignature fn) {
#ifdef _WIN32
        std::uint8_t *ptr = reinterpret_cast<std::uint8_t *>(fn);
        if (*ptr == 0xE9) {
            std::int32_t offset = *reinterpret_cast<std::int32_t *>(ptr + 1);
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
        std::uint8_t *originalFunctionPtr = unpackFunction(m_originalFunction);
        std::uint8_t *detourFunctionPtr = unpackFunction(m_interceptedFunction);

        DWORD oldFlags;
        VirtualProtect(originalFunctionPtr, m_savedBytes.size(), PAGE_EXECUTE_READWRITE, &oldFlags);

        std::memcpy(m_savedBytes.data(), originalFunctionPtr, m_savedBytes.size());
        // FF 25 00000000 = jmp qword ptr [rip+0]; the 8-byte absolute target follows immediately.
        const std::array<std::uint8_t, 6> jmp = {0xFF, 0x25, 0x00, 0x00, 0x00, 0x00};
        std::memcpy(originalFunctionPtr, jmp.data(), jmp.size());
        std::memcpy(originalFunctionPtr + jmp.size(), &detourFunctionPtr, sizeof(std::uint8_t *));

        VirtualProtect(originalFunctionPtr, m_savedBytes.size(), oldFlags, &oldFlags);
#else
        assert(0);
#endif
        m_enabled = true;

        // Arm the guest-side breakpoint once, up front. enable() is also called by callUndetoured on every detour
        // invocation, so guard against re-arming (each setBreakpoint is a mailbox round-trip). The breakpoint persists
        // in Ymir until Reset; it is never removed here.
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
        std::uint8_t *originalFunctionPtr = unpackFunction(m_originalFunction);

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
    std::array<std::uint8_t, 16> m_savedBytes{};
    functionSignature m_originalFunction;
    functionSignature m_interceptedFunction;
    std::uint32_t m_saturnBreakpoint = 0; // guest PC to arm in Ymir on enable(); 0 = none
    bool m_breakpointArmed = false;
};
