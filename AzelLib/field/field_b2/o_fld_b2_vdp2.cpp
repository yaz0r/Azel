#include "PDS.h"
#include "o_fld_b2.h"
#include "field/field_a3/o_fld_a3.h"
#include "shared/vdp2PlaneTask.h"
#include "battle/BTL_A3/BTL_A3_map6.h"
#include "kernel/cinematicBarsTask.h"
#include "kernel/loadSavegameScreen.h"

void setupCinematicBarData(int param1, std::array<u32, 256>& dataArray, u32 vdpOffset, int numEntries);

// ============================================================
// Subfield 0 VDP2 task (desert)
// ============================================================

// 0605A610
static void b2Vdp0Init(sVdp2PlaneTask* pThis)
{
    getFieldTaskPtr()->m8_pSubFieldData->m350_fieldPaletteTask = pThis;
    reinitVdp2();
    initNBG1Layer();

    pThis->m78_auxBuffer = (u8*)allocateHeapForTask(pThis, 8);

    asyncDmaCopy(gFLD_B2->getSaturnPtr(0x0608abfc), getVdp2Cram(0x800), 0x200, 0);
    asyncDmaCopy(gFLD_B2->getSaturnPtr(0x0608adfc), getVdp2Cram(0xA00), 0x200, 0);
    asyncDmaCopy(gFLD_B2->getSaturnPtr(0x0608affc), vdp2Palette, 0x200, 0);
    asyncDmaCopy(gFLD_B2->getSaturnPtr(0x0608b1fc), getVdp2Cram(0), 0x80, 0);
    asyncDmaCopy(gFLD_B2->getSaturnPtr(0x0608b27c), getVdp2Cram(0x80), 0x20, 0);
    asyncDmaCopy(gFLD_B2->getSaturnPtr(0x0608b29c), getVdp2Cram(0xA0), 0x20, 0);

    static const sLayerConfig nbg0Setup[] = {
        {m2_CHCN, 1}, {m5_CHSZ, 1}, {m6_PNB, 1}, {m7_CNSM, 0},
        {m34_W0E, 1}, {m37_W0A, 1}, {m44_CCEN, 1},
        {m0_END},
    };
    setupNBG0(nbg0Setup);

    static const sLayerConfig rgbSetup[] = {
        {m2_CHCN, 1}, {m5_CHSZ, 1}, {m6_PNB, 1}, {m7_CNSM, 0},
        {m27_RPMD, 2}, {m11_SCN, 8},
        {m34_W0E, 1}, {m37_W0A, 1}, {m44_CCEN, 0}, {m45_COEN, 0},
        {m0_END},
    };
    setupRGB0(rgbSetup);

    static const sLayerConfig rotParams[] = {
        {m31_RxKTE, 1}, {m13, 1},
        {m0_END},
    };
    setupRotationParams(rotParams);

    static const sLayerConfig rotParams2[] = {
        {m0_END},
    };
    setupRotationParams2(rotParams2);

    loadFile("FNS_B2_1.SCB", getVdp2Vram(0x10000), 0);
    loadFile("FNS_B2_1.PNB", getVdp2Vram(0x1F000), 0);
    loadFile("FRS_B2_1.SCB", getVdp2Vram(0x40000), 0);
    loadFile("FRS_B2_1.PNB", getVdp2Vram(0x60000), 0);

    vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL = (vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL & 0xFF00) | 0xB4;
    vdp2Controls.m4_pendingVdp2Regs->m10_CYCA0 = 0x310F7544;
    vdp2Controls.m_isDirty = 1;

    initLayerMap(0, 0x25E1F000, 0x25E1F000, 0x25E1F000, 0x25E1F000);
    setupRotationMapPlanes(0, gFLD_B2->getSaturnPtr(0x0608b2bc));
    setupRotationMapPlanes(1, gFLD_B2->getSaturnPtr(0x0608b2fc));
    setupVdp2Table(6, coefficientA0, coefficientA1, getVdp2Vram(0x20000), 0x80);
    setupVdp2Table(7, coefficientB0, coefficientB1, getVdp2Vram(0x22000), 0x80);

    initRotationCoefficientTables(5, getVdp2Vram(0x2A000));
    setVdp2VramU16(0x25E2A400, 0x700);
    vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA = (vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA & 0xFFF80000) | 0x15200;
    setVdp2VramU16(0x25E2A600, 0x5E92);
    vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA = (vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA & 0xFFF80000) | 0x15300;

    vdp2Controls.m4_pendingVdp2Regs->mF0_PRISA = 0x304;
    vdp2Controls.m4_pendingVdp2Regs->mF2_PRISB = 0x407;
    vdp2Controls.m4_pendingVdp2Regs->mF4_PRISC = 0x505;
    vdp2Controls.m4_pendingVdp2Regs->mF6_PRISD = 0x505;
    vdp2Controls.m4_pendingVdp2Regs->mF8_PRINA = 0x605;
    vdp2Controls.m4_pendingVdp2Regs->mFA_PRINB = 0x700;
    vdp2Controls.m4_pendingVdp2Regs->mFC_PRIR = 2;
    vdp2Controls.m_isDirty = 1;

    pThis->m3C_scale = fixedPoint(0x6666);

    static const std::vector<std::array<s32, 2>> layerDisplay = {
        {m44_CCEN, 1},
    };
    applyLayerDisplayConfig(layerDisplay);

    vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL = vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL & 0xFEFF;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xF8FF) | 0x300;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xFFF0) | 3;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xCFFF) | 0x1000;

    pThis->m74_colorNBG = 0;
    pThis->m70_colorR = 0xD;
    pThis->m71_colorG = 0x18;

    auto* regs = vdp2Controls.m4_pendingVdp2Regs;
    regs->m100_CCRSA = (s16)pThis->m70_colorR | ((s16)pThis->m71_colorG << 8);
    regs->m102_CCRSB = 0;
    regs->m104_CCRSC = 0;
    regs->m106_CCRSD = 0;

    pThis->m4C_wavePhase = 0;
    pThis->m40_waveSpeed = 0xD5A6;
    pThis->m48 = 0xA3A;
    pThis->m44_waveFreq = 0x6E5D4D;

    regs->mB8_OVPNRA = 0x5002;
    regs->mBA_OVPNRB = 0x5002;

    s32* auxBuf = (s32*)pThis->m78_auxBuffer;
    auxBuf[0] = 0x27715;
    auxBuf[1] = 0;
}

