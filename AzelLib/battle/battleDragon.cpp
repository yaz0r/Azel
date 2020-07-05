#include "PDS.h"
#include "battleDragon.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"
#include "battleDebug.h"
#include "battleGrid.h"
#include "battleDamageDisplay.h"
#include "kernel/fileBundle.h"
#include "kernel/debug/trace.h"
#include "kernel/animation.h"
#include "audio/systemSounds.h"
#include "commonOverlay.h"

void s_battleDragon_InitSub4Sub0()
{
    updateAndInterpolateAnimation(&gDragonState->m28_dragon3dModel);
    updateAnimationMatrices(&gDragonState->m78_animData, &gDragonState->m28_dragon3dModel);
}

void s_battleDragon_InitSub4(int arg0, int interpolationLength)
{
    playAnimation(&gDragonState->m28_dragon3dModel, gDragonState->m0_pDragonModelBundle->getAnimation(gDragonState->m20_dragonAnimOffsets[arg0]), interpolationLength);
    s_battleDragon_InitSub4Sub0();
}

void s_battleDragon_InitSub5(sBattleTargetable* pThis, s_battleDragon* param2, sVec3_FP* param3, s32 param4, s32 param5)
{
    pThis->m0 = param2;
    pThis->m4_pPosition = param3;
    pThis->m4C = param4;
    pThis->m40 = *param3;
    battleTargetable_updatePosition(pThis);
    pThis->m8 = &pThis->m10_position;
    pThis->mC = 0;
    pThis->m50_flags = param5;
    pThis->m58 = 0;
    pThis->m5A = 0;
    pThis->m5E_impactForce = 0;
    pThis->m5F = 0;
    pThis->m5C = 0;
    pThis->m54 = 0;
    pThis->m60 = 10;
}

void s_battleDragon_InitSub3(s_loadRiderWorkArea* pRider, s16 param2, s32 param3)
{
    playAnimationGeneric(&pRider->m18_3dModel, pRider1State->m0_riderBundle->getAnimation(param2), param3);
    updateAndInterpolateAnimation(&pRider->m18_3dModel);
}

void s_battleDragon_InitSub1(s_loadRiderWorkArea* pRider, s16 param2, s32 param3)
{
    playAnimationGeneric(&pRider->m18_3dModel, pRider1State->m0_riderBundle->getAnimation(param2), param3);
    updateAndInterpolateAnimation(&pRider->m18_3dModel);
}

static void s_battleDragon_Init(s_battleDragon* pThis)
{
    gBattleManager->m10_battleOverlay->m18_dragon = pThis;

    pThis->m8_position = gBattleManager->m10_battleOverlay->m4_battleEngine->m104_dragonPosition;
    resetMatrixStack();
    pThis->m1CE_positionInAnimList = 0;

    if (isBattleAutoScroll() == 0)
    {
        pThis->m74_targetRotation[1] = pThis->m14_rotation[1] = readSaturnFP(gCurrentBattleOverlay->getSaturnPtr(0x60ae3bc) + gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant * 4);

        if (gDragonState->mC_dragonType == 8)
        {
            assert(0);
        }
        else
        {
            s_battleDragon_InitSub3(pRider1State, 0x5C, 0);
            s_battleDragon_InitSub3(pRider2State, 0x50, 0);
            pThis->m1CC_currentAnimation = 2;
        }
    }
    else
    {
        if (gDragonState->mC_dragonType == 8)
        {
            assert(0);
        }
        else
        {
            s_battleDragon_InitSub3(pRider1State, readSaturnS16(gCurrentBattleOverlay->getSaturnPtr(0x60ae368) + gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant * 2), 0);
            s_battleDragon_InitSub3(pRider2State, readSaturnS16(gCurrentBattleOverlay->getSaturnPtr(0x60ae370) + gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant * 2), 0);
            pThis->m1CC_currentAnimation = 2;
        }
    }

    s_battleDragon_InitSub4(pThis->m1CC_currentAnimation, 0);
    s_battleDragon_InitSub5(&pThis->m8C, pThis, &pThis->m1A4, 0x1000, 0xf0000001);
    pThis->m1C8 = 0x111111;

    if ((mainGameState.gameStats.m1_dragonLevel == 6) && (pThis->m24C_lightWingEffectTask == 0))
    {
        FunctionUnimplemented();
    }
    else
    {
        pThis->m24C_lightWingEffectTask = 0;
    }
}

void battleEngine_InitSub11();

