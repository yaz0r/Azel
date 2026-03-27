#include "PDS.h"
#include "baldor.h"
#include "battle/battleFormation.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "battle/battleOverlay_C.h"
#include "battle/battleEngine.h"
#include "battle/battleDebug.h"
#include "battle/battleDamageDisplay.h"
#include "battle/battleEnemyLifeMeter.h"
#include "kernel/fileBundle.h"
#include "kernel/animation.h"
#include "kernel/debug/trace.h"
#include "kernel/vdp1Allocator.h"
#include "audio/systemSounds.h"
#include "battle/battleGenericData.h"
#include "BTL_A3.h"
#include "BTL_A3_data.h"

#include "battle/battleDragon.h" // todo: clean by moving s_battleDragon_8C to its own file
#include "mainMenuDebugTasks.h"
#include "kernel/graphicalObject.h"
#include "battle/particleEffect.h"
#include "battle/battleGrid.h"
#include "battle/battleDebris.h"
#include "field.h" // for s_RGB8

void Baldor_initSub0Sub2(sBaldorBase* pThis, sFormationData* pFormationEntry)
{
    pThis->m34_formationEntry = pFormationEntry;

    pThis->m1C_translation.m0_current = &pFormationEntry->m0_translation.m0_current;
    pThis->m1C_translation.m4_target = &pFormationEntry->m0_translation.mC_target;
    pThis->m1C_translation.m8 = &pFormationEntry->m0_translation.m18;

    pThis->m28_rotation.m0_current = &pFormationEntry->m24_rotation.m0_current;
    pThis->m28_rotation.m4_target = &pFormationEntry->m24_rotation.mC_target;
    pThis->m28_rotation.m8 = &pFormationEntry->m24_rotation.m18;

    pFormationEntry->m48 = 0;
    pFormationEntry->m49 = 0;
}

s_3dModel* Baldor_create3dModel(sBaldorBase* pThis, sSaturnPtr dataPtr, s32 arg)
{
    u8 fileBundleIndex = readSaturnS8(dataPtr);
    s_fileBundle* pFileBundle = dramAllocatorEnd[fileBundleIndex].mC_fileBundle->m0_fileBundle;
    sSaturnPtr animData = readSaturnEA(dataPtr + 8) + arg * 8;

    s_3dModel* pOutputModel = new s_3dModel;

    sSaturnPtr temp = readSaturnEA(dataPtr + 4);
    assert(temp.isNull());

    sModelHierarchy* pHierarchy = pFileBundle->getModelHierarchy(readSaturnU16(animData));


    sSaturnPtr hotSpotDataEA = readSaturnEA(animData + 4);
    sHotpointBundle* pHotSpotsData = nullptr;
    //assert(animData == hotSpotDataEA); // else we need to load the data
    if (!hotSpotDataEA.isNull()) {
        int numBones = pHierarchy->countNumberOfBones();
        pOutputModel->m_hotpointBundles.reserve(numBones);
        for (int i = 0; i < numBones; i++) {
            pOutputModel->m_hotpointBundles.emplace_back(hotSpotDataEA + i * 8);
        }
        pHotSpotsData = pOutputModel->m_hotpointBundles.data();
    }

    init3DModelRawData(pThis, pOutputModel, 0, pFileBundle, readSaturnU16(animData), 0, pFileBundle->getStaticPose(readSaturnU16(animData + 2), pHierarchy->countNumberOfBones()), nullptr, pHotSpotsData);

    return pOutputModel;
}

void initBattleTargetables(p_workArea pThis, s_3dModel* pModel, s16* param3, std::vector<sBattleTargetable>& param4, std::vector<sVec3_FP>& param5)
{
    if(pModel->m40 == nullptr)
    {
        *param3 = 0;
        param4.clear();
        param5.clear();
    }
    else
    {
        int count = 0;
        for (int i=0; i<pModel->m12_numBones; i++)
        {
            count += (*pModel->m40)[i].m4_count;
        }

        *param3 = count;
        param4.resize(count);
        param5.resize(count);

        int currentEntryIndex = 0;

        for (int i=0; i<pModel->m12_numBones; i++)
        {
            if (pModel->m44_hotpointData[i].size())
            {
                for (u32 j = 0; j < (*pModel->m40)[i].m4_count; j++)
                {
                    param4[currentEntryIndex].m4_pPosition = nullptr;

                    s_hotpoinEntry& puVar1 = (*pModel->m40)[i].m0[j];

                    initTargetable(&param4[currentEntryIndex], nullptr, &param5[currentEntryIndex], puVar1.m10, puVar1.m0, 0, 0, 10);

                    currentEntryIndex++;
                }
            }
        }
    }
}

void Baldor_initSub0(sBaldorBase* pThis, sSaturnPtr dataPtr, sFormationData* pFormationEntry, s32 arg)
{
    pThis->m3C_dataPtr = dataPtr;
    if (readSaturnS8(dataPtr + 2) == 0)
    {
        pThis->m38_3dModel = nullptr;
        pThis->mC_numTargetables = 0;
    }
    else
    {
        pThis->m38_3dModel = Baldor_create3dModel(pThis, dataPtr, 0);
        initBattleTargetables(pThis, pThis->m38_3dModel, &pThis->mC_numTargetables, pThis->m14_targetable, pThis->m18_position);
    }

    Baldor_initSub0Sub2(pThis, pFormationEntry);
    pThis->m40_enemyLifeMeterTask = createEnemyLifeMeterTask(pThis->m1C_translation.m0_current, 0, &pThis->m10_HP, readSaturnS8(pThis->m3C_dataPtr + 1));
    if (-1 < arg)
    {
        u8 bundleIdx = readSaturnS8(dataPtr);
        u32 offset = readSaturnU16(readSaturnEA(dataPtr + 0xC) + arg * 2);

        sAnimationData* pAnimation = dramAllocatorEnd[bundleIdx].mC_fileBundle->m0_fileBundle->getAnimation(offset);

        initAnimation(pThis->m38_3dModel, pAnimation);

        int animationSteps = randomNumber() & 0x1F;
        while (animationSteps)
        {
            stepAnimation(pThis->m38_3dModel);
            animationSteps--;
        }
    }
}

void monsterPart_defaultUpdate(sBaldorBodyPart*, const sVec3_FP*, const sVec3_FP*, const sVec3_FP*)
{
    assert(0);
}

void monsterPart_defaultDraw(sBaldor*, sBaldorBodyPart*)
{
    assert(0);
}

void monsterPart_defaultDelete()
{
    assert(0);
}

