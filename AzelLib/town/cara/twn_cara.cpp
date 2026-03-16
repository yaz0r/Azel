#include "PDS.h"
#include "twn_cara.h"
#include "town/town.h"
#include "town/townScript.h"
#include "town/townEdge.h"
#include "town/townMainLogic.h"
#include "kernel/vdp1Allocator.h"
#include "audio/soundDriver.h"
#include "audio/systemSounds.h"
#include "town/townCamera.h"
#include "town/excaEntity.h"
#include "town/townCutscene.h"
#include "battle/BTL_A3/BTL_A3_map6.h"
#include "field/field_a3/o_fld_a3.h"
#include "kernel/cinematicBarsTask.h"
#include "3dEngine.h"
#include "kernel/loadSavegameScreen.h"
#include "town/collisionRegistry.h"
#include "kernel/graphicalObject.h"
#include "kernel/fileBundle.h"
#include "kernel/animation.h"

void unloadFnt(); // TODO: fix

struct sCaraVdp2Plane;
static p_workArea gCaraVdp2PlaneTask;

// 06070c0c
static s32 updateWorldGridFromEdgeTask() {
    if (twnMainLogicTask->m14_EdgeTask) {
        updateWorldGrid(twnMainLogicTask->m14_EdgeTask->mE8.m0_position[0], twnMainLogicTask->m14_EdgeTask->mE8.m0_position[2]);
    }
    return fileInfoStruct.m2C_allocatedHead == 0;
}

// 060540f8
static s32 returnZero() {
    return 0;
}

static void cameraUpdate_noop(sMainLogic*)
{
}

static s32 caravanCameraSetup(s32 rotationEA, s32 lightData);
s32 setupCameraModeFixed(s32 arg0, s32 arg1, s32 arg2);

// 0602c308
static s32 playSoundEffect_bank5(s32 soundId, s32 volume, s32 unk) {
    enqueuePlaySoundEffect(soundId, 5, volume, unk);
    return 0;
}

// cutscene functions factored into townCutscene.cpp

// 0606ed3a
static void caravanMainLogicDraw(sMainLogic* pThis) {
    if (pThis->m14_EdgeTask) {
        sNPCE8& edge = pThis->m14_EdgeTask->mE8;
        updateWorldGrid(edge.m0_position.m0_X, edge.m0_position.m8_Z);
        pThis->m18_position.m0_X = edge.m0_position.m0_X;
        pThis->m18_position.m4_Y = edge.m0_position.m4_Y + 0x1800;
        pThis->m18_position.m8_Z = edge.m0_position.m8_Z;
    }
    Unimplemented(); // FUN_TWN_CARA__0606edae, 0606eec4, 0606ef84 — camera interpolation
    pThis->m10(pThis); // call camera update function pointer
    Unimplemented(); // copy interpolated position, FUN_TWN_CARA__06070c50
}

// 06070434
static void caravanMainLogicUpdate(sMainLogic* pThis) {
    sVec3_FP lookAt;
    lookAt.m0_X = pThis->m44_cameraTarget.m0_X * 0x10 + pThis->m38_interpolatedCameraPosition.m0_X * -0xF;
    lookAt.m4_Y = pThis->m44_cameraTarget.m4_Y * 0x10 + pThis->m38_interpolatedCameraPosition.m4_Y * -0xF;
    lookAt.m8_Z = pThis->m44_cameraTarget.m8_Z * 0x10 + pThis->m38_interpolatedCameraPosition.m8_Z * -0xF;

    generateCameraMatrix(&cameraProperties2, pThis->m38_interpolatedCameraPosition, lookAt, pThis->m50_upVector);
    drawLcs();

    if (enableDebugTask) {
        Unimplemented(); // debug info display
    }
}

// 060709fa
static s32 setupCameraFollowMode_Cara() {
    twnMainLogicTask->m_DrawMethod = &caravanMainLogicDraw;
    twnMainLogicTask->m_UpdateMethod = &caravanMainLogicUpdate;
    return 0;
}

// 060749ea
static s32 enableNpcLookAtDecay_Cara(s32 npcIndex) {
    sNPC* pNPC = getNpcDataByIndex(npcIndex);
    pNPC->mC &= ~8;
    return 0;
}

// 06070bfc
static s32 resetFieldOfView() {
    initVDP1Projection(0x1C71C71, 0);
    return 0;
}

// 06072440
static s32 createCaraSubTask(s32 param_1) {
    Unimplemented(); // creates sub-task on townVar0 with definition 0x06087930
    return 0;
}

// 060706f4
static s32 setupNpcWalkForward() {
    setupCameraUpdateForCurrentMode();
    sNPC* pNPC = getNpcDataByIndex(0);
    // Set step translation to {0, 0, -0xE3} (forward in local space)
    pNPC->mE8.m18_stepTranslationInWorld = { 0, 0, fixedPoint(0xFFFFFF1D) };

    // Transform step by NPC rotation to get world-space direction
    sMatrix4x3 mat;
    initMatrixToIdentity(&mat);
    rotateMatrixShiftedY(pNPC->mE8.mC_rotation.m4_Y, &mat);
    rotateMatrixShiftedX(pNPC->mE8.mC_rotation.m0_X, &mat);
    sVec3_FP transformed;
    transformVec(pNPC->mE8.m18_stepTranslationInWorld, transformed, mat);

    // Compute target position = current position + transformed * 0x24
    pNPC->mE8.m3C_targetPosition.m0_X = transformed.m0_X * 0x24 + pNPC->mE8.m0_position.m0_X;
    pNPC->mE8.m3C_targetPosition.m4_Y = transformed.m4_Y * 0x24 + pNPC->mE8.m0_position.m4_Y;
    pNPC->mE8.m3C_targetPosition.m8_Z = transformed.m8_Z * 0x24 + pNPC->mE8.m0_position.m8_Z;

    // Copy current rotation to target rotation
    pNPC->mE8.m48_targetRotation = pNPC->mE8.mC_rotation;

    // Set movement flags
    pNPC->mF = (pNPC->mF & 0xF9) | 1;
    pNPC->mC |= 4;
    return 0;
}

// 0606e8a8
static s32 enableBackgroundWithTimer(s32 duration) {
    vdp2Controls.m4_pendingVdp2Regs->m20_BGON |= 1;
    vdp2Controls.m_isDirty = 1;
    if (gCaraVdp2PlaneTask) {
        // m40_timer and m44 on sCaraVdp2Plane
        *(s32*)((u8*)gCaraVdp2PlaneTask + 0x40) = duration;
        *(s32*)((u8*)gCaraVdp2PlaneTask + 0x44) = 1;
    }
    vdp2Controls.m4_pendingVdp2Regs->m20_BGON |= 1;
    vdp2Controls.m_isDirty = 1;
    return 0;
}

// 060710c6
static s32 createCutsceneWaitTask(s32 param_1) {
    if (townIsCutsceneDone() == 0) {
        Unimplemented(); // creates a timer sub-task on townVar0
    }
    return 0;
}

// 06070a0a
static s32 disableCameraUpdate_Cara() {
    twnMainLogicTask->m10 = &cameraUpdate_noop;
    return 0;
}

// 06075bf8
static s32 caravanCameraSetup(s32 rotationEA, s32 lightData) {
    sSaturnPtr rotPtr = gCurrentTownOverlay->getSaturnPtr(rotationEA);
    sSaturnPtr lightPtr = gCurrentTownOverlay->getSaturnPtr(lightData);

    cameraTaskPtr->m8_colorData = lightPtr;

    sMatrix4x3 mat;
    initMatrixToIdentity(&mat);
    rotateMatrixShiftedY(readSaturnFP(rotPtr + 4), &mat);
    rotateMatrixShiftedX(readSaturnFP(rotPtr), &mat);
    cameraTaskPtr->m14.m0_X = mat.m[0][2];
    cameraTaskPtr->m14.m4_Y = mat.m[1][2];
    cameraTaskPtr->m14.m8_Z = mat.m[2][2];

    cameraTaskPtr->m10.m0 = readSaturnU8(lightPtr);
    cameraTaskPtr->m10.m1 = readSaturnU8(lightPtr + 1);
    cameraTaskPtr->m10.m2 = readSaturnU8(lightPtr + 2);
    cameraTaskPtr->m30_colorIntensity = 0x8000;

    u32 f0 = (u32)readSaturnU8(lightPtr + 5) << 16 | (u32)readSaturnU8(lightPtr + 4) << 8 | (u32)readSaturnU8(lightPtr + 3);
    u32 f1 = (u32)readSaturnU8(lightPtr + 8) << 16 | (u32)readSaturnU8(lightPtr + 7) << 8 | (u32)readSaturnU8(lightPtr + 6);
    u32 f2 = (u32)readSaturnU8(lightPtr + 11) << 16 | (u32)readSaturnU8(lightPtr + 10) << 8 | (u32)readSaturnU8(lightPtr + 9);
    generateLightFalloffMap(f0, f1, f2);

    cameraTaskPtr->m_UpdateMethod = townCamera_update;
    cameraTaskPtr->m_DrawMethod = townCamera_draw;

    if (g_fadeControls.m_4C <= g_fadeControls.m_4D) {
        vdp2Controls.m20_registers[0].m112_CLOFSL = 0x10;
        vdp2Controls.m20_registers[1].m112_CLOFSL = 0x10;
    }
    resetProjectVector();
    cameraTaskPtr->m2 = 0;
    cameraTaskPtr->m0_colorMode = 0;
    return 0;
}

