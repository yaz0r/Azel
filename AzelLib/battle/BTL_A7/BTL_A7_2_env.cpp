#include "PDS.h"
#include "BTL_A7_2_env.h"
#include "BTL_A7_2_data.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "battle/BTL_A3/BTL_A3_map6.h"
#include "kernel/cinematicBarsTask.h"
#include "field/field_a3/o_fld_a3.h"

void applyLayerDisplayConfig(const std::vector<std::array<s32, 2>>& r4);
void setupCinematicBarData(int param1, std::array<u32, 256>& dataArray, u32 vdpOffset, int numEntries);

// 06057C70
static void BTL_A7_2_env_Init(sVdp2PlaneTask* pThis)
{
    gBattleManager->m10_battleOverlay->m1C_envTask = pThis;
    reinitVdp2();
    initNBG1Layer();

    pThis->m7C_lineScrollParams.m0_buffer = allocateHeapForTask(pThis, 0xC00);

    asyncDmaCopy(g_BTL_A7_2->getSaturnPtr(0x060a5638), getVdp2Cram(0xA00), 0x20, 0);
    asyncDmaCopy(g_BTL_A7_2->getSaturnPtr(0x060a5658), (u8*)vdp2Palette, 0x200, 0);
    asyncDmaCopy(g_BTL_A7_2->getSaturnPtr(0x060a5858), getVdp2Cram(0x2A0), 0x20, 0);
    asyncDmaCopy(g_BTL_A7_2->getSaturnPtr(0x060a5878), getVdp2Cram(0x140), 0x20, 0);

    static const sLayerConfig nbg0Setup[] = {
        {m1_TPEN, 1}, {m2_CHCN, 0}, {m5_CHSZ, 1}, {m6_PNB, 1}, {m7_CNSM, 0},
        {m34_W0E, 1}, {m37_W0A, 1}, {m44_CCEN, 1},
        {m10_SPN, 5}, {m22_N0LSCX, 1}, {m21_LCSY, 1}, {m20_N0LSS, 1},
        {m0_END},
    };
    setupNBG0(nbg0Setup);

    static const sLayerConfig rgb0Setup[] = {
        {m2_CHCN, 1}, {m5_CHSZ, 1}, {m6_PNB, 1}, {m7_CNSM, 0},
        {m27_RPMD, 2}, {m11_SCN, 8},
        {m34_W0E, 1}, {m37_W0A, 1},
        {m0_END},
    };
    setupRGB0(rgb0Setup);

    static const sLayerConfig rotationParams[] = {
        {m31_RxKTE, 1},
        {m0_END},
    };
    setupRotationParams(rotationParams);

    static const sLayerConfig rotationParams2[] = {
        {m0_END},
    };
    setupRotationParams2(rotationParams2);

    loadFile("FNS_A7_2.SCB", getVdp2Vram(0x10000), 0);
    loadFile("FNS_A7_2.PNB", getVdp2Vram(0x1F000), 0);
    loadFile("FRS_A7_2.SCB", getVdp2Vram(0x40000), 0);
    loadFile("FRS_A7_2.PNB", getVdp2Vram(0x60000), 0);
    loadFile("SPACE.PNB", getVdp2Vram(0x64000), 0);

    initLayerMap(0, 0x1F000, 0x1F000, 0x1F000, 0x1F000);

    vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL = (vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL & 0xFF00) | 0xB4;
    vdp2Controls.m4_pendingVdp2Regs->m10_CYCA0 = 0x310F754F;
    vdp2Controls.m_isDirty = 1;

    setupRotationMapPlanes(0, g_BTL_A7_2->getSaturnPtr(0x060a5514));
    setupRotationMapPlanes(1, g_BTL_A7_2->getSaturnPtr(0x060a5554));

    setupVdp2Table(6, coefficientA0, coefficientA1, getVdp2Vram(0x20000), 0x80);
    setupVdp2Table(7, coefficientB0, coefficientB1, getVdp2Vram(0x22000), 0x80);
    initRotationCoefficientTables(5, getVdp2Vram(0x2A000));

    setupCinematicBarData(1, *(std::array<u32, 256>*)pThis->m7C_lineScrollParams.m0_buffer, 0x25E24000, 0xC0);

    *(u16*)getVdp2Vram(0x2A400) = 0x700;
    vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA = (vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA & 0xFFF80000) | 0x15200;

    *(u16*)getVdp2Vram(0x2A600) = 0xCDAB;
    vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA = (vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA & 0xFFF80000) | 0x15300;

    vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL = (vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL & 0xFEFF) | 0x100;
    vdp2Controls.m4_pendingVdp2Regs->mF0_PRISA = 0x405;
    vdp2Controls.m4_pendingVdp2Regs->mF2_PRISB = 0x507;
    vdp2Controls.m4_pendingVdp2Regs->mF4_PRISC = 0x505;
    vdp2Controls.m4_pendingVdp2Regs->mF6_PRISD = 0x505;
    vdp2Controls.m4_pendingVdp2Regs->mF8_PRINA = 0x604;
    vdp2Controls.m4_pendingVdp2Regs->mFA_PRINB = 0x700;
    vdp2Controls.m4_pendingVdp2Regs->mFC_PRIR = 3;
    vdp2Controls.m_isDirty = 1;

    pThis->m3C_scale = 0x10000;

    static const std::vector<std::array<s32, 2>> layerDisplayConfig = {
        { {0x2C, 0x1} }
    };
    applyLayerDisplayConfig(layerDisplayConfig);

    vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL = (vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL & 0xFEFF) | 0x100;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xF8FF) | 0x400;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xFFF0) | 3;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xCFFF) | 0x1000;

    pThis->m54 = 0x17;
    pThis->m50 = 0x0D;
    pThis->m51 = 0x18;

    vdp2Controls.m4_pendingVdp2Regs->m100_CCRSA = pThis->m50 | (pThis->m51 << 8);
    vdp2Controls.m4_pendingVdp2Regs->m102_CCRSB = 0;
    vdp2Controls.m4_pendingVdp2Regs->m104_CCRSC = 0;
    vdp2Controls.m4_pendingVdp2Regs->m106_CCRSD = 0;
    vdp2Controls.m_isDirty = 1;

    pThis->m4C_wavePhase = 0;
    pThis->m40_waveSpeed = 0xD5A6;
    pThis->m48 = 0xA3A;
    pThis->m44_waveFreq = 0x6E5D4D;

    pThis->m7C_lineScrollParams.m4_phaseSpeed = 0x1E572;
    pThis->m7C_lineScrollParams.m8_freqPerLine = 0xDA764;
    pThis->m7C_lineScrollParams.mC_zoomAmplitude = 0x4A31;
    pThis->m7C_lineScrollParams.m10_scrollBaseSpeed = (s32)0xFFFA0000;
    pThis->m7C_lineScrollParams.m14_scrollIncPerLine = 0x6C1CC;

    // Build cinematic bar coefficient table
    struct sCinematicBarEntry { s32 m0_scale; s32 m4_yOffset; s32 m8_coeff; };
    sCinematicBarEntry* barEntries = (sCinematicBarEntry*)pThis->m7C_lineScrollParams.m0_buffer;
    s32 yPos = -0x70;
    s32 accumY = 0;
    s32 scale = 0xB0;
    for (int i = 0; i < 0xE0; i++)
    {
        barEntries[i].m4_yOffset = accumY;
        s32 sqr = yPos * yPos;
        s32 divisor = sqr * 0x8000;
        barEntries[i].m8_coeff = (divisor != 0 ? (0x3100 / divisor) : 0) + 0x10000;
        barEntries[i].m0_scale = (0x10000 - barEntries[i].m8_coeff) * scale;
        yPos++;
        accumY += 0x10000;
    }

    sVdp2PlaneTask_InitVdp2Sub4(g_BTL_A7_2->getSaturnPtr(0x060a55f8));
}

