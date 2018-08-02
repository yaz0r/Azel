#include "PDS.h"

s_graphicEngineStatus graphicEngineStatus;

const u8 bitMasks[] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
const u8 reverseBitMasks[] = { 0x7F, 0xBF, 0xDF, 0xEF, 0xF7, 0xF8, 0xFD, 0xFE };

s_vdp1AllocatorNode* vdp1AllocatorHead = NULL;

u8 playerDataMemoryBuffer[0x28000];

s_mainGameState mainGameState;

const s_dragonPerLevelMaxHPBP dragonPerLevelMaxHPBP[DR_LEVEL_MAX] = {
    { 400, 100 },//DR_0_BASIC_WING = 0,
    { 400, 100 },//DR_1_VALIANT_WING,
    { 400, 100 },//DR_2_STRIPE_WING,
    { 400, 100 },//DR_3_PANZER_WING,
    { 400, 100 },//DR_4_EYE_WING,
    { 400, 100 },//DR_5_ARM_WING,
    { 400, 100 },//DR_6_LIGHT_WING,
    { 400, 100 },//DR_7_SOLO_WING,
    { 1200, 0 }  //DR_8_FLOATER,
};

const u32 longBitMask[] = {
    0x0,
    0x1,
    0x3,
    0x7,
    0xF,
    0x1F,
    0x3F,
    0x7F,
    0xFF,
    0x1FF,
    0x3FF,
    0x7FF,
    0xFFF,
    0x1FFF,
    0x3FFF,
    0x7FFF,
    0xFFFF,
    0x1FFFF,
    0x3FFFF,
    0x7FFFF,
    0xFFFFF,
    0x1FFFFF,
    0x3FFFFF,
    0x7FFFFF,
    0xFFFFFF,
    0x1FFFFFF,
    0x3FFFFFF,
    0x7FFFFFF,
    0xFFFFFFF,
    0x1FFFFFFF,
    0x3FFFFFFF,
    0x7FFFFFFF,
    0xFFFFFFFF,
};

const u32 fieldEnabledTable[23] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1
};

const char* m_A2Fields[] = {
    "  A2_0",
};

const char* m_A3Fields[] = {
    "  A3_0",
    "  A3_1",
    "  A3_2",
    "  A3_3",
    "  A3_4",
    "  A3_5",
    "  A3_6",
    "  A3_7",
    "  A3_8",
    "  A3_9",
    "  A3_A",
    "  A3_B",
    "  A3_C",
};

const char* m_A5Fields[] = {
    "  A5_0",
    "  A5_1",
    "  A5_2",
    "  A5_3",
    "  A5_4",
    "  A5_5",
    "  A5_6",
    "  A5_7",
    "  A5_8",
    "  A5_9",
    "  A5_A",
    "  A5_B",
    "  A5_C",
};
const char* m_A7Fields[] = {
    "  A7_0",
    "  A7_1",
    "  A7_2",
};

const char* m_B1Fields[] = {
    "  B1_0",
    "  B1_1",
};

const char* m_B3Fields[] = {
    "  B3_0",
};

const char* m_B2Fields[] = {
    "  B2_1",
    "  B2_3",
    "  B2_4",
    "  B2_5",
};

const char* m_B5Fields[] = {
    "  B5_0",
    "  B5_1",
    "  B5_2",
    "  B5_3",
    "  B5_4",
    "  B5_5",
    "  B5_6",
};

const char* m_B6Fields[] = {
    "  B6_0",
    "  B6_1",
    "  B6_2",
    "  B6_3",
    "  B6_4",
    "  B6_5",
    "  B6_6",
    "  B6_7",
    "  B6_8",
    "  B6_9",
};

const char* m_C2Fields[] = {
    "  C2_0",
    "  C2_1",
    "  C2_2",
};