void baldorPart_update(sBaldorBodyPart* pThis, const sVec3_FP* pTranslation, const sVec3_FP* pRotation, const sVec3_FP* param4)
{
    pThis->m34_rotationAcceleration += MTH_Mul((*param4 - pThis->m1C_rotation).normalized(), pThis->m44_springStiffness);

    pThis->m34_rotationAcceleration -= MTH_Mul(pThis->m50_damping, pThis->m28_rotationVelocity);

    pThis->m28_rotationVelocity += pThis->m34_rotationAcceleration;
    pThis->m1C_rotation += pThis->m28_rotationVelocity;

    pThis->m34_rotationAcceleration.zeroize();

    sMatrix4x3 pTemp;
    initMatrixToIdentity(&pTemp);
    translateMatrix(*pTranslation, &pTemp);
    rotateMatrixZYX(pRotation, &pTemp);
    translateMatrix(pThis->m10_translation, &pTemp);

    pThis->m4_worldPosition = pTemp.getTranslation();

    addTraceLog(pThis->m10_translation, "pThis->m10");
    addTraceLog(pThis->m1C_rotation, "pThis->m1C_rotation");
    addTraceLog(pThis->m28_rotationVelocity, "pThis->m28_rotationVelocity");
    addTraceLog(pThis->m34_rotationAcceleration, "pThis->m34_rotationAcceleration");
    addTraceLog(pThis->m50_damping, "pThis->m50_damping");
    addTraceLog(pThis->m4_worldPosition, "pThis->m4_worldPosition");

    if (pThis->m0_child)
    {
        baldorPart_update(pThis->m0_child, &pThis->m4_worldPosition, &pThis->m1C_rotation, &pThis->m1C_rotation);
    }
}

void baldorPart_draw(sBaldor* pBaltor, sBaldorBodyPart* pBaltorPart)
{
    while (1)
    {
        if (pBaltorPart->m40_modelIndex > 0)
        {
            pushCurrentMatrix();
            translateCurrentMatrix(pBaltorPart->m4_worldPosition);
            rotateCurrentMatrixZYX(pBaltorPart->m1C_rotation);
            addObjectToDrawList(pBaltor->m0_fileBundle->get3DModel(pBaltorPart->m40_modelIndex));
            popMatrix();
        }
        if (pBaltorPart->m0_child == nullptr)
            break;
        pBaltorPart = pBaltorPart->m0_child;
    }
}

void baldorPart_delete()
{
    assert(0);
}

void Baldor_createBodySub0(sBaldorBody* pData)
{
    pData->m0_translation.zeroize();
    pData->mC_rotation.zeroize();
    pData->m18_rotationTarget.zeroize();
    pData->m24_update = monsterPart_defaultUpdate;
    pData->m28_draw = monsterPart_defaultDraw;
    pData->m2C_delete = monsterPart_defaultDelete;
    pData->m30_parts.resize(0);
}

void Baldor_createBodySub1(sBaldorBodyPart* pEntry, sBaldorBodyPart* pNextEntry)
{
    pEntry->m0_child = pNextEntry;
    pEntry->m4_worldPosition.zeroize();
    pEntry->m10_translation.zeroize();
    pEntry->m1C_rotation.zeroize();
    pEntry->m28_rotationVelocity.zeroize();
    pEntry->m34_rotationAcceleration.zeroize();
    pEntry->m40_modelIndex = 0;
    pEntry->m44_springStiffness.zeroize();
    pEntry->m50_damping = 0;
}

sBaldorBody* Baldor_createBody(p_workArea parent, int numEntries)
{
    sBaldorBody* pNewData = new sBaldorBody;
    Baldor_createBodySub0(pNewData);

    pNewData->m30_parts.resize(numEntries);

    Baldor_createBodySub1(&pNewData->m30_parts[numEntries - 1], nullptr);
    int iVar1 = numEntries - 1;
    while (iVar1 != 0)
    {
        Baldor_createBodySub1(&pNewData->m30_parts[iVar1 - 1], &pNewData->m30_parts[iVar1]);
        iVar1--;
    }

    return pNewData;
}

void Baldor_loadBodyPartDataSub0(sBaldorBodyPart* dest, sSaturnPtr source)
{
    dest->m40_modelIndex = readSaturnS16(source);
    dest->m44_springStiffness = readSaturnVec3(source + 4);
    dest->m50_damping = readSaturnS32(source + 0x10);
}

void Baldor_loadBodyPartData(sBaldorBody* pThis, int arg2, sSaturnPtr arg3)
{
    if (arg2 == 1)
    {
        pThis->m24_update = baldorPart_update;
        pThis->m28_draw = baldorPart_draw;
        pThis->m2C_delete = baldorPart_delete;
    }

    if (!arg3.isNull())
    {
        sBaldorBodyPart* piVar1 = &pThis->m30_parts[0];
        do
        {
            Baldor_loadBodyPartDataSub0(piVar1, arg3);
            piVar1 = piVar1->m0_child;
            arg3 += 0x14;
        } while (piVar1);
    }
}

void Baldor_init(sBaldorBase* pThisBase, sFormationData* pFormationEntry)
{
    sBaldor* pThis = (sBaldor*)pThisBase;

    sSaturnPtr puVar7;
    if ((gBattleManager->m6_subBattleId == 8) || (gBattleManager->m6_subBattleId == 9)) // middle boss  (with queen)
    {
        // use simpler model from the baldor queen file
        puVar7 = g_BTL_A3->getSaturnPtr(0x60a75f0);
    }
    else
    {
        // use the normal detailed model
        puVar7 = g_BTL_A3->getSaturnPtr(0x60a73a0);
    }

    Baldor_initSub0(pThis, puVar7, pFormationEntry, 0);

    pThis->m14_targetable.resize(4);
    pThis->m18_position.resize(4);
    pThis->mC_numTargetables = 4;

    for (int i = 0; i < 4; i++)
    {
        u32 ivar2;
        if (i == 0)
        {
            ivar2 = 0xf0000000;
        }
        else
        {
            ivar2 = 0xf0000002;
        }

        initTargetable(&pThis->m14_targetable[i], nullptr, &pThis->m18_position[i], 0x1000, ivar2, 0, 0, 10);
    }

    *pThis->m28_rotation.m0_current = *pThis->m28_rotation.m4_target;

    pThis->m68_body = Baldor_createBody(pThis, 6);

    if ((gBattleManager->m6_subBattleId == 8) || (gBattleManager->m6_subBattleId == 9))
    {
        // During the Queen baldor fight
        Baldor_loadBodyPartData(pThis->m68_body, 1, g_BTL_A3->getSaturnPtr(0x60a7ed4));
        sSaturnPtr pDataSource = g_BTL_A3->getSaturnPtr(0x60a7f94);

        for (int i = 0; i < 6; i++)
        {
            sBaldorBodyPart& dest = pThis->m68_body->m30_parts[i];

            dest.m10_translation = readSaturnVec3(pDataSource + 0xC * i);
            dest.m1C_rotation[1] = MTH_Mul(0x1c71c71, (randomNumber() & 0x1ffff) - 0xffff) + pThis->m28_rotation.m4_target->m4_Y;
            dest.m1C_rotation[0] = MTH_Mul(0x1c71c71, (randomNumber() & 0x1ffff) - 0xffff);
            dest.m1C_rotation[2] = MTH_Mul(0x1c71c71, (randomNumber() & 0x1ffff) - 0xffff);
        }
    }
    else
    {
        Baldor_loadBodyPartData(pThis->m68_body, 1, g_BTL_A3->getSaturnPtr(0x60a7e5c));
        sSaturnPtr pDataSource = g_BTL_A3->getSaturnPtr(0x60a7f4c);

        for (int i = 0; i < 6; i++)
        {
            sBaldorBodyPart& dest = pThis->m68_body->m30_parts[i];

            dest.m10_translation = readSaturnVec3(pDataSource + 0xC * i);
            dest.m1C_rotation[1] = (*pThis->m28_rotation.m4_target)[1];
        }
    }

    pThis->m6C_oscillationPhase[0] = randomNumber();
    pThis->m6C_oscillationPhase[1] = randomNumber();
    pThis->m6C_oscillationPhase[2] = randomNumber();
}

