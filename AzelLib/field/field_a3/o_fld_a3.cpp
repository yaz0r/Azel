#include "PDS.h"
#include "debugWindows.h"
#include "o_fld_a3.h"
#include "a3_crane_upper.h"
#include "a3_crane_lower.h"
#include "a3_static_mine_cart.h"
#include "a3_dynamic_mine_cart.h"
#include "a3_fan.h"
#include "a3_2_crashedImperialShip.h"
#include "particlePool.h"

#include "audio/systemSounds.h"
#include "kernel/animation.h"
#include "kernel/fileBundle.h"
#include "kernel/cinematicBarsTask.h"
#include "collision.h"
#include "field/fieldVisibilityGrid.h"

#include "audio/soundDriver.h"

#include "items.h"

#include "kernel/dialogTask.h"
#include "processModel.h"

#include "kernel/textDisplay.h"
#include "kernel/loadSavegameScreen.h"
#include "battle/BTL_A3/BTL_A3_map6.h"
#include "field/fieldRadar.h"
#include "field/battleStart.h"
#include "field/fieldDragonMovement.h"
#include "kernel/rayDisplay.h"
#include "battle/gunShotRootTask.h"

sMatrix4x3* fieldCameraTask1DrawSub1();
fixedPoint interpolateDistance(fixedPoint r11, fixedPoint r12, fixedPoint stack0, fixedPoint r10, s32 r14);
void activateCameraFollowMode(u32 r4);
void fieldOverlaySubTaskInitSub5(u32 r4);
s32 checkPositionVisibility(const sVec3_FP* r4, s32 r5);

#ifdef PDS_TOOL
bool bMakeEverythingVisible = false;
#endif

FLD_A3_data* gFLD_A3 = NULL;
sSaturnPtr gFieldCameraConfigEA;
sSaturnPtr gFieldDragonAnimTableEA;
void(*gFieldCameraDrawFunc)(sFieldCameraStatus*) = nullptr;

const s_MCB_CGB fieldFileList[] =
{
{ "FLDCMN.MCB", "FLDCMN.CGB" },
{ "FLD_A3.MCB", "FLD_A3.CGB" },
{ "FLD_A3_0.MCB", "FLD_A3_0.CGB" },
{ "FLD_A3_1.MCB", "FLD_A3_1.CGB" },
{ "FLD_A3_2.MCB", "FLD_A3_2.CGB" },
{ "FLD_A3_3.MCB", "FLD_A3_3.CGB" },
{ (const char*)-1, NULL }
};

std::vector<sLCSTaskDrawSub5Sub1_Data1> readLCSTaskDrawSub5Sub1_Data1(sSaturnPtr source)
{
    std::vector<sLCSTaskDrawSub5Sub1_Data1> deserializedArray(0x15);

    for (int i = 0; i < deserializedArray.size(); i++)
    {
        sLCSTaskDrawSub5Sub1_Data1& entry = deserializedArray[i];

        entry.m2 = readSaturnS16(source + 0x2);
        entry.m4 = readSaturnS16(source + 0x4);
        entry.m6_CMDSRCA = readSaturnS16(source + 0x6);
        entry.m8 = readSaturnS16(source + 0x8);
        entry.mA_CMDCOLR = readSaturnS16(source + 0xA);
        entry.mC_spriteWidth = readSaturnS32(source + 0xC);
        entry.m10_spriteHeight = readSaturnS32(source + 0x10);
        entry.m14_offsetX = readSaturnS32(source + 0x14);
        entry.m18_offsetY = readSaturnS32(source + 0x18);
        source += 0x1C;
    }

    return deserializedArray;
}

std::vector<sLCSTaskDrawSub5Sub1_Data1> LCSTaskDrawSub5Sub1_Data1;

void dragonExitField(s_dragonTaskWorkArea* r14);
s8 isFieldCameraSlotActive(s32 index);

std::vector<std::vector<sCameraVisibility>>* readCameraVisbility(sSaturnPtr EA, s_DataTable3* pDataTable3)
{
    std::vector<std::vector<sCameraVisibility>>* pVisibility = new std::vector<std::vector<sCameraVisibility>>;
    pVisibility->resize(pDataTable3->m10_gridSize[0] * pDataTable3->m10_gridSize[1]);

    for (int i = 0; i < pDataTable3->m10_gridSize[0] * pDataTable3->m10_gridSize[1]; i++)
    {
        sSaturnPtr visibilityEA = readSaturnEA(EA); EA = EA + 4;
        assert(visibilityEA.m_offset);

        sCameraVisibility visibility;
        do
        {
            visibility.m0 = readSaturnS8(visibilityEA); visibilityEA = visibilityEA + 1;
            visibility.m1 = readSaturnS8(visibilityEA); visibilityEA = visibilityEA + 1;
            (*pVisibility)[i].push_back(visibility);
        } while (visibility.m0 != -1);
    }

    return pVisibility;
}

void s_visdibilityCellTask::fieldGridTask_Update(s_visdibilityCellTask*)
{
    // intentionally empty
}

void setupGridCell(s_visibilityGridWorkArea* r4, s_visdibilityCellTask* r5, int index)
{
    getMemoryArea(&r5->m0_memoryLayout, r4->m30->mC);
    r5->m14_index = index;
    if (r4->m30->m0_environmentGrid.size())
    {
        r5->m8_pEnvironmentCell = &r4->m30->m0_environmentGrid[index];
    }
    if (r4->m30->m4.size())
    {
        r5->mC_pCell2_billboards = &r4->m30->m4[index];
    }
    if (r4->m30->m8)
    {
        r5->m10_pCell3 = r4->m30->m8[index];
    }
}


void s_visdibilityCellTask::gridCellDraw_untextured(s_visdibilityCellTask*)
{
    assert(0);
}

void s_visdibilityCellTask::gridCellDraw_collision(s_visdibilityCellTask*)
{
    assert(0);
}

u8 gridCellDraw_normalSub0(sProcessed3dModel* r4, const sVec3_FP& r5)
{
    s_visibilityGridWorkArea* var_1C = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;
    sVec3_FP DragonPos = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m8_pos;

    sVec3_FP dist;

    {
        fixedPoint XDist = r5[0] - DragonPos[0];
        if (XDist >= 0)
        {
            dist[0] = XDist;
        }
        else
        {
            dist[0] = DragonPos[0] - r5[0];
        }
    }

    {
        fixedPoint YDist = r5[1] - DragonPos[1];
        if (YDist >= 0)
        {
            dist[1] = YDist;
        }
        else
        {
            dist[1] = DragonPos[1] - r5[1];
        }
    }

    {
        fixedPoint ZDist = r5[2] - DragonPos[2];
        if (ZDist >= 0)
        {
            dist[2] = ZDist;
        }
        else
        {
            dist[2] = DragonPos[2] - r5[2];
        }
    }

    s32 distanceThreshold = r4->m0_radius + 0x8000;
    if (dist[0] > distanceThreshold)
        return 0;
    if (dist[1] > distanceThreshold)
        return 0;
    if (dist[2] > distanceThreshold)
        return 0;

    allocateLCSEntry(var_1C, r4, 0x10000);

    return 1;
}

void getCameraProperties2Matrix(sMatrix4x3* pOutput)
{
    *pOutput = cameraProperties2.m88_billboardViewMatrix;
}

void s_visdibilityCellTask::gridCellDraw_normal(s_visdibilityCellTask* pTypedWorkAread)
{
    s_visibilityGridWorkArea* r13 = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;

    s32 r15 = graphicEngineStatus.m405C.m14_farClipDistance;

    if (pTypedWorkAread->m8_pEnvironmentCell)
    {
        std::vector<s_grid1>::const_iterator r14 = pTypedWorkAread->m8_pEnvironmentCell->begin();
        while (r14 != pTypedWorkAread->m8_pEnvironmentCell->end())
        {
            r13->m12E0++;
            if (r14->m0_modelsOffets[0])
            {
                r13->m12F0 += pTypedWorkAread->m0_memoryLayout.m0_mainMemoryBundle->get3DModel(r14->m0_modelsOffets[0])->m4_numVertices;
            }

            if (!r13->m12FC_isObjectClipped(&r14->m4, r15))
            {
                u32 var_54 = 0;
                r13->m12E2++;

                if (r14->m0_modelsOffets[4])
                {
                    sProcessed3dModel* pCollisionModel = pTypedWorkAread->m0_memoryLayout.m0_mainMemoryBundle->get3DModel(r14->m0_modelsOffets[4]);
                    var_54 = gridCellDraw_normalSub0(pCollisionModel, r14->m4);
                }

                pushCurrentMatrix();
                translateCurrentMatrix(&r14->m4);
                rotateCurrentMatrixZ(r14->m10[2]);
                rotateCurrentMatrixY(r14->m10[1]);
                rotateCurrentMatrixX(r14->m10[0]);

                u32 depthRangeIndex = gridCellDraw_GetDepthRange(pCurrentMatrix->m[2][3]);

                if (r14->m0_modelsOffets[depthRangeIndex])
                {
                    sProcessed3dModel* pModel = pTypedWorkAread->m0_memoryLayout.m0_mainMemoryBundle->get3DModel(r14->m0_modelsOffets[depthRangeIndex]);
                    addObjectToDrawList(pModel);
                }

                if (var_54)
                {
                    sMatrix4x3 var_C;
                    initMatrixToIdentity(&var_C);
                    translateMatrix(&r14->m4, &var_C);
                    rotateMatrixZYX_s16(r14->m10, &var_C);
                    copyMatrix(&var_C, &r13->m44[-1].m4_matrix); // Gross. This was incremented in gridCellDraw_normalSub0
                }

                popMatrix();
            }

            r14++;
        }
    }

    if (pTypedWorkAread->mC_pCell2_billboards)
    {
        std::vector<s_grid2>::iterator r14 = pTypedWorkAread->mC_pCell2_billboards->begin();
        while (r14 != pTypedWorkAread->mC_pCell2_billboards->end())
        {
            r13->m12E0++;
            r13->m12F0 += pTypedWorkAread->m0_memoryLayout.m0_mainMemoryBundle->get3DModel(readSaturnS16(r14->m0))->m4_numVertices;

            if (!r13->m12FC_isObjectClipped(&r14->m4, r15))
            {
                r13->m12E2++;

                pushCurrentMatrix();
                translateCurrentMatrix(&r14->m4);

                u32 depthRangeIndex = gridCellDraw_GetDepthRange(pCurrentMatrix->m[2][3]);
                if (depthRangeIndex <= r13->m1300)
                {
                    addBillBoardToDrawList(pTypedWorkAread->m0_memoryLayout.m0_mainMemoryBundle->get3DModel(readSaturnS16(r14->m0)));

                    if (readSaturnS16(r14->m0 + 2))
                    {
                        getCameraProperties2Matrix(pCurrentMatrix);
                        gridCellDraw_normalSub2(pTypedWorkAread->m0_memoryLayout.m0_mainMemoryBundle, readSaturnS16(r14->m0 + 2), 0x10000);
                    }
                }
                popMatrix();
            }

            r14++;
        }
    }

    if (pTypedWorkAread->m10_pCell3)
    {
        assert(0);
    }
}

s_visdibilityCellTask::FunctionType gridCellDrawFunctions[3] =
{
    &s_visdibilityCellTask::gridCellDraw_untextured, // untextured
    &s_visdibilityCellTask::gridCellDraw_collision, // textures collision geo
    &s_visdibilityCellTask::gridCellDraw_normal, // normal textured
};

s_visdibilityCellTask* createGridCellTask(s_visibilityGridWorkArea* r4, p_workArea r5, int cellIndex)
{
    s_visdibilityCellTask* pNewTask = createSubTask<s_visdibilityCellTask>(r5);

    if (pNewTask)
    {
        setupGridCell(r4, pNewTask, cellIndex);

        pNewTask->m_DrawMethod = gridCellDrawFunctions[r4->m12F2_renderMode];
    }

    return pNewTask;
}

void enableCellsBasedOnVisibilityList(s_visibilityGridWorkArea* r14, std::vector<sCameraVisibility>& r5)
{
    for (int i = 0; i < r5.size(); i++)
    {
        if (r5[i].m0 >= 0)
        {
            r14->m3C_cellRenderingTasks[(r14->m30->m10_gridSize[0] * r5[i].m1) + r5[i].m0]->getTask()->clearPaused();
        }
    }
}

s32 cellsToVisitForVisibility[9][2] =
{
    { -1,-1 },
{ 0,-1 },
{ 1,-1 },
{ -1,0 },
{ 0,0 },
{ 1,0 },
{ -1,1 },
{ 0,1 },
{ 1,1 }
};

void enableCellsAroundCamera(s_visibilityGridWorkArea* r14)
{
    if (r14->m30 == NULL)
        return;

    if (r14->m3C_cellRenderingTasks == NULL)
        return;

    if (r14->m34_cameraVisibilityTable &&
        (r14->m18_cameraGridLocation[0] >= 0) &&
        (r14->m18_cameraGridLocation[0] < r14->m30->m10_gridSize[0]) &&
        (r14->m18_cameraGridLocation[1] >= 0) &&
        (r14->m18_cameraGridLocation[1] < r14->m30->m10_gridSize[1]) //&&
        /*(r14->m34_cameraVisibilityTable[r14->m18_cameraGridLocation[0] + r14->m18_cameraGridLocation[1] * r14->m30->m10_gridSize[0]])*/)
    {
        return enableCellsBasedOnVisibilityList(r14, (*r14->m34_cameraVisibilityTable)[r14->m18_cameraGridLocation[0] + r14->m18_cameraGridLocation[1] * r14->m30->m10_gridSize[0]]);
    }

    for (int i = 0; i < 9; i++)
    {
        s32 r5 = r14->m18_cameraGridLocation[0] + cellsToVisitForVisibility[i][0];
        s32 r6 = r14->m18_cameraGridLocation[1] + cellsToVisitForVisibility[i][1];

        if ((r5 >= 0) && (r5 < r14->m30->m10_gridSize[0]) && (r6 >= 0) && (r6 < r14->m30->m10_gridSize[1]))
        {
            r14->m3C_cellRenderingTasks[r5 + r6 * r14->m30->m10_gridSize[0]]->getTask()->clearPaused();
        }
    }
}

void pauseAllCells(s_visibilityGridWorkArea* pFieldCameraTask1)
{
    if (pFieldCameraTask1->m30 == NULL)
        return;

    if (pFieldCameraTask1->m3C_cellRenderingTasks == NULL)
        return;

    for (int i = 0; i < pFieldCameraTask1->m30->m10_gridSize[0] * pFieldCameraTask1->m30->m10_gridSize[1]; i++)
    {
        pFieldCameraTask1->m3C_cellRenderingTasks[i]->getTask()->markPaused();
    }
}

void updateCellGridIfDirty(s_visibilityGridWorkArea* pFieldCameraTask1)
{
#ifdef PDS_TOOL
    if (bMakeEverythingVisible)
    {
        for (int i = 0; i < pFieldCameraTask1->m30->m10_gridSize[0] * pFieldCameraTask1->m30->m10_gridSize[1]; i++)
        {
            pFieldCameraTask1->m3C_cellRenderingTasks[i]->getTask()->clearPaused();
        }
        pFieldCameraTask1->updateVisibleCells = false;
    }
#endif
    if (pFieldCameraTask1->updateVisibleCells)
    {
        pauseAllCells(pFieldCameraTask1);
        enableCellsAroundCamera(pFieldCameraTask1);
    }
}

void updateCellGridFromCameraPosition(s_visibilityGridWorkArea* pFieldCameraTask1)
{
    pFieldCameraTask1->m0_position[0] = cameraProperties2.m0_position[0];
    pFieldCameraTask1->m0_position[1] = cameraProperties2.m0_position[1];
    pFieldCameraTask1->m0_position[2] = cameraProperties2.m0_position[2];

    pFieldCameraTask1->updateVisibleCells = pFieldCameraTask1->m12F8_convertCameraPositionToGrid(pFieldCameraTask1);

    updateCellGridIfDirty(pFieldCameraTask1);
}

struct s_s_visibilityGridWorkArea_38 : public s_workAreaTemplate<s_s_visibilityGridWorkArea_38>
{

};

void setupField2(s_DataTable3* r4, void(*r5)(p_workArea workArea))
{
    s_visibilityGridWorkArea* pFieldCameraTask1 = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;
    if (pFieldCameraTask1->m38)
    {
        assert(0);
    }

    pFieldCameraTask1->m38 = createSubTaskFromFunction<s_s_visibilityGridWorkArea_38>(pFieldCameraTask1, NULL);

    if (r4)
    {
        pFieldCameraTask1->m30 = r4;
        pFieldCameraTask1->m20_cellDimensions[0] = r4->m18_cellDimensions[0] * 2;
        pFieldCameraTask1->m20_cellDimensions[1] = r4->m18_cellDimensions[1] * 2;
        pFieldCameraTask1->m28 = r4->m20;
        loadFileFromFileList(r4->mC);

        if (pFieldCameraTask1->m28 & 1)
        {
            //06070FE0 — vertical grid mode (tower): grid X maps to world Y
            pFieldCameraTask1->mC[0] = 0;
            {
                s32 r3 = r4->m10_gridSize[0] * pFieldCameraTask1->m20_cellDimensions[0];
                if (r3 < 0)
                {
                    r3++;
                }
                pFieldCameraTask1->mC[1] = r3 / 2;
            }
            {
                s32 r3 = -(r4->m10_gridSize[1] * pFieldCameraTask1->m20_cellDimensions[1]);
                if (r3 < 0)
                {
                    r3++;
                }
                pFieldCameraTask1->mC[2] = r3 / 2;
            }
        }
        else
        {
            //0607100A
            {
                s32 r3 = r4->m10_gridSize[0] * pFieldCameraTask1->m20_cellDimensions[0];
                if (r3 < 0)
                {
                    r3++;
                }
                r3 /= 2;
                pFieldCameraTask1->mC[0] = r3 / 2;

            }
            pFieldCameraTask1->mC[1] = 0;
            {
                s32 r3 = -(r4->m10_gridSize[1] * pFieldCameraTask1->m20_cellDimensions[1]);
                if (r3 < 0)
                {
                    r3++;
                }
                pFieldCameraTask1->mC[2] = r3 / 2;
            }
        }
        //06071032

        pFieldCameraTask1->m0_position = cameraProperties2.m0_position;
        pFieldCameraTask1->m12F8_convertCameraPositionToGrid(pFieldCameraTask1);

        s32* var_1C = pFieldCameraTask1->m30->m10_gridSize;
        int gridSize = pFieldCameraTask1->m30->m10_gridSize[0] * pFieldCameraTask1->m30->m10_gridSize[1];

        pFieldCameraTask1->m3C_cellRenderingTasks = (s_visdibilityCellTask**)allocateHeapForTask(pFieldCameraTask1->m38, gridSize * sizeof(s_visdibilityCellTask*));

        for (int cellIndex = 0; cellIndex < gridSize; cellIndex++)
        {
            pFieldCameraTask1->m3C_cellRenderingTasks[cellIndex] = createGridCellTask(pFieldCameraTask1, pFieldCameraTask1->m38, cellIndex);
            pFieldCameraTask1->m3C_cellRenderingTasks[cellIndex]->getTask()->markPaused();
        }

        //060710C4
        enableCellsAroundCamera(pFieldCameraTask1);
        pFieldCameraTask1->m_UpdateMethod = &updateCellGridFromCameraPosition;
    }
    else
    {
        pFieldCameraTask1->m20_cellDimensions[0] = graphicEngineStatus.m405C.m14_farClipDistance;
        pFieldCameraTask1->m20_cellDimensions[1] = graphicEngineStatus.m405C.m14_farClipDistance;
    }

    if (r5)
    {
        r5(pFieldCameraTask1->m38);
    }
}

struct s_A3_Obj2 : public s_workAreaTemplate<s_A3_Obj2>
{
    s_memoryAreaOutput m0;
    s_DataTable2Sub0* m8;
    u32* mC_verticalOffset;
    sVec3_FP m10_position;
    sVec3_FP m1C_nodeLength;
    s32 m28_numNodes;
    s32 m2C;
    s32 m30;
    s32 m34;
    s32 m38;
    s32 m3C;
    s32 m40;
    s32 m44;
    s32 m48;
    s32 m4C;
    fixedPoint m50;
    fixedPoint m54;
    sSaturnPtr m58;
    std::vector<fixedPoint> m5C_perNodeRotation;
    sLCSTarget m60;
    // size 0x94
};

void getDragonDeltaMovement(sVec3_FP* r4)
{
    *r4 = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m160_deltaTranslation;
}

void A3_Obj2_Update(s_A3_Obj2* r14)
{
    s32 r10 = r14->m30;
    std::vector<fixedPoint>::iterator var8 = r14->m5C_perNodeRotation.begin();
    sSaturnPtr var4 = sSaturnPtr({ 0x6092964, gFLD_A3 });

    if (getFieldSpecificData_A3()->m8)
    {
        r14->m60.m18_diableFlags = 0;
    }
    else
    {
        r14->m60.m18_diableFlags |= 1;
    }

    updateLCSTarget(&r14->m60);

    r14->m10_position[1] = *r14->mC_verticalOffset + r14->m8->m4_position[1];

    fixedPoint r11 = MTH_Mul(r14->m38, getFieldSpecificData_A3()->m0);
    if (r11 == r14->m40)
    {
        // if dragon is moving fast enough and close enough to the flag
        if ((getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m235_dragonSpeedIndex >= 3) && (vecDistance(r14->m10_position, getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m8_pos) < fixedPoint(0xC8000)))
        {
            sVec3_FP dragonDeltaMovement;
            getDragonDeltaMovement(&dragonDeltaMovement);

            // is the dragon over or under the flag?
            if (getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m8_pos[1] < r14->m10_position[1])
            {
                r14->m40 = MTH_Mul(0x38E38E3, getFieldSpecificData_A3()->m0);
            }
            else
            {
                r14->m40 = MTH_Mul(0xE38E38, getFieldSpecificData_A3()->m0);
            }

            if (dragonDeltaMovement[2] > 0)
            {
                r14->m40 = -r14->m40;
            }
        }
        else
        {
            //605FC2E
            if (r14->mC_verticalOffset[0])
            {
                r14->m40 = 0;
            }
        }
    }

    //605FC3A
    if (r14->m3C > r14->m40)
    {
        //605FC60
        r14->m3C -= r14->m44;
        if (r14->m3C < r14->m40)
        {
            r14->m3C = r14->m40;
        }
    }
    else if (r14->m3C < r14->m40)
    {
        //605FC60
        r14->m3C += r14->m44;
        if (r14->m3C > r14->m40)
        {
            r14->m3C = r14->m40;
        }
    }
    else
    {
        //605FCA8
        if (r14->m40 != r11)
        {
            r14->m40 = r11;
        }
    }

    //605FCB4
    r14->m30 -= r14->m34;
    r14->m48 += r14->m4C;

    fixedPoint var0 = MTH_Mul(r14->m50, getSin(r14->m48 >> 16)) + r14->m3C + fixedPoint(0x8000000);

    s32 r12 = r14->m28_numNodes;
    if (r12 == 0)
    {
        return;
    }

    fixedPoint r9_previousRotation = 0;

    do
    {
        s32 r6 = readSaturnS32(var4);
        var4 += 4;

        fixedPoint r13 = MTH_Mul_5_6(getSin((s16(r10 >> 16)) & 0xFFF), r14->m54, r6);

        r10 += r14->m2C;

        *(var8++) = atan2_FP(r13 - r9_previousRotation, r14->m1C_nodeLength[1]) + var0;

        r9_previousRotation = r13;
    } while (--r12);
}

void A3_Obj2_Draw(s_A3_Obj2* r14)
{
    std::vector<fixedPoint>::iterator r12 = r14->m5C_perNodeRotation.begin();
    sSaturnPtr r10 = r14->m58;

    sMatrix4x3 var0_baseMatrix;
    sMatrix4x3 var30_nodeMatrix;
    sVec3_FP var60_currentNodePosition = r14->m10_position;
    initMatrixToIdentity(&var0_baseMatrix);
    s_DataTable2Sub0* r13 = r14->m8;
    rotateMatrixZ(r13->m10_rotation[2], &var0_baseMatrix);
    rotateMatrixY(r13->m10_rotation[1], &var0_baseMatrix);

    s32 r11 = r14->m28_numNodes;
    if (r11 <= 0)
    {
        return;
    }

    do
    {
        copyMatrix(&var0_baseMatrix, &var30_nodeMatrix);
        rotateMatrixShiftedX(*r12, &var30_nodeMatrix);

        sVec3_FP var6C_nodeIncrement;
        transformAndAddVec(r14->m1C_nodeLength, var6C_nodeIncrement, var30_nodeMatrix);
        var60_currentNodePosition += var6C_nodeIncrement;

        pushCurrentMatrix();
        translateCurrentMatrix(&var60_currentNodePosition);
        rotateCurrentMatrixZ(r13->m10_rotation[2]);
        rotateCurrentMatrixY(r13->m10_rotation[1]);
        rotateCurrentMatrixShiftedX(*r12);

        addObjectToDrawList(r14->m0.m0_mainMemoryBundle->get3DModel(readSaturnS16(r10)));

        popMatrix();

        r12++;
        r10 += 2;
    } while (--r11);
}

void create_A3_Obj2_Sub1(p_workArea, sLCSTarget*)
{
    assert(0);
}

s32 checkPositionVisibilityAgainstFarPlane(sVec3_FP* r4)
{
    return checkPositionVisibility(r4, graphicEngineStatus.m405C.m14_farClipDistance);
}

void getFieldDragonPosition(sVec3_FP* r4)
{
    *r4 = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m8_pos;
}

void create_A3_Obj2(s_visdibilityCellTask* r4, s_DataTable2Sub0& r5, s32 r6, s32 r7)
{
    s_A3_Obj2* pNewObj = createSubTaskFromFunction<s_A3_Obj2>(r4, NULL);

    getMemoryArea(&pNewObj->m0, r6);

    pNewObj->m8 = &r5;
    pNewObj->m10_position = r5.m4_position;
    pNewObj->m1C_nodeLength[2] = 0;
    pNewObj->m1C_nodeLength[0] = 0;

    sSaturnPtr r11;

    switch (r7)
    {
    case 0:
        r11 = sSaturnPtr({ 0x6092984, gFLD_A3 }) + 0x1C * r5.m18;
        pNewObj->mC_verticalOffset = &getFieldSpecificData_A3()->m50[readSaturnS8(sSaturnPtr({ 0x609290E, gFLD_A3 }) + r5.m18)];
        pNewObj->m28_numNodes = 8;
        pNewObj->m1C_nodeLength[1] = 0x6000;
        pNewObj->m58 = sSaturnPtr({ 0x609293A, gFLD_A3 });
        break;
    case 1:
        r11 = sSaturnPtr({ 0x6092A48, gFLD_A3 }) + 0x1C * r5.m18;
        pNewObj->mC_verticalOffset = &getFieldSpecificData_A3()->m50[readSaturnS8(sSaturnPtr({ 0x6092915, gFLD_A3 }) + r5.m18)];
        pNewObj->m28_numNodes = 4;
        pNewObj->m1C_nodeLength[1] = 0x6000;
        pNewObj->m58 = sSaturnPtr({ 0x609294A, gFLD_A3 });
        break;
    case 2:
        r11 = sSaturnPtr({ 0x6092B28, gFLD_A3 }) + 0x1C * r5.m18;
        pNewObj->mC_verticalOffset = &getFieldSpecificData_A3()->m50[readSaturnS8(sSaturnPtr({ 0x609291D, gFLD_A3 }) + r5.m18)];
        pNewObj->m28_numNodes = 4;
        pNewObj->m1C_nodeLength[1] = 0x3000;
        pNewObj->m58 = sSaturnPtr({ 0x6092952, gFLD_A3 });
        break;
    case 3:
        r11 = sSaturnPtr({ 0x6092BEC, gFLD_A3 }) + 0x1C * r5.m18;
        pNewObj->mC_verticalOffset = &getFieldSpecificData_A3()->m50[readSaturnS8(sSaturnPtr({ 0x6092924, gFLD_A3 }) + r5.m18)];
        pNewObj->m28_numNodes = 4;
        pNewObj->m1C_nodeLength[1] = 0x3000;
        pNewObj->m58 = sSaturnPtr({ 0x609295A, gFLD_A3 });
        break;
    default:
        assert(0);
        break;
    }

    pNewObj->m2C = readSaturnS32(r11);
    pNewObj->m34 = readSaturnS32(r11 + 4);
    pNewObj->m40 = pNewObj->m3C = pNewObj->m38 = MTH_Mul(readSaturnS32(r11 + 8), getFieldSpecificData_A3()->m0);
    pNewObj->m44 = readSaturnS32(r11 + 0xC);
    pNewObj->m4C = readSaturnS32(r11 + 0x10);

    if (getFieldSpecificData_A3()->m0 >= 0)
    {
        pNewObj->m50 = MTH_Mul(readSaturnS32(r11 + 0x14), getFieldSpecificData_A3()->m0);
    }
    else
    {
        pNewObj->m50 = MTH_Mul(readSaturnS32(r11 + 0x14), -getFieldSpecificData_A3()->m0);
    }

    if (getFieldSpecificData_A3()->m0 >= 0)
    {
        pNewObj->m54 = MTH_Mul(readSaturnS32(r11 + 0x18), getFieldSpecificData_A3()->m0);
    }
    else
    {
        pNewObj->m54 = MTH_Mul(readSaturnS32(r11 + 0x18), -getFieldSpecificData_A3()->m0);
    }

    pNewObj->m30 = randomNumber();
    pNewObj->m48 = randomNumber();

    pNewObj->m5C_perNodeRotation.resize(pNewObj->m28_numNodes);

    pNewObj->m_UpdateMethod = &A3_Obj2_Update;
    pNewObj->m_DrawMethod = &A3_Obj2_Draw;

    createLCSTarget(&pNewObj->m60, pNewObj, &create_A3_Obj2_Sub1, &pNewObj->m10_position, 0, 0, 0, eItems::mMinusOne, 0, 0);

    getFieldSpecificData_A3()->m8 = 0;
}


struct s_A3_Obj0 : public s_workAreaTemplate<s_A3_Obj0>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { NULL, NULL, &s_A3_Obj0::Draw, NULL };
        return &taskDefinition;
    }
    static void Draw(s_A3_Obj0* pThis)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->m14_position);
        rotateCurrentMatrixY(pThis->m3A_rotation);

        addObjectToDrawList(pThis->m0.m0_mainMemoryBundle->get3DModel(pThis->m38_modelOffset));

        popMatrix();
    }

    s_memoryAreaOutput m0;
    s_DataTable2Sub0* m8;
    u32* mC;
    sVec3_FP m14_position;
    s32 m24;
    sVec3_FP m28;
    s32 m2C;
    u16 m38_modelOffset;
    s16 m3A_rotation;
};

p_workArea create_A3_Obj0(s_visdibilityCellTask* r4, s_DataTable2Sub0& r5, s32 r6, s32 r7)
{
    s_A3_Obj0* pNewTask = createSubTask<s_A3_Obj0>(r4);
    getMemoryArea(&pNewTask->m0, r6);
    pNewTask->m8 = &r5;

    switch (r7)
    {
    case 0:
        pNewTask->m24 = 0x71C71C;
        pNewTask->m2C = 0x147;
        pNewTask->mC = &getFieldSpecificData_A3()->m50[r5.m18 + 0];
        pNewTask->m38_modelOffset = 0x22C;
        break;
    case 1:
        pNewTask->m24 = 0x6C16C1;
        pNewTask->m2C = 0xF5;
        pNewTask->mC = &getFieldSpecificData_A3()->m50[r5.m18 + 5];
        pNewTask->m38_modelOffset = 0x228;
        break;
    case 2:
        pNewTask->m24 = 0x666666;
        pNewTask->m2C = 0xA3;
        pNewTask->mC = &getFieldSpecificData_A3()->m50[r5.m18 + 11];
        pNewTask->m38_modelOffset = 0x224;
        break;
    default:
        assert(0);
        break;
    }

    pNewTask->m14_position = r5.m4_position;
    *pNewTask->mC = 0;
    pNewTask->m28.zeroize();
    if (r5.m10_rotation[0])
    {
        pNewTask->m3A_rotation = 0x800 - r5.m10_rotation[1];
    }
    else
    {
        pNewTask->m3A_rotation = r5.m10_rotation[1];
    }

    return pNewTask;
}

struct s_A3_Obj4 : public s_workAreaTemplate<s_A3_Obj4>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { NULL, &s_A3_Obj4::Update, &s_A3_Obj4::Draw, NULL };
        return &taskDefinition;
    }

    static void Update(s_A3_Obj4* pThis)
    {
        pThis->mC += 0xB60B6;
        fixedPoint var1C = getFieldSpecificData_A3()->mA4[pThis->m8->m18] = MTH_Mul(fixedPoint(0x71C71C), getSin((pThis->mC >> 16) & 0xFFF)) - fixedPoint(0x71C71C);

        sVec3_FP& r12 = getFieldSpecificData_A3()->mC0[pThis->m8->m18];
        r12[0] = pThis->m8->m4_position[0] - MTH_Mul(fixedPoint(0xE333), getSin((var1C >> 16) & 0xFFF));
        r12[1] = pThis->m8->m4_position[1] + MTH_Mul(fixedPoint(0xE333), getSin((var1C >> 16) & 0xFFF));

        if (checkPositionVisibilityAgainstFarPlane(&r12))
        {
            pThis->m10 = 1;
        }
    }

    static void Draw(s_A3_Obj4* pThis)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->m8->m4_position);
        rotateCurrentMatrixShiftedZ(getFieldSpecificData_A3()->mA4[pThis->m8->m18]);

        addObjectToDrawList(pThis->m0.m0_mainMemoryBundle->get3DModel(0x29C));

        popMatrix();
    }

    s_memoryAreaOutput m0;
    s_DataTable2Sub0* m8;
    u32 mC;
    s32 m10;
};

struct s_A3_Obj3 : public s_workAreaTemplate<s_A3_Obj3>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { NULL, &s_A3_Obj3::Update, &s_A3_Obj3::Draw, NULL };
        return &taskDefinition;
    }

    static void Update(s_A3_Obj3*)
    {
        Unimplemented();
    }

    static void Draw(s_A3_Obj3*)
    {
        Unimplemented();
    }

    s_memoryAreaOutput m0;
    s_DataTable2Sub0* m8;
    s32 mC;
    s32 m10;
    sVec3_FP m14;
    sVec3_FP m20;
    sVec3_FP m2C;
};

void create_A3_Obj4(s_visdibilityCellTask* r4, s_DataTable2Sub0& r5, s32 r6)
{
    s_A3_Obj4* pNewTask = createSubTask<s_A3_Obj4>(r4);
    getMemoryArea(&pNewTask->m0, r6);
    pNewTask->m8 = &r5;
    getFieldSpecificData_A3()->mC0[r5.m18] = r5.m4_position;
    pNewTask->mC = randomNumber();
}

void create_A3_Obj3(s_visdibilityCellTask* r4, s_DataTable2Sub0& r5, s32 r6)
{
    s_A3_Obj3* pNewTask = createSubTask<s_A3_Obj3>(r4);
    getMemoryArea(&pNewTask->m0, r6);
    pNewTask->m8 = &r5;
    getFieldSpecificData_A3()->mC0[r5.m18] = r5.m4_position;
    pNewTask->mC = 0x6000;
    pNewTask->m10 = 0;
    pNewTask->m14[0] = 0;
    pNewTask->m14[1] = 0x2000000;
    pNewTask->m20[0] = 0xC16C16;
    pNewTask->m20[1] = 0xB60B6;
    pNewTask->m2C[0] = 0x2D82D8;
    pNewTask->m2C[1] = 0xAAAAAA;
    pNewTask->m2C[2] = 0x2400;
    pNewTask->m14[2] = randomNumber();
    pNewTask->m20[2] = randomNumber();
}

// 06078a1c — simple particle draw (axis-aligned sprite)
static void particleDrawSimple(sParticleSlot* pSlot)
{
    drawProjectedParticle(&pSlot->m2C_animQuad, &pSlot->m0_position);
}

// 06078a2a — billboard particle draw (oriented sprite)
static void particleDrawBillboard(sParticleSlot* pSlot)
{
    // TODO: writeBillBoardToVDP1 not yet implemented
    drawProjectedParticle(&pSlot->m2C_animQuad, &pSlot->m0_position);
}

// 0607895c — particle update: moves by velocity, expires when animation ends
s32 particleUpdateMoving(sParticleSlot* pSlot)
{
    pSlot->m0_position.m0_X += pSlot->mC_velocity.m0_X;
    pSlot->m0_position.m4_Y += pSlot->mC_velocity.m4_Y;
    pSlot->m0_position.m8_Z += pSlot->mC_velocity.m8_Z;
    return sGunShotTask_UpdateSub4(&pSlot->m2C_animQuad) & 2;
}

// 0605a032 — particle update: static position, never expires
s32 particleUpdateStatic(sParticleSlot* pSlot)
{
    sGunShotTask_UpdateSub4(&pSlot->m2C_animQuad);
    return 0;
}

