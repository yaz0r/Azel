#include "PDS.h"

#include "3dEngine.h"
#include "kernel/fade.h"
#include "kernel/worldGrid.h"
#include "town/town.h"
#include "town/townMainLogic.h"
#include "town/townEdge.h"
#include "town/collisionRegistry.h"
#include "town/townLCS.h"
#include "town/townScript.h"
#include "processModel.h"

#include "validation/validation.h"
#include "validation/validationHooks_town.h"
#include "town/zoah/twn_zoah.h"

#include <cstdio>
#include <string>
#include <unordered_map>

extern s32 gCollisionPositionBias;

constexpr u32 kResetCollisionFrameEntry = 0x060079f0;
constexpr u32 kGetCellAtWorldPosEntry = 0x06014f70;       // R4 = x, R5 = z at entry
constexpr u32 kProcessTownMeshCollisionEntry = 0x06009324;
constexpr u32 kHandleCollisionWithTownEnvEntry = 0x0600887c;
constexpr u32 kComputeCollisionSeparationEntry = 0x06007c50;
constexpr u32 kGContactFaces = 0x0604a188;       // std::array<sContactFace,12>, 0x14 stride
constexpr u32 kGContactConstraints = 0x0604a170; // s32 m0/m4/m8/mC

constexpr u32 kGCollisionPositionBias = 0x0604a180;
constexpr u32 kNpcData0 = 0x0604a284;     // npcData0 (sNpcData) base (mFC literal 0x0604a380 - 0xFC)
constexpr u32 kNpcData0_mFC = 0x0604a380; // npcData0 + 0xFC: bit0 gates the day/night timer increment
constexpr u32 kFileInfoStruct_allocatedHead = 0x0604bad8; // fileInfoStruct.m2C_allocatedHead: 0 == load done
constexpr u32 kGFadeControls = 0x0604b484;
constexpr u32 kGTownGrid = 0x060526dc; // m0_sizeX @ +0, m4_sizeY @ +4
constexpr u32 kPCurrentMatrixPtr = 0x0604aea8; // pointer global -> the current sMatrix4x3

constexpr u32 kTwnMainLogicTask = 0x06052658;
constexpr u32 kIsDataLoadedEntry = 0x06032140;
constexpr u32 kAddBackgroundScriptEntry = 0x0600ce58; // R4 = script start EA, R5 = script type
constexpr u32 kRunScriptEntry = 0x0600d008;           // R4 = &npcData0; returns the advanced script IP in R0

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

// Per-town overlay breakpoint PCs; 0 means that hook stays un-armed for the town
struct sTownValidationAddresses {
    u32 cameraSetupReturn;            // scriptFunction_6057058_sub0Sub0 return/stop
    u32 updateEdgePositionSub1Entry;
    u32 updateEdgePositionSub1Return;
    u32 updateEdgePositionEntry;
    u32 updateEdgePositionReturn;
    u32 cameraUpdateFollowEntry;
    u32 updateEdgeLookAtEntry;
    u32 updateEdgeLookAtReturn;
    u32 zoahCameraUpdateEntry;        // zoahCamera_update entry (Zoah only)
};

static const std::unordered_map<std::string, sTownValidationAddresses> kTownValidationAddresses = {
    {"TWN_RUIN.PRG", {0x0605704c, 0x0605bcc4, 0x0605b8f6, 0x0605b8d4, 0x0605bc38, 0x06055db6, 0x0605beb8, 0x0605bc02, 0}},
    {"TWN_ZOAH.PRG", {0x06098a0c, 0,          0,          0x0609d294, 0x0609d5f8, 0x06097776, 0,          0,          0x0609e3fe}},
};

static const sTownValidationAddresses* gActiveTownValidationAddrs = nullptr;
static u32 gSatCameraTaskAddr = 0;

// Returns true on the frame the town changed
static bool refreshTownValidationAddresses();

static void syncFadeControlsFromGuest();