// 06070bd0
static s32 setFieldOfView(s32 fov) {
    initVDP1Projection(fov, 0);
    return 0;
}

// 060749d6
static s32 disableNpcLookAtDecay_Cara(s32 npcIndex) {
    sNPC* pNPC = getNpcDataByIndex(npcIndex);
    pNPC->mC |= 8;
    return 0;
}

// 06073f5c
static s32 setNpcDrawMode(s32 npcIndex, s32 mode) {
    Unimplemented(); // Sets overlay-specific draw method on NPC based on mode (0-3)
    return 0;
}

// 06071484
static s32 createCutscene_Cara(s32 arg) {
    npcData0.mF4 = 1;
    npcData0.mF0 = 0;
    gCutsceneTask = createSubTaskWithArg<sCutsceneTask>(twnMainLogicTask, arg);
    return 0;
}

// 0606e6d4
static s32 setupCaravanBackground(s32 param_1) {
    static const char* scbFiles[] = { "CARASCR0.SCB", "CARASCR1.SCB", "CARASCR2.SCB", "CARASCR3.SCB", "CARASCR4.SCB", "CARASCR5.SCB" };
    static const char* pnbFiles[] = { "CARASCR0.PNB", "CARASCR1.PNB", "CARASCR2.PNB", "CARASCR3.PNB", "CARASCR4.PNB", "CARASCR5.PNB" };
    // Palette data addresses per area (0x200 bytes each, two sets: CRAM 0xA00 and vdp2Palette)
    static const u32 paletteOffsets[] = { 0x060849b4, 0x06084db4, 0x060851b4, 0x060855b4, 0x060859b4, 0x06085db4 };
    static const u32 palette2Offsets[] = { 0x06084bb4, 0x06084fb4, 0x060853b4, 0x060857b4, 0x06085bb4, 0x06085fb4 };

    if (param_1 < 0 || param_1 > 5) return param_1;

    if (param_1 == 4) {
        setupRotationMapPlanes(0, gCurrentTownOverlay->getSaturnPtr(0x06086324));
        setupRotationMapPlanes(1, gCurrentTownOverlay->getSaturnPtr(0x06086364));
    }
    else {
        setupRotationMapPlanes(0, gCurrentTownOverlay->getSaturnPtr(0x060862a4));
        setupRotationMapPlanes(1, gCurrentTownOverlay->getSaturnPtr(0x060862e4));
    }

    asyncDmaCopy(gCurrentTownOverlay->getSaturnPtr(paletteOffsets[param_1]), getVdp2Cram(0xA00), 0x200, 0);
    asyncDmaCopy(gCurrentTownOverlay->getSaturnPtr(palette2Offsets[param_1]), vdp2Palette, 0x200, 0);

    loadFile(scbFiles[param_1], getVdp2Vram(0x40000), 0);
    return loadFile(pnbFiles[param_1], getVdp2Vram(0x60000), 0);
}

// 060760da — same as zoahCamera_update
static void caravanCamera_update(sCameraTask* pThis)
{
    townCamera_update(pThis);

    sSaturnPtr colorSet0, colorSet1;
    u32 baseAddr = pThis->mC_colorTableBase;
    if (!mainGameState.getBit(8)) {
        colorSet0 = sSaturnPtr::createFromRaw(baseAddr, gCurrentTownOverlay);
        colorSet1 = sSaturnPtr::createFromRaw(baseAddr + 0x10, gCurrentTownOverlay);
    } else {
        colorSet0 = sSaturnPtr::createFromRaw(baseAddr + 0x30, gCurrentTownOverlay);
        colorSet1 = sSaturnPtr::createFromRaw(baseAddr + 0x40, gCurrentTownOverlay);
    }

    s32 time = pThis->m4_dayNightTimer;
    if (time > 0xe10) time = 0xe10;

    s32 fadeTime = 0x708;
    if (time > 0x707) {
        colorSet0 = colorSet0 + 0x10;
        colorSet1 = colorSet1 + 0x10;
        time = time - 0x708;
        fadeTime = 0xe10;
    }

    if (pThis->m1_fadeActive != 0 && g_fadeControls.m24_fade1.m20_stopped != 0 && (0x708 - time) > 0) {
        u16 fadeColor = computeTimeOfDayColor(fadeTime);
        s32 curColor = convertColorToU32ForFade(g_fadeControls.m24_fade1.m0_color);
        fadePalette(&g_fadeControls.m24_fade1, curColor, fadeColor, 0x708 - time);
    }

    fixedPoint t = FP_Div(time, 0x708);
    s32 invT = 0x10000 - (s32)t;

    for (int i = 0; i < 12; i++) {
        s8 c_target = readSaturnS8(colorSet1 + 4 + i);
        s8 c_source = readSaturnS8(colorSet0 + 4 + i);
        pThis->m34_interpolatedLightData[i] = fixedPoint((s32)c_target * (s32)t + (s32)c_source * invT);
    }

    s16 rotX_target = readSaturnS16(colorSet1);
    s16 rotX_source = readSaturnS16(colorSet0);
    s16 rotY_target = readSaturnS16(colorSet1 + 2);
    s16 rotY_source = readSaturnS16(colorSet0 + 2);

    sMatrix4x3 mat;
    initMatrixToIdentity(&mat);
    rotateMatrixShiftedY(fixedPoint((s32)rotY_target * (s32)t + (s32)rotY_source * invT), &mat);
    rotateMatrixShiftedX(fixedPoint((s32)rotX_target * (s32)t + (s32)rotX_source * invT), &mat);

    pThis->m14[0] = mat.m[0][3];
    pThis->m14[1] = mat.m[1][3];
    pThis->m14[2] = mat.m[2][3];

    fixedPoint* d = pThis->m34_interpolatedLightData;
    pThis->m10.m0 = (s8)(((u32)((s32)d[0] + 0x8000) >> 16) & 0xFF);
    pThis->m10.m1 = (s8)(((u32)((s32)d[1] + 0x8000) >> 16) & 0xFF);
    pThis->m10.m2 = (s8)(((u32)((s32)d[2] + 0x8000) >> 16) & 0xFF);
}

// 06076284 — same as zoahCamera_draw
static void caravanCamera_draw(sCameraTask* pThis)
{
    townCamera_draw(pThis);

    fixedPoint* d = pThis->m34_interpolatedLightData;
    auto fpToU8 = [](fixedPoint fp) -> u32 { return ((u32)((s32)fp + 0x8000) >> 16) & 0xFF; };
    u32 f0 = fpToU8(d[3]) | (fpToU8(d[4]) << 8) | (fpToU8(d[5]) << 16);
    u32 f1 = fpToU8(d[6]) | (fpToU8(d[7]) << 8) | (fpToU8(d[8]) << 16);
    u32 f2 = fpToU8(d[9]) | (fpToU8(d[10]) << 8) | (fpToU8(d[11]) << 16);
    generateLightFalloffMap(f0, f1, f2);
}

// 06075cda
static s32 setupCaravanCamera(s32 param_1) {
    cameraTaskPtr->mC_colorTableBase = param_1;
    cameraTaskPtr->m30_colorIntensity = 0x8000;
    cameraTaskPtr->m_UpdateMethod = caravanCamera_update;
    cameraTaskPtr->m_DrawMethod = caravanCamera_draw;
    if (g_fadeControls.m_4C <= g_fadeControls.m_4D) {
        vdp2Controls.m20_registers[0].m112_CLOFSL = 0x10;
        vdp2Controls.m20_registers[1].m112_CLOFSL = 0x10;
    }
    resetProjectVector();
    cameraTaskPtr->m2 = 0;
    cameraTaskPtr->m0_colorMode = 1;
    return 0;
}