const char* m_C4Fields[] = {
    "  C4_0",
    "  C4_1",
    "  C4_2",
    "  C4_3",
    "  C4_4",
    "  C4_5",
    "  C4_6",
    "  C4_7",
    "  C4_8",
};

const char* m_TowerFields[] = {
    "  T0_0",
    "  T0_1",
    "  T0_2",
    "  T0_3",
    "  T0_4",
    "  T0_5",

    "  T1_0",
    "  T1_1",
    "  T1_2",

    "  T2_0",
    "  T2_1",
    "  T2_2",
    "  T2_3",
    "  T2_4",
    "  T2_5",
    "  T2_6",
    "  T2_7",
    "  T2_8",
    "  T2_9",
    "  T2_a",
    "  T2_b",
    "  T2_c",
    "  T2_d",

    "  T3_0",
    "  T3_1",
    "  T3_2",

    "  T7_0",
    "  T7_1",
    "  T7_2",
    "  T7_3",
    "  T7_4",
    "  T7_5",
    "  T7_6",
    "  T7_7",
};

const char* m_D2Fields[] = {
    "  D2_0",
    "  D2_1",
};

const char* m_D3Fields[] = {
    "  D3_0",
};

const char* m_D5Fields[] = {
    "  D5_0",
};

const char* m_DummyFields[] = {
    "  DUMM",
};

const char* m_NameFields[] = {
    "  NAME",
};

const char* m_DemoFields[] = {
    "  DEMO",
};

const char* m_GameFields[] = {
    "  OVER",
};

const char* m_BTFields[] = {
    "  BT  ",
};

const s_fieldDefinition fieldDefinitions[] =
{
    { "  A0  ", "FLD_D5.PRG", "FLD_D5.FNT", sizeof(m_NameFields) / sizeof(m_NameFields[0]), m_NameFields },
    { "  A2  ", "FLD_A3.PRG", "FLD_A3.FNT", sizeof(m_A2Fields) / sizeof(m_A2Fields[0]), m_A2Fields },
    { "  A3  ", "FLD_A3.PRG", "FLD_A3.FNT", sizeof(m_A3Fields) / sizeof(m_A3Fields[0]), m_A3Fields },
    { "  A5  ", "FLD_A5.PRG", "FLD_A5.FNT", sizeof(m_A5Fields) / sizeof(m_A5Fields[0]), m_A5Fields },
    { "  A7  ", "FLD_A7.PRG", "FLD_A7.FNT", sizeof(m_A7Fields) / sizeof(m_A7Fields[0]), m_A7Fields },

    { "  B1  ", "FLD_B1.PRG", "FLD_B1.FNT", sizeof(m_B1Fields) / sizeof(m_B1Fields[0]), m_B1Fields },
    { "  B3  ", "FLD_B2.PRG", "FLD_B2.FNT", sizeof(m_B3Fields) / sizeof(m_B3Fields[0]), m_B3Fields },
    { "  B2  ", "FLD_B2.PRG", "FLD_B2.FNT", sizeof(m_B2Fields) / sizeof(m_B2Fields[0]), m_B2Fields },
    { "  B4  ", "FLD_B5.PRG", "FLD_B5.FNT", sizeof(m_DemoFields) / sizeof(m_DemoFields[0]), m_DemoFields },
    { "  B5  ", "FLD_B5.PRG", "FLD_B5.FNT", sizeof(m_B5Fields) / sizeof(m_B5Fields[0]), m_B5Fields },
    { "  B6  ", "FLD_B6.PRG", "FLD_B6.FNT", sizeof(m_B6Fields) / sizeof(m_B6Fields[0]), m_B6Fields },

    { "  C2  ", "FLD_C2.PRG", "FLD_C2.FNT", sizeof(m_C2Fields) / sizeof(m_C2Fields[0]), m_C2Fields },
    { "  C3  ", "FLD_C4.PRG", "FLD_C4.FNT", sizeof(m_DemoFields) / sizeof(m_DemoFields[0]), m_DemoFields },
    { "  C4  ", "FLD_C4.PRG", "FLD_C4.FNT", sizeof(m_C4Fields) / sizeof(m_C4Fields[0]), m_C4Fields },
    { "  C5  ", NULL,         NULL,         sizeof(m_DummyFields) / sizeof(m_DummyFields[0]), m_DummyFields },
    { "  C8  ", "FLD_C8.PRG", "FLD_T0.FNT", sizeof(m_TowerFields) / sizeof(m_TowerFields[0]), m_TowerFields },

    { "  D2  ", "FLD_D2.PRG", "FLD_D2.FNT", sizeof(m_D2Fields) / sizeof(m_D2Fields[0]), m_D2Fields },
    { "  D3  ", "FLD_D3.PRG", "FLD_D3.FNT", sizeof(m_D3Fields) / sizeof(m_D3Fields[0]), m_D3Fields },
    { "  D4  ", "FLD_C8.PRG", "FLD_T0.FNT", sizeof(m_TowerFields) / sizeof(m_TowerFields[0]), m_TowerFields },
    { "  D5  ", "FLD_D5.PRG", "FLD_D5.FNT", sizeof(m_D5Fields) / sizeof(m_D5Fields[0]), m_D5Fields },

    { "  GAME", "FLD_D5.PRG", "FLD_D5.FNT", sizeof(m_GameFields) / sizeof(m_GameFields[0]), m_GameFields },
    { "  BT0 ", "FLD_A3.PRG", "FLD_A3.FNT", sizeof(m_BTFields) / sizeof(m_BTFields[0]), m_BTFields },
    { "  BT1 ", "FLD_A7.PRG", "FLD_A7.FNT", sizeof(m_BTFields) / sizeof(m_BTFields[0]), m_BTFields },
};

