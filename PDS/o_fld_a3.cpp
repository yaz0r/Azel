#include "PDS.h"

namespace FLD_A3_OVERLAY {

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

    void playRiderAnimFans()
    {
        assert(0);
    }

    const u8 FLD_A3_Script_0[] =
    {
        op_START_CUTSCENE(),
        op_WAIT(10),
        op_WAIT(0),
        op_DISPLAY_DIALOG_STRING(FLD_A3_Script_0_String0), //"(Imperial piece of crap.)"
        op_WAIT(45),
        op_CLEAR_DIALOG_STRING(),
        op_END_CUTSCENE(),
        op_END()
    };

    const u8 FLD_A3_Script_1[] =
    {
        op_CALL_NATIVE_0(playRiderAnimFans),
        op_ADD_CINEMATIC_BARS(),
        op_CALL_NATIVE_0(sub_606AD04),
        op_WAIT(100),
        op_PLAY_PCM(0, EV03_1_PCM),
        op_WAIT(0),
        op_DISPLAY_DIALOG_STRING(FLD_A3_Script_1_String0), //"Damn!"
        op_WAIT(50),
        op_CLEAR_DIALOG_STRING(),
        op_WAIT(5),
        op_DISPLAY_DIALOG_STRING(FLD_A3_Script_1_String1), //"A wind net to keep monsters away."
        op_WAIT(80),
        op_CLEAR_DIALOG_STRING(),
        op_WAIT(0),
        op_CALL_NATIVE_1(sub_6067E68, 10),
        op_WAIT(10),
        op_CALL_NATIVE_0(sub_606ACEC),
        op_CALL_NATIVE_0(sub_6057E1C),
        op_END_CUTSCENE(),
        op_CALL_NATIVE_1(sub_6067EC0, 10),
        op_END()
    };
    const void* FLD_A3_Scripts[]
    {
        FLD_A3_Script_0,
        FLD_A3_Script_1,
        (void*)1,
        (void*)1,
        (void*)1,
        (void*)1,
        (void*)1,
        (void*)1,
        (void*)1,
        (void*)1,
        (void*)1,
        (void*)1,
        (void*)1,
        (void*)1,
        (void*)1,
        (void*)1,
        (void*)1,
        (void*)1,
        (void*)1,
        (void*)1,
        (void*)1,
        (void*)1,
        (void*)1,
        (void*)1,
        (void*)1,
        (void*)1,
        (void*)1,
        (void*)1,
    };

    s_grid1* fieldA3_0_dataTable3_grid1[4 * 12] =
    {
        0, (s_grid1*)1, 0, 0,
        0, (s_grid1*)1, 0, 0,
        0, (s_grid1*)1, 0, 0,
        0, (s_grid1*)1, 0, 0,
        0, (s_grid1*)1, 0, 0,
        0, (s_grid1*)1, (s_grid1*)1, 0,
        0, (s_grid1*)1, (s_grid1*)1, 0,
        0, (s_grid1*)1, 0, 0,
        0, (s_grid1*)1, 0, (s_grid1*)1,
        (s_grid1*)1, (s_grid1*)1, (s_grid1*)1, (s_grid1*)1,
        (s_grid1*)1, (s_grid1*)1, 0, 0,
        (s_grid1*)1, 0, 0, 0,
    };

    s_grid2* fieldA3_0_dataTable3_grid2[4 * 12] =
    {
        0, (s_grid2*)1, 0, 0,
        0, (s_grid2*)1, 0, 0,
        0, (s_grid2*)1, 0, 0,
        0, (s_grid2*)1, 0, 0,
        0, 0, 0, 0,
        0, (s_grid2*)1, 0, 0,
        0, (s_grid2*)1, (s_grid2*)1, 0,
        0, (s_grid2*)1, 0, 0,
        0, (s_grid2*)1, 0, 0,
        0, (s_grid2*)1, 0, 0,
        (s_grid2*)1, 0, 0, 0,
        (s_grid2*)1, 0, 0, 0,
    };

    s_DataTable3 fieldA3_0_dataTable3 =
    {
        fieldA3_0_dataTable3_grid1,
        fieldA3_0_dataTable3_grid2,
        NULL,
        2,
    { 4, 12 }, // grid size
    0x100000,
    0x100000,
    0,
    /*
    {
    0x1002AC,
    0x1402B0,
    0x1802B4,
    0x2402C0,
    0x5402F0,
    0x640300,
    0x4402E0,
    0x1C02B8,
    0x740310,
    0x840320,
    }
    */
    };
    u8 fieldA3_0_dataTable2[] = { 1 };

    sCameraVisibility fieldA3_0_dataTable1_0[] =
    {
        { -1, 0 },
    };

    sCameraVisibility fieldA3_0_dataTable1_1[] =
    {
        { 1, 1 },
    { 1, 0 },
    { -1, 0 },
    };

    sCameraVisibility* fieldA3_0_dataTable1[] = {
        fieldA3_0_dataTable1_0,
        fieldA3_0_dataTable1_1
    };

    void fieldA3_0_startTasks(p_workArea workArea)
    {
        unimplemented("fieldA3_0_startTasks");
    }

    void fieldGridTask_Update(p_workArea)
    {

    }

