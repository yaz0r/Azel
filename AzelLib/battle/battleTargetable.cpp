#include "PDS.h"
#include "battleTargetable.h"
#include "battleManager.h"
#include "battleEngine.h"
#include "battleOverlay.h"
#include "battleDragon.h"
#include "battleDebug.h"
#include "battleGenericData.h"
#include "kernel/debug/trace.h"
#include "kernel/graphicalObject.h"
#include "kernel/vdp1Allocator.h"
#include "audio/systemSounds.h"

void battleTargetable_updatePosition(sBattleTargetable* pThis)
{
    transformAndAddVec(*pThis->m4_pPosition, pThis->m10_position, cameraProperties2.m28[0]);

    addTraceLog(*pThis->m4_pPosition, "targetablePositionSource");
    addTraceLog(pThis->m10_position, "targetablePosition");
}

sVec3_FP* getBattleTargetablePosition(sBattleTargetable& param1)
{
    return &param1.m10_position;
}

void applyDamageSub(sBattleTargetable& param_1, sVec3_FP& param_2)
{
    param_1.mC = &param_2;
    param_1.m28 = param_2;
    transformAndAddVecByCurrentMatrix(&param_1.m28, &param_1.m1C);
    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x14])
    {
        assert(0);
    }
}

void applyDamageToDragon(sBattleTargetable& pThis, s32 damageValue, sVec3_FP& param_3, s32 param_4, const sVec3_FP& param_5, s32 param_8)
{
    gBattleManager->m10_battleOverlay->m18_dragon->m1D6 += damageValue;
    pThis.m50_flags |= 0x80000;
    pThis.m58 = gBattleManager->m10_battleOverlay->m18_dragon->m1D6;

    applyDamageSub(pThis, param_3);

    pThis.m5E_impactForce = param_4;
    pThis.m54 = 0;
    pThis.m54 = param_8;

    pThis.m34_impactVector = MTH_Mul(FP_Div(0x1000, sqrt_F(MTH_Product3d_FP(param_5, param_5))), param_5);
}

fixedPoint sGunShotTask_UpdateSub2Sub0Sub0(sBattleTargetable* pThis, s16 param_1)
{
    return MTH_Mul(fixedPoint::fromInteger(param_1), setDividend(0x10000, fixedPoint::fromInteger(pThis->m60), 0xA0000)).toInteger();
}

void applyDamageToEnnemy(sBattleTargetable* pThis, s16 param_2, sVec3_FP* param_3, s8 param_4, const sVec3_FP& param_5, fixedPoint param_8)
{
    if (param_4 != 3)
    {
        param_2 = sGunShotTask_UpdateSub2Sub0Sub0(pThis, param_2);
    }

    pThis->m50_flags |= 0x80000;
    pThis->m58 = param_2;
    applyDamageSub(*pThis, *param_3);

    pThis->m5E_impactForce = param_4;
    pThis->m54 = 0;
    pThis->m54 = param_8;

    pThis->m34_impactVector = MTH_Mul(FP_Div(0x1000, sqrt_F(MTH_Product3d_FP(param_5, param_5))), param_5);
}

void deleteTargetable(sBattleTargetable* pThis)
{
    pThis->m50_flags |= 0x40000;
    if (!BattleEngineSub0_UpdateSub0())
    {
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2_needToSortEnemiesByDistanceFromDragon = 1;
    }
}

struct sBaldorSubTask : public s_workAreaTemplateWithCopy<sBaldorSubTask>
{
    fixedPoint m1C;
    sBattleTargetable* m84_pTargetable;
    sVec3_FP* m94;
    s32 m98;
    fixedPoint m9C;
    s16 mA6_cursorType;
    s16 mA4;
    s16 mA8_cursorFrameCounter;
    u16 mAC_vdp1Offset;
    s8 mAF;
    // size 0xB0
};

void getVdp1ScreenResolution(s16(&screenResolution)[4])
{
    screenResolution[0] = graphicEngineStatus.m405C.mC;
    screenResolution[1] = graphicEngineStatus.m405C.m8;
    screenResolution[2] = graphicEngineStatus.m405C.mE;
    screenResolution[3] = graphicEngineStatus.m405C.mA;
}

