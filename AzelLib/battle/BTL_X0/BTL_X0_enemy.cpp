#include "PDS.h"
#include "BTL_X0_enemy.h"
#include "BTL_X0_data.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "battle/battleEngine.h"
#include "battle/battleEnemyModels.h"
#include "battle/battleTargetable.h"
#include "battle/battleEnemyLifeMeter.h"
#include "kernel/graphicalObject.h"
#include "kernel/animation.h"
#include "kernel/fileBundle.h"
#include "mainMenuDebugTasks.h"
#include "audio/soundDriver.h"

#include "battle/battleTextDisplay.h"
#include "battle/battleDebug.h"

// 06063544
static void BTL_X0_updateBodyAnimation(sBTL_X0_EnemyModel* pThis)
{
    if (pThis->m310_attackActive == 1)
        updateAndInterpolateAnimation(&pThis->m98_models[0]);
    else
        stepAnimation(&pThis->m98_models[0]);
}

// Stubs for command handlers
// 0606335c
static void BTL_X0_cmd_idle(sBTL_X0_EnemyModel* pThis)
{
    if (battleEngine_isPlayerTurnActive() != 0)
        return;

    s32 done = vec2FPInterpolator_Step(&pThis->m1AC_interpolator);
    if (done != 0)
    {
        pThis->m1AC_interpolator.mC_startValue = pThis->m7C_position;

        if (pThis->m30E_flag2 == 1)
        {
            pThis->m1AC_interpolator.m24_targetValue.m0_X = (randomNumber() % 0x14000) - 0xA000;
            pThis->m1AC_interpolator.m24_targetValue.m4_Y = (randomNumber() % 0x14000) - 0xA000;
            if (pThis->m308_variantIndex == 2)
                pThis->m1AC_interpolator.m24_targetValue.m8_Z = (randomNumber() % 0x14000) - 0x19000;
            else
                pThis->m1AC_interpolator.m24_targetValue.m8_Z = (randomNumber() % 0x14000) - 0xA000;
            pThis->m1AC_interpolator.m38_interpolationLength = 0x5A;
        }
        else
        {
            pThis->m1AC_interpolator.m24_targetValue.m0_X = 0;
            pThis->m1AC_interpolator.m24_targetValue.m4_Y = 0;
            pThis->m1AC_interpolator.m24_targetValue.m8_Z = 0;
            pThis->m1AC_interpolator.m38_interpolationLength = 0x3C;
        }

        sSaturnPtr baseTable = g_BTL_X0->getSaturnPtr(0x060b7f18) + (s8)(pThis->m308_variantIndex * 0xC);
        pThis->m1AC_interpolator.m24_targetValue.m0_X += readSaturnS32(baseTable);
        pThis->m1AC_interpolator.m24_targetValue.m4_Y += readSaturnS32(baseTable + 4);
        pThis->m1AC_interpolator.m24_targetValue.m8_Z += readSaturnS32(baseTable + 8);

        vec2FPInterpolator_Init(&pThis->m1AC_interpolator);
    }

    pThis->m7C_position.m0_X = pThis->m1AC_interpolator.m0_currentValue.m0_X;
    pThis->m7C_position.m4_Y = pThis->m1AC_interpolator.m0_currentValue.m4_Y;
    pThis->m7C_position.m8_Z = pThis->m1AC_interpolator.m0_currentValue.m8_Z;
}
static void BTL_X0_cmd_attack2(sBTL_X0_EnemyModel* pThis) { Unimplemented(); }         // 060644a2
static void BTL_X0_cmd_attack5(sBTL_X0_EnemyModel* pThis) { Unimplemented(); }         // 06063ca6
static void BTL_X0_cmd_attack7(sBTL_X0_EnemyModel* pThis) { Unimplemented(); }         // 060641c2
static void BTL_X0_cmd_attack8(sBTL_X0_EnemyModel* pThis) { Unimplemented(); }         // 0606426e
static void BTL_X0_cmd_attack9(sBTL_X0_EnemyModel* pThis) { Unimplemented(); }         // 060643ba
static void BTL_X0_cmd_attackA(sBTL_X0_EnemyModel* pThis) { Unimplemented(); }         // 06063f76
static void BTL_X0_cmd_attackC(sBTL_X0_EnemyModel* pThis) { Unimplemented(); }         // 060565ca
static void BTL_X0_cmd_attackE(sBTL_X0_EnemyModel* pThis) { Unimplemented(); }         // 06056a32
static void BTL_X0_cmd_attackF(sBTL_X0_EnemyModel* pThis) { Unimplemented(); }         // 06056b40
static void BTL_X0_cmd_attack10(sBTL_X0_EnemyModel* pThis) { Unimplemented(); }        // 06056c4c
// 0606326e
static void BTL_X0_cmd_genericAttack(sBTL_X0_EnemyModel* pThis)
{
    if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished)
    {
        pThis->m307_pad = 0;
        pThis->m30A_commandIndex = 0;
    }
}
static void BTL_X0_cmd_death(sBTL_X0_EnemyModel* pThis) { Unimplemented(); }           // 06063630 = processCommand
static void BTL_X0_cmd_deathSub(sBTL_X0_EnemyModel* pThis) { Unimplemented(); }        // 06063bb6 = processCommandSub
static void BTL_X0_cmd_attack18(sBTL_X0_EnemyModel* pThis) { Unimplemented(); }        // 06056fae
static void BTL_X0_cmd_attack19(sBTL_X0_EnemyModel* pThis) { Unimplemented(); }        // 06057534
static void BTL_X0_cmd_attack1A(sBTL_X0_EnemyModel* pThis) { Unimplemented(); }        // 060576da

