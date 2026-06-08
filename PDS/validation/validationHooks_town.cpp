#include "PDS.h"

#include "3dEngine.h"
#include "kernel/worldGrid.h"
#include "town/town.h"
#include "town/townMainLogic.h"
#include "town/townEdge.h"
#include "town/collisionRegistry.h"

#include "validation/validation.h"
#include "validation/validationHooks_town.h"

extern s32 gCollisionPositionBias;

namespace {
constexpr std::uint32_t kResetCollisionFrameEntry = 0x060079f0;
constexpr std::uint32_t kGetCellAtWorldPosEntry = 0x06014f70;       // R4 = x, R5 = z at entry
constexpr std::uint32_t kProcessTownMeshCollisionEntry = 0x06009324;
constexpr std::uint32_t kHandleCollisionWithTownEnvEntry = 0x0600887c;
constexpr std::uint32_t kComputeCollisionSeparationEntry = 0x06007c50;
constexpr std::uint32_t kGContactFaces = 0x0604a188;       // std::array<sContactFace,12>, 0x14 stride
constexpr std::uint32_t kGContactConstraints = 0x0604a170; // s32 m0/m4/m8/mC

constexpr std::uint32_t kGCollisionPositionBias = 0x0604a180;
constexpr std::uint32_t kGTownGrid = 0x060526dc; // m0_sizeX @ +0, m4_sizeY @ +4
constexpr std::uint32_t kPCurrentMatrixPtr = 0x0604aea8; // pointer global -> the current sMatrix4x3

constexpr std::uint32_t kTwnMainLogicTask = 0x06052658;

constexpr std::uint32_t kMainLogic_m14_EdgeTask = 0x14;
constexpr std::uint32_t kMainLogic_m38_interpolatedCameraPosition = 0x38;
constexpr std::uint32_t kMainLogic_m44_cameraTarget = 0x44;
constexpr std::uint32_t kMainLogic_m5C_rawCameraPosition = 0x5C;
constexpr std::uint32_t kMainLogic_m68_cameraRotation = 0x68;

constexpr std::uint32_t kMainLogic_m18_position = 0x18;
constexpr std::uint32_t kMainLogic_m24_distance = 0x24;
constexpr std::uint32_t kMainLogic_m2C = 0x2C;
constexpr std::uint32_t kMainLogic_m30 = 0x30;
constexpr std::uint32_t kMainLogic_m50_upVector = 0x50;

constexpr std::uint32_t kMainLogic_camera_m8_position = 0x74 + 0x8;
constexpr std::uint32_t kMainLogic_camera_m20_AABBCenter = 0x74 + 0x20;
constexpr std::uint32_t kMainLogic_camera_m44 = 0x74 + 0x44;
constexpr std::uint32_t kMainLogic_camera_m4C = 0x74 + 0x4C;
constexpr std::uint32_t kMainLogic_camera_m58_collisionSolveTranslation = 0x74 + 0x58;

constexpr std::uint32_t kEdge_mE8 = 0xE8;
constexpr std::uint32_t kNpcE8_position = 0x0;
constexpr std::uint32_t kNpcE8_rotation = 0xC;
constexpr std::uint32_t kNpcE8_stepRotation = 0x24; // feeds the m44 camera-target rotation (varC)

constexpr std::uint32_t kEdge_m14C_inputFlags = 0x14C;
constexpr std::uint32_t kEdge_m150_inputX = 0x150;
constexpr std::uint32_t kEdge_m154_inputY = 0x154;

constexpr std::uint32_t kNpcE8_stepTranslationInWorld = 0x18;
constexpr std::uint32_t kNpcE8_oldPosition = 0x54;
constexpr std::uint32_t kNpcE8_stepTranslation = 0x30;

constexpr std::uint32_t kEdge_m84_m20_AABBCenter = 0x84 + 0x20;
constexpr std::uint32_t kEdge_m84_m8_position = 0x84 + 0x8;
constexpr std::uint32_t kEdge_m84_m44_contactFlags = 0x84 + 0x44;
constexpr std::uint32_t kEdge_m84_m4C = 0x84 + 0x4C;
constexpr std::uint32_t kEdge_m84_m58_collisionSolveTranslation = 0x84 + 0x58;
} // namespace

