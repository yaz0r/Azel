#include "PDS.h"
#include "battleDebug.h"
#include "battleManager.h"
#include "battleOverlay.h"

static void s_battleDebug_Init(s_battleDebug* pThis)
{
    gBattleManager->m10_battleOverlay->m10_inBattleDebug = pThis;
    for (int i = 0; i < 0x1D; i++)
    {
        // This init the debug flags based on the gamestate internal flags
        int bitIndex = i + 1000;
        if (999 < bitIndex)
        {
            bitIndex = i + 0x1026;
        }
        if (mainGameState.getBit(bitIndex))
        {
            pThis->mFlags[i] = 1;
        }
        else
        {
            pThis->mFlags[i] = 0;
        }
    }
    pThis->mFlags[4] = 0;
    pThis->mFlags[1] = 6;
}

static void s_battleDebug_Update(s_battleDebug* pThis)
{
    FunctionUnimplemented();
}

static void s_battleDebug_Draw(s_battleDebug* pThis)
{
    FunctionUnimplemented();
}

static void s_battleDebug_Delete(s_battleDebug* pThis)
{
    FunctionUnimplemented();
}

void createInBattleDebugTask(p_workArea parent)
{
    static const s_battleDebug::TypedTaskDefinition definition = {
        &s_battleDebug_Init,
        &s_battleDebug_Update,
        &s_battleDebug_Draw,
        &s_battleDebug_Delete,
    };

    createSubTask<s_battleDebug>(parent, &definition);
}