void s_battleDragon_UpdateSub0(s_battleDragon* pThis)
{
    if (gDragonState->m88)
    {
        battleEngine_InitSub11();
        if (mainGameState.gameStats.m1_dragonLevel == 8)
        {
            assert(0);
        }
        else
        {
            if (pRider1State->mC_riderType > 5)
            {
                loadRiderIfChanged(1);
            }
            if (pRider2State->mC_riderType > 5)
            {
                loadRider2IfChanged(4);
            }
            if (mainGameState.gameStats.m1_dragonLevel == 6)
            {
                FunctionUnimplemented();
            }
            else
            {
                if (pThis->m24C_lightWingEffectTask)
                {
                    pThis->m24C_lightWingEffectTask->getTask()->markFinished();
                    pThis->m24C_lightWingEffectTask = nullptr;
                }
            }
        }
    }
}

void s_battleDragon_UpdateSub1(s_battleDragon* pThis)
{
    s_battleEngine* pBattleEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0xF])
    {
        assert(0);
    }

    sVec3_FP iVar3 = pThis->m8_position;
    sVec3_FP local_44 = pBattleEngine->m104_dragonPosition + pBattleEngine->m164;
    local_44 -= pBattleEngine->m234;
    
    pushCurrentMatrix();
    translateCurrentMatrix(pBattleEngine->m234);
    rotateCurrentMatrixZYX(pBattleEngine->m220_battleVector);
    sVec3_FP auStack56;
    transformAndAddVecByCurrentMatrix(&local_44, &auStack56);
    transformAndAddVec(auStack56, pThis->m8_position, cameraProperties2.m28[0]);

    if (isTraceEnabled())
    {
        addTraceLog(pBattleEngine->m234, "pBattleEngine->m234");
        addTraceLog(pBattleEngine->m220_battleVector, "pBattleEngine->m220");
        addTraceLog(cameraProperties2.m28[0], "cameraProperties2.m28[0]");
        addTraceLog(pThis->m8_position, "DragonPosition");
    }

    popMatrix();
    pThis->m5C_deltaPosition = pThis->m8_position - iVar3;
}

void battleDragon_updateAnimation(s_battleDragon* pThis, sSaturnPtr animData)
{
    s16 originalAnimation = pThis->m1CC_currentAnimation;
    int interpolationLength = 0;

    int numFrames = 0;
    if (gDragonState->m28_dragon3dModel.m30_pCurrentAnimation)
    {
        numFrames = gDragonState->m28_dragon3dModel.m30_pCurrentAnimation->m4_numFrames;
    }

    if (numFrames - 1 <= gDragonState->m28_dragon3dModel.m16_previousAnimationFrame)
    {
        sSaturnPtr pDragonAnimData = readSaturnEA(animData + gDragonState->m1C_dragonArchetype * 4 + gDragonState->mC_dragonType * 0x14);

        pThis->m1CE_positionInAnimList++;
        pThis->m1CC_currentAnimation = readSaturnS8(pDragonAnimData + pThis->m1CE_positionInAnimList);

        if (pThis->m1CC_currentAnimation < 0)
        {
            pThis->m1CE_positionInAnimList = 0;
            pThis->m1CC_currentAnimation = readSaturnS8(pDragonAnimData + pThis->m1CE_positionInAnimList);
        }

        if (pThis->m1CC_currentAnimation != originalAnimation)
        {
            interpolationLength = 9;
        }

        s_battleDragon_InitSub4(pThis->m1CC_currentAnimation, interpolationLength);
    }
}