s32 checkTargetablesForDamage(p_workArea pThis, std::vector<sBattleTargetable>& param2, s16 entriesToParse, s16& param4)
{
    int uVar4 = 0;
    int sVar3 = 0;
    if(entriesToParse > 0)
    {
        std::vector<sBattleTargetable>::iterator uVar5 = param2.begin();
        int sVar1 = 0;
        while (uVar5 - param2.begin() < entriesToParse)
        {
            sVar3 = sVar1;
            if (uVar5->m50_flags & 0x80000)
            {
                uVar4 = 1;
                sVar3 += uVar5->m58;
            }
            uVar5++;
            sVar1 = sVar3;
        }
    }
    param4 = sVar3;
    return uVar4;
}

sVec3_FP* getHitKnockbackDirection(std::vector<sBattleTargetable>& param1, int param2)
{
    sVec3_FP* iVar3 = nullptr;
    for (int i=0; i<param2; i++)
    {
        sBattleTargetable& value = param1[i];
        if (value.m50_flags & 0x80000)
        {
            iVar3 = &value.m34_impactVector;
        }
    }
    return iVar3;
}

void Baldor_updateSub0Sub1(sBaldor* pThis)
{
    pThis->m5C_rotationDelta[0] += (randomNumber() & 0x3fffff) - 0x1fffff;
    pThis->m5C_rotationDelta[1] += (randomNumber() & 0x3fffff) - 0x1fffff;
    pThis->m5C_rotationDelta[2] += (randomNumber() & 0x7fffff) - 0x3fffff;

    sVec3_FP* piVar2 = getHitKnockbackDirection(pThis->m14_targetable, pThis->mC_numTargetables);
    if (piVar2)
    {
        pThis->m50_translationDelta += *piVar2;
    }

    if (isTraceEnabled())
    {
        addTraceLog(pThis->m50_translationDelta, "BaldorTranslationDelta");
    }
}

void CreateDamageSpriteForCurrentBattleOverlay(sVec3_FP* param1, sVec3_FP* param2, s32 param3, s8 param4)
{
    int iVar2;

    switch (param4)
    {
    case 0:
        if (gBattleManager->m2_currentBattleOverlayId == 1)
        {
            iVar2 = 3;
        }
        else
        {
            iVar2 = 2;
        }
        break;
    case 1:
    default:
        if (gBattleManager->m2_currentBattleOverlayId == 1)
        {
            iVar2 = 5;
        }
        else
        {
            iVar2 = 4;
        }
        break;
    case 2:
        if (gBattleManager->m2_currentBattleOverlayId == 1)
        {
            iVar2 = 8;
        }
        else
        {
            iVar2 = 7;
        }
        break;
    }

    createParticleEffect(dramAllocatorEnd[0].mC_fileBundle, &g_BTL_GenericData->m_0x60abef4_animatedQuads[iVar2], param1, param2, 0, param3, 0, 0);
}

// 0605bae8 — implemented in battleDamageDisplay.cpp

void processHitTargetables(p_workArea pThis, std::vector<sBattleTargetable>& param2, int param3, int param4, sEnemyLifeMeterTask* param5)
{
    for (int i = 0; i < param3; i++)
    {
        sBattleTargetable& value = param2[i];
        if (value.m50_flags & 0x80000)
        {
            if (param4)
            {
                createHitSparkEffect(pThis, value, 0);
            }

            value.m50_flags &= ~0x80000;
            value.m50_flags &= ~0x20000;

            CreateDamageSpriteForCurrentBattleOverlay(getBattleTargetablePosition(value), nullptr, 0x30000, 1);
        }
    }

    if (param5)
    {
        param5->m31 |= 2;
    }
}

void Baldor_updateSub0(sBaldor* pThis)
{
    s16 damageValue;

    if (!(pThis->m34_formationEntry->m48 & 4))
    {
        if (checkTargetablesForDamage(pThis, pThis->m14_targetable, pThis->mC_numTargetables, damageValue))
        {
            pThis->mE_damageValue += damageValue;
            pThis->m12_damagePending = 1;
            pThis->mB_flags |= 8;
            Baldor_updateSub0Sub1(pThis);
            pThis->m10_HP -= damageValue;
            if (pThis->m10_HP < 1)
            {
                processHitTargetables(pThis, pThis->m14_targetable, pThis->mC_numTargetables, 0, pThis->m40_enemyLifeMeterTask);
                pThis->m34_formationEntry->m48 |= 4;
                createDamageDisplayTask(pThis, pThis->mE_damageValue, pThis->m1C_translation.m0_current, 1);
                playSystemSoundEffect(0x66);
                pThis->m8_mode = 0xB;
                pThis->m9_attackStatus = 0;
            }
            else
            {
                processHitTargetables(pThis, pThis->m14_targetable, pThis->mC_numTargetables, 1, pThis->m40_enemyLifeMeterTask);
                playSystemSoundEffect(0x65);
            }
        }

        if (pThis->m12_damagePending && (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m1000))
        {
            createDamageDisplayTask(pThis, pThis->mE_damageValue, pThis->m1C_translation.m0_current, 1);
            pThis->mE_damageValue = 0;
            pThis->m12_damagePending = 0;
        }
    }
}

void springDampedStep(sVec3_FP* pCurrent, sVec3_FP* pDelta, sVec3_FP* pTarget, s32 pDeltaFactor, s32 pDistanceToTargetFactor, s8 translationOrRotation)
{
    switch(translationOrRotation)
    {
    case 0:
        if (pDelta != nullptr)
        {
            *pDelta -= MTH_Mul(pDeltaFactor, *pDelta);
            *pDelta += MTH_Mul(pDistanceToTargetFactor, *pTarget - *pCurrent);
            *pCurrent += *pDelta;
        }
        else
        {
            *pCurrent += MTH_Mul(pDistanceToTargetFactor, *pTarget - *pCurrent);
        }
        break;
    case 1:
        *pDelta = (*pDelta - MTH_Mul(pDeltaFactor, *pDelta)).normalized();
        *pDelta = (*pDelta + MTH_Mul(pDistanceToTargetFactor, (*pTarget - *pCurrent).normalized())).normalized();
        *pCurrent = (*pCurrent + *pDelta).normalized();
        break;
    default:
        assert(0);
    }
}