bool isSpriteVisible(sVec3_FP* pPosition, sVec2_S16& outputProjected)
{
    fixedPoint nearPlane;
    fixedPoint farPlane;
    getVdp1ClippingPlanes(nearPlane, farPlane);
    if (((*pPosition)[2] > nearPlane) && ((*pPosition)[2] < farPlane))
    {
        s16 xProj;
        s16 yProj;
        getVdp1ProjectionParams(&xProj, &yProj);
        outputProjected[0] = setDividend(xProj, (*pPosition)[0], (*pPosition)[2]);
        outputProjected[1] = setDividend(yProj, (*pPosition)[1], (*pPosition)[2]);

        s16 screenResolution[4];
        getVdp1ScreenResolution(screenResolution);

        if ((outputProjected[1] <= screenResolution[1]) && (outputProjected[1] >= screenResolution[3]) && (outputProjected[0] >= screenResolution[0]) && (outputProjected[0] <= screenResolution[2]))
        {
            return 1;
        }
    }

    return 0;
}

void sBaldorSubTask1_drawSub1(sVec2_S16& projected, sVec2_S16& pValue, sVec2_S16& pOutput)
{
    pOutput[0] = projected[0] - (pValue[0] / 2);
    pOutput[1] = projected[1] + (pValue[1] / 2);
}

void sBaldorSubTask1_drawSub0(sVec2_S16& projected, sVec2_S16& pValue, s32(&pOutput)[4])
{
    pOutput[0] = projected[0] - (pValue[0] / 2);
    pOutput[1] = projected[1] + (pValue[1] / 2);
    pOutput[2] = projected[0] - (pValue[0] / 2) + pValue[0];
    pOutput[3] = projected[1] + (pValue[1] / 2) - pValue[1];
}

static const std::array<sVec3_FP, 4> squareData = {
    {
        {0, 0x2800, 0},
        {0x2800, 0, 0},
        {0, -0x2800, 0},
        {-0x2800, 0, 0},
    }
};

// todo: kernel
void projectPoint(const sVec3_FP& inCoordinate, sVec2_FP& outCoordinate)
{
    s16 xProj;
    s16 yProj;
    getVdp1ProjectionParams(&xProj, &yProj);
    outCoordinate[0] = setDividend(xProj, inCoordinate[0], inCoordinate[2]);
    outCoordinate[1] = setDividend(yProj, inCoordinate[1], inCoordinate[2]);
}

// todo: kernel
// todo: this is missing the visibility check
s32 drawLineSquareProject(const std::array<sVec3_FP, 4>& coordinates, std::array<sVec2_FP, 4>& projectedCoordinates)
{
    for (int i = 0; i < 4; i++)
    {
        projectPoint(coordinates[i], projectedCoordinates[i]);
    }

    return 1;
}

// todo: kernel
void drawLineSquareVdp1(u16 param1, std::array < sVec2_FP, 4>& projectedCoordinates, u16 color, fixedPoint depth)
{
    s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
    vdp1WriteEA.m0_CMDCTRL = 0x1005; // command 0
    vdp1WriteEA.m4_CMDPMOD = 0x400 | param1; // CMDPMOD
    vdp1WriteEA.m6_CMDCOLR = color; // CMDCOLR
    vdp1WriteEA.mC_CMDXA = projectedCoordinates[0][0]; // CMDXA
    vdp1WriteEA.mE_CMDYA = -projectedCoordinates[0][1]; // CMDYA
    vdp1WriteEA.m10_CMDXB = projectedCoordinates[1][0]; // CMDXB
    vdp1WriteEA.m12_CMDYB = -projectedCoordinates[1][1]; // CMDYB
    vdp1WriteEA.m14_CMDXC = projectedCoordinates[2][0]; // CMDXC
    vdp1WriteEA.m16_CMDYC = -projectedCoordinates[2][1]; // CMDYC
    vdp1WriteEA.m18_CMDXD = projectedCoordinates[3][0]; // CMDXD
    vdp1WriteEA.m1A_CMDYD = -projectedCoordinates[3][1]; // CMDYD

    fixedPoint computedDepth = depth * graphicEngineStatus.m405C.m38_oneOverFarClip;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = computedDepth.getInteger();
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

    graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
    graphicEngineStatus.m14_vdp1Context[0].mC += 1;
}