const s_dragonFileConfig dragonFilenameTable[DR_LEVEL_MAX] = {
    //DR_LEVEL_0_BASIC_WING
    {
        { "DRAGON0.MCB",    "DRAGON0.CGB" },
        { NULL,             NULL },
        { "DRAGONC0.MCB",   "DRAGONC0.CGB" },
    },

    //DR_LEVEL_1_VALIANT_WING
    {
        { "DRAGON1.MCB",     "DRAGON1.CGB" },
        { "DRAGONM1.MCB",    "DRAGONM1.CGB" },
        { "DRAGONC1.MCB",    "DRAGONC1.CGB" },
    },

    //DR_LEVEL_2_STRIPE_WING
    {
        { "DRAGON2.MCB",     "DRAGON2.CGB" },
        { "DRAGONM2.MCB",    "DRAGONM2.CGB" },
        { "DRAGONC2.MCB",    "DRAGONC2.CGB" },
    },

    //DR_LEVEL_3_PANZER_WING
    {
        { "DRAGON3.MCB",     "DRAGON3.CGB" },
        { "DRAGONM3.MCB",    "DRAGONM3.CGB" },
        { "DRAGONC3.MCB",    "DRAGONC3.CGB" },
    },

    //DR_LEVEL_4_EYE_WING
    {
        { "DRAGON4.MCB",     "DRAGON4.CGB" },
        { "DRAGONM4.MCB",    "DRAGONM4.CGB" },
        { "DRAGONC4.MCB",    "DRAGONC4.CGB" },
    },

    //DR_LEVEL_5_ARM_WING
    {
        { "DRAGON5.MCB",     "DRAGON5.CGB" },
        { "DRAGONM5.MCB",    "DRAGONM5.CGB" },
        { NULL,              NULL },
    },

    //DR_LEVEL_6_LIGHT_WING
    {
        { "DRAGON6.MCB",    "DRAGON6.CGB" },
        { NULL,             NULL },
        { NULL,             NULL },
    },

    //DR_LEVEL_7_SOLO_WING
    {
        { "DRAGON7.MCB",     "DRAGON7.CGB" },
        { "DRAGONM7.MCB",    "DRAGONM7.CGB" },
        { NULL,              NULL },
    },

    //DR_LEVEL_8_FLOATER
    {
        { "KTEI.MCB",       "KTEI.CGB" },
        { NULL,             NULL },
        { NULL,             NULL },
    },
};

