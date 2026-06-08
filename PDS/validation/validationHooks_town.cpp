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

constexpr u32 kResetCollisionFrameEntry = 0x060079f0;
constexpr u32 kGetCellAtWorldPosEntry = 0x06014f70;       // R4 = x, R5 = z at entry
constexpr u32 kProcessTownMeshCollisionEntry = 0x06009324;
constexpr u32 kHandleCollisionWithTownEnvEntry = 0x0600887c;
constexpr u32 kComputeCollisionSeparationEntry = 0x06007c50;
constexpr u32 kGContactFaces = 0x0604a188;       // std::array<sContactFace,12>, 0x14 stride
constexpr u32 kGContactConstraints = 0x0604a170; // s32 m0/m4/m8/mC

constexpr u32 kGCollisionPositionBias = 0x0604a180;
constexpr u32 kGTownGrid = 0x060526dc; // m0_sizeX @ +0, m4_sizeY @ +4
constexpr u32 kPCurrentMatrixPtr = 0x0604aea8; // pointer global -> the current sMatrix4x3

constexpr u32 kTwnMainLogicTask = 0x06052658;

constexpr u32 kMainLogic_m14_EdgeTask = 0x14;
constexpr u32 kMainLogic_m38_interpolatedCameraPosition = 0x38;
constexpr u32 kMainLogic_m44_cameraTarget = 0x44;
constexpr u32 kMainLogic_m5C_rawCameraPosition = 0x5C;
constexpr u32 kMainLogic_m68_cameraRotation = 0x68;

constexpr u32 kMainLogic_m18_position = 0x18;
constexpr u32 kMainLogic_m24_distance = 0x24;
constexpr u32 kMainLogic_m2C = 0x2C;
constexpr u32 kMainLogic_m30 = 0x30;
constexpr u32 kMainLogic_m50_upVector = 0x50;

constexpr u32 kMainLogic_camera_m8_position = 0x74 + 0x8;
constexpr u32 kMainLogic_camera_m20_AABBCenter = 0x74 + 0x20;
constexpr u32 kMainLogic_camera_m44 = 0x74 + 0x44;
constexpr u32 kMainLogic_camera_m4C = 0x74 + 0x4C;
constexpr u32 kMainLogic_camera_m58_collisionSolveTranslation = 0x74 + 0x58;

constexpr u32 kEdge_mE8 = 0xE8;
constexpr u32 kNpcE8_position = 0x0;
constexpr u32 kNpcE8_rotation = 0xC;
constexpr u32 kNpcE8_stepRotation = 0x24; // feeds the m44 camera-target rotation (varC)

constexpr u32 kEdge_m14C_inputFlags = 0x14C;
constexpr u32 kEdge_m150_inputX = 0x150;
constexpr u32 kEdge_m154_inputY = 0x154;

constexpr u32 kNpcE8_stepTranslationInWorld = 0x18;
constexpr u32 kNpcE8_oldPosition = 0x54;
constexpr u32 kNpcE8_stepTranslation = 0x30;

constexpr u32 kEdge_m84_m20_AABBCenter = 0x84 + 0x20;
constexpr u32 kEdge_m84_m8_position = 0x84 + 0x8;
constexpr u32 kEdge_m84_m44_contactFlags = 0x84 + 0x44;
constexpr u32 kEdge_m84_m4C = 0x84 + 0x4C;
constexpr u32 kEdge_m84_m58_collisionSolveTranslation = 0x84 + 0x58;

