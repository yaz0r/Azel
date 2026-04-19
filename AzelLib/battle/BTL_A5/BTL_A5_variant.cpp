#include "PDS.h"
#include "BTL_A5_variant.h"
#include "BTL_A5_data.h"
#include "BTL_A5_enemy.h"
#include "BTL_A5_formation.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "battle/battleEngine.h"
#include "battle/battleEngineSub0.h"
#include "battle/battleEnemyLifeMeter.h"
#include "battle/battleTargetable.h"
#include "battle/battleDamageDisplay.h"
#include "battle/battleDebris.h"
#include "battle/battleTextDisplay.h"
#include "battle/BTL_A3/baldor.h"
#include "kernel/graphicalObject.h"
#include "kernel/animation.h"
#include "kernel/fileBundle.h"
#include "kernel/vdp1AnimatedQuad.h"
#include "mainMenuDebugTasks.h"
#include "audio/systemSounds.h"
#include "town/town.h"

void setupConditionalLightColor(int param_1);
void clearLightColor();
void battleEngine_UpdateSub1Sub0(s32 param_1);
void initBattleTargetables(p_workArea pThis, s_3dModel* pModel, s16* param3, std::vector<sBattleTargetable>& param4, std::vector<sVec3_FP>& param5);

struct sBTL_A5_VariantEnemy : public s_workAreaTemplateWithCopy<sBTL_A5_VariantEnemy>
{
    s_fileBundle* m0_fileBundle;
    u32 m14_flags;
    s32 m1C_entityIndex;
    s16 m24_counter1;
    s16 m26_counter2;
    u8 m28_state;
    u8 m29_modelCount;
    std::vector<s_3dModel> m2C_models;
    sVec3_FP m30_position;
    sVec3_FP m3C_rotation;
    sVec3_FP m48;
    u8 m54_pad[0x60 - 0x54];
    sVec3_FP m60_targetPosition;
    u8 m6C_pad[0x84 - 0x6C];
    sVec3_FP m84_displayRotation;
    sVec3_FP m90_initialPosition;
    sVec3_FP m9C_targetRotation;
    s32 mA8_soundEffectId;
    sEnemyLifeMeterTask* mAC_lifeMeter;
    s16 mB0;
    s16 mB2_health;
    s16 mB4;
    s16 mB6_numTargetables;
    std::vector<sBattleTargetable> mC0_targetables;
    std::vector<sVec3_FP> mC4_targetablePositions;
    u8 mCC_pad[0x14C - 0xCC];
    s16 m14C_lightColor;
    u8 m14E_pad[0x158 - 0x14E];
    sBTL_A5_FormationTask* m158_formation;
    s_3dModel m160_collisionModel;
    s32 m16C_isLead;
    u8 m170_pad2[0x1B0 - 0x170];
    sVec3_FP m1B0_shadowPosition;
    u8 m1BC_pad3[0x1D5 - 0x1BC];
    u8 m1D5_wasDamaged;
    // size 0x1E4
};

// 06055488
static void BTL_A5_enemyVariant_Draw(sBTL_A5_VariantEnemy* pThis)
{
    // shared main model draw
    if (pThis->m14_flags & 0x800000)
    {
        setupConditionalLightColor(pThis->m14C_lightColor);
    }
    for (int i = 0; i < pThis->m29_modelCount; i++)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->m30_position);
        rotateCurrentMatrixYXZ(&pThis->m3C_rotation);
        s_3dModel* pModel = &pThis->m2C_models[i];
        pModel->m18_drawFunction(pModel);
        popMatrix();
    }
    if (pThis->m14_flags & 0x800000)
    {
        pThis->m14_flags &= ~0x800000;
        clearLightColor();
    }

    // shadow/collision model
    pThis->m1B0_shadowPosition.m0_X = pThis->m30_position.m0_X;
    pThis->m1B0_shadowPosition.m4_Y = 0;
    pThis->m1B0_shadowPosition.m8_Z = pThis->m30_position.m8_Z + 0x1000;
    pushCurrentMatrix();
    translateCurrentMatrix(&pThis->m1B0_shadowPosition);
    scaleCurrentMatrixRow0(0x28000);
    scaleCurrentMatrixRow1(0x28000);
    scaleCurrentMatrixRow2(0x28000);
    pThis->m160_collisionModel.m18_drawFunction(&pThis->m160_collisionModel);
    popMatrix();
}

