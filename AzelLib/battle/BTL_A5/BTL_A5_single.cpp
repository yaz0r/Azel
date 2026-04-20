#include "PDS.h"
#include "BTL_A5_single.h"
#include "BTL_A5_data.h"
#include "BTL_A5_formation.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "battle/battleEngine.h"
#include "battle/battleEngineSub0.h"
#include "battle/battleEnemyLifeMeter.h"
#include "battle/battleTargetable.h"
#include "battle/battleDamageDisplay.h"
#include "battle/battleDebris.h"
#include "battle/battleDebug.h"
#include "battle/battleTextDisplay.h"
#include "kernel/graphicalObject.h"
#include "kernel/animation.h"
#include "kernel/fileBundle.h"
#include "mainMenuDebugTasks.h"
#include "audio/systemSounds.h"
#include "town/town.h"

void setupConditionalLightColor(int param_1);
void clearLightColor();
void battleEngine_UpdateSub1Sub0(s32 param_1);
void initBattleTargetables(p_workArea pThis, s_3dModel* pModel, s16* param3, std::vector<sBattleTargetable>& param4, std::vector<sVec3_FP>& param5);
void increaseStatsCount(int statIndex);

struct sBTL_A5_SingleEnemy : public s_workAreaTemplateWithCopy<sBTL_A5_SingleEnemy>
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
    sVec3_FP m48_scaledAutoScroll;
    u8 m54_pad[0x60 - 0x54];
    sVec3_FP m60_targetPosition;
    s32 m6C_deathRotX;
    u8 m70_pad[0x84 - 0x70];
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
    s_3dModel m15C_collisionModel;
    s32 m16C_isLead;
    u8 m170_pad2[0x17C - 0x170];
    s16 m17C_hitFlashCount;
    s16 m17E_hitFlashColor;
    u8 m180_pad3[0x1AC - 0x180];
    sVec3_FP m1AC_shadowPosition;
    u8 m1B8_pad3[0x1BC - 0x1B8];
    s32 m1BC_isLead2;
    // size 0x1C0
};

// 060a3bc0 — shared main model draw
static void BTL_A5_singleEnemy_DrawMain(sBTL_A5_SingleEnemy* pThis)
{
    if (pThis->m14_flags & 0x800000)
        setupConditionalLightColor(pThis->m14C_lightColor);
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
}

// 0605c4cc
static void BTL_A5_singleEnemy_Draw(sBTL_A5_SingleEnemy* pThis)
{
    BTL_A5_singleEnemy_DrawMain(pThis);
    pThis->m1AC_shadowPosition.m0_X = pThis->m30_position.m0_X;
    pThis->m1AC_shadowPosition.m4_Y = 0;
    pThis->m1AC_shadowPosition.m8_Z = pThis->m30_position.m8_Z + 0x1000;
    pushCurrentMatrix();
    translateCurrentMatrix(&pThis->m1AC_shadowPosition);
    pThis->m15C_collisionModel.m18_drawFunction(&pThis->m15C_collisionModel);
    popMatrix();
}

// 060a36d6
static s32 BTL_A5_enemy_damageCheck(sBTL_A5_SingleEnemy* pThis)
{
    Unimplemented();
    return 0;
}

// 060a3b74
static void BTL_A5_enemy_hitReaction(sBTL_A5_SingleEnemy* pThis, void (*callback)(sBTL_A5_SingleEnemy*))
{
    if (!(pThis->m14_flags & 0x40000000))
    {
        pThis->m14_flags |= 0x40000000;
        if (callback)
        {
            callback(pThis);
        }
    }
}

// 0605bdfa
static void BTL_A5_singleEnemy_onDamaged(sBTL_A5_SingleEnemy* pThis)
{
    pThis->m48_scaledAutoScroll[1] = 0x1000;
    pThis->m6C_deathRotX = 0xaaaaaa;
    pThis->m28_state = 3;
}

// 060a23c0
static void FUN_BTL_A5_060a23c0(sVec3_FP* position, s32 param2)
{
    Unimplemented();
}

// 0605c42a
static void BTL_A5_singleEnemy_updateShadow(sBTL_A5_SingleEnemy* pThis)
{
    Unimplemented();
}

