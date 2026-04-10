#include "PDS.h"
#include "o_fld_a7.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldRadar.h"
#include "field/fieldVisibilityGrid.h"
#include "battle/BTL_A3/BTL_A3_map6.h"
#include "trigo.h"
#include "kernel/cinematicBarsTask.h"
#include "kernel/loadSavegameScreen.h"

void setupCinematicBarData(int param1, std::array<u32, 256>& dataArray, u32 vdpOffset, int numEntries);

// VDP2 background task for field A7 — uses shared sVdp2PlaneTask from field.h

// 06057528 — VDP2 init for subfield 0
static void initVdp2_A7_0(sVdp2PlaneTask* pThis)
{
    getFieldTaskPtr()->m8_pSubFieldData->m350_fieldPaletteTask = pThis;
    reinitVdp2();
    initNBG1Layer();
    pThis->m50_lineScrollBuffer = allocateHeapForTask(pThis, 0xc00);

    asyncDmaCopy(gFLD_A7->getSaturnPtr(0x060848f4), getVdp2Cram(0x800), 0x20, 0);
    asyncDmaCopy(gFLD_A7->getSaturnPtr(0x06084914), getVdp2Cram(0x820), 0x20, 0);
    asyncDmaCopy(gFLD_A7->getSaturnPtr(0x06084934), getVdp2Cram(0xa00), 0x200, 0);
    asyncDmaCopy(gFLD_A7->getSaturnPtr(0x06084b34), vdp2Palette, 0x200, 0);
    asyncDmaCopy(gFLD_A7->getSaturnPtr(0x06084d34), getVdp2Cram(0x000), 0x80, 0);
    asyncDmaCopy(gFLD_A7->getSaturnPtr(0x06084db4), getVdp2Cram(0x080), 0x80, 0);
    asyncDmaCopy(gFLD_A7->getSaturnPtr(0x06084e14), getVdp2Cram(0x100), 0x80, 0);
    asyncDmaCopy(gFLD_A7->getSaturnPtr(0x06084e94), getVdp2Cram(0x180), 0x80, 0);

    static const sLayerConfig nbg0Setup[] = {
        {m2_CHCN, 0},
        {m5_CHSZ, 1},
        {m6_PNB, 1},
        {m7_CNSM, 0},
        {m34_W0E, 1},
        {m37_W0A, 1},
        {m44_CCEN, 1},
        {m10_SPN, 4},
        {m22_N0LSCX, 1},
        {m21_LCSY, 1},
        {m20_N0LSS, 1},
        {m0_END},
    };
    setupNBG0(nbg0Setup);

    static const sLayerConfig rgbSetup[] = {
        {m2_CHCN, 1},
        {m5_CHSZ, 1},
        {m6_PNB, 1},
        {m7_CNSM, 0},
        {m27_RPMD, 2},
        {m11_SCN, 8},
        {m34_W0E, 1},
        {m37_W0A, 1},
        {m44_CCEN, 1},
        {m45_COEN, 0},
        {m0_END},
    };
    setupRGB0(rgbSetup);

    static const sLayerConfig rotParams[] = { {m31_RxKTE, 1}, {m0_END} };
    setupRotationParams(rotParams);

    static const sLayerConfig rotParams2[] = { {m0_END} };
    setupRotationParams2(rotParams2);

    loadFile("FNS_A7_0.SCB", getVdp2Vram(0x10000), 0);
    loadFile("FNS_A7_0.PNB", getVdp2Vram(0x1e000), 0);
    loadFile("FRS_A7_0.SCB", getVdp2Vram(0x40000), 0);
    loadFile("FRS_A7_0.PNB", getVdp2Vram(0x60000), 0);

    initLayerMap(0, 0x25e1e000, 0x25e1e000, 0x25e1e000, 0x25e1e000);

    vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL = (vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL & 0xff00) | 0xb4;
    vdp2Controls.m4_pendingVdp2Regs->m10_CYCA0 = 0x310f754f;
    vdp2Controls.m_isDirty = 1;

    static const std::array<u32, 16> rotPlanesA = { 0x25E60000, 0x25E60000, 0x25E60000, 0x25E60000, 0x25E60000, 0x25E60000, 0x25E60000, 0x25E60000, 0x25E60000, 0x25E60000, 0x25E60000, 0x25E60000, 0x25E60000, 0x25E60000, 0x25E60000, 0x25E60000 };
    static const std::array<u32, 16> rotPlanesB = { 0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800 };
    setupRotationMapPlanes(0, rotPlanesA);
    setupRotationMapPlanes(1, rotPlanesB);

    setupVdp2Table(6, coefficientA0, coefficientA1, getVdp2Vram(0x20000), 0x80);
    setupVdp2Table(7, coefficientB0, coefficientB1, getVdp2Vram(0x22000), 0x80);
    initRotationCoefficientTables(5, getVdp2Vram(0x2a000));
    setupCinematicBarData(1, *(std::array<u32, 256>*)pThis->m50_lineScrollBuffer, 0x25e24000, 0xc0);

    setVdp2VramU16(0x25e2a400, 0x700);
    vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA = (vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA & 0xFFF80000) | 0x15200;

    setVdp2VramU16(0x25e2a600, 0xdef4);
    vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA = (vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA & 0xFFF80000) | 0x15300;

    vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL = vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL & 0xFEFF;

    vdp2Controls.m4_pendingVdp2Regs->mF0_PRISA = 0x405;
    vdp2Controls.m4_pendingVdp2Regs->mF2_PRISB = 0x507;
    vdp2Controls.m4_pendingVdp2Regs->mF4_PRISC = 0x505;
    vdp2Controls.m4_pendingVdp2Regs->mF6_PRISD = 0x505;
    vdp2Controls.m4_pendingVdp2Regs->mF8_PRINA = 0x604;
    vdp2Controls.m4_pendingVdp2Regs->mFA_PRINB = 0x700;
    vdp2Controls.m4_pendingVdp2Regs->mFC_PRIR = 3;
    vdp2Controls.m_isDirty = 1;

    pThis->m3C_scale = fixedPoint(0x10000);

    static const std::vector<std::array<s32, 2>> layerDisplay = {
        {m44_CCEN, 1},
    };
    applyLayerDisplayConfig(layerDisplay);

    vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL = vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL & 0xFEFF; // Clear CCMD (use color calculation register value)

    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xF8FF) | 0x400;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xFFF0) | 3;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xCFFF) | 0x1000;

    pThis->m74_colorNBG = 0;
    pThis->m70_colorR = 0x0D;
    pThis->m71_colorG = 0x18;

    auto* regs = vdp2Controls.m4_pendingVdp2Regs;
    regs->m100_CCRSA = (s16)pThis->m70_colorR | ((s16)pThis->m71_colorG << 8);
    regs->m102_CCRSB = 0;
    regs->m104_CCRSC = 0;
    regs->m106_CCRSD = 0;
    vdp2Controls.m_isDirty = 1;

    // Wave distortion params
    pThis->m4C_wavePhase = 0;
    pThis->m40_waveSpeed = 0x5c390;
    pThis->m44_waveFreq = (s32)0xfe147bb8;
    pThis->m48_waveAmplitude = (s32)0xfffff333;

    // Line scroll params
    pThis->m54_lsPhaseSpeed = 0x1ef27;
    pThis->m58_lsFreqPerLine = (s32)0xfffa4fcf;
    pThis->m5C_lsZoomAmplitude = 0x7fec;
    pThis->m60_lsScrollBaseSpeed = 0x20000;
    pThis->m64_lsScrollIncPerLine = (s32)0xfff8d7d0;

    // Initialize line scroll buffer with parabolic zoom
    s32* buf = (s32*)pThis->m50_lineScrollBuffer;
    s32 lineIdx = -0x70;
    s32 scrollAccum = 0;
    for (s32 i = 0; i < 224; i++)
    {
        buf[i * 3 + 1] = scrollAccum;
        s32 zoom = FP_Div(0x3100, fixedPoint(lineIdx * lineIdx * 0x8000)).asS32() + 0x10000;
        buf[i * 3 + 2] = zoom;
        buf[i * 3 + 0] = (0x10000 - zoom) * 0xB0;
        lineIdx++;
        scrollAccum += 0x10000;
    }
}

