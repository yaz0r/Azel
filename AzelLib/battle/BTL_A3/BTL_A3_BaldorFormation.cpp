#include "PDS.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "battle/battleEngine.h"
#include "battle/battleEngineSub0.h"
#include "battle/battleFormation.h"
#include "battle/battleDebug.h"
#include "BTL_A3_BaldorFormation.h"
#include "baldor.h"

#include "town/town.h" // TODO: cleanup

// internal name: Hebi (snake)

struct BTL_A3_BaldorFormation : public s_workAreaTemplateWithArg<BTL_A3_BaldorFormation, u32>
{
    s8 m0;
    s8 m1;
    std::vector<sFormationData> m4_formationData;
    s16 m8;
    s16 m10;
    s8 m12_formationSize;
    // size 0x14
};

void BTL_A3_BaldorFormation_InitSub0Sub0(s16 uParm1)
{
    if (uParm1 == 0)
    {
        getBattleManager()->m10_battleOverlay->m4_battleEngine->m1E0 &= ~0xF;
    }
    else
    {
        getBattleManager()->m10_battleOverlay->m4_battleEngine->m1E0 |= uParm1 & 0xF;
    }
}

void BTL_A3_BaldorFormation_InitSub0(u32 uParm1)
{
    int uVar1;

    uParm1 = uParm1 & 0xff;
    if (uParm1 == 0) {
        uVar1 = 1;
    }
    else {
        if (uParm1 == 1) {
            uVar1 = 2;
        }
        else {
            if (uParm1 == 2) {
                uVar1 = 4;
            }
            else {
                if (uParm1 != 3) {
                    return;
                }
                uVar1 = 8;
            }
        }
    }
    BTL_A3_BaldorFormation_InitSub0Sub0(uVar1);
}

void BTL_A3_BaldorFormation_InitSub1Sub0(s16 uParm1)
{
    if (uParm1 == 0)
    {
        getBattleManager()->m10_battleOverlay->m4_battleEngine->m1E0 &= ~0xF0;
    }
    else
    {
        getBattleManager()->m10_battleOverlay->m4_battleEngine->m1E0 |= (uParm1 & 0xF) << 4;
    }
}

void BTL_A3_BaldorFormation_InitSub1(u32 param_1)

{
    int uVar1;

    param_1 = param_1 & 0xff;
    if (param_1 == 0) {
        uVar1 = 1;
    }
    else {
        if (param_1 == 1) {
            uVar1 = 2;
        }
        else {
            if (param_1 == 2) {
                uVar1 = 4;
            }
            else {
                if (param_1 != 3) {
                    return;
                }
uVar1 = 8;
            }
        }
    }
    BTL_A3_BaldorFormation_InitSub1Sub0(uVar1);
}

void displayFormationName(short uParm1, char uParm2, char uParm3)
{
    FunctionUnimplemented();
}

void BTL_A3_BaldorFormation_Init(BTL_A3_BaldorFormation* pThis, u32 formationID)
{
    allocateNPC(pThis, 10);

    npcFileDeleter* pBuffer = dramAllocatorEnd[10].mC_buffer;

    if (formationID == 0)
    {
        pThis->m12_formationSize = 1;
    }
    else
    {
        if ((formationID == 1) || (formationID == 2))
        {
            pThis->m12_formationSize = 2;
        }
    }

    pThis->m4_formationData = allocateFormationData(pThis, pThis->m12_formationSize);

    sSaturnPtr formationDataSource = gCurrentBattleOverlay->getSaturnPtr(0x60a809c) + 4 * formationID;
    for (int i = 0; i < pThis->m12_formationSize; i++)
    {
        pThis->m4_formationData[i].m0.mC = readSaturnVec3(readSaturnEA(formationDataSource) + 0xC * i + 0);
        pThis->m4_formationData[i].m0.m18 = readSaturnVec3(readSaturnEA(formationDataSource) + 0xC * i + 0);
        pThis->m4_formationData[i].m24.m18[1] = 0x8000000;

        sBaldor* pBaldor = createBaldor(pBuffer, &pThis->m4_formationData[i]);
        pBaldor->mA_indexInFormation = i;
    }

    BTL_A3_BaldorFormation_InitSub0(2);
    BTL_A3_BaldorFormation_InitSub1(0);
    pThis->m8 = 0x3C;
    displayFormationName(0, 1, 9);
    pThis->m0 = 0;
    pThis->m1 = 0;
}

void BTL_A3_BaldorFormation_UpdateSub0(int param1)
{
    battleEngine_SetBattleIntroType(8);
    getBattleManager()->m10_battleOverlay->m4_battleEngine->m433 = param1;
}

void BTL_A3_BaldorFormation_UpdateSub1(int param1, int param2, int param3)
{
    FunctionUnimplemented();
}

