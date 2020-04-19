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

// is targetable for gun
bool sBattleOverlayTask_C_IsTargetableForGun(sBattleTargetable* pThis)
{
    bool isVisible;
    sVec3_FP position = *pThis->m4_pPosition;

    if (((((((pThis->m50_flags) & 1) == 0) && (((pThis->m50_flags) & 0x100000) == 0)) &&
        ((gBattleManager->m10_battleOverlay->mC_targetSystem->m204_cameraMaxAltitude) <= pThis->m10_position[1])) &&
        (pThis->m10_position[1] <= (gBattleManager->m10_battleOverlay->mC_targetSystem->m200_cameraMinAltitude))) &&
        (((gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m1) != 0 ||
            (((pThis->m50_flags) & 2) == 0))))
    {
        fixedPoint clippingPlaneNear;
        fixedPoint clippingPlaneFar;
        getVdp1ClippingPlanes(clippingPlaneNear, clippingPlaneFar);

        isVisible = position[2] <= clippingPlaneFar;
        if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x1F] == 0)
        {
            isVisible = 1;
        }
        else
        {
            assert(0);
        }

    }
    else
    {
        isVisible = false;
    }
    return isVisible;
}

void sortEnemyByDistanceToDragon(s32 start, s32 end)
{
    do 
    {
        std::array<struct s_battleEnemy*, 0x80>& arrayToSort = gBattleManager->m10_battleOverlay->mC_targetSystem->m0_enemyTargetables;
        s32 iVar3 = arrayToSort[(start + end) / 2]->m8_distanceToDragonSquare;
        s32 iVar1 = start;
        s32 iVar5 = end;
        while (1)
        {
            while (arrayToSort[iVar1]->m8_distanceToDragonSquare < iVar3)
            {
                iVar1++;
            }
            while (iVar3 < arrayToSort[iVar5]->m8_distanceToDragonSquare)
            {
                iVar5--;
            }
            if (iVar5 <= iVar1)
            {
                break;
            }
            std::swap(arrayToSort[iVar1], arrayToSort[iVar5]);
            iVar1++;
            iVar5--;
        }
        if (start < iVar1 - 1)
        {
            sortEnemyByDistanceToDragon(start, iVar1 - 1);
        }
        start = iVar5 + 1;
    } while (start < end);
    
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
            if ((psVar8->m4_targetable->m50_flags & 0x40000) == 0) {
                psVar8->m4_targetable->m40 = *psVar8->m4_targetable->m4_pPosition;
                battleTargetable_updatePosition(psVar8->m4_targetable);
            }
            else
            {
                if ((psVar8->m4_targetable->m50_flags & 1) == 0)
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
            if (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2_needToSortEnemiesByDistanceFromDragon)
            {
                return;
            }
        }
        else
        {
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2_needToSortEnemiesByDistanceFromDragon = 1;
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
                if ((psVar8->m4_targetable->m50_flags & 0x80000000) && sBattleOverlayTask_C_IsTargetableForGun(psVar8->m4_targetable))
                {
                    psVar8->m0_isActive = 1;
                    psVar8->m8_distanceToDragonSquare = distanceSquareBetween2Points(psVar8->m4_targetable->m10_position, gBattleManager->m10_battleOverlay->m18_dragon->m8_position);
                    pThis->m20A_numSelectableEnemies++;
                }
                break;
            case 1:
                psVar8->m4_targetable->m5A = 0;
                if ((psVar8->m4_targetable->m50_flags & 0x40000000) && sBattleOverlayTask_C_IsTargetableForGun(psVar8->m4_targetable))
                {
                    psVar8->m0_isActive = 1;
                    psVar8->m8_distanceToDragonSquare = distanceSquareBetween2Points(psVar8->m4_targetable->m10_position, gBattleManager->m10_battleOverlay->m18_dragon->m8_position);
                    pThis->m20A_numSelectableEnemies++;
                }
                break;
            case 2:
                psVar8->m4_targetable->m5A = 0;
                if ((psVar8->m4_targetable->m50_flags & 0x20000000) && sBattleOverlayTask_C_IsTargetableForGun(psVar8->m4_targetable))
                {
                    psVar8->m0_isActive = 1;
                    psVar8->m8_distanceToDragonSquare = distanceSquareBetween2Points(psVar8->m4_targetable->m10_position, gBattleManager->m10_battleOverlay->m18_dragon->m8_position);
                    pThis->m20A_numSelectableEnemies++;
                }
                break;
            case 3:
                psVar8->m4_targetable->m5A = 0;
                if ((psVar8->m4_targetable->m50_flags & 0x10000000) && sBattleOverlayTask_C_IsTargetableForGun(psVar8->m4_targetable))
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

    if ((pThis->m20A_numSelectableEnemies > 0) && (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2_needToSortEnemiesByDistanceFromDragon))
    {
        sortEnemyByDistanceToDragon(0, 0x7F);
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2_needToSortEnemiesByDistanceFromDragon = 0;
    }

    for (int i=0; i<pThis->m20A_numSelectableEnemies; i++)
    {
        pThis->m0_enemyTargetables[i]->m0_isActive = i + 1;
        pThis->m0_enemyTargetables[i]->m4_targetable->m5A = i + 1;
    }

    if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m1)
    {
        for (int i=0; i<gBattleManager->m10_battleOverlay->mC_targetSystem->m20A_numSelectableEnemies; i++)
        {
            gBattleManager->m10_battleOverlay->mC_targetSystem->m0_enemyTargetables[i]->m4_targetable->m50_flags &= ~0x20000;
        }
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m1 = 0;
    }
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
