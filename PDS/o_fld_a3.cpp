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

s_taskDefinition fieldScriptTaskDefinition = { dummyTaskInit, dummyTaskUpdate, NULL, NULL, "fieldScriptTask" };

void createFieldScriptTask(s_workArea* pWorkArea)
{
    createSubTask(pWorkArea, &fieldScriptTaskDefinition, new s_dummyWorkArea);
}

s_taskDefinition fieldOverlaySubTaskDefinition = { dummyTaskInit, NULL, NULL, NULL, "fieldOverlaySubTask" };

void createFieldOverlaySubTask(s_workArea* pWorkArea)
{
    createSubTask(pWorkArea, &fieldOverlaySubTaskDefinition, new s_dummyWorkArea);
}

s_taskDefinitionWithArg dragonFieldTaskDefinition = { dummyTaskInitWithArg, dummyTaskUpdate, dummyTaskDraw, dummyTaskDelete, "dragonFieldTask" };

void initFieldDragon(s_workArea* pWorkArea, u32 param)
{
    if (getFieldTaskPtr()->updateDragonAndRiderOnInit)
    {
        assert(0);
    }

    if (gDragonState->dragonType == DR_LEVEL_8_FLOATER)
    {
        assert(0);
    }
    else
    {
        createSubTaskWithArg(pWorkArea, &dragonFieldTaskDefinition, new s_dummyWorkArea, param);
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
    /*createFieldOverlaySubTask2(workArea);
    createFieldCameraTask(workArea);
    createRandomBattleTask(workArea);
    resetProjectVector();

    getFieldTaskPtr()->pSubFieldData->field_334->field_50D = 1;

    getFieldTaskPtr()->pSubFieldData->pUpdateFunction2 = fieldUpdateFunction2;
    getFieldTaskPtr()->pSubFieldData->pUpdateFunction1 = fieldUpdateFunction1;

    pauseEngine[2] = 1;
    */
    assert(0);
    return true;
}

p_workArea overlayStart(p_workArea workArea, u32 arg)
{
    initField(workArea, fieldFileList, arg);

    return NULL;
}

};