static const char* listOfFilesToLoad[] = {
    "COMMON3.MCB",
    "COMMON3.CGB",
    "CARAVMP.MCB",
    "CARAVMP.CGB",
    "TENTSMMP.MCB",
    "TENTSMMP.CGB",
    "Z_A_KH.MCB",
    "Z_A_KH.CGB",
    "X_F_LR.MCB",
    "X_F_LR.CGB",
    "X_A_BK.MCB",
    "X_A_BK.CGB",
    "Z_A_AY.MCB",
    "Z_A_AY.CGB",
    "Z_F_AY.MCB",
    "Z_F_AY.CGB",
    "Z_A_FE.MCB",
    "Z_A_FE.CGB",
    "Z_F_FE.MCB",
    "Z_F_FE.CGB",
    "Z_A_EK.MCB",
    "Z_A_EK.CGB",
    "X_A_CR.MCB",
    "X_A_CR.CGB",
    "X_A_TB.MCB",
    "X_A_TB.CGB",
    nullptr
};

// 06028986
static void setVdp2ScrollPosition(s32 scrollX, s32 scrollY) {
    s_VDP2Regs* regs = vdp2Controls.m4_pendingVdp2Regs;
    if (pauseEngine[4] == 0) {
        regs->m70_SCXN0 = scrollX;
        regs->m74_SCYN0 = scrollY;
    }
    else if (pauseEngine[4] == 1) {
        regs->m80_SCXN1 = scrollX;
        regs->m84_SCYN1 = scrollY;
    }
}

// 0606dd7c
static s32 computeGroundY() {
    auto& coeff = gCoefficientTables[gRotationPassState.m0_planeIndex][vdp2Controls.m0_doubleBufferIndex];
    s32 iVar2 = gVdp2RotationMatrix.m[2][0].asS32() * coeff.m34;
    s32 iVar3 = gVdp2RotationMatrix.m[2][1].asS32() * coeff.m36;
    s32 iVar1 = MTH_Mul(gVdp2RotationMatrix.m[2][2],
                        fixedPoint(coeff.m38 * 0x10000 - coeff.m8_Zst));
    return FP_Div(iVar2 + iVar3 + iVar1, gVdp2RotationMatrix.m[2][1]).getInteger();
}

// Caravan VDP2 background plane task
struct sCaraVdp2Plane : public s_workAreaTemplate<sCaraVdp2Plane>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = {
            &sCaraVdp2Plane::Init,
            &sCaraVdp2Plane::Update,
            &sCaraVdp2Plane::Draw,
            nullptr
        };
        return &taskDefinition;
    }

    // 0606e01c
    static void Init(sCaraVdp2Plane* pThis)
    {
        reinitVdp2();
        initNBG1Layer();

        asyncDmaCopy(gCurrentTownOverlay->getSaturnPtr(0x060861b4), getVdp2Cram(0x200), 0x20, 0);

        static const sLayerConfig nbg0Setup[] = {
            {m1_TPEN, 1}, {m2_CHCN, 0}, {m5_CHSZ, 1}, {m6_PNB, 1}, {m7_CNSM, 0},
            {m34_W0E, 1}, {m37_W0A, 1}, {m44_CCEN, 1}, {m45_COEN, 0x10}, {m10_SPN, 4},
            {m0_END},
        };
        setupNBG0(nbg0Setup);

        static const sLayerConfig rgb0Setup[] = {
            {m1_TPEN, 1}, {m2_CHCN, 1}, {m5_CHSZ, 1}, {m6_PNB, 1}, {m7_CNSM, 0},
            {m27_RPMD, 2}, {m11_SCN, 8}, {m34_W0E, 1}, {m37_W0A, 1}, {m44_CCEN, 1},
            {m0_END},
        };
        setupRGB0(rgb0Setup);

        static const sLayerConfig rotParamsSetup[] = {
            {m31_RxKTE, 1},
            {m0_END},
        };
        setupRotationParams(rotParamsSetup);

        static const sLayerConfig rotParams2Setup[] = {
            {m0_END},
        };
        setupRotationParams2(rotParams2Setup);

        loadFile("EVT024N.SCB", getVdp2Vram(0x10000), 0);
        loadFile("EVT024N.PNB", getVdp2Vram(0x1F000), 0);

        vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL = (vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL & 0xFF00) | 0xB4;
        vdp2Controls.m4_pendingVdp2Regs->m10_CYCA0 = 0x310F7544;
        vdp2Controls.m_isDirty = 1;

        initLayerMap(0, 0x1F000, 0x1F000, 0x1F000, 0x1F000);
        setupRotationMapPlanes(0, gCurrentTownOverlay->getSaturnPtr(0x060862a4));
        setupRotationMapPlanes(1, gCurrentTownOverlay->getSaturnPtr(0x060862e4));

        setupVdp2Table(6, coefficientA0, coefficientA1, getVdp2Vram(0x20000), 0x80);
        setupVdp2Table(7, coefficientB0, coefficientB1, getVdp2Vram(0x22000), 0x80);
        s_BTL_A3_Env_InitVdp2Sub3(5, getVdp2Vram(0x2A000));
        *(u16*)getVdp2Vram(0x2A400) = 0x700;

        vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA = (vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA & 0xFFF80000) | 0x15200;
        *(u16*)getVdp2Vram(0x2A600) = 0;
        vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA = (vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA & 0xFFF80000) | 0x15300;

        vdp2Controls.m4_pendingVdp2Regs->mF0_PRISA = 0x204;
        vdp2Controls.m4_pendingVdp2Regs->mF2_PRISB = 0x407;
        vdp2Controls.m4_pendingVdp2Regs->mF4_PRISC = 0x404;
        vdp2Controls.m4_pendingVdp2Regs->mF6_PRISD = 0x404;
        vdp2Controls.m4_pendingVdp2Regs->mF8_PRINA = 0x605;
        vdp2Controls.m4_pendingVdp2Regs->mFA_PRINB = 0x700;
        vdp2Controls.m4_pendingVdp2Regs->mFC_PRIR = 3;
        vdp2Controls.m_isDirty = 1;

        pThis->m3C_scale = 0x100000;
        static const std::vector<std::array<s32, 2>> bgConfig = { {{0x2C, 1}} };
        applyLayerDisplayConfig(bgConfig);

        vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL = (vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL & 0xFEFF) | 0x100;
        vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xF8FF) | 0x200;
        vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xFFF0) | 3;
        vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xCFFF) | 0x1000;

        pThis->m40_timer = 0;
        vdp2Controls.m4_pendingVdp2Regs->m20_BGON = vdp2Controls.m4_pendingVdp2Regs->m20_BGON & ~1;
        vdp2Controls.m_isDirty = 1;
    }

    // 0606e2a8
    static void Update(sCaraVdp2Plane* pThis)
    {
        if (pThis->m40_timer > 0) {
            Unimplemented(); // timer-based fade logic
            pThis->m40_timer--;
            if (pThis->m40_timer < 1) {
                vdp2Controls.m4_pendingVdp2Regs->m20_BGON = vdp2Controls.m4_pendingVdp2Regs->m20_BGON & ~1;
                vdp2Controls.m_isDirty = 1;
            }
        }
        pThis->m38 = 0;
    }

    // 0606e52e
    static void DrawSub0(sCaraVdp2Plane* pThis) {
        s32 rotX = pThis->m18_cameraRotation.m0_X.asS32();
        s32 rotY = pThis->m18_cameraRotation.m4_Y.asS32();
        s32 rotZ = pThis->m18_cameraRotation.m8_Z.asS32();

        auto& coeff = gCoefficientTables[gRotationPassState.m0_planeIndex][vdp2Controls.m0_doubleBufferIndex];

        coeff.m34 = (s16)((pThis->m24_vdp1Clipping[0] + pThis->m24_vdp1Clipping[2] + (pThis->m24_vdp1Clipping[0] + pThis->m24_vdp1Clipping[2] < 0 ? 1 : 0)) >> 1);
        coeff.m36 = (s16)((pThis->m24_vdp1Clipping[1] + pThis->m24_vdp1Clipping[3] + (pThis->m24_vdp1Clipping[1] + pThis->m24_vdp1Clipping[3] < 0 ? 1 : 0)) >> 1);
        coeff.m38 = pThis->m32_projParam1;
        coeff.m3C = coeff.m34;
        coeff.m3E = coeff.m36;
        coeff.m40 = 0;

        buildRotationMatrixPitchYaw(-0x4000000 - rotX, -rotY);
        scaleRotationMatrix(pThis->m3C_scale >> 2);
        writeRotationParams(-rotZ);

        s32 dx = coeff.m34 - coeff.m3C;
        s32 dy = coeff.m36 - coeff.m3E;
        s32 dz = coeff.m38 - coeff.m40;

        s32 scaledX = MTH_Mul(pThis->m3C_scale, pThis->mC_cameraPosition.m0_X.asS32() << 4);
        gVdp2RotationMatrix.Mx = ((scaledX - gVdp2RotationMatrix.m[0][0].asS32() * dx) -
            gVdp2RotationMatrix.m[0][1].asS32() * dy - gVdp2RotationMatrix.m[0][2].asS32() * dz) +
            coeff.m3C * -0x10000;

        s32 scaledY = MTH_Mul(pThis->m3C_scale, pThis->mC_cameraPosition.m8_Z.asS32() << 4);
        gVdp2RotationMatrix.My = ((scaledY - gVdp2RotationMatrix.m[1][0].asS32() * dx) -
            gVdp2RotationMatrix.m[1][1].asS32() * dy - gVdp2RotationMatrix.m[1][2].asS32() * dz) +
            coeff.m3E * -0x10000;

        gVdp2RotationMatrix.Mz = ((((pThis->mC_cameraPosition.m4_Y.asS32() - pThis->m38) * 0x40 -
            gVdp2RotationMatrix.m[2][0].asS32() * dx) - gVdp2RotationMatrix.m[2][1].asS32() * dy) -
            gVdp2RotationMatrix.m[2][2].asS32() * dz) + coeff.m40 * -0x10000;
    }

    // 0606e334
    static void Draw(sCaraVdp2Plane* pThis)
    {
        pThis->mC_cameraPosition = cameraProperties2.m0_position;
        pThis->m18_cameraRotation.m0_X = (s32)(s16)cameraProperties2.mC_rotation[0] << 16;
        pThis->m18_cameraRotation.m4_Y = (s32)(s16)cameraProperties2.mC_rotation[1] << 16;
        pThis->m18_cameraRotation.m8_Z = (s32)(s16)cameraProperties2.mC_rotation[2] << 16;
        getVdp1ClippingCoordinates(pThis->m24_vdp1Clipping);
        getVdp1ProjectionParams(&pThis->m30_projParam0, &pThis->m32_projParam1);

        fixedPoint projScale = performDivision((s32)pThis->m30_projParam0, (s32)pThis->m32_projParam1 << 16);
        beginRotationPass(0, projScale);
        DrawSub0(pThis);
        drawCinematicBar(6);
        commitRotationPass();

        pThis->m34_groundY = computeGroundY();
        pThis->m0_scrollX = (pThis->m18_cameraRotation.m4_Y.asS32() >> 12) * -0x400;
        pThis->m4_scrollY = (0x1FF - pThis->m34_groundY) * 0x10000;

        projScale = performDivision((s32)pThis->m30_projParam0, (s32)pThis->m32_projParam1 << 16);
        beginRotationPass(1, projScale);

        auto& coeff = gCoefficientTables[gRotationPassState.m0_planeIndex][vdp2Controls.m0_doubleBufferIndex];

        coeff.m34 = (s16)((pThis->m24_vdp1Clipping[0] + pThis->m24_vdp1Clipping[2] + (pThis->m24_vdp1Clipping[0] + pThis->m24_vdp1Clipping[2] < 0 ? 1 : 0)) >> 1);
        coeff.m36 = (s16)((pThis->m24_vdp1Clipping[1] + pThis->m24_vdp1Clipping[3] + (pThis->m24_vdp1Clipping[1] + pThis->m24_vdp1Clipping[3] < 0 ? 1 : 0)) >> 1);
        coeff.m38 = pThis->m32_projParam1;
        coeff.m3C = coeff.m34;
        coeff.m3E = coeff.m36;
        coeff.m40 = 0;

        buildRotationMatrixRoll(-pThis->m18_cameraRotation.m8_Z.asS32());
        performDivision((s32)pThis->m24_vdp1Clipping[3] - (s32)pThis->m24_vdp1Clipping[1], 0xE00000);
        fixedPoint scaleVal = performDivision((s32)pThis->m24_vdp1Clipping[2] - (s32)pThis->m24_vdp1Clipping[0], 0x1600000);
        scaleRotationMatrix(scaleVal);
        setRotationScrollOffset(pThis->m0_scrollX, pThis->m4_scrollY);
        commitRotationPass();

        pauseEngine[4] = 0;
        setVdp2ScrollPosition(0, 0);

        s_VDP2Regs* regs = vdp2Controls.m4_pendingVdp2Regs;
        pauseEngine[4] = 4;
        regs->mC0_WPSX0 = pThis->m24_vdp1Clipping[0] << 1;
        regs->mC2_WPSY0 = pThis->m24_vdp1Clipping[1];
        regs->mC4_WPEX0 = pThis->m24_vdp1Clipping[2] << 1;
        regs->mC6_WPEY0 = pThis->m24_vdp1Clipping[3];
    }

    s32 m0_scrollX;
    s32 m4_scrollY;
    sVec3_FP mC_cameraPosition;
    sVec3_FP m18_cameraRotation;
    std::array<s16, 4> m24_vdp1Clipping;
    s16 m30_projParam0;
    s16 m32_projParam1;
    s32 m34_groundY;
    s32 m38;
    s32 m3C_scale;
    s32 m40_timer;
    s32 m44;
    // size 0x48
};


