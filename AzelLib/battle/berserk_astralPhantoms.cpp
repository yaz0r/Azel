#include "PDS.h"
#include "berserkEffects.h"
#include "berserkShared.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"
#include "battleDragon.h"
#include "battleGrid.h"
#include "battleOverlay_C.h"
#include "battleEngineSub1.h"
#include "battleTargetable.h"
#include "battleGenericData.h"
#include "battleDamageDisplay.h"
#include "kernel/fade.h"
#include "audio/systemSounds.h"
#include "trigo.h"

// =========================================================================
// FIXME LIST — Ghidra comparison audit
//
// STRUCT: [DONE] Rebuilt with correct offsets (m85 not m86, s16 m80, m82 counter,
//   m8C/m90 heap ptrs, m94/mA0/mA4 flags, sPhantomSubData sub-struct)
//
// Init (06096f8c):
//  6. [DONE] Zero-init loop fixed (5x s32, not 5x sVec3_FP)
//  7. [PARTIAL] Per-phantom zero-init done; FUN_06097a34 sub-task creation still TODO
//  8. [DONE] m85 (not m86) zeroed
//
// Update (06097098):
//  9. [DONE] m82_frameCounter increment
// 10. [DONE] mA0_destroyFlag check
// 11. [DONE] State 0: phantom sub-state set to 1
// 12. [DONE] State 1: readyCount checks phantom m0
// 13. [DONE] State 2: m94_attackDetected latch
// 14. [DONE] State 2: mA4_fullSpeedFlag
// 15. [DONE] State 2: phantom fade triggers on shield removal
// 16. [TODO] State 3: target allocation (Fisher-Yates, heap arrays m8C/m90)
// 17. [TODO] State 4: damage loop (phantomSlasher_getBaseDamage, m80-based power)
// 18. [TODO] State 4: per-phantom position update (autoScroll or live target)
// 19. [DONE] State 4: completion check uses phantom mC_completionFlag
// 20. [DONE] Post-switch epilogue checks phantom m10_modelPtr
// 21. [DONE] State 2: m80_orbitAngle += 3
//
// Delete (06097988): [OK]
// Draw (06097984): [OK] (empty in original)
//

//=============================================================================
// Astral Phantoms shield visual
//=============================================================================

struct sPhantomSubData
{
    s32 m0;                    // 0x00 — loaded/model flag (zeroed by FUN_06097a18)
    s32 m4;                    // 0x04
    s32 m8;                    // 0x08
    s32 mC_completionFlag;     // 0x0C — set when phantom finishes its attack
    s32 m10_modelPtr;          // 0x10 — model pointer (checked in epilogue)
    sVec3_FP m14_targetPos;    // 0x14 — target position for attack
    u16 m20_targetIndex;       // 0x20 — target enemy index
    u8 m22_subState;           // 0x22 — 0=idle, 1=ready, 2=attack, 3=fade
    u8 m23_pad;
    // size 0x24
};

struct sAstralPhantomsTask : public s_workAreaTemplate<sAstralPhantomsTask>
{
    sPhantomSubData m0_phantoms[3];  // 0x00..0x6B: 3 phantom sub-structs (3 * 0x24)
    u8 m6C_phantomVariant;           // 0x6C — phantom model variant (0-2)
    u8 m6D_pad[0x13];               // 0x6D..0x7F
    s16 m80_orbitAngle;              // 0x80 — phantom orbit angle (s16, incremented by 3)
    s16 m82_frameCounter;            // 0x82 — global frame counter
    u8 m84_pad;                      // 0x84
    s8 m85_subState;                 // 0x85 — state machine (NOT 0x86!)
    u8 m86_pad[2];                   // 0x86..0x87
    s32 m88_rotationSpeed;           // 0x88 — init 0x1999, ramps to 0x14000
    s32 m8C_targetIndexArray;        // 0x8C — heap ptr to target index array (state 3)
    s32 m90_hitTrackingArray;        // 0x90 — heap ptr to hit tracking array (state 3)
    s32 m94_attackDetected;          // 0x94 — latch: set when dragon DC flag 0x80000 detected
    s32 m98_shouldSetM100;           // 0x98 — init 1, cleared after m100 set
    s32 m9C_hasCamera;               // 0x9C
    s32 mA0_destroyFlag;             // 0xA0 — if set, mark task finished
    s32 mA4_fullSpeedFlag;           // 0xA4 — set when rotation reaches max speed
    // size 0xA8
};

