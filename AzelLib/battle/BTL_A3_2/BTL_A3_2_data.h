#pragma once

#include "battle/battleOverlay.h"

struct BTL_A3_2_data : public battleOverlay
{
    BTL_A3_2_data();

    sSaturnPtr getEncounterDataTable() override;
    void invoke(sSaturnPtr Func, s_workAreaCopy* pParent, u32 arg0, u32 arg1) override;
    void invoke(sSaturnPtr Func, s_workAreaCopy* pParent) override;
    const struct sGrid* m_map;
};

extern BTL_A3_2_data* g_BTL_A3_2;
