#include "PDS.h"
#include "baldor.h"
#include "battle/battleFormation.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "battle/battleEngine.h"
#include "battle/battleDebug.h"
#include "kernel/fileBundle.h"
#include "kernel/animation.h"
#include "kernel/debug/trace.h"

#include "battle/battleDragon.h" // todo: clean by moving s_battleDragon_8C to its own file
#include "mainMenuDebugTasks.h"
#include "town/town.h"

void Baldor_initSub0Sub2(sBaldor* pThis, sFormationData* pFormationEntry)
{
    pThis->m34_formationEntry = pFormationEntry;

    pThis->m1C_translation.m0_current = &pFormationEntry->m0.m0;
    pThis->m1C_translation.m4 = &pFormationEntry->m0.mC;
    pThis->m1C_translation.m8 = &pFormationEntry->m0.m18;

    pThis->m28_rotation.m0_current = &pFormationEntry->m24.m0;
    pThis->m28_rotation.m4 = &pFormationEntry->m24.mC;
    pThis->m28_rotation.m8 = &pFormationEntry->m24.m18;

    pFormationEntry->m48 = 0;
    pFormationEntry->m49 = 0;
}

p_workArea createBaldorSubTask0(sVec3_FP* arg0, s32 arg1, s8* arg2, s8 arg3)
{
    FunctionUnimplemented();
    return nullptr;
}

s_3dModel* Baldor_create3dModel(sBaldor* pThis, sSaturnPtr dataPtr, s32 arg)
{
    u8 fileBundleIndex = readSaturnS8(dataPtr);
    s_fileBundle* pFileBundle = dramAllocatorEnd[fileBundleIndex].mC_buffer->m0_dramAllocation;
    sSaturnPtr animData = readSaturnEA(dataPtr + 8) + arg * 8;

    s_3dModel* pOutputModel = new s_3dModel;

    sSaturnPtr temp = readSaturnEA(dataPtr + 4);
    assert(temp.isNull());

    sModelHierarchy* pHierarchy = pFileBundle->getModelHierarchy(readSaturnU16(animData));
    init3DModelRawData(pThis, pOutputModel, 0, pFileBundle, readSaturnU16(animData), 0, pFileBundle->getStaticPose(readSaturnU16(animData + 2), pHierarchy->countNumberOfBones()), nullptr, nullptr);

    return pOutputModel;
}

void Baldor_initSub0Sub1(sBaldor* pThis, p_workArea, s16, std::vector<s_battleDragon_8C>&, std::vector<sVec3_FP>&)
{
    FunctionUnimplemented();
}

void Baldor_initSub0(sBaldor* pThis, sSaturnPtr dataPtr, sFormationData* pFormationEntry, s32 arg)
{
    pThis->m3C_dataPtr = dataPtr;
    if (readSaturnS8(dataPtr + 2) == 0)
    {
        pThis->m38_3dModel = nullptr;
        pThis->mC = 0;
    }
    else
    {
        pThis->m38_3dModel = Baldor_create3dModel(pThis, dataPtr, 0);
        Baldor_initSub0Sub1(pThis, pThis->m38_3dModel->m0_pOwnerTask, pThis->mC, pThis->m14, pThis->m18);
    }

    Baldor_initSub0Sub2(pThis, pFormationEntry);
    pThis->m40 = createBaldorSubTask0(pThis->m1C_translation.m0_current, 0, &pThis->m10_HP, readSaturnS8(pThis->m3C_dataPtr + 1));
    if (-1 < arg)
    {
        u8 bundleIdx = readSaturnS8(dataPtr);
        u32 offset = readSaturnU16(readSaturnEA(dataPtr + 0xC) + arg * 2);

        sAnimationData* pAnimation = dramAllocatorEnd[bundleIdx].mC_buffer->m0_dramAllocation->getAnimation(offset);

        riderInit(pThis->m38_3dModel, pAnimation);

        int animationSteps = randomNumber() & 0x1F;
        while (animationSteps)
        {
            stepAnimation(pThis->m38_3dModel);
            animationSteps--;
        }
    }
}

