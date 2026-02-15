#include "PDS.h"
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
#include "BTL_A3_2_data.h"
#include "kernel/fileBundle.h"
#include "kernel/animation.h"
#include "kernel/graphicalObject.h"
#include "audio/systemSounds.h"
#include "ArachnothSubPart.h"

//https://youtu.be/Txks9hG21qs?t=3130

//Enrage:
//https://youtu.be/Txks9hG21qs?t=3194

//Charge:
//https://youtu.be/Txks9hG21qs?t=3225

void battleEngine_displayAttackName(int param1, int param2, int param3); // TODO: Cleanup
void playBattleSoundEffect(s16 effectIndex); // TODO: cleanup

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
    s32 m2BC;
    s32 m2C0;
    s32 m2C4;
    s32 m2C8;
    sVec3_FP m2CC;
    sVec3_FP m2D8;
    sVec3_FP m2E4;
    std::array<fixedPoint, 6> m314;
    std::array<s32, 2> m344;
    //size 0x34C
};

void arachnothUpdateQuadrants(sArachnothFormation* pThis);

void arachnoth_updateState1_sub0(sArachnothFormation* pThis)
{
    int oldTargetedQuadrant = pThis->m278_targetedQuadrant;
    s32 quadrantDiff = gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant - pThis->m278_targetedQuadrant;
    quadrantDiff &= 3;

    switch (quadrantDiff)
    {
    case 0:
        break;
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
    createArachnothFormationSub0(&pThis->m8_normalBody, pThis->m278_targetedQuadrant);
    createArachnothFormationSub0(&pThis->m98_poisonDart, pThis->m278_targetedQuadrant);
    gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2_needToSortEnemiesByDistanceFromDragon = 1;
    arachnothUpdateQuadrants(pThis);
}

void arachnoth_updateState1_sub1(sArachnothFormation* pThis)
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
        createArachnothFormationSub0(&pThis->m8_normalBody, pThis->m278_targetedQuadrant);
        createArachnothFormationSub0(&pThis->m98_poisonDart, pThis->m278_targetedQuadrant);
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2_needToSortEnemiesByDistanceFromDragon = 1;
        arachnothUpdateQuadrants(pThis);
    }
}

bool arachnoth_updateState0_sub3_sub(sArachnothFormation* pThis)
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

void arachnoth_enterMode6(s16 param1)
{
    gBattleManager->m10_battleOverlay->m4_battleEngine->m22E_dragonMoveDirection = 3;
    battleEngine_SetBattleMode(mA);
    gBattleManager->m10_battleOverlay->m4_battleEngine->m27C_dragonMovementInterpolator1.m68_rate = param1;
    gBattleManager->m10_battleOverlay->m4_battleEngine->m2E8_dragonMovementInterpolator2.m68_rate = param1;
    gBattleManager->m10_battleOverlay->m4_battleEngine->m38E = 0;
}

void arachnoth_updateState0_sub6(sArachnothFormation* pThis)
{
    Unimplemented();
}

