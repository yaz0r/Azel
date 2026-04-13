#include "PDS.h"
#include "o_fld_a7.h"
#include "a7_skyTransporter.h"
#include "a7_delayedExitTrigger.h"
#include "a7_cameraScriptSubtask.h"
#include "a7_effectTask.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldModelRender.h"
#include "kernel/fileBundle.h"
#include "3dModels.h"
#include "field/fieldVisibilityGrid.h"
#include "audio/systemSounds.h"

// Camera shake rotation impulse table (06084228) — 4 entries of sVec3_FP, cycled per frame
static const sVec3_FP cameraShakeRotations[] = {
    { fixedPoint(0xFFEEEEEF), fixedPoint(0), fixedPoint(0) },
    { fixedPoint(0x00111111), fixedPoint(0), fixedPoint(0) },
    { fixedPoint(0xFFF6E5D5), fixedPoint(0), fixedPoint(0) },
    { fixedPoint(0x00091A2B), fixedPoint(0), fixedPoint(0) },
};

// Script index/param lookup table (06084294) — 9 entries
static const struct { s16 scriptIndex; s16 scriptParam; } a7ScriptLookup[] = {
    { 0x04, 0x05CC }, { 0x05, 0x05CD }, { 0x06, 0x05CE }, { 0x07, 0x05CF },
    { 0x08, 0x05D0 }, { 0x09, 0x05D1 }, { 0x0A, 0x05D2 }, { 0x0B, 0x05D3 },
    { 0x0C, 0x05D4 },
};

struct s_skyTransporter;
static void skyTransporter_RenderCallback(p_workArea pWorkArea, sFieldModelRenderContext* pCtx);

struct s_skyTransporter : public s_workAreaTemplate<s_skyTransporter>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { NULL, &Update, &Draw, NULL };
        return &taskDefinition;
    }

    // 060547d0
    static void Update(s_skyTransporter* pThis)
    {
        s_DataTable2Sub0* pData = pThis->m8;
        s_fieldSpecificData_A7* pFieldData = getFieldSpecificData_A7();

        // 0606e2f4 — visibility check
        s32 vis = checkPositionVisibilityAgainstFarPlane(&pData->m4_position);
        pThis->m51_visible = (vis == 0);

        // 06070ca4 — empty function (no-op)

        switch (pThis->m50_state)
        {
        case 0:
            pThis->m1C_renderCtx.m18_visibilityFlags |= 1;
            pThis->m14_subtask = a7CreateEffectTask(pThis, &pData->m4_position, gFLD_A7->getSaturnPtr(0x06084258), 0);
            pFieldData->m27A = 0;
            pThis->m50_state = 1;
            break;
        case 1:
            pFieldData->m274 += 0x20;
            pThis->m10_rotation = (s16)(pData->m10_rotation[1] + pFieldData->m274);
            if ((pFieldData->m274 & 0xFFF) == 0)
            {
                pThis->m18_revolutions++;
                if (pThis->m18_revolutions == 4)
                {
                    pThis->m1C_renderCtx.m18_visibilityFlags = 0;
                    if (pThis->m14_subtask)
                    {
                        // mark particle task's m0C byte to signal completion
                        *((u8*)pThis->m14_subtask + 0x2C) = 1;
                    }
                    pFieldData->m27A = 1;
                    pThis->m50_state = 2;
                }
            }
            break;
        case 2:
            if (pFieldData->m277_segmentsReturned == 8)
            {
                a7CreateEffectTask(pThis, &pData->m4_position, gFLD_A7->getSaturnPtr(0x06084258), 0);
                playSystemSoundEffect(0x71);
                pThis->m50_state = 3;
            }
            break;
        case 3:
            pFieldData->m274 += 0x20;
            pThis->m10_rotation = (s16)(pData->m10_rotation[1] + pFieldData->m274);
            break;
        case 4:
            if (isScriptActive() != 0)
            {
                pThis->m50_state = 5;
            }
            break;
        case 5:
            pFieldData->m274 += 0x20;
            pThis->m10_rotation = (s16)(pData->m10_rotation[1] + pFieldData->m274);
            pThis->mC_counter++;
            addCameraImpulse(nullptr, (sVec3_FP*)&cameraShakeRotations[pThis->mC_counter & 3]);
            if (isScriptActive() == 0)
            {
                pThis->m50_state = 6;
            }
            break;
        case 6:
            pFieldData->m274 += 0x20;
            pThis->m10_rotation = (s16)(pData->m10_rotation[1] + pFieldData->m274);
            mainGameState.bitField[0xa3] |= 4;
            playSystemSoundEffect(0x74);
            a7StartCameraScript(pThis, gFLD_A7->getSaturnPtr(0x06084260), 2, 0, -1);
            a7CreateDelayedExitTrigger(pThis);
            pThis->m50_state = 7;
            break;
        case 7:
            // idle — only model render context update below
            break;
        default:
            assert(0);
            break;
        }

        updateFieldModelRenderContext(&pThis->m1C_renderCtx);

        // Keyboard interaction check (debug trigger)
        s_fieldTaskWorkArea* pFieldTask = getFieldTaskPtr();
        if (((pFieldTask->m8_pSubFieldData->m370_fieldDebuggerWho & 2) != 0)
            && (pFieldTask->m8_pSubFieldData->m37C_debugMenuStatus1[1] == 0)
            && (pFieldTask->m8_pSubFieldData->m369 == 0)
            && readKeyboardToggle(0xC3))
        {
            a7CreateEffectTask(pThis, &pData->m4_position, gFLD_A7->getSaturnPtr(0x06084258), 0);
            pFieldData->m276 = 8;
            pFieldData->m277_segmentsReturned = 8;
        }
    }

    // 060546cc
    static void Draw(s_skyTransporter* pThis)
    {
        if (!pThis->m51_visible)
            return;

        s_DataTable2Sub0* pData = pThis->m8;
        pushCurrentMatrix();
        translateCurrentMatrix(&pData->m4_position);
        rotateCurrentMatrixY(pThis->m10_rotation);

        s16 modelHierarchyOffset = readSaturnS16(pData->m1C_modelData);
        s16 modelPoseOffset = readSaturnS16(pData->m1C_modelData + 2);
        LCSItemBox_DrawType0Sub0(pThis->m0.m0_mainMemoryBundle, modelHierarchyOffset, modelPoseOffset);
        callGridCellDraw_normalSub2(pThis->m0.m0_mainMemoryBundle, 0x214);
        popMatrix();
    }

    s_memoryAreaOutput m0;               // 0
    s_DataTable2Sub0* m8;                // 8
    s32 mC_counter;                      // C
    s16 m10_rotation;                    // 10
    s16 m12_pad;                         // 12
    p_workArea m14_subtask;              // 14
    s32 m18_revolutions;                 // 18
    sFieldModelRenderContext m1C_renderCtx; // 1C (0x34 bytes)
    u8 m50_state;                        // 50
    u8 m51_visible;                      // 51
    // size 0x54
};

