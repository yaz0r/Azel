#include "PDS.h"
#include "debugWindows.h"
#include "Arachnoth.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "battle/battleDebug.h"
#include "battle/battleEngine.h"
#include "battle/battleEnemyLifeMeter.h"
#include "battle/battleTextDisplay.h"
#include "battle/battleEngineSub0.h"
#include "battle/battleIntro.h"
#include "battle/battleDragon.h"
#include "battle/battleGrid.h"
#include "BTL_A3_2_data.h"
#include "kernel/fileBundle.h"
#include "kernel/animation.h"
#include "kernel/graphicalObject.h"
#include "audio/systemSounds.h"
#include "ArachnothSubPart.h"
#include "ArachnothTentacle.h"
#include "BTL_A3_2_particles.h"
#include "battle/battleTrail.h"
#include "battle/battleDebris.h"
#include "kernel/vdp1Allocator.h"
#include "battle/battleDamageNumber.h"

//https://youtu.be/Txks9hG21qs?t=3130

//Enrage:
//https://youtu.be/Txks9hG21qs?t=3194

//Charge:
//https://youtu.be/Txks9hG21qs?t=3225

//Tentacle:
//https://www.youtube.com/watch?v=s7ei8s5Smuw&t=224s

void battleEngine_displayAttackName(int param1, int param2, int param3); // TODO: Cleanup
s32 playBattleSoundEffect(s32 effectIndex); // TODO: cleanup

struct sArachnothFormation : public s_workAreaTemplateWithCopy<sArachnothFormation>
{
    sArachnothSubModel m8_normalBody;
    sArachnothSubModel m98_poisonDart;
    sArachnothSubModel m128_eatDragonBody;
    sArachnothSubModel* m1B8_currentActiveModel;
    s_3dModel m1BC_debrits; // -> 0x208
    sVec3_FP m20C;
    sVec3_FP m218;
    sVec3_FP m224_translation;
    sVec3_FP m230;
    s32 m23C_chargeDirection;
    sVec3_FP* m240;
    sVec3_FP m244;
    fixedPoint m250;
    sVec3_FP m254;
    sVec3_FP m260;
    sVec3_FP m26C_rotation;
    s32 m278_targetedQuadrant;
    sVec3_FP m27C;
    sVec3_FP m288;
    s32 m294;
    s16 m298_life;
    sEnemyLifeMeterTask* m29C_lifeMeter;
    s32 m2A0;
    s32 m2A4;
    s32 m2A8;
    s32 m2AC_enrageState;
    s32 m2B0_arachnothState;
    s32 m2B4;
    s32 m2B8;
    s32 m2BC_attackTimer;
    s32 m2C0_currentAttackState;
    s32 m2C4_currentAttackDelay;
    s32 m2C8;
    sVec3_FP m2CC;
    sVec3_FP m2D8;
    sVec3_FP m2E4;
    sVec3_FP m2F0_eatDragonVelocity;
    sVec3_FP m2FC_eatDragonAcceleration;
    sVec3_FP m308_eatDragonPosition;
    std::array<std::array<sArachnothTentacle*, 3>, 2> m314_tentacles;
    s32 m32C;
    s32 m330_idleJitterX;
    s32 m334_idleJitterY;
    s32 m338_idleJitterZ;
    s32 m33C;
    s32 m340;
    std::array<s32, 2> m344;
    //size 0x34C
};

void arachnoth_updateQuadrantRadar(sArachnothFormation* pThis);
static void arachnoth_setIdleAnimation(sArachnothFormation* pThis);
static void arachnoth_setEnragedIdleAnimation(sArachnothFormation* pThis);
static void arachnoth_setUnconsciousAnimation(sArachnothFormation* pThis);
static void arachnoth_resumeAnimationAfterDamage(sArachnothFormation* pThis);
static void arachnoth_restoreCameraAfterAttack();
static void arachnoth_eatDragonAttack_update(sArachnothFormation* pThis);
void arachnoth_spawnSmokePuff(sArachnothFormation* pThis, sVec3_FP* position, sVec3_FP* rotation, sVec3_FP* param_4);

// 060566c8
static void arachnoth_spawnSmokeAndDebris(sArachnothFormation* pThis)
{
    sVec3_FP position;
    position[0] = MTH_Mul(randomNumber() >> 0x10, 0x14000) - 0xA000 + 0x201000;
    position[1] = MTH_Mul(randomNumber() >> 0x10, 0xA000) + 0x1D000;
    position[2] = (pThis->m23C_chargeDirection == 0) ? fixedPoint(-0x246000) : fixedPoint(-0x1BC000);

    // Spawn smoke particle
    static std::vector<sVdp1Quad> smokeQuads;
    smokeQuads = initVdp1Quad(g_BTL_A3_2->getSaturnPtr(0x060a999c));
    createBattleParticle((s_workAreaCopy*)dramAllocatorEnd[8].mC_fileBundle, &smokeQuads, &position, nullptr, nullptr, 0x10000, nullptr, 0);

    // Spawn bone debris
    sVec3_FP zeroRot; zeroRot.zeroize();
    createBoneDebris((s_workAreaCopy*)pThis, dramAllocatorEnd[8].mC_fileBundle,
        &pThis->m1BC_debrits, &position, &zeroRot,
        nullptr, nullptr, -0x2C, 0x1FFFFF, 0, 0, 0, 0x10000, 0);
}

void arachnoth_turnToFaceDragon(sArachnothFormation* pThis)
{
    int oldTargetedQuadrant = pThis->m278_targetedQuadrant;
    s32 quadrantDiff = gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant - pThis->m278_targetedQuadrant;
    quadrantDiff &= 3;

    switch (quadrantDiff)
    {
    case 0:
        return;
    case 1:
        pThis->m278_targetedQuadrant++;
        pThis->m254[2] -= 0x16C16C;
        break;
    case 2:
        if (pThis->m23C_chargeDirection == 0)
        {
            pThis->m278_targetedQuadrant = 0;
        }
        else
        {
            pThis->m278_targetedQuadrant = 2;
        }
        if (oldTargetedQuadrant < pThis->m278_targetedQuadrant)
        {
            pThis->m254[2] -= 0x16C16C;
        }
        else
        {
            pThis->m254[2] += 0x16C16C;
        }
        break;
    case 3:
        pThis->m278_targetedQuadrant--;
        pThis->m254[2] += 0x16C16C;
        break;
    default:
        assert(0);
    }

    pThis->m278_targetedQuadrant &= 3;
    pThis->m27C[1] = pThis->m278_targetedQuadrant << 0x1A;
    arachnoth_rotateTargetableFlags(&pThis->m8_normalBody, pThis->m278_targetedQuadrant);
    arachnoth_rotateTargetableFlags(&pThis->m98_poisonDart, pThis->m278_targetedQuadrant);
    gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2_needToSortEnemiesByDistanceFromDragon = 1;
    arachnoth_updateQuadrantRadar(pThis);
}

void arachnoth_snapToFaceDragon(sArachnothFormation* pThis)
{
    s32 quadrantDiff = gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant - pThis->m278_targetedQuadrant;
    if (quadrantDiff)
    {
        if ((quadrantDiff & 3) < 3)
        {
            pThis->m254[2] -= 0x16C16C;
        }
        else
        {
            pThis->m254[2] += 0x16C16C;
        }

        pThis->m278_targetedQuadrant = gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant;
        pThis->m27C[1] = pThis->m278_targetedQuadrant << 0x1A;
        arachnoth_rotateTargetableFlags(&pThis->m8_normalBody, pThis->m278_targetedQuadrant);
        arachnoth_rotateTargetableFlags(&pThis->m98_poisonDart, pThis->m278_targetedQuadrant);
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2_needToSortEnemiesByDistanceFromDragon = 1;
        arachnoth_updateQuadrantRadar(pThis);
    }
}

bool arachnoth_chooseNextAttack_sub(sArachnothFormation* pThis)
{
    if (pThis->m23C_chargeDirection == 0)
    {
        if (pThis->m278_targetedQuadrant != 0)
        {
            return false;
        }
        if (gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant != 0)
        {
            return false;
        }
    }
    else
    {
        if (pThis->m278_targetedQuadrant != 2)
        {
            return false;
        }
        if (gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant != 2)
        {
            return false;
        }
    }
    return true;
}

void arachnoth_startDragonEvade(s16 param1)
{
    gBattleManager->m10_battleOverlay->m4_battleEngine->m22E_dragonMoveDirection = 3;
    battleEngine_SetBattleMode(mA);
    gBattleManager->m10_battleOverlay->m4_battleEngine->m27C_dragonMovementInterpolator1.m68_rate = param1;
    gBattleManager->m10_battleOverlay->m4_battleEngine->m2E8_dragonMovementInterpolator2.m68_rate = param1;
    gBattleManager->m10_battleOverlay->m4_battleEngine->m38E = 0;
}

// 060587d0
void arachnoth_enterDeathState(sArachnothFormation* pThis)
{
    pThis->m278_targetedQuadrant = gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant;
    pThis->m27C[1] = pThis->m278_targetedQuadrant << 0x1A;
    pThis->m230 = *pThis->m240;
    arachnoth_setEnragedIdleAnimation(pThis);
    // 060562c6 — reset quadrant radar for death transition
    battleEngine_FlagQuadrantBitForDanger(0);
    for (u32 q = 0; q < 4; q++)
    {
        if ((pThis->m23C_chargeDirection == 0 && q != 2) ||
            (pThis->m23C_chargeDirection != 0 && q != 0))
        {
            battleEngine_FlagQuadrantForDanger(q);
        }
    }
    pThis->m2BC_attackTimer = 0x1FE;
    pThis->m2B0_arachnothState = 8;
    pThis->m2C0_currentAttackState = 0;
    pThis->m2C4_currentAttackDelay = 0;
    pThis->m2C8 = 0;
}