// 060630cc
static void BTL_X0_updateIdlePosition(sBTL_X0_EnemyModel* pThis)
{
    switch (pThis->m30A_commandIndex)
    {
    case 0:
        BTL_X0_cmd_idle(pThis);
        break;
    case 1:
    case 3:
    case 4:
        break;
    case 2:
        BTL_X0_cmd_attack2(pThis);
        break;
    case 5:
        BTL_X0_cmd_attack5(pThis);
        break;
    case 6:
    case 0x14:
        pThis->m304_state = 0xF;
        pThis->m307_pad = 0;
        pThis->m30A_commandIndex = 0;
        break;
    case 7:
        BTL_X0_cmd_attack7(pThis);
        break;
    case 8:
        BTL_X0_cmd_attack8(pThis);
        break;
    case 9:
        BTL_X0_cmd_attack9(pThis);
        break;
    case 10:
        BTL_X0_cmd_attackA(pThis);
        break;
    case 0xB:
        pThis->m304_state = 4;
        pThis->m306_dangerQuadrant = 0;
        pThis->m307_pad = 0;
        pThis->m30A_commandIndex = 0;
        break;
    case 0xC:
        BTL_X0_cmd_attackC(pThis);
        break;
    case 0xD:
        pThis->m304_state = 0;
        pThis->m306_dangerQuadrant = 4;
        pThis->m307_pad = 0;
        pThis->m30A_commandIndex = 0;
        break;
    case 0xE:
        BTL_X0_cmd_attackE(pThis);
        break;
    case 0xF:
        BTL_X0_cmd_attackF(pThis);
        break;
    case 0x10:
        BTL_X0_cmd_attack10(pThis);
        break;
    case 0x11:
    case 0x12:
    case 0x13:
    case 0x15:
        BTL_X0_cmd_genericAttack(pThis);
        break;
    case 0x16:
        BTL_X0_cmd_death(pThis);
        break;
    case 0x17:
        BTL_X0_cmd_deathSub(pThis);
        break;
    case 0x18:
        BTL_X0_cmd_attack18(pThis);
        break;
    case 0x19:
        BTL_X0_cmd_attack19(pThis);
        break;
    case 0x1A:
        BTL_X0_cmd_attack1A(pThis);
        break;
    }
}

// 060632ac
static s32 BTL_X0_isAnimationFinished(sBTL_X0_EnemyModel* pThis)
{
    s_3dModel* pModel = &pThis->m98_models[0];
    s32 totalFrames = 0;
    if (pModel->m30_pCurrentAnimation != nullptr)
    {
        totalFrames = (s32)pModel->m30_pCurrentAnimation->m4_numFrames;
    }
    if ((s32)pModel->m16_previousAnimationFrame >= totalFrames - 1)
        return 1;
    return 0;
}

// 060634d4
static void BTL_X0_initDeathAnimation(sBTL_X0_EnemyModel* pThis, s8 param)
{
    pThis->m30B_subCommand = param;
    pThis->m310_attackActive = 0;
    s16 animOffset = readSaturnS16(g_BTL_X0->getSaturnPtr(0x060b7e7c) + (u8)pThis->m30B_subCommand * 2);
    sAnimationData* pAnim = pThis->m0_fileBundle->getAnimation(animOffset);
    initAnimation(&pThis->m98_models[0], pAnim);
    pThis->m311_animPending = 1;
}

// 0606350c
static void BTL_X0_playHitReaction(sBTL_X0_EnemyModel* pThis, s8 param1, s32 interpLength)
{
    pThis->m30B_subCommand = param1;
    pThis->m310_attackActive = 1;
    s16 animOffset = readSaturnS16(g_BTL_X0->getSaturnPtr(0x060b7e7c) + (u8)pThis->m30B_subCommand * 2);
    sAnimationData* pAnim = pThis->m0_fileBundle->getAnimation(animOffset);
    playAnimationGeneric(&pThis->m98_models[0], pAnim, interpLength);
    pThis->m311_animPending = 1;
}

// 060632ce
static void BTL_X0_checkCollisionAndDamage(sBTL_X0_EnemyModel* pThis)
{
    Unimplemented();
}

// 06062a2e
static void BTL_X0_updateTargetableQuadrants(sBTL_X0_EnemyModel* pThis, s32 param)
{
    Unimplemented();
}