struct sBaldorAttack : public s_workAreaTemplate<sBaldorAttack>
{
    sBaldorBody* m0;
    s16 m4_delay;
    s16 m6_baldorPartEmittingAttack;
    s16 m8_numAttackRotation;
    u8 mA_state;
    sBaldor* mC;

    // size: 0x10
};

// 060a4428
struct sBaldorProjectile : public s_workAreaTemplateWithCopy<sBaldorProjectile>
{
    sVec3_FP m8_position;
    sVec3_FP m14_velocity;
    sVec3_FP m20_acceleration;
    void* m2C_visual; // sAnimatedQuad* or s_3dModel* depending on m30
    u8 m30_isModelType;
    s16 m32_timer;
    s16 m34_damageValue;
    u8 m36_damageType;
    sSaturnPtr m38_trailQuadData;   // Saturn 0x38: quad data for trail particles
    u8 m3C_trailFileBundleIdx;     // Saturn 0x3C: file bundle index for trail
    sSaturnPtr m40_hitQuadData;     // Saturn 0x40: quad data for hit particles
    u8 m44_hitFileBundleIdx;        // Saturn 0x44: file bundle index for hit
    s16 m46_trailFrequencyMask;     // Saturn 0x46: random mask for trail spawn frequency
    u8 m48_hitSoundId;              // Saturn 0x48: sound effect ID on expiry (0xFF = none)
    // size 0x4C
};

static void BaldorProjectile_update(sBaldorProjectile* pThis);
static void BaldorProjectile_drawParticle(sBaldorProjectile* pThis);
static void BaldorProjectile_drawModel(sBaldorProjectile* pThis);

s32 sGunShotTask_UpdateSub0(sVec3_FP* param1, sVec3_FP* param2, sVec3_FP* param3); // from gunShotRootTask.cpp

// 060a44de
static void BaldorProjectile_update(sBaldorProjectile* pThis)
{
    // move: position += velocity, velocity += acceleration
    pThis->m8_position += pThis->m14_velocity;
    pThis->m14_velocity += pThis->m20_acceleration;

    // advance visual animation
    if (pThis->m30_isModelType == 0)
        sGunShotTask_UpdateSub4((sAnimatedQuad*)pThis->m2C_visual);
    else
        stepAnimation((s_3dModel*)pThis->m2C_visual);

    // optional trail particle (spawns randomly based on m46 mask)
    if (!pThis->m38_trailQuadData.isNull())
    {
        u32 rnd = randomNumber();
        if (((s32)pThis->m46_trailFrequencyMask & rnd) == 0)
        {
            createParticleEffect(
                dramAllocatorEnd[pThis->m3C_trailFileBundleIdx].mC_fileBundle,
                &g_BTL_GenericData->m_0x60a8c24_animatedQuad, // TODO: resolve quad from m38_trailQuadData
                &pThis->m8_position, nullptr, nullptr, 0x10000, 0, 0);
        }
    }

    // collision check against dragon
    struct { sVec3_FP pos; s32 radius; } projView = {}, dragonView = {};
    transformAndAddVecByCurrentMatrix(&pThis->m8_position, &projView.pos);
    projView.radius = 0;
    sVec3_FP dragonPos = gBattleManager->m10_battleOverlay->m18_dragon->m8_position;
    transformAndAddVecByCurrentMatrix(&dragonPos, &dragonView.pos);
    dragonView.radius = 0x5000;

    sVec3_FP hitInfo = {};
    s32 hitResult = sGunShotTask_UpdateSub0(&projView.pos, &dragonView.pos, &hitInfo);
    if (hitResult > 0)
    {
        // hit the dragon — apply damage
        applyDamageToDragon(
            gBattleManager->m10_battleOverlay->m18_dragon->m8C,
            pThis->m34_damageValue,
            gBattleManager->m10_battleOverlay->m18_dragon->m8_position,
            3,
            pThis->m14_velocity,
            pThis->m36_damageType);

        // spawn hit particle effect
        if (!pThis->m40_hitQuadData.isNull())
        {
            createParticleEffect(
                dramAllocatorEnd[pThis->m44_hitFileBundleIdx].mC_fileBundle,
                &g_BTL_GenericData->m_0x60a8c24_animatedQuad, // TODO: resolve quad from m40_hitQuadData
                &pThis->m8_position, nullptr, nullptr, 0x10000, 0, 0);
        }
        pThis->m32_timer = 0;
    }

    // timer countdown — destroy when expired
    if (--pThis->m32_timer < 0)
    {
        // spawn expiry particle effect
        if (!pThis->m40_hitQuadData.isNull())
        {
            createParticleEffect(
                dramAllocatorEnd[pThis->m44_hitFileBundleIdx].mC_fileBundle,
                &g_BTL_GenericData->m_0x60a8c24_animatedQuad, // TODO: resolve quad from m40_hitQuadData
                &pThis->m8_position, nullptr, nullptr, 0x10000, 0, 0);
        }
        // play expiry sound
        if ((s8)pThis->m48_hitSoundId >= 0)
        {
            playSystemSoundEffect((s8)pThis->m48_hitSoundId);
        }
        pThis->getTask()->markFinished();
    }
}

// 060a4720
static void BaldorProjectile_drawParticle(sBaldorProjectile* pThis)
{
    if (pThis->m32_timer >= 0)
    {
        drawProjectedParticle((sAnimatedQuad*)pThis->m2C_visual, &pThis->m8_position);
    }
}

// 060a46c8
static void BaldorProjectile_drawModel(sBaldorProjectile* pThis)
{
    if (pThis->m32_timer >= 0)
    {
        sVec2_FP angles;
        computeLookAt(pThis->m14_velocity, angles);
        sVec3_FP rotation;
        rotation[0] = -angles[0];
        rotation[1] = angles[1] + fixedPoint(0x8000000);
        rotation[2] = 0;
        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->m8_position);
        rotateCurrentMatrixYXZ(&rotation);
        ((s_3dModel*)pThis->m2C_visual)->m18_drawFunction((s_3dModel*)pThis->m2C_visual);
        popMatrix();
    }
}

