#include "PDS.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "battle/battleEngine.h"
#include "battle/battleFormation.h"
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

void BTL_A3_BaldorFormation_Update(BTL_A3_BaldorFormation* pThis)
{
    FunctionUnimplemented();
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