// 06063566 — transforms 7 selected hotpoints of model[0] by cameraProperties2.m28[0]
// and writes them into the 0x54-byte buffer at m1A8_attackDataBuffer.
static void BTL_X0_transformTargetablePositions(sBTL_X0_EnemyModel* pThis)
{
    if (pThis->m1A8_attackDataBuffer == nullptr || pThis->m98_models.empty())
        return;

    s_3dModel& model0 = pThis->m98_models[0];
    sVec3_FP* dst = (sVec3_FP*)pThis->m1A8_attackDataBuffer;
    const sMatrix4x3& cam = cameraProperties2.m28[0];

    auto get = [&](u32 bone, u32 hp) -> const sVec3_FP* {
        if (bone < model0.m44_hotpointData.size() && hp < model0.m44_hotpointData[bone].size())
            return &model0.m44_hotpointData[bone][hp];
        return nullptr;
    };

    // Ghidra: src offset encodes bone (byte/4) and hotpoint (byte/12 within bone's vec3 array).
    struct Entry { u32 bone; u32 hp; };
    static const Entry entries[7] = {
        {10, 5}, // (m44 + 0x28) + 0x3c → m44[10], hotpoint 5
        {26, 0}, // *(sVec3_FP**)(m44 + 0x68) → m44[26], hotpoint 0
        {33, 0}, // *(sVec3_FP**)(m44 + 0x84) → m44[33], hotpoint 0
        {10, 1}, // (m44 + 0x28) + 0x0c → m44[10], hotpoint 1
        {10, 2}, // + 0x18 → hotpoint 2
        {10, 3}, // + 0x24 → hotpoint 3
        {10, 4}, // + 0x30 → hotpoint 4
    };
    for (int i = 0; i < 7; i++)
    {
        const sVec3_FP* src = get(entries[i].bone, entries[i].hp);
        if (src)
            transformAndAddVec(*src, dst[i], cam);
    }
}

// 06062966 — copies raw hotpoint positions of model[0] into the m1A4 targetable-position buffer,
// in order (skipping bones with no hotpoint pointer). Iterates model[0].m12_numBones.
static void BTL_X0_copyTargetablePositions(sBTL_X0_EnemyModel* pThis)
{
    if (pThis->m1A4_targetablePositionData == nullptr || pThis->m98_models.empty())
        return;

    s_3dModel& model0 = pThis->m98_models[0];
    if (model0.m40 == nullptr)
        return;

    sVec3_FP* dst = (sVec3_FP*)pThis->m1A4_targetablePositionData;
    s32 dstIdx = 0;
    std::vector<s_hotpointDefinition>& defs = *model0.m40;

    for (u32 bone = 0; bone < model0.m12_numBones && bone < defs.size() && bone < model0.m44_hotpointData.size(); bone++)
    {
        if (model0.m44_hotpointData[bone].empty())
            continue;
        s32 count = (s32)defs[bone].m4_count;
        for (s32 h = 0; h < count && h < (s32)model0.m44_hotpointData[bone].size(); h++)
        {
            dst[dstIdx++] = model0.m44_hotpointData[bone][h];
        }
    }
}

// 06064714
static void BTL_X0_enemyDebugDraw(sBTL_X0_EnemyModel* pThis)
{
    Unimplemented();
}

void setupConditionalLightColor(int param_1);
void clearLightColor();

// Light-color lookup table at BTL_X0::060b7e98 — indexed by quadrant + flag3*4
static u8 BTL_X0_lightColorTable_cache(int idx)
{
    static u8 cached[32] = {0};
    static bool loaded = false;
    if (!loaded)
    {
        sSaturnPtr base = g_BTL_X0->getSaturnPtr(0x060b7e98);
        for (int i = 0; i < 32; i++)
            cached[i] = (u8)readSaturnU8(base + i);
        loaded = true;
    }
    return cached[idx];
}

// 06062a8e
static void BTL_X0_processDamage(sBTL_X0_EnemyModel* pThis, s16 param)
{
    Unimplemented();
}

// 0606259c
static void BTL_X0_enemyModel_Update(sBTL_X0_EnemyModel* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    pThis->m1C_lifeMeterPosition.m0_X = pEngine->mC_battleCenter.m0_X + pThis->m7C_position.m0_X;
    pThis->m1C_lifeMeterPosition.m4_Y = pEngine->mC_battleCenter.m4_Y + pThis->m7C_position.m4_Y;
    pThis->m1C_lifeMeterPosition.m8_Z = pEngine->mC_battleCenter.m8_Z + pThis->m7C_position.m8_Z;

    if (pThis->mFC_idleState == 0)
    {
        pThis->m1AC_interpolator.mC_startValue = pThis->m7C_position;
        pThis->m1AC_interpolator.m24_targetValue.m0_X = (randomNumber() % 0x14000) - 0xA000;
        pThis->m1AC_interpolator.m24_targetValue.m4_Y = (randomNumber() % 0x14000) - 0xA000;
        pThis->m1AC_interpolator.m24_targetValue.m8_Z = (randomNumber() % 0x14000) - 0xA000;
        pThis->m1AC_interpolator.m38_interpolationLength = 0x5A;
        vec2FPInterpolator_Init(&pThis->m1AC_interpolator);
        pThis->mFC_idleState++;
    }
    else if (pThis->mFC_idleState == 1)
    {
        BTL_X0_updateIdlePosition(pThis);
        if (pThis->m311_animPending == 0)
        {
            if (BTL_X0_isAnimationFinished(pThis))
            {
                if (pThis->m30A_commandIndex == 0x16 || pThis->m30A_commandIndex == 0x18)
                {
                    BTL_X0_initDeathAnimation(pThis, 3);
                }
                else if (pThis->m30B_subCommand == 3)
                {
                    BTL_X0_playHitReaction(pThis, 0, 5);
                }
                else
                {
                    s8 animIdx = (randomNumber() & 1) ? 3 : 0;
                    BTL_X0_playHitReaction(pThis, animIdx, 5);
                }
            }
        }
        else
        {
            pThis->m311_animPending = 0;
        }
    }

    if ((pThis->m14_flags & 1) == 0)
    {
        BTL_X0_updateBodyAnimation(pThis);
        if (pThis->m98_models.size() > 1)
            stepAnimation(&pThis->m98_models[1]);
        if (pThis->m98_models.size() > 2)
            stepAnimation(&pThis->m98_models[2]);
    }

    BTL_X0_checkCollisionAndDamage(pThis);
    s32 flag3 = (s32)pThis->m30F_flag3;
    BTL_X0_updateTargetableQuadrants(pThis, flag3);
    BTL_X0_processDamage(pThis, (s16)flag3);

    battleEngine_FlagQuadrantBitForSafety(0);
    battleEngine_FlagQuadrantBitForSafety((u16)(u8)pThis->m304_state);
    battleEngine_FlagQuadrantBitForDanger(0);
    battleEngine_FlagQuadrantBitForDanger((u16)(u8)pThis->m306_dangerQuadrant);
}