// 0605c2bc
static void BTL_A5_singleEnemy_updatePositionFromFormation(sBTL_A5_SingleEnemy* pThis)
{
    s32* formationPositions = *(s32**)(((u8*)pThis->m158_formation) + 8);
    pThis->m60_targetPosition[0] = formationPositions[pThis->m1C_entityIndex * 3 + 0];
    pThis->m60_targetPosition[1] = formationPositions[pThis->m1C_entityIndex * 3 + 1];
    pThis->m60_targetPosition[2] = formationPositions[pThis->m1C_entityIndex * 3 + 2];
    FUN_BTL_A5_060a23c0(&pThis->m30_position, *(s32*)(((u8*)pThis->m158_formation) + 4));
    BTL_A5_singleEnemy_updateShadow(pThis);
}

// 0605c34c
static bool BTL_A5_singleEnemy_applyAutoScrollAndCheckDistance(sBTL_A5_SingleEnemy* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    pThis->m48_scaledAutoScroll[0] = MTH_Mul(0x20000, pEngine->m1A0_battleAutoScrollDelta.m0_X);
    pThis->m48_scaledAutoScroll[1] = MTH_Mul(0x20000, pEngine->m1A0_battleAutoScrollDelta.m4_Y);
    pThis->m48_scaledAutoScroll[2] = MTH_Mul(0x20000, pEngine->m1A0_battleAutoScrollDelta.m8_Z);
    pThis->m30_position[0] = pThis->m30_position[0] + pThis->m48_scaledAutoScroll[0];
    pThis->m30_position[1] = pThis->m30_position[1] + pThis->m48_scaledAutoScroll[1];
    pThis->m30_position[2] = pThis->m30_position[2] + pThis->m48_scaledAutoScroll[2];
    s32* formationCenter = *(s32**)(((u8*)pThis->m158_formation) + 4);
    pThis->m60_targetPosition[0] = pThis->m90_initialPosition[0] + formationCenter[0];
    pThis->m60_targetPosition[1] = pThis->m90_initialPosition[1] + formationCenter[1];
    pThis->m60_targetPosition[2] = pThis->m90_initialPosition[2] + formationCenter[2];
    pThis->m60_targetPosition[0] = pThis->m60_targetPosition[0] - pThis->m30_position[0];
    pThis->m60_targetPosition[1] = pThis->m60_targetPosition[1] - pThis->m30_position[1];
    pThis->m60_targetPosition[2] = pThis->m60_targetPosition[2] - pThis->m30_position[2];
    BTL_A5_singleEnemy_updateShadow(pThis);
    return pThis->m60_targetPosition[2] >= 0;
}

// 0605c7a8
static void BTL_A5_singleEnemy_createGroundEffect(npcFileDeleter* param1, sVec3_FP* param2, sVec3_FP* param3, s32 param4)
{
    Unimplemented();
}

// 060a411c
static void BTL_A5_enemy_debugDisplay(sBTL_A5_SingleEnemy* pThis, const char** nameTable, s32 row)
{
    Unimplemented();
}

// 06061cf0
static void BTL_A5_increaseKillCount()
{
    increaseStatsCount(8);
}

// 060a3750
static void BTL_A5_enemy_cleanup(sBTL_A5_SingleEnemy* pThis, void (*callback)(sBTL_A5_SingleEnemy*))
{
    pThis->m14_flags |= 0x80000000;
    pThis->m14_flags &= ~0x40000000;
    if (callback)
    {
        callback(pThis);
    }
    if (!(pThis->m14_flags & 0x10000000))
    {
        pThis->m14_flags |= 0x10000000;
        for (int i = 0; i < pThis->m29_modelCount; i++)
        {
            s_3dModel* pModel = &pThis->m2C_models[i];
            for (int j = 0; j < pThis->mC0_targetables.size(); j++)
            {
                deleteTargetable(&pThis->mC0_targetables[j]);
            }
        }
    }
}

