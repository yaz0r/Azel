#include "PDS.h"
#include "battleTextDisplay.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"
#include "kernel/textDisplay.h"

#include "BTL_A3/BTL_A3_data.h"


void battleText_update(sBattleTextDisplayTask* pThis)
{
    switch (pThis->m16)
    {
    case 0:
        if (
            (gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode == eBattleModes::mE_battleIntro) &&
            (gBattleManager->m10_battleOverlay->m4_battleEngine->m38D_battleSubMode == 3) &&
            (pThis->m8 != nullptr))
        {
            pThis->m8->getTask()->markFinished();
        }
        break;
    case 1:
        setupVDP2StringRendering(8, 9, 0x1e, 2);
        clearVdp2TextArea();
        VDP2DrawString(readSaturnString(readSaturnEA(pThis->m0_texts + pThis->m12_textIndex * 4)).c_str());
        pThis->m16 = 0;
        break;
    default:
        assert(0);
    }
}

void battleText_delete(sBattleTextDisplayTask* pThis)
{
    FunctionUnimplemented();
}

void createBattleTextDisplay(p_workArea parent, sSaturnPtr data)
{
    static const sBattleTextDisplayTask::TypedTaskDefinition definition = {
        nullptr,
        &battleText_update,
        nullptr,
        &battleText_delete,
    };

    sBattleTextDisplayTask* pNewTask = createSubTask<sBattleTextDisplayTask>(parent, &definition);
    gBattleManager->m10_battleOverlay->m14_textDisplay = pNewTask;

    pNewTask->m0_texts = data;
    pNewTask->m4 = g_BTL_A3->getSaturnPtr(0x060ab658);
    pNewTask->m17 = pVdp2StringControl->m0_index;
    pNewTask->m18 = pNewTask->m17 - 1;
}