// 06014274 — update line scroll table with wave distortion
static void updateLineScrollTable(sVdp2PlaneTask* pThis)
{
    s32 phase = pThis->m68_lsPhaseAccum;
    pThis->m68_lsPhaseAccum = pThis->m54_lsPhaseSpeed + phase;
    s32 scrollBase = pThis->m6C_lsScrollBaseAccum + pThis->m60_lsScrollBaseSpeed;
    pThis->m6C_lsScrollBaseAccum = scrollBase;

    s32 amplitude = pThis->m5C_lsZoomAmplitude;
    s32 freq = pThis->m58_lsFreqPerLine;
    s32 scrollInc = pThis->m64_lsScrollIncPerLine;
    s32* buf = (s32*)pThis->m50_lineScrollBuffer;

    // Each line has 3 entries: {position, scroll, zoomCoeff}
    // Total lines: 0xDE (222) in loop + 2 post-loop = 224
    u32 phaseIdx = (u32)phase;

    for (s32 i = 0; i < 0xDE + 2; i++)
    {
        phaseIdx += (u32)freq;

        // Look up sine value and compute zoom coefficient
        fixedPoint sinVal = getSin((phaseIdx >> 16) & 0xFFF);
        s32 divisor = amplitude + 0x10000 + MTH_Mul(fixedPoint(amplitude), sinVal).asS32();
        s32 quotient = (s32)(0x100000000LL / (s64)divisor);

        // Update per-line scroll (accumulated, masked to 24 bits)
        buf[i * 3 + 1] = (buf[i * 3 + 1] + scrollInc) & 0xFFFFFF;

        // Zoom coefficient
        buf[i * 3 + 2] = quotient;

        // Horizontal position from zoom
        buf[i * 3 + 0] = (0x10000 - quotient) * 0xB0 + scrollBase;
    }
}

// 0605798c
static void updateVdp2_A7_0(sVdp2PlaneTask* pThis)
{
    vdp2Controls.m20_registers[0].m108_CCRNA = (vdp2Controls.m4_pendingVdp2Regs->m108_CCRNA & 0xFFE0) | (u16)(u8)pThis->m74_colorNBG;
    vdp2Controls.m20_registers[1].m108_CCRNA = vdp2Controls.m20_registers[0].m108_CCRNA;
    updateLineScrollTable(pThis);
    drawCinematicBar(1);
}

