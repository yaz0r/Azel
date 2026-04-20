#include "PDS.h"
#include "BTL_A7_env.h"
#include "BTL_A7_data.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "battle/BTL_A3/BTL_A3_map6.h"
#include "shared/vdp2PlaneTask.h"

// 06056d8c
static void BTL_A7_env0_Init(sVdp2PlaneTask* pThis)
{
    gBattleManager->m10_battleOverlay->m1C_envTask = (sVdp2PlaneTask*)pThis;
    Unimplemented();
}

// 06057224
static void BTL_A7_env0_Update(sVdp2PlaneTask* pThis)
{
    Unimplemented();
}

// 060572c0
static void BTL_A7_env0_Draw(sVdp2PlaneTask* pThis)
{
    Unimplemented();
}

// 06057788
void Create_BTL_A7_env0(s_workAreaCopy* parent)
{
    static const sVdp2PlaneTask::TypedTaskDefinition def = {
        &BTL_A7_env0_Init,
        &BTL_A7_env0_Update,
        &BTL_A7_env0_Draw,
        nullptr,
    };
    createSubTask<sVdp2PlaneTask>((s_workArea*)parent, &def);
}

// 060565b0
static void BTL_A7_env1_Init(sVdp2PlaneTask* pThis)
{
    gBattleManager->m10_battleOverlay->m1C_envTask = (sVdp2PlaneTask*)pThis;
    Unimplemented();
}

// 06056136
static void BTL_A7_env12_Update(sVdp2PlaneTask* pThis)
{
    Unimplemented();
}

// 06056a24
static void BTL_A7_env1_Draw(sVdp2PlaneTask* pThis)
{
    Unimplemented();
}

// 06056c54
void Create_BTL_A7_env1(s_workAreaCopy* parent)
{
    static const sVdp2PlaneTask::TypedTaskDefinition def = {
        &BTL_A7_env1_Init,
        &BTL_A7_env12_Update,
        &BTL_A7_env1_Draw,
        nullptr,
    };
    createSubTask<sVdp2PlaneTask>((s_workArea*)parent, &def);
}

// 06055d00
static void BTL_A7_env2_Init(sVdp2PlaneTask* pThis)
{
    gBattleManager->m10_battleOverlay->m1C_envTask = (sVdp2PlaneTask*)pThis;
    Unimplemented();
}

// 060561ac
static void BTL_A7_env2_Draw(sVdp2PlaneTask* pThis)
{
    Unimplemented();
}

// 06056588
void Create_BTL_A7_env2(s_workAreaCopy* parent)
{
    static const sVdp2PlaneTask::TypedTaskDefinition def = {
        &BTL_A7_env2_Init,
        &BTL_A7_env12_Update,
        &BTL_A7_env2_Draw,
        nullptr,
    };
    createSubTask<sVdp2PlaneTask>((s_workArea*)parent, &def);
}