void arachnoth_updateState0_sub3(sArachnothFormation* pThis)
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
            arachnoth_updateState1_sub1(pThis);
            mode = 3;
        }
        break;
    case 1:
        if ((((gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant) == pThis->m278_targetedQuadrant) ||
            ((gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant) == (pThis->m278_targetedQuadrant - 1U & 3))) ||
            ((gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant) == (pThis->m278_targetedQuadrant + 1U & 3)))
        {
            arachnoth_updateState1_sub0(pThis);
            if (!arachnoth_updateState0_sub3_sub(pThis))
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

    if ((mode != 1) && (mode != 2) && (mode != 3) && pThis->m314[0])
    {
        Unimplemented();
    }

    switch (mode)
    {
    case 0:
        break;
    case 1:
        switch (pThis->m2AC_enrageState)
        {
        case 0:
            arachnoth_updateState1_sub0(pThis);
            break;
        case 1:
            arachnoth_updateState1_sub1(pThis);
            break;
        default:
            assert(0);
        }
        battleEngine_SetBattleMode(m7);
        pThis->m2BC = 0;
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

        createBattleIntroTaskSub0();
        battleEngine_setCurrentCameraPositionPointer(&pThis->m2CC);
        battleEngine_setDesiredCameraPositionPointer(&pThis->m224_translation);
        battleEngine_InitSub8();

        pThis->m2BC = 0x78;
        pThis->m2B0_arachnothState = 2;
        pThis->m2C0 = 0;
        pThis->m2C4 = 0x1E;
        break;
    case 3: // Attack: tentacles
        battleEngine_SetBattleMode(m7);
        battleEngine_displayAttackName(0x5, 0x5A, 0);
        battleEngine_FlagQuadrantForAttack(pThis->m278_targetedQuadrant);

        if (pThis->m314[0] == 0)
        {
            Unimplemented();
        }
        
        pThis->m2D8[0] = 0;
        pThis->m2D8[1] = (pThis->m278_targetedQuadrant + 1) * 0x4000000;
        pThis->m2D8[2] = 0;

        pThis->m2CC[0] = MTH_Mul_5_6(getCos(pThis->m2D8[0].getInteger()), getSin(pThis->m2D8[1].getInteger()), 0x3C000);
        pThis->m2CC[1] = MTH_Mul(-getSin(pThis->m2D8[0].getInteger()), 0x3C000);
        pThis->m2CC[2] = MTH_Mul_5_6(getCos(pThis->m2D8[0].getInteger()), getCos(pThis->m2D8[1].getInteger()), 0x3C000);

        pThis->m2CC += *pThis->m240;
        pThis->m2E4 = pThis->m224_translation;

        createBattleIntroTaskSub0();
        battleEngine_setCurrentCameraPositionPointer(&pThis->m2CC);
        battleEngine_setDesiredCameraPositionPointer(&pThis->m224_translation);
        battleEngine_InitSub8();

        pThis->m2BC = 0xD2;
        pThis->m2B0_arachnothState = 3;
        pThis->m2C0 = 0;
        pThis->m2C4 = 0x3C;
        break;
    case 6:
        pThis->m2B0_arachnothState = 6;
        pThis->m2BC = 300;
        arachnoth_enterMode6((pThis->m2BC >> 16) - 0x96);
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
            createBattleIntroTaskSub0();
            battleEngine_setCurrentCameraPositionPointer(&pThis->m2CC);
            battleEngine_setDesiredCameraPositionPointer(&pThis->m224_translation);
            battleEngine_InitSub8();
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
            pThis->m2C0 = 0;
        }
        break;
    case 8:
        arachnoth_updateState0_sub6(pThis);
        break;
    default:
        assert(0);
    }
}

void arachnoth_updateState0_sub5Sub(sArachnothFormation* pThis)
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

    createBattleIntroTaskSub0();
    battleEngine_setCurrentCameraPositionPointer(&pThis->m2CC);
    battleEngine_setDesiredCameraPositionPointer(&pThis->m224_translation);
    battleEngine_InitSub8();
    arachnoth_updateState0_sub5Sub(pThis);

    pThis->m2BC = 0xB4;
    pThis->m2B0_arachnothState = 4;
    pThis->m2C0 = 0;
    pThis->m2C4 = 0x5A;
}

void createArachnothDigestiveEffect(p_workArea parent, sVec3_FP*, sVec3_FP* pPosition, s32 param4)
{
    Unimplemented();
}

