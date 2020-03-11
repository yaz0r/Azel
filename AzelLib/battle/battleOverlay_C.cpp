#include "PDS.h"
#include "battleOverlay_C.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"
#include "battleDragon.h"
#include "battleDebug.h"

void sBattleOverlayTask_C_Init(sBattleOverlayTask_C* pThis)
{
    gBattleManager->m10_battleOverlay->mC_targetSystem = pThis;

    for (int i = 0; i < 0x80; i++)
    {
        pThis->m0_enemyTargetables[i] = &gBattleManager->m10_battleOverlay->m4_battleEngine->m49C_enemies[i];
    }
}

s32 sBattleOverlayTask_C_UpdateSub0(sBattleTargetable* pThis)
{
    FunctionUnimplemented();
    return 1;
}

void sBattleOverlayTask_C_Update(sBattleOverlayTask_C* pThis)
{
    pThis->m200_cameraMinAltitude = gBattleManager->m10_battleOverlay->m4_battleEngine->m35C_cameraAltitudeMinMax[0];
    pThis->m204_cameraMaxAltitude = gBattleManager->m10_battleOverlay->m4_battleEngine->m35C_cameraAltitudeMinMax[1];

    std::array<s_battleEnemy, 0x80>::iterator psVar8 = gBattleManager->m10_battleOverlay->m4_battleEngine->m49C_enemies.begin();
    for (int i = 0; i < 0x80; i++)
    {
        if (psVar8->m0_isActive > -1)
        {
            if ((psVar8->m4_targetable->m50 & 0x40000) == 0) {
                psVar8->m4_targetable->m40 = *psVar8->m4_targetable->m4;
                battleTargetable_updatePosition(psVar8->m4_targetable);
            }
            else
            {
                if ((psVar8->m4_targetable->m50 & 1) == 0)
                {
                    gBattleManager->m10_battleOverlay->m4_battleEngine->m498_numEnemies--;
                }
                psVar8->m0_isActive = -1;
                psVar8->m8_distanceToDragonSquare = 0x7fffffff;
                psVar8->m4_targetable = nullptr;
            }
        }
        psVar8++;
    }
    
    if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m80)
    {
        if (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m1)
        {
            if (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2)
            {
                return;
            }
        }
        else
        {
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2 = 1;
        }
    }

    pThis->m20A_numSelectableEnemies = 0;

    psVar8 = gBattleManager->m10_battleOverlay->m4_battleEngine->m49C_enemies.begin();
    for (int i = 0; i < 0x80; i++)
    {
        psVar8->m8_distanceToDragonSquare = 0x7fffffff;
        if (psVar8->m0_isActive != -1)
        {
            psVar8->m8_distanceToDragonSquare = 0x7ffffffe;
            psVar8->m0_isActive = 0;
            switch (gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant)
            {
            case 0:
                psVar8->m4_targetable->m5A = 0;
                if ((psVar8->m4_targetable->m50 & 0x80000000) && sBattleOverlayTask_C_UpdateSub0(psVar8->m4_targetable))
                {
                    psVar8->m0_isActive = 1;
                    psVar8->m8_distanceToDragonSquare = distanceSquareBetween2Points(psVar8->m4_targetable->m10_position, gBattleManager->m10_battleOverlay->m18_dragon->m8_position);
                    pThis->m20A_numSelectableEnemies++;
                }
                break;
            case 1:
                psVar8->m4_targetable->m5A = 0;
                if ((psVar8->m4_targetable->m50 & 0x40000000) && sBattleOverlayTask_C_UpdateSub0(psVar8->m4_targetable))
                {
                    psVar8->m0_isActive = 1;
                    psVar8->m8_distanceToDragonSquare = distanceSquareBetween2Points(psVar8->m4_targetable->m10_position, gBattleManager->m10_battleOverlay->m18_dragon->m8_position);
                    pThis->m20A_numSelectableEnemies++;
                }
                break;
            case 2:
                psVar8->m4_targetable->m5A = 0;
                if ((psVar8->m4_targetable->m50 & 0x20000000) && sBattleOverlayTask_C_UpdateSub0(psVar8->m4_targetable))
                {
                    psVar8->m0_isActive = 1;
                    psVar8->m8_distanceToDragonSquare = distanceSquareBetween2Points(psVar8->m4_targetable->m10_position, gBattleManager->m10_battleOverlay->m18_dragon->m8_position);
                    pThis->m20A_numSelectableEnemies++;
                }
                break;
            case 3:
                psVar8->m4_targetable->m5A = 0;
                if ((psVar8->m4_targetable->m50 & 0x10000000) && sBattleOverlayTask_C_UpdateSub0(psVar8->m4_targetable))
                {
                    psVar8->m0_isActive = 1;
                    psVar8->m8_distanceToDragonSquare = distanceSquareBetween2Points(psVar8->m4_targetable->m10_position, gBattleManager->m10_battleOverlay->m18_dragon->m8_position);
                    pThis->m20A_numSelectableEnemies++;
                }
                break;
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
    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[9])
    {
        assert(0);
    }

    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x13])
    {
        assert(0);
    }

    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x14])
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
