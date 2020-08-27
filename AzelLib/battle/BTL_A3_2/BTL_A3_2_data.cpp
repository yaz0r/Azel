#include "PDS.h"
#include "BTL_A3_2_data.h"
#include "Arachnoth.h"

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
    default:
        FunctionUnimplemented();
        break;
    }
}

void BTL_A3_2_data::init()
{
}
