#include "PDS.h"
#include "BTL_X0_formation.h"
#include "BTL_X0_enemy.h"
#include "BTL_X0_data.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "battle/battleEngine.h"
#include "battle/battleEngineSub0.h"
#include "kernel/graphicalObject.h"
#include "battle/battleDebug.h"
#include "audio/soundDriver.h"
#include "town/town.h"
#include "battle/battleFormationBase.h"
#include "battle/battleDragon.h"

// 0606084c
static void BTL_X0_setEnemyCommandOnly(sBTL_X0_FormationTask* pThis, s8 command)
{
    sBTL_X0_EnemyModel* pEnemy = (sBTL_X0_EnemyModel*)pThis->m74_enemyModelTask;
    pEnemy->m30A_commandIndex = command;
}

// 06060856
static void BTL_X0_setEnemyCommand(sBTL_X0_FormationTask* pThis, s8 command)
{
    sBTL_X0_EnemyModel* pEnemy = (sBTL_X0_EnemyModel*)pThis->m74_enemyModelTask;
    pEnemy->m30A_commandIndex = command;
    battleEngine_SetBattleMode(eBattleModes::m9);
}

// 06060e20
static void BTL_X0_formation0_state1(sBTL_X0_FormationTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    s8 subState = pThis->m7C_subState;

    if (subState == 0)
    {
        if (battleEngine_isBattleIntroFinished())
        {
            pThis->m68_counter = 0;
            pThis->m6A_counter2 = 0;
            battleEngine_SetBattleMode(eBattleModes::m2);
            pThis->m7C_subState++;
        }
    }
    else if (subState == 1)
    {
        if (pEngine->m188_flags.m2000)
        {
            pThis->m78_introEffectTask = formationCreateIntroEffect((sFormationTaskBase*)pThis, g_BTL_X0->getSaturnPtr(0x060b7908));
            pThis->m7C_subState++;
        }
    }
    else if (subState == 2)
    {
        if (pThis->m78_introEffectTask == nullptr ||
            (pThis->m78_introEffectTask->getTask()->m14_flags & TASK_FLAGS_FINISHED))
        {
            pEngine->m188_flags.m100_attackAnimationFinished = 1;
            pThis->m7C_subState++;
        }
    }
    else if (subState == 3)
    {
        if (battleEngine_isBattleIntroFinished())
        {
            BTL_X0_setEnemyCommand(pThis, 10);
            pThis->m7C_subState++;
        }
    }
    else if (subState == 4)
    {
        if (battleEngine_isBattleIntroFinished())
        {
            pThis->m68_counter = 0;
            pThis->m6A_counter2 = 0;
            battleEngine_SetBattleMode(eBattleModes::m2);
            pThis->m7C_subState++;
        }
    }
    else if (subState == 5)
    {
        if (pEngine->m188_flags.m2000)
        {
            pThis->m78_introEffectTask = formationCreateIntroEffect((sFormationTaskBase*)pThis, g_BTL_X0->getSaturnPtr(0x060b7944));
            pThis->m7C_subState++;
        }
    }
    else if (subState == 6)
    {
        if (pThis->m78_introEffectTask == nullptr ||
            (pThis->m78_introEffectTask->getTask()->m14_flags & TASK_FLAGS_FINISHED))
        {
            pEngine->m188_flags.m100_attackAnimationFinished = 1;
            pThis->m7C_subState++;
        }
    }
    else if (subState == 7)
    {
        pThis->m70_state = 2;
        pThis->m7C_subState = 0;
    }
}

// 06060864
static s32 BTL_X0_isEnemyCommandIdle(sBTL_X0_FormationTask* pThis)
{
    sBTL_X0_EnemyModel* pEnemy = (sBTL_X0_EnemyModel*)pThis->m74_enemyModelTask;
    return (pEnemy->m30A_commandIndex == 0) ? 1 : 0;
}

// 06060fbe
static void BTL_X0_formation0_state3(sBTL_X0_FormationTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    s8 subState = pThis->m7C_subState;

    if (subState == 0)
    {
        if (pEngine->m188_flags.m2000)
        {
            pThis->m78_introEffectTask = formationCreateIntroEffect((sFormationTaskBase*)pThis, g_BTL_X0->getSaturnPtr(0x060b7974));
            pThis->m7C_subState++;
        }
    }
    else if (subState == 1)
    {
        if (pThis->m78_introEffectTask == nullptr ||
            (pThis->m78_introEffectTask->getTask()->m14_flags & TASK_FLAGS_FINISHED))
        {
            pThis->m7C_subState++;
            pEngine->m188_flags.m100_attackAnimationFinished = 1;
            battleEngine_resetBattleCameraPreset();
            battleEngine_restoreCameraAfterEnemyAttack();

            u8* pGrid = (u8*)gBattleManager->m10_battleOverlay->m8_gridTask;
            memcpy(pGrid + 0xE4, pGrid + 0x134, 0x18);
            memset(pGrid + 0x108, 0, 0x18);
        }
    }
    else if (subState == 2)
    {
        if (battleEngine_isBattleIntroFinished())
        {
            BTL_X0_setEnemyCommand(pThis, 0x16);
            pThis->m7C_subState++;
        }
    }
    else if (subState == 3)
    {
        if (BTL_X0_isEnemyCommandIdle(pThis))
        {
            pThis->getTask()->markFinished();
        }
    }
}

