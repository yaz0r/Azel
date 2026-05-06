#include "PDS.h"
#include "BTL_A5_data.h"
#include "BTL_A5_env.h"
#include "BTL_A5_formation.h"
#include "BTL_A5_variant.h"
#include "BTL_A5_single.h"
#include "BTL_A5_dual.h"

struct BTL_A5_data* g_BTL_A5 = nullptr;

sSaturnPtr BTL_A5_data::getEncounterDataTable()
{
    return getSaturnPtr(0x060b3198);
}

void BTL_A5_data::invoke(sSaturnPtr Func, s_workAreaCopy* pParent, u32 arg0, u32 arg1)
{
    switch (Func.m_offset)
    {
    case 0x0605cc8c:
        BTL_A5_createFormation(pParent, arg0);
        break;
    case 0x0605b112:
        BTL_A5_createFormation_single(pParent, arg0);
        break;
    case 0x060585e8:
        Unimplemented(); // BTL_A5_createFormation_multi — 0x50 task with dual NPC allocations
        break;
    case 0x060547da:
        BTL_A5_createFormation_variant(pParent, arg0);
        break;
    case 0x0605f31a:
        BTL_A5_createFormation_dual(pParent, arg0);
        break;
    case 0x06058df8:
        // 06058df8 — conditional: only fires when arg == 0xF (Pgoraias boss)
        if (arg0 == 0xF)
        {
            Unimplemented(); // BTL_A5_createPgoraias — 0x4C boss task with createSubTaskWithArg
        }
        break;
    default:
        assert(0);
        break;
    }
}

void BTL_A5_data::invoke(sSaturnPtr Func, s_workAreaCopy* pParent)
{
    switch (Func.m_offset)
    {
    case 0x0605a738:
        Create_BTL_A5_env(pParent);
        break;
    case 0x0605a71a:
        Create_BTL_A5_env_grid(pParent);
        break;
    default:
        assert(0);
        break;
    }
}

p_workArea BTL_A5_data::invokeCreateEffect(sSaturnPtr Func, s_workAreaCopy* pParent)
{
    switch (Func.m_offset)
    {
    default:
        assert(0);
        return nullptr;
    }
}

BTL_A5_data::BTL_A5_data() : battleOverlay("BTL_A5.PRG")
{
    sSaturnPtr battleData = getSaturnPtr(0x060ac9f8);
    for (int i = 0; i < 16; i++)
    {
        m_battleEntries.push_back(sOverlayBattleEntry::read(battleData + i * 0x20));
    }
}
