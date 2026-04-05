#include "PDS.h"
#include "battleGenericData.h"

const std::vector<quadColor> BTL_GenericData::m60AE424 = { {0xFE00, 0xC208, 0xC208, 0xFE00} };
const std::vector<quadColor> BTL_GenericData::m60AE42C = { {0x8E1F, 0x8F1F, 0x8F1F, 0x8F1F} };
const std::vector<quadColor> BTL_GenericData::m60AE434 = { {0x83EE, 0x93EE, 0x93EE, 0x83EE} };
const std::vector<quadColor> BTL_GenericData::m60AE43C = { {0x83FF, 0x83FF, 0x83FF, 0x83FF} };
sLaserData BTL_GenericData::mLaserData;
sLaserData BTL_GenericData::mBoltLaserData0;
sLaserData BTL_GenericData::mBoltLaserData1;
std::vector<sVdp1Quad> BTL_GenericData::m_0x60a7848_animatedQuad;
std::vector<sVdp1Quad> BTL_GenericData::m_0x60a8b0c_animatedQuad;
std::vector<sVdp1Quad> BTL_GenericData::m_0x60a8c24_animatedQuad;
std::vector<sVdp1Quad> BTL_GenericData::m_0x60a8fb4_animatedQuad;
std::vector<sVdp1Quad> BTL_GenericData::m_0x60b0658_animatedQuad;
std::vector<sVdp1Quad> BTL_GenericData::m_0x60a9120_animatedQuad;
std::vector<sVdp1Quad> BTL_GenericData::m_0x60ae454_animatedQuad;
quadColor BTL_GenericData::m_0x60ad80a_gouraudColors;
std::vector<std::vector<sVdp1Quad>> BTL_GenericData::m_0x60abef4_animatedQuads;

BTL_GenericData* g_BTL_GenericData = nullptr;

std::vector<sVdp1Quad> initVdp1Quad(sSaturnPtr ptr) {
    std::vector<sVdp1Quad> value;
    while(true) {
        auto& newValue = value.emplace_back();
        newValue.m0_isLast = readSaturnU8(ptr + 0);
        newValue.m1 = readSaturnU8(ptr + 1);
        newValue.m2_CMDCTRL = readSaturnU16(ptr + 2);
        newValue.m4_CMDPMOD = readSaturnU16(ptr + 4);
        newValue.m6_CMDSRCA = readSaturnU16(ptr + 6);
        newValue.m8_CMDSIZE = readSaturnU16(ptr + 8);
        newValue.mA_CMDCOLR = readSaturnU16(ptr + 0xA);
        newValue.mC_width = readSaturnFP(ptr + 0xC);
        newValue.m10_height = readSaturnFP(ptr + 0x10);
        newValue.m14_X = readSaturnFP(ptr + 0x14);
        newValue.m18_Y = readSaturnFP(ptr + 0x18);

        if (newValue.m0_isLast == 1) {
            break;
        }

        ptr += 0x1C;
    }
    return value;
}

// Parse a sLaserData from Saturn overlay address
void BTL_GenericData::parseLaserData(sLaserData& out, sSaturnPtr ea)
{
    out.m4 = readSaturnS16(ea + 0x4);
    out.m6 = readSaturnS16(ea + 0x6);
    out.m8 = readSaturnS16(ea + 0x8);
    out.mA = readSaturnS16(ea + 0xA);
    out.mC = readSaturnS16(ea + 0xC);
    out.mE = readSaturnS16(ea + 0xE);
    out.m10 = readSaturnS16(ea + 0x10);
    out.m12 = readSaturnS16(ea + 0x12);
    out.m14 = readSaturnS16(ea + 0x14);
    sSaturnPtr verticesEA = readSaturnEA(ea + 0x18);
    sSaturnPtr colorsEA = readSaturnEA(ea + 0x1C);
    out.m20_numLaserNodes = readSaturnU32(ea + 0x20);

    out.m18_vertices.resize(out.m20_numLaserNodes);
    out.m1C_colors.resize(out.m20_numLaserNodes);

    for (int i = 0; i < out.m20_numLaserNodes; i++)
    {
        out.m18_vertices[i] = readSaturnFP(verticesEA + 4 * i);
        out.m1C_colors[i][0] = readSaturnU16(colorsEA + 4 * 2 * i + 0);
        out.m1C_colors[i][1] = readSaturnU16(colorsEA + 4 * 2 * i + 2);
        out.m1C_colors[i][2] = readSaturnU16(colorsEA + 4 * 2 * i + 4);
        out.m1C_colors[i][3] = readSaturnU16(colorsEA + 4 * 2 * i + 6);
    }
}

BTL_GenericData::BTL_GenericData() : sSaturnMemoryFile("BTL_A3.PRG")
{
    parseLaserData(mLaserData, getSaturnPtr(0x60adce4));
    parseLaserData(mBoltLaserData0, getSaturnPtr(0x060acb74));
    parseLaserData(mBoltLaserData1, getSaturnPtr(0x060acb98));

    m_0x60a7848_animatedQuad = initVdp1Quad(getSaturnPtr(0x60a7848));
    m_0x60a8b0c_animatedQuad = initVdp1Quad(getSaturnPtr(0x60a8b0c));
    m_0x60a8c24_animatedQuad = initVdp1Quad(getSaturnPtr(0x60a8c24));
    m_0x60a8fb4_animatedQuad = initVdp1Quad(getSaturnPtr(0x60a8fb4));
    m_0x60b0658_animatedQuad = initVdp1Quad(getSaturnPtr(0x60b0658));
    m_0x60a9120_animatedQuad = initVdp1Quad(getSaturnPtr(0x60a9120));
    m_0x60ae454_animatedQuad = initVdp1Quad(getSaturnPtr(0x60ae454));
    {
        sSaturnPtr colorPtr = getSaturnPtr(0x060ad80a);
        for (int i = 0; i < 4; i++)
            m_0x60ad80a_gouraudColors[i] = readSaturnU16(colorPtr + i * 2);
    }

    // TODO: figure out how many there are (at least 9)
    for (int i = 0; i < 9; i++) {
        m_0x60abef4_animatedQuads.push_back(initVdp1Quad(readSaturnEA(getSaturnPtr(0x60abef4 + 4 * i))));
    }
}

void BTL_GenericData::staticInit()
{
    assert(g_BTL_GenericData == nullptr);
    g_BTL_GenericData = new BTL_GenericData;
}

