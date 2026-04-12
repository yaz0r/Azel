#include "PDS.h"
#include "o_fld_a5.h"
#include "field/fieldModelRender.h"
#include "field/field_a3/o_fld_a3.h"
#include "kernel/fileBundle.h"
#include "kernel/animation.h"
#include "kernel/monsterPart.h"
#include "audio/systemSounds.h"
#include "menu_dragonMorph.h" // computeLookAt

// Local file-scope performDivision matching the inline in movie.cpp /
// menu_dragon.cpp: returns dividend / divisor, args ordered (divisor, dividend).
static inline s32 performDivision(s32 divisor, s32 dividend) { return dividend / divisor; }
#include "3dEngine.h"
#include "3dModels.h"
#include "a5_gridDeferredDraw.h"

// Worm-hole environment entity (subfield 0, Saturn size 0x144). Uses a
// Baldor-style segmented body with a head, repeating middle segments and
// a tail, plus two 3D models (head/tail) loaded from per-type tables.
// The update drives a 4-state swing/reset cycle and the draw walks the
// ring of parts submitting the head model, per-segment drawables and the
// tail model.
struct sA5WormHoleEntity : public s_workAreaTemplateWithArg<sA5WormHoleEntity, sSaturnPtr>
{
    s_memoryAreaOutput m0_memoryArea;       // 0x00..0x07
    s32*                m8_perPartInts;      // 0x08 — partCount s32 entries (heap)
    u8*                 mC_perPartBytes;     // 0x0C — partCount bytes (heap)
    sVec3_FP            m10_pos;             // 0x10..0x1B — base position (copy of arg)
    s32                 m1C_radius;          // 0x1C
    s32                 m20_param8;          // 0x20 — per-frame m60 step
    s32                 m24;                 // 0x24 — zero
    s32                 m28_param10;         // 0x28
    s32                 m2C_param2C;         // 0x2C
    s32                 m30_timer;           // 0x30 — decremented each update
    s32                 m34;                 // 0x34 — draw scratch (part world X)
    s32                 m38;                 // 0x38 — zero
    s32                 m3C;                 // 0x3C — draw scratch (part world Z)
    sVec3_FP            m40_renderPosition;  // 0x40..0x4B — second pos copy (render)
    sVec3_FP            m4C_rotationTarget;  // 0x4C..0x57
    sMonsterBody*        m58_body;            // 0x58 — Baldor body pointer
    s32                 m5C;                 // 0x5C — used by FUN_06059a86 as a sVec3_FP lane
    s32                 m60;                 // 0x60 — oscillation phase
    s32                 m64;                 // 0x64
    s32                 m68_partCount;       // 0x68
    s_3dModel           m6C_model1;          // 0x6C..0xBB — head model (0x50 bytes)
    s_3dModel           mBC_model2;          // 0xBC..0x10B — tail model (0x50 bytes)
    sFieldModelRenderContext m10C_modelCtx; // 0x10C..0x13F (0x34)
    u8                  m140_state;          // 0x140 — 4-state swing cycle
    u8                  m141_argFlag;        // 0x141 — arg byte 0x10, selects offset table
    u8                  m142_subState;       // 0x142 — zero
    u8                  m143_pad;            // 0x143
    // Saturn size 0x144 (C++ size differs on 64-bit hosts because of fat pointers)
};

