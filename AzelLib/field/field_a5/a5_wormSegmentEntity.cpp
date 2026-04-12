#include "PDS.h"
#include "o_fld_a5.h"
#include "field/fieldModelRender.h"
#include "field/fieldVisibilityGrid.h"
#include "kernel/fileBundle.h"
#include "kernel/animation.h"
#include "audio/soundDriver.h"
#include "3dEngine.h"
#include "3dModels.h"
#include "a5_gridDeferredDraw.h"
#include "field/field_a3/o_fld_a3_1.h"

extern void dispatchTutorialMultiChoiceSub2();

static inline s32 performDivision(s32 divisor, s32 dividend) { return dividend / divisor; }

s32 playBattleSoundEffect(s32 effectIndex);

// Worm segment entity (Saturn size 0xAC). Spawned per Saturn data record
// describing position/rotation and optional loaded 3D model. Handles the
// state machine that reveals/activates a worm segment and triggers
// subfield changes when the dragon crosses it.
struct sA5WormSegmentEntity : public s_workAreaTemplateWithArg<sA5WormSegmentEntity, sSaturnPtr>
{
    s_memoryAreaOutput m0_memoryArea;
    sSaturnPtr m8_dataPtr;
    sFieldModelRenderContext mC_modelCtx;
    s_3dModel m40_3dModel;
    sVec3_FP m90_normal;
    s32 m9C_config;
    s32 mA0_state;
    s32 mA4_pad;
    s16 mA8_modelIdx;
    s16 mAA_poseIdx;
    // Saturn size 0xAC
};

// 06077304 — forwards to the shared deferred-draw helper with the
// worm segment's owning bundle.
static void a5_dispatchWormCollision_06077304(sA5WormSegmentEntity* pThis, s16 entryKey)
{
    callGridCellDraw_normalSub2(pThis->m0_memoryArea.m0_mainMemoryBundle, entryKey);
}

// 06056EFC
static void a5WormSegmentEntity_Draw(sA5WormSegmentEntity* pThis)
{
    sSaturnPtr data = pThis->m8_dataPtr;
    if (readSaturnS16(data + 0x18) == 0)
        return;

    pushCurrentMatrix();

    sVec3_FP pos = readSaturnVec3(data);
    translateCurrentMatrix(&pos);

    sVec3_FP rot = { fixedPoint(readSaturnS32(data + 0xC)),
                     fixedPoint(readSaturnS32(data + 0x10)),
                     fixedPoint(readSaturnS32(data + 0x14)) };
    rotateCurrentMatrixZYX(&rot);

    // FUN_FLD_A5__06078fa6 is an overlay-local copy of gridCellDraw_GetDepthRange.
    u32 lod = gridCellDraw_GetDepthRange(pCurrentMatrix->m[2][3]);
    if (lod < 2)
    {
        // data[0x18] or data[0x1A] depending on LOD — a s16 bundle offset
        // to the sProcessed3dModel that should be drawn at this range.
        s16 modelOffset = readSaturnS16(data + 0x18 + lod * 2);
        sProcessed3dModel* pModel = pThis->m0_memoryArea.m0_mainMemoryBundle->get3DModel(modelOffset);
        addObjectToDrawList(pModel);
        a5_dispatchWormCollision_06077304(pThis, readSaturnS16(data + 0x1C));
    }

    popMatrix();
}

// 06056a22 — "unlock" transition helper fired by DrawWithModel the first
// frame the game-state bit trips from 0 → 1. Not yet reimplemented.
static void a5WormSegmentEntity_runUnlockTransition_06056A22(sA5WormSegmentEntity* /*pThis*/)
{
    Unimplemented();
}

// 06069490 — returns a pointer to the currently-active sFieldCameraStatus
// slot (`&m3E4_cameraSlots[m50C_activeCameraSlot]`). Each slot is 0x94
// bytes so the Ghidra arithmetic `m334 + 0x3E4 + m50C * 0x94` picks the
// right element.
static sFieldCameraStatus* a5_wormSegmentEntity_getActiveCameraSlot_06069490()
{
    sFieldCameraManager* pCam = getFieldTaskPtr()->m8_pSubFieldData->m334;
    return &pCam->m3E4_cameraSlots[pCam->m50C_activeCameraSlot];
}

// 060694d8 — tail-calls FUN_FLD_A5__0606932e(0, tableA[i], tableB[i])
// and then clears byte 0x8d of the follow-mode config returned by
// 06069490. Used to kick the "autopilot" follow mode. Not yet
// reimplemented.
void a5_wormSegmentEntity_startFollowMode_060694D8(s32 /*followMode*/)
{
    Unimplemented();
}