// 06078a3c — spawn a particle into the pool
s32 spawnParticleInPool(sParticlePoolManager* pPool, sParticleSpawnConfig* pConfig, s32 useVelocityScale)
{
    if (pPool->m18_activeCount >= pPool->m14_maxParticles)
        return 0;

    // Optionally allocate extra heap
    void* heapData = nullptr;
    if (pConfig->m18_heapSize > 0)
    {
        heapData = allocateHeapForTask((s_workArea*)pPool, pConfig->m18_heapSize);
        if (heapData == nullptr)
            return 0;
    }

    // Find a free slot (ring buffer scan)
    while (pPool->mC_currentSlot->m28_drawFunc != nullptr)
    {
        pPool->m10_currentIndex++;
        pPool->mC_currentSlot++;
        if (pPool->m10_currentIndex >= pPool->m14_maxParticles)
        {
            pPool->m10_currentIndex = 0;
            pPool->mC_currentSlot = pPool->m8_slotsBase;
        }
    }

    sParticleSlot* pSlot = pPool->mC_currentSlot;

    // Init animated quad
    u16 cmdsrca = (u16)((s32)(pPool->m4_vdp1Memory + 0xDA400000) >> 3);
    particleInitSub(&pSlot->m2C_animQuad, cmdsrca, pConfig->m8_pQuadData);

    // Copy position and velocity
    pSlot->m0_position = *pConfig->m0_pPosition;
    pSlot->mC_velocity = *pConfig->m4_pVelocity;

    // Extra data
    pSlot->m24_updateFunc = pConfig->m14_updateFunc;
    pSlot->m20_heapData = heapData;
    if (heapData != nullptr)
    {
        memcpy(heapData, pConfig->m1C_heapData, pConfig->m18_heapSize);
    }

    // Set draw function and velocity scales
    if (useVelocityScale == 0)
    {
        pSlot->m28_drawFunc = particleDrawSimple;
    }
    else
    {
        pSlot->m18_velocityScaleX = pConfig->mC_velocityScaleX;
        pSlot->m1C_velocityScaleY = pConfig->m10_velocityScaleY;
        pSlot->m28_drawFunc = particleDrawBillboard;
    }

    pPool->m18_activeCount++;
    return 1;
}

// 0605e818
struct sSmokePufTask : public s_workAreaTemplate<sSmokePufTask>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { nullptr, Update, nullptr, nullptr };
        return &taskDefinition;
    }

    static void Update(sSmokePufTask* pThis)
    {
        if (pThis->m14_countdown < 1)
        {
            pThis->getTask()->markFinished();
            return;
        }

        pThis->m14_countdown--;

        // Spawn a smoke particle every other frame
        // TODO: wire up to spawnParticleInPool when particle pool manager is available
        Unimplemented();
    }

    sVec3_FP m0_position;
    sVec3_FP* mC_pPositionRef;
    s32 m10;
    s32 m14_countdown;
    // size 0x18
};

// 0605e928
void createSmokePufTask(p_workArea pThis, sVec3_FP* r5, s32 r6)
{
    sSmokePufTask* pNewTask = createSubTask<sSmokePufTask>(pThis);
    pNewTask->m0_position = *r5;
    pNewTask->mC_pPositionRef = r5;
    pNewTask->m10 = r6;
    pNewTask->m14_countdown = 0x20;
}

void dispatchFunction(s_visdibilityCellTask* r4, s_DataTable2Sub0& r5, s32 r6)
{
    switch (r5.m0_function.m_offset)
    {
        // A0
    case 0x0605e012:
        create_A3_0_Obj0(r4, r5, r6); // static mine carts
        break;
    case 0x06055724:
        create_A3_0_Obj1(r4, r5, r6); // crane upper part
        break;
    case 0x0605580e:
        create_A3_0_Obj2(r4, r5, r6); // crane lower part
        break;
    case 0x0605e608:
        create_A3_0_Obj3(r4, r5, r6); // moving mine carts
        break;
    case 0x060567e4:
        create_A3_0_Obj4(r4, r5, r6);
        break;

        // ropes
    case 0x6060194:
        getFieldSpecificData_A3()->mC[r5.m18] = create_A3_Obj0(r4, r5, r6, 0);
        break;
    case 0x60601C8:
        getFieldSpecificData_A3()->mC[r5.m18 + 5] = create_A3_Obj0(r4, r5, r6, 1);
        break;
    case 0x6060228:
        getFieldSpecificData_A3()->mC[r5.m18 + 11] = create_A3_Obj0(r4, r5, r6, 2);
        break;

        // dangling flags
    case 0x605ffca:
        create_A3_Obj2(r4, r5, r6, 0);
        break;
    case 0x605ffce:
        create_A3_Obj2(r4, r5, r6, 1);
        break;
    case 0x605ffd2:
        create_A3_Obj2(r4, r5, r6, 2);
        break;
    case 0x605FFD6:
        create_A3_Obj2(r4, r5, r6, 3);
        break;

        // sideway flag pole
    case 0x605A94C:
        create_A3_Obj4(r4, r5, r6);
        break;
        // sideway flag
    case 0x605A7A8:
        create_A3_Obj3(r4, r5, r6);
        break;

    case 0x0606053e:
        //create_A3_Obj1(r4, r5, r6); ?
        break;

    case 0x06059674: // A2
        create_A3_2_crashedImperialShip2(r4, r5, r6);
        break;
    case 0x06059A2C: // A2 imperial ship
        create_A3_2_crashedImperialShip(r4, r5, r6);
        break;
    case 0x0605C274: // A2
        break;
    case 0x0605c27a: // A2
        break;
    case 0x0605c280: // A2
        break;
    case 0x0605bfea: // A2
        break;
    case 0x0605c0ae: // A2
        break;
    default:
        PDS_unimplemented("dispatchFunction");
        break;
        //assert(0);
    }
}

void setupFieldSub1Sub0(s_visdibilityCellTask* r4, std::vector<s_DataTable2Sub0>& r5, s32 r6)
{
    for (int i = 0; i < r5.size(); i++)
    {
        dispatchFunction(r4, r5[i], r6);
    }
}

void setupFieldSub1(s_DataTable3* r4, s_DataTable2* r5, void(*r6)(p_workArea workArea))
{
    s_visibilityGridWorkArea* pFieldCameraTask1 = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;

    setupField2(r4, r6);

    if (r5)
    {
        s_visdibilityCellTask** r12 = pFieldCameraTask1->m3C_cellRenderingTasks;
        if (r12)
        {
            s32 r11 = r5->m8.m0 * r5->m8.m4;
            std::vector<std::vector<s_DataTable2Sub0>>::iterator r14 = r5->m0.begin();

            do
            {
                if (r14->size())
                {
                    setupFieldSub1Sub0(*r12, *r14, r5->m4);
                }

                r14++;
                r12++;
            } while (--r11);
        }
    }
}

void setupField(s_DataTable3* r4, s_DataTable2* r5, void(*r6)(p_workArea workArea), std::vector<std::vector<sCameraVisibility>>* r7)
{
    getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1->m34_cameraVisibilityTable = r7;

    setupFieldSub1(r4, r5, r6);
}

void setupField3(s_DataTable3* r4, void(*r5)(p_workArea workArea), std::vector<std::vector<sCameraVisibility>>* r6)
{
    getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1->m34_cameraVisibilityTable = r6;

    setupField2(r4, r5);
}

void initDragonMovementMode()
{
    s_dragonTaskWorkArea* r14 = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    r14->m154_dragonSpeed = 0;
    initDragonSpeed(0);

    if (r14->m1D0_cameraScript)
    {
        r14->mF0 = dragonScriptMovement;
    }
    else if (r14->m1D4_cutsceneData)
    {
        r14->mF0 = dragonCutsceneUpdate;
    }
    else
    {
        r14->mF0 = dragonIdleUpdate;
    }

    r14->m104_dragonScriptStatus = 0;

    getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->mF8_Flags |= 0x10000;
}

void subfieldA3_1Sub0Sub2(s32 r4, s32 r5)
{
    exitCutsceneTaskUpdateSub0Sub0();
    exitCutsceneTaskUpdateSub0Sub1(getFieldTaskPtr()->m2C_currentFieldIndex, r4, 0, r5);
}


void setupDragonPositionSub0(const sVec3_FP* r4, const sVec3_FP* r5)
{
    if (r4)
    {
        assert(0);
    }
}

void setupDragonPosition(const sVec3_FP* r4, const sVec3_FP* r5)
{
    s_dragonTaskWorkArea* pDragonTask = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    sFieldCameraStatus* r15 = getFieldCameraStatus();

    if (r4)
    {
        pDragonTask->m8_pos[0] = (*r4)[0];
        pDragonTask->m8_pos[1] = (*r4)[1];
        pDragonTask->m8_pos[2] = (*r4)[2];
    }

    if (r5)
    {
        pDragonTask->m20_angle[0] = (*r5)[0];
        pDragonTask->m20_angle[1] = (*r5)[1];
        pDragonTask->m20_angle[2] = (*r5)[2];

        setupDragonPositionSub0(pDragonTask->mBC, r5);
    }
}

void setupDragonPosition(const sVec3_FP& r4, const sVec3_FP& r5) {setupDragonPosition(&r4, &r5);}


s32 queueNewFieldScript(sSaturnPtr r4, s32 r5)
{
    s_fieldScriptWorkArea* r13 = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE;
    if (r13->m4_currentScript.m_offset)
        return 0;

    if ((r5 >= 0) && mainGameState.getBit566(r5))
    {
        return 0;
    }

    r13->m60_canSkipScript = 0;
    r13->m4_currentScript = r4;
    r13->m2C_bitToSet = r5;
    r13->m58 = 0;
    r13->m50_scriptDelay = 0;

    return 1;
}

s32 queueNewFieldScriptSkippable(sSaturnPtr r4, s32 r5)
{
    s_fieldScriptWorkArea* r14 = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE;

    if (r14->m4_currentScript.m_offset)
        return 0;

    if ((r5 >= 0) && mainGameState.getBit566(r5))
    {
        r14->m60_canSkipScript = 1;
    }
    else
    {
        r14->m60_canSkipScript = 0;
    }

    r14->m4_currentScript = r4;
    r14->m2C_bitToSet = r5;
    r14->m58 = 0;
    r14->m50_scriptDelay = 0;
    return 1;
}

s32 startFieldScript(s32 r4, s32 r5)
{
    s_fieldScriptWorkArea* r14 = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE;
    if (r14)
    {
        return queueNewFieldScript(r14->m0_pScripts[r4], r5);
    }
    return 0;
}

s32 startFieldScriptSkippable(s32 r4, s32 r5)
{
    s_fieldScriptWorkArea* r14 = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE;
    if (r14)
    {
        return queueNewFieldScriptSkippable(r14->m0_pScripts[r4], r5);
    }
    return 0;
}

s_cameraScript* readCameraScript(sSaturnPtr EA)
{
    static std::unordered_map<u32, s_cameraScript*> cache;

    std::unordered_map<u32, s_cameraScript*>::iterator cacheEntry = cache.find(EA.m_offset);
    if (cacheEntry != cache.end())
    {
        return cacheEntry->second;
    }

    s_cameraScript* pNewCameraScript = new s_cameraScript;

    pNewCameraScript->m0_position[0] = readSaturnS32(EA + 0x00);
    pNewCameraScript->m0_position[1] = readSaturnS32(EA + 0x04);
    pNewCameraScript->m0_position[2] = readSaturnS32(EA + 0x08);

    pNewCameraScript->mC_rotation[0] = readSaturnS32(EA + 0x0C);
    pNewCameraScript->mC_rotation[1] = readSaturnS32(EA + 0x10);
    pNewCameraScript->mC_rotation[2] = readSaturnS32(EA + 0x14);

    pNewCameraScript->m18 = readSaturnS32(EA + 0x18);
    pNewCameraScript->m1C = readSaturnS32(EA + 0x1C);
    pNewCameraScript->m20_length = readSaturnS32(EA + 0x20);

    pNewCameraScript->m24_pos2[0] = readSaturnS32(EA + 0x24);
    pNewCameraScript->m24_pos2[1] = readSaturnS32(EA + 0x28);
    pNewCameraScript->m24_pos2[2] = readSaturnS32(EA + 0x2C);

    pNewCameraScript->m30_thresholdDistance = readSaturnS32(EA + 0x30);

    cache.insert_or_assign(EA.m_offset, pNewCameraScript);

    return pNewCameraScript;
}


std::vector<s_grid1> readEnvironmentGridCell(sSaturnPtr gridCellEA)
{
    assert(gridCellEA.m_offset);

    // counter number of entries in cell
    u32 numEntries = 0;
    while (readSaturnU32(gridCellEA))
    {
        gridCellEA = gridCellEA + 0x18;
        numEntries++;
    }
    gridCellEA = gridCellEA + (-0x18 * numEntries);

    std::vector<s_grid1> pCellArray;
    pCellArray.resize(numEntries);

    for (int i = 0; i < numEntries; i++)
    {
        pCellArray[i].EA = gridCellEA;
        sSaturnPtr modelsArrayStart = readSaturnEA(gridCellEA); gridCellEA = gridCellEA + 4;
        for (int j = 0; j < 5; j++)
        {
            pCellArray[i].m0_modelsOffets[j] = readSaturnU16(modelsArrayStart + j * 2);
        }
        pCellArray[i].m4 = readSaturnVec3(gridCellEA); gridCellEA = gridCellEA + 4 * 3;
        pCellArray[i].m10[0] = readSaturnS16(gridCellEA); gridCellEA = gridCellEA + 2;
        pCellArray[i].m10[1] = readSaturnS16(gridCellEA); gridCellEA = gridCellEA + 2;
        pCellArray[i].m10[2] = readSaturnS16(gridCellEA); gridCellEA = gridCellEA + 2;
        pCellArray[i].m16 = readSaturnS16(gridCellEA); gridCellEA = gridCellEA + 2;
    }

    return pCellArray;
}

std::vector<std::vector<s_grid1>> readEnvironmentGrid(sSaturnPtr gridEA, u32 gridWidth, u32 gridHeight)
{
    std::vector<std::vector<s_grid1>> pGrid;
    if (gridEA.m_offset == 0)
        return pGrid;

    pGrid.resize(gridWidth * gridHeight);

    for (int i = 0; i < gridWidth * gridHeight; i++)
    {
        sSaturnPtr cellEA = readSaturnEA(gridEA); gridEA = gridEA + 4;
        if (cellEA.m_offset)
        {
            pGrid[i] = readEnvironmentGridCell(cellEA);
        }
    }

    return pGrid;
}

std::vector<s_grid2> readGrid2Cell(sSaturnPtr gridCellEA)
{
    assert(gridCellEA.m_offset);

    // counter number of entries in cell
    u32 numEntries = 0;
    while (readSaturnU32(gridCellEA))
    {
        gridCellEA = gridCellEA + 0x10;
        numEntries++;
    }
    gridCellEA = gridCellEA + (-0x10 * numEntries);

    std::vector<s_grid2> pCellArray;
    pCellArray.resize(numEntries);

    for (int i = 0; i < numEntries; i++)
    {
        pCellArray[i].m0 = readSaturnEA(gridCellEA); gridCellEA = gridCellEA + 4;
        pCellArray[i].m4 = readSaturnVec3(gridCellEA); gridCellEA = gridCellEA + 4 * 3;
    }

    return pCellArray;
}

std::vector<std::vector<s_grid2>> readGrid2(sSaturnPtr gridEA, u32 gridWidth, u32 gridHeight)
{
    std::vector<std::vector<s_grid2>> pGrid;
    if (gridEA.m_offset == 0)
        return pGrid;

    pGrid.resize(gridWidth*gridHeight);
    for (int i = 0; i < gridWidth * gridHeight; i++)
    {
        sSaturnPtr cellEA = readSaturnEA(gridEA); gridEA += 4;
        if (cellEA.m_offset)
        {
            pGrid[i] = readGrid2Cell(cellEA);
        }
    }

    return pGrid;
}

s_grid3** readGrid3(sSaturnPtr gridEA, u32 gridWidth, u32 gridHeight)
{
    if (gridEA.m_offset != 0)
    {
        PDS_unimplemented("readGrid3");
    }
    return NULL;
}

s_DataTable3* readDataTable3(sSaturnPtr EA)
{
    sSaturnPtr grid1 = readSaturnEA(EA); EA = EA + 4;
    sSaturnPtr grid2 = readSaturnEA(EA); EA = EA + 4;
    sSaturnPtr grid3 = readSaturnEA(EA); EA = EA + 4;

    s_DataTable3* pNewData3 = new s_DataTable3;

    pNewData3->mC = readSaturnU32(EA); EA = EA + 4;
    pNewData3->m10_gridSize[0] = readSaturnS32(EA); EA = EA + 4;
    pNewData3->m10_gridSize[1] = readSaturnS32(EA); EA = EA + 4;
    pNewData3->m18_cellDimensions[0] = readSaturnU32(EA); EA = EA + 4;
    pNewData3->m18_cellDimensions[1] = readSaturnU32(EA); EA = EA + 4;
    pNewData3->m20 = readSaturnU32(EA); EA = EA + 4;

    pNewData3->m0_environmentGrid = readEnvironmentGrid(grid1, pNewData3->m10_gridSize[0], pNewData3->m10_gridSize[1]);
    pNewData3->m4 = readGrid2(grid2, pNewData3->m10_gridSize[0], pNewData3->m10_gridSize[1]);
    pNewData3->m8 = readGrid3(grid3, pNewData3->m10_gridSize[0], pNewData3->m10_gridSize[1]);

    return pNewData3;
}

s_DataTable2* readDataTable2(sSaturnPtr EA)
{
    s_DataTable2* pNewData2 = new s_DataTable2;

    sSaturnPtr gridEA = readSaturnEA(EA); EA = EA + 4;

    pNewData2->m4 = readSaturnU32(EA); EA = EA + 4;
    pNewData2->m8.m0 = readSaturnU32(EA); EA = EA + 4;
    pNewData2->m8.m4 = readSaturnU32(EA); EA = EA + 4;

    int gridSize = pNewData2->m8.m0 * pNewData2->m8.m4;
    pNewData2->m0.resize(gridSize);

    for (int i = 0; i < gridSize; i++)
    {
        sSaturnPtr cellEA = readSaturnEA(gridEA + i * 4);
        if (cellEA.m_offset)
        {
            while (readSaturnEA(cellEA).m_offset)
            {
                s_DataTable2Sub0 newEntry;
                newEntry.m0_function = readSaturnEA(cellEA);
                newEntry.m4_position = readSaturnVec3(cellEA + 4);
                newEntry.m10_rotation[0] = readSaturnS16(cellEA + 0x10);
                newEntry.m10_rotation[1] = readSaturnS16(cellEA + 0x12);
                newEntry.m10_rotation[2] = readSaturnS16(cellEA + 0x14);
                newEntry.m18 = readSaturnS32(cellEA + 0x18);
                newEntry.m1C_modelData = readSaturnEA(cellEA + 0x1C);

                pNewData2->m0[i].push_back(newEntry);

                cellEA += 0x20;
            }
        }
    }

    return pNewData2;
}

void fieldPaletteTaskInitSub0Sub0()
{
    vdp2Controls.m4_pendingVdp2Regs->m26_SFCODE = 0;
    vdp2Controls.m4_pendingVdp2Regs->m14_CYCA1 = 1;
}

void createNGB1DefaultBlackCell()
{
    for (int i = 0; i < 4 * 4; i++)
    {
        setVdp2VramU32(0x5C00 + i * 4, 0x509A509A);
    }
}

void resetNBG1Map()
{
    memset(getVdp2Vram(0x5800), 0, 0x80 * 4);
    createNGB1DefaultBlackCell();
}

// TODO: kernel
void initNBG1Layer()
{
    fieldPaletteTaskInitSub0Sub0();

    static const sLayerConfig setup[] =
    {
        m2_CHCN,  0, // 16 colors
        m5_CHSZ,  1, // character size is 2 cells x 2 cells (16*16)
        m6_PNB,  1, // pattern data size is 1 word
        m12_PLSZ, 0, // plane is 1H x 1V
        m46_SCCM, 2, // special color calculation mode 2 (per dot)
        m44_CCEN, 1, // color calculation mode enabled
        m41, 1,
        m40_CAOS, 7,
        m9_SCC, 1,
        m45_COEN, 16, // color offset mode enabled
        m21_LCSY, 1, // enable vertical scroll (for cutscene bars)
        m0_END,
    };

    setupNBG1(setup);

    initLayerMap(1, 0x5800, 0x5800, 0x5800, 0x5800);

    writeCinematicBarsToVdp2();
    resetNBG1Map();
}

// 06058978
void s_fieldPaletteTaskWorkArea::Init(s_fieldPaletteTaskWorkArea* pThis)
{
    getFieldTaskPtr()->m8_pSubFieldData->m350_fieldPaletteTask = pThis;

    reinitVdp2();
    initNBG1Layer();

    pThis->m78 = (s_fieldPaletteTaskWorkSub*)allocateHeapForTask(pThis, sizeof(s_fieldPaletteTaskWorkSub));
    pThis->m78->m0 = 1;
    pThis->m78->m4 = 0;
    pThis->m78->m8 = -0xF78000;
    pThis->m78->mC = -0x1194000;
    pThis->m78->m10 = -0xEC4000;
    pThis->m78->m14 = -0x1054000;

    asyncDmaCopy({ 0x060900A4, gFLD_A3 }, getVdp2Cram(0), 0x80, 0);
    asyncDmaCopy({ 0x06090124, gFLD_A3 }, getVdp2Cram(0x80), 0x80, 0);
    asyncDmaCopy({ 0x0608FC84, gFLD_A3 }, getVdp2Cram(0xE0), 0x20, 0);
    asyncDmaCopy({ 0x06090184, gFLD_A3 }, getVdp2Cram(0x600), 0x20, 0);
    asyncDmaCopy({ 0x060901A4, gFLD_A3 }, getVdp2Cram(0x620), 0x20, 0);
    asyncDmaCopy({ 0x0608FA84, gFLD_A3 }, getVdp2Cram(0x400), 0x200, 0);
    asyncDmaCopy({ 0x060901C4, gFLD_A3 }, getVdp2Cram(0x800), 0x200, 0);
    asyncDmaCopy({ 0x0608FCA4, gFLD_A3 }, getVdp2Cram(0xA00), 0x200, 0);
    asyncDmaCopy({ 0x0608FEA4, gFLD_A3 }, getVdp2Cram(0xC00), 0x200, 0);

    static const sLayerConfig rgbSetup[] = {
        {m2_CHCN, 1}, {m5_CHSZ, 1}, {m6_PNB, 1}, {m7_CNSM, 0},
        {m27_RPMD, 2}, {m11_SCN, 8}, {m34_W0E, 1}, {m37_W0A, 1},
        {m0_END},
    };
    setupRGB0(rgbSetup);
    static const sLayerConfig rotParams[] = {
        {m31_RxKTE, 1},
        {m0_END},
    };
    setupRotationParams(rotParams);
    static const sLayerConfig rotParams2[] = {
        {m0_END},
    };
    setupRotationParams2(rotParams2);
    static const sLayerConfig nbg0Setup[] = {
        {m2_CHCN, 1}, {m5_CHSZ, 1}, {m6_PNB, 1}, {m7_CNSM, 0},
        {m44_CCEN, 1}, {m34_W0E, 1}, {m37_W0A, 1},
        {m0_END},
    };
    setupNBG0(nbg0Setup);

    vdp2Controls.m4_pendingVdp2Regs->m20_BGON &= 0xFFFE;
    vdp2Controls.m_isDirty = 1;

    loadFile("FNS_A3_0.SCB", getVdp2Vram(0x10000), 0);
    loadFile("FNS_A3_0.PNB", getVdp2Vram(0x1E000), 0);
    loadFile("FRS_A3_0.SCB", getVdp2Vram(0x40000), 0);
    loadFile("FRS_A3_0.PNB", getVdp2Vram(0x60000), 0);
    loadFile("SPACE.PNB", getVdp2Vram(0x64000), 0);

    vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL = (vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL & 0xFF00) | 0xB4;
    vdp2Controls.m4_pendingVdp2Regs->m10_CYCA0 = 0x310F7544;
    vdp2Controls.m_isDirty = 1;

    setupRotationMapPlanes(0, gFLD_A3->getSaturnPtr(0x06090404));
    setupRotationMapPlanes(1, gFLD_A3->getSaturnPtr(0x060903C4));
    initLayerMap(0, 0x1E000, 0x1E000, 0x1E000, 0x1E000);
    setupVdp2Table(6, coefficientA0, coefficientA1, getVdp2Vram(0x20000), 0x80);
    setupVdp2Table(7, coefficientB0, coefficientB1, getVdp2Vram(0x22000), 0x80);

    s_BTL_A3_Env_InitVdp2Sub3(5, getVdp2Vram(0x2A000));
    setVdp2VramU16(0x25E2A400, 0x700);
    vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA = (vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA & 0xFFF80000) | 0x15200;

    setVdp2VramU16(0x25E2A600, 0);
    vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA = (vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA & 0xFFF80000) | 0x15300;

    vdp2Controls.m4_pendingVdp2Regs->mF0_PRISA = 0x304;
    vdp2Controls.m4_pendingVdp2Regs->mF2_PRISB = 0x407;
    vdp2Controls.m4_pendingVdp2Regs->mF4_PRISC = 0x404;
    vdp2Controls.m4_pendingVdp2Regs->mF6_PRISD = 0x404;
    vdp2Controls.m4_pendingVdp2Regs->mF8_PRINA = 0x605;
    vdp2Controls.m4_pendingVdp2Regs->mFA_PRINB = 0x700;
    vdp2Controls.m4_pendingVdp2Regs->mFC_PRIR = 3;

    pThis->m3C_scale = fixedPoint(0x10000);
    pThis->m38_groundY = fixedPoint(0xFFFF8000);

    vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL = vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL & 0xFEFF;
    vdp2Controls.m_isDirty = 1;

    static const std::vector<std::array<s32, 2>> layerDisplay = {
        {m44_CCEN, 1},
    };
    applyLayerDisplayConfig(layerDisplay);

    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xF8FF) | 0x300;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xFFF0) | 3;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xCFFF) | 0x1000;

    pThis->m70_colorR = 0x10;
    pThis->m71_colorG = 0x12;
    pThis->m72_colorB = 0x14;

    auto* regs = vdp2Controls.m4_pendingVdp2Regs;
    regs->m100_CCRSA = (s16)pThis->m70_colorR | ((s16)pThis->m71_colorG << 8);
    regs->m102_CCRSB = (s16)pThis->m72_colorB;
    regs->m104_CCRSC = 0;
    regs->m106_CCRSD = 0;

    pThis->m75_colorRBG0 = 8;
    regs->m10C_CCRR = (s16)pThis->m75_colorRBG0;

    pThis->m74_colorNBG = 0x14;
    regs->m108_CCRNA = (regs->m108_CCRNA & 0xFFE0) | (s16)pThis->m74_colorNBG;
    vdp2Controls.m_isDirty = 1;

    pThis->m4C_wavePhase = 0;
    pThis->m40_waveSpeed = 0x0021EFCB;
    pThis->m44_waveFreq = 0x4D5E540;
    pThis->m48_waveAmplitude = 0xF5A;
}

// 060590ae
static void fieldPaletteDrawPass0Sub(s_fieldPaletteTaskWorkArea* pThis)
{
    sCoefficientTableData& t = gCoefficientTables[gRotationPassState.m0_planeIndex][(s32)vdp2Controls.m0_doubleBufferIndex];

    fixedPoint rotX = pThis->m18_cameraRotation.m0_X;
    if ((s32)rotX == 0) rotX = fixedPoint(0xFFF49F4A);
    fixedPoint rotY = pThis->m18_cameraRotation.m4_Y;
    fixedPoint rotZ = pThis->m18_cameraRotation.m8_Z;

    s32 sumX = (s32)pThis->m24_vdp1Clipping[0] + (s32)pThis->m24_vdp1Clipping[2];
    t.m34 = (s16)((sumX + (int)(sumX < 0)) >> 1);
    s32 sumY = (s32)pThis->m24_vdp1Clipping[1] + (s32)pThis->m24_vdp1Clipping[3];
    t.m36 = (s16)((sumY + (int)(sumY < 0)) >> 1);
    t.m38 = pThis->m32_projParam1;
    t.m3C = t.m34;
    t.m3E = t.m36;
    t.m40 = 0;

    buildRotationMatrixPitchYaw(-0x4000000 - rotX, -rotY);
    scaleRotationMatrix(pThis->m3C_scale);
    writeRotationParams(-rotZ);

    s32 diffX = (s32)t.m34 - (s32)t.m3C;
    s32 diffY = (s32)t.m36 - (s32)t.m3E;
    s32 diffZ = (s32)t.m38 - (s32)t.m40;

    gVdp2RotationMatrix.Mx = MTH_Mul(pThis->m3C_scale, (s32)pThis->mC_cameraPosition.m0_X << 4)
                    - gVdp2RotationMatrix.m[0][0] * diffX - gVdp2RotationMatrix.m[0][1] * diffY - gVdp2RotationMatrix.m[0][2] * diffZ
                    + (s32)(s16)t.m3C * -0x10000;
    gVdp2RotationMatrix.My = MTH_Mul(pThis->m3C_scale, (s32)pThis->mC_cameraPosition.m8_Z << 4)
                    - gVdp2RotationMatrix.m[1][0] * diffX - gVdp2RotationMatrix.m[1][1] * diffY - gVdp2RotationMatrix.m[1][2] * diffZ
                    + (s32)(s16)t.m3E * -0x10000;
    gVdp2RotationMatrix.Mz = ((pThis->mC_cameraPosition.m4_Y - pThis->m38_groundY) * 0x10)
                    - gVdp2RotationMatrix.m[2][0] * diffX - gVdp2RotationMatrix.m[2][1] * diffY - gVdp2RotationMatrix.m[2][2] * diffZ
                    + (s32)(s16)t.m40 * -0x10000;
}

// 060588e0
static void fieldPaletteWaveDistortion(s_fieldPaletteTaskWorkArea* pThis)
{
    std::vector<fixedPoint>& coefficients = *gVdp2CoefficientTables[gRotationPassState.m0_planeIndex][vdp2Controls.m0_doubleBufferIndex];
    s32 phase = pThis->m4C_wavePhase;
    for (int i = 0; i < 0x1A8 && i < (int)coefficients.size(); i++)
    {
        s32 sinVal = getSin((u16)((u32)phase >> 16) & 0xFFF);
        fixedPoint modulated = MTH_Mul(pThis->m48_waveAmplitude, sinVal);
        coefficients[i] = MTH_Mul(coefficients[i], modulated + 0x10000);
        phase += pThis->m44_waveFreq;
    }
    pThis->m4C_wavePhase += pThis->m40_waveSpeed;
}

// 06058d94
static void fieldPaletteNegateCoefficients()
{
    std::vector<fixedPoint>& coefficients = *gVdp2CoefficientTables[gRotationPassState.m0_planeIndex][vdp2Controls.m0_doubleBufferIndex];
    for (int i = 0; i < 0x1A8 && i < (int)coefficients.size(); i++)
    {
        coefficients[i] = fixedPoint((u32)coefficients[i] | 0x80000000);
    }
}

// 06058dba
void s_fieldPaletteTaskWorkArea::Draw(s_fieldPaletteTaskWorkArea* pThis)
{
    s_fieldPaletteTaskWorkSub* pSub = pThis->m78;

    pThis->mC_cameraPosition = cameraProperties2.m0_position;
    pThis->m18_cameraRotation = cameraProperties2.mC_rotation.toSVec3_FP();

    getVdp1ClippingCoordinates(pThis->m24_vdp1Clipping);
    getVdp1LocalCoordinates(pThis->m2C_localCoordinates);
    getVdp1ProjectionParams(&pThis->m30_projParam0, &pThis->m32_projParam1);

    // Subfield 2: dynamic rotation map plane switching based on camera
    if (getFieldTaskPtr()->m2E_currentSubFieldIndex == 2)
    {
        if (pThis->mC_cameraPosition.m4_Y < 1)
        {
            pSub->m4 = 0;
        }
        else
        {
            // Sign-extend 28-bit angle
            s32 rotY = (s32)pThis->m18_cameraRotation.m4_Y;
            s32 absRotY = (rotY & 0x8000000) ? (rotY | 0xF0000000) : (rotY & 0xFFFFFFF);
            if (absRotY < 0) absRotY = -absRotY;

            if (absRotY < 0x4000000)
            {
                if (pSub->m10 < (s32)pThis->mC_cameraPosition.m8_Z) { pSub->m4 = 0; }
                else if (pSub->m14 < (s32)pThis->mC_cameraPosition.m8_Z) { pSub->m4 = -1; }
                else { pSub->m4 = -2; }
            }
            else
            {
                if (pSub->m8 < (s32)pThis->mC_cameraPosition.m8_Z) { pSub->m4 = 0; }
                else if (pSub->mC < (s32)pThis->mC_cameraPosition.m8_Z) { pSub->m4 = 1; }
                else { pSub->m4 = 2; }
            }
        }

        s32 mode = pSub->m4;
        if (mode == -2 || mode == 2)
        {
            pSub->m0 = 0;
            setupRotationMapPlanes(0, gFLD_A3->getSaturnPtr(0x06090444));
        }
        else if (mode == 0)
        {
            pSub->m0 = 1;
            setupRotationMapPlanes(0, gFLD_A3->getSaturnPtr(0x06090404));
        }
        else if (mode == -1 || mode == 1)
        {
            pSub->m0 = 0;
            setupRotationMapPlanes(0, gFLD_A3->getSaturnPtr(0x06090404));
        }
    }

    // Pass 0: ground plane
    beginRotationPass(0, intDivide(pThis->m30_projParam0, fixedPoint::fromInteger(pThis->m32_projParam1)));
    fieldPaletteDrawPass0Sub(pThis);
    drawCinematicBar(6);
    commitRotationPass();

    pThis->m34_scrollValue = computeRotationScrollOffset();

    if (pThis->m78->m0 == 0)
    {
        fieldPaletteWaveDistortion(pThis);
    }
    else
    {
        fieldPaletteNegateCoefficients();
    }

    // Pass 1: sky scroll
    pThis->m0_scrollX = ((s32)pThis->m18_cameraRotation.m4_Y >> 0xC) * -0x400;
    pThis->m4_scrollY = (0x200 - pThis->m34_scrollValue) * 0x10000;

    beginRotationPass(1, intDivide(pThis->m30_projParam0, fixedPoint::fromInteger(pThis->m32_projParam1)));

    sCoefficientTableData& t = gCoefficientTables[gRotationPassState.m0_planeIndex][(s32)vdp2Controls.m0_doubleBufferIndex];
    s32 iX = (s32)pThis->m24_vdp1Clipping[0] + (s32)pThis->m24_vdp1Clipping[2];
    t.m34 = (s16)((iX + (int)(iX < 0)) >> 1);
    s32 iY = (s32)pThis->m24_vdp1Clipping[1] + (s32)pThis->m24_vdp1Clipping[3];
    t.m36 = (s16)((iY + (int)(iY < 0)) >> 1);
    t.m38 = pThis->m32_projParam1;
    t.m3C = t.m34;
    t.m3E = t.m36;
    t.m40 = 0;

    buildRotationMatrixRoll(-pThis->m18_cameraRotation.m8_Z);
    intDivide(pThis->m24_vdp1Clipping[3] - pThis->m24_vdp1Clipping[1], 0x00E00000);
    scaleRotationMatrix(intDivide(pThis->m24_vdp1Clipping[2] - pThis->m24_vdp1Clipping[0], 0x01600000));
    setRotationScrollOffset(pThis->m0_scrollX, pThis->m4_scrollY);
    commitRotationPass();

    auto* regs = vdp2Controls.m4_pendingVdp2Regs;
    regs->mC0_WPSX0 = pThis->m24_vdp1Clipping[0] << 1;
    regs->mC2_WPSY0 = pThis->m24_vdp1Clipping[1];
    regs->mC4_WPEX0 = pThis->m24_vdp1Clipping[2] << 1;
    regs->mC6_WPEY0 = pThis->m24_vdp1Clipping[3];
}

void createFieldPaletteTask(p_workArea parent)
{
    createSubTask<s_fieldPaletteTaskWorkArea>(parent);
}

void adjustVerticalLimits(fixedPoint r4, fixedPoint r5)
{
    getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m134_minY = r4;
    getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m140_maxY = r5;
}

u32 cutsceneTaskInitSub0(std::vector<s_scriptData3>& r4, std::vector<s_scriptData3>& r5)
{
    if (r4.size() == 0)
        return 0;

    u32 r6 = 0;
    int i = 0;
    while (r4[i].m0_duration)
    {
        r6 += r4[i].m0_duration;
        r5[i] = r4[i];

        i++;
    }

    r5[i].m0_duration = r4[i].m0_duration;
    return r6;
}

