#include "PDS.h"
#include "BTL_A5_2_data.h"
#include "BTL_A5_2_env.h"

struct BTL_A5_2_data* g_BTL_A5_2 = nullptr;

sSaturnPtr BTL_A5_2_data::getEncounterDataTable()
{
    return getSaturnPtr(0x060b35c0);
}

void BTL_A5_2_data::invoke(sSaturnPtr Func, s_workAreaCopy* pParent, u32 arg0, u32 arg1)
{
    switch (Func.m_offset)
    {
    case 0x06054078:
        Unimplemented(); // BTL_A5_2_createFormation — creates 0x100 formation task + palette + enemy/env sub-tasks
        break;
    default:
        assert(0);
        break;
    }
}

void BTL_A5_2_data::invoke(sSaturnPtr Func, s_workAreaCopy* pParent)
{
    switch (Func.m_offset)
    {
    case 0x060629fc:
        Create_BTL_A5_2_env(pParent);
        break;
    default:
        assert(0);
        break;
    }
}

p_workArea BTL_A5_2_data::invokeCreateEffect(sSaturnPtr Func, s_workAreaCopy* pParent)
{
    switch (Func.m_offset)
    {
    default:
        assert(0);
        return nullptr;
    }
}

BTL_A5_2_data::BTL_A5_2_data() : battleOverlay("BTL_A5_2.PRG")
{
    sSaturnPtr battleData = getSaturnPtr(0x060aeec4);
    m_battleEntries.push_back(sOverlayBattleEntry::read(battleData));
}
