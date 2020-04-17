#include "PDS.h"
#include "BTL_A3_data.h"
#include "BTL_A3_map3.h"
#include "BTL_A3_map4.h"
#include "BTL_A3_map6.h"
#include "BTL_A3_BaldorFormation.h"
#include "kernel/grid.h"

struct BTL_A3_data* g_BTL_A3 = nullptr;

sSaturnPtr BTL_A3_data::getBattleEngineInitData()
{
    return getSaturnPtr(0x60AAFA0);
}

void BTL_A3_data::invoke(sSaturnPtr Func, p_workArea pParent, u32 arg0, u32 arg1)
{
    switch (Func.m_offset)
    {
    case 0x060565da:
        Create_BTL_A3_BaldorFormation(pParent, arg0);
        break;
    default:
        FunctionUnimplemented();
        break;
    }
}

void BTL_A3_data::invoke(sSaturnPtr Func, p_workArea pParent)
{
    switch (Func.m_offset)
    {
    case 0x06054b58: // map6
        Create_BTL_A3_map6(pParent);
        break;
    case 0x06054b44: // map3
        Create_BTL_A3_map3(pParent);
        break;
    case 0x06054b4e: // map4
        Create_BTL_A3_map4(pParent);
    default:
        FunctionUnimplemented();
        break;
    }
}

void BTL_A3_data::init()
{
    m_map3 = readGrid(getSaturnPtr(0x60a5c78), 2, 2);
    m_map4 = readGrid(getSaturnPtr(0x60a605c), 2, 2);
    m_map6 = readGrid(getSaturnPtr(0x60a6698), 2, 2);

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

    for (int i=0; i< mLaserData.m20_numLaserNodes; i++)
    {
        mLaserData.m18_vertices[i] = readSaturnFP(laserVerticesEA + 4 * i);
        mLaserData.m1C_colors[i].m0_colors[0] = readSaturnU16(laserColorsEA + 4 * 2 * i + 0);
        mLaserData.m1C_colors[i].m0_colors[1] = readSaturnU16(laserColorsEA + 4 * 2 * i + 2);
        mLaserData.m1C_colors[i].m0_colors[2] = readSaturnU16(laserColorsEA + 4 * 2 * i + 4);
        mLaserData.m1C_colors[i].m0_colors[3] = readSaturnU16(laserColorsEA + 4 * 2 * i + 6);
    }
}