// 0606279a
static void BTL_X0_enemyModel_Draw(sBTL_X0_EnemyModel* pThis)
{
    // Debug keyboard toggle: swap between model[0] and model[1]
    if (readKeyboardToggle(0xC5) != 0)
    {
        pThis->m30D_flag1 = (pThis->m30D_flag1 == 0) ? 1 : 0;
    }

    // Conditional light color — table indexed by dragonCurrentQuadrant + m30F_flag3 * 4
    if ((pThis->m14_flags & 0x800000) != 0)
    {
        s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
        int idx = (u8)pEngine->m22C_dragonCurrentQuadrant + (s8)pThis->m30F_flag3 * 4;
        setupConditionalLightColor((u32)BTL_X0_lightColorTable_cache(idx));
    }

    pushCurrentMatrix();
    translateCurrentMatrix(&pThis->m1C_lifeMeterPosition);
    scaleCurrentMatrixRow0(pThis->m26C_scale.m0_X);
    scaleCurrentMatrixRow1(pThis->m26C_scale.m4_Y);
    scaleCurrentMatrixRow2(pThis->m26C_scale.m8_Z);
    rotateCurrentMatrixYXZ(pThis->m28_rotation);

    {
        // flag1 selects model[0] or model[1] for the primary draw
        s_3dModel* pPrimary = (pThis->m30D_flag1 == 0) ? &pThis->m98_models[0] : &pThis->m98_models[1];
        if (pPrimary->m18_drawFunction)
            pPrimary->m18_drawFunction(pPrimary);
    }
    popMatrix();

    // Second pass: draw Azel (m98_models[2]) at world-space position computed from
    // Atolm's head attach point (model[0].m44_hotpointData[10][0]) — Ghidra m44 + 0x28 = bone 10.
    if (pThis->m30D_flag1 == 0 && pThis->m98_models.size() > 2 && pThis->m98_models[2].m18_drawFunction)
    {
        sVec3_FP local;
        auto& hp = pThis->m98_models[0].m44_hotpointData;
        if (hp.size() > 10 && !hp[10].empty())
            transformAndAddVec(hp[10][0], local, cameraProperties2.m28[0]);
        else
            local = pThis->m1C_lifeMeterPosition;

        pushCurrentMatrix();
        translateCurrentMatrix(&local);
        scaleCurrentMatrixRow0(pThis->m26C_scale.m0_X);
        scaleCurrentMatrixRow1(pThis->m26C_scale.m4_Y);
        scaleCurrentMatrixRow2(pThis->m26C_scale.m8_Z);
        rotateCurrentMatrixYXZ(pThis->m28_rotation);
        pThis->m98_models[2].m18_drawFunction(&pThis->m98_models[2]);
        popMatrix();
    }

    // Clear conditional light color (also clears the flag bit)
    if ((pThis->m14_flags & 0x800000) != 0)
    {
        pThis->m14_flags &= ~0x800000;
        clearLightColor();
    }

    BTL_X0_transformTargetablePositions(pThis);
    BTL_X0_copyTargetablePositions(pThis);

    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x19] != 0)
    {
        BTL_X0_enemyDebugDraw(pThis);
    }
}

// 06062930
static void BTL_X0_enemyModel_Delete(sBTL_X0_EnemyModel* pThis)
{
    for (u32 i = 0; i < pThis->m98_models.size(); i++)
    {
        s_3dModel* pModel = &pThis->m98_models[i];
        if (pModel->m54_targetables != nullptr)
        {
            for (s32 j = 0; j < pModel->m52_targetableCount; j++)
            {
                deleteTargetable(&pModel->m54_targetables[j]);
            }
        }
    }
}

// 060593b8
static sBTL_X0_AttackSubPart* BTL_X0_createAttackSubPart(sBTL_X0_AttackSubTask* pParent, u8 partIndex)
{
    sBTL_X0_AttackSubPart* pPart = createSubTask<sBTL_X0_AttackSubPart>((p_workArea)pParent);
    if (pPart == nullptr)
        return nullptr;

    pPart->m0_fileBundle = pParent->m0_parentEnemy->m0_fileBundle;
    pPart->m4_vd1Allocation = pParent->m0_parentEnemy->m4_vd1Allocation;

    pPart->mFC_state = 0;
    pPart->m154_parentAttackTask = pParent;
    pPart->m300_partIndex = partIndex;
    pPart->m208_value = 0;
    pPart->m304_state = 0;
    pPart->m301_flag = 0;
    pPart->m29C_value = 0x28F;
    pPart->m2A0_value = 0x28F;
    pPart->m14_flags |= 0x80000000;
    pPart->m14_flags |= 0x10000000;

    return pPart;
}