// todo: kernel
void drawLineSquare(const std::array<sVec3_FP, 4>& coordinates, u16 color, s32 depth)
{
    std::array<sVec2_FP, 4> projectedCoordinates;

    if (drawLineSquareProject(coordinates, projectedCoordinates))
    {
        if (color < 0)
        {
            color = coordinates[0][2];
        }
        drawLineSquareVdp1(0xc0, projectedCoordinates, color, depth);
    }
}

void sBaldorSubTask0_draw1(sBaldorSubTask* pThis)
{
    std::array<sVec3_FP, 4> tempCoordinates;
    std::array<sVec3_FP, 4> outputCoordinates;

    for (int i = 3; i >= 0; i--)
    {
        fixedPoint iVar1 = MTH_Mul(squareData[i][0], getCos(pThis->m1C.getInteger()));
        fixedPoint iVar2 = MTH_Mul(squareData[i][1], getSin(pThis->m1C.getInteger()));
        tempCoordinates[i][0] = iVar1 - iVar2;

        iVar1 = MTH_Mul(squareData[i][0], getSin(pThis->m1C.getInteger()));
        iVar2 = MTH_Mul(squareData[i][1], getCos(pThis->m1C.getInteger()));
        tempCoordinates[i][1] = iVar1 + iVar2;
        tempCoordinates[i][2] = 0;

        tempCoordinates[i] = MTH_Mul(pThis->m9C, tempCoordinates[i]);
        outputCoordinates[i] = *pThis->m84_pTargetable->m4_pPosition + tempCoordinates[i];
    }

    drawLineSquare(outputCoordinates, -1, 0xA000);
}

