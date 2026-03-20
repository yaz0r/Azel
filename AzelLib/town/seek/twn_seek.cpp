#include "PDS.h"
#include "twn_seek.h"
#include "town/town.h"
#include "town/townScript.h"
#include "town/townEdge.h"
#include "town/townMainLogic.h"
#include "kernel/vdp1Allocator.h"
#include "audio/soundDriver.h"
#include "town/townCamera.h"
#include "town/excaEntity.h"
#include "battle/BTL_A3/BTL_A3_map6.h"
#include "field/field_a3/o_fld_a3.h"
#include "kernel/loadSavegameScreen.h"
#include "kernel/cinematicBarsTask.h"

void unloadFnt(); // TODO: fix

struct sSeekVdp2Plane : public s_workAreaTemplate<sSeekVdp2Plane> {
    static const TypedTaskDefinition* getTypedTaskDefinition() {
        static const TypedTaskDefinition td = { nullptr, nullptr, nullptr, nullptr };
        return &td;
    }

    s32 m0_scrollX;
    s32 m4_scrollY;
    s32 m8;
    sVec3_FP mC_cameraPosition;
    sVec3_FP m18_cameraRotation;
    std::array<s16, 4> m24_vdp1Clipping;
    std::array<s16, 2> m2C_vdp1LocalCoords;
    s16 m30_projParam0;
    s16 m32_projParam1;
    s32 m34_groundY;
    s32 m38_scale;
    s32 m3C;
    s32 m40;
    s32 m44;
    s32 m48;
    s32 m4C;
    u8 m50;
    u8 m51;
    u8 m52;
    u8 m53;
    u8 m54;
    u8 m55;
    u8 m56;
    u8 m57;
    s32 m58;
    s32 m5C;
    s32 m60;
    s32 m64;
    s32 m68;
    u8 m_pad[0x128 - 0x6C];
    // size 0x128
};

static sSeekVdp2Plane* gSeekVdp2PlaneTask = nullptr;

// Forward declarations for background mode 0 methods
static void seekBgMode0_Init(sSeekVdp2Plane* pThis);
static void seekBgMode0_Update(sSeekVdp2Plane* pThis);
static void seekBgMode0_Draw(sSeekVdp2Plane* pThis);
static void seekBgMode1_Init(sSeekVdp2Plane* pThis);
static void seekBgMode1_Update(sSeekVdp2Plane* pThis);
static void seekBgMode1_Draw(sSeekVdp2Plane* pThis);

// 0606e572
static void reinitVdp2PlaneTask(sSeekVdp2Plane* pTask,
    void(*initFn)(sSeekVdp2Plane*), void(*updateFn)(sSeekVdp2Plane*), void(*drawFn)(sSeekVdp2Plane*)) {
    memset(&pTask->m0_scrollX, 0, sizeof(sSeekVdp2Plane) - offsetof(sSeekVdp2Plane, m0_scrollX));
    pTask->m_UpdateMethod = (decltype(pTask->m_UpdateMethod))updateFn;
    pTask->m_DrawMethod = (decltype(pTask->m_DrawMethod))drawFn;
    if (initFn) {
        initFn(pTask);
    }
}

// 06070c80
static s32 updateWorldGridFromEdgeTask() {
    if (twnMainLogicTask->m14_EdgeTask) {
        updateWorldGrid(twnMainLogicTask->m14_EdgeTask->mE8.m0_position[0], twnMainLogicTask->m14_EdgeTask->mE8.m0_position[2]);
    }
    return fileInfoStruct.m2C_allocatedHead == 0;
}

// 0606e5a4
static s32 setupSeekerBackground(s32 param_1) {
    if (!gSeekVdp2PlaneTask) return 0;
    switch (param_1) {
    case 0:
        reinitVdp2PlaneTask(gSeekVdp2PlaneTask, &seekBgMode0_Init, &seekBgMode0_Update, &seekBgMode0_Draw);
        break;
    case 1:
        reinitVdp2PlaneTask(gSeekVdp2PlaneTask, &seekBgMode1_Init, &seekBgMode1_Update, &seekBgMode1_Draw);
        break;
    default:
        Unimplemented(); // variant background (init only, no update/draw)
        break;
    }
    return 0;
}

// 0606e560
static void createSeekerVdp2Plane(p_workArea pParent) {
    gSeekVdp2PlaneTask = createSubTask<sSeekVdp2Plane>(pParent);
}