void Baldor_initSub1Sub0(p_workArea, s_battleDragon_8C*)
{
    FunctionUnimplemented();
}

void Baldor_initSub1Sub1(p_workArea, s_battleDragon_8C*)
{
    FunctionUnimplemented();
}

void Baldor_initSub1(s_battleDragon_8C* param_1, s_battleDragon* param_2, sVec3_FP* param_3, s32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8)
{
    param_1->m0 = param_2;
    param_1->m4 = param_3;
    param_1->m4C = param_4;
    param_1->m40 = *param_3;
    s_battleDragon_InitSub5Sub0(param_1);
    param_1->m8 = &param_1->m10;
    param_1->mC = 0;
    param_1->m1C.zeroize();
    param_1->m28.zeroize();
    param_1->m50 = param_5;
    param_1->m58 = 0;
    param_1->m5A = 0;
    param_1->m5E = 0;
    param_1->m5F = param_6;
    param_1->m5C = param_7;
    param_1->m54 = 0;
    param_1->m60 = param_8;

    std::array<s_battleEngineSub, 0x80>::iterator psVar4 = getBattleManager()->m10_battleOverlay->m4_battleEngine->m49C.begin();

    if (getBattleManager()->m10_battleOverlay->m4_battleEngine->m498 < 0x80)
    {
        int iVar3 = 0;
        do {
            if (psVar4->m0 == -1)
            {
                psVar4->m0 = 0;
                psVar4->m4 = param_1;
                psVar4->m8 = 0x7fffffff;
                if (param_1->m50 & 1)
                    return;

                Baldor_initSub1Sub0(getBattleManager()->m10_battleOverlay->m4_battleEngine, param_1);
                Baldor_initSub1Sub1(getBattleManager()->m10_battleOverlay->m4_battleEngine, param_1);

                getBattleManager()->m10_battleOverlay->m4_battleEngine->m498++;
            }
            psVar4++;
        } while (++iVar3 < 0x80);
    }
}

void monsterPart_defaultUpdate(sBaldor_68_30*, sVec3_FP*, sVec3_FP*, sVec3_FP*)
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

void baldorPart_update(sBaldor_68_30* pThis, sVec3_FP* pTranslation, sVec3_FP* pRotation, sVec3_FP* param4)
{
    pThis->m34[0] += MTH_Mul(fixedPoint((*param4)[0] - pThis->m1C[0]).normalized(), pThis->m44[0]);
    pThis->m34[1] += MTH_Mul(fixedPoint((*param4)[1] - pThis->m1C[1]).normalized(), pThis->m44[1]);
    pThis->m34[2] += MTH_Mul(fixedPoint((*param4)[2] - pThis->m1C[2]).normalized(), pThis->m44[2]);

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

    if (pThis->m0)
    {
        baldorPart_update(pThis->m0, &pThis->m4, &pThis->m1C, &pThis->m1C);
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
            addObjectToDrawList(pBaltor->m0_dramAllocation->get3DModel(pBaltorPart->m40));
            popMatrix();
        }
        if (pBaltorPart->m0 == nullptr)
            break;
        pBaltorPart = pBaltorPart->m0;
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
    pEntry->m0 = pNextEntry;
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
            piVar1 = piVar1->m0;
            arg3 += 0x14;
        } while (piVar1);
    }
}