// 060a473c — compute projectile velocity from source to target with acceleration
static void BaldorProjectile_computeVelocity(sBaldorProjectile* pThis, sVec3_FP* source, sVec3_FP* target, sSaturnPtr param3)
{
    s16 travelTime = readSaturnS16(param3 + 0xC);
    sVec3_FP delta = *target - *source;
    sVec2_FP angles;
    computeLookAt(delta, angles);

    fixedPoint speed = readSaturnS32(param3 + 0x10);
    pThis->m20_acceleration[0] = MTH_Mul(speed, getCos(angles[1].getInteger()));
    pThis->m20_acceleration[1].m_value = readSaturnS32(param3 + 0x14);
    pThis->m20_acceleration[2] = MTH_Mul(speed, getSin(angles[1].getInteger()));

    fixedPoint timeFixed = fixedPoint((s32)travelTime << 16);
    pThis->m14_velocity[0] = FP_Div(delta[0], timeFixed) - fixedPoint((pThis->m20_acceleration[0].m_value >> 1) * (s32)travelTime);
    pThis->m14_velocity[1] = FP_Div(delta[1], timeFixed) - fixedPoint((pThis->m20_acceleration[1].m_value >> 1) * (s32)travelTime);
    pThis->m14_velocity[2] = FP_Div(delta[2], timeFixed) - fixedPoint((pThis->m20_acceleration[2].m_value >> 1) * (s32)travelTime);

    pThis->m14_velocity += gBattleManager->m10_battleOverlay->m4_battleEngine->m1A0_battleAutoScrollDelta;
}

// 060a4428
p_workArea BaldorAttack_createAttackModel(sVec3_FP* partPosition, sVec3_FP* target, sSaturnPtr param3)
{
    u8 fileBundleIdx = readSaturnU8(param3);
    npcFileDeleter* pFileBundle = dramAllocatorEnd[fileBundleIdx].mC_fileBundle;

    sBaldorProjectile* pProj = nullptr;
    if (readSaturnS16(param3 + 2) == 0)
    {
        // particle-based projectile
        static const sBaldorProjectile::TypedTaskDefinition def0 = {
            nullptr, BaldorProjectile_update, BaldorProjectile_drawParticle, nullptr
        };
        pProj = createSubTaskWithCopy<sBaldorProjectile>(gBattleManager->m10_battleOverlay->m18_dragon, &def0);
        pProj->m0_fileBundle = pFileBundle->m0_fileBundle;
        pProj->m4_vd1Allocation = pFileBundle->m4_vd1Allocation;
        pProj->m30_isModelType = 0;

        sAnimatedQuad* pAnim = (sAnimatedQuad*)allocateHeapForTask(pProj, sizeof(sAnimatedQuad));
        pProj->m2C_visual = pAnim;
        sSaturnPtr quadData = readSaturnEA(param3 + 8);
        particleInitSub(pAnim, pFileBundle->m4_vd1Allocation->m4_vdp1Memory, &g_BTL_GenericData->m_0x60a8c24_animatedQuad); // TODO: use proper quad from param3+8
        u32 rndFrames = randomNumber() & 0xF;
        for (u32 i = 0; i < rndFrames; i++)
            sGunShotTask_UpdateSub4(pAnim);
    }
    else
    {
        // 3D model-based projectile
        static const sBaldorProjectile::TypedTaskDefinition def1 = {
            nullptr, BaldorProjectile_update, BaldorProjectile_drawModel, nullptr
        };
        pProj = createSubTaskWithCopy<sBaldorProjectile>(gBattleManager->m10_battleOverlay->m18_dragon, &def1);
        pProj->m0_fileBundle = pFileBundle->m0_fileBundle;
        pProj->m4_vd1Allocation = pFileBundle->m4_vd1Allocation;
        pProj->m30_isModelType = 1;

        s_3dModel* pModel = (s_3dModel*)allocateHeapForTask(pProj, sizeof(s_3dModel));
        pProj->m2C_visual = pModel;
        u16 modelOffset = readSaturnU16(param3 + 2);
        u16 animOffset = readSaturnU16(param3 + 6);
        sAnimationData* pAnim = nullptr;
        if (animOffset != 0)
            pAnim = pFileBundle->m0_fileBundle->getAnimation(animOffset);
        sModelHierarchy* pHierarchy = pFileBundle->m0_fileBundle->getModelHierarchy(modelOffset);
        sStaticPoseData* pPose = pFileBundle->m0_fileBundle->getStaticPose(readSaturnU16(param3 + 4), pHierarchy->countNumberOfBones());
        init3DModelRawData(pProj, pModel, 0, pFileBundle->m0_fileBundle, modelOffset, pAnim, pPose, nullptr, nullptr);
    }

    pProj->m8_position = *partPosition;
    BaldorProjectile_computeVelocity(pProj, partPosition, target, param3);
    pProj->m32_timer = readSaturnS16(param3 + 0xE);
    pProj->m34_damageValue = readSaturnS16(param3 + 0x18);
    pProj->m36_damageType = readSaturnU8(param3 + 0x1A);

    if (!readSaturnEA(param3 + 0x1C).isNull())
    {
        createParticleEffect(
            dramAllocatorEnd[readSaturnU8(param3 + 0x2A)].mC_fileBundle,
            &g_BTL_GenericData->m_0x60a8c24_animatedQuad, // TODO: use proper quad from param3+0x1C
            partPosition, nullptr, nullptr, 0x10000, 0, 0);
    }

    pProj->m38_trailQuadData = readSaturnEA(param3 + 0x20);
    if (!pProj->m38_trailQuadData.isNull())
    {
        pProj->m3C_trailFileBundleIdx = readSaturnU8(param3 + 0x2B);
        pProj->m46_trailFrequencyMask = readSaturnS16(param3 + 0x28);
    }

    pProj->m40_hitQuadData = readSaturnEA(param3 + 0x24);
    if (!pProj->m40_hitQuadData.isNull())
    {
        pProj->m44_hitFileBundleIdx = readSaturnU8(param3 + 0x2C);
    }

    pProj->m48_hitSoundId = 0xFF;
    return pProj;
}

// Queen acid spray — builds attack params inline instead of from Saturn data
p_workArea BaldorAttack_createQueenAcidProjectile(sVec3_FP* partPosition, sVec3_FP* target, u8 fileBundleIdx, s16 timer, s16 lifetime)
{
    npcFileDeleter* pFileBundle = dramAllocatorEnd[fileBundleIdx].mC_fileBundle;

    static const sBaldorProjectile::TypedTaskDefinition def0 = {
        nullptr, BaldorProjectile_update, BaldorProjectile_drawParticle, nullptr
    };
    sBaldorProjectile* pProj = createSubTaskWithCopy<sBaldorProjectile>(gBattleManager->m10_battleOverlay->m18_dragon, &def0);
    if (!pProj) return nullptr;

    pProj->m0_fileBundle = pFileBundle->m0_fileBundle;
    pProj->m4_vd1Allocation = pFileBundle->m4_vd1Allocation;
    pProj->m30_isModelType = 0;

    sAnimatedQuad* pAnim = (sAnimatedQuad*)allocateHeapForTask(pProj, sizeof(sAnimatedQuad));
    pProj->m2C_visual = pAnim;
    particleInitSub(pAnim, pFileBundle->m4_vd1Allocation->m4_vdp1Memory, &g_BTL_GenericData->m_0x60a8c24_animatedQuad);
    u32 rndFrames = randomNumber() & 0xF;
    for (u32 i = 0; i < rndFrames; i++)
        sGunShotTask_UpdateSub4(pAnim);

    pProj->m8_position = *partPosition;

    // Compute velocity: direction from source to target with no horizontal speed, gravity pulling down
    sVec3_FP delta = *target - *partPosition;
    sVec2_FP angles;
    computeLookAt(delta, angles);
    pProj->m20_acceleration[0] = 0;
    pProj->m20_acceleration[1].m_value = -0x160; // gravity
    pProj->m20_acceleration[2] = 0;

    // No horizontal speed (speed param = 0), just arc from source to target
    fixedPoint timeFixed = fixedPoint((s32)timer << 16);
    if (timer > 0) {
        pProj->m14_velocity[0] = FP_Div(delta[0], timeFixed);
        pProj->m14_velocity[1] = FP_Div(delta[1], timeFixed) - fixedPoint((pProj->m20_acceleration[1].m_value >> 1) * (s32)timer);
        pProj->m14_velocity[2] = FP_Div(delta[2], timeFixed);
    }
    pProj->m14_velocity += gBattleManager->m10_battleOverlay->m4_battleEngine->m1A0_battleAutoScrollDelta;

    pProj->m32_timer = lifetime; // display lifetime (param3 offset 0x0E)
    pProj->m34_damageValue = 3;  // damage (param3 offset 0x18)
    pProj->m36_damageType = 0;
    pProj->m38_trailQuadData = {};
    pProj->m40_hitQuadData = {};
    pProj->m46_trailFrequencyMask = 0;
    pProj->m48_hitSoundId = 0xFF;
    return pProj;
}