// 06060ac0
static void BTL_X0_formation0_Update(sBTL_X0_FormationTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    pEngine->m3B2_numBattleFormationRunning++;

    s8 state = pThis->m70_state;
    if (state == 0)
    {
        pThis->m70_state++;
        pEngine->m3CC->m2 = 0x5A;
        return;
    }
    if (state == 1)
    {
        BTL_X0_formation0_state1(pThis);
        return;
    }
    if (state != 2)
    {
        if (state == 3)
        {
            BTL_X0_formation0_state3(pThis);
        }
        return;
    }

    // State 2: main battle phase
    if (battleEngine_isBattleIntroFinished() == 0)
        return;

    sBTL_X0_EnemyModel* pEnemy = (sBTL_X0_EnemyModel*)pThis->m74_enemyModelTask;
    if (pEnemy->mEE_hpCurrent < 1)
    {
        BTL_X0_setEnemyCommand(pThis, 0x17);
        pThis->m70_state++;
        pThis->m68_counter = 0;
        pThis->m7C_subState = 0;
        return;
    }

    if (battleEngine_isPlayerTurnActive() != 0)
        return;

    // Debug keyboard inputs
    if (readKeyboardToggle(0xB2))
    {
        BTL_X0_setEnemyCommand(pThis, 0x11);
    }
    else if (readKeyboardToggle(0xAC))
    {
        BTL_X0_setEnemyCommand(pThis, 7);
    }
    else if (readKeyboardToggle(0xC3))
    {
        BTL_X0_setEnemyCommand(pThis, 9);
    }
    else if (readKeyboardToggle(0xB3))
    {
        BTL_X0_setEnemyCommand(pThis, 5);
    }
    else if (readKeyboardToggle(0x9D))
    {
        BTL_X0_setEnemyCommandOnly(pThis, 2);
    }
    else if (readKeyboardToggle(0xCD))
    {
        BTL_X0_setEnemyCommand(pThis, 8);
    }

    if (pEngine->m3CC->m8 == 0)
        return;

    pEngine->m3CC->m8 = 0;
    pEngine->m3CC->m0 = 0;

    s8 act = pThis->m7D_act0;
    if (act == 1)
    {
        if (pEngine->m3CC->m4 < 2)
        {
            BTL_X0_setEnemyCommand(pThis, 8);
            return;
        }
        BTL_X0_setEnemyCommand(pThis, 9);
        pThis->m7D_act0 = 0;
        pEngine->m3CC->m4 = 0;
        pEngine->m3CC->m2 = 0x5A;
        return;
    }
    if (act != 0)
        return;

    // AI decision based on dragon quadrant
    s8 quadrant = pEngine->m22C_dragonCurrentQuadrant;
    if (quadrant == 0)
    {
        BTL_X0_setEnemyCommand(pThis, 2);
    }
    else if (quadrant == 2)
    {
        BTL_X0_setEnemyCommand(pThis, 5);
    }
    else if (quadrant == 1 || quadrant == 3)
    {
        u32 rng = randomNumber() & 0xFFFF;
        if (rng < 0x9999)
        {
            BTL_X0_setEnemyCommandOnly(pThis, 3);
            s8 moveDir = (quadrant == 3) ? 1 : 2;
            battleEngine_initiateEnemyMoveDragon(moveDir, 0x1E);
        }
        else if (rng < 0xCCCC)
        {
            BTL_X0_setEnemyCommandOnly(pThis, 4);
            battleEngine_initiateEnemyMoveDragon(3, 0x2D);
        }
        else
        {
            BTL_X0_setEnemyCommand(pThis, 5);
        }
    }

    if (pEngine->m3CC->m4 > 5)
    {
        pThis->m7D_act0 = 1;
        pEngine->m3CC->m4 = 0;
        pEngine->m3CC->m2 = 0x5A;
    }
}

// 06060980
static void BTL_X0_formation_DebugDraw(sBTL_X0_FormationTask* pThis)
{
    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x19] != 0)
    {
        vdp2PrintStatus.m14_oldPalette = vdp2PrintStatus.m10_palette;
        vdp2PrintStatus.m10_palette = 0xC000;
        vdp2DebugPrintSetPosition(0x1E, 10);
        vdp2PrintfSmallFont("phase[%d:%d]", (int)pThis->m70_state, (int)pThis->m7C_subState);
        vdp2DebugPrintSetPosition(0x1E, 0xB);
        vdp2PrintfSmallFont("act  [%d:%d]", (int)pThis->m7D_act0, (int)pThis->m7E_act1);
        vdp2DebugPrintSetPosition(0x1E, 0xD);
        vdp2PrintfSmallFont("mode1[%d:%d]", (int)pThis->m80_mode1a, (int)pThis->m81_mode1b);
        vdp2DebugPrintSetPosition(0x1E, 0xE);
        vdp2PrintfSmallFont("mode2[%d:%d]", (int)pThis->m82_mode2a, (int)pThis->m83_mode2b);
        vdp2DebugPrintSetPosition(0x1E, 0xF);
        vdp2PrintfSmallFont("mode3[%d:%d]", (int)pThis->m84_mode3a, (int)pThis->m85_mode3b);
        vdp2DebugPrintSetPosition(0x1E, 0x10);
        vdp2PrintfSmallFont("mode4[%d:%d]", (int)pThis->m86_mode4a, (int)pThis->m87_mode4b);
        vdp2PrintStatus.m10_palette = vdp2PrintStatus.m14_oldPalette;
    }
}

// 06060d8a (thunk to 06060980)
static void BTL_X0_formation0_Draw(sBTL_X0_FormationTask* pThis)
{
    BTL_X0_formation_DebugDraw(pThis);
}

// 06060d90
static void BTL_X0_formation0_Delete(sBTL_X0_FormationTask* pThis)
{
}