// 0606d600 — outdoor background init
static void seekBgMode0_Init(sSeekVdp2Plane* pThis) {
    reinitVdp2();
    initNBG1Layer();
    asyncDmaCopy(gCurrentTownOverlay->getSaturnPtr(0x06080f60), getVdp2Cram(0x800), 0x200, 0);
    asyncDmaCopy(gCurrentTownOverlay->getSaturnPtr(0x06081264), vdp2Palette, 0x200, 0);

    static const sLayerConfig rgb0Setup[] = {
        {m1_TPEN, 1}, {m2_CHCN, 1}, {m5_CHSZ, 1}, {m6_PNB, 1}, {m7_CNSM, 0},
        {m27_RPMD, 2}, {m11_SCN, 8}, {m34_W0E, 1}, {m37_W0A, 1}, {m44_CCEN, 1},
        {m0_END},
    };
    setupRGB0(rgb0Setup);

    static const sLayerConfig rotParamsSetup[] = { {m31_RxKTE, 1}, {m0_END} };
    setupRotationParams(rotParamsSetup);

    static const sLayerConfig rotParams2Setup[] = { {m0_END} };
    setupRotationParams2(rotParams2Setup);

    loadFile("SEEKSCR.SCB", getVdp2Vram(0x40000), 0);
    loadFile("SEEKSCR.PNB", getVdp2Vram(0x60000), 0);
    loadFile("SPACE.PNB", getVdp2Vram(0x64000), 0);

    vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL = (vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL & 0xFF00) | 0xB4;
    vdp2Controls.m4_pendingVdp2Regs->m10_CYCA0 = 0x13FF57FF;
    vdp2Controls.m_isDirty = 1;

    setupRotationMapPlanes(0, gCurrentTownOverlay->getSaturnPtr(0x06081160));
    setupRotationMapPlanes(1, gCurrentTownOverlay->getSaturnPtr(0x060811a0));

    setupVdp2Table(6, coefficientA0, coefficientA1, getVdp2Vram(0x22000), 0x80);
    s_BTL_A3_Env_InitVdp2Sub3(5, getVdp2Vram(0x2A000));
    *(u16*)getVdp2Vram(0x2A400) = 0x400;

    vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA = (vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA & 0xFFF80000) | 0x15200;
    *(u16*)getVdp2Vram(0x25002) = 0x8000;
    vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA = (vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA & 0xFFF80000) | 0x12801;

    vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL = (vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL & 0xFEFF) | 0x100;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xFFF0) | 3;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xF8FF) | 0x200;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xCFFF) | 0x1000;

    vdp2Controls.m4_pendingVdp2Regs->mF0_PRISA = 0x204;
    vdp2Controls.m4_pendingVdp2Regs->mF2_PRISB = 0x407;
    vdp2Controls.m4_pendingVdp2Regs->mF4_PRISC = 0x404;
    vdp2Controls.m4_pendingVdp2Regs->mF6_PRISD = 0x404;
    vdp2Controls.m4_pendingVdp2Regs->mF8_PRINA = 0x600;
    vdp2Controls.m4_pendingVdp2Regs->mFA_PRINB = 0x700;
    vdp2Controls.m4_pendingVdp2Regs->mFC_PRIR = 3;
    vdp2Controls.m_isDirty = 1;

    pThis->m38_scale = 0x100000;
}

// 0606d846
static void seekBgMode0_Update(sSeekVdp2Plane* pThis) {
    pThis->m34_groundY = 0;
}

// 0606da6c — rotation pass 0 sub (same pattern as CARA's DrawSub0)
static void seekBgDrawSub0(sSeekVdp2Plane* pThis) {
    auto& coeff = gCoefficientTables[gRotationPassState.m0_planeIndex][vdp2Controls.m0_doubleBufferIndex];

    coeff.m34 = (s16)((pThis->m24_vdp1Clipping[0] + pThis->m24_vdp1Clipping[2] + (pThis->m24_vdp1Clipping[0] + pThis->m24_vdp1Clipping[2] < 0 ? 1 : 0)) >> 1);
    coeff.m36 = (s16)((pThis->m24_vdp1Clipping[1] + pThis->m24_vdp1Clipping[3] + (pThis->m24_vdp1Clipping[1] + pThis->m24_vdp1Clipping[3] < 0 ? 1 : 0)) >> 1);
    coeff.m38 = pThis->m32_projParam1;
    coeff.m3C = coeff.m34;
    coeff.m3E = coeff.m36;
    coeff.m40 = 0;

    buildRotationMatrixPitchYaw(-0x4000000 - pThis->m18_cameraRotation.m0_X.asS32(), -pThis->m18_cameraRotation.m4_Y.asS32());
    scaleRotationMatrix(pThis->m38_scale >> 2);
    writeRotationParams(-pThis->m18_cameraRotation.m8_Z.asS32());

    s32 dx = coeff.m34 - coeff.m3C;
    s32 dy = coeff.m36 - coeff.m3E;
    s32 dz = coeff.m38 - coeff.m40;

    s32 scaledX = MTH_Mul(pThis->m38_scale, pThis->mC_cameraPosition.m0_X.asS32() << 4);
    gVdp2RotationMatrix.Mx = ((scaledX - gVdp2RotationMatrix.m[0][0].asS32() * dx) -
        gVdp2RotationMatrix.m[0][1].asS32() * dy - gVdp2RotationMatrix.m[0][2].asS32() * dz) +
        coeff.m3C * -0x10000;

    s32 scaledY = MTH_Mul(pThis->m38_scale, pThis->mC_cameraPosition.m8_Z.asS32() << 4);
    gVdp2RotationMatrix.My = ((scaledY - gVdp2RotationMatrix.m[1][0].asS32() * dx) -
        gVdp2RotationMatrix.m[1][1].asS32() * dy - gVdp2RotationMatrix.m[1][2].asS32() * dz) +
        coeff.m3E * -0x10000;

    gVdp2RotationMatrix.Mz = ((((pThis->mC_cameraPosition.m4_Y.asS32() - pThis->m34_groundY) * 0x40 -
        gVdp2RotationMatrix.m[2][0].asS32() * dx) - gVdp2RotationMatrix.m[2][1].asS32() * dy) -
        gVdp2RotationMatrix.m[2][2].asS32() * dz) + coeff.m40 * -0x10000;
}

