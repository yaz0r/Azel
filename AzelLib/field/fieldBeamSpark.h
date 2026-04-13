#pragma once

// Beam spark effect — shared across all field overlays.
// A short-lived VDP1 spark particle with position, velocity, color fade.

struct sBeamSparkArg
{
    sVec3_FP m0_pos;   // 0x00
    u32      mC;       // 0x0C
    u32      m10;      // 0x10
    u32      m14;      // 0x14
    u32      m18;      // 0x18
    u32      m1C;      // 0x1C
    u32      m20;      // 0x20
    u32      m24;      // 0x24
    s32      m28;      // 0x28
    u16      m2C;      // 0x2C
    u16      m2E;      // 0x2E
    // size 0x30
};

struct sBeamSpark : public s_workAreaTemplateWithArg<sBeamSpark, sBeamSparkArg*>
{
    sVec3_FP m0_pos;   // 0x00
    u32      mC;       // 0x0C
    u32      m10;      // 0x10
    u32      m14;      // 0x14
    u32      m18;      // 0x18
    u32      m1C;      // 0x1C
    u32      m20;      // 0x20
    u32      m24;      // 0x24
    s32      m28;      // 0x28
    u16      m2C;      // 0x2C
    u16      m2E;      // 0x2E (masked with 0x7FFF on init)
    // size 0x30
};

void beamSpark_Init(sBeamSpark* pThis, sBeamSparkArg* pArg);
void beamSpark_Update(sBeamSpark* pThis);
void beamSpark_UpdateAlt(sBeamSpark* pThis);
void beamSpark_Draw(sBeamSpark* pThis);
void beamSpark_spawn(p_workArea parent, sBeamSparkArg* pArg);
