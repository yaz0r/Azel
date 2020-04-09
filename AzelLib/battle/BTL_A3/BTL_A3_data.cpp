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
}