// 0606d584 — compute ground Y (same pattern as CARA)
static s32 seekComputeGroundY() {
    auto& coeff = gCoefficientTables[gRotationPassState.m0_planeIndex][vdp2Controls.m0_doubleBufferIndex];
    s32 iVar2 = gVdp2RotationMatrix.m[2][0].asS32() * coeff.m34;
    s32 iVar3 = gVdp2RotationMatrix.m[2][1].asS32() * coeff.m36;
    s32 iVar1 = MTH_Mul(gVdp2RotationMatrix.m[2][2], fixedPoint(coeff.m38 * 0x10000 - coeff.m8_Zst));
    return FP_Div(iVar2 + iVar3 + iVar1, gVdp2RotationMatrix.m[2][1]).getInteger();
}

// 0606d86c
static void seekBgMode0_Draw(sSeekVdp2Plane* pThis) {
    pThis->mC_cameraPosition = cameraProperties2.m0_position;
    pThis->m18_cameraRotation.m0_X = (s32)(s16)cameraProperties2.mC_rotation[0] << 16;
    pThis->m18_cameraRotation.m4_Y = (s32)(s16)cameraProperties2.mC_rotation[1] << 16;
    pThis->m18_cameraRotation.m8_Z = (s32)(s16)cameraProperties2.mC_rotation[2] << 16;
    getVdp1ClippingCoordinates(pThis->m24_vdp1Clipping);
    getVdp1ProjectionParams(&pThis->m30_projParam0, &pThis->m32_projParam1);

    fixedPoint projScale = intDivide((s32)pThis->m30_projParam0, (s32)pThis->m32_projParam1 << 16);
    beginRotationPass(0, projScale);
    seekBgDrawSub0(pThis);
    drawCinematicBar(6);
    commitRotationPass();

    pThis->m34_groundY = seekComputeGroundY();

    // Sky gradient update
    Unimplemented(); // FUN_060297f0 — sky color gradient based on camera height + ground Y

    pThis->m0_scrollX = (pThis->m18_cameraRotation.m4_Y.asS32() >> 12) * -0x400;
    pThis->m4_scrollY = (0x15A - pThis->m34_groundY) * 0x10000;

    projScale = intDivide((s32)pThis->m30_projParam0, (s32)pThis->m32_projParam1 << 16);
    beginRotationPass(1, projScale);

    auto& coeff = gCoefficientTables[gRotationPassState.m0_planeIndex][vdp2Controls.m0_doubleBufferIndex];
    coeff.m34 = (s16)((pThis->m24_vdp1Clipping[0] + pThis->m24_vdp1Clipping[2] + (pThis->m24_vdp1Clipping[0] + pThis->m24_vdp1Clipping[2] < 0 ? 1 : 0)) >> 1);
    coeff.m36 = (s16)((pThis->m24_vdp1Clipping[1] + pThis->m24_vdp1Clipping[3] + (pThis->m24_vdp1Clipping[1] + pThis->m24_vdp1Clipping[3] < 0 ? 1 : 0)) >> 1);
    coeff.m38 = pThis->m32_projParam1;
    coeff.m3C = coeff.m34;
    coeff.m3E = coeff.m36;
    coeff.m40 = 0;

    buildRotationMatrixRoll(-pThis->m18_cameraRotation.m8_Z.asS32());
    intDivide((s32)pThis->m24_vdp1Clipping[3] - (s32)pThis->m24_vdp1Clipping[1], 0xE00000);
    fixedPoint scaleVal = intDivide((s32)pThis->m24_vdp1Clipping[2] - (s32)pThis->m24_vdp1Clipping[0], 0x1600000);
    scaleRotationMatrix(scaleVal);
    setRotationScrollOffset(pThis->m0_scrollX, pThis->m4_scrollY);
    commitRotationPass();

    s_VDP2Regs* regs = vdp2Controls.m4_pendingVdp2Regs;
    regs->mC0_WPSX0 = pThis->m24_vdp1Clipping[0] << 1;
    regs->mC2_WPSY0 = pThis->m24_vdp1Clipping[1];
    regs->mC4_WPEX0 = pThis->m24_vdp1Clipping[2] << 1;
    regs->mC6_WPEY0 = pThis->m24_vdp1Clipping[3];
}

