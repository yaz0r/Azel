#pragma once

#include "battle/battleOverlay.h"

struct BTL_A3_data : public battleOverlay
{
    sSaturnPtr getBattleEngineInitData() override;
    void invoke(sSaturnPtr Func, p_workArea pParent, u32 arg0, u32 arg1) override;
    void invoke(sSaturnPtr Func, p_workArea pParent) override;
    virtual void init();

    const struct sGrid* m_map3;
    const struct sGrid* m_map4;
    const struct sGrid* m_map6;
};

extern BTL_A3_data* g_BTL_A3;