static s32 BTL_X0_isAttackSubPartActive(sBTL_X0_FormationTask* pThis, s8 partIndex);
static s8 BTL_X0_countFrontBackParts(sBTL_X0_FormationTask* pThis);
static s8 BTL_X0_countSideParts(sBTL_X0_FormationTask* pThis);
static s16 BTL_X0_getDamageTaken(sBTL_X0_FormationTask* pThis);
static void BTL_X0_updatePartFlags(sBTL_X0_FormationTask* pThis);
void battleEngine_displayAttackName(int p1, int p2, int p3);

// 0606204c
static s32 BTL_X0_isDragonStatusAffected()
{
    return (gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 0x1F4780) != 0 ? 1 : 0;
}

// 06061114
static void BTL_X0_formation1_Update(sBTL_X0_FormationTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    pEngine->m3B2_numBattleFormationRunning++;

    s8 state = pThis->m70_state;
    if (state == 0)
    {
        pThis->m70_state = 1;
        pEngine->m3CC->m2 = 0x4E;
        return;
    }
    if (state == 1)
    {
        if (battleEngine_isBattleIntroFinished())
        {
            BTL_X0_setEnemyCommand(pThis, 0x13);
            pThis->m70_state = 2;
        }
        return;
    }
    if (state == 3)
    {
        if (BTL_X0_isEnemyCommandIdle(pThis))
        {
            pThis->getTask()->markFinished();
        }
        return;
    }
    if (state != 2)
        return;

    // State 2: main battle phase
    if (pThis->m7D_act0 == 1)
    {
        BTL_X0_updatePartFlags(pThis);
    }

    if (battleEngine_isBattleIntroFinished() == 0)
        return;

    sBTL_X0_EnemyModel* pEnemy = (sBTL_X0_EnemyModel*)pThis->m74_enemyModelTask;

    if (pEnemy->mEE_hpCurrent < 1)
    {
        BTL_X0_setEnemyCommand(pThis, 0x16);
        pThis->m70_state++;
        pThis->m68_counter = 0;
        return;
    }

    if (battleEngine_isPlayerTurnActive() != 0)
        return;

    // Debug keyboard inputs
    if (readKeyboardToggle(0xB2))
        BTL_X0_setEnemyCommand(pThis, 0x11);
    else if (readKeyboardToggle(0xAC))
        BTL_X0_setEnemyCommand(pThis, 7);
    else if (readKeyboardToggle(0xC3))
        BTL_X0_setEnemyCommand(pThis, 9);
    else if (readKeyboardToggle(0xB3))
        BTL_X0_setEnemyCommand(pThis, 5);
    else if (readKeyboardToggle(0x9D))
        BTL_X0_setEnemyCommand(pThis, 2);
    else if (readKeyboardToggle(0xAD))
        BTL_X0_setEnemyCommand(pThis, 0x13);
    else if (readKeyboardToggle(0x95))
        BTL_X0_setEnemyCommand(pThis, 0x15);
    else if (readKeyboardToggle(0xA1))
        BTL_X0_setEnemyCommand(pThis, 0x12);
    else if (readKeyboardToggle(0xCD))
        BTL_X0_setEnemyCommand(pThis, 8);

    if (pEngine->m3CC->m8 == 0)
        return;

    pEngine->m3CC->m8 = 0;
    pEngine->m3CC->m0 = 0;

    // HP ratio check for phase transitions
    if (pThis->m7D_act0 == 0)
    {
        s32 hpThreshold = FP_Div(3, fixedPoint(pEnemy->mEC_hpMax)).getInteger();
        if (hpThreshold < (s32)pEnemy->mEE_hpCurrent)
        {
            if (pEngine->m3CC->m4 < 6)
            {
                s8 frontBack = BTL_X0_countFrontBackParts(pThis);
                s8 sides = BTL_X0_countSideParts(pThis);
                if ((s32)frontBack + (s32)sides == 0)
                {
                    pThis->m7D_act0 = 1;
                    pThis->m7E_act1 = 0;
                    pEngine->m3CC->m4 = 0;
                    pEngine->m3CC->m2 = 0x4E;
                }
            }
            else
            {
                pThis->m7D_act0 = 2;
                pThis->m7E_act1 = 0;
                pEngine->m3CC->m4 = 0;
                pEngine->m3CC->m2 = 0x4E;
            }
        }
        else
        {
            pThis->m7D_act0 = 3;
            pThis->m7E_act1 = 0;
            pEngine->m3CC->m4 = 0;
            pEngine->m3CC->m2 = 0x5A;
        }
    }
    else if (pThis->m7D_act0 == 1)
    {
        s32 hpThreshold = FP_Div(3, fixedPoint(pEnemy->mEC_hpMax)).getInteger();
        if ((s32)pEnemy->mEE_hpCurrent <= hpThreshold)
        {
            pThis->m7D_act0 = 3;
            pThis->m7E_act1 = 1;
            pEngine->m3CC->m4 = 0;
            pEngine->m3CC->m2 = 0x5A;
        }
        else if (pEngine->m3CC->m4 > 5)
        {
            pThis->m7D_act0 = 2;
            pThis->m7E_act1 = 1;
            pEngine->m3CC->m4 = 0;
            pEngine->m3CC->m2 = 0x4E;
        }
    }

    // AI action based on m7D_act0
    s8 act = pThis->m7D_act0;
    if (act == 0)
    {
        if (pThis->m80_mode1a == 1)
        {
            s16 dmg = BTL_X0_getDamageTaken(pThis);
            if (dmg > 0)
            {
                s8 sides = BTL_X0_countSideParts(pThis);
                if (sides > 0)
                {
                    BTL_X0_setEnemyCommand(pThis, 0x12);
                    battleEngine_displayAttackName(0x19, 0x1E, 0);
                }
            }
            pThis->m80_mode1a = 0;
            return;
        }
        pThis->m80_mode1a = 1;

        s8 quadrant = pEngine->m22C_dragonCurrentQuadrant;
        if (quadrant == 0)
        {
            BTL_X0_setEnemyCommand(pThis, 2);
        }
        else if (quadrant == 2)
        {
            BTL_X0_setEnemyCommand(pThis, 5);
        }
        else if (quadrant == 1 || quadrant == 3)
        {
            u32 rng = randomNumber() & 0xFFFF;
            if (rng < 0x4000)
            {
                BTL_X0_setEnemyCommandOnly(pThis, 3);
                s8 moveDir = (quadrant == 3) ? 1 : 2;
                battleEngine_initiateEnemyMoveDragon(moveDir, 0x1E);
                return;
            }
            if (rng < 0x6666)
            {
                BTL_X0_setEnemyCommandOnly(pThis, 4);
                battleEngine_initiateEnemyMoveDragon(3, 0x2D);
                return;
            }
            if (quadrant == 1 && BTL_X0_isAttackSubPartActive(pThis, 3))
            {
                BTL_X0_setEnemyCommand(pThis, 0x11);
                return;
            }
            if (quadrant == 3 && BTL_X0_isAttackSubPartActive(pThis, 0))
            {
                BTL_X0_setEnemyCommand(pThis, 0x11);
                return;
            }
            BTL_X0_setEnemyCommand(pThis, 5);
        }
        return;
    }
    if (act == 1)
    {
        s8 quadrant = pEngine->m22C_dragonCurrentQuadrant;
        if (quadrant == 0)
        {
            BTL_X0_setEnemyCommand(pThis, 2);
        }
        else if (quadrant == 2)
        {
            BTL_X0_setEnemyCommand(pThis, 5);
        }
        else if (quadrant == 1 || quadrant == 3)
        {
            u32 rng = randomNumber() & 0xFFFF;
            if (rng < 0x9999)
            {
                BTL_X0_setEnemyCommandOnly(pThis, 3);
                s8 moveDir = (quadrant == 3) ? 1 : 2;
                battleEngine_initiateEnemyMoveDragon(moveDir, 0x1E);
                return;
            }
            if (rng < 0xCCCC)
            {
                BTL_X0_setEnemyCommandOnly(pThis, 4);
                battleEngine_initiateEnemyMoveDragon(3, 0x2D);
                return;
            }
            if (quadrant == 1 && BTL_X0_isAttackSubPartActive(pThis, 3))
            {
                BTL_X0_setEnemyCommand(pThis, 0x11);
                return;
            }
            if (quadrant == 3 && BTL_X0_isAttackSubPartActive(pThis, 0))
            {
                BTL_X0_setEnemyCommand(pThis, 0x11);
                return;
            }
            BTL_X0_setEnemyCommand(pThis, 5);
        }
        return;
    }
    if (act == 2)
    {
        if (pEngine->m3CC->m4 > 0)
        {
            s8 fb = BTL_X0_countFrontBackParts(pThis);
            if (fb == 2)
            {
                BTL_X0_setEnemyCommand(pThis, 0x15);
                battleEngine_displayAttackName(0x33, 0x1E, 0);
            }
            else
            {
                BTL_X0_setEnemyCommand(pThis, 9);
            }
            pThis->m7D_act0 = 1;
            pThis->m84_mode3a++;
            pEngine->m3CC->m4 = 0;
            pEngine->m3CC->m2 = 0x42;
            return;
        }
        s8 fb = BTL_X0_countFrontBackParts(pThis);
        if (fb == 2)
        {
            BTL_X0_setEnemyCommandOnly(pThis, 0x14);
            return;
        }
        BTL_X0_setEnemyCommand(pThis, 8);
        pEngine->m3CC->m2 = 0x2D;
        return;
    }
    if (act == 3)
    {
        s8 subMode = pThis->m86_mode4a;
        if (subMode == 0)
        {
            s8 quadrant = pEngine->m22C_dragonCurrentQuadrant;
            if (quadrant == 1 && BTL_X0_isAttackSubPartActive(pThis, 3))
                BTL_X0_setEnemyCommand(pThis, 0x11);
            else if (quadrant == 3 && BTL_X0_isAttackSubPartActive(pThis, 0))
                BTL_X0_setEnemyCommand(pThis, 0x11);
            else
                BTL_X0_setEnemyCommand(pThis, 5);
            pEngine->m3CC->m2 = 0x2D;
            pThis->m86_mode4a++;
        }
        else if (subMode == 1)
        {
            BTL_X0_setEnemyCommandOnly(pThis, 6);
            pEngine->m3CC->m2 = 0x42;
            pThis->m86_mode4a++;
        }
        else if (subMode == 2)
        {
            BTL_X0_setEnemyCommand(pThis, 7);
            pThis->m86_mode4a++;
        }
        else if (subMode == 3)
        {
            BTL_X0_setEnemyCommand(pThis, 8);
            pEngine->m3CC->m2 = 0x2D;
            pThis->m86_mode4a++;
        }
        else if (subMode == 4)
        {
            BTL_X0_setEnemyCommand(pThis, 9);
            pThis->m86_mode4a = 0;
            pEngine->m3CC->m2 = 0x42;
        }
        return;
    }

    // act >= 4: counter check
    if (pThis->m84_mode3a > 2)
    {
        pThis->m84_mode3a = 0;
        s8 fb = BTL_X0_countFrontBackParts(pThis);
        s8 sides = BTL_X0_countSideParts(pThis);
        if ((s32)fb + (s32)sides < 4)
        {
            BTL_X0_setEnemyCommand(pThis, 0x13);
            return;
        }
    }

    if (pThis->m82_mode2a == 1)
    {
        s16 dmg = BTL_X0_getDamageTaken(pThis);
        if (dmg > 0)
        {
            s8 sides = BTL_X0_countSideParts(pThis);
            if (sides > 0)
            {
                BTL_X0_setEnemyCommand(pThis, 0x12);
            }
        }
        pThis->m82_mode2a = 0;
        return;
    }
    pThis->m82_mode2a = 1;

    s8 quadrant = pEngine->m22C_dragonCurrentQuadrant;
    if (quadrant == 0)
    {
        BTL_X0_setEnemyCommand(pThis, 2);
    }
    else if (quadrant == 2)
    {
        BTL_X0_setEnemyCommand(pThis, 5);
    }
    else if (quadrant == 1 || quadrant == 3)
    {
        u32 rng = randomNumber() & 0xFFFF;
        if (rng < 0x8000)
        {
            BTL_X0_setEnemyCommandOnly(pThis, 3);
            s8 moveDir = (quadrant == 3) ? 1 : 2;
            battleEngine_initiateEnemyMoveDragon(moveDir, 0x1E);
            return;
        }
        if (rng < 0xA666)
        {
            BTL_X0_setEnemyCommandOnly(pThis, 4);
            battleEngine_initiateEnemyMoveDragon(3, 0x2D);
            return;
        }
        if (quadrant == 1 && BTL_X0_isAttackSubPartActive(pThis, 3))
        {
            BTL_X0_setEnemyCommand(pThis, 0x11);
            return;
        }
        if (quadrant == 3 && BTL_X0_isAttackSubPartActive(pThis, 0))
        {
            BTL_X0_setEnemyCommand(pThis, 0x11);
            return;
        }
        BTL_X0_setEnemyCommand(pThis, 5);
    }
}