static void validateTownEdgeAndCamera() {
    if (kTwnMainLogicTask == 0 || twnMainLogicTask == nullptr) {
        return;
    }
    const std::uint32_t mainLogic = g_validationConnection->readU32(kTwnMainLogicTask);
    if (mainLogic == 0) {
        return;
    }

    validate(kGTownGrid + 0x28, gTownGrid.m28_cellSize);

    // Camera
    validate(mainLogic + kMainLogic_m18_position, twnMainLogicTask->m18_position);
    validate(mainLogic + kMainLogic_m24_distance, twnMainLogicTask->m24_distance);
    validate(mainLogic + kMainLogic_m2C, twnMainLogicTask->m2C);
    validate(mainLogic + kMainLogic_m30, twnMainLogicTask->m30);

    // Camera position/rotation
    validate(mainLogic + kMainLogic_m38_interpolatedCameraPosition, twnMainLogicTask->m38_interpolatedCameraPosition);
    validate(mainLogic + kMainLogic_m44_cameraTarget, twnMainLogicTask->m44_cameraTarget);
    validate(mainLogic + kMainLogic_m50_upVector, twnMainLogicTask->m50_upVector);
    validate(mainLogic + kMainLogic_m5C_rawCameraPosition, twnMainLogicTask->m5C_rawCameraPosition);
    validate(mainLogic + kMainLogic_m68_cameraRotation, twnMainLogicTask->m68_cameraRotation);

    // Camera collision
    validate(mainLogic + kMainLogic_camera_m20_AABBCenter, twnMainLogicTask->m74_townCamera.m20_AABBCenter);
    validate(mainLogic + kMainLogic_camera_m8_position, twnMainLogicTask->m74_townCamera.m8_position);
    validate(mainLogic + kMainLogic_camera_m4C, twnMainLogicTask->m74_townCamera.m4C);
    validate(mainLogic + kMainLogic_camera_m58_collisionSolveTranslation, twnMainLogicTask->m74_townCamera.m58_collisionSolveTranslation);

    // Edge position/rotation
    const std::uint32_t edge = g_validationConnection->readU32(mainLogic + kMainLogic_m14_EdgeTask);
    if (edge == 0 || twnMainLogicTask->m14_EdgeTask == nullptr) {
        return;
    }
    validate(edge + kEdge_mE8 + kNpcE8_position, twnMainLogicTask->m14_EdgeTask->mE8.m0_position);
    validate(edge + kEdge_mE8 + kNpcE8_rotation, twnMainLogicTask->m14_EdgeTask->mE8.mC_rotation);
    validate(edge + kEdge_mE8 + kNpcE8_stepRotation, twnMainLogicTask->m14_EdgeTask->mE8.m24_stepRotation);

    // stepNPCForward
    validate(edge + kEdge_mE8 + kNpcE8_stepTranslation, twnMainLogicTask->m14_EdgeTask->mE8.m30_stepTranslation);
    validate(edge + kEdge_mE8 + kNpcE8_stepTranslationInWorld, twnMainLogicTask->m14_EdgeTask->mE8.m18_stepTranslationInWorld);
    validate(edge + kEdge_mE8 + kNpcE8_oldPosition, twnMainLogicTask->m14_EdgeTask->mE8.m54_oldPosition);

    // Edge collision
    validate(edge + kEdge_m84_m20_AABBCenter, twnMainLogicTask->m14_EdgeTask->m84.m20_AABBCenter);
    validate(edge + kEdge_m84_m8_position, twnMainLogicTask->m14_EdgeTask->m84.m8_position);
}

// --- Point 6: resetCollisionFrame (gCollisionPositionBias) -----------------------------------------------------------
DECLARE_HOOK_VOID(resetCollisionFrame, kResetCollisionFrameEntry, void)

void resetCollisionFrame_detour() {
    if (g_validationConnection != nullptr && isValidationContextEnabled(VCTX_Town)) {
        g_validationConnection->executeUntilAddress(kResetCollisionFrameEntry);
        validate(kGCollisionPositionBias, (std::int32_t)gCollisionPositionBias);
        validateTownEdgeAndCamera();
    }
    resetCollisionFrame_intercept.callUndetoured();
}

