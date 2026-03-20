#include "PDS.h"
#include "o_fld_c8.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldRadar.h"
#include "audio/soundDriver.h"

static void fieldC8_1_startTasks(p_workArea workArea) { Unimplemented(); }

// 06054502
void subfieldC8_1(p_workArea workArea)
{
    // 0605f026 — create VDP2 task (different from subfield 0)
    Unimplemented(); // creates task with definition at 060b2ab4

    // 060544e0 — shared subfield init
    {
        // 06054274 — clip distances
        graphicEngineStatus.m405C.m10_nearClipDistance = 0x3000;
        graphicEngineStatus.m405C.m30_oneOverNearClip = FP_Div(0x10000, graphicEngineStatus.m405C.m10_nearClipDistance);

        s_fieldSpecificData_C8* pFieldData = (s_fieldSpecificData_C8*)getFieldTaskPtr()->mC;
        if (pFieldData && pFieldData->m25 == 0)
        {
            graphicEngineStatus.m405C.m14_farClipDistance = 0x258000;
            graphicEngineStatus.m405C.m38_oneOverFarClip = FP_Div(0x8000, graphicEngineStatus.m405C.m14_farClipDistance);
            graphicEngineStatus.m405C.m34_oneOverFarClip256 = graphicEngineStatus.m405C.m38_oneOverFarClip << 8;
        }

        // 06055bdc — camera config
        setupFieldCameraConfigs(readCameraConfig(gFLD_C8->getSaturnPtr(0x0608B11C)), 1);
        getFieldTaskPtr()->m8_pSubFieldData->m334->m50E = 1;
        // 0607b4e0
        fieldRadar_setEncounterDistance(fixedPoint(0x200000));

        // 06079e26 — setupFieldC8 with visibility grid
        setupFieldC8(gFLD_C8->getSaturnPtr(0x06090120), gFLD_C8->getSaturnPtr(0x0608FEE0), fieldC8_1_startTasks);

        // 0607d8ca — adjustVerticalLimits
        adjustVerticalLimits(0, 0);

        // 0607baa0 — random battle init
        fieldRadar_initRandomBattle(0x37);
    }

    // 06057f1a — set field-specific data flags
    {
        s_fieldSpecificData_C8* pFieldData = (s_fieldSpecificData_C8*)getFieldTaskPtr()->mC;
        if (pFieldData)
        {
            pFieldData->m25 = 0;
            pFieldData->m27 = 1;
        }
    }

    Unimplemented(); // remaining subfield 1 init (camera setup, dragon position, etc.)
}
