#include "PDS.h"
#include "exitField.h"
#include "field_a3/o_fld_a3.h"
#include "field/fieldDragonMovement.h"

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
        updateDragonCollision(r4);
        updateCameraScriptSub0Sub2(r4);
        r4->m104_dragonScriptStatus++;
        [[fallthrough]];
    case 1:
        buildDragonRotationMatrix(&r4->m48, &r4->m20_angle);
        copyMatrix(&r4->m48.m0_matrix, &r4->m88_matrix);
        r4->m8_pos += r4->m160_deltaTranslation;
        break;
    default:
        assert(0);
        break;
    }

    updateDragonCollision(r4);
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
            r14->m160_deltaTranslation[1] = intDivide(pThis->m10_length, pThis->m0_pScript->m0_position[1] - r14->m8_pos[1]);
            r14->m160_deltaTranslation[2] = MTH_Mul(-pThis->m0_pScript->m1C, getCos(r14->m20_angle[1].getInteger() & 0xFFF));

            r14->m8_pos[0] = pThis->m0_pScript->m0_position[0] - ((pThis->m10_length + 5) * r14->m160_deltaTranslation[0]);
            r14->m8_pos[1] = pThis->m0_pScript->m0_position[1] - ((pThis->m10_length + 5) * r14->m160_deltaTranslation[1]);
            r14->m8_pos[2] = pThis->m0_pScript->m0_position[2] - ((pThis->m10_length + 5) * r14->m160_deltaTranslation[2]);

            r14->mF0 = DragonUpdateCutscene;
            r14->m108 = 0;
            r14->mF0(r14);

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

// 0607cdcc (A5) / 060751d8 (A3) — start exit cutscene with cached cutscene data
static s_cutsceneData* s_cachedCutsceneData = nullptr;

void startExitCutsceneCached(sSaturnPtr pEffectData, s32 fieldIndex, s32 fieldParam, s32 exitIndex, s16 arg0)
{
    if (!s_cachedCutsceneData)
    {
        s_cachedCutsceneData = loadCutsceneData(pEffectData);
    }
    startExitFieldCutscene2Sub0(nullptr, s_cachedCutsceneData, fieldIndex, fieldParam, exitIndex, arg0);
}

// 0607ce18 (A5) / 06075224 (A3) — start exit cutscene using current field index
void startExitCutsceneForCurrentField(sSaturnPtr pEffectData, s32 param3, s32 param4, s16 param5)
{
    s_cutsceneData* pData = loadCutsceneData(pEffectData);
    startExitFieldCutscene2Sub0(nullptr, pData, getFieldTaskPtr()->m2C_currentFieldIndex, param3, param4, param5);
}

// 06073500 (A7) / 06074f34 (A3) — camera script cutscene variant (reads from sSaturnPtr)
void startCameraScriptCutscene(p_workArea parent, const sSaturnPtr& scriptDataEA,
                                s32 param3, s32 param4, s16 param5)
{
    s_exitCutsceneTask* pTask = createSubTaskFromFunction<s_exitCutsceneTask>(parent, &s_exitCutsceneTask::Update);
    if (pTask != nullptr)
    {
        s_cameraScript* pScript = readCameraScript(scriptDataEA);
        s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
        pTask->m0_pScript = pScript;
        pTask->m8_param = param3;
        pTask->mC_exitNumber = param4;
        pTask->m14 = param5;
        pDragon->m1D0_cameraScript = pScript;
        pDragon->mF8_Flags &= ~0x400;
    }
}