void sBaldorSubTask0_draw(sBaldorSubTask* pThis)
{
    pThis->mA8_cursorFrameCounter = (pThis->mA8_cursorFrameCounter + 1) & 0xFF;

    sSaturnPtr spriteDef = g_BTL_GenericData->getSaturnPtr(0x60AB2DC);
    if (!(pThis->m84_pTargetable->m50_flags & 0x20000))
    {
        spriteDef = g_BTL_GenericData->getSaturnPtr(0x60ab2ec);
    }
    else if ((pThis->mA6_cursorType == 1) && (pThis->mA4 != 0))
    {
        spriteDef = g_BTL_GenericData->getSaturnPtr(0x60ab2e4);
    }

    sVec2_S16 coordinates;
    if (isSpriteVisible(pThis->m84_pTargetable->m4_pPosition, coordinates))
    {
        if (pThis->m84_pTargetable->m50_flags & 0x20000)
        {
            // display the normal reticule
            sVec2_S16 size;
            size[0] = 0x18;
            size[1] = 0x20;

            sVec2_S16 finalSize;
            sBaldorSubTask1_drawSub1(coordinates, size, finalSize);

            s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
            vdp1WriteEA.m0_CMDCTRL = 0x1000; // command 0
            vdp1WriteEA.m4_CMDPMOD = 0x148C; // CMDPMOD
            vdp1WriteEA.m6_CMDCOLR = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2EBC; // CMDCOLR
            vdp1WriteEA.m8_CMDSRCA = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0xE88; // CMDSRCA
            vdp1WriteEA.mA_CMDSIZE = 0x318; // CMDSIZE
            vdp1WriteEA.mC_CMDXA = finalSize[0]; // CMDXA
            vdp1WriteEA.mE_CMDYA = -finalSize[1]; // CMDYA

            // setup gradient
            (*graphicEngineStatus.m14_vdp1Context[0].m10)[0] = readSaturnU16(spriteDef + 0);
            (*graphicEngineStatus.m14_vdp1Context[0].m10)[1] = readSaturnU16(spriteDef + 2);
            (*graphicEngineStatus.m14_vdp1Context[0].m10)[2] = readSaturnU16(spriteDef + 4);
            (*graphicEngineStatus.m14_vdp1Context[0].m10)[3] = readSaturnU16(spriteDef + 6);
            vdp1WriteEA.m1C_CMDGRA = graphicEngineStatus.m14_vdp1Context[0].m10 - graphicEngineStatus.m14_vdp1Context[0].m14->begin();
            graphicEngineStatus.m14_vdp1Context[0].m10++;

            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

            graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
            graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
            graphicEngineStatus.m14_vdp1Context[0].mC += 1;
        }
        else
        {
            // display the Weak reticule
            int uVar3 = performModulo(8, pThis->mA8_cursorFrameCounter) & 0xFF;
            if (3 < uVar3)
            {
                spriteDef = g_BTL_GenericData->getSaturnPtr(0x60AB2DC);
            }

            sVec2_S16 size;
            size[0] = 0x18;
            size[1] = 0x20;

            sVec2_S16 finalSize;
            sBaldorSubTask1_drawSub1(coordinates, size, finalSize);

            s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
            vdp1WriteEA.m0_CMDCTRL = 0x1000; // command 0
            vdp1WriteEA.m4_CMDPMOD = 0x148C; // CMDPMOD
            vdp1WriteEA.m6_CMDCOLR = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2ED8; // CMDCOLR
            vdp1WriteEA.m8_CMDSRCA = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0xE28; // CMDSRCA
            vdp1WriteEA.mA_CMDSIZE = 0x320; // CMDSIZE
            vdp1WriteEA.mC_CMDXA = finalSize[0]; // CMDXA
            vdp1WriteEA.mE_CMDYA = -finalSize[1]; // CMDYA

            // setup gradient
            (*graphicEngineStatus.m14_vdp1Context[0].m10)[0] = readSaturnU16(spriteDef + 0);
            (*graphicEngineStatus.m14_vdp1Context[0].m10)[1] = readSaturnU16(spriteDef + 2);
            (*graphicEngineStatus.m14_vdp1Context[0].m10)[2] = readSaturnU16(spriteDef + 4);
            (*graphicEngineStatus.m14_vdp1Context[0].m10)[3] = readSaturnU16(spriteDef + 6);
            vdp1WriteEA.m1C_CMDGRA = graphicEngineStatus.m14_vdp1Context[0].m10 - graphicEngineStatus.m14_vdp1Context[0].m14->begin();
            graphicEngineStatus.m14_vdp1Context[0].m10++;

            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

            graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
            graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
            graphicEngineStatus.m14_vdp1Context[0].mC += 1;

            if (uVar3 < 4)
            {
                sVec2_S16 size;
                size[0] = 0x18;
                size[1] = 0x20;

                sVec2_S16 finalSize;
                sBaldorSubTask1_drawSub1(coordinates, size, finalSize);

                s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
                vdp1WriteEA.m0_CMDCTRL = 0x1000; // command 0
                vdp1WriteEA.m4_CMDPMOD = 0x148C; // CMDPMOD
                vdp1WriteEA.m6_CMDCOLR = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2ED8; // CMDCOLR
                vdp1WriteEA.m8_CMDSRCA = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0xE58; // CMDSRCA
                vdp1WriteEA.mA_CMDSIZE = 0x320; // CMDSIZE
                vdp1WriteEA.mC_CMDXA = finalSize[0]; // CMDXA
                vdp1WriteEA.mE_CMDYA = -finalSize[1]; // CMDYA

                // setup gradient
                (*graphicEngineStatus.m14_vdp1Context[0].m10)[0] = 0xFFFF;
                (*graphicEngineStatus.m14_vdp1Context[0].m10)[1] = 0xFFFF;
                (*graphicEngineStatus.m14_vdp1Context[0].m10)[2] = 0xFFFF;
                (*graphicEngineStatus.m14_vdp1Context[0].m10)[3] = 0xFFFF;
                vdp1WriteEA.m1C_CMDGRA = graphicEngineStatus.m14_vdp1Context[0].m10 - graphicEngineStatus.m14_vdp1Context[0].m14->begin();
                graphicEngineStatus.m14_vdp1Context[0].m10++;

                graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
                graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
                graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

                graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
                graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
                graphicEngineStatus.m14_vdp1Context[0].mC += 1;

            }
        }
    }
}

void sBaldorSubTask0_update(sBaldorSubTask* pThis);

