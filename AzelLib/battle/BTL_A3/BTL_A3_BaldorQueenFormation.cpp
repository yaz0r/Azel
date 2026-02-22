#include "PDS.h"
#include "BTL_A3_BaldorQueenFormation.h"
#include "kernel/graphicalObject.h"
#include "battle/battleFormation.h"
#include "battle/battleTextDisplay.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "battle/battleDebug.h"
#include "battle/battleEngine.h"
#include "battle/battleEngineSub0.h"
#include "baldorQueen.h"

void battleEngine_PlayAttackCamera(int param1); // TODO cleanup
void battleEngine_displayAttackName(int param1, int param2, int param3); // TODO: Cleanup

struct BTL_A3_BaldorQueenFormation : public s_workAreaTemplateWithArg<BTL_A3_BaldorQueenFormation, u32>
{
    u8 m0_formationState;
    u8 m1_formationSubState;
    std::vector<sFormationData> m4;
    s16 m8;
    u8 mB;
    std::array<u8, 6> mC;
    u8 mA;
    p_workArea m18;
    // size 0x24
};

void BTL_A3_BaldorQueenFormation_Init(BTL_A3_BaldorQueenFormation* pThis, u32 arg) {
    allocateNPC(pThis, 0x10);
    pThis->mA = 0;
    pThis->m4 = allocateFormationData(pThis, 7);

    pThis->m4[0].m0_translation.mC_target.zeroize();

    pThis->m18 = baldorQueenCreateQueenTask(dramAllocatorEnd[0x10].mC_fileBundle, &pThis->m4[0]);

    for (int i = 0; i < 6; i++) {
        pThis->m4[i + 1].m48 = 4;
        pThis->m4[i + 1].m49 = 0;
        pThis->mC[i] = i;
    }

    pThis->m8 = 0x9C;
    displayFormationName(10, 1, 0xd);
    pThis->m0_formationState = 0;
    pThis->m1_formationSubState = 0;
}

void baldorQueenForamtion_updateMode0(BTL_A3_BaldorQueenFormation* pThis) {
    char cVar1;
    unsigned int uVar2;
    int iVar3;
    int iVar4;
    int iVar5;

    iVar5 = 0;
    iVar4 = 0;
    iVar3 = 0;
    while (iVar4 < 2) {
        if (((&pThis->m4[1].m48)[iVar3] & 4) == 0) {
            iVar5 = iVar5 + 1;
        }
        if (((&pThis->m4[2].m48)[iVar3] & 4) == 0) {
            iVar5 = iVar5 + 1;
        }
        if (((&pThis->m4[3].m48)[iVar3] & 4) == 0) {
            iVar5 = iVar5 + 1;
        }
        iVar4 = iVar4 + 4;
        if (((&pThis->m4[4].m48)[iVar3] & 4) == 0) {
            iVar5 = iVar5 + 1;
        }
        iVar3 = iVar3 + 0x130;
    }
    while (iVar4 < 6) {
        iVar4 = iVar4 + 1;
        if (((&pThis->m4[1].m48)[iVar3] & 4) == 0) {
            iVar5 = iVar5 + 1;
        }
        iVar3 = iVar3 + 0x4c;
    }
    battleEngine_FlagQuadrantBitForSafety(0);
    battleEngine_FlagQuadrantBitForDanger(0);
    if (iVar5 == 0) {
        battleEngine_FlagQuadrantForDanger(0);
        uVar2 = 2;
    }
    else {
        battleEngine_FlagQuadrantForSafety((int)(char)pThis->mB);
        uVar2 = performModulo(4, pThis->mB + 2);
    }
    battleEngine_FlagQuadrantForDanger(uVar2);
    if ((pThis->m4[0].m48 & 4) != 0) {
        pThis->m0_formationState = 4;
        pThis->m1_formationSubState = 0;
        return;
    }
    if (gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m8 == 0) {
        return;
    }
    iVar3 = BattleEngineSub0_UpdateSub0();
    if (iVar3 != 0) {
        return;
    }
    (gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m8) = 0;
    gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m0 = 0;
    cVar1 = gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant;
    if (cVar1 != '\0') {
        if (cVar1 == '\x01') {
        LAB_BTL_A3__0605785e:
            uVar2 = randomNumber();

            static const std::array<u8, 4> attackList = { {0,1,2,4} };

            battleEngine_PlayAttackCamera(attackList[uVar2 & 3]);
            pThis->m4[0].m49 = 2;
            battleEngine_displayAttackName(0xd, 0x1e, 0);
            pThis->m0_formationState = 2;
            return;
        }
        if (cVar1 != '\x02') {
            if (cVar1 != '\x03') {
                return;
            }
            goto LAB_BTL_A3__0605785e;
        }
    }
    if (pThis->mB == gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant) {
        if (iVar5 == 0) {
        LAB_BTL_A3__060578b6:
            iVar3 = 0;
            pThis->mB = gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant;
            goto LAB_BTL_A3__060578fe;
        }
        if (iVar5 == 6) {
            iVar3 = 1;
            goto LAB_BTL_A3__060578fe;
        }
        uVar2 = randomNumber();
        if ((uVar2 & 1) == 0) {
            iVar3 = 1;
            goto LAB_BTL_A3__060578fe;
        }
    }
    else {
        if (iVar5 == 0) goto LAB_BTL_A3__060578b6;
        if (iVar5 == 6) {
            iVar3 = 2;
            goto LAB_BTL_A3__060578fe;
        }
        uVar2 = randomNumber();
        if ((uVar2 & 1) == 0) {
            iVar3 = 2;
            goto LAB_BTL_A3__060578fe;
        }
    }
    iVar3 = 0;
LAB_BTL_A3__060578fe:
    /*
    if (iVar3 == 0) {
        FUN_BTL_A3__06057438(pThis);
        return;
    }
    if (iVar3 != 1) {
        if (iVar3 != 2) {
            return;
        }
        FUN_BTL_A3__06057500(pThis);
        return;
    }
    FUN_BTL_A3__06057480(pThis);*/
    return;
}

void BTL_A3_BaldorQueenFormation_Update(BTL_A3_BaldorQueenFormation* pThis) {
    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x1B]) {
        assert(0);
    }
    if (pThis->m8 > -1) {
        gBattleManager->m10_battleOverlay->m4_battleEngine->m3B2_numBattleFormationRunning++;
    }
    
    Unimplemented();

    switch (pThis->m0_formationState) {
    case 0:
        baldorQueenForamtion_updateMode0(pThis);
        break;
    default:
        assert(0);
    }

    Unimplemented();
}

p_workArea Create_BTL_A3_BaldorQueenFormation(p_workArea parent, u32 arg) {
    static const BTL_A3_BaldorQueenFormation::TypedTaskDefinition definition = {
        BTL_A3_BaldorQueenFormation_Init,
        BTL_A3_BaldorQueenFormation_Update,
        nullptr,
        nullptr,
    };

    return createSubTaskWithArg<BTL_A3_BaldorQueenFormation>(parent, arg, &definition);
}