// 0605A9A0
static void b2Vdp0Update(sVdp2PlaneTask* pThis)
{
    s32* auxBuf = (s32*)pThis->m78_auxBuffer;
    auxBuf[1] += auxBuf[0];

    vdp2Controls.m4_pendingVdp2Regs->m108_CCRNA = (vdp2Controls.m4_pendingVdp2Regs->m108_CCRNA & 0xFFE0) | (s16)pThis->m74_colorNBG;
    vdp2Controls.m_isDirty = 1;
}

// 0605A9D8
static void b2Vdp0Draw(sVdp2PlaneTask* pThis)
{
    pThis->mC_cameraPosition = cameraProperties2.m0_position;
    pThis->m18_cameraRotation = cameraProperties2.mC_rotation.toSVec3_FP();

    getVdp1ClippingCoordinates(pThis->m24_vdp1Clipping);
    getVdp1LocalCoordinates(pThis->m2C_vdp1LocalCoordinates);
    getVdp1ProjectionParams(&pThis->m30_vdp1ProjectionParam[0], &pThis->m30_vdp1ProjectionParam[1]);

    // Pass 0: ground plane
    beginRotationPass(0, intDivide(pThis->m30_vdp1ProjectionParam[0], fixedPoint::fromInteger(pThis->m30_vdp1ProjectionParam[1])));
    vdp2SetupRotationPass(pThis);
    drawCinematicBar(6);
    commitRotationPass();

    pThis->m34 = computeRotationScrollOffset();
    vdp2ApplyWaveDistortion(pThis);

    // Pass 1: sky plane
    pThis->m0_scrollX = (pThis->m18_cameraRotation.m4_Y.asS32() >> 12) * -0x400;
    pThis->m4_scrollY = (0xFF - pThis->m34) * 0x10000;

    beginRotationPass(1, intDivide(pThis->m30_vdp1ProjectionParam[0], fixedPoint::fromInteger(pThis->m30_vdp1ProjectionParam[1])));

    sCoefficientTableData& t = gCoefficientTables[gRotationPassState.m0_planeIndex][(s32)vdp2Controls.m0_doubleBufferIndex];
    s32 sumX = (s32)pThis->m24_vdp1Clipping[0] + (s32)pThis->m24_vdp1Clipping[2];
    t.m34 = (s16)((sumX + (int)(sumX < 0)) >> 1);
    s32 sumY = (s32)pThis->m24_vdp1Clipping[1] + (s32)pThis->m24_vdp1Clipping[3];
    t.m36 = (s16)((sumY + (int)(sumY < 0)) >> 1);
    t.m38 = pThis->m30_vdp1ProjectionParam[1];
    t.m3C = t.m34;
    t.m3E = t.m36;
    t.m40 = 0;

    buildRotationMatrixRoll(-pThis->m18_cameraRotation.m8_Z);
    fixedPoint scaleY = intDivide((s32)pThis->m24_vdp1Clipping[3] - (s32)pThis->m24_vdp1Clipping[1], fixedPoint(0xE00000));
    fixedPoint scaleX = intDivide((s32)pThis->m24_vdp1Clipping[2] - (s32)pThis->m24_vdp1Clipping[0], fixedPoint(0x1600000));
    scaleRotationMatrix(scaleX);
    setRotationScrollOffset(pThis->m0_scrollX, pThis->m4_scrollY);
    commitRotationPass();

    auto* regs = vdp2Controls.m4_pendingVdp2Regs;
    regs->mC0_WPSX0 = pThis->m24_vdp1Clipping[0] << 1;
    regs->mC2_WPSY0 = pThis->m24_vdp1Clipping[1];
    regs->mC4_WPEX0 = pThis->m24_vdp1Clipping[2] << 1;
    regs->mC6_WPEY0 = pThis->m24_vdp1Clipping[3];
    regs->mC8_WPSX1 = pThis->m24_vdp1Clipping[0] << 1;
    regs->mCA_WPSY1 = pThis->m24_vdp1Clipping[1];
    regs->mCC_WPEX1 = pThis->m24_vdp1Clipping[2] << 1;
    regs->mCE_WPEY1 = pThis->m24_vdp1Clipping[3];
}