s_RiderDefinitionSub dragon0_sub0[31] = {
    { {0x20212C, &gCommonFile }, 2 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { {0x202154, &gCommonFile }, 1 },
    { NULL, 0 },
    { NULL, 0 },
    { {0x202168, &gCommonFile }, 1 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { {0x20217C, &gCommonFile }, 2 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
};

const sDragonData3 dragonData3[DR_LEVEL_MAX] =
{
    //0
    {
        0,
        0,
        {
            4,8,260,0, dragon0_sub0,
            4,0,0,0, NULL,
            4,0,0,0, NULL,
            4,0,0,0, NULL,
            4,0,0,0, NULL,
            4,0,0,0, NULL,
            4,8,260,0, (s_RiderDefinitionSub*)1,
        }
    },
    //1
    {
        75,
        32,
        {
            4,8,268,1424, (s_RiderDefinitionSub*)1,
            4,24,1324,1424, (s_RiderDefinitionSub*)1,
            8,28,1328,1724, (s_RiderDefinitionSub*)1,
            12,32,1332,2024, (s_RiderDefinitionSub*)1,
            16,36,1336,2324, (s_RiderDefinitionSub*)1,
            20,40,1340,2624, (s_RiderDefinitionSub*)1,
            4,8,268,1424, (s_RiderDefinitionSub*)1,
        }
    },
    //2
    {
        77,
        32,
        {
            4,8,268,1540, (s_RiderDefinitionSub*)1,
            4,24,1324,1540, (s_RiderDefinitionSub*)1,
            8,28,1328,1848, (s_RiderDefinitionSub*)1,
            12,32,1332,2156, (s_RiderDefinitionSub*)1,
            16,36,1336,2464, (s_RiderDefinitionSub*)1,
            20,40,1340,2772, (s_RiderDefinitionSub*)1,
            4,8,268,1540, (s_RiderDefinitionSub*)1,
        }
    },
    //3
    {
        78,
        31,
        {
            4,8,260,1612, (s_RiderDefinitionSub*)1,
            4,24,1284,1612, (s_RiderDefinitionSub*)1,
            8,28,1288,1924, (s_RiderDefinitionSub*)1,
            12,32,1292,2236, (s_RiderDefinitionSub*)1,
            16,36,1296,2548, (s_RiderDefinitionSub*)1,
            20,40,1300,2860, (s_RiderDefinitionSub*)1,
            4,8,260,1612, (s_RiderDefinitionSub*)1,
        }
    },
    //4
    {
        77,
        33,
        {
            4,8,276,1568, (s_RiderDefinitionSub*)1,
            4,24,1364,1568, (s_RiderDefinitionSub*)1,
            8,28,1368,1876, (s_RiderDefinitionSub*)1,
            12,32,1372,2184, (s_RiderDefinitionSub*)1,
            16,36,1376,2492, (s_RiderDefinitionSub*)1,
            20,40,1380,2800, (s_RiderDefinitionSub*)1,
            4,8,276,1568, (s_RiderDefinitionSub*)1,
        }
    },
    //5
    {
        80,
        30,
        {
            4,8,252,1628, (s_RiderDefinitionSub*)1,
            4,24,1244,1628, (s_RiderDefinitionSub*)1,
            8,28,1248,1948, (s_RiderDefinitionSub*)1,
            12,32,1252,2268, (s_RiderDefinitionSub*)1,
            16,36,1256,2588, (s_RiderDefinitionSub*)1,
            20,40,1260,2908, (s_RiderDefinitionSub*)1,
            4,0,0,0, NULL,
        }
    },
    //6
    {
        0,
        0,
        {
            4,8,220,0, (s_RiderDefinitionSub*)1,
            4,0,0,0, NULL,
            4,0,0,0, NULL,
            4,0,0,0, NULL,
            4,0,0,0, NULL,
            4,0,0,0, NULL,
            4,0,0,0, NULL,
        }
    },
    //7
    {
        80,
        28,
        {
            4,8,236,1680, (s_RiderDefinitionSub*)1,
            4,24,1164,1680, (s_RiderDefinitionSub*)1,
            8,28,1168,2000, (s_RiderDefinitionSub*)1,
            12,32,1172,2320, (s_RiderDefinitionSub*)1,
            16,36,1176,2640, (s_RiderDefinitionSub*)1,
            20,40,1180,2960, (s_RiderDefinitionSub*)1,
            4,0,0,0, NULL,
        }
    },
    //8
    {
        0,
        0,
        {
            4,0,192,0, NULL,
            4,0,0,0, NULL,
            4,0,0,0, NULL,
            4,0,0,0, NULL,
            4,0,0,0, NULL,
            4,0,0,0, NULL,
            4,0,0,0, NULL,
        }
    },
};

const u16 dragonData2_0[] = {
    0x10C,
    0x110,
    0x114,
    0x118,
    0x11C,
    0x120,
    0x124,
    0x128,
    0x12C,
    0x130,
    0x134,
    0x138,
    0x13C,
    0x140,
    0x144,
    0x148,
    0x14C,
    0x150
};

const u16 dragonData2_1[] = {
    0x114,
    0x118,
    0x11C,
    0x120,
    0x124,
    0x128,
    0x12C,
    0x130,
    0x134,
    0x138,
    0x13C,
    0x140,
    0x144,
    0x148,
    0x14C,
    0x150,
    0x154,
    0x158,
};

const u16 dragonData2_2[] = {
    0x114,
    0x118,
    0x11C,
    0x120,
    0x124,
    0x128,
    0x12C,
    0x130,
    0x134,
    0x138,
    0x13C,
    0x140,
    0x144,
    0x148,
    0x14C,
    0x150,
    0x154,
    0x158,
};

const u16 dragonData2_3[] = {
    0x10C,
    0x110,
    0x114,
    0x118,
    0x11C,
    0x120,
    0x124,
    0x128,
    0x12C,
    0x130,
    0x134,
    0x138,
    0x13C,
    0x140,
    0x144,
    0x148,
    0x14C,
    0x150
};

const u16 dragonData2_4[] = {
    0x11C,
    0x120,
    0x124,
    0x128,
    0x12C,
    0x130,
    0x134,
    0x138,
    0x13C,
    0x140,
    0x144,
    0x148,
    0x14C,
    0x150,
    0x154,
    0x158,
    0x15C,
    0x160
};

const u16 dragonData2_5[] = {
    0x104,
    0x108,
    0x10C,
    0x110,
    0x114,
    0x118,
    0x11C,
    0x120,
    0x124,
    0x128,
    0x12C,
    0x130,
    0x134,
    0x138,
    0x13C,
    0x140,
    0x144,
    0x148,
};

const u16 dragonData2_6[] = {
    0xE4,
    0xE8,
    0xEC,
    0xF0,
    0xF4,
    0xF8,
    0xFC,
    0x100,
    0x104,
    0x108,
    0x10C,
    0x110,
    0x114,
    0x118,
    0x11C,
    0x120,
    0x124,
    0x128,
};

const u16 dragonData2_7[] = {
    0xF4,
    0xF8,
    0xFC,
    0x100,
    0x104,
    0x108,
    0x10C,
    0x110,
    0x114,
    0x118,
    0x11C,
    0x120,
    0x124,
    0x128,
    0x12C,
    0x130,
    0x134,
    0x138,
};

const u16 dragonData2_8[] = {
    0xD0,
    0xD4,
    0xD8,
    0xDC,
};

const s_dragonData2 dragonAnimOffsets[DR_LEVEL_MAX] = {
    { dragonData2_0, 0x12 },
    { dragonData2_1, 0x12 },
    { dragonData2_2, 0x12 },
    { dragonData2_3, 0x12 },
    { dragonData2_4, 0x12 },
    { dragonData2_5, 0x12 },
    { dragonData2_6, 0x12 },
    { dragonData2_7, 0x12 },
    { dragonData2_8, 4 },
};

s_dragonState* gDragonState = NULL;

void memcpy_dma(void* src, void* dst, u32 size)
{
    memcpy(dst, src, size);
}

extern s32 CosSinTable[4096];

fixedPoint getCos(u32 value)
{
    return fixedPoint(CosSinTable[value + 1024]);
}

fixedPoint getSin(u32 value)
{
    return fixedPoint(CosSinTable[value]);
}

u8 readSaturnU8(sSaturnPtr& ptr)
{
    sSaturnMemoryFile* pFile = ptr.m_file;
    u32 offsetInFile = ptr.m_offset - pFile->m_base;
    assert(offsetInFile + 1 <= pFile->m_dataSize);

    return READ_BE_U8(pFile->m_data + offsetInFile);
}

s8 readSaturnS8(sSaturnPtr& ptr)
{
    sSaturnMemoryFile* pFile = ptr.m_file;
    u32 offsetInFile = ptr.m_offset - pFile->m_base;
    assert(offsetInFile + 1 <= pFile->m_dataSize);

    return READ_BE_S8(pFile->m_data + offsetInFile);
}

s16 readSaturnS16(sSaturnPtr& ptr)
{
    sSaturnMemoryFile* pFile = ptr.m_file;
    u32 offsetInFile = ptr.m_offset - pFile->m_base;
    assert(offsetInFile + 2 <= pFile->m_dataSize);

    return READ_BE_S16(pFile->m_data + offsetInFile);
}

u16 readSaturnU16(sSaturnPtr& ptr)
{
    sSaturnMemoryFile* pFile = ptr.m_file;
    u32 offsetInFile = ptr.m_offset - pFile->m_base;
    assert(offsetInFile + 2 <= pFile->m_dataSize);

    return READ_BE_U16(pFile->m_data + offsetInFile);
}

s32 readSaturnS32(sSaturnPtr& ptr)
{
    sSaturnMemoryFile* pFile = ptr.m_file;
    u32 offsetInFile = ptr.m_offset - pFile->m_base;
    assert(offsetInFile + 4 <= pFile->m_dataSize);

    return READ_BE_S32(pFile->m_data + offsetInFile);
}

u32 readSaturnU32(sSaturnPtr& ptr)
{
    sSaturnMemoryFile* pFile = ptr.m_file;
    u32 offsetInFile = ptr.m_offset - pFile->m_base;
    assert(offsetInFile + 4 <= pFile->m_dataSize);

    return READ_BE_U32(pFile->m_data + offsetInFile);
}

sSaturnPtr readSaturnEA(sSaturnPtr& ptr)
{
    sSaturnMemoryFile* pFile = ptr.m_file;
    u32 offsetInFile = ptr.m_offset - pFile->m_base;
    assert(offsetInFile + 4 <= pFile->m_dataSize);

    u32 EA = READ_BE_U32(pFile->m_data + offsetInFile);

    sSaturnPtr newPtr = ptr;
    newPtr.m_offset = EA;

    return newPtr;
}

sVec3_FP readSaturnVec3(sSaturnPtr& ptr)
{
    sVec3_FP newVec;

    newVec[0] = readSaturnS32(ptr + 0);
    newVec[1] = readSaturnS32(ptr + 4);
    newVec[2] = readSaturnS32(ptr + 8);

    return newVec;
}