void sBaldorSubTask0_update1(sBaldorSubTask* pThis)
{
    if (pThis->m84_pTargetable == nullptr)
    {
        pThis->m_DrawMethod = nullptr;
        pThis->getTask()->markFinished();
        return;
    }

    if ((pThis->m84_pTargetable->m50_flags & 0x40000) == 0)
    {
        if (pThis->m84_pTargetable->m5A < 1)
        {
            pThis->m84_pTargetable->m50_flags &= ~0x20000;
            pThis->m_UpdateMethod = sBaldorSubTask0_update;
            pThis->m_DrawMethod = nullptr;
            pThis->m98 &= ~1;
        }
        else
        {
            if (!(pThis->m84_pTargetable->m50_flags & 0x20000))
            {
                pThis->mAF = 0;
                pThis->mA4 = 0;
                pThis->m_UpdateMethod = sBaldorSubTask0_update;
                pThis->m_DrawMethod = nullptr;
            }
            else
            {
                if (!(pThis->m84_pTargetable->m50_flags & 0x100000))
                {
                    if (gBattleManager->m10_battleOverlay->m4_battleEngine->m396 == gBattleManager->m10_battleOverlay->m4_battleEngine->m394)
                    {
                        pThis->mA6_cursorType = 1;
                        if (pThis->mA4 == 0)
                        {
                            pThis->mA4 = 1;
                        }
                        else
                        {
                            pThis->mA4 = 0;
                        }
                    }
                    else
                    {
                        pThis->mA6_cursorType = 0;
                        pThis->mA4 = 0;
                    }


                    if (pThis->mAF == 0)
                    {
                        pThis->m9C = 0x80000;
                        pThis->m1C = randomNumber();
                        pThis->mAF++;
                        return;
                    }

                    if (pThis->mAF != 1)
                    {
                        return;
                    }

                    pThis->m1C += 0x1555555;

                    if (pThis->m9C > 0x10000)
                    {
                        pThis->m9C -= 0x10000;
                        return;
                    }

                    pThis->m9C = 0x10000;
                    pThis->mAF += 1;
                    pThis->m98 |= 1;
                    pThis->m_DrawMethod = sBaldorSubTask0_draw;
                }
                pThis->m84_pTargetable->m50_flags &= ~0x20000;
                pThis->mAF = 0;
                pThis->mA4 = 0;
                pThis->m_UpdateMethod = sBaldorSubTask0_update;
                pThis->m_DrawMethod = nullptr;
            }

            pThis->m98 &= ~1;
        }
    }
    else
    {
        pThis->m84_pTargetable->m50_flags &= ~0x20000;
        pThis->m84_pTargetable->m50_flags &= ~0x10000;
        pThis->getTask()->markFinished();
    }
}

void sBaldorSubTask0_update(sBaldorSubTask* pThis)
{
    if (pThis->m84_pTargetable == nullptr)
    {
        pThis->m_DrawMethod = nullptr;
        pThis->getTask()->markFinished();
        return;
    }

    if ((pThis->m84_pTargetable->m50_flags & 0x40000) == 0)
    {
        if ((pThis->m84_pTargetable->m5A > 0) && !(pThis->m84_pTargetable->m50_flags & 0x100000))
        {
            if (pThis->m84_pTargetable->m50_flags & 0x20000)
            {
                playSystemSoundEffect(10);
                pThis->mAF = 0;
                pThis->mA4 = 0x10;
                pThis->m_UpdateMethod = sBaldorSubTask0_update1;
                pThis->m_DrawMethod = sBaldorSubTask0_draw1;
            }
            else if (battleEngine_UpdateSub7Sub0Sub2Sub0()
                || (gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode == m0_shootEnemyWithGun)
                || (gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode == m6_dragonMoving)
                || (gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode == mB_enemyMovingDragon)
                || (gBattleManager->m10_battleOverlay->m4_battleEngine->m38C_battleMode == mC_commandMenuOpen)
                )
            {
                pThis->m_DrawMethod = sBaldorSubTask0_draw;
                pThis->mA4++;
                if (pThis->mA4 >= 9)
                {
                    pThis->mA4 = 0;
                }
            }
        }
        else
        {
            pThis->m_DrawMethod = nullptr;
        }
    }
    else
    {
        pThis->m84_pTargetable->m50_flags &= ~0x20000;
        pThis->m84_pTargetable->m50_flags &= ~0x10000;
        pThis->getTask()->markFinished();
    }
}

