#include "PDS.h"
#include "battleOverlay_C.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"
#include "battleDragon.h"
#include "battleDebug.h"

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
    pThis->m200_cameraMinAltitude = getBattleManager()->m10_battleOverlay->m4_battleEngine->m35C_cameraAltitudeMinMax[0];
    pThis->m204_cameraMaxAltitude = getBattleManager()->m10_battleOverlay->m4_battleEngine->m35C_cameraAltitudeMinMax[1];

    std::array<s_battleEngineSub, 0x80>::iterator psVar8 = getBattleManager()->m10_battleOverlay->m4_battleEngine->m49C.begin();
    for (int i = 0; i < 0x80; i++)
    {
        if (psVar8->m0 > -1)
        {
            if ((psVar8->m4->m50 & 0x40000) == 0) {
                psVar8->m4->m40 = *psVar8->m4->m4;
                s_battleDragon_InitSub5Sub0(psVar8->m4);
            }
            else
            {
                if ((psVar8->m4->m50 & 1) == 0)
                {
                    getBattleManager()->m10_battleOverlay->m4_battleEngine->m498--;
                }
                psVar8->m0 = -1;
                psVar8->m8 = 0x7fffffff;
                psVar8->m4 = nullptr;
            }
        }
        psVar8++;
    }
    
    if (getBattleManager()->m10_battleOverlay->m4_battleEngine->m188_flags.m80)
    {
        if (!getBattleManager()->m10_battleOverlay->m4_battleEngine->m188_flags.m1)
        {
            if (!getBattleManager()->m10_battleOverlay->m4_battleEngine->m188_flags.m2)
            {
                return;
            }
        }
        else
        {
            getBattleManager()->m10_battleOverlay->m4_battleEngine->m188_flags.m2 = 1;
        }
    }

    pThis->m204_cameraMaxAltitude = 0;

    psVar8 = getBattleManager()->m10_battleOverlay->m4_battleEngine->m49C.begin();
    for (int i = 0; i < 0x80; i++)
    {
        psVar8->m8 = 0x7fffffff;
        if (psVar8->m0 != -1)
        {
            psVar8->m8 = 0x7ffffffe;
            psVar8->m0 = 0;
            switch (getBattleManager()->m10_battleOverlay->m4_battleEngine->m22C_battleDirection)
            {
            default:
                assert(0);
            }
        }
        psVar8++;
    }

    FunctionUnimplemented();
}

void sBattleOverlayTask_C_Draw(sBattleOverlayTask_C* pThis)
{
    if (getBattleManager()->m10_battleOverlay->m10_inBattleDebug->mFlags[9])
    {
        assert(0);
    }

    if (getBattleManager()->m10_battleOverlay->m10_inBattleDebug->mFlags[0x13])
    {
        assert(0);
    }

    if (getBattleManager()->m10_battleOverlay->m10_inBattleDebug->mFlags[0x14])
    {
        assert(0);
    }
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