// 060617c6
static void BTL_X0_formation1_Draw(sBTL_X0_FormationTask* pThis)
{
    BTL_X0_formation_DebugDraw(pThis);
}

// 060617cc
static void BTL_X0_formation1_Delete(sBTL_X0_FormationTask* pThis)
{
}

s32 playBattleSoundEffect(s32 effectIndex);

// 06060904
static void BTL_X0_updatePartFlags(sBTL_X0_FormationTask* pThis);

// 0606087a
static s32 BTL_X0_isAttackSubPartActive(sBTL_X0_FormationTask* pThis, s8 partIndex);

// 0606087a
static s32 BTL_X0_isAttackSubPartActive(sBTL_X0_FormationTask* pThis, s8 partIndex)
{
    sBTL_X0_EnemyModel* pEnemy = (sBTL_X0_EnemyModel*)pThis->m74_enemyModelTask;
    if (pEnemy->m314_attackSubTask != nullptr)
    {
        s32* attackData = (s32*)((u8*)pEnemy->m314_attackSubTask + 0x70);
        if (attackData[partIndex] != 0)
            return 1;
    }
    return 0;
}

// 0606089c
static s8 BTL_X0_countFrontBackParts(sBTL_X0_FormationTask* pThis)
{
    s8 count = (BTL_X0_isAttackSubPartActive(pThis, 0) != 0) ? 1 : 0;
    if (BTL_X0_isAttackSubPartActive(pThis, 3) != 0)
        count++;
    return count;
}