// 06057a00
static void drawVdp2_A7_0(sVdp2PlaneTask* pThis)
{
    pThis->mC_cameraPosition = cameraProperties2.m0_position;
    pThis->m18_cameraRotation.m0_X = (s32)(s16)cameraProperties2.mC_rotation[0] << 16;
    pThis->m18_cameraRotation.m4_Y = (s32)(s16)cameraProperties2.mC_rotation[1] << 16;
    pThis->m18_cameraRotation.m8_Z = (s32)(s16)cameraProperties2.mC_rotation[2] << 16;
    getVdp1ClippingCoordinates(pThis->m24_vdp1Clipping);
    getVdp1LocalCoordinates(pThis->m2C_localCoordinates);
    getVdp1ProjectionParams(&pThis->m30_projParam0, &pThis->m32_projParam1);

    fixedPoint projScale = intDivide((s32)pThis->m30_projParam0, (s32)pThis->m32_projParam1 << 16);
    beginRotationPass(0, projScale);
    vdp2SetupRotationPass(pThis);
    drawCinematicBar(6);
    commitRotationPass();

    pThis->m34_scrollValue = computeRotationScrollOffset();
    vdp2ApplyWaveDistortion(pThis);

    pThis->m0_scrollX = (pThis->m18_cameraRotation.m4_Y.asS32() >> 12) * -0x400;
    pThis->m4_scrollY = (0xFE - pThis->m34_scrollValue) * 0x10000;

    projScale = intDivide((s32)pThis->m30_projParam0, (s32)pThis->m32_projParam1 << 16);
    beginRotationPass(1, projScale);

    auto& coeff = gCoefficientTables[gRotationPassState.m0_planeIndex][(s32)vdp2Controls.m0_doubleBufferIndex];

    s32 pxSum = (s32)pThis->m24_vdp1Clipping[0] + (s32)pThis->m24_vdp1Clipping[2];
    coeff.m34 = (s16)((pxSum + (pxSum < 0 ? 1 : 0)) >> 1);
    s32 pySum = (s32)pThis->m24_vdp1Clipping[1] + (s32)pThis->m24_vdp1Clipping[3];
    coeff.m36 = (s16)((pySum + (pySum < 0 ? 1 : 0)) >> 1);
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

    gCurrentVDP2ScrollLayer = 0;
    setupVDP2CoordinatesIncrement2(pThis->m0_scrollX, 0);
    s_VDP2Regs* pRegs = vdp2Controls.m4_pendingVdp2Regs;
    gCurrentVDP2ScrollLayer = 4;
    pRegs->mC0_WPSX0 = pThis->m24_vdp1Clipping[0] << 1;
    pRegs->mC2_WPSY0 = pThis->m24_vdp1Clipping[1];
    pRegs->mC4_WPEX0 = pThis->m24_vdp1Clipping[2] << 1;
    pRegs->mC6_WPEY0 = pThis->m24_vdp1Clipping[3];
    pRegs->mC8_WPSX1 = pThis->m24_vdp1Clipping[0] << 1;
    pRegs->mCA_WPSY1 = pThis->m24_vdp1Clipping[1];
    pRegs->mCC_WPEX1 = pThis->m24_vdp1Clipping[2] << 1;
    pRegs->mCE_WPEY1 = pThis->m24_vdp1Clipping[3];
}

// 06057dd4 — create subfield 0 VDP2 task
void createA7_0_envTask(p_workArea parent)
{
    static sVdp2PlaneTask::TypedTaskDefinition td = { &initVdp2_A7_0, &updateVdp2_A7_0, &drawVdp2_A7_0, nullptr };
    createSubTask<sVdp2PlaneTask>(parent, &td);
}

