#include "PDS.h"
#include "BTL_A7_2_data.h"
#include "BTL_A7_2_formation.h"
#include "BTL_A7_2_env.h"

BTL_A7_2_data* g_BTL_A7_2 = nullptr;

sSaturnPtr BTL_A7_2_data::getEncounterDataTable()
{
    return getSaturnPtr(0x060a767c);
}

sSaturnPtr BTL_A7_2_data::getBerserkScrollConfigTable()
{
    return getSaturnPtr(0x060adcac);
}

void BTL_A7_2_data::invoke(sSaturnPtr Func, s_workAreaCopy* pParent, u32 arg0, u32 arg1)
{
    switch (Func.m_offset)
    {
    case 0x060545e0:
        BTL_A7_2_createFormation(pParent, arg0);
        break;
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
        Create_BTL_A7_2_env(pParent);
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
