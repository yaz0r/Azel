#include "PDS.h"
#include "baltor.h"
#include "battle/battleFormation.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "battle/battleEngine.h"
#include "battle/battleDebug.h"
#include "kernel/fileBundle.h"
#include "kernel/animation.h"

#include "battle/battleDragon.h" // todo: clean by moving s_battleDragon_8C to its own file
#include "mainMenuDebugTasks.h"
#include "town/town.h"

void Baltor_initSub0Sub2(sBaltor* pThis, sFormationData* pFormationEntry)
{
    pThis->m34_formationEntry = pFormationEntry;
    for (int i = 0; i < 3; i++)
    {
        pThis->m1C[i] = &pFormationEntry->m0[i];
        pThis->m28[i] = &pFormationEntry->m24[i];
    }

    pFormationEntry->m48 = 0;
    pFormationEntry->m49 = 0;
}

p_workArea createBaldorSubTask0(sVec3_FP* arg0, s32 arg1, s8* arg2, s8 arg3)
{
    FunctionUnimplemented();
    return nullptr;
}

s_3dModel* Baltor_create3dModel(sBaltor* pThis, sSaturnPtr dataPtr, s32 arg)
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

void Baltor_initSub0Sub1(sBaltor* pThis, p_workArea, s16, std::vector<s_battleDragon_8C>&, std::vector<sVec3_FP>&)
{
    FunctionUnimplemented();
}

void Baltor_initSub0(sBaltor* pThis, sSaturnPtr dataPtr, sFormationData* pFormationEntry, s32 arg)
{
    pThis->m3C_dataPtr = dataPtr;
    if (readSaturnS8(dataPtr + 2) == 0)
    {
        pThis->m38_3dModel = nullptr;
        pThis->mC = 0;
    }
    else
    {
        pThis->m38_3dModel = Baltor_create3dModel(pThis, dataPtr, 0);
        Baltor_initSub0Sub1(pThis, pThis->m38_3dModel->m0_pOwnerTask, pThis->mC, pThis->m14, pThis->m18);
    }

    Baltor_initSub0Sub2(pThis, pFormationEntry);
    pThis->m40 = createBaldorSubTask0(pThis->m1C[0], 0, &pThis->m10_HP, readSaturnS8(pThis->m3C_dataPtr + 1));
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

void Baltor_initSub1Sub0(p_workArea, s_battleDragon_8C*)
{
    FunctionUnimplemented();
}

void Baltor_initSub1Sub1(p_workArea, s_battleDragon_8C*)
{
    FunctionUnimplemented();
}

void Baltor_initSub1(s_battleDragon_8C* param_1, s_battleDragon* param_2, sVec3_FP* param_3, s32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8)
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

                Baltor_initSub1Sub0(getBattleManager()->m10_battleOverlay->m4_battleEngine, param_1);
                Baltor_initSub1Sub1(getBattleManager()->m10_battleOverlay->m4_battleEngine, param_1);

                getBattleManager()->m10_battleOverlay->m4_battleEngine->m498++;
            }
            psVar4++;
        } while (++iVar3 < 0x80);
    }
}

void monsterPart_defaultUpdate(sBaltor_68_30*, sVec3_FP*, sVec3_FP*, sVec3_FP*)
{
    assert(0);
}

void monsterPart_defaultDraw()
{
    assert(0);
}

void monsterPart_defaultDelete()
{
    assert(0);
}

void baldorPart_update(sBaltor_68_30* pThis, sVec3_FP* pTranslation, sVec3_FP* pRotation, sVec3_FP* param4)
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

void baldorPart_draw()
{
    assert(0);
}

void baldorPart_delete()
{
    assert(0);
}

void Baltor_initSub2Sub0(sBaltor_68* pData)
{
    pData->m0_translation.zeroize();
    pData->mC_rotation.zeroize();
    pData->m18.zeroize();
    pData->m24_update = monsterPart_defaultUpdate;
    pData->m28_draw = monsterPart_defaultDraw;
    pData->m2C_delete = monsterPart_defaultDelete;
    pData->m30.resize(0);
}

void Baltor_initSub2Sub1(sBaltor_68_30* pEntry, sBaltor_68_30* pNextEntry)
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

sBaltor_68* Baltor_initSub2(p_workArea parent, int numEntries)
{
    sBaltor_68* pNewData = new sBaltor_68;
    Baltor_initSub2Sub0(pNewData);

    pNewData->m30.resize(numEntries);

    Baltor_initSub2Sub1(&pNewData->m30[numEntries - 1], nullptr);
    int iVar1 = numEntries - 1;
    while (iVar1 != 0)
    {
        Baltor_initSub2Sub1(&pNewData->m30[iVar1 - 1], &pNewData->m30[iVar1]);
        iVar1--;
    }

    return pNewData;
}

void Baltor_initSub3Sub0(sBaltor_68_30* dest, sSaturnPtr source)
{
    dest->m40 = readSaturnS16(source);
    dest->m44 = readSaturnVec3(source + 4);
    dest->m50 = readSaturnS32(source + 0x10);
}

void Baltor_initSub3(sBaltor_68* pThis, int arg2, sSaturnPtr arg3)
{
    if (arg2 == 1)
    {
        pThis->m24_update = baldorPart_update;
        pThis->m28_draw = baldorPart_draw;
        pThis->m2C_delete = baldorPart_delete;
    }

    if (!arg3.isNull())
    {
        sBaltor_68_30* piVar1 = &pThis->m30[0];
        do
        {
            Baltor_initSub3Sub0(piVar1, arg3);
            piVar1 = piVar1->m0;
            arg3 += 0x14;
        } while (piVar1);
    }
}

