#pragma once

#include "field.h"
#include "kernel/vdp1AnimatedQuad.h"

struct sFieldSceneManagerEntry
{
    sVec3_FP m0_position;   // 0x00
    sVec3_FP mC_velocity;   // 0x0C
    s32 m18_paramA;         // 0x18
    s32 m1C_paramB;         // 0x1C
    void* m20_vdp1Block;    // 0x20 — also used as payload ptr
    s32(*m24_updateFunc)(sFieldSceneManagerEntry*);
    void(*m28_drawFunc)(sFieldSceneManagerEntry*);
    sAnimatedQuad m2C_quad; // 0x2C — size 8
    // size 0x34
};

struct sFieldSceneManager : public s_workAreaTemplate<sFieldSceneManager>
{
    s_memoryAreaOutput m0_memoryArea;
    sFieldSceneManagerEntry* m8_entries;
    sFieldSceneManagerEntry* mC_currentEntry;
    s32 m10;
    s32 m14_count;
    s32 m18;
    s32 m1C;
    // size 0x20
};

sFieldSceneManager* createFieldSceneManager(p_workArea parent, s32 areaIndex, s32 count);
