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

    pFieldTaskWorkArea->pSubFieldData->ptrToE = pFieldScriptWorkArea;

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

void fieldOverlaySubTaskInitSub6(s_workArea* pWorkArea)
{
    unimplemented("fieldOverlaySubTaskInitSub6");
}

void *unk_6092EF0 = NULL;

void fieldOverlaySubTaskInit(s_workArea* pWorkArea)
{
    s_fieldOverlaySubTaskWorkArea* pTypedWorkArea = static_cast<s_fieldOverlaySubTaskWorkArea*>(pWorkArea);

    getFieldTaskPtr()->pSubFieldData->field_334 = pTypedWorkArea;

    fieldOverlaySubTaskInitSub1(0, &fieldOverlaySubTaskInitSub2, 0);
    fieldOverlaySubTaskInitSub3(0);
    fieldOverlaySubTaskInitSub4(unk_6092EF0, 1);

    getFieldTaskPtr()->pSubFieldData->field_334->field_50E = 1;

    fieldOverlaySubTaskInitSub5(1);

    fieldOverlaySubTaskInitSub6(pTypedWorkArea);

    getFieldTaskPtr()->pSubFieldData->field_334->field_50D = 1;
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

void initMatrixToIdentity(s32* matrix)
{
    matrix[0] = 0x10000;
    matrix[1] = 0;
    matrix[2] = 0;
    matrix[3] = 0;

    matrix[4] = 0;
    matrix[5] = 0x10000;
    matrix[6] = 0;
    matrix[7] = 0;

    matrix[8] = 0;
    matrix[9] = 0;
    matrix[10] = 0x10000;
    matrix[11] = 0;
}

void dragonFieldTaskInitSub2Sub4(s_dragonTaskWorkArea_48* field_48)
{
    initMatrixToIdentity(field_48->matrix);

    field_48->field_30 = 0;
    field_48->field_34 = 0;
    field_48->field_38 = 0;

    field_48->field_3C = 1;
}

void dragonFieldTaskInitSub2Sub5(u32 arg)
{
    if ((arg >= 0) && (arg <= 2))
    {
        s_dragonTaskWorkArea* pDragonTask = getFieldTaskPtr()->pSubFieldData->pDragonTask;

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
    return &getFieldTaskPtr()->pSubFieldData->field_334->field_3E4[getFieldTaskPtr()->pSubFieldData->field_334->field_50C];
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

    pWorkArea->posX = 0;
    pWorkArea->posY = 0x1E000;
    pWorkArea->posZ = 0;

    pWorkArea->angleX = 0;
    pWorkArea->angleY = 0;
    pWorkArea->angleZ = 0;

    pWorkArea->field_154 = 0;

    pWorkArea->field_1B8 = 0xB333;
    pWorkArea->field_1BC = 0x200000;

    initMatrixToIdentity(pWorkArea->matrix);

    dragonFieldTaskInitSub2Sub4(&pWorkArea->field_48);

    pWorkArea->field_1CC = 0x38E38E3;
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

    pFieldCameraStatus->angle_y = pWorkArea->angleY;

    pWorkArea->field_14C = 0x2AAAAAA;
    pWorkArea->field_148 = -0x2AAAAAA;
    pWorkArea->field_140 = 0x300000;
    pWorkArea->field_134 = -0x300000;
    pWorkArea->field_130 = 0x80000000;
    pWorkArea->field_13C = 0x7FFFFFFF;
    pWorkArea->field_138 = 0x80000000;
    pWorkArea->field_144 = 0x7FFFFFFF;
}

void dragonFieldTaskInit(s_workArea* pWorkArea, u32 arg)
{
    s_dragonTaskWorkArea* pTypedWorkArea = static_cast<s_dragonTaskWorkArea*>(pWorkArea);

    getFieldTaskPtr()->pSubFieldData->pDragonTask = pTypedWorkArea;

    getMemoryArea(&pTypedWorkArea->field_0, 0);
    dragonFieldTaskInitSub2(pTypedWorkArea);
    /*dragonFieldTaskInitSub3(pTypedWorkArea, gDragonState, 5);
    pTypedWorkArea->field_F0 = dragonFieldTaskInitSub4;

    createSubTask(pWorkArea, &dragonFieldSubTaskDefinition, new s_dummyWorkArea);
    */
    assert(0);
}

s_taskDefinitionWithArg dragonFieldTaskDefinition = { dragonFieldTaskInit, dummyTaskUpdate, dummyTaskDraw, dummyTaskDelete, "dragonFieldTask" };

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

    if (gDragonState->dragonType == DR_LEVEL_8_FLOATER)
    {
        assert(0);
    }
    else
    {
        createSubTaskWithArg(pWorkArea, &dragonFieldTaskDefinition, new s_dragonTaskWorkArea, param);
    }
}

s_taskDefinition fieldOverlaySubTask2Definition = { dummyTaskInit, NULL, NULL, NULL, "fieldOverlaySubTask2" };

void createFieldOverlaySubTask2(s_workArea* pWorkArea)
{
    createSubTask(pWorkArea, &fieldOverlaySubTask2Definition, new s_dummyWorkArea);
}

s_taskDefinition fieldCameraTask1Definition = { dummyTaskInit, dummyTaskUpdate, dummyTaskDraw, NULL, "fieldCameraTask1" };
s_taskDefinition fieldCameraTask2Definition = { dummyTaskInit, NULL, dummyTaskDraw, NULL, "fieldCameraTask2" };

void createFieldCameraTask(s_workArea* pWorkArea)
{
    createSubTask(pWorkArea, &fieldCameraTask1Definition, new s_dummyWorkArea);
    createSubTask(pWorkArea, &fieldCameraTask2Definition, new s_dummyWorkArea);
}

s_taskDefinition randomBattleTaskDefinition = { dummyTaskInit, NULL, dummyTaskDraw, NULL, "randomBattleTask" };

void createRandomBattleTask(s_workArea* pWorkArea)
{
    createSubTask(pWorkArea, &randomBattleTaskDefinition, new s_dummyWorkArea);
}

void fieldDebugMenuUpdate1()
{
    s_FieldSubTaskWorkArea* pSubFieldData = getFieldTaskPtr()->pSubFieldData;
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
            getFieldTaskPtr()->fStatus |= 8;
        }
        else
        {
            getFieldTaskPtr()->fStatus &= 0xFFFFFFF7;
        }
    }
}
void fieldDebugMenuUpdate2()
{
    //unimplemented("fieldUpdateFunction2");
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

    getFieldTaskPtr()->pSubFieldData->field_334->field_50D = 1;

    getFieldTaskPtr()->pSubFieldData->pUpdateFunction2 = fieldDebugMenuUpdate2;
    getFieldTaskPtr()->pSubFieldData->pUpdateFunction1 = fieldDebugMenuUpdate1;

    pauseEngine[2] = 1;

    return true;
}

p_workArea overlayStart(p_workArea workArea, u32 arg)
{
    initField(workArea, fieldFileList, arg);

    return NULL;
}

};