void s_battleDragon_UpdateSub2Sub0(s_battleDragon* pThis)
{
    int uVar5 = pThis->m84 & 0x1F;
    int uVar7 = pThis->m84 & 0x20;
    if (mainGameState.gameStats.m1_dragonLevel == 8)
    {
        assert(0);
    }
    
    if (pThis->m1C4 & 0x40)
    {
        assert(0);
    }

    if (uVar7 == 0)
    {
        pThis->m1C4 &= ~1;
    }
    else
    {
        pThis->m1C4 |= 1;
    }

    if ((pThis->m84 & 2) == 0)
    {
        if ((pThis->m84 & 4) == 0)
        {
            if (pThis->m84 & 1)
            {
                if(uVar7 == 0)
                {
                    pThis->m1CC_currentAnimation = 10;
                }
                else
                {
                    pThis->m1CC_currentAnimation = 0xB;
                }
                s_battleDragon_InitSub4(pThis->m1CC_currentAnimation, 9);
                return;
            }
            if (pThis->m84 & 0x10)
            {
                if (uVar7 == 0)
                {
                    pThis->m1CC_currentAnimation = 0xE;
                }
                else
                {
                    pThis->m1CC_currentAnimation = 0xF;
                }
                incrementAnimationRootY(&gDragonState->m78_animData, pThis->m1C8 * -2);
                pThis->m1CE_positionInAnimList = 0;
                s_battleDragon_InitSub4(pThis->m1CC_currentAnimation, 0);
                return;
            }
            if(pThis->m84 & 0x8)
            {
                assert(0);
            }
        }
        else
        {
            pThis->m1CE_positionInAnimList = -1;
            pThis->m1C4 |= 4;
        }
    }
    else
    {
        pThis->m1CE_positionInAnimList = -1;
        pThis->m1C4 |= 2;
    }

    if ((pThis->m1C4 & 2) == 0)
    {
        if ((pThis->m1C4 & 4) == 0)
        {
            if (uVar5)
                return;

            if (uVar7 == 0)
            {
                if ((mainGameState.gameStats.mB8_maxHP / 4) > mainGameState.gameStats.m10_currentHP)
                {
                    battleDragon_updateAnimation(pThis, gCurrentBattleOverlay->getSaturnPtr(0x60adf08));
                }
                else
                {
                    battleDragon_updateAnimation(pThis, gCurrentBattleOverlay->getSaturnPtr(0x60ade68));
                }
            }
            else
            {
                if ((mainGameState.gameStats.mB8_maxHP / 4) > mainGameState.gameStats.m10_currentHP)
                {
                    battleDragon_updateAnimation(pThis, gCurrentBattleOverlay->getSaturnPtr(0x60ae048));
                }
                else
                {
                    battleDragon_updateAnimation(pThis, gCurrentBattleOverlay->getSaturnPtr(0x60adfa8));
                }
            }

            s_battleDragon_InitSub4Sub0();
            if (gDragonState->mC_dragonType == 8)
            {
                assert(0);
            }
            else
            {
                int iVar2 = pThis->m1CC_currentAnimation;
                if (((((iVar2 == 0) || (iVar2 == 1)) || (iVar2 == 5)) || (iVar2 == 6)) &&
                    ((gDragonState->m28_dragon3dModel).m16_previousAnimationFrame == 0xc)) {
                    playSystemSoundEffect(0xb);
                }
            }
            return;
        }
        else
        {
            pThis->m1CC_currentAnimation = 0x11;
            s_battleDragon_InitSub4(pThis->m1CC_currentAnimation, 9);
            pThis->m1CE_positionInAnimList = -1;
            pThis->m1C4 &= ~4;
        }
    }
    else
    {
        pThis->m1CC_currentAnimation = 0x10;
        playSystemSoundEffect(0x12);
        s_battleDragon_InitSub4(pThis->m1CC_currentAnimation, 9);
        pThis->m1CE_positionInAnimList = -1;
        pThis->m1C4 &= ~2;
    }
}

void s_battleDragon_UpdateSub2Sub1Sub0()
{
    if (gDragonState->mC_dragonType != 8)
    {
        u16 offset;
        if (isBattleAutoScroll() == 0)
        {
            offset = 0x5C;
        }
        else
        {
            offset = readSaturnS16(gCurrentBattleOverlay->getSaturnPtr(0x60ae368) + gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant * 2);
        }
        s_battleDragon_InitSub3(pRider1State, offset, 0);
    }
    else
    {
        if (isBattleAutoScroll())
        {
            s_battleDragon_InitSub1(pRider1State, readSaturnS16(gCurrentBattleOverlay->getSaturnPtr(0x60ae378) + gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant * 2), 10);
        }
        else
        {
            s_battleDragon_InitSub1(pRider1State, 0x114, 10);
        }
    }
}

void s_battleDragon_UpdateSub2Sub1Sub1()
{
    u16 offset;
    if (isBattleAutoScroll() == 0)
    {
        offset = 0x5C;
    }
    else
    {
        offset = readSaturnS16(gCurrentBattleOverlay->getSaturnPtr(0x60ae370) + gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant * 2);
    }
    s_battleDragon_InitSub3(pRider2State, offset, 0xF);
}

s32 isRiderAnimUpdateNecessary(s_loadRiderWorkArea* pRider)
{
    int numFramesInCurrentAnimation;
    if (pRider->m18_3dModel.m30_pCurrentAnimation == 0)
    {
        numFramesInCurrentAnimation = 0;
    }
    else
    {
        numFramesInCurrentAnimation = pRider->m18_3dModel.m30_pCurrentAnimation->m4_numFrames;
    }

    if (numFramesInCurrentAnimation - 1 <= pRider->m18_3dModel.m16_previousAnimationFrame)
    {
        return 1;
    }

    return 0;
}