void BaldorAttack_update(sBaldorAttack* pThis)
{
    switch (pThis->mA_state)
    {
    case 0:
        if (--pThis->m4_delay < 0)
        {
            pThis->m4_delay = 3;
            sVec3_FP local1C = gBattleManager->m10_battleOverlay->m18_dragon->m8_position;
            local1C[2] += pThis->m6_baldorPartEmittingAttack * 0x2000;

            BaldorAttack_createAttackModel(&pThis->m0->m30_parts[pThis->m6_baldorPartEmittingAttack].m4_worldPosition, &local1C, g_BTL_A3->getSaturnPtr(0x60a8018));

            if (++pThis->m6_baldorPartEmittingAttack > 5)
            {
                pThis->m6_baldorPartEmittingAttack = 0;
                if (++pThis->m8_numAttackRotation > 0)
                {
                    pThis->m4_delay = 9;
                    pThis->mA_state++;
                }
            }
        }
        break;
    case 1:
        if (--pThis->m4_delay < 0)
        {
            pThis->getTask()->markFinished();
        }
        break;
    default:
        assert(0);
    }
}

p_workArea Baldor_createAttackTask(sBaldor* pThis)
{
    sBaldorAttack* pNewTask = createSubTaskFromFunction<sBaldorAttack>(pThis, &BaldorAttack_update);
    pNewTask->m0 = pThis->m68_body;
    pNewTask->mC = pThis;

    return pNewTask;
}

// front attack (terrible bite)
void Baldor_update_mode1(sBaldor* pThis)
{
    switch (pThis->m9_attackStatus)
    {
    case 0:
        if (pThis->m34_formationEntry->m49 != 1)
        {
            return;
        }

        pThis->m34_formationEntry->m48 &= ~1;
        pThis->m34_formationEntry->m48 |= 2;
        pThis->m9_attackStatus++;
        break;
    case 1:
        pThis->m44_translationTarget = gBattleManager->m10_battleOverlay->m18_dragon->m8_position - gBattleManager->m10_battleOverlay->m4_battleEngine->mC_battleCenter;
        switch (gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant)
        {
        case 0:
            pThis->m44_translationTarget[2] -= 0x3000;
            break;
        case 1:
            pThis->m44_translationTarget[0] -= 0x3000;
            break;
        case 2:
            pThis->m44_translationTarget[2] += 0x3000;
            break;
        case 3:
            pThis->m44_translationTarget[0] += 0x3000;
            break;
        default:
            assert(0);
        }

        initAnimation(pThis->m38_3dModel, dramAllocatorEnd[readSaturnU8(pThis->m3C_dataPtr)].mC_fileBundle->m0_fileBundle->getAnimation(readSaturnU16(readSaturnEA(pThis->m3C_dataPtr + 0xC) + 2)));
        pThis->m9_attackStatus = 2;
        break;
    case 2:
        pThis->m44_translationTarget = gBattleManager->m10_battleOverlay->m18_dragon->m8_position - gBattleManager->m10_battleOverlay->m4_battleEngine->mC_battleCenter;
        switch (gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant)
        {
        case 0:
            pThis->m44_translationTarget[2] -= 0x3000;
            break;
        case 1:
            pThis->m44_translationTarget[0] -= 0x3000;
            break;
        case 2:
            pThis->m44_translationTarget[2] += 0x3000;
            break;
        case 3:
            pThis->m44_translationTarget[0] += 0x3000;
            break;
        default:
            assert(0);
        }

        if (pThis->m38_3dModel->m16_previousAnimationFrame != 0x1E)
            return;

        applyDamageToDragon(gBattleManager->m10_battleOverlay->m18_dragon->m8C, 27, gBattleManager->m10_battleOverlay->m18_dragon->m8_position, 3, gBattleManager->m10_battleOverlay->m18_dragon->m8_position - *pThis->m1C_translation.m0_current, 0);
        createParticleEffect(dramAllocatorEnd[6].mC_fileBundle, &g_BTL_GenericData->m_0x60a9120_animatedQuad, &gBattleManager->m10_battleOverlay->m18_dragon->m8_position, nullptr, nullptr, 0x10000, 0, 0);
        playSystemSoundEffect(0x67);
        pThis->m9_attackStatus = 3;
        break;
    case 3:
        // animation finished
        if (pThis->m38_3dModel->m16_previousAnimationFrame < (pThis->m38_3dModel->m30_pCurrentAnimation ? pThis->m38_3dModel->m30_pCurrentAnimation->m4_numFrames : 0) - 1)
        {
            return;
        }
        initAnimation(pThis->m38_3dModel, dramAllocatorEnd[readSaturnU8(pThis->m3C_dataPtr)].mC_fileBundle->m0_fileBundle->getAnimation(readSaturnU16(readSaturnEA(pThis->m3C_dataPtr + 0xC) + 0)));
        pThis->m34_formationEntry->m48 &= ~2;
        pThis->m9_attackStatus = 4;
        break;
    case 4:
        pThis->m8_mode = 0;
        pThis->m9_attackStatus = 0;
        break;
    default:
        assert(0);
    }
}