static void validateTownEdgeAndCamera() {
    if (kTwnMainLogicTask == 0 || twnMainLogicTask == nullptr) {
        return;
    }
    const u32 mainLogic = g_validationConnection->readU32(kTwnMainLogicTask);
    if (mainLogic == 0) {
        return;
    }

    validate(kGTownGrid + 0x28, gTownGrid.m28_cellSize);

    // Script-run state; mFC bit0 gates the day/night timer
    validate(kNpcData0 + 0x0, (s32)npcData0.m0_numBackgroundScripts);
    validate(kNpcData0 + 0x100, (s32)npcData0.m100);
    validate(kNpcData0 + 0x104, (s32)npcData0.m104_currentScript.m0_scriptPtr.m_offset);
    validate(kNpcData0_mFC, (s32)npcData0.mFC);

    validate(mainLogic + kMainLogic_m18_position, twnMainLogicTask->m18_position);
    validate(mainLogic + kMainLogic_m68_cameraRotation, twnMainLogicTask->m68_cameraRotation);
    validate(mainLogic + kMainLogic_m30, twnMainLogicTask->m30);
    validate(mainLogic + kMainLogic_m24_distance, twnMainLogicTask->m24_distance);
    validate(mainLogic + kMainLogic_m5C_rawCameraPosition, twnMainLogicTask->m5C_rawCameraPosition);
    validate(mainLogic + kMainLogic_m38_interpolatedCameraPosition, twnMainLogicTask->m38_interpolatedCameraPosition);
    validate(mainLogic + kMainLogic_m44_cameraTarget, twnMainLogicTask->m44_cameraTarget);
    validate(mainLogic + kMainLogic_m50_upVector, twnMainLogicTask->m50_upVector);
    validate(mainLogic + kMainLogic_m2C, twnMainLogicTask->m2C);

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

    // Ground contact; Zoah's per-frame edge hooks are un-armed, so m44 is only checked here
    validate(edge + kEdge_m84_m44_contactFlags, (s32)twnMainLogicTask->m14_EdgeTask->m84.m44);
    validate(edge + kEdge_m84_m4C, twnMainLogicTask->m14_EdgeTask->m84.m4C);
    validate(edge + kEdge_m84_m58_collisionSolveTranslation, twnMainLogicTask->m14_EdgeTask->m84.m58_collisionSolveTranslation);

    // Edge animation state; m14E is the idle re-roll countdown
    const sEdgeTask* edgeTask = twnMainLogicTask->m14_EdgeTask;

    // Anim identity before playback position; the model sits at edge+0x34
    validate(edge + 0x2C, edgeTask->m2C_currentAnimation);
    validate(edge + 0x34 + 0x10, (u16)edgeTask->m34_3dModel.m10_currentAnimationFrame);
    validate(edge + 0x34 + 0x16, (u16)edgeTask->m34_3dModel.m16_previousAnimationFrame);
    validate(edge + 0x178, (s8)edgeTask->m178); // write index
    for (int i = 0; i < 16; i++) // queued {animId, mode} pairs
        validate(edge + 0x158 + i, (s8)edgeTask->m158_animQueue[i]);
    validate(edge + 0x179, (s8)edgeTask->m179); // read index
    validate(edge + 0x17A, (s8)edgeTask->m17A); // pending count
    validate(edge + 0xE, (s8)edgeTask->mE_controlState);
    validate(edge + 0x28, edgeTask->m28_animationLeftOver);
    validate(edge + 0x14E, (s16)edgeTask->m14E);

    // Zoah camera outputs
    if (gSatCameraTaskAddr != 0 && cameraTaskPtr != nullptr) {
        validate(gSatCameraTaskAddr + 0x4, (s32)cameraTaskPtr->m4_dayNightTimer);
        for (int i = 0; i < 12; ++i)
            validate(gSatCameraTaskAddr + 0x34 + i * 4, cameraTaskPtr->m34_interpolatedLightData[i]);
        validate(gSatCameraTaskAddr + 0x10, (u32)cameraTaskPtr->m10.toU32());
    }
}

DECLARE_HOOK(isDataLoaded, kIsDataLoadedEntry, s32, s32)

s32 isDataLoaded_detour(s32 fileIndex) {
    if (g_validationConnection == nullptr || !isValidationContextEnabled(VCTX_Town)) {
        return isDataLoaded_intercept.callUndetoured(fileIndex);
    }
    g_validationConnection->executeUntilAddress(kIsDataLoadedEntry);
    const u32 returnAddr = g_validationConnection->getRegister(azelval::REG_PR);
    g_validationConnection->executeUntilAddress(returnAddr);
    return (s32)g_validationConnection->getRegister(azelval::REG_R0 + 0);
}

DECLARE_HOOK_VOID(resetCollisionFrame, kResetCollisionFrameEntry, void)