void dragonCutsceneUpdate(s_dragonTaskWorkArea* r14)
{
    r14->m24A_runningCameraScript = 4;
    s_scriptData3* r13 = r14->m1E4_cutsceneKeyFrame;
    if (r13 == NULL)
    {
        r14->m104_dragonScriptStatus = 3;
    }

    getFieldTaskPtr()->m28_status |= 0x10000;
    getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m8 |= 1;

    switch (r14->m104_dragonScriptStatus)
    {
    case 0:
        r14->mF8_Flags &= ~0x400;
        r14->mF8_Flags |= 0x20000;
        r14->m1E8_cameraScriptDelay = r13->m0_duration;
        r14->m1EA = r13->m10_rotationDuration;
        r14->m160_deltaTranslation[0] = intDivide(r13->m0_duration, r13->m4_pos[0] - r14->m8_pos[0]);
        r14->m160_deltaTranslation[1] = intDivide(r13->m0_duration, r13->m4_pos[1] - r14->m8_pos[1]);
        r14->m160_deltaTranslation[2] = intDivide(r13->m0_duration, r13->m4_pos[2] - r14->m8_pos[2]);

        if (r13->m10_rotationDuration)
        {
            r14->m16C_deltaRotation[0] = intDivide(r13->m10_rotationDuration, fixedPoint(r13->m14_rot[0] - r14->m20_angle[0]).normalized());
            r14->m16C_deltaRotation[1] = intDivide(r13->m10_rotationDuration, fixedPoint(r13->m14_rot[1] - r14->m20_angle[1]).normalized());
            r14->m16C_deltaRotation[2] = intDivide(r13->m10_rotationDuration, fixedPoint(r13->m14_rot[2] - r14->m20_angle[2]).normalized());
        }
        else
        {
            r14->m16C_deltaRotation[0] = 0;
            r14->m16C_deltaRotation[1] = 0;
            r14->m16C_deltaRotation[2] = 0;
        }

        updateDragonCollision(r14);
        if (r14->mF8_Flags & 0x40000)
        {
            updateCameraScriptSub0Sub2(r14);
            updateCameraScriptSub0(r14->mB8_lightWingEffect);
            r14->mF8_Flags &= ~0x40000;
        }
        r14->m104_dragonScriptStatus++;
    case 1:

        r14->m20_angle += r14->m16C_deltaRotation;
        r14->m8_pos += r14->m160_deltaTranslation;

        if (--r14->m1EA <= 0)
        {
            r14->m104_dragonScriptStatus++;
        }

        if (--r14->m1E8_cameraScriptDelay <= 0)
        {
            r14->m104_dragonScriptStatus = 3;
            r14->m1E4_cutsceneKeyFrame = NULL;
        }
        break;
    case 2:
        r14->m8_pos += r14->m160_deltaTranslation;
        if (--r14->m1E8_cameraScriptDelay <= 0)
        {
            r14->m104_dragonScriptStatus = 3;
            r14->m1E4_cutsceneKeyFrame = NULL;
        }
        break;
    case 3:
        break;
    default:
        assert(0);
        break;
    }

    buildDragonRotationMatrix(&r14->m48, &r14->m20_angle);
    copyMatrix(&r14->m48.m0_matrix, &r14->m88_matrix);

    if (--r14->m1EE < 0)
    {
        r14->m1EE = 0;
    }
    updateDragonCollision(r14);
}

void dragonIdleUpdate(s_dragonTaskWorkArea*)
{
    PDS_unimplemented("dragonIdleUpdate");
}

void cutsceneTaskInitSub1(s_scriptData3* r15)
{
    s_dragonTaskWorkArea* r4 = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    r4->m1E4_cutsceneKeyFrame = r15;
    r4->m104_dragonScriptStatus = 0;

    r4->mF0 = dragonCutsceneUpdate;
    r4->mF8_Flags &= ~0x400;
}

// 060625d8
void updateCutsceneCameraInterpolation(sFieldCameraManager* r4, sFieldCameraStatus* r5)
{
    r4->m378_cutsceneFrameCounter++;
    r5->m5C_rotationSpring[0] = interpolateRotation(r5->m5C_rotationSpring[0], 0, 0x2000, 0x444444, 0);
    r5->m5C_rotationSpring[1] = interpolateRotation(r5->m5C_rotationSpring[1], 0, 0x2000, 0x444444, 0);
    r5->m5C_rotationSpring[2] = interpolateRotation(r5->m5C_rotationSpring[2], 0, 0x2000, 0x444444, 0);

    r5->m5C_rotationSpring += r5->m68_rotationImpulse;
    r5->m68_rotationImpulse.zeroize();

    r5->m44_positionSpring[0] = interpolateDistance(r5->m44_positionSpring[0], 0, 0x2000, 0xAAA, 0);
    r5->m44_positionSpring[1] = interpolateDistance(r5->m44_positionSpring[1], 0, 0x2000, 0xAAA, 0);
    r5->m44_positionSpring[2] = interpolateDistance(r5->m44_positionSpring[2], 0, 0x2000, 0xAAA, 0);

    r5->m44_positionSpring += r5->m50_positionImpulse;
    r5->m50_positionImpulse.zeroize();

    r5->m0_position = r5->m44_positionSpring + *r4->m370_cutsceneLookAtPtr;

    sVec3_FP var18;
    var18 = *r4->m374_cutsceneCameraPos - *r4->m370_cutsceneLookAtPtr;

    sVec2_FP dummy;
    computeLookAt(var18, dummy);

    r5->mC_rotation[0] = r5->m5C_rotationSpring[0] + dummy[0];
    r5->mC_rotation[1] = r5->m5C_rotationSpring[1] + dummy[1];
    r5->mC_rotation[2] += r5->m5C_rotationSpring[2];

    r5->m24_distanceToDestination = vecDistance(*r4->m370_cutsceneLookAtPtr, *r4->m374_cutsceneCameraPos);
}

void startCutsceneCameraTracking(sVec3_FP* r4, sVec3_FP* r5)
{
    sFieldCameraManager* r14 = getFieldTaskPtr()->m8_pSubFieldData->m334;
    r14->m370_cutsceneLookAtPtr = r4;
    r14->m374_cutsceneCameraPos = r5;
    r14->m378_cutsceneFrameCounter = 0;
    r14->m37C_isCutsceneCameraActive = 1;

    initDragonMovementMode();

    initCameraSlotWithFunctions(1, NULL, NULL);
    fieldOverlaySubTaskInitSub3(1);

    updateCutsceneCameraInterpolation(r14, getFieldCameraStatus());
}

void cutsceneTaskInitSub2Sub1(std::vector<s_scriptData1>& r4, std::vector<s_scriptData1>& r5)
{
    r5 = r4;
}

void cutsceneTaskInitSub2Sub2(s_workArea* r4)
{
    if (r4)
    {
        r4->getTask()->markFinished();
    }

    if (getFieldTaskPtr()->m8_pSubFieldData)
    {
        getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m48_cutsceneTask = NULL;
    }
}

void s_cutsceneTask2::Init(s_cutsceneTask2* pThis, std::vector<s_scriptData1>* argument)
{
    getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m48_cutsceneTask = pThis;
    pThis->m4 = argument;
}

s32 s_cutsceneTask2::UpdateSub0()
{
    m3C = &(*m4)[m38];
    m40 = m3C->m0;
    if (m40 == 0)
    {
        return -1;
    }

    m44 = m3C->m1C - m3C->m4;
    m50 = m3C->m28 - m3C->m10;

    m44[0] = intDivide(m40, m44[0]);
    m44[1] = intDivide(m40, m44[1]);
    m44[2] = intDivide(m40, m44[2]);

    m50[0] = intDivide(m40, m50[0]);
    m50[1] = intDivide(m40, m50[1]);
    m50[2] = intDivide(m40, m50[2]);

    m5C = intDivide(m40, fixedPoint(m38 - m34).normalized());

    m8 = m3C->m4;
    m28 = m3C->m10;
    m20 = m3C->m34;

    if (m24)
    {
        m14 = m28 + *m24;
    }
    else
    {
        m14 = m28;
    }

    return 1;
}

void endCutsceneCameraWithRestore()
{
    sFieldCameraManager* r13 = getFieldTaskPtr()->m8_pSubFieldData->m334;
    r13->m378_cutsceneFrameCounter = 0;
    r13->m37C_isCutsceneCameraActive = 0;
    dragonFieldTaskInitSub4Sub4();
    fieldOverlaySubTaskInitSub5(1);

    r13->m3E4_cameraSlots[0].m0_position = r13->m3E4_cameraSlots[1].m0_position;
    r13->m3E4_cameraSlots[0].mC_rotation = r13->m3E4_cameraSlots[1].mC_rotation;
    r13->m3E4_cameraSlots[0].m18 = r13->m3E4_cameraSlots[1].m18;
    r13->m3E4_cameraSlots[0].m1C = r13->m3E4_cameraSlots[1].m1C;
    r13->m3E4_cameraSlots[0].m20 = r13->m3E4_cameraSlots[1].m20;
    r13->m3E4_cameraSlots[0].m24_distanceToDestination = r13->m3E4_cameraSlots[1].m24_distanceToDestination;

    fieldOverlaySubTaskInitSub3(0);

    activateCameraFollowMode(getFieldTaskPtr()->m8_pSubFieldData->m334->m50E_followModeIndex);
}

void endCutsceneCameraNoRestore()
{
    sFieldCameraManager* r13 = getFieldTaskPtr()->m8_pSubFieldData->m334;
    r13->m378_cutsceneFrameCounter = 0;
    r13->m37C_isCutsceneCameraActive = 0;
    dragonFieldTaskInitSub4Sub4();
    fieldOverlaySubTaskInitSub5(1);
    fieldOverlaySubTaskInitSub3(0);
    dragonFieldTaskInitSub4Sub3(getFieldTaskPtr()->m8_pSubFieldData->m334->m50E_followModeIndex);
}

void s_cutsceneTask2::UpdateSub1()
{
    getTask()->markFinished();

    if (getFieldTaskPtr()->m8_pSubFieldData)
    {
        getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m48_cutsceneTask = NULL;
        getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m5C = 0;
        if (getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m78)
        {
            return;
        }

        if (m0 & 2)
        {
            endCutsceneCameraWithRestore();
        }
        else
        {
            endCutsceneCameraNoRestore();
        }
    }
}

void s_cutsceneTask2::Update(s_cutsceneTask2* pThis)
{
    switch (pThis->m34)
    {
    case 0:
        pThis->m34 = pThis->UpdateSub0();
        startCutsceneCameraTracking(&pThis->m8, &pThis->m14);
        getFieldCameraStatus()->mC_rotation[2] = pThis->m20;
        return;
    case 1:
        pThis->m8 += pThis->m44;
        pThis->m28 += pThis->m50;
        pThis->m20 += pThis->m5C;

        getFieldCameraStatus()->mC_rotation[1] = pThis->m20;
        pThis->m60++;

        if (pThis->m24)
        {
            //0606A12C
            assert(0);
        }
        else
        {
            pThis->m14 = pThis->m28;
        }

        if (--pThis->m40)
        {
            return;
        }

        if (++pThis->m38 < 0x10)
        {
            pThis->m34 = pThis->UpdateSub0();
        }
        else
        {
            pThis->m34 = -1;
        }
        break;
    default:
        if (getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m5C)
            return;
        pThis->UpdateSub1();
        break;
    }
}

struct s_cutsceneTask3 : public s_workAreaTemplate<s_cutsceneTask3>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &s_cutsceneTask3::Init, &s_cutsceneTask3::Update, &s_cutsceneTask3::Draw, NULL };
        return &taskDefinition;
    }
    static void Init(s_cutsceneTask3*);
    s32 UpdateSub0();
    void UpdateSub1();
    static void Update(s_cutsceneTask3*);
    static void Draw(s_cutsceneTask3*);

    s32 m0;
    std::vector<s_scriptData2>* m4;
    sVec3_FP m8;
    sVec3_FP* m14;
    sVec3_FP* m18;
    sVec3_FP m1C;
    s32 m28;
    s32 m2C;
    s32 m30;
    s_scriptData2* m34;
    s32 m38;
    sVec3_FP m3C;
    fixedPoint m48;
    s32 m4C;
    //size = 0x50
};

// TODO: this is shared between s_cutsceneTask2 and 3
void s_cutsceneTask3::UpdateSub1()
{
    getTask()->markFinished();

    if (getFieldTaskPtr()->m8_pSubFieldData)
    {
        getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m48_cutsceneTask = NULL;
        getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m5C = 0;
        if (getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m78)
        {
            return;
        }

        if (m0 & 2)
        {
            endCutsceneCameraWithRestore();
        }
        else
        {
            endCutsceneCameraNoRestore();
        }
    }
}

void s_cutsceneTask3::Init(s_cutsceneTask3* pThis)
{
    getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m48_cutsceneTask = pThis;
}

s32 s_cutsceneTask3::UpdateSub0()
{
    m34 = &(*m4)[m30];
    m38 = m34->m0;
    if (m38 == 0)
    {
        return -1;
    }

    m3C[0] = fixedPoint(m34->m14 - m34->m4).normalized();
    m3C[1] = fixedPoint(m34->m18 - m34->m8).normalized();
    m3C[2] = fixedPoint(m34->m1C - m34->mC).normalized();
    m48 = m34->m20 - m34->m10;

    m3C[0] = intDivide(m38, m3C[0]);
    m3C[1] = intDivide(m38, m3C[1]);
    m3C[2] = intDivide(m38, m3C[2]);
    m48 = intDivide(m38, m48);

    m1C[0] = m34->m4;
    m1C[1] = m34->m8;
    m1C[2] = m34->mC;
    m28 = m34->m10;

    m8[0] = (*m14)[0] + MTH_Mul_5_6(m28, getCos(m1C[0].getInteger() & 0xFFF), getSin(m1C[1].getInteger() & 0xFFF));
    m8[1] = (*m14)[1] - MTH_Mul(m28, getSin(m1C[0].getInteger() & 0xFFF));
    m8[2] = (*m14)[2] + MTH_Mul_5_6(m28, getCos(m1C[0].getInteger() & 0xFFF), getCos(m1C[1].getInteger() & 0xFFF));

    return 1;
}

void s_cutsceneTask3::Update(s_cutsceneTask3* pThis)
{
    switch (pThis->m2C)
    {
    case 0:
        pThis->m2C = pThis->UpdateSub0();
        if (pThis->m18)
        {
            startCutsceneCameraTracking(&pThis->m8, pThis->m18);
        }
        else
        {
            startCutsceneCameraTracking(&pThis->m8, pThis->m14);
        }
        getFieldCameraStatus()->mC_rotation[2] = pThis->m1C[2];
        return;
    case 1:
        pThis->m1C += pThis->m3C;
        getFieldCameraStatus()->mC_rotation[2] = pThis->m1C[2];
        pThis->m8[0] = (* pThis->m14)[0] + MTH_Mul_5_6(pThis->m28, getCos(pThis->m1C[0].getInteger() & 0xFFF), getSin(pThis->m1C[1].getInteger() & 0xFFF));
        pThis->m8[1] = (* pThis->m14)[1] - MTH_Mul(pThis->m28, getSin(pThis->m1C[0].getInteger() & 0xFFF));
        pThis->m8[2] = (* pThis->m14)[2] + MTH_Mul_5_6(pThis->m28, getCos(pThis->m1C[0].getInteger() & 0xFFF), getCos(pThis->m1C[1].getInteger() & 0xFFF));
        pThis->m4C++;
        // end of key frame?
        if (--pThis->m38 != 0)
        {
            return;
        }
        // change key frame
        if (++pThis->m30 >= 0x10)
        {
            pThis->m2C = -1;
        }
        else
        {
            pThis->m2C = pThis->UpdateSub0();
        }
        break;
    default:
        if (!getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m5C)
        {
            pThis->UpdateSub1();
        }
        break;
    }
}

void s_cutsceneTask3::Draw(s_cutsceneTask3*)
{
    Unimplemented();
}

void cutsceneTaskInitSub2(p_workArea r4, std::vector<s_scriptData1>& r11, s32 r6, sVec3_FP* r7, u32 arg0)
{
    s_fieldScriptWorkArea* r14 = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE;

    r14->m80 = r4;
    r14->m7C = r6;
    r14->m84 = r7;

    if (r14->m78)
    {
        startCutsceneCameraTracking(&r14->m78->m3C, &r14->m78->m48);
        r14->m48_cutsceneTask = r4;
        return;
    }

    if (r11.size() == 0)
        return;

    cutsceneTaskInitSub2Sub1(r11, *r14->m88);

    if (r14->m48_cutsceneTask)
    {
        cutsceneTaskInitSub2Sub2(r14->m48_cutsceneTask);
    }

    s_cutsceneTask2* pNewTask = createSiblingTaskWithArg<s_cutsceneTask2>(r4, &r11);

    pNewTask->m0 = arg0;
    if (arg0 & 1)
    {
        pNewTask->m24 = r7;
    }

    pNewTask->Update(pNewTask);
}

void cutsceneTaskInitSub3Sub0(std::vector<s_scriptData2>& r4, std::vector<s_scriptData2>& r5)
{
    r5 = r4;
}

void cutsceneTaskInitSub3(p_workArea r4, std::vector<s_scriptData2>& r11, s32 r6, sVec3_FP* r7, u32 arg0)
{
    s_fieldScriptWorkArea* r14 = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE;
    r14->m80 = r4;
    r14->m7C = r6;
    r14->m84 = r7;

    if (r14->m78)
    {
        startCutsceneCameraTracking(&r14->m78->m3C, r7);
        r14->m48_cutsceneTask = r4;
        return;
    }

    if (r11.size() == 0)
        return;

    cutsceneTaskInitSub3Sub0(r11, *r14->m8C);

    if (r7 == NULL)
        return;

    if (r14->m48_cutsceneTask)
    {
        cutsceneTaskInitSub2Sub2(r14->m48_cutsceneTask);
    }

    s_cutsceneTask3* pNewTask = createSiblingTask<s_cutsceneTask3>(r4);

    pNewTask->m4 = &r11;
    pNewTask->m14 = r7;
    pNewTask->m0 = arg0;

    pNewTask->Update(pNewTask);
}

void s_cutsceneTask::Init(s_cutsceneTask* pThis, s_cutsceneData* pCutsceneData)
{
    s_dragonTaskWorkArea* r14 = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    u32 r11 = (pCutsceneData->m8 & ~3) / 2;

    if (r14->m1D8_cutscene)
    {
        r14->m1D8_cutscene->getTask()->markFinished();
    }

    r14->mF8_Flags &= ~0x400;
    r14->mF8_Flags |= 0x40000;

    r14->m1D8_cutscene = pThis;

    r14->m1D4_cutsceneData = pCutsceneData;

    r14->m1EC = cutsceneTaskInitSub0(pCutsceneData->m0, *getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m90);

    r14->m1EE = r14->m1EC;

    cutsceneTaskInitSub1(&pCutsceneData->m0[pThis->m0]);

    r14->mF0(r14);

    switch (pCutsceneData->m8 & 3)
    {
    case 1:
        assert(pCutsceneData->m4.size());
        cutsceneTaskInitSub2(pThis, pCutsceneData->m4, 0, &r14->m8_pos, r11);
        break;
    case 2:
        assert(pCutsceneData->m4.size());
        cutsceneTaskInitSub2(pThis, pCutsceneData->m4, 0, &r14->m8_pos, 1 | r11);
        break;
    case 3:
        assert(pCutsceneData->m4bis.size());
        cutsceneTaskInitSub3(pThis, pCutsceneData->m4bis, 0, &r14->m8_pos, 1 | r11);
        break;
    default:
        assert(0);
        break;
    }
}

void cutsceneTaskUpdateSub0()
{
    s_dragonTaskWorkArea* pDragonTask = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    pDragonTask->m1E4_cutsceneKeyFrame = 0;
}

void cutsceneTaskUpdateSub1(s32 fieldIndex, s32 fieldParam, s32 fieldExitIndex, s32 r7)
{
    exitCutsceneTaskUpdateSub0Sub0();
    exitCutsceneTaskUpdateSub0Sub1(fieldIndex, fieldParam, fieldExitIndex, r7);
}

void s_cutsceneTask::Update(s_cutsceneTask* pThis)
{
    s_dragonTaskWorkArea* r13 = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    if (r13->m1D4_cutsceneData == NULL)
    {
        cutsceneTaskUpdateSub0();
        dragonFieldTaskInitSub4Sub4();
        pThis->getTask()->markFinished();
        r13->m1D8_cutscene = 0;
        return;
    }
    if (r13->m1E4_cutsceneKeyFrame == NULL)
    {
        s_scriptData3* pData = &r13->m1D4_cutsceneData->m0[++pThis->m0];
        if (pData->m0_duration)
        {
            cutsceneTaskInitSub1(pData);
        }
        else
        {
            r13->m1D4_cutsceneData = NULL;
        }
    }

    pThis->m18_frameCount++;

    if (pThis->m4_changeField)
    {
        if (r13->m1EE <= 20)
        {
            cutsceneTaskUpdateSub1(pThis->m8_fieldIndex, pThis->mC_fieldParam, pThis->m10_fieldExitIndex, pThis->m14);
            pThis->m4_changeField = 0;
        }
    }
}

void startCutscene(s_cutsceneData* r4)
{
    createSubTaskWithArg<s_cutsceneTask>(getFieldTaskPtr()->m8_pSubFieldData, r4);
}

void loadScriptData1(std::vector<s_scriptData1>& output, const sSaturnPtr& constEA)
{
    sSaturnPtr EA = constEA;

    int numEntries = 0;
    while (readSaturnU32(EA))
    {
        numEntries++;
        EA = EA + 0x3C;
    }
    EA -= 0x3C * numEntries;
    numEntries++;

    output.resize(numEntries);
    for (int i = 0; i < numEntries; i++)
    {
        output[i].m0 = readSaturnS32(EA); EA = EA + 4;
        output[i].m4 = readSaturnVec3(EA); EA = EA + 4 * 3;
        output[i].m10 = readSaturnVec3(EA); EA = EA + 4 * 3;
        output[i].m1C = readSaturnVec3(EA); EA = EA + 4 * 3;
        output[i].m28 = readSaturnVec3(EA); EA = EA + 4 * 3;
        output[i].m34 = readSaturnS32(EA); EA = EA + 4;
        output[i].m38 = readSaturnS32(EA); EA = EA + 4;
    }

}

s_cutsceneData* loadCutsceneData(sSaturnPtr EA)
{
    s_cutsceneData* pData = new s_cutsceneData;

    sSaturnPtr table0 = readSaturnEA(EA); EA = EA + 4;
    sSaturnPtr table1 = readSaturnEA(EA); EA = EA + 4;
    pData->m8 = readSaturnU8(EA); EA = EA + 1;

    // read table 0
    {
        int numEntries = 0;
        while (readSaturnU32(table0))
        {
            numEntries++;
            table0 = table0 + 0x20;
        }
        table0 -= 0x20 * numEntries;
        numEntries++;

        pData->m0.resize(numEntries);
        for (int i = 0; i < numEntries; i++)
        {
            pData->m0[i].m0_duration = readSaturnS32(table0); table0 = table0 + 4;
            pData->m0[i].m4_pos = readSaturnVec3(table0); table0 = table0 + 4 * 3;
            pData->m0[i].m10_rotationDuration = readSaturnS32(table0); table0 = table0 + 4;
            pData->m0[i].m14_rot = readSaturnVec3(table0); table0 = table0 + 4 * 3;
        }
    }

    // read table 1
    switch (pData->m8 & 3)
    {
    case 1:
    case 2:
    {
        loadScriptData1(pData->m4, table1);
        break;
    }
    case 3:
    {
        int numEntries = 0;
        while (readSaturnU32(table1))
        {
            numEntries++;
            table1 = table1 + 0x24;
        }
        table1 -= 0x24 * numEntries;
        numEntries++;

        pData->m4bis.resize(numEntries);
        for (int i = 0; i < numEntries; i++)
        {
            pData->m4bis[i].m0 = readSaturnS32(table1); table1 = table1 + 4;
            pData->m4bis[i].m4 = readSaturnS32(table1); table1 = table1 + 4;
            pData->m4bis[i].m8 = readSaturnS32(table1); table1 = table1 + 4;
            pData->m4bis[i].mC = readSaturnS32(table1); table1 = table1 + 4;
            pData->m4bis[i].m10 = readSaturnS32(table1); table1 = table1 + 4;
            pData->m4bis[i].m14 = readSaturnS32(table1); table1 = table1 + 4;
            pData->m4bis[i].m18 = readSaturnS32(table1); table1 = table1 + 4;
            pData->m4bis[i].m1C = readSaturnS32(table1); table1 = table1 + 4;
            pData->m4bis[i].m20 = readSaturnS32(table1); table1 = table1 + 4;
        }
        break;
    }
    default:
        assert(0);
    }

    return pData;
}

void nullBattle()
{
    // intentionally empty
}


void(*subfieldTable1[])(p_workArea workArea) =
{
    subfieldA3_0,
    subfieldA3_1,
    subfieldA3_2,
    subfieldA3_3,
    subfieldA3_4,
    subfieldA3_5,
    subfieldA3_6,
    subfieldA3_7,
    subfieldA3_8,
    subfieldA3_9,
    subfieldA3_A,
    subfieldA3_B,
    subfieldA3_C,
};

void(*subfieldTable2[])(p_workArea workArea) =
{
    subfieldA3_1,
    subfieldA3_1,
    subfieldA3_2,
    subfieldA3_3,
    subfieldA3_4,
    subfieldA3_5,
    subfieldA3_6,
    subfieldA3_7,
    subfieldA3_8,
    subfieldA3_9,
    subfieldA3_A,
    subfieldA3_B,
    subfieldA3_C,
};

void initScriptTable1(std::vector<s_scriptData1>& pData1)
{
    for (int i = 0; i < pData1.size(); i++)
    {
        pData1[i].m0 = 0;
        pData1[i].m4.zeroize();
        pData1[i].m10.zeroize();
        pData1[i].m1C.zeroize();
        pData1[i].m28.zeroize();
        pData1[i].m34 = 0;
        pData1[i].m38 = 0;
    }
}

void initScriptTable2(std::vector<s_scriptData2>& pData2)
{
    for (int i = 0; i < pData2.size(); i++)
    {
        pData2[i].m0 = 0;
        pData2[i].m4 = 0;
        pData2[i].m8 = 0;
        pData2[i].mC = 0;
        pData2[i].m10 = 0xA000;
        pData2[i].m14 = 0;
        pData2[i].m18 = 0;
        pData2[i].m1C = 0;
        pData2[i].m20 = 0xA000;
    }
}

void initScriptTable3(std::vector<s_scriptData3>& pData3)
{
    for (int i = 0; i < pData3.size(); i++)
    {
        pData3[i].m0_duration = 0;
        pData3[i].m4_pos[0] = 0;
        pData3[i].m4_pos[1] = 0;
        pData3[i].m4_pos[2] = 0;
        pData3[i].m10_rotationDuration = 0;
        pData3[i].m14_rot[0] = 0;
        pData3[i].m14_rot[1] = 0;
        pData3[i].m14_rot[2] = 0;
    }
}

void initScriptTable4(std::vector<s_animDataFrame>& pData4)
{
    for (int i = 0; i < pData4.size(); i++)
    {
        pData4[i].m0 = -1;
        pData4[i].m1 = 0;
        pData4[i].m2 = 0;
    }
}

void s_fieldScriptWorkArea::Init(s_fieldScriptWorkArea* pFieldScriptWorkArea)
{
    s_fieldTaskWorkArea* pFieldTaskWorkArea = getFieldTaskPtr();

    pFieldTaskWorkArea->m8_pSubFieldData->m34C_ptrToE = pFieldScriptWorkArea;

    if (pFieldTaskWorkArea->m40.size() == 0)
    {
        pFieldTaskWorkArea->m40.resize(0x11);
        initScriptTable1(pFieldTaskWorkArea->m40);
    }

    if (pFieldTaskWorkArea->m44.size() == 0)
    {
        pFieldTaskWorkArea->m44.resize(0x11);
        initScriptTable2(pFieldTaskWorkArea->m44);
    }

    if (pFieldTaskWorkArea->m48.size() == 0)
    {
        pFieldTaskWorkArea->m48.resize(0x11);
        initScriptTable3(pFieldTaskWorkArea->m48);
    }

    if (pFieldTaskWorkArea->m4C.size() == 0)
    {
        pFieldTaskWorkArea->m4C.resize(0x11);
        initScriptTable4(pFieldTaskWorkArea->m4C);

    }

    pFieldScriptWorkArea->m8_stackPointer = &pFieldScriptWorkArea->mC_stack[8];

    pFieldScriptWorkArea->m88 = &pFieldTaskWorkArea->m40;
    pFieldScriptWorkArea->m8C = &pFieldTaskWorkArea->m44;
    pFieldScriptWorkArea->m90 = &pFieldTaskWorkArea->m48;
    pFieldScriptWorkArea->m94 = &pFieldTaskWorkArea->m4C;

    pFieldScriptWorkArea->m6C = 1000;
    pFieldScriptWorkArea->m70 = 1;
}

void fieldScriptTaskUpdateSub1()
{
    PDS_unimplemented("fieldScriptTaskUpdateSub1");
}

s32 fieldScriptTaskUpdateSub4()
{
    s_fieldScriptWorkArea* pFieldScript = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE;
    if (!pFieldScript->m4_currentScript.m_offset && !pFieldScript->m30_cinematicBarTask && !pFieldScript->m34 && !pFieldScript->m38_dialogStringTask && !pFieldScript->m3C_multichoiceTask && !pFieldScript->m40_receivedItemTask)
        return 0;

    return 1;

}

s32 isNormalCameraMode()
{
    return getFieldTaskPtr()->m8_pSubFieldData->m334->m37C_isCutsceneCameraActive == 0;
}

void s_fieldScriptWorkArea::fieldScriptTaskUpdateSub3()
{
    getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m8 &= ~0x40;
    getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m8 |= 0x20;

    if (m2C_bitToSet >= 0)
    {
        mainGameState.setBit566(m2C_bitToSet);
        m2C_bitToSet = -1;
    }

    m58 = 0;
    m50_scriptDelay = 0;
}

void endScript()
{
    s_fieldScriptWorkArea* pScript = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE;
    if (pScript->m4C_PCMPlaying)
    {
        pScript->getTask()->markFinished();
    }
}

void s_fieldScriptWorkArea::fieldScriptTaskUpdateSub2()
{
    fieldScriptTaskUpdateSub2Sub1();

    if (readKeyboardToggle(KEY_CODE_F12))
    {
        assert(0);
    }

    if (graphicEngineStatus.m4514.m0_inputDevices->m0_current.m8_newButtonDown & 6)
    {
        if (m60_canSkipScript)
        {
            endScript();
        }
        m50_scriptDelay = 0;
    }
}

s32 FLD_A3_Script_21_Sub0()
{
    s_dragonTaskWorkArea* pDragonTask = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;

    if ((pDragonTask->m1D0_cameraScript == NULL) && (pDragonTask->m1D4_cutsceneData == 0))
    {
        return 1;
    }

    return 0;
}

void dispatchTutorialMultiChoiceSub1()
{
    PDS_unimplemented("dispatchTutorialMultiChoiceSub1();");
}

void dispatchTutorialMultiChoiceSub2()
{
    if (fieldTaskVar0)
    {
        fieldTaskVar0->getTask()->markFinished();
    }
    fieldTaskPtr->m3C_fieldTaskState = 7;
    fieldTaskPtr->m3D = -1;
}

void markMultiChoiceEntrySeen(s32 result)
{
    if (getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m44_multiChoiceData)
    {
        getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m44_multiChoiceData->m0_choiceTable[result] = 9;
    }
}

void dispatchTutorialMultiChoiceSub3(s_fieldScriptWorkArea* pThis, int result)
{
    Unimplemented();
}

s32 battleIndex;

void startBattleTutorial(int tutorialIndex, int param2)
{
    if ((fieldTaskPtr->m28_status & 1) == 0)
    {
        fieldTaskPtr->m8_pSubFieldData->m344_randomBattleTask->m4 = 2;
        battleIndex = tutorialIndex;
    }
    else
    {
        // Bug in the original code, this wasn't using a createSubTaskWithArg, but createSubTask, and then setting the m0 battle index
        sBattleLoadingTask* pNewTask = createSubTaskWithArg<sBattleLoadingTask, s32>(getFieldTaskPtr(), tutorialIndex, &battleStartTaskDefinition);
        pNewTask->m4 = param2;
    }
}

s32 dispatchTutorialMultiChoice()
{
    s32 result = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m44_multiChoiceData->m4_currentChoice;
    if (result == 8) // quit tutorial
    {
        if (getFieldTaskPtr()->m2C_currentFieldIndex == 2)
        {
            mainGameState.setBit(0xA2 * 8 + 2); // tutorial bit
            dispatchTutorialMultiChoiceSub1();
        }
        else
        {
            fadeOutAllSequences();
            dispatchTutorialMultiChoiceSub2();
        }
    }
    else
    {
        static const std::array<s8, 8> tutorialIndex = {
            0xB,
            0xC,
            0xD,
            0xE,
            0xF,
            -1,
            -1,
            -1
        };

        if (tutorialIndex[result] < 0)
        {
            getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m44_multiChoiceData->mC = 0;
            dispatchTutorialMultiChoiceSub3(getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE, result);
            playSystemSoundEffect(3);
        }
        else
        {
            getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m44_multiChoiceData->mC = 1;
            startBattleTutorial(tutorialIndex[result], -1);
        }

        markMultiChoiceEntrySeen(result);
    }

    return result;
}

s_animDataFrame* readRiderAnimData(sSaturnPtr ptr)
{
    int numFrames = 0;
    while (readSaturnS8(ptr) != -1)
    {
        numFrames++;
        ptr += 4;
    }
    ptr -= 4 * numFrames;
    numFrames++;

    s_animDataFrame* pAnimDataFrame = new s_animDataFrame[numFrames];

    for (int i = 0; i < numFrames; i++)
    {
        pAnimDataFrame[i].m0 = readSaturnS8(ptr); ptr += 1;
        pAnimDataFrame[i].m1 = readSaturnS8(ptr); ptr += 1;
        pAnimDataFrame[i].m2 = readSaturnS16(ptr); ptr += 2;
    }

    return pAnimDataFrame;
}

void copyAnimation(s_animDataFrame* source, std::vector<s_animDataFrame>& destination)
{
    int i = 0;
    while (source->m0 != -1)
    {
        destination[i].m0 = source->m0;
        destination[i].m1 = source->m1;
        destination[i].m2 = source->m2;

        source++;
        i++;
    }

    destination[i].m0 = source->m0;
}

const s32 RiderAnimations[] =
{
    0x30,
    0x34,
    0x38,
    0x3C,
    0x40,
    0x44,
    0x48,
    0x4C,
    0x50,
    0x54,
    0x58,
    0x5C,
    0x60,
    0x64,
    0x68,
    0x6C,
    0x70,
    0x74,
    0x78,
    0x7C,
    0x80,
    0x84,
    0x88,
    0x8C,
    0x90,
};

const s32 rider1Table[] =
{
    0x108,
    0xE8,
    0xEC,
    0xF0,
    0x108,
    0x108,
    0x108,
    0x108,
    0x108,
    0x108,
    0x108,
    0x108,
    0x108,
    0x108,
    0x108,
    0x108,
    0xF4,
    0x108,
    0xF8,
    0xFC,
    0x108,
};

const s32 rider2Table[] =
{
    0x12C,
    0x130,
    0x14C,
    0x134,
    0x14C,
    0x14C,
    0x14C,
    0x14C,
    0x14C,
    0x14C,
    0x14C,
    0x14C,
    0x14C,
    0x14C,
    0x14C,
    0x14C,
    0x14C,
    0x14C,
    0x138,
    0x13C,
    0x140,
};

void s_riderAnimTask::Delete(s_riderAnimTask* pThis)
{
    getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1DC_ridersAnimation[pThis->m4_riderIndex] = NULL;
}

void s_riderAnimTask::Update(s_riderAnimTask* pThis)
{
    switch (pThis->m0_status)
    {
    case 0:
        switch (pThis->m10_animSequence->m0)
        {
        case -1:
            pThis->getTask()->markFinished();
            return;
        case 0:
            pThis->m8_delay = pThis->m10_animSequence->m2;
            playAnimationGeneric(&pThis->m14_riderState->m18_3dModel, pThis->m18->m0_riderBundle->getAnimation(pThis->m1C[pThis->m10_animSequence->m1]), pThis->m10_animSequence->m2);
            break;
        case 1:
            pThis->m8_delay = pThis->m10_animSequence->m2;
            playAnimationGeneric(&pThis->m14_riderState->m18_3dModel, pThis->m18->m0_riderBundle->getAnimation(pThis->m1C[pThis->m10_animSequence->m1 + 0x10]), 15);
            break;
        default:
            assert(0);
            break;
        }
        pThis->mC_previousFrame = 0;
        updateAndInterpolateAnimation(&pThis->m14_riderState->m18_3dModel);
        pThis->m0_status++;
        [[fallthrough]];
    case 1:
        switch (pThis->m10_animSequence->m0)
        {
        case 0:
            if ((--pThis->m8_delay) < -1)
            {
                pThis->m0_status = 0;
                pThis->m10_animSequence++;
            }
            break;
        case 1:
            if (pThis->m14_riderState->m18_3dModel.m16_previousAnimationFrame < pThis->mC_previousFrame)
            {
                if ((--pThis->m8_delay) < 1)
                {
                    pThis->m0_status = 0;
                    pThis->m10_animSequence++;
                }
            }
            pThis->mC_previousFrame = pThis->m14_riderState->m18_3dModel.m16_previousAnimationFrame;
            break;
        }
        break;
    default:
        assert(false);
        break;
    }
}

