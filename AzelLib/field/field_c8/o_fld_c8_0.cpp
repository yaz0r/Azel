#include "PDS.h"
#include "o_fld_c8.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldRadar.h"
#include "audio/soundDriver.h"

static void fieldC8_0_startTasks(p_workArea workArea) { Unimplemented(); }

// 06054460
void subfieldC8_0(p_workArea workArea)
{
    // 0605f794 — create VDP2 palette task
    Unimplemented(); // createSubTaskWithArg with data at 060b2ddc/060b2d74

    // 06079e26 — setupField with data tables
    s_DataTable3* pDT3 = readDataTable3(gFLD_C8->getSaturnPtr(0x0608FDE4));
    setupField2(pDT3, fieldC8_0_startTasks);

    // 0607d8ca — adjustVerticalLimits
    adjustVerticalLimits(0x6C000, 0xF91000);

    // 06055bb4 — camera config
    setupFieldCameraConfigs(readCameraConfig(gFLD_C8->getSaturnPtr(0x0608AF64)), 1);

    // 06054210 — clip distances and overlay init
    graphicEngineStatus.m405C.m10_nearClipDistance = 0x3000;
    graphicEngineStatus.m405C.m30_oneOverNearClip = FP_Div(0x10000, graphicEngineStatus.m405C.m10_nearClipDistance);
    graphicEngineStatus.m405C.m14_farClipDistance = 0x200000;
    graphicEngineStatus.m405C.m38_oneOverFarClip = FP_Div(0x8000, graphicEngineStatus.m405C.m14_farClipDistance);
    graphicEngineStatus.m405C.m34_oneOverFarClip256 = graphicEngineStatus.m405C.m38_oneOverFarClip << 8;
    // FUN_FLD_C8__06078a26 — sets m12FC clipping function
    Unimplemented();
    getFieldTaskPtr()->m8_pSubFieldData->m334->m50E = 2;
    Unimplemented(); // FUN_FLD_C8__0607b4e0(0x12c000) — radar distance?

    // 06057f1a
    Unimplemented(); // needs Ghidra analysis

    // 0605cbec — empty function

    // Dragon update override: mF4 = 0605e7be (the decompilation writes to offset 0xF4, not 0xF0)
    Unimplemented(); // set mF4 dragon update to FLD_C8::0605e7be

    // Camera setup
    Unimplemented(); // FUN_FLD_C8__0607d684 — setup with data at 0608A868/0608A874

    s16 entryPoint = getFieldTaskPtr()->m30_fieldEntryPoint;
    if (entryPoint < 0)
    {
        Unimplemented(); // FUN_FLD_C8__0607d684 with data at 0608AA00
    }
    else
    {
        Unimplemented(); // FUN_FLD_C8__060542e6 with data at 0608A880
    }

    Unimplemented(); // random battle init (0x32)
}