// Saturn data tables at FLD_A5::06099cXX:
//   06099C40/C4C  — s32[3] model index (typeIdx), for model1/model2
//   06099C58/C64  — s32[3] pose offset (bundle-relative), model1/model2
//   06099C70/C7C  — s32[3] anim/hierarchy offset, model1/model2
//   06099C88      — sVec3_FP[3] per-type part translation (copied into each body part)
//   06099C1C/C28/C34 — 3 ptr[3] tables of head/mid/tail Baldor-part template EAs
static constexpr u32 kWormHoleTable_Model1Idx  = 0x06099C40;
static constexpr u32 kWormHoleTable_Model2Idx  = 0x06099C4C;
static constexpr u32 kWormHoleTable_Pose1      = 0x06099C58;
static constexpr u32 kWormHoleTable_Pose2      = 0x06099C64;
static constexpr u32 kWormHoleTable_Anim1      = 0x06099C70;
static constexpr u32 kWormHoleTable_Anim2      = 0x06099C7C;
static constexpr u32 kWormHoleTable_PartTrans  = 0x06099C88;
static constexpr u32 kWormHoleTable_HeadTemplates = 0x06099C1C;
static constexpr u32 kWormHoleTable_MidTemplates  = 0x06099C28;
static constexpr u32 kWormHoleTable_TailTemplates = 0x06099C34;
// Offset tables used by Update state 2
static constexpr u32 kWormHoleTable_StateOffsetX = 0x06099CB8;
static constexpr u32 kWormHoleTable_StateOffsetZ = 0x06099CD8;

// 06077304 — forwards to the shared deferred-draw helper with the
// worm-hole's owning bundle.
static void a5_dispatchWormCollision_06077304(sA5WormHoleEntity* pThis, s16 entryKey)
{
    callGridCellDraw_normalSub2(pThis->m0_memoryArea.m0_mainMemoryBundle, entryKey);
}

// 06077074 — variant of 06077304 with explicit pos/rot override.
// Forwards to the shared deferred-draw helper.
static void a5_dispatchWormCollisionWithTransform_06077074(sA5WormHoleEntity* pThis, s16 entryKey, sVec3_FP* pPos, sVec3_FP* pRot)
{
    a5GridDraw_cullAndEnqueueWithTransform_06077074(
        pThis->m0_memoryArea.m0_mainMemoryBundle, entryKey, pPos, pRot);
}

// 06079106 — transforms a world position into view space and tail-calls the
// A5 on-screen test. Matches the existing field_a7 / field_a5 inlining of
// (transformAndAddVecByCurrentMatrix, isPointOnScreen) used in lieu of a
// dedicated shared helper.
static s32 a5_isPointVisible_06079106(sVec3_FP* pPos)
{
    sVec3_FP viewPos;
    transformAndAddVecByCurrentMatrix(pPos, &viewPos);
    return isPointOnScreen(&viewPos, graphicEngineStatus.m405C.m14_farClipDistance.m_value) ? 1 : 0;
}

// 06059ae0 — picks a random step direction from a 12x5 permutation table at
// FLD_A5::06099CF8, applies the matching (X, Z) offset from the state-2
// offset tables (06099CB8/CD8), and rejects bounds/quadrant violations.
// Up to 5 attempts; if they all fail the last attempted state is kept.
//
// Direction bytes in the table are in [6..0xA]; after masking with 7 they
// form a rotation of {6,7,0,1,2} around the 8-direction ring.
static void a5_wormHoleEntity_updateEntry_06059ae0(sVec3_FP* pPos, u8* pArgFlag)
{
    s32 startX = pPos->m0_X.m_value;
    s32 startZ = pPos->m8_Z.m_value;
    u8  startDir = *pArgFlag;

    s32 tableRowBase = 0x06099CF8 + (performModulo2(12, randomNumber()) * 5);

    s32 attempts = 5;
    s32 attemptIdx = 0;
    while (attempts >= 1)
    {
        // Reset position to the entry point before each trial.
        pPos->m0_X = fixedPoint(startX);
        pPos->m8_Z = fixedPoint(startZ);

        s8 delta = (s8)readSaturnU8(gFLD_A5->getSaturnPtr(tableRowBase + attemptIdx));
        attemptIdx++;
        *pArgFlag = (startDir + delta) & 7;

        pPos->m0_X = fixedPoint(pPos->m0_X.m_value
            + (s32)readSaturnU32(gFLD_A5->getSaturnPtr(kWormHoleTable_StateOffsetX + (*pArgFlag) * 4)));
        pPos->m8_Z = fixedPoint(pPos->m8_Z.m_value
            + (s32)readSaturnU32(gFLD_A5->getSaturnPtr(kWormHoleTable_StateOffsetZ + (*pArgFlag) * 4)));

        // Outer bounding box: X in [0x4F5000, 0x6F5000], Z in [-0x700000, -0x100000).
        bool oob = pPos->m0_X.m_value < 0x4F5000
                || pPos->m0_X.m_value > 0x6F5000
                || pPos->m8_Z.m_value < -0x700000
                || pPos->m8_Z.m_value >= -0x100000;

        if (oob)
        {
            attempts--;
            continue;
        }

        // Per-diagonal-direction quadrant bans: cut off the impassable corners
        // of the worm-hole ring for odd directions only.
        bool bad = false;
        switch (*pArgFlag)
        {
        case 1:
            if (pPos->m0_X.m_value >= 0x675000 && pPos->m8_Z.m_value >= -0x680000)
                bad = true;
            break;
        case 3:
            if (pPos->m0_X.m_value >= 0x675000 && pPos->m8_Z.m_value >= -0x180000)
                bad = true;
            break;
        case 5:
            if (pPos->m0_X.m_value <= 0x575000 && pPos->m8_Z.m_value >= -0x180000)
                bad = true;
            break;
        case 7:
            if (pPos->m0_X.m_value <= 0x575000 && pPos->m8_Z.m_value < -0x680000)
                bad = true;
            break;
        default:
            break;
        }

        if (bad)
        {
            attempts--;
        }
        else
        {
            attempts = 0; // accepted
        }
    }
}