void resetCollisionFrame_detour() {
    if (g_validationConnection != nullptr && isValidationContextEnabled(VCTX_Town)) {
        // The first resetCollisionFrame after a town change is the init-time one; drop the per-frame
        // breakpoints so the drive can run the guest through the original's async load frames
        const bool townChanged = refreshTownValidationAddresses();
        if (townChanged) {
            setFrameSyncBreakpointsEnabled(false);
        }
        g_validationConnection->executeUntilAddress(kResetCollisionFrameEntry);
        if (townChanged) {
            setFrameSyncBreakpointsEnabled(true);
        }
        syncFadeControlsFromGuest();
        validate(kGCollisionPositionBias, (s32)gCollisionPositionBias);
        validateTownEdgeAndCamera();
    }
    resetCollisionFrame_intercept.callUndetoured();
}

DECLARE_HOOK(addBackgroundScript, kAddBackgroundScriptEntry, void, sSaturnPtr, s32, p_workArea, const sVec3_S16_12_4 *)

void addBackgroundScript_detour(sSaturnPtr r4, s32 r5, p_workArea r6, const sVec3_S16_12_4 *r7) {
    if (g_validationConnection != nullptr && isValidationContextEnabled(VCTX_Town)) {
        g_validationConnection->executeUntilAddress(kAddBackgroundScriptEntry);
        validateRegister(azelval::REG_R0 + 4, (u32)r4.m_offset); // script start EA
        validateRegister(azelval::REG_R0 + 5, (u32)r5);          // script type
    }
    addBackgroundScript_intercept.callUndetoured(r4, r5, r6, r7);
}

DECLARE_HOOK(runScript, kRunScriptEntry, sSaturnPtr, sNpcData *)

sSaturnPtr runScript_detour(sNpcData *r4) {
    if (g_validationConnection == nullptr || !isValidationContextEnabled(VCTX_Town)) {
        return runScript_intercept.callUndetoured(r4);
    }
    g_validationConnection->executeUntilAddress(kRunScriptEntry);
    const u32 entryIP = (u32)r4->m104_currentScript.m0_scriptPtr.m_offset;
    validate(kNpcData0 + 0x104, (s32)entryIP);
    const u32 returnAddr = g_validationConnection->getRegister(azelval::REG_PR);

    const sSaturnPtr result = runScript_intercept.callUndetoured(r4);

    g_validationConnection->executeUntilAddress(returnAddr);
    const u32 emuExit = g_validationConnection->getRegister(azelval::REG_R0 + 0);
    if ((u32)result.m_offset != emuExit) {
        std::printf("[validation] runScript diverged: entryIP=%08X  PDS_exit=%08X  emu_exit=%08X\n", entryIP,
                    (u32)result.m_offset, emuExit);
    }
    validateRegister(azelval::REG_R0 + 0, (u32)result.m_offset); // advanced IP
    return result;
}

s32 updateWorldGridFromEdgeTask();
s32 updateWorldGridFromEdgeTask_detour();
interceptor<s32> updateWorldGridFromEdgeTask_intercept(updateWorldGridFromEdgeTask, updateWorldGridFromEdgeTask_detour, 0);

s32 updateWorldGridFromEdgeTask_detour() {
    const s32 pdsResult = updateWorldGridFromEdgeTask_intercept.callUndetoured();
    if (g_validationConnection != nullptr && isValidationContextEnabled(VCTX_Town)) {
        return (s32)(g_validationConnection->readU32(kFileInfoStruct_allocatedHead) == 0); // guest's load gate
    }
    return pdsResult;
}

static void syncFadeChannelFromGuest(u32 guestBase, sFadeControlsChannel &dst) {
    MailboxConnection *c = g_validationConnection;
    dst.m0_color.m0_X = fixedPoint((s32)c->readU32(guestBase + 0x00));
    dst.m0_color.m4_Y = fixedPoint((s32)c->readU32(guestBase + 0x04));
    dst.m0_color.m8_Z = fixedPoint((s32)c->readU32(guestBase + 0x08));
    dst.mC_colorStep.m0_X = fixedPoint((s32)c->readU32(guestBase + 0x0C));
    dst.mC_colorStep.m4_Y = fixedPoint((s32)c->readU32(guestBase + 0x10));
    dst.mC_colorStep.m8_Z = fixedPoint((s32)c->readU32(guestBase + 0x14));
    dst.m18_targetColor[0] = fp16(c->readS16(guestBase + 0x18));
    dst.m18_targetColor[1] = fp16(c->readS16(guestBase + 0x1A));
    dst.m18_targetColor[2] = fp16(c->readS16(guestBase + 0x1C));
    dst.m1E_counter = c->readS16(guestBase + 0x1E);
    dst.m20_stopped = c->readU8(guestBase + 0x20);
}