void arachnoth_chooseNextAttack(sArachnothFormation* pThis)
{
    int mode;
    switch (pThis->m2AC_enrageState)
    {
    case 0:
        if (pThis->m2B8 < 6)
        {
            if (gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant != pThis->m278_targetedQuadrant)
            {
                mode = 1;
            }
            else
            {
                if (randomNumber() & 1)
                {
                    mode = 2;
                }
                else
                {
                    mode = 3;
                }
            }
        }
        else
        {
            arachnoth_snapToFaceDragon(pThis);
            mode = 3;
        }
        break;
    case 1:
        if ((((gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant) == pThis->m278_targetedQuadrant) ||
            ((gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant) == (pThis->m278_targetedQuadrant - 1U & 3))) ||
            ((gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant) == (pThis->m278_targetedQuadrant + 1U & 3)))
        {
            arachnoth_turnToFaceDragon(pThis);
            if (!arachnoth_chooseNextAttack_sub(pThis))
            {
                mode = 5;
            }
            else
            {
                mode = 6;
            }
        }
        else
        {
            mode = 1;
        }
        break;
    default:
        assert(0);
    }

    pThis->m2B8++;

    if ((mode != 1) && (mode != 2) && (mode != 3) && pThis->m314_tentacles[0][0])
    {
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 3; j++) {
                arachnoth_disableTentacle(pThis->m314_tentacles[i][j]);
                pThis->m314_tentacles[i][j] = nullptr;
            }
        }
    }

    switch (mode)
    {
    case 0:
        break;
    case 1:
        switch (pThis->m2AC_enrageState)
        {
        case 0:
            arachnoth_turnToFaceDragon(pThis);
            break;
        case 1:
            arachnoth_snapToFaceDragon(pThis);
            break;
        default:
            assert(0);
        }
        battleEngine_SetBattleMode(m7);
        pThis->m2BC_attackTimer = 0;
        pThis->m2B0_arachnothState = 1;
        break;
    case 2: // Attack: digestive bile
        battleEngine_SetBattleMode(m7);
        battleEngine_displayAttackName(0xD, 0x5A, 0);
        battleEngine_FlagQuadrantForAttack(pThis->m278_targetedQuadrant);
        pThis->m2D8[0] = -0x2000000;
        pThis->m2D8[1] = randomNumber() & 0xfffffff;
        pThis->m2D8[2] = 0;

        pThis->m2CC[0] = MTH_Mul_5_6(getCos(pThis->m2D8[0].getInteger()), getSin(pThis->m2D8[1].getInteger()), 0x32000);
        pThis->m2CC[1] = MTH_Mul(-getSin(pThis->m2D8[0].getInteger()), 0x32000);
        pThis->m2CC[2] = MTH_Mul_5_6(getCos(pThis->m2D8[0].getInteger()), getCos(pThis->m2D8[1].getInteger()), 0x32000);

        pThis->m2CC += *pThis->m240;
        pThis->m2E4 = pThis->m224_translation;

        battleEngine_enableAttackCamera();
        battleEngine_setCurrentCameraPositionPointer(&pThis->m2CC);
        battleEngine_setDesiredCameraPositionPointer(&pThis->m224_translation);
        battleEngine_resetCameraInterpolation();

        pThis->m2BC_attackTimer = 0x78;
        pThis->m2B0_arachnothState = 2;
        pThis->m2C0_currentAttackState = 0;
        pThis->m2C4_currentAttackDelay = 0x1E;
        break;
    case 3: // Attack: tentacles
        battleEngine_SetBattleMode(m7);
        battleEngine_displayAttackName(0x5, 0x5A, 0);
        battleEngine_FlagQuadrantForAttack(pThis->m278_targetedQuadrant);

        if (pThis->m314_tentacles[0][0] == 0)
        {
            static std::array<std::array<sVec3_FP, 3>, 2> arachnothTentacleInitialPosition = { {
                {   sVec3_FP(0,-0x900, 0x6000),
                    sVec3_FP(0,-0x1000, 0x5000),
                    sVec3_FP(0,-0x2000, 0x4000) },
                {   sVec3_FP(0,-0x900, 0x6000),
                    sVec3_FP(0,-0x1000, 0x5000),
                    sVec3_FP(0,-0x2000, 0x4000) },
            } };

            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 3; j++) {
                    pThis->m314_tentacles[i][j] = createArachnothTentacle(pThis, &pThis->m224_translation, &pThis->m26C_rotation, &arachnothTentacleInitialPosition[i][j]);
                }
            }
        }
        
        pThis->m2D8[0] = 0;
        pThis->m2D8[1] = (pThis->m278_targetedQuadrant + 1) * 0x4000000;
        pThis->m2D8[2] = 0;

        pThis->m2CC[0] = MTH_Mul_5_6(getCos(pThis->m2D8[0].getInteger()), getSin(pThis->m2D8[1].getInteger()), 0x3C000);
        pThis->m2CC[1] = MTH_Mul(-getSin(pThis->m2D8[0].getInteger()), 0x3C000);
        pThis->m2CC[2] = MTH_Mul_5_6(getCos(pThis->m2D8[0].getInteger()), getCos(pThis->m2D8[1].getInteger()), 0x3C000);

        pThis->m2CC += *pThis->m240;
        pThis->m2E4 = pThis->m224_translation;

        battleEngine_enableAttackCamera();
        battleEngine_setCurrentCameraPositionPointer(&pThis->m2CC);
        battleEngine_setDesiredCameraPositionPointer(&pThis->m224_translation);
        battleEngine_resetCameraInterpolation();

        pThis->m2BC_attackTimer = 0xD2;
        pThis->m2B0_arachnothState = 3;
        pThis->m2C0_currentAttackState = 0;
        pThis->m2C4_currentAttackDelay = 0x3C;
        break;
    case 6:
        pThis->m2B0_arachnothState = 6;
        pThis->m2BC_attackTimer = 300;
        arachnoth_startDragonEvade((pThis->m2BC_attackTimer >> 16) - 0x96);
        battleEngine_displayAttackName(3, 0x5a, 0);
        battleEngine_FlagQuadrantForAttack(pThis->m278_targetedQuadrant);
        if (!(((pThis->m23C_chargeDirection == 0) && (gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant != 0)) || ((pThis->m23C_chargeDirection != 0) && (gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant != 2))))
        {
            pThis->m2D8.zeroize();
            if (pThis->m23C_chargeDirection == 0)
            {
                pThis->m2D8[1] = 0x1555555;
            }
            else
            {
                pThis->m2D8[1] = 0x6aaaaaa;
            }

            sMatrix4x3 tempMatrix;
            initMatrixToIdentity(&tempMatrix);
            rotateMatrixShiftedY(pThis->m2D8[1], &tempMatrix);
            rotateMatrixShiftedX(pThis->m2D8[0], &tempMatrix);
            rotateMatrixShiftedZ(pThis->m2D8[2], &tempMatrix);
            transformAndAddVec(sVec3_FP(0, -0xA000, 0x1E000), pThis->m2CC, tempMatrix);
            pThis->m2CC += *pThis->m240;
            battleEngine_enableAttackCamera();
            battleEngine_setCurrentCameraPositionPointer(&pThis->m2CC);
            battleEngine_setDesiredCameraPositionPointer(&pThis->m224_translation);
            battleEngine_resetCameraInterpolation();
            if (pThis->m23C_chargeDirection == 0) {
                gBattleManager->m10_battleOverlay->m4_battleEngine->m45C_perQuadrantDragonSpeed[2] = 0x2d000;
                gBattleManager->m10_battleOverlay->m4_battleEngine->m45C_perQuadrantDragonSpeed[0] = 0x31000;
                gBattleManager->m10_battleOverlay->m4_battleEngine->m45C_perQuadrantDragonSpeed[1] = 0x1e000;
                gBattleManager->m10_battleOverlay->m4_battleEngine->m45C_perQuadrantDragonSpeed[3] = 0x1e000;
                gBattleManager->m10_battleOverlay->m4_battleEngine->m364_perQuadrantDragonAltitude[1] = 0x64000;
                gBattleManager->m10_battleOverlay->m4_battleEngine->m364_perQuadrantDragonAltitude[3] = 0x64000;
                pThis->m23C_chargeDirection = 1;
                static sVec3_FP tempVec(0x201000, 0x13000, 0x1D2000);
                pThis->m240 = &tempVec;
                pThis->m244[0] = 0;
                pThis->m244[1] = 0;
                pThis->m244[2] = gBattleManager->m10_battleOverlay->m4_battleEngine->m45C_perQuadrantDragonSpeed[2];
            }
            else
            {
                gBattleManager->m10_battleOverlay->m4_battleEngine->m45C_perQuadrantDragonSpeed[0] = 0x2d000;
                gBattleManager->m10_battleOverlay->m4_battleEngine->m45C_perQuadrantDragonSpeed[2] = 0x31000;
                gBattleManager->m10_battleOverlay->m4_battleEngine->m45C_perQuadrantDragonSpeed[1] = 0x1e000;
                gBattleManager->m10_battleOverlay->m4_battleEngine->m45C_perQuadrantDragonSpeed[3] = 0x1e000;
                gBattleManager->m10_battleOverlay->m4_battleEngine->m364_perQuadrantDragonAltitude[1] = 0x64000;
                gBattleManager->m10_battleOverlay->m4_battleEngine->m364_perQuadrantDragonAltitude[3] = 0x64000;
                pThis->m23C_chargeDirection = 0;
                static sVec3_FP tempVec(0x201000, 0x13000, 0x230000);
                pThis->m240 = &tempVec;
                pThis->m244[0] = 0;
                pThis->m244[1] = 0;
                pThis->m244[2] = gBattleManager->m10_battleOverlay->m4_battleEngine->m45C_perQuadrantDragonSpeed[0];
            }
            pThis->m250 = 0x31000;
            pThis->m27C[0] = 0x31C71C7;
            pThis->m2C0_currentAttackState = 0;
        }
        break;
    case 5: // eat dragon attack
        Unimplemented(); // setup eat dragon attack (camera, movement, animation)
        pThis->m2B0_arachnothState = 5;
        break;
    case 8:
        arachnoth_enterDeathState(pThis);
        break;
    default:
        assert(0);
    }
}

void arachnoth_setEnragedIdleAnim(sArachnothFormation* pThis)
{
    arachnothInitSubModelAnimation(&pThis->m8_normalBody, 1, -1);
    arachnothInitSubModelAnimation(&pThis->m98_poisonDart, 1, -1);
    pThis->m1B8_currentActiveModel = &pThis->m8_normalBody;
}

void arachnoth_startEnrage(sArachnothFormation* pThis)
{
    battleEngine_SetBattleMode(m7);
    battleEngine_displayAttackName(2, 0x5a, 0);
    pThis->m2D8[0] = -0x1000000;
    pThis->m2D8[1] = 0x4000000;
    pThis->m2D8[2] = 0;

    pThis->m2CC[0] = MTH_Mul_5_6(getCos(pThis->m2D8[0].getInteger()), getSin(pThis->m2D8[1].getInteger()), 0x3C000);
    pThis->m2CC[1] = MTH_Mul(-getSin(pThis->m2D8[0].getInteger()), 0x3C000);
    pThis->m2CC[2] = MTH_Mul_5_6(getCos(pThis->m2D8[0].getInteger()), getCos(pThis->m2D8[1].getInteger()), 0x3C000);

    pThis->m2CC += *pThis->m240;

    battleEngine_enableAttackCamera();
    battleEngine_setCurrentCameraPositionPointer(&pThis->m2CC);
    battleEngine_setDesiredCameraPositionPointer(&pThis->m224_translation);
    battleEngine_resetCameraInterpolation();
    arachnoth_setEnragedIdleAnim(pThis);

    pThis->m2BC_attackTimer = 0xB4;
    pThis->m2B0_arachnothState = 4;
    pThis->m2C0_currentAttackState = 0;
    pThis->m2C4_currentAttackDelay = 0x5A;
}

struct sBileProjectile : public s_workAreaTemplateWithCopy<sBileProjectile>
{
    sVec3_FP m8_acceleration;
    sVec3_FP m14_velocity;
    sVec3_FP m20_position;
    sTrailRenderer m2C_trail;
    s32 m44_lifetime;
    // size 0x48
};

// 0605a654
static void bileProjectile_update(sBileProjectile* pThis)
{
    pThis->m8_acceleration[1] += -0xA3; // gravity
    pThis->m14_velocity += pThis->m8_acceleration;
    pThis->m20_position += pThis->m14_velocity;
    pThis->m8_acceleration.zeroize();

    trailRenderer_update(&pThis->m2C_trail, &pThis->m20_position,
        &gBattleManager->m10_battleOverlay->m4_battleEngine->m1A0_battleAutoScrollDelta);

    if (--pThis->m44_lifetime < 0)
    {
        pThis->getTask()->markFinished();
    }
}

// 0605a72a
static void bileProjectile_draw(sBileProjectile* pThis)
{
    trailRenderer_draw(&pThis->m2C_trail);
}

