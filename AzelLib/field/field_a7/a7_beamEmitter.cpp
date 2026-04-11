#include "PDS.h"
#include "a7_beamEmitter.h"
#include "a7_beamChargeWobble.h"
#include "a7_beamSpark.h"
#include "a7_beam.h"
#include "a7_cellObj3.h"
#include "o_fld_a7.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldModelRender.h"
#include "field/fieldVisibilityGrid.h"
#include "kernel/fileBundle.h"
#include "3dModels.h"
#include "audio/systemSounds.h"

// Cell object type 2 (FLD_A7 overlay, size 0x78)
// Creator at 06055da6, update at 06055bfe, draw at 06055d3c
struct sA7BeamEmitter : public s_workAreaTemplate<sA7BeamEmitter>
{
    s_memoryAreaOutput m0;              // 0x00
    s_DataTable2Sub0* m8_pData;         // 0x08
    sVec3_FP mC_pos1;                   // 0x0C
    u16 m18_counter;                    // 0x18
    u8 m1A_animTick;                    // 0x1A
    u8 m1B_visible;                     // 0x1B
    s16 m1C_rotY;                       // 0x1C
    s16 m1E;                            // 0x1E
    sVec3_FP m20_pos2;                  // 0x20
    u8 m2C;                             // 0x2C
    u8 m2D;                             // 0x2D
    u8 m2E;                             // 0x2E
    u8 m2F;                             // 0x2F
    sSaturnPtr m30;                     // 0x30 (Saturn 4 bytes)
    sSaturnPtr m34;                     // 0x34 (Saturn 4 bytes)
    sSaturnPtr m38;                     // 0x38 (Saturn 4 bytes)
    u8 m3C;                             // 0x3C
    sFieldModelRenderContext m40_modelCtx;  // 0x40 (size 0x34)
    u8 m74_destroyed;                   // 0x74
    // size 0x78
};

// 06055bfe
static void a7BeamEmitter_Update_06055bfe(sA7BeamEmitter* pThis)
{
    s_DataTable2Sub0* pData = pThis->m8_pData;
    updateFieldModelRenderContext(&pThis->m40_modelCtx);
    s32 vis = checkPositionVisibilityAgainstFarPlane(&pData->m4_position);
    pThis->m1B_visible = (vis == 0);

    if (pThis->m74_destroyed == 0)
    {
        return;
    }

    s16 counter = (s16)pThis->m18_counter;
    if (counter == 0)
    {
        sA7BeamArg trailArg;
        trailArg.m0_pos = pThis->m20_pos2;
        trailArg.mC_paramA = (u32)pThis->m2C | ((u32)pThis->m2D << 8)
                           | ((u32)pThis->m2E << 16) | ((u32)pThis->m2F << 24);
        trailArg.m10_paramB = pThis->m30.m_offset;
        trailArg.m14_paramC = pThis->m34.m_offset;
        trailArg.m18_paramD = pThis->m38.m_offset;
        trailArg.m1C_shape = pThis->m3C;
        a7BeamEmitter_spawnBeam_06055a38((p_workArea)pThis, &trailArg);
        pThis->m18_counter = (u16)((randomNumber() & 0x7F) + 0x60);
        return;
    }

    if (counter == 1)
    {
        sVec3_FP viewPos;
        transformAndAddVecByCurrentMatrix(&pData->m4_position, &viewPos);
        if (isPointOnScreen(&viewPos, graphicEngineStatus.m405C.m14_farClipDistance))
        {
            playSystemSoundEffect(0x73);
        }
        pThis->m18_counter = (u16)(counter - 1);
        return;
    }

    if (counter == 2)
    {
        sVec3_FP raisedPos;
        raisedPos.m0_X = pData->m4_position.m0_X;
        raisedPos.m4_Y = fixedPoint(pData->m4_position.m4_Y.m_value + 0x80000);
        raisedPos.m8_Z = pData->m4_position.m8_Z;
        a7BeamEmitter_spawnSpark_06055aea((p_workArea)pThis, &raisedPos);
        pThis->m18_counter = (u16)(counter - 1);
        pThis->m1A_animTick = 1;
        return;
    }

    if (counter == 0x3C)
    {
        if (pThis->m1B_visible == 0)
        {
            pThis->m18_counter = (u16)((randomNumber() & 0x7F) + 0x60);
        }
        else
        {
            sA7BeamChargeWobbleArg arg;
            arg.m0_pos.m0_X = pData->m4_position.m0_X;
            arg.m0_pos.m4_Y = fixedPoint(pData->m4_position.m4_Y.m_value + 0x80000);
            arg.m0_pos.m8_Z = pData->m4_position.m8_Z;
            arg.mC_templateEA = gFLD_A7->getSaturnPtr(0x06084400);
            a7BeamEmitter_spawnChargeWobble_06055ada((p_workArea)pThis, &arg);
            pThis->m18_counter = (u16)(counter - 1);
        }
        return;
    }

    // default
    if (pThis->m1A_animTick != 0)
    {
        pThis->m1A_animTick = (pThis->m1A_animTick < 0x1B) ? (pThis->m1A_animTick + 1) : 0;
    }
    pThis->m18_counter = (u16)(counter - 1);
}