void Baltor_init(sBaltor* pThis, sFormationData* pFormationEntry)
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

    Baltor_initSub0(pThis, puVar7, pFormationEntry, 0);

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

        Baltor_initSub1(&pThis->m14[i], nullptr, &pThis->m18[i], 0x1000, ivar2, 0, 0, 10);
    }

    pThis->m28[0] = pThis->m28[1];

    pThis->m68 = Baltor_initSub2(pThis, 6);

    if ((getBattleManager()->m6_subBattleId == 8) || (getBattleManager()->m6_subBattleId == 9))
    {
        assert(0);
    }
    else
    {
        Baltor_initSub3(pThis->m68, 1, gCurrentBattleOverlay->getSaturnPtr(0x60a7e5c));
        sSaturnPtr pDataSource = gCurrentBattleOverlay->getSaturnPtr(0x60a7f4c);

        std::vector<sBaltor_68_30>::iterator dest = pThis->m68->m30.begin();
        for (int i = 0; i < 2; i++)
        {
            dest->m10_translation = readSaturnVec3(pDataSource + 0);
            dest->m1C[1] = (*pThis->m28[1])[1];
            dest++;

            dest->m10_translation = readSaturnVec3(pDataSource + 0xC);
            dest->m1C[1] = (*pThis->m28[1])[1];
            dest++;

            dest->m10_translation = readSaturnVec3(pDataSource + 0xC);
            dest->m1C[1] = (*pThis->m28[1])[1];
            dest++;
        }
    }

    pThis->m6C[0] = randomNumber();
    pThis->m6C[1] = randomNumber();
    pThis->m6C[2] = randomNumber();
}

void Baltor_updateSub0(sBaltor* pThis)
{
    FunctionUnimplemented();
}

void Baltor_updateSub1(sVec3_FP* param1, sVec3_FP* param2, sVec3_FP* param3, s32 param4, s32 param5, s8 param6)
{
    switch(param6)
    {
    case 0:
        if (param2 == nullptr)
        {
            assert(0);
        }
        else
        {
            *param2 -= MTH_Mul(param4, *param2);
            *param2 += MTH_Mul(param5, *param3 - *param1);
        }
        *param1 += *param2;
        break;
    case 1:
        *param2 = (*param2 - MTH_Mul(param4, *param2)).normalized();
        *param2 = (*param2 + MTH_Mul(param5, (*param3 - *param1).normalized())).normalized();
        *param1 = (*param1 + *param2).normalized();
        break;
    default:
        assert(0);
    }
}

void Baltor_update(sBaltor* pThis)
{
    if (getBattleManager()->m10_battleOverlay->m10_inBattleDebug->mFlags[0x1B])
    {
        assert(0);
    }

    *pThis->m1C[0] = getBattleManager()->m10_battleOverlay->m4_battleEngine->mC + *pThis->m1C[1];

    if (0 < pThis->mC)
    {
        assert(0);
    }

    stepAnimation(pThis->m38_3dModel);
    pThis->m44 = *pThis->m1C[2];

    if ((getBattleManager()->m6_subBattleId != 8) && (getBattleManager()->m6_subBattleId != 9))
    {
        pThis->m6C += sVec3_FP(0x222222, 0x16c16c, 0xb60b6);

        pThis->m44[0] += MTH_Mul(0xA000, getSin(pThis->m6C[0]));
        pThis->m44[1] += MTH_Mul(0xA000, getSin(pThis->m6C[1]));
        pThis->m44[2] += MTH_Mul(0xA000, getSin(pThis->m6C[2]));
    }
    else
    {
        assert(0);
    }

    pThis->m68->m0_translation = *pThis->m1C[0];
    pThis->m68->mC_rotation = *pThis->m28[0];
    pThis->m68->m18 = *pThis->m28[0];

    sBaltor_68* pData = pThis->m68;
    pData->m24_update(&pData->m30[0], &pData->m0_translation, &pData->mC_rotation, &pData->m18);

    pThis->m78 = pThis->m50;

    Baltor_updateSub0(pThis);

    switch (pThis->m8_mode)
    {
    case 0:
        FunctionUnimplemented();
        break;
    case 1:
        FunctionUnimplemented();
        break;
    case 2:
        FunctionUnimplemented();
        break;
    case 0xb:
        FunctionUnimplemented();
        break;
    default:
        assert(0);
    }

    Baltor_updateSub1(pThis->m1C[1], &pThis->m50, &pThis->m44, 0x1999, 0x147, 0);

    sVec2_FP temp;
    generateCameraMatrixSub1(getBattleManager()->m10_battleOverlay->m4_battleEngine->m1A0 + pThis->m78, temp);

    (*pThis->m28[1])[0] = -temp[0];
    (*pThis->m28[1])[1] = temp[1] + 0x8000000;
    (*pThis->m28[1])[2] = 0;

    Baltor_updateSub1(pThis->m28[0], &pThis->m5C, pThis->m28[1], 0x1999, 0x28F, 1);
}

void Baltor_draw(sBaltor* pThis)
{
    FunctionUnimplemented();
}

sBaltor* createBaltor(s_workAreaCopy* parent, sFormationData* pFormationEntry)
{
    static const sBaltor::TypedTaskDefinition definition = {
        Baltor_init,
        Baltor_update,
        Baltor_draw,
        nullptr,
    };
    return createSiblingTaskWithArgWithCopy<sBaltor, sFormationData*>(parent, pFormationEntry, &definition);
}
