#include "PDS.h"
#include "BTL_A3_Env.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "field/field_a3/o_fld_a3.h" // TODO: remove

struct s_BTL_A3_Env : public s_workAreaTemplate<s_BTL_A3_Env>
{
    s32 m38;
    // 0x9C
};

static void s_BTL_A3_Env_InitVoid(s_BTL_A3_Env* pThis)
{
    getBattleManager()->m10_battleOverlay->m1C_envTask = pThis;
    reinitVdp2();
    fieldPaletteTaskInitSub0();

    FunctionUnimplemented();
}

static void s_BTL_A3_Env_Init(s_BTL_A3_Env* pThis)
{
    loadFile("SCBTLA31.SCB", getVdp2Vram(0x40000), 0);
    loadFile("SCBTL_A3.PNB", getVdp2Vram(0x62800), 0);
    loadFile("SPACE.PNB", getVdp2Vram(0x64000), 0);

    pThis->m38 = 0x64000;

    s_BTL_A3_Env_InitVoid(pThis);

    FunctionUnimplemented();
}

static void s_BTL_A3_Env_Update(s_BTL_A3_Env* pThis)
{
    FunctionUnimplemented();
}

static void s_BTL_A3_Env_Draw(s_BTL_A3_Env* pThis)
{
    FunctionUnimplemented();
}

p_workArea Create_BTL_A3_Env(p_workArea parent)
{
    static const s_BTL_A3_Env::TypedTaskDefinition definition = {
        &s_BTL_A3_Env_Init,
        &s_BTL_A3_Env_Update,
        &s_BTL_A3_Env_Draw,
        nullptr,
    };

    return createSubTask<s_BTL_A3_Env>(parent, &definition);
}
