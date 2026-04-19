#include "PDS.h"
#include "BTL_A7_data.h"
#include "BTL_A7_env.h"

BTL_A7_data* g_BTL_A7 = nullptr;

sSaturnPtr BTL_A7_data::getEncounterDataTable()
{
    return getSaturnPtr(0x060ac8f8);
}

void BTL_A7_data::invoke(sSaturnPtr Func, s_workAreaCopy* pParent, u32 arg0, u32 arg1)
{
    switch (Func.m_offset)
    {
    default:
        assert(0);
        break;
    }
}

void BTL_A7_data::invoke(sSaturnPtr Func, s_workAreaCopy* pParent)
{
    switch (Func.m_offset)
    {
    case 0x06057788:
        Create_BTL_A7_env0(pParent);
        break;
    case 0x06056c54:
        Create_BTL_A7_env1(pParent);
        break;
    case 0x06056588:
        Create_BTL_A7_env2(pParent);
        break;
    default:
        assert(0);
        break;
    }
}

p_workArea BTL_A7_data::invokeCreateEffect(sSaturnPtr Func, s_workAreaCopy* pParent)
{
    switch (Func.m_offset)
    {
    default:
        Unimplemented();
        return nullptr;
    }
}

BTL_A7_data::BTL_A7_data() : battleOverlay("BTL_A7.PRG")
{
    sSaturnPtr battleData = getSaturnPtr(0x060a8414);
    for (int i = 0; i < 9; i++)
    {
        m_battleEntries.push_back(sOverlayBattleEntry::read(battleData + i * 0x20));
    }
}