// 0605a418
void createArachnothDigestiveEffect(p_workArea parent, sVec3_FP* pSource, sVec3_FP* pTarget, s32 param4)
{
    static const sBileProjectile::TypedTaskDefinition definition = {
        nullptr, &bileProjectile_update, &bileProjectile_draw, nullptr
    };

    sBileProjectile* pThis = createSubTaskWithCopy<sBileProjectile>((s_workAreaCopy*)parent, &definition);
    if (!pThis) return;

    pThis->m8_acceleration.zeroize();
    pThis->m14_velocity.zeroize();
    pThis->m20_position = *pSource;

    // Compute initial velocity toward target: (target - source) >> param4
    s32 shift = param4;
    sVec3_FP diff;
    diff[0] = (*pTarget)[0] - pThis->m20_position[0];
    diff[1] = (*pTarget)[1] - pThis->m20_position[1];
    diff[2] = (*pTarget)[2] - pThis->m20_position[2];
    pThis->m14_velocity[0] = diff[0] >> shift;
    pThis->m14_velocity[1] = diff[1] >> shift;
    pThis->m14_velocity[2] = diff[2] >> shift;

    trailRenderer_init(&pThis->m2C_trail, pThis, pSource,
        dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory,
        g_BTL_A3_2->getSaturnPtr(0x060a95c4));

    pThis->m44_lifetime = shiftLeft32(1, param4) + 0xF;
}

void arachnoth_digestiveFluidAttack_update(sArachnothFormation* pThis)
{
    switch (pThis->m2C0_currentAttackState)
    {
    case 0:
        if (--pThis->m2C4_currentAttackDelay < 0)
        {
            pThis->m2C0_currentAttackState++;
            pThis->m2C4_currentAttackDelay = 0;
        }
        break;
    case 1:
        if ((randomNumber() & 3) == 0)
        {
            if (pThis->m2C4_currentAttackDelay == 4)
            {
                sMatrix4x3 matrix;
                initMatrixToIdentity(&matrix);
                rotateMatrixShiftedY(pThis->m26C_rotation[1], &matrix);
                rotateMatrixShiftedX(pThis->m26C_rotation[0], &matrix);
                rotateMatrixShiftedZ(pThis->m26C_rotation[2], &matrix);
                sVec3_FP local_2c;
                transformAndAddVec(sVec3_FP(0, 0, 0x5000), local_2c, matrix);
                local_2c += pThis->m224_translation;
                createArachnothDigestiveEffect(pThis, &local_2c, &gBattleManager->m10_battleOverlay->m18_dragon->m8_position, 5);
                playSystemSoundEffect(0x71);
                pThis->m254[0] -= 0xB60B6;
                battleEngine_setDesiredCameraPositionPointer(&gBattleManager->m10_battleOverlay->m18_dragon->m8_position);
                pThis->m2C0_currentAttackState++;
                pThis->m2C4_currentAttackDelay = 0x20;
            }
            else
            {
                // Alternate bile shot: compute source from rotation, random target near dragon
                sVec3_FP source;
                fixedPoint cosX = getCos(pThis->m26C_rotation[0].toInteger());
                source[0] = MTH_Mul_5_6(cosX, getSin(pThis->m26C_rotation[1].toInteger()), 0x5000);
                source[1] = MTH_Mul(-getSin(pThis->m26C_rotation[0].toInteger()), 0x5000);
                source[2] = MTH_Mul_5_6(cosX, getCos(pThis->m26C_rotation[1].toInteger()), 0x5000);
                source += pThis->m224_translation;

                sVec3_FP target;
                target[0] = MTH_Mul(randomNumber() >> 0x10, 0x14000) - 0xA000 + gBattleManager->m10_battleOverlay->m18_dragon->m8_position[0];
                target[1] = MTH_Mul(randomNumber() >> 0x10, 0x14000) - 0xA000 + gBattleManager->m10_battleOverlay->m18_dragon->m8_position[1];
                target[2] = MTH_Mul(randomNumber() >> 0x10, 0x14000) - 0xA000 + gBattleManager->m10_battleOverlay->m18_dragon->m8_position[2];

                createArachnothDigestiveEffect(pThis, &source, &target, 5);
                playSystemSoundEffect(0x71);
                pThis->m254[0] -= 0xB60b6;
                pThis->m2C4_currentAttackDelay++;
            }
        }
        break;
    case 2:
        if (--pThis->m2C4_currentAttackDelay < 0)
        {
            sVec3_FP local_2c;
            fixedPoint local_68 = MTH_Mul(fixedPoint(randomNumber()).toInteger(), 0x1000);
            local_2c[1] = MTH_Mul(getSin(MTH_Mul(fixedPoint(randomNumber()).toInteger(), 0x10000000).toInteger()), local_68);

            local_68 = MTH_Mul(fixedPoint(randomNumber()).toInteger(), 0x1000);
            local_68 = MTH_Mul(getCos(MTH_Mul(fixedPoint(randomNumber()).toInteger(), 0x10000000).toInteger()), local_68);
            local_2c[2] = MTH_Mul(getSin(MTH_Mul(fixedPoint(randomNumber()).toInteger(), 0x10000000).toInteger()), local_68);

            local_2c[0] = MTH_Mul(getSin(MTH_Mul(fixedPoint(randomNumber()).toInteger(), 0x10000000).toInteger()), local_68);

            applyDamageToDragon(gBattleManager->m10_battleOverlay->m18_dragon->m8C, 40, gBattleManager->m10_battleOverlay->m18_dragon->m8_position, 3, local_2c, 0);
            pThis->m2C0_currentAttackState++;
        }
        break;
    case 3:
        break;
    default:
        assert(0);
    }

    pThis->m2D8[1] += 0xB60B6;

    pThis->m2CC[0] = MTH_Mul_5_6(getCos(pThis->m2D8[0].getInteger()), getSin(pThis->m2D8[1].getInteger()), 0x32000);
    pThis->m2CC[1] = MTH_Mul(-getSin(pThis->m2D8[0].getInteger()), 0x32000);
    pThis->m2CC[2] = MTH_Mul_5_6(getCos(pThis->m2D8[0].getInteger()), getCos(pThis->m2D8[1].getInteger()), 0x32000);

    pThis->m2CC += *pThis->m240;
}

bool arachnoth_isCurrentAttackDone(sArachnothFormation* pThis)
{
    if (--pThis->m2BC_attackTimer < 0)
    {
        return true;
    }
    return false;
}

// 06056a1e
void arachnoth_idle_update(sArachnothFormation* pThis)
{
    if ((randomNumber() & 0x7F) == 0) pThis->m330_idleJitterX = 0xF;
    if ((randomNumber() & 0x7F) == 0) pThis->m330_idleJitterX = -0xF;
    if ((randomNumber() & 0x7F) == 0) pThis->m334_idleJitterY = 0xF;
    if ((randomNumber() & 0x7F) == 0) pThis->m334_idleJitterY = -0xF;
    if ((randomNumber() & 0x7F) == 0) pThis->m338_idleJitterZ = 0xF;
    if ((randomNumber() & 0x7F) == 0) pThis->m338_idleJitterZ = -0xF;

    if (pThis->m330_idleJitterX > 0) { pThis->m20C[0] += 0xCC; pThis->m330_idleJitterX--; }
    if (pThis->m330_idleJitterX < 0) { pThis->m20C[0] -= 0xCC; pThis->m330_idleJitterX++; }
    if (pThis->m334_idleJitterY > 0) { pThis->m20C[1] += 0xCC; pThis->m334_idleJitterY--; }
    if (pThis->m334_idleJitterY < 0) { pThis->m20C[1] -= 0xCC; pThis->m334_idleJitterY++; }
    if (pThis->m338_idleJitterZ > 0) { pThis->m20C[2] += 0xCC; pThis->m338_idleJitterZ--; }
    if (pThis->m338_idleJitterZ < 0) { pThis->m20C[2] -= 0xCC; pThis->m338_idleJitterZ++; }

    if ((randomNumber() & 0x3F) == 0) pThis->m254[0] += 0xB60B6;
    if ((randomNumber() & 0x3F) == 0) pThis->m254[0] -= 0xB60B6;
    if ((randomNumber() & 0x3F) == 0) pThis->m254[2] += 0xB60B6;
    if ((randomNumber() & 0x3F) == 0) pThis->m254[2] -= 0xB60B6;
}

void arachnoth_createChargeDebris(sArachnothFormation* pThis, npcFileDeleter* param_2, s_3dModel* p3dModel, sVec3_FP* position, sVec3_FP* rotation, s32 param6, s32 param7, s32 param8, s32 param9, s32 param10, s32 param11, s32 param12, s32 param13, s32 param14)
{
    createBoneDebris((s_workAreaCopy*)param_2, param_2, p3dModel, position, rotation,
        nullptr, nullptr, param8, param9, param10, param11, (s16)param12, param13, (s16)param14);
}

// 0605a1d0
void arachnoth_createChargeImpact(sArachnothFormation* pThis)
{
    sVec3_FP position;
    position[0] = 0x201000;
    position[1] = 0x27000; // 0x1D000 + 0xA000
    position[2] = (pThis->m23C_chargeDirection == 0) ? fixedPoint(-0x246000) : fixedPoint(-0x1BC000);

    sVec3_FP rotation;
    rotation[0] = (pThis->m23C_chargeDirection == 0) ? fixedPoint(0x4000000) : fixedPoint(-0x4000000);
    rotation[1] = 0;
    rotation[2] = MTH_Mul(randomNumber() >> 0x10, 0x10000000);

    // Create impact model at position with animation
    // FUN_06062eaa creates a 3D model task using model 0x18, pose 0x1CC, animation 0x210
    // For now, spawn debris as visual approximation
    sVec3_FP zeroRot; zeroRot.zeroize();
    createBoneDebris((s_workAreaCopy*)dramAllocatorEnd[8].mC_fileBundle,
        dramAllocatorEnd[8].mC_fileBundle, &pThis->m1BC_debrits,
        &position, &rotation, nullptr, nullptr,
        -0x2C, 0x1FFFFF, 0, 0, 0, 0x10000, 0x1D);
}

void arachnoth_createSmokeParticle(npcFileDeleter* param_2, sSaturnPtr spriteDataEA, sVec3_FP* position, s32 velX, s32 velY, s32 scale, s32 unused1, s32 unused2)
{
    static std::vector<sVdp1Quad> cachedQuads;
    cachedQuads = initVdp1Quad(spriteDataEA);
    sVec3_FP velocity;
    velocity[0] = velX;
    velocity[1] = velY;
    velocity[2] = 0;
    createBattleParticle((s_workAreaCopy*)param_2, &cachedQuads, position, &velocity, nullptr, scale, nullptr, 0);
}

