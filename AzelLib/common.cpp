#include "PDS.h"
#include "commonOverlay.h"

s_graphicEngineStatus graphicEngineStatus;

const u8 bitMasks[] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
const u8 reverseBitMasks[] = { 0x7F, 0xBF, 0xDF, 0xEF, 0xF7, 0xF8, 0xFD, 0xFE };

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
    //1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1
    0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
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

void memcpy_dma(void* src, void* dst, u32 size)
{
    memcpy(dst, src, size);
}

u8 readSaturnU8(sSaturnPtr ptr)
{
    sSaturnMemoryFile* pFile = ptr.m_file;
    u32 offsetInFile = ptr.m_offset - pFile->m_base;
    assert(offsetInFile + 1 <= pFile->m_dataSize);

    return READ_BE_U8(pFile->m_data + offsetInFile);
}

u8* getSaturnPtr(sSaturnPtr ptr)
{
    sSaturnMemoryFile* pFile = ptr.m_file;
    u32 offsetInFile = ptr.m_offset - pFile->m_base;
    assert(offsetInFile + 1 <= pFile->m_dataSize);

    return pFile->m_data + offsetInFile;
}

s8 readSaturnS8(sSaturnPtr ptr)
{
    sSaturnMemoryFile* pFile = ptr.m_file;
    u32 offsetInFile = ptr.m_offset - pFile->m_base;
    assert(offsetInFile + 1 <= pFile->m_dataSize);

    return READ_BE_S8(pFile->m_data + offsetInFile);
}

s16 readSaturnS16(sSaturnPtr ptr)
{
    sSaturnMemoryFile* pFile = ptr.m_file;
    u32 offsetInFile = ptr.m_offset - pFile->m_base;
    assert(offsetInFile + 2 <= pFile->m_dataSize);

    return READ_BE_S16(pFile->m_data + offsetInFile);
}

u16 readSaturnU16(sSaturnPtr ptr)
{
    sSaturnMemoryFile* pFile = ptr.m_file;
    u32 offsetInFile = ptr.m_offset - pFile->m_base;
    assert(offsetInFile + 2 <= pFile->m_dataSize);

    return READ_BE_U16(pFile->m_data + offsetInFile);
}

s32 readSaturnS32(sSaturnPtr ptr)
{
    sSaturnMemoryFile* pFile = ptr.m_file;
    u32 offsetInFile = ptr.m_offset - pFile->m_base;
    assert(offsetInFile + 4 <= pFile->m_dataSize);

    return READ_BE_S32(pFile->m_data + offsetInFile);
}

u32 readSaturnU32(sSaturnPtr ptr)
{
    sSaturnMemoryFile* pFile = ptr.m_file;
    u32 offsetInFile = ptr.m_offset - pFile->m_base;
    assert(offsetInFile + 4 <= pFile->m_dataSize);

    return READ_BE_U32(pFile->m_data + offsetInFile);
}

sSaturnPtr readSaturnEA(sSaturnPtr ptr)
{
    sSaturnMemoryFile* pFile = ptr.m_file;
    u32 offsetInFile = ptr.m_offset - pFile->m_base;
    assert(offsetInFile + 4 <= pFile->m_dataSize);

    u32 EA = READ_BE_U32(pFile->m_data + offsetInFile);

    sSaturnPtr newPtr = ptr;
    newPtr.m_offset = EA;

    return newPtr;
}

std::string readSaturnString(sSaturnPtr ptr)
{
    std::string newString;
    if (ptr.isNull())
    {
        return newString;
    }

    while (s8 newChar = readSaturnS8(ptr))
    {
        ptr += 1;
        newString += newChar;
    }

    return newString;
}

fixedPoint readSaturnFP(sSaturnPtr ptr)
{
    return readSaturnS32(ptr);
}

sVec3_FP readSaturnVec3(sSaturnPtr ptr)
{
    sVec3_FP newVec;

    newVec[0] = readSaturnS32(ptr + 0);
    newVec[1] = readSaturnS32(ptr + 4);
    newVec[2] = readSaturnS32(ptr + 8);

    return newVec;
}