// side attack
void Baldor_update_mode2(sBaldor* pThis)
{
    switch (pThis->m9_attackStatus)
    {
    case 0: // init attack
        if (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2000)
        {
            return;
        }

        if (pThis->m34_formationEntry->m49 != 1)
        {
            return;
        }

        pThis->m34_formationEntry->m48 &= ~1;
        pThis->m34_formationEntry->m48 |= 2;

        switch (gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant)
        {
        case 1:
            pThis->m5C_rotationDelta[1] += 0x71C71C;
            break;
        case 3:
            pThis->m5C_rotationDelta[1] -= 0x71C71C;
            break;
        default:
            break;
        }

        pThis->m90_attackTask = Baldor_createAttackTask(pThis);
        playSystemSoundEffect(0x68);
        pThis->m9_attackStatus++;
        break;
    case 1: // attack is running
        if (pThis->m90_attackTask && !pThis->m90_attackTask->getTask()->isFinished())
            return;
        pThis->m34_formationEntry->m48 &= ~2;
        pThis->m9_attackStatus++;
        break;
    case 2: // attack is finished
        pThis->m8_mode = 0;
        pThis->m9_attackStatus = 0;
        break;
    default:
        assert(0);
    }
}

void Baldor_update_mode0(sBaldor* pThis)
{
    switch (pThis->m34_formationEntry->m49)
    {
    case 0:
    case 1:
        break;
    case 2:
        pThis->m8_mode = 1;
        pThis->m34_formationEntry->m48 |= 1;
        pThis->m34_formationEntry->m49 = 0;
        break;
    case 3: // attack queued
        pThis->m8_mode = 2; // enter attack mode
        pThis->m34_formationEntry->m48 |= 1;
        pThis->m34_formationEntry->m49 = 0;
        break;
    case 4: // death trigger from formation
        pThis->m8_mode = 0xB;
        pThis->m34_formationEntry->m49 = 0;
        break;
    default:
        assert(0);
        break;
    }
}

// 060556e4 — body part debris task
struct sBaldorPartDebris : public s_workAreaTemplateWithCopy<sBaldorPartDebris>
{
    sVec3_FP m8_position;
    sVec3_FP m14_velocity;
    sVec3_FP m20_rotation;
    sVec3_FP m2C_angularVelocity;
    u16 m38_modelIndex;
    // size 0x3C
};

// 06055650
static void baldorPartDebris_update(sBaldorPartDebris* pThis)
{
    pThis->m14_velocity[1].m_value += -0x2C; // gravity
    pThis->m8_position += pThis->m14_velocity;
    pThis->m20_rotation += pThis->m2C_angularVelocity;
    // kill when below ground
    if (pThis->m8_position[1] <= gBattleManager->m10_battleOverlay->mC_targetSystem->m204_cameraMaxAltitude)
    {
        pThis->getTask()->markFinished();
    }
}

// 060556b2
static void baldorPartDebris_draw(sBaldorPartDebris* pThis)
{
    pushCurrentMatrix();
    translateCurrentMatrix(&pThis->m8_position);
    rotateCurrentMatrixYXZ(&pThis->m20_rotation);
    addObjectToDrawList(pThis->m0_fileBundle->get3DModel(pThis->m38_modelIndex));
    popMatrix();
}

// 060556e4
static void createBaldorPartDebris(sVec3_FP* position, sVec3_FP* rotation, u16 modelIndex)
{
    s16 fileBundleIdx = ((gBattleManager->m6_subBattleId == 8) || (gBattleManager->m6_subBattleId == 9)) ? 0x10 : 0xA;
    npcFileDeleter* pFileBundle = dramAllocatorEnd[fileBundleIdx].mC_fileBundle;

    static const sBaldorPartDebris::TypedTaskDefinition def = {
        nullptr, baldorPartDebris_update, baldorPartDebris_draw, nullptr
    };
    sBaldorPartDebris* pThis = createSubTaskWithCopy<sBaldorPartDebris>(pFileBundle, &def);
    if (!pThis) return;

    pThis->m8_position = *position;
    pThis->m20_rotation = *rotation;
    pThis->m14_velocity[0].m_value = (s32)((randomNumber() & 0xFFF) - 0x7FF);
    pThis->m14_velocity[1].m_value = (s32)((randomNumber() & 0xFFF) - 0x7FF);
    pThis->m14_velocity[2].m_value = (s32)((randomNumber() & 0xFFF) - 0x7FF);
    pThis->m2C_angularVelocity[0].m_value = (s32)(((randomNumber() & 0x1F) - 0xF)) * 0xB60B6;
    pThis->m2C_angularVelocity[1].m_value = (s32)(((randomNumber() & 0x1F) - 0xF)) * 0xB60B6;
    pThis->m2C_angularVelocity[2].m_value = (s32)(((randomNumber() & 0x1F) - 0xF)) * 0xB60B6;
    pThis->m38_modelIndex = modelIndex;
}

// 06055698
void createBaldorDeathEffect(sBaldor* pThis)
{
    u8 fileBundleIdx = readSaturnU8(pThis->m3C_dataPtr);
    createBoneDebrisExplosion(
        pThis,
        pThis->m38_3dModel,
        pThis->m1C_translation.m0_current,
        pThis->m28_rotation.m0_current,
        nullptr, nullptr,
        -0x2C, nullptr, 0, 0, 0, 0x10000, 0);

    // 060556e4 — individual body part debris (each tentacle segment flies off separately)
    for (int i = 0; i < 6; i++)
    {
        sBaldorBodyPart& part = pThis->m68_body->m30_parts[i];
        createBaldorPartDebris(&part.m4_worldPosition, &part.m1C_rotation, part.m40_modelIndex);
    }
}

// Taking damage
void Baldor_update_modeB(sBaldor* pThis)
{
    if (pThis->m9_attackStatus == 0)
    {
        for (int i = 0; i < pThis->mC_numTargetables; i++)
        {
            deleteTargetable(&pThis->m14_targetable[i]);
        }

        pThis->mC_numTargetables = 0;
        pThis->m40_enemyLifeMeterTask->m31 |= 1;
        pThis->m40_enemyLifeMeterTask->m31 |= 8;

        createBaldorDeathEffect(pThis);

        pThis->m34_formationEntry->m48 |= 0x40;

        pThis->getTask()->markFinished();
    }
}