s32 playRiderAnim(s32 riderIndex, s_animDataFrame* r5)
{
    s_riderAnimTask* r14 = createSubTask<s_riderAnimTask>(getFieldTaskPtr()->m8_pSubFieldData);

    copyAnimation(r5, *getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m94);

    s_dragonTaskWorkArea* pDragonTask = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    pDragonTask->m1DC_ridersAnimation[riderIndex] = r14;

    r14->m4_riderIndex = riderIndex;
    r14->m10_animSequence = r5;
    if (riderIndex)
    {
        r14->m14_riderState = pRider2State;
    }
    else
    {
        r14->m14_riderState = pRider1State;
    }

    s32 r0;

    if (riderIndex)
    {
        r0 = mainGameState.gameStats.m3_rider2;
        if (r0 == 7)
        {
            r14->m1C = rider2Table;
            r14->m18 = pRider2State;
        }
        else
        {
            r14->m1C = RiderAnimations;
            r14->m18 = pRider1State;
        }
    }
    else
    {
        r0 = mainGameState.gameStats.m2_rider1;
        if (r0 == 6)
        {
            r14->m1C = rider1Table;
            r14->m18 = pRider1State;
        }
        else
        {
            r14->m1C = RiderAnimations;
            r14->m18 = pRider1State;
        }
    }

    return r0;
}

static s32 radarBuildFilteredChoiceList(s_FieldRadar* pRadar);
static s32 radarFindFilteredIndex(s_FieldRadar* pRadar, s32 selection);
void createMultiChoice(s32 r4, s32 r5);

s32 executeNative(sSaturnPtr ptr)
{
    assert(ptr.m_file == gFLD_A3);

    switch (ptr.m_offset)
    {
    case 0x0605F180:
        return dispatchTutorialMultiChoice();
    case 0x06074F9A:
        return FLD_A3_Script_21_Sub0();
    case 0x060558A0:
        return playRiderAnim(0, readRiderAnimData({ 0x60832F8, gFLD_A3 }));
    case 0x060558a8:
        return playRiderAnim(0, readRiderAnimData({ 0x6083318, gFLD_A3 }));
    case 0x605F22C:
        return getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m44_multiChoiceData->mC;
    case 0x060688d6:
        if ((encounterTaskVar0 != 2) && (getFieldTaskPtr()->m8_pSubFieldData->m344_randomBattleTask->m5 != 3))
        {
            return 0;
        }
        else
        {
            return 1;
        }
        break;
    case 0x0606ad04:
        getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m5C = 1;
        return 0; // result ignored?
    case 0x0606acec:
        getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m5C = 0;
        return 0; // result ignored?
    case 0x06057e1c:
        setupDragonPosition(readSaturnVec3({ 0x608FA20, gFLD_A3 }), readSaturnVec3({ 0x608FA2C, gFLD_A3 }));
        return 0; // result ignored?
    case 0x6074C78:
        getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m249_noCollisionAndHideDragon = 0;
        if (getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->mB8_lightWingEffect)
        {
            assert(0);
        }
        return 0; // result ignored?
    case 0x06074C36:
        getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m249_noCollisionAndHideDragon = 1;
        if (getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->mB8_lightWingEffect)
        {
            assert(0);
        }
        return 0; // result ignored?
    case 0x060730bc: // 060730bc — destination multichoice dialog
    {
        s_FieldRadar* pRadar = getFieldTaskPtr()->m8_pSubFieldData->m33C_fieldRadar;
        if (pRadar->m1C_encounterList != nullptr)
        {
            s32 numChoices = radarBuildFilteredChoiceList(pRadar);
            if (numChoices != 0)
            {
                s32 defaultChoice = radarFindFilteredIndex(pRadar, pRadar->m18_currentSelection);
                createMultiChoice(pRadar->m24_filteredChoiceCount, defaultChoice);
            }
            return pRadar->m24_filteredChoiceCount;
        }
        return 0;
    }
    case 0x0605985c: // used in A2 destroyed ship cutscene
        return initCrashedShipDestruction();
    case 0x606ACB0:
        if (getFieldTaskPtr()->m8_pSubFieldData)
        {
            s_fieldScriptWorkArea* r14 = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE;
            if (r14->m48_cutsceneTask)
            {
                if (r14->m5C == 0)
                {
                    PDS_unimplemented("!!!!!!!!!!!!!!!!!! Casting cutscenes!");
                    //TODO: this is widely incorrect!
                    s_cutsceneTask2* r4 = static_cast<s_cutsceneTask2*>(r14->m48_cutsceneTask);
                    r4->UpdateSub1();
                }
            }
        }
        return 0; // result ignored?
    default:
        assert(0);
        break;
    }
    return 0;
}

void scriptFunction_6067E68_fadeOut(s32 arg0)
{
    if (g_fadeControls.m_4D >= g_fadeControls.m_4C)
    {
        vdp2Controls.m20_registers[0].m112_CLOFSL = 0x10;
        vdp2Controls.m20_registers[1].m112_CLOFSL = 0x10;
    }

    fadePalette(&g_fadeControls.m0_fade0, convertColorToU32ForFade(g_fadeControls.m0_fade0.m0_color), 0x8000, arg0*2);
    fadePalette(&g_fadeControls.m24_fade1, convertColorToU32ForFade(g_fadeControls.m24_fade1.m0_color), 0x8000, arg0 * 2);
}

void scriptFunction_6067ec0_fadeIn(s32 arg0)
{
    if (g_fadeControls.m_4D >= g_fadeControls.m_4C)
    {
        vdp2Controls.m20_registers[0].m112_CLOFSL = 0x10;
        vdp2Controls.m20_registers[1].m112_CLOFSL = 0x10;
    }

    fadePalette(&g_fadeControls.m0_fade0, 0x8000, g_fadeControls.m_48, arg0 * 2);
    fadePalette(&g_fadeControls.m24_fade1, 0x8000, g_fadeControls.m_48, arg0 * 2);

}

s32 executeNative(sSaturnPtr ptr, s32 arg0)
{
    assert(ptr.m_file == gFLD_A3);

    switch (ptr.m_offset)
    {
    case 0x06067e68:
        scriptFunction_6067E68_fadeOut(arg0);
        break;
    case 0x06067ec0:
        scriptFunction_6067ec0_fadeIn(arg0);
        break;
    default:
        assert(0);
        break;
    }
    return 0;
}

sSaturnPtr s_fieldScriptWorkArea::callNative(sSaturnPtr r5)
{
    u8 numArguments = readSaturnU8(r5);
    r5 = r5 + 1;
    sSaturnPtr r14 = r5 + 3;
    r14.m_offset &= ~3;

    switch (numArguments)
    {
    case 0:
        m54_currentResult = executeNative(readSaturnEA(r14));
        break;
    case 1:
        m54_currentResult = executeNative(readSaturnEA(r14), readSaturnS32(r14 + 4));
        break;
    default:
        assert(0);
        break;
    }

    return r14 + (numArguments + 1) * 4;
}

void createMultiChoiceSub1()
{
    s_multiChoice* r4 = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m44_multiChoiceData;
    if (r4)
    {
        for (int i = 0; i < r4->m8_numChoices; i++)
        {
            r4->m0_choiceTable[i] = 0;
        }
    }
}

void createMultiChoice(s32 r4, s32 r5)
{
    s_multiChoice* r14 = (s_multiChoice*)allocateHeapForTask(getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE, sizeof(s_multiChoice));
    getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m44_multiChoiceData = r14;

    r14->m8_numChoices = r4;
    r14->m0_choiceTable = (s16*)allocateHeapForTask(getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE, r4 * 2);

    if (r5 >= 0)
    {
        //06079234
        assert(0);
    }
    else
    {
        r14->m4_currentChoice = 0;
        createMultiChoiceSub1();
    }

    vdp2Controls.m20_registers[0].m26_SFCODE = vdp2Controls.m20_registers[1].m26_SFCODE = VDP2Regs_.m26_SFCODE = 0xC000;
    vdp2Controls.m20_registers[0].m10A_CCRNB = vdp2Controls.m20_registers[1].m10A_CCRNB = VDP2Regs_.m10A_CCRNB = (vdp2Controls.m4_pendingVdp2Regs->m10A_CCRNB & 0xFFE0) | 0x10;
}

void createMultiChoiceDefault(s32 r4)
{
    createMultiChoice(r4, -1);
}

s32 fieldPlayPCM(sSaturnPtr pPcmNameEA)
{
    char* pPcmName = (char*)getSaturnPtr(pPcmNameEA);

    FILE* fHandle = fopen(pPcmName, "rb");
    if (fHandle)
    {
        fseek(fHandle, 0, SEEK_END);
        int size = ftell(fHandle) / 2;
        fseek(fHandle, 0, SEEK_SET);
        s16* buffer = new s16[size];
        fread(buffer, 2, size, fHandle);

        for (int i = 0; i < size; i++)
        {
            buffer[i] = READ_BE_S16(buffer + i);
        }

        SoLoud::Wav* newWav = new SoLoud::Wav();
        newWav->loadRawWave16(buffer, size, 22050, 1);

        gSoloud.play(*newWav);
    }
    else
    {
        printf("Failed to play PCM %s\n", pPcmName);
    }

    return 0;
}

sSaturnPtr s_fieldScriptWorkArea::runFieldScript()
{
    sSaturnPtr pScript = m4_currentScript;
    while (1)
    {
        u8 opCode = readSaturnU8(pScript); pScript = pScript + 1;

        switch (opCode)
        {
        case 0: // Ignore, happens sometimes
            break;
        case 1: // END
            if (m8_stackPointer == &mC_stack[8])
            {
                endScript();
                return sSaturnPtr::getNull();
            }
            else
            {
                pScript = *m8_stackPointer;
                m8_stackPointer++;
            }
            break;
        case 2: // wait
            pScript = pScript + 1;
            pScript.m_offset &= ~1;
            m50_scriptDelay = readSaturnS16(pScript);
            if (m50_scriptDelay)
            {
                m50_scriptDelay--;
                pScript = pScript + 2;
                return pScript;
            }
            break;
        case 4: // if not
            pScript = pScript + 1;
            pScript.m_offset &= ~1;

            if (m54_currentResult == readSaturnS16(pScript))
            {
                m54_currentResult = 0;
            }
            else
            {
                m54_currentResult = -1;
            }
            pScript = pScript + 2;
            break;
        case 7:
            pScript = pScript + 1;
            pScript.m_offset &= ~1;

            if (m54_currentResult >= readSaturnS16(pScript))
            {
                m54_currentResult = 0;
            }
            else
            {
                m54_currentResult = -1;
            }
            pScript = pScript + 2;
            break;
        case 10: // unconditional jump
        {
            sSaturnPtr r3 = pScript + 3;
            r3.m_offset &= ~3;
            pScript = readSaturnEA(r3);
            continue;
        }
        case 11: // jump if result != 0
            if (m54_currentResult == 0)
            {
                pScript = pScript + 7;
                pScript.m_offset &= ~3;
            }
            else
            {
                sSaturnPtr r3 = pScript + 3;
                r3.m_offset &= ~3;
                pScript = readSaturnEA(r3);
            }
            continue;
        case 12: // jump if result == 0
            if (m54_currentResult == 0)
            {
                sSaturnPtr r3 = pScript + 3;
                r3.m_offset &= ~3;
                pScript = readSaturnEA(r3);
            }
            else
            {
                pScript = pScript + 7;
                pScript.m_offset &= ~3;
            }
            break;
        case 14:
            pScript = callNative(pScript);
            break;
        case 21: // display string at bottom of screen
        {
            pScript = pScript + 3;
            pScript.m_offset &= ~3;

            sSaturnPtr string = readSaturnEA(pScript);
            pScript = pScript + 4;

            setupVDP2StringRendering(3, 25, 38, 2);
            vdp2StringContext.m0 = 0;
            VDP2DrawString((char*)getSaturnPtr(string));
            break;
        }
        case 22: // clear string
            setupVDP2StringRendering(3, 0x19, 0x26, 2);
            clearVdp2TextArea();
            break;
        case 24: // cinematic
            if (m30_cinematicBarTask)
            {
                if (m30_cinematicBarTask->m0_status == 1)
                {
                    continue;
                }
                else
                {
                    pScript = pScript - 1;
                    return pScript;
                }
            }
            else
            {
                m30_cinematicBarTask = createCinematicBarTask(this);
                cinematicBars_startClosing(m30_cinematicBarTask, 4);
                pScript = pScript - 1;
                return pScript;
            }
            break;
        case 0x24: // wait for button released
            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & 7)
            {
                return pScript;
            }
            break;
        case 25:
            if (m30_cinematicBarTask == NULL)
            {
                m30_cinematicBarTask = createCinematicBarTask(this);
                cinematicBars_startClosing(m30_cinematicBarTask, 1);
            }
            break;
        case 26: // clean cinematic bars
            if (m30_cinematicBarTask)
            {
                if (m30_cinematicBarTask->m0_status == 1)
                {
                    m30_cinematicBarTask->cinematicBars_startOpening(5);
                    pScript = pScript - 1;
                    return pScript;
                }
                else
                {
                    if (m30_cinematicBarTask->m0_status == 0)
                    {
                        m30_cinematicBarTask->getTask()->markFinished();
                        m30_cinematicBarTask = NULL;
                        return pScript;
                    }
                    else
                    {
                        pScript = pScript - 1;
                        return pScript;
                    }
                }
            }
            break;
        case 27: // display string bottom of screen
            if (m38_dialogStringTask)
            {
                //6068E76
                if ((m38_dialogStringTask->m0_status == 4) || m58)
                {
                    pScript = pScript + 1;
                    pScript.m_offset &= ~1;

                    pScript = pScript + 5;
                    pScript.m_offset &= ~3;

                    pScript = pScript + 4;
                    return pScript;
                }

                pScript = pScript - 1;
                return pScript;
            }
            else
            {
                //6068EC0
                sSaturnPtr r10 = pScript - 1; //stay on that opcode

                // Align
                pScript = pScript + 1;
                pScript.m_offset &= ~1;

                s16 duration = readSaturnS16(pScript);
                pScript = pScript + 5;
                pScript.m_offset &= ~3;

                sSaturnPtr stringPtr = readSaturnEA(pScript);

                createDisplayStringBorromScreenTask(this, &m38_dialogStringTask, duration, stringPtr);

                return r10;
            }
            break;
        case 31: // used for dialog "do you want to save"
        {
            sSaturnPtr opcodeStart = pScript - 1;
            s8 numChoices = readSaturnS8(pScript);
            if (m3C_multichoiceTask != nullptr)
            {
                if (m3C_multichoiceTask->m0_Status != 4)
                {
                    return opcodeStart;
                }
                if (numChoices < 1)
                {
                    return opcodeStart + 2;
                }
                pScript = pScript + 1;
                if (numChoices > 0)
                {
                    pScript = pScript + 3;
                    pScript.m_offset &= ~3;
                    pScript = pScript + numChoices * 4;
                }
                return pScript;
            }
            pScript = pScript + 1;
            pScript = pScript + 3;

            pScript.m_offset &= ~3;
            sSaturnPtr choicesList = pScript;
            if (numChoices)
            {
                startDialogTask(this, &m3C_multichoiceTask, &m54_currentResult, -numChoices, choicesList);
                return opcodeStart;
            }
            break;
        }
        case 33: // multi choice
            if (m3C_multichoiceTask)
            {
                //0606907A
                if (m3C_multichoiceTask->m0_Status == 4)
                {
                    s8 r4 = readSaturnS8(pScript);
                    pScript = pScript + 1;
                    if (r4 > 0)
                    {
                        pScript = pScript + 3;
                        pScript.m_offset &= ~3;
                        pScript = pScript + r4 * 4;
                    }
                    return pScript;
                }
                else
                {
                    pScript = pScript - 1;
                    return pScript;
                }
            }
            else
            {
                sSaturnPtr var4 = pScript - 1;
                s8 numChoices = readSaturnS8(pScript);
                pScript = pScript + 1;
                if (numChoices == 0)
                {
                    continue;
                }
                else
                {
                    pScript = pScript + 3;
                    pScript.m_offset &= ~3;

                    if (m44_multiChoiceData == NULL)
                    {
                        createMultiChoiceDefault(numChoices);
                    }
                    updateMultiChoice(this, &m3C_multichoiceTask, &m44_multiChoiceData->m4_currentChoice, -numChoices, pScript, m44_multiChoiceData->m0_choiceTable, m44_multiChoiceData->m4_currentChoice);

                    return var4;
                }
            }
        case 34: // play PCM
            if (!m4C_PCMPlaying)
            {
                pScript = pScript + 3;
                pScript.m_offset &= ~3;

                sSaturnPtr PCM_Name = readSaturnEA(pScript);

                fieldPlayPCM(PCM_Name);
                pScript = pScript + 4;
                continue;
            }
            else
            {
                pScript = pScript - 1;
                return pScript;
            }
        case 38: // call native
        {
            sSaturnPtr r15 = pScript - 1; //stay on that opcode
            pScript = callNative(pScript);
            if (m54_currentResult)
            {
                continue;
            }
            else
            {
                return r15;
            }
        }
        case 39: // call script
        {
            sSaturnPtr r3 = pScript + 3;
            r3.m_offset &= ~3;
            *(--m8_stackPointer) = r3 + 4; // store the return address
            pScript = readSaturnEA(r3);
            continue;
        }
        default:
            assert(0);
            break;
        }
    }

    return m4_currentScript;
}

static void openDestinationScript(); // forward declaration
static s32 radarBuildFilteredChoiceList(s_FieldRadar* pRadar);
static s32 radarFindFilteredIndex(s_FieldRadar* pRadar, s32 selection);
void createMultiChoice(s32 r4, s32 r5);
void s_fieldScriptWorkArea::Update(s_fieldScriptWorkArea* pThis)
{
    fieldScriptTaskUpdateSub1();

    if (pThis->m80)
    {
        if (pThis->m80->getTask()->isFinished())
        {
            pThis->m80 = NULL;
        }
    }

    if (readKeyboardToggle(KEY_CODE_F12))
    {
        assert(0);
    }

    s_LCSTask* pLCS = getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS;
    if ((pLCS->m83F_activeLaserCount == 0) && (pThis->m40_receivedItemTask == 0) && (pThis->m4_currentScript.m_offset))
    {
        pThis->fieldScriptTaskUpdateSub2();

        if ((pThis->m50_scriptDelay == 0) || (pThis->m58 != 0))
        {
            pThis->m4_currentScript = pThis->runFieldScript();

            if (pThis->m4_currentScript.m_offset == 0)
            {
                pThis->fieldScriptTaskUpdateSub3();
            }
        }
        else
        {
            pThis->m50_scriptDelay--;
        }
    }

    if (((!fieldScriptTaskUpdateSub4()) && (isNormalCameraMode() != 0)) &&
        ((getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m83F_activeLaserCount) == '\0'
            && ((getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->mF8_Flags & 0x20000) == 0)))
    {
        //06069A2C
        fieldRadar_show();
        getFieldTaskPtr()->m28_status &= ~0x40;
    }
    else
    {
        //06069A18
        fieldRadar_hide();
        getFieldTaskPtr()->m28_status |= 0x40;
    }

    if (getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m8 ||
        pThis->m64 || pThis->m30_cinematicBarTask || pThis->m34 || pThis->m38_dialogStringTask || pThis->m3C_multichoiceTask || pThis->m40_receivedItemTask ||
        getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->mF8_Flags & 0x20000)
    {
        graphicEngineStatus.m40AC.m1_isMenuAllowed = 0;
    }
    else
    {
        //06069A92
        graphicEngineStatus.m40AC.m1_isMenuAllowed = 1;
        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][13])
        {
            playSystemSoundEffect(0);
            graphicEngineStatus.m40AC.m0_menuId = 5;
            return;
        }
        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][12])
        {
            //06069AF0
            openDestinationScript();
            return;
        }
        return;
    }
}

// Helper: resolve game state bit index (values >= 1000 are offset by 0x236)
static bool checkGameStateBit(u32 bitIndex)
{
    u32 resolved = bitIndex;
    if ((s32)bitIndex > 999)
        resolved = bitIndex - 0x236;
    u32 byteIndex = resolved >> 3;
    u32 bitInByte = resolved & 7;
    return (bitMasks[bitInByte] & mainGameState.bitField[byteIndex]) != 0;
}

// 06072f8a — build filtered choice list from radar destination entries
static s32 radarBuildFilteredChoiceList(s_FieldRadar* pRadar)
{
    s32 count = pRadar->m14_encounterCount - 1;
    pRadar->m24_filteredChoiceCount = 0;

    // First pass: count available destinations and validate current selection
    s32* pEntry = (s32*)pRadar->m1C_encounterList;
    for (s32 i = count; i > 0; i--)
    {
        u32 bitIndex = *(u32*)(((u8*)pEntry) + 0x18);
        if (!checkGameStateBit(bitIndex))
        {
            // Destination not available — if current selection points here, reset it
            if (pRadar->m18_currentSelection == pRadar->m14_encounterCount - i)
            {
                pRadar->m18_currentSelection = 0;
            }
        }
        else
        {
            pRadar->m24_filteredChoiceCount++;
        }
        pEntry += 5; // advance by 0x14 bytes
    }

    // Set string table entry 2 to current selection text
    const char* selText = "";
    if (pRadar->m18_currentSelection > 0)
    {
        pRadar->m24_filteredChoiceCount++;
        selText = *(const char**)((u8*)pRadar->m1C_encounterList + pRadar->m18_currentSelection * 0x14);
    }
    setGlobalStringTableEntry(2, selText);

    // Second pass: allocate and fill choice array
    if (pRadar->m24_filteredChoiceCount != 0)
    {
        pRadar->m20_choiceArray = (s32*)allocateHeapForTask((s_workArea*)pRadar,
            pRadar->m24_filteredChoiceCount * 4);
        if (pRadar->m20_choiceArray != nullptr)
        {
            s32* pEntry2 = (s32*)pRadar->m1C_encounterList;
            s32 choiceIdx = 0;
            for (s32 i = count; i > 0; i--)
            {
                u32 bitIndex = *(u32*)(((u8*)pEntry2) + 0x18);
                if (checkGameStateBit(bitIndex))
                {
                    pRadar->m20_choiceArray[choiceIdx] = *(s32*)(((u8*)pEntry2) + 0x14);
                    choiceIdx++;
                }
                pEntry2 += 5;
            }
            // Add "back" option if current selection is set
            if (pRadar->m18_currentSelection > 0)
            {
                pRadar->m20_choiceArray[choiceIdx] = *(s32*)pRadar->m1C_encounterList;
            }
        }
    }

    return pRadar->m24_filteredChoiceCount;
}

// 06072df0 — find the filtered index for the current selection
static s32 radarFindFilteredIndex(s_FieldRadar* pRadar, s32 selection)
{
    s32 filteredIndex = -1;
    u8* pEntry = (u8*)pRadar->m1C_encounterList + 0x14; // start at entry 1

    for (s32 i = selection - 1; i >= 0; i--)
    {
        u32 bitIndex = *(u32*)(pEntry + 4);
        if (checkGameStateBit(bitIndex))
        {
            filteredIndex++;
        }
        pEntry += 0x14;
    }
    return filteredIndex;
}

// 06012768 — check if destination selection is available at current field/subfield
static bool canSelectDestinationAtCurrentField()
{
    s16 fieldIndex = getFieldTaskPtr()->m2C_currentFieldIndex;

    // These fields always allow destination selection
    if (fieldIndex == 4 || fieldIndex == 9 || fieldIndex == 0xB ||
        fieldIndex == 0xF || fieldIndex == 0x10 || fieldIndex == 0x11 || fieldIndex == 0x12)
    {
        return true;
    }

    // Per-subfield destination tables (from Saturn RAM at 0x0020A5F4)
    static const s32 field1_subfields[] = { 1, 1, 0 };
    static const s32 field2_subfields[] = { 1, 1, 1 };
    static const s32 field3_subfields[] = { 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0 };

    const s32* subTable = nullptr;
    s32 subTableSize = 0;
    switch (fieldIndex)
    {
    case 1: subTable = field1_subfields; subTableSize = 3; break;
    case 2: subTable = field2_subfields; subTableSize = 3; break;
    case 3: subTable = field3_subfields; subTableSize = 11; break;
    default: return false;
    }

    s16 subFieldIndex = getFieldTaskPtr()->m2E_currentSubFieldIndex;
    if (subFieldIndex < subTableSize && subTable[subFieldIndex] != 0)
        return true;

    return false;
}

// 06071c34 — open destination script based on availability
static void openDestinationScript()
{
    sSaturnPtr scriptEA;
    if (canSelectDestinationAtCurrentField())
    {
        scriptEA = gFLD_A3->getSaturnPtr(0x06080FAD);
    }
    else
    {
        scriptEA = gFLD_A3->getSaturnPtr(0x0608106F);
    }
    queueNewFieldScript(scriptEA, -1);
}

void createFieldScriptTask(s_workArea* pWorkArea)
{
    createSubTask<s_fieldScriptWorkArea>(pWorkArea);
}

void fieldOverlaySubTaskInitSub2Sub1Sub1(sFieldCameraStatus* r11, s_dragonTaskWorkArea* stack_4)
{
    sFieldCameraManager* pCameraData = getFieldTaskPtr()->m8_pSubFieldData->m334;
    s_fieldCameraConfig* r14 = &pCameraData->m10[pCameraData->m0_nextCamera];

    fixedPoint r14FP = FP_Div(stack_4->m8_pos[1] - r14->m50, r14->m54 - r14->m50);

    if (r14FP < 0)
    {
        r14FP = 0;
    }
    else if (r14FP > 0x10000)
    {
        r14FP = 0x10000;
    }

    fixedPoint r4 = r14->m34[0] - r14->m18[0];
    r4 = r4.normalized();
    r11->m28 = r14->m18[0] + MTH_Mul(r4, r14FP);

    r4 = r14->m34[3] - r14->m18[3];
    r4 = r4.normalized();
    r11->m34 = r14->m18[3] + MTH_Mul(r4, r14FP);

    r4 = r14->m34[6] - r14->m18[6];
    r4 = r4.normalized();
    r11->m40 = r14->m18[6] + MTH_Mul(r4, r14FP);
}

fixedPoint interpolateDistance(fixedPoint r11, fixedPoint r12, fixedPoint stack0, fixedPoint r10, s32 r14)
{
    fixedPoint r13 = r12 - r11;
    fixedPoint r4 = MTH_Mul(stack0, r13);

    if (r13 < 0)
    {
        fixedPoint r5 = -r14;
        fixedPoint r6 = -r10;
        fixedPoint r3;
        if (r4 < r5)
        {
            r3 = r4;
        }
        else
        {
            r3 = r5;
        }

        if (r6 >= r3)
        {
            r4 = r6;
        }
        else if (r4 >= r5)
        {
            r4 = r5;
        }

        r13 -= r4;

        if (r13 > 0)
        {
            return r12;
        }
        else
        {
            return r11 + r4;
        }
    }
    else
    {
        //60606F4
        fixedPoint r2;
        if (r4 >= r14)
        {
            r2 = r4;
        }
        else
        {
            r2 = r14;
        }

        if (r10 < r2)
        {
            r4 = r10;
        }
        else if (r4 < r14)
        {
            r4 = r14;
        }

        r13 -= r4;
        if (r13 > 0)
        {
            return r12;
        }
        else
        {
            return r11 + r4;
        }
    }
}

fixedPoint interpolateRotation(fixedPoint r10_currentValue, fixedPoint r12_targetValue, fixedPoint stack0, fixedPoint r11, s32 r13)
{
    fixedPoint r14 = r12_targetValue - r10_currentValue;
    r14 = r14.normalized();
    fixedPoint r4 = MTH_Mul(stack0, r14);

    if (r14 < 0)
    {
        fixedPoint r5 = -r13;
        fixedPoint r6 = -r11;
        fixedPoint r3;
        if (r4 < r5)
        {
            r3 = r4;
        }
        else
        {
            r3 = r5;
        }

        if (r6 >= r3)
        {
            r4 = r6;
        }
        else if (r4 >= r5)
        {
            r4 = r5;
        }

        r5 = r14 - r4;
        r3 = r5.normalized();

        if (r3 > 0)
        {
            return r12_targetValue;
        }
        else
        {
            return r10_currentValue + r4;
        }
    }
    else
    {
        //60607A0
        fixedPoint r5;
        if (r4 >= r13)
        {
            r5 = r4;
        }
        else
        {
            r5 = r13;
        }

        if (r11 < r5)
        {
            r5 = r11;
        }
        else if (r4 >= r13)
        {
            r5 = r4;
        }
        else
        {
            r5 = r13;
        }

        r4 = r14 - r5;
        fixedPoint r3 = r4.normalized();

        if (r3 >= 0)
        {
            return r10_currentValue + r5;
        }
        else
        {
            return r12_targetValue;
        }
    }
}

void fieldOverlaySubTaskInitSub2Sub1Sub2(sFieldCameraStatus* r14, s_dragonTaskWorkArea* r9)
{
    r14->m5C_rotationSpring[0] = interpolateRotation(r14->m5C_rotationSpring[0], 0, 0x2000, 0x444444, 0);
    r14->m5C_rotationSpring[1] = interpolateRotation(r14->m5C_rotationSpring[1], 0, 0x2000, 0x444444, 0);
    r14->m5C_rotationSpring[2] = interpolateRotation(r14->m5C_rotationSpring[2], 0, 0x2000, 0x444444, 0);

    r14->m5C_rotationSpring += r14->m68_rotationImpulse;
    r14->mC_rotation += r14->m5C_rotationSpring;

    r14->m68_rotationImpulse.zeroize();

    r14->mC_rotation[0] = r14->mC_rotation[0].normalized();
    r14->m18 = r14->m18.normalized();

    fixedPoint r11 = r14->mC_rotation[0] + r14->m18;

    if (r11 < -0x31C71C7)
    {
        r14->mC_rotation[0] = -r14->m18 - 0x31C71C7;
        r11 = -0x31C71C7;
    }
    else if (r11 > 0x31C71C7)
    {
        r14->mC_rotation[0] = -r14->m18 + 0x31C71C7;
        r11 = 0x31C71C7;
    }

    sVec3_FP var10;
    var10[0] = -MTH_Mul_5_6(r14->m24_distanceToDestination, getCos(r11.getInteger() & 0xFFF), getSin(r14->mC_rotation[1].getInteger() & 0xFFF));
    var10[1] = MTH_Mul(r14->m24_distanceToDestination, getSin(r11.getInteger() & 0xFFF));
    var10[2] = -MTH_Mul_5_6(r14->m24_distanceToDestination, getCos(r11.getInteger() & 0xFFF), getCos(r14->mC_rotation[1].getInteger() & 0xFFF));

    r14->m0_position = r9->m8_pos - var10;

    r14->m44_positionSpring[0] = interpolateDistance(r14->m44_positionSpring[0], 0, 0x2000, 0xAAA, 0);
    r14->m44_positionSpring[1] = interpolateDistance(r14->m44_positionSpring[1], 0, 0x2000, 0xAAA, 0);
    r14->m44_positionSpring[2] = interpolateDistance(r14->m44_positionSpring[2], 0, 0x2000, 0xAAA, 0);

    r14->m44_positionSpring += r14->m50_positionImpulse;

    r14->m0_position += r14->m44_positionSpring;
    r14->m50_positionImpulse.zeroize();
}

void fieldOverlaySubTaskInitSub2Sub2(sFieldCameraStatus* r14, s_dragonTaskWorkArea* r12)
{
    sVec2_FP var0;
    sVec3_FP stack_8;

    stack_8[0] = -r12->m88_matrix.m[0][2];
    stack_8[1] = -r12->m88_matrix.m[1][2];
    stack_8[2] = -r12->m88_matrix.m[2][2];

    computeLookAt(stack_8, var0);

    fieldOverlaySubTaskInitSub2Sub1Sub1(r14, r12);

    if (r14->m7C & 1)
    {
        r14->mC_rotation[0] = r14->m28 - r12->m20_angle[0];
    }
    else
    {
        r14->mC_rotation[0] = r14->m28;
    }

    if (r14->m7C & 2)
    {
        r14->mC_rotation[1] = var0[1];
    }

    if (r14->m7C & 4)
    {
        r14->mC_rotation[2] = r14->m30;
    }

    r14->m18 = r14->m34;

    r14->m24_distanceToDestination = r14->m40;

    fieldOverlaySubTaskInitSub2Sub1Sub2(r14, r12);
}

void fieldOverlaySubTaskInitSub2Sub1(sFieldCameraStatus* r14, s_dragonTaskWorkArea* r12)
{
    sVec2_FP var0;
    sVec3_FP stack_8;

    stack_8[0] = -r12->m88_matrix.m[0][2];
    stack_8[1] = -r12->m88_matrix.m[1][2];
    stack_8[2] = -r12->m88_matrix.m[2][2];

    computeLookAt(stack_8, var0);

    fieldOverlaySubTaskInitSub2Sub1Sub1(r14, r12);

    if (r14->m7C & 1)
    {
        r14->mC_rotation[0] = interpolateRotation(r14->mC_rotation[0], r14->m28 - r12->m20_angle[0], 0x2000, 0x111111, 0);
    }
    else
    {
        r14->mC_rotation[0] = interpolateRotation(r14->mC_rotation[0], r14->m28, 0x2000, 0x111111, 0);
    }

    // TODO: recheck, this is sketchy (the m34[2])
    if (getFieldTaskPtr()->m8_pSubFieldData->m334->m10[0].m34[2])
    {
        r14->m7C &= 0xFFFFFFFD;
    }
    else
    {
        r14->m7C |= 2;
    }

    if (r14->m7C & 2)
    {
        r14->mC_rotation[1] = interpolateRotation(r14->mC_rotation[1], var0[1], 0x2000, 0x222222, 0);
    }

    if ((r14->m7C & 4) == 0)
    {
        r14->m30 = 0;
    }

    r14->mC_rotation[2] = interpolateRotation(r14->mC_rotation[2], r14->m30, 0x2000, 0x222222, 0);

    r14->m24_distanceToDestination = interpolateDistance(r14->m24_distanceToDestination, r14->m40, 0x2000, 0xAAA, 0);

    fieldOverlaySubTaskInitSub2Sub1Sub2(r14, r12);
}

void fieldOverlaySubTaskInitSub2(sFieldCameraStatus* r14)
{
    s_dragonTaskWorkArea* pDragonTask = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    if (pDragonTask == NULL)
        return;

    switch (r14->m8D_followState)
    {
    case 0:
        r14->m8F_followType = 1;
        r14->m90_followMode = 1;
        r14->m7C = 2;
        r14->m8D_followState = 2;
        fieldOverlaySubTaskInitSub2Sub2(r14, pDragonTask);
        return;
    case 1:
        r14->m8F_followType = 1;
        r14->m90_followMode = 1;
        r14->m7C = 2;
        r14->m8D_followState = 2;
    case 2:
        fieldOverlaySubTaskInitSub2Sub1(r14, pDragonTask);
        return;
    default:
        assert(0);
    }
}

// 06062228 — camera draw mode 2 (tower/vertical)
void fieldOverlaySubTaskInitSub2_mode2(sFieldCameraStatus* r14)
{
    s_dragonTaskWorkArea* pDragonTask = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    if (pDragonTask == NULL)
        return;

    switch (r14->m8D_followState)
    {
    case 0:
        r14->m8F_followType = 2;
        r14->m90_followMode = 2;
        r14->m7C = 3;
        r14->m8D_followState = 2;
        fieldOverlaySubTaskInitSub2Sub2(r14, pDragonTask);
        return;
    case 1:
        r14->m8F_followType = 2;
        r14->m90_followMode = 2;
        r14->m7C = 3;
        r14->m8D_followState = 2;
    case 2:
        fieldOverlaySubTaskInitSub2Sub1(r14, pDragonTask);
        return;
    default:
        assert(0);
    }
}

void resetCameraStatus(sFieldCameraStatus* r4)
{
    r4->m74_updateFunc = 0;
    r4->m78_drawFunc = 0;
    r4->m0_position[0] = 0;
    r4->m0_position[1] = 0;
    r4->m0_position[2] = 0;
    r4->mC_rotation[0] = 0;
    r4->mC_rotation[1] = 0;
    r4->mC_rotation[2] = 0;
    r4->m18 = 0;
    r4->m24_distanceToDestination = 0xF000;
    r4->m28 = 0;
    r4->m2C = 0;
    r4->m30 = 0;
    r4->m34 = 0;
    r4->m40 = 0xF000;
    r4->m80_frameCounter = 0;
    r4->m84 = 0;
    r4->m88 = 0;

    r4->m89 = 0;
    r4->m8A = 0;
}

