#include "PDS.h"
#include "o_fld_a5.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldModelRender.h"
#include "field/fieldDragonInput.h"
#include "field/fieldVisibilityGrid.h"
#include "field/fieldDebrisScatter.h"
#include "kernel/fileBundle.h"
#include "kernel/animation.h"
#include "audio/soundDriver.h"
#include "audio/systemSounds.h"
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

// 06056a22 — "unlock" transition: spawn debris scatter + play sound when segment unlocks
static void a5WormSegmentEntity_runUnlockTransition_06056A22(sA5WormSegmentEntity* pThis)
{
    sSaturnPtr data = pThis->m8_dataPtr;

    // Read model/pose offsets from Saturn data at +0x5A and +0x5C
    u16 bundleOffset = readSaturnU16(data + 0x5A);
    u16 poseOffset = readSaturnU16(data + 0x5C);

    sDebrisScatterParams params;
    initDebrisScatterConfig(&params, bundleOffset, poseOffset);

    // Read max scalar from the bundle tree
    s_fileBundle* pBundle = pThis->m0_memoryArea.m0_mainMemoryBundle;
    u8* pRaw = pBundle->getRawBuffer();
    u32 treeRootOffset = READ_BE_U32(pRaw + bundleOffset);
    params.m8_spread = fixedPoint(readMaxScalarFromBundleTree(pRaw, treeRootOffset));

    params.m0_gravity = fixedPoint(0x14a);
    params.m4_bounce = fixedPoint((s32)0xFFFFE667);
    params.mC_randomMask = fixedPoint(0x3FFFFF);

    // Position from Saturn data at +0xC (the entity's world position)
    sVec3_FP pos = readSaturnVec3(data + 0xC);
    params.m10_pPosition = &pos;
    params.m14_pRotation = nullptr;

    params.m18_velX = 0;
    params.m1C_velY = 0;
    params.m20_velZ = fixedPoint((s32)0xFFFFF800);

    params.m_pBundle = pBundle;

    createDebrisScatterTask((p_workArea)pThis, &params, false);

    // Play the unlock sound effect from Saturn data at +0x54
    s16 soundId = readSaturnS16(data + 0x54);
    playSystemSoundEffect((s32)soundId);
}

// 06069490 — shared: getActiveCameraSlot. A5 wrapper kept for external callers.
sFieldCameraStatus* a5_wormSegmentEntity_getActiveCameraSlot_06069490()
{
    return getActiveCameraSlot();
}