// 06057A58
static void BTL_A7_2_env_Update(sVdp2PlaneTask* pThis)
{
    u16 ccrna = (vdp2Controls.m4_pendingVdp2Regs->m108_CCRNA & 0xFFE0) | ((u8)pThis->m54 & 0x1F);
    vdp2Controls.m20_registers[0].m108_CCRNA = ccrna;
    vdp2Controls.m20_registers[1].m108_CCRNA = ccrna;

    updateWaveDistortionParams(&pThis->m7C_lineScrollParams);
    drawCinematicBar(1);
}

// 06057a96
static void BTL_A7_2_env_DrawRotationPass0(sVdp2PlaneTask* pThis)
{
    s32 coeffIdx = (s16)((gRotationPassState.m0_planeIndex * 2 + (s16)vdp2Controls.m0_doubleBufferIndex) * 0x70);

    s32 rotX = pThis->m18_cameraRotation.m0_X;
    if (rotX == 0) rotX = -0x12345;
    s32 rotY = pThis->m18_cameraRotation.m4_Y;
    s32 rotZ = pThis->m18_cameraRotation.m8_Z;

    s16 midX = (s16)(((s32)pThis->m24_vdp1Clipping[0] + (s32)pThis->m24_vdp1Clipping[2]) / 2);
    s16 midY = (s16)(((s32)pThis->m24_vdp1Clipping[1] + (s32)pThis->m24_vdp1Clipping[3]) / 2);

    *(s16*)((u8*)&gCoefficientTables[0][0].m34 + coeffIdx) = midX;
    *(s16*)((u8*)&gCoefficientTables[0][0].m36 + coeffIdx) = midY;
    *(s16*)((u8*)&gCoefficientTables[0][0].m38 + coeffIdx) = pThis->m30_vdp1ProjectionParam[1];
    *(s16*)((u8*)&gCoefficientTables[0][0].m3C + coeffIdx) = midX;
    *(s16*)((u8*)&gCoefficientTables[0][0].m3E + coeffIdx) = midY;
    *(s16*)((u8*)&gCoefficientTables[0][0].m40 + coeffIdx) = 0;

    buildRotationMatrixPitchYaw(-0x4000000 - rotX, -rotY);
    scaleRotationMatrix(pThis->m3C_scale);
    writeRotationParams(-rotZ);

    s16 Px = *(s16*)((u8*)&gCoefficientTables[0][0].m34 + coeffIdx);
    s16 Py = *(s16*)((u8*)&gCoefficientTables[0][0].m36 + coeffIdx);
    s16 Pz = *(s16*)((u8*)&gCoefficientTables[0][0].m38 + coeffIdx);
    s16 Cx = *(s16*)((u8*)&gCoefficientTables[0][0].m3C + coeffIdx);
    s16 Cy = *(s16*)((u8*)&gCoefficientTables[0][0].m3E + coeffIdx);
    s16 Cz = *(s16*)((u8*)&gCoefficientTables[0][0].m40 + coeffIdx);

    s32 dX = Px - Cx;
    s32 dY = Py - Cy;
    s32 dZ = Pz - Cz;

    s32 camX = MTH_Mul(pThis->m3C_scale, pThis->mC_cameraPosition.m0_X << 4);
    gVdp2RotationMatrix.Mx = camX
        - gVdp2RotationMatrix.m[0][0] * dX
        - gVdp2RotationMatrix.m[0][1] * dY
        - gVdp2RotationMatrix.m[0][2] * dZ
        + Cx * -0x10000;

    s32 camZ = MTH_Mul(pThis->m3C_scale, pThis->mC_cameraPosition.m8_Z << 4);
    gVdp2RotationMatrix.My = camZ
        - gVdp2RotationMatrix.m[1][0] * dX
        - gVdp2RotationMatrix.m[1][1] * dY
        - gVdp2RotationMatrix.m[1][2] * dZ
        + Cy * -0x10000;

    gVdp2RotationMatrix.Mz = (pThis->mC_cameraPosition.m4_Y - pThis->m38) * 0x10
        - gVdp2RotationMatrix.m[2][0] * dX
        - gVdp2RotationMatrix.m[2][1] * dY
        - gVdp2RotationMatrix.m[2][2] * dZ
        + Cz * -0x10000;
}