void initCameraSlotWithFunctions(u32 r4, void(*r5)(sFieldCameraStatus*), void(*r6)(sFieldCameraStatus*))
{
    sFieldCameraStatus* pFieldCameraStatus = &getFieldTaskPtr()->m8_pSubFieldData->m334->m3E4_cameraSlots[r4];
    resetCameraStatus(pFieldCameraStatus);

    pFieldCameraStatus->m74_updateFunc = r5;
    pFieldCameraStatus->m78_drawFunc = r6;
    pFieldCameraStatus->m8C_isActive = 1;
}

u32 fieldOverlaySubTaskInitSub3(u32 r4)
{
    if (isFieldCameraSlotActive(r4))
    {
        sFieldCameraManager* p334 = getFieldTaskPtr()->m8_pSubFieldData->m334;
        p334->m50C_activeCameraSlot = r4;
        p334->m3E4_cameraSlots[r4].m80_frameCounter = 0;
        return 1;
    }

    return 0;
}

void fieldOverlaySubTaskInitSub4Sub0Sub0(s32* r4, s32* r5)
{
    for (int i = 0; i < 7; i++)
    {
        r5[i] = r4[i];
    }
}

void copyFieldCameraConfig(s_fieldCameraConfig* r4, s_fieldCameraConfig* r5)
{
    r5->m0_min = r4->m0_min;
    r5->mC_max = r4->mC_max;

    fieldOverlaySubTaskInitSub4Sub0Sub0(r4->m18, r5->m18);
    fieldOverlaySubTaskInitSub4Sub0Sub0(r4->m34, r5->m34);

    r5->m50 = r4->m50;
    r5->m54 = r4->m54;
}

void setupFieldCameraConfigs(s_fieldCameraConfig* r4, u32 r5)
{
    sFieldCameraManager* p334 = getFieldTaskPtr()->m8_pSubFieldData->m334;

    p334->mC = 8;
    p334->m2DC_numCameraZones = 4;

    for (int i = 0; i < r5; i++)
    {
        copyFieldCameraConfig(&r4[i], &p334->m10[i]);
    }
}

void fieldOverlaySubTaskInitSub5(u32 r4)
{
    sFieldCameraManager* p334 = getFieldTaskPtr()->m8_pSubFieldData->m334;
    p334->m3E4_cameraSlots[r4].m74_updateFunc = 0;
    p334->m3E4_cameraSlots[r4].m78_drawFunc = 0;
    p334->m3E4_cameraSlots[r4].m8C_isActive = 0;
}

void applyCameraStatusToEngine(sFieldCameraManager* pTypedWorkArea)
{
    sFieldCameraStatus* r13 = &pTypedWorkArea->m3E4_cameraSlots[pTypedWorkArea->m50C_activeCameraSlot];
    sVec3_S16 r15;
    r15[0] = r13->mC_rotation[0] >> 16;
    r15[1] = r13->mC_rotation[1] >> 16;
    r15[2] = r13->mC_rotation[2] >> 16;

    updateEngineCamera(&cameraProperties2, r13->m0_position, r15);

    copyMatrix(pCurrentMatrix, &pTypedWorkArea->m384_viewMatrix);
    copyMatrix(&cameraProperties2.m28[0], &pTypedWorkArea->m3B4_projectionMatrix);
}

s_fieldCameraConfig* readCameraConfig(sSaturnPtr EA)
{
    s_fieldCameraConfig* pCameraConfig = new s_fieldCameraConfig;

    pCameraConfig->m0_min[0] = readSaturnS32(EA); EA = EA + 4;
    pCameraConfig->m0_min[1] = readSaturnS32(EA); EA = EA + 4;
    pCameraConfig->m0_min[2] = readSaturnS32(EA); EA = EA + 4;

    pCameraConfig->mC_max[0] = readSaturnS32(EA); EA = EA + 4;
    pCameraConfig->mC_max[1] = readSaturnS32(EA); EA = EA + 4;
    pCameraConfig->mC_max[2] = readSaturnS32(EA); EA = EA + 4;

    for (int i = 0; i < 7; i++)
    {
        pCameraConfig->m18[i] = readSaturnS32(EA); EA = EA + 4;
    }

    for (int i = 0; i < 7; i++)
    {
        pCameraConfig->m34[i] = readSaturnS32(EA); EA = EA + 4;
    }

    pCameraConfig->m50 = readSaturnS32(EA); EA = EA + 4;
    pCameraConfig->m54 = readSaturnS32(EA); EA = EA + 4;

    return pCameraConfig;
}

void sFieldCameraManager::fieldCameraManagerInit(sFieldCameraManager* pTypedWorkArea)
{
    getFieldTaskPtr()->m8_pSubFieldData->m334 = pTypedWorkArea;

    initCameraSlotWithFunctions(0, gFieldCameraDrawFunc ? gFieldCameraDrawFunc : &fieldOverlaySubTaskInitSub2, 0);
    fieldOverlaySubTaskInitSub3(0);
    setupFieldCameraConfigs(readCameraConfig(gFieldCameraConfigEA), 1);

    getFieldTaskPtr()->m8_pSubFieldData->m334->m50E_followModeIndex = 1;

    fieldOverlaySubTaskInitSub5(1);

    applyCameraStatusToEngine(pTypedWorkArea);

    getFieldTaskPtr()->m8_pSubFieldData->m334->m50D_isInitialized = 1;
}

void createFieldOverlaySubTask(s_workArea* pWorkArea)
{
    createSubTask<sFieldCameraManager>(pWorkArea);
}

struct s_DragonRiderTask : public s_workAreaTemplate<s_DragonRiderTask>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &dragonRidersTaskInit, &dragonRidersTaskUpdate, NULL, NULL };
        return &taskDefinition;
    }
    
    static void dragonRidersTaskInit(s_DragonRiderTask* pWorkArea);
    static void dragonRidersTaskUpdate(s_DragonRiderTask* pWorkArea);
};

// 0607b280
void s_DragonRiderTask::dragonRidersTaskInit(s_DragonRiderTask* pWorkArea)
{
    initAnimation(&pRider1State->m18_3dModel, pRider1State->m0_riderBundle->getAnimation(0x30));
    updateAndInterpolateAnimation(&pRider1State->m18_3dModel);

    if (mainGameState.gameStats.m3_rider2 == 7)
    {
        initAnimation(&pRider2State->m18_3dModel, pRider2State->m0_riderBundle->getAnimation(0x30));
        updateAndInterpolateAnimation(&pRider2State->m18_3dModel);
    }
}

void s_DragonRiderTask::dragonRidersTaskUpdate(s_DragonRiderTask* pWorkArea)
{
    PDS_unimplemented("dragonRidersTaskUpdate");
}

void dragonFieldTaskInitSub2Sub2(s_dragonTaskWorkArea* pDragon)
{
    pDragon->m178_turnRate = 0x222222;
    pDragon->m17C = 0x4CCC;
    pDragon->m180 = 0x16;
    pDragon->m184_animRate = 0x111111;
}

void dragonFieldTaskInitSub2Sub3(s_dragonTaskWorkArea* pWorkArea)
{
    pWorkArea->m1F0.m_0 = 0;
    pWorkArea->m1F0.m_4 = 0;
    pWorkArea->m1F0.m_8 = 0;
    pWorkArea->m1F0.m_C = 0;
    pWorkArea->m1F0.m_E = 0;
    pWorkArea->m1F0.m_10 = 0;
}

void dragonFieldTaskInitSub2Sub4(s_dragonTaskWorkArea_48* m48)
{
    initMatrixToIdentity(&m48->m0_matrix);

    m48->m30 = 0;
    m48->m34 = 0;
    m48->m38 = 0;

    m48->m3C = 1;
}

void initDragonSpeed(u32 arg)
{
    if ((arg >= 0) && (arg <= 2))
    {
        s_dragonTaskWorkArea* pDragonTask = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;

        pDragonTask->m235_dragonSpeedIndex = arg;

        pDragonTask->m154_dragonSpeed = pDragonTask->m21C_DragonSpeedValues[arg];

        s32 r2 = pDragonTask->m21C_DragonSpeedValues[0] + pDragonTask->m21C_DragonSpeedValues[1];
        if (r2 < 0)
        {
            assert(0);
            r2++;
        }
        r2 /= 2;

        if (pDragonTask->m154_dragonSpeed > r2)
        {
            pDragonTask->m237 = 4;
            pDragonTask->m238 = 4;
        }
        else
        {
            pDragonTask->m237 = 0;
            pDragonTask->m238 = 0;
        }
    }
}

sFieldCameraStatus* getFieldCameraStatus()
{
    return &getFieldTaskPtr()->m8_pSubFieldData->m334->m3E4_cameraSlots[getFieldTaskPtr()->m8_pSubFieldData->m334->m50C_activeCameraSlot];
}

void dragonFieldTaskInitSub2(s_dragonTaskWorkArea* pWorkArea)
{
    createFieldRadar(pWorkArea);

    dragonFieldTaskInitSub2Sub2(pWorkArea);

    dragonFieldTaskInitSub2Sub3(pWorkArea);

    pWorkArea->m208 = 0x960000;
    pWorkArea->m20C = 0x960000;
    pWorkArea->m210 = 0xB333;
    pWorkArea->m214 = 0xB333;
    pWorkArea->m150 = 0x10000;

    pWorkArea->m8_pos[0] = 0;
    pWorkArea->m8_pos[1] = 0x1E000;
    pWorkArea->m8_pos[2] = 0;

    pWorkArea->m20_angle[0] = 0;
    pWorkArea->m20_angle[1] = 0;
    pWorkArea->m20_angle[2] = 0;

    pWorkArea->m154_dragonSpeed = 0;

    pWorkArea->m1B8 = 0xB333;
    pWorkArea->m1BC = 0x200000;

    initMatrixToIdentity(&pWorkArea->m88_matrix);

    dragonFieldTaskInitSub2Sub4(&pWorkArea->m48);

    pWorkArea->m1CC_fieldOfView = DEG_80; // field of view
    pWorkArea->m234 = 0;

    pWorkArea->m21C_DragonSpeedValues[0] = 0;
    pWorkArea->m21C_DragonSpeedValues[1] = 0x1284;
    pWorkArea->m21C_DragonSpeedValues[2] = 0x2509;
    pWorkArea->m21C_DragonSpeedValues[3] = 0x3B42;
    pWorkArea->m21C_DragonSpeedValues[4] = 0x58E3;

    initDragonSpeed(0);

    pWorkArea->m230 = 0x1999;

    //060738C0

    pWorkArea->m154_dragonSpeed = pWorkArea->m21C_DragonSpeedValues[pWorkArea->m235_dragonSpeedIndex];

    pWorkArea->m238 = 0;
    pWorkArea->m237 = 0;
    pWorkArea->mC0_lightRotationAroundDragon = 0xC000000;
    pWorkArea->mC4 = 0;

    pWorkArea->mC8_normalLightColor.m0 = 0x10;
    pWorkArea->mC8_normalLightColor.m1 = 0x10;
    pWorkArea->mC8_normalLightColor.m2 = 0x10;

    pWorkArea->mCB_falloffColor0.m0 = 0x8;
    pWorkArea->mCB_falloffColor0.m1 = 0x8;
    pWorkArea->mCB_falloffColor0.m2 = 0x8;

    pWorkArea->mCE_falloffColor1.m0 = 0x14;
    pWorkArea->mCE_falloffColor1.m1 = 0x14;
    pWorkArea->mCE_falloffColor1.m2 = 0x14;

    pWorkArea->mD1_falloffColor2.m0 = 0xC;
    pWorkArea->mD1_falloffColor2.m1 = 0xC;
    pWorkArea->mD1_falloffColor2.m2 = 0xC;

    pWorkArea->mD4.m0 = 0x10;
    pWorkArea->mD4.m1 = 0x10;
    pWorkArea->mD4.m2 = 0x10;

    sFieldCameraStatus* pFieldCameraStatus = getFieldCameraStatus();

    pFieldCameraStatus->mC_rotation[0] = pWorkArea->m20_angle[1];

    pWorkArea->m14C_pitchMax = 0x2AAAAAA;
    pWorkArea->m148_pitchMin = -0x2AAAAAA;
    pWorkArea->m140_maxY = 0x300000;
    pWorkArea->m134_minY = -0x300000;
    pWorkArea->m130_minX = 0x80000000;
    pWorkArea->m13C_maxX = 0x7FFFFFFF;
    pWorkArea->m138_minZ = 0x80000000;
    pWorkArea->m144_maxZ = 0x7FFFFFFF;
}

void dragonFieldTaskInitSub3(s_dragonTaskWorkArea* pWorkArea, s_dragonState* pDragonState, int param2)
{
    setupModelAnimation(&pDragonState->m28_dragon3dModel, pDragonState->m0_pDragonModelBundle->getAnimation(pDragonState->m20_dragonAnimOffsets[param2]));
    updateAndInterpolateAnimation(&pDragonState->m28_dragon3dModel);

    pWorkArea->m23A_dragonAnimation = param2;
    pWorkArea->m237 = pWorkArea->m238;
    pWorkArea->m23B = 1;
}

s32 isDragonInFieldBounds(s_dragonTaskWorkArea* r4)
{
    if (r4->m8_pos[0] <= r4->m130_minX + 0x20000)
        return 0;
    if (r4->m8_pos[0] >= r4->m13C_maxX - 0x20000)
        return 0;
    if (r4->m8_pos[2] <= r4->m138_minZ + 0x20000)
        return 0;
    if (r4->m8_pos[2] >= r4->m144_maxZ - 0x20000)
        return 0;

    return 1;
}

void startScriptLeaveArea()
{
    PDS_unimplemented("startScriptLeaveArea");
}

void buildDragonRotationMatrix(s_dragonTaskWorkArea_48* r14, sVec3_FP* r13)
{
    r14->m30 = r13->m0_X;
    r14->m34 = r13->m4_Y;
    r14->m38 = r13->m8_Z;
    r14->m3C = 1;

    initMatrixToIdentity(&r14->m0_matrix);
    rotateMatrixShiftedY(r13->m4_Y, &r14->m0_matrix);
    rotateMatrixShiftedX(r13->m0_X, &r14->m0_matrix);
    rotateMatrixShiftedZ(r13->m8_Z, &r14->m0_matrix);
}

void computeDragonDeltaTranslation(s_dragonTaskWorkArea* r14)
{
    PDS_unimplemented("updateDragonRotationSub1");
}

void dragonFieldTaskInitSub4Sub4Sub2()
{
    getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->mF8_Flags &= ~0x10000;
}

void updateDragonMovementNoInputSub1(s_dragonTaskWorkArea* r14)
{
    r14->m254 = 0;
    r14->m250 = 0;
    r14->m24A_runningCameraScript = 0;
    r14->m258 = 0;
    r14->m25C = 0;
    r14->m25D = 0;
}

void updateDragonMovementNoInput(s_dragonTaskWorkArea* r14)
{
    if (--r14->m25D < 0)
    {
        updateDragonMovementNoInputSub1(r14);
    }

    // update yaw
    {
        fixedPoint tempRotX = r14->m3C_targetAngles[0] - r14->m20_angle[0];
        r14->m20_angle[0] += r14->m3C_targetAngles[0] - intDivide(0x10, tempRotX.normalized() * 15) - r14->m20_angle[0];
    }

    if (r14->m25D == 2)
    {
        //0607EA3E
        r14->m20_angle[2] += r14->m254;
    }
    else
    {
        // update roll
        fixedPoint tempRotZ = r14->m3C_targetAngles[2] - r14->m20_angle[2];
        r14->m20_angle[2] += r14->m3C_targetAngles[2] - intDivide(0x10, tempRotZ.normalized() * 15) - r14->m20_angle[2];
    }

    //607EA84
    // clamp angle.x to valid range
    if (r14->m20_angle[0].normalized() > r14->m14C_pitchMax)
    {
        r14->m20_angle[0] = r14->m14C_pitchMax;
    }
    if (r14->m20_angle[0].normalized() < r14->m148_pitchMin)
    {
        r14->m20_angle[0] = r14->m148_pitchMin;
    }

    r14->m247 = 0;
    r14->m246_previousAnalogY = 0;
    r14->m245_previousAnalogX = 0;

    r14->m25C &= ~1;
}

u32 isDragonInputAllowed(s_dragonTaskWorkArea* r4)
{
    s_fieldScriptWorkArea* r14 = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE;

    if (getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->mF8_Flags & 0x10000)
        return 0;

    if (getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m8)
        return 0;

    if (r14->m4_currentScript.m_offset)
        return 0;

    if (r14->m30_cinematicBarTask)
        return 0;

    if (r14->m34)
        return 0;

    if (r14->m38_dialogStringTask)
        return 0;

    if (r14->m3C_multichoiceTask)
        return 0;

    return 1;
}

void applyDragonAnimationFromInput(s_dragonTaskWorkArea* r14, s_dragonState* r12)
{
    if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][5]) // down
    {
        incrementAnimationRootY(&r12->m78_animData, r14->m184_animRate);
    }
    else if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][4]) // up
    {
        incrementAnimationRootY(&r12->m78_animData, -r14->m184_animRate);
    }

    if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][7]) // right
    {
        incrementAnimationRootX(&r12->m78_animData, r14->m184_animRate);
        incrementAnimationRootZ(&r12->m78_animData, -r14->m184_animRate);
    }
    else if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][6]) // left
    {
        incrementAnimationRootX(&r12->m78_animData, -r14->m184_animRate);
        incrementAnimationRootZ(&r12->m78_animData, r14->m184_animRate);
    }
}

void applyDragonAnimationFromAnalog(s_dragonTaskWorkArea* r11, s_dragonState* r12)
{
    s32 r4_y;
    if (graphicEngineStatus.m4514.m138[1])
    {
        r4_y = -graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m3_analogY;
    }
    else
    {
        r4_y = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m3_analogY;
    }

    s32 var0 = r11->m246_previousAnalogY - r4_y;
    if (var0 > 0x40)
    {
        incrementAnimationRootY(&r12->m78_animData, intDivide(0x80, r11->m184_animRate.asS32() * var0));
    }
    else if (var0 < -0x40)
    {
        incrementAnimationRootY(&r12->m78_animData, intDivide(0x80, r11->m184_animRate.asS32() * var0));
    }

    //607F1E8
    s32 r4_x = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m2_analogX;
    s32 r10 = r4_x - r11->m245_previousAnalogX;
    if (r10 > 0x40)
    {
        incrementAnimationRootX(&r12->m78_animData, intDivide(0x80, -(r11->m184_animRate.asS32() * r10)));
        incrementAnimationRootZ(&r12->m78_animData, intDivide(0x80, r11->m184_animRate.asS32() * r10));
    }
    else if (r10 < -0x40)
    {
        incrementAnimationRootX(&r12->m78_animData, intDivide(0x80, -(r11->m184_animRate.asS32() * r10)));
        incrementAnimationRootZ(&r12->m78_animData, intDivide(0x80, r11->m184_animRate.asS32() * r10));
    }
}

static void startBarrelRollMode(s_dragonTaskWorkArea* r14);
void handleBarrelRollInputAnalog(s_dragonTaskWorkArea* r4)
{
    s32 timer = (s32)r4->m258 - 1;
    r4->m258 = timer;
    if (timer < 1)
    {
        r4->m25D = 0;
        r4->m258 = 0;

        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][15])
        {
            // 0607E7F8: forward + barrel roll button → reverse barrel roll
            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][0])
            {
                if (gDragonState->mC_dragonType != 8) // not floater
                {
                    r4->m25D = 2;
                    r4->m258 = 0x18;
                    if (r4->m25E == 0)
                    {
                        r4->m254 = 0x84BDA1;
                    }
                    else
                    {
                        r4->m254 = -0x84BDA1;
                    }
                    r4->m23C |= 4;
                    r4->m244 = 9;
                }
                return;
            }

            // Analog stick rotation dash (only at idle speed)
            if (r4->m235_dragonSpeedIndex == 0)
            {
                s8 analogX = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m2_analogX;
                if (analogX < 0)
                {
                    // Tilt left → barrel roll left
                    r4->m250 = fixedPoint((s32)r4->m20_angle[1] + (s32)0xF838E38F).normalized();
                    startBarrelRollMode(r4);
                    return;
                }
                if (analogX > 0)
                {
                    // Tilt right → barrel roll right
                    r4->m250 = fixedPoint((s32)r4->m20_angle[1] + (s32)0x7C71C71).normalized();
                    startBarrelRollMode(r4);
                    return;
                }
            }
        }
    }
}

void interpolateYawWithBanking(s_dragonTaskWorkArea* r14); // forward decl

void applyDragonPitchYawRollAnalog(s_dragonTaskWorkArea* r14)
{
    s32 r9_y;
    if (graphicEngineStatus.m4514.m138[1])
    {
        r9_y = -graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m3_analogY;
    }
    else
    {
        r9_y = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m3_analogY;
    }
    r9_y = -r9_y;

    // TODO: understand the code that tests angle and 0x80000000
    handleBarrelRollInputAnalog(r14);

    fixedPoint r3;
    if (r14->m154_dragonSpeed >= 0)
    {
        r3 = r14->m154_dragonSpeed;
    }
    else
    {
        r3 = -r14->m154_dragonSpeed;
    }

    //607F2F8
    if (r3 >= 0x100)
    {
        if (r9_y > 0)
        {
            // down
            r14->m1F0.m_8 = intDivide(0x7F, r9_y * r14->m178_turnRate.asS32());
            r14->m20_angle[0] += r14->m1F0.m_8;
            r14->m238 |= 2;
            r14->mFC |= 2;
        }
        else if(r9_y < 0)
        {
            // up
            r14->m1F0.m_8 = intDivide(0x7F, r9_y * r14->m178_turnRate.asS32());
            r14->m20_angle[0] += r14->m1F0.m_8;
            r14->m238 |= 1;
            r14->mFC |= 1;
        }
        else
        {
            //607F360
            fixedPoint r1 = r14->m3C_targetAngles[0] - r14->m20_angle[0];
            r14->m20_angle[0] += r14->m3C_targetAngles[0] - intDivide(0x10, r1.normalized() * 15) - r14->m20_angle[0];
        }
    }
    else
    {
        //607F3B4
        fixedPoint r4 = intDivide(0x7F, -(r9_y << 11));
        if (r9_y > 0)
        {
            r14->m160_deltaTranslation[1] += r4;
            r14->m238 |= 2;
            r14->m1F0.m_8 = intDivide(0x7F, r9_y * r14->m178_turnRate.asS32());
            r14->mFC |= 2;
        }
        else if (r9_y < 0)
        {
            r14->m160_deltaTranslation[1] += r4;
            r14->m238 |= 1;
            r14->m1F0.m_8 = intDivide(0x7F, r9_y * r14->m178_turnRate.asS32());
            r14->mFC |= 1;
        }

        //607F428
        fixedPoint r1 = r14->m3C_targetAngles[0] - r14->m20_angle[0];
        r14->m20_angle[0] += r14->m3C_targetAngles[0] - intDivide(0x10, r1.normalized() * 15) - r14->m20_angle[0];
    }

    //607F45A
    if (r14->m20_angle[0].normalized() > r14->m14C_pitchMax)
    {
        r14->m20_angle[0] = r14->m14C_pitchMax;
    }
    if (r14->m20_angle[0].normalized() < r14->m148_pitchMin)
    {
        r14->m20_angle[0] = r14->m148_pitchMin;
    }

    //607F49A
    r14->m1F0.m_C = intDivide(0x7F, graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m2_analogX * r14->m178_turnRate.asS32());

    if (r14->m25D == 1)
    {
        interpolateYawWithBanking(r14);
    }
    else
    {
        if ((r14->mF8_Flags & 0x8000) == 0)
        {
            r14->m20_angle[1] += r14->m1F0.m_C;
        }
    }

    //0607F4F0
    if (r14->m25D == 2)
    {
        r14->m20_angle[2] += r14->m254;
    }
    else
    {
        r14->m20_angle[2] += r14->m3C_targetAngles[2] - intDivide(0x10, fixedPoint(r14->m3C_targetAngles[2] - r14->m20_angle[2]).normalized() * 15) - r14->m20_angle[2];
        s32 analogX = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m2_analogX;
        if (analogX > 0)
        {
            //0607F540
            fixedPoint r2 = r14->m20_angle[1] - r14->m30;
            if (r2 >= intDivide(0x7F, analogX * r14->m178_turnRate.asS32()))
            {
                r14->m20_angle[2] -= intDivide(4, r14->m178_turnRate * 3);
                r14->mFC |= 4;
                r14->m25E = 1;
            }
        }
        else if (analogX < 0)
        {
            //607F57E
            fixedPoint r2 = r14->m30 - r14->m20_angle[1];
            if (r2 >= intDivide(0x7F, -analogX * r14->m178_turnRate.asS32()))
            {
                r14->m20_angle[2] += intDivide(4, r14->m178_turnRate * 3);
                r14->mFC |= 8;
                r14->m25E = 0;
            }
        }
    }

    //607F5BA
    r14->m245_previousAnalogX = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m2_analogX;
    if (graphicEngineStatus.m4514.m138[1])
    {
        r14->m246_previousAnalogY = -graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m3_analogY;
    }
    else
    {
        r14->m246_previousAnalogY = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m3_analogY;
    }

    r14->m247 = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m4;
}

// 0607e638
void interpolateYawWithBanking(s_dragonTaskWorkArea* r14)
{
    r14->m20_angle[1] = interpolateRotation(r14->m20_angle[1], r14->m250, 0x2000, 0x888889, 0x10);
    if (r14->m178_turnRate <= r14->m30 - r14->m20_angle[1])
    {
        r14->m20_angle[2] += intDivide(4, r14->m178_turnRate * 3);
    }
    else if (r14->m178_turnRate <= r14->m20_angle[1] - r14->m30)
    {
        r14->m20_angle[2] -= intDivide(4, r14->m178_turnRate * 3);
    }
}

// 0607e606
static void startBarrelRollMode(s_dragonTaskWorkArea* r14)
{
    r14->m25D = 1;
    r14->m258 = 0x1E;
    r14->m23C |= 4;
    r14->m244 = 4;
}

// 0607e718
void handleBarrelRollInput(s_dragonTaskWorkArea* r14)
{
    s32 timer = (s32)r14->m258 - 1;
    r14->m258 = timer;
    if (timer < 1)
    {
        r14->m25D = 0;
        r14->m258 = 0;

        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][0xF])
        {
            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][0])
            {
                if (gDragonState->mC_dragonType != 8)
                {
                    r14->m25D = 2;
                    r14->m258 = 0x18;
                    if (r14->m25E == 0)
                    {
                        r14->m254 = 0x84BDA1;
                    }
                    else
                    {
                        r14->m254 = -0x84BDA1;
                    }
                    r14->m23C |= 4;
                    r14->m244 = 9;
                }
                return;
            }

            if (r14->m235_dragonSpeedIndex == 0)
            {
                if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][7])
                {
                    r14->m250 = fixedPoint((s32)r14->m20_angle[1] + (s32)0xF838E38F).normalized();
                    startBarrelRollMode(r14);
                    return;
                }

                if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][6])
                {
                    r14->m250 = fixedPoint((s32)r14->m20_angle[1] + (s32)0x7C71C71).normalized();
                    startBarrelRollMode(r14);
                    return;
                }
            }
        }
    }
}

void applyDragonPitchYawRoll(s_dragonTaskWorkArea* r14)
{
    handleBarrelRollInput(r14);

    fixedPoint r2;
    if (r14->m154_dragonSpeed >= 0)
    {
        r2 = r14->m154_dragonSpeed;
    }
    else
    {
        r2 = -r14->m154_dragonSpeed;
    }

    if (r2 >= 0x100)
    {
        //0607EC06
        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][5]) // Go down
        {
            //0607EC12
            r14->m20_angle[0] += r14->m178_turnRate;
            r14->m238 |= 2;
            r14->m1F0.m_8 = r14->m178_turnRate;
            r14->mFC |= 2;
        }
        else if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][4]) // Go up
        {
            //0607EC44
            r14->m20_angle[0] -= r14->m178_turnRate;
            r14->m238 |= 1;
            r14->m1F0.m_8 = -r14->m178_turnRate;
            r14->mFC |= 1;
        }
        else
        {
            //607EC6C
            fixedPoint r1 = r14->m3C_targetAngles[0] - r14->m20_angle[0];
            r14->m20_angle[0] += r14->m3C_targetAngles[0] - intDivide(0x10, r1.normalized() * 15) - r14->m20_angle[0];
        }
    }
    else
    {
        //0607ECA4
        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][5]) // Go down
        {
            //607ECB0
            r14->m160_deltaTranslation[1] -= 0x800;
            r14->m238 |= 2;
            r14->m1F0.m_8 = r14->m178_turnRate;
            r14->mFC |= 2;
        }
        else if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][4]) // Go up
        {
            //607ED00
            r14->m160_deltaTranslation[1] += 0x800;
            r14->m238 |= 1;
            r14->m1F0.m_8 = -r14->m178_turnRate;
            r14->mFC |= 1;
        }

        //607ED32
        fixedPoint r1 = r14->m3C_targetAngles[0] - r14->m20_angle[0];
        r14->m20_angle[0] += r14->m3C_targetAngles[0] - intDivide(0x10, r1.normalized() * 15) - r14->m20_angle[0];
    }

    //0607ED68
    if (r14->m20_angle[0].normalized() > r14->m14C_pitchMax)
    {
        r14->m20_angle[0] = r14->m14C_pitchMax;
    }
    if (r14->m20_angle[0].normalized() < r14->m148_pitchMin)
    {
        r14->m20_angle[0] = r14->m148_pitchMin;
    }



    if (r14->m25D == 2)
    {
        //0607EDB0
        r14->m20_angle[2] += r14->m254;
    }
    else
    {
        fixedPoint r1 = r14->m3C_targetAngles[2] - r14->m20_angle[2];
        r14->m20_angle[2] += r14->m3C_targetAngles[2] - intDivide(0x10, r1.normalized() * 15) - r14->m20_angle[2];

        if (r14->m25D == 1)
        {
            interpolateYawWithBanking(r14);
        }
        else
        {
            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][7]) // right
            {
                //0607EE34
                if ((r14->mF8_Flags & 0x8000) == 0)
                {
                    r14->m20_angle[1] -= r14->m178_turnRate;
                }

                r14->m1F0.m_C = -r14->m178_turnRate;
                if (r14->m30 - r14->m20_angle[1] >= r14->m178_turnRate)
                {
                    r14->m20_angle[2] += intDivide(4, r14->m178_turnRate * 3);
                }

                r14->mFC |= 8;
                r14->m25E = 0;
            }
            else if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][6]) // left
            {
                //0607EE8C
                if ((r14->mF8_Flags & 0x8000) == 0)
                {
                    r14->m20_angle[1] += r14->m178_turnRate;
                }

                r14->m1F0.m_C = r14->m178_turnRate;
                if (r14->m20_angle[1] - r14->m30 >= r14->m178_turnRate)
                {
                    r14->m20_angle[2] -= intDivide(4, r14->m178_turnRate * 3);
                }

                r14->mFC |= 4;
                r14->m25E = 1;
            }
        }
    }

    r14->m247 = 0;
    r14->m246_previousAnalogY = 0;
    r14->m245_previousAnalogX = 0;
}

void updateDragonMovementDigital(s_dragonTaskWorkArea* r14)
{
    if (isDragonInputAllowed(r14))
    {
        applyDragonAnimationFromInput(r14, gDragonState);
        applyDragonPitchYawRoll(r14);
    }
    else
    {
        updateDragonMovementNoInput(r14);
    }
}

void updateDragonMovementAnalog(s_dragonTaskWorkArea* r14)
{
    if (isDragonInputAllowed(r14))
    {
        applyDragonAnimationFromAnalog(r14, gDragonState);
        applyDragonPitchYawRollAnalog(r14);
    }
    else
    {
        updateDragonMovementNoInput(r14);
    }
}

u32 isDragonControlledByScripts()
{
    s_fieldScriptWorkArea* r14 = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE;
    if (getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->mF8_Flags & 0x10000)
    {
        return 1;
    }

    if (getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m8 & 0xC8)
    {
        return 1;
    }

    if (r14->m4_currentScript.m_offset)
    {
        return 1;
    }

    if (r14->m30_cinematicBarTask)
    {
        return 1;
    }

    if (r14->m34)
    {
        return 1;
    }

    if (r14->m38_dialogStringTask)
    {
        return 1;
    }

    if (r14->m3C_multichoiceTask)
    {
        return 1;
    }

    return 0;
}

u32 isDragonPlayerControlAllowed()
{
    s_fieldScriptWorkArea* r14 = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE;

    if (getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m8)
    {
        return 0;
    }

    if (r14->m4_currentScript.m_offset)
    {
        return 0;
    }

    if (r14->m30_cinematicBarTask)
    {
        return 0;
    }

    if (r14->m34)
    {
        return 0;
    }

    if (r14->m38_dialogStringTask)
    {
        return 0;
    }

    if (r14->m3C_multichoiceTask)
    {
        return 0;
    }

    return 1;
}

void clearDragonPlayerInputs()
{
    s_dragonTaskWorkArea* pDragonTask = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;

    pDragonTask->m188[0] = 0;
    pDragonTask->m188[1] = 0;
    pDragonTask->m188[2] = 0;

    pDragonTask->m194[0] = 0;
    pDragonTask->m194[1] = 0;
    pDragonTask->m194[2] = 0;

    pDragonTask->m1A0[0] = 0;
    pDragonTask->m1A0[1] = 0;
    pDragonTask->m1A0[2] = 0;

    pDragonTask->m1AC[0] = 0;
    pDragonTask->m1AC[1] = 0;
    pDragonTask->m1AC[2] = 0;
}

u32 isDragonBoostAvailable()
{
    u32 T = !mainGameState.getBit(0x2A * 8 + 6);
    return T ^ 1;
}

u32 isDragonBoostLocked()
{
    u32 T = !mainGameState.getBit(0x2A * 8 + 7);
    return T ^ 1;
}

void resetDragonSpeedIndex(s_dragonTaskWorkArea* r4)
{
    if ((r4->m25C & 1) == 0)
    {
        r4->m235_dragonSpeedIndex = 0;
        r4->m234 = 0;
        r4->m25C = 0;
    }
}

void updateDragonSpeed(s_dragonTaskWorkArea* r14)
{
    // 0607E8B6: clear boost flag if new button pressed or speed <= 0
    if ((r14->m25C & 0x1) &&
        (((graphicEngineStatus.m4514.mD8_buttonConfig[1][0] | graphicEngineStatus.m4514.mD8_buttonConfig[1][11]) &
          graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown) != 0 ||
         r14->m235_dragonSpeedIndex < 1))
    {
        r14->m25C &= ~1;
    }

    // 0607E8E4: brake button check (buttonConfig[1][11] without forward)
    if ((r14->m25C & 0x2) == 0 &&
        (graphicEngineStatus.m4514.mD8_buttonConfig[1][11] & graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown))
    {
        if (graphicEngineStatus.m4514.mD8_buttonConfig[1][0] & graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown)
        {
            // Both brake and forward held → set speed to -1
            r14->m235_dragonSpeedIndex = -1;
        }
        else
        {
            // 0607E910: brake only → reset speed and return
            resetDragonSpeedIndex(r14);
            return;
        }
    }
    else
    {
        // 0607E920: normal speed update path
        r14->m25C |= 2;

        if (isDragonBoostLocked() &&
            (graphicEngineStatus.m4514.mD8_buttonConfig[1][15] & graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown))
        {
            r14->m25C |= 4;
        }

        s8 maxSpeed = (r14->m25C & 4) ? 4 : 3;

        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][0])
        {
            // 0607E960: forward held — accelerate
            if (++r14->m234 > 4)
            {
                if (++r14->m235_dragonSpeedIndex >= maxSpeed)
                {
                    r14->m235_dragonSpeedIndex = maxSpeed;
                }
                r14->m234 = 0;
            }
        }
        else
        {
            resetDragonSpeedIndex(r14);
        }

        // 0607E9AE: boost initiation
        if (isDragonBoostAvailable() && r14->m235_dragonSpeedIndex > 0 && (s32)r14->m154_dragonSpeed > 0 &&
            (graphicEngineStatus.m4514.mD8_buttonConfig[1][11] & graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown))
        {
            r14->m25C |= 1;
            r14->m158 = r14->m154_dragonSpeed;
        }
    }
}