// 0606dfd6
static void createCaravanVdp2Plane(p_workArea pParent)
{
    gCaraVdp2PlaneTask = createSubTask<sCaraVdp2Plane>(pParent);
}

void initEdgeNPC(sNPC* pThis, sSaturnPtr arg); // from townEdge.cpp

// Helper: get animation from table entry
static sAnimationData* getAnimFromTable(sNPC* pThis, s32 animIndex) {
    sSaturnPtr animEntry = pThis->m30_animationTable + animIndex * 4;
    s16 fileIdx = readSaturnS16(animEntry);
    u16 animOffset = readSaturnU16(animEntry + 2);
    s_fileBundle* pBundle = (fileIdx != 0) ? dramAllocatorEnd[0].mC_fileBundle->m0_fileBundle : pThis->m0_fileBundle;
    return pBundle->getAnimation(animOffset);
}

// 06075858 — idle animation randomizer (when NPC stops walking)
static void updateNPCIdleAnimation(sNPC* pThis) {
    pThis->m14E--;
    if (pThis->m14E == 0 || pThis->m2C_currentAnimation < 5 || pThis->m2C_currentAnimation > 8) {
        s32 newAnim;
        u16 timer;

        if (pThis->m2C_currentAnimation == 5 && (npcData0.mFC & 0x11) == 0) {
            u32 rng = performModulo2(100, randomNumber() & 0x7FFFFFFF);
            if (rng < 0x42) {
                newAnim = readSaturnS16(gCurrentTownOverlay->getSaturnPtr(0x06087b24));
                timer = readSaturnU8(gCurrentTownOverlay->getSaturnPtr(0x06087b24) + 1);
            } else if (rng < 99) {
                newAnim = readSaturnS16(gCurrentTownOverlay->getSaturnPtr(0x06087b28));
                timer = readSaturnU8(gCurrentTownOverlay->getSaturnPtr(0x06087b28) + 1);
            } else {
                newAnim = readSaturnS16(gCurrentTownOverlay->getSaturnPtr(0x06087b2c));
                timer = readSaturnU8(gCurrentTownOverlay->getSaturnPtr(0x06087b2c) + 1);
            }
        } else {
            newAnim = 5;
            u32 rng = performModulo2(0x514, randomNumber() & 0x7FFFFFFF);
            timer = (u16)rng + 200;
        }

        pThis->m14E = timer;
        if (pThis->m2C_currentAnimation != newAnim) {
            s32 interpLen = (pThis->m2C_currentAnimation < 5) ? 5 : 10;
            pThis->m2C_currentAnimation = newAnim;
            playAnimationGeneric(&pThis->m34_3dModel, getAnimFromTable(pThis, newAnim), interpLen);
        }
    }
    updateAndInterpolateAnimation(&pThis->m34_3dModel);
}

