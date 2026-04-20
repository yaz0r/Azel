#include "PDS.h"
#include "BTL_X0_data.h"
#include "BTL_X0_formation.h"
#include "BTL_X0_env.h"

BTL_X0_data* g_BTL_X0 = nullptr;

sSaturnPtr BTL_X0_data::getEncounterDataTable()
{
    return getSaturnPtr(0x060b9e08);
}

void BTL_X0_data::invoke(sSaturnPtr Func, s_workAreaCopy* pParent, u32 arg0, u32 arg1)
{
    switch (Func.m_offset)
    {
    case 0x06060d94:
        BTL_X0_createFormation0(pParent, arg0);
        break;
    case 0x060617d0:
        BTL_X0_createFormation1(pParent, arg0);
        break;
    case 0x06061ff2:
        BTL_X0_createFormation2(pParent, arg0);
        break;
    default:
        assert(0);
        break;
    }
}

void BTL_X0_data::invoke(sSaturnPtr Func, s_workAreaCopy* pParent)
{
    switch (Func.m_offset)
    {
    case 0x0605498a:
        Create_BTL_X0_env(pParent);
        break;
    case 0x0605535c:
        Create_BTL_X0_env2(pParent);
        break;
    default:
        assert(0);
        break;
    }
}

p_workArea BTL_X0_data::invokeCreateEffect(sSaturnPtr Func, s_workAreaCopy* pParent)
{
    switch (Func.m_offset)
    {
    default:
        Unimplemented();
        return nullptr;
    }
}

BTL_X0_data::BTL_X0_data() : battleOverlay("BTL_X0.PRG")
{
    sSaturnPtr battleData = getSaturnPtr(0x060b286c);
    for (int i = 0; i < 3; i++)
    {
        m_battleEntries.push_back(sOverlayBattleEntry::read(battleData + i * 0x20));
    }
}
