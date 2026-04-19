#pragma once

#include "battle/battleOverlay.h"

struct BTL_A5_3_data : public battleOverlay
{
    BTL_A5_3_data();

    sSaturnPtr getEncounterDataTable() override;
    void invoke(sSaturnPtr Func, s_workAreaCopy* pParent, u32 arg0, u32 arg1) override;
    void invoke(sSaturnPtr Func, s_workAreaCopy* pParent) override;
    p_workArea invokeCreateEffect(sSaturnPtr Func, s_workAreaCopy* pParent) override;

    std::vector<sOverlayBattleEntry> m_battleEntries;
};

extern BTL_A5_3_data* g_BTL_A5_3;