// 06075976 — walk/run animation controller (control state 4)
static void updateNPCWalkRun(sNPC* pThis) {
    sVec3_FP delta;
    delta.m0_X = pThis->mE8.m0_position.m0_X - pThis->mE8.m54_oldPosition.m0_X;
    delta.m4_Y = pThis->mE8.m0_position.m4_Y - pThis->mE8.m54_oldPosition.m4_Y;
    delta.m8_Z = pThis->mE8.m0_position.m8_Z - pThis->mE8.m54_oldPosition.m8_Z;
    s32 dist = sqrt_I((s64)delta.m0_X.asS32() * delta.m0_X.asS32() +
                      (s64)delta.m4_Y.asS32() * delta.m4_Y.asS32() +
                      (s64)delta.m8_Z.asS32() * delta.m8_Z.asS32());
    s32 speed = dist * 0x1E1;
    s32 oldAccum = pThis->m28_animationLeftOver.asS32();
    pThis->m28_animationLeftOver = (oldAccum + speed) & 0xFFFF;

    if (speed > 0x666) {
        // Moving — switch between walk (1) and run (2)
        if (pThis->m2C_currentAnimation == 2) {
            if (speed < 0x28000) {
                // Slow down: run → walk
                pThis->m2C_currentAnimation = 1;
                playAnimationGeneric(&pThis->m34_3dModel, getAnimFromTable(pThis, 1), 5);
            }
        } else if (pThis->m2C_currentAnimation == 1) {
            if (speed > 0x30000) {
                // Speed up: walk → run
                pThis->m2C_currentAnimation = 2;
                playAnimationGeneric(&pThis->m34_3dModel, getAnimFromTable(pThis, 2), 5);
            }
        } else {
            // Start walking
            pThis->m2C_currentAnimation = 1;
            playAnimationGeneric(&pThis->m34_3dModel, getAnimFromTable(pThis, 1), 5);
        }

        // Step animation with footstep sounds
        u32 steps = (u32)(oldAccum + speed) >> 16;
        if (steps != 0) {
            if (pThis->m2C_currentAnimation == 1) {
                for (u32 i = 0; i < steps; i++) {
                    s16 frame = stepAnimation(&pThis->m34_3dModel);
                    if (frame == 8 || frame == 0x1B) {
                        playSystemSoundEffect(0x22);
                    }
                }
            } else {
                for (u32 i = 0; i < steps; i++) {
                    s16 frame = stepAnimation(&pThis->m34_3dModel);
                    if (frame == 0xB || frame == 0x2B) {
                        playSystemSoundEffect(0x23);
                    }
                }
            }
        }
        interpolateAnimation(&pThis->m34_3dModel);
        pThis->m14E = 0;
    } else {
        // Stopped — idle animation
        updateNPCIdleAnimation(pThis);
    }
}
void scheduleNPCAnimationFromTable(sNPC* pThis, s8 animId, u8 mode); // from townEdge.cpp

// 06074342
static void updateNPCRotationAndMovement(sNPC* pThis)
{
    if (pThis->mF & 2) {
        // Head/body rotation towards target angle
        fixedPoint maxSpeed = MTH_Mul(0x2D82D8, pThis->m1C);
        fixedPoint negSpeed = -maxSpeed;

        if ((pThis->mF & 4) == 0) {
            // Rotate body Y towards target
            s32 diff = pThis->mE8.m48_targetRotation.m4_Y.asS32() - pThis->mE8.mC_rotation.m4_Y.asS32();
            diff = (diff & 0x8000000) ? (diff | 0xF0000000) : (diff & 0xFFFFFFF);

            s32 speed;
            if (diff < 0) {
                speed = negSpeed.asS32();
                if (negSpeed.asS32() <= diff) {
                    pThis->mF &= ~2;
                    speed = diff;
                }
                s32 clampedDiff = (diff < -0x1C71C71) ? -0x1C71C71 : diff;
                pThis->m20_lookAtAngle[1] = pThis->m20_lookAtAngle[1].asS32() + speed;
                if (pThis->m20_lookAtAngle[1].asS32() < clampedDiff) {
                    pThis->m20_lookAtAngle[1] = clampedDiff;
                }
            } else {
                speed = maxSpeed.asS32();
                if (diff <= maxSpeed.asS32()) {
                    pThis->mF &= ~2;
                    speed = diff;
                }
                s32 clampedDiff = (diff > 0x1C71C71) ? 0x1C71C71 : diff;
                pThis->m20_lookAtAngle[1] = pThis->m20_lookAtAngle[1].asS32() + speed;
                if (pThis->m20_lookAtAngle[1].asS32() > clampedDiff) {
                    pThis->m20_lookAtAngle[1] = clampedDiff;
                }
            }
            pThis->mE8.mC_rotation.m4_Y = pThis->mE8.mC_rotation.m4_Y.asS32() + speed;
        } else {
            // Rotate look-at angle towards target (with body limit check)
            s32 diff = pThis->mE8.m48_targetRotation.m4_Y.asS32() - pThis->m20_lookAtAngle[1].asS32();
            diff = (diff & 0x8000000) ? (diff | 0xF0000000) : (diff & 0xFFFFFFF);

            if (diff < 0) {
                if (diff < negSpeed.asS32()) {
                    pThis->m20_lookAtAngle[1] = pThis->m20_lookAtAngle[1] - maxSpeed;
                } else {
                    pThis->m20_lookAtAngle[1] = pThis->m20_lookAtAngle[1].asS32() + diff;
                    pThis->mF &= ~2;
                }
            } else {
                if (diff > maxSpeed.asS32()) {
                    pThis->m20_lookAtAngle[1] = pThis->m20_lookAtAngle[1] + maxSpeed;
                } else {
                    pThis->m20_lookAtAngle[1] = pThis->m20_lookAtAngle[1].asS32() + diff;
                    pThis->mF &= ~2;
                }
            }
        }
    } else if (pThis->mF & 1) {
        // Walk towards target position
        fixedPoint distSq = distanceSquareBetween2Points(pThis->mE8.m3C_targetPosition, pThis->mE8.m0_position);
        fixedPoint threshSq = FP_Pow2(pThis->mE8.m30_stepTranslation.m8_Z);

        if (threshSq.asS32() < distSq.asS32()) {
            // Steer towards target
            s32 targetAngle = atan2_FP(pThis->mE8.m0_position.m0_X - pThis->mE8.m3C_targetPosition.m0_X,
                                       pThis->mE8.m0_position.m8_Z - pThis->mE8.m3C_targetPosition.m8_Z);
            s32 diff = targetAngle - pThis->mE8.mC_rotation.m4_Y.asS32();
            diff = (diff & 0x8000000) ? (diff | 0xF0000000) : (diff & 0xFFFFFFF);
            if (diff < -0x2D82D8) diff = -0x2D82D8;
            if (diff > 0x2D82D8) diff = 0x2D82D8;
            pThis->mE8.mC_rotation.m4_Y = pThis->mE8.mC_rotation.m4_Y + diff;
            stepNPCForward(&pThis->mE8);
        } else {
            // Arrived at target
            pThis->mE8.m0_position = pThis->mE8.m3C_targetPosition;
            pThis->mF &= ~1;
        }
    } else {
        pThis->mC &= ~4;
    }
}

// 06074250
static s32 turnNpcTowardsNpc_Cara(s32 npcIndex, s32 targetIndex, s32 mode) {
    sNPC* pNPC1 = getNpcDataByIndex(npcIndex);
    sNPC* pNPC2 = getNpcDataByIndex(targetIndex);
    s32 angle = atan2_FP(pNPC1->mE8.m0_position.m0_X - pNPC2->mE8.m0_position.m0_X,
                         pNPC1->mE8.m0_position.m8_Z - pNPC2->mE8.m0_position.m8_Z);
    if (mode != 0) {
        angle = angle - pNPC1->mE8.mC_rotation.m4_Y.asS32();
    }
    pNPC1 = getNpcDataByIndex(npcIndex);
    pNPC1->mE8.m48_targetRotation.m4_Y = angle;
    if (mode == 0) {
        pNPC1->m148_savedAngle = pNPC1->mE8.mC_rotation.m4_Y;
    } else {
        pNPC1->m148_savedAngle = pNPC1->m20_lookAtAngle[1];
    }
    pNPC1->mF = (pNPC1->mF & ~4);
    if (mode == 0) {
        pNPC1->mF |= 2;
        scheduleNPCAnimationFromTable(pNPC1, 3, 2);
    } else {
        pNPC1->mF |= 6;
    }
    pNPC1->mC |= 4;
    return 0;
}