// 06055d3c
static void a7BeamEmitter_Draw_06055d3c(sA7BeamEmitter* pThis)
{
    if (!pThis->m1B_visible)
        return;

    pushCurrentMatrix();
    translateCurrentMatrix(&pThis->m8_pData->m4_position);
    rotateCurrentMatrixY(pThis->m1C_rotY);

    u32 lodIndex = gridCellDraw_GetDepthRange(pCurrentMatrix->m[2][3]);
    u32 tableEA = (lodIndex == 0) ? 0x060845cc : 0x06084604;
    s16 modelOffset = readSaturnS16(gFLD_A7->getSaturnPtr(tableEA) + pThis->m1A_animTick * 2);

    addObjectToDrawList(pThis->m0.m0_mainMemoryBundle->get3DModel(modelOffset));
    callGridCellDraw_normalSub2(pThis->m0.m0_mainMemoryBundle, 0x2C0);
    popMatrix();
}

// 06055da6
void create_A7_CellObj2(s_visdibilityCellTask* r4, s_DataTable2Sub0& r5, s32 r6)
{
    if ((r5.m18 & 1) != 0)
        return;

    static const sA7BeamEmitter::TypedTaskDefinition definition = {
        nullptr,
        &a7BeamEmitter_Update_06055bfe,
        &a7BeamEmitter_Draw_06055d3c,
        nullptr,
    };

    sA7BeamEmitter* pTask = createSubTask<sA7BeamEmitter>((p_workArea)r4, &definition);
    if (pTask == nullptr)
        return;

    getMemoryArea(&pTask->m0, r6);
    pTask->m8_pData = &r5;

    pTask->m1C_rotY = (r5.m10_rotation[0] == 0)
        ? (s16)r5.m10_rotation[1]
        : (s16)(0x800 - (s16)r5.m10_rotation[1]);

    pTask->mC_pos1.m0_X = r5.m4_position.m0_X;
    pTask->mC_pos1.m4_Y = fixedPoint(r5.m4_position.m4_Y.m_value + 0x46000);
    pTask->mC_pos1.m8_Z = r5.m4_position.m8_Z;

    pTask->m20_pos2.m0_X = r5.m4_position.m0_X;
    pTask->m20_pos2.m4_Y = fixedPoint(r5.m4_position.m4_Y.m_value + 0x74000);
    pTask->m20_pos2.m8_Z = r5.m4_position.m8_Z;

    pTask->m2C = 0;
    pTask->m2D = 0;
    pTask->m2E = 0;
    pTask->m2F = 2;

    pTask->m30 = gFLD_A7->getSaturnPtr(0x06084400);
    pTask->m34 = gFLD_A7->getSaturnPtr(0x060845c0);
    pTask->m38 = gFLD_A7->getSaturnPtr(0x060845c6);
    pTask->m3C = 0;

    initFieldModelRenderContext(&pTask->m40_modelCtx, pTask, (void*)0x06055b74,
        &r5.m4_position, nullptr, 0, 0, -1, 0, 0);

    pTask->m1A_animTick = 0;

    if (isObjectDestroyedA7(r5.m18))
    {
        pTask->m40_modelCtx.m18_visibilityFlags |= 1;
        pTask->m18_counter = (u16)((randomNumber() & 0x7F) + 0x60);
        pTask->m74_destroyed = 1;
    }
    else
    {
        pTask->m74_destroyed = 0;
    }
}
