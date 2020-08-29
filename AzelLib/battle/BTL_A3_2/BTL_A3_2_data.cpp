#include "PDS.h"
#include "BTL_A3_2_data.h"
#include "Arachnoth.h"
#include "BTL_A3_2_map.h"
#include "kernel/grid.h"

// Arachnoth boss battle

struct BTL_A3_2_data* g_BTL_A3_2 = nullptr;

sSaturnPtr BTL_A3_2_data::getEncounterDataTable()
{
    return getSaturnPtr(0x60ab8dc);
}

void BTL_A3_2_data::invoke(sSaturnPtr Func, s_workAreaCopy* pParent, u32 arg0, u32 arg1)
{
    switch (Func.m_offset)
    {
    case 0x60551A4:
        createArachnothFormation(pParent, arg0, arg1);
        break;
    default:
        FunctionUnimplemented();
        break;
    }
}

void BTL_A3_2_data::invoke(sSaturnPtr Func, s_workAreaCopy* pParent)
{
    switch (Func.m_offset)
    {
    case 0x6054842:
        BTL_A3_2_createMap(pParent);
        break;
    default:
        FunctionUnimplemented();
        break;
    }
}

BTL_A3_2_data::BTL_A3_2_data() : battleOverlay("BTL_A3_2.PRG")
{
    m_map = readGrid(getSaturnPtr(0x60a86bc), 2, 2);
}