// 06057f04 — VDP2 init for subfield 1
static void initVdp2_A7_1(sVdp2PlaneTask* pThis)
{
    getFieldTaskPtr()->m8_pSubFieldData->m350_fieldPaletteTask = pThis;
    reinitVdp2();
    initNBG1Layer();
    pThis->m50_lineScrollBuffer = allocateHeapForTask(pThis, 0xc00);
    pThis->m78_skyRotationBuffer = (s32*)allocateHeapForTask(pThis, 8);

    asyncDmaCopy(gFLD_A7->getSaturnPtr(0x060854d0), getVdp2Cram(0x200), 0x20, 0);
    asyncDmaCopy(gFLD_A7->getSaturnPtr(0x06085090), getVdp2Cram(0x080), 0x20, 0);
    asyncDmaCopy(gFLD_A7->getSaturnPtr(0x060850b0), getVdp2Cram(0x0a0), 0x20, 0);
    asyncDmaCopy(gFLD_A7->getSaturnPtr(0x060850d0), getVdp2Cram(0xa00), 0x200, 0);
    asyncDmaCopy(gFLD_A7->getSaturnPtr(0x060852d0), vdp2Palette, 0x200, 0);

    static const sLayerConfig nbg0Setup[] = {
        {m2_CHCN, 0}, {m5_CHSZ, 1}, {m6_PNB, 1}, {m7_CNSM, 0},
        {m34_W0E, 1}, {m37_W0A, 1}, {m44_CCEN, 1}, {m10_SPN, 1},
        {m22_N0LSCX, 1}, {m21_LCSY, 1}, {m20_N0LSS, 1},
        {m0_END},
    };
    setupNBG0(nbg0Setup);

    static const sLayerConfig rgbSetup[] = {
        {m2_CHCN, 1}, {m5_CHSZ, 1}, {m6_PNB, 1}, {m7_CNSM, 0},
        {m27_RPMD, 2}, {m11_SCN, 8}, {m34_W0E, 1}, {m37_W0A, 1},
        {m44_CCEN, 1}, {m45_COEN, 0},
        {m0_END},
    };
    setupRGB0(rgbSetup);

    static const sLayerConfig rotParams[] = { {m31_RxKTE, 1}, {eVdp2LayerConfig(0x0D), 1}, {m0_END} };
    setupRotationParams(rotParams);

    static const sLayerConfig rotParams2[] = { {m0_END} };
    setupRotationParams2(rotParams2);

    loadFile("FNS_A7_1.SCB", getVdp2Vram(0x10000), 0);
    loadFile("FNS_A7_1.PNB", getVdp2Vram(0x1e000), 0);
    loadFile("FRS_A7_1.SCB", getVdp2Vram(0x40000), 0);
    loadFile("FRS_A7_1.PNB", getVdp2Vram(0x60000), 0);
    loadFile("SPACE.PNB", getVdp2Vram(0x64000), 0);

    initLayerMap(0, 0x25e1e000, 0x25e1e000, 0x25e1e000, 0x25e1e000);

    vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL = (vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL & 0xff00) | 0xb4;
    vdp2Controls.m4_pendingVdp2Regs->m10_CYCA0 = 0x310f754f;
    vdp2Controls.m_isDirty = 1;

    static const std::array<u32, 16> rotPlanesA = { 0x25E60000, 0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800 };
    static const std::array<u32, 16> rotPlanesB = { 0x25E61000, 0x25E61000, 0x25E61000, 0x25E61000, 0x25E64000, 0x25E64000, 0x25E64000, 0x25E64000, 0x25E64000, 0x25E64000, 0x25E64000, 0x25E64000, 0x25E64000, 0x25E64000, 0x25E64000, 0x25E64000 };
    setupRotationMapPlanes(0, rotPlanesA);
    setupRotationMapPlanes(1, rotPlanesB);

    setupVdp2Table(6, coefficientA0, coefficientA1, getVdp2Vram(0x20000), 0x80);
    setupVdp2Table(7, coefficientB0, coefficientB1, getVdp2Vram(0x22000), 0x80);
    initRotationCoefficientTables(5, getVdp2Vram(0x2a000));
    setupCinematicBarData(1, *(std::array<u32, 256>*)pThis->m50_lineScrollBuffer, 0x25e24000, 0xc0);

    setVdp2VramU16(0x25e2a400, 0x700);
    vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA = (vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA & 0xFFF80000) | 0x15200;

    setVdp2VramU16(0x25e2a600, 0xb4e2);
    vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA = (vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA & 0xFFF80000) | 0x15300;

    vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL = vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL & 0xFEFF;

    vdp2Controls.m4_pendingVdp2Regs->mF0_PRISA = 0x404;
    vdp2Controls.m4_pendingVdp2Regs->mF2_PRISB = 0x407;
    vdp2Controls.m4_pendingVdp2Regs->mF4_PRISC = 0x404;
    vdp2Controls.m4_pendingVdp2Regs->mF6_PRISD = 0x404;
    vdp2Controls.m4_pendingVdp2Regs->mF8_PRINA = 0x605;
    vdp2Controls.m4_pendingVdp2Regs->mFA_PRINB = 0x700;
    vdp2Controls.m4_pendingVdp2Regs->mFC_PRIR = 3;
    vdp2Controls.m_isDirty = 1;

    pThis->m3C_scale = fixedPoint(0x6666);

    static const std::vector<std::array<s32, 2>> layerDisplay = {
        {m44_CCEN, 1},
    };
    applyLayerDisplayConfig(layerDisplay);

    vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL = vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL & 0xFEFF;

    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xF8FF) | 0x400;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xFFF0) | 3;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xCFFF) | 0x1000;

    pThis->m74_colorNBG = 0x17;
    pThis->m70_colorR = 0x0D;
    pThis->m71_colorG = 0x18;

    auto* regs = vdp2Controls.m4_pendingVdp2Regs;
    regs->m100_CCRSA = (s16)pThis->m70_colorR | ((s16)pThis->m71_colorG << 8);
    regs->m102_CCRSB = 0;
    regs->m104_CCRSC = 0;
    regs->m106_CCRSD = 0;
    vdp2Controls.m_isDirty = 1;

    pThis->m4C_wavePhase = 0;
    pThis->m40_waveSpeed = 0xd5a6;
    pThis->m48_waveAmplitude = 0xa3a;
    pThis->m44_waveFreq = 0x6e5d4d;
    pThis->m68_lsPhaseAccum = 0;
    pThis->m54_lsPhaseSpeed = 0x2d81f;
    pThis->m58_lsFreqPerLine = 0x9abcd0;
    pThis->m5C_lsZoomAmplitude = 0xf58;
    pThis->m60_lsScrollBaseSpeed = 0;
    pThis->m64_lsScrollIncPerLine = 0xb6628;

    // Initialize line scroll table
    {
        s32* pLineScroll = (s32*)pThis->m50_lineScrollBuffer;
        s32 scrollAccum = 0;
        s32 lineY = -0x70;
        for (s32 i = 0; scrollAccum < 0xe00000; i += 0xC)
        {
            pLineScroll[i / 4 + 1] = scrollAccum;
            pLineScroll[i / 4 + 2] = intDivide(0x3100, lineY * lineY * 0x8000) + 0x10000;
            lineY++;
            scrollAccum += 0x10000;
            pLineScroll[i / 4] = (0x10000 - pLineScroll[i / 4 + 2]) * 0xB0;

            pLineScroll[i / 4 + 4] = scrollAccum;
            pLineScroll[i / 4 + 5] = intDivide(0x3100, lineY * lineY * 0x8000) + 0x10000;
            lineY++;
            scrollAccum += 0x10000;
            pLineScroll[i / 4 + 3] = (0x10000 - pLineScroll[i / 4 + 5]) * 0xB0;
        }
    }

    vdp2Controls.m4_pendingVdp2Regs->mB8_OVPNRA = 0x5002;
    vdp2Controls.m4_pendingVdp2Regs->mBA_OVPNRB = 0x5002;

    pThis->m78_skyRotationBuffer[0] = 0x1a5bbe;
    pThis->m78_skyRotationBuffer[1] = 0;
}

// 06058392
static void updateVdp2_A7_1(sVdp2PlaneTask* pThis)
{
    // Advance sky rotation
    pThis->m78_skyRotationBuffer[1] += pThis->m78_skyRotationBuffer[0];

    vdp2Controls.m20_registers[0].m108_CCRNA = (vdp2Controls.m4_pendingVdp2Regs->m108_CCRNA & 0xFFE0) | (u16)(u8)pThis->m74_colorNBG;
    vdp2Controls.m20_registers[1].m108_CCRNA = vdp2Controls.m20_registers[0].m108_CCRNA;
    updateLineScrollTable(pThis);
    drawCinematicBar(1);
}