// --- Points 2 + 3: getCellAtWorldPos (input coords + grid dimensions) ------------------------------------------------
// 06014f70
DECLARE_HOOK(getCellAtWorldPos, kGetCellAtWorldPosEntry, sTownCellTask *, fixedPoint, fixedPoint)

sTownCellTask *getCellAtWorldPos_detour(fixedPoint r4_x, fixedPoint r5_z) {
    if (g_validationConnection != nullptr && isValidationContextEnabled(VCTX_Town)) {
        g_validationConnection->executeUntilAddress(kGetCellAtWorldPosEntry);
        validateRegister(azelval::REG_R0 + 4, (std::uint32_t)r4_x.asS32());
        validateRegister(azelval::REG_R0 + 5, (std::uint32_t)r5_z.asS32());
        validate(kGTownGrid + 0, (std::int32_t)gTownGrid.m0_sizeX);
        validate(kGTownGrid + 4, (std::int32_t)gTownGrid.m4_sizeY);
    }
    return getCellAtWorldPos_intercept.callUndetoured(r4_x, r5_z);
}

// --- Point 4: processTownMeshCollision (current matrix) --------------------------------------------------------------
// 06009324
DECLARE_HOOK(processTownMeshCollision, kProcessTownMeshCollisionEntry, void, sCollisionBody *, const sProcessed3dModel *)

void processTownMeshCollision_detour(sCollisionBody *r4, const sProcessed3dModel *r5) {
    if (g_validationConnection != nullptr && isValidationContextEnabled(VCTX_Town)) {
        g_validationConnection->executeUntilAddress(kProcessTownMeshCollisionEntry);
        const std::uint32_t matrixAddr = g_validationConnection->readU32(kPCurrentMatrixPtr);
        validate(matrixAddr, *pCurrentMatrix);
    }
    processTownMeshCollision_intercept.callUndetoured(r4, r5);
}

DECLARE_HOOK(handleCollisionWithTownEnv, kHandleCollisionWithTownEnvEntry, void, sCollisionBody *)

void handleCollisionWithTownEnv_detour(sCollisionBody *r4) {
    if (g_validationConnection != nullptr && isValidationContextEnabled(VCTX_Town)) {
        g_validationConnection->executeUntilAddress(kHandleCollisionWithTownEnvEntry);
        const std::uint32_t emuBody = g_validationConnection->getRegister(azelval::REG_R0 + 4); // R4 = body pointer

        validate(emuBody + 0x20, r4->m20_AABBCenter);

        validate(emuBody + 0x58, r4->m58_collisionSolveTranslation);
        const std::uint32_t emuPosPtr = g_validationConnection->readU32(emuBody + 0x30); // m30_pPosition (4-byte Saturn ptr)

        if (twnMainLogicTask != nullptr && twnMainLogicTask->m14_EdgeTask != nullptr &&
            r4->m30_pPosition == &twnMainLogicTask->m14_EdgeTask->mE8.m0_position) {

            const std::uint32_t mainLogic = g_validationConnection->readU32(kTwnMainLogicTask);
            if (mainLogic != 0) {
                validate(mainLogic + 0x134, twnMainLogicTask->m134_autoWalkPositionStep);
                validate(mainLogic + 0x11C, twnMainLogicTask->m11C_autoWalkStartPosition);
            }
            const sNPCE8 &e8 = twnMainLogicTask->m14_EdgeTask->mE8;
            validate(emuPosPtr + 0x54, e8.m54_oldPosition);
            validate(emuPosPtr + 0xC, e8.mC_rotation);
            validate(emuPosPtr + 0x30, e8.m30_stepTranslation);
            validate(emuPosPtr + 0x18, e8.m18_stepTranslationInWorld);
        }
        if (r4->m30_pPosition != nullptr) {
            validate(emuPosPtr + 0x0, (*r4->m30_pPosition)[0]);
            validate(emuPosPtr + 0x4, (*r4->m30_pPosition)[1]);
            validate(emuPosPtr + 0x8, (*r4->m30_pPosition)[2]);
        }
        validate(emuBody + 0x8, r4->m8_position);
    }
    handleCollisionWithTownEnv_intercept.callUndetoured(r4);
}

