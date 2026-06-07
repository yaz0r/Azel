#pragma once

#include "validation_mailbox.hpp"

#include <cstddef>
#include <cstdint>

// This header is used from translation units that do not include PDS.h
using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using s8 = std::int8_t;
using s16 = std::int16_t;
using s32 = std::int32_t;

class MailboxConnection {
public:
    using Cpu = azelval::Cpu;

    MailboxConnection() = default;
    ~MailboxConnection();

    bool open();
    void close();
    [[nodiscard]] bool isValid() const {
        return m_mailbox != nullptr;
    }

    void readMemory(u32 address, void *buffer, std::size_t size);
    void writeMemory(u32 address, const void *buffer, std::size_t size);

    u8 readU8(u32 address);
    s8 readS8(u32 address);
    u16 readU16(u32 address);
    s16 readS16(u32 address);
    u32 readU32(u32 address);
    s32 readS32(u32 address);

    void writeU8(u32 address, u8 value);
    void writeU16(u32 address, u16 value);
    void writeU32(u32 address, u32 value);

    void fillRange(u32 address, u32 length, u8 value, Cpu cpu = Cpu::Master);

    void mirrorWram();

    u32 getRegister(u32 index, Cpu cpu = Cpu::Master);
    void setRegister(u32 index, u32 value, Cpu cpu = Cpu::Master);
    void getAllRegisters(u32 out[azelval::REG_COUNT], Cpu cpu = Cpu::Master);

    void setBreakpoint(u32 address, Cpu cpu = Cpu::Master);
    void removeBreakpoint(u32 address, Cpu cpu = Cpu::Master);

    azelval::Status executeUntilAddress(u32 address, Cpu cpu = Cpu::Master);

    u64 stepInstruction(Cpu cpu = Cpu::Master);
    void resetTarget();

    [[nodiscard]] u32 stoppedPc() const {
        return m_lastStoppedPc;
    }

private:
    u8 *mirrorPtr(u32 address, std::size_t size);

    azelval::Status exec(azelval::Command command, Cpu cpu, u32 address, u32 index,
                         u32 length, u32 value);

    [[nodiscard]] bool serverAlive() const;

    void *m_mailboxHandle = nullptr;
    void *m_wramLoHandle = nullptr;
    void *m_wramHiHandle = nullptr;
    azelval::Mailbox *m_mailbox = nullptr;
    u8 *m_wramLo = nullptr;
    u8 *m_wramHi = nullptr;

    u32 m_seq = 0;
    u32 m_lastStoppedPc = 0;
    void *m_serverProcess = nullptr;
};

extern MailboxConnection *g_validationConnection;