// 0606a6b4 — debug free camera: reads player-2 controller to move/rotate camera
static void a5_debugFreeCamera_0606a6b4()
{
    sFieldCameraStatus* pCam = getActiveCameraSlot();
    auto& input = graphicEngineStatus.m4514.m0_inputDevices[1].m0_current;

    // Rotation: d-pad slow, d-pad+held fast
    if (input.m6_buttonDown & 0x2000) pCam->mC_rotation.m0_X.m_value += 0xB60B6;
    if (input.m6_buttonDown & 0x0001) pCam->mC_rotation.m0_X.m_value -= 0xB60B6;
    if (input.m6_buttonDown & 0x8000) pCam->mC_rotation.m4_Y.m_value -= 0xB60B6;
    if (input.m6_buttonDown & 0x0800) pCam->mC_rotation.m4_Y.m_value += 0xB60B6;

    if (input.mE & 0x2000) pCam->mC_rotation.m0_X.m_value += 0x2D82D8;
    if (input.mE & 0x0001) pCam->mC_rotation.m0_X.m_value -= 0x2D82D8;
    if (input.mE & 0x8000) pCam->mC_rotation.m4_Y.m_value -= 0x2D82D8;
    if (input.mE & 0x0800) pCam->mC_rotation.m4_Y.m_value += 0x2D82D8;

    u16 yawIdx = (u16)((u32)pCam->mC_rotation.m4_Y.m_value >> 16) & 0xFFF;

    // Movement: forward/back/strafe slow
    if (input.m6_buttonDown & 0x10)
    {
        pCam->m0_position.m0_X.m_value -= MTH_Mul(fixedPoint(0x1000), getSin(yawIdx)).m_value;
        pCam->m0_position.m8_Z.m_value -= MTH_Mul(fixedPoint(0x1000), getCos(yawIdx)).m_value;
    }
    if (input.m6_buttonDown & 0x20)
    {
        pCam->m0_position.m0_X.m_value += MTH_Mul(fixedPoint(0x1000), getSin(yawIdx)).m_value;
        pCam->m0_position.m8_Z.m_value += MTH_Mul(fixedPoint(0x1000), getCos(yawIdx)).m_value;
    }
    if (input.m6_buttonDown & 0x80)
    {
        pCam->m0_position.m0_X.m_value += MTH_Mul(fixedPoint(0x1000), getCos(yawIdx)).m_value;
        pCam->m0_position.m8_Z.m_value -= MTH_Mul(fixedPoint(0x1000), getSin(yawIdx)).m_value;
    }
    if (input.m6_buttonDown & 0x40)
    {
        pCam->m0_position.m0_X.m_value -= MTH_Mul(fixedPoint(0x1000), getCos(yawIdx)).m_value;
        pCam->m0_position.m8_Z.m_value += MTH_Mul(fixedPoint(0x1000), getSin(yawIdx)).m_value;
    }

    // Up/down slow
    if (input.m6_buttonDown & 0x4000) pCam->m0_position.m4_Y.m_value += 0x1000;
    if (input.m6_buttonDown & 0x0004) pCam->m0_position.m4_Y.m_value -= 0x1000;

    // Movement: forward/back/strafe fast (held buttons)
    if (input.mE & 0x10)
    {
        pCam->m0_position.m0_X.m_value -= MTH_Mul(fixedPoint(0x4000), getSin(yawIdx)).m_value;
        pCam->m0_position.m8_Z.m_value -= MTH_Mul(fixedPoint(0x4000), getCos(yawIdx)).m_value;
    }
    if (input.mE & 0x20)
    {
        pCam->m0_position.m0_X.m_value += MTH_Mul(fixedPoint(0x4000), getSin(yawIdx)).m_value;
        pCam->m0_position.m8_Z.m_value += MTH_Mul(fixedPoint(0x4000), getCos(yawIdx)).m_value;
    }
    if (input.mE & 0x80)
    {
        pCam->m0_position.m0_X.m_value += MTH_Mul(fixedPoint(0x4000), getCos(yawIdx)).m_value;
        pCam->m0_position.m8_Z.m_value -= MTH_Mul(fixedPoint(0x4000), getSin(yawIdx)).m_value;
    }
    if (input.mE & 0x40)
    {
        pCam->m0_position.m0_X.m_value -= MTH_Mul(fixedPoint(0x4000), getCos(yawIdx)).m_value;
        pCam->m0_position.m8_Z.m_value += MTH_Mul(fixedPoint(0x4000), getSin(yawIdx)).m_value;
    }

    // Up/down fast
    if (input.mE & 0x4000) pCam->m0_position.m4_Y.m_value += 0x4000;
    if (input.mE & 0x0004) pCam->m0_position.m4_Y.m_value -= 0x4000;
}

// 0606A984 — camera follow mode 7 draw: debug free-camera mode
static void a5_cameraFollowMode7_Draw(sFieldCameraStatus*)
{
    s_FieldSubTaskWorkArea* pSub = getFieldTaskPtr()->m8_pSubFieldData;
    if (pSub->m380_debugMenuStatus3 != 0 && pSub->m37E_debugMenuStatus2_a == 1)
    {
        a5_debugFreeCamera_0606a6b4();
    }

    if (pSub->m37C_debugMenuStatus1[0] == 0)
    {
        sFieldCameraManager* pCam = pSub->m334;
        activateCameraFollowMode((u32)(s8)pCam->m50E_followModeIndex);
    }
}

// Camera follow modes 3-6 are not yet implemented in C++ (exist in binary).
// They assert in A3 and byte-match across all overlays.
void cameraFollowMode3(sFieldCameraStatus*);
void cameraFollowMode4(sFieldCameraStatus*);
void cameraFollowMode5(sFieldCameraStatus*);
void cameraFollowMode6(sFieldCameraStatus*);

