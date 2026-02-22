#include "PDS.h"
#include "BTL_A3_gnd.h"
#include "BTL_A3_map6.h" // todo: clean

static void BTL_A3_gnd_Init(s_BTL_A3_Env* pThis)
{
    pThis->m38 = -0x96000;

    loadFile("SCBTLA31.SCB", getVdp2Vram(0x40000), 0);
    loadFile("SCBTL_A3.PNB", getVdp2Vram(0x62800), 0);
    loadFile("SPACE.PNB", getVdp2Vram(0x64000), 0);

    BTL_A3_Env_InitVdp2(pThis);
}

p_workArea Create_BTL_A3_gnd(p_workArea parent)
{
    static const s_BTL_A3_Env::TypedTaskDefinition definition = {
        &BTL_A3_gnd_Init,
        &BTL_A3_Env_Update,
        &BTL_A3_Env_Draw,
        nullptr,
    };

    return createSubTask<s_BTL_A3_Env>(parent, &definition);
}