    void setupGridCell(s_visibilityGridWorkArea* r4, s_visdibilityCellTask* r5, int index)
    {
        getMemoryArea(&r5->m0_memoryLayout, r4->field_30->mC);
        r5->index = index;
        if (r4->field_30->m0_environmentGrid)
        {
            r5->m8_pEnvironmentCell = r4->field_30->m0_environmentGrid[index];
        }
        if (r4->field_30->m4)
        {
            r5->pCell2 = r4->field_30->m4[index];
        }
        if (r4->field_30->m8)
        {
            r5->pCell3 = r4->field_30->m8[index];
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

        s32 r15 = graphicEngineStatus.field_4070;

        if (pTypedWorkAread->m8_pEnvironmentCell)
        {
            s_grid1* r14 = pTypedWorkAread->m8_pEnvironmentCell;
            while (r14->m0.m_offset)
            {
                r13->m12E0++;

                s16 r2 = readSaturnS16(r14->m0);
                u32 r1 = READ_BE_U32(pTypedWorkAread->m0_memoryLayout.mainMemory + r2);
                r13->m12F0 += READ_BE_U32(pTypedWorkAread->m0_memoryLayout.mainMemory + r1 + 4);

                if (r13->field_12FC(&r14->m4, r15))
                {
                    u32 var_54 = 0;
                    r13->m12E2++;

                    if (readSaturnS16(r14->m0 + 8))
                    {
                        u32 offset = READ_BE_U32(pTypedWorkAread->m0_memoryLayout.mainMemory + readSaturnS16(r14->m0 + 8));
                        unimplemented("Disabled LCS generation because of crash");
                        //var_54 = gridCellDraw_normalSub0(pTypedWorkAread->m0_memoryLayout.mainMemory + offset, r14->m4);
                    }

                    pushCurrentMatrix();
                    translateCurrentMatrix(&r14->m4);
                    rotateCurrentMatrixZ(r14->m10[2]);
                    rotateCurrentMatrixY(r14->m10[1]);
                    rotateCurrentMatrixX(r14->m10[0]);

                    u32 depthRangeIndex = gridCellDraw_GetDepthRange(pCurrentMatrix->matrix[11]);

                    if (readSaturnS16(r14->m0 + depthRangeIndex * 2))
                    {
                        u32 offset = READ_BE_U32(pTypedWorkAread->m0_memoryLayout.mainMemory + readSaturnS16(r14->m0 + depthRangeIndex * 2));
                        addObjectToDrawList(pTypedWorkAread->m0_memoryLayout.mainMemory, offset);
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

        if (pTypedWorkAread->pCell2)
        {
            //assert(0);
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

            pNewTask->getTask()->m_pDraw = gridCellDrawFunctions[r4->m12F2_renderMode];
        }

        return pNewTask;
    }

    void enableCellsBasedOnVisibilityList(s_visibilityGridWorkArea* r14, sCameraVisibility* r5)
    {
        // TODO: Original was different, but I think it was buggy
        while (r5->field_0 >= 0)
        {
            r14->m3C_cellRenderingTasks[(r14->field_30->m10_gridSize[0] * r5->field_1) + r5->field_0]->getTask()->clearPaused();
            r5++;
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
        if (r14->field_30 == NULL)
            return;

        if (r14->m3C_cellRenderingTasks == NULL)
            return;

        if (r14->m34_cameraVisibilityTable &&
            (r14->m18_cameraGridLocation[0] >= 0) &&
            (r14->m18_cameraGridLocation[0] < r14->field_30->m10_gridSize[0]) &&
            (r14->m18_cameraGridLocation[1] >= 0) &&
            (r14->m18_cameraGridLocation[1] < r14->field_30->m10_gridSize[1]) &&
            (r14->m34_cameraVisibilityTable[r14->m18_cameraGridLocation[0] + r14->m18_cameraGridLocation[1] * r14->field_30->m10_gridSize[0]]))
        {
            return enableCellsBasedOnVisibilityList(r14, r14->m34_cameraVisibilityTable[r14->m18_cameraGridLocation[0] + r14->m18_cameraGridLocation[1] * r14->field_30->m10_gridSize[0]]);
        }

        for (int i = 0; i < 9; i++)
        {
            s32 r5 = r14->m18_cameraGridLocation[0] + cellsToVisitForVisibility[i][0];
            s32 r6 = r14->m18_cameraGridLocation[1] + cellsToVisitForVisibility[i][1];

            if ((r5 >= 0) && (r5 < r14->field_30->m10_gridSize[0]) && (r6 >= 0) && (r6 < r14->field_30->m10_gridSize[1]))
            {
                r14->m3C_cellRenderingTasks[r5 + r6 * r14->field_30->m10_gridSize[0]]->getTask()->clearPaused();
            }
        }
    }

    void pauseAllCells(s_visibilityGridWorkArea* pFieldCameraTask1)
    {
        if (pFieldCameraTask1->field_30 == NULL)
            return;

        if (pFieldCameraTask1->m3C_cellRenderingTasks == NULL)
            return;

        for (int i = 0; i < pFieldCameraTask1->field_30->m10_gridSize[0] * pFieldCameraTask1->field_30->m10_gridSize[1]; i++)
        {
            pFieldCameraTask1->m3C_cellRenderingTasks[i]->getTask()->markPaused();
        }
    }

    void updateCellGridIfDirty(s_visibilityGridWorkArea* pFieldCameraTask1)
    {
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
        if (pFieldCameraTask1->field_38)
        {
            assert(0);
        }

        pFieldCameraTask1->field_38 = createSubTaskFromFunction(pFieldCameraTask1, NULL, new s_dummyWorkArea, "Unnamed");

        if (r4)
        {
            pFieldCameraTask1->field_30 = r4;
            pFieldCameraTask1->field_20 = r4->m18 * 2;
            pFieldCameraTask1->field_24 = r4->m1C * 2;
            pFieldCameraTask1->field_28 = r4->m20;
            loadFileFromFileList(r4->mC);

            if (pFieldCameraTask1->field_28 & 1)
            {
                //06070FE0
                assert(0);
            }
            else
            {
                //0607100A
                {
                    s32 r3 = r4->m10_gridSize[0] * pFieldCameraTask1->field_20;
                    if (r3 < 0)
                    {
                        r3++;
                    }
                    r3 /= 2;
                    pFieldCameraTask1->mC[0] = r3 / 2;

                }
                pFieldCameraTask1->mC[1] = 0;
                {
                    s32 r3 = -(r4->m10_gridSize[1] * pFieldCameraTask1->field_24);
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

            s32* var_1C = pFieldCameraTask1->field_30->m10_gridSize;
            int gridSize = pFieldCameraTask1->field_30->m10_gridSize[0] * pFieldCameraTask1->field_30->m10_gridSize[1];

            pFieldCameraTask1->m3C_cellRenderingTasks = (s_visdibilityCellTask**)allocateHeapForTask(pFieldCameraTask1->field_38, gridSize * sizeof(s_visdibilityCellTask*));

            for (int cellIndex = 0; cellIndex < gridSize; cellIndex++)
            {
                pFieldCameraTask1->m3C_cellRenderingTasks[cellIndex] = createGridCellTask(pFieldCameraTask1, pFieldCameraTask1->field_38, cellIndex);
                pFieldCameraTask1->m3C_cellRenderingTasks[cellIndex]->getTask()->markPaused();
            }

            //060710C4
            enableCellsAroundCamera(pFieldCameraTask1);
            pFieldCameraTask1->getTask()->m_pUpdate = updateCellGridFromCameraPosition;
        }
        else
        {
            pFieldCameraTask1->field_20 = graphicEngineStatus.field_4070;
            pFieldCameraTask1->field_24 = graphicEngineStatus.field_4070;
        }

        if (r5)
        {
            r5(pFieldCameraTask1->field_38);
        }
    }

    void setupFieldSub1(s_DataTable3* r4, u8* r5, void(*r6)(p_workArea workArea))
    {
        s_visibilityGridWorkArea* pFieldCameraTask1 = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;

        setupField2(r4, r6);

        if (r5)
        {
            unimplemented("setupFieldSub1");
        }
    }

    void setupField(s_DataTable3* r4, u8* r5, void(*r6)(p_workArea workArea), sCameraVisibility** r7)
    {
        getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1->m34_cameraVisibilityTable = r7;

        setupFieldSub1(r4, r5, r6);
    }

    void subfieldA3_0Sub0(s_dragonTaskWorkArea* r4)
    {
        unimplemented("subfieldA3_0Sub0");
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

    void startFieldScript(s32 r4, s32 r5)
    {
        unimplemented("startFieldScript");
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
        pNewData3->m18 = readSaturnU32(EA); EA = EA + 4;
        pNewData3->m1C = readSaturnU32(EA); EA = EA + 4;
        pNewData3->m20 = readSaturnU32(EA); EA = EA + 4;

        pNewData3->m0_environmentGrid = readEnvironmentGrid(grid1, pNewData3->m10_gridSize[0], pNewData3->m10_gridSize[1]);
        pNewData3->m4 = readGrid2(grid2, pNewData3->m10_gridSize[0], pNewData3->m10_gridSize[1]);
        pNewData3->m8 = readGrid3(grid3, pNewData3->m10_gridSize[0], pNewData3->m10_gridSize[1]);

        return pNewData3;
    }

    void subfieldA3_0(p_workArea workArea)
    {
        s16 r13 = getFieldTaskPtr()->field_30;

        playPCM(workArea, 100);
        playPCM(workArea, 101);

        setupField(readDataTable3({ 0x6085AA4, gFLD_A3 }), fieldA3_0_dataTable2, fieldA3_0_startTasks, fieldA3_0_dataTable1);

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

                graphicEngineStatus.field_4070 = 0x2AE000;
                graphicEngineStatus.field_4094 = FP_Div(0x8000, 0x2AE000);

                graphicEngineStatus.field_4090 = graphicEngineStatus.field_4094 << 8;
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
        unimplemented("subfieldA3_0");
    }
    void subfieldA3_1(p_workArea workArea) { unimplemented("subfieldA3_1"); }
    void subfieldA3_2(p_workArea workArea) { unimplemented("subfieldA3_2"); }
    void subfieldA3_3(p_workArea workArea) { unimplemented("subfieldA3_3"); }
    void subfieldA3_4(p_workArea workArea) { unimplemented("subfieldA3_4"); }
    void subfieldA3_5(p_workArea workArea) { unimplemented("subfieldA3_5"); }
    void subfieldA3_6(p_workArea workArea) { unimplemented("subfieldA3_6"); }
    void subfieldA3_7(p_workArea workArea) { unimplemented("subfieldA3_7"); }
    void subfieldA3_8(p_workArea workArea) { unimplemented("subfieldA3_8"); }
    void subfieldA3_9(p_workArea workArea) { unimplemented("subfieldA3_9"); }
    void subfieldA3_A(p_workArea workArea) { unimplemented("subfieldA3_A"); }
    void subfieldA3_B(p_workArea workArea) { unimplemented("subfieldA3_B"); }
    void subfieldA3_C(p_workArea workArea) { unimplemented("subfieldA3_C"); }

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

    void initScriptTable1(s_scriptData1* pData1)
    {
        for (int i = 0; i < 0x11; i++)
        {
            pData1[i].field_0 = 0;
            pData1[i].field_4 = 0;
            pData1[i].field_8 = 0;
            pData1[i].field_C = 0;
            pData1[i].field_10 = 0;
            pData1[i].field_14 = 0;
            pData1[i].field_18 = 0;
            pData1[i].field_1C = 0;
            pData1[i].field_20 = 0;
            pData1[i].field_24 = 0;
            pData1[i].field_28 = 0;
            pData1[i].field_2C = 0;
            pData1[i].field_30 = 0;
            pData1[i].field_34 = 0;
            pData1[i].field_38 = 0;
        }
    }

    void initScriptTable2(s_scriptData2* pData2)
    {
        for (int i = 0; i < 0x11; i++)
        {
            pData2[i].field_0 = 0;
            pData2[i].field_4 = 0;
            pData2[i].field_8 = 0;
            pData2[i].field_C = 0;
            pData2[i].field_10 = 0xA000;
            pData2[i].field_14 = 0;
            pData2[i].field_18 = 0;
            pData2[i].field_1C = 0;
            pData2[i].field_20 = 0xA000;
        }
    }

    void initScriptTable3(s_scriptData3* pData3)
    {
        for (int i = 0; i < 0x11; i++)
        {
            pData3[i].field_0 = 0;
            pData3[i].field_4 = 0;
            pData3[i].field_8 = 0;
            pData3[i].field_C = 0;
            pData3[i].field_10 = 0;
            pData3[i].field_14 = 0;
            pData3[i].field_18 = 0;
            pData3[i].field_1C = 0;
        }
    }

    void initScriptTable4(s_scriptData4* pData4)
    {
        for (int i = 0; i < 0x11; i++)
        {
            pData4[i].field_0 = -1;
            pData4[i].field_1 = 0;
            pData4[i].field_2 = 0;
        }
    }

    void fieldScriptTaskInit(s_workArea* pWorkArea)
    {
        s_fieldScriptWorkArea* pFieldScriptWorkArea = static_cast<s_fieldScriptWorkArea*>(pWorkArea);

        s_fieldTaskWorkArea* pFieldTaskWorkArea = getFieldTaskPtr();

        pFieldTaskWorkArea->m8_pSubFieldData->m34C_ptrToE = pFieldScriptWorkArea;

        if (pFieldTaskWorkArea->field_40 == NULL)
        {
            pFieldTaskWorkArea->field_40 = (s_scriptData1*)allocateHeapForTask(pWorkArea, 0x11 * sizeof(s_scriptData1));
            initScriptTable1(pFieldTaskWorkArea->field_40);
        }

        if (pFieldTaskWorkArea->field_44 == NULL)
        {
            pFieldTaskWorkArea->field_44 = (s_scriptData2*)allocateHeapForTask(pWorkArea, 0x11 * sizeof(s_scriptData2));
            initScriptTable2(pFieldTaskWorkArea->field_44);
        }

        if (pFieldTaskWorkArea->field_48 == NULL)
        {
            pFieldTaskWorkArea->field_48 = (s_scriptData3*)allocateHeapForTask(pWorkArea, 0x11 * sizeof(s_scriptData3));
            initScriptTable3(pFieldTaskWorkArea->field_48);
        }

        if (pFieldTaskWorkArea->field_4C == NULL)
        {
            pFieldTaskWorkArea->field_4C = (s_scriptData4*)allocateHeapForTask(pWorkArea, 0x11 * sizeof(s_scriptData4));
            initScriptTable4(pFieldTaskWorkArea->field_4C);

        }

        pFieldScriptWorkArea->field_8 = &pFieldScriptWorkArea->field_2C;

        pFieldScriptWorkArea->field_88 = pFieldTaskWorkArea->field_40;
        pFieldScriptWorkArea->field_8C = pFieldTaskWorkArea->field_44;
        pFieldScriptWorkArea->field_90 = pFieldTaskWorkArea->field_48;
        pFieldScriptWorkArea->field_94 = pFieldTaskWorkArea->field_4C;

        pFieldScriptWorkArea->field_6C = 1000;
        pFieldScriptWorkArea->field_70 = 1;
    }

    void fieldScriptTaskUpdateSub1()
    {
        unimplemented("fieldScriptTaskUpdateSub1");
    }

    void fieldScriptTaskUpdate(s_workArea* pWorkArea)
    {
        fieldScriptTaskUpdateSub1();

        unimplemented("fieldScriptTaskUpdate");
    }

    s_taskDefinition fieldScriptTaskDefinition = { fieldScriptTaskInit, fieldScriptTaskUpdate, NULL, NULL, "fieldScriptTask" };

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

    fixedPoint integrateDragonMovementSub5(fixedPoint r11, fixedPoint r12, fixedPoint stack0, fixedPoint r10, s32 r14)
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

    fixedPoint dragonFieldTaskInitSub4Sub4Sub1Sub1Sub1(fixedPoint r10, fixedPoint r12, fixedPoint stack0, fixedPoint r11, s32 r13)
    {
        fixedPoint r14 = r12 - r10;
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

            if (r13 > 0)
            {
                return r12;
            }
            else
            {
                return r10 + r4;
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
            else if (r4 < r13)
            {
                r5 = r4;
            }
            else
            {
                r5 = r13;
            }

            r4 = r14 - r5;
            fixedPoint r3 = r5.normalized();

            if (r3 > 0)
            {
                return r12;
            }
            else
            {
                return r10 + r5;
            }
        }
    }

    void fieldOverlaySubTaskInitSub2Sub1Sub2(sFieldCameraStatus* r14, s_dragonTaskWorkArea* r9)
    {
        r14->m5C[0] = dragonFieldTaskInitSub4Sub4Sub1Sub1Sub1(r14->m5C[0], 0, 0x2000, 0x444444, 0);
        r14->m5C[1] = dragonFieldTaskInitSub4Sub4Sub1Sub1Sub1(r14->m5C[1], 0, 0x2000, 0x444444, 0);
        r14->m5C[2] = dragonFieldTaskInitSub4Sub4Sub1Sub1Sub1(r14->m5C[2], 0, 0x2000, 0x444444, 0);

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

        r14->m44[0] = integrateDragonMovementSub5(r14->m44[0], 0, 0x2000, 0xAAA, 0);
        r14->m44[1] = integrateDragonMovementSub5(r14->m44[1], 0, 0x2000, 0xAAA, 0);
        r14->m44[2] = integrateDragonMovementSub5(r14->m44[2], 0, 0x2000, 0xAAA, 0);

        r14->m44 += r14->m50;

        r14->m0_position += r14->m44;
        r14->m50.zero();

        unimplemented("fieldOverlaySubTaskInitSub2Sub1Sub2");
    }

    void fieldOverlaySubTaskInitSub2Sub1(sFieldCameraStatus* r14, s_dragonTaskWorkArea* r12)
    {
        u32 stack_0[2];
        sVec3_FP stack_8;

        stack_8[0] = -r12->m88_matrix.matrix[2];
        stack_8[1] = -r12->m88_matrix.matrix[6];
        stack_8[2] = -r12->m88_matrix.matrix[10];

        generateCameraMatrixSub1(stack_8, stack_0);

        fieldOverlaySubTaskInitSub2Sub1Sub1(r14, r12);

        if (r14->m7C & 1)
        {
            r14->mC_rotation[0] = dragonFieldTaskInitSub4Sub4Sub1Sub1Sub1(r14->mC_rotation[0], r14->m28 - r12->m20_angle[0], 0x2000, 0x111111, 0);
        }
        else
        {
            r14->mC_rotation[0] = dragonFieldTaskInitSub4Sub4Sub1Sub1Sub1(r14->mC_rotation[0], r14->m28, 0x2000, 0x111111, 0);
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
            r14->mC_rotation[1] = dragonFieldTaskInitSub4Sub4Sub1Sub1Sub1(r14->mC_rotation[1], stack_0[1] , 0x2000, 0x222222, 0);
        }

        if ((r14->m7C & 4) == 0)
        {
            r14->m30 = 0;
        }

        r14->mC_rotation[2] = dragonFieldTaskInitSub4Sub4Sub1Sub1Sub1(r14->mC_rotation[2], r14->m30, 0x2000, 0x222222, 0);

        r14->m24_distanceToDestination = integrateDragonMovementSub5(r14->m24_distanceToDestination, r14->m40, 0x2000, 0xAAA, 0);

        fieldOverlaySubTaskInitSub2Sub1Sub2(r14, r12);
    }

    void fieldOverlaySubTaskInitSub2(sFieldCameraStatus* r14)
    {
        s_dragonTaskWorkArea* pDragonTask = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
        if (pDragonTask == NULL)
            return;

        switch (r14->m8D)
        {
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
        sFieldCameraStatus* pFieldCameraStatus = &getFieldTaskPtr()->m8_pSubFieldData->m334->field_3E4[r4];
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
            p334->field_3E4[r4].m80 = 0;
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
        p334->field_3E4[r4].m74 = 0;
        p334->field_3E4[r4].m78 = 0;
        p334->field_3E4[r4].m8C = 0;
    }

    void updateCameraFromDragonSub2(s_fieldOverlaySubTaskWorkArea* pTypedWorkArea)
    {
        assert(pTypedWorkArea->m50C == 0);
        sFieldCameraStatus* r13 = &pTypedWorkArea->field_3E4[pTypedWorkArea->m50C];
        s16 r15[3];
        r15[0] = r13->mC_rotation[0] >> 16;
        r15[1] = r13->mC_rotation[1] >> 16;
        r15[2] = r13->mC_rotation[2] >> 16;

        updateEngineCamera(&cameraProperties2, r13, r15);

        copyMatrix(pCurrentMatrix, &pTypedWorkArea->field_384);
        copyMatrix(&cameraProperties2.m28[0], &pTypedWorkArea->field_3B4);
    }

    s_fieldCameraConfig unk_6092EF0 = {
        0,0,0,
        0,0,0,
    { -0xE38E38, 0, 0, 0, 0, 0, 0xA000 },
    { -0xE38E38, 0, 0, 0, 0, 0, 0xA000 },
    0,
    0x80000,
    };

    void fieldOverlaySubTaskInit(s_workArea* pWorkArea)
    {
        s_fieldOverlaySubTaskWorkArea* pTypedWorkArea = static_cast<s_fieldOverlaySubTaskWorkArea*>(pWorkArea);

        getFieldTaskPtr()->m8_pSubFieldData->m334 = pTypedWorkArea;

        fieldOverlaySubTaskInitSub1(0, &fieldOverlaySubTaskInitSub2, 0);
        fieldOverlaySubTaskInitSub3(0);
        setupFieldCameraConfigs(&unk_6092EF0, 1);

        getFieldTaskPtr()->m8_pSubFieldData->m334->field_50E = 1;

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
            if (u32 offset = READ_BE_U32(pRiderState->m0_riderModel + 0x30))
            {
                pData = pRiderState->m0_riderModel + offset;
            }
            riderInit(&pRiderState->m18_3dModel, pData);
            updateAndInterpolateAnimation(&pRiderState->m18_3dModel);
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
        unimplemented("dragonRidersTaskUpdate");
    }

    s_taskDefinition dragonRidersTaskDefinition = { dragonRidersTaskInit, dragonRidersTaskUpdate, NULL, NULL, "dragonRidersTask" };

    void dragonFieldSubTask2Init(s_workArea* pWorkArea)
    {
        unimplemented("dragonFieldSubTask2Init");
    }

    void dragonFieldSubTask2Update(s_workArea* pWorkArea)
    {
        unimplemented("dragonFieldSubTask2Update");
    }

    void dragonFieldSubTask2Draw(s_workArea* pWorkArea)
    {
        unimplemented("dragonFieldSubTask2Draw");
    }


    s_taskDefinition dragonFieldSubTask2Definition = { dragonFieldSubTask2Init, dragonFieldSubTask2Update, dragonFieldSubTask2Draw, NULL, "dragonFieldSubTask2" };

    void initDragonFieldSubTask2(s_workArea* pWorkArea)
    {
        createSubTask(pWorkArea, &dragonFieldSubTask2Definition, new s_dummyWorkArea);
    }

    void dragonFieldTaskInitSub2Sub2(fixedPoint* field_178)
    {
        field_178[0] = 0x222222;
        field_178[1] = 0x4CCC;
        field_178[2] = 0x16;
        field_178[3] = 0x111111;
    }

    void dragonFieldTaskInitSub2Sub3(s_dragonTaskWorkArea* pWorkArea)
    {
        pWorkArea->field_1F0.m_0 = 0;
        pWorkArea->field_1F0.m_4 = 0;
        pWorkArea->field_1F0.m_8 = 0;
        pWorkArea->field_1F0.m_C = 0;
        pWorkArea->field_1F0.m_E = 0;
        pWorkArea->field_1F0.m_10 = 0;
    }

    void dragonFieldTaskInitSub2Sub4(s_dragonTaskWorkArea_48* field_48)
    {
        initMatrixToIdentity(&field_48->m0_matrix);

        field_48->field_30 = 0;
        field_48->field_34 = 0;
        field_48->field_38 = 0;

        field_48->field_3C = 1;
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
                pDragonTask->field_237 = 4;
                pDragonTask->field_238 = 4;
            }
            else
            {
                pDragonTask->field_237 = 0;
                pDragonTask->field_238 = 0;
            }
        }
    }

    sFieldCameraStatus* getFieldCameraStatus()
    {
        return &getFieldTaskPtr()->m8_pSubFieldData->m334->field_3E4[getFieldTaskPtr()->m8_pSubFieldData->m334->m50C];
    }

    void dragonFieldTaskInitSub2(s_dragonTaskWorkArea* pWorkArea)
    {
        initDragonFieldSubTask2(pWorkArea);

        dragonFieldTaskInitSub2Sub2(pWorkArea->field_178);

        dragonFieldTaskInitSub2Sub3(pWorkArea);

        pWorkArea->field_208 = 0x960000;
        pWorkArea->field_20C = 0x960000;
        pWorkArea->field_210 = 0xB333;
        pWorkArea->field_214 = 0xB333;
        pWorkArea->field_150 = 0x10000;

        pWorkArea->m8_pos[0] = 0;
        pWorkArea->m8_pos[1] = 0x1E000;
        pWorkArea->m8_pos[2] = 0;

        pWorkArea->m20_angle[0] = 0;
        pWorkArea->m20_angle[1] = 0;
        pWorkArea->m20_angle[2] = 0;

        pWorkArea->m154_dragonSpeed = 0;

        pWorkArea->field_1B8 = 0xB333;
        pWorkArea->field_1BC = 0x200000;

        initMatrixToIdentity(&pWorkArea->m88_matrix);

        dragonFieldTaskInitSub2Sub4(&pWorkArea->m48);

        pWorkArea->field_1CC = 0x38E38E3; // field of view
        pWorkArea->field_234 = 0;

        pWorkArea->m21C_DragonSpeedValues[0] = 0;
        pWorkArea->m21C_DragonSpeedValues[1] = 0x1284;
        pWorkArea->m21C_DragonSpeedValues[2] = 0x2509;
        pWorkArea->m21C_DragonSpeedValues[3] = 0x3B42;
        pWorkArea->m21C_DragonSpeedValues[4] = 0x58E3;

        initDragonSpeed(0);

        pWorkArea->field_230 = 0x1999;

        //060738C0

        pWorkArea->m154_dragonSpeed = pWorkArea->m21C_DragonSpeedValues[pWorkArea->m235_dragonSpeedIndex];

        pWorkArea->field_238 = 0;
        pWorkArea->field_237 = 0;
        pWorkArea->field_C0 = 0xC000000;
        pWorkArea->field_C4 = 0;

        pWorkArea->field_C8[0] = 0x10;
        pWorkArea->field_C8[1] = 0x10;
        pWorkArea->field_C8[2] = 0x10;

        pWorkArea->field_CB[0] = 0x8;
        pWorkArea->field_CB[1] = 0x8;
        pWorkArea->field_CB[2] = 0x8;

        pWorkArea->field_CE[0] = 0x14;
        pWorkArea->field_CE[1] = 0x14;
        pWorkArea->field_CE[2] = 0x14;

        pWorkArea->field_D1[0] = 0xC;
        pWorkArea->field_D1[1] = 0xC;
        pWorkArea->field_D1[2] = 0xC;

        pWorkArea->field_D4[0] = 0x10;
        pWorkArea->field_D4[1] = 0x10;
        pWorkArea->field_D4[2] = 0x10;

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
        pWorkArea->field_237 = pWorkArea->field_238;
        pWorkArea->field_23B = 1;
    }

    void dragonFieldTaskInitSub4Sub3(u8 r4)
    {
        unimplemented("dragonFieldTaskInitSub4Sub3");
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
        unimplemented("startScriptLeaveArea");
    }

    void dragonFieldTaskInitSub4Sub5(s_dragonTaskWorkArea_48* r14, sVec3_FP* r13)
    {
        r14->field_30 = r13->m_value[0];
        r14->field_34 = r13->m_value[1];
        r14->field_38 = r13->m_value[2];
        r14->field_3C = 1;

        initMatrixToIdentity(&r14->m0_matrix);
        rotateMatrixShiftedY(r13->m_value[1], &r14->m0_matrix);
        rotateMatrixShiftedX(r13->m_value[0], &r14->m0_matrix);
        rotateMatrixShiftedZ(r13->m_value[2], &r14->m0_matrix);
    }

    void computeDragonDeltaTranslation(s_dragonTaskWorkArea* r14)
    {
        unimplemented("updateDragonRotationSub1");
    }

    void dragonFieldTaskInitSub4Sub4Sub2()
    {
        getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->mF8_Flags &= 0xFFFEFFFF;
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

    u32 updateDragonMovementFromControllerType1Sub1(s_dragonTaskWorkArea* r14)
    {
        unimplemented("updateDragonMovementFromControllerType1Sub1");
        return 1;
    }

    void updateDragonMovementFromControllerType1Sub2(s_dragonTaskWorkArea* r14, s_dragonState* r12)
    {
        if (graphicEngineStatus.m4514.m0[0].m0_current.field_8 & graphicEngineStatus.m4514.mD8[1][5]) // down
        {
            updateDragonMovementFromControllerType1Sub2Sub1(&r12->m78_animData, r14->field_178[3]);
        }
        else if (graphicEngineStatus.m4514.m0[0].m0_current.field_8 & graphicEngineStatus.m4514.mD8[1][4]) // up
        {
            updateDragonMovementFromControllerType1Sub2Sub1(&r12->m78_animData, -r14->field_178[3]);
        }

        if (graphicEngineStatus.m4514.m0[0].m0_current.field_8 & graphicEngineStatus.m4514.mD8[1][7]) // right
        {
            updateDragonMovementFromControllerType1Sub2Sub2(&r12->m78_animData, r14->field_178[3]);
            updateDragonMovementFromControllerType1Sub2Sub3(&r12->m78_animData, -r14->field_178[3]);
        }
        else if (graphicEngineStatus.m4514.m0[0].m0_current.field_8 & graphicEngineStatus.m4514.mD8[1][6]) // left
        {
            updateDragonMovementFromControllerType1Sub2Sub2(&r12->m78_animData, -r14->field_178[3]);
            updateDragonMovementFromControllerType1Sub2Sub3(&r12->m78_animData, r14->field_178[3]);
        }
    }

    void updateDragonMovementFromControllerType1Sub3(s_dragonTaskWorkArea* r14)
    {
        unimplemented("updateDragonMovementFromControllerType1Sub3");
        updateDragonMovementFromControllerType0(r14);
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

        if (r14->m4)
        {
            return 1;
        }

        if (r14->m30)
        {
            return 1;
        }

        if (r14->m34)
        {
            return 1;
        }

        if (r14->m38)
        {
            return 1;
        }

        if (r14->m3C)
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

        if (r14->m4)
        {
            return 0;
        }

        if (r14->m30)
        {
            return 0;
        }

        if (r14->m34)
        {
            return 0;
        }

        if (r14->m38)
        {
            return 0;
        }

        if (r14->m3C)
        {
            return 0;
        }

        return 1;
    }

    void clearDragonPlayerInputs()
    {
        unimplemented("integrateDragonMovementSub2");
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
        if (r4->m25C & 1)
        {
            r4->m25C = 0;
            r4->m25B = 0;
            r4->m20_angle[2] = 0;
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
            if (graphicEngineStatus.m4514.m0[0].m0_current.field_6 & graphicEngineStatus.m4514.mD8[1][11])
            {
                if (graphicEngineStatus.m4514.m0[0].m0_current.field_6 & graphicEngineStatus.m4514.mD8[1][0])
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

        if (graphicEngineStatus.m4514.m0[0].m0_current.field_6 & graphicEngineStatus.m4514.mD8[1][0])
        {
            //0607E960
            if (++r14->field_234 > 4)
            {
                if (++r14->m235_dragonSpeedIndex >= r5)
                {
                    r14->m235_dragonSpeedIndex = r5;
                }
                r14->field_234 = 0;
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
                r14->m15C_dragonSpeedIncrement = MTH_Mul( -r14->m154_dragonSpeed - 0x1284, r14->field_230);
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
                    r14->m15C_dragonSpeedIncrement = MTH_Mul(-r12, r14->field_230);
                }
                else
                {
                    r14->m15C_dragonSpeedIncrement = -MTH_Mul(r12, r14->field_230);
                }
            }
        }
        else
        {
            //607FE98
            assert(0);
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

        r14->m1AC[0] = dragonFieldTaskInitSub4Sub4Sub1Sub1Sub1(r14->m1AC[0], 0, 0x2000, 0x444444, 0x10);
        r14->m1AC[1] = dragonFieldTaskInitSub4Sub4Sub1Sub1Sub1(r14->m1AC[1], 0, 0x2000, 0x444444, 0x10);
        r14->m1AC[2] = dragonFieldTaskInitSub4Sub4Sub1Sub1Sub1(r14->m1AC[2], 0, 0x2000, 0x444444, 0x10);

        r14->m1AC += r14->m1A0;

        r14->m20_angle += r14->m1AC;

        r14->m1A0.zero();

        // ~0607FFEC 
        r14->m194[0] = integrateDragonMovementSub5(r14->m194[0], 0, 0x2000, 0xAAA, 0x10);
        r14->m194[1] = integrateDragonMovementSub5(r14->m194[1], 0, 0x2000, 0xAAA, 0x10);
        r14->m194[2] = integrateDragonMovementSub5(r14->m194[2], 0, 0x2000, 0xAAA, 0x10);

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
            getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m8 &= 0xFFFFFFFE;
            r4->mF8_Flags |= 0x400;
            r4->m104_dragonScriptStatus++;
        case 1:
            if (!isDragonInValidArea(r4))
            {
                r4->field_F0 = dragonLeaveArea;
                r4->m104_dragonScriptStatus = 0;
                dragonLeaveArea(r4);
                return;
            }
        }

        r4->m160_deltaTranslation[0] = 0;
        r4->m160_deltaTranslation[1] = 0;
        r4->m160_deltaTranslation[2] = 0;

        r4->field_238 &= 0xFFFFFFFC;

        unimplemented("Hacking input to type 1");
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
            pDragonTask->field_F0 = updateDragonMovement;
        }

        pDragonTask->m104_dragonScriptStatus = 0;

        dragonFieldTaskInitSub4Sub4Sub2();
    }

    void dragonLeaveArea(s_dragonTaskWorkArea* r14)
    {
        s_visibilityGridWorkArea* r12 = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;
        //r13 = r4->field_178

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
            if (getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m3C == 0)
            {
                dragonFieldTaskInitSub4Sub4();
                r14->field_F0(r14);
                getFieldTaskPtr()->m28_status &= 0xFFFFFDFF;
                return;
            }
            break;
        default:
            assert(0);
            break;
        }

        //607FA40
        r14->field_238 &= 0xFFFFFFFC;
        sVec3_FP var_8;
        var_8[0] = r12->mC[0] - r14->m8_pos[0];
        var_8[1] = 0;
        var_8[2] = r12->mC[2] - r14->m8_pos[2];

        u32 var_0[2];
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
        r14->m20_angle[1] = dragonFieldTaskInitSub4Sub4Sub1Sub1Sub1(r14->m20_angle[1], var_8[0], 0x2000, 0x444444, 0x10);

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

        if (r14->m30 - r14->m20_angle[1] < r14->field_178[0])
        {
            r14->m20_angle[2] += performDivision(4, r14->field_178[0] * 3);
        }
        else if (r14->m20_angle[1] - r14->m30 < r14->field_178[0])
        {
            r14->m20_angle[2] -= performDivision(4, r14->field_178[0] * 3);
        }

        //607FB2A
        // clamp angle.x to valid range
        fixedPoint r2;
        if (r14->m20_angle[0] & 0x8000000)
        {
            r2 = 0xF0000000 | r14->m20_angle[0];
        }
        else
        {
            r2 = 0x0FFFFFFF & r14->m20_angle[0];
        }
        if (r2 > r14->m14C_pitchMax)
        {
            r14->m20_angle[0] = r14->m14C_pitchMax;
        }

        fixedPoint r3;
        if (r14->m20_angle[0] & 0x8000000)
        {
            r3 = 0xF0000000 | r14->m20_angle[0];
        }
        else
        {
            r3 = 0x0FFFFFFF & r14->m20_angle[0];
        }
        if (r3 < r14->m148_pitchMin)
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
            sFieldCameraStatus* pCamera = &getFieldTaskPtr()->m8_pSubFieldData->m334->field_3E4[r4];
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

            r4->field_238 = 4;
            r4->field_237 = 4;
            r4->m244 = 0;
        }
        else
        {
            if (r4->m23A_dragonAnimation == 5)
                return;

            r4->field_238 = 0;
            r4->field_237 = 0;
            r4->m244 = 5;
        }

        r4->field_23C |= 5;
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
            updateCameraScriptSub1(getFieldTaskPtr()->m8_pSubFieldData->m334->field_50E);
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

        if (pTypedWorkArea->field_1D4)
        {
            assert(0);
        }
        else if (pTypedWorkArea->m1D0_cameraScript)
        {
            updateCameraScript(pTypedWorkArea, pTypedWorkArea->m1D0_cameraScript);
        }
        else
        {
            dragonFieldTaskInitSub4Sub3(getFieldTaskPtr()->m8_pSubFieldData->m334->field_50E);
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
        unimplemented("dragonFieldTaskInitSub5");
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
        pTypedWorkArea->field_F0 = dragonFieldTaskInitSub4;

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
        pTypedWorkArea->field_FC = 0;
        pTypedWorkArea->m100 = 0;

        pTypedWorkArea->m14_oldPos = pTypedWorkArea->m8_pos;

        pTypedWorkArea->field_F0(pTypedWorkArea);

        dragonFieldTaskUpdateSub1Sub1();
    }

    void setLCSField83E(s_workArea* pLCS, u32 value)
    {
        unimplemented("setLCSField83E");
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
            r14->field_23B = 1;
        }
        else
        {
            playAnimation(&r13->m28_dragon3dModel, r13->m0_pDragonModelRawData + READ_BE_U32(r13->m0_pDragonModelRawData + r13->m20_dragonAnimOffsets[r12]), 10);
            r14->field_23B = 0;
        }

        updateAndInterpolateAnimation(&r13->m28_dragon3dModel);

        r14->m23A_dragonAnimation = r12;
        r14->field_237 = r14->field_238;
    }

    s8 dragonFieldAnimation[] = {
        5,7,8,11,9,9,9,10,
        4,4,4,11,0,0,2,10,
    };

    s32 getDragonFieldAnimation(s_dragonTaskWorkArea* pTypedWorkArea)
    {
        return dragonFieldAnimation[(pTypedWorkArea->field_238 >> 2) * 8 + pTypedWorkArea->field_238];
    }

    void dragonFieldAnimationUpdate(s_dragonTaskWorkArea* pTypedWorkArea, s_dragonState* r5)
    {
        u8 var = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m235_dragonSpeedIndex;

        if (pTypedWorkArea->field_23C & 4)
        {
            switch (pTypedWorkArea->field_23C & 3)
            {
            case 1:
                dragonFieldPlayAnimation(pTypedWorkArea, r5, pTypedWorkArea->m244);
                pTypedWorkArea->field_23C &= ~1;
                break;
            case 2:
                assert(0);
            default:
                break;
            }

            pTypedWorkArea->field_23C &= ~4;
        }
        else
        {
            if (getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m8 & 0x10)
            {
                assert(0);
            }
            else
            {
                if (pTypedWorkArea->field_238 & 4)
                {
                    if (getDragonSpeedIndex(pTypedWorkArea) <= 0)
                    {
                        pTypedWorkArea->field_238 &= 0xFFFFFFFB;
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
                    pTypedWorkArea->field_238 |= 4;
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
                if (pTypedWorkArea->field_23C)
                {
                    switch (pTypedWorkArea->field_23C)
                    {
                    case 1:
                        dragonFieldPlayAnimation(pTypedWorkArea, r5, pTypedWorkArea->m244);
                        pTypedWorkArea->field_23C &= ~1;
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
                        //assert(0);
                        unimplemented("unimplemented logic in dragonFieldAnimationUpdate!!!!");
                        dragonFieldPlayAnimation(pTypedWorkArea, r5, r6);
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
        unimplemented("dragonFieldTaskUpdateSub3");
    }

    void playDragonSoundEffect(s_dragonTaskWorkArea* pTypedWorkArea, s_dragonState* r5)
    {
        unimplemented("playDragonSoundEffect");
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
                u32 r15[2];

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
                    assert(0);
                }
                else
                {
                    if ((r13->field_3E4[i].m74 == dragonFieldTaskUpdateSub5Sub3) || (r13->field_3E4[i].m74 == dragonFieldTaskUpdateSub5Sub4))
                    {
                        r13->field_3E4[i].m74(&r13->field_3E4[i]);
                    }
                    else
                    {
                        if ((r13->m2E0 > 0) || (r13->m2D0 > 0))
                        {
                            dragonFieldTaskUpdateSub5Sub5(r13);
                        }
                        else
                        {
                            if (r13->field_3E4[i].m74)
                            {
                                r13->field_3E4[i].m74(&r13->field_3E4[i]);
                            }
                        }
                    }
                }
                r13->field_3E4[i].m84++;
            }
        }
    }

    void dragonFieldTaskUpdateSub6(s_dragonTaskWorkArea* pTypedWorkArea)
    {
        unimplemented("dragonFieldTaskUpdateSub6");
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
        updateAndInterpolateAnimation(&pRiderState->m18_3dModel);
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
        return getFieldTaskPtr()->m8_pSubFieldData->m334->field_3E4[index].m8C;
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
                    sFieldCameraStatus* pFieldCameraStatus = &getFieldTaskPtr()->m8_pSubFieldData->m334->field_3E4[i];
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
                varC[0] = -MTH_Mul_5_6(fixedPoint(0x10000), getCos(pTypedWorkArea->field_C0.getInteger() & 0xFFF), getSin(pTypedWorkArea->field_C4.getInteger() & 0xFFF));
                varC[1] = MTH_Mul(fixedPoint(0x10000), getSin(pTypedWorkArea->field_C0.getInteger() & 0xFFF));
                varC[2] = -MTH_Mul_5_6(fixedPoint(0x10000), getCos(pTypedWorkArea->field_C0.getInteger() & 0xFFF), getCos(pTypedWorkArea->field_C4.getInteger() & 0xFFF));

                sVec3_FP var0;
                //transformVecByCurrentMatrix(&varC, &var0);
//                assert(0);
                unimplemented("dragonFieldTaskDrawSub1");
            }

        }

        unimplemented("dragonFieldTaskDrawSub1");

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
        scaleCurrentMatrixRow0(pTypedWorkArea->field_150);
        scaleCurrentMatrixRow1(pTypedWorkArea->field_150);
        scaleCurrentMatrixRow2(pTypedWorkArea->field_150);

        gDragonState->m28_dragon3dModel.mC_modelIndexOffset = gDragonState->m14_modelIndex;
        gDragonState->m28_dragon3dModel.m18_drawFunction(&gDragonState->m28_dragon3dModel);
        popMatrix();

        unimplemented("dragonFieldTaskDraw");
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

        if (pFieldCameraTask1->field_28 & 1)
        {
            assert(0);
        }
        else
        {
            X = performDivision(pFieldCameraTask1->field_20, pFieldCameraTask1->m0_position[0]);
            if (pFieldCameraTask1->m0_position[0] < 0)
            {
                X--;
            }
        }

        s32 Z = performDivision(pFieldCameraTask1->field_24, -pFieldCameraTask1->m0_position[2]);
        if (pFieldCameraTask1->m0_position[2] < 0)
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

        return bDirty;
    }

    s32 fieldCameraTask1InitSub2(sVec3_FP* r4, s32 r5)
    {
        unimplemented("fieldCameraTask1InitSub2");
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
        pTypedWorkArea->field_12FC = fieldCameraTask1InitSub2;
        pTypedWorkArea->m12F2_renderMode = 2;

        pTypedWorkArea->m18_cameraGridLocation[0] = -1;
        pTypedWorkArea->m18_cameraGridLocation[1] = -1;
        pTypedWorkArea->field_30 = 0;
        pTypedWorkArea->m34_cameraVisibilityTable = 0;
        pTypedWorkArea->m2C_depthRangeTable = fieldCameraTask1InitData1_depthRangeTable;
        pTypedWorkArea->field_1300 = 3;
    }

    void fieldCameraTask1Update(s_workArea* pWorkArea)
    {
        s_visibilityGridWorkArea* pTypedWorkArea = static_cast<s_visibilityGridWorkArea*>(pWorkArea);

        pTypedWorkArea->m0_position = cameraProperties2.m0_position;
        pTypedWorkArea->m12F8_convertCameraPositionToGrid(pTypedWorkArea);
    }

    sMatrix4x3* fieldCameraTask1DrawSub1()
    {
        return &getFieldTaskPtr()->m8_pSubFieldData->m334->field_384;
    }

    void fieldCameraTask1Draw(s_workArea* pWorkArea)
    {
        s_visibilityGridWorkArea* pTypedWorkArea = static_cast<s_visibilityGridWorkArea*>(pWorkArea);

        sMatrix4x3* r13 = fieldCameraTask1DrawSub1();

        asyncDivStart(graphicEngineStatus.field_4070, fixedPoint(0xC422));

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
        copyMatrix(r13, &var30);
        rotateMatrixShiftedX(fixedPoint(0x238E38F), &var30);

        pTypedWorkArea->m12AC[0] = var90.matrix[0 * 4 + 3]; // 8
        pTypedWorkArea->m12AC[1] = var90.matrix[1 * 4 + 3]; // 18
        pTypedWorkArea->m12AC[2] = var90.matrix[2 * 4 + 3]; // 28

        pTypedWorkArea->m12B8[0] = var60.matrix[0 * 4 + 3]; // 8
        pTypedWorkArea->m12B8[1] = var60.matrix[1 * 4 + 3]; // 18
        pTypedWorkArea->m12B8[2] = var60.matrix[2 * 4 + 3]; // 28

        pTypedWorkArea->m12C4[0] = var30.matrix[0 * 4 + 3]; // 8
        pTypedWorkArea->m12C4[1] = var30.matrix[1 * 4 + 3]; // 18
        pTypedWorkArea->m12C4[2] = var30.matrix[2 * 4 + 3]; // 28

        pTypedWorkArea->m12D0[0] = var00.matrix[0 * 4 + 3]; // 8
        pTypedWorkArea->m12D0[1] = var00.matrix[1 * 4 + 3]; // 18
        pTypedWorkArea->m12D0[2] = var00.matrix[2 * 4 + 3]; // 28

        pTypedWorkArea->m12DC = asyncDivEnd();

        if (getFieldTaskPtr()->m8_pSubFieldData->fieldDebuggerWho & 1)
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

        r4->m44 = r4->m68;
        r4->m68[0].m0 = 0;
    }

    void LCSTaskDrawSub1(s_LCSTask* r4)
    {
        unimplemented("LCSTaskDrawSub1");
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

        if (getFieldTaskPtr()->m8_pSubFieldData->field_354 > 1)
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
        unimplemented("overlayStart_Sub1");
    }

    p_workArea overlayStart(p_workArea workArea, u32 arg)
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


        graphicEngineStatus.field_406C = 0x3000;
        graphicEngineStatus.field_408C = FP_Div(0x10000, 0x3000);

        graphicEngineStatus.field_4070 = 0x200000;
        graphicEngineStatus.field_4094 = FP_Div(0x8000, 0x200000);

        graphicEngineStatus.field_4090 = graphicEngineStatus.field_4094 << 8;

        getFieldTaskPtr()->m8_pSubFieldData->m334->field_50E = 1;
        getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m0_pScripts = FLD_A3_Scripts;

        switch (getFieldTaskPtr()->m2C_currentFieldIndex)
        {
        case 1:
            unimplemented("Hacked the field table!");
            subfieldTable1[0](workArea);
            break;
        case 2:
            assert(0);
        default:
            subfieldTable1[0](workArea);
            break;
        }

        getFieldTaskPtr()->m8_pSubFieldData->pUpdateFunction3 = overlayStart_Sub1;

        return NULL;
    }

};
