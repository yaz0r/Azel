#include "PDS.h"
#include "battlePowerGauge.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"
#include "battleDebug.h"
#include "town/town.h" // todo: for npcFileDeleter

struct battlePowerGauge : public s_workAreaTemplate<battlePowerGauge>
{
    s32 m0;
    //size 0x28
};

void battlePowerGauge_update(battlePowerGauge* pThis)
{
    FunctionUnimplemented();
}

void battlePowerGauge_draw(battlePowerGauge* pThis)
{
    FunctionUnimplemented();
}


void createPowerGaugeTask(npcFileDeleter* parent, u16*, s32)
{
    static const battlePowerGauge::TypedTaskDefinition definition = {
        nullptr,
        &battlePowerGauge_update,
        &battlePowerGauge_draw,
        nullptr,
    };

    battlePowerGauge* pNewTask = createSubTask<battlePowerGauge>(parent, &definition);

    pNewTask->m0 = getBattleManager()->m10_battleOverlay->m4_battleEngine->m3B4;

    FunctionUnimplemented();
}
