#include "PDS.h"
#include "o_fld_a7.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldRadar.h"
#include "field/fieldVisibilityGrid.h"
#include "audio/soundDriver.h"
#include "battle/BTL_A3/BTL_A3_map6.h"
#include "trigo.h"
#include "kernel/cinematicBarsTask.h"

void setupCinematicBarData(int param1, std::array<u32, 256>& dataArray, u32 vdpOffset, int numEntries);

FLD_A7_data* gFLD_A7 = nullptr;

static const s_MCB_CGB fieldFileList[] =
{
    { "FLDCMN.MCB", "FLDCMN.CGB" },       // 0
    { nullptr, nullptr },                    // 1 (no FLD_A7.MCB)
    { "FLD_A7_0.MCB", "FLD_A7_0.CGB" },    // 2
    { "FLD_A7_1.MCB", "FLD_A7_1.CGB" },    // 3
    { "FLD_A7_2.MCB", "FLD_A7_2.CGB" },    // 4
    { (const char*)-1, nullptr }
};

// 06054474
static void createFieldSpecificDataTask_A7(p_workArea parent)
{
    s_fieldSpecificData_A7* p = createSubTaskFromFunction<s_fieldSpecificData_A7>(parent, nullptr);
    if (p)
    {
        getFieldTaskPtr()->mC = p;
    }
}

// 06059418
static void initDragonLightParams(fixedPoint lightRotation, fixedPoint lightParam2,
    s8 n0, s8 n1, s8 n2,
    s8 f0_0, s8 f0_1, s8 f0_2,
    s8 f1_0, s8 f1_1, s8 f1_2,
    s8 f2_0, s8 f2_1, s8 f2_2,
    s8 f3_0, s8 f3_1, s8 f3_2)
{
    s_dragonTaskWorkArea* p = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    p->mC0_lightRotationAroundDragon = lightRotation;
    p->mC4 = lightParam2;
    p->mC8_normalLightColor = { n0, n1, n2 };
    p->mCB_falloffColor0 = { f0_0, f0_1, f0_2 };
    p->mCE_falloffColor1 = { f1_0, f1_1, f1_2 };
    p->mD1_falloffColor2 = { f2_0, f2_1, f2_2 };
    p->mD4 = { f3_0, f3_1, f3_2 };
}

// 06059538 — dragon light params for subfield 0
static void initDragonParams_A7_0()
{
    // Data at 06085aa4
    initDragonLightParams(fixedPoint(0x0871C71C), fixedPoint(0),
        0x03, 0x03, 0x03,
        0x0A, 0x0A, 0x0A,
        (s8)0xE0, (s8)0xFB, (s8)0xFB,
        0x0A, 0x0A, 0x0A,
        0x10, 0x10, 0x10);
}

// 0605953e — dragon light params for subfield 1
static void initDragonParams_A7_1()
{
    // Data at 06085abc
    initDragonLightParams(fixedPoint(0x0871C71C), fixedPoint(0),
        0x00, 0x00, 0x00,
        0x0A, 0x10, 0x14,
        0x09, 0x0E, 0x07,
        0x17, 0x17, 0x1D,
        0x10, 0x10, 0x10);
}

// 06059544 — dragon light params for subfield 2
static void initDragonParams_A7_2()
{
    // Data at 06085ad4
    initDragonLightParams(fixedPoint(0x0871C71C), fixedPoint(0),
        0x10, 0x1B, 0x1C,
        0x01, 0x05, 0x08,
        (s8)0xE0, (s8)0xF2, (s8)0xEE,
        (s8)0xE0, (s8)0xF4, (s8)0xEE,
        0x10, 0x10, 0x10);
}

// 06056bb0 — start tasks for subfield 0
static void startTasksA7_0(p_workArea parent)
{
    createFieldSpecificDataTask_A7(parent);
    Unimplemented(); // FUN_FLD_A7__06076ea8 — create 3D scene subtask
    Unimplemented(); // FUN_FLD_A7__06054536 — create environment objects
    Unimplemented(); // FUN_FLD_A7__0605600a — create environment objects
    Unimplemented(); // FUN_FLD_A7__060565a6 — create environment objects
    Unimplemented(); // FUN_FLD_A7__0606a65c — create save point
    Unimplemented(); // FUN_FLD_A7__06059eb8 — create environment objects
    Unimplemented(); // FUN_FLD_A7__0605c75c — create environment objects
    Unimplemented(); // FUN_FLD_A7__0605e930 — create environment objects
    Unimplemented(); // FUN_FLD_A7__06054684 — create environment objects
}

