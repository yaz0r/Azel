#include "PDS.h"
#include "twn_cara.h"
#include "town/town.h"
#include "town/townScript.h"
#include "town/townEdge.h"
#include "town/townMainLogic.h"
#include "kernel/vdp1Allocator.h"
#include "audio/soundDriver.h"
#include "town/townCamera.h"
#include "town/excaEntity.h"
#include "town/e006/twn_e006.h"
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

static s32 isCutsceneDone();
s32 getPositionInEDK(sStreamingFile* iParm1);
static s32 caravanCameraSetup(s32 rotationEA, s32 lightData);
s32 setupCameraModeFixed(s32 arg0, s32 arg1, s32 arg2);

// 0607150c
static s32 getCutsceneHalfPosition() {
    if (e006Task0 != nullptr && npcData0.mF0 == 0) {
        s32 pos = getPositionInEDK(e006Task0->m0);
        return (pos + (pos < 0 ? 1 : 0)) >> 1;
    }
    return 0;
}

// 0602c308
static s32 playSoundEffect_bank5(s32 soundId, s32 volume, s32 unk) {
    enqueuePlaySoundEffect(soundId, 5, volume, unk);
    return 0;
}

// 06071544
static s32 deleteCutscene_Cara() {
    if (e006Task0 != nullptr) {
        npcData0.mF4 = 0;
        updateStreamingFileReadSub0(e006Task0->m0);
        sE006Task0* temp = e006Task0;
        e006Task0 = nullptr;
        temp->getTask()->markFinished();
    }
    return 0;
}

// 060709fa
static s32 setupCameraFollowMode_Cara() {
    Unimplemented(); // sets overlay-specific camera update/draw on twnMainLogicTask
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

// 06075d5c — same as camp/zoah setupCameraWithPosition


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
    if (isCutsceneDone() == 0) {
        Unimplemented(); // creates a timer sub-task on townVar0
    }
    return 0;
}

// 060714ce
static s32 isCutsceneDone() {
    if ((e006Task0 != nullptr) && (npcData0.mF0 == 0)) {
        s32 state = scriptFunction_605861eSub0Sub0(e006Task0->m0);
        if (state != 5 && state != -1) {
            return 0;
        }
    }
    return 1;
}

// 060714ac
static s32 startCutscenePlayback() {
    npcData0.mF0 = 0;
    if (e006Task0 != nullptr) {
        scriptFunction_60573d8Sub0(e006Task0->m0);
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
    e006Task0 = createSubTaskWithArg<sE006Task0>(twnMainLogicTask, arg);
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

void initEdgeNPC(sEdgeTask* pThis, sSaturnPtr arg); // from townEdge.cpp

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
        initEdgeNPC((sEdgeTask*)pThis, arg);
    }

    static void Update(sCaraNPC* pThis)
    {
        Unimplemented();
    }

    static void Delete(sCaraNPC* pThis)
    {
        Unimplemented();
    }
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

        overlayScriptFunctions.m_zeroArg[0x060714ac] = {&startCutscenePlayback, "startCutscenePlayback"};
        overlayScriptFunctions.m_zeroArg[0x060714ce] = {&isCutsceneDone, "isCutsceneDone"};
        overlayScriptFunctions.m_zeroArg[0x0607150c] = {&getCutsceneHalfPosition, "getCutsceneHalfPosition"};
        overlayScriptFunctions.m_zeroArg[0x06071544] = {&deleteCutscene_Cara, "deleteCutscene_Cara"};
        overlayScriptFunctions.m_zeroArg[0x060709fa] = {&setupCameraFollowMode_Cara, "setupCameraFollowMode_Cara"};
        overlayScriptFunctions.m_zeroArg[0x06070bfc] = {&resetFieldOfView, "resetFieldOfView"};
        overlayScriptFunctions.m_zeroArg[0x060706f4] = {&setupNpcWalkForward, "setupNpcWalkForward"};

        overlayScriptFunctions.m_twoArg[0x06075bf8] = {&caravanCameraSetup, "caravanCameraSetup"};

        overlayScriptFunctions.m_threeArg[0x0602c308] = {&playSoundEffect_bank5, "playSoundEffect_bank5"};
        overlayScriptFunctions.m_threeArg[0x06070738] = {&setupCameraModeFixed, "setupCameraModeFixed"};
        overlayScriptFunctions.m_twoArg[0x06073f5c] = {&setNpcDrawMode, "setNpcDrawMode"};

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

    sTownObject* createObjectTaskFromEA_siblingTaskWithEAArgWithCopy(struct npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg) override
    {
        switch (definitionEA.m_offset) {
        case 0x06087ae4:
            return createCaraNPC((s_workAreaCopy*)parent, arg);
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
        pCamera->m4_dayNightTimer = 0x1518;
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
