#include "PDS.h"
#include "battleFormationBase.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"
#include "battleEngineSub0.h"
#include "battleDebug.h"
#include "battleTextDisplay.h"

void battleEngine_displayAttackName(int param1, int param2, int param3);
void battleEngine_PlayAttackCamera(int param1);

// 0609b740
void formationCleanupEntityArray(sEntityGroup* pGroup)
{
    for (int i = 0; i < pGroup->mE_numEntities; i++)
    {
        p_workArea entity = pGroup->m0_entityArray[i];
        if (entity == nullptr || (entity->getTask()->m14_flags & TASK_FLAGS_FINISHED))
        {
            pGroup->m0_entityArray[i] = nullptr;
        }
    }
}

// 0609b72c
s32 formationAreAllEntitiesDead(sEntityGroup* pGroup)
{
    if (pGroup->mF_deadCount >= pGroup->mE_numEntities)
    {
        return 1;
    }
    return 0;
}

// 0609b6cc
s32 formationCheckEntityFlag(sEntityGroup* pGroup, u32 flagBit)
{
    u32 mask = 1 << (flagBit & 0xFF);
    for (int i = 0; i < pGroup->mE_numEntities; i++)
    {
        p_workArea entity = pGroup->m0_entityArray[i];
        if (entity == nullptr || (entity->getTask()->m14_flags & TASK_FLAGS_FINISHED))
        {
            pGroup->m0_entityArray[i] = nullptr;
        }
        else if ((entity->getTask()->m14_flags & mask) != 0)
        {
            return 1;
        }
    }
    return 0;
}

// 0609ae9c
void formationUpdatePosition(sFormationPositionBlock* pBlock, sVec3_FP* pBattleCenter)
{
    pBlock->m18_velocity.m0_X = pBlock->m30_target.m0_X - pBlock->m60_currentInterp.m0_X;
    pBlock->m18_velocity.m4_Y = pBlock->m30_target.m4_Y - pBlock->m60_currentInterp.m4_Y;
    pBlock->m18_velocity.m8_Z = pBlock->m30_target.m8_Z - pBlock->m60_currentInterp.m8_Z;

    pBlock->m18_velocity.m0_X = MTH_Mul(pBlock->m78_interpRate, pBlock->m18_velocity.m0_X);
    pBlock->m18_velocity.m4_Y = MTH_Mul(pBlock->m78_interpRate, pBlock->m18_velocity.m4_Y);
    pBlock->m18_velocity.m8_Z = MTH_Mul(pBlock->m78_interpRate, pBlock->m18_velocity.m8_Z);

    pBlock->m60_currentInterp += pBlock->m18_velocity;

    pBlock->m0_position.m0_X = pBattleCenter->m0_X + pBlock->m60_currentInterp.m0_X;
    pBlock->m0_position.m4_Y = pBattleCenter->m4_Y + pBlock->m60_currentInterp.m4_Y;
    pBlock->m0_position.m8_Z = pBattleCenter->m8_Z + pBlock->m60_currentInterp.m8_Z;
}

// 0609b2fc
void formationApplyAutoScroll(sFormationTaskBase* pThis)
{
    sVec3_FP& autoScroll = gBattleManager->m10_battleOverlay->m4_battleEngine->m1A0_battleAutoScrollDelta;
    pThis->mA8_position += autoScroll;
    pThis->mC0_position2 += autoScroll;
}

// 0609b60a
void formationTriggerAttack(sEntityGroup* pGroup, s8 attackType)
{
    u32 attackBit = 1 << ((u8)attackType);
    for (s32 i = 0; i < pGroup->mE_numEntities; i++)
    {
        p_workArea pEntity = pGroup->m0_entityArray[i];
        if (pEntity == nullptr || pEntity->getTask()->isFinished())
        {
            pGroup->m0_entityArray[i] = nullptr;
        }
        else
        {
            Unimplemented(); // entity->m14_attackFlags |= attackBit — needs common battle entity base struct
        }
    }
}

