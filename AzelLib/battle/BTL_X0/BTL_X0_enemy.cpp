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
        pThis->m1B8_targetPosition = pThis->m7C_position;

        if (pThis->m30E_flag2 == 1)
        {
            pThis->m1D0_randomOffset.m0_X = (randomNumber() % 0x14000) - 0xA000;
            pThis->m1D0_randomOffset.m4_Y = (randomNumber() % 0x14000) - 0xA000;
            if (pThis->m308_variantIndex == 2)
                pThis->m1D0_randomOffset.m8_Z = (randomNumber() % 0x14000) - 0x19000;
            else
                pThis->m1D0_randomOffset.m8_Z = (randomNumber() % 0x14000) - 0xA000;
            pThis->m1E4_timer = 0x5A;
        }
        else
        {
            pThis->m1D0_randomOffset.m0_X = 0;
            pThis->m1D0_randomOffset.m4_Y = 0;
            pThis->m1D0_randomOffset.m8_Z = 0;
            pThis->m1E4_timer = 0x3C;
        }

        sSaturnPtr baseTable = g_BTL_X0->getSaturnPtr(0x060b7f18) + (s8)(pThis->m308_variantIndex * 0xC);
        pThis->m1D0_randomOffset.m0_X += readSaturnS32(baseTable);
        pThis->m1D0_randomOffset.m4_Y += readSaturnS32(baseTable + 4);
        pThis->m1D0_randomOffset.m8_Z += readSaturnS32(baseTable + 8);

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
        pThis->m1B8_targetPosition = pThis->m7C_position;
        pThis->m1D0_randomOffset.m0_X = (randomNumber() % 0x14000) - 0xA000;
        pThis->m1D0_randomOffset.m4_Y = (randomNumber() % 0x14000) - 0xA000;
        pThis->m1D0_randomOffset.m8_Z = (randomNumber() % 0x14000) - 0xA000;
        pThis->m1E4_timer = 0x5A;
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

    BTL_X0_updateBodyAnimation(pThis);
    if (pThis->m98_models.size() > 1)
        stepAnimation(&pThis->m98_models[1]);
    if (pThis->m98_models.size() > 2)
        stepAnimation(&pThis->m98_models[2]);

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
    pushCurrentMatrix();
    translateCurrentMatrix(&pThis->m1C_lifeMeterPosition);
    scaleCurrentMatrixRow0(pThis->m268_scale.m0_X);
    scaleCurrentMatrixRow1(pThis->m268_scale.m4_Y);
    scaleCurrentMatrixRow2(pThis->m268_scale.m8_Z);

    if (pThis->m98_models.size() > 0 && pThis->m98_models[0].m18_drawFunction)
    {
        pThis->m98_models[0].m18_drawFunction(&pThis->m98_models[0]);
    }
    popMatrix();

    if (pThis->m98_models.size() > 2 && pThis->m98_models[2].m18_drawFunction)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->m1C_lifeMeterPosition);
        scaleCurrentMatrixRow0(pThis->m268_scale.m0_X);
        scaleCurrentMatrixRow1(pThis->m268_scale.m4_Y);
        scaleCurrentMatrixRow2(pThis->m268_scale.m8_Z);
        pThis->m98_models[2].m18_drawFunction(&pThis->m98_models[2]);
        popMatrix();
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
            nullptr, pBundle->getStaticPose(0x1F4, numBones0), nullptr, pHotspots);
    }
    stepAnimation(&pThis->m98_models[0]);

    sModelHierarchy* pHierarchy1 = pBundle->getModelHierarchy(0xC);
    u32 numBones1 = pHierarchy1->countNumberOfBones();
    init3DModelRawData(pThis, &pThis->m98_models[1], 0, pBundle, 0xC,
        nullptr, pBundle->getStaticPose(0x1FC, numBones1), nullptr, nullptr);
    stepAnimation(&pThis->m98_models[1]);

    sModelHierarchy* pHierarchy2 = pBundle->getModelHierarchy(8);
    u32 numBones2 = pHierarchy2->countNumberOfBones();
    init3DModelRawData(pThis, &pThis->m98_models[2], 0, pBundle, 8,
        nullptr, pBundle->getStaticPose(0x1F8, numBones2), nullptr, nullptr);
    stepAnimation(&pThis->m98_models[2]);

    if (variant == 0)
    {
        pThis->mEC_hpMax = 3000;
        pThis->mEE_hpCurrent = 3000;
        pThis->m10_lifeMeterTask = createEnemyLifeMeterTask(&pThis->m1C_lifeMeterPosition, 0, &pThis->mEE_hpCurrent, 0x24);
        pThis->m308_variantIndex = 0;
        displayFormationName(0, 0, 0xB);
        pThis->m304_state = 5;
    }
    else if (variant == 1)
    {
        pThis->mEC_hpMax = 6000;
        pThis->mEE_hpCurrent = 6000;
        pThis->m10_lifeMeterTask = createEnemyLifeMeterTask(&pThis->m1C_lifeMeterPosition, 0, &pThis->mEE_hpCurrent, 0x33);
        pThis->m308_variantIndex = 1;
        displayFormationName(0, 0, 0xB);
        pThis->m304_state = 0xF;
    }
    else if (variant == 3)
    {
        pThis->mEC_hpMax = 8000;
        pThis->mEE_hpCurrent = 8000;
        pThis->m10_lifeMeterTask = createEnemyLifeMeterTask(&pThis->m1C_lifeMeterPosition, 0, &pThis->mEE_hpCurrent, 0x60);
        pThis->m308_variantIndex = 3;
        pThis->m304_state = 0xF;
    }

    pThis->m268_scale.m0_X = 0x10000;
    pThis->m268_scale.m4_Y = 0x10000;
    pThis->m268_scale.m8_Z = 0x10000;
    pThis->m30C_flag0 = 0;
    pThis->m30D_flag1 = 0;
    pThis->m30E_flag2 = 1;
    pThis->m30F_flag3 = 0;
    pThis->m310_attackActive = 0;

    initBattleEnemyTargetables(pThis, 3, pThis->m98_models);

    pThis->m7C_position.m0_X = 0;
    pThis->m7C_position.m4_Y = 0;
    pThis->m7C_position.m8_Z = 0;

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
    scaleCurrentMatrixRow0(pThis->m268_scale.m0_X);
    scaleCurrentMatrixRow1(pThis->m268_scale.m4_Y);
    scaleCurrentMatrixRow2(pThis->m268_scale.m8_Z);

    if (pThis->m98_models.size() > 0 && pThis->m98_models[0].m18_drawFunction)
    {
        pThis->m98_models[0].m18_drawFunction(&pThis->m98_models[0]);
    }
    popMatrix();

    if (pThis->m98_models.size() > 1 && pThis->m98_models[1].m18_drawFunction)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->m1C_lifeMeterPosition);
        scaleCurrentMatrixRow0(pThis->m268_scale.m0_X);
        scaleCurrentMatrixRow1(pThis->m268_scale.m4_Y);
        scaleCurrentMatrixRow2(pThis->m268_scale.m8_Z);
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
    pThis->m30F_flag3 = 0;
    pThis->m310_attackActive = 0;

    pThis->m98_models.resize(2);

    sModelHierarchy* pHierarchy0 = pBundle->getModelHierarchy(4);
    u32 numBones0 = pHierarchy0->countNumberOfBones();
    {
        sSaturnPtr hotspotEA = g_BTL_X0->getSaturnPtr(0x060b6be0);
        pThis->m98_models[0].m_hotpointBundles.reserve(numBones0);
        for (u32 b = 0; b < numBones0; b++)
            pThis->m98_models[0].m_hotpointBundles.emplace_back(hotspotEA + b * 8);
        init3DModelRawData(pThis, &pThis->m98_models[0], 0, pBundle, 4,
            nullptr, pBundle->getStaticPose(0xD8, numBones0), nullptr,
            pThis->m98_models[0].m_hotpointBundles.data());
    }
    stepAnimation(&pThis->m98_models[0]);

    sModelHierarchy* pHierarchy1 = pBundle->getModelHierarchy(8);
    u32 numBones1 = pHierarchy1->countNumberOfBones();
    init3DModelRawData(pThis, &pThis->m98_models[1], 0, pBundle, 8,
        nullptr, pBundle->getStaticPose(0xDC, numBones1), nullptr, nullptr);
    stepAnimation(&pThis->m98_models[1]);

    pThis->mEC_hpMax = 0x1964;
    pThis->mEE_hpCurrent = 0x1964;
    pThis->m10_lifeMeterTask = createEnemyLifeMeterTask(&pThis->m1C_lifeMeterPosition, 0, &pThis->mEE_hpCurrent, 0x5F);
    pThis->m308_variantIndex = 2;

    pThis->m268_scale.m0_X = 0x10000;
    pThis->m268_scale.m4_Y = 0x10000;
    pThis->m268_scale.m8_Z = 0x10000;

    displayFormationName(0x1F, 0, 0xB);

    pThis->m304_state = 0;
    pThis->m30E_flag2 = 4;
    pThis->m30F_flag3 = 1;

    initBattleEnemyTargetables(pThis, 2, pThis->m98_models);

    pThis->m7C_position.m0_X = 0;
    pThis->m7C_position.m4_Y = 0;
    pThis->m7C_position.m8_Z = 0;

    playPCM(pThis, 0x6C);

    return pThis;
}