// 060599a8 — builds the per-frame velocity from the difference between
// the current base position (m10_pos) and the render position, then
// projects it onto the XZ plane so the swing update can integrate it.
//
// Layout of the scratch velocity lane:
//   m5C = speed * cos(atan2(dZ, dX))   — X velocity per frame
//   m60 = (radius * 0x40000) / timer   — Y rotation step per frame
//   m64 = speed * sin(atan2(dZ, dX))   — Z velocity per frame
//
// Where `speed = |delta| / timer` and `timer` is m2C_param2C scaled by
// 1.5 when (argFlag & 1) is set.
static void a5_wormHoleEntity_commitPose_060599a8(sA5WormHoleEntity* pThis)
{
    s32 radius = pThis->m1C_radius;
    s32 timer;
    if ((pThis->m141_argFlag & 1) == 0)
    {
        timer = pThis->m2C_param2C;
    }
    else
    {
        timer = (pThis->m2C_param2C * 3) >> 1;
    }
    pThis->m30_timer = timer;

    // delta = m10_pos - m40_renderPosition, stored in the m5C/m60/m64 lane.
    sVec3_FP delta;
    delta.m0_X = fixedPoint(pThis->m10_pos.m0_X.m_value - pThis->m40_renderPosition.m0_X.m_value);
    delta.m4_Y = fixedPoint(pThis->m10_pos.m4_Y.m_value - pThis->m40_renderPosition.m4_Y.m_value);
    delta.m8_Z = fixedPoint(pThis->m10_pos.m8_Z.m_value - pThis->m40_renderPosition.m8_Z.m_value);
    pThis->m5C = delta.m0_X.m_value;
    pThis->m60 = delta.m4_Y.m_value;
    pThis->m64 = delta.m8_Z.m_value;

    fixedPoint lengthSq = MTH_Product3d_FP(delta, delta);
    fixedPoint length   = sqrt_F(lengthSq);
    s32 speed = performDivision(pThis->m30_timer, length.m_value);

    // Y rotation step: constant based on radius/timer.
    s32 radiusScaled = MTH_Mul(fixedPoint(radius), fixedPoint(0x40000)).m_value;
    pThis->m60 = performDivision(pThis->m30_timer, radiusScaled);

    // Project speed onto the XZ plane using atan2(dZ, dX).
    s32 angle = atan2_FP(delta.m8_Z.m_value, delta.m0_X.m_value);
    u16 idx = (u16)((u32)angle >> 16) & 0xFFF;

    pThis->m5C = MTH_Mul(fixedPoint(speed), getCos(idx)).m_value;
    pThis->m64 = MTH_Mul(fixedPoint(speed), getSin(idx)).m_value;
}

