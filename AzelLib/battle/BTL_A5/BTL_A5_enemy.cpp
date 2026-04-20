#include "PDS.h"
#include "BTL_A5_enemy.h"
#include "BTL_A5_data.h"
#include "BTL_A5_formation.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "battle/battleEngine.h"
#include "battle/battleEnemyLifeMeter.h"
#include "battle/BTL_A3/baldor.h"
#include "kernel/graphicalObject.h"
#include "kernel/animation.h"
#include "kernel/fileBundle.h"
#include "kernel/vdp1AnimatedQuad.h"
#include "battle/battleTargetable.h"
#include "battle/battleDamageDisplay.h"
#include "battle/battleEnemyModels.h"
#include "audio/systemSounds.h"
#include "mainMenuDebugTasks.h"
#include "BTL_A5_formation.h"

void setupConditionalLightColor(int param_1);
void clearLightColor();
void initBattleTargetables(p_workArea pThis, s_3dModel* pModel, s16* param3, std::vector<sBattleTargetable>& param4, std::vector<sVec3_FP>& param5);

struct sBTL_A5_Enemy : public s_workAreaTemplateWithCopy<sBTL_A5_Enemy>
{
    s_fileBundle* m0_fileBundle;
    sBTL_A5_FormationTask* m8_formation;
    u32 m14_flags;
    s32 m1C_entityIndex;
    s16 m24_counter;
    u8 m28_state;
    u8 m29_modelCount;
    std::vector<s_3dModel> m2C_models;
    sVec3_FP m30_position;
    sVec3_FP m3C_rotation;
    s32 m4C_velocityY;
    s32 m58_gravityY;
    sVec3_FP m60_targetPosition;
    s32 m6C_deathRotX;
    s32 m70_deathRotY;
    sVec3_FP m84_displayRotation;
    sVec3_FP m90_initialPosition;
    sVec3_FP m9C_targetRotation;
    s32 mA8_soundEffectId;
    sEnemyLifeMeterTask* mAC_lifeMeter;
    s16 mB0;
    s16 mB2_health;
    s16 mB4;
    s16 mB6;
    u32 mB8_damageCallback0;
    u32 mBC_damageCallback1;
    u32 mC0_damageCallback2;
    u32 mC4_damageCallback3;
    u8 mC8_pad[0x144 - 0xC8];
    s16 m144_hitbox0;
    s16 m146_hitbox1;
    s16 m148_hitbox2;
    s16 m14A_hitbox3;
    s16 m14C_lightColor;
    u8 m14E_pad2[0x150 - 0x14E];
    u8 m150;
    u8 m151_pad[3];
    u32 m154;
    sBTL_A5_FormationTask* m158_formationData;
    u8 m15C_pad3[0x16C - 0x15C];
    s32 m16C_isLead;
    u8 m170_pad4[0x17C - 0x170];
    s16 m17C_hitFlashCount;
    s16 m17E_hitFlashColor;
    s32 m180_movementAngleX;
    s32 m184_movementX;
    s32 m188_movementAccelX;
    s32 m18C_movementAngleZ;
    s32 m190_movementZ;
    s32 m194_movementAccelZ;
    sVec3_FP m198_orbitVelocity;
    s16 m1A4_timer;
    s16 m1A6_timerMax;
    // size 0x1A8
};

// 060a3bc0
static void BTL_A5_enemy_Draw(sBTL_A5_Enemy* pThis)
{
    if (pThis->m14_flags & 0x800000)
    {
        setupConditionalLightColor(pThis->m14C_lightColor);
    }

    for (int i = 0; i < pThis->m29_modelCount; i++)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->m30_position);
        rotateCurrentMatrixYXZ(&pThis->m3C_rotation);
        s_3dModel* pModel = &pThis->m2C_models[i];
        pModel->m18_drawFunction(pModel);
        popMatrix();
    }

    if (pThis->m14_flags & 0x800000)
    {
        pThis->m14_flags &= ~0x800000;
        clearLightColor();
    }
}

