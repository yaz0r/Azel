#include "PDS.h"
#include "a7_cellObj1.h"
#include "o_fld_a7.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldModelRender.h"
#include "field/fieldVisibilityGrid.h"
#include "kernel/fileBundle.h"
#include "3dModels.h"
#include "audio/systemSounds.h"
#include "trigo.h"
#include "a7_sceneParticle.h"
#include "kernel/vdp1AnimatedQuad.h"
#include <vector>

extern s32 playBattleSoundEffect(s32 effectIndex);

// Lazy-parsed VDP1 quad template used by cellObj1's trail spawn helper.
static const std::vector<sVdp1Quad>* a7CellObj1_getTrailQuadList()
{
    static std::vector<sVdp1Quad> s_quadList;
    if (s_quadList.empty())
    {
        s_quadList = initVdp1Quad(gFLD_A7->getSaturnPtr(0x06086280));
    }
    return &s_quadList;
}


// Cell object type 1 (FLD_A7 overlay, size 0x68).
// Creator at 0605ccd0, update at 0605cac0, draw at 0605c96e.
struct s_A7_CellObj1 : public s_workAreaTemplate<s_A7_CellObj1>
{
    s_memoryAreaOutput        m0;              // 0x00
    s_DataTable2Sub0*         m8_pData;        // 0x08
    sVec3_FP                  mC_pos;          // 0x0C
    sVec3_FP                  m18_velocity;    // 0x18
    s32                       m24_radius;      // 0x24 (BE packed {0, 5, 0x8E, 0x30})
    s16                       m28_angleInit;   // 0x28
    s16                       m2A_angle;       // 0x2A
    s32                       m2C_counter;     // 0x2C
    sFieldModelRenderContext  m30_modelCtx;    // 0x30 (size 0x34)
    u8                        m64_state;       // 0x64
    u8                        m65_visible;     // 0x65
    // size 0x68
};

// 0605c650 — look up a bit index from the classification table at 06086260
// and return the tested bit from mainGameState.bitField.
static s32 a7CellObj1_classify_0605c650(s32 index)
{
    s16 bitIndex = readSaturnS16(gFLD_A7->getSaturnPtr(0x06086260) + index * 2);
    u32 adjusted = (bitIndex < 1000) ? (u32)bitIndex : (u32)(bitIndex - 0x236);
    return (s32)(s8)(bitMasks[adjusted & 7] & mainGameState.bitField[adjusted >> 3]);
}

// 0605c828 — compute 2D velocity aimed toward anchor (0x5FFFFF, _, -0x5FFFFF)
static void a7CellObj1_aimAtAnchor_0605c828(s_A7_CellObj1* pThis)
{
    s32 dx =  0x5FFFFF - pThis->mC_pos.m0_X.m_value;
    s32 dz = -0x5FFFFF - pThis->mC_pos.m8_Z.m_value;

    s32 angle = atan2_FP(dx, dz);
    u16 idx = (u16)((u32)angle >> 16) & 0xFFF;
    pThis->m18_velocity.m0_X = MTH_Mul(fixedPoint(0x2F68), getSin(idx));
    pThis->m18_velocity.m4_Y = fixedPoint(0);
    pThis->m18_velocity.m8_Z = MTH_Mul(fixedPoint(0x2F68), getCos(idx));

    sVec2_FP delta;
    delta[0] = fixedPoint(dx);
    delta[1] = fixedPoint(dz);
    fixedPoint lenSq = MulVec2(delta, delta);
    fixedPoint len = sqrt_F(lenSq);
    pThis->m2C_counter = FP_Div(0x2F68, len).m_value;
}

// 0605c8dc — copy seed pos from pData, aim velocity at anchor, fast-forward
// position by velocity until the counter drops into the final 0x1E window.
static void a7CellObj1_flipVelocity_0605c8dc(s_A7_CellObj1* pThis)
{
    pThis->mC_pos = pThis->m8_pData->m4_position;
    a7CellObj1_aimAtAnchor_0605c828(pThis);
    s32 i = pThis->m2C_counter;
    do
    {
        i--;
        pThis->mC_pos.m0_X = fixedPoint(pThis->mC_pos.m0_X.m_value + pThis->m18_velocity.m0_X.m_value);
        pThis->mC_pos.m8_Z = fixedPoint(pThis->mC_pos.m8_Z.m_value + pThis->m18_velocity.m8_Z.m_value);
    } while (i > 0x1E);
}