// 06056c0a — start tasks for subfield 1
static void startTasksA7_1(p_workArea parent)
{
    createFieldSpecificDataTask_A7(parent);
    Unimplemented(); // FUN_FLD_A7__06076ea8 — create 3D scene subtask
    Unimplemented(); // FUN_FLD_A7__0605a456 — create environment objects
    Unimplemented(); // FUN_FLD_A7__06059e6e — create environment objects
    Unimplemented(); // FUN_FLD_A7__06059f00 — create environment objects
    Unimplemented(); // FUN_FLD_A7__0605a2fe — create environment objects
    Unimplemented(); // FUN_FLD_A7__060565aa — create environment objects
    Unimplemented(); // FUN_FLD_A7__0605eba4 — create environment objects
    Unimplemented(); // FUN_FLD_A7__060551ee — create environment objects
    Unimplemented(); // FUN_FLD_A7__0605e7c2 — create environment objects
}

// 06056c6a — start tasks for subfield 2
static void startTasksA7_2(p_workArea parent)
{
    createFieldSpecificDataTask_A7(parent);
    Unimplemented(); // FUN_FLD_A7__06076ea8 — create 3D scene subtask
    Unimplemented(); // FUN_FLD_A7__0605600a — create environment objects
    Unimplemented(); // FUN_FLD_A7__060565b2 — create environment objects
    Unimplemented(); // FUN_FLD_A7__0605e034 — create environment objects
    Unimplemented(); // FUN_FLD_A7__0605eb96 — create environment objects
}

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
        {m2_CHCN, 0}, {m5_CHSZ, 1}, {m6_PNB, 1}, {m7_CNSM, 0},
        {m34_W0E, 1}, {m37_W0A, 1}, {m44_CCEN, 1}, {m10_SPN, 4},
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
    s_BTL_A3_Env_InitVdp2Sub3(5, getVdp2Vram(0x2a000));
    setupCinematicBarData(1, *(std::array<u32, 256>*)pThis->m50_lineScrollBuffer, 0x25e24000, 0xc0);
}

// 06057c22 — set up rotation scroll for pass 0 (ground plane)
static void drawVdp2Sub_setupRotationPass0(sVdp2PlaneTask* pThis)
{
    auto& coeff = gCoefficientTables[gRotationPassState.m0_planeIndex][(s32)vdp2Controls.m0_doubleBufferIndex];

    fixedPoint camRotX = pThis->m18_cameraRotation.m0_X;
    if (camRotX == 0)
    {
        camRotX = fixedPoint(0xFFF49F4A); // -0xB60B6
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

    buildRotationMatrixPitchYaw(fixedPoint(0xFC000000) - camRotX, -camRotY);
    scaleRotationMatrix(pThis->m3C_scale);
    writeRotationParams(-camRotZ);

    // Compute Mx, My, Mz from camera position, scale, and rotation matrix
    s32 dPx = coeff.m34 - coeff.m3C;
    s32 dPy = coeff.m36 - coeff.m3E;
    s32 dPz = coeff.m38 - coeff.m40;

    s32 scaledX = MTH_Mul(pThis->m3C_scale, fixedPoint(pThis->mC_cameraPosition.m0_X.asS32() << 4)).asS32();
    gVdp2RotationMatrix.Mx = fixedPoint(scaledX
        - gVdp2RotationMatrix.m[0][0].asS32() * dPx
        - gVdp2RotationMatrix.m[0][1].asS32() * dPy
        - gVdp2RotationMatrix.m[0][2].asS32() * dPz
        + coeff.m3C * -0x10000);

    s32 scaledY = MTH_Mul(pThis->m3C_scale, fixedPoint(pThis->mC_cameraPosition.m4_Y.asS32() << 4)).asS32();
    gVdp2RotationMatrix.My = fixedPoint(scaledY
        - gVdp2RotationMatrix.m[1][0].asS32() * dPx
        - gVdp2RotationMatrix.m[1][1].asS32() * dPy
        - gVdp2RotationMatrix.m[1][2].asS32() * dPz
        + coeff.m3E * -0x10000);

    s32 scaledZ = (pThis->mC_cameraPosition.m8_Z.asS32() - pThis->m38_groundY) * 0x10;
    gVdp2RotationMatrix.Mz = fixedPoint(scaledZ
        - gVdp2RotationMatrix.m[2][0].asS32() * dPx
        - gVdp2RotationMatrix.m[2][1].asS32() * dPy
        - gVdp2RotationMatrix.m[2][2].asS32() * dPz
        + coeff.m40 * -0x10000);
}


// 06057420 — moved to shared/vdp2PlaneTask.cpp as vdp2ApplyWaveDistortion

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
    drawVdp2Sub_setupRotationPass0(pThis);
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
    fixedPoint scaleVal = intDivide((s32)pThis->m24_vdp1Clipping[3] - (s32)pThis->m24_vdp1Clipping[1], 0xE00000);
    scaleRotationMatrix(scaleVal);
    setRotationScrollOffset(pThis->m0_scrollX, pThis->m4_scrollY);
    commitRotationPass();
}

