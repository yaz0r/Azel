#pragma once

#include "validation_mailbox.hpp"

#include <cstddef>
#include <cstdint>

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

    void readMemory(std::uint32_t address, void *buffer, std::size_t size);
    void writeMemory(std::uint32_t address, const void *buffer, std::size_t size);

    std::uint8_t readU8(std::uint32_t address);
    std::int8_t readS8(std::uint32_t address);
    std::uint16_t readU16(std::uint32_t address);
    std::int16_t readS16(std::uint32_t address);
    std::uint32_t readU32(std::uint32_t address);
    std::int32_t readS32(std::uint32_t address);

    void writeU8(std::uint32_t address, std::uint8_t value);
    void writeU16(std::uint32_t address, std::uint16_t value);
    void writeU32(std::uint32_t address, std::uint32_t value);

    void fillRange(std::uint32_t address, std::uint32_t length, std::uint8_t value, Cpu cpu = Cpu::Master);

    void mirrorWram();

    std::uint32_t getRegister(std::uint32_t index, Cpu cpu = Cpu::Master);
    void setRegister(std::uint32_t index, std::uint32_t value, Cpu cpu = Cpu::Master);
    void getAllRegisters(std::uint32_t out[azelval::REG_COUNT], Cpu cpu = Cpu::Master);

    void setBreakpoint(std::uint32_t address, Cpu cpu = Cpu::Master);
    void removeBreakpoint(std::uint32_t address, Cpu cpu = Cpu::Master);

    azelval::Status executeUntilAddress(std::uint32_t address, Cpu cpu = Cpu::Master);

    std::uint64_t stepInstruction(Cpu cpu = Cpu::Master);
    void resetTarget();

    [[nodiscard]] std::uint32_t stoppedPc() const {
        return m_lastStoppedPc;
    }

private:
    std::uint8_t *mirrorPtr(std::uint32_t address, std::size_t size);

    azelval::Status exec(azelval::Command command, Cpu cpu, std::uint32_t address, std::uint32_t index,
                         std::uint32_t length, std::uint32_t value);

    [[nodiscard]] bool serverAlive() const;

    void *m_mailboxHandle = nullptr;
    void *m_wramLoHandle = nullptr;
    void *m_wramHiHandle = nullptr;
    azelval::Mailbox *m_mailbox = nullptr;
    std::uint8_t *m_wramLo = nullptr;
    std::uint8_t *m_wramHi = nullptr;

    std::uint32_t m_seq = 0;
    std::uint32_t m_lastStoppedPc = 0;
    void *m_serverProcess = nullptr;
};

extern MailboxConnection *g_validationConnection;