// 0605be3c
static void BTL_A5_singleEnemy_Update(sBTL_A5_SingleEnemy* pThis)
{
    s32 damaged = BTL_A5_enemy_damageCheck(pThis);
    if (damaged == 1)
    {
        s32 bitMask = shiftLeft32(1, pThis->m1C_entityIndex);
        ((u8*)pThis->m158_formation)[0xF0] |= (u8)bitMask;
        pThis->m14_flags &= ~0x06000000;
        if (pThis->mB2_health < 1)
        {
            BTL_A5_enemy_cleanup(pThis, nullptr);
            playSystemSoundEffect(0x66);
            BTL_A5_increaseKillCount();
            ((u8*)pThis->mAC_lifeMeter)[0x31] |= 1;
            ((u8*)pThis->mAC_lifeMeter)[0x31] |= 8;
            pThis->m28_state = 7;
        }
        else
        {
            playSystemSoundEffect(0x65);
            BTL_A5_enemy_hitReaction(pThis, &BTL_A5_singleEnemy_onDamaged);
        }
    }

    switch (pThis->m28_state)
    {
    case 0:
        stepAnimation(&pThis->m2C_models[0]);
        BTL_A5_singleEnemy_updatePositionFromFormation(pThis);
        if (pThis->m14_flags & 0x04000000)
        {
            pThis->m28_state = 1;
            // 0605c51c: create attack particle based on lead status
            Unimplemented();
            char* formationByte = (char*)((u8*)pThis->m158_formation + 0x12);
            s32 rnd = performModulo2(3, randomNumber());
            *formationByte += (char)rnd;
        }
        break;
    case 1:
        stepAnimation(&pThis->m2C_models[0]);
        BTL_A5_singleEnemy_updatePositionFromFormation(pThis);
        if (pThis->m14_flags & 0x02000000)
        {
            pThis->m28_state = 2;
            // 06054080: create attack effect based on lead status
            Unimplemented();
            playSystemSoundEffect(0x69);
        }
        break;
    case 2:
    {
        stepAnimation(&pThis->m2C_models[0]);
        BTL_A5_singleEnemy_updatePositionFromFormation(pThis);
        s16 c = pThis->m24_counter1++;
        if (c > 0x4B)
        {
            pThis->m24_counter1 = 0;
            pThis->m14_flags &= ~0x02000000;
            pThis->m28_state = 0;
        }
        break;
    }
    case 3:
    {
        pThis->m3C_rotation[0] = pThis->m3C_rotation[0] - pThis->m6C_deathRotX;
        s32 newY = (s32)pThis->m30_position[1] - (s32)pThis->m48_scaledAutoScroll[1];
        pThis->m30_position[1] = newY;
        if (newY < 1)
        {
            pThis->m30_position[1] = 0;
            pThis->m3C_rotation[0] = 0;
            initAnimation(&pThis->m2C_models[0], pThis->m0_fileBundle->getAnimation(0x74));
            stepAnimation(&pThis->m2C_models[0]);
            pThis->m28_state = 4;
        }
        break;
    }
    case 4:
    {
        s16 c = pThis->m26_counter2++;
        if (c > 0x3C)
        {
            pThis->m26_counter2 = 0;
            pThis->m28_state = 5;
            pThis->m14_flags &= ~0x40000000;
            initAnimation(&pThis->m2C_models[0], pThis->m0_fileBundle->getAnimation(0x70));
        }
        BTL_A5_singleEnemy_createGroundEffect(dramAllocatorEnd[6].mC_fileBundle,
            &pThis->m30_position, &pThis->m3C_rotation, 0);
        stepAnimation(&pThis->m2C_models[0]);
        break;
    }
    case 5:
        stepAnimation(&pThis->m2C_models[0]);
        stepAnimation(&pThis->m2C_models[0]);
        stepAnimation(&pThis->m2C_models[0]);
        if (BTL_A5_singleEnemy_applyAutoScrollAndCheckDistance(pThis))
        {
            pThis->m28_state = 6;
            char* formationByte = (char*)((u8*)pThis->m158_formation + 0x12);
            s32 rnd = randomNumber();
            *formationByte += (rnd & 1);
        }
        break;
    case 6:
    {
        stepAnimation(&pThis->m2C_models[0]);
        stepAnimation(&pThis->m2C_models[0]);
        BTL_A5_singleEnemy_updatePositionFromFormation(pThis);
        s16 c = pThis->m26_counter2++;
        if (c > 0xF)
        {
            pThis->m26_counter2 = 0;
            pThis->m28_state = 0;
            s32 bitMask = shiftLeft32(1, pThis->m1C_entityIndex);
            ((u8*)pThis->m158_formation)[0xF0] &= ~(u8)bitMask;
            pThis->m14_flags &= ~0x06000000;
        }
        break;
    }
    case 7:
        pThis->m28_state = 8;
        break;
    case 8:
    {
        s32 bitMask = shiftLeft32(1, pThis->m1C_entityIndex);
        ((u8*)pThis->m158_formation)[0xF0] &= ~(u8)bitMask;
        pThis->m14_flags &= ~0x06000000;
        createBoneDebris(pThis, dramAllocatorEnd[0xC].mC_fileBundle, &pThis->m2C_models[0],
            &pThis->m30_position, &pThis->m3C_rotation, nullptr, nullptr,
            -44, 0x1FFFFF, 0, 0, 0, 0x10000, 0);
        pThis->getTask()->markFinished();
        ((u8*)pThis->m158_formation)[0xF] += 1;
        *(s32*)((u8*)pThis->m158_formation + pThis->m1C_entityIndex * 4 + 0x14) = 1;
        break;
    }
    }

    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x1A] != 0 ||
        gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x1C] != 0)
    {
        BTL_A5_enemy_debugDisplay(pThis, nullptr, pThis->m1C_entityIndex + 0x11);
    }
}

