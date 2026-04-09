#pragma once

#include "field.h"

struct sFieldSceneManagerEntry
{
    u8 m0_data[0x20];
    void* m20_vdp1Block;
    s32(*m24_updateFunc)(sFieldSceneManagerEntry*);
    void(*m28_drawFunc)(sFieldSceneManagerEntry*);
    u8 m2C_pad[8];
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