// 06057dd4 — create subfield 0 VDP2 task
static void createA7_0_envTask(p_workArea parent)
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
    s_BTL_A3_Env_InitVdp2Sub3(5, getVdp2Vram(0x2a000));
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
        camRotX = fixedPoint(0xFFF49F4A); // -0xB60B6
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

    buildRotationMatrixPitchYaw(fixedPoint(0xFC000000) - camRotX, -(camRotY + fixedPoint(skyRotation)));
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

    s32 scaledY = MTH_Mul(pThis->m3C_scale, fixedPoint(pThis->mC_cameraPosition.m4_Y.asS32() << 4)).asS32();
    gVdp2RotationMatrix.My = fixedPoint(scaledY
        - gVdp2RotationMatrix.m[1][0].asS32() * dPx
        - gVdp2RotationMatrix.m[1][1].asS32() * dPy
        - gVdp2RotationMatrix.m[1][2].asS32() * dPz
        + coeff.m3E * -0x10000);

    s32 scaledZ = (pThis->mC_cameraPosition.m8_Z.asS32() - pThis->m38_groundY) * 0x10;
    gVdp2RotationMatrix.Mz = fixedPoint(scaledZ
        - gVdp2RotationMatrix.m[2][0].asS32() * dPx
        - gVdp2RotationMatrix.m[2][1].asS32() * dPy
        - gVdp2RotationMatrix.m[2][2].asS32() * dPz
        + coeff.m40 * -0x10000);
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
    Unimplemented(); // FUN_FLD_A7__06076584 — horizon gauge VDP1 drawing
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
    fixedPoint scaleVal = intDivide((s32)pThis->m24_vdp1Clipping[3] - (s32)pThis->m24_vdp1Clipping[1], 0xE00000);
    scaleRotationMatrix(scaleVal);
    setRotationScrollOffset(pThis->m0_scrollX, pThis->m4_scrollY);
    commitRotationPass();
}

// 06058a74 — create subfield 1 VDP2 task
static void createA7_1_envTask(p_workArea parent)
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
    s_BTL_A3_Env_InitVdp2Sub3(5, getVdp2Vram(0x2a000));
    setupCinematicBarData(1, *(std::array<u32, 256>*)pThis->m50_lineScrollBuffer, 0x25e24000, 0xc0);
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
        camRotX = fixedPoint(0xFFF49F4A); // -0xB60B6
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

    buildRotationMatrixPitchYaw(fixedPoint(0xFC000000) - camRotX, -camRotY);
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

    s32 scaledY = MTH_Mul(pThis->m3C_scale, fixedPoint(pThis->mC_cameraPosition.m4_Y.asS32() << 4)).asS32();
    gVdp2RotationMatrix.My = fixedPoint(scaledY
        - gVdp2RotationMatrix.m[1][0].asS32() * dPx
        - gVdp2RotationMatrix.m[1][1].asS32() * dPy
        - gVdp2RotationMatrix.m[1][2].asS32() * dPz
        + coeff.m3E * -0x10000);

    s32 scaledZ = (pThis->mC_cameraPosition.m8_Z.asS32() - pThis->m38_groundY) * 0x10;
    gVdp2RotationMatrix.Mz = fixedPoint(scaledZ
        - gVdp2RotationMatrix.m[2][0].asS32() * dPx
        - gVdp2RotationMatrix.m[2][1].asS32() * dPy
        - gVdp2RotationMatrix.m[2][2].asS32() * dPz
        + coeff.m40 * -0x10000);
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

    pThis->m34_scrollValue = computeRotationScrollOffset();

    pThis->m0_scrollX = (pThis->m18_cameraRotation.m4_Y.asS32() >> 12) * -0x400;
    pThis->m4_scrollY = (0xFE - pThis->m34_scrollValue) * 0x10000;

    intDivide((s32)pThis->m30_projParam0, (s32)pThis->m32_projParam1 << 16);
}