// 060882e4 — dragon task update function pointer installed when the
// worm segment autopilot starts. Drives the scripted camera/dragon
// motion through the tunnel. Not reimplemented yet.
extern "C" void FUN_FLD_A5__060882e4(s_dragonTaskWorkArea*);
static void a5_wormSegmentEntity_dragonAutopilotUpdate_060882E4(s_dragonTaskWorkArea* /*pDragon*/)
{
    Unimplemented();
}

// 06056F6A — worm segment draw installed by Init when a 3D model was
// loaded. Lazy-updates the model indices when the entity's "unlocked"
// game-state bit flips mid-draw, then runs the normal LOD-gated draw
// path using mA8_modelIdx / mAA_poseIdx as the two LOD slots.
static void a5WormSegmentEntity_DrawWithModel(sA5WormSegmentEntity* pThis)
{
    sSaturnPtr data = pThis->m8_dataPtr;

    if (pThis->mA0_state == 1)
    {
        u32 bitIdx = (u32)readSaturnS16(data + 0x5E);
        u32 adjIdx = bitIdx;
        if ((s32)bitIdx > 999)
        {
            adjIdx = bitIdx - 0x236;
            bitIdx = bitIdx - 0x236;
        }
        if ((mainGameState.bitField[adjIdx >> 3] & bitMasks[bitIdx & 7]) != 0)
        {
            a5WormSegmentEntity_runUnlockTransition_06056A22(pThis);
            pThis->mA8_modelIdx = readSaturnS16(data + 0x56);
            pThis->mAA_poseIdx  = readSaturnS16(data + 0x58);
            pThis->mA0_state++;
        }
    }

    pushCurrentMatrix();
    sVec3_FP pos = readSaturnVec3(data);
    translateCurrentMatrix(&pos);
    sVec3_FP rot = { fixedPoint(readSaturnS32(data + 0xC)),
                     fixedPoint(readSaturnS32(data + 0x10)),
                     fixedPoint(readSaturnS32(data + 0x14)) };
    rotateCurrentMatrixZYX(&rot);

    u32 lod = gridCellDraw_GetDepthRange(pCurrentMatrix->m[2][3]);
    if (lod < 2)
    {
        // LOD 0 → mA8_modelIdx (near), LOD 1 → mAA_poseIdx (far). The
        // Ghidra side reads these from a packed s16 pair at byte 0xA8.
        s16 modelOffset = (lod == 0) ? pThis->mA8_modelIdx : pThis->mAA_poseIdx;
        sProcessed3dModel* pModel = pThis->m0_memoryArea.m0_mainMemoryBundle->get3DModel(modelOffset);
        addObjectToDrawList(pModel);
        a5_dispatchWormCollision_06077304(pThis, readSaturnS16(data + 0x1C));
    }
    popMatrix();
}

// 06056A9A
static void a5WormSegmentEntity_Init(sA5WormSegmentEntity* pThis, sSaturnPtr arg)
{
    s16 areaIdx = readSaturnS16(arg + 0x1E);
    getMemoryArea(&pThis->m0_memoryArea, areaIdx);
    pThis->m8_dataPtr = arg;
    pThis->m9C_config = readSaturnS16(arg + 0x20);

    // If data has a 3D model index
    s16 modelOffset = readSaturnS16(arg + 0x5A);
    if (modelOffset != 0)
    {
        s_fileBundle* pBundle = pThis->m0_memoryArea.m0_mainMemoryBundle;
        sStaticPoseData* pPose = pBundle->getStaticPose(readSaturnS16(arg + 0x5C), pBundle->getModelHierarchy(modelOffset)->countNumberOfBones());
        init3DModelRawData(pThis, &pThis->m40_3dModel, 0, pBundle, modelOffset, nullptr, pPose, nullptr, nullptr);
        stepAnimation(&pThis->m40_3dModel);
        pThis->m_DrawMethod = &a5WormSegmentEntity_DrawWithModel;

        // Check game state bit from data
        u32 bitIdx = (u32)readSaturnS16(arg + 0x5E);
        u32 adjIdx = (bitIdx < 1000) ? bitIdx : (bitIdx - 0x236);
        if ((mainGameState.bitField[adjIdx >> 3] & (1 << (bitIdx & 7))) == 0)
        {
            pThis->mA8_modelIdx = readSaturnS16(arg + 0x18);
            pThis->mAA_poseIdx = readSaturnS16(arg + 0x1A);
            pThis->mA0_state = 1;
        }
        else
        {
            pThis->mA8_modelIdx = readSaturnS16(arg + 0x56);
            pThis->mAA_poseIdx = readSaturnS16(arg + 0x58);
            pThis->mA0_state = 2;
        }
    }

    // Set up model render context
    s16 rotFlag = readSaturnS16(arg + 0x26);
    if (rotFlag == 0)
    {
        initFieldModelRenderContext(&pThis->mC_modelCtx, pThis, nullptr,
            (sVec3_FP*)getSaturnPtr(arg), nullptr, 3, 0, -1, 0, 0);
    }
    else
    {
        // Compute rotated normal vector
        sMatrix4x3 rotMatrix;
        initMatrixToIdentity(&rotMatrix);
        rotateMatrixShiftedY(fixedPoint(readSaturnS32(arg + 0x10)), &rotMatrix);
        transformVec(*(sVec3_FP*)getSaturnPtr(gFLD_A5->getSaturnPtr(0x06098AE4)), pThis->m90_normal, rotMatrix);
        initFieldModelRenderContext(&pThis->mC_modelCtx, pThis, nullptr,
            (sVec3_FP*)getSaturnPtr(arg), &pThis->m90_normal, 3, 0, -1, 0, 0);
    }
}