void s_battleDragon_UpdateSub2Sub1(s_battleDragon* pThis)
{
    s32 uVar4 = pThis->m84;
    if (gDragonState->mC_dragonType == 8)
    {
        assert(0);
    }

    if (uVar4 & 0x2E00)
    {
        pThis->m1C4 |= 0x80;
        if (pThis->m84 & 0x200)
        {
            s_battleDragon_InitSub3(pRider1State, 0x70, 5);
        }
        else if (pThis->m84 & 0x400)
        {
            s_battleDragon_InitSub3(pRider1State, 0x74, 5);
        }
        else if (pThis->m84 & 0x800)
        {
            s_battleDragon_InitSub3(pRider1State, 0x78, 5);
        }
        else if (pThis->m84 & 0x2000)
        {
            s_battleDragon_InitSub3(pRider1State, 0x80, 5);
            pThis->m1C4 |= 0x200;
        }
    }

    if (uVar4 & 0x5000)
    {
        pThis->m1C4 |= 0x100;
        if (pThis->m84 & 0x1000)
        {
            s_battleDragon_InitSub3(pRider2State, 0x7c, 5);
        }
        else if (pThis->m84 & 0x4000)
        {
            s_battleDragon_InitSub3(pRider2State, 0x84, 5);
        }
    }

    if ((pThis->m1C4 & 0x80) == 0)
    {
        if (pThis->m1D0 < 1)
        {
            pThis->m1D0 = 0;
            if ((gBattleManager->m10_battleOverlay->m18_dragon->m84 & 0x100) == 0)
            {
                if (pThis->m84 & 0x18)
                {
                    s_battleDragon_InitSub3(pRider1State, 0x58, 5);
                }
            }
            else
            {
                s_battleDragon_UpdateSub2Sub1Sub0();
            }
        }
        else
        {
            assert(0);
        }
    }
    else
    {
        if (isRiderAnimUpdateNecessary(pRider1State))
        {
            pThis->m1C4 &= ~0x80;
            if (pThis->m1C4 & 0x200)
            {
                pThis->m1D0 = 5;
                s_battleDragon_InitSub3(pRider1State, 0x6C, pThis->m1D0);
            }
            else
            {
                s_battleDragon_UpdateSub2Sub1Sub0();
            }
        }
    }

    updateAndInterpolateAnimation(&pRider1State->m18_3dModel);
    if ((pThis->m1C4 & 0x100) == 0)
    {
        if ((gBattleManager->m10_battleOverlay->m18_dragon->m84 & 0x100) == 0)
        {
            if (pThis->m84 & 0x18)
            {
                s_battleDragon_InitSub3(pRider2State, 0x58, 5);
            }
        }
        else
        {
            s_battleDragon_UpdateSub2Sub1Sub1();
        }
    }
    else
    {
        if (isRiderAnimUpdateNecessary(pRider2State))
        {
            pThis->m1C4 &= ~0x100;
            s_battleDragon_UpdateSub2Sub1Sub1();
        }
    }

    updateAndInterpolateAnimation(&pRider2State->m18_3dModel);
}

void s_battleDragon_UpdateSub2(s_battleDragon* pThis)
{
    int gridMode = gBattleManager->m10_battleOverlay->m8_gridTask->m2;
    if ((gridMode == 3) || (gridMode != 4))
        return;

    pThis->m44_deltaRotation[0] += MTH_Mul(pThis->m74_targetRotation[0] - pThis->m14_rotation[0], 0x51E);
    pThis->m44_deltaRotation[0] -= MTH_Mul(pThis->m44_deltaRotation[0], 0x3333);
    pThis->m14_rotation[0] += pThis->m44_deltaRotation[0];

    pThis->m44_deltaRotation[1] += MTH_Mul(pThis->m74_targetRotation[1] - pThis->m14_rotation[1], 0x51E);
    pThis->m44_deltaRotation[1] -= MTH_Mul(pThis->m44_deltaRotation[1], 0x3333);
    pThis->m14_rotation[1] += pThis->m44_deltaRotation[1];

    pThis->m44_deltaRotation[2] += MTH_Mul(pThis->m74_targetRotation[2] - pThis->m14_rotation[2], 0x51E);
    pThis->m44_deltaRotation[2] -= MTH_Mul(pThis->m44_deltaRotation[2], 0x3333);
    pThis->m14_rotation[2] += pThis->m44_deltaRotation[2];

    s_battleDragon_UpdateSub2Sub0(pThis);
    s_battleDragon_UpdateSub2Sub1(pThis);
}

