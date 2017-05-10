#include "PDS.h"

namespace FLD_A3_OVERLAY {

const char* fieldFileList[] =
{
    "FLDCMN.MCB",
    "FLDCMN.CGB",
    "FLD_A3.MCB",
    "FLD_A3.CGB",
    "FLD_A3_0.MCB",
    "FLD_A3_0.CGB",
    "FLD_A3_1.MCB",
    "FLD_A3_1.CGB",
    "FLD_A3_2.MCB",
    "FLD_A3_2.CGB",
    "FLD_A3_3.MCB",
    "FLD_A3_3.CGB",
    NULL
};

bool initField(p_workArea workArea, const char** fieldFileList, u32 arg)
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
    initFieldDragon(workArea);
    createFieldOverlaySubTask2(workArea);
    createFieldCameraTask(workArea);
    createRandomBattleTask(workArea);
    resetProjectVector();

    getFieldTaskPtr()->pSubFieldData->field_334->field_50D = 1;

    getFieldTaskPtr()->pSubFieldData->pUpdateFunction2 = fieldUpdateFunction2;
    getFieldTaskPtr()->pSubFieldData->pUpdateFunction1 = fieldUpdateFunction1;

    pauseEngine[2] = 1;

    return true;
}

p_workArea overlayStart(p_workArea workArea, u32 arg)
{
    initField(workArea, fieldFileList, arg);

    return NULL;
}

};