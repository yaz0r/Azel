#include "PDS.h"
#include "baldorQueen.h"
#include "baldor.h"
#include "BTL_A3_data.h"
#include "battle/battleGenericData.h"
#include "battle/battleManager.h"
#include "battle/battleDebug.h"
#include "battle/battleEngine.h"
#include "kernel/animation.h"
#include "battle/particleEffect.h"
#include "battle/battleTargetable.h"
#include "audio/systemSounds.h"
#include "kernel/graphicalObject.h"
#include "kernel/fileBundle.h"
#include "battle/battleDamageDisplay.h"
#include "battle/battleDragon.h"

void Baldor_initSub0(sBaldorBase* pThis, sSaturnPtr dataPtr, sFormationData* pFormationEntry, s32 arg); // TODO: cleanup
void urchinUpdateSub3(s_3dModel* pModel, std::vector<sVec3_FP>& pPosition); // TODO: cleanup
s32 checkTargetablesForDamage(p_workArea pThis, std::vector<sBattleTargetable>& param2, s16 entriesToParse, s16& param4); // TODO: cleanup
void processHitTargetables(p_workArea pThis, std::vector<sBattleTargetable>& param2, int param3, int param4, struct sEnemyLifeMeterTask* param5); // TODO: cleanup

struct sBaldorQueen : public sBaldorBase
{
    s16 m44_timer;
    // size 0x48
};

void BaldorQueen_init(sBaldorBase* pThisBase, sFormationData* pFormationData) {
    sBaldorQueen* pThis = (sBaldorQueen*)pThisBase;
    Baldor_initSub0(pThis, g_BTL_A3->getSaturnPtr(0x60A75CC), pFormationData, 0);
}

// 06057108
void BaldorQueen_processDamage(sBaldorQueen* pThis) {
    if (pThis->m34_formationEntry->m48 & 4)
        return;

    s16 damage = 0;
    s32 wasHit = checkTargetablesForDamage(pThis, pThis->m14_targetable, pThis->mC_numTargetables, damage);
    if (wasHit)
    {
        pThis->m12_damagePending = 1;
        pThis->mE_damageValue += damage;
        pThis->mB_flags |= 8;
        playSystemSoundEffect(0x65);

        // spawn hit particles at each hit targetable
        for (int i = 0; i < pThis->mC_numTargetables; i++)
        {
            if (pThis->m14_targetable[i].m50_flags & 0x80000)
            {
                sVec3_FP worldPos;
                transformAndAddVec(pThis->m18_position[i], worldPos, cameraProperties2.m28[1]);
                for (int j = 0; j < 2; j++)
                {
                    sVec3_FP velocity;
                    velocity[0] = -pThis->m14_targetable[i].m34_impactVector[0] + fixedPoint((s32)((randomNumber() & 0x1FFF) - 0xFFF));
                    velocity[1] = -pThis->m14_targetable[i].m34_impactVector[1] + fixedPoint((s32)((randomNumber() & 0x1FFF) - 0xFFF));
                    velocity[2] = -pThis->m14_targetable[i].m34_impactVector[2] + fixedPoint((s32)((randomNumber() & 0x1FFF) - 0xFFF));
                    createParticleEffect(dramAllocatorEnd[0x10].mC_fileBundle, &g_BTL_GenericData->m_0x60a8b0c_animatedQuad, &worldPos, &velocity, nullptr, 0x10000, 0, 0);
                }
            }
        }

        pThis->m10_HP -= damage;
        if (pThis->m10_HP < 1)
        {
            // death
            processHitTargetables(pThis, pThis->m14_targetable, pThis->mC_numTargetables, 0, pThis->m40_enemyLifeMeterTask);
            pThis->m34_formationEntry->m48 |= 4;
            createDamageDisplayTask(pThis, pThis->mE_damageValue, pThis->m1C_translation.m0_current, 0);
            for (int i = 0; i < pThis->mC_numTargetables; i++)
            {
                pThis->m14_targetable[i].m50_flags &= ~0x20000;
            }
            pThis->m8_mode = 0xB;
            pThis->m9_attackStatus = 0;
            return;
        }
        processHitTargetables(pThis, pThis->m14_targetable, pThis->mC_numTargetables, 1, pThis->m40_enemyLifeMeterTask);
    }

    // display accumulated damage when m1000 flag is set
    if (pThis->m12_damagePending && gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m1000)
    {
        createDamageDisplayTask(pThis, pThis->mE_damageValue, pThis->m1C_translation.m0_current, 0);
        pThis->mE_damageValue = 0;
        pThis->m12_damagePending = 0;
    }

    // set per-quadrant collision offset based on dragon position
    u8 quadrant = gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant;
    u8 collisionOffset = (quadrant == 0 || quadrant == 2) ? 0x14 : 5;
    for (int i = 0; i < pThis->mC_numTargetables; i++)
    {
        pThis->m14_targetable[i].m60 = collisionOffset;
    }
}