// 06054724 — model render context callback (interaction handler)
static void skyTransporter_RenderCallback(p_workArea pWorkArea, sFieldModelRenderContext* pCtx)
{
    s_fieldSpecificData_A7* pFieldData = getFieldSpecificData_A7();
    s32 index = pFieldData->m277_segmentsReturned;
    s32 scriptIndex = a7ScriptLookup[index].scriptIndex;
    s32 scriptParam = a7ScriptLookup[index].scriptParam;

    if (scriptIndex == 0x0C)
    {
        startFieldScript(scriptIndex, scriptParam);
    }
    else
    {
        startFieldScriptWithFlagCheck(scriptIndex, scriptParam);
    }

    if (pFieldData->m277_segmentsReturned == 8)
    {
        s_skyTransporter* pThis = (s_skyTransporter*)pWorkArea;
        if ((mainGameState.bitField[0xa3] & 4) == 0)
        {
            pThis->m50_state = 4;
        }
        else
        {
            pThis->m50_state = 6;
        }
    }

    mainGameState.bitField[0x59] |= 4;
}

// 06054a8c
void create_skyTransporter(s_visdibilityCellTask* r4, s_DataTable2Sub0& r5, s32 r6)
{
    s_fieldSpecificData_A7* pFieldData = getFieldSpecificData_A7();

    s_visibilityGridWorkArea* pVisGrid = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;
    s_skyTransporter* pTask = createSubTask<s_skyTransporter>((p_workArea)pVisGrid->m38);
    if (pTask)
    {
        getMemoryArea(&pTask->m0, r6);
        pTask->m8 = &r5;
        pFieldData->m274 = 0;
        pTask->m10_rotation = r5.m10_rotation[1];
        pTask->mC_counter = 0;
        pTask->m12_pad = 0;
        pTask->m14_subtask = nullptr;
        pTask->m18_revolutions = 0;

        initFieldModelRenderContext(&pTask->m1C_renderCtx, pTask,
            (void*)&skyTransporter_RenderCallback,
            &r5.m4_position, nullptr, 0, 0, -1, 0, 0);

        if (getFieldTaskPtr()->m2C_currentFieldIndex == 0x16)
        {
            pTask->m50_state = 2;
            pFieldData->m277_segmentsReturned = 8;
        }
        else
        {
            if ((mainGameState.bitField[0x74] & 0x80) == 0)
            {
                pTask->m50_state = 0;
            }
            else
            {
                pTask->m50_state = 2;
            }
        }
    }
}
