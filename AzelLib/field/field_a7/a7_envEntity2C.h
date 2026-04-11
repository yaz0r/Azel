#pragma once

#include "task.h"
#include "common.h"

struct sA7EnvEntity2C;

// 06059b8c
void a7EnvEntity2C_Init(sA7EnvEntity2C* pThis, sSaturnPtr arg);
// 06059cd4
void a7EnvEntity2C_Update(sA7EnvEntity2C* pThis);

// 0x2C-byte sibling encounter spawner. Owns up to 5 sub-tasks (0xE0 each)
// loaded from a 0x1C-byte table at the per-encounter Saturn arg base.
struct sA7EnvEntity2C : public s_workAreaTemplateWithArg<sA7EnvEntity2C, sSaturnPtr>
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