// 06007c50 computeCollisionSeparation -- validate the contact-table INPUTS at entry (before the m44 resolution
// block runs) for the edge body. If these all match but m58 still diverges, the bug is the X/Z solve arithmetic;
// if one differs, the bug is upstream in recordContact*/testTownMeshQuadForCollision.
DECLARE_HOOK(computeCollisionSeparation, kComputeCollisionSeparationEntry, void, sCollisionBody *)

void computeCollisionSeparation_detour(sCollisionBody *r4) {
    if (g_validationConnection != nullptr && isValidationContextEnabled(VCTX_Town)) {
        g_validationConnection->executeUntilAddress(kComputeCollisionSeparationEntry);
        const std::uint32_t emuBody = g_validationConnection->getRegister(azelval::REG_R0 + 4);

        // Body identity first: if PDS r4 and the emu body have desynced (a different body at this Nth call),
        // these diverge before any contact data is compared.
        validate(emuBody + 0x8, r4->m8_position);
        validate(emuBody + 0x20, r4->m20_AABBCenter);
        validate(emuBody + 0x14, r4->m14_halfAABB);
        validate(emuBody + 0x44, (std::int32_t)r4->m44);
        // beginBodyCollisionTest only resets mC_distance; m0_position/m10_y stay stale for un-recorded faces,
        // so only compare those when the face's recorded bit is set (gCF[i] <-> these m44 bits).
        static const std::int32_t kFaceBit[12] = {0x20, 0x10, 0x4, 0x8, 0x2, 0x1,
                                                  0x2000, 0x1000, 0x400, 0x800, 0x200, 0x100};
        for (std::uint32_t i = 0; i < 12; i++) {
            const std::uint32_t base = kGContactFaces + i * 0x14;
            validate(base + 0xC, (std::int32_t)gContactFaces[i].mC_distance);
            if (r4->m44 & kFaceBit[i]) {
                validate(base + 0x0, gContactFaces[i].m0_position);
                validate(base + 0x10, gContactFaces[i].m10_y);
            }
        }
        validate(kGContactConstraints + 0x0, (std::int32_t)gContactConstraints.m0);
        validate(kGContactConstraints + 0x4, (std::int32_t)gContactConstraints.m4);
        validate(kGContactConstraints + 0x8, (std::int32_t)gContactConstraints.m8);
        validate(kGContactConstraints + 0xC, (std::int32_t)gContactConstraints.mC);
        const std::uint32_t emuRot = g_validationConnection->readU32(emuBody + 0x34); // m34_pRotation (Saturn 4-byte ptr)
        if (emuRot != 0 && r4->m34_pRotation != nullptr)
            validate(emuRot, *r4->m34_pRotation);
    }
    computeCollisionSeparation_intercept.callUndetoured(r4);
}


namespace {
constexpr std::uint32_t kCameraSetupReturn = 0x0605704c;
} // namespace

DECLARE_HOOK_VOID(scriptFunction_6057058_sub0Sub0, kCameraSetupReturn, void)

void scriptFunction_6057058_sub0Sub0_detour() {
    scriptFunction_6057058_sub0Sub0_intercept.callUndetoured();
    if (g_validationConnection == nullptr || !isValidationContextEnabled(VCTX_Town)) {
        return;
    }
    g_validationConnection->executeUntilAddress(kCameraSetupReturn);
    const std::uint32_t mainLogic = g_validationConnection->readU32(kTwnMainLogicTask);
    if (mainLogic != 0) {
        validate(mainLogic + kMainLogic_m5C_rawCameraPosition, twnMainLogicTask->m5C_rawCameraPosition);
        validate(mainLogic + kMainLogic_m38_interpolatedCameraPosition, twnMainLogicTask->m38_interpolatedCameraPosition);
        validate(mainLogic + kMainLogic_m50_upVector, twnMainLogicTask->m50_upVector);
        validate(mainLogic + kMainLogic_m44_cameraTarget, twnMainLogicTask->m44_cameraTarget);
    }
}

