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

void enableTownHooks() {
    resetCollisionFrame_intercept.enable();
    getCellAtWorldPos_intercept.enable();
    processTownMeshCollision_intercept.enable();
    handleCollisionWithTownEnv_intercept.enable();
    scriptFunction_6057058_sub0Sub0_intercept.enable();
    updateEdgePositionSub1_intercept.enable();
}