// 060587f4 — set up rotation scroll for pass 0 (subfield 1 with sky rotation)
static void drawVdp2Sub_setupRotationPass0_A7_1(sVdp2PlaneTask* pThis)
{
    auto& coeff = gCoefficientTables[gRotationPassState.m0_planeIndex][(s32)vdp2Controls.m0_doubleBufferIndex];

    fixedPoint camRotX = pThis->m18_cameraRotation.m0_X;
    if (camRotX == 0)
    {
        camRotX = fixedPoint(-0xB60B6); // -0xB60B6
    }
    s32 skyRotation = pThis->m78_skyRotationBuffer[1];
    fixedPoint camRotY = pThis->m18_cameraRotation.m4_Y;
    fixedPoint camRotZ = pThis->m18_cameraRotation.m8_Z;

    // Offset camera position
    pThis->mC_cameraPosition.m0_X = fixedPoint(pThis->mC_cameraPosition.m0_X.asS32() + -0x380000);
    pThis->mC_cameraPosition.m8_Z = fixedPoint(pThis->mC_cameraPosition.m8_Z.asS32() + 0x880000);

    // Rotate camera position around center (0x280000, 0x280000)
    s32 dx = pThis->mC_cameraPosition.m0_X.asS32() + -0x280000;
    s32 dz = pThis->mC_cameraPosition.m8_Z.asS32() + -0x280000;
    s16 angle = (s16)((u32)skyRotation >> 16);
    fixedPoint cosA = getCos(angle & 0xFFF);
    fixedPoint sinA = getSin(angle & 0xFFF);
    pThis->mC_cameraPosition.m0_X = fixedPoint(MTH_Mul(fixedPoint(dx), cosA).asS32() + MTH_Mul(fixedPoint(dz), sinA).asS32() + 0x280000);
    pThis->mC_cameraPosition.m8_Z = fixedPoint(MTH_Mul(fixedPoint(dz), cosA).asS32() - MTH_Mul(fixedPoint(dx), sinA).asS32() + 0x280000);

    // Set coefficient table params
    s32 pxSum = (s32)pThis->m24_vdp1Clipping[0] + (s32)pThis->m24_vdp1Clipping[2];
    coeff.m34 = (s16)((pxSum + (pxSum < 0 ? 1 : 0)) >> 1);
    s32 pySum = (s32)pThis->m24_vdp1Clipping[1] + (s32)pThis->m24_vdp1Clipping[3];
    coeff.m36 = (s16)((pySum + (pySum < 0 ? 1 : 0)) >> 1);
    coeff.m38 = pThis->m32_projParam1;
    coeff.m3C = coeff.m34;
    coeff.m3E = coeff.m36;
    coeff.m40 = 0;

    buildRotationMatrixPitchYaw(fixedPoint(-0x4000000) - camRotX, -(camRotY + fixedPoint(skyRotation)));
    scaleRotationMatrix(pThis->m3C_scale);
    writeRotationParams(-camRotZ);

    // Compute Mx, My, Mz
    s32 dPx = coeff.m34 - coeff.m3C;
    s32 dPy = coeff.m36 - coeff.m3E;
    s32 dPz = coeff.m38 - coeff.m40;

    s32 scaledX = MTH_Mul(pThis->m3C_scale, fixedPoint(pThis->mC_cameraPosition.m0_X.asS32() << 4)).asS32();
    gVdp2RotationMatrix.Mx = fixedPoint(scaledX
        - gVdp2RotationMatrix.m[0][0].asS32() * dPx
        - gVdp2RotationMatrix.m[0][1].asS32() * dPy
        - gVdp2RotationMatrix.m[0][2].asS32() * dPz
        + coeff.m3C * -0x10000);

    s32 scaledY = MTH_Mul(pThis->m3C_scale, fixedPoint(pThis->mC_cameraPosition.m8_Z.asS32() << 4)).asS32();
    gVdp2RotationMatrix.My = fixedPoint(scaledY
        - gVdp2RotationMatrix.m[1][0].asS32() * dPx
        - gVdp2RotationMatrix.m[1][1].asS32() * dPy
        - gVdp2RotationMatrix.m[1][2].asS32() * dPz
        + coeff.m3E * -0x10000);

    s32 scaledZ = (pThis->mC_cameraPosition.m4_Y.asS32() - pThis->m38_groundY) * 0x10;
    gVdp2RotationMatrix.Mz = fixedPoint(scaledZ
        - gVdp2RotationMatrix.m[2][0].asS32() * dPx
        - gVdp2RotationMatrix.m[2][1].asS32() * dPy
        - gVdp2RotationMatrix.m[2][2].asS32() * dPz
        + coeff.m40 * -0x10000);
}

void drawGaugeVdp1(u16 mode, std::array<sVec2_S16, 4>& params, u16 color, fixedPoint depth);

// 0607653c — compute horizon Y position from X coordinate
static s32 horizonScreenY(sCoefficientTableData& coeff, s32 screenHeight, s32 xPos, s32 scrollValue)
{
    s32 numerator = ((scrollValue + 100) * 0x10000 - (coeff.m54 & 0x7FFFFFFF)) - coeff.m5C * xPos;
    return screenHeight - FP_Div(numerator, fixedPoint(coeff.m58)).getInteger();
}

// 060764f8 — compute horizon X position from Y coordinate
static s32 horizonScreenX(sCoefficientTableData& coeff, s32 yPos, s32 scrollValue)
{
    s32 numerator = ((scrollValue + 100) * 0x10000 - (coeff.m54 & 0x7FFFFFFF)) - coeff.m58 * yPos;
    return FP_Div(numerator, fixedPoint(coeff.m5C)).getInteger() - 0xB0;
}

