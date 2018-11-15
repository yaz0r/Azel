#include "PDS.h"

void updateDragonDefault(s_dragonTaskWorkArea*);
void updateCutscene(s_dragonTaskWorkArea* r14);
sMatrix4x3* fieldCameraTask1DrawSub1();
fixedPoint interpolateRotation(fixedPoint r10_currentValue, fixedPoint r12_targetValue, fixedPoint stack0, fixedPoint r11, s32 r13);
fixedPoint interpolateDistance(fixedPoint r11, fixedPoint r12, fixedPoint stack0, fixedPoint r10, s32 r14);
void updateCameraScriptSub1(u32 r4);
void fieldOverlaySubTaskInitSub5(u32 r4);
s32 checkPositionVisibility(sVec3_FP* r4, s32 r5);

//namespace FLD_A3_OVERLAY {

    sSaturnMemoryFile* gFLD_A3 = NULL;

    const s_MCB_CGB fieldFileList[] =
    {
    { "FLDCMN.MCB", "FLDCMN.CGB" },
    { "FLD_A3.MCB", "FLD_A3.CGB" },
    { "FLD_A3_0.MCB", "FLD_A3_0.CGB" },
    { "FLD_A3_1.MCB", "FLD_A3_1.CGB" },
    { "FLD_A3_2.MCB", "FLD_A3_2.CGB" },
    { "FLD_A3_3.MCB", "FLD_A3_3.CGB" },
    { NULL, NULL }
    };

    const char EV03_1_PCM[] = "EV03_1.PCM";

    const char FLD_A3_Script_0_String0[] = "(Imperial piece of crap.)";
    const char FLD_A3_Script_1_String0[] = "Damn!";
    const char FLD_A3_Script_1_String1[] = "A wind net to keep monsters away.";

    void dragonLeaveArea(s_dragonTaskWorkArea* r14);
    s8 updateCameraFromDragonSub1(s32 index);
    sFieldCameraStatus* getFieldCameraStatus();

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

    p_workArea create_fieldA3_0_task0(p_workArea workArea)
    {
        s_fieldTaskWorkArea_C* newWorkArea = static_cast<s_fieldTaskWorkArea_C*>(createSubTaskFromFunction(workArea, NULL, new s_fieldTaskWorkArea_C, "fieldTaskWorkArea_C"));
        getFieldTaskPtr()->mC = newWorkArea;
        return newWorkArea;
    }

    p_workArea create_fieldA3_0_task1(p_workArea workArea, s32 r5, s32 r6)
    {
        PDS_unimplemented("create_fieldA3_0_task1");
        return workArea;
    }

    void fieldA3_0_tutorialTask_update(p_workArea workArea)
    {
        if (startFieldScript(21, -1))
        {
            workArea->getTask()->markFinished();
        }
    }

    void create_fieldA3_0_tutorialTask(p_workArea workArea)
    {
        if ((getFieldTaskPtr()->m2C_currentFieldIndex != 2) || mainGameState.getBit(0xA2, 2))
        {
            createSubTaskFromFunction(workArea, fieldA3_0_tutorialTask_update, new s_dummyWorkArea, "fieldA3_0_tutorialTask_update");
        }
    }

    void fieldA3_0_startTasks(p_workArea workArea)
    {
        create_fieldA3_0_task0(workArea);

        getFieldTaskPtr()->mC->m168 = create_fieldA3_0_task1(workArea, 2, 0x20);

        PDS_unimplemented("fieldA3_0_startTasks");

        create_fieldA3_0_tutorialTask(workArea);
    }

    struct sFieldA3_1_fieldIntroTask : public s_workArea
    {
        static void StaticUpdate(p_workArea pWorkArea)
        {
            ConvertType(pWorkArea)->Update();
        }
        static sFieldA3_1_fieldIntroTask* ConvertType(p_workArea pWorkArea)
        {
            return static_cast<sFieldA3_1_fieldIntroTask*>(pWorkArea);
        }
        void Update() override
        {
            if (startFieldScript(14, 1453))
            {
                getTask()->markFinished();
            }
        }
    };

    void create_fieldA3_1_fieldIntroTask(p_workArea workArea)
    {
        if (!mainGameState.getBit(0xB5, 5))
        {
            createSubTaskFromFunction(workArea, sFieldA3_1_fieldIntroTask::StaticUpdate, new sFieldA3_1_fieldIntroTask, "fieldA3_1_fieldIntroTask_Update");
        }
    }

    void fieldA3_1_startTasks_sub0()
    {
        getFieldTaskPtr()->mC->m0 = fixedPoint(0x10000);
    }

    void fieldA3_1_startTasks(p_workArea workArea)
    {
        create_fieldA3_0_task0(workArea);

        PDS_unimplemented("fieldA3_1_startTasks");

        create_fieldA3_1_fieldIntroTask(workArea);

        fieldA3_1_startTasks_sub0();
    }

    void fieldGridTask_Update(p_workArea)
    {

    }

    void setupGridCell(s_visibilityGridWorkArea* r4, s_visdibilityCellTask* r5, int index)
    {
        getMemoryArea(&r5->m0_memoryLayout, r4->m30->mC);
        r5->index = index;
        if (r4->m30->m0_environmentGrid)
        {
            r5->m8_pEnvironmentCell = r4->m30->m0_environmentGrid[index];
        }
        if (r4->m30->m4)
        {
            r5->mC_pCell2_billboards = r4->m30->m4[index];
        }
        if (r4->m30->m8)
        {
            r5->pCell3 = r4->m30->m8[index];
        }
    }


    void gridCellDraw_untextured(p_workArea)
    {
        assert(0);
    }

    void gridCellDraw_collision(p_workArea)
    {
        assert(0);
    }

    s_taskDefinition fieldGridTaskDefinition = { NULL, fieldGridTask_Update, gridCellDraw_untextured, NULL, "fieldGridTask" };

    u32 gridCellDraw_GetDepthRange(fixedPoint r4)
    {
        s_visibilityGridWorkArea* r5 = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;
        u32 rangeIndex = 0;

        while (r4 > r5->m2C_depthRangeTable[rangeIndex])
        {
            rangeIndex++;
        }

        return rangeIndex;
    }

    void allocateLCSEntry(s_visibilityGridWorkArea* r4, u8* r5, u32 r6)
    {
        r4->m12E4++;

        if (r4->m12E4 >= 24)
        {
            return;
        }

        r4->m44[0].m0 = r5;
        r4->m44[1].m34 = r6;
        r4->m44++;
    }

    u8 gridCellDraw_normalSub0(u8* r4, sVec3_FP& r5)
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
                dist[2] = DragonPos[1] - r5[2];
            }
        }

        s32 distanceThreshold = READ_BE_U32(r4) + +0x8000;
        if (dist[0] > distanceThreshold)
            return 0;
        if (dist[1] > distanceThreshold)
            return 0;
        if (dist[2] > distanceThreshold)
            return 0;

        allocateLCSEntry(var_1C, r4, 0x10000);

        return 1;
    }

    void gridCellDraw_normal(p_workArea workArea)
    {
        s_visdibilityCellTask* pTypedWorkAread = static_cast<s_visdibilityCellTask*>(workArea);

        s_visibilityGridWorkArea* r13 = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;

        s32 r15 = graphicEngineStatus.m4070_farClipDistance;

        if (pTypedWorkAread->m8_pEnvironmentCell)
        {
            s_grid1* r14 = pTypedWorkAread->m8_pEnvironmentCell;
            while (r14->m0.m_offset)
            {
                r13->m12E0++;

                s16 r2 = readSaturnS16(r14->m0);
                u32 r1 = READ_BE_U32(pTypedWorkAread->m0_memoryLayout.m0_mainMemory + r2);
                r13->m12F0 += READ_BE_U32(pTypedWorkAread->m0_memoryLayout.m0_mainMemory + r1 + 4);

                if (!r13->m12FC_isObjectClipped(&r14->m4, r15))
                {
                    u32 var_54 = 0;
                    r13->m12E2++;

                    if (readSaturnS16(r14->m0 + 8))
                    {
                        u32 offset = READ_BE_U32(pTypedWorkAread->m0_memoryLayout.m0_mainMemory + readSaturnS16(r14->m0 + 8));
                        var_54 = gridCellDraw_normalSub0(pTypedWorkAread->m0_memoryLayout.m0_mainMemory + offset, r14->m4);
                    }

                    pushCurrentMatrix();
                    translateCurrentMatrix(&r14->m4);
                    rotateCurrentMatrixZ(r14->m10[2]);
                    rotateCurrentMatrixY(r14->m10[1]);
                    rotateCurrentMatrixX(r14->m10[0]);

                    u32 depthRangeIndex = gridCellDraw_GetDepthRange(pCurrentMatrix->matrix[11]);

                    if (readSaturnS16(r14->m0 + depthRangeIndex * 2))
                    {
                        u32 offset = READ_BE_U32(pTypedWorkAread->m0_memoryLayout.m0_mainMemory + readSaturnS16(r14->m0 + depthRangeIndex * 2));
                        addObjectToDrawList(pTypedWorkAread->m0_memoryLayout.m0_mainMemory, offset);
                    }

                    if (var_54)
                    {
                        sMatrix4x3 var_C;
                        initMatrixToIdentity(&var_C);
                        translateMatrix(&r14->m4, &var_C);
                        rotateMatrixZYX_s16(r14->m10, &var_C);
                        copyMatrix(&var_C, &r13->m44[-1].m4); // Gross. This was incremented in gridCellDraw_normalSub0
                    }

                    popMatrix();
                }

                r14++;
            }
        }

        if (pTypedWorkAread->mC_pCell2_billboards)
        {
            s_grid2* r14 = pTypedWorkAread->mC_pCell2_billboards;
            while (r14->m0.m_offset)
            {
                r13->m12E0++;

                s16 r2 = readSaturnS16(r14->m0);
                u32 r1 = READ_BE_U32(pTypedWorkAread->m0_memoryLayout.m0_mainMemory + r2);
                r13->m12F0 += READ_BE_U32(pTypedWorkAread->m0_memoryLayout.m0_mainMemory + r1 + 4);

                if (!r13->m12FC_isObjectClipped(&r14->m4, r15))
                {
                    r13->m12E2++;

                    pushCurrentMatrix();
                    translateCurrentMatrix(&r14->m4);

                    u32 depthRangeIndex = gridCellDraw_GetDepthRange(pCurrentMatrix->matrix[11]);
                    if (depthRangeIndex <= r13->m1300)
                    {
                        u32 offset = READ_BE_U32(pTypedWorkAread->m0_memoryLayout.m0_mainMemory + readSaturnS16(r14->m0));
                        addBillBoardtToDrawList(pTypedWorkAread->m0_memoryLayout.m0_mainMemory, offset);

                        if (readSaturnS16(r14->m0 + 2))
                        {
                            PDS_unimplemented("Billboard with LCS");
                        }
                    }
                    popMatrix();
                }

                r14++;
            }
        }

        if (pTypedWorkAread->pCell3)
        {
            assert(0);
        }
    }


    void(*gridCellDrawFunctions[3])(p_workArea) =
    {
        gridCellDraw_untextured, // untextured
        gridCellDraw_collision, // textures collision geo
        gridCellDraw_normal, // normal textured
    };

    s_visdibilityCellTask* createGridCellTask(s_visibilityGridWorkArea* r4, p_workArea r5, int cellIndex)
    {
        s_visdibilityCellTask* pNewTask = (s_visdibilityCellTask*)createSubTask(r5, &fieldGridTaskDefinition, new s_visdibilityCellTask);

        if (pNewTask)
        {
            setupGridCell(r4, pNewTask, cellIndex);

            pNewTask->getTask()->mC_pDraw = gridCellDrawFunctions[r4->m12F2_renderMode];
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
        static bool bEnableAllGrid = false;
        if (ImGui::Begin("Grid"))
        {
            if (ImGui::Checkbox("Enable all grid", &bEnableAllGrid))
            {
                pFieldCameraTask1->updateVisibleCells = true;
            }
        }
        ImGui::End();

        if (bEnableAllGrid)
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

    void updateCellGridFromCameraPosition(p_workArea workArea)
    {
        s_visibilityGridWorkArea* pFieldCameraTask1 = static_cast<s_visibilityGridWorkArea*>(workArea);

        pFieldCameraTask1->m0_position[0] = cameraProperties2.m0_position[0];
        pFieldCameraTask1->m0_position[1] = cameraProperties2.m0_position[1];
        pFieldCameraTask1->m0_position[2] = cameraProperties2.m0_position[2];

        pFieldCameraTask1->updateVisibleCells = pFieldCameraTask1->m12F8_convertCameraPositionToGrid(pFieldCameraTask1);

        updateCellGridIfDirty(pFieldCameraTask1);
    }


    void setupField2(s_DataTable3* r4, void(*r5)(p_workArea workArea))
    {
        s_visibilityGridWorkArea* pFieldCameraTask1 = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;
        if (pFieldCameraTask1->m38)
        {
            assert(0);
        }

        pFieldCameraTask1->m38 = createSubTaskFromFunction(pFieldCameraTask1, NULL, new s_dummyWorkArea, "BackgroundTasks");

        if (r4)
        {
            pFieldCameraTask1->m30 = r4;
            pFieldCameraTask1->m20_cellDimensions[0] = r4->m18_cellDimensions[0] * 2;
            pFieldCameraTask1->m20_cellDimensions[1] = r4->m18_cellDimensions[1] * 2;
            pFieldCameraTask1->m28 = r4->m20;
            loadFileFromFileList(r4->mC);

            if (pFieldCameraTask1->m28 & 1)
            {
                //06070FE0
                assert(0);
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
            pFieldCameraTask1->getTask()->m8_pUpdate = updateCellGridFromCameraPosition;
        }
        else
        {
            pFieldCameraTask1->m20_cellDimensions[0] = graphicEngineStatus.m4070_farClipDistance;
            pFieldCameraTask1->m20_cellDimensions[1] = graphicEngineStatus.m4070_farClipDistance;
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
        s_A3_Obj2_60 m60;
    };

    void A3_Obj2_UpdateSub0(s_A3_Obj2_60* r14)
    {
        sVec3_FP var0;
        sVec3_FP varC;

        s_LCSTask* r13 = getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS;

        r14->m19 &= ~0x30;
        r14->m1C = 0;
        r14->m18 &= ~4;
        if (r14->m18 & 1)
        {
            return;
        }
        if (r14->mC)
        {
            //606CF1C
            if (r14->m10 & 0x100)
            {
                //0606CF24
                varC = *r14->m8;
            }
            else
            {
                //606CF46
                transformAndAddVecByCurrentMatrix(r14->m8, &varC);
            }

            //606CF50
            if (r14->m10 & 0x200)
            {
                var0 = *r14->mC;
            }
            else
            {
                //606CF74
                transformVecByCurrentMatrix(*r14->mC, var0);
            }

            //606CF7C
            if (dot3_FP(&varC, &var0) < 0)
            {
                r14->m18 |= 4;
            }
        }
        //606CF96
        assert(r13);

        r13->m10++;

        if (r13->mC < 0x100)
        {
            r13->m14[r13->mC].m0 = r14;
            r13->m14[r13->mC].m4 = 0;
            r13->mC++;
        }
    }

    void getDragonDeltaMovement(sVec3_FP* r4)
    {
        *r4 = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m160_deltaTranslation;
    }

    void A3_Obj2_Update(p_workArea pWorkArea)
    {
        s_A3_Obj2* r14 = static_cast<s_A3_Obj2*>(pWorkArea);

        s32 r10 = r14->m30;
        std::vector<fixedPoint>::iterator var8 = r14->m5C_perNodeRotation.begin();
        sSaturnPtr var4 = sSaturnPtr({ 0x6092964, gFLD_A3 });

        if (getFieldTaskPtr()->mC->m8)
        {
            r14->m60.m18 = 0;
        }
        else
        {
            r14->m60.m18 |= 1;
        }

        A3_Obj2_UpdateSub0(&r14->m60);

        r14->m10_position[1] = *r14->mC_verticalOffset + r14->m8->m4_position[1];

        fixedPoint r11 = MTH_Mul(r14->m38, getFieldTaskPtr()->mC->m0);
        if ( r11 == r14->m40)
        {
            // if dragon is moving fast enough and close enough to the flag
            if ((getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m235_dragonSpeedIndex >= 3) && (vecDistance(r14->m10_position, getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m8_pos) < fixedPoint(0xC8000)))
            {
                sVec3_FP dragonDeltaMovement;
                getDragonDeltaMovement(&dragonDeltaMovement);

                // is the dragon over or under the flag?
                if (getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m8_pos[1] < r14->m10_position[1])
                {
                    r14->m40 = MTH_Mul(0x38E38E3, getFieldTaskPtr()->mC->m0);
                }
                else
                {
                    r14->m40 = MTH_Mul(0xE38E38, getFieldTaskPtr()->mC->m0);
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
        else if(r14->m3C < r14->m40)
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

        fixedPoint var0 = MTH_Mul(r14->m50, getSin((s16(r14->m48 >> 16)) & 0xFFF)) + r14->m3C + fixedPoint(0x8000000);

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

    void A3_Obj2_Draw(p_workArea pWorkArea)
    {
        s_A3_Obj2* r14 = static_cast<s_A3_Obj2*>(pWorkArea);

        std::vector<fixedPoint>::iterator r12 = r14->m5C_perNodeRotation.begin();
        sSaturnPtr r10 = r14->m58;

        sMatrix4x3 var0_baseMatrix;
        sMatrix4x3 var30_nodeMatrix;
        sVec3_FP var60_currentNodePosition = r14->m10_position;
        initMatrixToIdentity(&var0_baseMatrix);
        s_DataTable2Sub0* r13 = r14->m8;
        rotateMatrixZ(r13->m12_rotation[1], &var0_baseMatrix);
        rotateMatrixY(r13->m12_rotation[0], &var0_baseMatrix);

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
            rotateCurrentMatrixZ(r13->m12_rotation[1]);
            rotateCurrentMatrixY(r13->m12_rotation[0]);
            rotateCurrentMatrixShiftedX(*r12);

            u32 offset = READ_BE_U32(r14->m0.m0_mainMemory + readSaturnS16(r10));
            addObjectToDrawList(r14->m0.m0_mainMemory, offset);

            popMatrix();

            r12++;
            r10 += 2;
        } while (--r11);
    }

    void create_A3_Obj2_Sub0(s_A3_Obj2_60* r4, s_A3_Obj2* r5, void* r6, sVec3_FP* r7, sVec3_FP* arg0, s16 arg6, s16 argA, s16 argE, s32 arg10, s32 arg14)
    {
        r4->m0 = r5;
        r4->m4 = r6;
        r4->m8 = r7;
        r4->mC = arg0;
        r4->m12 = argA;
        r4->m14 = argE;
        if (r4->m14 >= 0)
        {
            r4->m16 = arg10;
        }
        else
        {
            r4->m16 = 0;
        }
        r4->m10 = arg6;
        r4->m17 = arg14;
        r4->m18 = 0;
        r4->m19 = 0;
        r4->m1A = 0;
        r4->m1B = 0;
        r4->m1C = 0;
        r4->m20 = 0;
    }

    void create_A3_Obj2_Sub1()
    {
        assert(0);
    }

    void create_A3_Obj2(s_visdibilityCellTask* r4, s_DataTable2Sub0& r5, s32 r6, s32 r7)
    {
        s_A3_Obj2* pNewObj = new s_A3_Obj2;
        createSubTaskFromFunction(r4, NULL, pNewObj, "A3_Obj2");

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
            pNewObj->mC_verticalOffset = &getFieldTaskPtr()->mC->m50[readSaturnS8(sSaturnPtr({ 0x609290E, gFLD_A3 }) + r5.m18)];
            pNewObj->m28_numNodes = 8;
            pNewObj->m1C_nodeLength[1] = 0x6000;
            pNewObj->m58 = sSaturnPtr({ 0x609293A, gFLD_A3 });
            break;
        case 1:
            r11 = sSaturnPtr({ 0x6092A48, gFLD_A3 }) + 0x1C * r5.m18;
            pNewObj->mC_verticalOffset = &getFieldTaskPtr()->mC->m50[readSaturnS8(sSaturnPtr({ 0x6092915, gFLD_A3 }) + r5.m18)];
            pNewObj->m28_numNodes = 4;
            pNewObj->m1C_nodeLength[1] = 0x6000;
            pNewObj->m58 = sSaturnPtr({ 0x609294A, gFLD_A3 });
            break;
        case 2:
            r11 = sSaturnPtr({ 0x6092B28, gFLD_A3 }) + 0x1C * r5.m18;
            pNewObj->mC_verticalOffset = &getFieldTaskPtr()->mC->m50[readSaturnS8(sSaturnPtr({ 0x609291D, gFLD_A3 }) + r5.m18)];
            pNewObj->m28_numNodes = 4;
            pNewObj->m1C_nodeLength[1] = 0x3000;
            pNewObj->m58 = sSaturnPtr({ 0x6092952, gFLD_A3 });
            break;
        case 3:
            r11 = sSaturnPtr({ 0x6092BEC, gFLD_A3 }) + 0x1C * r5.m18;
            pNewObj->mC_verticalOffset = &getFieldTaskPtr()->mC->m50[readSaturnS8(sSaturnPtr({ 0x6092924, gFLD_A3 }) + r5.m18)];
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
        pNewObj->m40 = pNewObj->m3C = pNewObj->m38 = MTH_Mul(readSaturnS32(r11 + 8), getFieldTaskPtr()->mC->m0);
        pNewObj->m44 = readSaturnS32(r11 + 0xC);
        pNewObj->m4C = readSaturnS32(r11 + 0x10);

        if (getFieldTaskPtr()->mC->m0 >= 0)
        {
            pNewObj->m50 = MTH_Mul(readSaturnS32(r11 + 0x14), getFieldTaskPtr()->mC->m0);
        }
        else
        {
            pNewObj->m50 = MTH_Mul(readSaturnS32(r11 + 0x14), -getFieldTaskPtr()->mC->m0);
        }

        if (getFieldTaskPtr()->mC->m0 >= 0)
        {
            pNewObj->m54 = MTH_Mul(readSaturnS32(r11 + 0x18), getFieldTaskPtr()->mC->m0);
        }
        else
        {
            pNewObj->m54 = MTH_Mul(readSaturnS32(r11 + 0x18), -getFieldTaskPtr()->mC->m0);
        }

        pNewObj->m30 = randomNumber();
        pNewObj->m48 = randomNumber();

        pNewObj->m5C_perNodeRotation.resize(pNewObj->m28_numNodes);

        pNewObj->getTask()->m8_pUpdate = A3_Obj2_Update;
        pNewObj->getTask()->mC_pDraw = A3_Obj2_Draw;

        create_A3_Obj2_Sub0(&pNewObj->m60, pNewObj, create_A3_Obj2_Sub1, &pNewObj->m10_position, 0, 0, 0, -1, 0, 0);

        getFieldTaskPtr()->mC->m8 = 0;
    }


    struct s_A3_Obj0 : public s_workAreaTemplate<s_A3_Obj0>
    {
        static s_taskDefinition* getTaskDefinition()
        {
            static s_taskDefinition taskDefinition = { NULL, NULL, s_A3_Obj0::StaticDraw, NULL, "s_A3_Obj0" };
            return &taskDefinition;
        }
        void Draw() override
        {
            pushCurrentMatrix();
            translateCurrentMatrix(&m14_position);
            rotateCurrentMatrixY(m3A_rotation);

            addObjectToDrawList(m0.m0_mainMemory, READ_BE_U32(m0.m0_mainMemory + m38_modelOffset));

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
            pNewTask->mC = &getFieldTaskPtr()->mC->m50[r5.m18 + 0];
            pNewTask->m38_modelOffset = 0x22C;
            break;
        case 1:
            pNewTask->m24 = 0x6C16C1;
            pNewTask->m2C = 0xF5;
            pNewTask->mC = &getFieldTaskPtr()->mC->m50[r5.m18 + 5];
            pNewTask->m38_modelOffset = 0x228;
            break;
        case 2:
            pNewTask->m24 = 0x666666;
            pNewTask->m2C = 0xA3;
            pNewTask->mC = &getFieldTaskPtr()->mC->m50[r5.m18 + 11];
            pNewTask->m38_modelOffset = 0x224;
            break;
        default:
            assert(0);
            break;
        }

        pNewTask->m14_position = r5.m4_position;
        *pNewTask->mC = 0;
        pNewTask->m28.zero();
        if (r5.m10)
        {
            pNewTask->m3A_rotation = 0x800 - r5.m12_rotation[0];
        }
        else
        {
            pNewTask->m3A_rotation = r5.m12_rotation[0];
        }

        return pNewTask;
    }

    s32 checkPositionVisibilityAgainstFarPlane(sVec3_FP* r4)
    {
        return checkPositionVisibility(r4, graphicEngineStatus.m4070_farClipDistance);
    }

    struct s_A3_Obj4 : public s_workAreaTemplate<s_A3_Obj0>
    {
        static s_taskDefinition* getTaskDefinition()
        {
            static s_taskDefinition taskDefinition = { NULL, s_A3_Obj4::StaticUpdate, s_A3_Obj4::StaticDraw, NULL, "s_A3_Obj4" };
            return &taskDefinition;
        }

        void Update() override
        {
            mC += 0xB60B6;
            fixedPoint var1C = getFieldTaskPtr()->mC->mA4[m8->m18] = MTH_Mul(fixedPoint(0x71C71C), getSin((mC >> 16) & 0xFFF)) - fixedPoint(0x71C71C);

            sVec3_FP& r12 = getFieldTaskPtr()->mC->mC0[m8->m18];
            r12[0] = m8->m4_position[0] - MTH_Mul(fixedPoint(0xE333), getSin((var1C >> 16) & 0xFFF));
            r12[1] = m8->m4_position[1] + MTH_Mul(fixedPoint(0xE333), getSin((var1C >> 16) & 0xFFF));

            if (checkPositionVisibilityAgainstFarPlane(&r12))
            {
                m10 = 1;
            }
        }

        void Draw() override
        {
            pushCurrentMatrix();
            translateCurrentMatrix(&m8->m4_position);
            rotateCurrentMatrixShiftedZ(getFieldTaskPtr()->mC->mA4[m8->m18]);

            addObjectToDrawList(m0.m0_mainMemory, READ_BE_U32(m0.m0_mainMemory + 0x29C));

            popMatrix();
        }

        s_memoryAreaOutput m0;
        s_DataTable2Sub0* m8;
        u32 mC;
        s32 m10;
    };

    void create_A3_Obj4(s_visdibilityCellTask* r4, s_DataTable2Sub0& r5, s32 r6)
    {
        s_A3_Obj4* pNewTask = createSubTask<s_A3_Obj4>(r4);
        getMemoryArea(&pNewTask->m0, r6);
        pNewTask->m8 = &r5;
        getFieldTaskPtr()->mC->mC0[r5.m18] = r5.m4_position;
        pNewTask->mC = randomNumber();
    }

    void dispatchFunction(s_visdibilityCellTask* r4, s_DataTable2Sub0& r5, s32 r6)
    {
        switch (r5.m0_function.m_offset)
        {
        // ropes
        case 0x6060194:
            getFieldTaskPtr()->mC->mC[r5.m18] = create_A3_Obj0(r4, r5, r6, 0);
            break;
        case 0x60601C8:
            getFieldTaskPtr()->mC->mC[r5.m18 + 5] = create_A3_Obj0(r4, r5, r6, 1);
            break;
        case 0x6060228:
            getFieldTaskPtr()->mC->mC[r5.m18 + 11] = create_A3_Obj0(r4, r5, r6, 2);
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

        // flag pole
        case 0x605A94C:
            create_A3_Obj4(r4, r5, r6);
            break;

        default:
            PDS_unimplemented("dispatchFunction");
            break;
            //assert(0);
        }
    }

    void setupFieldSub1Sub0(s_visdibilityCellTask* r4, std::vector<s_DataTable2Sub0>& r5, s32 r6)
    {
        for(int i=0; i<r5.size(); i++)
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

    void subfieldA3_0Sub0(s_dragonTaskWorkArea* r4)
    {
        PDS_unimplemented("subfieldA3_0Sub0");
    }

    void subfieldA3_1Sub0Sub0()
    {
        s_dragonTaskWorkArea* r14 = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
        r14->m154_dragonSpeed = 0;
        initDragonSpeed(0);

        if (r14->m1D0_cameraScript)
        {
            r14->mF0 = dragonFieldTaskInitSub4;
        }
        else if (r14->m1D4_cutsceneData)
        {
            r14->mF0 = updateCutscene;
        }
        else
        {
            r14->mF0 = updateDragonDefault;
        }

        r14->m104_dragonScriptStatus = 0;

        getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->mF8_Flags |= 0x10000;
    }

    void subfieldA3_1Sub0Sub1()
    {
        PDS_unimplemented("subfieldA3_1Sub0Sub1");
    }

    void subfieldA3_1Sub0Sub2(s32 r4, s32 r5)
    {
        PDS_unimplemented("subfieldA3_1Sub0Sub2");
    }

    void subfieldA3_1Sub0(s_dragonTaskWorkArea* r4)
    {
        switch (r4->m108)
        {
        case 0:
            if (getFieldTaskPtr()->mC->m130 != 1)
                return;
            subfieldA3_1Sub0Sub0();
            r4->m108++;
        case 1:
            subfieldA3_1Sub0Sub1();
            if (getFieldTaskPtr()->mC->m130 != 2)
                return;
            subfieldA3_1Sub0Sub2(4, 0x8000);
            r4->m108++;
        case 2:
            return;
        default:
            assert(0);
            break;
        }
    }

    void setupDragonPositionSub0(sVec3_FP* r4, sVec3_FP* r5)
    {
        if (r4)
        {
            assert(0);
        }
    }

    void setupDragonPosition(sVec3_FP* r4, sVec3_FP* r5)
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

    s32 queueNewFieldScript(sSaturnPtr r4, s32 r5)
    {
        s_fieldScriptWorkArea* r13 = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE;
        if (r13->m4_currentScript.m_offset)
            return 0;

        if (r5 >=0)
        {
            if (r5 > 1000)
            {
                r5 -= 566;
            }

            if (mainGameState.getBit(r5))
                return 0;
        }

        r13->m60_canSkipScript = 0;
        r13->m4_currentScript = r4;
        r13->m2C_bitToSet = r5;
        r13->m58 = 0;
        r13->m50_scriptDelay = 0;

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

    s_cameraScript cameraScript0 =
    {
        { 0x2F7000, 0x34000, -0x258000 }, //m0_position
    { 0, 0, 0 }, // mC_rotation
    0x34000, // m18
    0xED0, // m1C
    0x36, // m20
    { 0x2F9000, 0x39000, -0x280000 }, // m24_pos2;
    0xC8000, // m30
    };

    s_grid1* readEnvironmentGridCell(sSaturnPtr gridCellEA)
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

        s_grid1* pCellArray = new s_grid1[numEntries + 1];
        s_grid1* pCell = pCellArray;
        for (int i = 0; i < numEntries; i++)
        {
            pCell->m0 = readSaturnEA(gridCellEA); gridCellEA = gridCellEA + 4;
            pCell->m4 = readSaturnVec3(gridCellEA); gridCellEA = gridCellEA + 4 * 3;
            pCell->m10[0] = readSaturnS16(gridCellEA); gridCellEA = gridCellEA + 2;
            pCell->m10[1] = readSaturnS16(gridCellEA); gridCellEA = gridCellEA + 2;
            pCell->m10[2] = readSaturnS16(gridCellEA); gridCellEA = gridCellEA + 2;
            pCell->m16 = readSaturnS32(gridCellEA); gridCellEA = gridCellEA + 2;
            pCell++;
        }
        memset(pCell, 0, sizeof(s_grid1));

        return pCellArray;
    }

    s_grid1** readEnvironmentGrid(sSaturnPtr gridEA, u32 gridWidth, u32 gridHeight)
    {
        if (gridEA.m_offset == 0)
            return NULL;

        s_grid1** pGrid = new s_grid1*[gridWidth*gridHeight];
        memset(pGrid, 0, sizeof(s_grid1*) * gridWidth*gridHeight);

        for (int i = 0; i < gridWidth * gridHeight; i++)
        {
            pGrid[i] = NULL;

            sSaturnPtr cellEA = readSaturnEA(gridEA); gridEA = gridEA + 4;
            if (cellEA.m_offset)
            {
                pGrid[i] = readEnvironmentGridCell(cellEA);
            }
        }

        return pGrid;
    }

    s_grid2* readGrid2Cell(sSaturnPtr gridCellEA)
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

        s_grid2* pCellArray = new s_grid2[numEntries + 1];
        s_grid2* pCell = pCellArray;
        for (int i = 0; i < numEntries; i++)
        {
            pCell->m0 = readSaturnEA(gridCellEA); gridCellEA = gridCellEA + 4;
            pCell->m4 = readSaturnVec3(gridCellEA); gridCellEA = gridCellEA + 4 * 3;
            pCell++;
        }
        memset(pCell, 0, sizeof(s_grid1));

        return pCellArray;
    }

    s_grid2** readGrid2(sSaturnPtr gridEA, u32 gridWidth, u32 gridHeight)
    {
        if (gridEA.m_offset == 0)
            return NULL;

        s_grid2** pGrid = new s_grid2*[gridWidth*gridHeight];
        memset(pGrid, 0, sizeof(s_grid2*) * gridWidth*gridHeight);

        for (int i = 0; i < gridWidth * gridHeight; i++)
        {
            pGrid[i] = NULL;

            sSaturnPtr cellEA = readSaturnEA(gridEA); gridEA = gridEA + 4;
            if (cellEA.m_offset)
            {
                pGrid[i] = readGrid2Cell(cellEA);
            }
        }

        return pGrid;
    }

    s_grid3** readGrid3(sSaturnPtr gridEA, u32 gridWidth, u32 gridHeight)
    {
        assert(gridEA.m_offset == 0);
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
                    newEntry.m10 = readSaturnS16(cellEA + 0x10);
                    newEntry.m12_rotation[0] = readSaturnS16(cellEA + 0x12);
                    newEntry.m12_rotation[1] = readSaturnS16(cellEA + 0x14);
                    newEntry.m18 = readSaturnS32(cellEA + 0x18);

                    pNewData2->m0[i].push_back(newEntry);

                    cellEA += 0x20;
                }
            }
        }

        return pNewData2;
    }

    void fieldPaletteTaskInit(p_workArea pWorkArea)
    {
        s_fieldPaletteTaskWorkArea* r14 = static_cast<s_fieldPaletteTaskWorkArea*>(pWorkArea);

        getFieldTaskPtr()->m8_pSubFieldData->m350_fieldPaletteTask = r14;

        reinitVdp2();

        PDS_unimplemented("call in fieldPaletteTaskInit");
        
        r14->m78 = (s_fieldPaletteTaskWorkSub*)allocateHeapForTask(r14, sizeof(s_fieldPaletteTaskWorkSub));
        r14->m78->m0 = 1;
        r14->m78->m4 = 0;
        r14->m78->m8 = -0xF78000;
        r14->m78->mC= -0x1194000;
        r14->m78->m10 = -0xEC4000;
        r14->m78->m14 = -0x1054000;

        asyncDmaCopy({ 0x060900A4, gFLD_A3 }, getVdp2Cram(0), 0x80, 0);
        asyncDmaCopy({ 0x06090124, gFLD_A3 }, getVdp2Cram(0x80), 0x80, 0);
        asyncDmaCopy({ 0x0608FC84, gFLD_A3 }, getVdp2Cram(0xE0), 0x20, 0);
        asyncDmaCopy({ 0x06090184, gFLD_A3 }, getVdp2Cram(0x600), 0x20, 0);
        asyncDmaCopy({ 0x060901A4, gFLD_A3 }, getVdp2Cram(0x620), 0x20, 0);
        asyncDmaCopy({ 0x0608FA84, gFLD_A3 }, getVdp2Cram(0x400), 0x200, 0);
        asyncDmaCopy({ 0x060901C4, gFLD_A3 }, getVdp2Cram(0x800), 0x200, 0);
        asyncDmaCopy({ 0x0608FCA4, gFLD_A3 }, getVdp2Cram(0xA00), 0x200, 0);
        asyncDmaCopy({ 0x0608FEA4, gFLD_A3 }, getVdp2Cram(0xC00), 0x200, 0);
    }

    void fieldPaletteTaskDraw(p_workArea pWorkArea)
    {
        PDS_unimplemented("fieldPaletteTaskDraw");
    }

    s_taskDefinition fieldPaletteTaskDefinition = { fieldPaletteTaskInit, NULL, fieldPaletteTaskDraw, NULL, "fieldPaletteTask" };

    void createFieldPaletteTask(p_workArea parent)
    {
        createSubTask(parent, &fieldPaletteTaskDefinition, new s_fieldPaletteTaskWorkArea);
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

    void updateCutscene(s_dragonTaskWorkArea* r14)
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
            r14->m160_deltaTranslation[0] = performDivision(r13->m0_duration, r13->m4_pos[0] - r14->m8_pos[0]);
            r14->m160_deltaTranslation[1] = performDivision(r13->m0_duration, r13->m4_pos[1] - r14->m8_pos[1]);
            r14->m160_deltaTranslation[2] = performDivision(r13->m0_duration, r13->m4_pos[2] - r14->m8_pos[2]);

            if (r13->m10_rotationDuration)
            {
                r14->m16C_deltaRotation[0] = performDivision(r13->m10_rotationDuration, fixedPoint(r13->m14_rot[0] - r14->m20_angle[0]).normalized());
                r14->m16C_deltaRotation[1] = performDivision(r13->m10_rotationDuration, fixedPoint(r13->m14_rot[1] - r14->m20_angle[1]).normalized());
                r14->m16C_deltaRotation[2] = performDivision(r13->m10_rotationDuration, fixedPoint(r13->m14_rot[2] - r14->m20_angle[2]).normalized());
            }
            else
            {
                r14->m16C_deltaRotation[0] = 0;
                r14->m16C_deltaRotation[1] = 0;
                r14->m16C_deltaRotation[2] = 0;
            }

            dragonFieldTaskInitSub4Sub6(r14);
            if (r14->mF8_Flags & 0x40000)
            {
                updateCameraScriptSub0Sub2(r14);
                updateCameraScriptSub0(r14->mB8);
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

        dragonFieldTaskInitSub4Sub5(&r14->m48, &r14->m20_angle);
        copyMatrix(&r14->m48.m0_matrix, &r14->m88_matrix);

        if (--r14->m1EE < 0)
        {
            r14->m1EE = 0;
        }
        dragonFieldTaskInitSub4Sub6(r14);
    }

    void updateDragonDefault(s_dragonTaskWorkArea*)
    {
        PDS_unimplemented("updateDragonDefault");
    }

    void cutsceneTaskInitSub1(s_scriptData3* r15)
    {
        s_dragonTaskWorkArea* r4 = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
        r4->m1E4_cutsceneKeyFrame = r15;
        r4->m104_dragonScriptStatus = 0;

        r4->mF0 = updateCutscene;
        r4->mF8_Flags &= ~0x400;
    }

    void dragonFieldTaskUpdateSub5Sub2(s_fieldOverlaySubTaskWorkArea* r4, sFieldCameraStatus* r5)
    {
        r4->m2E4[4].m14++;
        r5->m5C[0] = interpolateRotation(r5->m5C[0], 0, 0x2000, 0x444444, 0);
        r5->m5C[1] = interpolateRotation(r5->m5C[1], 0, 0x2000, 0x444444, 0);
        r5->m5C[2] = interpolateRotation(r5->m5C[2], 0, 0x2000, 0x444444, 0);

        r5->m5C += r5->m68;
        r5->m68.zero();

        r5->m44[0] = interpolateDistance(r5->m44[0], 0, 0x2000, 0xAAA, 0);
        r5->m44[1] = interpolateDistance(r5->m44[1], 0, 0x2000, 0xAAA, 0);
        r5->m44[2] = interpolateDistance(r5->m44[2], 0, 0x2000, 0xAAA, 0);

        r5->m44 += r5->m50;
        r5->m50.zero();

        r5->m0_position = r5->m44 + *r4->m2E4[4].mC_pPosition;

        sVec3_FP var18;
        var18 = *r4->m2E4[4].m10_pPosition2 - *r4->m2E4[4].mC_pPosition;

        fixedPoint dummy[2];
        generateCameraMatrixSub1(var18, dummy);

        r5->mC_rotation[0] = r5->m5C[0] + dummy[0];
        r5->mC_rotation[1] = r5->m5C[1] + dummy[1];
        r5->mC_rotation[2] += r5->m5C[2];

        r5->m24_distanceToDestination = vecDistance(*r4->m2E4[4].mC_pPosition, *r4->m2E4[4].m10_pPosition2);
    }

    void cutsceneTaskInitSub2Sub0(sVec3_FP* r4, sVec3_FP* r5)
    {
        s_fieldOverlaySubTaskWorkArea* r14 = getFieldTaskPtr()->m8_pSubFieldData->m334;
        r14->m2E4[4].mC_pPosition = r4;
        r14->m2E4[4].m10_pPosition2 = r5;
        r14->m2E4[4].m14 = 0;
        r14->m2E4[4].m18_maxDistanceSquare = 1;

        subfieldA3_1Sub0Sub0();

        fieldOverlaySubTaskInitSub1(1, NULL, NULL);
        fieldOverlaySubTaskInitSub3(1);

        dragonFieldTaskUpdateSub5Sub2(r14, getFieldCameraStatus());
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

    void s_cutsceneTask2::Init(void* argument)
    {
        getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m48_cutsceneTask = this;
        m4 = (std::vector<s_scriptData1>*)argument;
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

        m44[0] = performDivision(m40, m44[0]);
        m44[1] = performDivision(m40, m44[1]);
        m44[2] = performDivision(m40, m44[2]);

        m50[0] = performDivision(m40, m50[0]);
        m50[1] = performDivision(m40, m50[1]);
        m50[2] = performDivision(m40, m50[2]);

        m5C = performDivision(m40, fixedPoint(m38 - m34).normalized());

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

    void UpdateSub1Sub0()
    {
        s_fieldOverlaySubTaskWorkArea* r13 = getFieldTaskPtr()->m8_pSubFieldData->m334;
        r13->m2E4[4].m14 = 0;
        r13->m2E4[4].m18_maxDistanceSquare = 0;
        dragonFieldTaskInitSub4Sub4();
        fieldOverlaySubTaskInitSub5(1);

        r13->m3E4[0].m0_position = r13->m3E4[1].m0_position;
        r13->m3E4[0].mC_rotation = r13->m3E4[1].mC_rotation;
        r13->m3E4[0].m18 = r13->m3E4[1].m18;
        r13->m3E4[0].m1C = r13->m3E4[1].m1C;
        r13->m3E4[0].m20 = r13->m3E4[1].m20;
        r13->m3E4[0].m24_distanceToDestination = r13->m3E4[1].m24_distanceToDestination;

        fieldOverlaySubTaskInitSub3(0);

        updateCameraScriptSub1(getFieldTaskPtr()->m8_pSubFieldData->m334->m50E);
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
                UpdateSub1Sub0();
            }
            else
            {
                assert(0);
            }
        }
    }

    void s_cutsceneTask2::Update()
    {
        switch (m34)
        {
        case 0:
            m34 = UpdateSub0();
            cutsceneTaskInitSub2Sub0(&m8, &m14);
            getFieldCameraStatus()->mC_rotation[2] = m20;
            return;
        case 1:
            m8 += m44;
            m28 += m50;
            m20 += m5C;

            getFieldCameraStatus()->mC_rotation[1] = m20;
            m60++;

            if (m24)
            {
                //0606A12C
                assert(0);
            }
            else
            {
                m14 = m28;
            }

            if (--m40)
            {
                return;
            }

            if (++m38 < 0x10)
            {
                m34 = UpdateSub0();
            }
            else
            {
                m34 = -1;
            }
            break;
        default:
            if (getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m5C)
                return;
            UpdateSub1();
            break;
        }
    }

    void s_cutsceneTask::cutsceneTaskInitSub2(std::vector<s_scriptData1>& r11, s32 r6, sVec3_FP* r7, u32 arg0)
    {
        s_fieldScriptWorkArea* r14 = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE;

        r14->m80 = this;
        r14->m7C = r6;
        r14->m84 = r7;

        if (r14->m78)
        {
            cutsceneTaskInitSub2Sub0(&r14->m78->m3C, &r14->m78->m48);
            r14->m48_cutsceneTask = this;
            return;
        }

        if (r11.size() == 0)
            return;

        cutsceneTaskInitSub2Sub1(r11, *getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m88);

        if (r14->m48_cutsceneTask)
        {
            cutsceneTaskInitSub2Sub2(r14->m48_cutsceneTask);
        }

        s_cutsceneTask2* pNewTask = createSiblingTaskWithArg<s_cutsceneTask2>(this, &r11);
        assert(pNewTask);

        pNewTask->m0 = arg0;
        if (arg0 & 1)
        {
            pNewTask->m24 = r7;
        }

        pNewTask->Update();
    }

    void s_cutsceneTask::Init(void* argument)
    {
        s_cutsceneData* pCutsceneData = static_cast<s_cutsceneData*>(argument);
        s_dragonTaskWorkArea* r14 = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
        u32 r11 = (pCutsceneData->m8 & ~3) / 2;

        if (r14->m1D8_cutscene)
        {
            r14->m1D8_cutscene->getTask()->markFinished();
        }

        r14->mF8_Flags &= ~0x400;
        r14->mF8_Flags |= 0x40000;

        r14->m1D8_cutscene = this;

        r14->m1D4_cutsceneData = pCutsceneData;

        r14->m1EC = cutsceneTaskInitSub0(pCutsceneData->m0, *getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m90);

        r14->m1EC += 2;

        cutsceneTaskInitSub1(&pCutsceneData->m0[m0]);

        r14->mF0(r14);

        switch (pCutsceneData->m8 & 3)
        {
        case 1:
            cutsceneTaskInitSub2(pCutsceneData->m4, 0, &r14->m8_pos, r11);
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

    void s_cutsceneTask::Update()
    {
        s_dragonTaskWorkArea* r13 = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
        if (r13->m1D4_cutsceneData == NULL)
        {
            cutsceneTaskUpdateSub0();
            dragonFieldTaskInitSub4Sub4();
            getTask()->markFinished();
            r13->m1D8_cutscene = 0;
            return;
        }
        if (r13->m1E4_cutsceneKeyFrame == NULL)
        {
            s_scriptData3* pData = &r13->m1D4_cutsceneData->m0[++m0];
            if (pData->m0_duration)
            {
                cutsceneTaskInitSub1(pData);
            }
            else
            {
                r13->m1D4_cutsceneData = NULL;
            }
        }

        m18_frameCount++;

        if (m4)
        {
            assert(0);
        }
    }
    
    void startCutscene(s_cutsceneData* r4)
    {
        createSubTaskWithArg(getFieldTaskPtr()->m8_pSubFieldData, s_cutsceneTask::getTaskDefinition(), new s_cutsceneTask, r4);
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
                pData->m0[i].m4_pos = readSaturnVec3(table0); table0 = table0 + 4*3;
                pData->m0[i].m10_rotationDuration = readSaturnS32(table0); table0 = table0 + 4;
                pData->m0[i].m14_rot = readSaturnVec3(table0); table0 = table0 + 4 * 3;
            }
        }

        // read table 1
        {
            int numEntries = 0;
            while (readSaturnU32(table1))
            {
                numEntries++;
                table1 = table1 + 0x3C;
            }
            table1 -= 0x3C * numEntries;
            numEntries++;

            pData->m4.resize(numEntries);
            for (int i = 0; i < numEntries; i++)
            {
                pData->m4[i].m0 = readSaturnS32(table0); table0 = table0 + 4;
                pData->m4[i].m4 = readSaturnVec3(table0); table0 = table0 + 4*3;
                pData->m4[i].m10 = readSaturnVec3(table0); table0 = table0 + 4 * 3;
                pData->m4[i].m1C = readSaturnVec3(table0); table0 = table0 + 4 * 3;
                pData->m4[i].m28 = readSaturnVec3(table0); table0 = table0 + 4 * 3;
                pData->m4[i].m34 = readSaturnS32(table0); table0 = table0 + 4;
                pData->m4[i].m38 = readSaturnS32(table0); table0 = table0 + 4;
            }
        }

        return pData;
    }

    void subfieldA3_0(p_workArea workArea)
    {
        s16 r13 = getFieldTaskPtr()->m30;

        playPCM(workArea, 100);
        playPCM(workArea, 101);
        
        s_DataTable3* pDataTable3 = readDataTable3({ 0x6085AA4, gFLD_A3 });
        std::vector<std::vector<sCameraVisibility>>* pVisibility = readCameraVisbility({ 0x608F410, gFLD_A3 }, pDataTable3);
        s_DataTable2* pDataTable2 = readDataTable2({ 0x6083D3C, gFLD_A3 });
        setupField(pDataTable3, pDataTable2, fieldA3_0_startTasks, pVisibility);

        getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->mF4 = subfieldA3_0Sub0;

        {
            sVec3_FP position = { 0x28A000, 0x32000, 0x1294000 };
            sVec3_FP rotation = { 0x0, 0x4000000, 0x0 };
            setupDragonPosition(&position, &rotation);
        }

        if (r13 != -1)
        {
            if (getFieldTaskPtr()->m2C_currentFieldIndex == 21)
            {
                {
                    sVec3_FP position = { 0x2EF000, 0x3C000, -0x2D1000 };
                    sVec3_FP rotation = { 0x0, 0x16C16C, 0x0 };
                    setupDragonPosition(&position, &rotation);
                }

                graphicEngineStatus.m4070_farClipDistance = 0x2AE000;
                graphicEngineStatus.m4094 = FP_Div(0x8000, 0x2AE000);

                graphicEngineStatus.m4090 = graphicEngineStatus.m4094 << 8;
            }
            else
            {
                //060542E4
                switch (getFieldTaskPtr()->m32)
                {
                case 4:
                case 5:
                case 6:
                case 8:
                case 10:
                    assert(0);
                default:
                    getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = &cameraScript0;
                    startFieldScript(17, -1);
                    break;
                }
            }
        }
        //060543E0
        PDS_unimplemented("subfieldA3_0");

        createFieldPaletteTask(workArea);

        //TODO: more stuff here

        adjustVerticalLimits(-0x54000, 0x76000);
    }

    void setupFieldCameraConfig_A3_1()
    {
        setupFieldCameraConfigs(readCameraConfig({ 0x6081F9C, gFLD_A3 }), 1);
    }

    void subfieldA3_1(p_workArea workArea)
    {
        playPCM(workArea, 100);
        playPCM(workArea, 101);
        
        s_DataTable3* pDataTable3 = readDataTable3({ 0x608838C, gFLD_A3 });
        std::vector<std::vector<sCameraVisibility>>* pVisibility = readCameraVisbility({ 0x608F628, gFLD_A3 }, pDataTable3);
        s_DataTable2* pDataTable2 = readDataTable2({ 0x60866D0, gFLD_A3 });
        setupField(pDataTable3, pDataTable2, fieldA3_1_startTasks, pVisibility);

        getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->mF4 = subfieldA3_1Sub0;

        {
            sVec3_FP position = { 0x0, 0x0, 0x0 };
            sVec3_FP rotation = { 0x154000, 0x0, -0x18E000 };
            setupDragonPosition(&position, &rotation);
        }

        if (getFieldTaskPtr()->m30 != -1)
        {
            //6054472
            switch (getFieldTaskPtr()->m32)
            {
            case 8:
                assert(0);
            case 9:
                assert(0);
            default:
                if (mainGameState.getBit(0xA, 6))
                {
                    startCutscene(loadCutsceneData({ 0x6091688, gFLD_A3 }));
                }
                else
                {
                    startCutscene(loadCutsceneData({ 0x60915A4, gFLD_A3 }));
                }
                break;
            }
        }
        //6054526
        PDS_unimplemented("subfieldA3_1");

        createFieldPaletteTask(workArea);

        setupFieldCameraConfig_A3_1();

        adjustVerticalLimits(-0x58000, 0x76000);

        //TODO: more stuff here

        startFieldScript(18, -1);

        //TODO: more stuff here
    }
    void subfieldA3_2(p_workArea workArea) { PDS_unimplemented("subfieldA3_2"); assert(false); }
    void subfieldA3_3(p_workArea workArea) { PDS_unimplemented("subfieldA3_3"); assert(false); }
    void subfieldA3_4(p_workArea workArea) { PDS_unimplemented("subfieldA3_4"); assert(false); }
    void subfieldA3_5(p_workArea workArea) { PDS_unimplemented("subfieldA3_5"); assert(false); }
    void subfieldA3_6(p_workArea workArea) { PDS_unimplemented("subfieldA3_6"); assert(false); }
    void subfieldA3_7(p_workArea workArea) { PDS_unimplemented("subfieldA3_7"); assert(false); }
    void subfieldA3_8(p_workArea workArea) { PDS_unimplemented("subfieldA3_8"); assert(false); }
    void subfieldA3_9(p_workArea workArea) { PDS_unimplemented("subfieldA3_9"); assert(false); }
    void subfieldA3_A(p_workArea workArea) { PDS_unimplemented("subfieldA3_A"); assert(false); }
    void subfieldA3_B(p_workArea workArea) { PDS_unimplemented("subfieldA3_B"); assert(false); }
    void subfieldA3_C(p_workArea workArea) { PDS_unimplemented("subfieldA3_C"); assert(false); }

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
            pData1[i].m4.zero();
            pData1[i].m10.zero();
            pData1[i].m1C.zero();
            pData1[i].m28.zero();
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

    void fieldScriptTaskInit(s_workArea* pWorkArea)
    {
        s_fieldScriptWorkArea* pFieldScriptWorkArea = static_cast<s_fieldScriptWorkArea*>(pWorkArea);

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
        if (pFieldScript->m4_currentScript.m_offset && pFieldScript->m30_cinematicBarTask && pFieldScript->m34 && pFieldScript->m38_dialogStringTask && pFieldScript->m3C_multichoiceTask && pFieldScript->m40)
            return 0;

        return 1;

    }

    s32 fieldScriptTaskUpdateSub5()
    {
        return getFieldTaskPtr()->m8_pSubFieldData->m334->m2E4[4].m18_maxDistanceSquare == 0;
    }

    void fieldScriptTaskUpdateSub6()
    {
        getFieldTaskPtr()->m8_pSubFieldData->m33C_pPaletteTask->m5A = 1;
    }

    void fieldScriptTaskUpdateSub7()
    {
        getFieldTaskPtr()->m8_pSubFieldData->m33C_pPaletteTask->m5A = 0;
    }

    void s_fieldScriptWorkArea::fieldScriptTaskUpdateSub3()
    {
        getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m8 &= ~0x40;
        getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m8 |= 0x20;

        if (m2C_bitToSet >= 0)
        {
            if (m2C_bitToSet < 1000)
            {
                mainGameState.setBit(m2C_bitToSet - 566);
            }
            else
            {
                mainGameState.setBit(m2C_bitToSet);
            }

            m2C_bitToSet = -1;
        }

        m58 = 0;
        m50_scriptDelay = 0;
    }

    void fieldScriptTaskUpdateSub2Sub1Sub1(s_LCSTask* r4)
    {
        s_LCSTask_828* r5 = r4->m828;
        while (r5)
        {
            s_LCSTask_828* next = r5->m4_next;
            assert(0);
            //fieldScriptTaskUpdateSub2Sub1Sub1Sub1(r5);
            r5 = next;
        }
    }

    void fieldScriptTaskUpdateSub2Sub1()
    {
        getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m8 |= 0x40;

        fieldScriptTaskUpdateSub2Sub1Sub1(getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS);
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

        if (readKeyboardToggle(0x87))
        {
            assert(0);
        }

        if (graphicEngineStatus.m4514.m0->m0_current.m8_newButtonDown & 6)
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

    s32 dispatchTutorialMultiChoice()
    {
        s32 result = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m44_multiChoiceData->m4_currentChoice;
        if (result == 8) // quit tutorial
        {
            if (getFieldTaskPtr()->m2C_currentFieldIndex == 2)
            {
                mainGameState.setBit(0xA2, 2); // tutorial bit
                dispatchTutorialMultiChoiceSub1();
            }
            else
            {
                stopAllSounds();
                dispatchTutorialMultiChoiceSub2();
            }

            return result;
        }

        assert(0);
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

    void s_riderAnimTask::Delete()
    {
        getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1DC_ridersAnimation[m4_riderIndex] = NULL;
    }

    void s_riderAnimTask::Update()
    {
        switch (m0_status)
        {
        case 0:
            switch (m10_animSequence->m0)
            {
            case -1:
                getTask()->markFinished();
                return;
            case 0:
                m8_delay = m10_animSequence->m2;
                playAnimationGeneric(&m14_riderState->m18_3dModel, m18->m0_riderModel + READ_BE_U32(m18->m0_riderModel + m1C[m10_animSequence->m1]), m10_animSequence->m2);
                break;
            case 1:
                m8_delay = m10_animSequence->m2;
                playAnimationGeneric(&m14_riderState->m18_3dModel, m18->m0_riderModel + READ_BE_U32(m18->m0_riderModel + m1C[m10_animSequence->m1+0x10]), 15);
                break;
            default:
                assert(0);
                break;
            }
            updateAndInterpolateAnimation(&m14_riderState->m18_3dModel);
            m0_status++;
        case 1:
            switch (m10_animSequence->m0)
            {
            case 0:
                if ((--m8_delay) > 0)
                    return;
                m0_status = 0;
                m10_animSequence++;
                return;
            case 1:
                if (m14_riderState->m18_3dModel.m16 < mC)
                {
                    if ((--m8_delay) <= 0)
                    {
                        m10_animSequence++;
                    }
                }
                mC = m14_riderState->m18_3dModel.m16;
                return;
            }
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
        default:
            assert(0);
            break;
        }
        return 0;
    }

    u32 interpolateCinematicBarData[512];

    void s_cinematicBarTask::interpolateCinematicBarSub1()
    {
        for (int i = 0; i < 0xE0; i++)
        {
            if ((i < m3) || (i >= 0xE0 - m4))
            {
                interpolateCinematicBarData[i] = 0x1010000;
            }
            else
            {
                interpolateCinematicBarData[i] = i << 16;
            }
        }
    }

    void s_cinematicBarTask::cinematicBarTaskSub0(s32 r5)
    {
        m1 = r5;
        m2 = r5;
        m0_status = 3;
    }

    void s_cinematicBarTask::interpolateCinematicBar()
    {
        fixedPoint r5 = performDivision(fixedPoint(m2), fixedPoint(m1 * 16));
        if (fixedPoint(m3) != r5)
        {
            m3 = r5;
            m11 = 1;
        }

        r5 = performDivision(fixedPoint(m2), fixedPoint(m1 * 32));
        if (fixedPoint(m4) != r5)
        {
            m4 = r5;
            m11 = 1;
        }

        if (m11)
        {
            interpolateCinematicBarSub1();
        }
    }

    void s_cinematicBarTask::Update()
    {
        switch (m0_status)
        {
        case 2:
            if (m2 == ++m1)
            {
                m0_status = 1;
            }
            interpolateCinematicBar();
            break;
        case 1:
            return;
        case 3:
            if (--m1 == 0)
            {
                m0_status = 0;
            }
            interpolateCinematicBar();
            break;
        default:
            assert(0);
            break;
        }
    }

    void s_cinematicBarTask::Draw()
    {
        PDS_unimplemented("s_cinematicBarTask::Draw");
    }

    s_cinematicBarTask* s_fieldScriptWorkArea::startCinmaticBarTask()
    {
        return static_cast<s_cinematicBarTask*>(createSubTask(this, s_cinematicBarTask::getTaskDefinition(), new s_cinematicBarTask));
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
        default:
            assert(0);
            break;
        }

        return r14 + (numArguments + 1) * 4;
    }

    void setupCinematicBars(s_cinematicBarTask* pCinematicBar, s32 r5)
    {
        pCinematicBar->m1 = 0;
        pCinematicBar->m2 = r5;
        pCinematicBar->m0_status = 2;
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

        vdp2Controls.m20_registers[0].SFCODE = vdp2Controls.m20_registers[1].SFCODE = VDP2Regs_.SFCODE = 0xC000;
        vdp2Controls.m20_registers[0].CCRNB = vdp2Controls.m20_registers[1].CCRNB = VDP2Regs_.CCRNB = (vdp2Controls.m4_pendingVdp2Regs->CCRNB & 0xFFE0) | 0x10;
    }

    void createMultiChoiceDefault(s32 r4)
    {
        createMultiChoice(r4, -1);
    }

    void s_multiChoiceTask2::drawMultiChoice()
    {
        drawBlueBox(m14_x, m16_y, m1A_width, m1C_height);

        setupVDP2StringRendering(m14_x + 2, m16_y + 1, m1A_width - 4, m1C_height - 2);

        for (int i = 0; i < m6_numEntries; i++)
        {
            vdp2StringContext.m4_cursorX = vdp2StringContext.mC_X + 2;
            vdp2StringContext.m8_cursorY = vdp2StringContext.m10_Y + i * 2;
            if(i < 0)
            {
                vdp2StringContext.m8_cursorY = vdp2StringContext.m10_Y + i * 2 + vdp2StringContext.m18_Height;
            }

            vdp2StringContext.m0 = m28_colors[i];
            drawInventoryString((char*)getSaturnPtr(readSaturnEA(m24_strings + i * 4)));
        }
    }

    void s_multiChoiceTask2::Update()
    {
        switch (m0_Status)
        {
        case 0:
            m0_Status++;
        case 1:
            drawMultiChoice();
            playSoundEffect(3);
            m0_Status++;
            return;
        case 2:
            if (graphicEngineStatus.m4514.m0->m0_current.m8_newButtonDown & 0x10) // up
            {
                m5_selectedEntry--;
                if (m5_selectedEntry < 0)
                {
                    m5_selectedEntry += m6_numEntries;
                }
                playSoundEffect(2);
            }
            else if(graphicEngineStatus.m4514.m0->m0_current.m8_newButtonDown & 0x20) // down
            {
                m5_selectedEntry++;
                if (m5_selectedEntry >= m6_numEntries)
                {
                    m5_selectedEntry -= m6_numEntries;
                }
                playSoundEffect(2);
            }

            if (graphicEngineStatus.m4514.m0->m0_current.m8_newButtonDown & 6) // select
            {
                *mC_result = m5_selectedEntry;
                playSoundEffect(0);
                m0_Status++;
            }
            else if(graphicEngineStatus.m4514.m0->m0_current.m8_newButtonDown & 1) // cancel
            {
                if (m2_defaultResult)
                {
                    *mC_result = m2_defaultResult - 1;
                    playSoundEffect(1);
                    m0_Status++;
                }
            }
            return;
        case 3:
            mC_result = NULL;
            m0_Status++;
            return;
        case 4:
            getTask()->markFinished();
            return;
        default:
            assert(0);
            break;
        }
    }

    void s_multiChoiceTask2::Draw()
    {
        PDS_unimplemented("s_multiChoiceTask2::Draw");
    }

    s_multiChoiceTask2* updateMultiChoice(p_workArea parentTask, s_multiChoiceTask2** r5, s32* r6_currentChoice, s32 r7_minusCurrentChoice, sSaturnPtr scriptPtr, s16* choiceTable, s32 moreCurrentChoice)
    {
        s_multiChoiceTask2* r14 = static_cast<s_multiChoiceTask2*>(createSubTask(parentTask, s_multiChoiceTask2::getTaskDefinition(), new s_multiChoiceTask2));

        r14->m0_Status = 0;
        r14->m5_selectedEntry = moreCurrentChoice;

        if (r7_minusCurrentChoice >= 0)
        {
            r14->m1 = r7_minusCurrentChoice;
            r14->m2_defaultResult = 1;
        }
        else
        {
            r14->m1 = -r7_minusCurrentChoice;
            r14->m2_defaultResult = 0;
        }

        r14->m24_strings = scriptPtr;
        r14->m28_colors = choiceTable;
        r14->m3 = 0;
        r14->m4 = 0;
        r14->m6_numEntries = r14->m1;
        r14->m5_selectedEntry = moreCurrentChoice;
        r14->m7 = 0;
        r14->m8 = 0;
        r14->m14_x = 2;
        r14->m16_y = 4;

        s32 r12 = 0;
        for (int i = 0; i < r14->m1; i++)
        {
            s32 stringLength = computeStringLength(scriptPtr + i * 4, 38);
            if (stringLength > r12)
            {
                r12 = stringLength;
            }
        }

        r14->m1A_width = ((r12 + 1) & ~1) + 6;
        r14->m1C_height = (r14->m1 * 2) + 2;
        r14->mC_result = r6_currentChoice;

        if (r5)
        {
            *r5 = r14;
        }

        return r14;
    }

    s32 fieldPlayPCM(sSaturnPtr pPcmNameEA)
    {
        char* pPcmName = (char*)getSaturnPtr(pPcmNameEA);

        printf("trying to play PCM %s\n", pPcmName);

        PDS_unimplemented("fieldPlayPCM");

        return 0;
    }

    sSaturnPtr s_fieldScriptWorkArea::runFieldScript()
    {
        sSaturnPtr pScript = m4_currentScript;
        while(1)
        {
            u8 opCode = readSaturnU8(pScript); pScript = pScript + 1;

            switch (opCode)
            {
            case 0: // Ignore, happens sometimes
                continue;
            case 1: // END
                if (m8_stackPointer == &mC_stack[8])
                {
                    endScript();
                    return sSaturnPtr::getNull();
                }
                else
                {
                    assert(0);
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
                else
                {
                    continue;
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
            case 14:
                pScript = callNative(pScript);
                continue;
            case 21: // display string at bottom of screen
                {
                    pScript = pScript + 3;
                    pScript.m_offset &= ~3;

                    sSaturnPtr string = readSaturnEA(pScript);
                    pScript = pScript + 4;

                    setupVDP2StringRendering(3, 25, 38, 2);
                    vdp2StringContext.m0 = 0;
                    VDP2DrawString((char*)getSaturnPtr(string));
                    continue;
                }
            case 22: // clear string
                setupVDP2StringRendering(3, 0x19, 0x26, 2);
                clearVdp2TextArea();
                continue;
            case 24: // cinematic
                if(m30_cinematicBarTask)
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
                    m30_cinematicBarTask = startCinmaticBarTask();
                    setupCinematicBars(m30_cinematicBarTask, 4);
                    pScript = pScript - 1;
                    return pScript;
                }
            case 26: // clean cinematic bars
                if (m30_cinematicBarTask)
                {
                    if (m30_cinematicBarTask->m0_status == 1)
                    {
                        m30_cinematicBarTask->cinematicBarTaskSub0(5);
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
                else
                {
                    continue;
                }
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
                *(--m8_stackPointer) = pScript + 4; // store the return address
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

    void s_fieldScriptWorkArea::Update()
    {
        fieldScriptTaskUpdateSub1();

        if (m80)
        {
            if (m80->getTask()->isFinished())
            {
                m80 = NULL;
            }
        }

        if (readKeyboardToggle(0x87))
        {
            assert(0);
        }

        s_LCSTask* pLCS = getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS;
        if ((pLCS->m83F == 0) && (m40 == 0) && (m4_currentScript.m_offset))
        {
            fieldScriptTaskUpdateSub2();

            if ((m50_scriptDelay == 0) || (m58 != 0))
            {
                m4_currentScript = runFieldScript();

                if (m4_currentScript.m_offset == NULL)
                {
                    fieldScriptTaskUpdateSub3();
                }
            }
            else
            {
                m50_scriptDelay--;
            }
        }

        if(!fieldScriptTaskUpdateSub4() || fieldScriptTaskUpdateSub5() || !getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m83F || getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->mF8_Flags & 0x20000)
        {
            //06069A18
            fieldScriptTaskUpdateSub6();
            getFieldTaskPtr()->m28_status |= 0x40;
        }
        else
        {
            //06069A2C
            fieldScriptTaskUpdateSub7();
            getFieldTaskPtr()->m28_status &= ~0x40;
        }

        if (getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m8 ||
            m64 || m30_cinematicBarTask || m34 || m38_dialogStringTask || m3C_multichoiceTask || m40 ||
            getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->mF8_Flags & 0x20000)
        {
            graphicEngineStatus.m40AC.m1_isMenuAllowed = 0;
        }
        else
        {
            //06069A92
            graphicEngineStatus.m40AC.m1_isMenuAllowed = 1;
            if (graphicEngineStatus.m4514.m0[0].m0_current.m8_newButtonDown & graphicEngineStatus.m4514.mD8[1][13])
            {
                playSoundEffect(0);
                graphicEngineStatus.m40AC.m0_menuId = 5;
                return;
            }
            if (graphicEngineStatus.m4514.m0[0].m0_current.m8_newButtonDown & graphicEngineStatus.m4514.mD8[1][12])
            {
                //06069AF0
                assert(0);
            }
            return;
        }
    }

    s_taskDefinition fieldScriptTaskDefinition = { fieldScriptTaskInit, s_fieldScriptWorkArea::StaticUpdate, NULL, NULL, "fieldScriptTask" };

    void createFieldScriptTask(s_workArea* pWorkArea)
    {
        createSubTask(pWorkArea, &fieldScriptTaskDefinition, new s_fieldScriptWorkArea);
    }

    void fieldOverlaySubTaskInitSub2Sub1Sub1(sFieldCameraStatus* r11, s_dragonTaskWorkArea* stack_4)
    {
        s_fieldOverlaySubTaskWorkArea* pCameraData = getFieldTaskPtr()->m8_pSubFieldData->m334;
        s_fieldCameraConfig* r14 = &pCameraData->m10[pCameraData->m0_nextCamera];

        fixedPoint r14FP = FP_Div(stack_4->m8_pos[1] - r14->m50, r14->m54 - r14->m50);

        if (r14FP < 0)
        {
            r14FP = 0;
        }
        else if(r14FP > 0x10000)
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
            else if(r4 >= r5)
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
        r14->m5C[0] = interpolateRotation(r14->m5C[0], 0, 0x2000, 0x444444, 0);
        r14->m5C[1] = interpolateRotation(r14->m5C[1], 0, 0x2000, 0x444444, 0);
        r14->m5C[2] = interpolateRotation(r14->m5C[2], 0, 0x2000, 0x444444, 0);

        r14->m5C += r14->m68;
        r14->mC_rotation += r14->m5C;

        r14->m68.zero();

        r14->mC_rotation[0] = r14->mC_rotation[0].normalized();
        r14->m18 = r14->m18.normalized();

        fixedPoint r11 = r14->mC_rotation[0] + r14->m18;

        if (r11 < -0x31C71C7)
        {
            r14->mC_rotation[0] = -r14->m18 - 0x31C71C7;
            r11 = -0x31C71C7;
        }
        else if(r11 > 0x31C71C7)
        {
            r14->mC_rotation[0] = -r14->m18 + 0x31C71C7;
            r11 = 0x31C71C7;
        }

        sVec3_FP var10;
        var10[0] = -MTH_Mul_5_6(r14->m24_distanceToDestination, getCos(r11.getInteger() & 0xFFF), getSin(r14->mC_rotation[1].getInteger() & 0xFFF));
        var10[1] = MTH_Mul(r14->m24_distanceToDestination, getSin(r11.getInteger() & 0xFFF));
        var10[2] = -MTH_Mul_5_6(r14->m24_distanceToDestination, getCos(r11.getInteger() & 0xFFF), getCos(r14->mC_rotation[1].getInteger() & 0xFFF));

        r14->m0_position = r9->m8_pos - var10;

        r14->m44[0] = interpolateDistance(r14->m44[0], 0, 0x2000, 0xAAA, 0);
        r14->m44[1] = interpolateDistance(r14->m44[1], 0, 0x2000, 0xAAA, 0);
        r14->m44[2] = interpolateDistance(r14->m44[2], 0, 0x2000, 0xAAA, 0);

        r14->m44 += r14->m50;

        r14->m0_position += r14->m44;
        r14->m50.zero();
    }

    void fieldOverlaySubTaskInitSub2Sub2(sFieldCameraStatus* r14, s_dragonTaskWorkArea* r12)
    {
        fixedPoint stack_0[2];
        sVec3_FP stack_8;

        stack_8[0] = -r12->m88_matrix.matrix[2];
        stack_8[1] = -r12->m88_matrix.matrix[6];
        stack_8[2] = -r12->m88_matrix.matrix[10];

        generateCameraMatrixSub1(stack_8, stack_0);

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
            r14->mC_rotation[1] = stack_0[1];
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
        fixedPoint stack_0[2];
        sVec3_FP stack_8;

        stack_8[0] = -r12->m88_matrix.matrix[2];
        stack_8[1] = -r12->m88_matrix.matrix[6];
        stack_8[2] = -r12->m88_matrix.matrix[10];

        generateCameraMatrixSub1(stack_8, stack_0);

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
            r14->mC_rotation[1] = interpolateRotation(r14->mC_rotation[1], stack_0[1] , 0x2000, 0x222222, 0);
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

        switch (r14->m8D)
        {
        case 0:
            r14->m8F = 1;
            r14->m90 = 1;
            r14->m7C = 2;
            r14->m8D = 2;
            fieldOverlaySubTaskInitSub2Sub2(r14, pDragonTask);
            return;
        case 1:
            r14->m8F = 1;
            r14->m90 = 1;
            r14->m7C = 2;
            r14->m8D = 2;
        case 2:
            fieldOverlaySubTaskInitSub2Sub1(r14, pDragonTask);
            return;
        default:
            assert(0);
        }
    }

    void fieldOverlaySubTaskInitSub1Sub0(sFieldCameraStatus* r4)
    {
        r4->m74 = 0;
        r4->m78 = 0;
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
        r4->m80 = 0;
        r4->m84 = 0;
        r4->m88 = 0;

        r4->m89 = 0;
        r4->m8A = 0;
    }

    void fieldOverlaySubTaskInitSub1(u32 r4, void(*r5)(sFieldCameraStatus*), void(*r6)(sFieldCameraStatus*))
    {
        sFieldCameraStatus* pFieldCameraStatus = &getFieldTaskPtr()->m8_pSubFieldData->m334->m3E4[r4];
        fieldOverlaySubTaskInitSub1Sub0(pFieldCameraStatus);

        pFieldCameraStatus->m74 = r5;
        pFieldCameraStatus->m78 = r6;
        pFieldCameraStatus->m8C = 1;
    }

    u32 fieldOverlaySubTaskInitSub3(u32 r4)
    {
        if (updateCameraFromDragonSub1(r4))
        {
            s_fieldOverlaySubTaskWorkArea* p334 = getFieldTaskPtr()->m8_pSubFieldData->m334;
            p334->m50C = r4;
            p334->m3E4[r4].m80 = 0;
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
        s_fieldOverlaySubTaskWorkArea* p334 = getFieldTaskPtr()->m8_pSubFieldData->m334;

        p334->mC = 8;
        p334->m2DC = 4;

        for (int i = 0; i < r5; i++)
        {
            copyFieldCameraConfig(&r4[i], &p334->m10[i]);
        }
    }

    void fieldOverlaySubTaskInitSub5(u32 r4)
    {
        s_fieldOverlaySubTaskWorkArea* p334 = getFieldTaskPtr()->m8_pSubFieldData->m334;
        p334->m3E4[r4].m74 = 0;
        p334->m3E4[r4].m78 = 0;
        p334->m3E4[r4].m8C = 0;
    }

    void updateCameraFromDragonSub2(s_fieldOverlaySubTaskWorkArea* pTypedWorkArea)
    {
        sFieldCameraStatus* r13 = &pTypedWorkArea->m3E4[pTypedWorkArea->m50C];
        s16 r15[3];
        r15[0] = r13->mC_rotation[0] >> 16;
        r15[1] = r13->mC_rotation[1] >> 16;
        r15[2] = r13->mC_rotation[2] >> 16;

        updateEngineCamera(&cameraProperties2, r13, r15);

        copyMatrix(pCurrentMatrix, &pTypedWorkArea->m384);
        copyMatrix(&cameraProperties2.m28[0], &pTypedWorkArea->m3B4);
    }

    s_fieldCameraConfig unk_6092EF0 = {
        0,0,0,
        0,0,0,
    { -0xE38E38, 0, 0, 0, 0, 0, 0xA000 },
    { -0xE38E38, 0, 0, 0, 0, 0, 0xA000 },
    0,
    0x80000,
    };

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

    void fieldOverlaySubTaskInit(s_workArea* pWorkArea)
    {
        s_fieldOverlaySubTaskWorkArea* pTypedWorkArea = static_cast<s_fieldOverlaySubTaskWorkArea*>(pWorkArea);

        getFieldTaskPtr()->m8_pSubFieldData->m334 = pTypedWorkArea;

        fieldOverlaySubTaskInitSub1(0, &fieldOverlaySubTaskInitSub2, 0);
        fieldOverlaySubTaskInitSub3(0);
        setupFieldCameraConfigs(&unk_6092EF0, 1);

        getFieldTaskPtr()->m8_pSubFieldData->m334->m50E = 1;

        fieldOverlaySubTaskInitSub5(1);

        updateCameraFromDragonSub2(pTypedWorkArea);

        getFieldTaskPtr()->m8_pSubFieldData->m334->m50D = 1;
    }

    s_taskDefinition fieldOverlaySubTaskDefinition = { fieldOverlaySubTaskInit, NULL, NULL, NULL, "fieldOverlaySubTask" };

    void createFieldOverlaySubTask(s_workArea* pWorkArea)
    {
        createSubTask(pWorkArea, &fieldOverlaySubTaskDefinition, new s_fieldOverlaySubTaskWorkArea);
    }

    void dragonRidersTaskInit(s_workArea* pWorkArea)
    {
        {
            u8* pData = NULL;
            if (u32 offset = READ_BE_U32(pRider1State->m0_riderModel + 0x30))
            {
                pData = pRider1State->m0_riderModel + offset;
            }
            riderInit(&pRider1State->m18_3dModel, pData);
            updateAndInterpolateAnimation(&pRider1State->m18_3dModel);
        }

        {
            u8* pData = NULL;
            if (u32 offset = READ_BE_U32(pRider2State->m0_riderModel + 0x30))
            {
                pData = pRider2State->m0_riderModel + offset;
            }
            riderInit(&pRider2State->m18_3dModel, pData);
            updateAndInterpolateAnimation(&pRider2State->m18_3dModel);
        }
    }

    void dragonRidersTaskUpdate(s_workArea* pWorkArea)
    {
        PDS_unimplemented("dragonRidersTaskUpdate");
    }

    s_taskDefinition dragonRidersTaskDefinition = { dragonRidersTaskInit, dragonRidersTaskUpdate, NULL, NULL, "dragonRidersTask" };

    void dragonFieldSubTask2InitSub1(s32 r4)
    {
        s_PaletteTaskWorkArea* r14 = getFieldTaskPtr()->m8_pSubFieldData->m33C_pPaletteTask;
        r14->m64 = r4;
        r14->m68 = MTH_Mul(r4, r4);
    }

    void dragonFieldSubTask2Init(s_workArea* pWorkArea)
    {
        s_PaletteTaskWorkArea* pTypedWorkArea = static_cast<s_PaletteTaskWorkArea*>(pWorkArea);
        
        getFieldTaskPtr()->m8_pSubFieldData->m33C_pPaletteTask = pTypedWorkArea;

        getMemoryArea(&pTypedWorkArea->m0, 0);
        u32 r2 = pTypedWorkArea->m0.m4_characterArea - getVdp1Pointer(0x25C00000);
        pTypedWorkArea->m3C = getVdp1Pointer(0x25C00000) + ((0x1748 + (r2 >> 3)) << 3);
        pTypedWorkArea->m8 = 0x50;
        pTypedWorkArea->mC = pTypedWorkArea->m8 + 0x30;
        pTypedWorkArea->mE = -pTypedWorkArea->mA + 0x60;
        pTypedWorkArea->m30 = 0x18;
        pTypedWorkArea->m34 = 0x18;
        pTypedWorkArea->m10 = 6;
        pTypedWorkArea->m12 = -14;
        pTypedWorkArea->m28 = -0x1D;
        pTypedWorkArea->m2A = -0x21;

        pTypedWorkArea->m50 = 0x1C;
        pTypedWorkArea->m54 = 0x18;
        pTypedWorkArea->m5C = 0x40000;
        pTypedWorkArea->m60 = 0x40000;

        dragonFieldSubTask2InitSub1(0x200000);
    }

    u8 fieldPalettes[1][0x20] =
    {
        {
            0xA0,
            0xC4,
            0x9C,
            0x00,
            0xA8,
            0x00,
            0xB8,
            0x00,
            0xC8,
            0x00,
            0xCC,
            0x00,
            0xD4,
            0x00,
            0xDC,
            0x00,
            0xE4,
            0x00,
            0xF0,
            0x00,
            0xFC,
            0x00,
            0xFC,
            0x20,
            0xFC,
            0x61,
            0xFC,
            0xA2,
            0xFC,
            0xE3,
            0xFD,
            0x04,
        },
    };

    s8 paletteIndexTable[4] = {
        0,1,2,2
    };

    void dragonFieldSubTask2Update(s_workArea* pWorkArea)
    {
        s_PaletteTaskWorkArea* pTypedWorkArea = static_cast<s_PaletteTaskWorkArea*>(pWorkArea);
        PDS_unimplemented("dragonFieldSubTask2Update");

        //if (graphicEngineStatus.m40AC.m8 == 2)
        {
            s8 paletteIndex = paletteIndexTable[pTypedWorkArea->m4C];

            asyncDmaCopy(fieldPalettes[paletteIndex], pTypedWorkArea->m3C, 0x20, 0);
        }

        
    }

    void dragonFieldSubTask2Draw(s_workArea* pWorkArea)
    {
        s_PaletteTaskWorkArea* pTypedWorkArea = static_cast<s_PaletteTaskWorkArea*>(pWorkArea);
        PDS_unimplemented("dragonFieldSubTask2Draw");
    }

    s_taskDefinition dragonFieldSubTask2Definition = { dragonFieldSubTask2Init, dragonFieldSubTask2Update, dragonFieldSubTask2Draw, NULL, "dragonFieldSubTask2" };

    void initDragonFieldSubTask2(s_workArea* pWorkArea)
    {
        createSubTask(pWorkArea, &dragonFieldSubTask2Definition, new s_PaletteTaskWorkArea);
    }

    void dragonFieldTaskInitSub2Sub2(fixedPoint* m178)
    {
        m178[0] = 0x222222;
        m178[1] = 0x4CCC;
        m178[2] = 0x16;
        m178[3] = 0x111111;
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
        return &getFieldTaskPtr()->m8_pSubFieldData->m334->m3E4[getFieldTaskPtr()->m8_pSubFieldData->m334->m50C];
    }

    void dragonFieldTaskInitSub2(s_dragonTaskWorkArea* pWorkArea)
    {
        initDragonFieldSubTask2(pWorkArea);

        dragonFieldTaskInitSub2Sub2(pWorkArea->m178);

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

        pWorkArea->m1CC = 0x38E38E3; // field of view
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
        pWorkArea->mC0 = 0xC000000;
        pWorkArea->mC4 = 0;

        pWorkArea->mC8[0] = 0x10;
        pWorkArea->mC8[1] = 0x10;
        pWorkArea->mC8[2] = 0x10;

        pWorkArea->mCB[0] = 0x8;
        pWorkArea->mCB[1] = 0x8;
        pWorkArea->mCB[2] = 0x8;

        pWorkArea->mCE[0] = 0x14;
        pWorkArea->mCE[1] = 0x14;
        pWorkArea->mCE[2] = 0x14;

        pWorkArea->mD1[0] = 0xC;
        pWorkArea->mD1[1] = 0xC;
        pWorkArea->mD1[2] = 0xC;

        pWorkArea->mD4[0] = 0x10;
        pWorkArea->mD4[1] = 0x10;
        pWorkArea->mD4[2] = 0x10;

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
        dragonFieldTaskInitSub3Sub1(&pDragonState->m28_dragon3dModel, pDragonState->m0_pDragonModelRawData + READ_BE_U32(pDragonState->m0_pDragonModelRawData + pDragonState->m20_dragonAnimOffsets[param2]));
        updateAndInterpolateAnimation(&pDragonState->m28_dragon3dModel);

        pWorkArea->m23A_dragonAnimation = param2;
        pWorkArea->m237 = pWorkArea->m238;
        pWorkArea->m23B = 1;
    }

    void dragonFieldTaskInitSub4Sub3(u8 r4)
    {
        PDS_unimplemented("dragonFieldTaskInitSub4Sub3");
    }

    s32 isDragonInValidArea(s_dragonTaskWorkArea* r4)
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

    void dragonFieldTaskInitSub4Sub5(s_dragonTaskWorkArea_48* r14, sVec3_FP* r13)
    {
        r14->m30 = r13->m_value[0];
        r14->m34 = r13->m_value[1];
        r14->m38 = r13->m_value[2];
        r14->m3C = 1;

        initMatrixToIdentity(&r14->m0_matrix);
        rotateMatrixShiftedY(r13->m_value[1], &r14->m0_matrix);
        rotateMatrixShiftedX(r13->m_value[0], &r14->m0_matrix);
        rotateMatrixShiftedZ(r13->m_value[2], &r14->m0_matrix);
    }

    void computeDragonDeltaTranslation(s_dragonTaskWorkArea* r14)
    {
        PDS_unimplemented("updateDragonRotationSub1");
    }

    void dragonFieldTaskInitSub4Sub4Sub2()
    {
        getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->mF8_Flags &= ~0x10000;
    }

    void updateDragonMovementFromControllerType0Sub1(s_dragonTaskWorkArea* r14)
    {
        r14->m254 = 0;
        r14->m250 = 0;
        r14->m24A_runningCameraScript = 0;
        r14->m258 = 0;
        r14->m25C = 0;
        r14->m25D = 0;
    }

    void updateDragonMovementFromControllerType0(s_dragonTaskWorkArea* r14)
    {
        if (--r14->m25D < 0)
        {
            updateDragonMovementFromControllerType0Sub1(r14);
        }

        // update yaw
        {
            fixedPoint tempRotX = r14->m3C[0] - r14->m20_angle[0];
            r14->m20_angle[0] += r14->m3C[0] - performDivision(0x10, tempRotX.normalized() * 15) - r14->m20_angle[0];
        }

        if (r14->m25D == 2)
        {
            //0607EA3E
            r14->m20_angle[2] += r14->m254;
        }
        else
        {
            // update roll
            fixedPoint tempRotZ = r14->m3C[2] - r14->m20_angle[2];
            r14->m20_angle[2] += r14->m3C[2] - performDivision(0x10, tempRotZ.normalized() * 15) - r14->m20_angle[2];
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
        r14->m246 = 0;
        r14->m245 = 0;

        r14->m25C &= 0xFFFFFFFE;
    }

    u32 updateDragonMovementFromControllerType1Sub1(s_dragonTaskWorkArea* r4)
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

    void updateDragonMovementFromControllerType1Sub2(s_dragonTaskWorkArea* r14, s_dragonState* r12)
    {
        if (graphicEngineStatus.m4514.m0[0].m0_current.m8_newButtonDown & graphicEngineStatus.m4514.mD8[1][5]) // down
        {
            updateDragonMovementFromControllerType1Sub2Sub1(&r12->m78_animData, r14->m178[3]);
        }
        else if (graphicEngineStatus.m4514.m0[0].m0_current.m8_newButtonDown & graphicEngineStatus.m4514.mD8[1][4]) // up
        {
            updateDragonMovementFromControllerType1Sub2Sub1(&r12->m78_animData, -r14->m178[3]);
        }

        if (graphicEngineStatus.m4514.m0[0].m0_current.m8_newButtonDown & graphicEngineStatus.m4514.mD8[1][7]) // right
        {
            updateDragonMovementFromControllerType1Sub2Sub2(&r12->m78_animData, r14->m178[3]);
            updateDragonMovementFromControllerType1Sub2Sub3(&r12->m78_animData, -r14->m178[3]);
        }
        else if (graphicEngineStatus.m4514.m0[0].m0_current.m8_newButtonDown & graphicEngineStatus.m4514.mD8[1][6]) // left
        {
            updateDragonMovementFromControllerType1Sub2Sub2(&r12->m78_animData, -r14->m178[3]);
            updateDragonMovementFromControllerType1Sub2Sub3(&r12->m78_animData, r14->m178[3]);
        }
    }

    void updateDragonMovementFromControllerType1Sub3Sub1(s_dragonTaskWorkArea* r14)
    {
        PDS_unimplemented("updateDragonMovementFromControllerType1Sub3Sub1");
    }

    void updateDragonMovementFromControllerType1Sub3(s_dragonTaskWorkArea* r14)
    {
        updateDragonMovementFromControllerType1Sub3Sub1(r14);

        fixedPoint r2;
        if (r14->m154_dragonSpeed >= 0)
        {
            r2 = r14->m154_dragonSpeed;
        }
        else
        {
            r2 = -r14->m154_dragonSpeed;
        }

        if (r2 >= 256)
        {
            //0607EC06
            if (graphicEngineStatus.m4514.m0[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8[1][5]) // Go down
            {
                //0607EC12
                r14->m20_angle[0] += r14->m178[0];
                r14->m238 |= 2;
                r14->m1F0.m_8 = r14->m178[0];
                r14->mFC |= 2;
            }
            else if (graphicEngineStatus.m4514.m0[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8[1][4]) // Go up
            {
                //0607EC44
                r14->m20_angle[0] -= r14->m178[0];
                r14->m238 |= 1;
                r14->m1F0.m_8 = -r14->m178[0];
                r14->mFC |= 1;
            }
            //607EC6C
            fixedPoint r1 = r14->m3C[0] - r14->m20_angle[0];
            r14->m20_angle[0] += r14->m3C[0] - performDivision(0x10, r1.normalized() * 15) - r14->m20_angle[0];
        }
        else
        {
            //0607ECA4
            if (graphicEngineStatus.m4514.m0[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8[1][5]) // Go down
            {
                //607ECB0
                r14->m160_deltaTranslation[1] -= 0x800;
                r14->m238 |= 2;
                r14->m1F0.m_8 = -r14->m178[0];
                r14->mFC |= 2;
            }
            else if (graphicEngineStatus.m4514.m0[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8[1][4]) // Go up
            {
                //607ED00
                r14->m160_deltaTranslation[1] += 0x800;
                r14->m238 |= 1;
                r14->m1F0.m_8 = -r14->m178[0];
                r14->mFC |= 1;
            }
            //607ED32
            fixedPoint r1 = r14->m3C[0] - r14->m20_angle[0];
            r14->m20_angle[0] += r14->m3C[0] - performDivision(0x10, r1.normalized() * 15) - r14->m20_angle[0];
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
            fixedPoint r1 = r14->m3C[2] - r14->m20_angle[2];
            r14->m20_angle[2] += r14->m3C[2] - performDivision(0x10, r1.normalized() * 15) - r14->m20_angle[2];

            if (r14->m25D == 1)
            {
                assert(0);
                //updateDragonMovementFromControllerType1Sub3Sub1(r14);
            }
            else
            {
                if (graphicEngineStatus.m4514.m0[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8[1][7]) // right
                {
                    //0607EE34
                    if ((r14->mF8_Flags & 0x8000) == 0)
                    {
                        r14->m20_angle[1] -= r14->m178[0];
                    }

                    r14->m1F0.m_C = -r14->m178[0];
                    if (r14->m30 - r14->m20_angle[1] >= r14->m178[0])
                    {
                        r14->m20_angle[2] += performDivision(4, r14->m178[0] * 3);
                    }

                    r14->mFC |= 8;
                    r14->m25E = 0;
                }
                else if (graphicEngineStatus.m4514.m0[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8[1][6]) // left
                {
                    //0607EE8C
                    if ((r14->mF8_Flags & 0x8000) == 0)
                    {
                        r14->m20_angle[1] += r14->m178[0];
                    }

                    r14->m1F0.m_C = r14->m178[0];
                    if (r14->m20_angle[1] - r14->m30 >= r14->m178[0])
                    {
                        r14->m20_angle[2] -= performDivision(4, r14->m178[0] * 3);
                    }

                    r14->mFC |= 4;
                    r14->m25E = 1;
                }
            }
        }

        r14->m247 = 0;
        r14->m246 = 0;
        r14->m245 = 0;
    }

    void updateDragonMovementFromControllerType1(s_dragonTaskWorkArea* r14)
    {
        if (updateDragonMovementFromControllerType1Sub1(r14))
        {
            updateDragonMovementFromControllerType1Sub2(r14, gDragonState);
            updateDragonMovementFromControllerType1Sub3(r14);
            return;
        }
        updateDragonMovementFromControllerType0(r14);
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

    u32 integrateDragonMovementSub4Sub3()
    {
        u32 T = !mainGameState.getBit(0x2A, 6);
        return T ^ 1;
    }

    u32 integrateDragonMovementSub4Sub2()
    {
        u32 T = !mainGameState.getBit(0x2A, 7);
        return T ^ 1;
    }

    void integrateDragonMovementSub4Sub1(s_dragonTaskWorkArea* r4)
    {
        if ((r4->m25C & 1) == 0)
        {
            r4->m235_dragonSpeedIndex = 0;
            r4->m234 = 0;
            r4->m25C = 0;
        }
    }

    void integrateDragonMovementSub4(s_dragonTaskWorkArea* r14)
    {
        if (r14->m25C & 0x1)
        {
            assert(0);
        }

        if ((r14->m25C & 0x2) == 0)
        {
            if (graphicEngineStatus.m4514.m0[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8[1][11])
            {
                if (graphicEngineStatus.m4514.m0[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8[1][0])
                {
                    r14->m235_dragonSpeedIndex = -1;
                }
                else
                {
                    //0607E910
                    assert(0);
                }
            }
        }

        r14->m25C |= 2;

        if (integrateDragonMovementSub4Sub2())
        {
            assert(0);
        }

        u8 r5;
        if (r14->m25C & 4)
        {
            r5 = 4;
        }
        else
        {
            r5 = 3;
        }

        if (graphicEngineStatus.m4514.m0[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8[1][0]) // go forward
        {
            //0607E960
            if (++r14->m234 > 4)
            {
                if (++r14->m235_dragonSpeedIndex >= r5)
                {
                    r14->m235_dragonSpeedIndex = r5;
                }
                r14->m234 = 0;
            }
        }
        else
        {
            integrateDragonMovementSub4Sub1(r14);
        }

        if (integrateDragonMovementSub4Sub3())
        {
            //0607E9AE
            assert(0);
        }
    }

    void integrateDragonMovement(s_dragonTaskWorkArea* r14)
    {
        getFieldTaskPtr()->m28_status &= 0xFFFEFFFF;

        dragonFieldTaskInitSub4Sub5(&r14->m48, &r14->m20_angle);
        copyMatrix(&r14->m48.m0_matrix, &r14->m88_matrix);

        if (isDragonControlledByScripts())
        {
            clearDragonPlayerInputs();
        }

        if (isDragonPlayerControlAllowed())
        {
            if ((r14->mF8_Flags & 0x10000) == 0)
            {
                integrateDragonMovementSub4(r14);
            }

            if (r14->m235_dragonSpeedIndex < 0)
            {
                //0607FE38
                r14->m15C_dragonSpeedIncrement = MTH_Mul( -r14->m154_dragonSpeed - 0x1284, r14->m230);
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
                    r14->m15C_dragonSpeedIncrement = MTH_Mul(-r12, r14->m230);
                }
                else
                {
                    r14->m15C_dragonSpeedIncrement = -MTH_Mul(r12, r14->m230);
                }
            }
        }
        else
        {
            //607FE98
            if (r14->m154_dragonSpeed >= 0)
            {
                r14->m15C_dragonSpeedIncrement = MTH_Mul(r14->m154_dragonSpeed, r14->m230);
            }
            else
            {
                r14->m15C_dragonSpeedIncrement = -MTH_Mul(r14->m154_dragonSpeed, r14->m230);
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

        r14->m1A0.zero();

        // ~0607FFEC 
        r14->m194[0] = interpolateDistance(r14->m194[0], 0, 0x2000, 0xAAA, 0x10);
        r14->m194[1] = interpolateDistance(r14->m194[1], 0, 0x2000, 0xAAA, 0x10);
        r14->m194[2] = interpolateDistance(r14->m194[2], 0, 0x2000, 0xAAA, 0x10);

        r14->m194 += r14->m188;

        if (r14->m154_dragonSpeed < 0)
        {
            //06080066
            r14->m160_deltaTranslation[0] += r14->m194[0] - MTH_Mul(r14->m88_matrix.matrix[2], r14->m154_dragonSpeed);
            r14->m160_deltaTranslation[1] += r14->m194[1];
        }
        else
        {
            r14->m160_deltaTranslation[0] += r14->m194[0] - MTH_Mul(r14->m88_matrix.matrix[2], r14->m154_dragonSpeed);
            r14->m160_deltaTranslation[1] += r14->m194[1] + MTH_Mul(r14->m88_matrix.matrix[6], r14->m154_dragonSpeed);
        }
        r14->m160_deltaTranslation[2] += r14->m194[2] - MTH_Mul(r14->m88_matrix.matrix[10], r14->m154_dragonSpeed);

        r14->m188.zero();

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

        // Adjust pitch min/max when close to the min/maxY
        {
            fixedPoint r2;
            fixedPoint r6 = -(r14->m140_maxY - r14->m8_pos[1]) * 0x111;
            if (r6 >= -0x3555555)
            {
                r2 = r6;
            }
            else
            {
                r2 = -0x3555555;
            }

            fixedPoint r3;
            if (r2 >= 0)
            {
                r3 = 0;
            }
            else if (r6 >= -0x3555555)
            {
                r3 = r6;
            }
            else
            {
                r3 = -0x3555555;
            }
            r14->m148_pitchMin = r3;
        }

        // same for min
        {
            fixedPoint r2;
            fixedPoint r5 = (r14->m8_pos[1] - r14->m134_minY) * 0x111;
            if (r5 < 0x3555555)
            {
                r2 = r5;
            }
            else
            {
                r2 = 0x3555555;
            }

            fixedPoint r3;
            if (r2 < 0)
            {
                r3 = 0;
            }
            else if (r5 < 0x3555555)
            {
                r3 = r5;
            }
            else
            {
                r3 = 0x3555555;
            }
            r14->m14C_pitchMax = r3;
        }
    }

    void updateDragonMovement(s_dragonTaskWorkArea* r4)
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
            if (!isDragonInValidArea(r4))
            {
                r4->mF0 = dragonLeaveArea;
                r4->m104_dragonScriptStatus = 0;
                dragonLeaveArea(r4);
                return;
            }
        }

        r4->m160_deltaTranslation[0] = 0;
        r4->m160_deltaTranslation[1] = 0;
        r4->m160_deltaTranslation[2] = 0;

        r4->m238 &= ~3;

        PDS_unimplemented("Hacking input to type 1");
        graphicEngineStatus.m4514.m0[0].m0_current.m0 = 1;
        switch (graphicEngineStatus.m4514.m0[0].m0_current.m0)
        {
        case 1:
            updateDragonMovementFromControllerType1(r4);
            break;
        default:
            assert(0);
        }

        integrateDragonMovement(r4);
    }

    void dragonFieldTaskInitSub4Sub4()
    {
        s_dragonTaskWorkArea* pDragonTask = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
        if (gDragonState->mC_dragonType == DR_LEVEL_8_FLOATER)
        {
            assert(0);
        }
        else
        {
            pDragonTask->mF0 = updateDragonMovement;
        }

        pDragonTask->m104_dragonScriptStatus = 0;

        dragonFieldTaskInitSub4Sub4Sub2();
    }

    void dragonLeaveArea(s_dragonTaskWorkArea* r14)
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

        fixedPoint var_0[2];
        generateCameraMatrixSub1(var_8, var_0);

        // update yaw
        s32 tempRotX = r14->m3C[0] - r14->m20_angle[0];
        if (tempRotX & 0x8000000)
        {
            tempRotX |= 0xF0000000;
        }
        else
        {
            tempRotX &= 0xFFFFFFF;
        }

        r14->m20_angle[0] += r14->m3C[0] - performDivision(0x10, (tempRotX << 4) - tempRotX) - r14->m20_angle[0];

        // update pitch
        r14->m20_angle[1] = interpolateRotation(r14->m20_angle[1], var_8[0], 0x2000, 0x444444, 0x10);

        // update roll
        s32 tempRotZ = r14->m3C[2] - r14->m20_angle[2];
        if (tempRotZ & 0x8000000)
        {
            tempRotZ |= 0xF0000000;
        }
        else
        {
            tempRotZ &= 0xFFFFFFF;
        }
        r14->m20_angle[2] += r14->m3C[2] - performDivision(0x10, (tempRotZ << 4) - tempRotZ) - r14->m20_angle[2];

        if (r14->m30 - r14->m20_angle[1] < r14->m178[0])
        {
            r14->m20_angle[2] += performDivision(4, r14->m178[0] * 3);
        }
        else if (r14->m20_angle[1] - r14->m30 < r14->m178[0])
        {
            r14->m20_angle[2] -= performDivision(4, r14->m178[0] * 3);
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
        dragonFieldTaskInitSub4Sub5(&r14->m48, &r14->m20_angle);
        copyMatrix(&r14->m48.m0_matrix, &r14->m88_matrix);

        r14->m15C_dragonSpeedIncrement = 0;
        r14->m154_dragonSpeed = 0;

        computeDragonDeltaTranslation(r14);

        r14->m8_pos += r14->m160_deltaTranslation;

        // this is all copied from dragonLeaveArea but looks like it's exactly the same
        if ((r14->m134_minY == 0) && (r14->m140_maxY == 0))
            return;

        if (r14->m8_pos[1] < r14->m134_minY)
            r14->m8_pos[1] = r14->m134_minY;

        if (r14->m8_pos[1] > r14->m140_maxY)
            r14->m8_pos[1] = r14->m140_maxY;

        r14->m160_deltaTranslation = r14->m8_pos - r14->m14_oldPos;

        // Adjust pitch min/max when close to the min/maxY
        {
            fixedPoint r2;
            fixedPoint r6 = -(r14->m140_maxY - r14->m8_pos[1]) * 0x111;
            if (r6 >= -0x3555555)
            {
                r2 = r6;
            }
            else
            {
                r2 = -0x3555555;
            }

            fixedPoint r3;
            if (r2 >= 0)
            {
                r3 = 0;
            }
            else if (r6 >= -0x3555555)
            {
                r3 = r6;
            }
            else
            {
                r3 = -0x3555555;
            }
            r14->m148_pitchMin = r3;
        }

        // same for min
        {
            fixedPoint r2;
            fixedPoint r5 = (r14->m8_pos[1] - r14->m134_minY) * 0x111;
            if (r5 < 0x3555555)
            {
                r2 = r5;
            }
            else
            {
                r2 = 0x3555555;
            }

            fixedPoint r3;
            if (r2 < 0)
            {
                r3 = 0;
            }
            else if (r5 < 0x3555555)
            {
                r3 = r5;
            }
            else
            {
                r3 = 0x3555555;
            }
            r14->m14C_pitchMax = r3;
        }
    }

    void dragonFieldTaskInitSub4Sub6(s_dragonTaskWorkArea* r4)
    {
        sVec3_FP var;

        var[0] = r4->m160_deltaTranslation[0] << 8;
        var[1] = r4->m160_deltaTranslation[1] << 8;
        var[2] = r4->m160_deltaTranslation[2] << 8;

        s32 r0 = dot3_FP(&var, &var);
        r4->m154_dragonSpeed = sqrt_F(r0) >> 8;
    }

    void updateCameraScriptSub0(u32 r4)
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

    void(*updateCameraScriptSub1Table1[10])(sFieldCameraStatus*) = {
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

    void dragonFieldTaskUpdateSub5Sub4(sFieldCameraStatus* r4);
    void dragonFieldTaskUpdateSub5Sub3(sFieldCameraStatus* r4);

    void(*updateCameraScriptSub1Table2[10])(sFieldCameraStatus*) = {
        dragonFieldTaskUpdateSub5Sub3,
        fieldOverlaySubTaskInitSub2,
        dummyFunct,
        dummyFunct,
        dummyFunct,
        dummyFunct,
        dummyFunct,
        0,
        dragonFieldTaskUpdateSub5Sub4,
        0,
    };

    s32 updateCameraScriptSub1Sub(u32 r4, void(*r5)(sFieldCameraStatus*), void(*r6)(sFieldCameraStatus*))
    {
        if (updateCameraFromDragonSub1(r4))
        {
            sFieldCameraStatus* pCamera = &getFieldTaskPtr()->m8_pSubFieldData->m334->m3E4[r4];
            pCamera->m74 = r5;
            pCamera->m78 = r6;
            pCamera->m8D = 0;
            pCamera->m8E = 0;
            return 1;
        }
        return 0;
    }

    void updateCameraScriptSub1(u32 r4)
    {
        updateCameraScriptSub1Sub(0, updateCameraScriptSub1Table1[r4], updateCameraScriptSub1Table2[r4]);

        getFieldCameraStatus()->m8D = 1;
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
            updateCameraScriptSub0(r4->mB8);
            updateCameraScriptSub1(0);
            r4->mF8_Flags &= 0xFFFFFBFF;
            r4->mF8_Flags |= 0x20000;
            r4->m1E8_cameraScriptDelay = r5->m20;
            r4->m20_angle = r5->mC_rotation;
            r4->m8_pos = r5->m0_position;

            r4->m160_deltaTranslation[0] = MTH_Mul(-r5->m1C, getSin(r4->m20_angle[1].getInteger() & 0xFFF));
            r4->m160_deltaTranslation[1] = performDivision(r4->m1E8_cameraScriptDelay, r5->m18 - r4->m8_pos[1]);
            r4->m160_deltaTranslation[2] = MTH_Mul(-r5->m1C, getCos(r4->m20_angle[2].getInteger() & 0xFFF));

            dragonFieldTaskInitSub4Sub6(r4);
            updateCameraScriptSub0Sub2(r4);

            getFieldCameraStatus()->m0_position = r5->m24_pos2;
            getFieldCameraStatus()->m88 = r4->m1E8_cameraScriptDelay;
            r4->m104_dragonScriptStatus++;
        case 1:
            if (--r4->m1E8_cameraScriptDelay)
            {
                return;
            }
            getFieldCameraStatus()->m88 = 30;
            r4->m1E8_cameraScriptDelay = 30;
            updateCameraScriptSub1(getFieldTaskPtr()->m8_pSubFieldData->m334->m50E);
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

    void dragonFieldTaskInitSub4(s_dragonTaskWorkArea* pTypedWorkArea)
    {
        getFieldTaskPtr()->m28_status |= 0x10000;

        if (pTypedWorkArea->m1D4_cutsceneData)
        {
            assert(0);
        }
        else if (pTypedWorkArea->m1D0_cameraScript)
        {
            updateCameraScript(pTypedWorkArea, pTypedWorkArea->m1D0_cameraScript);
        }
        else
        {
            dragonFieldTaskInitSub4Sub3(getFieldTaskPtr()->m8_pSubFieldData->m334->m50E);
            dragonFieldTaskInitSub4Sub4();
        }

        dragonFieldTaskInitSub4Sub5(&pTypedWorkArea->m48, &pTypedWorkArea->m20_angle);

        copyMatrix(&pTypedWorkArea->m48.m0_matrix, &pTypedWorkArea->m88_matrix);

        pTypedWorkArea->m8_pos[0] += pTypedWorkArea->m160_deltaTranslation[0];
        pTypedWorkArea->m8_pos[1] += pTypedWorkArea->m160_deltaTranslation[1];
        pTypedWorkArea->m8_pos[2] += pTypedWorkArea->m160_deltaTranslation[2];

        dragonFieldTaskInitSub4Sub6(pTypedWorkArea);
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

    void dragonFieldTaskInit(s_workArea* pWorkArea, void* argument)
    {
        s_dragonTaskWorkArea* pTypedWorkArea = static_cast<s_dragonTaskWorkArea*>(pWorkArea);
        u32 arg = (u32)argument;

        getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask = pTypedWorkArea;

        getMemoryArea(&pTypedWorkArea->m0, 0);
        dragonFieldTaskInitSub2(pTypedWorkArea);
        dragonFieldTaskInitSub3(pTypedWorkArea, gDragonState, 5);
        pTypedWorkArea->mF0 = dragonFieldTaskInitSub4;

        createSubTask(pWorkArea, &dragonRidersTaskDefinition, new s_dummyWorkArea);

        if (gDragonState->mC_dragonType == DR_LEVEL_6_LIGHT_WING)
        {
            assert(0);
        }

        dragonFieldTaskInitSub5(pTypedWorkArea);

        if (shouldLoadPup())
        {
            assert(0);
        }
    }

    void dragonFieldTaskUpdateSub1Sub1()
    {
        s_dragonTaskWorkArea* pDragonTask = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;

        if (pDragonTask->mF8_Flags & 0x400)
            return;

        if (!pDragonTask->m249)
            return;

        assert(0);
    }

    void dragonFieldTaskUpdateSub1(s_dragonTaskWorkArea* pTypedWorkArea)
    {
        pTypedWorkArea->mFC = 0;
        pTypedWorkArea->m100 = 0;

        pTypedWorkArea->m14_oldPos = pTypedWorkArea->m8_pos;

        pTypedWorkArea->mF0(pTypedWorkArea);

        dragonFieldTaskUpdateSub1Sub1();
    }

    void setLCSField83E(s_workArea* pLCS, u32 value)
    {
        PDS_unimplemented("setLCSField83E");
    }

    void dragonFieldTaskUpdateSub2(u32 r4)
    {
        setLCSField83E(getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS, r4);
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
            dragonFieldTaskInitSub3Sub1(&r13->m28_dragon3dModel, r13->m0_pDragonModelRawData + READ_BE_U32(r13->m0_pDragonModelRawData + r13->m20_dragonAnimOffsets[r12]));
            r14->m23B = 1;
        }
        else
        {
            playAnimation(&r13->m28_dragon3dModel, r13->m0_pDragonModelRawData + READ_BE_U32(r13->m0_pDragonModelRawData + r13->m20_dragonAnimOffsets[r12]), 10);
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
        sSaturnPtr EA = readSaturnEA(sSaturnPtr{ 0x06094134 + (u32)((type * 5) + subtype) * 4, gFLD_A3 });

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
            if (getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m8 & 0x10)
            {
                assert(0);
            }
            else
            {
                if (pTypedWorkArea->m238 & 4)
                {
                    if (getDragonSpeedIndex(pTypedWorkArea) <= 0)
                    {
                        pTypedWorkArea->m238 &= 0xFFFFFFFB;
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
                if (r5->m28_dragon3dModel.m16)
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
                        PDS_warning("recheck the logic here, it might be accessing mC_dragonType as a byte (read byte at F)\n");
                        std::vector<s8> r3= getFieldDragonAnimTable(r5->mC_dragonType, r5->m1C_dragonArchetype);

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
                    if(r6 > 0)
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

    void selectCamera(s_fieldOverlaySubTaskWorkArea* r4, s_dragonTaskWorkArea* r5)
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

    void dragonFieldTaskUpdateSub5Sub1(s_fieldOverlaySubTaskWorkArea* r4, s_dragonTaskWorkArea* r5)
    {
        if (r4->m2E0)
        {
            assert(0);
        }

        s32 r13 = r4->m2DC;
        s32 var8[3];

        for (s32 r13 = r4->m2DC; r13--; r13 >= 0)
        {
            if (r13)
            {
                s_fieldOverlaySubTaskWorkArea2E4* r12 = &r4->m2E4[r13];
                s32 r6 = r5->m8_pos[0] - r12->m0[0];
                if (r6 >= 0)
                {
                    var8[0] = r6;
                }
                else
                {
                    var8[0] = r12->m0[0] - r5->m8_pos[0];
                }

                if (var8[0] >= r12->m14)
                    continue;

                if (r5->m8_pos[2] - r12->m0[2] < 0)
                {
                    var8[2] = r5->m8_pos[2] - r12->m0[2];
                }
                else
                {
                    var8[2] = r12->m0[2] - r5->m8_pos[2];
                }

                if (var8[2] >= r12->m14)
                    continue;

                if (MTH_Mul(var8[0], var8[0]) + MTH_Mul(var8[2], var8[2]) >= r12->m18_maxDistanceSquare)
                    continue;
            }

            //6061554
            if ((r4->m2D0 != r13) && (r4->m2D4 == r13))
            {
                r4->m2D8++;
            }
            else
            {
                r4->m2D4 = r13;
                r4->m2D8 = 0;
            }

            if (r4->m2D8 >= 30)
            {
                r4->m2D0 = r13;
                r4->m2D8 = 0;
            }

            return;
        }
    }

    void dragonFieldTaskUpdateSub5Sub3(sFieldCameraStatus* r4)
    {
        s_dragonTaskWorkArea* pDragonTask = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
        switch (r4->m8D)
        {
        case 0:
        case 1:
            r4->m8F = 0;
            r4->m90 = 0;
            r4->m8D = 2;
        case 2:
        default:
            if (pDragonTask->m1D0_cameraScript)
            {
                sVec3_FP r15_8 = pDragonTask->m8_pos - pDragonTask->m1D0_cameraScript->m24_pos2;
                fixedPoint r15[2];

                generateCameraMatrixSub1(r15_8, r15);

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

    void dragonFieldTaskUpdateSub5Sub4(sFieldCameraStatus* r4)
    {
        assert(0);
    }

    void dragonFieldTaskUpdateSub5Sub5(s_fieldOverlaySubTaskWorkArea* r4)
    {
        assert(0);
    }

    void dragonFieldTaskUpdateSub5()
    {
        s_fieldOverlaySubTaskWorkArea* r13 = getFieldTaskPtr()->m8_pSubFieldData->m334;
        s_dragonTaskWorkArea* r14 = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;

        if (r14)
        {
            selectCamera(r13, r14);
            dragonFieldTaskUpdateSub5Sub1(r13, r14);
        }

        for (int i = 0; i < 2; i++)
        {
            if (updateCameraFromDragonSub1(i))
            {
                if (r13->m2E4[4].m18_maxDistanceSquare)
                {
                    dragonFieldTaskUpdateSub5Sub2(r13, &r13->m3E4[i]);
                }
                else
                {
                    if ((r13->m3E4[i].m74 == dragonFieldTaskUpdateSub5Sub3) || (r13->m3E4[i].m74 == dragonFieldTaskUpdateSub5Sub4))
                    {
                        r13->m3E4[i].m74(&r13->m3E4[i]);
                    }
                    else
                    {
                        if ((r13->m2E0 > 0) || (r13->m2D0 > 0))
                        {
                            dragonFieldTaskUpdateSub5Sub5(r13);
                        }
                        else
                        {
                            if (r13->m3E4[i].m74)
                            {
                                r13->m3E4[i].m74(&r13->m3E4[i]);
                            }
                        }
                    }
                }
                r13->m3E4[i].m84++;
            }
        }
    }

    void dragonFieldTaskUpdateSub6(s_dragonTaskWorkArea* pTypedWorkArea)
    {
        PDS_unimplemented("dragonFieldTaskUpdateSub6");
    }

    void dragonFieldTaskUpdate(s_workArea* pWorkArea)
    {
        s_dragonTaskWorkArea* pTypedWorkArea = static_cast<s_dragonTaskWorkArea*>(pWorkArea);

        dragonFieldTaskUpdateSub1(pTypedWorkArea);

        fieldTaskVar2 = gDragonState->mC_dragonType;

        if (pTypedWorkArea->mB8)
        {
            assert(0);
        }

        if (gDragonState->mC_dragonType != pTypedWorkArea->m100)
        {
            assert(0); // because untested
            pTypedWorkArea->m100 = gDragonState->mC_dragonType;
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
            // TODO: recheck
            s32 r3 = -pTypedWorkArea->m20_angle[2];

            if (0 > r3)
            {
                r3++;
            }
            r3 >> 1;
            getFieldCameraStatus()->m30 = r3;
        }

        dragonFieldTaskUpdateSub5();

        dragonFieldTaskUpdateSub6(pTypedWorkArea);
    }

    s8 updateCameraFromDragonSub1(s32 index)
    {
        return getFieldTaskPtr()->m8_pSubFieldData->m334->m3E4[index].m8C;
    }

    void updateCameraFromDragon()
    {
        s_fieldOverlaySubTaskWorkArea* r12 = getFieldTaskPtr()->m8_pSubFieldData->m334;

        for (int i = 0; i < 2; i++)
        {
            if (updateCameraFromDragonSub1(i))
            {
                if (getFieldTaskPtr()->m8_pSubFieldData->debugMenuStatus1[i] == 0)
                {
                    sFieldCameraStatus* pFieldCameraStatus = &getFieldTaskPtr()->m8_pSubFieldData->m334->m3E4[i];
                    if (pFieldCameraStatus->m78)
                    {
                        pFieldCameraStatus->m78(pFieldCameraStatus);
                    }
                }
            }
        }

        updateCameraFromDragonSub2(r12);
    }

    void dragonFieldTaskDrawSub1(s_dragonTaskWorkArea* pTypedWorkArea)
    {
        updateCameraFromDragon();

        if ((pTypedWorkArea->m_EC & 1) == 0)
        {
            if (pTypedWorkArea->m_EB)
            {
                assert(false);
            }
            else
            {
                sVec3_FP varC;
                varC[0] = -MTH_Mul_5_6(fixedPoint(0x10000), getCos(pTypedWorkArea->mC0.getInteger() & 0xFFF), getSin(pTypedWorkArea->mC4.getInteger() & 0xFFF));
                varC[1] = MTH_Mul(fixedPoint(0x10000), getSin(pTypedWorkArea->mC0.getInteger() & 0xFFF));
                varC[2] = -MTH_Mul_5_6(fixedPoint(0x10000), getCos(pTypedWorkArea->mC0.getInteger() & 0xFFF), getCos(pTypedWorkArea->mC4.getInteger() & 0xFFF));

                sVec3_FP var0;
                //transformVecByCurrentMatrix(&varC, &var0);
//                assert(0);
                PDS_unimplemented("dragonFieldTaskDrawSub1");
            }

        }

        PDS_unimplemented("dragonFieldTaskDrawSub1");

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

        sVec3_FP* pVec = &r4->m28_dragon3dModel.m44[pHotSpotData->m0][pHotSpotData->m4];
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
        r5->m8 = 0;
        r5->m4 = 0;

        r4->m44 = r4->m68.begin();
    }

    void dragonFieldTaskDrawSub3(s_dragonTaskWorkArea* pTypedWorkArea)
    {
        if (((pTypedWorkArea->m_EC & 1) == 0) && (pTypedWorkArea->m_EB))
        {
            assert(0);
            PDS_unimplemented("dragonFieldTaskDrawSub3");
        }

        dragonFieldTaskDrawSub3Sub0();
    }

    void dragonFieldTaskDraw(s_workArea* pWorkArea)
    {
        s_dragonTaskWorkArea* pTypedWorkArea = static_cast<s_dragonTaskWorkArea*>(pWorkArea);

        dragonFieldTaskDrawSub1(pTypedWorkArea);

        // if we need to draw the dragon shadow (and dragon Y >= 0)
        if (!pTypedWorkArea->m249 && pTypedWorkArea->m248 && (pTypedWorkArea->m8_pos[1] >= 0))
        {
            assert(0);
        }

        if (pTypedWorkArea->m249)
        {
            assert(0);
        }
        else
        {
            WRITE_BE_U16(gDragonState->m0_pDragonModelRawData + 0x30, READ_BE_U16(gDragonState->m0_pDragonModelRawData + 0x30) | 1);
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

        if (pTypedWorkArea->m249 == 0)
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
                if (pRider1State->m18_3dModel.m44[5].size())
                {
                    //060744AA
                    transformAndAddVec(pRider1State->m18_3dModel.m44[5][0], rider1_hotSpot, cameraProperties2.m28[0]);
                    pushCurrentMatrix();
                    translateCurrentMatrix(&rider1_hotSpot);
                    rotateCurrentMatrixShiftedY(0x8000000);
                    multiplyCurrentMatrixSaveStack(&pTypedWorkArea->m48.m0_matrix);
                    u32 offset = READ_BE_U32(pRider1State->m0_riderModel + pRider1State->m_14);
                    addObjectToDrawList(pRider1State->m0_riderModel, offset);
                    popMatrix();
                }
            }
            //60744E4
            if (mainGameState.gameStats.m3_rider2)
            {
                assert(0);
            }
        }

        //06074520
        dragonFieldTaskDrawSub3(pTypedWorkArea);
    }

    s_taskDefinitionWithArg dragonFieldTaskDefinition = { dragonFieldTaskInit, dragonFieldTaskUpdate, dragonFieldTaskDraw, dummyTaskDelete, "dragonFieldTask" };

    void initFieldDragon(s_workArea* pWorkArea, u32 param)
    {
        if (getFieldTaskPtr()->updateDragonAndRiderOnInit == 1)
        {
            assert(0);
            //updateDragonIfCursorChanged(mainGameState.gameStats.dragonLevel);
            //loadRiderIfChanged(mainGameState.gameStats.rider1);
            //loadRider2IfChanged(mainGameState.gameStats.rider1);

            mainGameState.gameStats.currentHP = mainGameState.gameStats.maxHP;
            mainGameState.gameStats.currentBP = mainGameState.gameStats.maxBP;

            getFieldTaskPtr()->updateDragonAndRiderOnInit = 2;
        }

        if (gDragonState->mC_dragonType == DR_LEVEL_8_FLOATER)
        {
            assert(0);
        }
        else
        {
            createSubTaskWithArg(pWorkArea, &dragonFieldTaskDefinition, new s_dragonTaskWorkArea, (void*)param);
        }
    }

    void fieldOverlaySubTask2Init(p_workArea pWorkArea)
    {
        s_LCSTask* pTypedWorkArea = static_cast<s_LCSTask*>(pWorkArea);

        getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS = pTypedWorkArea;
        getMemoryArea(&pTypedWorkArea->m0, 0);
        pTypedWorkArea->m814 = 0x200000;
        if (gDragonState->mC_dragonType == 8)
        {
            setLCSField83E(pTypedWorkArea, 1);
        }
        else
        {
            setLCSField83E(pTypedWorkArea, gDragonState->mC_dragonType + 1);
        }

        pTypedWorkArea->m9C0 = allocateHeapForTask(pTypedWorkArea, 0x40);
    }

    s_taskDefinition fieldOverlaySubTask2Definition = { fieldOverlaySubTask2Init, NULL, NULL, NULL, "fieldOverlaySubTask2" };

    void createFieldOverlaySubTask2(s_workArea* pWorkArea)
    {
        createSubTask(pWorkArea, &fieldOverlaySubTask2Definition, new s_LCSTask);
    }

    u8 convertCameraPositionTo2dGrid(s_visibilityGridWorkArea* pFieldCameraTask1)
    {
        u8 bDirty = 0;
        s32 X;

        if (pFieldCameraTask1->m28 & 1)
        {
            assert(0);
        }
        else
        {
            X = performDivision(pFieldCameraTask1->m20_cellDimensions[0], pFieldCameraTask1->m0_position[0]);
            if (pFieldCameraTask1->m0_position[0] < 0)
            {
                X--;
            }
        }

        s32 Z = performDivision(pFieldCameraTask1->m20_cellDimensions[1], -pFieldCameraTask1->m0_position[2]);
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

        if (ImGui::Begin("Camera"))
        {
            ImGui::Text("cell: X: %d, Y:%d", pFieldCameraTask1->m18_cameraGridLocation[0], pFieldCameraTask1->m18_cameraGridLocation[1]);
        }
        ImGui::End();

        return bDirty;
    }

    s32 checkPositionVisibility(sVec3_FP* r4, s32 r5)
    {
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
            varC[0] = r5->matrix[2];
            varC[1] = r5->matrix[6];
            varC[2] = r5->matrix[0xA];

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

    fixedPoint fieldCameraTask1InitData1_depthRangeTable[] =
    {
        fixedPoint(0x80000),
        fixedPoint(0x100000),
        fixedPoint(0x200000),
        fixedPoint(0x7FFFFFFF),
    };

    void fieldCameraTask1Init(s_workArea* pWorkArea)
    {
        s_visibilityGridWorkArea* pTypedWorkArea = static_cast<s_visibilityGridWorkArea*>(pWorkArea);
        getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1 = pTypedWorkArea;

        pTypedWorkArea->m12F8_convertCameraPositionToGrid = convertCameraPositionTo2dGrid;
        pTypedWorkArea->m12FC_isObjectClipped = checkPositionVisibility;
        pTypedWorkArea->m12F2_renderMode = 2;

        pTypedWorkArea->m18_cameraGridLocation[0] = -1;
        pTypedWorkArea->m18_cameraGridLocation[1] = -1;
        pTypedWorkArea->m30 = 0;
        pTypedWorkArea->m34_cameraVisibilityTable = 0;
        pTypedWorkArea->m2C_depthRangeTable = fieldCameraTask1InitData1_depthRangeTable;
        pTypedWorkArea->m1300 = 3;
    }

    void fieldCameraTask1Update(s_workArea* pWorkArea)
    {
        s_visibilityGridWorkArea* pTypedWorkArea = static_cast<s_visibilityGridWorkArea*>(pWorkArea);

        pTypedWorkArea->m0_position = cameraProperties2.m0_position;
        pTypedWorkArea->m12F8_convertCameraPositionToGrid(pTypedWorkArea);
    }

    sMatrix4x3* fieldCameraTask1DrawSub1()
    {
        return &getFieldTaskPtr()->m8_pSubFieldData->m334->m384;
    }

    void fieldCameraTask1Draw(s_workArea* pWorkArea)
    {
        s_visibilityGridWorkArea* pTypedWorkArea = static_cast<s_visibilityGridWorkArea*>(pWorkArea);

        sMatrix4x3* r13 = fieldCameraTask1DrawSub1();

        asyncDivStart(graphicEngineStatus.m4070_farClipDistance, fixedPoint(0xC422));

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

        pTypedWorkArea->m12AC[0] = var90.matrix[0 * 4 + 2]; // 8
        pTypedWorkArea->m12AC[1] = var90.matrix[1 * 4 + 2]; // 18
        pTypedWorkArea->m12AC[2] = var90.matrix[2 * 4 + 2]; // 28

        pTypedWorkArea->m12B8[0] = var60.matrix[0 * 4 + 2]; // 8
        pTypedWorkArea->m12B8[1] = var60.matrix[1 * 4 + 2]; // 18
        pTypedWorkArea->m12B8[2] = var60.matrix[2 * 4 + 2]; // 28

        pTypedWorkArea->m12C4[0] = var30.matrix[0 * 4 + 2]; // 8
        pTypedWorkArea->m12C4[1] = var30.matrix[1 * 4 + 2]; // 18
        pTypedWorkArea->m12C4[2] = var30.matrix[2 * 4 + 2]; // 28

        pTypedWorkArea->m12D0[0] = var00.matrix[0 * 4 + 2]; // 8
        pTypedWorkArea->m12D0[1] = var00.matrix[1 * 4 + 2]; // 18
        pTypedWorkArea->m12D0[2] = var00.matrix[2 * 4 + 2]; // 28

        pTypedWorkArea->m12DC = asyncDivEnd();

        if (getFieldTaskPtr()->m8_pSubFieldData->m370_fieldDebuggerWho & 1)
        {
            assert(0);
        }

        pTypedWorkArea->m12E0 = 0;
        pTypedWorkArea->m12E2 = 0;
        pTypedWorkArea->m12E4 = 0;

        pTypedWorkArea->m12F0 = 0;
    }

    s_taskDefinition fieldCameraTask1Definition = { fieldCameraTask1Init, fieldCameraTask1Update, fieldCameraTask1Draw, NULL, "fieldCameraTask1" };

    void LCSTaskInit(p_workArea pWorkArea)
    {
        s_visibilityGridWorkArea* r4 = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;
        r4->m1290_vdp2VramOffset = 0x25E68000;
        setVdp2VramU32(r4->m1290_vdp2VramOffset, 0xFFFFFFFF);

        r4->m128C_vdp2VramOffset2 = 0x25E68800;
        setVdp2VramU32(r4->m128C_vdp2VramOffset2, 0xFFFFFFFF);

        r4->m44 = r4->m68.begin();
        r4->m68[0].m0 = 0;
    }

    void LCSTaskDrawSub1Sub0(s_LCSTask* r4)
    {
        PDS_unimplemented("LCSTaskDrawSub1Sub0");
    }

    void LCSTaskDrawSub1Sub1(s_LCSTask* r4)
    {
        PDS_unimplemented("LCSTaskDrawSub1Sub1");
    }

    void LCSTaskDrawSub1Sub2(s_LCSTask* r4)
    {
        PDS_unimplemented("LCSTaskDrawSub1Sub2");
    }

    void LCSTaskDrawSub1Sub3()
    {
        assert(0);
    }

    void LCSTaskDrawSub1(s_LCSTask* r4)
    {
        LCSTaskDrawSub1Sub0(r4);

        if (getFieldTaskPtr()->m8_pSubFieldData)
        {
            getFieldTaskPtr()->m28_status |= 0x4;
            if (r4->m8 & 0x40)
            {
                return;
            }
        }
        else
        {
            getFieldTaskPtr()->m28_status &= ~0x4;
        }

        LCSTaskDrawSub1Sub1(r4);

        r4->m8 &= ~0x14;
        if (r4->m8 & 1)
        {
            r4->m8 |= 0x20;
        }

        //0606E5E6
        if ((r4->m8 & 0x8B) == 0)
        {
            if (--r4->m83C <= 0)
            {
                r4->m83C = 0;
            }

            if ((r4->m83C == 0) && (r4->m9DA))
            {
                //0606E612
                assert(0);
            }
        }

        //0606E644
        if (r4->m8 & 2)
        {
            assert(0);
        }
        else
        {
            r4->m8 &= ~0x20;
        }

        //606E6AC
        LCSTaskDrawSub1Sub2(r4);

        if (r4->m8 & 0x4)
        {
            LCSTaskDrawSub1Sub3();
        }

        //606E6BC
        if (r4->m8 & 0x80)
        {
            assert(0);
        }

        if (r4->m8 & 0x100)
        {
            assert(0);
        }
    }

    void LCSTaskDrawSubSub(s_LCSTask* r4)
    {
        if (r4->m8 & 2)
        {
            if (pauseEngine[0])
            {
                r4->m8 |= 0x20;
            }
        }

        if (getFieldTaskPtr()->m8_pSubFieldData->m354 > 1)
        {
            LCSTaskDrawSub1(r4);
        }

        for (int r12 = 0; r12 < r4->mC; r12++)
        {
            assert(0);
        }

        if (enableDebugTask)
        {
            assert(0);
        }

        r4->m9C8 = 0;
        r4->m9CC = 0;
        r4->mC = 0;
        r4->m10 = 0;
        r4->m9C4 = 0;
    }

    void LCSTaskDrawSub()
    {
        if (!getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->getTask()->isPaused())
        {
            LCSTaskDrawSubSub(getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS);
        }
    }

    void LCSTaskDraw(p_workArea pWorkArea)
    {
        LCSTaskDrawSub();
    }

    s_taskDefinition LCSTaskDefinition = { LCSTaskInit, NULL, LCSTaskDraw, NULL, "LCSTask" };

    p_workArea createFieldCameraTask(s_workArea* pWorkArea)
    {
        p_workArea pCameraTask = createSubTask(pWorkArea, &fieldCameraTask1Definition, new s_visibilityGridWorkArea);
        createSubTask(pWorkArea, &LCSTaskDefinition, new s_workArea);

        return pCameraTask;
    }

    void randomBattleTaskInit(p_workArea pTypelessWorkArea)
    {
        s_randomBattleWorkArea* pWorkArea = static_cast<s_randomBattleWorkArea*>(pTypelessWorkArea);

        getFieldTaskPtr()->m8_pSubFieldData->m344_randomBattleTask = pWorkArea;
    }

    bool isBattleEnabled()
    {
        // TODO: recheck this
        if (fieldTaskPtr->m28_status)
        {
            return true;
        }
        return false;
    }

    void randomBattleTaskDraw(p_workArea pTypelessWorkArea)
    {
        s_randomBattleWorkArea* pWorkArea = static_cast<s_randomBattleWorkArea*>(pTypelessWorkArea);

        if ((pWorkArea->m4 == 2) || isBattleEnabled() || (pWorkArea->m4 == 0) || ((getFieldTaskPtr()->m28_status & 0xFFFF) == 0))
        {
            hasEncounterData = 1;
        }
        else
        {
            hasEncounterData = 0;
        }
    }

    s_taskDefinition randomBattleTaskDefinition = { randomBattleTaskInit, NULL, randomBattleTaskDraw, NULL, "randomBattleTask" };

    void createRandomBattleTask(s_workArea* pWorkArea)
    {
        createSubTask(pWorkArea, &randomBattleTaskDefinition, new s_randomBattleWorkArea);
    }

    void fieldDebugMenuUpdate1()
    {
        s_FieldSubTaskWorkArea* pSubFieldData = getFieldTaskPtr()->m8_pSubFieldData;
        if ((pSubFieldData->debugMenuStatus1[0] == 0) && (pSubFieldData->m369 == 0))
        {
            if (pSubFieldData->debugMenuStatus3 == 0)
            {
                if (readKeyboardToggle(0x84))
                {
                    pSubFieldData->debugMenuStatus1[1]++;
                    pSubFieldData->debugMenuStatus2_a = 0;
                    clearVdp2TextMemory();
                }
                else
                {
                    if (readKeyboardToggle(0xF6))
                    {
                        pSubFieldData->debugMenuStatus1[1] = 0;
                        clearVdp2TextMemory();
                    }
                }
            }

            switch (pSubFieldData->debugMenuStatus1[1])
            {
            case 0:
                break;
            default:
                assert(0);
                break;
            }

            if (pSubFieldData->debugMenuStatus1[1])
            {
                getFieldTaskPtr()->m28_status |= 8;
            }
            else
            {
                getFieldTaskPtr()->m28_status &= 0xFFFFFFF7;
            }
        }
    }
    void fieldDebugMenuUpdate2()
    {
        s_FieldSubTaskWorkArea* r14 = getFieldTaskPtr()->m8_pSubFieldData;
        if (!r14->m36C)
        {
            if (readKeyboardToggle(0x86))
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

        getFieldTaskPtr()->m8_pSubFieldData->m334->m50D = 1;

        getFieldTaskPtr()->m8_pSubFieldData->pUpdateFunction2 = fieldDebugMenuUpdate2;
        getFieldTaskPtr()->m8_pSubFieldData->pUpdateFunction1 = fieldDebugMenuUpdate1;

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
            FILE* fHandle = fopen("FLD_A3.PRG", "rb");
            assert(fHandle);

            fseek(fHandle, 0, SEEK_END);
            u32 fileSize = ftell(fHandle);

            fseek(fHandle, 0, SEEK_SET);
            u8* fileData = new u8[fileSize];
            fread(fileData, fileSize, 1, fHandle);
            fclose(fHandle);

            gFLD_A3 = new sSaturnMemoryFile();
            gFLD_A3->m_name = "FLD_A3.PRG";
            gFLD_A3->m_data = fileData;
            gFLD_A3->m_dataSize = fileSize;
            gFLD_A3->m_base = 0x6054000;
        }

        ////
        if (!initField(workArea, fieldFileList, arg))
        {
            return NULL;
        }

        if (getFieldTaskPtr()->m2E_currentSubFieldIndex == 2)
        {
            playMusic(3, 0);
        }
        else
        {
            if (getFieldTaskPtr()->m32 == -1)
            {
                playMusic(1, 0);
            }
            else
            {
                playMusic(2, 0);
            }
        }
        //0605404C
        loadFileFromFileList(1);


        graphicEngineStatus.m406C = 0x3000;
        graphicEngineStatus.m408C = FP_Div(0x10000, 0x3000);

        graphicEngineStatus.m4070_farClipDistance = 0x200000;
        graphicEngineStatus.m4094 = FP_Div(0x8000, 0x200000);

        graphicEngineStatus.m4090 = graphicEngineStatus.m4094 << 8;

        getFieldTaskPtr()->m8_pSubFieldData->m334->m50E = 1;
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

//};