void BaldorQueen_updateMode0(sBaldorQueen* pThis) {
    stepAnimation(pThis->m38_3dModel);
    if (pThis->m10_HP < 415)
    {
        stepAnimation(pThis->m38_3dModel); // double animation speed when damaged
    }
    if ((randomNumber() & 0x1F) == 0) {
        sVec3_FP tempVec;
        transformAndAddVec(pThis->m38_3dModel->m44_hotpointData[1][3], tempVec, cameraProperties2.m28[1]);
        for (int i = 0; i < 10; i++) {
            sVec3_FP spawnLocation;
            spawnLocation[0] = (randomNumber() & 0xFFF) - 0x7FF;
            spawnLocation[1] = (randomNumber() & 0xFFF) - 0x17FF;
            spawnLocation[2] = (randomNumber() & 0xFFF) - 0x7FF;

            sVec3_FP tempVector2 = {0, -0x2C, 0};
            createParticleEffect(dramAllocatorEnd[readSaturnU8(pThis->m3C_dataPtr)].mC_fileBundle, &g_BTL_GenericData->m_0x60a8c24_animatedQuad, &tempVec, &spawnLocation, &tempVector2, 0x10000, 0, (randomNumber() & 0x1F) + 0x18);
        }
    }
    if (pThis->m34_formationEntry->m49 == 2) {
        pThis->m8_mode = 1;
        pThis->m34_formationEntry->m48 |= 1;
        pThis->m34_formationEntry->m49 = 0;
    }
}

p_workArea BaldorAttack_createAttackModel(sVec3_FP* partPosition, sVec3_FP* target, sSaturnPtr param3); // from baldor.cpp
p_workArea BaldorAttack_createQueenAcidProjectile(sVec3_FP* partPosition, sVec3_FP* target, u8 fileBundleIdx, s16 timer, s16 lifetime); // from baldor.cpp

// 06054e88
void BaldorQueen_updateMode1(sBaldorQueen* pThis) {
    stepAnimation(pThis->m38_3dModel);
    if (pThis->m10_HP < 415) {
        stepAnimation(pThis->m38_3dModel);
    }
    switch (pThis->m9_attackStatus) {
    case 0:
        if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2000 &&
            pThis->m34_formationEntry->m49 == 1) {
            pThis->m34_formationEntry->m48 &= ~1;
            pThis->m34_formationEntry->m48 |= 2;
            pThis->m9_attackStatus++;
        }
        break;
    case 1:
    {
        sVec3_FP spawnPos;
        transformAndAddVec(pThis->m38_3dModel->m44_hotpointData[0][0], spawnPos, cameraProperties2.m28[1]);
        u8 fileBundleIdx = readSaturnU8(pThis->m3C_dataPtr);

        for (int i = 0; i < 30; i++) {
            sVec3_FP target;
            target[0] = gBattleManager->m10_battleOverlay->m18_dragon->m8_position[0] + fixedPoint((s32)((randomNumber() & 0xFFFF) - 0x7FFF));
            target[1] = gBattleManager->m10_battleOverlay->m18_dragon->m8_position[1] + fixedPoint((s32)((randomNumber() & 0xFFFF) - 0x7FFF));
            target[2] = gBattleManager->m10_battleOverlay->m18_dragon->m8_position[2] + fixedPoint((s32)((randomNumber() & 0xFFFF) - 0x7FFF));
            u16 rndVal = (u16)randomNumber() & 0x1F;
            BaldorAttack_createQueenAcidProjectile(&spawnPos, &target, fileBundleIdx, (s16)(rndVal + 0xF), (s16)(rndVal + 0x15));
            playSystemSoundEffect(0x67);
        }
        pThis->m44_timer = 0x2D;
        pThis->m9_attackStatus++;
        break;
    }
    case 2:
        if (--pThis->m44_timer < 0) {
            pThis->m34_formationEntry->m48 &= ~2;
            pThis->m8_mode = 0;
            pThis->m9_attackStatus = 0;
        }
        break;
    }
}