// 06076584 — draw horizon gauge line on VDP1
static void drawHorizonGauge_A7(s32 scrollValue, u16 color)
{
    s32 screenHeight = (VDP2Regs_.m4_TVSTAT & 1) == 0 ? 0x70 : 0x80;

    s8 coeffIdx = (s8)(gRotationPassState.m0_planeIndex * 2 + (s32)vdp2Controls.m0_doubleBufferIndex);
    sCoefficientTableData& coeff = gCoefficientTables[0][coeffIdx];

    s32 adjustedScroll = scrollValue;
    if ((VDP2Regs_.m4_TVSTAT & 1) != 0)
    {
        adjustedScroll = scrollValue + 0x10;
    }

    s32 maxDist = screenHeight + 8;

    // Try to find horizon at left edge (X = -8)
    s32 leftY = horizonScreenY(coeff, screenHeight, -8, adjustedScroll);
    s32 absLeftY = leftY < 0 ? -leftY : leftY;

    std::array<sVec2_S16, 4> gaugeQuad;
    u32 pointsNeeded = (u32)(maxDist < absLeftY);

    if (maxDist >= absLeftY)
    {
        gaugeQuad[2][0] = -0xB8;
        gaugeQuad[0][0] = -0xB8;
        gaugeQuad[0][1] = (s16)leftY + 2;
        gaugeQuad[2][1] = (s16)leftY - 2;
    }

    // Try to find horizon at right edge (X = 0x168)
    s32 rightY = horizonScreenY(coeff, screenHeight, 0x168, adjustedScroll);
    s32 absRightY = rightY < 0 ? -rightY : rightY;

    u32 remaining = pointsNeeded;
    if (absRightY <= maxDist)
    {
        remaining = pointsNeeded - 1;
        gaugeQuad[pointsNeeded * 2 + 4 > 7 ? 3 : pointsNeeded] = { 0xB8, (s16)(rightY - 2) };
        gaugeQuad[(1 - pointsNeeded)] = { 0xB8, (s16)(rightY + 2) };
    }

    // Try to find horizon at top edge (Y = -8)
    if ((s32)remaining >= 0)
    {
        s32 topX = horizonScreenX(coeff, -8, adjustedScroll);
        s32 absTopX = topX < 0 ? -topX : topX;
        if (absTopX < 0xB9)
        {
            s16 edgeY = (s16)screenHeight + 8;
            gaugeQuad[(1 - remaining)] = { (s16)(topX - 2), edgeY };
            gaugeQuad[remaining == 0 ? 2 : 3] = { (s16)(topX + 2), edgeY };
            remaining--;
        }
    }

    // Try to find horizon at bottom edge (Y = screenHeight*2 + 8)
    if ((s32)remaining >= 0)
    {
        s32 botX = horizonScreenX(coeff, screenHeight * 2 + 8, adjustedScroll);
        s32 absBotX = botX < 0 ? -botX : botX;
        if (absBotX < 0xB9)
        {
            s16 edgeY = -8 - (s16)screenHeight;
            gaugeQuad[(1 - remaining)] = { (s16)(botX - 2), edgeY };
            gaugeQuad[remaining == 0 ? 2 : 3] = { (s16)(botX + 2), edgeY };
            remaining--;
        }
    }

    // If all 4 corners found, draw the gauge
    if ((s32)remaining < 0)
    {
        drawGaugeVdp1(0xC0, gaugeQuad, color, graphicEngineStatus.m405C.m14_farClipDistance - 1);
    }
}

// 06058410
static void drawVdp2_A7_1(sVdp2PlaneTask* pThis)
{
    pThis->mC_cameraPosition = cameraProperties2.m0_position;
    pThis->m18_cameraRotation.m0_X = (s32)(s16)cameraProperties2.mC_rotation[0] << 16;
    pThis->m18_cameraRotation.m4_Y = (s32)(s16)cameraProperties2.mC_rotation[1] << 16;
    pThis->m18_cameraRotation.m8_Z = (s32)(s16)cameraProperties2.mC_rotation[2] << 16;
    getVdp1ClippingCoordinates(pThis->m24_vdp1Clipping);
    getVdp1LocalCoordinates(pThis->m2C_localCoordinates);
    getVdp1ProjectionParams(&pThis->m30_projParam0, &pThis->m32_projParam1);

    fixedPoint projScale = intDivide((s32)pThis->m30_projParam0, (s32)pThis->m32_projParam1 << 16);
    beginRotationPass(0, projScale);
    drawVdp2Sub_setupRotationPass0_A7_1(pThis);
    drawCinematicBar(6);
    commitRotationPass();

    pThis->m34_scrollValue = computeRotationScrollOffset();
    drawHorizonGauge_A7(pThis->m34_scrollValue, 0x9020);
    vdp2ApplyWaveDistortion(pThis);

    pThis->m0_scrollX = (pThis->m18_cameraRotation.m4_Y.asS32() >> 12) * -0x400;
    pThis->m4_scrollY = (0xFF - pThis->m34_scrollValue) * 0x10000;

    projScale = intDivide((s32)pThis->m30_projParam0, (s32)pThis->m32_projParam1 << 16);
    beginRotationPass(1, projScale);

    auto& coeff = gCoefficientTables[gRotationPassState.m0_planeIndex][(s32)vdp2Controls.m0_doubleBufferIndex];

    s32 pxSum = (s32)pThis->m24_vdp1Clipping[0] + (s32)pThis->m24_vdp1Clipping[2];
    coeff.m34 = (s16)((pxSum + (pxSum < 0 ? 1 : 0)) >> 1);
    s32 pySum = (s32)pThis->m24_vdp1Clipping[1] + (s32)pThis->m24_vdp1Clipping[3];
    coeff.m36 = (s16)((pySum + (pySum < 0 ? 1 : 0)) >> 1);
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

    gCurrentVDP2ScrollLayer = 0;
    setupVDP2CoordinatesIncrement2(pThis->m0_scrollX, 0);
    s_VDP2Regs* pRegs = vdp2Controls.m4_pendingVdp2Regs;
    gCurrentVDP2ScrollLayer = 4;
    pRegs->mC0_WPSX0 = pThis->m24_vdp1Clipping[0] << 1;
    pRegs->mC2_WPSY0 = pThis->m24_vdp1Clipping[1];
    pRegs->mC4_WPEX0 = pThis->m24_vdp1Clipping[2] << 1;
    pRegs->mC6_WPEY0 = pThis->m24_vdp1Clipping[3];
    pRegs->mC8_WPSX1 = pThis->m24_vdp1Clipping[0] << 1;
    pRegs->mCA_WPSY1 = pThis->m24_vdp1Clipping[1];
    pRegs->mCC_WPEX1 = pThis->m24_vdp1Clipping[2] << 1;
    pRegs->mCE_WPEY1 = pThis->m24_vdp1Clipping[3];
}

// 06058a74 — create subfield 1 VDP2 task
void createA7_1_envTask(p_workArea parent)
{
    static sVdp2PlaneTask::TypedTaskDefinition td = { &initVdp2_A7_1, &updateVdp2_A7_1, &drawVdp2_A7_1, nullptr };
    createSubTask<sVdp2PlaneTask>(parent, &td);
}