// 06059444
void sBTL_X0_AttackSubPart::Update(sBTL_X0_AttackSubPart* pThis)
{
    Unimplemented();
}

// 06059b12
void sBTL_X0_AttackSubPart::Draw(sBTL_X0_AttackSubPart* pThis)
{
    Unimplemented();
}

// 06059b8e
void sBTL_X0_AttackSubPart::Delete(sBTL_X0_AttackSubPart* pThis)
{
}

static const sVec3_FP BTL_X0_attackPositions[6] = {
    { fixedPoint(0x00000000), fixedPoint(0x0001E000), fixedPoint(0x00000000) },
    { fixedPoint(0x0001E000), fixedPoint(0x00000000), fixedPoint(0x00000000) },
    { fixedPoint(0x00000000), fixedPoint(0x0001E000), fixedPoint((s32)0xFFFE2000) },
    { fixedPoint(0x00000000), fixedPoint(0x00000000), fixedPoint((s32)0xFFFE2000) },
    { fixedPoint(0x00000000), fixedPoint((s32)0xFFFE2000), fixedPoint(0x00000000) },
    { fixedPoint((s32)0xFFFE2000), fixedPoint(0x00000000), fixedPoint(0x00000000) },
};

// 06057e04
void sBTL_X0_AttackSubTask::Update(sBTL_X0_AttackSubTask* pThis)
{
    sBTL_X0_EnemyModel* pEnemy = pThis->m0_parentEnemy;
    s8 currentCmd = pEnemy->m30A_commandIndex;

    if (currentCmd != pThis->m6B_lastCommand)
    {
        if (currentCmd != 0x13 && pThis->mB8_pass == 0)
        {
            // no-op: skip command dispatch unless it's 0x13 or pass > 0
        }
        else if (currentCmd == 0x09)
        {
            for (int i = 0; i < 4; i++)
                pThis->m5E_partCmd[i] = 6;
        }
        else if (currentCmd == 0x11)
        {
            u32 rng = randomNumber();
            pThis->m6A_targetPart = ((rng & 1) == 0) ? 3 : 0;
            if (pThis->m70_partAlive[pThis->m6A_targetPart] == 0)
                pThis->m6A_targetPart = (pThis->m6A_targetPart == 3) ? 0 : 3;

            for (int i = 0; i < 4; i++)
            {
                if (pThis->m70_partAlive[i] == 1)
                    pThis->m5E_partCmd[i] = (pThis->m6A_targetPart == i) ? 4 : 3;
                else
                    pThis->m5E_partCmd[i] = 8;
            }
        }
        else if (currentCmd == 0x12)
        {
            u32 rng = randomNumber();
            pThis->m6A_targetPart = ((rng & 1) == 0) ? 1 : 2;
            if (pThis->m70_partAlive[pThis->m6A_targetPart] == 0)
                pThis->m6A_targetPart = (pThis->m6A_targetPart == 1) ? 2 : 1;

            for (int i = 0; i < 4; i++)
            {
                if (pThis->m70_partAlive[i] == 1)
                    pThis->m5E_partCmd[i] = (pThis->m6A_targetPart == i) ? 4 : 3;
                else
                    pThis->m5E_partCmd[i] = 8;
            }
        }
        else if (currentCmd == 0x13)
        {
            if (pThis->mB8_pass == 0)
            {
                for (int i = 0; i < 4; i++)
                    pThis->m5E_partCmd[i] = 1;
                pThis->m6A_targetPart = (u8)MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(4)).getInteger();
                pThis->mB8_pass = 1;
            }
            else
            {
                s32 deadCount = 0;
                for (int i = 0; i < 4; i++)
                {
                    if (pThis->m70_partAlive[i] == 1)
                    {
                        pThis->m5E_partCmd[i] = 3;
                        pThis->m90_partComplete[i] = 0;
                    }
                    else
                    {
                        if (deadCount == 0 || (randomNumber() & 1) == 0)
                            pThis->m6A_targetPart = (u8)i;
                        pThis->m5E_partCmd[i] = 9;
                        deadCount++;
                    }
                }
            }
        }
        else if (currentCmd == 0x15)
        {
            pThis->m6A_targetPart = (u8)MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(4)).getInteger();

            fixedPoint randA = MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x10000000));
            fixedPoint randB = MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x10000000));
            pThis->m54_rotY = MTH_Mul(getSin(randB.getInteger()), randA);

            fixedPoint randC = MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x10000000));
            fixedPoint randD = MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x10000000));
            fixedPoint cosD_C = MTH_Mul(getCos(randD.getInteger()), randC);

            fixedPoint randE = MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x10000000));
            pThis->m58_rotZ = MTH_Mul(getSin(randE.getInteger()), cosD_C);

            fixedPoint randF = MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x10000000));
            pThis->m50_rotX = MTH_Mul(getCos(randF.getInteger()), cosD_C);

            for (int i = 0; i < 6; i++)
            {
                sMatrix4x3 mat;
                initMatrixToIdentity(&mat);
                rotateMatrixShiftedY(pThis->m54_rotY, &mat);
                rotateMatrixShiftedX(pThis->m50_rotX, &mat);
                rotateMatrixShiftedZ(pThis->m58_rotZ, &mat);
                transformAndAddVec(BTL_X0_attackPositions[i], pThis->m8_positions[i], mat);
            }

            // Fisher-Yates shuffle of partOrder
            for (int pair = 0; pair < 2; pair++)
            {
                s32 remaining0 = 4 - pair * 2;
                s32 idx0 = MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(remaining0)).getInteger();
                u8 tmp0 = pThis->m66_partOrder[idx0];
                pThis->m66_partOrder[idx0] = pThis->m66_partOrder[3 - pair * 2];
                pThis->m66_partOrder[3 - pair * 2] = tmp0;
                pThis->m5E_partCmd[pair * 2] = 7;

                s32 remaining1 = remaining0 - 1;
                s32 idx1 = MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(remaining1)).getInteger();
                u8 tmp1 = pThis->m66_partOrder[idx1];
                pThis->m66_partOrder[idx1] = pThis->m66_partOrder[3 - pair * 2 - 1];
                pThis->m66_partOrder[3 - pair * 2 - 1] = tmp1;
                pThis->m5E_partCmd[pair * 2 + 1] = 7;
            }

            pThis->mB0_value = MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(2)).getInteger();
        }
        else
        {
            // Default: reset m90 and set part cmds based on alive state
            for (int i = 0; i < 4; i++)
                pThis->m90_partComplete[i] = 1;

            for (int i = 0; i < 4; i++)
                pThis->m5E_partCmd[i] = (pThis->m70_partAlive[i] == 1) ? 2 : 8;
        }
    }

    pThis->m6B_lastCommand = (u8)pEnemy->m30A_commandIndex;

    // Variant 3: timer-based part visibility cycling
    if (pThis->m6C_variant == 3)
    {
        if (battleEngine_isPlayerTurnActive() == 0)
        {
            if (pThis->m5C_timer < 300)
                pThis->m5C_timer++;
            else
                pThis->m5C_timer = 0;
        }

        if (pThis->m5C_timer < 0x4B || pThis->m5C_timer > 0xE0)
        {
            pThis->mA0_partState[1] = 0;
            pThis->mA0_partState[2] = 0;
        }
        else
        {
            pThis->mA0_partState[1] = 1;
            pThis->mA0_partState[2] = 1;
        }

        if (pThis->m5C_timer < 0x96)
        {
            pThis->mA0_partState[3] = 1;
            pThis->mA0_partState[0] = 1;
        }
        else
        {
            pThis->mA0_partState[3] = 0;
            pThis->mA0_partState[0] = 0;
        }

        if (pThis->m5C_timer == 0x95 || pThis->m5C_timer == 0xE0)
            pThis->mB4_trigger = 1;
        else
            pThis->mB4_trigger = 0;
    }

    // Check if all parts completed for command 0x13
    s32 completedCount = 0;
    if (pEnemy->m30A_commandIndex == 0x13)
    {
        for (int i = 0; i < 4; i++)
        {
            if (pThis->m90_partComplete[i] == 0)
                completedCount++;
        }
    }
    if (completedCount == 4)
    {
        s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
        pEngine->m188_flags.m100_attackAnimationFinished = 1;
    }

    // Check for per-part error signals
    for (int i = 0; i < 4; i++)
    {
        if (pThis->m80_partError[i] != 0)
        {
            pThis->getTask()->markFinished();
        }
    }
}