void createB2Vdp2Task_0(p_workArea parent)
{
    static sVdp2PlaneTask::TypedTaskDefinition td = { &b2Vdp0Init, &b2Vdp0Update, &b2Vdp0Draw, nullptr };
    createSubTask<sVdp2PlaneTask>(parent, &td);
}

// ============================================================
// Subfield 1 VDP2 task (oasis — minimal, no rotation planes)
// ============================================================

// 0605A458
static void b2Vdp1Init(sVdp2PlaneTask* pThis)
{
    getFieldTaskPtr()->m8_pSubFieldData->m350_fieldPaletteTask = pThis;
    reinitVdp2();
    initNBG1Layer();

    vdp2Controls.m4_pendingVdp2Regs->m10_CYCA0 = 0x31FF75FF;
    vdp2Controls.m_isDirty = 1;

    asyncDmaCopy(gFLD_B2->getSaturnPtr(0x0608abcc), getVdp2Cram(0xA0), 0x20, 0);

    setVdp2VramU16(0x25E2A400, 0x700);
    vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA = (vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA & 0xFFF80000) | 0x15200;
    setVdp2VramU16(0x25E2A600, 0);
    vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA = (vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA & 0xFFF80000) | 0x15300;

    vdp2Controls.m4_pendingVdp2Regs->mF0_PRISA = 0x405;
    vdp2Controls.m4_pendingVdp2Regs->mF2_PRISB = 0x507;
    vdp2Controls.m4_pendingVdp2Regs->mF4_PRISC = 0x505;
    vdp2Controls.m4_pendingVdp2Regs->mF6_PRISD = 0x505;
    vdp2Controls.m4_pendingVdp2Regs->mF8_PRINA = 0x600;
    vdp2Controls.m4_pendingVdp2Regs->mFA_PRINB = 0x700;
    vdp2Controls.m4_pendingVdp2Regs->mFC_PRIR = 0;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xFFF0) | 3;
    vdp2Controls.m_isDirty = 1;
}

// 0605A516
static void b2Vdp1Update(sVdp2PlaneTask* pThis)
{
    // empty — confirmed from Ghidra
}

void createB2Vdp2Task_1(p_workArea parent)
{
    static sVdp2PlaneTask::TypedTaskDefinition td = { &b2Vdp1Init, &b2Vdp1Update, nullptr, nullptr };
    createSubTask<sVdp2PlaneTask>(parent, &td);
}