// 0606dd18 — indoor background init
static void seekBgMode1_Init(sSeekVdp2Plane* pThis) {
    reinitVdp2();
    initNBG1Layer();

    static const sLayerConfig rgb0Setup[] = {
        {m2_CHCN, 1}, {m5_CHSZ, 1}, {m6_PNB, 1}, {m7_CNSM, 0},
        {m27_RPMD, 2}, {m11_SCN, 8}, {m34_W0E, 1}, {m37_W0A, 1},
        {m44_CCEN, 1}, {m45_COEN, 0},
        {m0_END},
    };
    setupRGB0(rgb0Setup);

    static const sLayerConfig rotParamsSetup[] = { {m31_RxKTE, 1}, {m0_END} };
    setupRotationParams(rotParamsSetup);
    static const sLayerConfig rotParams2Setup[] = { {m0_END} };
    setupRotationParams2(rotParams2Setup);

    static const sLayerConfig nbg0Setup[] = {
        {m2_CHCN, 1}, {m5_CHSZ, 1}, {m6_PNB, 1}, {m7_CNSM, 0},
        {m23, 1},
        {m34_W0E, 1}, {m37_W0A, 1},
        {m0_END},
    };
    setupNBG0(nbg0Setup);

    asyncDmaCopy(gCurrentTownOverlay->getSaturnPtr(0x06081498), getVdp2Cram(0xA00), 0x200, 0);
    asyncDmaCopy(gCurrentTownOverlay->getSaturnPtr(0x06081698), vdp2Palette, 0x200, 0);

    loadFile("AJITOSCR.SCB", getVdp2Vram(0x40000), 0);
    loadFile("AJITOSCR.PNB", getVdp2Vram(0x60000), 0);
    loadFile("AJITSCN1.SCB", getVdp2Vram(0x10000), 0);
    loadFile("AJITSCN1.PNB", getVdp2Vram(0x1C000), 0);

    vdp2Controls.m4_pendingVdp2Regs->m9C_VCSTA = getVdp2Vram(0xFF80);

    pThis->m60 = 0xFFF54EFC;
    pThis->m68 = 0xB;

    vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL = (vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL & 0xFF00) | 0xB4;
    vdp2Controls.m4_pendingVdp2Regs->m10_CYCA0 = 0x0C134457;
    vdp2Controls.m_isDirty = 1;

    setupRotationMapPlanes(0, gCurrentTownOverlay->getSaturnPtr(0x06081898));
    setupRotationMapPlanes(1, gCurrentTownOverlay->getSaturnPtr(0x060818d8));
    initLayerMap(0, 0x1C000, 0x1C000, 0x1C000, 0x1C000);

    setupVdp2Table(6, coefficientA0, coefficientA1, getVdp2Vram(0x20000), 0x80);
    setupVdp2Table(7, coefficientB0, coefficientB1, getVdp2Vram(0x24000), 0x80);
    s_BTL_A3_Env_InitVdp2Sub3(5, getVdp2Vram(0x28000));
    *(u16*)getVdp2Vram(0x25000) = 0x700;

    vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA = (vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA & 0xFFF80000) | 0x12800;
    *(u16*)getVdp2Vram(0x25002) = 0x5E92;
    vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA = (vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA & 0xFFF80000) | 0x12801;

    vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL = (vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL & 0xFEFF) | 0x100;
    vdp2Controls.m4_pendingVdp2Regs->mF0_PRISA = 0x405;
    vdp2Controls.m4_pendingVdp2Regs->mF2_PRISB = 0x507;
    vdp2Controls.m4_pendingVdp2Regs->mF4_PRISC = 0x505;
    vdp2Controls.m4_pendingVdp2Regs->mF6_PRISD = 0x505;
    vdp2Controls.m4_pendingVdp2Regs->mF8_PRINA = 0x602;
    vdp2Controls.m4_pendingVdp2Regs->mFA_PRINB = 0x700;
    vdp2Controls.m4_pendingVdp2Regs->mFC_PRIR = 3;
    vdp2Controls.m_isDirty = 1;

    pThis->m3C = 0x40000;
    static const std::vector<std::array<s32, 2>> bgConfig = { {{0x2C, 1}} };
    applyLayerDisplayConfig(bgConfig);

    vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL = vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL & 0xFEFF;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xF8FF) | 0x400;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xFFF0) | 3;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xCFFF) | 0x1000;

    pThis->m50 = 0x0C;
    pThis->m51 = 0x0C;

    s_VDP2Regs* regs = vdp2Controls.m4_pendingVdp2Regs;
    regs->m100_CCRSA = (u16)pThis->m50 | ((u16)pThis->m51 << 8);
    regs->m102_CCRSB = 0;
    regs->m104_CCRSC = 0;
    regs->m106_CCRSD = 0;
    pThis->m55 = 0x10;
    regs->m10C_CCRR = (u16)pThis->m55;
    vdp2Controls.m_isDirty = 1;

    pThis->m4C = 0;
    pThis->m40 = 0x14;
    pThis->m44 = 0x2543ED1;
    pThis->m48 = 0xB6E;
}

// 0606e050 — indoor background update: fire/light flicker effect
static void seekBgMode1_Update(sSeekVdp2Plane* pThis) {
    pThis->m58 += pThis->m60;
    s32* flickerBuffer = (s32*)&pThis->m_pad[0x74 - 0x6C]; // offset 0x74 in work area
    for (int i = 0; i < 0x2D; i++) {
        u32 rng = randomNumber();
        flickerBuffer[i] = (s32)(rng >> 16) * pThis->m68 + pThis->m58;
    }
    asyncDmaCopy(flickerBuffer, (void*)getVdp2Vram(0x1FF00), 0xB4, 0);
}

