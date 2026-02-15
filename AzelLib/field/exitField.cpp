#include "PDS.h"
#include "exitField.h"
#include "field_a3/o_fld_a3.h"

// Used when dragon is flying away to change field
// Example is the waterfall at the end of A3_2

void DragonUpdateCutscene(s_dragonTaskWorkArea* r4)
{
    r4->m24A_runningCameraScript = 3;
    getFieldTaskPtr()->m28_status |= 0x10000;
    getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m8 |= 1;
    switch (r4->m104_dragonScriptStatus)
    {
    case 0:
        updateCameraScriptSub0(r4->mB8_lightWingEffect);
        r4->mF8_Flags &= ~0x400;
        r4->mF8_Flags |= 0x20000;
        dragonFieldTaskInitSub4Sub6(r4);
        updateCameraScriptSub0Sub2(r4);
        r4->m104_dragonScriptStatus++;
        [[fallthrough]];
    case 1:
        dragonFieldTaskInitSub4Sub5(&r4->m48, &r4->m20_angle);
        copyMatrix(&r4->m48.m0_matrix, &r4->m88_matrix);
        r4->m8_pos += r4->m160_deltaTranslation;
        break;
    default:
        assert(0);
        break;
    }

    dragonFieldTaskInitSub4Sub6(r4);
}

struct s_exitCutsceneTask : public s_workAreaTemplate<s_exitCutsceneTask>
{
    static void Update(s_exitCutsceneTask* pThis)
    {
        s_dragonTaskWorkArea* r14 = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;

        switch (pThis->m16_state)
        {
        case 0:
            getFieldCameraStatus()->m0_position = pThis->m0_pScript->m0_position;
            dragonFieldTaskInitSub4Sub3(0);
            pThis->m10_length = pThis->m0_pScript->m20_length;
            r14->m8_pos[1] = pThis->m0_pScript->m18;
            r14->m20_angle = pThis->m0_pScript->mC_rotation;

            r14->m160_deltaTranslation[0] = MTH_Mul(-pThis->m0_pScript->m1C, getSin(r14->m20_angle[1].getInteger() & 0xFFF));
            r14->m160_deltaTranslation[1] = performDivision(pThis->m10_length, pThis->m0_pScript->m0_position[1] - r14->m8_pos[1]);
            r14->m160_deltaTranslation[2] = MTH_Mul(-pThis->m0_pScript->m1C, getCos(r14->m20_angle[1].getInteger() & 0xFFF));

            r14->m8_pos[0] = pThis->m0_pScript->m0_position[0] - ((pThis->m10_length + 5) * r14->m160_deltaTranslation[0]);
            r14->m8_pos[1] = pThis->m0_pScript->m0_position[1] - ((pThis->m10_length + 5) * r14->m160_deltaTranslation[1]);
            r14->m8_pos[2] = pThis->m0_pScript->m0_position[2] - ((pThis->m10_length + 5) * r14->m160_deltaTranslation[2]);

            r14->mF0 = DragonUpdateCutscene;
            r14->m108 = 0;
            r14->mF4(r14);

            pThis->m16_state++;
            break;
        case 1:
            if (--pThis->m10_length <= 0)
            {
                exitCutsceneTaskUpdateSub0(pThis->m8_param, pThis->mC_exitNumber, pThis->m14);
                pThis->m16_state++;
            }
        case 2:
            break;
        default:
            assert(0);
            break;
        }
    }

    // size 0x18
    s_cameraScript* m0_pScript;
    s32 m8_param;
    s32 mC_exitNumber;
    s32 m10_length;
    s32 m14;
    s8 m16_state;
};

void startExitFieldCutscene(p_workArea parent, s_cameraScript* pScript, s32 param, s32 exitIndex, s32 arg0)
{
    s_exitCutsceneTask* pNewTask = createSubTaskFromFunction<s_exitCutsceneTask>(parent, &s_exitCutsceneTask::Update);

    s_dragonTaskWorkArea* pDragonTask = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;

    pNewTask->m0_pScript = pScript;
    pNewTask->m8_param = param;
    pNewTask->mC_exitNumber = exitIndex;
    pNewTask->m14 = arg0;


    pDragonTask->m1D0_cameraScript = pScript;
    pDragonTask->mF8_Flags &= ~0x400;
}