// 0605bbd0
static p_workArea BTL_A5_createEnemySingle(sBTL_A5_FormationTask* pParent, s32 entityIndex, s32 isLead)
{
    static const sBTL_A5_SingleEnemy::TypedTaskDefinition def = {
        nullptr,
        &BTL_A5_singleEnemy_Update,
        &BTL_A5_singleEnemy_Draw,
        nullptr,
    };

    sBTL_A5_SingleEnemy* pEntity = createSubTaskWithCopy<sBTL_A5_SingleEnemy>((s_workAreaCopy*)pParent, &def);
    if (!pEntity)
        return nullptr;

    s_fileBundle* pBundle = dramAllocatorEnd[0xC].mC_fileBundle->m0_fileBundle;
    pEntity->m0_fileBundle = pBundle;
    pEntity->m158_formation = pParent;
    pEntity->m1C_entityIndex = entityIndex;
    pEntity->m1BC_isLead2 = isLead;

    if (isLead == 1)
    {
        pEntity->m17C_hitFlashCount = 0x21;
        pEntity->m17E_hitFlashColor = 0xE;
    }
    else
    {
        pEntity->m17C_hitFlashCount = 0x18;
    }

    s16 enemyTypeId = readSaturnS16(pParent->mE0_entityGroup.m8_dataTable + 0xC) + isLead;
    pEntity->mAC_lifeMeter = createEnemyLifeMeterTask(
        &pEntity->m30_position, 0, &pEntity->mB2_health, enemyTypeId);

    // create model
    sSaturnPtr modelData = g_BTL_A5->getSaturnPtr(0x060b0028 + isLead * 8);
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

    init3DModelRawData(pEntity, &pEntity->m2C_models[0], 0, pBundle, modelOffset, nullptr,
                       pBundle->getStaticPose(poseOffset, numBones), nullptr, pHotSpotsData);
    stepAnimation(&pEntity->m2C_models[0]);

    // collision model
    init3DModelRawData(pEntity, &pEntity->m15C_collisionModel, 0, pBundle, 0xC, nullptr,
                       pBundle->getStaticPose(readSaturnU16(modelData + 2), numBones), nullptr, nullptr);
    stepAnimation(&pEntity->m15C_collisionModel);

    // animation
    initAnimation(&pEntity->m2C_models[0],pBundle->getAnimation(modelOffset));

    // targeting
    initBattleTargetables(pEntity, &pEntity->m2C_models[0], &pEntity->mB6_numTargetables,
                          pEntity->mC0_targetables, pEntity->mC4_targetablePositions);

    // rotation + position
    pEntity->m3C_rotation.m0_X = 0;
    pEntity->m3C_rotation.m4_Y = 0x8000000;
    pEntity->m3C_rotation.m8_Z = 0;
    pEntity->m84_displayRotation = pEntity->m3C_rotation;
    pEntity->m9C_targetRotation = pEntity->m3C_rotation;

    sSaturnPtr posTable = readSaturnEA(pParent->mE0_entityGroup.m8_dataTable + 8);
    pEntity->m90_initialPosition = readSaturnVec3(posTable + entityIndex * 0xC);
    pEntity->m60_targetPosition = pEntity->m90_initialPosition;

    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    pEntity->m30_position.m0_X = pEntity->m90_initialPosition.m0_X + pEngine->mC_battleCenter.m0_X;
    pEntity->m30_position.m4_Y = pEntity->m90_initialPosition.m4_Y + pEngine->mC_battleCenter.m4_Y;
    pEntity->m30_position.m8_Z = pEntity->m90_initialPosition.m8_Z + pEngine->mC_battleCenter.m8_Z;

    pEntity->mA8_soundEffectId = 0x51E;
    return pEntity;
}

