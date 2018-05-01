#include "PDS.h"

namespace FLD_A3_OVERLAY {

const s_MCB_CGB fieldFileList[] =
{
    {"FLDCMN.MCB", "FLDCMN.CGB"},
    {"FLD_A3.MCB", "FLD_A3.CGB"},
    {"FLD_A3_0.MCB", "FLD_A3_0.CGB"},
    {"FLD_A3_1.MCB", "FLD_A3_1.CGB"},
    {"FLD_A3_2.MCB", "FLD_A3_2.CGB"},
    {"FLD_A3_3.MCB", "FLD_A3_3.CGB"},
    {NULL, NULL}
};

#define WRITE_SCRIPT_U16(value) value & 0xFF, (value >> 8) & 0xFF
#define WRITE_SCRIPT_U32(value) value & 0xFF, (value >> 8) & 0xFF, (value >> 16) & 0xFF, (value >> 24) & 0xFF
#if 0
#define WRITE_SCRIPT_POINTER(stringPtr) (u64)stringPtr & 0xFF, ((u64)stringPtr >> 8) & 0xFF, ((u64)stringPtr >> 16) & 0xFF, ((u64)stringPtr >> 24) & 0xFF, ((u64)stringPtr >> 32) & 0xFF, ((u64)stringPtr >> 40) & 0xFF, ((u64)stringPtr >> 48) & 0xFF, ((u64)stringPtr >> 56) & 0xFF
#else 
#define WRITE_SCRIPT_POINTER(stringPtr) WRITE_SCRIPT_U32(0)
#endif

#define op_END() 0x1
#define op_CALL_NATIVE_0(funcPtr) 0xE, 0x0, WRITE_SCRIPT_POINTER(funcPtr)
#define op_CALL_NATIVE_1(funcPtr, arg0) 0xE, 0x1, WRITE_SCRIPT_POINTER(funcPtr), WRITE_SCRIPT_U32(arg0)
#define op_START_CUTSCENE() 0x18
#define op_WAIT(delay) 0x02, WRITE_SCRIPT_U16(delay)
#define op_DISPLAY_DIALOG_STRING(stringPtr) 0x15, WRITE_SCRIPT_POINTER(stringPtr)
#define op_CLEAR_DIALOG_STRING() 0x16
#define op_ADD_CINEMATIC_BARS() 0x19
#define op_END_CUTSCENE() 0x1A
#define op_PLAY_PCM(stringPtr) 0x22, WRITE_SCRIPT_POINTER(stringPtr)

const char EV03_1_PCM[] = "EV03_1.PCM";

const char FLD_A3_Script_0_String0[] = "(Imperial piece of crap.)";
const char FLD_A3_Script_1_String0[] = "Damn!";
const char FLD_A3_Script_1_String1[] = "A wind net to keep monsters away.";

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
    {4, 12}, // grid size
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

s_taskDefinition fieldGridTaskDefinition = { NULL, dummyTaskUpdate, dummyTaskDraw, NULL, "fieldGridTask" };

void setupGridCell(s_visibilityGridWorkArea* r4, s_visdibilityCellTask* r5, int index)
{
    getMemoryArea(&r5->memoryLayout, r4->field_30->mC);
    r5->index = index;
    if (r4->field_30->m0_environmentGrid)
    {
        r5->pEnvironmentCell = r4->field_30->m0_environmentGrid[index];
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

void gridCellDraw_normal(p_workArea workArea)
{
    s_visdibilityCellTask* pTypedWorkAread = static_cast<s_visdibilityCellTask*>(workArea);

    if (pTypedWorkAread->pEnvironmentCell)
    {
        assert(0);
    }

    if (pTypedWorkAread->pCell2)
    {
        assert(0);
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
    {-1,-1},
    {0,-1},
    {1,-1},
    {-1,0},
    {0,0},
    {1,0},
    {-1,1},
    {0,1},
    {1,1}
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
                pFieldCameraTask1->field_C = r3 / 2;
                
            }
            pFieldCameraTask1->field_10 = 0;
            {
                s32 r3 = -(r4->m10_gridSize[1] * pFieldCameraTask1->field_24);
                if (r3 < 0)
                {
                    r3++;
                }
                pFieldCameraTask1->field_14 = r3 / 2;
            }
        }
        //06071032

        pFieldCameraTask1->m0_position[0] = cameraProperties2.m0_position[0];
        pFieldCameraTask1->m0_position[1] = cameraProperties2.m0_position[1];
        pFieldCameraTask1->m0_position[2] = cameraProperties2.m0_position[2];

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

void subfieldA3_0(p_workArea workArea)
{
    playPCM(workArea, 100);
    playPCM(workArea, 101);

    setupField(&fieldA3_0_dataTable3, fieldA3_0_dataTable2, fieldA3_0_startTasks, fieldA3_0_dataTable1);

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

    pFieldTaskWorkArea->m8_pSubFieldData->ptrToE = pFieldScriptWorkArea;

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
    createSubTask(pWorkArea, &fieldScriptTaskDefinition, new s_dummyWorkArea);
}

void fieldOverlaySubTaskInitSub2(void)
{
    unimplemented("fieldOverlaySubTaskInitSub2");
}

void fieldOverlaySubTaskInitSub1(u32 r4, void(*r5)(), u32 r6)
{
    unimplemented("fieldOverlaySubTaskInitSub1");
}

void fieldOverlaySubTaskInitSub3(u32 r4)
{
    unimplemented("fieldOverlaySubTaskInitSub3");
}

void fieldOverlaySubTaskInitSub4(void* r4, u32 r5)
{
    unimplemented("fieldOverlaySubTaskInitSub4");
}

void fieldOverlaySubTaskInitSub5(u32 r4)
{
    unimplemented("fieldOverlaySubTaskInitSub5");
}

void updateCameraFromDragonSub2(s_fieldOverlaySubTaskWorkArea* pTypedWorkArea)
{
    assert(pTypedWorkArea->field_50C == 0);
    sFieldCameraStatus* r13 = &pTypedWorkArea->field_3E4[pTypedWorkArea->field_50C];
    s16 r15[3];
    r15[0] = r13->mC_rotation[0] >> 16;
    r15[1] = r13->mC_rotation[1] >> 16;
    r15[2] = r13->mC_rotation[2] >> 16;

    updateEngineCamera(&cameraProperties2, r13, r15);

    copyMatrix(pCurrentMatrix, &pTypedWorkArea->field_384);
    copyMatrix(&cameraProperties2.m28[0], &pTypedWorkArea->field_3B4);
}

void *unk_6092EF0 = NULL;

void fieldOverlaySubTaskInit(s_workArea* pWorkArea)
{
    s_fieldOverlaySubTaskWorkArea* pTypedWorkArea = static_cast<s_fieldOverlaySubTaskWorkArea*>(pWorkArea);

    getFieldTaskPtr()->m8_pSubFieldData->m334 = pTypedWorkArea;

    fieldOverlaySubTaskInitSub1(0, &fieldOverlaySubTaskInitSub2, 0);
    fieldOverlaySubTaskInitSub3(0);
    fieldOverlaySubTaskInitSub4(unk_6092EF0, 1);

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

s_taskDefinition dragonFieldSubTaskDefinition = { dummyTaskInit, dummyTaskUpdate, NULL, NULL, "dragonFieldSubTask" };

s_taskDefinition dragonFieldSubTask2Definition = { dummyTaskInit, dummyTaskUpdate, dummyTaskDraw, NULL, "dragonFieldSubTask2"};

void initDragonFieldSubTask2(s_workArea* pWorkArea)
{
    createSubTask(pWorkArea, &dragonFieldSubTask2Definition, new s_dummyWorkArea);
}

void dragonFieldTaskInitSub2Sub2(u32* field_178)
{
    field_178[0] = 0x222222;
    field_178[1] = 0x4CCC;
    field_178[3] = 0x16;
    field_178[4] = 0x111111;
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

void dragonFieldTaskInitSub2Sub5(u32 arg)
{
    if ((arg >= 0) && (arg <= 2))
    {
        s_dragonTaskWorkArea* pDragonTask = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;

        pDragonTask->field_235 = arg;

        pDragonTask->field_154 = pDragonTask->field_21C[arg];

        s32 r2 = pDragonTask->field_21C[0] + pDragonTask->field_21C[1];
        if (r2 < 0)
        {
            assert(0);
            r2++;
        }
        r2 /= 2;

        if (pDragonTask->field_154 > r2)
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
    return &getFieldTaskPtr()->m8_pSubFieldData->m334->field_3E4[getFieldTaskPtr()->m8_pSubFieldData->m334->field_50C];
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

    pWorkArea->field_154 = 0;

    pWorkArea->field_1B8 = 0xB333;
    pWorkArea->field_1BC = 0x200000;

    initMatrixToIdentity(&pWorkArea->m88_matrix);

    dragonFieldTaskInitSub2Sub4(&pWorkArea->m48);

    pWorkArea->field_1CC = 0x38E38E3; // field of view
    pWorkArea->field_234 = 0;

    pWorkArea->field_21C[0] = 0;
    pWorkArea->field_21C[1] = 0x1284;
    pWorkArea->field_21C[2] = 0x2509;
    pWorkArea->field_21C[3] = 0x3B42;
    pWorkArea->field_21C[4] = 0x58E3;

    dragonFieldTaskInitSub2Sub5(0);

    pWorkArea->field_230 = 0x1999;
    
    //060738C0

    pWorkArea->field_154 = pWorkArea->field_21C[pWorkArea->field_235];

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

    pWorkArea->field_14C = 0x2AAAAAA;
    pWorkArea->field_148 = -0x2AAAAAA;
    pWorkArea->field_140 = 0x300000;
    pWorkArea->field_134 = -0x300000;
    pWorkArea->field_130 = 0x80000000;
    pWorkArea->field_13C = 0x7FFFFFFF;
    pWorkArea->field_138 = 0x80000000;
    pWorkArea->field_144 = 0x7FFFFFFF;
}

void dragonFieldTaskInitSub3(s_dragonTaskWorkArea* pWorkArea, s_dragonState* pDragonState, int param2)
{
    dragonFieldTaskInitSub3Sub1(&pDragonState->m28_dragon3dModel, pDragonState->m0_pDragonModelRawData + READ_BE_U32(pDragonState->m0_pDragonModelRawData + pDragonState->m20_dragonAnimOffsets[param2]));
    dragonFieldTaskInitSub3Sub2(&pDragonState->m28_dragon3dModel);

    pWorkArea->field_23A = param2;
    pWorkArea->field_237 = pWorkArea->field_238;
    pWorkArea->field_23B = 1;
}

void dragonFieldTaskInitSub4Sub3(u8 r4)
{
    unimplemented("dragonFieldTaskInitSub4Sub3");
}

void dragonFieldTaskInitSub4Sub4()
{
    unimplemented("dragonFieldTaskInitSub4Sub4");
}

void dragonFieldTaskInitSub4Sub5(sMatrix4x3* r4, sVec3_FP* r5)
{
    unimplemented("dragonFieldTaskInitSub4Sub5");
}

void dragonFieldTaskInitSub4Sub6()
{
    unimplemented("dragonFieldTaskInitSub4Sub6");
}

void dragonFieldTaskInitSub4(s_dragonTaskWorkArea* pTypedWorkArea)
{
    getFieldTaskPtr()->m28_status |= 0x10000;

    if (pTypedWorkArea->field_1D4)
    {
        assert(0);
    }
    else if (pTypedWorkArea->field_1D0)
    {
        assert(0);
    }
    else
    {
        dragonFieldTaskInitSub4Sub3(getFieldTaskPtr()->m8_pSubFieldData->m334->field_50E);
        dragonFieldTaskInitSub4Sub4();
    }

    dragonFieldTaskInitSub4Sub5(&pTypedWorkArea->m48.m0_matrix, &pTypedWorkArea->m20_angle);

    copyMatrix(&pTypedWorkArea->m48.m0_matrix, &pTypedWorkArea->m88_matrix);

    pTypedWorkArea->m8_pos[0] += pTypedWorkArea->m160[0];
    pTypedWorkArea->m8_pos[1] += pTypedWorkArea->m160[1];
    pTypedWorkArea->m8_pos[2] += pTypedWorkArea->m160[2];

    dragonFieldTaskInitSub4Sub6();
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

    createSubTask(pWorkArea, &dragonFieldSubTaskDefinition, new s_dummyWorkArea);

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

    if (pDragonTask->field_F8 & 0x400)
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

s32 dragonFieldAnimationUpdateSub1(s_dragonTaskWorkArea* pTypedWorkArea)
{
    if (pTypedWorkArea->field_F8 & 0x20000)
    {
        assert(0);
    }
    else
    {
        return pTypedWorkArea->field_235;
    }
}

void dragonFieldPlayAnimation(s_workArea* r4, s_dragonState* r5, u8 r6)
{
    unimplemented("dragonFieldPlayAnimation");
}

s32 getDragonFieldAnimation(s_dragonTaskWorkArea* pTypedWorkArea)
{
    unimplemented("getDragonFieldAnimation");
    return 1;
}

void dragonFieldAnimationUpdate(s_dragonTaskWorkArea* pTypedWorkArea, s_dragonState* r5)
{
    u8 var = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->field_235;

    if (pTypedWorkArea->field_23C & 4)
    {
        switch (pTypedWorkArea->field_23C & 3)
        {
        case 1:
            assert(0);
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
                assert(0);
            }

            if (dragonFieldAnimationUpdateSub1(pTypedWorkArea) > 0)
            {
                assert(0);
            }

            if (r5->m28_dragon3dModel.field_16 == 0)
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
                    assert(0);
                }
                dragonFieldPlayAnimation(pTypedWorkArea, r5, r6);
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

void dragonFieldTaskUpdateSub5()
{
    unimplemented("dragonFieldTaskUpdateSub5");
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

    if(pTypedWorkArea->mB8)
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
        else
        {
            dragonFieldTaskInitSub3Sub2(&gDragonState->m28_dragon3dModel);
            dragonFieldTaskInitSub3Sub2(&pRiderState->m18_3dModel);
            if (mainGameState.gameStats.m3_rider2)
            {
                dragonFieldTaskInitSub3Sub2(&pRider2State->m18_3dModel);
            }
        }
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
            assert(0);
        }
    }

    updateCameraFromDragonSub2(r12);
}

void dragonFieldTaskDrawSub1(s_dragonTaskWorkArea* pTypedWorkArea)
{
    updateCameraFromDragon();

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
    gDragonState->m28_dragon3dModel.drawFunction(&gDragonState->m28_dragon3dModel);
    popMatrix();

    unimplemented("dragonFieldTaskDrawSub1");
}

void dragonFieldTaskDraw(s_workArea* pWorkArea)
{
    s_dragonTaskWorkArea* pTypedWorkArea = static_cast<s_dragonTaskWorkArea*>(pWorkArea);

    dragonFieldTaskDrawSub1(pTypedWorkArea);

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

    s32 Z = performDivision(pFieldCameraTask1->field_24, pFieldCameraTask1->m0_position[2]);
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

void fieldCameraTask1InitSub2()
{
    assert(0);
}

u8* fieldCameraTask1InitData1 = (u8*)1;

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
    pTypedWorkArea->field_2C = fieldCameraTask1InitData1;
    pTypedWorkArea->field_1300 = 3;
}

s_taskDefinition fieldCameraTask1Definition = { fieldCameraTask1Init, dummyTaskUpdate, dummyTaskDraw, NULL, "fieldCameraTask1" };

void LCSTaskInit(p_workArea pWorkArea)
{
    s_visibilityGridWorkArea* r4 = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;
    r4->m1290_vdp2VramOffset = 0x25E68000;
    setVdp2VramU32(r4->m1290_vdp2VramOffset, 0xFFFFFFFF);

    r4->m128C_vdp2VramOffset2 = 0x25E68800;
    setVdp2VramU32(r4->m128C_vdp2VramOffset2, 0xFFFFFFFF);

    r4->m44 = r4->m68;
    r4->m68[0] = 0;
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
    if ((pSubFieldData->debugMenuStatus1_a == 0) && (pSubFieldData->field_369 == 0))
    {
        if (pSubFieldData->debugMenuStatus3 == 0)
        {
            if (readKeyboardToggle(0x84))
            {
                pSubFieldData->debugMenuStatus1_b++;
                pSubFieldData->debugMenuStatus2_a = 0;
                clearVdp2TextMemory();
            }
            else
            {
                if (readKeyboardToggle(0xF6))
                {
                    pSubFieldData->debugMenuStatus1_b = 0;
                    clearVdp2TextMemory();
                }
            }
        }
        
        switch (pSubFieldData->debugMenuStatus1_b)
        {
        case 0:
            break;
        default:
            assert(0);
            break;
        }

        if (pSubFieldData->debugMenuStatus1_b)
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
    unimplemented("fieldDebugMenuUpdate2");
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
    getFieldTaskPtr()->m8_pSubFieldData->ptrToE->pScripts = FLD_A3_Scripts;

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
