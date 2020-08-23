#include "PDS.h"
#include "fieldSavePoint.h"
#include "fieldDialog.h"
#include "fieldItemBox.h"
#include "field_a3/o_fld_a3.h" // TODO: cleanup for gFLD_A3 common data access

void startScript_savePointCallback(s32 entryPointIndex)
{
    setupSaveParams(getFieldTaskPtr()->m2C_currentFieldIndex, getFieldTaskPtr()->m2E_currentSubFieldIndex, entryPointIndex);
    if (getFieldTaskPtr()->m8_pSubFieldData->m370_fieldDebuggerWho & 1)
    {
        assert(0);
    }
}

void startScript_savePoint(s32 index)
{
    sSaturnPtr pScript = gFLD_A3->getSaturnPtr(0x06080efb);
    if (queueNewFieldScript(pScript, -1) != 0)
    {
        registerCallbackForDialog(startScript_savePointCallback, nullptr, index);
    }
}

void LCSItemBox_CallbackSavePoint(p_workArea r4, sLCSTarget*)
{
    s_itemBoxType1* pThis = (s_itemBoxType1*)r4;
    startScript_savePoint(pThis->m84_savePointIndex - 1);
    if (pThis->m80_bitIndex > 999)
    {
        mainGameState.setBit566(pThis->m80_bitIndex);
    }
}