// 0605ae48
static void BTL_A5_formationSingle_Init(sBTL_A5_FormationTask* pThis)
{
    allocateNPC(pThis, 0xC);
    pThis->mE0_entityGroup.mC_formationNameIndex = 0x11;
    gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m2 = 0x78;
    battleEngine_UpdateSub1Sub0(0);
    displayFormationName(6, 1, 9);

    pThis->mE0_entityGroup.mE_numEntities = 6;
    pThis->mE0_entityGroup.mF_deadCount = 0;
    pThis->mE0_entityGroup.m0_entityArray = (p_workArea*)allocateHeapForTask(pThis, pThis->mE0_entityGroup.mE_numEntities * sizeof(p_workArea));

    for (int i = 0; i < pThis->mE0_entityGroup.mE_numEntities; i++)
    {
        s32 isLead = (i == 0 || i == 1) ? 1 : 0;
        pThis->mE0_entityGroup.m0_entityArray[i] = BTL_A5_createEnemySingle(pThis, i, isLead);
        if (pThis->mE0_entityGroup.m0_entityArray[i] == nullptr)
            pThis->mE0_entityGroup.mF_deadCount++;
    }
}

// 0605b1a8
static void BTL_A5_formationSingle_Update(sBTL_A5_FormationTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    pEngine->m3B2_numBattleFormationRunning++;

    for (int i = 0; i < pThis->mE0_entityGroup.mE_numEntities; i++)
    {
        p_workArea entity = pThis->mE0_entityGroup.m0_entityArray[i];
        if (entity == nullptr) continue;
        if (entity->getTask()->m14_flags & TASK_FLAGS_FINISHED)
            pThis->mE0_entityGroup.m0_entityArray[i] = nullptr;
    }

    if (pThis->mE0_entityGroup.mF_deadCount >= pThis->mE0_entityGroup.mE_numEntities)
    {
        pThis->m28_state = 0xB;
        pEngine->m3CC->m8 = 0;
        pEngine->m3CC->m0 = 0;
        pEngine->m3CC->m2 = 0;
        pEngine->m3CC->m4 = 0;
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
            decreaseNPCRefCount(0xC);
            pThis->getTask()->markFinished();
        }
        break;
    }
    default:
        pThis->m28_state = 0xB;
        break;
    }
}

// 0605b112
void BTL_A5_createFormation_single(s_workAreaCopy* pParent, u32 arg0)
{
    static const sBTL_A5_FormationTask::TypedTaskDefinition def = {
        nullptr,
        &BTL_A5_formationSingle_Update,
        nullptr,
        nullptr,
    };

    sBTL_A5_FormationTask* pTask = createSubTaskWithCopy<sBTL_A5_FormationTask>(pParent, &def);
    if (!pTask) return;

    BTL_A5_formationSingle_Init(pTask);

    battleEngine_FlagQuadrantBitForSafety(1);
    battleEngine_FlagQuadrantBitForDanger(0xE);
    pTask->m2C_posBlock.m78_interpRate = 0xA3D;

    if (gBattleManager->m6_subBattleId == 4)
        pTask->m28_state = 0;
    else
        pTask->m28_state = 3;
}
