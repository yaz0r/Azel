#include "PDS.h"
#include "BTL_A3_UrchinFormation.h"
#include "BTL_A3_data.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "battle/battleEngine.h"
#include "town/town.h" //TODO cleanup
#include "urchin.h"

s32 createBattleIntroTaskSub1(); //TODO cleanup
void displayFormationName(short uParm1, char uParm2, char uParm3); //TODO cleanup

struct BTL_A3_UrchinFormation : public s_workAreaTemplateWithArg<BTL_A3_UrchinFormation, const sGenericFormationData*>
{
    s8 m1;
    s8 m2;
    s8 m3_formationSize;
    s8 m7;
    s16 m14;
    sBTL_A3_UrchinFormation_18 m18;
    const sGenericFormationData* m30_config;
    s8 m38;
    std::array<s8, 3> m44;
    // size 0x4C
};

void formationCopyParams(sBTL_A3_UrchinFormation_18* pDest, const std::vector<sVec3_FP>& pSource, int count)
{
    for (int i = 0; i < 3; i++)
    {
        pDest->m4[i] = 1;
        pDest->m7[i] = 0;
        pDest->mA[i] = 0;
    }
    pDest->m10 = 1;

    for (int i = 0; i < count; i++)
    {
        pDest->m14[i].m0.zeroize();
        pDest->m14[i].mC = pSource[i];
        pDest->m14[i].m19 = 0;
        pDest->m14[i].m18 = 4;
        pDest->m14[i].m1C = 0;
    }
}

void BTL_A3_UrchinFormation_Init(BTL_A3_UrchinFormation* pThis, const sGenericFormationData* config)
{
    pThis->m30_config = config;
    pThis->m3_formationSize = config->m0_formationSize;
    pThis->m18.m14.resize(pThis->m3_formationSize);
    formationCopyParams(&pThis->m18, config->m10_formationSubData->m0_perEnemyPosition, pThis->m3_formationSize);
    if (createBattleIntroTaskSub1() == 0)
    {
        pThis->m1 = 1;
        pThis->m2 = 0;
    }
    else
    {
        if (gBattleManager->m10_battleOverlay->m4_battleEngine->m230 != 8)
        {
            pThis->m1 = 0;
            pThis->m2 = 0;
            return;
        }
        pThis->m1 = 1;
        pThis->m2 = 0;
    }

    bool bVar3 = false;
    bool bVar2 = false;
    int numModelIndex = 0;
    for (int i = 0; i < 3; i++)
    {
        pThis->m44[i] = -1;
        sGenericFormationPerTypeData* pEnemyEntry = config->m4_perTypeParams[i];
        if (pEnemyEntry)
        {
            allocateNPC(pThis, pEnemyEntry->m1);
            pThis->m18.mD[i] = config->m18[i];
            for (int j = 0; j < config->m1_perTypeCount[i]; j++)
            {
                createUrchin(pEnemyEntry, pThis->m18, numModelIndex, i);
                numModelIndex++;
            }

            if (config->m20[i] != -1)
            {
                assert(0);
            }

            if (pEnemyEntry->m24)
            {
                bVar3 = true;
            }

            if (pEnemyEntry->m2)
            {
                bVar2 = true;
            }
        }
    }

    if (bVar2 && bVar3)
    {
        pThis->m7 = 2;
    }
    pThis->m14 = 0x3C;
    if (pThis->m30_config->m15 == 0)
    {
        pThis->m38 = 1;
    }
    else
    {
        pThis->m38 = 0;
    }

    displayFormationName(config->m15, config->m16, config->m17);
    if ((gBattleManager->m4 == 7) &&
        (((gBattleManager->m6_subBattleId == 4 || (gBattleManager->m6_subBattleId == 8)) ||
            (gBattleManager->m6_subBattleId == 9))))
    {
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m10000 = 1;
    }
}

void BTL_A3_UrchinFormation_Update(BTL_A3_UrchinFormation* pThis)
{
    FunctionUnimplemented();
}

void BTL_A3_UrchinFormation_Delete(BTL_A3_UrchinFormation* pThis)
{
    //empty
}

p_workArea Create_BTL_A3_UrchinFormation(p_workArea parent, const sGenericFormationData* config)
{
    static const BTL_A3_UrchinFormation::TypedTaskDefinition definition = {
    BTL_A3_UrchinFormation_Init,
    BTL_A3_UrchinFormation_Update,
    nullptr,
    BTL_A3_UrchinFormation_Delete,
    };

    return createSubTaskWithArg<BTL_A3_UrchinFormation>(parent, config, &definition);
}

p_workArea Create_BTL_A3_UrchinFormationConfig(p_workArea parent, u32 arg)
{
    switch (arg)
    {
    case 3:
        return Create_BTL_A3_UrchinFormation(parent, g_BTL_A3->m_60A8AE8_urchinFormation); // KURAGE(Urchin) Fx2
    case 4:
    case 5:
        return Create_BTL_A3_UrchinFormation(parent, g_BTL_A3->m_60a8ac4_urchinFormation); // KURAGE(Urchin) M&F
    case 6:
        return Create_BTL_A3_UrchinFormation(parent, g_BTL_A3->m_60a7d34_urchinFormation); // BENITATENA (Pattergo) Zako
    case 7:
        return Create_BTL_A3_UrchinFormation(parent, g_BTL_A3->m_60a7d58_urchinFormation); // BENITATENA (Pattergo) Boss
    default:
        assert(0);
        return nullptr;
    }
}