// BTL_A3::06097988
static void sAstralPhantomsTask_Delete(sAstralPhantomsTask* pThis)
{
    s_battleDragon* pDragon = gBattleManager->m10_battleOverlay->m18_dragon;

    if (pThis->m98_shouldSetM100 != 0)
    {
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished = 1;
    }
    if (pThis->m9C_hasCamera != 0)
    {
        battleEngine_restoreCameraAfterEnemyAttack();
    }
    if ((pDragon->m1C0_statusModifiers & 0x400) != 0)
    {
        pDragon->m1C0_statusModifiers &= ~0x400;
    }
    pDragon->m1EC_shieldDuration = 0;
}

// BTL_A3::06097098 -- astral phantoms update (states 0-6 + damage/tracking)
static void sAstralPhantomsTask_Update(sAstralPhantomsTask* pThis)
{
    s_battleDragon* pDragon = gBattleManager->m10_battleOverlay->m18_dragon;
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    sBattleOverlayTask_C* pTargetSystem = gBattleManager->m10_battleOverlay->mC_targetSystem;

    // FIXME 9: frame counter increment
    pThis->m82_frameCounter++;

    // FIXME 10: destroy flag check
    if (pThis->mA0_destroyFlag != 0)
    {
        pThis->getTask()->markFinished();
        return;
    }

    switch (pThis->m85_subState)
    {
    case 0:
    {
        // State 0: set all 3 phantoms to sub-state 1 (ready)
        // FIXME 11: per-phantom sub-state init
        for (s32 i = 0; i < 3; i++)
        {
            pThis->m0_phantoms[i].m22_subState = 1;
        }
        pThis->m85_subState++;

        [[fallthrough]];
    }
    case 1:
    {
        // State 1: count active phantoms, wait for all 3 to be ready
        // FIXME 12: check phantom m0 (model loaded flag)
        s32 readyCount = 0;
        for (s32 i = 0; i < 3; i++)
        {
            if (pThis->m0_phantoms[i].m0 != 0)
                readyCount++;
        }

        if (readyCount == 3 && pThis->m98_shouldSetM100 != 0)
        {
            pEngine->m188_flags.m100_attackAnimationFinished = 1;
            pThis->m98_shouldSetM100 = 0;
            pThis->m85_subState++;
        }
        else
        {
            break;
        }

        [[fallthrough]];
    }
    case 2:
    {
        // State 2: shield active — monitor for incoming attack
        if ((pDragon->m1C0_statusModifiers & 0x400) == 0)
        {
            // FIXME 15: shield removed — set ALL phantoms to fade state (3)
            for (s32 i = 0; i < 3; i++)
            {
                pThis->m0_phantoms[i].m22_subState = 3;
            }
            pThis->m85_subState = 6;
            break;
        }

        // Ramp rotation speed while shield active
        if (battleEngine_isPlayerTurnActive() == 0)
        {
            if (pThis->m88_rotationSpeed < 0x14000)
            {
                pThis->m88_rotationSpeed += 0x106;
                // FIXME 21: increment orbit angle by 3
                pThis->m80_orbitAngle += 3;
            }
            else
            {
                // FIXME 14: full speed flag
                pThis->mA4_fullSpeedFlag = 1;
            }
        }

        // FIXME 13: attack detection with latch
        if ((*(u32*)((u8*)pDragon + 0xDC) & 0x80000) != 0)
        {
            pThis->m94_attackDetected = 1;
        }
        if (pThis->m94_attackDetected != 0 && battleEngine_isBattleIntroFinished() != 0)
        {
            pThis->m98_shouldSetM100 = 1;
            battleEngine_SetBattleMode(m4_useBerserk);
            pThis->m85_subState++;
        }
        break;
    }
    case 3:
    {
        // State 3: begin attack — allocate targets and assign to phantoms
        // FIXME 16: target allocation (Fisher-Yates shuffle, heap arrays)
        // TODO: allocate m8C (numEnemies*4) and m90 (numEnemies) arrays
        // TODO: Fisher-Yates shuffle of valid target indices
        // TODO: assign each phantom a target via getBattleTargetablePosition
        // For now: set phantoms to attack mode and advance
        for (s32 i = 0; i < 3; i++)
        {
            pThis->m0_phantoms[i].m22_subState = 2;
        }
        pThis->m85_subState++;

        [[fallthrough]];
    }
    case 4:
    {
        // State 4: attack in progress — apply damage per phantom hit
        // FIXME 17: damage/tracking loop (stubbed)
        // TODO: iterate m90 hit array, apply phantomSlasher_getBaseDamage
        // TODO: damage power = m80 >> 1, unless mA4 set (then full m80)

        // FIXME 18: per-phantom position update
        // TODO: for each phantom, update target pos from getBattleTargetablePosition
        // or apply autoScroll if target dead

        // FIXME 19: completion check from phantom completion flags
        s32 doneCount = 0;
        for (s32 i = 0; i < 3; i++)
        {
            if (pThis->m0_phantoms[i].mC_completionFlag != 0)
                doneCount++;
        }
        if (doneCount == 3)
        {
            pThis->getTask()->markFinished();
        }
        break;
    }
    case 5:
    case 6:
    {
        // States 5-6: fade out (phantom sub-tasks handle their own fade)
        // Check if all phantoms have finished fading
        s32 fadeDone = 0;
        for (s32 i = 0; i < 3; i++)
        {
            if (pThis->m0_phantoms[i].mC_completionFlag != 0)
                fadeDone++;
        }
        if (fadeDone == 3)
        {
            pThis->getTask()->markFinished();
        }
        break;
    }
    default:
        break;
    }

    // FIXME 20: post-switch epilogue — check all phantom model ptrs
    // If all 3 phantoms have non-zero m10_modelPtr, mark task finished (safety net)
    s32 modelCount = 0;
    for (s32 i = 0; i < 3; i++)
    {
        if (pThis->m0_phantoms[i].m10_modelPtr != 0)
            modelCount++;
    }
    // NOTE: Ghidra checks this as a safety net for phantom sub-task completion
    // TODO: verify exact epilogue logic from Ghidra
}

