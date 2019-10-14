#include "PDS.h"
#include "battleOverlay_C.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"

void sBattleOverlayTask_C_Init(sBattleOverlayTask_C* pThis)
{
    getBattleManager()->m10_battleOverlay->mC = pThis;

    for (int i = 0; i < 0x80; i++)
    {
        pThis->m0[i] = &getBattleManager()->m10_battleOverlay->m4_battleEngine->m49C[i];
    }
}

void sBattleOverlayTask_C_Update(sBattleOverlayTask_C* pThis)
{
    pThis->m200_cameraMinAltitude = getBattleManager()->m10_battleOverlay->m4_battleEngine->m354[2];
    pThis->m204_cameraMaxAltitude = getBattleManager()->m10_battleOverlay->m4_battleEngine->m354[3];
    FunctionUnimplemented();
}

void sBattleOverlayTask_C_Draw(sBattleOverlayTask_C* pThis)
{
    FunctionUnimplemented();
}

void sBattleOverlayTask_C_Delete(sBattleOverlayTask_C* pThis)
{
    FunctionUnimplemented();
}


void battleEngine_UpdateSub2(p_workArea parent)
{
    static const sBattleOverlayTask_C::TypedTaskDefinition definition = {
        &sBattleOverlayTask_C_Init,
        &sBattleOverlayTask_C_Update,
        &sBattleOverlayTask_C_Draw,
        &sBattleOverlayTask_C_Delete,
    };

    createSubTask<sBattleOverlayTask_C>(parent, &definition);
}