// 060a3abc
static void BTL_A5_enemy_cleanup(sBTL_A5_Enemy* pThis, void (*callback)(sBTL_A5_Enemy*))
{
    pThis->m14_flags |= 0x80000000;
    pThis->m14_flags &= ~0x40000000;
    if (callback)
    {
        callback(pThis);
    }
    if (!(pThis->m14_flags & 0x10000000))
    {
        pThis->m14_flags |= 0x10000000;
        for (int i = 0; i < pThis->m29_modelCount; i++)
        {
            s_3dModel* pModel = &pThis->m2C_models[i];
            for (int j = 0; j < pModel->m52_targetableCount; j++)
            {
                deleteTargetable(&pModel->m54_targetables[j]);
            }
        }
    }
}

// 060a3c88
static void BTL_A5_enemy_Delete(sBTL_A5_Enemy* pThis)
{
    BTL_A5_enemy_cleanup(pThis, nullptr);
}

// 0605f09e
static void BTL_A5_enemy_updateMovement(sBTL_A5_Enemy* pThis)
{
    pThis->m184_movementX += pThis->m188_movementAccelX;
    pThis->m190_movementZ += pThis->m194_movementAccelZ;
    pThis->m180_movementAngleX += pThis->m184_movementX;
    pThis->m18C_movementAngleZ += pThis->m190_movementZ;
}

// 060a36d6 — simplified damage check
static bool BTL_A5_enemy_checkDamage(sBTL_A5_Enemy* pThis)
{
    if (pThis->m14_flags & 0x10000000)
        return false;

    bool damaged = false;
    for (int i = 0; i < pThis->m29_modelCount; i++)
    {
        s_3dModel* pModel = &pThis->m2C_models[i];
        for (int j = 0; j < pModel->m52_targetableCount; j++)
        {
            sBattleTargetable& t = pModel->m54_targetables[j];
            if (t.m50_flags & 0x80000)
            {
                t.m50_flags &= ~0x80000;
                t.m50_flags &= ~0x20000;
                damaged = true;
                pThis->mB2_health -= t.m58;
                if (pThis->mB2_health >= 0)
                {
                    createHitSparkEffect(pThis, t, 0);
                }
                if (pThis->mAC_lifeMeter)
                {
                    pThis->mAC_lifeMeter->m31 |= 2;
                }
            }
        }
    }
    return damaged;
}