void sBaldorSubTask1_draw(sBaldorSubTask* pThis)
{
    pThis->mA4++;
    sVec2_S16 projected;
    if (!isSpriteVisible(pThis->m84_pTargetable->m4_pPosition, projected))
    {
        return;
    }

    s16 uVar4 = performModulo(8, pThis->mA4);
    if (uVar4 < 4)
    {
        pThis->mA8_cursorFrameCounter = 0;
    }
    else
    {
        pThis->mA8_cursorFrameCounter = 1;
    }

    sVec2_S16 local_28;
    sVec2_S16 local_30;
    s32 local_40[4];

    switch (pThis->mA6_cursorType)
    {
    case 0:
        local_28[0] = 0x20;
        local_28[1] = 0x20;

        if (pThis->m84_pTargetable->m60 > 10)
        {
            if (uVar4 < 2)
            {
                local_28[0] = 0x10;
                local_28[1] = 0x10;
            }
            else if (uVar4 < 5)
            {
                local_28[0] = 0x18;
                local_28[1] = 0x18;
            }
        }

        sBaldorSubTask1_drawSub0(projected, local_28, local_40);

        {
            s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
            vdp1WriteEA.m0_CMDCTRL = 0x1001; // command 0
            vdp1WriteEA.m4_CMDPMOD = 0x148C; // CMDPMOD
            vdp1WriteEA.m6_CMDCOLR = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2EBC; // CMDCOLR
            vdp1WriteEA.m8_CMDSRCA = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0xEB0; // CMDSRCA
            vdp1WriteEA.mA_CMDSIZE = 0x420; // CMDSIZE
            vdp1WriteEA.mC_CMDXA = local_40[0]; // CMDXA
            vdp1WriteEA.mE_CMDYA = -local_40[1]; // CMDYA
            vdp1WriteEA.m14_CMDXC = local_40[2]; // CMDXB
            vdp1WriteEA.m16_CMDYC = -local_40[3]; // CMDYB

                // setup gradient
            (*graphicEngineStatus.m14_vdp1Context[0].m10)[0] = 0xC3FF;
            (*graphicEngineStatus.m14_vdp1Context[0].m10)[1] = 0xC3FF;
            (*graphicEngineStatus.m14_vdp1Context[0].m10)[2] = 0xC3FF;
            (*graphicEngineStatus.m14_vdp1Context[0].m10)[3] = 0xC3FF;
            vdp1WriteEA.m1C_CMDGRA = graphicEngineStatus.m14_vdp1Context[0].m10 - graphicEngineStatus.m14_vdp1Context[0].m14->begin();
            graphicEngineStatus.m14_vdp1Context[0].m10++;

            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

            graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
            graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
            graphicEngineStatus.m14_vdp1Context[0].mC += 1;
        }

        if (pThis->mA8_cursorFrameCounter) // blinking part of the cursor
        {
            s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
            vdp1WriteEA.m0_CMDCTRL = 0x1001; // command 0
            vdp1WriteEA.m4_CMDPMOD = 0x148C; // CMDPMOD
            vdp1WriteEA.m6_CMDCOLR = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2EBC; // CMDCOLR
            vdp1WriteEA.m8_CMDSRCA = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0xEF0; // CMDSRCA
            vdp1WriteEA.mA_CMDSIZE = 0x420; // CMDSIZE
            vdp1WriteEA.mC_CMDXA = local_40[0]; // CMDXA
            vdp1WriteEA.mE_CMDYA = -local_40[1]; // CMDYA
            vdp1WriteEA.m14_CMDXC = local_40[2]; // CMDXB
            vdp1WriteEA.m16_CMDYC = -local_40[3]; // CMDYB

            // setup gradient
            (*graphicEngineStatus.m14_vdp1Context[0].m10)[0] = 0xC3FF;
            (*graphicEngineStatus.m14_vdp1Context[0].m10)[1] = 0xC3FF;
            (*graphicEngineStatus.m14_vdp1Context[0].m10)[2] = 0xC3FF;
            (*graphicEngineStatus.m14_vdp1Context[0].m10)[3] = 0xC3FF;
            vdp1WriteEA.m1C_CMDGRA = graphicEngineStatus.m14_vdp1Context[0].m10 - graphicEngineStatus.m14_vdp1Context[0].m14->begin();
            graphicEngineStatus.m14_vdp1Context[0].m10++;

            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

            graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
            graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
            graphicEngineStatus.m14_vdp1Context[0].mC += 1;
        }
        break;
    case 1: // anti laser
        local_28[0] = 0x30;
        local_28[1] = 0x20;

        sBaldorSubTask1_drawSub1(projected, local_28, local_30);

        {
            s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
            vdp1WriteEA.m0_CMDCTRL = 0x1000; // command 0
            vdp1WriteEA.m4_CMDPMOD = 0x1488; // CMDPMOD
            vdp1WriteEA.m6_CMDCOLR = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2F08; // CMDCOLR
            vdp1WriteEA.m8_CMDSRCA = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0xF30; // CMDSRCA
            vdp1WriteEA.mA_CMDSIZE = 0x620; // CMDSIZE
            vdp1WriteEA.mC_CMDXA = local_30.m_value[0]; // CMDXA
            vdp1WriteEA.mE_CMDYA = -local_30.m_value[1]; // CMDYA

            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

            graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
            graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
            graphicEngineStatus.m14_vdp1Context[0].mC += 1;
        }

        if (pThis->mA8_cursorFrameCounter) // blinking part of the cursor
        {
            local_28[0] = 0x30;
            local_28[1] = 0x20;

            sBaldorSubTask1_drawSub1(projected, local_28, local_30);

            s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
            vdp1WriteEA.m0_CMDCTRL = 0x1000; // command 0
            vdp1WriteEA.m4_CMDPMOD = 0x1488; // CMDPMOD
            vdp1WriteEA.m6_CMDCOLR = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2EE4; // CMDCOLR
            vdp1WriteEA.m8_CMDSRCA = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0xF90; // CMDSRCA
            vdp1WriteEA.mA_CMDSIZE = 0x620; // CMDSIZE
            vdp1WriteEA.mC_CMDXA = local_30.m_value[0]; // CMDXA
            vdp1WriteEA.mE_CMDYA = -local_30.m_value[1]; // CMDYA

            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

            graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
            graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
            graphicEngineStatus.m14_vdp1Context[0].mC += 1;
        }
        break;
    case 2: // anti-shot
        local_28[0] = 0x30;
        local_28[1] = 0x20;

        sBaldorSubTask1_drawSub1(projected, local_28, local_30);

        {
            s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
            vdp1WriteEA.m0_CMDCTRL = 0x1000; // command 0
            vdp1WriteEA.m4_CMDPMOD = 0x1488; // CMDPMOD
            vdp1WriteEA.m6_CMDCOLR = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2F08; // CMDCOLR
            vdp1WriteEA.m8_CMDSRCA = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0xFF0; // CMDSRCA
            vdp1WriteEA.mA_CMDSIZE = 0x620; // CMDSIZE
            vdp1WriteEA.mC_CMDXA = local_30.m_value[0]; // CMDXA
            vdp1WriteEA.mE_CMDYA = -local_30.m_value[1]; // CMDYA

            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

            graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
            graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
            graphicEngineStatus.m14_vdp1Context[0].mC += 1;
        }

        if (pThis->mA8_cursorFrameCounter) // blinking part of the cursor
        {
            local_28[0] = 0x30;
            local_28[1] = 0x20;

            sBaldorSubTask1_drawSub1(projected, local_28, local_30);

            s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
            vdp1WriteEA.m0_CMDCTRL = 0x1000; // command 0
            vdp1WriteEA.m4_CMDPMOD = 0x1488; // CMDPMOD
            vdp1WriteEA.m6_CMDCOLR = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2EE4; // CMDCOLR
            vdp1WriteEA.m8_CMDSRCA = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x1050; // CMDSRCA
            vdp1WriteEA.mA_CMDSIZE = 0x620; // CMDSIZE
            vdp1WriteEA.mC_CMDXA = local_30.m_value[0]; // CMDXA
            vdp1WriteEA.mE_CMDYA = -local_30.m_value[1]; // CMDYA

            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

            graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
            graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
            graphicEngineStatus.m14_vdp1Context[0].mC += 1;
        }
        break;
    default:
        assert(0);
    }
}

