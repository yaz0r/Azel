#include "PDS.h"
#include "baldor.h"
#include "battle/battleFormation.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "battle/battleEngine.h"
#include "battle/battleDebug.h"
#include "battle/battleDamageDisplay.h"
#include "kernel/fileBundle.h"
#include "kernel/animation.h"
#include "kernel/debug/trace.h"
#include "kernel/vdp1Allocator.h"
#include "audio/systemSounds.h"

#include "battle/battleDragon.h" // todo: clean by moving s_battleDragon_8C to its own file
#include "mainMenuDebugTasks.h"
#include "town/town.h"

void createDamageSpriteEffect(npcFileDeleter* param1, sSaturnPtr param2, const sVec3_FP* param3, sVec3_FP* param4, sVec3_FP* param5, s32 param6, s32 param7, s32 param8); // todo: remove


void Baldor_initSub0Sub2(sBaldor* pThis, sFormationData* pFormationEntry)
{
    pThis->m34_formationEntry = pFormationEntry;

    pThis->m1C_translation.m0_current = &pFormationEntry->m0_translation.m0_current;
    pThis->m1C_translation.m4_target = &pFormationEntry->m0_translation.mC_target;
    pThis->m1C_translation.m8 = &pFormationEntry->m0_translation.m18;

    pThis->m28_rotation.m0_current = &pFormationEntry->m24_rotation.m0_current;
    pThis->m28_rotation.m4_target = &pFormationEntry->m24_rotation.mC_target;
    pThis->m28_rotation.m8 = &pFormationEntry->m24_rotation.m18;

    pFormationEntry->m48 = 0;
    pFormationEntry->m49 = 0;
}

p_workArea createBaldorSubTask0(sVec3_FP* arg0, s32 arg1, s8* arg2, s8 arg3)
{
    FunctionUnimplemented();
    return nullptr;
}

s_3dModel* Baldor_create3dModel(sBaldor* pThis, sSaturnPtr dataPtr, s32 arg)
{
    u8 fileBundleIndex = readSaturnS8(dataPtr);
    s_fileBundle* pFileBundle = dramAllocatorEnd[fileBundleIndex].mC_fileBundle->m0_fileBundle;
    sSaturnPtr animData = readSaturnEA(dataPtr + 8) + arg * 8;

    s_3dModel* pOutputModel = new s_3dModel;

    sSaturnPtr temp = readSaturnEA(dataPtr + 4);
    assert(temp.isNull());

    sModelHierarchy* pHierarchy = pFileBundle->getModelHierarchy(readSaturnU16(animData));


    sSaturnPtr hotSpotDataEA = animData + readSaturnU32(animData + 4);
    sHotpointBundle* pHotSpotsData = nullptr;
    assert(animData == hotSpotDataEA); // else we need to load the data

    init3DModelRawData(pThis, pOutputModel, 0, pFileBundle, readSaturnU16(animData), 0, pFileBundle->getStaticPose(readSaturnU16(animData + 2), pHierarchy->countNumberOfBones()), nullptr, pHotSpotsData);

    return pOutputModel;
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
    for (int i=0; i<4; i++)
    {
        projectPoint(coordinates[i], projectedCoordinates[i]);
    }

    return 1;
}