void arachnoth_chargeAttack_update(sArachnothFormation* pThis)
{
    switch (pThis->m2C0_currentAttackState)
    {
    case 0:
        if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2000)
        {
            pThis->m2C0_currentAttackState++;
            pThis->m2C4_currentAttackDelay = 0x3C;
        }
        break;
    case 1:
        if (--pThis->m2C4_currentAttackDelay < 0x1F)
        {
            if (pThis->m23C_chargeDirection == 0)
            {
                pThis->m230 = sVec3_FP(0x201000, 0x1D000, -0x1BC000);
            }
            else
            {
                pThis->m230 = sVec3_FP(0x201000, 0x1D000, -0x246000);
            }
            pThis->m288[0] += MTH_Mul(0x10000 - pThis->m288[2], 0x28F);
        }
        if (pThis->m2C4_currentAttackDelay < 1)
        {
            if (pThis->m23C_chargeDirection == 0)
            {
                pThis->m230 = sVec3_FP(0x201000, 0x1D000, -0x246000);
                pThis->m230[2] -= 0x32000;
            }
            else
            {
                pThis->m230 = sVec3_FP(0x201000, 0x1D000, -0x1BC000);
                pThis->m230[2] += 0x32000;
            }
            pThis->m254[0] += 0x2D82D8;

            sVec3_FP temp;
            if (pThis->m23C_chargeDirection == 0)
            {
                temp = sVec3_FP(0x201000, 0x1D000, -0x1BC000);
            }
            else
            {
                temp = sVec3_FP(0x201000, 0x1D000, -0x246000);
            }

            sVec3_FP temp2;
            temp2.zeroize();
            arachnoth_createChargeDebris(pThis, dramAllocatorEnd[8].mC_fileBundle, &pThis->m1BC_debrits, &temp, &temp2, 0, 0, -0x2C, 0x1fffff, 0, 0, 0, 0x10000, 0);
            playSystemSoundEffect(0x6b);
            pThis->m2C4_currentAttackDelay = 0x5a;
            pThis->m2C0_currentAttackState++;
        }
        break;
    case 2:
        pThis->m288[0] += MTH_Mul(-pThis->m288[2], 0x28F);
        if (pThis->m2C4_currentAttackDelay == 0x46)
        {
            playSystemSoundEffect(0x76);
        }
        if (pThis->m2C4_currentAttackDelay == 0x42)
        {
            sVec3_FP temp;
            temp[1] = MTH_Mul(getSin(MTH_Mul(randomNumber() >> 16, 0x10000000).getInteger()), MTH_Mul(randomNumber() >> 16, 0x1000));
            fixedPoint temp2 = MTH_Mul(getCos(MTH_Mul(randomNumber() >> 16, 0x10000000).getInteger()), MTH_Mul(randomNumber() >> 16, 0x1000));
            temp[2] = MTH_Mul(getSin(MTH_Mul(randomNumber() >> 16, 0x10000000).getInteger()), temp2);
            temp[0] = MTH_Mul(getCos(MTH_Mul(randomNumber() >> 16, 0x10000000).getInteger()), temp2);
            applyDamageToDragon(gBattleManager->m10_battleOverlay->m18_dragon->m8C, 0x52, gBattleManager->m10_battleOverlay->m18_dragon->m8_position, 3, temp, 0);
        }
        if (pThis->m23C_chargeDirection == 0)
        {
            if (pThis->m224_translation[2] < -0x246000)
            {
                pThis->m20C.zeroize();
                pThis->m218.zeroize();
                pThis->m254[0] -= 0x2D82D8;
                pThis->m230 = sVec3_FP(0x201000, 0x1D000, -0x246000);
                pThis->m2C4_currentAttackDelay = 0;
            }
        }
        else
        {
            if (pThis->m224_translation[2] < -0x1BC000)
            {
                pThis->m20C.zeroize();
                pThis->m218.zeroize();
                pThis->m254[0] -= 0x2D82D8;
                pThis->m230 = sVec3_FP(0x201000, 0x1D000, -0x1BC000);
                pThis->m2C4_currentAttackDelay = 0;
            }
        }
        if (--pThis->m2C4_currentAttackDelay < 1)
        {
            playBattleSoundEffect(0x76);
            playSystemSoundEffect(0x74);
            arachnoth_createChargeImpact(pThis);
            pThis->m2C4_currentAttackDelay = 0x5A;
            pThis->m2C0_currentAttackState++;
        }
        break;
    case 3:
    {
        sVec3_FP temp;
        temp[0] = MTH_Mul(randomNumber() >> 0x10, 0x1E000) - 0xF000;
        temp[1] = MTH_Mul(randomNumber() >> 0x10, 0x19000) + 0x1D000;
        if (pThis->m23C_chargeDirection == 0)
        {
            temp[2] = -0x246000;
        }
        else
        {
            temp[2] = -0x1BC000;
        }

        temp[0] += 0x201000;
        if (!(randomNumber() & 1))
        {
            arachnoth_createSmokeParticle(dramAllocatorEnd[8].mC_fileBundle, g_BTL_A3_2->getSaturnPtr(0x60A9814), &temp, 0, 0, 0x10000, 0, 0);
        }
        else
        {
            arachnoth_createSmokeParticle(dramAllocatorEnd[8].mC_fileBundle, g_BTL_A3_2->getSaturnPtr(0x60a999c), &temp, 0, 0, 0x10000, 0, 0);
        }
        if (!(randomNumber() & 0xF))
        {
            sVec3_FP temp2;
            temp2.zeroize();
            arachnoth_createChargeDebris(pThis, dramAllocatorEnd[8].mC_fileBundle, &pThis->m1BC_debrits, &temp, &temp2, 0, 0, -0x2C, 0x1fffff, 0, 0, 0, 0x10000, 0);
        }
        if (!(randomNumber() & 0xF))
        {
            pThis->m254[2] += MTH_Mul(randomNumber() >> 16, 0x71C71C) - 0x38E38E;
            arachnoth_createChargeImpact(pThis);
        }
        pThis->m288[0] += MTH_Mul(-pThis->m288[2], 0x28F);
        if (--pThis->m2C4_currentAttackDelay < 1)
        {
            pThis->m2C0_currentAttackState++;
        }
        break;
    }
    case 4: // Unconscious
        if (pThis->m2BC_attackTimer == 0x1E)
        {
            battleEngine_displayAttackName(0xc, 0x1e, 0); // Unconscious
        }
        pThis->m244[0] += MTH_Mul(pThis->m250 - pThis->m244[2], 0x28F);
        pThis->m244[1] += pThis->m244[0];
        pThis->m244[1] -= MTH_Mul(pThis->m244[1], 0x3333);
        pThis->m244[2] += pThis->m244[1];
        pThis->m244[0] = 0;

        if (pThis->m23C_chargeDirection == 0)
        {
            gBattleManager->m10_battleOverlay->m4_battleEngine->m45C_perQuadrantDragonSpeed[0] = pThis->m244[2];
        }
        else
        {
            gBattleManager->m10_battleOverlay->m4_battleEngine->m45C_perQuadrantDragonSpeed[2] = pThis->m244[2];
        }
        break;
    default:
        assert(0);
    }

    pThis->m20C += MTH_Mul(-0x28F, pThis->m230 - pThis->m224_translation);
    pThis->m20C += MTH_Mul(0xA3, pThis->m230 - pThis->m224_translation);

    battleEngine_setDesiredCameraPositionPointer(&pThis->m224_translation);
    battleEngine_resetCameraInterpolation();
    battleEngine_setCurrentCameraPositionPointer(&pThis->m2CC);
}

void arachnoth_enrageAnimation_update(sArachnothFormation* pThis)
{
    switch (pThis->m2C0_currentAttackState)
    {
    case 0:
        if (--pThis->m2C4_currentAttackDelay < 0)
        {
            pThis->m2C0_currentAttackState++;
            pThis->m2C4_currentAttackDelay = 7;
            pThis->m2AC_enrageState = 1;
            playSystemSoundEffect(0x70);
            pThis->m2C8 = 0x1E;
        }
        break;
    case 1:
        if (pThis->m2C4_currentAttackDelay < 1)
        {
            pThis->m2C4_currentAttackDelay++;
            if (pThis->m2C4_currentAttackDelay == 0)
            {
                pThis->m2C4_currentAttackDelay = 7;
            }
            pThis->m254[1] -= 0xB60B6;
        }
        else
        {
            pThis->m2C4_currentAttackDelay--;
            if (pThis->m2C4_currentAttackDelay == 0)
            {
                pThis->m2C4_currentAttackDelay = -7;
            }
            pThis->m254[1] += 0xB60B6;
        }
        break;
    default:
        assert(0);
    }

    pThis->m254[0] -= 0x2468A;
    pThis->m2D8[1] += 0xB60B6;

    pThis->m2CC[0] = MTH_Mul_5_6(getCos(pThis->m2D8[0].getInteger()), getSin(pThis->m2D8[1].getInteger()), 0x3C000);
    pThis->m2CC[1] = MTH_Mul(-getSin(pThis->m2D8[0].getInteger()), 0x3C000);
    pThis->m2CC[2] = MTH_Mul_5_6(getCos(pThis->m2D8[0].getInteger()), getCos(pThis->m2D8[1].getInteger()), 0x3C000);

    pThis->m2CC += *pThis->m240;

}

void arachnoth_tentacleAttack_update(sArachnothFormation* pThis)
{
    switch (pThis->m2C0_currentAttackState)
    {
    case 0:
        if (--pThis->m2C4_currentAttackDelay < 0)
        {
            pThis->m2C0_currentAttackState++;
            pThis->m2C4_currentAttackDelay = 90;

            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 3; j++) {
                    arachnothTentacle_setMode1(pThis->m314_tentacles[i][j]);
                }
            }
        }
        break;
    case 1:
        if (--pThis->m2C4_currentAttackDelay < 1)
        {
            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 3; j++) {
                    arachnothTentacle_setMode2(pThis->m314_tentacles[i][j]);
                }
            }
            battleEngine_setDesiredCameraPositionPointer(&gBattleManager->m10_battleOverlay->m18_dragon->m8_position);
            pThis->m254[0] -= 0x444444;
            playSystemSoundEffect(0x72);
            pThis->m2C4_currentAttackDelay = 60;
            pThis->m2C0_currentAttackState++;
        }
        pThis->m2D8[1] -= 0x5b05b;

        pThis->m2CC[0] = MTH_Mul_5_6(getCos(pThis->m2D8[0].getInteger()), getSin(pThis->m2D8[1].getInteger()), 0x3C000);
        pThis->m2CC[1] = MTH_Mul(-getSin(pThis->m2D8[0].getInteger()), 0x3C000);
        pThis->m2CC[2] = MTH_Mul_5_6(getCos(pThis->m2D8[0].getInteger()), getCos(pThis->m2D8[1].getInteger()), 0x3C000);

        pThis->m2CC += *pThis->m240;
        break;
    case 2:
        if (--pThis->m2C4_currentAttackDelay == 0x2D) {
            sVec3_FP attackVector;
            attackVector[1] = MTH_Mul(getSin(MTH_Mul(fixedPoint(randomNumber()).getInteger(), 0x10000000).getInteger()), MTH_Mul(fixedPoint(randomNumber()).getInteger(), 0x1000));
            fixedPoint temp = MTH_Mul(getCos(MTH_Mul(fixedPoint(randomNumber()).getInteger(), 0x10000000).getInteger()), MTH_Mul(fixedPoint(randomNumber()).getInteger(), 0x1000));
            attackVector[2] = MTH_Mul(getSin(MTH_Mul(fixedPoint(randomNumber()).getInteger(), 0x10000000).getInteger()), temp);
            attackVector[0] = MTH_Mul(getCos(MTH_Mul(fixedPoint(randomNumber()).getInteger(), 0x10000000).getInteger()), temp);
            applyDamageToDragon(gBattleManager->m10_battleOverlay->m18_dragon->m8C, 55, gBattleManager->m10_battleOverlay->m18_dragon->m8_position, 3, attackVector, 0);
        }
        pThis->m2D8[1] -= 0x5b05b;

        pThis->m2CC[0] = MTH_Mul_5_6(getCos(pThis->m2D8[0].getInteger()), getSin(pThis->m2D8[1].getInteger()), 0x3C000);
        pThis->m2CC[1] = MTH_Mul(-getSin(pThis->m2D8[0].getInteger()), 0x3C000);
        pThis->m2CC[2] = MTH_Mul_5_6(getCos(pThis->m2D8[0].getInteger()), getCos(pThis->m2D8[1].getInteger()), 0x3C000);

        pThis->m2CC += *pThis->m240;
        break;
    default:
        assert(0);
    }
}

void arachnoth_resetAnimAfterDamage(sArachnothFormation* pThis)
{
    arachnoth_resumeAnimationAfterDamage(pThis);
}