// 060749fe
static s32 isNpcDoneMoving(s32 npcIndex) {
    sNPC* pNPC = getNpcDataByIndex(npcIndex);
    if ((pNPC->mC & 6) != 0) return 0;
    return 1;
}

// 06073b68 / 06073cf4 — same as shared applyEdgeAnimation / applyEdgeAnimation2 in townEdge.cpp
// (CARA version also adds lookAtAngle[0] to head pitch, minor difference)

// CARA entity type - small static object (size 0x28)
struct sCaraEntitySmall : public s_workAreaTemplateWithArgAndBase<sCaraEntitySmall, sTownObject, sSaturnPtr>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sCaraEntitySmall::Init, &sCaraEntitySmall::Update, nullptr, &sCaraEntitySmall::Delete };
        return &taskDefinition;
    }

    // 06073268
    static void Init(sCaraEntitySmall* pThis, sSaturnPtr arg)
    {
        pThis->mC = arg;
        pThis->m18_position = readSaturnVec3(arg + 8);
        pThis->m24_status = readSaturnU8(arg + 0x24);
        s16 npcIndex = readSaturnS16(arg + 0x26);
        if (npcIndex > -1) {
            npcData0.m70_npcPointerArray[npcIndex].workArea = pThis;
        }
    }

    // 06073292 — loading state
    static void Update(sCaraEntitySmall* pThis)
    {
        if (isDataLoaded(readSaturnS32(pThis->mC))) {
            Unimplemented(); // particleInitSub — needs VDP1 memory pointer from file bundle
            pThis->m_UpdateMethod = &sCaraEntitySmall::Update2;
            pThis->m_DrawMethod = &sCaraEntitySmall::Draw;
        }
    }

    // 0607322c
    static void Update2(sCaraEntitySmall* pThis)
    {
        if (pThis->m24_status == 0) {
            sGunShotTask_UpdateSub4(&pThis->m10_animatedQuad);
        }
        else if (pThis->m24_status == 1) {
            Unimplemented(); // FUN_06014db4
        }
    }

    // 06073258
    static void Draw(sCaraEntitySmall* pThis)
    {
        Unimplemented();
    }

    static void Delete(sCaraEntitySmall* pThis) { Unimplemented(); }

    sSaturnPtr mC;
    sAnimatedQuad m10_animatedQuad;
    sVec3_FP m18_position;
    u8 m24_status;
    // size 0x28
};

// CARA entity type - medium entity with collision (size 0x90)
struct sCaraEntityMedium : public s_workAreaTemplateWithArgAndBase<sCaraEntityMedium, sTownObject, sSaturnPtr>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sCaraEntityMedium::Init, &sCaraEntityMedium::Update, nullptr, &sCaraEntityMedium::Delete };
        return &taskDefinition;
    }

    // 06072f5a
    static void Init(sCaraEntityMedium* pThis, sSaturnPtr arg)
    {
        pThis->mC = arg;
        pThis->m74_position = readSaturnVec3(arg + 8);
        pThis->m80_rotation = readSaturnVec3(arg + 0x14);
        pThis->m8C_status = readSaturnU8(arg + 0x22);
        s16 npcIndex = readSaturnS16(arg + 0x28);
        if (npcIndex > -1) {
            npcData0.m70_npcPointerArray[npcIndex].workArea = pThis;
        }
    }

    // 06072f9a — loading state
    static void Update(sCaraEntityMedium* pThis)
    {
        if (isDataLoaded(readSaturnS32(pThis->mC))) {
            // Set up collision body from entity config at arg+0x24
            sSaturnPtr collisionConfig = readSaturnEA(pThis->mC + 0x24);
            if (!collisionConfig.isNull()) {
                pThis->m10_collisionBody.m30_pPosition = &pThis->m74_position;
                pThis->m10_collisionBody.m34_pRotation = &pThis->m80_rotation;
                pThis->m10_collisionBody.m38_pOwner = pThis;
                pThis->m10_collisionBody.m3C_scriptEA = readSaturnEA(collisionConfig + 4);

                s16 collisionModelOffset = readSaturnS16(collisionConfig + 2);
                if (collisionModelOffset == 0) {
                    pThis->m10_collisionBody.m40 = nullptr;
                } else {
                    pThis->m10_collisionBody.m40 = pThis->m0_fileBundle->getCollisionModel(collisionModelOffset);
                }

                setCollisionSetup(&pThis->m10_collisionBody, readSaturnS8(collisionConfig));
                setCollisionBounds(&pThis->m10_collisionBody, readSaturnVec3(collisionConfig + 0x8), readSaturnVec3(collisionConfig + 0x14));
            }
            pThis->m_UpdateMethod = &sCaraEntityMedium::Update2;
            pThis->m_DrawMethod = &sCaraEntityMedium::Draw;
        }
    }

    // 06072ef0
    static void Update2(sCaraEntityMedium* pThis)
    {
        if (pThis->m8C_status == 1) {
            Unimplemented(); // FUN_06014db4
        }
        if (!readSaturnEA(pThis->mC + 0x24).isNull()) {
            registerCollisionBody(&pThis->m10_collisionBody);
        }
    }

    // 06072f28
    static void Draw(sCaraEntityMedium* pThis)
    {
        Unimplemented();
    }

    static void Delete(sCaraEntityMedium* pThis) { Unimplemented(); }

    sSaturnPtr mC;
    sCollisionBody m10_collisionBody;
    sVec3_FP m74_position;
    sVec3_FP m80_rotation;
    u8 m8C_status;
    // size 0x90
};

static sTownObject* createCaraEntitySmall(s_workAreaCopy* parent, sSaturnPtr arg)
{
    return createSubTaskWithArgWithCopy<sCaraEntitySmall, sSaturnPtr>(parent, arg);
}

static sTownObject* createCaraEntityMedium(s_workAreaCopy* parent, sSaturnPtr arg)
{
    return createSubTaskWithArgWithCopy<sCaraEntityMedium, sSaturnPtr>(parent, arg);
}