// 06055086
static void BTL_A5_enemyVariant_Update(sBTL_A5_VariantEnemy* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;

    // damage check
    if (!(pThis->m14_flags & 0x10000000))
    {
        bool damaged = false;
        for (auto& t : pThis->mC0_targetables)
        {
            if (t.m50_flags & 0x80000)
            {
                t.m50_flags &= ~0x80000;
                t.m50_flags &= ~0x20000;
                damaged = true;
                pThis->mB2_health -= t.m58;
                if (pThis->mB2_health >= 0)
                    createHitSparkEffect(pThis, t, 0);
                if (pThis->mAC_lifeMeter)
                    ((u8*)pThis->mAC_lifeMeter)[0x31] |= 2;
            }
        }
        if (damaged)
        {
            pThis->m14_flags &= ~0x04000000;
            pThis->m14_flags &= ~0x02000000;
            if (pThis->mB2_health < 1)
            {
                playSystemSoundEffect(0x66);
                // cleanup targetables
                pThis->m14_flags |= 0x80000000;
                pThis->m14_flags &= ~0x40000000;
                if (!(pThis->m14_flags & 0x10000000))
                {
                    pThis->m14_flags |= 0x10000000;
                    for (auto& t2 : pThis->mC0_targetables)
                        deleteTargetable(&t2);
                }
                if (pThis->mAC_lifeMeter)
                {
                    ((u8*)pThis->mAC_lifeMeter)[0x31] |= 1;
                    ((u8*)pThis->mAC_lifeMeter)[0x31] |= 8;
                }
                pThis->m28_state = 9;
            }
            else
            {
                pThis->m1D5_wasDamaged = 1;
                playSystemSoundEffect(0x65);
                if (!(pThis->m14_flags & 4))
                {
                    initAnimation(&pThis->m2C_models[0], pThis->m0_fileBundle->getAnimation(readSaturnU16(g_BTL_A5->getSaturnPtr(0x060ad5c0) + 4)));
                    pThis->m14_flags |= 4;
                    pThis->m28_state = 6;
                }
            }
        }
    }

    // auto-scroll position
    pThis->m30_position.m0_X = pThis->m30_position.m0_X + pEngine->m1A0_battleAutoScrollDelta.m0_X;
    pThis->m30_position.m4_Y = pThis->m30_position.m4_Y + pEngine->m1A0_battleAutoScrollDelta.m4_Y;
    pThis->m30_position.m8_Z = pThis->m30_position.m8_Z + pEngine->m1A0_battleAutoScrollDelta.m8_Z;

    // state machine
    switch (pThis->m28_state)
    {
    case 0:
        stepAnimation(&pThis->m2C_models[0]);
        if (pThis->m14_flags & 0x04000000)
            pThis->m28_state = 3;
        else if (pThis->m14_flags & 1)
            pThis->m28_state = 1;
        else if (pThis->m14_flags & 2)
            pThis->m28_state = 2;
        break;
    case 1:
        pThis->m158_formation->mF4_entityPositions[pThis->m1C_entityIndex]++;
        if (pThis->m158_formation->mF4_entityPositions[pThis->m1C_entityIndex] > 3)
            pThis->m158_formation->mF4_entityPositions[pThis->m1C_entityIndex] = 0;
        pThis->m14_flags &= ~1;
        pThis->m28_state = 0;
        break;
    case 2:
        pThis->m158_formation->mF4_entityPositions[pThis->m1C_entityIndex]--;
        if ((s8)pThis->m158_formation->mF4_entityPositions[pThis->m1C_entityIndex] < 0)
            pThis->m158_formation->mF4_entityPositions[pThis->m1C_entityIndex] = 3;
        pThis->m14_flags &= ~2;
        pThis->m28_state = 0;
        break;
    case 3:
        stepAnimation(&pThis->m2C_models[0]);
        if ((pThis->m2C_models[0].m16_previousAnimationFrame >> 1) == 0)
        {
            initAnimation(&pThis->m2C_models[0], pThis->m0_fileBundle->getAnimation(readSaturnU16(g_BTL_A5->getSaturnPtr(0x060ad5c0) + 8)));
            pThis->m28_state = 4;
        }
        break;
    case 4:
    {
        s16 frame = stepAnimation(&pThis->m2C_models[0]);
        if (frame == 0x2E)
        {
            playSystemSoundEffect(0x6D);
            pThis->m28_state = 5;
        }
        break;
    }
    case 5:
    {
        s16 frame = stepAnimation(&pThis->m2C_models[0]);
        s32 totalFrames = pThis->m2C_models[0].m30_pCurrentAnimation ? (s16)*(u16*)((u8*)pThis->m2C_models[0].m30_pCurrentAnimation + 4) : 0;
        if (frame == totalFrames - 1)
        {
            initAnimation(&pThis->m2C_models[0], pThis->m0_fileBundle->getAnimation(readSaturnU16(g_BTL_A5->getSaturnPtr(0x060ad5c0))));
            pThis->m28_state = 0;
        }
        break;
    }
    case 6:
    {
        s16 frame = stepAnimation(&pThis->m2C_models[0]);
        if (frame == 0x2B)
        {
            pThis->m158_formation->mF4_entityPositions[pThis->m1C_entityIndex] = pEngine->m22C_dragonCurrentQuadrant;
            pThis->m28_state = 8;
        }
        break;
    }
    case 7:
    case 8:
    {
        s16 frame = stepAnimation(&pThis->m2C_models[0]);
        s32 totalFrames = pThis->m2C_models[0].m30_pCurrentAnimation ? (s16)*(u16*)((u8*)pThis->m2C_models[0].m30_pCurrentAnimation + 4) : 0;
        if (frame == totalFrames - 1)
        {
            initAnimation(&pThis->m2C_models[0], pThis->m0_fileBundle->getAnimation(readSaturnU16(g_BTL_A5->getSaturnPtr(0x060ad5c0))));
            stepAnimation(&pThis->m2C_models[0]);
            pThis->m14_flags &= ~4;
            pThis->m28_state = 0;
        }
        break;
    }
    case 9:
        pThis->m28_state = 10;
        break;
    case 10:
    {
        createBoneDebrisExplosion(pThis, &pThis->m2C_models[0],
            &pThis->m30_position, &pThis->m3C_rotation, nullptr, nullptr,
            -44, dramAllocatorEnd[6].mC_fileBundle, 0, 0x1FFFFF, 0, 0x10000, 0);
        pThis->m158_formation->mF4_entityPositions[pThis->m1C_entityIndex] = 4;
        pThis->m158_formation->mEF_deadCount++;
        pThis->getTask()->markFinished();
        break;
    }
    }
}