void arachnoth_enterUnconsciousState(sArachnothFormation* pThis)
{
    pThis->m2BC_attackTimer = MTH_Mul(randomNumber() >> 16, 0x78) + 0xB4;
    pThis->m2B0_arachnothState = 7;
    arachnoth_resetAnimAfterDamage(pThis);
    arachnoth_updateQuadrantRadar(pThis);
    if (!(randomNumber() & 1))
    {
        pThis->m27C[2] = 0x4000000;
        pThis->m27C[1] = 0x2aaaaaa;
    }
    else
    {
        pThis->m27C[2] = -0x4000000;
        pThis->m27C[1] = -0x2aaaaaa;
    }
    if (pThis->m23C_chargeDirection == 0)
    {
        pThis->m27C[1] += 0x8000000;
    }

    pThis->m27C[0] = MTH_Mul(randomNumber() >> 16, 0x1000000) + 0x2000000;
}

// 0605696e
static void arachnoth_setEatDragonAnim(sArachnothFormation* pThis, s32 animEndFrame)
{
    arachnothInitSubModelAnimation(&pThis->m128_eatDragonBody, 0, animEndFrame);
    arachnothInitSubModelAnimation(&pThis->m98_poisonDart, 5, -1);
    pThis->m1B8_currentActiveModel = &pThis->m128_eatDragonBody;
}

// 06057df8
static void arachnoth_eatDragonAttack_update(sArachnothFormation* pThis)
{
    switch (pThis->m2C0_currentAttackState)
    {
    case 0:
        // Wait for delay, then switch to eat-dragon model
        if (--pThis->m2C4_currentAttackDelay < 1)
        {
            arachnoth_setEatDragonAnim(pThis, -1);
            playSystemSoundEffect(0x6F);
            pThis->m2C4_currentAttackDelay = 0;
            pThis->m2C0_currentAttackState++;
        }
        break;
    case 1:
    {
        // Move toward dragon
        fixedPoint diff = MTH_Mul(gBattleManager->m10_battleOverlay->m18_dragon->m8_position[0] - pThis->m308_eatDragonPosition[0], 0x28F);
        // Apply to boss movement (simplified — original uses this for camera tracking)
        break;
    }
    case 2:
    {
        // Spring toward target position with damping
        sVec3_FP diff;
        diff[0] = MTH_Mul((*pThis->m240)[0] - pThis->m308_eatDragonPosition[0], 0xA3);
        diff[1] = MTH_Mul((*pThis->m240)[1] - pThis->m308_eatDragonPosition[1], 0xA3);
        diff[2] = MTH_Mul((*pThis->m240)[2] - pThis->m308_eatDragonPosition[2], 0xA3);

        pThis->m2F0_eatDragonVelocity += diff;
        pThis->m2FC_eatDragonAcceleration += pThis->m2F0_eatDragonVelocity;

        sVec3_FP drag;
        drag[0] = MTH_Mul(pThis->m2FC_eatDragonAcceleration[0], 0x3333);
        drag[1] = MTH_Mul(pThis->m2FC_eatDragonAcceleration[1], 0x3333);
        drag[2] = MTH_Mul(pThis->m2FC_eatDragonAcceleration[2], 0x3333);
        pThis->m2FC_eatDragonAcceleration -= drag;

        pThis->m308_eatDragonPosition += pThis->m2FC_eatDragonAcceleration;
        pThis->m2F0_eatDragonVelocity.zeroize();

        if (pThis->m2C4_currentAttackDelay == 0x3E)
        {
            g_fadeControls.m_4D = 6;
            if ((s8)g_fadeControls.m_4C < 7)
            {
                vdp2Controls.m20_registers[1].m112_CLOFSL = 0;
                vdp2Controls.m20_registers[0].m112_CLOFSL = 0;
            }
            fadePalette(&g_fadeControls.m0_fade0, 0xC210, 0x8000, 10);
            g_fadeControls.m_4D = 5;
            playSystemSoundEffect(0x73);
            pThis->m2C0_currentAttackState++;
        }
        break;
    }
    case 3:
        // Wait for fade, then set up camera on dragon
        if (pThis->m2C4_currentAttackDelay == 0x4F)
        {
            pThis->m2CC = gBattleManager->m10_battleOverlay->m18_dragon->m8_position;
            pThis->m2CC[1] += 0xA000;
            pThis->m2CC[0] += MTH_Mul(0x4000, getSin((pThis->m278_targetedQuadrant << 0x1A) >> 16));
        }
        if (pThis->m2C4_currentAttackDelay == 0x50)
        {
            g_fadeControls.m_4D = 6;
            if ((s8)g_fadeControls.m_4C < 7)
            {
                vdp2Controls.m20_registers[1].m112_CLOFSL = 0;
                vdp2Controls.m20_registers[0].m112_CLOFSL = 0;
            }
            fadePalette(&g_fadeControls.m0_fade0, 0x8000, 0xC210, 8);
            g_fadeControls.m_4D = 5;
            pThis->m2C0_currentAttackState++;
        }
        break;
    case 4:
    {
        // Damage phase — random angular jitter, apply damage at frame 0x71
        if ((randomNumber() & 7) == 0) pThis->m254[2] += 0x16C16C;
        if ((randomNumber() & 7) == 0) pThis->m254[2] -= 0x16C16C;

        pThis->m2CC[1] -= 0x355;

        if (pThis->m2C4_currentAttackDelay == 0x71)
        {
            sVec3_FP attackVector;
            fixedPoint temp1 = MTH_Mul(randomNumber() >> 0x10, 0x1000);
            fixedPoint angle1 = MTH_Mul(randomNumber() >> 0x10, 0x10000000);
            attackVector[1] = MTH_Mul(getSin(angle1.toInteger()), temp1);

            fixedPoint temp2 = MTH_Mul(getCos(MTH_Mul(randomNumber() >> 0x10, 0x10000000).toInteger()), MTH_Mul(randomNumber() >> 0x10, 0x1000));
            attackVector[2] = MTH_Mul(getSin(MTH_Mul(randomNumber() >> 0x10, 0x10000000).toInteger()), temp2);
            attackVector[0] = MTH_Mul(getCos(MTH_Mul(randomNumber() >> 0x10, 0x10000000).toInteger()), temp2);

            applyDamageToDragon(gBattleManager->m10_battleOverlay->m18_dragon->m8C, 0x4E,
                gBattleManager->m10_battleOverlay->m18_dragon->m8_position, 3, attackVector, 0);
        }

        if (pThis->m2C4_currentAttackDelay == 0x80)
        {
            pThis->m230 = *pThis->m240;
            pThis->m2C0_currentAttackState++;
        }
        break;
    }
    case 5:
    {
        // Release phase — camera descends, smoke puffs
        pThis->m2CC[1] -= 0x280;
        if (pThis->m2C4_currentAttackDelay < 0x8C && (randomNumber() & 1) == 0)
        {
            sVec3_FP smokeOffset;
            smokeOffset[0] = MTH_Mul(randomNumber() >> 0x10, 0xC000) - 0x6000;
            smokeOffset[1] = MTH_Mul(randomNumber() >> 0x10, 0x9000) - 0x1000;
            smokeOffset[2] = MTH_Mul(randomNumber() >> 0x10, 0x3000) + 0x2000;
            arachnoth_spawnSmokePuff(pThis, &pThis->m224_translation, &pThis->m26C_rotation, &smokeOffset);
        }
        break;
    }
    default:
        break;
    }

    pThis->m2C4_currentAttackDelay++;
}

void arachnoth_idle_finish(sArachnothFormation* pThis) {
    if (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m400000)
    {
        if (!battleEngine_isPlayerTurnActive() || battleEngine_isBattleIntroFinished())
        {
            if (gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m8 == 1)
            {
                arachnoth_chooseNextAttack(pThis);
            }
        }
    }

    if (pThis->m2AC_enrageState == 0)
    {
        if (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m400000)
        {
            if (!battleEngine_isPlayerTurnActive() || battleEngine_isBattleIntroFinished())
            {
                if (pThis->m298_life < 1500)
                {
                    if (pThis->m314_tentacles[0][0])
                    {
                        for (int i = 0; i < 2; i++) {
                            for (int j = 0; j < 3; j++) {
                                arachnoth_disableTentacle(pThis->m314_tentacles[i][j]);
                                pThis->m314_tentacles[i][j] = nullptr;
                            }
                        }
                    }
                    arachnoth_startEnrage(pThis);
                }
            }
        }
    }

    if (pThis->m298_life < 1)
    {
        arachnoth_enterDeathState(pThis);
    }
}

void arachnoth_idleHitReturn(sArachnothFormation* pThis) {
    gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished = 1;
    arachnoth_rotateTargetableFlags(&pThis->m8_normalBody, pThis->m278_targetedQuadrant);
    arachnoth_rotateTargetableFlags(&pThis->m98_poisonDart, pThis->m278_targetedQuadrant);
    gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2_needToSortEnemiesByDistanceFromDragon = 1;
    arachnoth_updateQuadrantRadar(pThis);
    if (((5 < pThis->m2B8) && (pThis->m2AC_enrageState == 0)) && (pThis->m314_tentacles[0][0] == nullptr))
    {
        static std::array<std::array<sVec3_FP, 3>, 2> arachnothTentacleInitialPosition = { {
            {   sVec3_FP(0,-0x900, 0x6000),
                sVec3_FP(0,-0x1000, 0x5000),
                sVec3_FP(0,-0x2000, 0x4000) },
            {   sVec3_FP(0,-0x900, 0x6000),
                sVec3_FP(0,-0x1000, 0x5000),
                sVec3_FP(0,-0x2000, 0x4000) },
        } };

        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 3; j++) {
                pThis->m314_tentacles[i][j] = createArachnothTentacle(pThis, &pThis->m224_translation, &pThis->m26C_rotation, &arachnothTentacleInitialPosition[i][j]);
            }
        }
    }
    gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m8 = 0;
    gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m0 = 0;

    pThis->m2B0_arachnothState = 0;
}

void arachnoth_digestiveFluidAttack_finish(sArachnothFormation* pThis) {
    gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished = 1;
    gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m8 = 0;
    gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m0 = 0;
    arachnoth_restoreCameraAfterAttack();
    pThis->m2B0_arachnothState = 0;
    pThis->m2B8 = 0;
    arachnoth_updateQuadrantRadar(pThis);
}

void arachnoth_tentacleAttack_finish(sArachnothFormation* pThis) {
    gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished = 1;
    gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m8 = 0;
    gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m0 = 0;
    arachnoth_restoreCameraAfterAttack();
    pThis->m2B0_arachnothState = 0;
    if (pThis->m314_tentacles[0][0])
    {
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 3; j++) {
                arachnoth_disableTentacle(pThis->m314_tentacles[i][j]);
                pThis->m314_tentacles[i][j] = nullptr;
            }
        }
    }
    pThis->m2B8 = 0;
    arachnoth_updateQuadrantRadar(pThis);
}

void arachnoth_enrageAnimation_finish(sArachnothFormation* pThis) {
    arachnoth_snapToFaceDragon(pThis);
    gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished = 1;
    gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m8 = 0;
    gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m0 = 0;
    arachnoth_restoreCameraAfterAttack();
    gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m2 = 0x2D;
    arachnoth_updateQuadrantRadar(pThis);
    pThis->m2B0_arachnothState = 0;
}