// 0606e2c6 — indoor rotation pass 0 sub (different from mode 0 — uses fixed pitch/yaw)
static void seekBgMode1DrawSub0(sSeekVdp2Plane* pThis) {
    s32 rotZ = pThis->m18_cameraRotation.m8_Z.asS32();
    auto& coeff = gCoefficientTables[gRotationPassState.m0_planeIndex][vdp2Controls.m0_doubleBufferIndex];

    coeff.m34 = (s16)((pThis->m24_vdp1Clipping[0] + pThis->m24_vdp1Clipping[2] + (pThis->m24_vdp1Clipping[0] + pThis->m24_vdp1Clipping[2] < 0 ? 1 : 0)) >> 1);
    coeff.m36 = (s16)((pThis->m24_vdp1Clipping[1] + pThis->m24_vdp1Clipping[3] + (pThis->m24_vdp1Clipping[1] + pThis->m24_vdp1Clipping[3] < 0 ? 1 : 0)) >> 1);
    coeff.m38 = pThis->m32_projParam1;
    coeff.m3C = coeff.m34;
    coeff.m3E = coeff.m36;
    coeff.m40 = 0;

    buildRotationMatrixPitchYaw(0, 0x4000000); // fixed: pitch=0, yaw=90°
    scaleRotationMatrix(pThis->m3C);
    writeRotationParams(-rotZ);

    s32 dx = coeff.m34 - coeff.m3C;
    s32 dy = coeff.m36 - coeff.m3E;
    s32 dz = coeff.m38 - coeff.m40;

    s32 scaledX = MTH_Mul(pThis->m3C, pThis->mC_cameraPosition.m0_X.asS32() << 4);
    gVdp2RotationMatrix.Mx = ((scaledX - gVdp2RotationMatrix.m[0][0].asS32() * dx) -
        gVdp2RotationMatrix.m[0][1].asS32() * dy - gVdp2RotationMatrix.m[0][2].asS32() * dz) +
        coeff.m3C * -0x10000;

    s32 scaledY = MTH_Mul(pThis->m3C, pThis->mC_cameraPosition.m8_Z.asS32() << 4);
    gVdp2RotationMatrix.My = ((scaledY - gVdp2RotationMatrix.m[1][0].asS32() * dx) -
        gVdp2RotationMatrix.m[1][1].asS32() * dy - gVdp2RotationMatrix.m[1][2].asS32() * dz) +
        coeff.m3E * -0x10000;

    gVdp2RotationMatrix.Mz = ((((pThis->mC_cameraPosition.m4_Y.asS32() - pThis->m38_scale) * 0x10 -
        gVdp2RotationMatrix.m[2][0].asS32() * dx) - gVdp2RotationMatrix.m[2][1].asS32() * dy) -
        gVdp2RotationMatrix.m[2][2].asS32() * dz) + coeff.m40 * -0x10000;
}

// 0606dc8a — indoor compute ground Y
static s32 seekBgMode1ComputeGroundY() {
    auto& coeff = gCoefficientTables[gRotationPassState.m0_planeIndex][vdp2Controls.m0_doubleBufferIndex];
    s32 iVar2 = gVdp2RotationMatrix.m[2][0].asS32() * coeff.m34;
    s32 iVar3 = gVdp2RotationMatrix.m[2][1].asS32() * coeff.m36;
    s32 iVar1 = MTH_Mul(gVdp2RotationMatrix.m[2][2], fixedPoint(coeff.m38 * 0x10000 - coeff.m8_Zst));
    return FP_Div(iVar2 + iVar3 + iVar1, gVdp2RotationMatrix.m[2][1]).getInteger();
}

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

// 0606dc10 — apply sinusoidal distortion to rotation coefficient table
static void seekBgMode1DistortCoefficients(sSeekVdp2Plane* pThis) {
    std::vector<fixedPoint>* pTable = gVdp2CoefficientTables[gRotationPassState.m0_planeIndex][vdp2Controls.m0_doubleBufferIndex];
    if (!pTable) return;
    s32 phase = pThis->m4C;
    for (int i = 0; i < 0x1A8 && i < (int)pTable->size(); i++) {
        fixedPoint sinVal = getSin((u16)(phase >> 16) & 0xFFF);
        fixedPoint modulation = MTH_Mul(fixedPoint(pThis->m48), sinVal) + 0x10000;
        (*pTable)[i] = MTH_Mul((*pTable)[i], modulation);
        phase += pThis->m44;
    }
    pThis->m4C += pThis->m40;
}