// 06059a86 — takes a world-space direction vector, runs computeLookAt to
// produce (yaw, pitch), then writes the swapped/offset pair to the caller's
// s32[2] target (pitch = -yaw, yaw = pitch + 0x8000000).
static void a5_wormHoleEntity_stepRotation_06059a86(sVec3_FP* pVec, s32* pTarget)
{
    sVec2_FP lookAt;
    computeLookAt(*pVec, lookAt);
    pTarget[0] = -lookAt[0].m_value;
    pTarget[1] = lookAt[1].m_value + 0x8000000;
}

// 060597da — dragon-entered-zone trigger hook. Installed as the
// sFieldModelRenderContext m4_initCallback, invoked by the LCS render
// pipeline when the dragon crosses into this worm-hole's grid cell.
// Plays the discovery SFX, bumps the packed worm-hole discovery counter,
// flips the model-context visibility flag and sub-state, and (unless the
// tutorial "already-shown" bit is set) kicks script 0x5B6 through the
// overlay script dispatcher.
static void a5_wormHoleEntity_renderContextHook_060597da(sA5WormHoleEntity* pThis)
{
    playSystemSoundEffect(0x6B);

    u32 count = mainGameState.readPackedBits(0x578, 8);
    mainGameState.setPackedBits(0x578, 8, count + 1);

    pThis->m10C_modelCtx.m18_visibilityFlags |= 1;
    pThis->m142_subState = 1;

    if ((mainGameState.bitField[0xB] & 0x80) == 0)
    {
        startFieldScript(1, 0x5B6);
    }
}

// 06059854 — draws head model, each visible middle segment and the tail
// model (depending on the tail part's rotation sign).
static void a5WormHoleEntity_Draw(sA5WormHoleEntity* pThis)
{
    PDS_Log("a5WormHoleEntity_Draw fired, head.Y = 0x%x, partCount = %d\n",
        pThis->m58_body->m30_parts[0].m4_worldPosition.m4_Y.m_value,
        pThis->m68_partCount);

    sMonsterBody* pBody = pThis->m58_body;
    sMonsterBodyPart* pHead = &pBody->m30_parts[0];

    // Head segment — gated on the first part's world-position.Y being >= 0.
    if (pHead->m4_worldPosition.m4_Y.m_value >= 0)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pHead->m4_worldPosition);
        rotateCurrentMatrixYXZ(&pHead->m1C_rotation);
        pThis->m6C_model1.m18_drawFunction(&pThis->m6C_model1);
        a5_dispatchWormCollision_06077304(pThis, 0x140);
        popMatrix();
    }

    // Middle segments — walk from count-2 down to 1 using the body's
    // installed per-part drawer (m2C_drawPart).
    for (s32 i = pThis->m68_partCount - 2; i > 0; i--)
    {
        sMonsterBodyPart* pPart = &pBody->m30_parts[i];
        if (pPart->m4_worldPosition.m4_Y.m_value < 0)
            continue;

        pBody->m2C_drawPart(pThis->m0_memoryArea.m0_mainMemoryBundle, pPart);
        a5_dispatchWormCollisionWithTransform_06077074(pThis, 0x144, &pPart->m4_worldPosition, &pPart->m1C_rotation);

        // Scratch copy: m34 = worldPos.X, m3C = worldPos.Z.
        pThis->m34 = pPart->m4_worldPosition.m0_X.m_value;
        pThis->m3C = pPart->m4_worldPosition.m8_Z.m_value;

        pushCurrentMatrix();
        // translate to (m34, worldPos.Y, m3C) — same as part worldPos.
        translateCurrentMatrix(&pPart->m4_worldPosition);
        rotateCurrentMatrixShiftedX(fixedPoint(0x4000000));
        addObjectToDrawList(pThis->m0_memoryArea.m0_mainMemoryBundle->get3DModel(0xB0));
        popMatrix();
    }

    // Tail segment — last part. The Y cutoff flips based on the rotation sign.
    sMonsterBodyPart* pTail = &pBody->m30_parts[pThis->m68_partCount - 1];
    s32 cutoff = (pTail->m1C_rotation.m0_X.m_value < 0) ? 0x8000 : -0x8000;
    if (cutoff <= pTail->m4_worldPosition.m4_Y.m_value)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pTail->m4_worldPosition);
        rotateCurrentMatrixYXZ(&pTail->m1C_rotation);
        pThis->mBC_model2.m18_drawFunction(&pThis->mBC_model2);
        a5_dispatchWormCollision_06077304(pThis, 0x148);
        popMatrix();
    }
}