// 0605e54a
static void BTL_A5_enemy_Update(sBTL_A5_Enemy* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;

    // death fall state
    if (pThis->m28_state == 7)
    {
        pThis->m3C_rotation.m0_X = pThis->m3C_rotation.m0_X - pThis->m6C_deathRotX;
        pThis->m3C_rotation.m4_Y = pThis->m3C_rotation.m4_Y + pThis->m70_deathRotY;
        pThis->m4C_velocityY += pThis->m58_gravityY;
        pThis->m30_position.m4_Y = pThis->m30_position.m4_Y + pThis->m4C_velocityY;
        if ((s32)pThis->m30_position.m4_Y < 0)
        {
            pThis->m30_position.m4_Y = 0;
            pThis->m28_state = 8;
            playSystemSoundEffect(0x66);
        }
        return;
    }

    // explosion + destroy
    if (pThis->m28_state == 8)
    {
        pThis->getTask()->markFinished();
        return;
    }

    // check for damage (only in alive state)
    if (pThis->m28_state == 0)
    {
        bool wasDamaged = BTL_A5_enemy_checkDamage(pThis);
        if (wasDamaged && pThis->mB2_health <= 0)
        {
            playSystemSoundEffect(0x65);
            pThis->m28_state = 7;
            if (pThis->m16C_isLead == 1)
            {
                pThis->m8_formation->mF0_flag++;
            }
            pThis->m8_formation->mE0_entityGroup.mF_deadCount++;
            return;
        }
    }

    pThis->m1A4_timer++;
    s32 absMovX = pThis->m184_movementX < 0 ? -pThis->m184_movementX : pThis->m184_movementX;
    if (absMovX > 0xAAAAAA)
    {
        pThis->m1A6_timerMax = 0xF;
        pThis->m188_movementAccelX = (pThis->m184_movementX < 1) ? 0x91A2 : (s32)0xFFFF6E5E;
    }
    s32 absMovZ = pThis->m190_movementZ < 0 ? -pThis->m190_movementZ : pThis->m190_movementZ;
    if (absMovZ > 0xAAAAAA)
    {
        pThis->m1A6_timerMax = 0xF;
        pThis->m194_movementAccelZ = (pThis->m190_movementZ < 1) ? 0x91A2 : (s32)0xFFFF6E5E;
    }

    if (pThis->m1A4_timer > pThis->m1A6_timerMax)
    {
        u32 r = randomNumber();
        u32 rDir = randomNumber();
        s32 accelX;
        if (rDir & 0x8000000)
        {
            accelX = (s32)MTH_Mul((randomNumber() | 0xF0000000), 0x20);
        }
        else
        {
            accelX = (s32)MTH_Mul((randomNumber() & 0xFFFFFFF), 0x20);
        }
        pThis->m188_movementAccelX = accelX;

        rDir = randomNumber();
        s32 accelZ;
        if (rDir & 0x8000000)
        {
            accelZ = (s32)MTH_Mul((randomNumber() | 0xF0000000), 0x20);
        }
        else
        {
            accelZ = (s32)MTH_Mul((randomNumber() & 0xFFFFFFF), 0x20);
        }
        pThis->m194_movementAccelZ = accelZ;
        pThis->m1A6_timerMax = (s16)(r >> 0x1B);
        pThis->m1A4_timer = 0;
    }

    BTL_A5_enemy_updateMovement(pThis);

    if (pThis->mB2_health > 0)
    {
        pThis->m198_orbitVelocity.m0_X = MTH_Mul(0xC000, MTH_Mul(getCos(pThis->m180_movementAngleX >> 16), getCos(pThis->m18C_movementAngleZ >> 16)));
        pThis->m198_orbitVelocity.m4_Y = MTH_Mul(0xC000, getSin(pThis->m180_movementAngleX >> 16));
        pThis->m198_orbitVelocity.m8_Z = MTH_Mul(MTH_Mul(0xC000, getCos(pThis->m180_movementAngleX >> 16)), getSin(pThis->m18C_movementAngleZ >> 16));
    }

    stepAnimation(&pThis->m2C_models[0]);

    pThis->m84_displayRotation.m0_X = 0;
    pThis->m84_displayRotation.m4_Y = 0x8000000;
    pThis->m84_displayRotation.m8_Z = 0;

    // position = initial formation position + orbit velocity + battle center
    pThis->m60_targetPosition.m0_X = pThis->m90_initialPosition.m0_X + pThis->m198_orbitVelocity.m0_X;
    pThis->m60_targetPosition.m4_Y = pThis->m90_initialPosition.m4_Y + pThis->m198_orbitVelocity.m4_Y;
    pThis->m60_targetPosition.m8_Z = pThis->m90_initialPosition.m8_Z + pThis->m198_orbitVelocity.m8_Z;

    // simple position interpolation toward target (move 1/4 of the way each frame)
    pThis->m30_position.m0_X = pThis->m30_position.m0_X + ((s32)(pThis->m60_targetPosition.m0_X + pEngine->mC_battleCenter.m0_X) - (s32)pThis->m30_position.m0_X) / 4;
    pThis->m30_position.m4_Y = pThis->m30_position.m4_Y + ((s32)(pThis->m60_targetPosition.m4_Y + pEngine->mC_battleCenter.m4_Y) - (s32)pThis->m30_position.m4_Y) / 4;
    pThis->m30_position.m8_Z = pThis->m30_position.m8_Z + ((s32)(pThis->m60_targetPosition.m8_Z + pEngine->mC_battleCenter.m8_Z) - (s32)pThis->m30_position.m8_Z) / 4;

    // auto-scroll
    pThis->m30_position.m0_X = pThis->m30_position.m0_X + pEngine->m1A0_battleAutoScrollDelta.m0_X;
    pThis->m30_position.m4_Y = pThis->m30_position.m4_Y + pEngine->m1A0_battleAutoScrollDelta.m4_Y;
    pThis->m30_position.m8_Z = pThis->m30_position.m8_Z + pEngine->m1A0_battleAutoScrollDelta.m8_Z;
}



