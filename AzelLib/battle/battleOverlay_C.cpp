#include "PDS.h"
#include "battleOverlay_C.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"
#include "battleDragon.h"
#include "battleDebug.h"

// 0605ceac
void sBattleOverlayTask_C_InitTargetablesArray(sBattleOverlayTask_C* pThis)
{
    for (int i = 0; i < 0x80; i++)
    {
        pThis->m0_enemyTargetables[i] = &gBattleManager->m10_battleOverlay->m4_battleEngine->m49C_enemies[i];
    }
}

// 0605ce86
void sBattleOverlayTask_C_Init(sBattleOverlayTask_C* pThis)
{
    gBattleManager->m10_battleOverlay->mC_targetSystem = pThis;
    sBattleOverlayTask_C_InitTargetablesArray(pThis);
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
            isVisible = true;
        }
        else
        {
            // Debug flag set: perform full projection + viewport checks
            getVdp1ClippingPlanes(clippingPlaneNear, clippingPlaneFar);
            if ((position[2] < clippingPlaneNear) || (clippingPlaneFar < position[2]))
            {
                isVisible = false;
            }
            else
            {
                s16 widthScale, heightScale;
                getVdp1ProjectionParams(&widthScale, &heightScale);

                s16 projX = (s16)setDividend(widthScale, position[0], position[2]);
                s16 projY = (s16)setDividend(heightScale, position[1], position[2]);

                s16 screenResolution[4];  // [left, bottom, right, top]
                getVdp1ScreenResolution(screenResolution);

                // Viewport bounds check
                if ((screenResolution[0] <= projX) && (screenResolution[1] <= projY) &&
                    (projY <= screenResolution[3]) && (projX <= screenResolution[2]))
                {
                    isVisible = true;
                }
                else
                {
                    isVisible = false;
                }
            }
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

// 0605cfac
void sBattleOverlayTask_C_Update(sBattleOverlayTask_C* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;

    // Update camera altitude bounds
    pThis->m200_cameraMinAltitude = pEngine->m35C_cameraAltitudeMinMax[0];
    pThis->m204_cameraMaxAltitude = pEngine->m35C_cameraAltitudeMinMax[1];

    // Process enemy activity state
    for (int i = 0; i < 0x80; i++)
    {
        s_battleEnemy* pEnemy = &pEngine->m49C_enemies[i];
        if (pEnemy->m0_isActive > -1)
        {
            if ((pEnemy->m4_targetable->m50_flags & 0x40000) == 0)
            {
                // Update position for active enemy
                pEnemy->m4_targetable->m40 = *pEnemy->m4_targetable->m4_pPosition;
                battleTargetable_updatePosition(pEnemy->m4_targetable);
            }
            else
            {
                // Deactivate enemy
                if ((pEnemy->m4_targetable->m50_flags & 1) == 0)
                {
                    pEngine->m498_numEnemies--;
                }
                pEnemy->m0_isActive = -1;
                pEnemy->m8_distanceToDragonSquare = 0x7fffffff;
                pEnemy->m4_targetable = nullptr;
            }
        }
    }

    // Check if we should process targeting
    if (pEngine->m188_flags.m80)
    {
        if (!pEngine->m188_flags.m1)
        {
            if (!pEngine->m188_flags.m2_needToSortEnemiesByDistanceFromDragon)
            {
                return;
            }
        }
        else
        {
            pEngine->m188_flags.m2_needToSortEnemiesByDistanceFromDragon = 1;
        }
    }

    pThis->m20A_numSelectableEnemies = 0;

    // Select targetable enemies based on quadrant
    for (int i = 0; i < 0x80; i++)
    {
        s_battleEnemy* pEnemy = &pEngine->m49C_enemies[i];
        pEnemy->m8_distanceToDragonSquare = 0x7fffffff;

        if (pEnemy->m0_isActive != -1)
        {
            pEnemy->m8_distanceToDragonSquare = 0x7ffffffe;
            pEnemy->m0_isActive = 0;
            u8 quadrant = pEngine->m22C_dragonCurrentQuadrant;
            u32 quadrantMask = 0;

            switch (quadrant)
            {
            case 0: quadrantMask = 0x80000000; break;
            case 1: quadrantMask = 0x40000000; break;
            case 2: quadrantMask = 0x20000000; break;
            case 3: quadrantMask = 0x10000000; break;
            default: assert(0);
            }

            pEnemy->m4_targetable->m5A = 0;
            if ((pEnemy->m4_targetable->m50_flags & quadrantMask) &&
                sBattleOverlayTask_C_IsTargetableForGun(pEnemy->m4_targetable))
            {
                pEnemy->m0_isActive = 1;
                pEnemy->m8_distanceToDragonSquare = distanceSquareBetween2Points(
                    pEnemy->m4_targetable->m10_position,
                    gBattleManager->m10_battleOverlay->m18_dragon->m8_position);
                pThis->m20A_numSelectableEnemies++;
            }
        }
    }

    // Sort enemies by distance if needed
    if ((pThis->m20A_numSelectableEnemies > 0) &&
        (pEngine->m188_flags.m2_needToSortEnemiesByDistanceFromDragon))
    {
        sortEnemyByDistanceToDragon(0, 0x7F);
        pEngine->m188_flags.m2_needToSortEnemiesByDistanceFromDragon = 0;
    }

    // Set selected enemy indices
    for (int i = 0; i < pThis->m20A_numSelectableEnemies; i++)
    {
        pThis->m0_enemyTargetables[i]->m0_isActive = i + 1;
        pThis->m0_enemyTargetables[i]->m4_targetable->m5A = i + 1;
    }

    // Clear flag on selected enemies if needed
    if (pEngine->m188_flags.m1)
    {
        for (int i = 0; i < pThis->m20A_numSelectableEnemies; i++)
        {
            pThis->m0_enemyTargetables[i]->m4_targetable->m50_flags &= ~0x20000;
        }
        pEngine->m188_flags.m1 = 0;
    }
}

void sBattleOverlayTask_C_Draw(sBattleOverlayTask_C* pThis)
{
    // Debug flag 9: Draw enemy count info
    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[9])
    {
        vdp2PrintStatus.m14_oldPalette = vdp2PrintStatus.m10_palette;
        vdp2PrintStatus.m10_palette = 0xc000;
        vdp2DebugPrintSetPosition(1, 9);

        s32 numActive = battleEngine_getNumActiveEnemies();
        s32 maxEnemies = (s32)gBattleManager->m10_battleOverlay->m4_battleEngine->m498_numEnemies;
        vdp2PrintfSmallFont("COL#=%2d,MAX=%2d", numActive, maxEnemies);

        vdp2PrintStatus.m10_palette = vdp2PrintStatus.m14_oldPalette;
    }

    // Debug flag 0x13: Draw target crosshairs (positions for each selectable enemy)
    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x13])
    {
        for (int i = 0; i < pThis->m20A_numSelectableEnemies; i++)
        {
            if (pThis->m0_enemyTargetables[i]->m0_isActive > 0)
            {
                battleTargetable_updatePosition(pThis->m0_enemyTargetables[i]->m4_targetable);

                // Get position data to draw crosshair
                sVec3_FP posData;
                posData.m0_X = pThis->m0_enemyTargetables[i]->m4_targetable->m10_position[0];
                posData.m4_Y = pThis->m0_enemyTargetables[i]->m4_targetable->m10_position[1];
                posData.m8_Z = pThis->m0_enemyTargetables[i]->m4_targetable->m10_position[2];
                s32 pal = pThis->m0_enemyTargetables[i]->m4_targetable->m4C;

                // 0605ca34: Draw crosshair/marker at target position
                Unimplemented(); // FUN_BTL_A3__0605ca34(&posData);
            }
        }
    }

    // Debug flag 0x14: Print selectable enemy list (up to 10 entries with positions)
    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x14])
    {
        vdp2PrintStatus.m14_oldPalette = vdp2PrintStatus.m10_palette;
        vdp2PrintStatus.m10_palette = 0xc000;

        int numToPrint = pThis->m20A_numSelectableEnemies;
        if (numToPrint > 10)
            numToPrint = 10;

        int screenY = 4;
        for (int i = 0; i < numToPrint; i++)
        {
            vdp2DebugPrintSetPosition(10, screenY);

            if (pThis->m0_enemyTargetables[i]->m0_isActive > 0)
            {
                sBattleTargetable* pTargetable = pThis->m0_enemyTargetables[i]->m4_targetable;
                s32 x = pTargetable->m10_position[0] >> 12;
                s32 y = pTargetable->m10_position[1] >> 12;
                vdp2PrintfSmallFont("%3d,%3d,%3d", x, y, 0);
            }
            screenY++;
        }

        vdp2PrintStatus.m10_palette = vdp2PrintStatus.m14_oldPalette;
    }
}

void sBattleOverlayTask_C_Delete(sBattleOverlayTask_C* pThis)
{
    // No cleanup needed
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
