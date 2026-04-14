#pragma once

#include "task.h"
#include "common.h"

// RepairBitFormation — spawns and manages up to 5 RepairBit children
struct sRepairBitFormation;

void repairBitFormation_Init(sRepairBitFormation* pThis, sSaturnPtr arg);
void repairBitFormation_Update(sRepairBitFormation* pThis);

struct sRepairBitFormation : public s_workAreaTemplateWithArg<sRepairBitFormation, sSaturnPtr>
{
    sSaturnPtr  m0_arg;            // 0x00 — table base (Saturn EA)
    s32         m4_count;          // 0x04 — number of children currently alive
    s32         m8_iter;           // 0x08 — round-robin iterator over alive children
    s32         mC_phaseCounter;   // 0x0C — phase counter (bumped each frame)
    p_workArea  m10_children[5];   // 0x10..0x20 — child sub-task pointers
    u8          m24_alive[5];      // 0x24..0x28 — per-slot alive flag
    u8          m29_pad[3];        // 0x29..0x2B
    // Saturn size 0x2C
};

// RepairBit — individual repair bit entity (0xE0 bytes), child of RepairBitFormation
p_workArea repairBit_spawn(p_workArea parent, sSaturnPtr entry, u8* aliveFlag);