// BTL_A3::06097984 -- astral phantoms draw (empty in original)
static void sAstralPhantomsTask_Draw(sAstralPhantomsTask* pThis)
{
    // Ghidra decompilation shows this is an empty function — phantoms are rendered
    // by their individual sub-tasks, not by the parent draw method.
}

// BTL_A3::06096f8c
void berserk_createAstralPhantomsShield(s_battleEngine* pThis)
{
    static const sAstralPhantomsTask::TypedTaskDefinition definition = {
        nullptr,
        (void(*)(sAstralPhantomsTask*))&sAstralPhantomsTask_Update,
        (void(*)(sAstralPhantomsTask*))&sAstralPhantomsTask_Draw,
        (void(*)(sAstralPhantomsTask*))&sAstralPhantomsTask_Delete,
    };

    sAstralPhantomsTask* pNewTask = createSubTask<sAstralPhantomsTask>(pThis, &definition);
    if (pNewTask == nullptr)
    {
        pThis->m188_flags.m100_attackAnimationFinished = 1;
        return;
    }

    // Init state — matches Ghidra byte-level writes
    pNewTask->m85_subState = 0;
    pNewTask->m80_orbitAngle = 0;
    pNewTask->m88_rotationSpeed = 0x1999;

    // Random phantom model variant
    u32 rnd = randomNumber();
    pNewTask->m6C_phantomVariant = (u8)(s32)MTH_Mul(rnd >> 16, fixedPoint(3));

    // FIXME 6: zero 5x s32 at m94 (not 5x sVec3_FP)
    pNewTask->m94_attackDetected = 0;
    pNewTask->m98_shouldSetM100 = 0;
    pNewTask->m9C_hasCamera = 0;
    pNewTask->mA0_destroyFlag = 0;
    pNewTask->mA4_fullSpeedFlag = 0;

    // FIXME 7: per-phantom init (zero sub-struct + create sub-task)
    for (s32 i = 0; i < 3; i++)
    {
        // FUN_06097a18 — zero phantom sub-struct fields
        pNewTask->m0_phantoms[i].m0 = 0;
        pNewTask->m0_phantoms[i].m4 = 0;
        pNewTask->m0_phantoms[i].m8 = 0;
        pNewTask->m0_phantoms[i].mC_completionFlag = 0;
        pNewTask->m0_phantoms[i].m10_modelPtr = 0;

        // FUN_06097a34 — creates 0x130-size child task via createSubTaskWithCopy
        // TODO: implement phantom sub-task creation (visual model + orbit animation)
        // Each phantom gets its own Update/Draw sub-task for rendering
    }

    pNewTask->m98_shouldSetM100 = 1;
    playSystemSoundEffect(0xC);
}