// todo: kernel
void drawLineSquareVdp1(u16 param1, std::array < sVec2_FP, 4>& projectedCoordinates, u16 color, fixedPoint depth)
{
    u32 vdp1WriteEA = graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
    setVdp1VramU16(vdp1WriteEA + 0x00, 0x1005); // command 0
    setVdp1VramU16(vdp1WriteEA + 0x04, 0x400 | param1); // CMDPMOD
    setVdp1VramU16(vdp1WriteEA + 0x06, color); // CMDCOLR
    setVdp1VramU16(vdp1WriteEA + 0x0C, projectedCoordinates[0][0]); // CMDXA
    setVdp1VramU16(vdp1WriteEA + 0x0E, -projectedCoordinates[0][1]); // CMDYA
    setVdp1VramU16(vdp1WriteEA + 0x10, projectedCoordinates[1][0]); // CMDXB
    setVdp1VramU16(vdp1WriteEA + 0x12, -projectedCoordinates[1][1]); // CMDYB
    setVdp1VramU16(vdp1WriteEA + 0x14, projectedCoordinates[2][0]); // CMDXC
    setVdp1VramU16(vdp1WriteEA + 0x16, -projectedCoordinates[2][1]); // CMDYC
    setVdp1VramU16(vdp1WriteEA + 0x18, projectedCoordinates[3][0]); // CMDXD
    setVdp1VramU16(vdp1WriteEA + 0x1A, -projectedCoordinates[3][1]); // CMDYD

    fixedPoint computedDepth = depth * graphicEngineStatus.m405C.m38;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = computedDepth.getInteger();
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = vdp1WriteEA >> 3;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

    graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
    graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA = vdp1WriteEA + 0x20;
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

    sSaturnPtr spriteDef = gCurrentBattleOverlay->getSaturnPtr(0x60AB2DC);
    if (!(pThis->m84_pTargetable->m50_flags & 0x20000))
    {
        spriteDef = gCurrentBattleOverlay->getSaturnPtr(0x60ab2ec);
    }
    else if ((pThis->mA6_cursorType == 1) && (pThis->mA4 != 0))
    {
        spriteDef = gCurrentBattleOverlay->getSaturnPtr(0x60ab2e4);
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

            u32 vdp1WriteEA = graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
            setVdp1VramU16(vdp1WriteEA + 0x00, 0x1000); // command 0
            setVdp1VramU16(vdp1WriteEA + 0x04, 0x148C); // CMDPMOD
            setVdp1VramU16(vdp1WriteEA + 0x06, dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2EBC); // CMDCOLR
            setVdp1VramU16(vdp1WriteEA + 0x08, dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0xE88); // CMDSRCA
            setVdp1VramU16(vdp1WriteEA + 0x0A, 0x318); // CMDSIZE
            setVdp1VramU16(vdp1WriteEA + 0x0C, finalSize[0]); // CMDXA
            setVdp1VramU16(vdp1WriteEA + 0x0E, -finalSize[1]); // CMDYA

            // setup gradient
            graphicEngineStatus.m14_vdp1Context[0].m10->m0[0] = readSaturnU16(spriteDef + 0);
            graphicEngineStatus.m14_vdp1Context[0].m10->m0[1] = readSaturnU16(spriteDef + 2);
            graphicEngineStatus.m14_vdp1Context[0].m10->m0[2] = readSaturnU16(spriteDef + 4);
            graphicEngineStatus.m14_vdp1Context[0].m10->m0[3] = readSaturnU16(spriteDef + 6);
            setVdp1VramU16(vdp1WriteEA + 0x1C, graphicEngineStatus.m14_vdp1Context[0].m10 - graphicEngineStatus.m14_vdp1Context[0].m14->begin());
            graphicEngineStatus.m14_vdp1Context[0].m10++;

            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = vdp1WriteEA >> 3;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

            graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
            graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA = vdp1WriteEA + 0x20;
            graphicEngineStatus.m14_vdp1Context[0].mC += 1;
        }
        else
        {
            // display the Weak reticule
            int uVar3 = performModulo(8, pThis->mA8_cursorFrameCounter) & 0xFF;
            if (3 < uVar3)
            {
                spriteDef = gCurrentBattleOverlay->getSaturnPtr(0x60AB2DC);
            }

            sVec2_S16 size;
            size[0] = 0x18;
            size[1] = 0x20;

            sVec2_S16 finalSize;
            sBaldorSubTask1_drawSub1(coordinates, size, finalSize);

            u32 vdp1WriteEA = graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
            setVdp1VramU16(vdp1WriteEA + 0x00, 0x1000); // command 0
            setVdp1VramU16(vdp1WriteEA + 0x04, 0x148C); // CMDPMOD
            setVdp1VramU16(vdp1WriteEA + 0x06, dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2ED8); // CMDCOLR
            setVdp1VramU16(vdp1WriteEA + 0x08, dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0xE28); // CMDSRCA
            setVdp1VramU16(vdp1WriteEA + 0x0A, 0x320); // CMDSIZE
            setVdp1VramU16(vdp1WriteEA + 0x0C, finalSize[0]); // CMDXA
            setVdp1VramU16(vdp1WriteEA + 0x0E, -finalSize[1]); // CMDYA

            // setup gradient
            graphicEngineStatus.m14_vdp1Context[0].m10->m0[0] = readSaturnU16(spriteDef + 0);
            graphicEngineStatus.m14_vdp1Context[0].m10->m0[1] = readSaturnU16(spriteDef + 2);
            graphicEngineStatus.m14_vdp1Context[0].m10->m0[2] = readSaturnU16(spriteDef + 4);
            graphicEngineStatus.m14_vdp1Context[0].m10->m0[3] = readSaturnU16(spriteDef + 6);
            setVdp1VramU16(vdp1WriteEA + 0x1C, graphicEngineStatus.m14_vdp1Context[0].m10 - graphicEngineStatus.m14_vdp1Context[0].m14->begin());
            graphicEngineStatus.m14_vdp1Context[0].m10++;

            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = vdp1WriteEA >> 3;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

            graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
            graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA = vdp1WriteEA + 0x20;
            graphicEngineStatus.m14_vdp1Context[0].mC += 1;

            if (uVar3 < 4)
            {
                sVec2_S16 size;
                size[0] = 0x18;
                size[1] = 0x20;

                sVec2_S16 finalSize;
                sBaldorSubTask1_drawSub1(coordinates, size, finalSize);

                u32 vdp1WriteEA = graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
                setVdp1VramU16(vdp1WriteEA + 0x00, 0x1000); // command 0
                setVdp1VramU16(vdp1WriteEA + 0x04, 0x148C); // CMDPMOD
                setVdp1VramU16(vdp1WriteEA + 0x06, dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2ED8); // CMDCOLR
                setVdp1VramU16(vdp1WriteEA + 0x08, dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0xE58); // CMDSRCA
                setVdp1VramU16(vdp1WriteEA + 0x0A, 0x320); // CMDSIZE
                setVdp1VramU16(vdp1WriteEA + 0x0C, finalSize[0]); // CMDXA
                setVdp1VramU16(vdp1WriteEA + 0x0E, -finalSize[1]); // CMDYA

                // setup gradient
                graphicEngineStatus.m14_vdp1Context[0].m10->m0[0] = 0xFFFF;
                graphicEngineStatus.m14_vdp1Context[0].m10->m0[1] = 0xFFFF;
                graphicEngineStatus.m14_vdp1Context[0].m10->m0[2] = 0xFFFF;
                graphicEngineStatus.m14_vdp1Context[0].m10->m0[3] = 0xFFFF;
                setVdp1VramU16(vdp1WriteEA + 0x1C, graphicEngineStatus.m14_vdp1Context[0].m10 - graphicEngineStatus.m14_vdp1Context[0].m14->begin());
                graphicEngineStatus.m14_vdp1Context[0].m10++;

                graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
                graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = vdp1WriteEA >> 3;
                graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

                graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
                graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA = vdp1WriteEA + 0x20;
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
    u32 vdp1WriteEA = graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
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

        setVdp1VramU16(vdp1WriteEA + 0x00, 0x1001); // command 0
        setVdp1VramU16(vdp1WriteEA + 0x04, 0x148C); // CMDPMOD
        setVdp1VramU16(vdp1WriteEA + 0x06, dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2EBC); // CMDCOLR
        setVdp1VramU16(vdp1WriteEA + 0x08, dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0xEB0); // CMDSRCA
        setVdp1VramU16(vdp1WriteEA + 0x0A, 0x420); // CMDSIZE
        setVdp1VramU16(vdp1WriteEA + 0x0C, local_40[0]); // CMDXA
        setVdp1VramU16(vdp1WriteEA + 0x0E, -local_40[1]); // CMDYA
        setVdp1VramU16(vdp1WriteEA + 0x14, local_40[2]); // CMDXB
        setVdp1VramU16(vdp1WriteEA + 0x16, -local_40[3]); // CMDYB

            // setup gradient
        graphicEngineStatus.m14_vdp1Context[0].m10->m0[0] = 0xC3FF;
        graphicEngineStatus.m14_vdp1Context[0].m10->m0[1] = 0xC3FF;
        graphicEngineStatus.m14_vdp1Context[0].m10->m0[2] = 0xC3FF;
        graphicEngineStatus.m14_vdp1Context[0].m10->m0[3] = 0xC3FF;
        setVdp1VramU16(vdp1WriteEA + 0x1C, graphicEngineStatus.m14_vdp1Context[0].m10 - graphicEngineStatus.m14_vdp1Context[0].m14->begin());
        graphicEngineStatus.m14_vdp1Context[0].m10++;

        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = vdp1WriteEA >> 3;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

        graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
        graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA = vdp1WriteEA + 0x20;
        graphicEngineStatus.m14_vdp1Context[0].mC += 1;

        vdp1WriteEA = graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        if (pThis->mA8_cursorFrameCounter) // blinking part of the cursor
        {
            setVdp1VramU16(vdp1WriteEA + 0x00, 0x1001); // command 0
            setVdp1VramU16(vdp1WriteEA + 0x04, 0x148C); // CMDPMOD
            setVdp1VramU16(vdp1WriteEA + 0x06, dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2EBC); // CMDCOLR
            setVdp1VramU16(vdp1WriteEA + 0x08, dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0xEF0); // CMDSRCA
            setVdp1VramU16(vdp1WriteEA + 0x0A, 0x420); // CMDSIZE
            setVdp1VramU16(vdp1WriteEA + 0x0C, local_40[0]); // CMDXA
            setVdp1VramU16(vdp1WriteEA + 0x0E, -local_40[1]); // CMDYA
            setVdp1VramU16(vdp1WriteEA + 0x14, local_40[2]); // CMDXB
            setVdp1VramU16(vdp1WriteEA + 0x16, -local_40[3]); // CMDYB

            // setup gradient
            graphicEngineStatus.m14_vdp1Context[0].m10->m0[0] = 0xC3FF;
            graphicEngineStatus.m14_vdp1Context[0].m10->m0[1] = 0xC3FF;
            graphicEngineStatus.m14_vdp1Context[0].m10->m0[2] = 0xC3FF;
            graphicEngineStatus.m14_vdp1Context[0].m10->m0[3] = 0xC3FF;
            setVdp1VramU16(vdp1WriteEA + 0x1C, graphicEngineStatus.m14_vdp1Context[0].m10 - graphicEngineStatus.m14_vdp1Context[0].m14->begin());
            graphicEngineStatus.m14_vdp1Context[0].m10++;

            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = vdp1WriteEA >> 3;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

            graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
            graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA = vdp1WriteEA + 0x20;
            graphicEngineStatus.m14_vdp1Context[0].mC += 1;

            vdp1WriteEA = graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        }
        break;
    case 1: // anti laser
        local_28[0] = 0x30;
        local_28[1] = 0x20;

        sBaldorSubTask1_drawSub1(projected, local_28, local_30);

        setVdp1VramU16(vdp1WriteEA + 0x00, 0x1000); // command 0
        setVdp1VramU16(vdp1WriteEA + 0x04, 0x1488); // CMDPMOD
        setVdp1VramU16(vdp1WriteEA + 0x06, dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2F08); // CMDCOLR
        setVdp1VramU16(vdp1WriteEA + 0x08, dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0xF30); // CMDSRCA
        setVdp1VramU16(vdp1WriteEA + 0x0A, 0x620); // CMDSIZE
        setVdp1VramU16(vdp1WriteEA + 0x0C, local_30.m_value[0]); // CMDXA
        setVdp1VramU16(vdp1WriteEA + 0x0E, -local_30.m_value[1]); // CMDYA

        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = vdp1WriteEA >> 3;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

        graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
        graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA = vdp1WriteEA + 0x20;
        graphicEngineStatus.m14_vdp1Context[0].mC += 1;

        vdp1WriteEA = graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        if (pThis->mA8_cursorFrameCounter) // blinking part of the cursor
        {
            local_28[0] = 0x30;
            local_28[1] = 0x20;

            sBaldorSubTask1_drawSub1(projected, local_28, local_30);

            setVdp1VramU16(vdp1WriteEA + 0x00, 0x1000); // command 0
            setVdp1VramU16(vdp1WriteEA + 0x04, 0x1488); // CMDPMOD
            setVdp1VramU16(vdp1WriteEA + 0x06, dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2EE4); // CMDCOLR
            setVdp1VramU16(vdp1WriteEA + 0x08, dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0xF90); // CMDSRCA
            setVdp1VramU16(vdp1WriteEA + 0x0A, 0x620); // CMDSIZE
            setVdp1VramU16(vdp1WriteEA + 0x0C, local_30.m_value[0]); // CMDXA
            setVdp1VramU16(vdp1WriteEA + 0x0E, -local_30.m_value[1]); // CMDYA

            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = vdp1WriteEA >> 3;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

            graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
            graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA = vdp1WriteEA + 0x20;
            graphicEngineStatus.m14_vdp1Context[0].mC += 1;

            vdp1WriteEA = graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        }
        break;
    case 2: // anti-shot
        local_28[0] = 0x30;
        local_28[1] = 0x20;

        sBaldorSubTask1_drawSub1(projected, local_28, local_30);

        setVdp1VramU16(vdp1WriteEA + 0x00, 0x1000); // command 0
        setVdp1VramU16(vdp1WriteEA + 0x04, 0x1488); // CMDPMOD
        setVdp1VramU16(vdp1WriteEA + 0x06, dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2F08); // CMDCOLR
        setVdp1VramU16(vdp1WriteEA + 0x08, dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0xFF0); // CMDSRCA
        setVdp1VramU16(vdp1WriteEA + 0x0A, 0x620); // CMDSIZE
        setVdp1VramU16(vdp1WriteEA + 0x0C, local_30.m_value[0]); // CMDXA
        setVdp1VramU16(vdp1WriteEA + 0x0E, -local_30.m_value[1]); // CMDYA

        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = vdp1WriteEA >> 3;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

        graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
        graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA = vdp1WriteEA + 0x20;
        graphicEngineStatus.m14_vdp1Context[0].mC += 1;

        vdp1WriteEA = graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        if (pThis->mA8_cursorFrameCounter) // blinking part of the cursor
        {
            local_28[0] = 0x30;
            local_28[1] = 0x20;

            sBaldorSubTask1_drawSub1(projected, local_28, local_30);

            setVdp1VramU16(vdp1WriteEA + 0x00, 0x1000); // command 0
            setVdp1VramU16(vdp1WriteEA + 0x04, 0x1488); // CMDPMOD
            setVdp1VramU16(vdp1WriteEA + 0x06, dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x2EE4); // CMDCOLR
            setVdp1VramU16(vdp1WriteEA + 0x08, dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory + 0x1050); // CMDSRCA
            setVdp1VramU16(vdp1WriteEA + 0x0A, 0x620); // CMDSIZE
            setVdp1VramU16(vdp1WriteEA + 0x0C, local_30.m_value[0]); // CMDXA
            setVdp1VramU16(vdp1WriteEA + 0x0E, -local_30.m_value[1]); // CMDYA

            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = vdp1WriteEA >> 3;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

            graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
            graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA = vdp1WriteEA + 0x20;
            graphicEngineStatus.m14_vdp1Context[0].mC += 1;

            vdp1WriteEA = graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
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

void Baldor_initSub1(sBattleTargetable* param_1, s_battleDragon* param_2, sVec3_FP* param_3, s32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8)
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

void Baldor_initSub0Sub1(p_workArea pThis, s_3dModel* pModel, s16* param3, std::vector<sBattleTargetable>& param4, std::vector<sVec3_FP>& param5)
{
    if(pModel->m40 == nullptr)
    {
        *param3 = 0;
        param4.clear();
        param5.clear();
    }
    else
    {
        int count = 0;
        for (int i=0; i<pModel->m12_numBones; i++)
        {
            count += (*pModel->m40)[i].m4_count;
        }

        *param3 = count;
        param4.resize(count);
        param5.resize(count);

        int currentEntryIndex = 0;

        for (int i=0; i<pModel->m12_numBones; i++)
        {
            if (pModel->m44_hotpointData[i].size())
            {
                for (int j = 0; j < (*pModel->m40)[i].m4_count; j++)
                {
                    param4[currentEntryIndex].m4_pPosition = nullptr;

                    s_hotpoinEntry& puVar1 = (*pModel->m40)[i].m0[i];

                    Baldor_initSub1(&param4[currentEntryIndex], nullptr, &param5[currentEntryIndex], puVar1.m10, puVar1.m0, 0, 0, 10);

                    currentEntryIndex++;
                }
            }
        }
    }
}

void Baldor_initSub0(sBaldor* pThis, sSaturnPtr dataPtr, sFormationData* pFormationEntry, s32 arg)
{
    pThis->m3C_dataPtr = dataPtr;
    if (readSaturnS8(dataPtr + 2) == 0)
    {
        pThis->m38_3dModel = nullptr;
        pThis->mC_numTargetables = 0;
    }
    else
    {
        pThis->m38_3dModel = Baldor_create3dModel(pThis, dataPtr, 0);
        Baldor_initSub0Sub1(pThis, pThis->m38_3dModel, &pThis->mC_numTargetables, pThis->m14_targetable, pThis->m18_position);
    }

    Baldor_initSub0Sub2(pThis, pFormationEntry);
    pThis->m40 = createBaldorSubTask0(pThis->m1C_translation.m0_current, 0, &pThis->m10_HP, readSaturnS8(pThis->m3C_dataPtr + 1));
    if (-1 < arg)
    {
        u8 bundleIdx = readSaturnS8(dataPtr);
        u32 offset = readSaturnU16(readSaturnEA(dataPtr + 0xC) + arg * 2);

        sAnimationData* pAnimation = dramAllocatorEnd[bundleIdx].mC_fileBundle->m0_fileBundle->getAnimation(offset);

        initAnimation(pThis->m38_3dModel, pAnimation);

        int animationSteps = randomNumber() & 0x1F;
        while (animationSteps)
        {
            stepAnimation(pThis->m38_3dModel);
            animationSteps--;
        }
    }
}

void monsterPart_defaultUpdate(sBaldor_68_30*, const sVec3_FP*, const sVec3_FP*, const sVec3_FP*)
{
    assert(0);
}

void monsterPart_defaultDraw(sBaldor*, sBaldor_68_30*)
{
    assert(0);
}

void monsterPart_defaultDelete()
{
    assert(0);
}

void baldorPart_update(sBaldor_68_30* pThis, const sVec3_FP* pTranslation, const sVec3_FP* pRotation, const sVec3_FP* param4)
{
    pThis->m34 += MTH_Mul((*param4 - pThis->m1C).normalized(), pThis->m44);

    pThis->m34 -= MTH_Mul(pThis->m50, pThis->m28);

    pThis->m28 += pThis->m34;
    pThis->m1C += pThis->m28;

    pThis->m34.zeroize();

    sMatrix4x3 pTemp;
    initMatrixToIdentity(&pTemp);
    translateMatrix(*pTranslation, &pTemp);
    rotateMatrixZYX(pRotation, &pTemp);
    translateMatrix(pThis->m10_translation, &pTemp);

    pThis->m4 = pTemp.getTranslation();

    addTraceLog(pThis->m10_translation, "pThis->m10");
    addTraceLog(pThis->m1C, "pThis->m1C");
    addTraceLog(pThis->m28, "pThis->m28");
    addTraceLog(pThis->m34, "pThis->m34");
    addTraceLog(pThis->m50, "pThis->m50");
    addTraceLog(pThis->m4, "pThis->m4");

    if (pThis->m0_child)
    {
        baldorPart_update(pThis->m0_child, &pThis->m4, &pThis->m1C, &pThis->m1C);
    }
}

void baldorPart_draw(sBaldor* pBaltor, sBaldor_68_30* pBaltorPart)
{
    while (1)
    {
        if (pBaltorPart->m40 > 0)
        {
            pushCurrentMatrix();
            translateCurrentMatrix(pBaltorPart->m4);
            rotateCurrentMatrixZYX(pBaltorPart->m1C);
            addObjectToDrawList(pBaltor->m0_fileBundle->get3DModel(pBaltorPart->m40));
            popMatrix();
        }
        if (pBaltorPart->m0_child == nullptr)
            break;
        pBaltorPart = pBaltorPart->m0_child;
    }
}

void baldorPart_delete()
{
    assert(0);
}

void Baldor_initSub2Sub0(sBaldor_68* pData)
{
    pData->m0_translation.zeroize();
    pData->mC_rotation.zeroize();
    pData->m18.zeroize();
    pData->m24_update = monsterPart_defaultUpdate;
    pData->m28_draw = monsterPart_defaultDraw;
    pData->m2C_delete = monsterPart_defaultDelete;
    pData->m30.resize(0);
}

void Baldor_initSub2Sub1(sBaldor_68_30* pEntry, sBaldor_68_30* pNextEntry)
{
    pEntry->m0_child = pNextEntry;
    pEntry->m4.zeroize();
    pEntry->m10_translation.zeroize();
    pEntry->m1C.zeroize();
    pEntry->m28.zeroize();
    pEntry->m34.zeroize();
    pEntry->m40 = 0;
    pEntry->m44.zeroize();
    pEntry->m50 = 0;
}

sBaldor_68* Baldor_initSub2(p_workArea parent, int numEntries)
{
    sBaldor_68* pNewData = new sBaldor_68;
    Baldor_initSub2Sub0(pNewData);

    pNewData->m30.resize(numEntries);

    Baldor_initSub2Sub1(&pNewData->m30[numEntries - 1], nullptr);
    int iVar1 = numEntries - 1;
    while (iVar1 != 0)
    {
        Baldor_initSub2Sub1(&pNewData->m30[iVar1 - 1], &pNewData->m30[iVar1]);
        iVar1--;
    }

    return pNewData;
}

void Baldor_initSub3Sub0(sBaldor_68_30* dest, sSaturnPtr source)
{
    dest->m40 = readSaturnS16(source);
    dest->m44 = readSaturnVec3(source + 4);
    dest->m50 = readSaturnS32(source + 0x10);
}

void Baldor_initSub3(sBaldor_68* pThis, int arg2, sSaturnPtr arg3)
{
    if (arg2 == 1)
    {
        pThis->m24_update = baldorPart_update;
        pThis->m28_draw = baldorPart_draw;
        pThis->m2C_delete = baldorPart_delete;
    }

    if (!arg3.isNull())
    {
        sBaldor_68_30* piVar1 = &pThis->m30[0];
        do
        {
            Baldor_initSub3Sub0(piVar1, arg3);
            piVar1 = piVar1->m0_child;
            arg3 += 0x14;
        } while (piVar1);
    }
}

void Baldor_init(sBaldor* pThis, sFormationData* pFormationEntry)
{
    sSaturnPtr puVar7;
    if ((gBattleManager->m6_subBattleId == 8) || (gBattleManager->m6_subBattleId == 9)) // middle boss  (with queen)
    {
        puVar7 = gCurrentBattleOverlay->getSaturnPtr(0x60a75f0);
    }
    else
    {
        puVar7 = gCurrentBattleOverlay->getSaturnPtr(0x60a73a0);
    }

    Baldor_initSub0(pThis, puVar7, pFormationEntry, 0);

    pThis->m14_targetable.resize(4);
    pThis->m18_position.resize(4);
    pThis->mC_numTargetables = 4;

    for (int i = 0; i < 4; i++)
    {
        u32 ivar2;
        if (i == 0)
        {
            ivar2 = 0xf0000000;
        }
        else
        {
            ivar2 = 0xf0000002;
        }

        Baldor_initSub1(&pThis->m14_targetable[i], nullptr, &pThis->m18_position[i], 0x1000, ivar2, 0, 0, 10);
    }

    *pThis->m28_rotation.m0_current = *pThis->m28_rotation.m4_target;

    pThis->m68 = Baldor_initSub2(pThis, 6);

    if ((gBattleManager->m6_subBattleId == 8) || (gBattleManager->m6_subBattleId == 9))
    {
        assert(0);
    }
    else
    {
        Baldor_initSub3(pThis->m68, 1, gCurrentBattleOverlay->getSaturnPtr(0x60a7e5c));
        sSaturnPtr pDataSource = gCurrentBattleOverlay->getSaturnPtr(0x60a7f4c);

        for (int i = 0; i < 6; i++)
        {
            sBaldor_68_30& dest = pThis->m68->m30[i];

            dest.m10_translation = readSaturnVec3(pDataSource + 0xC * i);
            dest.m1C[1] = (*pThis->m28_rotation.m4_target)[1];
        }
    }

    pThis->m6C[0] = randomNumber();
    pThis->m6C[1] = randomNumber();
    pThis->m6C[2] = randomNumber();
}

s32 Baldor_updateSub0Sub0(p_workArea pThis, std::vector<sBattleTargetable>& param2, s16 entriesToParse, s16& param4)
{
    int uVar4 = 0;
    int sVar3 = 0;
    if(entriesToParse > 0)
    {
        std::vector<sBattleTargetable>::iterator uVar5 = param2.begin();
        int sVar1 = 0;
        while (uVar5 - param2.begin() < entriesToParse)
        {
            sVar3 = sVar1;
            if (uVar5->m50_flags & 0x80000)
            {
                uVar4 = 1;
                sVar3 += uVar5->m58;
            }
            uVar5++;
            sVar1 = sVar3;
        }
    }
    param4 = sVar3;
    return uVar4;
}

sVec3_FP* Baldor_updateSub0Sub1Sub0(std::vector<sBattleTargetable>& param1, int param2)
{
    sVec3_FP* iVar3 = nullptr;
    for (int i=0; i<param2; i++)
    {
        sBattleTargetable& value = param1[i];
        if (value.m50_flags & 0x80000)
        {
            iVar3 = &value.m34_impactVector;
        }
    }
    return iVar3;
}

void Baldor_updateSub0Sub1(sBaldor* pThis)
{
    pThis->m5C_rotationDelta[0] += (randomNumber() & 0x3fffff) - 0x1fffff;
    pThis->m5C_rotationDelta[1] += (randomNumber() & 0x3fffff) - 0x1fffff;
    pThis->m5C_rotationDelta[2] += (randomNumber() & 0x7fffff) - 0x3fffff;

    sVec3_FP* piVar2 = Baldor_updateSub0Sub1Sub0(pThis->m14_targetable, pThis->mC_numTargetables);
    if (piVar2)
    {
        pThis->m50_translationDelta += *piVar2;
    }

    if (isTraceEnabled())
    {
        addTraceLog(pThis->m50_translationDelta, "BaldorTranslationDelta");
    }
}

void Baldor_updateSub0Sub2Sub2(sVec3_FP* param1, sVec3_FP* param2, s32 param3, s8 param4)
{
    int iVar2;

    switch (param4)
    {
    case 0:
        if (gBattleManager->m2_currentBattleOverlayId == 1)
        {
            iVar2 = 3;
        }
        else
        {
            iVar2 = 2;
        }
        break;
    case 1:
    case 2:
        if (gBattleManager->m2_currentBattleOverlayId == 1)
        {
            iVar2 = 5;
        }
        else
        {
            iVar2 = 4;
        }
        break;
    default:
        if (gBattleManager->m2_currentBattleOverlayId == 1)
        {
            iVar2 = 8;
        }
        else
        {
            iVar2 = 7;
        }
        break;
    }

    createDamageSpriteEffect(dramAllocatorEnd[0].mC_fileBundle, readSaturnEA(gCurrentBattleOverlay->getSaturnPtr(0x060abef4) + iVar2 * 4), param1, param2, 0, param3, 0, 0);
}

void Baldor_updateSub0Sub2(sBaldor* pThis, std::vector<sBattleTargetable>& param2, int param3, int param4, p_workArea param5)
{
    for (int i = 0; i < param3; i++)
    {
        sBattleTargetable& value = param2[i];
        if (value.m50_flags & 0x80000)
        {
            if (param4)
            {
                assert(0);
            }

            value.m50_flags &= ~0x80000;
            value.m50_flags &= ~0x20000;

            Baldor_updateSub0Sub2Sub2(getBattleTargetablePosition(value), nullptr, 0x30000, 1);
        }
    }

    if (param5)
    {
        assert(0);
    }
}

void Baldor_updateSub0(sBaldor* pThis)
{
    s16 damageValue;

    if (!(pThis->m34_formationEntry->m48 & 4))
    {
        if (Baldor_updateSub0Sub0(pThis, pThis->m14_targetable, pThis->mC_numTargetables, damageValue))
        {
            pThis->mE_damageValue += damageValue;
            pThis->m12 = 1;
            pThis->mB |= 8;
            Baldor_updateSub0Sub1(pThis);
            pThis->m10_HP -= damageValue;
            if (pThis->m10_HP < 1)
            {
                Baldor_updateSub0Sub2(pThis, pThis->m14_targetable, pThis->mC_numTargetables, 0, pThis->m40);
                pThis->m34_formationEntry->m48 |= 4;
                createDamageDisplayTask(pThis, pThis->mE_damageValue, pThis->m1C_translation.m0_current, 1);
                playSystemSoundEffect(0x66);
                pThis->m8_mode = 0xB;
                pThis->m9_attackStatus = 0;
            }
            else
            {
                Baldor_updateSub0Sub2(pThis, pThis->m14_targetable, pThis->mC_numTargetables, 1, pThis->m40);
                playSystemSoundEffect(0x65);
            }
        }

        if (pThis->m12 && (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m1000))
        {
            createDamageDisplayTask(pThis, pThis->mE_damageValue, pThis->m1C_translation.m0_current, 1);
            pThis->mE_damageValue = 0;
            pThis->m12 = 0;
        }
    }
}

void Baldor_updateSub1(sVec3_FP* pCurrent, sVec3_FP* pDelta, sVec3_FP* pTarget, s32 pDeltaFactor, s32 pDistanceToTargetFactor, s8 translationOrRotation)
{
    switch(translationOrRotation)
    {
    case 0:
        if (pDelta == nullptr)
        {
            assert(0);
        }
        else
        {
            *pDelta -= MTH_Mul(pDeltaFactor, *pDelta);
            *pDelta += MTH_Mul(pDistanceToTargetFactor, *pTarget - *pCurrent);
        }
        *pCurrent += *pDelta;
        break;
    case 1:
        *pDelta = (*pDelta - MTH_Mul(pDeltaFactor, *pDelta)).normalized();
        *pDelta = (*pDelta + MTH_Mul(pDistanceToTargetFactor, (*pTarget - *pCurrent).normalized())).normalized();
        *pCurrent = (*pCurrent + *pDelta).normalized();
        break;
    default:
        assert(0);
    }
}

struct sBaldorAttack : public s_workAreaTemplate<sBaldorAttack>
{
    sBaldor_68* m0;
    s16 m4_delay;
    s16 m6_baldorPartEmittingAttack;
    s16 m8_numAttackRotation;
    u8 mA_state;
    sBaldor* mC;

    // size: 0x10
};

void BaldorAttack_createAttackModel(sVec3_FP* partPosition, sVec3_FP* target, sSaturnPtr param3)
{
    FunctionUnimplemented();
}

void BaldorAttack_update(sBaldorAttack* pThis)
{
    switch (pThis->mA_state)
    {
    case 0:
        if (--pThis->m4_delay < 0)
        {
            pThis->m4_delay = 3;
            sVec3_FP local1C = gBattleManager->m10_battleOverlay->m18_dragon->m8_position;
            local1C[2] += pThis->m6_baldorPartEmittingAttack * 0x2000;

            BaldorAttack_createAttackModel(&pThis->m0->m30[pThis->m6_baldorPartEmittingAttack].m4, &local1C, gCurrentBattleOverlay->getSaturnPtr(0x60a8018));

            if (++pThis->m6_baldorPartEmittingAttack > 5)
            {
                pThis->m6_baldorPartEmittingAttack = 0;
                if (++pThis->m8_numAttackRotation > 0)
                {
                    pThis->m4_delay = 9;
                    pThis->mA_state++;
                }
            }
        }
        break;
    case 1:
        if (--pThis->m4_delay < 0)
        {
            pThis->getTask()->markFinished();
        }
        break;
    default:
        assert(0);
    }
}

p_workArea Baldor_createAttackTask(sBaldor* pThis)
{
    sBaldorAttack* pNewTask = createSubTaskFromFunction<sBaldorAttack>(pThis, &BaldorAttack_update);
    pNewTask->m0 = pThis->m68;
    pNewTask->mC = pThis;

    return pNewTask;
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

void applyDamage(sBattleTargetable& param_1, s32 damageValue, sVec3_FP& param_3, s32 param_4, const sVec3_FP& param_5, s32 param_8)
{
    gBattleManager->m10_battleOverlay->m18_dragon->m1D6 += damageValue;
    param_1.m50_flags |= 0x80000;
    param_1.m58 = gBattleManager->m10_battleOverlay->m18_dragon->m1D6;

    applyDamageSub(param_1, param_3);

    param_1.m5E_impactForce = param_4;
    param_1.m54 = 0;
    param_1.m54 |= param_8;

    param_1.m34_impactVector = MTH_Mul(FP_Div(0x1000, sqrt_F(MTH_Product3d_FP(param_5, param_5))), param_5);
}

// front attack (evil bite)
void Baldor_update_mode1(sBaldor* pThis)
{
    switch (pThis->m9_attackStatus)
    {
    case 0:
        if (pThis->m34_formationEntry->m49 != 1)
        {
            return;
        }

        pThis->m34_formationEntry->m48 &= ~1;
        pThis->m34_formationEntry->m48 |= 2;
        pThis->m9_attackStatus++;
        break;
    case 1:
        pThis->m44_translationTarget = gBattleManager->m10_battleOverlay->m18_dragon->m8_position - gBattleManager->m10_battleOverlay->m4_battleEngine->mC_battleCenter;
        switch (gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant)
        {
        case 0:
            pThis->m44_translationTarget[2] -= 0x3000;
            break;
        case 1:
            pThis->m44_translationTarget[0] -= 0x3000;
            break;
        case 2:
            pThis->m44_translationTarget[2] += 0x3000;
            break;
        case 3:
            pThis->m44_translationTarget[0] += 0x3000;
            break;
        default:
            assert(0);
        }

        initAnimation(pThis->m38_3dModel, dramAllocatorEnd[readSaturnU8(pThis->m3C_dataPtr)].mC_fileBundle->m0_fileBundle->getAnimation(readSaturnU16(readSaturnEA(pThis->m3C_dataPtr + 0xC) + 2)));
        pThis->m9_attackStatus = 2;
        break;
    case 2:
        pThis->m44_translationTarget = gBattleManager->m10_battleOverlay->m18_dragon->m8_position - gBattleManager->m10_battleOverlay->m4_battleEngine->mC_battleCenter;
        switch (gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant)
        {
        case 0:
            pThis->m44_translationTarget[2] -= 0x3000;
            break;
        case 1:
            pThis->m44_translationTarget[0] -= 0x3000;
            break;
        case 2:
            pThis->m44_translationTarget[2] += 0x3000;
            break;
        case 3:
            pThis->m44_translationTarget[0] += 0x3000;
            break;
        default:
            assert(0);
        }

        if (pThis->m38_3dModel->m16_previousAnimationFrame != 0x1E)
            return;

        applyDamage(gBattleManager->m10_battleOverlay->m18_dragon->m8C, 27, gBattleManager->m10_battleOverlay->m18_dragon->m8_position, 3, gBattleManager->m10_battleOverlay->m18_dragon->m8_position - *pThis->m1C_translation.m0_current, 0);
        createDamageSpriteEffect(dramAllocatorEnd[6].mC_fileBundle, gCurrentBattleOverlay->getSaturnPtr(0x060a912), &gBattleManager->m10_battleOverlay->m18_dragon->m8_position, nullptr, nullptr, 0x10000, 0, 0);
        playSystemSoundEffect(0x67);
        pThis->m9_attackStatus = 3;
        break;
    case 3:
        // animation finished
        if (pThis->m38_3dModel->m16_previousAnimationFrame < (pThis->m38_3dModel->m30_pCurrentAnimation ? pThis->m38_3dModel->m30_pCurrentAnimation->m4_numFrames : 0) - 1)
        {
            return;
        }
        initAnimation(pThis->m38_3dModel, dramAllocatorEnd[readSaturnU8(pThis->m3C_dataPtr)].mC_fileBundle->m0_fileBundle->getAnimation(readSaturnU16(readSaturnEA(pThis->m3C_dataPtr + 0xC) + 0)));
        pThis->m34_formationEntry->m48 &= ~2;
        pThis->m9_attackStatus = 4;
        break;
    case 4:
        pThis->m8_mode = 0;
        pThis->m9_attackStatus = 0;
        break;
    default:
        assert(0);
    }
}

// side attack
void Baldor_update_mode2(sBaldor* pThis)
{
    switch (pThis->m9_attackStatus)
    {
    case 0: // init attack
        if (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2000)
        {
            return;
        }

        if (pThis->m34_formationEntry->m49 != 1)
        {
            return;
        }

        pThis->m34_formationEntry->m48 &= ~1;
        pThis->m34_formationEntry->m48 |= 2;

        switch (gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant)
        {
        case 1:
            pThis->m5C_rotationDelta[1] += 0x71C71C;
            break;
        case 3:
            pThis->m5C_rotationDelta[1] -= 0x71C71C;
            break;
        default:
            break;
        }

        pThis->m90_attackTask = Baldor_createAttackTask(pThis);
        playSystemSoundEffect(0x68);
        pThis->m9_attackStatus++;
        break;
    case 1: // attack is running
        if (pThis->m90_attackTask && !pThis->m90_attackTask->getTask()->isFinished())
            return;
        pThis->m34_formationEntry->m48 &= ~2;
        pThis->m9_attackStatus++;
        break;
    case 2: // attack is finished
        pThis->m8_mode = 0;
        pThis->m9_attackStatus = 0;
        break;
    default:
        assert(0);
    }
}

void Baldor_update_mode0(sBaldor* pThis)
{
    switch (pThis->m34_formationEntry->m49)
    {
    case 0:
    case 1:
        break;
    case 2:
        pThis->m8_mode = 1;
        pThis->m34_formationEntry->m48 |= 1;
        pThis->m34_formationEntry->m49 = 0;
        break;
    case 3: // attack queued
        pThis->m8_mode = 2; // enter attack mode
        pThis->m34_formationEntry->m48 |= 1;
        pThis->m34_formationEntry->m49 = 0;
        break;
    default:
        assert(0);
        break;
    }
}

void Baldor_update(sBaldor* pThis)
{
    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x1B])
    {
        assert(0);
    }

    if (isTraceEnabled())
    {
        addTraceLog(*pThis->m1C_translation.m4_target, "BaldorTranslation_m4");
        addTraceLog(gBattleManager->m10_battleOverlay->m4_battleEngine->mC_battleCenter, "battleCenter");
    }

    *pThis->m1C_translation.m0_current = gBattleManager->m10_battleOverlay->m4_battleEngine->mC_battleCenter + *pThis->m1C_translation.m4_target;

    if (pThis->mC_numTargetables > 0)
    {
        transformAndAddVecByCurrentMatrix(pThis->m1C_translation.m0_current, &pThis->m18_position[0]);
        for (int i = 1; i < 4; i++) // actually hard coded to 4
        {
            transformAndAddVecByCurrentMatrix(&pThis->m68->m30[i-1].m4, &pThis->m18_position[i]);
        }
    }

    stepAnimation(pThis->m38_3dModel);
    pThis->m44_translationTarget = *pThis->m1C_translation.m8;

    if ((gBattleManager->m6_subBattleId != 8) && (gBattleManager->m6_subBattleId != 9))
    {
        pThis->m6C += sVec3_FP(0x222222, 0x16c16c, 0xb60b6);

        pThis->m44_translationTarget[0] += MTH_Mul(0xA000, getSin(pThis->m6C[0].getInteger()));
        pThis->m44_translationTarget[1] += MTH_Mul(0xA000, getSin(pThis->m6C[1].getInteger()));
        pThis->m44_translationTarget[2] += MTH_Mul(0xA000, getSin(pThis->m6C[2].getInteger()));
    }
    else
    {
        assert(0);
    }

    pThis->m68->m0_translation = *pThis->m1C_translation.m0_current;
    pThis->m68->mC_rotation = *pThis->m28_rotation.m0_current;
    pThis->m68->m18 = *pThis->m28_rotation.m0_current;

    sBaldor_68* pData = pThis->m68;
    pData->m24_update(&pData->m30[0], &pData->m0_translation, &pData->mC_rotation, &pData->m18);

    pThis->m78 = pThis->m50_translationDelta;

    Baldor_updateSub0(pThis);

    switch (pThis->m8_mode)
    {
    case 0: //idle
        Baldor_update_mode0(pThis);
        break;
    case 1: // attacking in front
        Baldor_update_mode1(pThis);
        break;
    case 2: // attacking on the side
        Baldor_update_mode2(pThis);
        break;
    default:
        assert(0);
    }

    Baldor_updateSub1(pThis->m1C_translation.m4_target, &pThis->m50_translationDelta, &pThis->m44_translationTarget, 0x1999, 0x147, 0);

    sVec2_FP temp;
    computeVectorAngles(gBattleManager->m10_battleOverlay->m4_battleEngine->m1A0_battleAutoScrollDelta + pThis->m78, temp);

    if (isTraceEnabled())
    {
        addTraceLog(gBattleManager->m10_battleOverlay->m4_battleEngine->m1A0_battleAutoScrollDelta, "m1A0");
        addTraceLog(pThis->m78, "m78");
        addTraceLog(temp, "temp");
        addTraceLog(*pThis->m28_rotation.m0_current, "current");
        addTraceLog(pThis->m5C_rotationDelta, "delta");
        addTraceLog(*pThis->m28_rotation.m4_target, "target");
    }

    (*pThis->m28_rotation.m4_target)[0] = -temp[0];
    (*pThis->m28_rotation.m4_target)[1] = temp[1] + 0x8000000;
    (*pThis->m28_rotation.m4_target)[2] = 0;

    Baldor_updateSub1(pThis->m28_rotation.m0_current, &pThis->m5C_rotationDelta, pThis->m28_rotation.m4_target, 0x1999, 0x28F, 1);

    if (isTraceEnabled())
    {
        addTraceLog(*pThis->m1C_translation.m0_current, "BaldorTranslation");
        addTraceLog(*pThis->m28_rotation.m0_current, "BaldorRotation");
        addTraceLog(*pThis->m1C_translation.m4_target, "BaldorTranslation_m4");
    }
}

void Baldor_draw(sBaldor* pThis)
{
    if (isTraceEnabled())
    {
        addTraceLog(*pThis->m1C_translation.m0_current, "BaldorTranslation");
        addTraceLog(*pThis->m28_rotation.m0_current, "BaldorRotation");
    }

    if (pThis->mB & 8)
    {
        assert(0);
    }
    pushCurrentMatrix();
    translateCurrentMatrix(pThis->m1C_translation.m0_current);
    rotateCurrentMatrixYXZ(pThis->m28_rotation.m0_current);
    pThis->m38_3dModel->m18_drawFunction(pThis->m38_3dModel);
    popMatrix();

    sBaldor_68* pBaldorPart = pThis->m68;
    pBaldorPart->m28_draw(pThis, &pBaldorPart->m30[0]);

    if (pThis->mB & 8)
    {
        assert(0);
    }
}

sBaldor* createBaldor(s_workAreaCopy* parent, sFormationData* pFormationEntry)
{
    static const sBaldor::TypedTaskDefinition definition = {
        Baldor_init,
        Baldor_update,
        Baldor_draw,
        nullptr,
    };
    return createSiblingTaskWithArgWithCopy<sBaldor, sFormationData*>(parent, pFormationEntry, &definition);
}