u8* sSaturnPtr::getRawPointer()
{
    sSaturnMemoryFile* pFile = m_file;
    u32 offsetInFile = m_offset - m_file->m_base;
    assert(offsetInFile <= pFile->m_dataSize);

    return pFile->m_data + offsetInFile;
}

// TODO: move to kernel
void getVdp1ClippingCoordinates(std::array<s16, 4>& r4)
{
    r4[0] = graphicEngineStatus.m405C.VDP1_X1;
    r4[1] = graphicEngineStatus.m405C.VDP1_Y1;
    r4[2] = graphicEngineStatus.m405C.VDP1_X2;
    r4[3] = graphicEngineStatus.m405C.VDP1_Y2;
}

void getVdp1LocalCoordinates(std::array<s16, 2>& r4)
{
    r4[0] = graphicEngineStatus.m405C.m44_localCoordinatesX;
    r4[1] = graphicEngineStatus.m405C.m46_localCoordinatesY;
}

std::vector<s_hotpointDefinition>* sHotpointBundle::getData(struct s_3dModel* pModel)
{
    if(m_EA.isNull())
    {
        return nullptr;
    }

    if (m_cachedData.size())
    {
        assert(m_cachedData.size() == pModel->m12_numBones);
        return &m_cachedData;
    }

    for (int i = 0; i < pModel->m12_numBones; i++)
    {
        s_hotpointDefinition newValue;
        sSaturnPtr ptr = readSaturnEA(m_EA + 8 * i);
        newValue.m4_count = readSaturnU32(m_EA + 8 * i + 4);
        for (int j = 0; j < newValue.m4_count; j++)
        {
            s_hotpoinEntry entry;
            entry.m0 = readSaturnS32(ptr + 0);
            entry.m4 = readSaturnVec3(ptr + 4);
            entry.m10 = readSaturnS32(ptr + 0x10);
            ptr += 0x14;

            newValue.m0.push_back(entry);
        }
        m_cachedData.push_back(newValue);
    }

    return &m_cachedData;
}

#ifndef SHIPPING_BUILD
void InspectTask(const char* className, void* pBase, const std::vector<s_memberDefinitions>& members)
{
    ImGui::Begin(className);
    ImGui::Columns(3);
    for (int i = 0; i < members.size(); i++)
    {
        size_t sVec3_FP_type = typeid(sVec3_FP).hash_code();
        const s_memberDefinitions& member = members[i];
        if (member.m_type == typeid(sVec3_FP).hash_code())
        {
            ImGui::Text("sVec3_FP");
            ImGui::NextColumn();
            ImGui::Text(member.m_name.c_str());
            ImGui::NextColumn();
            Imgui_Vec3FP((sVec3_FP*)(member.m_ptr + (u8*)pBase));
            ImGui::NextColumn();
        }
        else if (member.m_type == typeid(s8).hash_code())
        {
            ImGui::Text("s8");
            ImGui::NextColumn();
            ImGui::Text(member.m_name.c_str());
            ImGui::NextColumn();
            ImGui::PushID(member.m_name.c_str());
            int value = *(s8*)(member.m_ptr + (u8*)pBase);
            ImGui::InputInt("", &value);
            ImGui::PopID();
            ImGui::NextColumn();
        }
        else if (member.m_type == typeid(s32).hash_code())
        {
            ImGui::Text("s32");
            ImGui::NextColumn();
            ImGui::Text(member.m_name.c_str());
            ImGui::NextColumn();
            ImGui::PushID(member.m_name.c_str());
            int value = *(s32*)(member.m_ptr + (u8*)pBase);
            ImGui::InputInt("", &value);
            ImGui::PopID();
            ImGui::NextColumn();
        }
        else
        {
            assert(0);
        }
    }
    ImGui::Columns(1);
    ImGui::End();
}
#endif