// 06059c20 — 4-state swing/reset update cycle. Each update decrements the
// timer, advances the render context, dispatches to the active state's
// transition-on-entry code, steps the rotation, and finally rolls a random
// sound effect near the entity.
static void a5WormHoleEntity_Update(sA5WormHoleEntity* pThis)
{
    static int s_updateCount = 0;
    if (s_updateCount < 3) {
        PDS_Log("a5WormHoleEntity_Update fired, state=%d\n", pThis->m140_state);
        s_updateCount++;
    }
    pThis->m30_timer--;
    updateFieldModelRenderContext(&pThis->m10C_modelCtx);

    s8 state = (s8)pThis->m140_state;
    bool inPositiveLane = false; // true for states 0/1 (m60 -= step), false for 2/3 (m60 += step)
    bool runStep = true;

    if (state == 0)
    {
        a5_wormHoleEntity_updateEntry_06059ae0(&pThis->m10_pos, &pThis->m141_argFlag);
        pThis->m40_renderPosition.m4_Y = fixedPoint(0);
        a5_wormHoleEntity_commitPose_060599a8(pThis);
        pThis->m140_state++;
        inPositiveLane = true;
    }
    else if (state == 1)
    {
        inPositiveLane = true;
    }
    else if (state == 2)
    {
        pThis->m10_pos.m0_X = fixedPoint(pThis->m10_pos.m0_X.m_value
            + (s32)readSaturnU32(gFLD_A5->getSaturnPtr(kWormHoleTable_StateOffsetX + pThis->m141_argFlag * 4)));
        pThis->m10_pos.m8_Z = fixedPoint(pThis->m10_pos.m8_Z.m_value
            + (s32)readSaturnU32(gFLD_A5->getSaturnPtr(kWormHoleTable_StateOffsetZ + pThis->m141_argFlag * 4)));
        a5_wormHoleEntity_commitPose_060599a8(pThis);
        pThis->m60 = -pThis->m60;
        pThis->m140_state++;
        inPositiveLane = false;
    }
    else if (state == 3)
    {
        inPositiveLane = false;
    }
    else
    {
        runStep = false;
    }

    if (runStep)
    {
        if (inPositiveLane)
        {
            pThis->m60 -= pThis->m20_param8;
            a5_wormHoleEntity_stepRotation_06059a86(reinterpret_cast<sVec3_FP*>(&pThis->m5C), &pThis->m4C_rotationTarget.m0_X.m_value);
            if (pThis->m30_timer < 1)
                pThis->m140_state++;
        }
        else
        {
            pThis->m60 += pThis->m20_param8;
            a5_wormHoleEntity_stepRotation_06059a86(reinterpret_cast<sVec3_FP*>(&pThis->m5C), &pThis->m4C_rotationTarget.m0_X.m_value);
            if (pThis->m30_timer < 1)
                pThis->m140_state = 0;
        }
    }

    // Random sound-effect roll (~1/256 chance when the position is visible).
    u32 r = randomNumber();
    if ((r & 0xFF) == 0 && a5_isPointVisible_06079106(&pThis->m40_renderPosition) != 0)
    {
        playSystemSoundEffect((r & 2) == 0 ? 0x6D : 0x6E);
    }
}