// 0606e0bc — indoor background draw
static void seekBgMode1_Draw(sSeekVdp2Plane* pThis) {
    pThis->mC_cameraPosition.m0_X = pThis->mC_cameraPosition.m0_X + 0x1000;
    pThis->mC_cameraPosition.m4_Y = 0xA000;
    getVdp1ClippingCoordinates(pThis->m24_vdp1Clipping);
    getVdp1LocalCoordinates(pThis->m2C_vdp1LocalCoords);
    getVdp1ProjectionParams(&pThis->m30_projParam0, &pThis->m32_projParam1);

    fixedPoint projScale = intDivide((s32)pThis->m30_projParam0, (s32)pThis->m32_projParam1 << 16);
    beginRotationPass(0, projScale);
    seekBgMode1DrawSub0(pThis);
    drawCinematicBar(6);
    commitRotationPass();

    s32 groundY = seekBgMode1ComputeGroundY();
    pThis->m34_groundY = groundY;

    seekBgMode1DistortCoefficients(pThis);

    pThis->m0_scrollX = (pThis->m18_cameraRotation.m4_Y.asS32() >> 12) * -0x400;
    pThis->m4_scrollY = (0xFF - pThis->m34_groundY) * 0x10000;

    projScale = intDivide((s32)pThis->m30_projParam0, (s32)pThis->m32_projParam1 << 16);
    beginRotationPass(1, projScale);

    auto& coeff = gCoefficientTables[gRotationPassState.m0_planeIndex][vdp2Controls.m0_doubleBufferIndex];
    coeff.m34 = (s16)((pThis->m24_vdp1Clipping[0] + pThis->m24_vdp1Clipping[2] + (pThis->m24_vdp1Clipping[0] + pThis->m24_vdp1Clipping[2] < 0 ? 1 : 0)) >> 1);
    coeff.m36 = (s16)((pThis->m24_vdp1Clipping[1] + pThis->m24_vdp1Clipping[3] + (pThis->m24_vdp1Clipping[1] + pThis->m24_vdp1Clipping[3] < 0 ? 1 : 0)) >> 1);
    coeff.m38 = pThis->m32_projParam1;
    coeff.m3C = coeff.m34;
    coeff.m3E = coeff.m36;
    coeff.m40 = 0;

    buildRotationMatrixRoll(-pThis->m18_cameraRotation.m8_Z.asS32());
    intDivide((s32)pThis->m24_vdp1Clipping[3] - (s32)pThis->m24_vdp1Clipping[1], 0xE00000);
    fixedPoint scaleVal = intDivide((s32)pThis->m24_vdp1Clipping[2] - (s32)pThis->m24_vdp1Clipping[0], 0x1600000);
    scaleRotationMatrix(scaleVal);
    setRotationScrollOffset(pThis->m0_scrollX, pThis->m4_scrollY);
    commitRotationPass();

    s_VDP2Regs* regs = vdp2Controls.m4_pendingVdp2Regs;
    regs->mC0_WPSX0 = pThis->m24_vdp1Clipping[0] << 1;
    regs->mC2_WPSY0 = pThis->m24_vdp1Clipping[1];
    regs->mC4_WPEX0 = pThis->m24_vdp1Clipping[2] << 1;
    regs->mC6_WPEY0 = pThis->m24_vdp1Clipping[3];

    // NBG0 scroll based on ground Y
    s32 nbg0ScrollY;
    if (pThis->m34_groundY < -0x14) {
        nbg0ScrollY = 0x140000;
    } else {
        nbg0ScrollY = pThis->m34_groundY * -0x10000;
    }
    pauseEngine[4] = 0;
    setVdp2ScrollPosition(pThis->m0_scrollX, nbg0ScrollY - 0x10000);
    pauseEngine[4] = 4;
}

// 0600cddc
static s32 getNpcData5E() {
    return (s32)npcData0.m5E;
}

// 060725b4
static s32 updateWorldGridFromNpc0() {
    sNPC* pNPC = (sNPC*)npcData0.m70_npcPointerArray[0].workArea;
    updateWorldGrid(pNPC->mE8.m0_position.m0_X, pNPC->mE8.m0_position.m8_Z);
    return 0;
}

// 060707e8 — extended camera mode setup (reads 11 params from pointer, sets mode 1)
static s32 setupCameraModeFixedExtended(sSaturnPtr arg) {
    twnMainLogicTask->m100_deltaPosition = readSaturnVec3(arg);
    twnMainLogicTask->mE4_fixedPosition = readSaturnVec3(arg + 0xC);
    s32 val = readSaturnS32(arg + 0x18);
    twnMainLogicTask->mDC = val;
    twnMainLogicTask->mD8 = val;
    twnMainLogicTask->mF0[0] = readSaturnFP(arg + 0x24);
    twnMainLogicTask->mF0[1] = readSaturnFP(arg + 0x28);
    twnMainLogicTask->mF8[0] = readSaturnFP(arg + 0x1C);
    twnMainLogicTask->mF8[1] = readSaturnFP(arg + 0x20);
    twnMainLogicTask->m2_cameraFollowMode = sMainLogic::TrackingMode_Indoor;
    setupCameraUpdateForCurrentMode();
    return 0;
}

// 060724d4
static s32 setupColorOffset_Seek(s32 param_1) {
    if (g_fadeControls.m_4C <= g_fadeControls.m_4D) {
        vdp2Controls.m20_registers[0].m112_CLOFSL = param_1;
        vdp2Controls.m20_registers[1].m112_CLOFSL = param_1;
    }
    return 0;
}

