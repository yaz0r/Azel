#pragma once

#include "battle/battleOverlay.h"

struct BTL_A3_data : public battleOverlay
{
    BTL_A3_data();

    sSaturnPtr getEncounterDataTable() override;
    void invoke(sSaturnPtr Func, s_workAreaCopy* pParent, u32 arg0, u32 arg1) override;
    void invoke(sSaturnPtr Func, s_workAreaCopy* pParent) override;

    const struct sGrid* m_map3;
    const struct sGrid* m_map4;
    const struct sGrid* m_map6;

    const struct sGenericFormationData* m_60A8AE8_urchinFormation;
    const struct sGenericFormationData* m_60a8ac4_urchinFormation;
    const struct sGenericFormationData* m_60a7d34_urchinFormation;
    const struct sGenericFormationData* m_60a7d58_urchinFormation;
};

extern BTL_A3_data* g_BTL_A3;