// ============================================================
// Subfield 2 VDP2 task (underground — single rotation pass)
// ============================================================

// 0605AF1C
static void b2Vdp2Init(sVdp2PlaneTask* pThis)
{
    getFieldTaskPtr()->m8_pSubFieldData->m350_fieldPaletteTask = pThis;
    reinitVdp2();
    initNBG1Layer();

    asyncDmaCopy(gFLD_B2->getSaturnPtr(0x0608b44c), getVdp2Cram(0xA00), 0x200, 0);

    static const sLayerConfig rgbSetup[] = {
        {m2_CHCN, 1}, {m5_CHSZ, 1}, {m6_PNB, 1}, {m7_CNSM, 0},
        {m27_RPMD, 2}, {m11_SCN, 8},
        {m34_W0E, 1}, {m37_W0A, 1},
        {m0_END},
    };
    setupRGB0(rgbSetup);

    static const sLayerConfig rotParams[] = {
        {m31_RxKTE, 1},
        {m0_END},
    };
    setupRotationParams(rotParams);

    loadFile("FRS_B2_4.SCB", getVdp2Vram(0x40000), 0);
    loadFile("FRS_B2_4.PNB", getVdp2Vram(0x60000), 0);

    vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL = (vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL & 0xFF00) | 0xB4;
    vdp2Controls.m4_pendingVdp2Regs->m10_CYCA0 = 0x31FF75FF;
    vdp2Controls.m_isDirty = 1;

    setupRotationMapPlanes(0, gFLD_B2->getSaturnPtr(0x0608b64c));
    setupVdp2Table(6, coefficientA0, coefficientA1, getVdp2Vram(0x20000), 0x80);

    initRotationCoefficientTables(4, getVdp2Vram(0x2A000));
    setVdp2VramU16(0x25E2A400, 0x700);
    vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA = (vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA & 0xFFF80000) | 0x15200;
    setVdp2VramU16(0x25E2A600, 0);
    vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA = (vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA & 0xFFF80000) | 0x15300;

    vdp2Controls.m4_pendingVdp2Regs->mF0_PRISA = 0x405;
    vdp2Controls.m4_pendingVdp2Regs->mF2_PRISB = 0x507;
    vdp2Controls.m4_pendingVdp2Regs->mF4_PRISC = 0x505;
    vdp2Controls.m4_pendingVdp2Regs->mF6_PRISD = 0x505;
    vdp2Controls.m4_pendingVdp2Regs->mF8_PRINA = 0x600;
    vdp2Controls.m4_pendingVdp2Regs->mFA_PRINB = 0x700;
    vdp2Controls.m4_pendingVdp2Regs->mFC_PRIR = 3;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xFFF0) | 3;
    vdp2Controls.m_isDirty = 1;

    pThis->m3C_scale = fixedPoint(0x20000);
    vdp2Controls.m4_pendingVdp2Regs->mB8_OVPNRA = 0x5002;
}

// 0605B04A
static void b2Vdp2Update(sVdp2PlaneTask* pThis)
{
    // empty — confirmed from Ghidra
}

// 0605B0EC
static void b2Vdp2Draw(sVdp2PlaneTask* pThis)
{
    pThis->mC_cameraPosition = cameraProperties2.m0_position;
    pThis->m18_cameraRotation = cameraProperties2.mC_rotation.toSVec3_FP();

    getVdp1ClippingCoordinates(pThis->m24_vdp1Clipping);
    getVdp1LocalCoordinates(pThis->m2C_vdp1LocalCoordinates);
    getVdp1ProjectionParams(&pThis->m30_vdp1ProjectionParam[0], &pThis->m30_vdp1ProjectionParam[1]);

    // Single rotation pass: ground plane
    beginRotationPass(0, intDivide(pThis->m30_vdp1ProjectionParam[0], fixedPoint::fromInteger(pThis->m30_vdp1ProjectionParam[1])));
    vdp2SetupRotationPass(pThis);
    drawCinematicBar(6);
    commitRotationPass();

    pThis->m34 = computeRotationScrollOffset();

    auto* regs = vdp2Controls.m4_pendingVdp2Regs;
    regs->mC0_WPSX0 = pThis->m24_vdp1Clipping[0] << 1;
    regs->mC2_WPSY0 = pThis->m24_vdp1Clipping[1];
    regs->mC4_WPEX0 = pThis->m24_vdp1Clipping[2] << 1;
    regs->mC6_WPEY0 = pThis->m24_vdp1Clipping[3];
    regs->mC8_WPSX1 = pThis->m24_vdp1Clipping[0] << 1;
    regs->mCA_WPSY1 = pThis->m24_vdp1Clipping[1];
    regs->mCC_WPEX1 = pThis->m24_vdp1Clipping[2] << 1;
    regs->mCE_WPEY1 = pThis->m24_vdp1Clipping[3];
}