// 06055088
void BaldorQueen_updateMode11(sBaldorQueen* pThis) {
    switch (pThis->m9_attackStatus) {
    case 0:
        if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2000 &&
            pThis->m34_formationEntry->m49 == 1) {
            pThis->m34_formationEntry->m48 |= 2;
            for (int i = 0; i < pThis->mC_numTargetables; i++) {
                deleteTargetable(&pThis->m14_targetable[i]);
            }
            pThis->mC_numTargetables = 0;
            ((u8*)pThis->m40_enemyLifeMeterTask)[0x31] |= 1;
            ((u8*)pThis->m40_enemyLifeMeterTask)[0x31] |= 8;
            initAnimation(pThis->m38_3dModel, dramAllocatorEnd[readSaturnU8(pThis->m3C_dataPtr)].mC_fileBundle->m0_fileBundle->getAnimation(0x118));
            playSystemSoundEffect(0x6B);
            pThis->m9_attackStatus++;
        }
        stepAnimation(pThis->m38_3dModel);
        break;
    case 1:
    {
        transformAndAddVec(pThis->m18_position[1], gBattleManager->m10_battleOverlay->m4_battleEngine->m3F4_cameraPositionWhileShooting, cameraProperties2.m28[1]);
        s32 numFrames = 0;
        if (pThis->m38_3dModel->m30_pCurrentAnimation != nullptr) {
            numFrames = pThis->m38_3dModel->m30_pCurrentAnimation->m4_numFrames;
        }
        if (pThis->m38_3dModel->m16_previousAnimationFrame < numFrames - 1) {
            stepAnimation(pThis->m38_3dModel);
        }
        break;
    }
    }
}

void BaldorQueen_update(sBaldorBase* pThisBase) {
    sBaldorQueen* pThis = (sBaldorQueen*)pThisBase;
    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x1B]) {
        assert(0);
    }
    *pThis->m1C_translation.m0_current = gBattleManager->m10_battleOverlay->m4_battleEngine->mC_battleCenter + *pThis->m1C_translation.m4_target;
    urchinUpdateSub3(pThis->m38_3dModel, pThis->m18_position);
    BaldorQueen_processDamage(pThis);
    switch (pThis->m8_mode) {
    case 0:
        BaldorQueen_updateMode0(pThis);
        break;
    case 1: // acid spray attack
        BaldorQueen_updateMode1(pThis);
        break;
    case 0xB: // death sequence
        BaldorQueen_updateMode11(pThis);
        break;
    default:
        assert(0);
    }
}

void BaldorQueen_draw(sBaldorBase* pThisBase) {
    sBaldorQueen* pThis = (sBaldorQueen*)pThisBase;
    pushCurrentMatrix();
    translateCurrentMatrix(pThis->m1C_translation.m0_current);
    rotateCurrentMatrixYXZ(pThis->m28_rotation.m0_current);
    pThis->m38_3dModel->m18_drawFunction(pThis->m38_3dModel);
    popMatrix();
}

p_workArea baldorQueenCreateQueenTask(s_workAreaCopy* pParent, sFormationData* param_2) {

    static const sBaldorQueen::TypedTaskDefinition definition = {
        BaldorQueen_init,
        BaldorQueen_update,
        BaldorQueen_draw,
        nullptr,
    };
    return createSubTaskWithArgWithCopy<sBaldorQueen, sFormationData*>(pParent, param_2, &definition);
}