// 06056C16
static void a5WormSegmentEntity_Update(sA5WormSegmentEntity* pThis)
{
    sSaturnPtr data = pThis->m8_dataPtr;
    s32 state = pThis->m9C_config;

    if (state == -1)
    {
        if (readSaturnS16(data + 0x24) == 0)
            return;
        // FUN_FLD_A5__0607a2cc — empty stub in the Saturn binary (proximity
        // check placeholder that the original code never wrote).
        return;
    }
    if (state == 0)
    {
        // Check activation game state bits
        bool activated = true;
        s16 bit1 = readSaturnS16(data + 0x22);
        if (bit1 != 0)
        {
            u32 adj1 = (bit1 < 1000) ? (u32)bit1 : (u32)(bit1 - 0x236);
            if ((mainGameState.bitField[adj1 >> 3] & (1 << (bit1 & 7))) == 0)
                activated = false;
        }
        if (activated)
        {
            s16 bit2 = readSaturnS16(data + 0x5E);
            if (bit2 != 0)
            {
                u32 adj2 = (bit2 < 1000) ? (u32)bit2 : (u32)(bit2 - 0x236);
                if ((mainGameState.bitField[adj2 >> 3] & (1 << (bit2 & 7))) == 0)
                    activated = false;
            }
        }
        if (activated)
            pThis->m9C_config++;

        // FUN_FLD_A5__0607a2cc — empty stub in the Saturn binary.
        return;
    }
    if (state == 1)
    {
        updateFieldModelRenderContext(&pThis->mC_modelCtx);
        // FUN_FLD_A5__0607a2cc — empty stub in the Saturn binary.
        return;
    }
    if (state == 2)
    {
        // Dragon autopilot setup through the worm tunnel:
        //   1. Copy the tunnel's "target" vec3 (data + 0x2c..0x37) into the
        //      current follow-mode config buffer (at 0x00 onwards).
        //   2. Start follow mode 0 via 060694d8.
        //   3. Seed the segment countdown (mA4_pad = 0x5A = 90 frames).
        //   4. Reprogram the dragon task for autopilot motion:
        //      - install 060882e4 as its update
        //      - clear script status (m104) and the collision flag (m_F8 & 0x400)
        //      - set y scale / hardcoded 0x14000
        //      - set yaw = data[0x48]
        //      - compute velocity (vx, vy, vz) from data[0x50] * -sin/cos(yaw)
        //        with performDivision for vy
        //      - back-project the starting position by (timer + 5) frames
        //        so the dragon arrives at data[0x38..0x40] exactly at the
        //        end of the countdown.
        sFieldCameraStatus* pActiveSlot = a5_wormSegmentEntity_getActiveCameraSlot_06069490();
        pActiveSlot->m0_position.m0_X = fixedPoint(readSaturnS32(data + 0x2C));
        pActiveSlot->m0_position.m4_Y = fixedPoint(readSaturnS32(data + 0x30));
        pActiveSlot->m0_position.m8_Z = fixedPoint(readSaturnS32(data + 0x34));
        a5_wormSegmentEntity_startFollowMode_060694D8(0);

        pThis->mA4_pad = 0x5A;

        s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
        pDragon->mF0 = &a5_wormSegmentEntity_dragonAutopilotUpdate_060882E4;
        pDragon->m104_dragonScriptStatus = 0;
        pDragon->mF8_Flags &= ~0x400;
        pDragon->m8_pos.m4_Y = fixedPoint(0x14000);
        pDragon->m20_angle.m0_X = fixedPoint(0);
        pDragon->m20_angle.m4_Y = fixedPoint(readSaturnS32(data + 0x48));
        pDragon->m20_angle.m8_Z = fixedPoint(0);

        s32 dragonSpeed = readSaturnS32(data + 0x50);
        u16 yawIdx = (u16)((u32)pDragon->m20_angle.m4_Y.m_value >> 16) & 0xFFF;
        pDragon->m160_deltaTranslation.m0_X = MTH_Mul(fixedPoint(-dragonSpeed), getSin(yawIdx));
        pDragon->m160_deltaTranslation.m4_Y = fixedPoint(performDivision(
            pThis->mA4_pad, readSaturnS32(data + 0x3C) - pDragon->m8_pos.m4_Y.m_value));
        pDragon->m160_deltaTranslation.m8_Z = MTH_Mul(fixedPoint(-dragonSpeed), getCos(yawIdx));

        pDragon->m8_pos.m0_X = fixedPoint(readSaturnS32(data + 0x38)
            - pDragon->m160_deltaTranslation.m0_X.m_value * (pThis->mA4_pad + 5));
        pDragon->m8_pos.m4_Y = fixedPoint(readSaturnS32(data + 0x3C)
            - pDragon->m160_deltaTranslation.m4_Y.m_value * (pThis->mA4_pad + 5));
        pDragon->m8_pos.m8_Z = fixedPoint(readSaturnS32(data + 0x40)
            - pDragon->m160_deltaTranslation.m8_Z.m_value * (pThis->mA4_pad + 5));

        pThis->m9C_config++;
        return;
    }
    if (state == 3)
    {
        pThis->mA4_pad--;
        if (pThis->mA4_pad > 0)
            return;

        s16 dest = readSaturnS16(data + 0x28);
        if (dest < 0)
        {
            // Tutorial path: negative destination selects either the
            // A3/1 "checkExits update2 sub1" helper (arg 0xB) or the
            // tutorial multi-choice dispatcher, depending on the
            // already-tutorialised game-state bit at bitField[0xB].0x80.
            if ((mainGameState.bitField[0xB] & 0x80) == 0)
            {
                fieldA3_1_checkExitsTaskUpdate2Sub1(0xB);
            }
            else
            {
                dispatchTutorialMultiChoiceSub2();
            }
        }
        else
        {
            triggerSubfieldChange(dest, -0x8000);
            playBattleSoundEffect(100);
            playBattleSoundEffect(0x65);
        }
        pThis->m9C_config++;
        return;
    }
}