void s_battleDragon_UpdateSub3(s_battleDragon* pThis)
{
    s_battleEngine* pBattleEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;

    fixedPoint stepY;
    fixedPoint stepX;
    fixedPoint stepZ;
    fixedPoint rotationX;
    fixedPoint rotationZ;
    if (isBattleAutoScroll() == 0)
    {
        stepX = MTH_Mul(-pThis->m1C8, pBattleEngine->m1BC_dragonYaw);
        stepY = MTH_Mul(-pThis->m1C8, pBattleEngine->m1B8_dragonPitch);
        stepZ = MTH_Mul(-pThis->m1C8, pBattleEngine->m1BC_dragonYaw);

        rotationX = MTH_Mul(0xE38E38, pBattleEngine->m1B8_dragonPitch);
        MTH_Mul(0x2000000, pBattleEngine->m1BC_dragonYaw);
        rotationZ = MTH_Mul(0x1555555, -pBattleEngine->m1BC_dragonYaw);
    }
    else
    {
        stepX = MTH_Mul_5_6(getCos(pBattleEngine->m440_battleDirectionAngle.getInteger()), -pThis->m1C8, pBattleEngine->m1BC_dragonYaw);
        stepY = MTH_Mul(-pThis->m1C8, pBattleEngine->m1B8_dragonPitch);
        stepZ = MTH_Mul_5_6(getCos(pBattleEngine->m440_battleDirectionAngle.getInteger()), -pThis->m1C8, pBattleEngine->m1BC_dragonYaw);

        rotationX = MTH_Mul(0xE38E38, pBattleEngine->m1B8_dragonPitch);
        MTH_Mul_5_6(getCos(pBattleEngine->m440_battleDirectionAngle.getInteger()), -0x2000000, pBattleEngine->m1BC_dragonYaw);
        rotationZ = MTH_Mul_5_6(getCos(pBattleEngine->m440_battleDirectionAngle.getInteger()), -0x1555555, -pBattleEngine->m1BC_dragonYaw);
    }

    if ((pThis->m1C4 & 8) == 0)
    {
        pThis->m74_targetRotation[0] = rotationX;
        pThis->m74_targetRotation[2] = rotationZ;
    }

    incrementAnimationRootY(&gDragonState->m78_animData, stepY);
    incrementAnimationRootX(&gDragonState->m78_animData, stepX);
    incrementAnimationRootZ(&gDragonState->m78_animData, stepZ);
}

int computeDragonDamage(int damageValue)
{
    fixedPoint computedDamage = MTH_Mul(fixedPoint::fromInteger(damageValue), 0x13333 - MTH_Mul(0x6666, FP_Div(mainGameState.gameStats.mBC_dragonDef, 200)));

    if (gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 0x20000)
    {
        computedDamage = MTH_Mul(computedDamage, 0x8000);
    }
    if (gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 0x100)
    {
        computedDamage = MTH_Mul(computedDamage, 0xab85);
    }
    if (gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 0x10)
    {
        computedDamage = MTH_Mul(computedDamage, 0x20000);
    }

    int finalComputedDamage = computedDamage.toInteger();
    if (finalComputedDamage > 9999)
    {
        finalComputedDamage = 9999;
    }

    return finalComputedDamage;
}

void createDamageDisplayNumber(p_workArea parent, int damageTaken, sVec2_FP* offset, int param4)
{
    FunctionUnimplemented();
}

