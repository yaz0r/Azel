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
#include "battle/battleIntro.h"
#include "BTL_A3_data.h"
#include "baldor.h"
#include "audio/systemSounds.h"

void battleEngine_PlayAttackCamera(int param1); // TODO cleanup
void battleEngine_displayAttackName(int param1, int param2, int param3); // TODO: Cleanup
void createDamageSpriteEffect(npcFileDeleter* param1, sSaturnPtr param2, const sVec3_FP* param3, sVec3_FP* param4, sVec3_FP* param5, s32 param6, s32 param7, s32 param8); // TODO: cleanup

struct BTL_A3_BaldorQueenFormation : public s_workAreaTemplateWithArg<BTL_A3_BaldorQueenFormation, u32>
{
    u8 m0_formationState;
    u8 m1_formationSubState;
    std::vector<sFormationData> m4;
    s16 m8;
    u8 mB;
    std::array<u8, 6> mC;
    u8 mA;
    s16 m14;
    p_workArea m18;
    fixedPoint m1C;
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

void baldorQueenFormation_goToMode3(BTL_A3_BaldorQueenFormation* pThis)
{
    battleEngine_SetBattleMode(eBattleModes::m7);
    battleEngine_displayAttackName(0xc, 0x1e, 0);
    pThis->m0_formationState = 3;
    return;
}

void baldorQueenFormation_goToMode1(BTL_A3_BaldorQueenFormation* pThis)
{
    battleEngine_SetBattleMode(eBattleModes::m7);
    int count = 0;
    for (int i = 0; i < 6; i++) {
        if ((pThis->m4[i + 1].m48 & 4) == 0)
        {
            count++;
            pThis->m4[i + 1].m49 = 2;
            if (count > 2) {
                break;
            }
        }
    }
    battleEngine_displayAttackName(1, 0x1e, 0);
    battleEngine_FlagQuadrantForAttack(pThis->mB);
    pThis->m0_formationState = 1;
    return;
}

void baldorQueenFormation_updateMode0(BTL_A3_BaldorQueenFormation* pThis) {
    char cVar1;
    unsigned int uVar2;
    int targetMode;
    int iVar4;
    int iVar5;

    iVar5 = 0;
    iVar4 = 0;
    targetMode = 0;
    while (iVar4 < 2) {
        if (((&pThis->m4[1].m48)[targetMode] & 4) == 0) {
            iVar5 = iVar5 + 1;
        }
        if (((&pThis->m4[2].m48)[targetMode] & 4) == 0) {
            iVar5 = iVar5 + 1;
        }
        if (((&pThis->m4[3].m48)[targetMode] & 4) == 0) {
            iVar5 = iVar5 + 1;
        }
        iVar4 = iVar4 + 4;
        if (((&pThis->m4[4].m48)[targetMode] & 4) == 0) {
            iVar5 = iVar5 + 1;
        }
        targetMode = targetMode + 0x130;
    }
    while (iVar4 < 6) {
        iVar4 = iVar4 + 1;
        if (((&pThis->m4[1].m48)[targetMode] & 4) == 0) {
            iVar5 = iVar5 + 1;
        }
        targetMode = targetMode + 0x4c;
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
    targetMode = BattleEngineSub0_UpdateSub0();
    if (targetMode != 0) {
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
            targetMode = 0;
            pThis->mB = gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant;
            goto LAB_BTL_A3__060578fe;
        }
        if (iVar5 == 6) {
            targetMode = 1;
            goto LAB_BTL_A3__060578fe;
        }
        uVar2 = randomNumber();
        if ((uVar2 & 1) == 0) {
            targetMode = 1;
            goto LAB_BTL_A3__060578fe;
        }
    }
    else {
        if (iVar5 == 0) goto LAB_BTL_A3__060578b6;
        if (iVar5 == 6) {
            targetMode = 2;
            goto LAB_BTL_A3__060578fe;
        }
        uVar2 = randomNumber();
        if ((uVar2 & 1) == 0) {
            targetMode = 2;
            goto LAB_BTL_A3__060578fe;
        }
    }
    targetMode = 0;
LAB_BTL_A3__060578fe:

    switch (targetMode) {
    case 0:
        baldorQueenFormation_goToMode3(pThis);
        break;
    case 1:
        baldorQueenFormation_goToMode1(pThis);
        break;
    default:
        assert(0);
    }
}

void baldorQueenFormation_updateMode3(BTL_A3_BaldorQueenFormation* pThis) {
    switch (pThis->m1_formationSubState) {
    case 0:
        if ((gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2000) == 0) {
            return;
        }
        if (pThis->mB == 0) {
            pThis->m1C = -0x11c71c7;
        }
        else {
            pThis->m1C = 0x6e38e38;
        }

        gBattleManager->m10_battleOverlay->m4_battleEngine->m3E8 = gBattleManager->m10_battleOverlay->m4_battleEngine->mC_battleCenter;

        gBattleManager->m10_battleOverlay->m4_battleEngine->m3E8[1] += MTH_Mul(-0x1e000, getSin(pThis->m1C.getInteger())) - 0x50000;
        gBattleManager->m10_battleOverlay->m4_battleEngine->m3E8[0] += MTH_Mul(0x50000, getSin(pThis->m1C.getInteger()));
        gBattleManager->m10_battleOverlay->m4_battleEngine->m3E8[2] += MTH_Mul(0x64000, getCos(pThis->m1C.getInteger()));

        (gBattleManager->m10_battleOverlay->m4_battleEngine->m3F4_cameraPositionWhileShooting) = (gBattleManager->m10_battleOverlay->m4_battleEngine->mC_battleCenter);

        gBattleManager->m10_battleOverlay->m4_battleEngine->m3F4_cameraPositionWhileShooting[1] += 0x1E000;

        battleEngine_setDesiredCameraPositionPointer(&gBattleManager->m10_battleOverlay->m4_battleEngine->m3F4_cameraPositionWhileShooting);
        battleEngine_setCurrentCameraPositionPointer(&gBattleManager->m10_battleOverlay->m4_battleEngine->m3E8);
        createBattleIntroTaskSub0();
        battleEngine_InitSub8();
        pThis->m1_formationSubState++;
        break;
    case 1:
        pThis->m1C += 0x2d82d;

        gBattleManager->m10_battleOverlay->m4_battleEngine->m3E8 = gBattleManager->m10_battleOverlay->m4_battleEngine->mC_battleCenter;

        gBattleManager->m10_battleOverlay->m4_battleEngine->m3E8[1] += MTH_Mul(-0x1e000, getSin(pThis->m1C.getInteger())) - 0x50000;
        gBattleManager->m10_battleOverlay->m4_battleEngine->m3E8[0] += MTH_Mul(0x50000, getSin(pThis->m1C.getInteger()));
        gBattleManager->m10_battleOverlay->m4_battleEngine->m3E8[2] += MTH_Mul(0x64000, getCos(pThis->m1C.getInteger()));

        (gBattleManager->m10_battleOverlay->m4_battleEngine->m3F4_cameraPositionWhileShooting) = (gBattleManager->m10_battleOverlay->m4_battleEngine->mC_battleCenter);

        gBattleManager->m10_battleOverlay->m4_battleEngine->m3F4_cameraPositionWhileShooting[1] += 0x1E000;

        // Try to spawn a new Baldor
        for (int i = 0; i < 6; i++) {
            if (pThis->m4[i + 1].m48 & 4) {
                pThis->m4[i + 1].m0_translation.m18 = readSaturnVec3(g_BTL_A3->getSaturnPtr(0x60a8d78) + pThis->mC[i] * 12 + pThis->mB * 0x48);

                if (pThis->mB == 0) {
                    pThis->m4[i + 1].m24_rotation.mC_target[1] = 0;
                }
                else {
                    pThis->m4[i + 1].m24_rotation.mC_target[1] = 0x8000000;
                }

                sVec3_FP temp;
                pThis->m4[i + 1].m0_translation.mC_target[0] = gBattleManager->m10_battleOverlay->m4_battleEngine->mC_battleCenter[0];
                temp[1] = pThis->m4[i + 1].m0_translation.mC_target[1] = gBattleManager->m10_battleOverlay->m4_battleEngine->mC_battleCenter[1] + 0xF000;
                pThis->m4[i + 1].m0_translation.mC_target[2] = gBattleManager->m10_battleOverlay->m4_battleEngine->mC_battleCenter[2];

                sBaldor* pNewBaldor = createBaldor(dramAllocatorEnd[0x10].mC_fileBundle, &pThis->m4[i+1]);
                pNewBaldor->mA_indexInFormation = i;
                for (int i = 0; i < 20; i++) {
                    sVec3_FP randPosition;
                    randPosition[0] = (randomNumber() & 0xFFF) - 0x7FF;
                    randPosition[1] = (randomNumber() & 0xFFF) - 0x7FF;
                    randPosition[2] = (randomNumber() & 0xFFF) - 0x7FF;
                    if (pThis->mB == 2) {
                        randPosition[2] = -randPosition[2];
                    }
                    createDamageSpriteEffect(dramAllocatorEnd[0x10].mC_fileBundle, g_BTL_A3->getSaturnPtr(0x60a8fb4), &temp, &randPosition, nullptr, 0x10000, 0, (randomNumber() & 0xF) + 8);
                    playSystemSoundEffect(0x66);
                    pThis->m1_formationSubState = pThis->m1_formationSubState + 1;
                    pThis->m14 = 0x1b;
                    return;
                }
            }
        }
        // Fallback if we failed to spawn a new Baldor
        pThis->m1_formationSubState = 10;
        break;
    case 2:
        pThis->m1C += 0x2d82d;
        gBattleManager->m10_battleOverlay->m4_battleEngine->m3E8 = gBattleManager->m10_battleOverlay->m4_battleEngine->mC_battleCenter;

        gBattleManager->m10_battleOverlay->m4_battleEngine->m3E8[1] += MTH_Mul(-0x1e000, getSin(pThis->m1C.getInteger())) - 0x50000;
        gBattleManager->m10_battleOverlay->m4_battleEngine->m3E8[0] += MTH_Mul(0x50000, getSin(pThis->m1C.getInteger()));
        gBattleManager->m10_battleOverlay->m4_battleEngine->m3E8[2] += MTH_Mul(0x64000, getCos(pThis->m1C.getInteger()));

        (gBattleManager->m10_battleOverlay->m4_battleEngine->m3F4_cameraPositionWhileShooting) = (gBattleManager->m10_battleOverlay->m4_battleEngine->mC_battleCenter);

        gBattleManager->m10_battleOverlay->m4_battleEngine->m3F4_cameraPositionWhileShooting[1] += 0x1E000;

        if(--pThis->m14 > -1) {
            return;
        }
        pThis->m1_formationSubState = 1;
        break;
    case 10:
        sEnemyAttackCamera_updateSub2();
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished = 1;
        pThis->m1_formationSubState = 0;
        pThis->m0_formationState = 0;
        break;
    default:
        assert(0);
    }

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
        baldorQueenFormation_updateMode0(pThis);
        break;
    case 3:
        baldorQueenFormation_updateMode3(pThis); // Spawn new Baldor
        break;
    default:
        assert(0);
    }
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