void Baldor_update(sBaldorBase* pThisBase)
{
    sBaldor* pThis = (sBaldor*)pThisBase;
    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x1B])
    {
        assert(0);
    }

    if (isTraceEnabled())
    {
        addTraceLog(*pThis->m1C_translation.m4_target, "BaldorTranslation_m4");
        addTraceLog(gBattleManager->m10_battleOverlay->m4_battleEngine->mC_battleCenter, "battleCenter");
    }

    *pThis->m1C_translation.m0_current = gBattleManager->m10_battleOverlay->m4_battleEngine->mC_battleCenter + *pThis->m1C_translation.m4_target;

    if (pThis->mC_numTargetables > 0)
    {
        transformAndAddVecByCurrentMatrix(pThis->m1C_translation.m0_current, &pThis->m18_position[0]);
        for (int i = 1; i < 4; i++) // actually hard coded to 4
        {
            transformAndAddVecByCurrentMatrix(&pThis->m68_body->m30_parts[i-1].m4_worldPosition, &pThis->m18_position[i]);
        }
    }

    stepAnimation(pThis->m38_3dModel);
    pThis->m44_translationTarget = *pThis->m1C_translation.m8;

    if ((gBattleManager->m6_subBattleId != 8) && (gBattleManager->m6_subBattleId != 9))
    {
        // Standalone battle
        pThis->m6C_oscillationPhase += sVec3_FP(0x222222, 0x16c16c, 0xb60b6);

        pThis->m44_translationTarget[0] += MTH_Mul(0xA000, getSin(pThis->m6C_oscillationPhase[0].getInteger()));
        pThis->m44_translationTarget[1] += MTH_Mul(0xA000, getSin(pThis->m6C_oscillationPhase[1].getInteger()));
        pThis->m44_translationTarget[2] += MTH_Mul(0xA000, getSin(pThis->m6C_oscillationPhase[2].getInteger()));
    }
    else
    {
        // With Baldor Queen
        int position = performModulo(3, pThis->mA_indexInFormation);
        switch (position) {
        case 0:
            pThis->m6C_oscillationPhase[0] += 0x16C16C;
            break;
        case 1:
            pThis->m6C_oscillationPhase[0] += -0x1907f6;
            break;
        case 2:
            pThis->m6C_oscillationPhase[0] += 0x1fdb97;
            break;
        }

        if ((pThis->mA_indexInFormation & 1) == 0) {
            pThis->m6C_oscillationPhase[1] += 0xB60B6;
        }
        else if ((pThis->mA_indexInFormation & 1) == 1) {
            pThis->m6C_oscillationPhase[1] += -0xECA86;
        }
        pThis->m44_translationTarget[0] += MTH_Mul(0xA000, getSin(pThis->m6C_oscillationPhase[0].getInteger()));
        pThis->m44_translationTarget[1] += MTH_Mul(0xA000, getSin(pThis->m6C_oscillationPhase[1].getInteger()));
        pThis->m44_translationTarget[2] += MTH_Mul(0xF000, getCos(pThis->m6C_oscillationPhase[2].getInteger()));
    }

    pThis->m68_body->m0_translation = *pThis->m1C_translation.m0_current;
    pThis->m68_body->mC_rotation = *pThis->m28_rotation.m0_current;
    pThis->m68_body->m18_rotationTarget = *pThis->m28_rotation.m0_current;

    sBaldorBody* pData = pThis->m68_body;
    pData->m24_update(&pData->m30_parts[0], &pData->m0_translation, &pData->mC_rotation, &pData->m18_rotationTarget);

    pThis->m78_movementVector = pThis->m50_translationDelta;

    Baldor_updateSub0(pThis);

    switch (pThis->m8_mode)
    {
    case 0: //idle
        Baldor_update_mode0(pThis);
        break;
    case 1: // attacking in front
        Baldor_update_mode1(pThis);
        break;
    case 2: // attacking on the side
        Baldor_update_mode2(pThis);
        break;
    case 11: // taking damage
        Baldor_update_modeB(pThis);
        break;
    default:
        assert(0);
    }

    springDampedStep(pThis->m1C_translation.m4_target, &pThis->m50_translationDelta, &pThis->m44_translationTarget, 0x1999, 0x147, 0);

    sVec2_FP temp;
    computeLookAt(gBattleManager->m10_battleOverlay->m4_battleEngine->m1A0_battleAutoScrollDelta + pThis->m78_movementVector, temp);

    if (isTraceEnabled())
    {
        addTraceLog(gBattleManager->m10_battleOverlay->m4_battleEngine->m1A0_battleAutoScrollDelta, "m1A0");
        addTraceLog(pThis->m78_movementVector, "m78_movementVector");
        addTraceLog(temp, "temp");
        addTraceLog(*pThis->m28_rotation.m0_current, "current");
        addTraceLog(pThis->m5C_rotationDelta, "delta");
        addTraceLog(*pThis->m28_rotation.m4_target, "target");
    }

    (*pThis->m28_rotation.m4_target)[0] = -temp[0];
    (*pThis->m28_rotation.m4_target)[1] = temp[1] + 0x8000000;
    (*pThis->m28_rotation.m4_target)[2] = 0;

    springDampedStep(pThis->m28_rotation.m0_current, &pThis->m5C_rotationDelta, pThis->m28_rotation.m4_target, 0x1999, 0x28F, 1);

    if (isTraceEnabled())
    {
        addTraceLog(*pThis->m1C_translation.m0_current, "BaldorTranslation");
        addTraceLog(*pThis->m28_rotation.m0_current, "BaldorRotation");
        addTraceLog(*pThis->m1C_translation.m4_target, "BaldorTranslation_m4");
    }
}

// 0607e946
static const u32 conditionalLightColorTable[4] = { 0x00181818, 0x00E1E110, 0x00E1E11F, 0x00E1E11F };

void setupConditionalLightColor(int param_1)
{
    int idx;
    if (param_1 >= 0x1F)
        idx = 3;
    else if (param_1 >= 0x15)
        idx = 2;
    else if (param_1 >= 0x0B)
        idx = 1;
    else
        idx = 0;
    setupLightColor(conditionalLightColorTable[idx]);
}

// 0607e976
void clearLightColor()
{
    auto* pGrid = gBattleManager->m10_battleOverlay->m8_gridTask;
    generateLightFalloffMap(
        s_RGB8::fromVector(pGrid->m1E4_lightFalloff0).toU32(),
        s_RGB8::fromVector(pGrid->m1FC_lightFalloff1).toU32(),
        s_RGB8::fromVector(pGrid->m208_lightFalloff2).toU32()
    );
}

void Baldor_draw(sBaldorBase* pThisBase)
{
    sBaldor* pThis = (sBaldor*)pThisBase;
    if (isTraceEnabled())
    {
        addTraceLog(*pThis->m1C_translation.m0_current, "BaldorTranslation");
        addTraceLog(*pThis->m28_rotation.m0_current, "BaldorRotation");
    }

    if (pThis->mB_flags & 8)
    {
        setupConditionalLightColor(10);
    }
    pushCurrentMatrix();
    translateCurrentMatrix(pThis->m1C_translation.m0_current);
    rotateCurrentMatrixYXZ(pThis->m28_rotation.m0_current);
    pThis->m38_3dModel->m18_drawFunction(pThis->m38_3dModel);
    popMatrix();

    sBaldorBody* pBaldorPart = pThis->m68_body;
    pBaldorPart->m28_draw(pThis, &pBaldorPart->m30_parts[0]);

    if (pThis->mB_flags & 8)
    {
        clearLightColor();
        pThis->mB_flags &= ~8;
    }
}

sBaldor* createBaldor(s_workAreaCopy* parent, sFormationData* pFormationEntry)
{
    static const sBaldor::TypedTaskDefinition definition = {
        Baldor_init,
        Baldor_update,
        Baldor_draw,
        nullptr,
    };
    return createSubTaskWithArgWithCopy<sBaldor, sFormationData*>(parent, pFormationEntry, &definition);
}