void sBaldorSubTask1_update(sBaldorSubTask* pThis)
{
    if (pThis->m84_pTargetable == nullptr)
    {
        pThis->m_DrawMethod = nullptr;
        pThis->getTask()->markFinished();
        return;
    }

    if (pThis->m84_pTargetable->m50_flags & 0x40000)
    {
        pThis->m_DrawMethod = nullptr;
        pThis->getTask()->markFinished();
        return;
    }

    if (pThis->m84_pTargetable->m50_flags & 0x100000)
    {
        pThis->m84_pTargetable->m50_flags &= ~0x200000;
    }

    if (!(pThis->m84_pTargetable->m50_flags & 0x200000))
    {
        pThis->m_DrawMethod = nullptr;
        return;
    }

    if (pThis->m84_pTargetable->m50_flags & 0x1000)
    {
        if (pThis->m84_pTargetable->m50_flags & 0x400)
        {
            pThis->mA6_cursorType = 0;
        }
        else
        {
            pThis->mA6_cursorType = 2;
        }
    }
    else if (pThis->m84_pTargetable->m50_flags & 0x400)
    {
        pThis->mA6_cursorType = 1;
    }
    else if (pThis->m84_pTargetable->m50_flags & 0x800)
    {
        if (pThis->m84_pTargetable->m50_flags & 0x200)
        {
            pThis->mA6_cursorType = 0;
        }
        else
        {
            pThis->mA6_cursorType = 2;
        }
    }
    else
    {
        if (pThis->m84_pTargetable->m50_flags & 0x200)
        {
            pThis->mA6_cursorType = 1;
        }
        else
        {
            pThis->mA6_cursorType = 0;
        }
    }

    pThis->m_DrawMethod = sBaldorSubTask1_draw;
}