void arachnoth_chargeAttack_finish(sArachnothFormation* pThis) {
    arachnoth_restoreCameraAfterAttack();
    gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished = 1;
    gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m8 = 0;
    gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m0 = 0;
    if (pThis->m23C_chargeDirection == 0)
    {
        gBattleManager->m10_battleOverlay->m4_battleEngine->m45C_perQuadrantDragonSpeed[0] = pThis->m250;
        gBattleManager->m10_battleOverlay->m4_battleEngine->m22F_battleRadarLockIcon = 4;
    }
    else
    {
        gBattleManager->m10_battleOverlay->m4_battleEngine->m45C_perQuadrantDragonSpeed[2] = pThis->m250;
        gBattleManager->m10_battleOverlay->m4_battleEngine->m22F_battleRadarLockIcon = 1;
    }

    pThis->m27C[0] = 0;
    arachnoth_enterUnconsciousState(pThis);
    pThis->m2B8 = 0;
    arachnoth_updateQuadrantRadar(pThis);
}

// shared camera restore logic
static void arachnoth_restoreCameraCommon()
{
    gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m800 = 0;
    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0xB] == 0)
    {
        s_battleGrid* grid = gBattleManager->m10_battleOverlay->m8_gridTask;
        grid->m34_cameraPosition =
            gBattleManager->m10_battleOverlay->m4_battleEngine->m104_dragonPosition
            + grid->m1C
            + grid->m28;
        battleEngine_setCurrentCameraPositionPointer(&grid->m34_cameraPosition);
        gBattleManager->m10_battleOverlay->m4_battleEngine->m3D8_pDesiredCameraPosition =
            &gBattleManager->m10_battleOverlay->m4_battleEngine->mC_battleCenter;
    }
}

// 0607e030 — camera restore after attack (used by most finish functions)
static void arachnoth_restoreCameraAfterAttack()
{
    arachnoth_restoreCameraCommon();
    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0xB] == 0)
    {
        battleEngine_resetCameraInterpolation();
        // 0607e254
        s_battleGrid* grid = gBattleManager->m10_battleOverlay->m8_gridTask;
        grid->m64_cameraRotationTarget.m8_Z = 0;
        grid->mB4_cameraRotation.m8_Z = 0;
    }
}

// 0607e14c — camera restore (used by state 5 finish / unconscious end)
static void arachnoth_restoreCamera()
{
    arachnoth_restoreCameraCommon();
    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0xB] == 0)
    {
        // 0607e254
        s_battleGrid* grid = gBattleManager->m10_battleOverlay->m8_gridTask;
        grid->m64_cameraRotationTarget.m8_Z = 0;
        grid->mB4_cameraRotation.m8_Z = 0;
    }
}

void arachnoth_updateState(sArachnothFormation* pThis)
{
    switch (pThis->m2B0_arachnothState)
    {
    case 0:
        arachnoth_idle_update(pThis);
        if (!gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x1D])
        {
            arachnoth_idle_finish(pThis);
        }
        break;
    case 1:
        if (arachnoth_isCurrentAttackDone(pThis))
        {
            arachnoth_idleHitReturn(pThis);
        }
        break;
    case 2: // digestive fluid
        arachnoth_digestiveFluidAttack_update(pThis);
        if (arachnoth_isCurrentAttackDone(pThis))
        {
            arachnoth_digestiveFluidAttack_finish(pThis);
        }
        break;
    case 3: // tentacle attack
        arachnoth_tentacleAttack_update(pThis);
        if (arachnoth_isCurrentAttackDone(pThis))
        {
            arachnoth_tentacleAttack_finish(pThis);
        }
        break;
    case 4:
        arachnoth_enrageAnimation_update(pThis);
        if (arachnoth_isCurrentAttackDone(pThis))
        {
            arachnoth_enrageAnimation_finish(pThis);
        }
        break;
    case 5:
        arachnoth_eatDragonAttack_update(pThis);
        if (arachnoth_isCurrentAttackDone(pThis))
        {
            // 060585ba
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished = 1;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m8 = 0;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m0 = 0;
            arachnoth_restoreCamera();
            arachnoth_resumeAnimationAfterDamage(pThis);
            pThis->m2B0_arachnothState = 0;
            pThis->m2B8 = 0;
            arachnoth_updateQuadrantRadar(pThis);
        }
        break;
    case 6: // charging
        arachnoth_chargeAttack_update(pThis);
        if (arachnoth_isCurrentAttackDone(pThis))
        {
            arachnoth_chargeAttack_finish(pThis);
        }
        break;
    case 7: // unconscious
        // 0605a336
        pThis->m288[0] -= 0x20;
        if (pThis->m298_life < 1)
        {
            pThis->m2BC_attackTimer = 0;
            arachnoth_enterDeathState(pThis);
            return;
        }
        if ((!battleEngine_isPlayerTurnActive() || battleEngine_isBattleIntroFinished()) &&
            arachnoth_isCurrentAttackDone(pThis))
        {
            // 0605a354 — unconscious end cleanup
            gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m8 = 0;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m0 = 0;
            pThis->m2B0_arachnothState = 0;
            arachnoth_resumeAnimationAfterDamage(pThis);
            pThis->m27C[2] = 0;
            pThis->m27C[1] = 0;
            pThis->m27C[0] = 0;
            pThis->m230 = *pThis->m240;
            arachnoth_spawnSmokeAndDebris(pThis);
            arachnoth_snapToFaceDragon(pThis);
        }
        break;
    case 8: // death animation
        Unimplemented(); // arachnoth_updateState8_sub0 (0605883e)
        break;
    case 9: // death cinematic/reward
        Unimplemented(); // arachnoth_updateState9_sub0 (06059074)
        break;
    default:
        assert(0);
        break;
    }
}

void arachnoth_updateTargetableVisibility(sArachnothSubModel* pThis, s32 param2, s32 param3)
{
    if (readSaturnS32(pThis->m4 + 8))
    {
        for (int i = 0; i < pThis->m64; i++)
        {
            pThis->m58_targetables[i].m60 = readSaturnS8(readSaturnEA(pThis->m4 + 8) + (param2 - param3 & 3));
        }
    }
}

struct sSmokePuffTask : public s_workAreaTemplateWithCopy<sSmokePuffTask>
{
    sVec3_FP* m8_positionPtr;   // pointer to parent position
    sVec3_FP* mC_rotationPtr;   // pointer to parent rotation
    sVec3_FP m10_velocity;
    sVec3_FP m1C_velocity2;
    sVec3_FP m28_position;
    sVec3_FP m34_acceleration;
    sVec3_FP m40_velocity3;
    sVec3_FP m4C_targetPos;
    sVec3_FP m58_offset;
    s32 m64_state;
    s32 m68_lifetime;
    // size 0x6C
};

// 0605a9b4
static void smokePuff_update(sSmokePuffTask* pThis)
{
    if (pThis->m64_state == 0)
    {
        // Phase 0: rising smoke
        pThis->m34_acceleration[1] -= 0x147;

        sVec3_FP diff1 = pThis->m4C_targetPos - pThis->m28_position;
        pThis->m10_velocity += MTH_Mul(0x20C, diff1);

        sVec3_FP diff2 = pThis->m28_position - pThis->m4C_targetPos;
        pThis->m34_acceleration += MTH_Mul(0x20C, diff2);

        if (pThis->m10_velocity[1] < -0x199)
        {
            pThis->m68_lifetime = 0x1E;
            pThis->m64_state = 1;
        }

        // Recompute attachment point
        sMatrix4x3 matrix;
        initMatrixToIdentity(&matrix);
        rotateMatrixShiftedY((*pThis->mC_rotationPtr)[1], &matrix);
        rotateMatrixShiftedX((*pThis->mC_rotationPtr)[0], &matrix);
        rotateMatrixShiftedZ((*pThis->mC_rotationPtr)[2], &matrix);
        sVec3_FP worldOffset;
        transformAndAddVec(pThis->m58_offset, worldOffset, matrix);
        worldOffset += *pThis->m8_positionPtr;

        pThis->m10_velocity.zeroize();
        pThis->m1C_velocity2.zeroize();
        pThis->m28_position = worldOffset;

        pThis->m40_velocity3 += pThis->m34_acceleration;
        pThis->m40_velocity3 -= MTH_Mul(0xCCC, pThis->m40_velocity3);
        pThis->m4C_targetPos += pThis->m40_velocity3;
        pThis->m34_acceleration.zeroize();
    }
    else if (pThis->m64_state == 1)
    {
        // Phase 1: falling smoke
        pThis->m10_velocity[1] -= 0x147;
        pThis->m34_acceleration[1] -= 0x147;

        sVec3_FP diff1 = pThis->m4C_targetPos - pThis->m28_position;
        pThis->m10_velocity += MTH_Mul(0x20C, diff1);

        sVec3_FP diff2 = pThis->m28_position - pThis->m4C_targetPos;
        pThis->m34_acceleration += MTH_Mul(0x20C, diff2);

        pThis->m1C_velocity2 += pThis->m10_velocity;
        pThis->m1C_velocity2 -= MTH_Mul(0xCCC, pThis->m1C_velocity2);
        pThis->m28_position += pThis->m1C_velocity2;
        pThis->m10_velocity.zeroize();

        pThis->m40_velocity3 += pThis->m34_acceleration;
        pThis->m40_velocity3 -= MTH_Mul(0xCCC, pThis->m40_velocity3);
        pThis->m4C_targetPos += pThis->m40_velocity3;
        pThis->m34_acceleration.zeroize();

        if (--pThis->m68_lifetime < 0)
        {
            pThis->getTask()->markFinished();
        }
    }
}

// 0605b412
static void smokePuff_draw(sSmokePuffTask* pThis)
{
    pushCurrentMatrix();
    translateCurrentMatrix(pThis->m28_position);
    rotateCurrentMatrixYXZ(*pThis->mC_rotationPtr);
    // Draw using the model's draw function — but we don't have a model
    // The smoke puff is a visual-only effect; skip for now
    popMatrix();
}

// 0605a75c
void arachnoth_spawnSmokePuff(sArachnothFormation* pThis, sVec3_FP* position, sVec3_FP* rotation, sVec3_FP* param_4)
{
    static const sSmokePuffTask::TypedTaskDefinition definition = {
        nullptr, &smokePuff_update, &smokePuff_draw, nullptr
    };

    sSmokePuffTask* pTask = createSubTaskWithCopy<sSmokePuffTask>((s_workAreaCopy*)pThis, &definition);
    if (!pTask) return;

    pTask->m8_positionPtr = position;
    pTask->mC_rotationPtr = rotation;
    pTask->m58_offset = *param_4;

    // Compute initial world-space position from parent rotation + offset
    sMatrix4x3 matrix;
    initMatrixToIdentity(&matrix);
    rotateMatrixShiftedY((*rotation)[1], &matrix);
    rotateMatrixShiftedX((*rotation)[0], &matrix);
    rotateMatrixShiftedZ((*rotation)[2], &matrix);
    sVec3_FP worldPos;
    transformAndAddVec(pTask->m58_offset, worldPos, matrix);
    worldPos += *position;

    pTask->m10_velocity.zeroize();
    pTask->m1C_velocity2.zeroize();
    pTask->m28_position = worldPos;
    pTask->m34_acceleration.zeroize();
    pTask->m40_velocity3.zeroize();
    pTask->m4C_targetPos = worldPos;
    pTask->m64_state = 0;
    pTask->m68_lifetime = 0;
}

// 0605680a
static void arachnoth_setIdleAnimation(sArachnothFormation* pThis)
{
    arachnothInitSubModelAnimation(&pThis->m8_normalBody, 0, -1);
    arachnothInitSubModelAnimation(&pThis->m98_poisonDart, 0, -1);
    pThis->m1B8_currentActiveModel = &pThis->m8_normalBody;
}

// 06056834
static void arachnoth_setEnragedIdleAnimation(sArachnothFormation* pThis)
{
    arachnothInitSubModelAnimation(&pThis->m8_normalBody, 1, -1);
    arachnothInitSubModelAnimation(&pThis->m98_poisonDart, 1, -1);
    pThis->m1B8_currentActiveModel = &pThis->m8_normalBody;
}