static void syncFadeControlsFromGuest() {
    syncFadeChannelFromGuest(kGFadeControls + 0x00, g_fadeControls.m0_fade0);
    syncFadeChannelFromGuest(kGFadeControls + 0x24, g_fadeControls.m24_fade1);
    g_fadeControls.m_48 = g_validationConnection->readU16(kGFadeControls + 0x48);
    g_fadeControls.m_4A = g_validationConnection->readU16(kGFadeControls + 0x4A);
    g_fadeControls.m_4C = g_validationConnection->readU8(kGFadeControls + 0x4C);
    g_fadeControls.m_4D = g_validationConnection->readU8(kGFadeControls + 0x4D);
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

constexpr u32 kSphereOverlapTestEntry = 0x060086d4;

static void validateCollisionBodyIdentity(u32 emuBody, const sCollisionBody *body) {
    validate(emuBody + 0x0, (s8)body->m0_collisionSetup.m0_collisionType);
    validate(emuBody + 0x2, (s16)body->m0_collisionSetup.m2_collisionLayersBitField);
    validate(emuBody + 0x4, body->m4_sphereRadius);
    validate(emuBody + 0x8, body->m8_position);
    validate(emuBody + 0x14, body->m14_halfAABB);
}

DECLARE_HOOK(sphereOverlapTest, kSphereOverlapTestEntry, s32, sCollisionBody *, sCollisionBody *)

s32 sphereOverlapTest_detour(sCollisionBody *r13, sCollisionBody *r14) {
    if (g_validationConnection != nullptr && isValidationContextEnabled(VCTX_Town)) {
        g_validationConnection->executeUntilAddress(kSphereOverlapTestEntry);
        const u32 emu13 = g_validationConnection->getRegister(azelval::REG_R0 + 4); // R4 = r13
        const u32 emu14 = g_validationConnection->getRegister(azelval::REG_R0 + 5); // R5 = r14
        validateCollisionBodyIdentity(emu13, r13);
        validateCollisionBodyIdentity(emu14, r14);
        // r14->m40 (wall mesh ptr) selects the wall vs sphere path (and the hooked-call count).
        const u32 emuM40 = g_validationConnection->readU32(emu14 + 0x40);
        validateAssert((emuM40 != 0) == (r14->m40 != nullptr));
    }
    return sphereOverlapTest_intercept.callUndetoured(r13, r14);
}

// Runs right after setupDataForLCSCollision, so this captures both its inputs and its outputs
constexpr u32 kFindLCSCollisionEntry = 0x06013e5a;
constexpr u32 kLCSCollisionData = 0x06052668;

DECLARE_HOOK_VOID(findLCSCollision, kFindLCSCollisionEntry, void)

void findLCSCollision_detour() {
    if (g_validationConnection != nullptr && isValidationContextEnabled(VCTX_Town)) {
        g_validationConnection->executeUntilAddress(kFindLCSCollisionEntry);
        // Cell selection: findLCSCollision walks gTownGrid.m40_cellTasks indexed by m8/mC
        validate(kGTownGrid + 0x8, (s32)gTownGrid.m8);
        validate(kGTownGrid + 0xC, (s32)gTownGrid.mC);
        validate(kGTownGrid + 0x10, (s32)gTownGrid.m10_currentX);
        validate(kGTownGrid + 0x14, (s32)gTownGrid.m14_currentY);
        const u32 lcs = kLCSCollisionData;
        // setupDataForLCSCollision inputs (cursor target + per-frame camera constants)
        validate(lcs + 0x0, LCSCollisionData.m0_LCS_X);
        validate(lcs + 0x4, LCSCollisionData.m4_LCS_Y);
        validate(lcs + 0x8, (s32)LCSCollisionData.m8_LCSWidth);
        validate(lcs + 0xC, LCSCollisionData.mC_LCSHeight);
        validate(lcs + 0x18, LCSCollisionData.m18);
        validate(lcs + 0x24, LCSCollisionData.m24_LCSDepthMin);
        validate(lcs + 0x2C, LCSCollisionData.m2C_projectionWidthScale);
        validate(lcs + 0x30, LCSCollisionData.m30_projectionHeightScale);
        // setupDataForLCSCollision outputs
        validate(lcs + 0x10, (s32)LCSCollisionData.m10_activeLCSType);
        validate(lcs + 0x1C, (s16)LCSCollisionData.m1C_LCSHeightMin);
        validate(lcs + 0x1E, (s16)LCSCollisionData.m1E_LCSHeightMax);
        validate(lcs + 0x20, (s16)LCSCollisionData.m20_LCSWidthMin);
        validate(lcs + 0x22, (s16)LCSCollisionData.m22_LCSWidthMax);
        validate(lcs + 0x28, LCSCollisionData.m28_LCSDepthMax);
        validate(lcs + 0x34, LCSCollisionData.m34_boundMinX);
        validate(lcs + 0x38, LCSCollisionData.m38_radiusScaleMinX);
        validate(lcs + 0x3C, LCSCollisionData.m3C_boundMaxX);
        validate(lcs + 0x40, LCSCollisionData.m40_radiusScaleMaxX);
        validate(lcs + 0x44, LCSCollisionData.m44_boundMinY);
        validate(lcs + 0x48, LCSCollisionData.m48_radiusScaleMinY);
        validate(lcs + 0x4C, LCSCollisionData.m4C_boundMaxY);
        validate(lcs + 0x50, LCSCollisionData.m50_radiusScaleMaxY);
        validate(lcs + 0x54, LCSCollisionData.m54);
        validate(lcs + 0x58, LCSCollisionData.m58);
    }
    findLCSCollision_intercept.callUndetoured();
}

constexpr u32 kTestMeshVisibilityEntry = 0x060140ce;

DECLARE_HOOK(testMeshVisibility, kTestMeshVisibilityEntry, s32, sResCameraProperties *, const sProcessed3dModel &)

s32 testMeshVisibility_detour(sResCameraProperties *r14, const sProcessed3dModel &r11) {
    if (g_validationConnection == nullptr || !isValidationContextEnabled(VCTX_Town)) {
        return testMeshVisibility_intercept.callUndetoured(r14, r11);
    }
    g_validationConnection->executeUntilAddress(kTestMeshVisibilityEntry);
    const u32 matrixAddr = g_validationConnection->readU32(kPCurrentMatrixPtr);
    validate(matrixAddr, *pCurrentMatrix);
    const u32 emuModel = g_validationConnection->getRegister(azelval::REG_R0 + 5); // R5 = r11 (sProcessed3dModel*)
    validate(emuModel + 0x0, r11.m0_radius);
    // r14 (R4) depth + LCS-box bounds: feed the clip flags and Sub0 edge math.
    const u32 emuR14 = g_validationConnection->getRegister(azelval::REG_R0 + 4); // R4 = r14
    validate(emuR14 + 0x24, r14->m24_LCSDepthMin);
    validate(emuR14 + 0x28, r14->m28_LCSDepthMax);
    validate(emuR14 + 0x1C, r14->m1C_LCSHeightMin);
    validate(emuR14 + 0x1E, r14->m1E_LCSHeightMax);
    validate(emuR14 + 0x20, r14->m20_LCSWidthMin);
    validate(emuR14 + 0x22, r14->m22_LCSWidthMax);
    // numVertices (+4) and the inline quad table (+0xC); CMDCOLR/CMDSRCA are per-side texture ptrs, so skipped
    validate(emuModel + 0x4, r11.m4_numVertices);
    u32 quadAddr = emuModel + 0xC;
    for (const auto &quad : r11.mC_Quads) {
        validate(quadAddr + 0x0, quad.m0_indices[0]);
        validate(quadAddr + 0x2, quad.m0_indices[1]);
        validate(quadAddr + 0x4, quad.m0_indices[2]);
        validate(quadAddr + 0x6, quad.m0_indices[3]);
        validate(quadAddr + 0x8, quad.m8_lightingControl);
        validate(quadAddr + 0xA, quad.mA_CMDCTRL);
        validate(quadAddr + 0xC, quad.mC_CMDPMOD);
        validate(quadAddr + 0x12, quad.m12_onCollisionScriptIndex);
        quadAddr += 0x14; // 8 (indices) + 12 (six control words)
        switch ((quad.m8_lightingControl >> 8) & 3) {
        case 1: quadAddr += 8; break;  // 1 normal (6) + 2 pad
        case 2: quadAddr += 48; break; // 4 x (normal 6 + color 6)
        case 3: quadAddr += 24; break; // 4 x normal 6
        }
    }
    const u32 returnAddr = g_validationConnection->getRegister(azelval::REG_PR);

    const s32 result = testMeshVisibility_intercept.callUndetoured(r14, r11);

    g_validationConnection->executeUntilAddress(returnAddr);
    validateRegister(azelval::REG_R0 + 0, (u32)result);
    return result;
}

constexpr u32 kTransformVerticesClippedEntry = 0x0601f320;

DECLARE_HOOK(transformVerticesClipped, kTransformVerticesClippedEntry, void, const sProcessed3dModel &,
             const sMatrix4x3 &, sResCameraProperties *, std::array<sTransformedVertice, 256> &)

void transformVerticesClipped_detour(const sProcessed3dModel &r4, const sMatrix4x3 &r5, sResCameraProperties *r6,
                                     std::array<sTransformedVertice, 256> &r7) {
    if (g_validationConnection == nullptr || !isValidationContextEnabled(VCTX_Town)) {
        transformVerticesClipped_intercept.callUndetoured(r4, r5, r6, r7);
        return;
    }
    g_validationConnection->executeUntilAddress(kTransformVerticesClippedEntry);
    const u32 emuArray = g_validationConnection->getRegister(azelval::REG_R0 + 7); // R7 = output array
    const u32 returnAddr = g_validationConnection->getRegister(azelval::REG_PR);
    const u32 numVertices = r4.m4_numVertices;

    transformVerticesClipped_intercept.callUndetoured(r4, r5, r6, r7);

    g_validationConnection->executeUntilAddress(returnAddr);
    for (u32 i = 0; i < numVertices; i++) {
        const u32 base = emuArray + i * 0x20;
        validate(base + 0x4, r7[i].m4_fullPrecisionZ);
        validate(base + 0x8, r7[i].m8_fullPrecisionX);
        validate(base + 0xC, r7[i].mC_fullPrecisionY);
        validate(base + 0x18, r7[i].m18_clipFlags);
    }
}


DECLARE_HOOK_VOID(scriptFunction_6057058_sub0Sub0, 0, void)

void scriptFunction_6057058_sub0Sub0_detour() {
    scriptFunction_6057058_sub0Sub0_intercept.callUndetoured();
    if (g_validationConnection == nullptr || !isValidationContextEnabled(VCTX_Town) ||
        gActiveTownValidationAddrs == nullptr || gActiveTownValidationAddrs->cameraSetupReturn == 0) {
        return;
    }
    g_validationConnection->executeUntilAddress(gActiveTownValidationAddrs->cameraSetupReturn);
    const u32 mainLogic = g_validationConnection->readU32(kTwnMainLogicTask);
    if (mainLogic != 0) {
        validate(mainLogic + kMainLogic_m5C_rawCameraPosition, twnMainLogicTask->m5C_rawCameraPosition);
        validate(mainLogic + kMainLogic_m38_interpolatedCameraPosition, twnMainLogicTask->m38_interpolatedCameraPosition);
        validate(mainLogic + kMainLogic_m50_upVector, twnMainLogicTask->m50_upVector);
        validate(mainLogic + kMainLogic_m44_cameraTarget, twnMainLogicTask->m44_cameraTarget);
    }
}

DECLARE_HOOK(updateEdgePositionSub1, 0, void, sEdgeTask *)

void updateEdgePositionSub1_detour(sEdgeTask *r4) {
    if (g_validationConnection == nullptr || !isValidationContextEnabled(VCTX_Town) ||
        gActiveTownValidationAddrs == nullptr || gActiveTownValidationAddrs->updateEdgePositionSub1Entry == 0) {
        updateEdgePositionSub1_intercept.callUndetoured(r4);
        return;
    }

    g_validationConnection->executeUntilAddress(gActiveTownValidationAddrs->updateEdgePositionSub1Entry);
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

    g_validationConnection->executeUntilAddress(gActiveTownValidationAddrs->updateEdgePositionSub1Return);
    validate(edge + kEdge_mE8 + kNpcE8_stepRotation, r4->mE8.m24_stepRotation);
    validate(edge + kEdge_mE8 + kNpcE8_stepTranslation, r4->mE8.m30_stepTranslation);
    validate(edge + kEdge_m14C_inputFlags, r4->m14C_inputFlags);
}

DECLARE_HOOK(updateEdgePosition, 0, void, sNPC *)

void updateEdgePosition_detour(sNPC *r4) {
    const bool active = g_validationConnection != nullptr && isValidationContextEnabled(VCTX_Town) &&
                        gActiveTownValidationAddrs != nullptr &&
                        gActiveTownValidationAddrs->updateEdgePositionEntry != 0 &&
                        twnMainLogicTask != nullptr && twnMainLogicTask->m14_EdgeTask != nullptr;
    if (active) {
        g_validationConnection->executeUntilAddress(gActiveTownValidationAddrs->updateEdgePositionEntry);
        const u32 mainLogic = g_validationConnection->readU32(kTwnMainLogicTask);
        const u32 edge = mainLogic ? g_validationConnection->readU32(mainLogic + kMainLogic_m14_EdgeTask) : 0;
        if (edge != 0) {
            validate(edge + kEdge_mE8 + kNpcE8_position, twnMainLogicTask->m14_EdgeTask->mE8.m0_position);
            // Branch selectors (m44, m4C) before the result (m58)
            const sCollisionBody &body = twnMainLogicTask->m14_EdgeTask->m84;
            validate(edge + kEdge_m84_m44_contactFlags, (s32)body.m44);
            validate(edge + kEdge_m84_m4C, body.m4C);
            validate(edge + kEdge_m84_m58_collisionSolveTranslation, body.m58_collisionSolveTranslation);
            // m30_stepTranslation is pre-gravity here
            validate(edge + 0x2C, twnMainLogicTask->m14_EdgeTask->m2C_currentAnimation);
            validate(edge + kEdge_mE8 + kNpcE8_stepTranslation, twnMainLogicTask->m14_EdgeTask->mE8.m30_stepTranslation);
            validate(edge + 0xE, (s8)twnMainLogicTask->m14_EdgeTask->mE_controlState);
        }
    }
    updateEdgePosition_intercept.callUndetoured(r4);
    if (!active)
        return;
    g_validationConnection->executeUntilAddress(gActiveTownValidationAddrs->updateEdgePositionReturn);
    const u32 mainLogic = g_validationConnection->readU32(kTwnMainLogicTask);
    const u32 edge = mainLogic ? g_validationConnection->readU32(mainLogic + kMainLogic_m14_EdgeTask) : 0;
    if (edge != 0) {
        validate(edge + kEdge_mE8 + kNpcE8_stepTranslationInWorld, twnMainLogicTask->m14_EdgeTask->mE8.m18_stepTranslationInWorld);
        validate(edge + kEdge_mE8 + kNpcE8_position, twnMainLogicTask->m14_EdgeTask->mE8.m0_position);
        // m30_stepTranslation is post-gravity here
        validate(edge + 0x2C, twnMainLogicTask->m14_EdgeTask->m2C_currentAnimation);
        validate(edge + kEdge_mE8 + kNpcE8_stepTranslation, twnMainLogicTask->m14_EdgeTask->mE8.m30_stepTranslation);
        validate(edge + 0xE, (s8)twnMainLogicTask->m14_EdgeTask->mE_controlState);
    }
}

DECLARE_HOOK(cameraUpdate_follow, 0, void, sMainLogic *)

void cameraUpdate_follow_detour(sMainLogic *r4) {
    if (g_validationConnection != nullptr && isValidationContextEnabled(VCTX_Town) &&
        gActiveTownValidationAddrs != nullptr && gActiveTownValidationAddrs->cameraUpdateFollowEntry != 0) {
        g_validationConnection->executeUntilAddress(gActiveTownValidationAddrs->cameraUpdateFollowEntry);
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

constexpr u32 kEdge_m20_lookAtAngle = 0x20; // sVec2_FP: [0] head pitch, [1] head yaw
constexpr u32 kNpcE8_targetRotation = 0x48;

DECLARE_HOOK(updateEdgeLookAt, 0, void, sEdgeTask *)

void updateEdgeLookAt_detour(sEdgeTask *r4) {
    if (g_validationConnection == nullptr || !isValidationContextEnabled(VCTX_Town) ||
        gActiveTownValidationAddrs == nullptr || gActiveTownValidationAddrs->updateEdgeLookAtEntry == 0) {
        updateEdgeLookAt_intercept.callUndetoured(r4);
        return;
    }
    g_validationConnection->executeUntilAddress(gActiveTownValidationAddrs->updateEdgeLookAtEntry);
    const u32 edge = g_validationConnection->getRegister(azelval::REG_R0 + 4); // R4 = sEdgeTask*
    validate(edge + kEdge_mE8 + kNpcE8_position, r4->mE8.m0_position);
    validate(edge + kEdge_mE8 + kNpcE8_rotation, r4->mE8.mC_rotation);
    validate(edge + kEdge_mE8 + kNpcE8_targetRotation, r4->mE8.m48_targetRotation);
    validate(edge + kEdge_mE8 + kNpcE8_stepRotation, r4->mE8.m24_stepRotation);
    validate(edge + kEdge_m20_lookAtAngle + 0x0, r4->m20_lookAtAngle[0]);
    validate(edge + kEdge_m20_lookAtAngle + 0x4, r4->m20_lookAtAngle[1]);

    updateEdgeLookAt_intercept.callUndetoured(r4);

    g_validationConnection->executeUntilAddress(gActiveTownValidationAddrs->updateEdgeLookAtReturn);
    validate(edge + kEdge_m20_lookAtAngle + 0x0, r4->m20_lookAtAngle[0]);
    validate(edge + kEdge_m20_lookAtAngle + 0x4, r4->m20_lookAtAngle[1]);
}

DECLARE_HOOK(zoahCamera_update, 0, void, sCameraTask*)

void zoahCamera_update_detour(sCameraTask* pThis) {
    if (g_validationConnection == nullptr || !isValidationContextEnabled(VCTX_Town) ||
        gActiveTownValidationAddrs == nullptr || gActiveTownValidationAddrs->zoahCameraUpdateEntry == 0) {
        zoahCamera_update_intercept.callUndetoured(pThis);
        return;
    }
    g_validationConnection->executeUntilAddress(gActiveTownValidationAddrs->zoahCameraUpdateEntry);
    gSatCameraTaskAddr = g_validationConnection->getRegister(azelval::REG_R0 + 4);

    // The script pointer first, so the underlying divergence asserts before the derived mFC bit
    validate(kNpcData0 + 0x104, (s32)npcData0.m104_currentScript.m0_scriptPtr.m_offset);
    validate(kNpcData0 + 0x0, (s32)npcData0.m0_numBackgroundScripts);
    validate(kNpcData0 + 0x100, (s32)npcData0.m100);

    // Gate for the timer increment: if ((npcData0.mFC & 1) == 0) ++m4_dayNightTimer
    validate(kNpcData0_mFC, (s32)npcData0.mFC);
    validate(gSatCameraTaskAddr + 0x4, (s32)pThis->m4_dayNightTimer);

    zoahCamera_update_intercept.callUndetoured(pThis);
}

// One-shot per town transition; the base hooks are armed once in enableTownHooks and never change
static bool refreshTownValidationAddresses() {
    static const sTownOverlay* sLastOverlay = nullptr;
    if (gCurrentTownOverlay == sLastOverlay) {
        return false;
    }
    sLastOverlay = gCurrentTownOverlay;
    gSatCameraTaskAddr = 0;

    gActiveTownValidationAddrs = nullptr;
    if (gCurrentTownOverlay != nullptr) {
        auto it = kTownValidationAddresses.find(gCurrentTownOverlay->m_name);
        if (it != kTownValidationAddresses.end()) {
            gActiveTownValidationAddrs = &it->second;
        }
    }

    const sTownValidationAddresses* a = gActiveTownValidationAddrs;
    scriptFunction_6057058_sub0Sub0_intercept.setSaturnBreakpoint(a ? a->cameraSetupReturn : 0);
    updateEdgePositionSub1_intercept.setSaturnBreakpoint(a ? a->updateEdgePositionSub1Entry : 0);
    updateEdgePosition_intercept.setSaturnBreakpoint(a ? a->updateEdgePositionEntry : 0);
    cameraUpdate_follow_intercept.setSaturnBreakpoint(a ? a->cameraUpdateFollowEntry : 0);
    updateEdgeLookAt_intercept.setSaturnBreakpoint(a ? a->updateEdgeLookAtEntry : 0);
    zoahCamera_update_intercept.setSaturnBreakpoint(a ? a->zoahCameraUpdateEntry : 0);
    return true;
}

void enableTownHooks() {
    isDataLoaded_intercept.enable();
    resetCollisionFrame_intercept.enable();
    // These drives pass through other hooked natives; each has its own hook to advance the guest past it
    addBackgroundScript_intercept.enable();
    runScript_intercept.enable();
    updateWorldGridFromEdgeTask_intercept.enable();
    updateEdgePosition_intercept.enable();
    cameraUpdate_follow_intercept.enable();
    getCellAtWorldPos_intercept.enable();
    processTownMeshCollision_intercept.enable();
    handleCollisionWithTownEnv_intercept.enable();
    computeCollisionSeparation_intercept.enable();
    sphereOverlapTest_intercept.enable();
    findLCSCollision_intercept.enable();
    testMeshVisibility_intercept.enable();
    // Not armed: shared with the object renderer, which PDS draws via bgfx
    // transformVerticesClipped_intercept.enable();
    scriptFunction_6057058_sub0Sub0_intercept.enable();
    updateEdgePositionSub1_intercept.enable();
    updateEdgeLookAt_intercept.enable();
    zoahCamera_update_intercept.enable();
}