// 06058b90 — VDP2 init for subfield 2
static void initVdp2_A7_2(sVdp2PlaneTask* pThis)
{
    getFieldTaskPtr()->m8_pSubFieldData->m350_fieldPaletteTask = pThis;
    reinitVdp2();
    initNBG1Layer();
    pThis->m50_lineScrollBuffer = allocateHeapForTask(pThis, 0xc00);

    asyncDmaCopy(gFLD_A7->getSaturnPtr(0x060856a0), getVdp2Cram(0xa00), 0x20, 0);
    asyncDmaCopy(gFLD_A7->getSaturnPtr(0x060856c0), vdp2Palette, 0x200, 0);
    asyncDmaCopy(gFLD_A7->getSaturnPtr(0x060858c0), getVdp2Cram(0x2a0), 0x20, 0);
    asyncDmaCopy(gFLD_A7->getSaturnPtr(0x060858e0), getVdp2Cram(0x140), 0x20, 0);

    static const sLayerConfig nbg0Setup[] = {
        {m1_TPEN, 1}, {m2_CHCN, 0}, {m5_CHSZ, 1}, {m6_PNB, 1}, {m7_CNSM, 0},
        {m34_W0E, 1}, {m37_W0A, 1}, {m44_CCEN, 1}, {m10_SPN, 5},
        {m22_N0LSCX, 1}, {m21_LCSY, 1}, {m20_N0LSS, 1},
        {m0_END},
    };
    setupNBG0(nbg0Setup);

    static const sLayerConfig rgbSetup[] = {
        {m2_CHCN, 1}, {m5_CHSZ, 1}, {m6_PNB, 1}, {m7_CNSM, 0},
        {m27_RPMD, 2}, {m11_SCN, 8}, {m34_W0E, 1}, {m37_W0A, 1},
        {m44_CCEN, 1}, {m45_COEN, 0},
        {m0_END},
    };
    setupRGB0(rgbSetup);

    static const sLayerConfig rotParams[] = { {m31_RxKTE, 1}, {m0_END} };
    setupRotationParams(rotParams);

    static const sLayerConfig rotParams2[] = { {m0_END} };
    setupRotationParams2(rotParams2);

    loadFile("FNS_A7_2.SCB", getVdp2Vram(0x10000), 0);
    loadFile("FNS_A7_2.PNB", getVdp2Vram(0x1e000), 0);
    loadFile("FRS_A7_2.SCB", getVdp2Vram(0x40000), 0);
    loadFile("FRS_A7_2.PNB", getVdp2Vram(0x60000), 0);

    initLayerMap(0, 0x25e1e000, 0x25e1e000, 0x25e1e000, 0x25e1e000);

    vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL = (vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL & 0xff00) | 0xb4;
    vdp2Controls.m4_pendingVdp2Regs->m10_CYCA0 = 0x310f754f;
    vdp2Controls.m_isDirty = 1;

    static const std::array<u32, 16> rotPlanesA = { 0x25E60000, 0x25E60000, 0x25E60000, 0x25E60000, 0x25E60000, 0x25E60000, 0x25E60000, 0x25E60000, 0x25E60000, 0x25E60000, 0x25E60000, 0x25E60000, 0x25E60000, 0x25E60000, 0x25E60000, 0x25E60000 };
    static const std::array<u32, 16> rotPlanesB = { 0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800 };
    setupRotationMapPlanes(0, rotPlanesA);
    setupRotationMapPlanes(1, rotPlanesB);

    setupVdp2Table(6, coefficientA0, coefficientA1, getVdp2Vram(0x20000), 0x80);
    setupVdp2Table(7, coefficientB0, coefficientB1, getVdp2Vram(0x22000), 0x80);
    initRotationCoefficientTables(5, getVdp2Vram(0x2a000));
    setupCinematicBarData(1, *(std::array<u32, 256>*)pThis->m50_lineScrollBuffer, 0x25e24000, 0xc0);

    setVdp2VramU16(0x25e2a400, 0x700);
    vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA = (vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA & 0xFFF80000) | 0x15200;

    setVdp2VramU16(0x25e2a600, 0x5e92);
    vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA = (vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA & 0xFFF80000) | 0x15300;

    vdp2Controls.m4_pendingVdp2Regs->mF0_PRISA = 0x405;
    vdp2Controls.m4_pendingVdp2Regs->mF2_PRISB = 0x507;
    vdp2Controls.m4_pendingVdp2Regs->mF4_PRISC = 0x505;
    vdp2Controls.m4_pendingVdp2Regs->mF6_PRISD = 0x505;
    vdp2Controls.m4_pendingVdp2Regs->mF8_PRINA = 0x605;
    vdp2Controls.m4_pendingVdp2Regs->mFA_PRINB = 0x700;
    vdp2Controls.m4_pendingVdp2Regs->mFC_PRIR = 3;
    vdp2Controls.m_isDirty = 1;

    pThis->m3C_scale = fixedPoint(0x10000);

    static const std::vector<std::array<s32, 2>> layerDisplay = {
        {m44_CCEN, 1},
    };
    applyLayerDisplayConfig(layerDisplay);

    vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL = vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL & 0xFEFF | 0x100; // CCMD to 1, don't do color calculation

    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xF8FF) | 0x400;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xFFF0) | 3;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xCFFF) | 0x1000;

    pThis->m74_colorNBG = 0x17;
    pThis->m70_colorR = 0x0D;
    pThis->m71_colorG = 0x18;

    auto* regs = vdp2Controls.m4_pendingVdp2Regs;
    regs->m100_CCRSA = (s16)pThis->m70_colorR | ((s16)pThis->m71_colorG << 8);
    regs->m102_CCRSB = 0;
    regs->m104_CCRSC = 0;
    regs->m106_CCRSD = 0;
    vdp2Controls.m_isDirty = 1;

    pThis->m4C_wavePhase = 0;
    pThis->m40_waveSpeed = 0xd5a6;
    pThis->m48_waveAmplitude = 0xa3a;
    pThis->m44_waveFreq = 0x6e5d4d;
    pThis->m54_lsPhaseSpeed = 0x1e572;
    pThis->m58_lsFreqPerLine = 0xda764;
}