// 0605c918 — per-frame orbital update around the anchor (0x5FFFFF, _, -0x5FFFFF)
static void a7CellObj1_perFrame_0605c918(s_A7_CellObj1* pThis)
{
    s_fieldSpecificData_A7* pFieldData = getFieldSpecificData_A7();
    pThis->m2A_angle = (s16)(pThis->m28_angleInit + pFieldData->m274);
    u16 idx = (u16)pThis->m2A_angle & 0xFFF;
    pThis->mC_pos.m0_X = fixedPoint(MTH_Mul(fixedPoint(pThis->m24_radius), -getSin(idx)).m_value + 0x5FFFFF);
    pThis->mC_pos.m8_Z = fixedPoint(MTH_Mul(fixedPoint(pThis->m24_radius), -getCos(idx)).m_value - 0x5FFFFF);
}

// 0605cdcc — spawn a trail particle at a jittered forward offset from pPos
static void a7CellObj1_spawnTrail_0605cdcc(s_A7_CellObj1* /*pThis*/, sVec3_FP* pPos, sVec3_FP* pVel, s32 radius)
{
    s32 angle = atan2_FP(pVel->m0_X.asS32(), pVel->m8_Z.asS32());
    u16 idx = (u16)((u32)(angle + 0x8000000) >> 16) & 0xFFF;
    s32 dx = MTH_Mul(fixedPoint(radius), getSin(idx)).m_value;
    s32 dz = MTH_Mul(fixedPoint(radius), getCos(idx)).m_value;

    sVec3_FP pos;
    pos.m0_X = fixedPoint(dx + pPos->m0_X.m_value + a7CenteredRandom((u32)(radius >> 1)));
    pos.m4_Y = pPos->m4_Y;
    pos.m8_Z = fixedPoint(dz + pPos->m8_Z.m_value + a7CenteredRandom((u32)(radius >> 1)));

    sVec3_FP vel;
    vel.m0_X = fixedPoint(pVel->m0_X.m_value >> 2);
    vel.m4_Y = fixedPoint(pVel->m4_Y.m_value >> 2);
    vel.m8_Z = fixedPoint(pVel->m8_Z.m_value >> 2);

    sA7SceneParticleDesc desc = {};
    desc.m8_pQuadList = a7CellObj1_getTrailQuadList();

    sFieldSceneManager* pManager = (sFieldSceneManager*)getFieldSpecificData_A7()->m280;
    a7SceneParticle_spawnProjected(pManager, &desc, &pos, &vel);
}

// 0605c686 — set or clear a mainGameState bit for the classification-table
// index. Uses the same table (06086260) as a7CellObj1_classify_0605c650.
static void a7CellObj1_stateAction_0605c686(s32 index, s32 set)
{
    s16 bitIndex = readSaturnS16(gFLD_A7->getSaturnPtr(0x06086260) + index * 2);
    u32 adjusted = (bitIndex < 1000) ? (u32)bitIndex : (u32)(bitIndex - 0x236);
    if (set == 0)
    {
        mainGameState.bitField[adjusted >> 3] &= reverseBitMasks[adjusted & 7];
    }
    else
    {
        mainGameState.bitField[adjusted >> 3] |= bitMasks[adjusted & 7];
    }
}