// 060608c8
static s8 BTL_X0_countSideParts(sBTL_X0_FormationTask* pThis)
{
    s8 count = (BTL_X0_isAttackSubPartActive(pThis, 1) != 0) ? 1 : 0;
    if (BTL_X0_isAttackSubPartActive(pThis, 2) != 0)
        count++;
    return count;
}

// 060608f4
static s16 BTL_X0_getDamageTaken(sBTL_X0_FormationTask* pThis)
{
    sBTL_X0_EnemyModel* pEnemy = (sBTL_X0_EnemyModel*)pThis->m74_enemyModelTask;
    return pEnemy->mEC_hpMax - pEnemy->mEE_hpCurrent;
}

// 06060904
static void BTL_X0_updatePartFlags(sBTL_X0_FormationTask* pThis)
{
    sBTL_X0_EnemyModel* pEnemy = (sBTL_X0_EnemyModel*)pThis->m74_enemyModelTask;
    if (pEnemy->m314_attackSubTask != nullptr)
    {
        if (BTL_X0_isAttackSubPartActive(pThis, 3) == 0)
            pEnemy->m304_state &= ~2;
        else
            pEnemy->m304_state |= 2;

        if (BTL_X0_isAttackSubPartActive(pThis, 0) == 0)
            pEnemy->m304_state &= ~8;
        else
            pEnemy->m304_state |= 8;
    }
}

