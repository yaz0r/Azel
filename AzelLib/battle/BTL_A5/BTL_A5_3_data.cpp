#include "PDS.h"
#include "BTL_A5_3_data.h"
#include "BTL_A5_3_env.h"

struct BTL_A5_3_data* g_BTL_A5_3 = nullptr;

struct sBTL_A5_3_LathumFormation : public s_workAreaTemplate<sBTL_A5_3_LathumFormation>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = {
            &sBTL_A5_3_LathumFormation::Init,
            &sBTL_A5_3_LathumFormation::Update,
            &sBTL_A5_3_LathumFormation::Draw,
            nullptr
        };
        return &taskDefinition;
    }

    // 060542b0
    static void Init(sBTL_A5_3_LathumFormation* pThis) { Unimplemented(); }
    // 06055ef8
    static void Update(sBTL_A5_3_LathumFormation* pThis) { Unimplemented(); }
    // 06056530
    static void Draw(sBTL_A5_3_LathumFormation* pThis) { Unimplemented(); }

    u8 m_pad[0x18C - sizeof(s_workAreaTemplate<sBTL_A5_3_LathumFormation>)];
    // size 0x18C
};

sSaturnPtr BTL_A5_3_data::getEncounterDataTable()
{
    return getSaturnPtr(0x060a64bc);
}

void BTL_A5_3_data::invoke(sSaturnPtr Func, s_workAreaCopy* pParent, u32 arg0, u32 arg1)
{
    switch (Func.m_offset)
    {
    case 0x060565da:
        createSubTask<sBTL_A5_3_LathumFormation>((p_workArea)pParent);
        break;
    default:
        assert(0);
        break;
    }
}

void BTL_A5_3_data::invoke(sSaturnPtr Func, s_workAreaCopy* pParent)
{
    switch (Func.m_offset)
    {
    case 0x06057d10:
        Create_BTL_A5_3_env(pParent);
        break;
    default:
        assert(0);
        break;
    }
}

p_workArea BTL_A5_3_data::invokeCreateEffect(sSaturnPtr Func, s_workAreaCopy* pParent)
{
    switch (Func.m_offset)
    {
    default:
        assert(0);
        return nullptr;
    }
}

BTL_A5_3_data::BTL_A5_3_data() : battleOverlay("BTL_A5_3.PRG")
{
    sSaturnPtr battleData = getSaturnPtr(0x060a3a98);
    m_battleEntries.push_back(sOverlayBattleEntry::read(battleData));
}