// 06054df8
static p_workArea BTL_A5_createEnemyVariant(sBTL_A5_FormationTask* pParent, s32 entityIndex)
{
    static const sBTL_A5_VariantEnemy::TypedTaskDefinition def = {
        nullptr,
        &BTL_A5_enemyVariant_Update,
        &BTL_A5_enemyVariant_Draw,
        nullptr, // shared delete handled by cleanup in update state 10
    };

    sBTL_A5_VariantEnemy* pEntity = createSubTaskWithCopy<sBTL_A5_VariantEnemy>((s_workAreaCopy*)pParent, &def);
    if (!pEntity)
        return nullptr;

    s_fileBundle* pBundle = dramAllocatorEnd[0xE].mC_fileBundle->m0_fileBundle;
    pEntity->m0_fileBundle = pBundle;
    pEntity->m158_formation = pParent;
    pEntity->m1C_entityIndex = entityIndex;

    sSaturnPtr modelData = g_BTL_A5->getSaturnPtr(0x060ad5c0);
    s16 enemyTypeId = readSaturnS16(pParent->mE8_dataTable + 0xC);

    pEntity->mAC_lifeMeter = createEnemyLifeMeterTask(
        &pEntity->m30_position, 0, &pEntity->mB2_health, enemyTypeId);

    // read direction from formation data table
    sSaturnPtr formationEA = pParent->mE8_dataTable;
    u8 dirBase = readSaturnU8(formationEA + 0x11);
    sSaturnPtr dirTable = g_BTL_A5->getSaturnPtr(0x060ad5b8);
    u8 dir = readSaturnU8(dirTable + entityIndex + dirBase * 3);

    // create 3D model
    pEntity->m29_modelCount = 1;
    pEntity->m2C_models.resize(1);

    u16 modelOffset = readSaturnU16(modelData);
    u16 poseOffset = readSaturnU16(modelData + 2);
    sSaturnPtr hotspotEA = readSaturnEA(modelData + 4);

    sModelHierarchy* pHierarchy = pBundle->getModelHierarchy(modelOffset);
    int numBones = pHierarchy->countNumberOfBones();

    sHotpointBundle* pHotSpotsData = nullptr;
    if (!hotspotEA.isNull())
    {
        pEntity->m2C_models[0].m_hotpointBundles.reserve(numBones);
        for (int b = 0; b < numBones; b++)
            pEntity->m2C_models[0].m_hotpointBundles.emplace_back(hotspotEA + b * 8);
        pHotSpotsData = pEntity->m2C_models[0].m_hotpointBundles.data();
    }

    s32 result = init3DModelRawData(pEntity, &pEntity->m2C_models[0], 0, pBundle, modelOffset, nullptr,
                                     pBundle->getStaticPose(poseOffset, numBones), nullptr, pHotSpotsData);
    if (result == 0)
    {
        pEntity->getTask()->markFinished();
        return nullptr;
    }
    stepAnimation(&pEntity->m2C_models[0]);

    // collision model
    init3DModelRawData(pEntity, &pEntity->m160_collisionModel, 0, pBundle, 8, nullptr,
                       pBundle->getStaticPose(readSaturnU16(g_BTL_A5->getSaturnPtr(0x060ad5c0) + 0x44 - 0x48 + 4), numBones), nullptr, nullptr);
    stepAnimation(&pEntity->m160_collisionModel);

    // init animation
    initAnimation(&pEntity->m2C_models[0],pBundle->getAnimation(modelOffset));

    // init targeting
    initBattleTargetables(pEntity, &pEntity->m2C_models[0], &pEntity->mB6_numTargetables,
                          pEntity->mC0_targetables, pEntity->mC4_targetablePositions);

    // rotation from direction table
    sSaturnPtr rotTable = g_BTL_A5->getSaturnPtr(0x060ad5a4);
    pEntity->m3C_rotation.m0_X = 0;
    pEntity->m3C_rotation.m4_Y = readSaturnS32(rotTable + dir * 4);
    pEntity->m3C_rotation.m8_Z = 0;
    pEntity->m84_displayRotation = pEntity->m3C_rotation;
    pEntity->m9C_targetRotation = pEntity->m3C_rotation;

    // position from formation table
    sSaturnPtr posTable = readSaturnEA(formationEA + 8);
    pEntity->m90_initialPosition = readSaturnVec3(posTable + entityIndex * 0xC);
    pEntity->m60_targetPosition = pEntity->m90_initialPosition;

    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    pEntity->m30_position.m0_X = pEntity->m90_initialPosition.m0_X + pEngine->mC_battleCenter.m0_X;
    pEntity->m30_position.m4_Y = pEntity->m90_initialPosition.m4_Y + pEngine->mC_battleCenter.m4_Y;
    pEntity->m30_position.m8_Z = pEntity->m90_initialPosition.m8_Z + pEngine->mC_battleCenter.m8_Z;

    pEntity->mA8_soundEffectId = 0x1999;
    return pEntity;
}