// 060620a0
static void BTL_X0_formation2_state1(sBTL_X0_FormationTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    s8 subState = pThis->m7C_subState;

    if (subState == 0)
    {
        if (battleEngine_isBattleIntroFinished())
        {
            pThis->m68_counter = 0;
            pThis->m6A_counter2 = 0;
            battleEngine_SetBattleMode(eBattleModes::m2);
            pThis->m7C_subState++;
        }
    }
    else if (subState == 1)
    {
        if (pEngine->m188_flags.m2000)
        {
            pThis->m78_introEffectTask = formationCreateIntroEffect((sFormationTaskBase*)pThis, g_BTL_X0->getSaturnPtr(0x060b7a24));
            pThis->m7C_subState++;
        }
    }
    else if (subState == 2)
    {
        if (pThis->m78_introEffectTask == nullptr ||
            (pThis->m78_introEffectTask->getTask()->m14_flags & TASK_FLAGS_FINISHED))
        {
            pEngine->m188_flags.m100_attackAnimationFinished = 1;
            pThis->m7C_subState++;
        }
    }
    else if (subState == 3)
    {
        if (battleEngine_isBattleIntroFinished())
        {
            BTL_X0_setEnemyCommand(pThis, 10);
            pThis->m7C_subState++;
        }
    }
    else if (subState == 4)
    {
        if (battleEngine_isBattleIntroFinished())
        {
            pThis->m68_counter = 0;
            pThis->m6A_counter2 = 0;
            battleEngine_SetBattleMode(eBattleModes::m2);
            pThis->m7C_subState++;
        }
    }
    else if (subState == 5)
    {
        if (pEngine->m188_flags.m2000)
        {
            pThis->m78_introEffectTask = formationCreateIntroEffect((sFormationTaskBase*)pThis, g_BTL_X0->getSaturnPtr(0x060b7a6c));
            pThis->m7C_subState++;
        }
    }
    else if (subState == 6)
    {
        if (pThis->m78_introEffectTask == nullptr ||
            (pThis->m78_introEffectTask->getTask()->m14_flags & TASK_FLAGS_FINISHED))
        {
            pEngine->m188_flags.m100_attackAnimationFinished = 1;
            pThis->m7C_subState++;
            gBattleManager->m10_battleOverlay->m3 = 1;
            playPCM(pThis, 100);
        }
    }
    else if (subState == 7)
    {
        pThis->m70_state = 2;
        pThis->m7C_subState = 0;
    }
}

// 06062254
static void BTL_X0_formation2_state3(sBTL_X0_FormationTask* pThis)
{
    s8 subState = pThis->m7C_subState;

    if (subState == 0)
    {
        if (BTL_X0_isEnemyCommandIdle(pThis) && battleEngine_isBattleIntroFinished())
        {
            playBattleSoundEffect(0x6C);
            if (pThis->m74_enemyModelTask != nullptr)
            {
                pThis->m74_enemyModelTask->getTask()->markFinished();
            }
            decreaseNPCRefCount(8);
            pThis->m7C_subState++;
            pThis->m68_counter = 0x5A;
            pThis->m7F_variantIndex = 3;
            allocateNPC(pThis, 10);
        }
    }
    else if (subState == 1)
    {
        u8* pGrid = (u8*)gBattleManager->m10_battleOverlay->m8_gridTask;
        *(s32*)(pGrid + 0x1C0) += 0x5B05B;
        pThis->m68_counter--;
        if ((s16)pThis->m68_counter < 0)
        {
            *(s32*)(pGrid + 0x1C0) = 0;
            if (isDataLoaded(6))
            {
                pThis->m74_enemyModelTask = BTL_X0_createEnemyModels((s_workArea*)pThis, 3);
                pThis->m70_state = 4;
                pThis->m7C_subState = 0;
                sBTL_X0_EnemyModel* pEnemy = (sBTL_X0_EnemyModel*)pThis->m74_enemyModelTask;
                if (pEnemy)
                {
                    pEnemy->m7C_position.m4_Y = (s32)0xFFF9C000;
                    pEnemy->m30A_commandIndex = 0x19;
                }
                battleEngine_SetBattleMode(eBattleModes::m9);
            }
        }
    }
}