// --- TWN_RUIN updateEdgePositionSub1 (before/after stepRotation + inputX/inputY) ------------------------------------
// 0605bcc4 entry; returns to updateEdgePosition at 0605b8f6 (instruction after the bsr's delay slot).
namespace {
constexpr std::uint32_t kUpdateEdgePositionSub1Entry = 0x0605bcc4;
constexpr std::uint32_t kUpdateEdgePositionSub1Return = 0x0605b8f6;
} // namespace

DECLARE_HOOK(updateEdgePositionSub1, kUpdateEdgePositionSub1Entry, void, sEdgeTask *)

void updateEdgePositionSub1_detour(sEdgeTask *r4) {
    if (g_validationConnection == nullptr || !isValidationContextEnabled(VCTX_Town)) {
        updateEdgePositionSub1_intercept.callUndetoured(r4);
        return;
    }

    // BEFORE: drive the guest to the function entry and compare the inputs it reads.
    g_validationConnection->executeUntilAddress(kUpdateEdgePositionSub1Entry);
    const std::uint32_t edge = g_validationConnection->getRegister(azelval::REG_R0 + 4); // R4 = sEdgeTask*

    // The branch selector: both the rotation and translation blocks switch on twnMainLogicTask->m0. If it diverges
    // the guest skips the writes entirely (m0 >= 2 leaves stepRotation/stepTranslation untouched), which is exactly
    // the leftover-value symptom seen on stepTranslation[2].
    const std::uint32_t mainLogic = g_validationConnection->readU32(kTwnMainLogicTask);
    if (mainLogic != 0 && twnMainLogicTask != nullptr) {
        validate(mainLogic + 0x0, (std::int8_t)twnMainLogicTask->m0);
        validate(mainLogic + 0x2, (std::int8_t)twnMainLogicTask->m2_cameraFollowMode);
    }

    validate(edge + kEdge_mE8 + kNpcE8_stepRotation, r4->mE8.m24_stepRotation);
    validate(edge + kEdge_mE8 + kNpcE8_stepTranslation, r4->mE8.m30_stepTranslation);
    validate(edge + kEdge_m14C_inputFlags, r4->m14C_inputFlags);
    validate(edge + kEdge_m150_inputX, r4->m150_inputX);
    validate(edge + kEdge_m154_inputY, r4->m154_inputY);

    updateEdgePositionSub1_intercept.callUndetoured(r4);

    // AFTER: drive the guest through the function (RTS lands back in updateEdgePosition) and compare the results.
    // The function rewrites stepRotation[1] / stepTranslation[0]/[2], masks inputFlags to &= 1, and may clamp
    // stepTranslation[1]; inputX/inputY are left untouched.
    g_validationConnection->executeUntilAddress(kUpdateEdgePositionSub1Return);
    validate(edge + kEdge_mE8 + kNpcE8_stepRotation, r4->mE8.m24_stepRotation);
    validate(edge + kEdge_mE8 + kNpcE8_stepTranslation, r4->mE8.m30_stepTranslation);
    validate(edge + kEdge_m14C_inputFlags, r4->m14C_inputFlags);
}

// 0605b8d4 (TWN_RUIN) updateEdgePosition -- entry unarmed (wraps nested hooks). Validate m0_position at entry (before
// the line-796 collision-solve add) and at the rts (after 796 + the line-946 movement) to split the two contributions.
DECLARE_HOOK(updateEdgePosition, 0, void, sNPC *)