// Shared: create one worm segment entity with specific data
void createA5_wormSegment(p_workArea parent, sSaturnPtr data)
{
    static sA5WormSegmentEntity::TypedTaskDefinition td = { &a5WormSegmentEntity_Init, &a5WormSegmentEntity_Update, &a5WormSegmentEntity_Draw, nullptr };
    createSubTaskWithArg<sA5WormSegmentEntity>(parent, data, &td);
}

// 06057066 — create 2 worm segment entities (subfields 0, 7)
void createA5_wormSegments_day(p_workArea parent)
{
    static sA5WormSegmentEntity::TypedTaskDefinition td = { &a5WormSegmentEntity_Init, &a5WormSegmentEntity_Update, &a5WormSegmentEntity_Draw, nullptr };
    createSubTaskWithArg<sA5WormSegmentEntity>(parent, gFLD_A5->getSaturnPtr(0x06098AF0), &td);
    createSubTaskWithArg<sA5WormSegmentEntity>(parent, gFLD_A5->getSaturnPtr(0x06098B50), &td);
}

// 0605708e — create 3 worm segment entities (subfields 2, 8)
void createA5_wormSegments_night(p_workArea parent)
{
    static sA5WormSegmentEntity::TypedTaskDefinition td = { &a5WormSegmentEntity_Init, &a5WormSegmentEntity_Update, &a5WormSegmentEntity_Draw, nullptr };
    createSubTaskWithArg<sA5WormSegmentEntity>(parent, gFLD_A5->getSaturnPtr(0x06098C10), &td);
    createSubTaskWithArg<sA5WormSegmentEntity>(parent, gFLD_A5->getSaturnPtr(0x06098BB0), &td);
    createSubTaskWithArg<sA5WormSegmentEntity>(parent, gFLD_A5->getSaturnPtr(0x06098C70), &td);
}

// 060570c2 — create 2 worm segment entities (subfield 4)
void createA5_envObjects_sub4_wormSegments(p_workArea parent)
{
    static sA5WormSegmentEntity::TypedTaskDefinition td = { &a5WormSegmentEntity_Init, &a5WormSegmentEntity_Update, &a5WormSegmentEntity_Draw, nullptr };
    createSubTaskWithArg<sA5WormSegmentEntity>(parent, gFLD_A5->getSaturnPtr(0x06098CD0), &td);
    createSubTaskWithArg<sA5WormSegmentEntity>(parent, gFLD_A5->getSaturnPtr(0x06098D30), &td);
}