// 060734a8 is initEdgeNPC — CARA NPCs use the standard sNPC/sEdgeTask layout
struct sCaraNPC : public s_workAreaTemplateWithArgAndBase<sCaraNPC, sNPC, sSaturnPtr>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sCaraNPC::Init, &sCaraNPC::Update, nullptr, &sCaraNPC::Delete };
        return &taskDefinition;
    }

    // 060734a8
    static void Init(sCaraNPC* pThis, sSaturnPtr arg)
    {
        initEdgeNPC(pThis, arg);
    }

    // 060735e8 — loading state, waits for data then inits model
    static void Update(sCaraNPC* pThis)
    {
        sSaturnPtr entityArg = pThis->m10_InitPtr;
        if (isDataLoaded(readSaturnS32(entityArg))) {
            s_fileBundle* pBundle = pThis->m0_fileBundle;
            u16 modelIndex = readSaturnU16(entityArg + 0x22);
            sStaticPoseData* pPose = pBundle->getStaticPose(readSaturnU16(entityArg + 0x24), pBundle->getModelHierarchy(modelIndex)->countNumberOfBones());
            init3DModelRawData(pThis, &pThis->m34_3dModel, 0x100, pBundle, modelIndex, nullptr, pPose, nullptr, nullptr);

            if (readSaturnU8(entityArg + 0x21) & 0x40) {
                Unimplemented(); // FUN_TWN_CARA__0606eca0
            }
            pThis->m_UpdateMethod = &sCaraNPC::Update2;
            pThis->m_DrawMethod = &sCaraNPC::Draw;
        }
    }

    // 06073652 — main update: handles animation, movement, collision
    static void Update2(sCaraNPC* pThis)
    {
        pThis->mE8.m54_oldPosition = pThis->mE8.m0_position;

        if (pThis->mC == 0) {
            // call m14_updateFunction
            if (pThis->m14_updateFunction) {
                pThis->m14_updateFunction(pThis);
            }
        }
        else {
            if (((pThis->mF & 2) == 0) && ((pThis->mC & 8) == 0)) {
                pThis->m20_lookAtAngle[1] = MTH_Mul(pThis->m20_lookAtAngle[1], 0xB333);
            }
            if (pThis->mC & 2) {
                Unimplemented(); // FUN_TWN_CARA__06074106 — movement towards target
            }
            if (pThis->mC & 4) {
                updateNPCRotationAndMovement(pThis);
            }
        }

        // Animation state machine
        s8 controlState = pThis->mE_controlState;
        if (controlState == 0) {
            // Process animation ring buffer
            if (pThis->m17A != 0) {
                s8 readIdx = pThis->m179;
                u8* animQueue = pThis->m158_animQueue;
                s8 animId = (s8)animQueue[readIdx * 2];
                u8 animMode = animQueue[readIdx * 2 + 1];

                if (pThis->m1C != (s32)animId) {
                    if (animId == 0) {
                        playAnimationGeneric(&pThis->m34_3dModel, nullptr, 10);
                        pThis->m1C = 0;
                    }
                    else {
                        pThis->m1C = (s32)animId;
                        sSaturnPtr animEntry = pThis->m30_animationTable + animId * 4;
                        s16 fileIdx = readSaturnS16(animEntry);
                        u16 animOffset = readSaturnU16(animEntry + 2);
                        s_fileBundle* pBundle = (fileIdx != 0) ? dramAllocatorEnd[0].mC_fileBundle->m0_fileBundle : pThis->m0_fileBundle;
                        sAnimationData* pAnim = pBundle->getAnimation(animOffset);
                        playAnimationGeneric(&pThis->m34_3dModel, pAnim, 10);
                        updateAndInterpolateAnimation(&pThis->m34_3dModel);
                    }
                }

                pThis->mE_controlState = animMode;
                readIdx++;
                if (readIdx > 7) readIdx = 0;
                pThis->m179 = readIdx;
                pThis->m17A--;
            }
        }
        else if (controlState == 1) {
            // Walking — step animation based on distance moved
            sVec3_FP delta;
            delta.m0_X = pThis->mE8.m0_position.m0_X - pThis->mE8.m54_oldPosition.m0_X;
            delta.m4_Y = pThis->mE8.m0_position.m4_Y - pThis->mE8.m54_oldPosition.m4_Y;
            delta.m8_Z = pThis->mE8.m0_position.m8_Z - pThis->mE8.m54_oldPosition.m8_Z;
            s32 dist = sqrt_I((s64)delta.m0_X.asS32() * delta.m0_X.asS32() + (s64)delta.m4_Y.asS32() * delta.m4_Y.asS32() + (s64)delta.m8_Z.asS32() * delta.m8_Z.asS32());

            u32 accumulator = pThis->m28_animationLeftOver.asS32() + dist * 0x1E1;
            pThis->m28_animationLeftOver = accumulator & 0xFFFF;

            u32 steps;
            if (dist * 0x1E1 == 0) {
                // Stopped — switch to idle animation
                if (pThis->m1C != 0) {
                    pThis->m1C = 0;
                    sSaturnPtr animEntry = pThis->m30_animationTable;
                    s16 fileIdx = readSaturnS16(animEntry);
                    s_fileBundle* pBundle = (fileIdx != 0) ? dramAllocatorEnd[0].mC_fileBundle->m0_fileBundle : pThis->m0_fileBundle;
                    playAnimationGeneric(&pThis->m34_3dModel, pBundle->getAnimation(readSaturnS16(animEntry + 2)), 5);
                }
                steps = 1;
            }
            else {
                // Moving — switch to walk animation
                if (pThis->m1C != 1) {
                    pThis->m1C = 1;
                    sSaturnPtr animEntry = pThis->m30_animationTable + 4;
                    s16 fileIdx = readSaturnS16(animEntry);
                    s_fileBundle* pBundle = (fileIdx != 0) ? dramAllocatorEnd[0].mC_fileBundle->m0_fileBundle : pThis->m0_fileBundle;
                    playAnimationGeneric(&pThis->m34_3dModel, pBundle->getAnimation(readSaturnS16(animEntry + 2)), 5);
                }
                steps = accumulator >> 16;
            }

            for (; steps != 0; steps--) {
                stepAnimation(&pThis->m34_3dModel);
            }
            interpolateAnimation(&pThis->m34_3dModel);
        }
        else if (controlState == 2) {
            if (pThis->m34_3dModel.m30_pCurrentAnimation != nullptr) {
                u32 frame = updateAndInterpolateAnimation(&pThis->m34_3dModel);
                s32 numFrames = pThis->m34_3dModel.m30_pCurrentAnimation ? pThis->m34_3dModel.m30_pCurrentAnimation->m4_numFrames : 0;
                if (numFrames - 1 <= (s32)frame) {
                    pThis->mE_controlState = 0;
                }
            }
            else if (pThis->m34_3dModel.m34_pDefaultPose == nullptr) {
                pThis->mE_controlState = 0;
            }
            else {
                updateAndInterpolateAnimation(&pThis->m34_3dModel);
            }
        }
        else if (controlState == 3) {
            if (pThis->m34_3dModel.m30_pCurrentAnimation == nullptr) {
                pThis->mE_controlState = 2;
            }
            else {
                updateAndInterpolateAnimation(&pThis->m34_3dModel);
            }
        }
        else if (controlState == 4) {
            updateNPCWalkRun(pThis);
        }

        registerCollisionBody(&pThis->m84);
    }

    // 06073e08
    static void Draw(sCaraNPC* pThis)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->mE8.m0_position);
        rotateCurrentMatrixShiftedY(pThis->mE8.mC_rotation.m4_Y);
        rotateCurrentMatrixShiftedX(pThis->mE8.mC_rotation.m0_X);
        rotateCurrentMatrixShiftedY(0x8000000);

        if ((pThis->mF & 0x80) == 0) {
            // Draw shadow model from dramAllocator bundle
            u16 shadowOffset = readSaturnU16(pThis->m30_animationTable + 2);
            sProcessed3dModel* pShadowModel = dramAllocatorEnd[0].mC_fileBundle->m0_fileBundle->getModelHierarchy(shadowOffset)->m0_3dModel;
            addObjectToDrawList(pShadowModel);
        }

        // Draw NPC model with bone hierarchy and look-at blending
        // Note: CARA's version also adds lookAtAngle[0] to head pitch, but using shared version for now
        if (pThis->m34_3dModel.m48_poseDataInterpolation.size() == 0) {
            applyEdgeAnimation(&pThis->m34_3dModel, &pThis->m20_lookAtAngle);
        }
        else {
            applyEdgeAnimation2(&pThis->m34_3dModel, &pThis->m20_lookAtAngle);
        }

        popMatrix();
    }

    // 06073fbc
    static void Delete(sCaraNPC* pThis)
    {
        s8 npcIndex = readSaturnS8(pThis->m10_InitPtr + 0x20);
        if (npcData0.m70_npcPointerArray[npcIndex].workArea == pThis) {
            npcData0.m70_npcPointerArray[npcIndex].workArea = nullptr;
        }
    }

    // size 0x17C
};

static sTownObject* createCaraNPC(s_workAreaCopy* parent, sSaturnPtr arg)
{
    return createSubTaskWithArgWithCopy<sCaraNPC, sSaturnPtr>(parent, arg);
}

struct TWN_CARA_data* gTWN_CARA = NULL;
struct TWN_CARA_data : public sTownOverlay
{
    static void makeCurrent()
    {
        if (gTWN_CARA == NULL)
        {
            gTWN_CARA = new TWN_CARA_data();
        }
        gCurrentTownOverlay = gTWN_CARA;
    }

