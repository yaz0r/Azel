#include "PDS.h"
#include "battle/battleOverlay.h"
#include "battle/battleFormation.h"
#include "BTL_A3_BaltorFormation.h"
#include "baltor.h"

#include "town/town.h" // TODO: cleanup

// internal name: Hebi (snake)

struct BTL_A3_BaltorFormation : public s_workAreaTemplateWithArg<BTL_A3_BaltorFormation, u32>
{
    std::vector<sFormationData> m4_formationData;
    s8 m12_formationSize;
    // size 0x14
};

void BTL_A3_BaltorFormation_Init(BTL_A3_BaltorFormation* pThis, u32 formationID)
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
        pThis->m4_formationData[i].m0[1] = readSaturnVec3(formationDataSource + 0xC * i + 0);
        pThis->m4_formationData[i].m0[2] = readSaturnVec3(formationDataSource + 0xC * i + 0);
        pThis->m4_formationData[i].m24[1][1] = 0x8000000;

        sBaltor* pBaltor = createBaltor(pBuffer, &pThis->m4_formationData[i]);
        pBaltor->mA_indexInFormation = i;
    }

    FunctionUnimplemented();
}

void BTL_A3_BaltorFormation_Update(BTL_A3_BaltorFormation* pThis)
{
    FunctionUnimplemented();
}

p_workArea Create_BTL_A3_BaltorFormation(p_workArea parent, u32 arg)
{
    static const BTL_A3_BaltorFormation::TypedTaskDefinition definition = {
        BTL_A3_BaltorFormation_Init,
        BTL_A3_BaltorFormation_Update,
        nullptr,
        nullptr,
    };

    return createSubTaskWithArg<BTL_A3_BaltorFormation>(parent, arg, &definition);
}