// 0606184c
static void BTL_X0_formation2_Update(sBTL_X0_FormationTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    pEngine->m3B2_numBattleFormationRunning++;

    s8 state = pThis->m70_state;
    if (state == 0)
    {
        pThis->m70_state = 1;
        pEngine->m3CC->m2 = 0x5A;
        return;
    }
    if (state == 1)
    {
        BTL_X0_formation2_state1(pThis);
        return;
    }
    if (state == 3)
    {
        BTL_X0_formation2_state3(pThis);
        return;
    }
    if (state == 4)
    {
        if (battleEngine_isBattleIntroFinished())
        {
            BTL_X0_setEnemyCommand(pThis, 0x13);
            pThis->m70_state = 2;
        }
        return;
    }
    if (state == 5)
    {
        if (BTL_X0_isEnemyCommandIdle(pThis))
        {
            pThis->m68_counter++;
            if ((s16)pThis->m68_counter >= 0x1F)
            {
                battleEngine_resetBattleCameraPreset();
                battleEngine_restoreCameraAfterEnemyAttack();
                pEngine->m188_flags.m100_attackAnimationFinished = 1;
                pThis->getTask()->markFinished();
            }
        }
        return;
    }
    if (state != 2)
        return;

    // State 2: main battle phase
    if (pThis->m7D_act0 == 1)
    {
        BTL_X0_updatePartFlags(pThis);
    }

    if (battleEngine_isBattleIntroFinished() == 0)
        return;

    sBTL_X0_EnemyModel* pEnemy = (sBTL_X0_EnemyModel*)pThis->m74_enemyModelTask;

    if (pThis->m7D_act0 == 0)
    {
        if (pEnemy->mEE_hpCurrent < 1)
        {
            pThis->m7D_act0 = 1;
            pEngine->m3CC->m4 = 0;
            pEngine->m3CC->m2 = 0x42;
            pThis->m70_state = 3;
            pThis->m7C_subState = 0;
            pThis->m68_counter = 0;
            BTL_X0_setEnemyCommand(pThis, 0x18);
            return;
        }
    }
    else if (pEnemy->mEE_hpCurrent < 1)
    {
        pThis->m70_state = 5;
        pThis->m68_counter = 0;
        BTL_X0_setEnemyCommand(pThis, 0x1A);
        return;
    }

    if (battleEngine_isPlayerTurnActive() != 0)
        return;

    // Debug keyboard handling
    if (readKeyboardToggle(0xB2))
        BTL_X0_setEnemyCommand(pThis, 0x11);
    else if (readKeyboardToggle(0xAC))
        BTL_X0_setEnemyCommand(pThis, 7);
    else if (readKeyboardToggle(0xC3))
        BTL_X0_setEnemyCommand(pThis, 9);
    else if (readKeyboardToggle(0xB3))
        BTL_X0_setEnemyCommand(pThis, 5);
    else if (readKeyboardToggle(0x9D))
        BTL_X0_setEnemyCommand(pThis, 2);
    else if (readKeyboardToggle(0xBC))
        BTL_X0_setEnemyCommand(pThis, 0xC);
    else if (readKeyboardToggle(0xAD))
        BTL_X0_setEnemyCommand(pThis, 0x10);

    if (pEngine->m3CC->m8 == 0)
        return;

    pEngine->m3CC->m8 = 0;
    pEngine->m3CC->m0 = 0;

    // HP ratio phase transition for m7D_act0 == 1
    if (pThis->m7D_act0 == 1)
    {
        s32 hpThreshold = FP_Div(3, fixedPoint(pEnemy->mEC_hpMax)).getInteger();
        if (hpThreshold < (s32)pEnemy->mEE_hpCurrent)
        {
            if (pEngine->m3CC->m4 > 5)
            {
                pThis->m7D_act0 = 2;
                pThis->m7E_act1 = 1;
                pEngine->m3CC->m4 = 0;
                pEngine->m3CC->m2 = 0x42;
            }
        }
        else
        {
            pThis->m7D_act0 = 3;
            pThis->m7E_act1 = 1;
            pEngine->m3CC->m4 = 0;
            pEngine->m3CC->m2 = 0x42;
        }
    }

    // AI action based on m7D_act0
    s8 act = pThis->m7D_act0;
    if (act == 0)
    {
        s8 mode = pThis->m80_mode1a;
        if (mode == 0)
        {
            if (pEngine->m22C_dragonCurrentQuadrant == 2)
            {
                pEngine->m3CC->m8 = 0;
                pEngine->m3CC->m0 = 0;
                pEngine->m3CC->m2 = 0x2D;
                BTL_X0_setEnemyCommandOnly(pThis, 0xB);
                pThis->m80_mode1a = 2;
            }
        }
        else if (mode == 3 && (*(u32*)((u8*)pEnemy + 0x14) & 4) != 0)
        {
            pEngine->m3CC->m8 = 0;
            pEngine->m3CC->m0 = 0;
            pEngine->m3CC->m2 = 0xB4;
            BTL_X0_setEnemyCommand(pThis, 0xE);
            pThis->m80_mode1a = 5;
        }
    }
    else if (act == 2)
    {
        if (pEngine->m3CC->m4 > 0)
        {
            s8 fb = BTL_X0_countFrontBackParts(pThis);
            if (fb == 2)
            {
                BTL_X0_setEnemyCommand(pThis, 0x15);
                battleEngine_displayAttackName(0x35, 0x1E, 0);
            }
            else
            {
                BTL_X0_setEnemyCommand(pThis, 9);
            }
            pThis->m7D_act0 = 1;
            pThis->m84_mode3a++;
            pEngine->m3CC->m4 = 0;
            pEngine->m3CC->m2 = 0x42;
            return;
        }
        s8 fb = BTL_X0_countFrontBackParts(pThis);
        if (fb == 2)
        {
            BTL_X0_setEnemyCommandOnly(pThis, 0x14);
            return;
        }
        BTL_X0_setEnemyCommand(pThis, 8);
        pEngine->m3CC->m2 = 0x2D;
        return;
    }
    else if (act == 3)
    {
        s8 subMode = pThis->m86_mode4a;
        if (subMode == 0)
        {
            s8 q = pEngine->m22C_dragonCurrentQuadrant;
            if (q == 1 && BTL_X0_isAttackSubPartActive(pThis, 3))
                BTL_X0_setEnemyCommand(pThis, 0x11);
            else if (q == 3 && BTL_X0_isAttackSubPartActive(pThis, 0))
                BTL_X0_setEnemyCommand(pThis, 0x11);
            else
                BTL_X0_setEnemyCommand(pThis, 5);
            pEngine->m3CC->m2 = 0x2D;
            pThis->m86_mode4a++;
        }
        else if (subMode == 1)
        {
            BTL_X0_setEnemyCommandOnly(pThis, 6);
            pEngine->m3CC->m2 = 0x42;
            pThis->m86_mode4a++;
        }
        else if (subMode == 2)
        {
            BTL_X0_setEnemyCommand(pThis, 7);
            pThis->m86_mode4a++;
        }
        else if (subMode == 3)
        {
            BTL_X0_setEnemyCommand(pThis, 8);
            pEngine->m3CC->m2 = 0x2D;
            pThis->m86_mode4a++;
        }
        else if (subMode == 4)
        {
            BTL_X0_setEnemyCommand(pThis, 9);
            pThis->m86_mode4a = 0;
            pEngine->m3CC->m2 = 0x42;
        }
        return;
    }

    if (act != 0)
    {
        // m7D_act0 == 1: counter check and quadrant AI
        if (pThis->m84_mode3a > 2)
        {
            pThis->m84_mode3a = 0;
            s8 fb = BTL_X0_countFrontBackParts(pThis);
            s8 sides = BTL_X0_countSideParts(pThis);
            if ((s32)fb + (s32)sides < 4)
            {
                BTL_X0_setEnemyCommand(pThis, 0x13);
                return;
            }
        }
        if (pThis->m82_mode2a == 1)
        {
            s16 dmg = BTL_X0_getDamageTaken(pThis);
            if (dmg > 0)
            {
                s8 sides = BTL_X0_countSideParts(pThis);
                if (sides > 0)
                    BTL_X0_setEnemyCommand(pThis, 0x12);
            }
            pThis->m82_mode2a = 0;
            return;
        }
        pThis->m82_mode2a = 1;

        s8 quadrant = pEngine->m22C_dragonCurrentQuadrant;
        if (quadrant == 0)
            BTL_X0_setEnemyCommand(pThis, 2);
        else if (quadrant == 2)
            BTL_X0_setEnemyCommand(pThis, 5);
        else if (quadrant == 1 || quadrant == 3)
        {
            u32 rng = randomNumber() & 0xFFFF;
            if (rng < 0x8000)
            {
                BTL_X0_setEnemyCommandOnly(pThis, 3);
                battleEngine_initiateEnemyMoveDragon((quadrant == 3) ? 1 : 2, 0x1E);
                return;
            }
            if (rng < 0xA666)
            {
                BTL_X0_setEnemyCommandOnly(pThis, 4);
                battleEngine_initiateEnemyMoveDragon(3, 0x2D);
                return;
            }
            if (quadrant == 1 && BTL_X0_isAttackSubPartActive(pThis, 3))
            {
                BTL_X0_setEnemyCommand(pThis, 0x11);
                return;
            }
            if (quadrant == 3 && BTL_X0_isAttackSubPartActive(pThis, 0))
            {
                BTL_X0_setEnemyCommand(pThis, 0x11);
                return;
            }
            BTL_X0_setEnemyCommand(pThis, 5);
        }
        return;
    }

    // m7D_act0 == 0, m80_mode1a sub-states
    s8 mode = pThis->m80_mode1a;
    if (mode == 0)
    {
        if (BTL_X0_isDragonStatusAffected())
        {
            u32 rng = randomNumber() & 0xFFFF;
            BTL_X0_setEnemyCommand(pThis, (rng < 0x3333) ? 0x10 : 5);
            return;
        }
        BTL_X0_setEnemyCommand(pThis, 5);
    }
    else if (mode == 2)
    {
        BTL_X0_setEnemyCommand(pThis, 0xC);
        pThis->m80_mode1a = 3;
        pEngine->m3CC->m2 = 0x3C;
    }
    else if (mode == 3)
    {
        pThis->m80_mode1a = 0;
        *(u32*)((u8*)pEnemy + 0x14) &= ~2;
        pEnemy->m30F_flag3 = 1;
        BTL_X0_setEnemyCommandOnly(pThis, 0xD);
        pEngine->m3CC->m2 = 0x5A;
    }
    else if (mode == 5)
    {
        BTL_X0_setEnemyCommand(pThis, 0xF);
        pEngine->m3CC->m2 = 0x5A;
        pThis->m80_mode1a = 0;
    }
}