// 06059f1c — creates one worm-hole entity from a 0x24-byte Saturn arg record.
// Arg layout (short*-indexed in the Ghidra decompile):
//   [0]  s16 modelTypeIdx   — 0..2, selects head/mid/tail templates and model indices
//   [1]  s16 partCount      — number of body segments
//   +4   s32 posX
//   +8   s32 posY
//   +12  s32 posZ
//   +16  s32 radius
//   +20  s32 param10
//   +24  s32 param2C
//   +32  u8  argFlag
static void a5_createWormHoleEntity_06059f1c(p_workArea parent, sSaturnPtr arg)
{
    PDS_Log("a5_createWormHoleEntity_06059f1c called%s\n", "");

    sA5WormHoleEntity* pThis =
        createSubTaskFromFunction<sA5WormHoleEntity>(parent, &a5WormHoleEntity_Update);
    if (pThis == nullptr)
        return;

    getMemoryArea(&pThis->m0_memoryArea, 1);

    // Saturn stores these s16 values in the first 4 bytes of the arg.
    s16 modelTypeIdx = readSaturnS16(arg);
    s16 partCount    = readSaturnS16(arg + 2);
    pThis->m68_partCount = partCount;

    // Build the segmented body and stamp each part with the per-role template.
    sMonsterBody* pBody = monsterBody_create(pThis, partCount);
    pThis->m58_body = pBody;
    monsterBody_loadPartData(pBody, 0, sSaturnPtr());

    for (s32 iPart = 0; iPart < partCount; iPart++)
    {
        u32 templateTable;
        if (iPart == 0)
            templateTable = kWormHoleTable_HeadTemplates;
        else if (iPart == partCount - 1)
            templateTable = kWormHoleTable_TailTemplates;
        else
            templateTable = kWormHoleTable_MidTemplates;

        sMonsterBodyPart* pPart = &pBody->m30_parts[iPart];

        // m10_translation <- per-type offset from table at 06099C88 (12 bytes * typeIdx).
        sSaturnPtr transEA = gFLD_A5->getSaturnPtr(kWormHoleTable_PartTrans + (u8)modelTypeIdx * 12);
        pPart->m10_translation = readSaturnVec3(transEA);

        // Copy the 0x14-byte Baldor part template (modelIndex + spring + damping).
        sSaturnPtr partTemplateEA = gFLD_A5->getSaturnPtr(
            readSaturnU32(gFLD_A5->getSaturnPtr(templateTable + (u8)modelTypeIdx * 4)));
        monsterBodyPart_loadTemplate(pPart, partTemplateEA);

        // Spring stiffness diagonal = 0xE00 on each axis; damping = 0x5000.
        pPart->m44_springStiffness.m0_X = fixedPoint(0xE00);
        pPart->m44_springStiffness.m4_Y = fixedPoint(0xE00);
        pPart->m44_springStiffness.m8_Z = fixedPoint(0xE00);
        pPart->m50_damping = fixedPoint(0x5000);
    }

    // Runs the body through its update once with the render position and
    // rotation taken from the task's m40_renderPosition/m4C_rotationTarget.
    monsterBody_initPose(pBody, &pThis->m40_renderPosition, &pThis->m4C_rotationTarget);

    // Head 3D model (tables indexed by typeIdx * 4). Pose size is derived
    // from the animation's bone count so init3DModelRawData's m12_numBones
    // assertion against the pose is satisfied.
    s_fileBundle* pBundle = pThis->m0_memoryArea.m0_mainMemoryBundle;
    {
        u32 idx4 = (u32)modelTypeIdx * 4;
        s16 modelOffset = (s16)readSaturnU32(gFLD_A5->getSaturnPtr(kWormHoleTable_Model1Idx + idx4));
        u32 animOffset  = readSaturnU32(gFLD_A5->getSaturnPtr(kWormHoleTable_Anim1 + idx4));
        u32 poseOffset  = readSaturnU32(gFLD_A5->getSaturnPtr(kWormHoleTable_Pose1 + idx4));
        sAnimationData*  pAnim = pBundle->getAnimation(animOffset);
        sStaticPoseData* pPose = pBundle->getStaticPose(poseOffset, pAnim->m2_numBones);
        init3DModelRawData(pThis, &pThis->m6C_model1, 0, pBundle, (u16)modelOffset, pAnim, pPose, nullptr, nullptr);
        stepAnimation(&pThis->m6C_model1);
    }

    // Tail 3D model (separate tables at +0xC offset).
    {
        u32 idx4 = (u32)modelTypeIdx * 4;
        s16 modelOffset = (s16)readSaturnU32(gFLD_A5->getSaturnPtr(kWormHoleTable_Model2Idx + idx4));
        u32 animOffset  = readSaturnU32(gFLD_A5->getSaturnPtr(kWormHoleTable_Anim2 + idx4));
        u32 poseOffset  = readSaturnU32(gFLD_A5->getSaturnPtr(kWormHoleTable_Pose2 + idx4));
        sAnimationData*  pAnim = pBundle->getAnimation(animOffset);
        sStaticPoseData* pPose = pBundle->getStaticPose(poseOffset, pAnim->m2_numBones);
        init3DModelRawData(pThis, &pThis->mBC_model2, 0, pBundle, (u16)modelOffset, pAnim, pPose, nullptr, nullptr);
        stepAnimation(&pThis->mBC_model2);
    }

    // Per-part heap buffers (one s32 array + one byte array, each partCount long).
    pThis->m8_perPartInts  = static_cast<s32*>(allocateHeapForTask(pThis, partCount * 4));
    pThis->mC_perPartBytes = static_cast<u8*> (allocateHeapForTask(pThis, partCount));
    if (pThis->m8_perPartInts == nullptr || pThis->mC_perPartBytes == nullptr)
    {
        pThis->getTask()->markFinished();
        return;
    }

    // Copy remaining arg fields into the task — same value written to the
    // "input" and "render" slots of m10/m40.
    s32 posX = readSaturnS32(arg + 4);
    s32 posY = readSaturnS32(arg + 8);
    s32 posZ = readSaturnS32(arg + 12);
    pThis->m10_pos.m0_X = fixedPoint(posX); pThis->m40_renderPosition.m0_X = fixedPoint(posX);
    pThis->m10_pos.m4_Y = fixedPoint(posY); pThis->m40_renderPosition.m4_Y = fixedPoint(posY);
    pThis->m10_pos.m8_Z = fixedPoint(posZ); pThis->m40_renderPosition.m8_Z = fixedPoint(posZ);

    pThis->m1C_radius   = readSaturnS32(arg + 16);
    pThis->m2C_param2C  = readSaturnS32(arg + 24);
    pThis->m141_argFlag = readSaturnU8 (arg + 32);
    pThis->m140_state   = 0;
    pThis->m28_param10  = readSaturnS32(arg + 20);
    pThis->m5C          = 0;
    pThis->m24          = 0;
    pThis->m142_subState = 0;
    pThis->m38          = 0;

    initFieldModelRenderContext(
        &pThis->m10C_modelCtx,
        pThis,
        reinterpret_cast<void*>(&a5_wormHoleEntity_renderContextHook_060597da),
        &pThis->m40_renderPosition,
        nullptr,
        1, 0, -1, 0, 0);

    pThis->m_UpdateMethod = &a5WormHoleEntity_Update;
    pThis->m_DrawMethod   = &a5WormHoleEntity_Draw;
}

// 0605a190 — create 4 worm-hole environment objects (subfield 0).
void createA5_envObjects_sub0(p_workArea parent)
{
    a5_createWormHoleEntity_06059f1c(parent, gFLD_A5->getSaturnPtr(0x06099DE8));
    a5_createWormHoleEntity_06059f1c(parent, gFLD_A5->getSaturnPtr(0x06099E0C));
    a5_createWormHoleEntity_06059f1c(parent, gFLD_A5->getSaturnPtr(0x06099E30));
    a5_createWormHoleEntity_06059f1c(parent, gFLD_A5->getSaturnPtr(0x06099E54));
}