// 060546fc
static void BTL_A5_formationVariant_Init(sBTL_A5_FormationTask* pThis)
{
    allocateNPC(pThis, 0xE);
    pThis->mEC_formationNameIndex = 0x13;
    gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m2 = 0x78;
    battleEngine_UpdateSub1Sub0(0);
    displayFormationName(0xE, 1, 9);

    u32 r = randomNumber();
    if ((r & 1) == 0)
    {
        pThis->mE8_dataTable = g_BTL_A5->getSaturnPtr(0x060ad13c);
    }
    else
    {
        pThis->mE8_dataTable = g_BTL_A5->getSaturnPtr(0x060ad160);
    }

    pThis->mEE_numEntities = 3;
    pThis->mEF_deadCount = 0;
    pThis->mE0_entityArray = (p_workArea*)allocateHeapForTask(pThis, pThis->mEE_numEntities * sizeof(p_workArea));

    for (int i = 0; i < pThis->mEE_numEntities; i++)
    {
        pThis->mE0_entityArray[i] = BTL_A5_createEnemyVariant(pThis, i);
        if (pThis->mE0_entityArray[i] == nullptr)
            pThis->mEF_deadCount++;
    }
}

// 0605493c
static void BTL_A5_formationVariant_Update(sBTL_A5_FormationTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    pEngine->m3B2_numBattleFormationRunning++;

    for (int i = 0; i < pThis->mEE_numEntities; i++)
    {
        p_workArea entity = pThis->mE0_entityArray[i];
        if (entity == nullptr) continue;
        if (entity->getTask()->m14_flags & TASK_FLAGS_FINISHED)
            pThis->mE0_entityArray[i] = nullptr;
    }

    if (pThis->mEF_deadCount >= pThis->mEE_numEntities)
    {
        pThis->m28_state = 0xB;
        pEngine->m3CC->m8 = 0;
        pEngine->m3CC->m0 = 0;
        pEngine->m3CC->m2 = 0;
        pEngine->m3CC->m4 = 0;
        pThis->m1D4_flag = 1;
    }

    switch (pThis->m28_state)
    {
    case 0:
        if (battleEngine_isBattleIntroFinished())
        {
            battleEngine_SetBattleMode(eBattleModes::m7);
            pThis->m28_state = 1;
        }
        break;
    case 1:
        if (pEngine->m188_flags.m2000)
            pThis->m28_state = 2;
        break;
    case 2:
    {
        s16 c = pThis->m24_counter1++;
        if (c > 0x54)
        {
            pThis->m24_counter1 = 0;
            pEngine->m188_flags.m100_attackAnimationFinished = 1;
            pThis->m28_state = 4;
        }
        break;
    }
    case 3:
        if (battleEngine_isPlayerTurnActive()) return;
        break;
    case 4:
        if (!pEngine->m188_flags.m400000)
        {
            pThis->m28_state = 3;
            pEngine->m3CC->m8 = 0;
            pEngine->m3CC->m0 = 0;
        }
        break;
    case 0xB:
    {
        s16 c = pThis->m26_counter2++;
        if (c > 0x4B)
        {
            pThis->m26_counter2 = 0;
            decreaseNPCRefCount(0xE);
            pThis->getTask()->markFinished();
        }
        break;
    }
    default:
        pThis->m28_state = 0xB;
        break;
    }
}

// 060547da
void BTL_A5_createFormation_variant(s_workAreaCopy* pParent, u32 arg0)
{
    static const sBTL_A5_FormationTask::TypedTaskDefinition def = {
        nullptr,
        &BTL_A5_formationVariant_Update,
        nullptr,
        nullptr,
    };

    sBTL_A5_FormationTask* pTask = createSubTaskWithCopy<sBTL_A5_FormationTask>(pParent, &def);
    if (!pTask) return;

    if (gBattleManager->m6_subBattleId == 9)
        pTask->m28_state = 0;
    else
        pTask->m28_state = 3;

    BTL_A5_formationVariant_Init(pTask);
    pTask->mA4_soundEffectId = 0xA3D;
}