// 060593f0 — create subfield 2 VDP2 task
static void createA7_2_envTask(p_workArea parent)
{
    static sVdp2PlaneTask::TypedTaskDefinition td = { &initVdp2_A7_2, &updateVdp2_A7_2, &drawVdp2_A7_2, nullptr };
    createSubTask<sVdp2PlaneTask>(parent, &td);
}

// 060540e8
void subfieldA7_0(p_workArea workArea)
{
    playPCM(workArea, 100);
    playPCM(workArea, 0x6C);

    s_fieldTaskWorkArea* pFieldTask = getFieldTaskPtr();
    if (pFieldTask->m2C_currentFieldIndex == 0x16)
    {
        // Position/rotation data at 060801e4 / 060801f0
        static const sVec3_FP pos_0x16 = { fixedPoint(0x005FF000), fixedPoint(0x0002F000), fixedPoint(0xFFDAC000) };
        static const sVec3_FP rot_0x16 = { fixedPoint(0), fixedPoint(0x005B05B0), fixedPoint(0) };
        setupDragonPosition(&pos_0x16, &rot_0x16);
    }
    else
    {
        pFieldTask = getFieldTaskPtr();
        if (pFieldTask->m32_previousSubField == (s16)0xFFFF)
        {
            // Position/rotation data at 060801b4 / 060801c0
            static const sVec3_FP pos_default = { fixedPoint(0x00600000), fixedPoint(0x000A0000), fixedPoint(0xFFE34000) };
            static const sVec3_FP rot_default = { fixedPoint(0), fixedPoint(0), fixedPoint(0) };
            setupDragonPosition(&pos_default, &rot_default);
        }
        else
        {
            // Camera script from 060800d0
            getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript(gFLD_A7->getSaturnPtr(0x060800d0));
        }
        startFieldScript(0x1D, -1);
    }

    // setupField with data table 3 at 06083584 and data table 2 at 060824e8
    s_DataTable3* pDT3 = readDataTable3(gFLD_A7->getSaturnPtr(0x06083584));
    s_DataTable2* pDT2 = readDataTable2(gFLD_A7->getSaturnPtr(0x060824e8));
    setupField(pDT3, pDT2, startTasksA7_0, nullptr);

    // 06072fc8 — enable dragon ground shadow
    enableDragonShadow();

    // 060544a0 — camera config from 06080274
    setupFieldCameraConfigs(readCameraConfig(gFLD_A7->getSaturnPtr(0x06080274)), 1);

    fieldRadar_enableAltitudeGauge();
    adjustVerticalLimits(0x5000, 0x136000);
    adjustHorizontalLimits(0, 0xC00000, 0xFF400000, 0);

    createA7_0_envTask(workArea);

    pFieldTask = getFieldTaskPtr();
    if (pFieldTask->m2C_currentFieldIndex == 0x16)
    {
        initDragonParams_A7_2();
    }
    else
    {
        initDragonParams_A7_0();
    }

    getFieldTaskPtr()->m8_pSubFieldData->m344_randomBattleTask->m0 = nullBattle;

    fieldRadar_initDestinations(0x1C);
}

// 06054268
void subfieldA7_1(p_workArea workArea)
{
    playPCM(workArea, 0x6F);

    // setupField2 with data table 3 at 06084204, no visibility
    s_DataTable3* pDT3 = readDataTable3(gFLD_A7->getSaturnPtr(0x06084204));
    setupField2(pDT3, startTasksA7_1);

    // Set dragon mF4 callback
    Unimplemented(); // m338->mF4 = FLD_A7::06056dbc

    // m360 = null function (060544bc)
    getFieldTaskPtr()->m8_pSubFieldData->pUpdateFunction3 = nullptr;

    // 060544a8 — camera config from 060802cc
    setupFieldCameraConfigs(readCameraConfig(gFLD_A7->getSaturnPtr(0x060802cc)), 1);

    fieldRadar_enableAltitudeGauge();
    adjustVerticalLimits(0x5000, 0x12C000);
    adjustHorizontalLimits(0, 0xC00000, 0xFF400000, 0);

    createA7_1_envTask(workArea);
    initDragonParams_A7_1();

    getFieldTaskPtr()->m8_pSubFieldData->m344_randomBattleTask->m0 = nullBattle;

    // Position/rotation data at 060801cc / 060801d8
    static const sVec3_FP pos = { fixedPoint(0x005FE000), fixedPoint(0x0005E000), fixedPoint(0xFF7CE000) };
    static const sVec3_FP rot = { fixedPoint(0), fixedPoint(0x08000000), fixedPoint(0) };
    setupDragonPosition(&pos, &rot);

    s_fieldTaskWorkArea* pFieldTask = getFieldTaskPtr();
    if (pFieldTask->m32_previousSubField == (s16)0xFFFF)
    {
        startFieldScript(0x1E, -1);
    }
    else
    {
        startFieldScript(0x13, -1);
    }
}