static void validateTownEdgeAndCamera() {
    if (kTwnMainLogicTask == 0 || twnMainLogicTask == nullptr) {
        return;
    }
    const u32 mainLogic = g_validationConnection->readU32(kTwnMainLogicTask);
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
    const u32 edge = g_validationConnection->readU32(mainLogic + kMainLogic_m14_EdgeTask);
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

DECLARE_HOOK_VOID(resetCollisionFrame, kResetCollisionFrameEntry, void)

void resetCollisionFrame_detour() {
    if (g_validationConnection != nullptr && isValidationContextEnabled(VCTX_Town)) {
        g_validationConnection->executeUntilAddress(kResetCollisionFrameEntry);
        validate(kGCollisionPositionBias, (s32)gCollisionPositionBias);
        validateTownEdgeAndCamera();
    }
    resetCollisionFrame_intercept.callUndetoured();
}

DECLARE_HOOK(getCellAtWorldPos, kGetCellAtWorldPosEntry, sTownCellTask *, fixedPoint, fixedPoint)

sTownCellTask *getCellAtWorldPos_detour(fixedPoint r4_x, fixedPoint r5_z) {
    if (g_validationConnection != nullptr && isValidationContextEnabled(VCTX_Town)) {
        g_validationConnection->executeUntilAddress(kGetCellAtWorldPosEntry);
        validateRegister(azelval::REG_R0 + 4, (u32)r4_x.asS32());
        validateRegister(azelval::REG_R0 + 5, (u32)r5_z.asS32());
        validate(kGTownGrid + 0, (s32)gTownGrid.m0_sizeX);
        validate(kGTownGrid + 4, (s32)gTownGrid.m4_sizeY);
    }
    return getCellAtWorldPos_intercept.callUndetoured(r4_x, r5_z);
}

DECLARE_HOOK(processTownMeshCollision, kProcessTownMeshCollisionEntry, void, sCollisionBody *, const sProcessed3dModel *)

void processTownMeshCollision_detour(sCollisionBody *r4, const sProcessed3dModel *r5) {
    if (g_validationConnection != nullptr && isValidationContextEnabled(VCTX_Town)) {
        g_validationConnection->executeUntilAddress(kProcessTownMeshCollisionEntry);
        const u32 matrixAddr = g_validationConnection->readU32(kPCurrentMatrixPtr);
        validate(matrixAddr, *pCurrentMatrix);
    }
    processTownMeshCollision_intercept.callUndetoured(r4, r5);
}

DECLARE_HOOK(handleCollisionWithTownEnv, kHandleCollisionWithTownEnvEntry, void, sCollisionBody *)

void handleCollisionWithTownEnv_detour(sCollisionBody *r4) {
    if (g_validationConnection != nullptr && isValidationContextEnabled(VCTX_Town)) {
        g_validationConnection->executeUntilAddress(kHandleCollisionWithTownEnvEntry);
        const u32 emuBody = g_validationConnection->getRegister(azelval::REG_R0 + 4); // R4 = body pointer

        validate(emuBody + 0x20, r4->m20_AABBCenter);

        validate(emuBody + 0x58, r4->m58_collisionSolveTranslation);
        const u32 emuPosPtr = g_validationConnection->readU32(emuBody + 0x30); // m30_pPosition (4-byte Saturn ptr)

        if (twnMainLogicTask != nullptr && twnMainLogicTask->m14_EdgeTask != nullptr &&
            r4->m30_pPosition == &twnMainLogicTask->m14_EdgeTask->mE8.m0_position) {

            const u32 mainLogic = g_validationConnection->readU32(kTwnMainLogicTask);
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

// Contact-table inputs at entry, before the m44 resolution block runs
DECLARE_HOOK(computeCollisionSeparation, kComputeCollisionSeparationEntry, void, sCollisionBody *)

void computeCollisionSeparation_detour(sCollisionBody *r4) {
    if (g_validationConnection != nullptr && isValidationContextEnabled(VCTX_Town)) {
        g_validationConnection->executeUntilAddress(kComputeCollisionSeparationEntry);
        const u32 emuBody = g_validationConnection->getRegister(azelval::REG_R0 + 4);

        validate(emuBody + 0x8, r4->m8_position);
        validate(emuBody + 0x20, r4->m20_AABBCenter);
        validate(emuBody + 0x14, r4->m14_halfAABB);
        validate(emuBody + 0x44, (s32)r4->m44);
        // m0_position/m10_y stay stale for un-recorded faces; compare them only when the recorded bit is set
        static const s32 kFaceBit[12] = {0x20, 0x10, 0x4, 0x8, 0x2, 0x1,
                                         0x2000, 0x1000, 0x400, 0x800, 0x200, 0x100};
        for (u32 i = 0; i < 12; i++) {
            const u32 base = kGContactFaces + i * 0x14;
            validate(base + 0xC, (s32)gContactFaces[i].mC_distance);
            if (r4->m44 & kFaceBit[i]) {
                validate(base + 0x0, gContactFaces[i].m0_position);
                validate(base + 0x10, gContactFaces[i].m10_y);
            }
        }
        validate(kGContactConstraints + 0x0, (s32)gContactConstraints.m0);
        validate(kGContactConstraints + 0x4, (s32)gContactConstraints.m4);
        validate(kGContactConstraints + 0x8, (s32)gContactConstraints.m8);
        validate(kGContactConstraints + 0xC, (s32)gContactConstraints.mC);
        const u32 emuRot = g_validationConnection->readU32(emuBody + 0x34); // m34_pRotation (Saturn 4-byte ptr)
        if (emuRot != 0 && r4->m34_pRotation != nullptr)
            validate(emuRot, *r4->m34_pRotation);
    }
    computeCollisionSeparation_intercept.callUndetoured(r4);
}


constexpr u32 kCameraSetupReturn = 0x0605704c;

DECLARE_HOOK_VOID(scriptFunction_6057058_sub0Sub0, kCameraSetupReturn, void)

void scriptFunction_6057058_sub0Sub0_detour() {
    scriptFunction_6057058_sub0Sub0_intercept.callUndetoured();
    if (g_validationConnection == nullptr || !isValidationContextEnabled(VCTX_Town)) {
        return;
    }
    g_validationConnection->executeUntilAddress(kCameraSetupReturn);
    const u32 mainLogic = g_validationConnection->readU32(kTwnMainLogicTask);
    if (mainLogic != 0) {
        validate(mainLogic + kMainLogic_m5C_rawCameraPosition, twnMainLogicTask->m5C_rawCameraPosition);
        validate(mainLogic + kMainLogic_m38_interpolatedCameraPosition, twnMainLogicTask->m38_interpolatedCameraPosition);
        validate(mainLogic + kMainLogic_m50_upVector, twnMainLogicTask->m50_upVector);
        validate(mainLogic + kMainLogic_m44_cameraTarget, twnMainLogicTask->m44_cameraTarget);
    }
}

// TWN_RUIN updateEdgePositionSub1; the return is the instruction after the bsr's delay slot
constexpr u32 kUpdateEdgePositionSub1Entry = 0x0605bcc4;
constexpr u32 kUpdateEdgePositionSub1Return = 0x0605b8f6;

DECLARE_HOOK(updateEdgePositionSub1, kUpdateEdgePositionSub1Entry, void, sEdgeTask *)

void updateEdgePositionSub1_detour(sEdgeTask *r4) {
    if (g_validationConnection == nullptr || !isValidationContextEnabled(VCTX_Town)) {
        updateEdgePositionSub1_intercept.callUndetoured(r4);
        return;
    }

    g_validationConnection->executeUntilAddress(kUpdateEdgePositionSub1Entry);
    const u32 edge = g_validationConnection->getRegister(azelval::REG_R0 + 4); // R4 = sEdgeTask*

    // Branch selector: m0 >= 2 leaves stepRotation/stepTranslation untouched
    const u32 mainLogic = g_validationConnection->readU32(kTwnMainLogicTask);
    if (mainLogic != 0 && twnMainLogicTask != nullptr) {
        validate(mainLogic + 0x0, (s8)twnMainLogicTask->m0);
        validate(mainLogic + 0x2, (s8)twnMainLogicTask->m2_cameraFollowMode);
    }

    validate(edge + kEdge_mE8 + kNpcE8_stepRotation, r4->mE8.m24_stepRotation);
    validate(edge + kEdge_mE8 + kNpcE8_stepTranslation, r4->mE8.m30_stepTranslation);
    validate(edge + kEdge_m14C_inputFlags, r4->m14C_inputFlags);
    validate(edge + kEdge_m150_inputX, r4->m150_inputX);
    validate(edge + kEdge_m154_inputY, r4->m154_inputY);

    updateEdgePositionSub1_intercept.callUndetoured(r4);

    g_validationConnection->executeUntilAddress(kUpdateEdgePositionSub1Return);
    validate(edge + kEdge_mE8 + kNpcE8_stepRotation, r4->mE8.m24_stepRotation);
    validate(edge + kEdge_mE8 + kNpcE8_stepTranslation, r4->mE8.m30_stepTranslation);
    validate(edge + kEdge_m14C_inputFlags, r4->m14C_inputFlags);
}

DECLARE_HOOK(updateEdgePosition, 0, void, sNPC *)

void updateEdgePosition_detour(sNPC *r4) {
    const bool active = g_validationConnection != nullptr && isValidationContextEnabled(VCTX_Town) &&
                        twnMainLogicTask != nullptr && twnMainLogicTask->m14_EdgeTask != nullptr;
    if (active) {
        g_validationConnection->executeUntilAddress(0x0605b8d4);
        const u32 mainLogic = g_validationConnection->readU32(kTwnMainLogicTask);
        const u32 edge = mainLogic ? g_validationConnection->readU32(mainLogic + kMainLogic_m14_EdgeTask) : 0;
        if (edge != 0) {
            validate(edge + kEdge_mE8 + kNpcE8_position, twnMainLogicTask->m14_EdgeTask->mE8.m0_position);
            // Branch selectors (m44, m4C) before the result (m58)
            const sCollisionBody &body = twnMainLogicTask->m14_EdgeTask->m84;
            validate(edge + kEdge_m84_m44_contactFlags, (s32)body.m44);
            validate(edge + kEdge_m84_m4C, body.m4C);
            validate(edge + kEdge_m84_m58_collisionSolveTranslation, body.m58_collisionSolveTranslation);
        }
    }
    updateEdgePosition_intercept.callUndetoured(r4);
    if (!active)
        return;
    g_validationConnection->executeUntilAddress(0x0605bc38);
    const u32 mainLogic = g_validationConnection->readU32(kTwnMainLogicTask);
    const u32 edge = mainLogic ? g_validationConnection->readU32(mainLogic + kMainLogic_m14_EdgeTask) : 0;
    if (edge != 0) {
        validate(edge + kEdge_mE8 + kNpcE8_stepTranslationInWorld, twnMainLogicTask->m14_EdgeTask->mE8.m18_stepTranslationInWorld);
        validate(edge + kEdge_mE8 + kNpcE8_position, twnMainLogicTask->m14_EdgeTask->mE8.m0_position);
    }
}

// Entry is before the collision-solve add into m5C
DECLARE_HOOK(cameraUpdate_follow, 0x06055db6, void, sMainLogic *)

void cameraUpdate_follow_detour(sMainLogic *r4) {
    if (g_validationConnection != nullptr && isValidationContextEnabled(VCTX_Town)) {
        g_validationConnection->executeUntilAddress(0x06055db6);
        const u32 mainLogic = g_validationConnection->readU32(kTwnMainLogicTask);
        if (mainLogic != 0) {
            validate(mainLogic + kMainLogic_m18_position, r4->m18_position);
            validate(mainLogic + kMainLogic_m5C_rawCameraPosition, r4->m5C_rawCameraPosition);
            // Camera body holds this frame's collision solve (m58 is added into the camera position)
            validate(mainLogic + kMainLogic_m24_distance, r4->m24_distance);
            validate(mainLogic + kMainLogic_camera_m44, (s32)r4->m74_townCamera.m44);
            validate(mainLogic + kMainLogic_camera_m4C, r4->m74_townCamera.m4C);
            validate(mainLogic + kMainLogic_camera_m58_collisionSolveTranslation, r4->m74_townCamera.m58_collisionSolveTranslation);
            const u32 emuCamPos = g_validationConnection->readU32(mainLogic + 0x74 + 0x30); // camera m30_pPosition
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