void Baldor_initSub1Sub0(s_workAreaCopy* parent, sBattleTargetable* pTargetable)
{
    static const sBaldorSubTask::TypedTaskDefinition pTaskDefinition = {
        nullptr,
        sBaldorSubTask0_update,
        sBaldorSubTask0_draw,
        nullptr,
    };

    sBaldorSubTask* pNewTask = createSubTaskWithCopy<sBaldorSubTask>(parent, &pTaskDefinition);
    pNewTask->m84_pTargetable = pTargetable;
    pNewTask->m94 = pNewTask->m84_pTargetable->m4_pPosition;
    pNewTask->m84_pTargetable->m50_flags |= 0x10000;
    pNewTask->mAC_vdp1Offset = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory;
}

void Baldor_initSub1Sub1(s_workAreaCopy* parent, sBattleTargetable* pTargetable)
{
    static const sBaldorSubTask::TypedTaskDefinition pTaskDefinition = {
        nullptr,
        sBaldorSubTask1_update,
        sBaldorSubTask1_draw,
        nullptr,
    };

    sBaldorSubTask* pNewTask = createSubTaskWithCopy<sBaldorSubTask>(parent, &pTaskDefinition);
    pNewTask->m84_pTargetable = pTargetable;
    pNewTask->m94 = pNewTask->m84_pTargetable->m4_pPosition;
    pNewTask->mAC_vdp1Offset = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory;
}

void initTargetable(sBattleTargetable* param_1, s_battleDragon* param_2, sVec3_FP* param_3, s32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8)
{
    param_1->m0 = param_2;
    param_1->m4_pPosition = param_3;
    param_1->m4C = param_4;
    param_1->m40 = *param_3;
    battleTargetable_updatePosition(param_1);
    param_1->m8 = &param_1->m10_position;
    param_1->mC = 0;
    param_1->m1C.zeroize();
    param_1->m28.zeroize();
    param_1->m50_flags = param_5;
    param_1->m58 = 0;
    param_1->m5A = 0;
    param_1->m5E_impactForce = 0;
    param_1->m5F = param_6;
    param_1->m5C = param_7;
    param_1->m54 = 0;
    param_1->m60 = param_8;

    std::array<s_battleEnemy, 0x80>::iterator psVar4 = gBattleManager->m10_battleOverlay->m4_battleEngine->m49C_enemies.begin();

    if (gBattleManager->m10_battleOverlay->m4_battleEngine->m498_numEnemies < 0x80)
    {
        int iVar3 = 0;
        do {
            if (psVar4->m0_isActive == -1)
            {
                psVar4->m0_isActive = 0;
                psVar4->m4_targetable = param_1;
                psVar4->m8_distanceToDragonSquare = 0x7fffffff;
                if (param_1->m50_flags & 1)
                    return;

                Baldor_initSub1Sub0(gBattleManager->m10_battleOverlay->m4_battleEngine, param_1);
                Baldor_initSub1Sub1(gBattleManager->m10_battleOverlay->m4_battleEngine, param_1);

                gBattleManager->m10_battleOverlay->m4_battleEngine->m498_numEnemies++;
                return;
            }
            psVar4++;
        } while (++iVar3 < 0x80);
    }
}