void s_battleDragon_UpdateSub4(s_battleDragon* pThis)
{
    if ((pThis->m1C4 & 8) == 0)
    {
        if (pThis->m8C.m50_flags & 0x80000)
        {
            // dragon took damage
            pThis->m8C.m50_flags &= ~0x80000;
            if (gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 0x400)
            {
                assert(0);
            }
            int damage = computeDragonDamage(pThis->m8C.m58);
            pThis->m1D6 = 0;
            pThis->m1C4 |= 0x10;

            if (!gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x17])
            {
                mainGameState.gameStats.m10_currentHP -= damage;
                pThis->m1D4_damageTaken += damage;
                if (mainGameState.gameStats.m10_currentHP < 1)
                {
                    // Dragon is dead!
                    playSystemSoundEffect(0xF);
                    FunctionUnimplemented();
                }
            }
            else
            {
                pThis->m1D4_damageTaken += damage;
                vdp2PrintStatus.m14_oldPalette = vdp2PrintStatus.m10_palette;
                vdp2PrintStatus.m10_palette = 0xb000;
                vdp2DebugPrintSetPosition(0x14, 0x1b);
                vdp2PrintfSmallFont("NO DEATH");
                vdp2PrintStatus.m10_palette = vdp2PrintStatus.m14_oldPalette;
            }

            gBattleManager->m10_battleOverlay->m4_battleEngine->m190[1] += damage;
            createDamageDisplayTask(pThis, damage, &pThis->m8_position, 0);

            fixedPoint impactForce;
            switch (pThis->m8C.m5E_impactForce)
            {
            case 1:
                pThis->m88 |= 8;
                playSystemSoundEffect(0xD);
                impactForce = 0x10000;
                break;
            case 2:
                pThis->m88 |= 8;
                playSystemSoundEffect(0xD);
                impactForce = 0x14000;
                break;
            case 3:
                pThis->m88 |= 0x10;
                playSystemSoundEffect(0xE);
                impactForce = 0x20000;
                break;
            default:
                assert(0);
            }
            
            // TODO: recheck this, this is pretty suspicious
            sVec3_FP impactVector = MTH_Mul(impactForce, pThis->m8C.m34_impactVector);
            sVec3_FP impactVector2;
            impactVector2[0] = MTH_Mul(impactForce, setDividend(impactVector[0], impactVector[0], pThis->m8C.m34_impactVector[0]));
            impactVector2[1] = MTH_Mul(impactForce, setDividend(impactVector[1], impactVector[1], pThis->m8C.m34_impactVector[1]));
            impactVector2[2] = MTH_Mul(impactForce, setDividend(impactVector[2], impactVector[2], pThis->m8C.m34_impactVector[2]));

            if (gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode == m8_playAttackCamera)
            {
                FunctionUnimplemented();
            }
            else
            {
                FunctionUnimplemented();
            }
            FunctionUnimplemented();
            incrementAnimationRootY(&gDragonState->m78_animData, impactVector[1]);
            incrementAnimationRootX(&gDragonState->m78_animData, impactVector[0]);
            incrementAnimationRootZ(&gDragonState->m78_animData, impactVector[2]);
        }

        if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m1000)
        {
            if (!(pThis->m1C4 & 0x10))
            {
                if (gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 0x400)
                {
                    pThis->m1D4_damageTaken = 0;
                }
            }
            else if (pThis->m1D4_damageTaken > -1)
            {
                pThis->m1C4 &= 0x10;
                sVec2_FP temp;
                temp[0] = 0;
                temp[1] = 0;
                createDamageDisplayNumber(pThis, pThis->m1D4_damageTaken, &temp, 1);
                pThis->m1D4_damageTaken = 0;
                if (mainGameState.gameStats.m10_currentHP < 1)
                {
                    playSystemSoundEffect(0xE);
                }
            }
        }
    }
}

void s_battleDragon_UpdateSub5(s_battleDragon* pThis)
{
    if (pThis->m1C4 & 8)
    {
        assert(0);
    }
}

void s_battleDragon_UpdateSub6()
{
    if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m4000000)
    {
        battleEngine_UpdateSub7Sub3();
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m4000000 = 0;
    }

    if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2000000)
    {
        sEnemyAttackCamera_updateSub2();
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2000000 = 0;
    }
}

static void s_battleDragon_Update(s_battleDragon* pThis)
{
    s_battleDragon_UpdateSub0(pThis);
    s_battleDragon_UpdateSub1(pThis);
    pThis->m84 = pThis->m88;
    pThis->m88 = 0;
    s_battleDragon_UpdateSub2(pThis);
    pThis->m14_rotation += pThis->m44_deltaRotation;
    s_battleDragon_UpdateSub3(pThis);
    if ((mainGameState.gameStats.m1_dragonLevel == 6) && (pThis->m24C_lightWingEffectTask != 0))
    {
        assert(0);
    }
    s_battleDragon_UpdateSub4(pThis);
    s_battleDragon_UpdateSub5(pThis);
    s_battleDragon_UpdateSub6();
}