void BTL_A3_BaldorFormation_UpdateSub2(int param2)
{
    FunctionUnimplemented();
}

void BTL_A3_BaldorFormation_Update(BTL_A3_BaldorFormation* pThis)
{
    if (getBattleManager()->m10_battleOverlay->m10_inBattleDebug->mFlags[0x1B])
    {
        assert(0);
    }

    getBattleManager()->m10_battleOverlay->m4_battleEngine->m3B2_numBattleFormationRunning++;

    switch (pThis->m0)
    {
    case 0:
        break;
    case 1:
    case 2:
        switch (pThis->m1)
        {
        case 0:
            if (!getBattleManager()->m10_battleOverlay->m4_battleEngine->m188_flags.m2000) {
                pThis->m10 = 18;
                pThis->m1++;
            }
            break;
        case 1:
            if (--pThis->m10 > 0)
            {
                return;
            }
            for (int i = 0; i < pThis->m12_formationSize; i++)
            {
                if ((pThis->m4_formationData[i].m48 & 1) != 0) {
                    pThis->m4_formationData[i].m49 = 1;
                    pThis->m1++;
                    return;
                }
            }
            pThis->m1 = 10;
            return;
        case 2:
            for (int i = 0; i < pThis->m12_formationSize; i++)
            {
                if ((pThis->m4_formationData[i].m48 & 1) != 0) {
                    return;
                }
            }
            pThis->m1 = 1;
            return;
        case 10:
            getBattleManager()->m10_battleOverlay->m4_battleEngine->m188_flags.m100 = 1;
            for (int i = 0; i < pThis->m12_formationSize; i++)
            {
                pThis->m4_formationData[i].m49 = 0;
            }
            pThis->m0 = 0;
            pThis->m1 = 0;
            return;
        default:
            assert(0);
            break;
        }
        return;
    default:
        assert(0);
        break;
    }

    bool bVar3 = false;
    for (int i = 0; i < pThis->m12_formationSize; i++)
    {
        if (pThis->m4_formationData[i].m48 == 0)
        {
            bVar3 = true;
            break;
        }
    }

    if (!bVar3)
    {
        assert(0);
    }

    if (getBattleManager()->m10_battleOverlay->m4_battleEngine->m3CC->m8 == 0)
        return;

    if (BattleEngineSub0_UpdateSub0() != 0)
        return;

    getBattleManager()->m10_battleOverlay->m4_battleEngine->m3CC->m8 = 0;
    getBattleManager()->m10_battleOverlay->m4_battleEngine->m3CC->m0 = 0;

    if (!bVar3)
        return;

    switch (getBattleManager()->m10_battleOverlay->m4_battleEngine->m22C_battleDirection)
    {
    case 1:
    case 3:
        if ((randomNumber() & 1) == 0) {
            BTL_A3_BaldorFormation_UpdateSub0(2);
        }
        else
        {
            BTL_A3_BaldorFormation_UpdateSub0(7);
        }
        for (int i = 0; i < pThis->m12_formationSize; i++)
        {
            pThis->m4_formationData[i].m49 = 3;
        }
        BTL_A3_BaldorFormation_UpdateSub1(2, 0x1E, 0);
        BTL_A3_BaldorFormation_UpdateSub2(2);
        BTL_A3_BaldorFormation_UpdateSub2(3);
        pThis->m0 = 2;
        break;
    case 2:
    {
        switch (performModulo2(3, randomNumber()))
        {
        case 0:
            battleEngine_SetBattleIntroType(7);
            break;
        case 1:
            BTL_A3_BaldorFormation_UpdateSub0(8);
            break;
        case 2:
            BTL_A3_BaldorFormation_UpdateSub0(1);
            break;
        default:
            assert(0);
        }

        int numEntriesFound = 0;
        for (int i = 0; i < pThis->m12_formationSize; i++)
        {
            if ((pThis->m4_formationData[i].m48 & 4) == 0) {
                pThis->m4_formationData[i].m49 = 2;
                numEntriesFound++;
                if (numEntriesFound > 1)
                    break;
            }
        }

        BTL_A3_BaldorFormation_UpdateSub1(1, 0x1E, 0);
        BTL_A3_BaldorFormation_UpdateSub2(2);
        pThis->m0 = 1;
        break;
    }
    default:
        assert(0);
    }
}

p_workArea Create_BTL_A3_BaldorFormation(p_workArea parent, u32 arg)
{
    static const BTL_A3_BaldorFormation::TypedTaskDefinition definition = {
        BTL_A3_BaldorFormation_Init,
        BTL_A3_BaldorFormation_Update,
        nullptr,
        nullptr,
    };

    return createSubTaskWithArg<BTL_A3_BaldorFormation>(parent, arg, &definition);
}
