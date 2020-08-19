#pragma once

#include "battle/battleOverlay.h"

struct BTL_A3_2_data : public battleOverlay
{
    BTL_A3_2_data() : battleOverlay("BTL_A3_2.PRG")
    {

    }

    sSaturnPtr getEncounterDataTable() override;
    void invoke(sSaturnPtr Func, p_workArea pParent, u32 arg0, u32 arg1) override;
    void invoke(sSaturnPtr Func, p_workArea pParent) override;
    virtual void init();
};

extern BTL_A3_2_data* g_BTL_A3_2;