// 06070768
static s32 setupNpcWalkForward_Seek() {
    setupCameraFollowMode();
    sNPC* pNPC = getNpcDataByIndex(0);
    pNPC->mE8.m30_stepTranslation = { 0, 0, fixedPoint(0xFFFFFF1D) };

    sMatrix4x3 mat;
    initMatrixToIdentity(&mat);
    rotateMatrixShiftedY(pNPC->mE8.mC_rotation.m4_Y, &mat);
    rotateMatrixShiftedX(pNPC->mE8.mC_rotation.m0_X, &mat);
    sVec3_FP transformed;
    transformVec(pNPC->mE8.m30_stepTranslation, transformed, mat);

    pNPC->mE8.m3C_targetPosition.m0_X = transformed.m0_X * 0x24 + pNPC->mE8.m0_position.m0_X;
    pNPC->mE8.m3C_targetPosition.m4_Y = transformed.m4_Y * 0x24 + pNPC->mE8.m0_position.m4_Y;
    pNPC->mE8.m3C_targetPosition.m8_Z = transformed.m8_Z * 0x24 + pNPC->mE8.m0_position.m8_Z;

    pNPC->mE8.m48_targetRotation = pNPC->mE8.mC_rotation;
    pNPC->mF = (pNPC->mF & 0xF9) | 1;
    pNPC->mC |= 4;
    return 0;
}

// 0607257e
static s32 setFarClipDistance(s32 farClip) {
    graphicEngineStatus.m405C.m14_farClipDistance = farClip;
    graphicEngineStatus.m405C.m38_oneOverFarClip = FP_Div(0x8000, farClip);
    graphicEngineStatus.m405C.m34_oneOverFarClip256 = graphicEngineStatus.m405C.m38_oneOverFarClip << 8;
    return 0;
}

static const char* listOfFilesToLoad[] = {
    "COMMON3.MCB",
    "COMMON3.CGB",
    "AJITOMP0.MCB",
    "AJITOMP0.CGB",
    "AJITOMP1.MCB",
    "AJITOMP1.CGB",
    "SEEKMP.MCB",
    "SEEKMP.CGB",
    "SEEKMP1.MCB",
    "SEEKMP1.CGB",
    "AZELMP.MCB",
    "AZELMP.CGB",
    "LIBRAMP.MCB",
    "LIBRAMP.CGB",
    "SKJKMP.MCB",
    "SKJKMP.CGB",
    "LODGEMP0.MCB",
    "LODGEMP0.CGB",
    "LODGEMP1.MCB",
    "LODGEMP1.CGB",
    "CHILDMP.MCB",
    "CHILDMP.CGB",
    "SICKMP.MCB",
    "SICKMP.CGB",
    "TAMAMP.MCB",
    "TAMAMP.CGB",
    "SOUKOMP.MCB",
    "SOUKOMP.CGB",
    "SOUKOMP1.MCB",
    "SOUKOMP1.CGB",
    "SOUKOMP2.MCB",
    "SOUKOMP2.CGB",
    "X_A_YR.MCB",
    "X_A_YR.CGB",
    "Z_A_VZ.MCB",
    "Z_A_VZ.CGB",
    "X_A_PT.MCB",
    "X_A_PT.CGB",
    "Z_A_GS.MCB",
    "Z_A_GS.CGB",
    "X_A_RB.MCB",
    "X_A_RB.CGB",
    "X_F_RB.MCB",
    "X_F_RB.CGB",
    "X_A_RG.MCB",
    "X_A_RG.CGB",
    "X_A_ZD.MCB",
    "X_A_ZD.CGB",
    "X_A_PR.MCB",
    "X_A_PR.CGB",
    "X_F_PR.MCB",
    "X_F_PR.CGB",
    "X_A_SM.MCB",
    "X_A_SM.CGB",
    "X_F_SM.MCB",
    "X_F_SM.CGB",
    "Z_A_LD.MCB",
    "Z_A_LD.CGB",
    "Z_A_SR.MCB",
    "Z_A_SR.CGB",
    "X_A_PT.MCB",
    "X_A_PT.CGB",
    nullptr
};

struct TWN_SEEK_data* gTWN_SEEK = NULL;
struct TWN_SEEK_data : public sTownOverlay
{
    static void makeCurrent()
    {
        if (gTWN_SEEK == NULL)
        {
            gTWN_SEEK = new TWN_SEEK_data();
        }
        gCurrentTownOverlay = gTWN_SEEK;
    }

