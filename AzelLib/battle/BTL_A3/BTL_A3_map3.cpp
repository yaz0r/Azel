#include "PDS.h"
#include "BTL_A3_map6.h"
#include "BTL_A3_map3.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "battle/battleGrid.h"
#include "field/field_a3/o_fld_a3.h" // TODO: remove
#include "kernel/graphicalObject.h"
#include "BTL_A3_data.h"
#include "town/town.h" // TODO: cleanup

static void BTL_A3_map3_Init(s_BTL_A3_Env* pThis)
{
    loadFile("SCBTLA31.SCB", getVdp2Vram(0x40000), 0);
    loadFile("SCBTL_A3.PNB", getVdp2Vram(0x62800), 0);
    loadFile("SPACE.PNB", getVdp2Vram(0x64000), 0);

    pThis->m38 = -0x8000;

    BTL_A3_Env_InitVdp2(pThis);

    allocateNPC(pThis, 8);
    initGridForBattle(dramAllocatorEnd[8].mC_fileBundle, g_BTL_A3->m_map3, 2, 2, 0x400000);
    pThis->m58 = dramAllocatorEnd[8].mC_fileBundle;

    gBattleManager->m10_battleOverlay->m8_gridTask->m1C8_flags |= 0x10;

    FunctionUnimplemented();

    allocateNPC(pThis, 0x14);

    // Create the waterfalls objects
    sSaturnPtr pVar2 = g_BTL_A3->getSaturnPtr(0x60a6f28);
    do 
    {
        initNPCFromStruct(pVar2);
        initNPCFromStruct(pVar2 + 0x20);
        initNPCFromStruct(pVar2 + 0x40);
        pVar2 += 0x60;
    } while (pVar2.m_offset < 0x60a6fe8);
}

p_workArea Create_BTL_A3_map3(p_workArea parent)
{
    static const s_BTL_A3_Env::TypedTaskDefinition definition = {
        &BTL_A3_map3_Init,
        &BTL_A3_Env_Update,
        &BTL_A3_Env_Draw,
        nullptr,
    };

    return createSubTask<s_BTL_A3_Env>(parent, &definition);
}