// 060586d0
void sBTL_X0_AttackSubTask::Delete(sBTL_X0_AttackSubTask* pThis)
{
}

// 06057d30
static p_workArea BTL_X0_createAttackSubTask(sBTL_X0_EnemyModel* pThis, void* attackDataBuffer)
{
    sBTL_X0_AttackSubTask* pTask = createSubTask<sBTL_X0_AttackSubTask>((p_workArea)pThis);
    if (pTask == nullptr)
        return nullptr;

    pTask->m0_parentEnemy = pThis;
    pTask->m6C_variant = (u8)pThis->m308_variantIndex;
    pTask->m6B_lastCommand = 0;
    pTask->m4_attackDataBuffer = attackDataBuffer;
    pTask->mB8_pass = 0;
    pTask->m5C_timer = 0;

    for (int i = 0; i < 4; i++)
    {
        pTask->m66_partOrder[i] = (u8)i;
        pTask->m5E_partCmd[i] = 0;
        pTask->m70_partAlive[i] = 0;
        pTask->m80_partError[i] = 0;
        pTask->m90_partComplete[i] = 1;
        pTask->mA0_partState[i] = 0;
    }

    for (int i = 0; i < 4; i++)
    {
        sBTL_X0_AttackSubPart* pPart = BTL_X0_createAttackSubPart(pTask, (u8)i);
        if (pPart == nullptr)
        {
            pTask->getTask()->markFinished();
            return pTask;
        }
    }

    return pTask;
}