// 06054304
void subfieldA7_2(p_workArea workArea)
{
    playPCM(workArea, 0x65);
    loadFileFromFileList(4);

    s_visibilityGridWorkArea* pCameraTask = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;

    // setupField2 with no data table
    setupField2(nullptr, startTasksA7_2);

    pCameraTask->mC.m0_X = 0;
    pCameraTask->mC.m8_Z = 0;

    // Set dragon mF4 callback
    Unimplemented(); // m338->mF4 = FLD_A7::06056dec

    fieldRadar_enableAltitudeGauge();
    adjustVerticalLimits(0, 0xB4000);
    adjustHorizontalLimits(fixedPoint(0xFFC00000), fixedPoint(0x400000), fixedPoint(0xFFC00000), fixedPoint(0x400000));

    createA7_2_envTask(workArea);

    getFieldTaskPtr()->m8_pSubFieldData->m344_randomBattleTask->m0 = nullBattle;

    startFieldScript(0x12, -1);
}

// 06054000
p_workArea overlayStart_FLD_A7(p_workArea workArea, u32 arg)
{
    if (gFLD_A7 == nullptr)
    {
        gFLD_A7 = new FLD_A7_data();
    }

    if (gFLD_A3 == nullptr)
    {
        gFLD_A3 = new FLD_A3_data();
    }

    gFieldCameraConfigEA = gFLD_A7->getSaturnPtr(0x060866d4);
    gFieldDragonAnimTableEA = { 0x06094134, gFLD_A3 };
    gFieldCameraDrawFunc = &fieldOverlaySubTaskInitSub2;

    if (!initField(workArea, fieldFileList, arg))
    {
        return nullptr;
    }

    graphicEngineStatus.m405C.m10_nearClipDistance = 0x3000;
    graphicEngineStatus.m405C.m30_oneOverNearClip = FP_Div(0x10000, graphicEngineStatus.m405C.m10_nearClipDistance);
    graphicEngineStatus.m405C.m14_farClipDistance = 0x400000;
    graphicEngineStatus.m405C.m38_oneOverFarClip = FP_Div(0x8000, graphicEngineStatus.m405C.m14_farClipDistance);
    graphicEngineStatus.m405C.m34_oneOverFarClip256 = graphicEngineStatus.m405C.m38_oneOverFarClip << 8;

    getFieldTaskPtr()->m8_pSubFieldData->m334->m50E_followModeIndex = 1;
    getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m0_pScripts = ReadScripts(gFLD_A7->getSaturnPtr(0x060861b0));

    s_fieldTaskWorkArea* pFieldTask = getFieldTaskPtr();
    if (pFieldTask->m2C_currentFieldIndex != 4 && pFieldTask->m2C_currentFieldIndex != 0x16)
    {
        if (pFieldTask->m32_previousSubField == (s16)0xFFFF)
        {
            loadSoundBanks(0xF, 0);
            playPCM(workArea, 100);
        }

        static void (*subfieldTable2[])(p_workArea) = {
            subfieldA7_2, subfieldA7_1,
        };
        subfieldTable2[getFieldTaskPtr()->m2E_currentSubFieldIndex](workArea);
    }
    else
    {
        loadSoundBanks(0xB, 0);

        static void (*subfieldTable1[])(p_workArea) = {
            subfieldA7_0, subfieldA7_1, subfieldA7_2,
        };
        subfieldTable1[getFieldTaskPtr()->m2E_currentSubFieldIndex](workArea);
    }

    return nullptr;
}