void integrateDragonMovement(s_dragonTaskWorkArea* r14)
{
    getFieldTaskPtr()->m28_status &= ~0x10000;

    buildDragonRotationMatrix(&r14->m48, &r14->m20_angle);
    copyMatrix(&r14->m48.m0_matrix, &r14->m88_matrix);

    if (isDragonControlledByScripts())
    {
        clearDragonPlayerInputs();
    }

    if (isDragonPlayerControlAllowed())
    {
        if ((r14->mF8_Flags & 0x10000) == 0)
        {
            updateDragonSpeed(r14);
        }

        if (r14->m235_dragonSpeedIndex < 0)
        {
            //0607FE38
            r14->m15C_dragonSpeedIncrement = MTH_Mul(-r14->m154_dragonSpeed - 0x1284, r14->m230);
        }
        else
        {
            //607FE50
            fixedPoint r3;
            if (r14->m25C & 1)
            {
                r3 = r14->m158;
            }
            else
            {
                r3 = r14->m21C_DragonSpeedValues[r14->m235_dragonSpeedIndex];
            }

            fixedPoint r12 = r14->m154_dragonSpeed - r3;
            if (r12 < 0)
            {
                r14->m15C_dragonSpeedIncrement = -MTH_Mul(r12, r14->m230);
            }
            else
            {
                r14->m15C_dragonSpeedIncrement = MTH_Mul(-r12, r14->m230);
            }
        }
    }
    else
    {
        //607FE98
        if (r14->m154_dragonSpeed < 0)
        {
            r14->m15C_dragonSpeedIncrement = -MTH_Mul(r14->m154_dragonSpeed, r14->m230);
        }
        else
        {
            r14->m15C_dragonSpeedIncrement = MTH_Mul(-r14->m154_dragonSpeed, r14->m230);
        }
    }

    // 607FF02
    r14->m154_dragonSpeed += r14->m15C_dragonSpeedIncrement;

    // speed up?
    if (keyboardIsKeyDown(0xA9))
    {
        r14->m154_dragonSpeed.m_value *= 4;
    }

    // Clamp dragon speed
    fixedPoint r2 = r14->m154_dragonSpeed;
    if (r14->m154_dragonSpeed >= 0)
    {
        if (r14->m154_dragonSpeed < 0x7000)
        {
            r2 = r14->m154_dragonSpeed;
        }
        else
        {
            r2 = 0x7000;
        }
    }
    else
    {
        if (r14->m154_dragonSpeed >= -0x7000)
        {
            r2 = r14->m154_dragonSpeed;
        }
        else
        {
            r2 = -0x7000;
        }
    }
    r14->m154_dragonSpeed = r2;

    r14->m1AC[0] = interpolateRotation(r14->m1AC[0], 0, 0x2000, 0x444444, 0x10);
    r14->m1AC[1] = interpolateRotation(r14->m1AC[1], 0, 0x2000, 0x444444, 0x10);
    r14->m1AC[2] = interpolateRotation(r14->m1AC[2], 0, 0x2000, 0x444444, 0x10);

    r14->m1AC += r14->m1A0;

    r14->m20_angle += r14->m1AC;

    r14->m1A0.zeroize();

    // ~0607FFEC 
    r14->m194[0] = interpolateDistance(r14->m194[0], 0, 0x2000, 0xAAA, 0x10);
    r14->m194[1] = interpolateDistance(r14->m194[1], 0, 0x2000, 0xAAA, 0x10);
    r14->m194[2] = interpolateDistance(r14->m194[2], 0, 0x2000, 0xAAA, 0x10);

    r14->m194 += r14->m188;

    if (r14->m154_dragonSpeed < 0)
    {
        //06080066
        r14->m160_deltaTranslation[0] += r14->m194[0] - MTH_Mul(r14->m88_matrix.m[0][2], r14->m154_dragonSpeed);
        r14->m160_deltaTranslation[1] += r14->m194[1];
    }
    else
    {
        r14->m160_deltaTranslation[0] += r14->m194[0] - MTH_Mul(r14->m88_matrix.m[0][2], r14->m154_dragonSpeed);
        r14->m160_deltaTranslation[1] += r14->m194[1] + MTH_Mul(r14->m88_matrix.m[1][2], r14->m154_dragonSpeed);
    }
    r14->m160_deltaTranslation[2] += r14->m194[2] - MTH_Mul(r14->m88_matrix.m[2][2], r14->m154_dragonSpeed);

    r14->m188.zeroize();

    if (r14->mF4)
    {
        r14->mF4(r14);
    }

    //6080140
    r14->m8_pos += r14->m160_deltaTranslation;

    if ((r14->m134_minY == 0) && (r14->m140_maxY == 0))
    {
        return;
    }

    if (r14->m8_pos[1] < r14->m134_minY)
        r14->m8_pos[1] = r14->m134_minY;

    if (r14->m8_pos[1] > r14->m140_maxY)
        r14->m8_pos[1] = r14->m140_maxY;

    //608018E
    r14->m160_deltaTranslation = r14->m8_pos - r14->m14_oldPos;

    // Adjust pitch min when close to maxY (ceiling)
    // Early-out when far from boundary to avoid s32 overflow in the multiplication
    if ((r14->m140_maxY - r14->m8_pos[1]) > 0x200000)
    {
        r14->m148_pitchMin = -0x3555555;
    }
    else
    {
        fixedPoint r6 = (r14->m140_maxY - r14->m8_pos[1]) * -0x111;
        fixedPoint r2 = (r6 >= -0x3555555) ? r6 : fixedPoint(-0x3555555);
        fixedPoint r3;
        if (r2 >= 0)
            r3 = 0;
        else if (r6 >= -0x3555555)
            r3 = r6;
        else
            r3 = -0x3555555;
        r14->m148_pitchMin = r3;
    }

    // Adjust pitch max when close to minY (floor)
    if ((r14->m8_pos[1] - r14->m134_minY) > 0x200000)
    {
        r14->m14C_pitchMax = 0x3555555;
    }
    else
    {
        fixedPoint r5 = (r14->m8_pos[1] - r14->m134_minY) * 0x111;
        fixedPoint r2 = (r5 < 0x3555555) ? r5 : fixedPoint(0x3555555);
        fixedPoint r3;
        if (r2 < 0)
            r3 = 0;
        else if (r5 < 0x3555555)
            r3 = r5;
        else
            r3 = 0x3555555;
        r14->m14C_pitchMax = r3;
    }
}

void dragonFlightUpdate(s_dragonTaskWorkArea* r4)
{
    r4->m24A_runningCameraScript = 0;

    switch (r4->m104_dragonScriptStatus)
    {
    case 0:
        r4->m154_dragonSpeed = 0;
        getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m8 &= ~1;
        r4->mF8_Flags |= 0x400;
        r4->mF8_Flags &= ~0x20000;
        r4->m104_dragonScriptStatus++;
    case 1:
        if (!isDragonInFieldBounds(r4))
        {
            r4->mF0 = dragonExitField;
            r4->m104_dragonScriptStatus = 0;
            dragonExitField(r4);
            return;
        }
    }

    r4->m160_deltaTranslation[0] = 0;
    r4->m160_deltaTranslation[1] = 0;
    r4->m160_deltaTranslation[2] = 0;

    r4->m238 &= ~3;

    switch (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m0_inputType)
    {
    case 1:
        updateDragonMovementDigital(r4);
        break;
    case 2:
        updateDragonMovementAnalog(r4);
        break;
    default:
        assert(0);
    }

    integrateDragonMovement(r4);
}

void dragonFieldTaskInitSub4Sub4()
{
    activateDragonFlight();
}

void dragonExitField(s_dragonTaskWorkArea* r14)
{
    s_visibilityGridWorkArea* r12 = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;
    //r13 = r4->m178

    r14->m24A_runningCameraScript = 6;
    getFieldTaskPtr()->m28_status |= 0x200;
    getFieldTaskPtr()->m28_status |= 0x10000;
    getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m8 |= 1;

    switch (r14->m104_dragonScriptStatus)
    {
    case 0:
        startScriptLeaveArea();
        initDragonSpeed(0);
        r14->m_1C4 = 45;
        r14->m104_dragonScriptStatus++;
    case 1:
        if (r14->m_1C4--)
            break;
        r14->m_1C4 = 1;
        r14->m104_dragonScriptStatus++;
        break;
    case 2:
        if (getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m3C_multichoiceTask == 0)
        {
            dragonFieldTaskInitSub4Sub4();
            r14->mF0(r14);
            getFieldTaskPtr()->m28_status &= 0xFFFFFDFF;
            return;
        }
        break;
    default:
        assert(0);
        break;
    }

    //607FA40
    r14->m238 &= 0xFFFFFFFC;
    sVec3_FP var_8;
    var_8[0] = r12->mC[0] - r14->m8_pos[0];
    var_8[1] = 0;
    var_8[2] = r12->mC[2] - r14->m8_pos[2];

    sVec2_FP var0;
    computeLookAt(var_8, var0);

    // update yaw
    s32 tempRotX = r14->m3C_targetAngles[0] - r14->m20_angle[0];
    if (tempRotX & 0x8000000)
    {
        tempRotX |= 0xF0000000;
    }
    else
    {
        tempRotX &= 0xFFFFFFF;
    }

    r14->m20_angle[0] += r14->m3C_targetAngles[0] - intDivide(0x10, (tempRotX << 4) - tempRotX) - r14->m20_angle[0];

    // update pitch
    r14->m20_angle[1] = interpolateRotation(r14->m20_angle[1], var_8[0], 0x2000, 0x444444, 0x10);

    // update roll
    s32 tempRotZ = r14->m3C_targetAngles[2] - r14->m20_angle[2];
    if (tempRotZ & 0x8000000)
    {
        tempRotZ |= 0xF0000000;
    }
    else
    {
        tempRotZ &= 0xFFFFFFF;
    }
    r14->m20_angle[2] += r14->m3C_targetAngles[2] - intDivide(0x10, (tempRotZ << 4) - tempRotZ) - r14->m20_angle[2];

    if (r14->m30 - r14->m20_angle[1] < r14->m178_turnRate)
    {
        r14->m20_angle[2] += intDivide(4, r14->m178_turnRate * 3);
    }
    else if (r14->m20_angle[1] - r14->m30 < r14->m178_turnRate)
    {
        r14->m20_angle[2] -= intDivide(4, r14->m178_turnRate * 3);
    }

    //607FB2A
    // clamp angle.x to valid range
    if (r14->m20_angle[0].normalized() > r14->m14C_pitchMax)
    {
        r14->m20_angle[0] = r14->m14C_pitchMax;
    }

    if (r14->m20_angle[0].normalized() < r14->m148_pitchMin)
    {
        r14->m20_angle[0] = r14->m148_pitchMin;
    }

    //607FB96
    buildDragonRotationMatrix(&r14->m48, &r14->m20_angle);
    copyMatrix(&r14->m48.m0_matrix, &r14->m88_matrix);

    r14->m15C_dragonSpeedIncrement = 0;
    r14->m154_dragonSpeed = 0;

    computeDragonDeltaTranslation(r14);

    r14->m8_pos += r14->m160_deltaTranslation;

    // this is all copied from dragonExitField but looks like it's exactly the same
    if ((r14->m134_minY == 0) && (r14->m140_maxY == 0))
        return;

    if (r14->m8_pos[1] < r14->m134_minY)
        r14->m8_pos[1] = r14->m134_minY;

    if (r14->m8_pos[1] > r14->m140_maxY)
        r14->m8_pos[1] = r14->m140_maxY;

    r14->m160_deltaTranslation = r14->m8_pos - r14->m14_oldPos;

    // Adjust pitch min when close to maxY (ceiling)
    // Early-out when far from boundary to avoid s32 overflow in the multiplication
    if ((r14->m140_maxY - r14->m8_pos[1]) > 0x200000)
    {
        r14->m148_pitchMin = -0x3555555;
    }
    else
    {
        fixedPoint r6 = (r14->m140_maxY - r14->m8_pos[1]) * -0x111;
        fixedPoint r2 = (r6 >= -0x3555555) ? r6 : fixedPoint(-0x3555555);
        fixedPoint r3;
        if (r2 >= 0)
            r3 = 0;
        else if (r6 >= -0x3555555)
            r3 = r6;
        else
            r3 = -0x3555555;
        r14->m148_pitchMin = r3;
    }

    // Adjust pitch max when close to minY (floor)
    if ((r14->m8_pos[1] - r14->m134_minY) > 0x200000)
    {
        r14->m14C_pitchMax = 0x3555555;
    }
    else
    {
        fixedPoint r5 = (r14->m8_pos[1] - r14->m134_minY) * 0x111;
        fixedPoint r2 = (r5 < 0x3555555) ? r5 : fixedPoint(0x3555555);
        fixedPoint r3;
        if (r2 < 0)
            r3 = 0;
        else if (r5 < 0x3555555)
            r3 = r5;
        else
            r3 = 0x3555555;
        r14->m14C_pitchMax = r3;
    }
}

void updateDragonCollision(s_dragonTaskWorkArea* r4)
{
    sVec3_FP var;

    var[0] = r4->m160_deltaTranslation[0] * 0x100;
    var[1] = r4->m160_deltaTranslation[1] * 0x100;
    var[2] = r4->m160_deltaTranslation[2] * 0x100;

    s32 r0 = dot3_FP(&var, &var);
    r4->m154_dragonSpeed = sqrt_F(r0) >> 8;
}

void updateCameraScriptSub0(p_workArea r4)
{
    if (r4)
    {
        assert(0); // r4 should be a pointer to something
    }
}

void dummyFunct(sFieldCameraStatus*)
{
    assert(0);
}

void(*activateCameraFollowModeTable1[10])(sFieldCameraStatus*) = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    dummyFunct,
    0,
    0,
};

void cameraFollowMode_idle(sFieldCameraStatus* r4);
void cameraFollowMode_scriptTarget(sFieldCameraStatus* r4);

void(*activateCameraFollowModeTable2[10])(sFieldCameraStatus*) = {
    cameraFollowMode_scriptTarget,           // [0] 06062900
    fieldOverlaySubTaskInitSub2,             // [1] 060621C6
    fieldOverlaySubTaskInitSub2_mode2,       // [2] 06062228
    dummyFunct,                              // [3] 0606229E — TODO
    dummyFunct,                              // [4] 06062302 — TODO
    dummyFunct,                              // [5] 06062370 — TODO
    dummyFunct,                              // [6] 0606240C — TODO
    0,
    cameraFollowMode_idle,           // [8] 06062474
    0,
};

s32 setCameraFollowFunctions(u32 r4, void(*r5)(sFieldCameraStatus*), void(*r6)(sFieldCameraStatus*))
{
    if (isFieldCameraSlotActive(r4))
    {
        sFieldCameraStatus* pCamera = &getFieldTaskPtr()->m8_pSubFieldData->m334->m3E4_cameraSlots[r4];
        pCamera->m74_updateFunc = r5;
        pCamera->m78_drawFunc = r6;
        pCamera->m8D_followState = 0;
        pCamera->m8E_followSubState = 0;
        return 1;
    }
    return 0;
}

void activateCameraFollowMode(u32 r4)
{
    setCameraFollowFunctions(0, activateCameraFollowModeTable1[r4], activateCameraFollowModeTable2[r4]);

    getFieldCameraStatus()->m8D_followState = 1;
}

void updateCameraScriptSub0Sub2(s_dragonTaskWorkArea* r4)
{
    s32 r2 = r4->m21C_DragonSpeedValues[0] + r4->m21C_DragonSpeedValues[1];

    if (0 > r2)
        r2++;
    r2 >>= 1;
    if (r4->m154_dragonSpeed > r2)
    {
        if (r4->m23A_dragonAnimation == 0)
            return;
        if (r4->m23A_dragonAnimation == 2)
            return;

        r4->m238 = 4;
        r4->m237 = 4;
        r4->m244 = 0;
    }
    else
    {
        if (r4->m23A_dragonAnimation == 5)
            return;

        r4->m238 = 0;
        r4->m237 = 0;
        r4->m244 = 5;
    }

    r4->m23C |= 5;
}

void updateCameraScript(s_dragonTaskWorkArea* r4, s_cameraScript* r5)
{
    r4->m24A_runningCameraScript = 1;
    getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m8 |= 1; // Disable LCS?

    switch (r4->m104_dragonScriptStatus)
    {
    case 0:
        updateCameraScriptSub0(r4->mB8_lightWingEffect);
        activateCameraFollowMode(0);
        r4->mF8_Flags &= 0xFFFFFBFF;
        r4->mF8_Flags |= 0x20000;
        r4->m1E8_cameraScriptDelay = r5->m20_length;
        r4->m20_angle = r5->mC_rotation;
        r4->m8_pos = r5->m0_position;

        r4->m160_deltaTranslation[0] = MTH_Mul(-r5->m1C, getSin(r4->m20_angle[1].getInteger() & 0xFFF));
        r4->m160_deltaTranslation[1] = intDivide(r4->m1E8_cameraScriptDelay, r5->m18 - r4->m8_pos[1]);
        r4->m160_deltaTranslation[2] = MTH_Mul(-r5->m1C, getCos(r4->m20_angle[1].getInteger() & 0xFFF));

        updateDragonCollision(r4);
        updateCameraScriptSub0Sub2(r4);

        getFieldCameraStatus()->m0_position = r5->m24_pos2;
        getFieldCameraStatus()->m88 = r4->m1E8_cameraScriptDelay;
        r4->m104_dragonScriptStatus++;
        break;
    case 1:
        if (--r4->m1E8_cameraScriptDelay)
        {
            return;
        }
        getFieldCameraStatus()->m88 = 30;
        r4->m1E8_cameraScriptDelay = 30;
        activateCameraFollowMode(getFieldTaskPtr()->m8_pSubFieldData->m334->m50E_followModeIndex);
        r4->m104_dragonScriptStatus++;
        break;
    case 2:
        if (--r4->m1E8_cameraScriptDelay)
        {
            return;
        }
        r4->m1D0_cameraScript = NULL;
        dragonFieldTaskInitSub4Sub4();
        break;
    default:
        assert(0);
    }
}

void dragonScriptMovement(s_dragonTaskWorkArea* pTypedWorkArea)
{
    getFieldTaskPtr()->m28_status |= 0x10000;

    if (pTypedWorkArea->m1D4_cutsceneData == nullptr)
    {
        if (pTypedWorkArea->m1D0_cameraScript == nullptr)
        {
            dragonFieldTaskInitSub4Sub3(getFieldTaskPtr()->m8_pSubFieldData->m334->m50E_followModeIndex);
            dragonFieldTaskInitSub4Sub4();
        }
        else
        {
            updateCameraScript(pTypedWorkArea, pTypedWorkArea->m1D0_cameraScript);
        }
    }
    else
    {
        assert(0);
    }

    buildDragonRotationMatrix(&pTypedWorkArea->m48, &pTypedWorkArea->m20_angle);

    copyMatrix(&pTypedWorkArea->m48.m0_matrix, &pTypedWorkArea->m88_matrix);

    pTypedWorkArea->m8_pos[0] += pTypedWorkArea->m160_deltaTranslation[0];
    pTypedWorkArea->m8_pos[1] += pTypedWorkArea->m160_deltaTranslation[1];
    pTypedWorkArea->m8_pos[2] += pTypedWorkArea->m160_deltaTranslation[2];

    updateDragonCollision(pTypedWorkArea);
}

void dragonFieldTaskInitSub5(s_dragonTaskWorkArea* pTypedWorkArea)
{
    PDS_unimplemented("dragonFieldTaskInitSub5");
}

bool shouldLoadPup()
{
    if (mainGameState.getBit(0x29 * 8 + 0))
    {
        assert(0);
    }
    return false;
}

void s_dragonTaskWorkArea::Init(s_dragonTaskWorkArea* pThis, s32 arg)
{
    getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask = pThis;

    getMemoryArea(&pThis->m0, 0);
    dragonFieldTaskInitSub2(pThis);
    dragonFieldTaskInitSub3(pThis, gDragonState, 5);
    pThis->mF0 = dragonScriptMovement;

    createSubTask<s_DragonRiderTask>(pThis);

    if ((gDragonState->mC_dragonType == DR_LEVEL_6_LIGHT_WING) && (pThis->mB8_lightWingEffect == nullptr))
    {
        Unimplemented();
    }

    dragonFieldTaskInitSub5(pThis);

    if (shouldLoadPup())
    {
        assert(0);
    }
}

void dragonFieldTaskUpdateSub1(s_dragonTaskWorkArea* pTypedWorkArea)
{
    pTypedWorkArea->mFC = 0;
    pTypedWorkArea->m1F0.m_8 = 0;
    pTypedWorkArea->m1F0.m_C = 0;

    pTypedWorkArea->m14_oldPos = pTypedWorkArea->m8_pos;

    pTypedWorkArea->mF0(pTypedWorkArea);

    resolveDragonTerrainCollision();
}

s32 getDragonSpeedIndex(s_dragonTaskWorkArea* pTypedWorkArea)
{
    if (pTypedWorkArea->mF8_Flags & 0x20000)
    {
        s32 r1 = pTypedWorkArea->m21C_DragonSpeedValues[0] + pTypedWorkArea->m21C_DragonSpeedValues[1];

        //Average speeds
        if (0 > r1)
            r1++;
        r1 >>= 1;

        if (pTypedWorkArea->m154_dragonSpeed >= r1)
        {
            return 1;
        }
        return 0;
    }
    else
    {
        return pTypedWorkArea->m235_dragonSpeedIndex;
    }
}

void dragonFieldPlayAnimation(s_dragonTaskWorkArea* r14, s_dragonState* r13, u8 r12)
{
    if (r14->m23A_dragonAnimation == r12)
        return;

    u8 r4 = r14->m23A_dragonAnimation;
    if (r4 == 0)
    {
        r4 = 2;
    }

    u8 r5;
    if (r12)
    {
        r5 = r12;
    }
    else
    {
        r5 = 2;
    }

    if (r5 == r4)
    {
        setupModelAnimation(&r13->m28_dragon3dModel, r13->m0_pDragonModelBundle->getAnimation(r13->m20_dragonAnimOffsets[r12]));
        r14->m23B = 1;
    }
    else
    {
        playAnimation(&r13->m28_dragon3dModel, r13->m0_pDragonModelBundle->getAnimation(r13->m20_dragonAnimOffsets[r12]), 10);
        r14->m23B = 0;
    }

    updateAndInterpolateAnimation(&r13->m28_dragon3dModel);

    r14->m23A_dragonAnimation = r12;
    r14->m237 = r14->m238;
}

s8 dragonFieldAnimation[] = {
    5,7,8,11,9,9,9,10,
    4,4,4,11,0,0,2,10,
};

s32 getDragonFieldAnimation(s_dragonTaskWorkArea* pTypedWorkArea)
{
    return dragonFieldAnimation[(pTypedWorkArea->m238 >> 2) * 8 + pTypedWorkArea->m238];
}

std::vector<s8> getFieldDragonAnimTable(int type, int subtype)
{
    sSaturnPtr EA = readSaturnEA(sSaturnPtr{ gFieldDragonAnimTableEA.m_offset + ((type * 5) + subtype) * 4, gFieldDragonAnimTableEA.m_file });

    std::vector<s8> result;

    int numEntries = 0;
    while (readSaturnS8(EA) != -1)
    {
        result.push_back(readSaturnS8(EA));
        EA += 1;
        numEntries++;
    }
    result.push_back(-1);

    return result;
}

void dragonFieldAnimationUpdate(s_dragonTaskWorkArea* pTypedWorkArea, s_dragonState* r5)
{
    u8 var = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m235_dragonSpeedIndex;

    if (pTypedWorkArea->m23C & 4)
    {
        switch (pTypedWorkArea->m23C & 3)
        {
        case 1:
            dragonFieldPlayAnimation(pTypedWorkArea, r5, pTypedWorkArea->m244);
            pTypedWorkArea->m23C &= ~1;
            break;
        case 2:
            assert(0);
        default:
            break;
        }

        pTypedWorkArea->m23C &= ~4;
    }
    else
    {
        // when activating LCS
        if (getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m8 & 0x10)
        {
            pTypedWorkArea->m238 |= 3;
            dragonFieldPlayAnimation(pTypedWorkArea, r5, getDragonFieldAnimation(pTypedWorkArea));
            return;
        }
        else
        {
            if (pTypedWorkArea->m238 & 4)
            {
                if (getDragonSpeedIndex(pTypedWorkArea) <= 0)
                {
                    pTypedWorkArea->m238 &= ~4;
                    if (pTypedWorkArea->m154_dragonSpeed > 0xDDD)
                    {
                        //060734B6
                        dragonFieldPlayAnimation(pTypedWorkArea, r5, getDragonFieldAnimation(pTypedWorkArea));
                        return;
                    }
                }
            }
            //6073508
            else if (getDragonSpeedIndex(pTypedWorkArea) > 0)
            {
                pTypedWorkArea->m238 |= 4;
                if (pTypedWorkArea->m154_dragonSpeed < 0x555)
                {
                    dragonFieldPlayAnimation(pTypedWorkArea, r5, getDragonFieldAnimation(pTypedWorkArea));
                    return;
                }
            }

            //6073524
            if (r5->m28_dragon3dModel.m16_previousAnimationFrame)
            {
                return;
            }

            //6073530
            if (pTypedWorkArea->m23C)
            {
                switch (pTypedWorkArea->m23C)
                {
                case 1:
                    dragonFieldPlayAnimation(pTypedWorkArea, r5, pTypedWorkArea->m244);
                    pTypedWorkArea->m23C &= ~1;
                    break;
                case 2:
                    assert(0);
                default:
                    break;
                }

                return;
            }

            //06073588
            if (var >= 0)
            {
                s32 r6 = getDragonFieldAnimation(pTypedWorkArea);
                if (r6 == 0)
                {
                    std::vector<s8> r3 = getFieldDragonAnimTable(r5->mC_dragonType, r5->m1C_dragonArchetype);

                    pTypedWorkArea->m239++;
                    r6 = r3[pTypedWorkArea->m239];

                    if (r6 < 0)
                    {
                        pTypedWorkArea->m239 = 0;
                        dragonFieldPlayAnimation(pTypedWorkArea, r5, r3[pTypedWorkArea->m239]);
                    }
                    else
                    {
                        dragonFieldPlayAnimation(pTypedWorkArea, r5, r6);
                    }
                }
                if (r6 > 0)
                {
                    dragonFieldPlayAnimation(pTypedWorkArea, r5, r6);
                }
                return;
            }
            else
            {
                dragonFieldPlayAnimation(pTypedWorkArea, r5, 5);
                return;
            }

            assert(0);
        }
    }
    PDS_unimplemented("dragonFieldAnimationUpdate");
}

void playDragonSoundEffect(s_dragonTaskWorkArea* pTypedWorkArea, s_dragonState* r5)
{
    PDS_unimplemented("playDragonSoundEffect");
}

void selectCamera(sFieldCameraManager* r4, s_dragonTaskWorkArea* r5)
{
    s32 r6 = r4->mC;

    while (--r6)
    {
        s_fieldCameraConfig* r7 = &r4->m10[r6];
        if (r6) // don't 2d check on first camera so it's always taken
        {
            if (r5->m8_pos[0] <= r7->m0_min[0])
                continue;
            if (r5->m8_pos[2] <= r7->m0_min[2])
                continue;
            if (r5->m8_pos[0] >= r7->mC_max[0])
                continue;
            if (r5->m8_pos[2] >= r7->mC_max[2])
                continue;
        }

        if ((r4->m0_nextCamera != r6) && (r4->m4_currentCamera == r6))
        {
            r4->m8_numFramesOnCurrentCamera++;
        }
        else
        {
            r4->m4_currentCamera = r6;
            r4->m8_numFramesOnCurrentCamera = 0;
        }

        if (r4->m8_numFramesOnCurrentCamera >= 30)
        {
            r4->m0_nextCamera = r6;
            r4->m8_numFramesOnCurrentCamera = 0;
        }

        return;
    }
}

void selectCameraZone(sFieldCameraManager* r4, s_dragonTaskWorkArea* r5)
{
    if (r4->m2E0_forcedZoneIndex)
    {
        assert(0);
    }

    s32 r13 = r4->m2DC_numCameraZones;
    s32 var8[3];

    for (s32 r13 = r4->m2DC_numCameraZones; r13 >= 0; r13--)
    {
        if (r13)
        {
            sFieldCameraZone* r12 = &r4->m2E4_cameraZones[r13];
            s32 r6 = r5->m8_pos[0] - r12->m0_center[0];
            if (r6 >= 0)
            {
                var8[0] = r6;
            }
            else
            {
                var8[0] = r12->m0_center[0] - r5->m8_pos[0];
            }

            if (var8[0] >= r12->m14_triggerRadius)
                continue;

            if (r5->m8_pos[2] - r12->m0_center[2] < 0)
            {
                var8[2] = r5->m8_pos[2] - r12->m0_center[2];
            }
            else
            {
                var8[2] = r12->m0_center[2] - r5->m8_pos[2];
            }

            if (var8[2] >= r12->m14_triggerRadius)
                continue;

            if (MTH_Mul(var8[0], var8[0]) + MTH_Mul(var8[2], var8[2]) >= r12->m18_maxDistanceSquare)
                continue;
        }

        //6061554
        if ((r4->m2D0_activeZoneIndex != r13) && (r4->m2D4_candidateZoneIndex == r13))
        {
            r4->m2D8_zoneDwellCounter++;
        }
        else
        {
            r4->m2D4_candidateZoneIndex = r13;
            r4->m2D8_zoneDwellCounter = 0;
        }

        if (r4->m2D8_zoneDwellCounter >= 30)
        {
            r4->m2D0_activeZoneIndex = r13;
            r4->m2D8_zoneDwellCounter = 0;
        }

        return;
    }
}

void cameraFollowMode_scriptTarget(sFieldCameraStatus* r4)
{
    s_dragonTaskWorkArea* pDragonTask = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    switch (r4->m8D_followState)
    {
    case 0:
    case 1:
        r4->m8F_followType = 0;
        r4->m90_followMode = 0;
        r4->m8D_followState = 2;
    case 2:
    default:
        if (pDragonTask->m1D0_cameraScript)
        {
            sVec3_FP r15_8 = pDragonTask->m8_pos - pDragonTask->m1D0_cameraScript->m24_pos2;

            sVec2_FP r15;
            computeLookAt(r15_8, r15);

            r4->mC_rotation[0] = r15[0];
            r4->mC_rotation[1] = r15[1];

            r4->m24_distanceToDestination = vecDistance(pDragonTask->m1D0_cameraScript->m24_pos2, pDragonTask->m8_pos);

            if (r4->m24_distanceToDestination < pDragonTask->m1D0_cameraScript->m30_thresholdDistance)
            {
                r4->m0_position = pDragonTask->m1D0_cameraScript->m24_pos2;
            }
            else
            {
                r4->m24_distanceToDestination = pDragonTask->m1D0_cameraScript->m30_thresholdDistance;

                fixedPoint var20 = -MTH_Mul_5_6(pDragonTask->m1D0_cameraScript->m30_thresholdDistance, getCos(r4->mC_rotation[0].getInteger() & 0xFFF), getSin(r4->mC_rotation[1].getInteger() & 0xFFF));
                fixedPoint var1C = MTH_Mul(r4->m24_distanceToDestination, getSin(r4->mC_rotation[0].getInteger() & 0xFFF));
                fixedPoint var18 = -MTH_Mul_5_6(pDragonTask->m1D0_cameraScript->m30_thresholdDistance, getCos(r4->mC_rotation[0].getInteger() & 0xFFF), getCos(r4->mC_rotation[1].getInteger() & 0xFFF));

                r4->m0_position[0] = pDragonTask->m8_pos[0] - var20;
                r4->m0_position[1] = pDragonTask->m8_pos[1] - var1C;
                r4->m0_position[2] = pDragonTask->m8_pos[2] - var18;
            }
        }
        else
        {
            assert(0);
        }
    }
}

void cameraFollowMode_idle(sFieldCameraStatus* r4)
{
    s_dragonTaskWorkArea* pDragonTask = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    if (pDragonTask == nullptr)
        return;

    switch (r4->m8D_followState)
    {
    case 0:
        r4->m8F_followType = 8;
        r4->m90_followMode = 8;
        r4->m7C = 0;
        r4->m8D_followState = 2;
    case 1:
        fieldOverlaySubTaskInitSub2Sub1Sub2(r4, pDragonTask);
        return;
    default:
        assert(0);
    }
}

void updateZoneCameraFollow(sFieldCameraManager* r4)
{
    assert(0);
}

void updateFieldCameraSlots()
{
    sFieldCameraManager* r13 = getFieldTaskPtr()->m8_pSubFieldData->m334;
    s_dragonTaskWorkArea* r14 = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;

    if (r14)
    {
        selectCamera(r13, r14);
        selectCameraZone(r13, r14);
    }

    for (int i = 0; i < 2; i++)
    {
        if (isFieldCameraSlotActive(i))
        {
            if (r13->m37C_isCutsceneCameraActive)
            {
                updateCutsceneCameraInterpolation(r13, &r13->m3E4_cameraSlots[i]);
            }
            else
            {
                if ((r13->m3E4_cameraSlots[i].m74_updateFunc == cameraFollowMode_scriptTarget) || (r13->m3E4_cameraSlots[i].m74_updateFunc == cameraFollowMode_idle))
                {
                    r13->m3E4_cameraSlots[i].m74_updateFunc(&r13->m3E4_cameraSlots[i]);
                }
                else
                {
                    if ((r13->m2E0_forcedZoneIndex > 0) || (r13->m2D0_activeZoneIndex > 0))
                    {
                        updateZoneCameraFollow(r13);
                    }
                    else
                    {
                        if (r13->m3E4_cameraSlots[i].m74_updateFunc)
                        {
                            r13->m3E4_cameraSlots[i].m74_updateFunc(&r13->m3E4_cameraSlots[i]);
                        }
                    }
                }
            }
            r13->m3E4_cameraSlots[i].m84++;
        }
    }
}

// 06012674 — per-field terrain callback
static void fieldTerrainCallback(s32 subFieldIndex, sVec3_FP* pPos, sVec3_FP* pAngle)
{
    // Table at 0x0020a388 indexed by current field index
    // Many entries are null (e.g., field 8, 0xC, 0xD, 0xE)
    // When null, this is a no-op
    Unimplemented();
}

// 0602f8e4 — read dragon pos and transform for special fields (0xF, 0x12)
static void readAndTransformDragonPos(sVec3_FP* pOut)
{
    s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    pOut->m0_X = pDragon->m8_pos[0];
    pOut->m4_Y = pDragon->m8_pos[1];
    pOut->m8_Z = pDragon->m8_pos[2];
    Unimplemented(); // FUN_0602f87c — transform pos based on sub-field offsets
}

// 06073c7a
void updateDragonSavedAnglesAndTerrain(s_dragonTaskWorkArea* r14)
{
    r14->m2C_savedPitch = r14->m20_angle[0];
    r14->m30 = r14->m20_angle[1];
    r14->m34_savedRoll = r14->m20_angle[2];

    fixedPoint speed = r14->m154_dragonSpeed;
    if (speed < 0) speed = -speed;
    r14->m38_distanceAccum = r14->m38_distanceAccum + speed;

    auto pFieldTask = getFieldTaskPtr();
    if (pFieldTask->m2C_currentFieldIndex == 0xF || pFieldTask->m2C_currentFieldIndex == 0x12)
    {
        sVec3_FP transformedPos;
        readAndTransformDragonPos(&transformedPos);
        fieldTerrainCallback(0, &transformedPos, nullptr);
    }
    else
    {
        fieldTerrainCallback((s32)pFieldTask->m2E_currentSubFieldIndex, &r14->m8_pos, &r14->m20_angle);
    }
}

void s_dragonTaskWorkArea::Update(s_dragonTaskWorkArea* pTypedWorkArea)
{
    dragonFieldTaskUpdateSub1(pTypedWorkArea);

    fieldTaskVar2 = gDragonState->mC_dragonType;

    if (pTypedWorkArea->mB8_lightWingEffect)
    {
        assert(0);
    }

    if (gDragonState->mC_dragonType != pTypedWorkArea->m100_previousDragonType)
    {
        pTypedWorkArea->m100_previousDragonType = gDragonState->mC_dragonType;
        dragonFieldTaskUpdateSub2(gDragonState->mC_dragonType + 1);

        if (gDragonState->mC_dragonType == DR_LEVEL_8_FLOATER)
        {
            assert(0);
        }
    }

    updateAndInterpolateAnimation(&gDragonState->m28_dragon3dModel);
    updateAndInterpolateAnimation(&pRider1State->m18_3dModel);
    if (mainGameState.gameStats.m3_rider2)
    {
        updateAndInterpolateAnimation(&pRider2State->m18_3dModel);
    }

    dragonFieldAnimationUpdate(pTypedWorkArea, gDragonState);
    playDragonSoundEffect(pTypedWorkArea, gDragonState);

    updateAnimationMatrices(&gDragonState->m78_animData, &gDragonState->m28_dragon3dModel);

    if (pTypedWorkArea->m25D != 2)
    {
        getFieldCameraStatus()->m30 = -pTypedWorkArea->m20_angle[2] / 2;
    }

    updateFieldCameraSlots();

    updateDragonSavedAnglesAndTerrain(pTypedWorkArea);
}