void updateEdgePosition_detour(sNPC *r4) {
    const bool active = g_validationConnection != nullptr && isValidationContextEnabled(VCTX_Town) &&
                        twnMainLogicTask != nullptr && twnMainLogicTask->m14_EdgeTask != nullptr;
    if (active) {
        g_validationConnection->executeUntilAddress(0x0605b8d4);
        const std::uint32_t mainLogic = g_validationConnection->readU32(kTwnMainLogicTask);
        const std::uint32_t edge = mainLogic ? g_validationConnection->readU32(mainLogic + kMainLogic_m14_EdgeTask) : 0;
        if (edge != 0) {
            validate(edge + kEdge_mE8 + kNpcE8_position, twnMainLogicTask->m14_EdgeTask->mE8.m0_position);
            // processAllCollisions runs before updateEdgePosition, so m84's collision-solve outputs hold this
            // frame's final values here -- before line-796 adds m58 into position. Validating them at entry
            // isolates a ground-collision-solve divergence from the later gravity/movement contribution.
            // Order: branch selectors first (m44 contact/resolution flags, m4C ground contact), then the
            // result (m58). The first break then tells whether divergence is upstream (face detection picks a
            // different solve branch) or only in the X-axis math that produces m58.
            const sCollisionBody &body = twnMainLogicTask->m14_EdgeTask->m84;
            validate(edge + kEdge_m84_m44_contactFlags, (std::int32_t)body.m44);
            validate(edge + kEdge_m84_m4C, body.m4C);
            validate(edge + kEdge_m84_m58_collisionSolveTranslation, body.m58_collisionSolveTranslation);
        }
    }
    updateEdgePosition_intercept.callUndetoured(r4);
    if (!active)
        return;
    g_validationConnection->executeUntilAddress(0x0605bc38);
    const std::uint32_t mainLogic = g_validationConnection->readU32(kTwnMainLogicTask);
    const std::uint32_t edge = mainLogic ? g_validationConnection->readU32(mainLogic + kMainLogic_m14_EdgeTask) : 0;
    if (edge != 0) {
        validate(edge + kEdge_mE8 + kNpcE8_stepTranslationInWorld, twnMainLogicTask->m14_EdgeTask->mE8.m18_stepTranslationInWorld);
        validate(edge + kEdge_mE8 + kNpcE8_position, twnMainLogicTask->m14_EdgeTask->mE8.m0_position);
    }
}

// 06055db6 (TWN_RUIN) -- validate m18/m5C at entry, before the 408-412 collision-solve add into m5C
DECLARE_HOOK(cameraUpdate_follow, 0x06055db6, void, sMainLogic *)

void cameraUpdate_follow_detour(sMainLogic *r4) {
    if (g_validationConnection != nullptr && isValidationContextEnabled(VCTX_Town)) {
        g_validationConnection->executeUntilAddress(0x06055db6);
        const std::uint32_t mainLogic = g_validationConnection->readU32(kTwnMainLogicTask);
        if (mainLogic != 0) {
            validate(mainLogic + kMainLogic_m18_position, r4->m18_position);
            validate(mainLogic + kMainLogic_m5C_rawCameraPosition, r4->m5C_rawCameraPosition);
            // This runs after processAllCollisions, so the camera body holds THIS frame's collision solve --
            // the m58 that lines 408-412 add into the camera position before var4 = m18 - m5C at line 416.
            validate(mainLogic + kMainLogic_m24_distance, r4->m24_distance);
            validate(mainLogic + kMainLogic_camera_m44, (std::int32_t)r4->m74_townCamera.m44);
            validate(mainLogic + kMainLogic_camera_m4C, r4->m74_townCamera.m4C);
            validate(mainLogic + kMainLogic_camera_m58_collisionSolveTranslation, r4->m74_townCamera.m58_collisionSolveTranslation);
            const std::uint32_t emuCamPos = g_validationConnection->readU32(mainLogic + 0x74 + 0x30); // camera m30_pPosition
            if (emuCamPos != 0 && r4->m74_townCamera.m30_pPosition != nullptr)
                validate(emuCamPos, *r4->m74_townCamera.m30_pPosition);
        }
    }
    cameraUpdate_follow_intercept.callUndetoured(r4);
}

void enableTownHooks() {
    resetCollisionFrame_intercept.enable();
    updateEdgePosition_intercept.enable();
    cameraUpdate_follow_intercept.enable();
    getCellAtWorldPos_intercept.enable();
    processTownMeshCollision_intercept.enable();
    handleCollisionWithTownEnv_intercept.enable();
    computeCollisionSeparation_intercept.enable();
    scriptFunction_6057058_sub0Sub0_intercept.enable();
    updateEdgePositionSub1_intercept.enable();
}
