#include "PDS.h"
#include "battleEngineSub1.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"
#include "battleDragon.h"

struct sBattleEngineSub1 : public s_workAreaTemplate<sBattleEngineSub1>
{
    s8 m1;
    s8 m2;
    s16 m6;
    //size 0x8C;
};

void battleEngineSub1_Update(sBattleEngineSub1* pThis)
{
    FunctionUnimplemented();

    sVec2_FP temp;
    generateCameraMatrixSub1(getBattleManager()->m10_battleOverlay->m4_battleEngine->mC - getBattleManager()->m10_battleOverlay->m18_dragon->m8_position, temp);

    FunctionUnimplemented();
}

void battleEngineSub1_Draw(sBattleEngineSub1* pThis)
{
    FunctionUnimplemented();
}

void createBattleEngineSub1(p_workArea parent)
{
    static const sBattleEngineSub1::TypedTaskDefinition definition = {
        nullptr,
        &battleEngineSub1_Update,
        &battleEngineSub1_Draw,
        nullptr,
    };

    sBattleEngineSub1* pNewTask = createSubTask<sBattleEngineSub1>(parent, &definition);
    pNewTask->m6 = 0x3C;
    pNewTask->m1 = randomNumber() & 7;
    pNewTask->m2 = randomNumber() & 7;
}