s8 isFieldCameraSlotActive(s32 index)
{
    return getFieldTaskPtr()->m8_pSubFieldData->m334->m3E4_cameraSlots[index].m8C_isActive;
}

void drawFieldCameraSlots()
{
    sFieldCameraManager* r12 = getFieldTaskPtr()->m8_pSubFieldData->m334;

    for (int i = 0; i < 2; i++)
    {
        if (isFieldCameraSlotActive(i))
        {
            if (r12->m37C_isCutsceneCameraActive == 0)
            {
                sFieldCameraStatus* pFieldCameraStatus = &r12->m3E4_cameraSlots[i];
                if (pFieldCameraStatus->m78_drawFunc)
                {
                    pFieldCameraStatus->m78_drawFunc(pFieldCameraStatus);
                }
            }
        }
    }

    applyCameraStatusToEngine(r12);
}

void printMainDebugStats(s_dragonTaskWorkArea* pTypedWorkArea)
{
    if (enableDebugTask)
    {
        assert(0);
    }
}

void dragonFieldTaskDrawSub1(s_dragonTaskWorkArea* pTypedWorkArea)
{
    drawFieldCameraSlots();

    if ((pTypedWorkArea->m_EC & 1) == 0)
    {
        s_RGB8* pColor;
        sVec3_FP lightLocation;
        if (pTypedWorkArea->m_EB_useSpecialColor)
        {
            dragonFieldTaskDrawSub1Sub0();

            sVec3_FP varC;
            varC[0] = pTypedWorkArea->m8_pos[0];
            varC[1] = pTypedWorkArea->m8_pos[1] + 0xA000;
            varC[2] = pTypedWorkArea->m8_pos[2];
            
            transformVecByCurrentMatrix(varC, lightLocation);

            dragonFieldTaskDrawSub1Sub1(lightLocation[0], lightLocation[1], lightLocation[2]);
            pColor = &pTypedWorkArea->m_E8_specialColor;
        }
        else
        {
            sVec3_FP varC;
            varC[0] = -MTH_Mul_5_6(fixedPoint(0x10000), getCos(pTypedWorkArea->mC0_lightRotationAroundDragon.getInteger()), getSin(pTypedWorkArea->mC4.getInteger()));
            varC[1] = MTH_Mul(fixedPoint(0x10000), getSin(pTypedWorkArea->mC0_lightRotationAroundDragon.getInteger()));
            varC[2] = -MTH_Mul_5_6(fixedPoint(0x10000), getCos(pTypedWorkArea->mC0_lightRotationAroundDragon.getInteger()), getCos(pTypedWorkArea->mC4.getInteger()));
            transformVecByCurrentMatrix(varC, lightLocation);
            pColor = &pTypedWorkArea->mC8_normalLightColor;
        }
        //060740F6
        setupLight(lightLocation[0], lightLocation[1], lightLocation[2], pColor->toU32());
        generateLightFalloffMap(pTypedWorkArea->mCB_falloffColor0.toU32(), pTypedWorkArea->mCE_falloffColor1.toU32(), pTypedWorkArea->mD1_falloffColor2.toU32());
    }

#ifndef SHIPPING_BUILD
    if (gDebugWindows.field)
    {
        if (ImGui::Begin("Field", &gDebugWindows.field))
        {
            Imgui_FP_Angle("Field of view", &pTypedWorkArea->m1CC_fieldOfView);
        }
        ImGui::End();
    }
#endif

    //0607416C
    initVDP1Projection(pTypedWorkArea->m1CC_fieldOfView / 2, 0);
    printMainDebugStats(pTypedWorkArea);
}

struct s_dragonHotspotPerDragonType
{
    u32 m0;
    u32 m4;
};

s_dragonHotspotPerDragonType dragonHotspotPerDragonType[DR_LEVEL_MAX][5] = {
    { {0, 0},{0,1},{17,1}, {9, 0}, {12,0} },
    { {15, 0},{15,1}, {20, 1}, {9, 0}, {26, 0} },
    { {12, 0},{12,1}, {16, 0}, {23, 0}, {26, 0} },
    { {1, 0}, {1,1}, {4,1}, {12,0}, {15,0} },
    { {1, 0}, {1,1}, {5,1}, {12,0}, {31,0} },
    { {10, 0}, {10,1}, {13,0}, {25,0}, {28,0}},
    { {1,0}, {1,1}, {2,1}, {23,0}, {25,0} },
    { {1,0}, {0,0}, {3,1}, {7,0}, {22,0} },
    { {0,3}, {0,4}, {0,0}, {0,1}, {0,0} },
};

void getDragonHotSpot(s_dragonState* r4, u32 r5, sVec3_FP* r6)
{
    s_dragonHotspotPerDragonType* pHotSpotData = &dragonHotspotPerDragonType[r4->mC_dragonType][r5];
    if (pHotSpotData->m0 < 0) // don't think that can ever happen
        return;

    sVec3_FP* pVec = &r4->m28_dragon3dModel.m44_hotpointData[pHotSpotData->m0][pHotSpotData->m4];
    transformAndAddVec(*pVec, *r6, cameraProperties2.m28[0]);
}

void dragonFieldTaskDrawSub3Sub0()
{
    s_visibilityGridWorkArea* r4 = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;
    s_visibilityGridWorkArea_1294* r5 = &r4->m1294;
    if (r5->m14 < r5->m10)
    {
        r5->m14 = r5->m10;
    }

    r5->m10 = 0;
    r5->mC = 0;
    r5->m8_processedQuadsForCollision3 = 0;
    r5->m4_processedQuadsForCollision2 = 0;

    r4->m44 = r4->m68.begin();
}

void dragonFieldTaskDrawSub3(s_dragonTaskWorkArea* pTypedWorkArea)
{
    if (((pTypedWorkArea->m_EC & 1) == 0) && (pTypedWorkArea->m_EB_useSpecialColor))
    {
        PDS_unimplemented("dragonFieldTaskDrawSub3 for setup light during dragon rendering");
    }

    dragonFieldTaskDrawSub3Sub0();
}

void s_dragonTaskWorkArea::Draw(s_dragonTaskWorkArea* pTypedWorkArea)
{
    dragonFieldTaskDrawSub1(pTypedWorkArea);

    // if we need to draw the dragon shadow (and dragon Y >= 0)
    if (!pTypedWorkArea->m249_noCollisionAndHideDragon && pTypedWorkArea->m248 && (pTypedWorkArea->m8_pos[1] >= 0))
    {
        assert(0);
    }

    if (pTypedWorkArea->m249_noCollisionAndHideDragon)
    {
        WRITE_BE_U16(gDragonState->m0_pDragonModelBundle->getRawBuffer() + 0x30, READ_BE_U16(gDragonState->m0_pDragonModelBundle->getRawBuffer() + 0x30) & ~1);
    }
    else
    {
        WRITE_BE_U16(gDragonState->m0_pDragonModelBundle->getRawBuffer() + 0x30, READ_BE_U16(gDragonState->m0_pDragonModelBundle->getRawBuffer() + 0x30) | 1);
    }

    pushCurrentMatrix();
    translateCurrentMatrix(&pTypedWorkArea->m8_pos);
    rotateCurrentMatrixShiftedY(0x8000000);
    multiplyCurrentMatrixSaveStack(&pTypedWorkArea->m48.m0_matrix);
    scaleCurrentMatrixRow0(pTypedWorkArea->m150);
    scaleCurrentMatrixRow1(pTypedWorkArea->m150);
    scaleCurrentMatrixRow2(pTypedWorkArea->m150);

    gDragonState->m28_dragon3dModel.mC_modelIndexOffset = gDragonState->m14_modelIndex;
    gDragonState->m28_dragon3dModel.m18_drawFunction(&gDragonState->m28_dragon3dModel);
    popMatrix();

    //06074438
    getDragonHotSpot(gDragonState, 2, &pTypedWorkArea->m10C_hotSpot2);
    getDragonHotSpot(gDragonState, 3, &pTypedWorkArea->m118_hotSpot3);
    getDragonHotSpot(gDragonState, 4, &pTypedWorkArea->m124_hotSpot4);

    if (pTypedWorkArea->m249_noCollisionAndHideDragon == 0)
    {
        if (mainGameState.gameStats.m2_rider1)
        {
            sVec3_FP rider1_hotSpot;
            getDragonHotSpot(gDragonState, 0, &rider1_hotSpot);

            pushCurrentMatrix();
            translateCurrentMatrix(&rider1_hotSpot);
            rotateCurrentMatrixShiftedY(0x8000000);
            multiplyCurrentMatrixSaveStack(&pTypedWorkArea->m48.m0_matrix);
            pRider1State->m18_3dModel.m18_drawFunction(&pRider1State->m18_3dModel);
            popMatrix();

            // draw rider's gun
            if (pRider1State->m18_3dModel.m44_hotpointData[5].size())
            {
                //060744AA
                transformAndAddVec(pRider1State->m18_3dModel.m44_hotpointData[5][0], rider1_hotSpot, cameraProperties2.m28[0]);
                pushCurrentMatrix();
                translateCurrentMatrix(&rider1_hotSpot);
                rotateCurrentMatrixShiftedY(0x8000000);
                multiplyCurrentMatrixSaveStack(&pTypedWorkArea->m48.m0_matrix);
                addObjectToDrawList(pRider1State->m0_riderBundle->get3DModel(pRider1State->m14_weaponModelIndex));
                popMatrix();
            }
        }
        //60744E4
        if (mainGameState.gameStats.m3_rider2)
        {
            sVec3_FP rider2_hotSpot;
            getDragonHotSpot(gDragonState, 1, &rider2_hotSpot);

            pushCurrentMatrix();
            translateCurrentMatrix(&rider2_hotSpot);
            rotateCurrentMatrixShiftedY(0x8000000);
            multiplyCurrentMatrixSaveStack(&pTypedWorkArea->m48.m0_matrix);
            pRider2State->m18_3dModel.m18_drawFunction(&pRider2State->m18_3dModel);
            popMatrix();
        }
    }

    //06074520
    dragonFieldTaskDrawSub3(pTypedWorkArea);
}

void initFieldDragon(s_workArea* pWorkArea, u32 param)
{
    if (getFieldTaskPtr()->updateDragonAndRiderOnInit == 1)
    {
        assert(0);
        //updateDragonIfCursorChanged(mainGameState.gameStats.dragonLevel);
        //loadRiderIfChanged(mainGameState.gameStats.rider1);
        //loadRider2IfChanged(mainGameState.gameStats.rider1);

        mainGameState.gameStats.m10_currentHP = mainGameState.gameStats.mB8_maxHP;
        mainGameState.gameStats.m14_currentBP = mainGameState.gameStats.mBA_maxBP;

        getFieldTaskPtr()->updateDragonAndRiderOnInit = 2;
    }

    if (gDragonState->mC_dragonType == DR_LEVEL_8_FLOATER)
    {
        assert(0);
    }
    else
    {
        createSubTaskWithArg<s_dragonTaskWorkArea, s32>(pWorkArea, param);
    }
}

u8 convertCameraPositionTo2dGrid(s_visibilityGridWorkArea* pFieldCameraTask1)
{
    u8 bDirty = 0;
    s32 X;

    if (pFieldCameraTask1->m28 & 1)
    {
        //06070FE0 — vertical grid mode (tower): grid X maps to world Y
        X = intDivide(pFieldCameraTask1->m20_cellDimensions[0], pFieldCameraTask1->m0_position[1]);
        if (pFieldCameraTask1->m0_position[1] < 0)
        {
            X--;
        }
    }
    else
    {
        X = intDivide(pFieldCameraTask1->m20_cellDimensions[0], pFieldCameraTask1->m0_position[0]);
        if (pFieldCameraTask1->m0_position[0] < 0)
        {
            X--;
        }
    }

    s32 Z = intDivide(pFieldCameraTask1->m20_cellDimensions[1], -pFieldCameraTask1->m0_position[2]);
    if (pFieldCameraTask1->m0_position[2] >= 0)
    {
        Z--;
    }

    if (pFieldCameraTask1->m18_cameraGridLocation[0] != X)
    {
        pFieldCameraTask1->m18_cameraGridLocation[0] = X;
        bDirty = 1;
    }

    if (pFieldCameraTask1->m18_cameraGridLocation[1] != Z)
    {
        pFieldCameraTask1->m18_cameraGridLocation[1] = Z;
        bDirty = 1;
    }

    if(!isShipping() && gDebugWindows.camera)
    {
        if (ImGui::Begin("Camera", &gDebugWindows.camera))
        {
            ImGui::Text("cell: X: %d, Y:%d", pFieldCameraTask1->m18_cameraGridLocation[0], pFieldCameraTask1->m18_cameraGridLocation[1]);
        }
        ImGui::End();
    }

    return bDirty;
}

s32 checkPositionVisibility(const sVec3_FP* r4, s32 r5)
{
#ifdef PDS_TOOL
    if (bMakeEverythingVisible)
    {
        return 0;
    }
#endif
    s_visibilityGridWorkArea* r13 = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;
    sVec3_FP var18 = cameraProperties2.m0_position;

    {
        fixedPoint r3 = (*r4)[0] - var18[0];
        if (r3 < 0)
        {
            r3 = var18[0] - (*r4)[0];
        }
        if (r3 > r5)
            return 1;
    }

    {
        fixedPoint r2 = (*r4)[1] - var18[1];
        if (r2 < 0)
        {
            r2 = var18[1] - (*r4)[1];
        }
        if (r2 > r5)
            return 1;
    }

    {
        fixedPoint r2 = (*r4)[2] - var18[2];
        if (r2 < 0)
        {
            r2 = var18[2] - (*r4)[2];
        }
        if (r2 > r5)
            return 1;
    }

    {
        sMatrix4x3* r5 = fieldCameraTask1DrawSub1();
        sVec3_FP varC;
        varC[0] = r5->m[0][2];
        varC[1] = r5->m[1][2];
        varC[2] = r5->m[2][2];

        var18[0] -= varC[0] * 32;
        var18[1] -= varC[1] * 32;
        var18[2] -= varC[2] * 32;

        sVec3_FP var0;

        var0[0] = (*r4)[0] - var18[0];
        var0[1] = (*r4)[1] - var18[1];
        var0[2] = (*r4)[2] - var18[2];

        if (dot3_FP(&r13->m12AC, &var0) <= 0)
            return 1;
        if (dot3_FP(&r13->m12B8, &var0) <= 0)
            return 1;
        if (dot3_FP(&r13->m12C4, &var0) <= 0)
            return 1;
        if (dot3_FP(&r13->m12D0, &var0) <= 0)
            return 1;

        return 0;
    }

    return 1;
}

std::vector<fixedPoint> fieldCameraTask1InitData1_depthRangeTable =
{
    fixedPoint(0x80000),
    fixedPoint(0x100000),
    fixedPoint(0x200000),
    fixedPoint(0x7FFFFFFF),
};

void s_visibilityGridWorkArea::fieldCameraTask1Init(s_visibilityGridWorkArea* pTypedWorkArea)
{
    getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1 = pTypedWorkArea;

    pTypedWorkArea->m12F8_convertCameraPositionToGrid = convertCameraPositionTo2dGrid;
    pTypedWorkArea->m12FC_isObjectClipped = checkPositionVisibility;
    pTypedWorkArea->m12F2_renderMode = 2;

    pTypedWorkArea->m18_cameraGridLocation[0] = -1;
    pTypedWorkArea->m18_cameraGridLocation[1] = -1;
    pTypedWorkArea->m30 = 0;
    pTypedWorkArea->m34_cameraVisibilityTable = 0;
    pTypedWorkArea->m2C_depthRangeTable = &fieldCameraTask1InitData1_depthRangeTable;
    pTypedWorkArea->m1300 = 3;
}

void s_visibilityGridWorkArea::fieldCameraTask1Update(s_visibilityGridWorkArea* pTypedWorkArea)
{
    pTypedWorkArea->m0_position = cameraProperties2.m0_position;
    pTypedWorkArea->m12F8_convertCameraPositionToGrid(pTypedWorkArea);
}

sMatrix4x3* fieldCameraTask1DrawSub1()
{
    return &getFieldTaskPtr()->m8_pSubFieldData->m334->m384_viewMatrix;
}

void s_visibilityGridWorkArea::fieldCameraTask1Draw(s_visibilityGridWorkArea* pTypedWorkArea)
{
    sMatrix4x3* r13 = fieldCameraTask1DrawSub1();

    asyncDivStart(graphicEngineStatus.m405C.m14_farClipDistance, fixedPoint(0xC422));

    sMatrix4x3 var90;
    copyMatrix(r13, &var90);
    rotateMatrixShiftedY(fixedPoint(0x238E38F), &var90);

    sMatrix4x3 var60;
    copyMatrix(r13, &var60);
    rotateMatrixShiftedY(fixedPoint(-0x238E38F), &var60);

    sMatrix4x3 var30;
    copyMatrix(r13, &var30);
    rotateMatrixShiftedX(fixedPoint(-0x238E38F), &var30);

    sMatrix4x3 var00;
    copyMatrix(r13, &var00);
    rotateMatrixShiftedX(fixedPoint(0x238E38F), &var00);

    pTypedWorkArea->m12AC[0] = var90.m[0][2]; // 8
    pTypedWorkArea->m12AC[1] = var90.m[1][2]; // 18
    pTypedWorkArea->m12AC[2] = var90.m[2][2]; // 28

    pTypedWorkArea->m12B8[0] = var60.m[0][2]; // 8
    pTypedWorkArea->m12B8[1] = var60.m[1][2]; // 18
    pTypedWorkArea->m12B8[2] = var60.m[2][2]; // 28

    pTypedWorkArea->m12C4[0] = var30.m[0][2]; // 8
    pTypedWorkArea->m12C4[1] = var30.m[1][2]; // 18
    pTypedWorkArea->m12C4[2] = var30.m[2][2]; // 28

    pTypedWorkArea->m12D0[0] = var00.m[0][2]; // 8
    pTypedWorkArea->m12D0[1] = var00.m[1][2]; // 18
    pTypedWorkArea->m12D0[2] = var00.m[2][2]; // 28

    pTypedWorkArea->m12DC = asyncDivEnd();

    if (getFieldTaskPtr()->m8_pSubFieldData->m370_fieldDebuggerWho & 1)
    {
        assert(0);
    }

    pTypedWorkArea->m12E0 = 0;
    pTypedWorkArea->m12E2 = 0;
    pTypedWorkArea->m12E4_numCollisionGeometries = 0;

    pTypedWorkArea->m12F0 = 0;
}

struct LCSTask : public s_workAreaTemplate<LCSTask>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { LCSTaskInit, NULL, LCSTaskDraw, NULL };
        return &taskDefinition;
    }

    static void LCSTaskInit(LCSTask*);
    static void LCSTaskDraw(LCSTask*);
};

void LCSTask::LCSTaskInit(LCSTask*)
{
    s_visibilityGridWorkArea* r4 = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;
    r4->m1290_vdp2VramOffset = 0x25E68000;
    setVdp2VramU32(r4->m1290_vdp2VramOffset, 0xFFFFFFFF);

    r4->m128C_vdp2VramOffset2 = 0x25E68800;
    setVdp2VramU32(r4->m128C_vdp2VramOffset2, 0xFFFFFFFF);

    r4->m44 = r4->m68.begin();
    r4->m68[0].m0_model = 0;
}

p_workArea createLCSShootTask(s_LCSTask* r4, sLCSTarget* r5)
{
    Unimplemented();
    return NULL;
}

void startScript_cantDestroy()
{
    queueNewFieldScript(sSaturnPtr({ 0x06080F4B, gFLD_A3 }), 1716);
}

void LCSUpdateCursorFromInputSub0(s32 r4, sVec3_FP* r5)
{
    sFieldCameraStatus* pCameraStatus = getFieldCameraStatus();

    if (r4)
    {
        assert(0);
    }
    if (r5)
    {
        pCameraStatus->m68_rotationImpulse += *r5;
    }
}

s8 LCSTaskDrawSub1Sub6()
{
    s8 r0 = !mainGameState.getBit(0x2B * 8 + 0);
    r0 ^= 1;
    return r0;
}

// Lock camera in LCS/CutScene
void LCSTaskDrawSub1Sub3()
{
    activateCameraFollowMode(8);
}

// Release camera at the end of LCS/CutScene
void LCSTaskDrawSub1Sub4()
{
    activateCameraFollowMode(getFieldTaskPtr()->m8_pSubFieldData->m334->m50E_followModeIndex);
}

void LCSTask::LCSTaskDraw(LCSTask*)
{
    LCSTaskDrawSub();
}

p_workArea createFieldCameraTask(s_workArea* pWorkArea)
{
    p_workArea pCameraTask = createSubTask<s_visibilityGridWorkArea>(pWorkArea);
    createSubTask<LCSTask>(pWorkArea);

    return pCameraTask;
}

void s_randomBattleWorkArea::randomBattleTaskInit(s_randomBattleWorkArea* pWorkArea)
{
    getFieldTaskPtr()->m8_pSubFieldData->m344_randomBattleTask = pWorkArea;
}

bool isBattleEnabled()
{
    if (fieldTaskPtr->m28_status == 0)
    {
        return true;
    }
    return false;
}

void s_randomBattleWorkArea::randomBattleTaskDraw(s_randomBattleWorkArea* pWorkArea)
{
    if ((pWorkArea->m4 == 2) || isBattleEnabled() || ((pWorkArea->m4 != 0) && ((getFieldTaskPtr()->m28_status & 0xFFFF) == 0)))
    {
        hasEncounterData = 1;
    }
    else
    {
        hasEncounterData = 0;
    }
}

void createRandomBattleTask(s_workArea* pWorkArea)
{
    createSubTask<s_randomBattleWorkArea>(pWorkArea);
}

void fieldDebugMenuUpdate1()
{
    s_FieldSubTaskWorkArea* pSubFieldData = getFieldTaskPtr()->m8_pSubFieldData;

    if (pSubFieldData->m37C_debugMenuStatus1[0])
    {
        return;
    }

    if (pSubFieldData->m369)
    {
        return;
    }

    if (pSubFieldData->m380_debugMenuStatus3 == 0)
    {
        if (readKeyboardToggle(KEY_CODE_F3))
        {
            pSubFieldData->m37C_debugMenuStatus1[1]++;
            pSubFieldData->m37E_debugMenuStatus2_a = 0;
            clearVdp2TextMemory();
        }
        else if(readKeyboardToggle(0xF6))
        {
            pSubFieldData->m37C_debugMenuStatus1[1] = 0;
            clearVdp2TextMemory();
        }
    }

    switch (pSubFieldData->m37C_debugMenuStatus1[1])
    {
    case 0:
        break;
    default:
        assert(0);
        break;
    }

    if (pSubFieldData->m37C_debugMenuStatus1[1])
    {
        fieldTaskPtr->m28_status |= 8;
    }
    else
    {
        fieldTaskPtr->m28_status &= ~8;
    }
}
void fieldDebugMenuUpdate2()
{
    s_FieldSubTaskWorkArea* r14 = getFieldTaskPtr()->m8_pSubFieldData;
    if (!r14->m36C)
    {
        if (readKeyboardToggle(KEY_CODE_F2))
        {
            assert(0);
        }
        else
        {
            if (readKeyboardToggle(0xF6))
            {
                clearVdp2TextMemory();
                r14->m369 = 0;
            }
        }
    }

    switch (r14->m369)
    {
    case 0:
        break;
    default:
        assert(0);
    }

    if (r14->m369)
    {
        assert(0);
    }
    else
    {
        getFieldTaskPtr()->m28_status &= 0xFFFFFFEF;
    }
}

bool initField(p_workArea workArea, const s_MCB_CGB* fieldFileList, u32 arg)
{
    reset3dEngine();

    loadCommonFieldResources();

    setupFileList(fieldFileList);

    if (getFieldMemoryAreaRemain() < 0)
    {
        return false;
    }

    if (getFieldCharacterAreaRemain() < 0)
    {
        return false;
    }

    loadFileFromFileList(0);
    createFieldScriptTask(workArea);
    createFieldOverlaySubTask(workArea);
    initFieldDragon(workArea, arg);
    createFieldOverlaySubTask2(workArea);
    createFieldCameraTask(workArea);
    createRandomBattleTask(workArea);
    resetProjectVector();

    getFieldTaskPtr()->m8_pSubFieldData->m334->m50D_isInitialized = 1;

    getFieldTaskPtr()->m8_pSubFieldData->pUpdateFunction2 = fieldDebugMenuUpdate2;
    getFieldTaskPtr()->m8_pSubFieldData->m374_pUpdateFunction1 = fieldDebugMenuUpdate1;

    pauseEngine[2] = 1;

    return true;
}

void overlayStart_Sub1()
{
    PDS_unimplemented("overlayStart_Sub1");
}

sSaturnPtr* ReadScripts(sSaturnPtr EA)
{
    int numScripts = 0;
    while (readSaturnEA(EA).m_offset)
    {
        numScripts++;
        EA = EA + 4;
    }

    EA = EA + (-numScripts * 4);
    sSaturnPtr* outputArray = new sSaturnPtr[numScripts];
    for (int i = 0; i < numScripts; i++)
    {
        outputArray[i] = readSaturnEA(EA);
        EA = EA + 4;
    }
    return outputArray;
}

p_workArea overlayStart_FLD_A3(p_workArea workArea, u32 arg)
{
    // Custom loader thing

    if (gFLD_A3 == NULL)
    {
        gFLD_A3 = new FLD_A3_data();
    }
    if (LCSTaskDrawSub5Sub1_Data1.empty())
    {
        LCSTaskDrawSub5Sub1_Data1 = readLCSTaskDrawSub5Sub1_Data1({ 0x06093B28, gFLD_A3 });
    }

    gFieldCameraConfigEA = { 0x6092EF0, gFLD_A3 };
    gFieldDragonAnimTableEA = { 0x06094134, gFLD_A3 };
    gFieldCameraDrawFunc = &fieldOverlaySubTaskInitSub2;

    if (!initField(workArea, fieldFileList, arg))
    {
        return NULL;
    }

    if (getFieldTaskPtr()->m2E_currentSubFieldIndex == 2)
    {
        loadSoundBanks(3, 0);
    }
    else
    {
        if (getFieldTaskPtr()->m32_previousSubField == -1)
        {
            loadSoundBanks(1, 0);
        }
        else
        {
            loadSoundBanks(2, 0);
        }
    }
    //0605404C
    loadFileFromFileList(1);


    graphicEngineStatus.m405C.m10_nearClipDistance = 0x3000;
    graphicEngineStatus.m405C.m30_oneOverNearClip = FP_Div(0x10000, graphicEngineStatus.m405C.m10_nearClipDistance);

    graphicEngineStatus.m405C.m14_farClipDistance = 0x200000;
    graphicEngineStatus.m405C.m38_oneOverFarClip = FP_Div(0x8000, graphicEngineStatus.m405C.m14_farClipDistance);

    graphicEngineStatus.m405C.m34_oneOverFarClip256 = graphicEngineStatus.m405C.m38_oneOverFarClip << 8;

    getFieldTaskPtr()->m8_pSubFieldData->m334->m50E_followModeIndex = 1;
    getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m0_pScripts = ReadScripts({ 0x60924FC, gFLD_A3 });

    switch (getFieldTaskPtr()->m2C_currentFieldIndex)
    {
    case 1:
        subfieldTable2[getFieldTaskPtr()->m2E_currentSubFieldIndex](workArea);
        break;
    case 2:
        subfieldTable1[getFieldTaskPtr()->m2E_currentSubFieldIndex](workArea);
        break;
    default:
        subfieldTable1[0](workArea);
        break;
    }

    getFieldTaskPtr()->m8_pSubFieldData->pUpdateFunction3 = overlayStart_Sub1;

    return NULL;
}

void s_LCSLaser::InitHoming(s_LCSLaser* pThis, sLaserArgs* arg)
{
    getMemoryArea(&pThis->m0, 0);
    pThis->m8 = arg->m0;
    pThis->mC = arg->m4;
    pThis->m10 = arg->m8;

    if (arg->m8 & 0x100)
    {
        pThis->m60 = *arg->m4;
    }
    else
    {
        transformAndAddVecByCurrentMatrix(arg->m4, &pThis->m60);
    }

    pThis->m14 = arg->mC;
    pThis->m18 = arg->m10;
    pThis->m1C = arg->m14;
    pThis->m20 = arg->m18;
    pThis->m27 = arg->m1F;

    pThis->m28_laserInit = &s_LCSLaser::LaserHomingInit;
    pThis->m2C_laserUpdate = &s_LCSLaser::LaserHomingUpdate;
    pThis->m30_laserDraw = &s_LCSLaser::LaserHomingDraw;

    pThis->m158 = 0x12;
    pThis->m6C[pThis->m154 & 0xF] = (*pThis->mC);
    pThis->m6C[0] = (*pThis->mC);
    pThis->m6C[1] = (*pThis->mC);
    pThis->m6C[2] = (*pThis->mC);

    pThis->m28_laserInit(pThis);

    pThis->m154++;

    particleInitSub(&pThis->m58, (pThis->m0.m4_characterArea - (0x25C00000)) >> 3, &gFLD_A3->m_0x0609518C_animatedQuad);
}

void s_LCSLaser::InitBeam(s_LCSLaser* pThis, sLaserArgs* arg)
{
    getMemoryArea(&pThis->m0, 0);
    pThis->m8 = arg->m0;
    pThis->mC = arg->m4;
    pThis->m10 = arg->m8;

    if (arg->m8 & 0x100)
    {
        pThis->m60 = *arg->m4;
    }
    else
    {
        transformAndAddVecByCurrentMatrix(arg->m4, &pThis->m60);
    }

    pThis->m14 = arg->mC;
    pThis->m18 = arg->m10;
    pThis->m1C = arg->m14;
    pThis->m20 = arg->m18;
    pThis->m27 = arg->m1F;

    pThis->m28_laserInit = &s_LCSLaser::LaserBeamInit;
    pThis->m2C_laserUpdate = &s_LCSLaser::LaserBeamUpdate;
    pThis->m30_laserDraw = &s_LCSLaser::LaserBeamDraw;

    pThis->m158 = 0x12;
    pThis->m6C[pThis->m154&0xF] = (*pThis->mC);
    pThis->m6C[0] = (*pThis->mC);
    pThis->m6C[1] = (*pThis->mC);

    pThis->m28_laserInit(pThis);

    pThis->m154++;
}

static const std::array<fixedPoint, 2> s_LCSLaser_InitTrailSub0Data0 = {
    0x2000000,
    0x6000000,
};

void s_LCSLaser::LaserTrailInit(s_LCSLaser* pThis)
{
    pThis->m34 = 0x37000;
    pThis->m38 = intDivide(pThis->m158, -0x37000);
    pThis->m3C = s_LCSLaser_InitTrailSub0Data0[(randomNumber() >> 16) & 1];
    pThis->m40 = 0;
}

// Trail particle sub-task: animates and draws a single trail sparkle
struct sTrailParticle : public s_workAreaTemplate<sTrailParticle>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { nullptr, Update, Draw, nullptr };
        return &taskDefinition;
    }

    // 0607AC16
    static void Update(sTrailParticle* pThis)
    {
        u32 result = sGunShotTask_UpdateSub4(&pThis->m8_animQuad);
        if (result & 2)
        {
            pThis->getTask()->markFinished();
        }
    }

    // 0607AC3C
    static void Draw(sTrailParticle* pThis)
    {
        drawProjectedParticleWithGouraud(&pThis->m8_animQuad, &pThis->m10_position);
    }

    s_memoryAreaOutput m0;
    sAnimatedQuad m8_animQuad;
    sVec3_FP m10_position;
    sSaturnPtr m1C_gouraudData;
    // size 0x20
};

// 0607ac58 — spawn a trail particle sub-task at the given position
static void spawnTrailParticle(const sVec3_FP* position, sSaturnPtr gouraudData)
{
    s_LCSTask* pLCSTask = getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS;
    sTrailParticle* pNewTask = createSubTask<sTrailParticle>(pLCSTask);
    if (pNewTask)
    {
        getMemoryArea(&pNewTask->m0, 0);
        pNewTask->m10_position = *position;
        pNewTask->m1C_gouraudData = gouraudData;

        u16 cmdsrca = (u16)((s32)(pNewTask->m0.m4_characterArea + 0xDA400000) >> 3);
        static std::vector<sVdp1Quad> trailQuadData; // TODO: init from DAT_060954d4
        if (trailQuadData.empty())
        {
            trailQuadData = initVdp1Quad(gFLD_A3->getSaturnPtr(0x060954d4));
        }
        particleInitSub(&pNewTask->m8_animQuad, cmdsrca, &trailQuadData);
    }
}

// Gouraud data table per laser color (indexed by m27)
static sSaturnPtr getTrailGouraudData(s8 colorIndex)
{
    // PTR_DAT_06094f48 is an array of pointers indexed by colorIndex
    return sSaturnPtr::createFromRaw(
        readSaturnU32(gFLD_A3->getSaturnPtr(0x06094f48) + colorIndex * 4),
        gFLD_A3);
}

// 0607acbc — spawn a trail particle at the current trail head position
static void spawnTrailParticleFromLaser(s_LCSLaser* pThis)
{
    // Select trail ring buffer index based on build-up state
    s32 trailIndex;
    if (pThis->m154 < 0x11)
    {
        // During buildup: use sequential index
        trailIndex = pThis->m154;
    }
    else
    {
        // After buildup: use wrapped index
        trailIndex = (pThis->m154 - 1) & 0xF;
    }

    // Get position from trail ring buffer (simplified — original uses lookup tables for segment selection)
    const sVec3_FP& trailPos = pThis->m6C[trailIndex & 0xF];

    spawnTrailParticle(&trailPos, getTrailGouraudData(pThis->m27));
}

// 0607C230
void s_LCSLaser::LaserTrailUpdate(s_LCSLaser* pThis)
{
    pThis->m34 += pThis->m38;
    pThis->m3C += pThis->m40;

    sVec3_FP localPos;
    localPos[0] = MTH_Mul(pThis->m34, getCos(pThis->m3C.getInteger() & 0xFFF)) + pThis->m60[0];
    localPos[1] = MTH_Mul(pThis->m34, getSin(pThis->m3C.getInteger() & 0xFFF)) + pThis->m60[1];
    localPos[2] = pThis->m60[2];

    sVec3_FP cameraPos;
    transformAndAddVec(localPos, cameraPos, cameraProperties2.m28[0]);

    // Interpolate trail positions in ring buffer
    s32 prevIndex = (pThis->m154 - 1) & 0xF;
    sVec3_FP& prevPos = pThis->m6C[prevIndex];

    sVec3_FP newPos;
    newPos[0] = prevPos[0] + intDivide(pThis->m158 + 1, (s32)(cameraPos[0] - prevPos[0]));
    newPos[1] = prevPos[1] + intDivide(pThis->m158 + 1, (s32)(cameraPos[1] - prevPos[1]));
    newPos[2] = prevPos[2] + intDivide(pThis->m158 + 1, (s32)(cameraPos[2] - prevPos[2]));

    pThis->m6C[pThis->m154 & 0xF] = newPos;
    pThis->m154++;

    pThis->m144 = newPos - prevPos;

    sGunShotTask_UpdateSub4(&pThis->m58);

    spawnTrailParticleFromLaser(pThis);
}

// 0607AD44
void s_LCSLaser::LaserTrailDraw(s_LCSLaser* pThis)
{
    // Select trail position for main laser sprite draw
    s32 trailIndex;
    if (pThis->m154 < 0x11)
    {
        trailIndex = pThis->m154;
    }
    else
    {
        trailIndex = (pThis->m154 - 1) & 0xF;
    }

    sVec3_FP& drawPos = pThis->m6C[trailIndex & 0xF];
    drawProjectedParticleWithGouraud(&pThis->m58, &drawPos);
}

void s_LCSLaser::InitWingRay(s_LCSLaser* pThis, sLaserArgs* arg)
{
    getMemoryArea(&pThis->m0, 0);
    pThis->m8 = arg->m0;
    pThis->mC = arg->m4;
    pThis->m10 = arg->m8;

    if (arg->m8 & 0x100)
    {
        pThis->m60 = *arg->m4;
    }
    else
    {
        transformAndAddVecByCurrentMatrix(arg->m4, &pThis->m60);
    }

    pThis->m14 = arg->mC;
    pThis->m18 = arg->m10;
    pThis->m1C = arg->m14;
    pThis->m20 = arg->m18;
    pThis->m27 = arg->m1F;

    pThis->m28_laserInit = &s_LCSLaser::LaserWingRayInit;
    pThis->m2C_laserUpdate = &s_LCSLaser::LaserWingRayUpdate;
    pThis->m30_laserDraw = &s_LCSLaser::LaserWingRayDraw;

    pThis->m158 = 0x1E;

    pThis->m28_laserInit(pThis);

    pThis->m6C[0] = (*pThis->mC);
    pThis->m6C[1] = (*pThis->mC);
}

void s_LCSLaser::LaserWingRayInit(s_LCSLaser*)
{
    // nothing on purpose
}

