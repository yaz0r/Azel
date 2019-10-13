#include "PDS.h"
#include "baltor.h"
#include "battle/battleFormation.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "battle/battleEngine.h"

#include "battle/battleDragon.h" // todo: clean by moving s_battleDragon_8C to its own file

void Baltor_initSub0Sub2(sBaltor* pThis, sFormationData* pFormationEntry)
{
    pThis->m34_formationEntry = pFormationEntry;
    for (int i = 0; i < 3; i++)
    {
        pThis->m1C[i] = &pFormationEntry->m0[i];
        pThis->m28[i] = &pFormationEntry->m24[i];
    }

    pThis->m48 = 0;
    pThis->m49 = 0;
}

p_workArea createBaldorSubTask0(sVec3_FP* arg0, s32 arg1, s8* arg2, s8 arg3)
{
    FunctionUnimplemented();
    return nullptr;
}

void Baltor_initSub0(sBaltor* pThis, sSaturnPtr dataPtr, sFormationData* pFormationEntry, u32 arg)
{
    pThis->m3C_dataPtr = dataPtr;
    if (pThis->m2 == 0)
    {
        pThis->m38_3dModel = nullptr;
        pThis->mC = 0;
    }
    else
    {
        assert(0);
    }

    Baltor_initSub0Sub2(pThis, pFormationEntry);
    pThis->m40 = createBaldorSubTask0(pThis->m1C[0], 0, &pThis->m10, readSaturnS8(pThis->m3C_dataPtr + 1));
    if (-1 < arg)
    {
assert(0);
//riderInit(pThis->m38_3dModel, dramAllocatorEnd[readSaturnS8(pThis->m3C_dataPtr)].mC_buffer->m0_dramAllocation->getAnimation());
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

void monsterPart_defaultUpdate()
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

void baldorPart_update()
{
    assert(0);
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
    pData->m0 = 0;
    pData->m4 = 0;
    pData->m8 = 0;
    pData->mC = 0;
    pData->m10 = 0;
    pData->m14 = 0;
    pData->m18 = 0;
    pData->m1C = 0;
    pData->m20 = 0;
    pData->m24_update = monsterPart_defaultUpdate;
    pData->m28_draw = monsterPart_defaultDraw;
    pData->m2C_delete = monsterPart_defaultDelete;
    pData->m30.resize(0);
}

void Baltor_initSub2Sub1(sBaltor_68_30* pEntry, sBaltor_68_30* pNextEntry)
{
    pEntry->m0 = pNextEntry;
    pEntry->m4 = 0;
    pEntry->m8 = 0;
    pEntry->mC = 0;
    pEntry->m10.zeroize();
    pEntry->m1C = 0;
    pEntry->m20 = 0;
    pEntry->m24 = 0;
    pEntry->m28 = 0;
    pEntry->m2C = 0;
    pEntry->m30 = 0;
    pEntry->m34 = 0;
    pEntry->m38 = 0;
    pEntry->m3C = 0;
    pEntry->m40 = 0;
    pEntry->m44 = 0;
    pEntry->m48 = 0;
    pEntry->m4C = 0;
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
    dest->m44 = readSaturnS32(source + 4);
    dest->m48 = readSaturnS32(source + 8);
    dest->m4C = readSaturnS32(source + 0xC);
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
            dest->m10 = readSaturnVec3(pDataSource + 0);
            dest->m20 = (*pThis->m28[1])[1];
            dest++;

            dest->m10 = readSaturnVec3(pDataSource + 0xC);
            dest->m20 = (*pThis->m28[1])[1];
            dest++;

            dest->m10 = readSaturnVec3(pDataSource + 0xC);
            dest->m20 = (*pThis->m28[1])[1];
            dest++;
        }
    }

    pThis->m6C = randomNumber();
    pThis->m70 = randomNumber();
    pThis->m74 = randomNumber();
}

void Baltor_update(sBaltor* pThis)
{
    FunctionUnimplemented();
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