static u32 s_battleDragon_UpdateAnimationState(s_battleDragon* pThis)
{
    bool bVar1 = false;
    int local_24 = 0;
    int cVar7;
    int currentIndex = 0;
    do
    {
        if (!(pThis->m244[currentIndex] & 1))
        {
            switch (currentIndex)
            {
            case 0:
                if (gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 0x100)
                {
                    bVar1 = true;
                    pThis->m210 = 0x1E;
                    pThis->m244[0] |= 1;
                    cVar7 = 0x1B;
                    playSystemSoundEffect(0x1B);
                }
                break;
            case 1:
                if (gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 0x200)
                {
                    bVar1 = true;
                    pThis->m210 = 0x1E;
                    pThis->m244[1] |= 1;
                    cVar7 = 0x1B;
                    playSystemSoundEffect(0x1B);
                }
                break;
            case 2:
                if (gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 0x80)
                {
                    bVar1 = true;
                    pThis->m210 = 0x1E;
                    pThis->m244[2] |= 1;
                    cVar7 = 0x1B;
                    playSystemSoundEffect(0x1B);
                }
                break;
            case 3:
                if (gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 0x8000)
                {
                    gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers &= ~0x8000;
                    int uVar6 = ((pThis->m244[3] & 0xF0) >> 4) + 1;
                    if (uVar6 < 4)
                    {
                        pThis->m210 = 0x28;
                    }
                    else
                    {
                        pThis->m210 = 0x50;
                    }
                    bVar1 = true;
                    cVar7 = uVar6 * 0x10;
                    pThis->m244[3] |= cVar7;
                }
                break;
            default:
                assert(0);
            }

            if (bVar1)
            {
                pThis->m208 = pThis->m210 << 16;
                pThis->m244[currentIndex] |= 2;
                pThis->m20C = 0;
            }
        }
        else
        {
            switch (currentIndex)
            {
            case 0:
                if (gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 0x100)
                {
                    pThis->m244[0] = 0;
                    pThis->m248[0] = 0;
                }
                break;
            case 1:
                if (gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 0x200)
                {
                    pThis->m244[1] = 0;
                    pThis->m248[1] = 0;
                }
                break;
            case 2:
                if (gBattleManager->m10_battleOverlay->m18_dragon->m1C0_statusModifiers & 0x80)
                {
                    pThis->m244[2] = 0;
                    pThis->m248[2] = 0;
                }
                break;
            default:
                assert(0);
            }
        }
        if ((pThis->m244[currentIndex] & 2) && (pThis->m248[currentIndex] == 0))
        {
            if (--pThis->m210 < 1)
            {
                if (pThis->m20C == 0)
                {
                    switch (currentIndex)
                    {
                    case 0:
                    case 1:
                    case 2:
                        pThis->m210 = 0x3C;
                        break;
                    case 3:
                        assert(0);
                    default:
                        break;
                    }

                    pThis->m208 = pThis->m210 << 16;
                    pThis->m20C = 1;
                }
                else
                {
                    pThis->m210 = 0;
                    if (currentIndex == 3)
                    {
                        pThis->m244[3] = 0;
                        pThis->m248[3] = 0;
                    }
                    else
                    {
                        pThis->m248[currentIndex] = 1;
                    }
                }
            }

            local_24 = 1;
            cVar7 = currentIndex;
            break;
        }

        currentIndex++;
    } while (currentIndex <= 3);

    if (local_24)
    {
        FunctionUnimplemented();
    }
    return local_24;
}

void s_battleDragon_getRiderRotation(sVec3_FP& outputRotation, const sMatrix4x3& inMatrix)
{
    outputRotation[0] = atan_FP(-inMatrix.matrix[6]);
    outputRotation[1] = atan2_FP(inMatrix.matrix[2], inMatrix.matrix[10]);
    outputRotation[2] = atan2_FP(inMatrix.matrix[4], inMatrix.matrix[5]);
}

void s_battleDragon_UpdateHotPoint(sVec3_FP& output, int index)
{
    sSaturnPtr pHotpointData = gCurrentBattleOverlay->getSaturnPtr(0x60ade04);
    sSaturnPtr pDragonHotpointData = readSaturnEA(pHotpointData + 4 * gDragonState->mC_dragonType);
    s8 data0 = readSaturnS8(pDragonHotpointData + index * 2 + 0);
    s8 data1 = readSaturnS8(pDragonHotpointData + index * 2 + 1);

    const sVec3_FP* pData = &gDragonState->m28_dragon3dModel.m44_hotpointData[data0][data1];
    if (pData)
    {
        transformAndAddVec(*pData, output, cameraProperties2.m28[0]);
    }
}

void s_battleDragon_UpdateHotPoints()
{
    for (int i = 0; i < 6; i++)
    {
        s_battleDragon_UpdateHotPoint(gBattleManager->m10_battleOverlay->m18_dragon->mFC_hotpoints[i], i);
    }
}

