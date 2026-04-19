#include "PDS.h"
#include "BTL_A7_2_data.h"

BTL_A7_2_data* g_BTL_A7_2 = nullptr;

sSaturnPtr BTL_A7_2_data::getEncounterDataTable()
{
    return getSaturnPtr(0x060a767c);
}

void BTL_A7_2_data::invoke(sSaturnPtr Func, s_workAreaCopy* pParent, u32 arg0, u32 arg1)
{
    switch (Func.m_offset)
    {
    default:
        assert(0);
        break;
    }
}

void BTL_A7_2_data::invoke(sSaturnPtr Func, s_workAreaCopy* pParent)
{
    switch (Func.m_offset)
    {
    case 0x0605830a:
        Unimplemented();
        break;
    default:
        assert(0);
        break;
    }
}

p_workArea BTL_A7_2_data::invokeCreateEffect(sSaturnPtr Func, s_workAreaCopy* pParent)
{
    switch (Func.m_offset)
    {
    default:
        Unimplemented();
        return nullptr;
    }
}

BTL_A7_2_data::BTL_A7_2_data() : battleOverlay("BTL_A7_2.PRG")
{
    sSaturnPtr battleData = getSaturnPtr(0x060a51bc);
    for (int i = 0; i < 1; i++)
    {
        m_battleEntries.push_back(sOverlayBattleEntry::read(battleData + i * 0x20));
    }
}