void createB2Vdp2Task_2(p_workArea parent)
{
    static sVdp2PlaneTask::TypedTaskDefinition td = { &b2Vdp2Init, &b2Vdp2Update, &b2Vdp2Draw, nullptr };
    createSubTask<sVdp2PlaneTask>(parent, &td);
}

// ============================================================
// Subfield 3 VDP2 task (return desert — night, with line scroll)
// ============================================================

// 0605b3d0 — line scroll update for subfield 3 (uses shared updateLineScrollTable)

// 0605B724
static void b2Vdp3Init(sVdp2PlaneTask* pThis)
{
    getFieldTaskPtr()->m8_pSubFieldData->m350_fieldPaletteTask = pThis;
    reinitVdp2();
    initNBG1Layer();

    pThis->m78_auxBuffer = (u8*)allocateHeapForTask(pThis, 8);
    pThis->m50_lineScrollParams.m0_buffer = allocateHeapForTask(pThis, 0xC00);

    asyncDmaCopy(gFLD_B2->getSaturnPtr(0x0608b71c), getVdp2Cram(0x800), 0x200, 0);
    asyncDmaCopy(gFLD_B2->getSaturnPtr(0x0608b91c), getVdp2Cram(0xA00), 0x200, 0);
    asyncDmaCopy(gFLD_B2->getSaturnPtr(0x0608bb1c), vdp2Palette, 0x200, 0);
    asyncDmaCopy(gFLD_B2->getSaturnPtr(0x0608bd1c), getVdp2Cram(0), 0x80, 0);
    asyncDmaCopy(gFLD_B2->getSaturnPtr(0x0608bd9c), getVdp2Cram(0x80), 0x20, 0);
    asyncDmaCopy(gFLD_B2->getSaturnPtr(0x0608bdbc), getVdp2Cram(0xA0), 0x20, 0);
    asyncDmaCopy(gFLD_B2->getSaturnPtr(0x0608bddc), getVdp2Cram(0x200), 0x20, 0);

    static const sLayerConfig nbg0Setup[] = {
        {m2_CHCN, 1}, {m5_CHSZ, 1}, {m6_PNB, 1}, {m7_CNSM, 0},
        {m34_W0E, 1}, {m37_W0A, 1}, {m44_CCEN, 1},
        {m0_END},
    };
    setupNBG0(nbg0Setup);

    static const sLayerConfig rgbSetup[] = {
        {m2_CHCN, 1}, {m5_CHSZ, 1}, {m6_PNB, 1}, {m7_CNSM, 0},
        {m27_RPMD, 2}, {m11_SCN, 8},
        {m34_W0E, 1}, {m37_W0A, 1}, {m44_CCEN, 0}, {m45_COEN, 0},
        {m0_END},
    };
    setupRGB0(rgbSetup);

    static const sLayerConfig rotParams[] = {
        {m31_RxKTE, 1}, {m13, 1},
        {m0_END},
    };
    setupRotationParams(rotParams);

    static const sLayerConfig rotParams2[] = {
        {m0_END},
    };
    setupRotationParams2(rotParams2);

    loadFile("FNS_A7_1.SCB", getVdp2Vram(0x10000), 0);
    loadFile("FNS_A7_1.PNB", getVdp2Vram(0x1F000), 0);
    loadFile("FRS_A7_1.SCB", getVdp2Vram(0x40000), 0);
    loadFile("FRS_A7_1.PNB", getVdp2Vram(0x60000), 0);

    vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL = (vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL & 0xFF00) | 0xB4;
    vdp2Controls.m4_pendingVdp2Regs->m10_CYCA0 = 0x310F7544;
    vdp2Controls.m_isDirty = 1;

    initLayerMap(0, 0x25E1F000, 0x25E1F000, 0x25E1F000, 0x25E1F000);
    setupRotationMapPlanes(0, gFLD_B2->getSaturnPtr(0x0608bdfc));
    setupRotationMapPlanes(1, gFLD_B2->getSaturnPtr(0x0608be3c));
    setupVdp2Table(6, coefficientA0, coefficientA1, getVdp2Vram(0x20000), 0x80);
    setupVdp2Table(7, coefficientB0, coefficientB1, getVdp2Vram(0x22000), 0x80);

    initRotationCoefficientTables(5, getVdp2Vram(0x2A000));
    setupCinematicBarData(1, *(std::array<u32, 256>*)pThis->m50_lineScrollParams.m0_buffer, 0x25E24000, 0xC0);

    setVdp2VramU16(0x25E2A400, 0x700);
    vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA = (vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA & 0xFFF80000) | 0x15200;
    setVdp2VramU16(0x25E2A600, 0x5E92);
    vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA = (vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA & 0xFFF80000) | 0x15300;

    vdp2Controls.m4_pendingVdp2Regs->mF0_PRISA = 0x304;
    vdp2Controls.m4_pendingVdp2Regs->mF2_PRISB = 0x407;
    vdp2Controls.m4_pendingVdp2Regs->mF4_PRISC = 0x505;
    vdp2Controls.m4_pendingVdp2Regs->mF6_PRISD = 0x505;
    vdp2Controls.m4_pendingVdp2Regs->mF8_PRINA = 0x605;
    vdp2Controls.m4_pendingVdp2Regs->mFA_PRINB = 0x700;
    vdp2Controls.m4_pendingVdp2Regs->mFC_PRIR = 2;
    vdp2Controls.m_isDirty = 1;

    pThis->m50_lineScrollParams.m18_phaseAccum = 0;
    pThis->m50_lineScrollParams.m4_phaseSpeed = 0x2D81F;
    pThis->m50_lineScrollParams.m8_freqPerLine = 0x9ABCD0;
    pThis->m50_lineScrollParams.mC_zoomAmplitude = 0xF58;
    pThis->m50_lineScrollParams.m10_scrollBaseSpeed = 0;
    pThis->m50_lineScrollParams.m14_scrollIncPerLine = 0xB6628;
    pThis->m3C_scale = fixedPoint(0x6666);

    static const std::vector<std::array<s32, 2>> layerDisplay = {
        {m44_CCEN, 1},
    };
    applyLayerDisplayConfig(layerDisplay);

    vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL = vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL & 0xFEFF;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xF8FF) | 0x300;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xFFF0) | 3;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xCFFF) | 0x1000;

    pThis->m74_colorNBG = 0;
    pThis->m70_colorR = 0xD;
    pThis->m71_colorG = 0x18;

    auto* regs = vdp2Controls.m4_pendingVdp2Regs;
    regs->m100_CCRSA = (s16)pThis->m70_colorR | ((s16)pThis->m71_colorG << 8);
    regs->m102_CCRSB = 0;
    regs->m104_CCRSC = 0;
    regs->m106_CCRSD = 0;

    pThis->m4C_wavePhase = 0;
    pThis->m40_waveSpeed = 0xD5A6;
    pThis->m48 = 0xA3A;
    pThis->m44_waveFreq = 0x6E5D4D;

    regs->mB8_OVPNRA = 0x5002;
    regs->mBA_OVPNRB = 0x5002;

    s32* auxBuf = (s32*)pThis->m78_auxBuffer;
    auxBuf[0] = 0x27715;
    auxBuf[1] = 0;
}