    TWN_SEEK_data() : sTownOverlay("TWN_SEEK.PRG")
    {
        // Script functions
        overlayScriptFunctions.m_zeroArg[0x06070c80] = {&updateWorldGridFromEdgeTask, "updateWorldGridFromEdgeTask"};

        overlayScriptFunctions.m_oneArg[0x06075ed4] = {&TwnFadeIn, "TwnFadeIn"};
        overlayScriptFunctions.m_oneArg[0x06075f4c] = {&TwnFadeOut, "TwnFadeOut"};
        overlayScriptFunctions.m_oneArg[0x0606e5a4] = {&setupSeekerBackground, "setupSeekerBackground"};
        overlayScriptFunctions.m_oneArg[0x0607257e] = {&setFarClipDistance, "setFarClipDistance"};
        overlayScriptFunctions.m_oneArg[0x060724d4] = {&setupColorOffset_Seek, "setupColorOffset_Seek"};

        overlayScriptFunctions.m_zeroArg[0x06070768] = {&setupNpcWalkForward_Seek, "setupNpcWalkForward_Seek"};
        overlayScriptFunctions.m_zeroArg[0x0600cddc] = {&getNpcData5E, "getNpcData5E"};
        overlayScriptFunctions.m_zeroArg[0x060725b4] = {&updateWorldGridFromNpc0, "updateWorldGridFromNpc0"};

        overlayScriptFunctions.m_twoArg[0x06075c6c] = {&townCamera_setup, "townCamera_setup"};

        overlayScriptFunctions.m_oneArgPtr[0x060707e8] = {&setupCameraModeFixedExtended, "setupCameraModeFixedExtended"};

        overlayScriptFunctions.m_fourArg[0x060745f0] = {&setNpcLocation, "setNpcLocation"};
        overlayScriptFunctions.m_fourArg[0x0607461e] = {&setNpcOrientation, "setNpcOrientation"};

        // Parse town setup table from Saturn binary (17 entries at 0x060781b0)
        // Entries 0-1: scriptList at 0x06077e30, numScripts=12
        for (int i = 0; i < 2; i++) {
            sSaturnPtr ptrBase = getSaturnPtr(0x060781b0) + 0xC * i;
            mTownSetups.push_back(readTownSetup(ptrBase, 12));
        }
        // Entries 2-5: scriptList at 0x06077e60, numScripts=16
        for (int i = 2; i < 6; i++) {
            sSaturnPtr ptrBase = getSaturnPtr(0x060781b0) + 0xC * i;
            mTownSetups.push_back(readTownSetup(ptrBase, 16));
        }
        // Entry 6: scriptList at 0x06077ea0, numScripts=19
        {
            sSaturnPtr ptrBase = getSaturnPtr(0x060781b0) + 0xC * 6;
            mTownSetups.push_back(readTownSetup(ptrBase, 19));
        }
        // Entries 7-16: numScripts=20 (safe upper bound)
        for (int i = 7; i < 17; i++) {
            sSaturnPtr ptrBase = getSaturnPtr(0x060781b0) + 0xC * i;
            mTownSetups.push_back(readTownSetup(ptrBase, 20));
        }
    }

    sTownObject* createObjectTaskFromEA_siblingTaskWithEAArgWithCopy(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg) override
    {
        switch (definitionEA.m_offset) {
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

// 06054114
static void initCameraTimerFromDayNight_Seek(sCameraTask* pCamera) {
    if (mainGameState.bitField[0] & 1) {
        pCamera->m4_dayNightTimer = 0x1518;
    }
    else {
        pCamera->m4_dayNightTimer = 0;
    }
}

// 0605412c
static void seekerToggleDayNight() {
    initCameraTimerFromDayNight_Seek(cameraTaskPtr);
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
static void townOverlayDelete_TwnSeek(townDebugTask2Function* pThis)
{
    seekerToggleDayNight();
    freeRamResources(pThis);
    vdp1FreeLastAllocation(pThis);
    unloadFnt();
}

// 06054000
p_workArea overlayStart_TWN_SEEK(p_workArea pUntypedThis, u32 arg)
{
    gTWN_SEEK->makeCurrent();

    townDebugTask2Function* pThis = static_cast<townDebugTask2Function*>(pUntypedThis);

    pThis->m_DeleteMethod = &townOverlayDelete_TwnSeek;

    loadSoundBanks(-1, 0);
    loadSoundBanks(0x41, 0);

    playPCM(pThis, 100);

    loadFnt("EVTSEEK.FNT");

    graphicEngineStatus.m405C.m10_nearClipDistance = 0x1000;
    graphicEngineStatus.m405C.m30_oneOverNearClip = FP_Div(0x10000, graphicEngineStatus.m405C.m10_nearClipDistance);

    graphicEngineStatus.m405C.m14_farClipDistance = 0x100000;
    graphicEngineStatus.m405C.m38_oneOverFarClip = FP_Div(0x8000, graphicEngineStatus.m405C.m14_farClipDistance);
    graphicEngineStatus.m405C.m34_oneOverFarClip256 = graphicEngineStatus.m405C.m38_oneOverFarClip << 8;

    initDramAllocator(pThis, townBuffer, 0xB0000, listOfFilesToLoad);

    initVdp1Ram(pThis, 0x25C18800, 0x63800);

    registerNpcs(gTWN_SEEK->mTownSetups, gTWN_SEEK->getSaturnPtr(0x06054190), arg);

    startScriptTask(pThis);

    // Background task
    createSeekerVdp2Plane(pThis);

    // Edge task
    sEdgeTask* pEdgeTask = startEdgeTask(gTWN_SEEK->getSaturnPtr(0x06078290));
    npcData0.m160_pEdgePosition = &pEdgeTask->m84.m8_position;

    // Main logic
    startMainLogic(pThis);

    twnMainLogicTask->m14_EdgeTask = pEdgeTask;

    // Camera task
    startCameraTask(pThis);

    return pThis;
}