// 0606237c
p_workArea BTL_X0_createEnemyModels(s_workArea* pFormation, s8 variant)
{
    static const sBTL_X0_EnemyModel::TypedTaskDefinition def = {
        nullptr,
        &BTL_X0_enemyModel_Update,
        &BTL_X0_enemyModel_Draw,
        &BTL_X0_enemyModel_Delete,
    };
    sBTL_X0_EnemyModel* pThis = createSubTaskWithCopy<sBTL_X0_EnemyModel>((s_workAreaCopy*)pFormation, &def);
    if (pThis == nullptr)
        return nullptr;

    s_fileBundle* pBundle = dramAllocatorEnd[6].mC_fileBundle->m0_fileBundle;
    pThis->m0_fileBundle = pBundle;
    pThis->m8_parentFormation = pFormation;

    pThis->m98_models.resize(3);

    // Model 0: Atolm body — uses hotpoint bundle at 060b7d20 (per-bone targeting points, including bone 10 = head attach)
    sModelHierarchy* pHierarchy0 = pBundle->getModelHierarchy(4);
    u32 numBones0 = pHierarchy0->countNumberOfBones();
    {
        sHotpointBundle* pHotspots = nullptr;
        sSaturnPtr hotspotEA = g_BTL_X0->getSaturnPtr(0x060b7d20);
        if (!hotspotEA.isNull())
        {
            pThis->m98_models[0].m_hotpointBundles.reserve(numBones0);
            for (u32 b = 0; b < numBones0; b++)
                pThis->m98_models[0].m_hotpointBundles.emplace_back(hotspotEA + b * 8);
            pHotspots = pThis->m98_models[0].m_hotpointBundles.data();
        }
        init3DModelRawData(pThis, &pThis->m98_models[0], 0, pBundle, 4,
            pBundle->getAnimation(0x20C), pBundle->getStaticPose(0x1F4, numBones0), nullptr, pHotspots);
    }
    stepAnimation(&pThis->m98_models[0]);

    // Model 1: Atolm debug/alternate — swapped in for model[0] when m30D_flag1 is toggled (debug key 0xC5)
    sModelHierarchy* pHierarchy1 = pBundle->getModelHierarchy(0xC);
    u32 numBones1 = pHierarchy1->countNumberOfBones();
    init3DModelRawData(pThis, &pThis->m98_models[1], 0, pBundle, 0xC,
        pBundle->getAnimation(0x224), pBundle->getStaticPose(0x1FC, numBones1), nullptr, nullptr);
    stepAnimation(&pThis->m98_models[1]);

    // Model 2: Azel (rider) — drawn at world-space position derived from Atolm's head hotpoint
    sModelHierarchy* pHierarchy2 = pBundle->getModelHierarchy(8);
    u32 numBones2 = pHierarchy2->countNumberOfBones();
    init3DModelRawData(pThis, &pThis->m98_models[2], 0, pBundle, 8,
        pBundle->getAnimation(0x22C), pBundle->getStaticPose(0x1F8, numBones2), nullptr, nullptr);
    stepAnimation(&pThis->m98_models[2]);

    // Allocate 0x54-byte attack-data buffer and zero-init
    pThis->m1A8_attackDataBuffer = allocateHeapForTask(pThis, 0x54);
    if (pThis->m1A8_attackDataBuffer)
        memset(pThis->m1A8_attackDataBuffer, 0, 0x54);

    if (variant == 0)
    {
        pThis->mEC_hpMax = 3000;
        pThis->mEE_hpCurrent = 3000;
        pThis->m10_lifeMeterTask = createEnemyLifeMeterTask(&pThis->m1C_lifeMeterPosition, 0, &pThis->mEE_hpCurrent, 0x24);
        pThis->m308_variantIndex = 0;
        displayFormationName(0, 0, 0xB);
        pThis->m304_state = 5;
    }
    else
    {
        if (variant == 1)
        {
            pThis->mEC_hpMax = 6000;
            pThis->mEE_hpCurrent = 6000;
            pThis->m10_lifeMeterTask = createEnemyLifeMeterTask(&pThis->m1C_lifeMeterPosition, 0, &pThis->mEE_hpCurrent, 0x33);
            pThis->m308_variantIndex = 1;
            displayFormationName(0, 0, 0xB);
        }
        else if (variant == 3)
        {
            pThis->mEC_hpMax = 8000;
            pThis->mEE_hpCurrent = 8000;
            pThis->m10_lifeMeterTask = createEnemyLifeMeterTask(&pThis->m1C_lifeMeterPosition, 0, &pThis->mEE_hpCurrent, 0x60);
            pThis->m308_variantIndex = 3;
        }
        // Variants 1 and 3 create an attack sub-task; variant 0 and other values skip it
        if (variant == 1 || variant == 3)
        {
            pThis->m314_attackSubTask = BTL_X0_createAttackSubTask(pThis, pThis->m1A8_attackDataBuffer);
            pThis->m304_state = 0xF;
        }
    }

    pThis->m26C_scale.m0_X = 0x10000;
    pThis->m26C_scale.m4_Y = 0x10000;
    pThis->m26C_scale.m8_Z = 0x10000;
    pThis->m30C_flag0 = 0;
    pThis->m30D_flag1 = 0;
    pThis->m30E_flag2 = 1;
    pThis->m30F_flag3 = 0;
    pThis->m310_attackActive = 0;

    initBattleEnemyTargetables(pThis, 3, pThis->m98_models);

    return pThis;
}

// 06055918
static void BTL_X0_enemyModel2_Update(sBTL_X0_EnemyModel* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    pThis->m1C_lifeMeterPosition.m0_X = pEngine->mC_battleCenter.m0_X + pThis->m7C_position.m0_X;
    pThis->m1C_lifeMeterPosition.m4_Y = pEngine->mC_battleCenter.m4_Y + pThis->m7C_position.m4_Y;
    pThis->m1C_lifeMeterPosition.m8_Z = pEngine->mC_battleCenter.m8_Z + pThis->m7C_position.m8_Z;

    if (pThis->m98_models.size() > 1)
        stepAnimation(&pThis->m98_models[1]);

    Unimplemented();
}

