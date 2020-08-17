#include "PDS.h"
#include "BTL_A3_2_data.h"

struct BTL_A3_2_data* g_BTL_A3_2 = nullptr;

sSaturnPtr BTL_A3_2_data::getEncounterDataTable()
{
    return getSaturnPtr(0x60ab8dc);
}

void BTL_A3_2_data::invoke(sSaturnPtr Func, p_workArea pParent, u32 arg0, u32 arg1)
{
    switch (Func.m_offset)
    {
    default:
        FunctionUnimplemented();
        break;
    }
}

void BTL_A3_2_data::invoke(sSaturnPtr Func, p_workArea pParent)
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
