#include "PDS.h"
#include "a7_cameraScriptSubtask.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldDragon.h"
#include "field/exitField.h"
#include "trigo.h"

// Forward-declared — defined in field/exitField.cpp and shared across overlays
// that drive the dragon into a cutscene camera-follow mode.
void DragonUpdateCutscene(s_dragonTaskWorkArea* r4);

// A7 overlay's local copy of the camera-script cutscene subtask
// (shared variant lives in exitField.cpp as s_exitCutsceneTask).
struct sA7CameraScriptSubtask : public s_workAreaTemplate<sA7CameraScriptSubtask>
{
    s_cameraScript* m0_pScript;  // Saturn 0x00 (4 bytes)
    s32 m8_param3;               // Saturn 0x08
    s32 mC_param4;               // Saturn 0x0C
    s32 m10_countdown;           // Saturn 0x10 — set in state 0 from script
    s16 m14_param5;              // Saturn 0x14
    u8 m16_state;                // Saturn 0x16
    // size 0x18
};

// 060733a6 — A7 overlay's local camera-script update. Matches
// s_exitCutsceneTask::Update byte-for-byte (same struct layout, same state
// machine, same math). Preserved as its own function to respect the binary's
// function boundary.
static void a7CameraScriptSubtask_Update(sA7CameraScriptSubtask* pThis)
{
    s_dragonTaskWorkArea* r14 = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;

    switch (pThis->m16_state)
    {
    case 0:
        getFieldCameraStatus()->m0_position = pThis->m0_pScript->m0_position;
        dragonFieldTaskInitSub4Sub3(0);
        pThis->m10_countdown = pThis->m0_pScript->m20_length;
        r14->m8_pos[1] = pThis->m0_pScript->m18;
        r14->m20_angle = pThis->m0_pScript->mC_rotation;

        r14->m160_deltaTranslation[0] = MTH_Mul(-pThis->m0_pScript->m1C, getSin(r14->m20_angle[1].getInteger() & 0xFFF));
        r14->m160_deltaTranslation[1] = intDivide(pThis->m10_countdown, pThis->m0_pScript->m0_position[1] - r14->m8_pos[1]);
        r14->m160_deltaTranslation[2] = MTH_Mul(-pThis->m0_pScript->m1C, getCos(r14->m20_angle[1].getInteger() & 0xFFF));

        r14->m8_pos[0] = pThis->m0_pScript->m0_position[0] - ((pThis->m10_countdown + 5) * r14->m160_deltaTranslation[0]);
        r14->m8_pos[1] = pThis->m0_pScript->m0_position[1] - ((pThis->m10_countdown + 5) * r14->m160_deltaTranslation[1]);
        r14->m8_pos[2] = pThis->m0_pScript->m0_position[2] - ((pThis->m10_countdown + 5) * r14->m160_deltaTranslation[2]);

        r14->mF0 = DragonUpdateCutscene;
        r14->m108 = 0;
        r14->mF0(r14);

        pThis->m16_state++;
        break;
    case 1:
        if (--pThis->m10_countdown <= 0)
        {
            exitCutsceneTaskUpdateSub0(pThis->m8_param3, pThis->mC_param4, pThis->m14_param5);
            pThis->m16_state++;
        }
    case 2:
        break;
    default:
        assert(0);
        break;
    }
}

// 06073500
void a7StartCameraScript(p_workArea parent, const sSaturnPtr& scriptDataEA,
                         s32 param3, s32 param4, s16 param5)
{
    sA7CameraScriptSubtask* pTask = createSubTaskFromFunction<sA7CameraScriptSubtask>(
        parent, &a7CameraScriptSubtask_Update);
    if (pTask != nullptr)
    {
        s_cameraScript* pScript = readCameraScript(scriptDataEA);
        s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
        pTask->m0_pScript = pScript;
        pTask->m8_param3 = param3;
        pTask->mC_param4 = param4;
        pTask->m14_param5 = param5;
        pDragon->m1D0_cameraScript = pScript;
        pDragon->mF8_Flags &= ~0x400;
    }
}