// 0605BB9C
static void b2Vdp3Update(sVdp2PlaneTask* pThis)
{
    s32* auxBuf = (s32*)pThis->m78_auxBuffer;
    auxBuf[1] += auxBuf[0];

    vdp2Controls.m4_pendingVdp2Regs->m108_CCRNA = (vdp2Controls.m4_pendingVdp2Regs->m108_CCRNA & 0xFFE0) | (s16)pThis->m74_colorNBG;
    vdp2Controls.m_isDirty = 1;

    vdp2Controls.m20_registers[0].m108_CCRNA = (vdp2Controls.m4_pendingVdp2Regs->m108_CCRNA & 0xFFE0) | (s16)pThis->m74_colorNBG;
    vdp2Controls.m20_registers[1].m108_CCRNA = vdp2Controls.m20_registers[0].m108_CCRNA;

    updateLineScrollTable(pThis);
    drawCinematicBar(1);
}

// 0605BC28
static void b2Vdp3Draw(sVdp2PlaneTask* pThis)
{
    pThis->mC_cameraPosition = cameraProperties2.m0_position;
    pThis->m18_cameraRotation = cameraProperties2.mC_rotation.toSVec3_FP();

    getVdp1ClippingCoordinates(pThis->m24_vdp1Clipping);
    getVdp1LocalCoordinates(pThis->m2C_vdp1LocalCoordinates);
    getVdp1ProjectionParams(&pThis->m30_vdp1ProjectionParam[0], &pThis->m30_vdp1ProjectionParam[1]);

    // Pass 0: ground plane
    beginRotationPass(0, intDivide(pThis->m30_vdp1ProjectionParam[0], fixedPoint::fromInteger(pThis->m30_vdp1ProjectionParam[1])));
    vdp2SetupRotationPass(pThis);
    drawCinematicBar(6);
    commitRotationPass();

    pThis->m34 = computeRotationScrollOffset();
    vdp2ApplyWaveDistortion(pThis);

    // Pass 1: sky plane
    pThis->m0_scrollX = (pThis->m18_cameraRotation.m4_Y.asS32() >> 12) * -0x400;
    pThis->m4_scrollY = (0xFF - pThis->m34) * 0x10000;

    beginRotationPass(1, intDivide(pThis->m30_vdp1ProjectionParam[0], fixedPoint::fromInteger(pThis->m30_vdp1ProjectionParam[1])));

    sCoefficientTableData& t = gCoefficientTables[gRotationPassState.m0_planeIndex][(s32)vdp2Controls.m0_doubleBufferIndex];
    s32 sumX = (s32)pThis->m24_vdp1Clipping[0] + (s32)pThis->m24_vdp1Clipping[2];
    t.m34 = (s16)((sumX + (int)(sumX < 0)) >> 1);
    s32 sumY = (s32)pThis->m24_vdp1Clipping[1] + (s32)pThis->m24_vdp1Clipping[3];
    t.m36 = (s16)((sumY + (int)(sumY < 0)) >> 1);
    t.m38 = pThis->m30_vdp1ProjectionParam[1];
    t.m3C = t.m34;
    t.m3E = t.m36;
    t.m40 = 0;

    buildRotationMatrixRoll(-pThis->m18_cameraRotation.m8_Z);
    fixedPoint scaleY = intDivide((s32)pThis->m24_vdp1Clipping[3] - (s32)pThis->m24_vdp1Clipping[1], fixedPoint(0xE00000));
    fixedPoint scaleX = intDivide((s32)pThis->m24_vdp1Clipping[2] - (s32)pThis->m24_vdp1Clipping[0], fixedPoint(0x1600000));
    scaleRotationMatrix(scaleX);
    setRotationScrollOffset(pThis->m0_scrollX, pThis->m4_scrollY);
    commitRotationPass();

    // NBG0 scroll
    gCurrentVDP2ScrollLayer = 0;
    setupVDP2CoordinatesIncrement2(pThis->m0_scrollX, 0);
    gCurrentVDP2ScrollLayer = 4;

    auto* regs = vdp2Controls.m4_pendingVdp2Regs;
    regs->mC0_WPSX0 = pThis->m24_vdp1Clipping[0] << 1;
    regs->mC2_WPSY0 = pThis->m24_vdp1Clipping[1];
    regs->mC4_WPEX0 = pThis->m24_vdp1Clipping[2] << 1;
    regs->mC6_WPEY0 = pThis->m24_vdp1Clipping[3];
    regs->mC8_WPSX1 = pThis->m24_vdp1Clipping[0] << 1;
    regs->mCA_WPSY1 = pThis->m24_vdp1Clipping[1];
    regs->mCC_WPEX1 = pThis->m24_vdp1Clipping[2] << 1;
    regs->mCE_WPEY1 = pThis->m24_vdp1Clipping[3];
}

void createB2Vdp2Task_3(p_workArea parent)
{
    static sVdp2PlaneTask::TypedTaskDefinition td = { &b2Vdp3Init, &b2Vdp3Update, &b2Vdp3Draw, nullptr };
    createSubTask<sVdp2PlaneTask>(parent, &td);
}
