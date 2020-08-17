#include "PDS.h"
#include "battleGenericData.h"

const std::vector<quadColor> BTL_GenericData::m60AE424 = { {0xFE00, 0xC208, 0xC208, 0xFE00} };
const std::vector<quadColor> BTL_GenericData::m60AE42C = { {0x8E1F, 0x8F1F, 0x8F1F, 0x8F1F} };
const std::vector<quadColor> BTL_GenericData::m60AE434 = { {0x83EE, 0x93EE, 0x93EE, 0x83EE} };
const std::vector<quadColor> BTL_GenericData::m60AE43C = { {0x83FF, 0x83FF, 0x83FF, 0x83FF} };
sLaserData BTL_GenericData::mLaserData;

BTL_GenericData* g_BTL_GenericData = nullptr;

BTL_GenericData::BTL_GenericData() : sSaturnMemoryFile("BTL_A3.PRG")
{
    sSaturnPtr laserDataEA = getSaturnPtr(0x60adce4);

    mLaserData.m4 = readSaturnS16(laserDataEA + 0x4);
    mLaserData.m6 = readSaturnS16(laserDataEA + 0x6);
    mLaserData.m8 = readSaturnS16(laserDataEA + 0x8);
    mLaserData.mA = readSaturnS16(laserDataEA + 0xA);
    mLaserData.mC = readSaturnS16(laserDataEA + 0xC);
    mLaserData.mE = readSaturnS16(laserDataEA + 0xE);
    mLaserData.m10 = readSaturnS16(laserDataEA + 0x10);
    mLaserData.m12 = readSaturnS16(laserDataEA + 0x12);
    mLaserData.m14 = readSaturnS16(laserDataEA + 0x14);
    sSaturnPtr laserVerticesEA = readSaturnEA(laserDataEA + 0x18);
    sSaturnPtr laserColorsEA = readSaturnEA(laserDataEA + 0x1C);
    mLaserData.m20_numLaserNodes = readSaturnU32(laserDataEA + 0x20);

    mLaserData.m18_vertices.resize(mLaserData.m20_numLaserNodes);
    mLaserData.m1C_colors.resize(mLaserData.m20_numLaserNodes);

    for (int i = 0; i < mLaserData.m20_numLaserNodes; i++)
    {
        mLaserData.m18_vertices[i] = readSaturnFP(laserVerticesEA + 4 * i);
        mLaserData.m1C_colors[i][0] = readSaturnU16(laserColorsEA + 4 * 2 * i + 0);
        mLaserData.m1C_colors[i][1] = readSaturnU16(laserColorsEA + 4 * 2 * i + 2);
        mLaserData.m1C_colors[i][2] = readSaturnU16(laserColorsEA + 4 * 2 * i + 4);
        mLaserData.m1C_colors[i][3] = readSaturnU16(laserColorsEA + 4 * 2 * i + 6);
    }
}

void BTL_GenericData::staticInit()
{
    assert(g_BTL_GenericData == nullptr);
    g_BTL_GenericData = new BTL_GenericData;
}