// 060579f6
static s32 BTL_A7_2_env_ComputeGroundY()
{
    s32 coeffIdx = (s16)((gRotationPassState.m0_planeIndex * 2 + (s16)vdp2Controls.m0_doubleBufferIndex) * 0x70);

    s16 Px = *(s16*)((u8*)&gCoefficientTables[0][0].m34 + coeffIdx);
    s16 Py = *(s16*)((u8*)&gCoefficientTables[0][0].m36 + coeffIdx);
    s16 Pz = *(s16*)((u8*)&gCoefficientTables[0][0].m38 + coeffIdx);
    s32 Zst = *(s32*)((u8*)&gCoefficientTables[0][0].m8_Zst + coeffIdx);

    s32 val = gVdp2RotationMatrix.m[2][0] * Px
            + gVdp2RotationMatrix.m[2][1] * Py
            + MTH_Mul(gVdp2RotationMatrix.m[2][2], (s32)Pz * 0x10000 - Zst);

    return FP_Div(val, gVdp2RotationMatrix.m[2][1]).getInteger();
}

// 060580E4
static void BTL_A7_2_env_Draw(sVdp2PlaneTask* pThis)
{
    pThis->mC_cameraPosition = cameraProperties2.m0_position;
    pThis->m18_cameraRotation.m0_X = (s32)cameraProperties2.mC_rotation[0].m_value << 16;
    pThis->m18_cameraRotation.m4_Y = (s32)cameraProperties2.mC_rotation[1].m_value << 16;
    pThis->m18_cameraRotation.m8_Z = (s32)cameraProperties2.mC_rotation[2].m_value << 16;

    getVdp1ClippingCoordinates(pThis->m24_vdp1Clipping);
    getVdp1LocalCoordinates(pThis->m2C_vdp1LocalCoordinates);
    getVdp1ProjectionParams(&pThis->m30_vdp1ProjectionParam[0], &pThis->m30_vdp1ProjectionParam[1]);

    fixedPoint fov = intDivide(pThis->m30_vdp1ProjectionParam[0], (s32)pThis->m30_vdp1ProjectionParam[1] << 16);

    beginRotationPass(0, fov);
    BTL_A7_2_env_DrawRotationPass0(pThis);
    drawCinematicBar(6);
    commitRotationPass();

    pThis->m34 = BTL_A7_2_env_ComputeGroundY();
    pThis->m0_scrollX = (pThis->m18_cameraRotation.m4_Y >> 0xC) * -0x400;
    pThis->m4_scrollY = (0xFE - pThis->m34) * 0x10000;

    fov = intDivide(pThis->m30_vdp1ProjectionParam[0], (s32)pThis->m30_vdp1ProjectionParam[1] << 16);
    beginRotationPass(1, fov);

    {
    sCoefficientTableData& ct = gCoefficientTables[gRotationPassState.m0_planeIndex][(s32)vdp2Controls.m0_doubleBufferIndex];
    s32 sumX = (s32)pThis->m24_vdp1Clipping[0] + (s32)pThis->m24_vdp1Clipping[2];
    ct.m34 = (s16)((sumX + (int)(sumX < 0)) >> 1);
    s32 sumY = (s32)pThis->m24_vdp1Clipping[1] + (s32)pThis->m24_vdp1Clipping[3];
    ct.m36 = (s16)((sumY + (int)(sumY < 0)) >> 1);
    ct.m38 = pThis->m30_vdp1ProjectionParam[1];
    ct.m3C = ct.m34;
    ct.m3E = ct.m36;
    ct.m40 = 0;
    }

    buildRotationMatrixRoll(-pThis->m18_cameraRotation.m8_Z);

    intDivide(pThis->m24_vdp1Clipping[3] - pThis->m24_vdp1Clipping[1], 0xE00000);
    scaleRotationMatrix(intDivide(pThis->m24_vdp1Clipping[2] - pThis->m24_vdp1Clipping[0], 0x1600000));

    setRotationScrollOffset(pThis->m0_scrollX, pThis->m4_scrollY);
    commitRotationPass();

    gCurrentVDP2ScrollLayer = 0;
    setupVDP2CoordinatesIncrement2(pThis->m0_scrollX + pThis->m7C_lineScrollParams.m1C_scrollBaseAccum, 0);
    gCurrentVDP2ScrollLayer = 4;

    auto* pRegs = vdp2Controls.m4_pendingVdp2Regs;
    pRegs->mC0_WPSX0 = pThis->m24_vdp1Clipping[0] << 1;
    pRegs->mC2_WPSY0 = pThis->m24_vdp1Clipping[1];
    pRegs->mC4_WPEX0 = pThis->m24_vdp1Clipping[2] << 1;
    pRegs->mC6_WPEY0 = pThis->m24_vdp1Clipping[3];

    pRegs->mC8_WPSX1 = pThis->m24_vdp1Clipping[0] << 1;
    pRegs->mCA_WPSY1 = pThis->m24_vdp1Clipping[1];
    pRegs->mCC_WPEX1 = pThis->m24_vdp1Clipping[2] << 1;
    pRegs->mCE_WPEY1 = pThis->m24_vdp1Clipping[3];
}

// 0605830a
void Create_BTL_A7_2_env(s_workAreaCopy* parent)
{
    static const sVdp2PlaneTask::TypedTaskDefinition def = {
        &BTL_A7_2_env_Init,
        &BTL_A7_2_env_Update,
        &BTL_A7_2_env_Draw,
        nullptr,
    };
    createSubTask<sVdp2PlaneTask>((s_workArea*)parent, &def);
}