void arachnoth_updateState2_sub0(sArachnothFormation* pThis)
{
    switch (pThis->m2C0)
    {
    case 0:
        if (--pThis->m2C4 < 0)
        {
            pThis->m2C0++;
            pThis->m2C4 = 0;
        }
        break;
    case 1:
        if ((randomNumber() & 3) == 0)
        {
            if (pThis->m2C4 == 4)
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
                pThis->m2C0++;
                pThis->m2C4 = 0x20;
            }
            else
            {
                Unimplemented();
                playSystemSoundEffect(0x71);
                pThis->m254[0] -= 0xB60b6;
                pThis->m2C4++;
            }
        }
        break;
    case 2:
        if (--pThis->m2C4 < 0)
        {
            sVec3_FP local_2c;
            fixedPoint local_68 = MTH_Mul(fixedPoint(randomNumber()).toInteger(), 0x1000);
            local_2c[1] = MTH_Mul(getSin(MTH_Mul(fixedPoint(randomNumber()).toInteger(), 0x10000000).toInteger()), local_68);

            local_68 = MTH_Mul(fixedPoint(randomNumber()).toInteger(), 0x1000);
            local_68 = MTH_Mul(getCos(MTH_Mul(fixedPoint(randomNumber()).toInteger(), 0x10000000).toInteger()), local_68);
            local_2c[2] = MTH_Mul(getSin(MTH_Mul(fixedPoint(randomNumber()).toInteger(), 0x10000000).toInteger()), local_68);

            local_2c[0] = MTH_Mul(getSin(MTH_Mul(fixedPoint(randomNumber()).toInteger(), 0x10000000).toInteger()), local_68);

            applyDamageToDragon(gBattleManager->m10_battleOverlay->m18_dragon->m8C, 40, gBattleManager->m10_battleOverlay->m18_dragon->m8_position, 3, local_2c, 0);
            pThis->m2C0++;
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

bool arachnoth_updateState2_sub1(sArachnothFormation* pThis)
{
    if (--pThis->m2BC < 0)
    {
        return true;
    }
    return false;
}

void arachnoth_updateState0_sub0(sArachnothFormation* pThis)
{
    Unimplemented();
}

void arachnoth_createChargeDebrits(sArachnothFormation* pThis, npcFileDeleter* param_2, s_3dModel* p3dModel, sVec3_FP* position, sVec3_FP* rotation, s32 param6, s32 param7, s32 param8, s32 param9, s32 param10, s32 param11, s32 param12, s32 param13, s32 param14)
{
    Unimplemented();
}

void arachnoth_createChargeImpact(sArachnothFormation* pThis)
{
    Unimplemented();
}

void arachnoth_createSmokeParticle(npcFileDeleter* param_2, sSaturnPtr, sVec3_FP*, s32, s32, s32, s32, s32)
{
    Unimplemented();
}

void arachnoth_updateState6_sub0(sArachnothFormation* pThis)
{
    switch (pThis->m2C0)
    {
    case 0:
        if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2000)
        {
            pThis->m2C0++;
            pThis->m2C4 = 0x3C;
        }
        break;
    case 1:
        if (--pThis->m2C4 < 0x1F)
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
        if (pThis->m2C4 < 1)
        {
            if (pThis->m23C_chargeDirection == 0)
            {
                pThis->m230 = sVec3_FP(0x201000, 0x1D000, -0x2488F0);
            }
            else
            {
                pThis->m230 = sVec3_FP(0x201000, 0x1D000, -0x1BC000);
            }
            pThis->m230[2] -= 0x32000;
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
            arachnoth_createChargeDebrits(pThis, dramAllocatorEnd[8].mC_fileBundle, &pThis->m1BC_debrits, &temp, &temp2, 0, 0, -0x2C, 0x1fffff, 0, 0, 0, 0x10000, 0);
            playSystemSoundEffect(0x6b);
            pThis->m2C4 = 0x5a;
            pThis->m2C0++;
        }
        break;
    case 2:
        pThis->m288[0] += MTH_Mul(-pThis->m288[2], 0x28F);
        if (pThis->m2C4 == 0x46)
        {
            playSystemSoundEffect(0x76);
        }
        if (pThis->m2C4 == 0x42)
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
                pThis->m2C4 = 0;
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
                pThis->m2C4 = 0;
            }
        }
        if (--pThis->m2C4 < 1)
        {
            playBattleSoundEffect(0x76);
            playSystemSoundEffect(0x74);
            arachnoth_createChargeImpact(pThis);
            pThis->m2C4 = 0x5A;
            pThis->m2C0++;
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
            arachnoth_createChargeDebrits(pThis, dramAllocatorEnd[8].mC_fileBundle, &pThis->m1BC_debrits, &temp, &temp2, 0, 0, -0x2C, 0x1fffff, 0, 0, 0, 0x10000, 0);
        }
        if (!(randomNumber() & 0xF))
        {
            pThis->m254[2] += MTH_Mul(randomNumber() >> 16, 0x71C71C) - 0x38E38E;
            arachnoth_createChargeImpact(pThis);
        }
        pThis->m288[0] += MTH_Mul(-pThis->m288[2], 0x28F);
        if (--pThis->m2C4 < 1)
        {
            pThis->m2C0++;
        }
        break;
    }
    case 4: // Unconscious
        if (pThis->m2BC == 0x1E)
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
    battleEngine_InitSub8();
    battleEngine_setCurrentCameraPositionPointer(&pThis->m2CC);
}