// 06055be8
static void BTL_X0_enemyModel2_Draw(sBTL_X0_EnemyModel* pThis)
{
    pushCurrentMatrix();
    translateCurrentMatrix(&pThis->m1C_lifeMeterPosition);
    scaleCurrentMatrixRow0(pThis->m26C_scale.m0_X);
    scaleCurrentMatrixRow1(pThis->m26C_scale.m4_Y);
    scaleCurrentMatrixRow2(pThis->m26C_scale.m8_Z);

    if (pThis->m98_models.size() > 0 && pThis->m98_models[0].m18_drawFunction)
    {
        pThis->m98_models[0].m18_drawFunction(&pThis->m98_models[0]);
    }
    popMatrix();

    if (pThis->m98_models.size() > 1 && pThis->m98_models[1].m18_drawFunction)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->m1C_lifeMeterPosition);
        scaleCurrentMatrixRow0(pThis->m26C_scale.m0_X);
        scaleCurrentMatrixRow1(pThis->m26C_scale.m4_Y);
        scaleCurrentMatrixRow2(pThis->m26C_scale.m8_Z);
        pThis->m98_models[1].m18_drawFunction(&pThis->m98_models[1]);
        popMatrix();
    }
}

// 06055750
p_workArea BTL_X0_createEnemyModels2(s_workArea* pFormation)
{
    static const sBTL_X0_EnemyModel::TypedTaskDefinition def = {
        nullptr,
        &BTL_X0_enemyModel2_Update,
        &BTL_X0_enemyModel2_Draw,
        &BTL_X0_enemyModel_Delete,
    };
    sBTL_X0_EnemyModel* pThis = createSubTaskWithCopy<sBTL_X0_EnemyModel>((s_workAreaCopy*)pFormation, &def);
    if (pThis == nullptr)
        return nullptr;

    s_fileBundle* pBundle = dramAllocatorEnd[8].mC_fileBundle->m0_fileBundle;
    pThis->m0_fileBundle = pBundle;
    pThis->m8_parentFormation = pFormation;
    pThis->m30B_subCommand = 0;    // byte at 0x30B
    pThis->m310_attackActive = 0;  // byte at 0x310

    pThis->m98_models.resize(2);

    // Variant 2 uses a different creature (2-model setup) — body with hotpoints + secondary sub-model
    sModelHierarchy* pHierarchy0 = pBundle->getModelHierarchy(4);
    u32 numBones0 = pHierarchy0->countNumberOfBones();
    {
        sSaturnPtr hotspotEA = g_BTL_X0->getSaturnPtr(0x060b6be0);
        pThis->m98_models[0].m_hotpointBundles.reserve(numBones0);
        for (u32 b = 0; b < numBones0; b++)
            pThis->m98_models[0].m_hotpointBundles.emplace_back(hotspotEA + b * 8);

        // Model 0 animation selected from short table at 060b6cd8 indexed by m308_variantIndex (0 at this point)
        u16 anim0Offset = readSaturnU16(g_BTL_X0->getSaturnPtr(0x060b6cd8) + (s8)pThis->m308_variantIndex * 2);
        init3DModelRawData(pThis, &pThis->m98_models[0], 0, pBundle, 4,
            pBundle->getAnimation(anim0Offset), pBundle->getStaticPose(0xD8, numBones0), nullptr,
            pThis->m98_models[0].m_hotpointBundles.data());
    }
    stepAnimation(&pThis->m98_models[0]);

    // Model 1: secondary sub-model for this variant (unused by the flag1 swap in variant-2 draw)
    sModelHierarchy* pHierarchy1 = pBundle->getModelHierarchy(8);
    u32 numBones1 = pHierarchy1->countNumberOfBones();
    init3DModelRawData(pThis, &pThis->m98_models[1], 0, pBundle, 8,
        pBundle->getAnimation(0xF4), pBundle->getStaticPose(0xDC, numBones1), nullptr, nullptr);
    stepAnimation(&pThis->m98_models[1]);

    BTL_X0_updateTargetableQuadrants(pThis, 1);

    pThis->mEC_hpMax = 0x1964;
    pThis->mEE_hpCurrent = 0x1964;
    pThis->m10_lifeMeterTask = createEnemyLifeMeterTask(&pThis->m1C_lifeMeterPosition, 0, &pThis->mEE_hpCurrent, 0x5F);
    pThis->m308_variantIndex = 2;

    pThis->m26C_scale.m0_X = 0x10000;
    pThis->m26C_scale.m4_Y = 0x10000;
    pThis->m26C_scale.m8_Z = 0x10000;

    displayFormationName(0x1F, 0, 0xB);

    // Allocate 0xB4-byte attack-data buffer and zero-init
    pThis->m1A8_attackDataBuffer = allocateHeapForTask(pThis, 0xB4);
    if (pThis->m1A8_attackDataBuffer)
        memset(pThis->m1A8_attackDataBuffer, 0, 0xB4);

    pThis->m304_state = 0;
    pThis->m306_dangerQuadrant = 4;

    pThis->m30F_flag3 = 1;

    playPCM(pThis, 0x6C);

    return pThis;
}