// 0605e220
p_workArea BTL_A5_createEnemy(sBTL_A5_FormationTask* pParent, s32 dataTableAddr, s32 entityIndex, s32 isLead)
{
    static const sBTL_A5_Enemy::TypedTaskDefinition def = {
        nullptr,
        &BTL_A5_enemy_Update,
        &BTL_A5_enemy_Draw,
        &BTL_A5_enemy_Delete,
    };

    s32 iVar6 = 0;
    sBTL_A5_Enemy* pEntity = createSubTaskWithCopy<sBTL_A5_Enemy>((s_workAreaCopy*)pParent, &def);
    if (!pEntity)
        return nullptr;

    pEntity->m0_fileBundle = dramAllocatorEnd[2].mC_fileBundle->m0_fileBundle;
    pEntity->m8_formation = pParent;
    pEntity->m1C_entityIndex = entityIndex;
    pEntity->m158_formationData = pParent;
    pEntity->m16C_isLead = isLead;

    if (isLead == 1)
    {
        iVar6 = 1;
        pEntity->m17C_hitFlashCount = 0x17;
        pEntity->m17E_hitFlashColor = 0xE;
    }
    else
    {
        pEntity->m17C_hitFlashCount = 7;
        if (gBattleManager->m6_subBattleId == 7 || gBattleManager->m6_subBattleId == 8)
            pEntity->m17E_hitFlashColor = 4;
        else
            pEntity->m17E_hitFlashColor = 2;
    }

    s16 enemyTypeId = readSaturnS16(pParent->mE0_entityGroup.m8_dataTable + 0xC) + iVar6;
    pEntity->mAC_lifeMeter = createEnemyLifeMeterTask(
        &pEntity->m30_position, 0, &pEntity->mB2_health, enemyTypeId);

    s32 result = createBattleEnemyModels(pEntity, pEntity->m0_fileBundle, pEntity->m29_modelCount, pEntity->m2C_models, g_BTL_A5->getSaturnPtr(0x060b1130 + iVar6 * 8), 1);
    if (result == 0)
        return nullptr;

    initAnimation(&pEntity->m2C_models[0], pEntity->m0_fileBundle->getAnimation(0x34));

    s32 animFrameCount = 0;
    if (pEntity->m2C_models[0].m30_pCurrentAnimation != nullptr)
    {
        animFrameCount = (s32)pEntity->m2C_models[0].m30_pCurrentAnimation->m4_numFrames;
    }
    u32 randomFrame = performModulo2(animFrameCount - 1, randomNumber());
    setAnimationFrame(&pEntity->m2C_models[0], randomFrame);
    stepAnimation(&pEntity->m2C_models[0]);

    initBattleEnemyTargetables(pEntity, pEntity->m29_modelCount, pEntity->m2C_models);

    pEntity->m1A6_timerMax = (u16)(randomNumber() >> 0x1b) + 0x19;
    pEntity->m1A4_timer = 0;
    pEntity->m194_movementAccelZ = 0;
    pEntity->m188_movementAccelX = 0;

    u32 rnd = randomNumber();
    if ((rnd & 0x8000000) == 0)
    {
        rnd = randomNumber() & 0xfffffff;
    }
    else
    {
        rnd = randomNumber() | 0xf0000000;
    }
    pEntity->m184_movementX = MTH_Mul(rnd, 0x28f);

    rnd = randomNumber();
    if ((rnd & 0x8000000) == 0)
    {
        rnd = randomNumber() & 0xfffffff;
    }
    else
    {
        rnd = randomNumber() | 0xf0000000;
    }
    pEntity->m190_movementZ = MTH_Mul(rnd, 0x28f);
 
    BTL_A5_enemy_updateMovement(pEntity);

    pEntity->m90_initialPosition = readSaturnVec3(pParent->mE0_entityGroup.m8_dataTable + entityIndex * 0xC);
    pEntity->m60_targetPosition = pEntity->m90_initialPosition;

    pEntity->mA8_soundEffectId = 0xA3D;
    pEntity->m3C_rotation.m0_X = 0;
    pEntity->m3C_rotation.m4_Y = 0x8000000;
    pEntity->m3C_rotation.m8_Z = 0;

    pEntity->m9C_targetRotation = pEntity->m3C_rotation;
    pEntity->m84_displayRotation = pEntity->m3C_rotation;

    return pEntity;
}