void arachnoth_updateState4_sub0(sArachnothFormation* pThis)
{
    switch (pThis->m2C0)
    {
    case 0:
        if (--pThis->m2C4 < 0)
        {
            pThis->m2C0++;
            pThis->m2C4 = 7;
            pThis->m2AC_enrageState = 1;
            playSystemSoundEffect(0x70);
            pThis->m2C8 = 0x1E;
        }
        break;
    case 1:
        if (pThis->m2C4 < 1)
        {
            pThis->m2C4++;
            if (pThis->m2C4 == 0)
            {
                pThis->m2C4 = 7;
            }
            pThis->m254[1] -= 0xB60B6;
        }
        else
        {
            pThis->m2C4--;
            if (pThis->m2C4 == 0)
            {
                pThis->m2C4 = -7;
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

void arachnoth_updateState3_sub0(sArachnothFormation* pThis)
{
    Unimplemented();
}

void arachnothFormation_updateSub10(sArachnothFormation* pThis)
{
    Unimplemented();
}

void arachnoth_enterUnconsciousState(sArachnothFormation* pThis)
{
    pThis->m2BC = MTH_Mul(randomNumber() >> 16, 0x78) + 0xB4;
    pThis->m2B0_arachnothState = 7;
    arachnothFormation_updateSub10(pThis);
    arachnothUpdateQuadrants(pThis);
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

void arachnoth_updateState(sArachnothFormation* pThis)
{
    switch (pThis->m2B0_arachnothState)
    {
    case 0:
        arachnoth_updateState0_sub0(pThis);
        if (!gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x1D])
        {
            if (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m400000)
            {
                if (!BattleEngineSub0_UpdateSub0() || battleEngine_UpdateSub7Sub0Sub0())
                {
                    if (gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m8 == 1)
                    {
                        arachnoth_updateState0_sub3(pThis);
                    }
                }
            }

            if (pThis->m2AC_enrageState == 0)
            {
                if (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m400000)
                {
                    if (!BattleEngineSub0_UpdateSub0() || battleEngine_UpdateSub7Sub0Sub0())
                    {
                        if (pThis->m298_life < 1500)
                        {
                            if (pThis->m314[0])
                            {
                                assert(0);
                            }
                            arachnoth_startEnrage(pThis);
                        }
                    }
                }
            }

            if (pThis->m298_life < 1)
            {
                arachnoth_updateState0_sub6(pThis);
            }
        }
        break;
    case 1:
        if (arachnoth_updateState2_sub1(pThis))
        {
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished = 1;
            createArachnothFormationSub0(&pThis->m8_normalBody, pThis->m278_targetedQuadrant);
            createArachnothFormationSub0(&pThis->m98_poisonDart, pThis->m278_targetedQuadrant);
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2_needToSortEnemiesByDistanceFromDragon = 1;
            arachnothUpdateQuadrants(pThis);
            if (((5 < pThis->m2B8) && (pThis->m2AC_enrageState == 0)) && (pThis->m314[0] == 0))
            {
                assert(0);
            }
            gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m8 = 0;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m0 = 0;

            pThis->m2B0_arachnothState = 0;
        }
        break;
    case 2: // wait for end of attack
        arachnoth_updateState2_sub0(pThis);
        if (arachnoth_updateState2_sub1(pThis))
        {
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished = 1;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m8 = 0;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m0 = 0;
            sEnemyAttackCamera_updateSub2();
            pThis->m2B0_arachnothState = 0;
            pThis->m2B8 = 0;
            arachnothUpdateQuadrants(pThis);
        }
        break;
    case 3:
        arachnoth_updateState3_sub0(pThis);
        if (arachnoth_updateState2_sub1(pThis))
        {
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished = 1;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m8 = 0;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m0 = 0;
            sEnemyAttackCamera_updateSub2();
            pThis->m2B0_arachnothState = 0;
            if(pThis->m314[0])
            {
                Unimplemented();
            }
            pThis->m2B8 = 0;
            arachnothUpdateQuadrants(pThis);
        }
        break;
    case 4:
        arachnoth_updateState4_sub0(pThis);
        if (arachnoth_updateState2_sub1(pThis))
        {
            arachnoth_updateState1_sub1(pThis);
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished = 1;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m8 = 0;
            gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m0 = 0;
            sEnemyAttackCamera_updateSub2();
            gBattleManager->m10_battleOverlay->m4_battleEngine->m3CC->m2 = 0x2D;
            arachnothUpdateQuadrants(pThis);
            pThis->m2B0_arachnothState = 0;
        }
        break;
    case 6: // charging
        arachnoth_updateState6_sub0(pThis);
        if (arachnoth_updateState2_sub1(pThis))
        {
            sEnemyAttackCamera_updateSub2();
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
            arachnothUpdateQuadrants(pThis);
        }
        break;
    case 7: // unconscious
        Unimplemented();
        break;
    default:
        assert(0);
        break;
    }
}

void arachnothFormation_updateSub6(sArachnothSubModel* pThis, s32 param2, s32 param3)
{
    if (readSaturnS32(pThis->m4 + 8))
    {
        for (int i = 0; i < pThis->m64; i++)
        {
            pThis->m58_targetables[i].m60 = readSaturnS8(readSaturnEA(pThis->m4 + 8) + (param2 - param3 & 3));
        }
    }
}

void arachnothFormation_updateSub9(sArachnothFormation* pThis, sVec3_FP* position, sVec3_FP* rotation, sVec3_FP* param_4)
{
    Unimplemented();
}

void arachnothFormation_update(sArachnothFormation* pThis)
{
    s32 damageTaken = arachnothSubPartGetDamage(&pThis->m8_normalBody) + arachnothSubPartGetDamage(&pThis->m98_poisonDart);
    if (damageTaken > 0)
    {
        assert(0);
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
        assert(0);
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
            assert(0);
        }
    }

    gBattleManager->m10_battleOverlay->m4_battleEngine->m3B2_numBattleFormationRunning++;

    arachnoth_updateState(pThis);

    arachnothFormation_updateSub6(&pThis->m8_normalBody, gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant, pThis->m278_targetedQuadrant);
    arachnothFormation_updateSub6(&pThis->m98_poisonDart, gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant, pThis->m278_targetedQuadrant);

    if ((pThis->m2B0_arachnothState != 8) && (pThis->m2B0_arachnothState != 9))
    {
        if ((randomNumber() & 0x1F) == 0)
        {
            sVec3_FP temp;
            temp[0] = MTH_Mul(fixedPoint(randomNumber()).toInteger(), 0x6000) - 0x3000;
            temp[1] = MTH_Mul(fixedPoint(randomNumber()).toInteger(), 0x2000) - 0x1000;
            temp[2] = MTH_Mul(fixedPoint(randomNumber()).toInteger(), 0x3000) + 0x2000;
            arachnothFormation_updateSub9(pThis, &pThis->m224_translation, &pThis->m26C_rotation, &temp);
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

        if (pThis->m314[0])
        {
            pThis->m288[0] = pThis->m254[0] + 0x12345;
        }
    }

    if (pThis->m314[0])
    {
        assert(0);
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

    //arachnothFormation_updateSub8(&pThis->m98_poisonDart, pThis, &pArachnothSubModelData3, &pArachnothSubModelData4, pThis->m288[2]);

    Unimplemented();
}

void arachnothFormation_draw(sArachnothFormation* pThis)
{
    if (!isShipping())
    {
        if (ImGui::Begin("Arachnoth"))
        {
            int life = pThis->m298_life;
            ImGui::InputInt("Life", &life);
            pThis->m298_life = life;
        }
        ImGui::End();
    }

    if (pThis->m1B8_currentActiveModel->m70_flags & 0x800000)
    {
        assert(0);
    }

    pushCurrentMatrix();
    translateCurrentMatrix(pThis->m224_translation);
    rotateCurrentMatrixYXZ(pThis->m26C_rotation);
    pThis->m1B8_currentActiveModel->m8_model.m18_drawFunction(&pThis->m1B8_currentActiveModel->m8_model);
    popMatrix();

    if (pThis->m1B8_currentActiveModel->m70_flags & 0x800000)
    {
        assert(0);
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
        assert(0);
    }

    pushCurrentMatrix();
    translateCurrentMatrix(dartPosition);
    rotateCurrentMatrixYXZ(pThis->m26C_rotation);
    pThis->m98_poisonDart.m8_model.m18_drawFunction(&pThis->m98_poisonDart.m8_model);
    popMatrix();

    if (pThis->m98_poisonDart.m70_flags & 0x800000)
    {
        assert(0);
    }

    if ((pThis->m2A0 > 0) && (pThis->m344[0] == 0))
    {
        pThis->m29C_lifeMeter->m31 |= 2;
    }
}

void arachnothUpdateQuadrants(sArachnothFormation* pThis)
{
    switch (pThis->m2AC_enrageState)
    {
    case 0:
        if (pThis->m2B8 > 5)
        {
            assert(0);
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
    allocateNPC(pThis, 8);
    pThis->m0_fileBundle = dramAllocatorEnd[8].mC_fileBundle->m0_fileBundle;

    arachnothCreateSubModel(&pThis->m8_normalBody, pThis, dramAllocatorEnd[8].mC_fileBundle, arg1, g_BTL_A3_2->getSaturnPtr(0x60A9578));
    arachnothInitSubModelAnimation(&pThis->m8_normalBody, 0, -1);
    arachnothInitSubModelFunctions(&pThis->m8_normalBody, nullptr, arachnothSubModelFunction0, arachnothSubModelFunction1, arachnothSubModelFunction2);

    arachnothCreateSubModel2(&pThis->m98_poisonDart, pThis, dramAllocatorEnd[8].mC_fileBundle, arg1, g_BTL_A3_2->getSaturnPtr(0x60A9584), g_BTL_A3_2->getSaturnPtr(0x60A9538), g_BTL_A3_2->getSaturnPtr(0x60A9540));
    arachnothInitSubModelAnimation(&pThis->m98_poisonDart, 0, -1);
    arachnothInitSubModelFunctions(&pThis->m98_poisonDart, nullptr, arachnothSubModelFunction0, arachnothSubModelFunction1, arachnothSubModelFunction2);

    arachnothCreateSubModel(&pThis->m128_eatDragonBody, pThis, dramAllocatorEnd[8].mC_fileBundle, arg1, g_BTL_A3_2->getSaturnPtr(0x60A9590));
    arachnothInitSubModelAnimation(&pThis->m128_eatDragonBody, 0, -1);
    arachnothInitSubModelFunctions(&pThis->m128_eatDragonBody, nullptr, arachnothSubModelFunction0, arachnothSubModelFunction1, arachnothSubModelFunction2);

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

    pThis->m314.fill(0);

    pThis->m278_targetedQuadrant = 2;
    pThis->m23C_chargeDirection = 1;

    static sVec3_FP offsetVector(0x201000, 0x13000, -0x1D2000);
    pThis->m240 = &offsetVector;

    createArachnothFormationSub0(&pThis->m8_normalBody, pThis->m278_targetedQuadrant);
    createArachnothFormationSub0(&pThis->m98_poisonDart, pThis->m278_targetedQuadrant);
    arachnothUpdateQuadrants(pThis);
    gBattleManager->m10_battleOverlay->m4_battleEngine->m22F_battleRadarLockIcon = 1;
    displayFormationName(0, 1, 11);
}