// 0605685e
static void arachnoth_damageReactionNormal(sArachnothFormation* pThis)
{
    arachnothInitSubModelAnimation(&pThis->m8_normalBody, 2, -1);
    arachnothInitSubModelAnimation(&pThis->m98_poisonDart, 2, -1);
    pThis->m1B8_currentActiveModel = &pThis->m8_normalBody;
    pThis->m2B4 = 0x39;
}

// 0605693e
static void arachnoth_damageReactionUnconscious(sArachnothFormation* pThis)
{
    arachnothInitSubModelAnimation(&pThis->m8_normalBody, 7, -1);
    arachnothInitSubModelAnimation(&pThis->m98_poisonDart, 4, -1);
    pThis->m1B8_currentActiveModel = &pThis->m8_normalBody;
    pThis->m2B4 = 0x21;
}

// 06056914
static void arachnoth_setUnconsciousAnimation(sArachnothFormation* pThis)
{
    arachnothInitSubModelAnimation(&pThis->m8_normalBody, 6, -1);
    arachnothInitSubModelAnimation(&pThis->m98_poisonDart, 3, -1);
    pThis->m1B8_currentActiveModel = &pThis->m8_normalBody;
}

// 060569a0
static void arachnoth_showBattleText(s16 textIndex, s16 duration)
{
    sBattleTextDisplayTask* pDisplayText = gBattleManager->m10_battleOverlay->m14_textDisplay;
    if (pDisplayText && pDisplayText->m0_texts.m_offset)
    {
        pDisplayText->m14 = duration;
        pDisplayText->m10 = textIndex;
        createDisplayStringBorromScreenTask(pDisplayText, &pDisplayText->m8, -pDisplayText->m14, readSaturnEA(pDisplayText->m0_texts + pDisplayText->m10 * 4));
    }
}

// 06055ddc
static void arachnoth_resumeAnimationAfterDamage(sArachnothFormation* pThis)
{
    if (pThis->m2B0_arachnothState == 7)
    {
        arachnoth_setUnconsciousAnimation(pThis);
    }
    else if (pThis->m2AC_enrageState == 0)
    {
        arachnoth_setIdleAnimation(pThis);
    }
    else if (pThis->m2AC_enrageState == 1)
    {
        arachnoth_setEnragedIdleAnimation(pThis);
    }
}

void arachnothFormation_update(sArachnothFormation* pThis)
{
    s32 damageTaken = arachnoth_processSubModelDamage(&pThis->m8_normalBody) + arachnoth_processSubModelDamage(&pThis->m98_poisonDart);
    if (damageTaken > 0)
    {
        playSystemSoundEffect(0x6E);
        pThis->m298_life -= (s16)damageTaken;
        pThis->m2A0 = 0x5A;
        if (pThis->m2B0_arachnothState == 7)
        {
            arachnoth_damageReactionUnconscious(pThis);
        }
        else
        {
            arachnoth_damageReactionNormal(pThis);
        }
        pThis->m2A8 += damageTaken;
    }

    if (pThis->m2A0)
    {
        pThis->m2A0--;
    }

    if (pThis->m8_normalBody.m6C || pThis->m98_poisonDart.m6C)
    {
        pThis->m2A4 = 1;
    }

    if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m1000 && (pThis->m2A4 == 1))
    {
        if (pThis->m2A8 < 1)
        {
            arachnoth_showBattleText(1, 0x13);
        }
        else
        {
            createDamageNumberFromValue(pThis, (s16)pThis->m2A8, &pThis->m224_translation);
        }
        pThis->m2A4 = 0;
        pThis->m2A8 = 0;
    }

    if (pThis->m1B8_currentActiveModel->m74 == 1)
    {
        int var5 = stepAnimation(&pThis->m1B8_currentActiveModel->m8_model);
        if (var5 == pThis->m1B8_currentActiveModel->m7C)
        {
            pThis->m1B8_currentActiveModel->m74 = 0;
        }
    }

    if (pThis->m98_poisonDart.m74 == 1)
    {
        int var5 = stepAnimation(&pThis->m98_poisonDart.m8_model);
        if (var5 == pThis->m98_poisonDart.m7C)
        {
            pThis->m98_poisonDart.m74 = 0;
        }
    }

    if (pThis->m2B4 > 0)
    {
        pThis->m2B4--;
        if (pThis->m2B4 == 0)
        {
            arachnoth_resumeAnimationAfterDamage(pThis);
        }
    }

    gBattleManager->m10_battleOverlay->m4_battleEngine->m3B2_numBattleFormationRunning++;

    arachnoth_updateState(pThis);

    arachnoth_updateTargetableVisibility(&pThis->m8_normalBody, gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant, pThis->m278_targetedQuadrant);
    arachnoth_updateTargetableVisibility(&pThis->m98_poisonDart, gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant, pThis->m278_targetedQuadrant);

    if ((pThis->m2B0_arachnothState != 8) && (pThis->m2B0_arachnothState != 9))
    {
        if ((randomNumber() & 0x1F) == 0)
        {
            sVec3_FP temp;
            temp[0] = MTH_Mul(fixedPoint(randomNumber()).toInteger(), 0x6000) - 0x3000;
            temp[1] = MTH_Mul(fixedPoint(randomNumber()).toInteger(), 0x2000) - 0x1000;
            temp[2] = MTH_Mul(fixedPoint(randomNumber()).toInteger(), 0x3000) + 0x2000;
            arachnoth_spawnSmokePuff(pThis, &pThis->m224_translation, &pThis->m26C_rotation, &temp);
        }

        pThis->m20C += MTH_Mul(0x28F, pThis->m230 - pThis->m224_translation);
        pThis->m254 += MTH_Mul(0x147, (pThis->m27C - pThis->m26C_rotation).normalized());

        if ((pThis->m2B0_arachnothState == 6) || (pThis->m2B0_arachnothState == 7))
        {
            pThis->m288[0] += MTH_Mul(0x8000 - pThis->m288[2], 0x41);
        }
        else if (pThis->m2AC_enrageState == 0)
        {
            if (--pThis->m294 < 1)
            {
                pThis->m288[0] += 0x28F;
                pThis->m294 = 0x1C;
            }
            pThis->m288[0] += MTH_Mul(0x4000 - pThis->m288[2], 0xCCC);
        }
        else if (pThis->m2AC_enrageState == 1)
        {
            if (--pThis->m294 < 1)
            {
                pThis->m288[0] += 0xA3D;
                pThis->m294 = 0x14;
            }
            pThis->m288[0] += MTH_Mul(0x8000 - pThis->m288[2], 0x147A);
        }

        if (pThis->m314_tentacles[0][0])
        {
            pThis->m288[0] = pThis->m254[0] + 0x12345;
        }
    }

    if (pThis->m314_tentacles[0][0])
    {
        static const sVec3_FP tentacleOffsets[6] = {
            sVec3_FP(-0xCC, 0x28, 0xCC),
            sVec3_FP(-0xCC, 0x00, 0x66),
            sVec3_FP(-0xCC, 0x00, 0x00),
            sVec3_FP( 0xCC, 0x28, 0xCC),
            sVec3_FP( 0xCC, 0x00, 0x66),
            sVec3_FP( 0xCC, 0x00, 0x00),
        };

        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 3; j++) {
                arachnoth_updateTentacle(pThis->m314_tentacles[i][j], &tentacleOffsets[i * 3 + j], 8);
            }
        }
    }

    pThis->m218 += pThis->m20C;
    pThis->m218 -= MTH_Mul(0x1999, pThis->m218);
    pThis->m224_translation += pThis->m218;
    pThis->m20C.zeroize();

    pThis->m260 += pThis->m254;
    pThis->m260 -= MTH_Mul(0xCCC, pThis->m260);

    pThis->m26C_rotation += pThis->m260;
    pThis->m26C_rotation[0].m_value &= 0xfffffff;
    pThis->m26C_rotation[1].m_value &= 0xfffffff;
    pThis->m26C_rotation[2].m_value &= 0xfffffff;

    pThis->m254.zeroize();
    pThis->m288[1] += pThis->m288[0];
    pThis->m288[1] -= MTH_Mul(0xA3D, pThis->m288[1]);
    pThis->m288[2] += pThis->m288[1];
    pThis->m288[0] = 0;

    // TODO: arachnothFormation_updateSub8 — applies m288[2] jaw rotation to sub-model bones
    // TODO: arachnoth_spawnSmokePuff — random smoke puff particles around boss
}

// 06054f2c
static s32 arachnothFormation_getHitDirection(sArachnothSubModel* pSubModel, s32 dragonQuadrant, s32 targetQuadrant)
{
    s32 tableAddr = readSaturnS32(pSubModel->m4 + 8);
    if (tableAddr)
    {
        return (s32)readSaturnS8(pSubModel->m4.m_file->getSaturnPtr(tableAddr) + ((dragonQuadrant - targetQuadrant) & 3));
    }
    return 0;
}

// 0607f7ce
static void arachnothFormation_setHitFlash(s32 direction)
{
    s32 colorIndex;
    if (direction < 0x1F) {
        if (direction < 0x15) {
            colorIndex = (direction < 0xB) ? 0 : 1;
        } else {
            colorIndex = 2;
        }
    } else {
        colorIndex = 3;
    }
    static const u32 hitFlashColors[] = { 0x181818, 0xE1E110, 0xE1E11F, 0xE1E11F };
    setupLightColor(hitFlashColors[colorIndex]);
}

// 0607f7fe
static void arachnothFormation_restoreLighting()
{
    s_battleGrid* grid = gBattleManager->m10_battleOverlay->m8_gridTask;
    u32 r4 = ((u32)grid->m1E4_lightFalloff0[0].getInteger() & 0xFF)
           | (((u32)grid->m1E4_lightFalloff0[1].getInteger() & 0xFF) << 8)
           | (((u32)grid->m1E4_lightFalloff0[2].getInteger() & 0xFF) << 16);
    u32 r5 = ((u32)grid->m1FC_lightFalloff1[0].getInteger() & 0xFF)
           | (((u32)grid->m1FC_lightFalloff1[1].getInteger() & 0xFF) << 8)
           | (((u32)grid->m1FC_lightFalloff1[2].getInteger() & 0xFF) << 16);
    u32 r6 = ((u32)grid->m208_lightFalloff2[0].getInteger() & 0xFF)
           | (((u32)grid->m208_lightFalloff2[1].getInteger() & 0xFF) << 8)
           | (((u32)grid->m208_lightFalloff2[2].getInteger() & 0xFF) << 16);
    generateLightFalloffMap(r4, r5, r6);
}