static void s_battleDragon_DrawRiderWeapon(s_battleDragon* pThis)
{
    if (mainGameState.gameStats.m1_dragonLevel == 8)
    {
        if (pRider1State->m18_3dModel.m44_hotpointData[3].size() == 0)
        {
            pThis->mF0 = pThis->m8_position;
        }
        else
        {
            transformAndAddVec(pRider1State->m18_3dModel.m44_hotpointData[3][0], pThis->mF0, cameraProperties2.m28[0]);
        }
    }
    else
    {
        if (pRider1State->m18_3dModel.m44_hotpointData[1].size() == 0)
        {
            pThis->mF0 = pThis->m8_position;
        }
        else
        {
            transformAndAddVec(pRider1State->m18_3dModel.m44_hotpointData[1][0], pThis->mF0, cameraProperties2.m28[0]);
        }
    }

    s32 iVar2 = isBattleAutoScroll();
    if ((iVar2 == 0) || ((pThis->m1C4 & 0x200) != 0))
    {
        iVar2 = 0;
    }
    else
    {
        iVar2 = readSaturnS32(gCurrentBattleOverlay->getSaturnPtr(0x060ae3bc) + gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant * 4);
    }

    sVec3_FP weaponRotation = pThis->m14_rotation + sVec3_FP(0, 0x8000000 + iVar2, 0);

    pushCurrentMatrix();
    translateCurrentMatrix(pThis->mF0);
    rotateCurrentMatrixYXZ(weaponRotation);
    addObjectToDrawList(pRider1State->m0_riderBundle->get3DModel(pRider1State->m14_weaponModelIndex));
    popMatrix();
}

static void s_battleDragon_Draw(s_battleDragon* pThis)
{
    if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m8_showingBattleResultScreen)
    {
        // This setup lights
        //setupLightColor(s_RGB8::fromVector(gBattleManager->m10_battleOverlay->m8_gridTask->m1F0).toU32());
        //battleEngine_UpdateSub7Sub1Sub0()
        FunctionUnimplemented();
    }

    u32 cVar3 = s_battleDragon_UpdateAnimationState(pThis);
    
    sVec3_FP rotation = pThis->m14_rotation + sVec3_FP(0, 0x8000000, 0);

    if ((gBattleManager->m2_currentBattleOverlayId == 0) || (gBattleManager->m2_currentBattleOverlayId == 0x10))
    {
        submitModelAndShadowModelToRendering(&gDragonState->m28_dragon3dModel, gDragonState->m14_modelIndex, 0, &pThis->m8_position, &rotation, 0);
    }
    else
    {
        if (mainGameState.gameStats.m1_dragonLevel == 8)
        {
            submitModelAndShadowModelToRendering(&gDragonState->m28_dragon3dModel, gDragonState->m14_modelIndex, 0, &pThis->m8_position, &rotation, 0);
        }
        else
        {
            submitModelAndShadowModelToRendering(&gDragonState->m28_dragon3dModel, gDragonState->m14_modelIndex, gDragonState->m18_shadowModelIndex, &pThis->m8_position, &rotation, gBattleManager->m10_battleOverlay->m4_battleEngine->m35C_cameraAltitudeMinMax[1]);
        }
    }

    sVec3_FP riderRotation;
    s_battleDragon_getRiderRotation(riderRotation, gDragonState->m28_dragon3dModel.m3C_boneMatrices[0]);
    s_battleDragon_UpdateHotPoints();

    if (cVar3)
    {
        assert(0);
    }

    riderRotation += pThis->m14_rotation + sVec3_FP(0, 0x8000000, 0);

    if (pRider1State->mC_riderType)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(gBattleManager->m10_battleOverlay->m18_dragon->mFC_hotpoints[0]);
        rotateCurrentMatrixYXZ(riderRotation);
        pRider1State->m18_3dModel.m18_drawFunction(&pRider1State->m18_3dModel);
        popMatrix();
    }
    s_battleDragon_DrawRiderWeapon(pThis);

    if (pRider2State->mC_riderType)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(gBattleManager->m10_battleOverlay->m18_dragon->mFC_hotpoints[1]);
        rotateCurrentMatrixYXZ(riderRotation);
        pRider2State->m18_3dModel.m18_drawFunction(&pRider2State->m18_3dModel);
        popMatrix();
    }

    if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m8_showingBattleResultScreen)
    {
        FunctionUnimplemented();
    }

    transformAndAddVecByCurrentMatrix(&pThis->m8_position, &pThis->m1A4);
    pThis->m8C.m40 = *pThis->m8C.m4_pPosition;
    pThis->m8C.m10_position = pThis->m8_position;
}

static void s_battleDragon_Delete(s_battleDragon* pThis)
{
    FunctionUnimplemented();
}

void battleEngine_createDragonTask(s_workAreaCopy* parent)
{
    static const s_battleDragon::TypedTaskDefinition definition = {
        &s_battleDragon_Init,
        &s_battleDragon_Update,
        &s_battleDragon_Draw,
        &s_battleDragon_Delete,
    };
    createSubTaskWithCopy<s_battleDragon>(parent, &definition);
}