// 06061fd4
static void BTL_X0_formation2_Draw(sBTL_X0_FormationTask* pThis)
{
    BTL_X0_formation_DebugDraw(pThis);
}

// 06061fda
static void BTL_X0_formation2_Delete(sBTL_X0_FormationTask* pThis)
{
}


// 06060d94
void BTL_X0_createFormation0(s_workAreaCopy* pParent, u32 arg0)
{
    static const sBTL_X0_FormationTask::TypedTaskDefinition def = {
        nullptr,
        &BTL_X0_formation0_Update,
        &BTL_X0_formation0_Draw,
        &BTL_X0_formation0_Delete,
    };
    sBTL_X0_FormationTask* pThis = createSubTask<sBTL_X0_FormationTask>((s_workArea*)pParent, &def);
    if (pThis != nullptr)
    {
        pThis->m7F_variantIndex = 0;
        allocateNPC((s_workArea*)pParent, 6);
        pThis->m74_enemyModelTask = BTL_X0_createEnemyModels((s_workArea*)pThis, 0);
        pThis->m64_formationData = arg0;
        pThis->m18_subBattleParam = (s8)gBattleManager->m6_subBattleId;
        pThis->m70_state = 0;
        pThis->m7C_subState = 0;
        gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m2 = 0x5A;
    }
}

// 060617d0
void BTL_X0_createFormation1(s_workAreaCopy* pParent, u32 arg0)
{
    static const sBTL_X0_FormationTask::TypedTaskDefinition def = {
        nullptr,
        &BTL_X0_formation1_Update,
        &BTL_X0_formation1_Draw,
        &BTL_X0_formation1_Delete,
    };
    sBTL_X0_FormationTask* pThis = createSubTask<sBTL_X0_FormationTask>((s_workArea*)pParent, &def);
    if (pThis != nullptr)
    {
        pThis->m7F_variantIndex = 1;
        allocateNPC((s_workArea*)pParent, 6);
        pThis->m74_enemyModelTask = BTL_X0_createEnemyModels((s_workArea*)pThis, 1);
        pThis->m64_formationData = arg0;
        pThis->m18_subBattleParam = (s8)gBattleManager->m6_subBattleId;
        pThis->m70_state = 0;
        pThis->m7C_subState = 0;
    }
}

// 06061ff2
void BTL_X0_createFormation2(s_workAreaCopy* pParent, u32 arg0)
{
    static const sBTL_X0_FormationTask::TypedTaskDefinition def = {
        nullptr,
        &BTL_X0_formation2_Update,
        &BTL_X0_formation2_Draw,
        &BTL_X0_formation2_Delete,
    };
    sBTL_X0_FormationTask* pThis = createSubTask<sBTL_X0_FormationTask>((s_workArea*)pParent, &def);
    if (pThis != nullptr)
    {
        pThis->m7F_variantIndex = 2;
        allocateNPC((s_workArea*)pParent, 8);
        pThis->m74_enemyModelTask = BTL_X0_createEnemyModels2((s_workArea*)pThis);
        pThis->m64_formationData = arg0;
        pThis->m18_subBattleParam = (s8)gBattleManager->m6_subBattleId;
        pThis->m70_state = 0;
        pThis->m7C_subState = 0;
    }
}