void arachnothFormation_draw(sArachnothFormation* pThis)
{
    if (!isShipping() && gDebugWindows.arachnoth)
    {
        if (ImGui::Begin("Arachnoth", &gDebugWindows.arachnoth))
        {
            int life = pThis->m298_life;
            ImGui::InputInt("Life", &life);
            pThis->m298_life = life;
        }
        ImGui::End();
    }

    if (pThis->m1B8_currentActiveModel->m70_flags & 0x800000)
    {
        s32 direction = arachnothFormation_getHitDirection(pThis->m1B8_currentActiveModel,
            gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant,
            pThis->m278_targetedQuadrant);
        arachnothFormation_setHitFlash(direction);
    }

    pushCurrentMatrix();
    translateCurrentMatrix(pThis->m224_translation);
    rotateCurrentMatrixYXZ(pThis->m26C_rotation);
    pThis->m1B8_currentActiveModel->m8_model.m18_drawFunction(&pThis->m1B8_currentActiveModel->m8_model);
    popMatrix();

    if (pThis->m1B8_currentActiveModel->m70_flags & 0x800000)
    {
        pThis->m1B8_currentActiveModel->m70_flags &= ~0x800000;
        arachnothFormation_restoreLighting();
    }

    sVec3_FP dartPosition;
    if (pThis->m1B8_currentActiveModel == &pThis->m8_normalBody)
    {
        transformAndAddVec(pThis->m1B8_currentActiveModel->m8_model.m44_hotpointData[3][0], dartPosition, cameraProperties2.m28[0]);
    }
    else
    {
        transformAndAddVec(pThis->m1B8_currentActiveModel->m8_model.m44_hotpointData[0x25][0], dartPosition, cameraProperties2.m28[0]);
    }

    if (pThis->m98_poisonDart.m70_flags & 0x800000)
    {
        s32 direction = arachnothFormation_getHitDirection(&pThis->m98_poisonDart,
            gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant,
            pThis->m278_targetedQuadrant);
        arachnothFormation_setHitFlash(direction);
    }

    pushCurrentMatrix();
    translateCurrentMatrix(dartPosition);
    rotateCurrentMatrixYXZ(pThis->m26C_rotation);
    pThis->m98_poisonDart.m8_model.m18_drawFunction(&pThis->m98_poisonDart.m8_model);
    popMatrix();

    if (pThis->m98_poisonDart.m70_flags & 0x800000)
    {
        pThis->m98_poisonDart.m70_flags &= ~0x800000;
        arachnothFormation_restoreLighting();
    }

    if ((pThis->m2A0 > 0) && (pThis->m344[0] == 0))
    {
        pThis->m29C_lifeMeter->m31 |= 2;
    }
}

void arachnoth_updateQuadrantRadar(sArachnothFormation* pThis)
{
    switch (pThis->m2AC_enrageState)
    {
    case 0:
        if (pThis->m2B8 > 5)
        {
            battleEngine_FlagQuadrantBitForDanger(0);
            battleEngine_FlagQuadrantBitForSafety(0);
            for (u32 q = 0; q < 4; q++)
            {
                if ((pThis->m23C_chargeDirection == 0 && q != 2) ||
                    (pThis->m23C_chargeDirection != 0 && q != 0))
                {
                    battleEngine_FlagQuadrantForSafety(q);
                }
            }
            return;
        }
        break;
    case 1:
        if (pThis->m2B0_arachnothState != 7)
        {
            battleEngine_FlagQuadrantBitForDanger(0);
            int quadrant = (pThis->m278_targetedQuadrant + 2) & 3;
            if (
                ((pThis->m23C_chargeDirection != 0 ) || (quadrant != 2)) &&
                ((pThis->m23C_chargeDirection != 1 ) || (quadrant != 0))
                )
            {
                battleEngine_FlagQuadrantBitForDanger(quadrant);
            }

            battleEngine_FlagQuadrantBitForSafety(0);

            quadrant = pThis->m278_targetedQuadrant & 3;
            if (
                ((pThis->m23C_chargeDirection != 0) || (quadrant != 2)) &&
                ((pThis->m23C_chargeDirection != 1) || (quadrant != 0))
                )
            {
                battleEngine_FlagQuadrantBitForSafety(quadrant);
            }

            quadrant = (pThis->m278_targetedQuadrant + 1) & 3;
            if (
                ((pThis->m23C_chargeDirection != 0) || (quadrant != 2)) &&
                ((pThis->m23C_chargeDirection != 1) || (quadrant != 0))
                )
            {
                battleEngine_FlagQuadrantBitForSafety(quadrant);
            }

            quadrant = (pThis->m278_targetedQuadrant - 1) & 3;
            if (
                ((pThis->m23C_chargeDirection != 0) || (quadrant != 2)) &&
                ((pThis->m23C_chargeDirection != 1) || (quadrant != 0))
                )
            {
                battleEngine_FlagQuadrantBitForSafety(quadrant);
            }

            return;
        }
        break;
    default:
        assert(0);
    }

    battleEngine_FlagQuadrantBitForDanger(0);
    for (int i=0; i<4;i++)
    {
        for (int j = 0; j <= 4; j++)
        {
            int quadrantToTest = i + j;
            if (pThis->m278_targetedQuadrant != quadrantToTest)
            {
                if (pThis->m23C_chargeDirection == 0)
                {
                    if (quadrantToTest != 2)
                    {
                        battleEngine_FlagQuadrantForDanger(quadrantToTest);
                    }
                }
                else
                {
                    if (quadrantToTest != 0)
                    {
                        battleEngine_FlagQuadrantForDanger(quadrantToTest);
                    }
                }
            }
        }
    }

    battleEngine_FlagQuadrantBitForSafety(0);
    if (pThis->m23C_chargeDirection == 0)
    {
        if (pThis->m278_targetedQuadrant != 2)
        {
            battleEngine_FlagQuadrantForSafety(pThis->m278_targetedQuadrant);
        }
    }
    else
    {
        if (pThis->m278_targetedQuadrant != 0)
        {
            battleEngine_FlagQuadrantForSafety(pThis->m278_targetedQuadrant);
        }
    }
}

// 0605654c
void arachnothSubModelFunction0(s_workAreaCopy* parent, sBattleTargetable* targetable)
{
    sArachnothFormation* pFormation = (sArachnothFormation*)parent;
    s32 offset = MTH_Mul(randomNumber() >> 0x10, 0x2D82D8);
    if (pFormation->m2B0_arachnothState == 7)
    {
        if ((randomNumber() & 3) == 0)
        {
            arachnoth_spawnSmokeAndDebris(pFormation);
        }
    }
    else
    {
        pFormation->m254[2] += offset - 0x16C16C;
    }
    sVec3_FP zero; zero.zeroize();
    spawnHitSpark(parent, &targetable->m10_position, &zero, 0x20000, 0);
}

// 060565c0
void arachnothSubModelFunction1(s_workAreaCopy* parent, sBattleTargetable* targetable)
{
    sArachnothFormation* pFormation = (sArachnothFormation*)parent;
    s32 offset = MTH_Mul(randomNumber() >> 0x10, 0x5B05B0);
    if (pFormation->m2B0_arachnothState == 7)
    {
        if ((randomNumber() & 3) == 0)
        {
            arachnoth_spawnSmokeAndDebris(pFormation);
        }
    }
    else
    {
        pFormation->m254[2] += offset - 0x2D82D8;
    }
    sVec3_FP zero; zero.zeroize();
    spawnHitSpark(parent, &targetable->m10_position, &zero, 0x20000, 0);
}

// 0605666c
void arachnothSubModelFunction2(s_workAreaCopy* parent, sBattleTargetable* targetable)
{
    sArachnothFormation* pFormation = (sArachnothFormation*)parent;
    s32 offset = MTH_Mul(randomNumber() >> 0x10, 0xB60B60);
    pFormation->m254[2] += offset - 0x5B05B0;
    sVec3_FP zero; zero.zeroize();
    spawnHitSpark(parent, &targetable->m10_position, &zero, 0x20000, 0);
}

void createArachnothFormation(s_workAreaCopy* pParent, u32 arg0, u32 arg1)
{
    sArachnothFormation::TypedTaskDefinition definition = {
        nullptr,
        arachnothFormation_update,
        arachnothFormation_draw,
        nullptr,
    };

    sArachnothFormation* pThis = createSubTaskWithCopy<sArachnothFormation>(pParent, &definition);
    pThis->m344.fill(0);
    pThis->m32C = 0;
    pThis->m330_idleJitterX = 0;
    pThis->m334_idleJitterY = 0;
    pThis->m338_idleJitterZ = 0;
    pThis->m33C = 0;
    pThis->m340 = 0;
    allocateNPC(pThis, 8);
    pThis->m0_fileBundle = dramAllocatorEnd[8].mC_fileBundle->m0_fileBundle;

    arachnothCreateSubModel(&pThis->m8_normalBody, pThis, dramAllocatorEnd[8].mC_fileBundle, arg1, g_BTL_A3_2->getSaturnPtr(0x60A9578));
    arachnothInitSubModelAnimation(&pThis->m8_normalBody, 0, -1);
    arachnoth_setSubModelCallbacks(&pThis->m8_normalBody, nullptr, arachnothSubModelFunction0, arachnothSubModelFunction1, arachnothSubModelFunction2);

    arachnothCreateSubModel2(&pThis->m98_poisonDart, pThis, dramAllocatorEnd[8].mC_fileBundle, arg1, g_BTL_A3_2->getSaturnPtr(0x60A9584), g_BTL_A3_2->getSaturnPtr(0x60A9538), g_BTL_A3_2->getSaturnPtr(0x60A9540));
    arachnothInitSubModelAnimation(&pThis->m98_poisonDart, 0, -1);
    arachnoth_setSubModelCallbacks(&pThis->m98_poisonDart, nullptr, arachnothSubModelFunction0, arachnothSubModelFunction1, arachnothSubModelFunction2);

    arachnothCreateSubModel(&pThis->m128_eatDragonBody, pThis, dramAllocatorEnd[8].mC_fileBundle, arg1, g_BTL_A3_2->getSaturnPtr(0x60A9590));
    arachnothInitSubModelAnimation(&pThis->m128_eatDragonBody, 0, -1);
    arachnoth_setSubModelCallbacks(&pThis->m128_eatDragonBody, nullptr, arachnothSubModelFunction0, arachnothSubModelFunction1, arachnothSubModelFunction2);

    sModelHierarchy* pHierarchy = pThis->m0_fileBundle->getModelHierarchy(0x18);
    sStaticPoseData* pStaticPose = pThis->m0_fileBundle->getStaticPose(0x1CC, pHierarchy->countNumberOfBones());
    init3DModelRawData(pThis, &pThis->m1BC_debrits, 0, pThis->m0_fileBundle, 0x18, nullptr, pStaticPose, nullptr, nullptr);

    pThis->m230[0] = 0x201000;
    pThis->m230[1] = 0x13000;
    pThis->m230[2] = -0x1D2000;

    pThis->m20C.zeroize();
    pThis->m218.zeroize();

    pThis->m224_translation = pThis->m230;

    pThis->m254.zeroize();
    pThis->m260.zeroize();
    pThis->m26C_rotation.zeroize();

    pThis->m26C_rotation[1] = 0x8000000;
    
    pThis->m288.zeroize();

    pThis->m298_life = 2500;

    pThis->m29C_lifeMeter = createEnemyLifeMeterTask(&pThis->m224_translation, 0, &pThis->m298_life, 7);

    pThis->m2A0 = 0;
    pThis->m2A4 = 0;
    pThis->m2A8 = 0;

    pThis->m1B8_currentActiveModel = &pThis->m8_normalBody;

    pThis->m2AC_enrageState = 0;
    pThis->m2B0_arachnothState = 0;
    pThis->m2B4 = 0;
    pThis->m27C.zeroize();
    pThis->m27C[1] = 0x8000000;

    for (int i = 0; i < 2; i++) {
        pThis->m314_tentacles[i].fill(nullptr);
    }

    pThis->m278_targetedQuadrant = 2;
    pThis->m23C_chargeDirection = 1;

    static sVec3_FP offsetVector(0x201000, 0x13000, -0x1D2000);
    pThis->m240 = &offsetVector;

    arachnoth_rotateTargetableFlags(&pThis->m8_normalBody, pThis->m278_targetedQuadrant);
    arachnoth_rotateTargetableFlags(&pThis->m98_poisonDart, pThis->m278_targetedQuadrant);
    arachnoth_updateQuadrantRadar(pThis);
    gBattleManager->m10_battleOverlay->m4_battleEngine->m22F_battleRadarLockIcon = 1;
    displayFormationName(0, 1, 11);
}