// 0605cac0 — update state machine
static void a7CellObj1_Update(s_A7_CellObj1* pThis)
{
    s_DataTable2Sub0* pData = pThis->m8_pData;
    s32 vis = checkPositionVisibilityAgainstFarPlane(&pThis->mC_pos);
    pThis->m65_visible = (vis == 0);

    s_fieldSpecificData_A7* pFieldData = getFieldSpecificData_A7();

    switch (pThis->m64_state)
    {
    case 0:
        pThis->m30_modelCtx.m18_visibilityFlags |= 1;
        a7CellObj1_flipVelocity_0605c8dc(pThis);
        pThis->m18_velocity.m0_X = fixedPoint(-pThis->m18_velocity.m0_X.m_value);
        pThis->m18_velocity.m8_Z = fixedPoint(-pThis->m18_velocity.m8_Z.m_value);
        pFieldData->m27B++;
        pThis->m64_state++;
        break;

    case 1:
        a7CellObj1_perFrame_0605c918(pThis);
        if (pFieldData->m27A != 0)
        {
            playSystemSoundEffect(0x70);
            pThis->m64_state++;
        }
        break;

    case 2:
        pThis->mC_pos.m0_X = fixedPoint(pThis->mC_pos.m0_X.m_value + pThis->m18_velocity.m0_X.m_value);
        pThis->mC_pos.m8_Z = fixedPoint(pThis->mC_pos.m8_Z.m_value + pThis->m18_velocity.m8_Z.m_value);
        pThis->m2C_counter--;
        if (pThis->m2C_counter >= 0x1E)
        {
            if ((pThis->m2C_counter & 7) == 0)
            {
                a7CellObj1_spawnTrail_0605cdcc(pThis, &pThis->mC_pos, &pThis->m18_velocity, 0x17FFF);
            }
        }
        else
        {
            pThis->m30_modelCtx.m18_visibilityFlags = 0;
            pFieldData->m278--;
            if ((s8)pFieldData->m278 <= 0)
            {
                playBattleSoundEffect(0x70);
            }
            if (pThis->m65_visible != 0)
            {
                playSystemSoundEffect(0x75);
            }
            pThis->mC_pos = pData->m4_position;
            pThis->m64_state++;
        }
        break;

    case 3:
        // Stall state — waits for an external bump of m64_state.
        break;

    case 4:
        pThis->mC_pos.m0_X = fixedPoint(pThis->mC_pos.m0_X.m_value + pThis->m18_velocity.m0_X.m_value);
        pThis->mC_pos.m8_Z = fixedPoint(pThis->mC_pos.m8_Z.m_value + pThis->m18_velocity.m8_Z.m_value);
        pThis->m2C_counter--;
        if (pThis->m2C_counter < 0x1E)
        {
            pFieldData->m278--;
            if ((s8)pFieldData->m278 <= 0)
            {
                playBattleSoundEffect(0x70);
            }
            playSystemSoundEffect(0x75);
            pThis->m64_state++;
        }
        else if ((pThis->m2C_counter & 7) == 0 && pThis->m2C_counter > 0x2E)
        {
            a7CellObj1_spawnTrail_0605cdcc(pThis, &pThis->mC_pos, &pThis->m18_velocity, 0x17FFF);
        }
        break;

    case 5:
        a7CellObj1_stateAction_0605c686((s32)pData->m18, 1);
        pFieldData->m277++;
        pThis->m64_state++;
        if (pFieldData->m277 == 8)
        {
            pThis->m64_state++;
        }
        break;

    case 6:
        if (pFieldData->m277 == 8)
        {
            pThis->m64_state++;
        }
        break;

    case 7:
        a7CellObj1_perFrame_0605c918(pThis);
        break;

    default:
        assert(0);
        break;
    }

    updateFieldModelRenderContext(&pThis->m30_modelCtx);
}

// 0605c96e — draw
static void a7CellObj1_Draw(s_A7_CellObj1* pThis)
{
    if (pThis->m65_visible != 0)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->mC_pos);
        rotateCurrentMatrixY(pThis->m2A_angle);
        addObjectToDrawList(pThis->m0.m0_mainMemoryBundle->get3DModel(0xA0));
        callGridCellDraw_normalSub2(pThis->m0.m0_mainMemoryBundle, 0xA4);
        popMatrix();
    }
}

// 0605ccd0
void create_A7_CellObj1(s_visdibilityCellTask* r4, s_DataTable2Sub0& r5, s32 r6)
{
    static const s_A7_CellObj1::TypedTaskDefinition definition = {
        nullptr,
        &a7CellObj1_Update,
        &a7CellObj1_Draw,
        nullptr,
    };

    s_visibilityGridWorkArea* pVisGrid = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;
    s_A7_CellObj1* pTask = createSubTask<s_A7_CellObj1>((p_workArea)pVisGrid->m38, &definition);
    if (pTask == nullptr)
    {
        return;
    }

    getMemoryArea(&pTask->m0, r6);
    pTask->m8_pData = &r5;

    pTask->mC_pos = r5.m4_position;

    s16 angle = (r5.m10_rotation[0] == 0)
        ? (s16)r5.m10_rotation[1]
        : (s16)(0x800 - (s16)r5.m10_rotation[1]);
    pTask->m28_angleInit = angle;
    pTask->m2A_angle = angle;

    pTask->m24_radius = 0x00058E30;

    initFieldModelRenderContext(&pTask->m30_modelCtx, pTask, (void*)0x0605C9B4,
        &pTask->mC_pos, nullptr, 0, 0, -1, 0, 0);

    s32 classify = a7CellObj1_classify_0605c650((s32)r5.m18);
    if (classify == 0 && getFieldTaskPtr()->m2C_currentFieldIndex != 0x16)
    {
        pTask->m64_state = (mainGameState.bitField[0x74] & 0x80) ? 3 : 0;
    }
    else
    {
        a7CellObj1_flipVelocity_0605c8dc(pTask);
        pTask->m30_modelCtx.m18_visibilityFlags |= 1;
        pTask->m64_state = 6;
    }
}
