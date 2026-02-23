#include "PDS.h"
#include "baldor.h"
#include "battle/battleFormation.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
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

void Baldor_initSub0Sub1(p_workArea pThis, s_3dModel* pModel, s16* param3, std::vector<sBattleTargetable>& param4, std::vector<sVec3_FP>& param5)
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
                for (int j = 0; j < (*pModel->m40)[i].m4_count; j++)
                {
                    param4[currentEntryIndex].m4_pPosition = nullptr;

                    s_hotpoinEntry& puVar1 = (*pModel->m40)[i].m0[i];

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
        Baldor_initSub0Sub1(pThis, pThis->m38_3dModel, &pThis->mC_numTargetables, pThis->m14_targetable, pThis->m18_position);
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

void monsterPart_defaultUpdate(sBaldor_68_30*, const sVec3_FP*, const sVec3_FP*, const sVec3_FP*)
{
    assert(0);
}

void monsterPart_defaultDraw(sBaldor*, sBaldor_68_30*)
{
    assert(0);
}

void monsterPart_defaultDelete()
{
    assert(0);
}

void baldorPart_update(sBaldor_68_30* pThis, const sVec3_FP* pTranslation, const sVec3_FP* pRotation, const sVec3_FP* param4)
{
    pThis->m34 += MTH_Mul((*param4 - pThis->m1C).normalized(), pThis->m44);

    pThis->m34 -= MTH_Mul(pThis->m50, pThis->m28);

    pThis->m28 += pThis->m34;
    pThis->m1C += pThis->m28;

    pThis->m34.zeroize();

    sMatrix4x3 pTemp;
    initMatrixToIdentity(&pTemp);
    translateMatrix(*pTranslation, &pTemp);
    rotateMatrixZYX(pRotation, &pTemp);
    translateMatrix(pThis->m10_translation, &pTemp);

    pThis->m4 = pTemp.getTranslation();

    addTraceLog(pThis->m10_translation, "pThis->m10");
    addTraceLog(pThis->m1C, "pThis->m1C");
    addTraceLog(pThis->m28, "pThis->m28");
    addTraceLog(pThis->m34, "pThis->m34");
    addTraceLog(pThis->m50, "pThis->m50");
    addTraceLog(pThis->m4, "pThis->m4");

    if (pThis->m0_child)
    {
        baldorPart_update(pThis->m0_child, &pThis->m4, &pThis->m1C, &pThis->m1C);
    }
}

void baldorPart_draw(sBaldor* pBaltor, sBaldor_68_30* pBaltorPart)
{
    while (1)
    {
        if (pBaltorPart->m40 > 0)
        {
            pushCurrentMatrix();
            translateCurrentMatrix(pBaltorPart->m4);
            rotateCurrentMatrixZYX(pBaltorPart->m1C);
            addObjectToDrawList(pBaltor->m0_fileBundle->get3DModel(pBaltorPart->m40));
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

void Baldor_initSub2Sub0(sBaldor_68* pData)
{
    pData->m0_translation.zeroize();
    pData->mC_rotation.zeroize();
    pData->m18.zeroize();
    pData->m24_update = monsterPart_defaultUpdate;
    pData->m28_draw = monsterPart_defaultDraw;
    pData->m2C_delete = monsterPart_defaultDelete;
    pData->m30.resize(0);
}

void Baldor_initSub2Sub1(sBaldor_68_30* pEntry, sBaldor_68_30* pNextEntry)
{
    pEntry->m0_child = pNextEntry;
    pEntry->m4.zeroize();
    pEntry->m10_translation.zeroize();
    pEntry->m1C.zeroize();
    pEntry->m28.zeroize();
    pEntry->m34.zeroize();
    pEntry->m40 = 0;
    pEntry->m44.zeroize();
    pEntry->m50 = 0;
}

sBaldor_68* Baldor_initSub2(p_workArea parent, int numEntries)
{
    sBaldor_68* pNewData = new sBaldor_68;
    Baldor_initSub2Sub0(pNewData);

    pNewData->m30.resize(numEntries);

    Baldor_initSub2Sub1(&pNewData->m30[numEntries - 1], nullptr);
    int iVar1 = numEntries - 1;
    while (iVar1 != 0)
    {
        Baldor_initSub2Sub1(&pNewData->m30[iVar1 - 1], &pNewData->m30[iVar1]);
        iVar1--;
    }

    return pNewData;
}

void Baldor_initSub3Sub0(sBaldor_68_30* dest, sSaturnPtr source)
{
    dest->m40 = readSaturnS16(source);
    dest->m44 = readSaturnVec3(source + 4);
    dest->m50 = readSaturnS32(source + 0x10);
}

void Baldor_initSub3(sBaldor_68* pThis, int arg2, sSaturnPtr arg3)
{
    if (arg2 == 1)
    {
        pThis->m24_update = baldorPart_update;
        pThis->m28_draw = baldorPart_draw;
        pThis->m2C_delete = baldorPart_delete;
    }

    if (!arg3.isNull())
    {
        sBaldor_68_30* piVar1 = &pThis->m30[0];
        do
        {
            Baldor_initSub3Sub0(piVar1, arg3);
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

    pThis->m68 = Baldor_initSub2(pThis, 6);

    if ((gBattleManager->m6_subBattleId == 8) || (gBattleManager->m6_subBattleId == 9))
    {
        // During the Queen baldor fight
        Baldor_initSub3(pThis->m68, 1, g_BTL_A3->getSaturnPtr(0x60a7ed4));
        sSaturnPtr pDataSource = g_BTL_A3->getSaturnPtr(0x60a7f94);

        for (int i = 0; i < 6; i++)
        {
            sBaldor_68_30& dest = pThis->m68->m30[i];

            dest.m10_translation = readSaturnVec3(pDataSource + 0xC * i);
            dest.m1C[1] = MTH_Mul(0x1c71c71, (randomNumber() & 0x1ffff) - 0xffff) + pThis->m28_rotation.m4_target->m_value[1];
            dest.m1C[0] = MTH_Mul(0x1c71c71, (randomNumber() & 0x1ffff) - 0xffff);
            dest.m1C[2] = MTH_Mul(0x1c71c71, (randomNumber() & 0x1ffff) - 0xffff);
        }
    }
    else
    {
        Baldor_initSub3(pThis->m68, 1, g_BTL_A3->getSaturnPtr(0x60a7e5c));
        sSaturnPtr pDataSource = g_BTL_A3->getSaturnPtr(0x60a7f4c);

        for (int i = 0; i < 6; i++)
        {
            sBaldor_68_30& dest = pThis->m68->m30[i];

            dest.m10_translation = readSaturnVec3(pDataSource + 0xC * i);
            dest.m1C[1] = (*pThis->m28_rotation.m4_target)[1];
        }
    }

    pThis->m6C[0] = randomNumber();
    pThis->m6C[1] = randomNumber();
    pThis->m6C[2] = randomNumber();
}

s32 Baldor_updateSub0Sub0(p_workArea pThis, std::vector<sBattleTargetable>& param2, s16 entriesToParse, s16& param4)
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

sVec3_FP* Baldor_updateSub0Sub1Sub0(std::vector<sBattleTargetable>& param1, int param2)
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

    sVec3_FP* piVar2 = Baldor_updateSub0Sub1Sub0(pThis->m14_targetable, pThis->mC_numTargetables);
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
    case 2:
        if (gBattleManager->m2_currentBattleOverlayId == 1)
        {
            iVar2 = 5;
        }
        else
        {
            iVar2 = 4;
        }
        break;
    default:
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

void Baldor_updateSub0Sub2Sub0(p_workArea, sBattleTargetable&, int)
{
    Unimplemented();
}

void Baldor_updateSub0Sub2(sBaldor* pThis, std::vector<sBattleTargetable>& param2, int param3, int param4, sEnemyLifeMeterTask* param5)
{
    for (int i = 0; i < param3; i++)
    {
        sBattleTargetable& value = param2[i];
        if (value.m50_flags & 0x80000)
        {
            if (param4)
            {
                Baldor_updateSub0Sub2Sub0(pThis, value, 0);
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
        if (Baldor_updateSub0Sub0(pThis, pThis->m14_targetable, pThis->mC_numTargetables, damageValue))
        {
            pThis->mE_damageValue += damageValue;
            pThis->m12 = 1;
            pThis->mB |= 8;
            Baldor_updateSub0Sub1(pThis);
            pThis->m10_HP -= damageValue;
            if (pThis->m10_HP < 1)
            {
                Baldor_updateSub0Sub2(pThis, pThis->m14_targetable, pThis->mC_numTargetables, 0, pThis->m40_enemyLifeMeterTask);
                pThis->m34_formationEntry->m48 |= 4;
                createDamageDisplayTask(pThis, pThis->mE_damageValue, pThis->m1C_translation.m0_current, 1);
                playSystemSoundEffect(0x66);
                pThis->m8_mode = 0xB;
                pThis->m9_attackStatus = 0;
            }
            else
            {
                Baldor_updateSub0Sub2(pThis, pThis->m14_targetable, pThis->mC_numTargetables, 1, pThis->m40_enemyLifeMeterTask);
                playSystemSoundEffect(0x65);
            }
        }

        if (pThis->m12 && (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m1000))
        {
            createDamageDisplayTask(pThis, pThis->mE_damageValue, pThis->m1C_translation.m0_current, 1);
            pThis->mE_damageValue = 0;
            pThis->m12 = 0;
        }
    }
}

void Baldor_updateSub1(sVec3_FP* pCurrent, sVec3_FP* pDelta, sVec3_FP* pTarget, s32 pDeltaFactor, s32 pDistanceToTargetFactor, s8 translationOrRotation)
{
    switch(translationOrRotation)
    {
    case 0:
        if (pDelta == nullptr)
        {
            assert(0);
        }
        else
        {
            *pDelta -= MTH_Mul(pDeltaFactor, *pDelta);
            *pDelta += MTH_Mul(pDistanceToTargetFactor, *pTarget - *pCurrent);
        }
        *pCurrent += *pDelta;
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
    sBaldor_68* m0;
    s16 m4_delay;
    s16 m6_baldorPartEmittingAttack;
    s16 m8_numAttackRotation;
    u8 mA_state;
    sBaldor* mC;

    // size: 0x10
};

void BaldorAttack_createAttackModel(sVec3_FP* partPosition, sVec3_FP* target, sSaturnPtr param3)
{
    Unimplemented();
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

            BaldorAttack_createAttackModel(&pThis->m0->m30[pThis->m6_baldorPartEmittingAttack].m4, &local1C, g_BTL_A3->getSaturnPtr(0x60a8018));

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
    pNewTask->m0 = pThis->m68;
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
    default:
        assert(0);
        break;
    }
}

void createBaldorDeathEffect(sBaldor* pThis)
{
    Unimplemented();
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
            transformAndAddVecByCurrentMatrix(&pThis->m68->m30[i-1].m4, &pThis->m18_position[i]);
        }
    }

    stepAnimation(pThis->m38_3dModel);
    pThis->m44_translationTarget = *pThis->m1C_translation.m8;

    if ((gBattleManager->m6_subBattleId != 8) && (gBattleManager->m6_subBattleId != 9))
    {
        // Standalone battle
        pThis->m6C += sVec3_FP(0x222222, 0x16c16c, 0xb60b6);

        pThis->m44_translationTarget[0] += MTH_Mul(0xA000, getSin(pThis->m6C[0].getInteger()));
        pThis->m44_translationTarget[1] += MTH_Mul(0xA000, getSin(pThis->m6C[1].getInteger()));
        pThis->m44_translationTarget[2] += MTH_Mul(0xA000, getSin(pThis->m6C[2].getInteger()));
    }
    else
    {
        // With Baldor Queen
        int position = performDivision(3, pThis->mA_indexInFormation);
        switch (position) {
        case 0:
            pThis->m6C[0] += 0x16C16C;
            break;
        case 1:
            pThis->m6C[0] += -0x1907f6;
            break;
        case 2:
            pThis->m6C[0] += 0x1fdb97;
            break;
        }

        if ((pThis->mA_indexInFormation & 1) == 0) {
            pThis->m6C[1] += 0xB60B6;
        }
        else if ((pThis->mA_indexInFormation & 1) == 1) {
            pThis->m6C[1] += -0xECA86;
        }
        pThis->m44_translationTarget[0] += MTH_Mul(0xA000, getSin(pThis->m6C[0].getInteger()));
        pThis->m44_translationTarget[1] += MTH_Mul(0xA000, getSin(pThis->m6C[1].getInteger()));
        pThis->m44_translationTarget[2] += MTH_Mul(0xF000, getCos(pThis->m6C[2].getInteger()));
    }

    pThis->m68->m0_translation = *pThis->m1C_translation.m0_current;
    pThis->m68->mC_rotation = *pThis->m28_rotation.m0_current;
    pThis->m68->m18 = *pThis->m28_rotation.m0_current;

    sBaldor_68* pData = pThis->m68;
    pData->m24_update(&pData->m30[0], &pData->m0_translation, &pData->mC_rotation, &pData->m18);

    pThis->m78 = pThis->m50_translationDelta;

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

    Baldor_updateSub1(pThis->m1C_translation.m4_target, &pThis->m50_translationDelta, &pThis->m44_translationTarget, 0x1999, 0x147, 0);

    sVec2_FP temp;
    computeVectorAngles(gBattleManager->m10_battleOverlay->m4_battleEngine->m1A0_battleAutoScrollDelta + pThis->m78, temp);

    if (isTraceEnabled())
    {
        addTraceLog(gBattleManager->m10_battleOverlay->m4_battleEngine->m1A0_battleAutoScrollDelta, "m1A0");
        addTraceLog(pThis->m78, "m78");
        addTraceLog(temp, "temp");
        addTraceLog(*pThis->m28_rotation.m0_current, "current");
        addTraceLog(pThis->m5C_rotationDelta, "delta");
        addTraceLog(*pThis->m28_rotation.m4_target, "target");
    }

    (*pThis->m28_rotation.m4_target)[0] = -temp[0];
    (*pThis->m28_rotation.m4_target)[1] = temp[1] + 0x8000000;
    (*pThis->m28_rotation.m4_target)[2] = 0;

    Baldor_updateSub1(pThis->m28_rotation.m0_current, &pThis->m5C_rotationDelta, pThis->m28_rotation.m4_target, 0x1999, 0x28F, 1);

    if (isTraceEnabled())
    {
        addTraceLog(*pThis->m1C_translation.m0_current, "BaldorTranslation");
        addTraceLog(*pThis->m28_rotation.m0_current, "BaldorRotation");
        addTraceLog(*pThis->m1C_translation.m4_target, "BaldorTranslation_m4");
    }
}

void setupConditionalLightColor(int )
{
    Unimplemented();
}

void clearLightColor()
{
    Unimplemented();
}

void Baldor_draw(sBaldorBase* pThisBase)
{
    sBaldor* pThis = (sBaldor*)pThisBase;
    if (isTraceEnabled())
    {
        addTraceLog(*pThis->m1C_translation.m0_current, "BaldorTranslation");
        addTraceLog(*pThis->m28_rotation.m0_current, "BaldorRotation");
    }

    if (pThis->mB & 8)
    {
        setupConditionalLightColor(10);
    }
    pushCurrentMatrix();
    translateCurrentMatrix(pThis->m1C_translation.m0_current);
    rotateCurrentMatrixYXZ(pThis->m28_rotation.m0_current);
    pThis->m38_3dModel->m18_drawFunction(pThis->m38_3dModel);
    popMatrix();

    sBaldor_68* pBaldorPart = pThis->m68;
    pBaldorPart->m28_draw(pThis, &pBaldorPart->m30[0]);

    if (pThis->mB & 8)
    {
        clearLightColor();
        pThis->mB &= ~8;
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