    TWN_CARA_data() : sTownOverlay("TWN_CARA.PRG")
    {
        // Script functions
        overlayScriptFunctions.m_zeroArg[0x060540f8] = {&returnZero, "returnZero"};
        overlayScriptFunctions.m_zeroArg[0x06070c0c] = {&updateWorldGridFromEdgeTask, "updateWorldGridFromEdgeTask"};
        overlayScriptFunctions.m_zeroArg[0x060706d4] = {&setupCameraUpdateForCurrentMode, "setupCameraUpdateForCurrentMode"};
        overlayScriptFunctions.m_zeroArg[0x060709ec] = {&setupCameraFollowMode, "setupCameraFollowMode"};
        overlayScriptFunctions.m_zeroArg[0x06070a0a] = {&disableCameraUpdate_Cara, "disableCameraUpdate_Cara"};

        overlayScriptFunctions.m_oneArg[0x06075e60] = {&TwnFadeIn, "TwnFadeIn"};
        overlayScriptFunctions.m_oneArg[0x06075ed8] = {&TwnFadeOut, "TwnFadeOut"};
        overlayScriptFunctions.m_oneArg[0x0606e6d4] = {&setupCaravanBackground, "setupCaravanBackground"};
        overlayScriptFunctions.m_oneArg[0x06075cda] = {&setupCaravanCamera, "setupCaravanCamera"};
        overlayScriptFunctions.m_oneArg[0x06070bd0] = {&setFieldOfView, "setFieldOfView"};
        overlayScriptFunctions.m_oneArg[0x060749d6] = {&disableNpcLookAtDecay_Cara, "disableNpcLookAtDecay_Cara"};
        overlayScriptFunctions.m_oneArg[0x06071484] = {&createCutscene_Cara, "createCutscene_Cara"};
        overlayScriptFunctions.m_oneArg[0x0606e8a8] = {&enableBackgroundWithTimer, "enableBackgroundWithTimer"};
        overlayScriptFunctions.m_oneArg[0x060710c6] = {&createCutsceneWaitTask, "createCutsceneWaitTask"};
        overlayScriptFunctions.m_oneArg[0x060749ea] = {&enableNpcLookAtDecay_Cara, "enableNpcLookAtDecay_Cara"};
        overlayScriptFunctions.m_oneArg[0x06072440] = {&createCaraSubTask, "createCaraSubTask"};

        overlayScriptFunctions.m_oneArgPtr[0x06075d5c] = {&townCamera_setupWithPosition, "townCamera_setupWithPosition"};

        overlayScriptFunctions.m_zeroArg[0x060714ac] = {&townStartCutscenePlayback, "townStartCutscenePlayback"};
        overlayScriptFunctions.m_zeroArg[0x060714ce] = {&townIsCutsceneDone, "townIsCutsceneDone"};
        overlayScriptFunctions.m_zeroArg[0x0607150c] = {&townGetCutsceneHalfPosition, "townGetCutsceneHalfPosition"};
        overlayScriptFunctions.m_zeroArg[0x06071544] = {&townDeleteCutscene, "townDeleteCutscene"};
        overlayScriptFunctions.m_zeroArg[0x060709fa] = {&setupCameraFollowMode_Cara, "setupCameraFollowMode_Cara"};
        overlayScriptFunctions.m_zeroArg[0x06070bfc] = {&resetFieldOfView, "resetFieldOfView"};
        overlayScriptFunctions.m_zeroArg[0x060706f4] = {&setupNpcWalkForward, "setupNpcWalkForward"};

        overlayScriptFunctions.m_twoArg[0x06075bf8] = {&caravanCameraSetup, "caravanCameraSetup"};

        overlayScriptFunctions.m_threeArg[0x0602c308] = {&playSoundEffect_bank5, "playSoundEffect_bank5"};
        overlayScriptFunctions.m_threeArg[0x06074250] = {&turnNpcTowardsNpc_Cara, "turnNpcTowardsNpc_Cara"};
        overlayScriptFunctions.m_threeArg[0x06070738] = {&setupCameraModeFixed, "setupCameraModeFixed"};
        overlayScriptFunctions.m_twoArg[0x06073f5c] = {&setNpcDrawMode, "setNpcDrawMode"};

        overlayScriptFunctions.m_zeroArg[0x060716aa] = {&isObjectCloseEnoughToActivate, "isObjectCloseEnoughToActivate"};

        overlayScriptFunctions.m_oneArg[0x060749fe] = {&isNpcDoneMoving, "isNpcDoneMoving"};

        overlayScriptFunctions.m_fourArg[0x0607457c] = {&setNpcLocation, "setNpcLocation"};
        overlayScriptFunctions.m_fourArg[0x060745aa] = {&setNpcOrientation, "setNpcOrientation"};

        // Parse town setup table from Saturn binary (10 entries)
        // First 6 entries: numScripts = 28
        for (int i = 0; i < 6; i++) {
            sSaturnPtr ptrBase = getSaturnPtr(0x06077e44) + 0xC * i;
            mTownSetups.push_back(readTownSetup(ptrBase, 28));
        }
        // Entries 6-7: numScripts = 16
        for (int i = 6; i < 8; i++) {
            sSaturnPtr ptrBase = getSaturnPtr(0x06077e44) + 0xC * i;
            mTownSetups.push_back(readTownSetup(ptrBase, 16));
        }
        // Entry 8: numScripts = 13
        {
            sSaturnPtr ptrBase = getSaturnPtr(0x06077e44) + 0xC * 8;
            mTownSetups.push_back(readTownSetup(ptrBase, 13));
        }
        // Entry 9: numScripts = 16
        {
            sSaturnPtr ptrBase = getSaturnPtr(0x06077e44) + 0xC * 9;
            mTownSetups.push_back(readTownSetup(ptrBase, 16));
        }
    }

    sTownObject* createObjectTaskFromEA_siblingTaskWithEAArgWithCopy(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg) override
    {
        switch (definitionEA.m_offset) {
        case 0x06087ae4:
            assert(size == 0x17C);
            return createCaraNPC(parent, arg);
        case 0x06087a98:
            assert(size == 0x90);
            return createCaraEntityMedium(parent, arg);
        case 0x06087ac0:
            assert(size == 0x28);
            return createCaraEntitySmall(parent, arg);
        case 0x06087a84:
            assert(size == 0xE0);
            return createExcaEntity(parent, arg);
        default:
            assert(0);
            return nullptr;
        }
    }

    sTownObject* createObjectTaskFromEA_subTaskWithEAArg(struct npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg) override
    {
        switch (definitionEA.m_offset) {
        default:
            assert(0);
            return nullptr;
        }
    }
};

// 06075b7e
static void initCameraTimerFromDayNight(sCameraTask* pCamera) {
    if (mainGameState.bitField[0] & 1) {
        pCamera->m4_dayNightTimer = 5400;
    }
    else {
        pCamera->m4_dayNightTimer = 0;
    }
}

// 06075b96
static void caravanToggleDayNight() {
    initCameraTimerFromDayNight(cameraTaskPtr);
    if (mainGameState.bitField[0] & 1) {
        return;
    }
    if (mainGameState.bitField[1] & 0x80) {
        mainGameState.bitField[1] &= 0x7F;
    }
    else {
        mainGameState.bitField[1] |= 0x80;
    }
}

// 060540d4
static void townOverlayDelete_TwnCara(townDebugTask2Function* pThis)
{
    caravanToggleDayNight();
    freeRamResources(pThis);
    vdp1FreeLastAllocation(pThis);
    unloadFnt();
}

// 06054000
p_workArea overlayStart_TWN_CARA(p_workArea pUntypedThis, u32 arg)
{
    gTWN_CARA->makeCurrent();

    townDebugTask2Function* pThis = static_cast<townDebugTask2Function*>(pUntypedThis);

    pThis->m_DeleteMethod = &townOverlayDelete_TwnCara;

    loadSoundBanks(-1, 0);
    loadSoundBanks(0x3F, 0);

    playPCM(pThis, 100);

    loadFnt("EVTCARA.FNT");

    graphicEngineStatus.m405C.m10_nearClipDistance = 0x800;
    graphicEngineStatus.m405C.m30_oneOverNearClip = FP_Div(0x10000, graphicEngineStatus.m405C.m10_nearClipDistance);

    graphicEngineStatus.m405C.m14_farClipDistance = 0x40000;
    graphicEngineStatus.m405C.m38_oneOverFarClip = FP_Div(0x8000, graphicEngineStatus.m405C.m14_farClipDistance);
    graphicEngineStatus.m405C.m34_oneOverFarClip256 = graphicEngineStatus.m405C.m38_oneOverFarClip << 8;

    initDramAllocator(pThis, townBuffer, 0xB0000, listOfFilesToLoad);

    initVdp1Ram(pThis, 0x25C18800, 0x63800);

    registerNpcs(gTWN_CARA->mTownSetups, gCurrentTownOverlay->getSaturnPtr(0x060542f8), arg);

    startScriptTask(pThis);

    // Background task
    createCaravanVdp2Plane(pThis);

    // Edge task
    sEdgeTask* pEdgeTask = startEdgeTask(gCurrentTownOverlay->getSaturnPtr(0x06077ebc));
    npcData0.m160_pEdgePosition = &pEdgeTask->m84.m8_position;

    // Main logic
    startMainLogic(pThis);

    twnMainLogicTask->m14_EdgeTask = pEdgeTask;

    // Camera task
    startCameraTask(pThis);

    return pThis;
}