void s_LCSLaser::LaserWingRayUpdate(s_LCSLaser* pThis)
{
    sVec3_FP var0;
    transformAndAddVec(pThis->m60, var0, cameraProperties2.m28[0]);

    switch (pThis->m15C)
    {
    case 0:
        pThis->m6C[0] = *pThis->m14;

        pThis->m6C[1][0] += intDivide(((pThis->m158 - pThis->m154) / 2) + 1, var0[0] - pThis->m6C[1][0]);
        pThis->m6C[1][1] += intDivide(((pThis->m158 - pThis->m154) / 2) + 1, var0[1] - pThis->m6C[1][1]);
        pThis->m6C[1][2] += intDivide(((pThis->m158 - pThis->m154) / 2) + 1, var0[2] - pThis->m6C[1][2]);

        if (pThis->m158 == pThis->m154)
        {
            pThis->m6C[1] = var0;
            pThis->m15C++;
        }
        break;
    case 1:
        pThis->m6C[0][0] += intDivide(pThis->m158 + 1, var0[0] - pThis->m6C[0][0]);
        pThis->m6C[0][1] += intDivide(pThis->m158 + 1, var0[1] - pThis->m6C[0][1]);
        pThis->m6C[0][2] += intDivide(pThis->m158 + 1, var0[2] - pThis->m6C[0][2]);

        pThis->m6C[1] = var0;
    default:
        break;
    }

    pThis->m154++;
}

// 0607AA3C
void s_LCSLaser::LaserWingRayDraw(s_LCSLaser* pThis)
{
    sVec3_FP viewStart, viewEnd;
    transformAndAddVecByCurrentMatrix(&pThis->m6C[0], &viewStart);
    transformAndAddVecByCurrentMatrix(&pThis->m6C[1], &viewEnd);

    if (viewEnd.m8_Z <= viewStart.m8_Z)
    {
        return;
    }

    fixedPoint ratio = FP_Div(0x4000, viewEnd.m8_Z - viewStart.m8_Z);
    fixedPoint deltaX = MTH_Mul(ratio, viewEnd.m0_X - viewStart.m0_X);
    fixedPoint deltaY = MTH_Mul(ratio, viewEnd.m4_Y - viewStart.m4_Y);
    fixedPoint deltaZ = MTH_Mul(ratio, viewEnd.m8_Z - viewStart.m8_Z);

    u16 vdp1Base = (pThis->m0.m4_characterArea - 0x25C00000) >> 3;

    sVec3_FP current = viewStart;
    sSaturnPtr gouraudTablePtr = gFLD_A3->getSaturnPtr(0x06094d60);

    for (s32 i = 0; i < 8; i++)
    {
        sVec3_FP segEnd;
        segEnd.m0_X = current.m0_X + deltaX + deltaX;
        segEnd.m4_Y = current.m4_Y + deltaY + deltaY;
        segEnd.m8_Z = current.m8_Z + deltaZ + deltaZ;

        if (viewEnd.m8_Z < segEnd.m8_Z)
        {
            break;
        }

        std::array<sVec3_FP, 2> segment = {current, segEnd};

        if (isGunShotVisible(segment, graphicEngineStatus.m405C))
        {
            // Read gouraud data (sliding window of 4 u16 from table at 06094d60)
            sSaturnPtr gouraudPtr = gouraudTablePtr + i * 2;
            quadColor gouraud;
            gouraud[0] = readSaturnU16(gouraudPtr);
            gouraud[1] = readSaturnU16(gouraudPtr + 2);
            gouraud[2] = readSaturnU16(gouraudPtr + 4);
            gouraud[3] = readSaturnU16(gouraudPtr + 6);

            displayRaySegmentFromViewSpace(segment, 0x333, vdp1Base + 0xF8, 0x210, vdp1Base + 0x173C, &gouraud, 8);
        }

        current.m0_X = segEnd.m0_X + deltaX;
        current.m4_Y = segEnd.m4_Y + deltaY;
        current.m8_Z = segEnd.m8_Z + deltaZ;
    }
}

void s_LCSLaser::InitTrail(s_LCSLaser* pThis, sLaserArgs* arg)
{
    getMemoryArea(&pThis->m0, 0);

    getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m83F_activeLaserCount++;

    pThis->m8 = arg->m0;
    if (randomNumber() & 1)
    {
        pThis->mC = &getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m118_hotSpot3;
    }
    else
    {
        pThis->mC = &getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m124_hotSpot4;
    }
    pThis->m10 = 0;
    pThis->m14 = arg->mC;
    pThis->m18 = 0;
    pThis->m1C = arg->m14;
    pThis->m20 = 0;
    pThis->m24_receivedItemId = arg->m1C_receivedItemId;
    pThis->m26_receivedItemQuantity = arg->m1E_receivedItemQuantity;
    pThis->m27 = arg->m1F;

    pThis->m28_laserInit = &s_LCSLaser::LaserTrailInit;
    pThis->m2C_laserUpdate = &s_LCSLaser::LaserTrailUpdate;
    pThis->m30_laserDraw = &s_LCSLaser::LaserTrailDraw;
    pThis->m158 = 0x1E;

    if (arg->m8 & 0x100)
    {
        transformAndAddVec(*arg->mC, pThis->m6C[0], cameraProperties2.m28[0]);
    }
    else
    {
        pThis->m6C[0] = *arg->mC;
    }

    pThis->m6C[1] = pThis->m6C[0];
    pThis->m6C[pThis->m154 & 0xF] = pThis->m6C[0];

    pThis->m28_laserInit(pThis);

    pThis->m154++;
    particleInitSub(&pThis->m58, (pThis->m0.m4_characterArea - (0x25C00000)) >> 3, &gFLD_A3->m_0x06095330_animatedQuad);
}

static const std::array<fixedPoint, 16> s_LCSLaser_InitBeamSub0Data0 = {
    0xE38E38,
    0x5555555,
    0x71C71C7,
    0x2AAAAAA,
    0x638E38E,
    0x38E38E3,
    0x1C71C71,
    0x471C71C,
};

void s_LCSLaser::LaserHomingInit(s_LCSLaser* pThis)
{
    LaserBeamInit(pThis);
}

// Static gradient data for Laser0 beam (from Saturn data at 0x06094D9A, all 0xE280)
static s_LCSTask_gradientData LaserHomingGradientData = {{
    {0xE280, 0xE280, 0xE280, 0xE280},
    {0xE280, 0xE280, 0xE280, 0xE280},
    {0xE280, 0xE280, 0xE280, 0xE280},
    {0xE280, 0xE280, 0xE280, 0xE280},
    {0xE280, 0xE280, 0xE280, 0xE280},
    {0xE280, 0xE280, 0xE280, 0xE280},
    {0xE280, 0xE280, 0xE280, 0xE280},
    {0xE280, 0xE280, 0xE280, 0xE280},
}};

static u16 LaserHomingGouraudColors[] = {0xE280, 0xE280, 0xE280, 0xE280};

// 0607a798
void s_LCSLaser::LaserHomingDraw(s_LCSLaser* pThis)
{
    s32 stack0 = 6;
    std::array<sVec3_FP, 8> r11;
    s32 r13 = 0;

    if (pThis->m154 <= 16)
    {
        auto r5 = r11.begin();
        sSaturnPtr r4 = gFLD_A3->getSaturnPtr(0x06094C38) + pThis->m154 * 8;
        sSaturnPtr r7 = r4 + 6;

        while (r4.m_offset < r7.m_offset)
        {
            s8 value = readSaturnS8(r4);
            if (value < 0)
            {
                break;
            }
            *r5 = pThis->m6C[value];
            r13++;
            r5++;
            r4++;
        }
        pThis->LaserBeamDrawSub0(r11, r13, gFLD_A3->getSaturnPtr(0x06094D40) + (stack0 - r13) * 4, &LaserHomingGradientData);
    }
    else
    {
        sSaturnPtr r5 = gFLD_A3->getSaturnPtr(0x06094CC0) + ((pThis->m154 - 1) & 0xF) * 8;
        auto r4 = r11.begin();
        for (int i = 0; i < 6; i++)
        {
            s8 value = readSaturnS8(r5);
            *r4 = pThis->m6C[value];
            r4++;
            r5++;
        }
        pThis->LaserBeamDrawSub0(r11, 6, gFLD_A3->getSaturnPtr(0x06094D40), &LaserHomingGradientData);
    }

    drawProjectedParticleWithGouraud(&pThis->m58, &r11[0], LaserHomingGouraudColors);
}

// 0607becc
void s_LCSLaser_LaserHomingUpdate(s_LCSLaser* pThis)
{
    sGunShotTask_UpdateSub4(&pThis->m58);
}

void s_LCSLaser::LaserHomingUpdate(s_LCSLaser* pThis)
{
    LaserBeamUpdate(pThis);

    s_LCSLaser_LaserHomingUpdate(pThis);
}

void s_LCSLaser::LaserBeamInit(s_LCSLaser* pThis)
{
    pThis->m34 = 0x37000;
    pThis->m38 = intDivide(pThis->m158, -0x37000);
    pThis->m3C = s_LCSLaser_InitBeamSub0Data0[(randomNumber() >> 16) & 7];
    pThis->m40 = 0;
}

void s_LCSLaser::UpdateHomingBeamWingRay(s_LCSLaser* pThis)
{
    if ((pThis->m8 == nullptr) || (pThis->m8->getTask()->isFinished()))
    {
        pThis->m20->m14 |= 2;
        return;
    }

    if (pThis->m158 < 0)
    {
        pThis->m20->m14 |= 1;
        return;
    }

    if (pThis->m10 & 0x100)
    {
        pThis->m60 = *pThis->mC;
    }
    else
    {
        transformAndAddVecByCurrentMatrix(pThis->mC, &pThis->m60);
    }

    pThis->m2C_laserUpdate(pThis);

    pThis->m158--;
}

void s_LCSLaser::UpdateTrail(s_LCSLaser* pThis)
{
    if (pThis->m158 < 0)
    {
        pThis->getTask()->markFinished();
    }
    else
    {
        transformAndAddVecByCurrentMatrix(pThis->mC, &pThis->m60);
        pThis->m2C_laserUpdate(pThis);
        pThis->m158--;
    }
}

static const std::array<s_RGB8, 7> s_LCSLaser_DeleteTrailSub0Data0 =
{
    {
        {0x17, 0x14, 6},
        {4, 0x14, 9},
        {0x19, 0xA, 0xE},
        {0xA, 0xE, 0x1A},
        {0x5, 0x19, 0x14},
        {0x1A, 0xF, 0x5},
        {0x14, 0xE, 0x19},
    }
};

void s_LCSLaser_DeleteTrailSub0(s32 r4)
{
    s_LCSLaserSparkle* r14 = createSubTask<s_LCSLaserSparkle>(getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS);

    r14->m0 = 30;
    r14->m4 = r4;

    s_dragonTaskWorkArea* pDragonTask = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    pDragonTask->m_EB_useSpecialColor = 1;

    pDragonTask->m_E8_specialColor = s_LCSLaser_DeleteTrailSub0Data0[r4];
}

void s_LCSLaser::DeleteTrail(s_LCSLaser* pThis)
{
    s_LCSLaser_DeleteTrailSub0(pThis->m27);
    playSystemSoundEffect(17);

    if (pThis->m24_receivedItemId >= 0)
    {
        s_fieldScriptWorkArea* r13 = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE;
        if (getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m40_receivedItemTask)
        {
            //0607A1A0
            assert(0);
        }
        else
        {
            //0607A1C2
            getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m83F_activeLaserCount--;
            createReceiveItemTask(r13, &r13->m40_receivedItemTask, 45, pThis->m24_receivedItemId, pThis->m26_receivedItemQuantity);
            mainGameState.addItemCount(pThis->m24_receivedItemId, pThis->m26_receivedItemQuantity);
        }
    }
    else
    {
        getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m83F_activeLaserCount--;
    }
}

void s_LCSLaser::InitBeamSub1Sub0()
{
    if (m18 == nullptr)
        return;

    s32 r6 = 0;
    s32 r7 = 0;
    s32 r5 = 0;
    s32 r14 = 0x10;

    do 
    {
        m6C[r5] += *m18;
        m6C[r5+1] += *m18;
        r6 += 2;
    } while (r6 < r14);

    if (m154 <= r14)
    {
        m6C[0] = *m14;
    }
}

void s_LCSLaser::LaserBeamUpdate(s_LCSLaser* pThis)
{
    sMatrix4x3* var0 = cameraProperties2.m28;
    pThis->m34 += pThis->m38;
    pThis->m3C += pThis->m40;

    sVec3_FP var1C;
    var1C[0] = MTH_Mul(pThis->m34, getCos(pThis->m3C.getInteger() & 0xFFF)) + pThis->m60[0];
    var1C[1] = MTH_Mul(pThis->m34, getSin(pThis->m3C.getInteger() & 0xFFF)) + pThis->m60[1];
    var1C[2] = pThis->m60[2];

    sVec3_FP var10;
    transformAndAddVec(var1C, var10, *var0);

    pThis->InitBeamSub1Sub0();

    sVec3_FP var4;
    sVec3_FP& r5 = pThis->m6C[(pThis->m154 - 1) & 0xF];
    var4[0] = r5[0] + intDivide(pThis->m158 + 1, var10[0] - r5[0]);
    var4[1] = r5[1] + intDivide(pThis->m158 + 1, var10[1] - r5[1]);
    var4[2] = r5[2] + intDivide(pThis->m158 + 1, var10[2] - r5[2]);

    pThis->m6C[pThis->m154 & 0xF] = var4;
    pThis->m154++;

    pThis->m144 = var4 - r5;
}

void LaserBeamDrawSub0Sub0(std::array<sVec3_FP, 2>&r4, s32 r5, sVec2_S16& r6, sVec2_S16& r7, fixedPoint maxDistance)
{
    const s_graphicEngineStatus_405C& r12 = graphicEngineStatus.m405C;
    auto var_28 = r5;
    auto var_24 = r6;

    sVec3_FP& var_2C = r4[0];
    sVec3_FP& var_30 = r4[1];

    fixedPoint r10 = FP_Div(maxDistance - r4[0][2], maxDistance - r4[1][2]);
    var_30[0] = var_2C[0] + MTH_Mul(r10, var_30[0] - var_2C[0]);
    var_30[1] = var_2C[1] + MTH_Mul(r10, var_30[1] - var_2C[1]);
    var_30[2] = maxDistance;

    r6[0] = setDividend(r12.m18_widthScale, r4[1][0], r4[1][2]);
    r6[1] = setDividend(r12.m1C_heightScale, r4[1][1], r4[1][2]);

    r7[0] = setDividend(r12.m18_widthScale, r5, maxDistance);
    r7[1] = setDividend(r12.m1C_heightScale, r5, maxDistance);

    if (r7[0] >= 80)
    {
        r7[0] = 80;
    }
    if (r7[1] >= 80)
    {
        r7[1] = 80;
    }
}

s32 LaserBeamDrawSub0Sub1(std::array<sVec2_S16,2>& r4)
{
    s32 r5 = 0;
    if (r4[0][0] - r4[1][0] >= 0)
    {
        r5 = 2;
    }
    if (r4[0][1] - r4[1][1] >= 0)
    {
        r5 |= 1;
    }
    return r5;
}

void LaserBeamDrawSub0Sub2(std::array<sVec2_S16, 2>& r4, std::array<sVec2_S16, 2>&r5, s32 r6, std::array<sVec3_FP, 4>& r7)
{
    switch (r6)
    {
    case 0:
        r7[0][0] = r4[0][0] + r5[0][0];
        r7[0][1] = r4[0][1] - r5[0][1];
        r7[1][0] = r4[0][0] - r5[0][0];
        r7[1][1] = r4[0][1] + r5[0][1];
        r7[2][0] = r4[1][0] - r5[1][0];
        r7[2][1] = r4[1][1] + r5[1][1];
        r7[3][0] = r4[1][0] + r5[1][0];
        r7[3][1] = r4[1][1] - r5[1][1];
        break;
    case 1:
        r7[0][0] = r4[0][0] - r5[0][0];
        r7[0][1] = r4[0][1] - r5[0][1];
        r7[1][0] = r4[0][0] + r5[0][0];
        r7[1][1] = r4[0][1] + r5[0][1];
        r7[2][0] = r4[1][0] + r5[1][0];
        r7[2][1] = r4[1][1] + r5[1][1];
        r7[3][0] = r4[1][0] - r5[1][0];
        r7[3][1] = r4[1][1] - r5[1][1];
        break;
    case 2:
        r7[0][0] = r4[0][0] + r5[0][0];
        r7[0][1] = r4[0][1] + r5[0][1];
        r7[1][0] = r4[0][0] - r5[0][0];
        r7[1][1] = r4[0][1] - r5[0][1];
        r7[2][0] = r4[1][0] - r5[1][0];
        r7[2][1] = r4[1][1] - r5[1][1];
        r7[3][0] = r4[1][0] + r5[1][0];
        r7[3][1] = r4[1][1] + r5[1][1];
        break;
    case 3:
        r7[0][0] = r4[0][0] - r5[0][0];
        r7[0][1] = r4[0][1] + r5[0][1];
        r7[1][0] = r4[0][0] + r5[0][0];
        r7[1][1] = r4[0][1] - r5[0][1];
        r7[2][0] = r4[1][0] + r5[1][0];
        r7[2][1] = r4[1][1] - r5[1][1];
        r7[3][0] = r4[1][0] - r5[1][0];
        r7[3][1] = r4[1][1] + r5[1][1];
        break;
    default:
        assert(0);
    }
}

struct sLaserBeamDrawSub4Data1
{
    s32 m0;
    s32 m4;
    s32 m8;
};

static const std::array< sLaserBeamDrawSub4Data1, 2> LaserBeamDrawSub4Data1 = {
    {
        {0x1734, 0x98, 0x610},
        {0x1730, 0x88, 0x210}
    }
};

static const sLaserBeamDrawSub4Data1 LaserBeamDrawSub3Data0 = {
        0x173C, 0xF8, 0x210
};

static const std::array<s32, 16> LaserBeamDrawSub4Data0 = {
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

void LaserBeamDrawSub4(s_LCSLaser* r4, std::array<sVec3_FP, 4>&r5, fixedPoint r6, s32 r7, s_LCSTask_gradientData* arg0)
{
    auto& r11 = graphicEngineStatus;
    auto& r13 = graphicEngineStatus.m14_vdp1Context[0];
    r13.m10 ++;
    r11.m14_vdp1Context[0].m10 ++;//?
    int gradiantIndex = r11.m14_vdp1Context[0].m10 - r11.m14_vdp1Context[0].m14->begin();
    quadColor& r12 = *(r11.m14_vdp1Context[0].m10 - 1);

    u16 CMDCOLR = ((r4->m0.m4_characterArea - (0x25C00000)) >> 3) + LaserBeamDrawSub4Data1[LaserBeamDrawSub4Data0[0]].m0;
    u16 CMDSRCA = ((r4->m0.m4_characterArea - (0x25C00000)) >> 3) + LaserBeamDrawSub4Data1[LaserBeamDrawSub4Data0[0]].m4;
    u16 CMDSIZE = LaserBeamDrawSub4Data1[0].m8;

    s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
    vdp1WriteEA.m0_CMDCTRL = 0x1002; // CMDCTRL distorted sprite
    vdp1WriteEA.m4_CMDPMOD = 0x048C; // CMDPMOD
    vdp1WriteEA.m6_CMDCOLR = CMDCOLR; // CMDCOLR
    vdp1WriteEA.m8_CMDSRCA = CMDSRCA; // CMDSRCA
    vdp1WriteEA.mA_CMDSIZE = CMDSIZE; // CMDSIZE
    vdp1WriteEA.mC_CMDXA = r5[3][0]; // CMDXA
    vdp1WriteEA.mE_CMDYA = -r5[3][1]; // CMDYA
    vdp1WriteEA.m18_CMDXD = r5[2][0]; // CMDXD
    vdp1WriteEA.m1A_CMDYD = -r5[2][1]; // CMDYD
    vdp1WriteEA.m14_CMDXC = r5[1][0]; // CMDXC
    vdp1WriteEA.m16_CMDYC = -r5[1][1]; // CMDYC
    vdp1WriteEA.m10_CMDXB = r5[0][0]; // CMDXB
    vdp1WriteEA.m12_CMDYB = -r5[0][1]; // CMDYB

    r12 = (*arg0)[r7];

    vdp1WriteEA.m1C_CMDGRA = gradiantIndex; //CMDGRDA

    r13.m20_pCurrentVdp1Packet->m4_bucketTypes = fixedPoint(r6 * graphicEngineStatus.m405C.m38_oneOverFarClip).getInteger();
    r13.m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
    r13.m20_pCurrentVdp1Packet++;

    graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
    graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
    graphicEngineStatus.m14_vdp1Context[0].mC += 1;
}

void LaserBeamDrawSub3(s_LCSLaser* r4, std::array<sVec3_FP, 4>&r5, fixedPoint r6, s32 r7, sVec2_S16& arg0, sVec2_S16& arg4, s_LCSTask_gradientData* arg8, fixedPoint argC)
{
    if (argC >= r6)
    {
        auto& r14 = graphicEngineStatus.m14_vdp1Context[0];
        graphicEngineStatus.m14_vdp1Context[0].m10++;
        int gradiantIndex = graphicEngineStatus.m14_vdp1Context[0].m10 - graphicEngineStatus.m14_vdp1Context[0].m14->begin();
        quadColor& r9 = *(graphicEngineStatus.m14_vdp1Context[0].m10 - 1);

        u16 CMDCOLR = ((r4->m0.m4_characterArea - (0x25C00000)) >> 3) + LaserBeamDrawSub3Data0.m0;
        u16 CMDSRCA = ((r4->m0.m4_characterArea - (0x25C00000)) >> 3) + LaserBeamDrawSub3Data0.m4;
        u16 CMDSIZE = LaserBeamDrawSub4Data1[0].m8;

        s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        vdp1WriteEA.m0_CMDCTRL = 0x1002; // CMDCTRL distorted sprite
        vdp1WriteEA.m4_CMDPMOD = 0x048C; // CMDPMOD
        vdp1WriteEA.m6_CMDCOLR = CMDCOLR; // CMDCOLR
        vdp1WriteEA.m8_CMDSRCA = CMDSRCA; // CMDSRCA
        vdp1WriteEA.mA_CMDSIZE = CMDSIZE; // CMDSIZE
        vdp1WriteEA.mC_CMDXA = arg0[0] - arg4[0]; // CMDXA
        vdp1WriteEA.mE_CMDYA = -arg0[1] - arg4[1]; // CMDYA
        vdp1WriteEA.m18_CMDXD = arg0[0] + arg4[0]; // CMDXD
        vdp1WriteEA.m1A_CMDYD = -arg0[1] - arg4[1]; // CMDYD
        vdp1WriteEA.m14_CMDXC = arg0[0] + arg4[0]; // CMDXC
        vdp1WriteEA.m16_CMDYC = -arg0[1] + arg4[1]; // CMDYC
        vdp1WriteEA.m10_CMDXB = arg0[0] - arg4[0]; // CMDXB
        vdp1WriteEA.m12_CMDYB = -arg0[1] + arg4[1]; // CMDYB

        r9 = (*arg8)[r7];

        vdp1WriteEA.m1C_CMDGRA = gradiantIndex; //CMDGRDA

        r14.m20_pCurrentVdp1Packet->m4_bucketTypes = fixedPoint(r6 * graphicEngineStatus.m405C.m38_oneOverFarClip).getInteger();
        r14.m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
        r14.m20_pCurrentVdp1Packet++;

        graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
        graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
        graphicEngineStatus.m14_vdp1Context[0].mC += 1;

    }

    auto& r14 = graphicEngineStatus.m14_vdp1Context[0];
    graphicEngineStatus.m14_vdp1Context[0].m10++;
    int gradiantIndex = graphicEngineStatus.m14_vdp1Context[0].m10 - graphicEngineStatus.m14_vdp1Context[0].m14->begin();
    quadColor& r9 = *(graphicEngineStatus.m14_vdp1Context[0].m10 - 1);

    u16 CMDCOLR = ((r4->m0.m4_characterArea - 0x25C00000) >> 3) + LaserBeamDrawSub4Data1[LaserBeamDrawSub4Data0[0]].m0;
    u16 CMDSRCA = ((r4->m0.m4_characterArea - 0x25C00000) >> 3) + LaserBeamDrawSub4Data1[LaserBeamDrawSub4Data0[0]].m4;
    u16 CMDSIZE = LaserBeamDrawSub4Data1[0].m8;

    s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
    vdp1WriteEA.m0_CMDCTRL = 0x1002; // CMDCTRL distorted sprite
    vdp1WriteEA.m4_CMDPMOD = 0x048C; // CMDPMOD
    vdp1WriteEA.m6_CMDCOLR = CMDCOLR; // CMDCOLR
    vdp1WriteEA.m8_CMDSRCA = CMDSRCA; // CMDSRCA
    vdp1WriteEA.mA_CMDSIZE = CMDSIZE; // CMDSIZE
    vdp1WriteEA.mC_CMDXA = r5[0][0]; // CMDXA
    vdp1WriteEA.mE_CMDYA = -r5[0][1]; // CMDYA
    vdp1WriteEA.m18_CMDXD = r5[1][0]; // CMDXD
    vdp1WriteEA.m1A_CMDYD = -r5[1][1]; // CMDYD
    vdp1WriteEA.m14_CMDXC = r5[2][0]; // CMDXC
    vdp1WriteEA.m16_CMDYC = -r5[2][1]; // CMDYC
    vdp1WriteEA.m10_CMDXB = r5[3][0]; // CMDXB
    vdp1WriteEA.m12_CMDYB = -r5[3][1]; // CMDYB

    r9 = (*arg8)[r7];

    vdp1WriteEA.m1C_CMDGRA = gradiantIndex; //CMDGRDA

    r14.m20_pCurrentVdp1Packet->m4_bucketTypes = fixedPoint(r6 * graphicEngineStatus.m405C.m38_oneOverFarClip).getInteger();
    r14.m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
    r14.m20_pCurrentVdp1Packet++;

    graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
    graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
    graphicEngineStatus.m14_vdp1Context[0].mC += 1;
}

void s_LCSLaser::LaserBeamDrawSub0(std::array<sVec3_FP, 8>& input_r5, s32 r6, sSaturnPtr r7, s_LCSTask_gradientData* arg0)
{
    if (gDirectRayRendering)
    {
        u16 vdp1Base = (this->m0.m4_characterArea - 0x25C00000) >> 3;

        for (s32 i = 0; i < r6 - 1; i++)
        {
            std::array<sVec3_FP, 2> segment;
            transformAndAddVecByCurrentMatrix(&input_r5[i], &segment[0]);
            transformAndAddVecByCurrentMatrix(&input_r5[i + 1], &segment[1]);

            s32 width = readSaturnS32(r7 + i * 4);

            u16 characterAddress;
            u16 characterColor;
            s16 characterSize;
            if (i == r6 - 2)
            {
                // Last segment uses different (larger) texture
                characterColor = vdp1Base + LaserBeamDrawSub4Data1[LaserBeamDrawSub4Data0[0]].m0;
                characterAddress = vdp1Base + LaserBeamDrawSub4Data1[LaserBeamDrawSub4Data0[0]].m4;
                characterSize = LaserBeamDrawSub4Data1[0].m8;
            }
            else
            {
                characterColor = vdp1Base + LaserBeamDrawSub3Data0.m0;
                characterAddress = vdp1Base + LaserBeamDrawSub3Data0.m4;
                characterSize = LaserBeamDrawSub4Data1[0].m8;
            }

            displayRaySegmentFromViewSpace(segment, width, characterAddress, characterSize, characterColor, &(*arg0)[i], 8);
        }
        return;
    }

#if 0
    input_r5[0][0] = 0x00353A19;
    input_r5[0][1] = 0x00016C99;
    input_r5[0][2] = 0xFFAA50AD;
    input_r5[1][0] = 0x00352902;

    input_r5[1][1] = 0x000138A0;
    input_r5[1][2] = 0xFFAAA3A8;
    input_r5[2][0] = 0x003515B8;
    input_r5[2][1] = 0x000102CE;

    input_r5[2][2] = 0xFFAAF6B8;
    input_r5[3][0] = 0x00350057;
    input_r5[3][1] = 0x0000C936;
    input_r5[3][2] = 0xFFAB4944;

    r6 = 4;
    r7.m_offset = 0x6094D50;
    
    pCurrentMatrix->m[0][0] = 0x0FFFC;
    pCurrentMatrix->m[0][1] = 0x0;
    pCurrentMatrix->m[0][2] = 0xFFFFFD41;
    pCurrentMatrix->m[0][3] = 0xFFCA17CD;
    pCurrentMatrix->m[1][0] = 0xFFFFFFEA;
    pCurrentMatrix->m[1][1] = 0x0000FFE1;
    pCurrentMatrix->m[1][2] = 0xFFFFF826;
    pCurrentMatrix->m[1][3] = 0xFFFCA50E;
    pCurrentMatrix->m[2][0] = 0xFFFFFD41;
    pCurrentMatrix->m[2][1] = 0xFFFFF826;
    pCurrentMatrix->m[2][2] = 0xFFFF0022;
    pCurrentMatrix->m[2][3] = 0xFFACD3CB;
    
#endif

    const s_graphicEngineStatus_405C& r14 = graphicEngineStatus.m405C;

    auto stack0 = r6;
    std::array<s32, 8>::iterator stack4;
    sVec2_S16 stack8;
    sVec2_S16 stackC;
    auto stack10 = r7;
    auto stack14 = this;
    auto stack1C = input_r5;
    auto stack20 = stack0 - 2;
    auto stack24 = stack0 - 1;
    std::array<sVec3_FP, 4> stack28; // some unknown size;
    sVec2_S16 stack58;
    std::array<sVec2_S16, 2> stack5C;
    sVec2_S16 stack64;
    std::array<sVec2_S16, 2> stack68;
    std::array<sVec3_FP, 2> stack70;
    std::array<s32, 8> stack88; // some unknown size

    transformAndAddVecByCurrentMatrix(&input_r5[0], &stack70[0]);

    stack5C[0][0] = setDividend(r14.m18_widthScale, stack70[0][0], stack70[0][2]);
    stack5C[0][1] = setDividend(r14.m1C_heightScale, stack70[0][1], stack70[0][2]);
    stack68[0][0] = setDividend(r14.m18_widthScale, readSaturnS32(r7), stack70[0][2]);
    stack68[0][1] = setDividend(r14.m1C_heightScale, readSaturnS32(r7), stack70[0][2]);

    auto r4 = stack88.begin();
    if (stack70[0][2] < 0x3000)
    {
        *r4 = 1;
    }
    else if(stack70[0][2] < graphicEngineStatus.m405C.m14_farClipDistance)
    {
        *r4 = 0;
    }
    else
    {
        *r4 = 2;
    }
    //0607B424
    sVec2_S16& r9 = stack5C[1];
    sVec2_S16& r10 = stack68[1];
    s32 r11 = 0;
    auto r8 = r4+1;
    auto r5 = r4;
    stack4 = r5;

    while (r11 < stack24)
    {
        transformAndAddVecByCurrentMatrix(&stack1C[r11 + 1], &stack70[1]);
        stack58[0] = r9[0] = setDividend(r14.m18_widthScale, stack70[1][0], stack70[1][2]);
        stack58[1] = r9[1] = setDividend(r14.m1C_heightScale, stack70[1][1], stack70[1][2]);

        auto stack18 = stack10 + r11 * 4;
        auto stack0 = stack18 + 4;
        stack64[0] = stack68[1][0] = setDividend(r14.m18_widthScale, readSaturnS32(stack0), stack70[1][2]);
        stack64[1] = stack68[1][1] = setDividend(r14.m1C_heightScale, readSaturnS32(stack0), stack70[1][2]);

        if (stack70[1][2] < 0x3000)
        {
            *r8 = 1;
        }
        else if (stack70[1][2] < r14.m14_farClipDistance)
        {
            *r8 = 0;
        }
        else
        {
            *r8 = 2;
        }

        if ((stack4[0] != *r8) ||(stack4[0] == 0))
        {
            //607B512
            if (stack4[0] == 1)
            {
                //0607B51A
                LaserBeamDrawSub0Sub0(stack70, readSaturnS32(stack18), stackC, stack8, 0x3000);
            }
            else if (stack4[0] == 2)
            {
                LaserBeamDrawSub0Sub0(stack70, readSaturnS32(stack18), stackC, stack8, r14.m14_farClipDistance);
            }

            //0607B54C
            if (*r8 == 1)
            {
                LaserBeamDrawSub0Sub0(stack70, readSaturnS32(stack0), r9, r10, 0x3000);
            }
            else if (*r8 == 2)
            {
                LaserBeamDrawSub0Sub0(stack70, readSaturnS32(stack0), r9, r10, r14.m14_farClipDistance);
            }

            //607B586
            LaserBeamDrawSub0Sub2(stack5C, stack68, LaserBeamDrawSub0Sub1(stack5C), stack28);

            //0607B5A2
            {
                s32 r4 = 0;
                s32 r7 = 0;
                s32 r5 = 0;
                s32 r6 = 0;

                do 
                {
                    // this was unrolled 4 times in the original code
                    //607B5AA
                    if ((r14.mC > stack28[r4][0]) && (r14.m4 > stack28[r4][0]))
                    {
                        stack28[r4][0] = r14.m4;
                    }
                    //607B5F4
                    if ((r14.mE < stack28[r4][0]) && (r14.m6 < stack28[r4][0]))
                    {
                        stack28[r4][0] = r14.m6;
                    }
                    //607B63E
                    if ((r14.mA > stack28[r4][1]) && (r14.m2 > stack28[r4][1]))
                    {
                        stack28[r4][1] = r14.m2;
                    }
                    //607B688
                    if ((r14.m8 < stack28[r4][1]) && (r14.m0 < stack28[r4][1]))
                    {
                        stack28[r4][1] = r14.m0;
                    }
                    r4++;
                } while (r4 < 4);

                //0607BA00
                if (stack20 != r11)
                {
                    LaserBeamDrawSub3(stack14, stack28, stack70[1][2], r11, r9, r10, arg0, stack70[0][2]);
                }
                else
                {
                    //607BA32
                    LaserBeamDrawSub4(stack14, stack28, stack70[1][2], r11, arg0);
                }
            }
        }

        //607BA48
        stack70[0] = stack70[1];
        stackC = stack58;
        stack8 = stack64;
        stack4 = r8;
        r11++;
    } 
}

void s_LCSLaser::LaserBeamDraw(s_LCSLaser* pThis)
{
    s_LCSTask_gradientData* r6 = getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m9C0;
    s32 stack0 = 8;
    std::array<sVec3_FP,8> r11;
    std::array<sVec3_FP, 8>::iterator r4 = r11.begin();
    s32 r13 = 0;
    if (pThis->m154 <= 16)
    {
        //0607A8C6
        auto r5 = r4;
        sSaturnPtr r4 = gFLD_A3->getSaturnPtr(0x06094C38) + pThis->m154 * 8;
        sSaturnPtr r7 = r4 + 8;

        while (r4.m_offset < r7.m_offset)
        {
            s8 value = readSaturnS8(r4);
            if (value < 0)
            {
                break;
            }
            *r5 = pThis->m6C[value];
            r13++;
            r5++;
            r4++;
        }
        return pThis->LaserBeamDrawSub0(r11, r13, gFLD_A3->getSaturnPtr(0x06094D40) + (stack0 - r13) * 4, r6);
    }
    else
    {
        //0607A91C
        sSaturnPtr r5 = gFLD_A3->getSaturnPtr(0x06094CC0) + ((pThis->m154 - 1) & 0xF) * 8;
        while (r4 < r11.end())
        {
            s8 value = readSaturnS8(r5);
            *r4 = pThis->m6C[value];
            r4++;
            r5++;
        }
        return pThis->LaserBeamDrawSub0(r11, 8, gFLD_A3->getSaturnPtr(0x06094D40), r6);
    }
}

void fieldScriptTaskUpdateSub2Sub1Sub1Sub1Sub2(s_LCSLaser* r4)
{
    if (r4)
    {
        r4->m_UpdateMethod = &s_LCSLaser::fieldScriptTaskUpdateSub2Sub1Sub1Sub1Sub2Sub;
        r4->m_DrawMethod = nullptr;
        r4->m15C = 0;
    }
}

s_LCSLaser* LCSTaskDrawSub1Sub2Sub0Sub2Sub0(s_LCSTask* r4, sLaserArgs* r5, s8 r6)
{
    return createSiblingTaskWithArg<s_LCSLaser>(r4, r5, &s_LCSLaser::constructionTable[r6]);
}

void dragonFieldTaskInitSub4Sub3(u8 r4)
{
    setCameraFollowFunctions(0, activateCameraFollowModeTable1[r4], activateCameraFollowModeTable2[r4]);

    getFieldCameraStatus()->m8D_followState = 0;
}

void FLD_A3_data::init()
{
    loadScriptData1(m6083244, getSaturnPtr(0x6083244));
}