// 0609b65e
void formationTriggerAttackFollowup(sEntityGroup* pGroup, s8 attackType)
{
    u32 attackBit = 1 << ((u8)attackType);
    for (s32 i = 0; i < pGroup->mE_numEntities; i++)
    {
        p_workArea pEntity = pGroup->m0_entityArray[i];
        if (pEntity == nullptr || pEntity->getTask()->isFinished())
        {
            pGroup->m0_entityArray[i] = nullptr;
        }
        else
        {
            Unimplemented(); // entity->m14_attackFlags &= ~attackBit — needs common battle entity base struct
        }
    }
}

// 060698e6
// formationCreateIntroEffect is defined in battleFormationIntroEffect.cpp
// (shared across BTL_X0 / BTL_A7_2 etc.) — delegates to
// formationCreateIntroEffectImpl with flag = 0, matching every known call site.

// 0609b9a0
void formationCreateTransitionTask(sFormationTaskBase* pThis, sSaturnPtr data)
{
    Unimplemented();
}

// 060698ea
static void formationCreateAttackDisplay(sFormationTaskBase* pThis, s16 nameIndex, s16 param3, s32 param4)
{
    Unimplemented();
}

// 0609b776
void formationSubStateMachine(sFormationTaskBase* pThis, sEntityGroup* pGroup)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;

    switch (pThis->m28_state)
    {
    case 0:
    {
        s32 active = battleEngine_isPlayerTurnActive();
        if (active != 0)
            return;
        if (pEngine->m3CC->m8 == 1)
        {
            pThis->mD8_updateCallback(pThis);
        }
        break;
    }
    case 1:
    {
        s32 active = battleEngine_isPlayerTurnActive();
        if (active == 0)
        {
            battleEngine_restoreCameraDefault();
            pThis->m28_state = 0;
        }
        break;
    }
    case 2:
        if (pEngine->m188_flags.m2000)
        {
            if (pThis->mCD_displayMode == 1)
            {
                battleEngine_displayAttackName(pThis->mD0_attackNameIndex, pThis->mCE_attackDuration, 0);
            }
            else if (pThis->mCD_displayMode == 2)
            {
                formationCreateAttackDisplay(pThis, pThis->mD0_attackNameIndex, pThis->m20, pThis->mD4_attackFlag);
            }
            formationTriggerAttack(pGroup, pThis->mCC_attackType);
            pThis->m28_state++;
        }
        break;
    case 3:
        formationTriggerAttackFollowup(pGroup, pThis->mCC_attackType);
        pThis->m28_state++;
        // fallthrough
    case 4:
    {
        s16 count = pThis->m24_counter1;
        pThis->m24_counter1 = count + 1;
        if (count > pThis->mCE_attackDuration)
        {
            pThis->m24_counter1 = 0;
            pEngine->m188_flags.m100_attackAnimationFinished = 1;
            pThis->m28_state = 1;
        }
        break;
    }
    case 5:
        if (pEngine->m188_flags.m2000)
        {
            battleEngine_initiateEnemyMoveDragon(pThis->mDD_moveDirection, pThis->mDE_moveAgility);
            pThis->m28_state = 6;
        }
        break;
    case 6:
    {
        s32 finished = battleEngine_isBattleIntroFinished();
        if (finished != 0)
        {
            if (pThis->mDC_attackCamera == -1)
            {
                battleEngine_SetBattleMode(eBattleModes::m9);
            }
            else
            {
                battleEngine_PlayAttackCamera(pThis->mDC_attackCamera);
            }
            pThis->m28_state = 2;
        }
        break;
    }
    case 7:
    {
        s16 count = pThis->m26_counter2;
        pThis->m26_counter2 = count + 1;
        if (count > 0x3C)
        {
            pThis->m26_counter2 = 0;
            pThis->getTask()->markFinished();
        }
        break;
    }
    default:
        pThis->m28_state = 7;
        break;
    }

    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x1A] != 0)
    {
        vdp2PrintStatus.m14_oldPalette = vdp2PrintStatus.m10_palette;
        vdp2PrintStatus.m10_palette = 0xC000;
        vdp2DebugPrintSetPosition(1, 2);
        vdp2PrintfSmallFont("FRM ST:%1d", pThis->m28_state);
        vdp2DebugPrintSetPosition(1, 3);
        vdp2PrintfSmallFont("FRM CT:%3d", pThis->m26_counter2);
        vdp2PrintStatus.m10_palette = vdp2PrintStatus.m14_oldPalette;
    }
}