void Baldor_init(sBaldor* pThis, sFormationData* pFormationEntry)
{
    sSaturnPtr puVar7;
    if ((getBattleManager()->m6_subBattleId == 8) || (getBattleManager()->m6_subBattleId == 9)) // middle boss  (with queen)
    {
        puVar7 = gCurrentBattleOverlay->getSaturnPtr(0x60a75f0);
    }
    else
    {
        puVar7 = gCurrentBattleOverlay->getSaturnPtr(0x60a73a0);
    }

    Baldor_initSub0(pThis, puVar7, pFormationEntry, 0);

    pThis->m14.resize(4);
    pThis->m18.resize(4);

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

        Baldor_initSub1(&pThis->m14[i], nullptr, &pThis->m18[i], 0x1000, ivar2, 0, 0, 10);
    }

    pThis->m28_rotation.m0_current = pThis->m28_rotation.m4;

    pThis->m68 = Baldor_initSub2(pThis, 6);

    if ((getBattleManager()->m6_subBattleId == 8) || (getBattleManager()->m6_subBattleId == 9))
    {
        assert(0);
    }
    else
    {
        Baldor_initSub3(pThis->m68, 1, gCurrentBattleOverlay->getSaturnPtr(0x60a7e5c));
        sSaturnPtr pDataSource = gCurrentBattleOverlay->getSaturnPtr(0x60a7f4c);

        std::vector<sBaldor_68_30>::iterator dest = pThis->m68->m30.begin();
        for (int i = 0; i < 2; i++)
        {
            dest->m10_translation = readSaturnVec3(pDataSource + 0);
            dest->m1C[1] = (*pThis->m28_rotation.m4)[1];
            dest++;

            dest->m10_translation = readSaturnVec3(pDataSource + 0xC);
            dest->m1C[1] = (*pThis->m28_rotation.m4)[1];
            dest++;

            dest->m10_translation = readSaturnVec3(pDataSource + 0xC);
            dest->m1C[1] = (*pThis->m28_rotation.m4)[1];
            dest++;
        }
    }

    pThis->m6C[0] = randomNumber();
    pThis->m6C[1] = randomNumber();
    pThis->m6C[2] = randomNumber();
}

void Baldor_updateSub0(sBaldor* pThis)
{
    FunctionUnimplemented();
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

void Baldor_update(sBaldor* pThis)
{
    if (getBattleManager()->m10_battleOverlay->m10_inBattleDebug->mFlags[0x1B])
    {
        assert(0);
    }

    *pThis->m1C_translation.m0_current = getBattleManager()->m10_battleOverlay->m4_battleEngine->mC_battleCenter + *pThis->m1C_translation.m4;

    if (0 < pThis->mC)
    {
        assert(0);
    }

    stepAnimation(pThis->m38_3dModel);
    pThis->m44_translationTarget = *pThis->m1C_translation.m8;

    if ((getBattleManager()->m6_subBattleId != 8) && (getBattleManager()->m6_subBattleId != 9))
    {
        pThis->m6C += sVec3_FP(0x222222, 0x16c16c, 0xb60b6);

        pThis->m44_translationTarget[0] += MTH_Mul(0xA000, getSin(pThis->m6C[0]));
        pThis->m44_translationTarget[1] += MTH_Mul(0xA000, getSin(pThis->m6C[1]));
        pThis->m44_translationTarget[2] += MTH_Mul(0xA000, getSin(pThis->m6C[2]));
    }
    else
    {
        assert(0);
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
    case 0:
        pThis->m6C[0] += 0x16C16C;
        break;
    case 1:
        pThis->m6C[0] -= 0x1907f6;
        break;
    case 2:
        pThis->m6C[0] += 0x1fdb97;
        break;
    default:
        assert(0);
    }

    Baldor_updateSub1(pThis->m1C_translation.m4, &pThis->m50_translationDelta, &pThis->m44_translationTarget, 0x1999, 0x147, 0);

    sVec2_FP temp;
    computeVectorAngles(getBattleManager()->m10_battleOverlay->m4_battleEngine->m1A0 + pThis->m78, temp);

    (*pThis->m28_rotation.m4)[0] = -temp[0];
    (*pThis->m28_rotation.m4)[1] = temp[1] + 0x8000000;
    (*pThis->m28_rotation.m4)[2] = 0;

    Baldor_updateSub1(pThis->m28_rotation.m0_current, &pThis->m5C_rotationDelta, pThis->m28_rotation.m4, 0x1999, 0x28F, 1);
}

void Baldor_draw(sBaldor* pThis)
{
    if (isTraceEnabled())
    {
        addTraceLog(*pThis->m1C_translation.m0_current, "BaldorTranslation");
        addTraceLog(*pThis->m28_rotation.m0_current, "BaldorRotation");
    }

    if (pThis->mB & 8)
    {
        assert(0);
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
        assert(0);
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
    return createSiblingTaskWithArgWithCopy<sBaldor, sFormationData*>(parent, pFormationEntry, &definition);
}