// 06058fc2
static void updateVdp2_A7_2(sVdp2PlaneTask* pThis)
{
    vdp2Controls.m20_registers[0].m108_CCRNA = (vdp2Controls.m4_pendingVdp2Regs->m108_CCRNA & 0xFFE0) | (u16)(u8)pThis->m74_colorNBG;
    vdp2Controls.m20_registers[1].m108_CCRNA = vdp2Controls.m20_registers[0].m108_CCRNA;
    updateLineScrollTable(pThis);
    drawCinematicBar(1);
}

// 0605923e — set up rotation scroll for pass 0 (subfield 2, simplified)
static void drawVdp2Sub_setupRotationPass0_A7_2(sVdp2PlaneTask* pThis)
{
    auto& coeff = gCoefficientTables[gRotationPassState.m0_planeIndex][(s32)vdp2Controls.m0_doubleBufferIndex];

    fixedPoint camRotX = pThis->m18_cameraRotation.m0_X;
    if (camRotX == 0)
    {
        camRotX = fixedPoint(-0xB60B6); // -0xB60B6
    }
    fixedPoint camRotY = pThis->m18_cameraRotation.m4_Y;
    fixedPoint camRotZ = pThis->m18_cameraRotation.m8_Z;

    s32 pxSum = (s32)pThis->m24_vdp1Clipping[0] + (s32)pThis->m24_vdp1Clipping[2];
    coeff.m34 = (s16)((pxSum + (pxSum < 0 ? 1 : 0)) >> 1);
    s32 pySum = (s32)pThis->m24_vdp1Clipping[1] + (s32)pThis->m24_vdp1Clipping[3];
    coeff.m36 = (s16)((pySum + (pySum < 0 ? 1 : 0)) >> 1);
    coeff.m38 = pThis->m32_projParam1;
    coeff.m3C = coeff.m34;
    coeff.m3E = coeff.m36;
    coeff.m40 = 0;

    buildRotationMatrixPitchYaw(fixedPoint(-0x4000000) - camRotX, -camRotY);
    scaleRotationMatrix(pThis->m3C_scale);
    writeRotationParams(-camRotZ);

    // Compute Mx, My, Mz
    s32 dPx = coeff.m34 - coeff.m3C;
    s32 dPy = coeff.m36 - coeff.m3E;
    s32 dPz = coeff.m38 - coeff.m40;

    s32 scaledX = MTH_Mul(pThis->m3C_scale, fixedPoint(pThis->mC_cameraPosition.m0_X.asS32() << 4)).asS32();
    gVdp2RotationMatrix.Mx = fixedPoint(scaledX
        - gVdp2RotationMatrix.m[0][0].asS32() * dPx
        - gVdp2RotationMatrix.m[0][1].asS32() * dPy
        - gVdp2RotationMatrix.m[0][2].asS32() * dPz
        + coeff.m3C * -0x10000);

    s32 scaledY = MTH_Mul(pThis->m3C_scale, fixedPoint(pThis->mC_cameraPosition.m8_Z.asS32() << 4)).asS32();
    gVdp2RotationMatrix.My = fixedPoint(scaledY
        - gVdp2RotationMatrix.m[1][0].asS32() * dPx
        - gVdp2RotationMatrix.m[1][1].asS32() * dPy
        - gVdp2RotationMatrix.m[1][2].asS32() * dPz
        + coeff.m3E * -0x10000);

    s32 scaledZ = (pThis->mC_cameraPosition.m4_Y.asS32() - pThis->m38_groundY) * 0x10;
    gVdp2RotationMatrix.Mz = fixedPoint(scaledZ
        - gVdp2RotationMatrix.m[2][0].asS32() * dPx
        - gVdp2RotationMatrix.m[2][1].asS32() * dPy
        - gVdp2RotationMatrix.m[2][2].asS32() * dPz
        + coeff.m40 * -0x10000);
}

// 06058b02 — simplified scroll offset for A7 subfield 2 (depth term only, no pivot)
static s32 computeGroundY_A7_2()
{
    sCoefficientTableData& t = gCoefficientTables[gRotationPassState.m0_planeIndex][(s32)vdp2Controls.m0_doubleBufferIndex];
    return MTH_Mul(gVdp2RotationMatrix.m[2][2], (s32)t.m38 * 0x10000 - t.m8_Zst);
}

// 06059018
static void drawVdp2_A7_2(sVdp2PlaneTask* pThis)
{
    pThis->mC_cameraPosition = cameraProperties2.m0_position;
    pThis->m18_cameraRotation.m0_X = (s32)(s16)cameraProperties2.mC_rotation[0] << 16;
    pThis->m18_cameraRotation.m4_Y = (s32)(s16)cameraProperties2.mC_rotation[1] << 16;
    pThis->m18_cameraRotation.m8_Z = (s32)(s16)cameraProperties2.mC_rotation[2] << 16;
    getVdp1ClippingCoordinates(pThis->m24_vdp1Clipping);
    getVdp1LocalCoordinates(pThis->m2C_localCoordinates);
    getVdp1ProjectionParams(&pThis->m30_projParam0, &pThis->m32_projParam1);

    fixedPoint projScale = intDivide((s32)pThis->m30_projParam0, (s32)pThis->m32_projParam1 << 16);
    beginRotationPass(0, projScale);
    drawVdp2Sub_setupRotationPass0_A7_2(pThis);
    drawCinematicBar(6);
    commitRotationPass();

    pThis->m34_scrollValue = computeGroundY_A7_2();

    pThis->m0_scrollX = (pThis->m18_cameraRotation.m4_Y.asS32() >> 12) * -0x400;
    pThis->m4_scrollY = (0xFE - pThis->m34_scrollValue) * 0x10000;

    intDivide((s32)pThis->m30_projParam0, (s32)pThis->m32_projParam1 << 16);
}

// 060593f0 — create subfield 2 VDP2 task
void createA7_2_envTask(p_workArea parent)
{
    static sVdp2PlaneTask::TypedTaskDefinition td = { &initVdp2_A7_2, &updateVdp2_A7_2, &drawVdp2_A7_2, nullptr };
    createSubTask<sVdp2PlaneTask>(parent, &td);
}