// Saturn table at FLD_A5::0609e8e8 — per-follow-mode update function pointers
// All entries are shared functions duplicated across every field overlay.
static void (*s_a5FollowModeUpdateTable[10])(sFieldCameraStatus*) = {
    &cameraFollowMode_scriptTarget,     // [0] 0606A4F4
    &fieldOverlaySubTaskInitSub2,       // [1] 06069DBA
    &fieldOverlaySubTaskInitSub2_mode2, // [2] 06069E1C
    &cameraFollowMode3,                 // [3] 06069E92
    &cameraFollowMode4,                 // [4] 06069EF6
    &cameraFollowMode5,                 // [5] 06069F64
    &cameraFollowMode6,                 // [6] 0606A000
    nullptr,
    &cameraFollowMode_idle,             // [8] 0606A068
    nullptr,
};

// Saturn table at FLD_A5::0609e910 — per-follow-mode draw function pointers
static void (*s_a5FollowModeDrawTable[10])(sFieldCameraStatus*) = {
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    &a5_cameraFollowMode7_Draw,
    nullptr, nullptr,
};

// 0606932e — shared: setCameraFollowFunctions
// 06069508 — shared: activateCameraFollowMode (calls setCameraFollowFunctions + sets followState=1)
// 060694d8 — shared: startCameraFollowMode (calls setCameraFollowFunctions + sets followState=0)
// A5 wrappers use the overlay-local follow mode tables (which differ only
// in the mode 7 draw slot — all update entries point to shared functions).
void a5_activateFollowMode_06069508(s32 followMode)
{
    setCameraFollowFunctions(0,
        s_a5FollowModeUpdateTable[followMode],
        s_a5FollowModeDrawTable[followMode]);
    getActiveCameraSlot()->m8D_followState = 1;
}

void a5_wormSegmentEntity_startFollowMode_060694D8(s32 followMode)
{
    setCameraFollowFunctions(0,
        s_a5FollowModeUpdateTable[followMode],
        s_a5FollowModeDrawTable[followMode]);
    getActiveCameraSlot()->m8D_followState = 0;
}

// 060882e4 — dragon task update for worm segment autopilot.
// 2-state: init (update camera, clear collision, compute speed), then integrate position.
static void a5_wormSegmentEntity_dragonAutopilotUpdate_060882E4(s_dragonTaskWorkArea* pDragon)
{
    pDragon->m24A_runningCameraScript = 3;
    getFieldTaskPtr()->m28_status |= 0x10000;
    getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m8 |= 1;

    if (pDragon->m104_dragonScriptStatus == 0)
    {
        updateCameraScriptSub0((p_workArea)pDragon->mB8_lightWingEffect);
        pDragon->mF8_Flags &= ~0x400;
        pDragon->mF8_Flags |= 0x20000;
        computeDragonSpeed(pDragon);
        setDragonAnimationFromSpeed(pDragon);
        pDragon->m104_dragonScriptStatus++;
    }
    else if (pDragon->m104_dragonScriptStatus != 1)
    {
        goto done;
    }

    // States 0 (after init) and 1: integrate position from delta translation
    buildDragonRotationMatrix(&pDragon->m48, &pDragon->m20_angle);
    copyMatrix(&pDragon->m48.m0_matrix, &pDragon->m88_matrix);

    pDragon->m8_pos.m0_X = fixedPoint(pDragon->m8_pos.m0_X.m_value + pDragon->m160_deltaTranslation.m0_X.m_value);
    pDragon->m8_pos.m4_Y = fixedPoint(pDragon->m8_pos.m4_Y.m_value + pDragon->m160_deltaTranslation.m4_Y.m_value);
    pDragon->m8_pos.m8_Z = fixedPoint(pDragon->m8_pos.m8_Z.m_value + pDragon->m160_deltaTranslation.m8_Z.m_value);

done:
    computeDragonSpeed(pDragon);
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
