#include "PDS.h"
#include "BTL_X0_env.h"
#include "BTL_X0_data.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "battle/BTL_A3/BTL_A3_map6.h"
#include "kernel/cinematicBarsTask.h"
#include "field/field_a3/o_fld_a3.h"
#include "shared/vdp2PlaneTask.h"
#include "kernel/graphicalObject.h"
#include "battle/battleDragon.h"

void applyLayerDisplayConfig(const std::vector<std::array<s32, 2>>& r4);
void setupCinematicBarData(int param1, std::array<u32, 256>& dataArray, u32 vdpOffset, int numEntries);

// 060b01e6
static bool BTL_X0_env_checkFlag()
{
    sVdp2PlaneTask* envTask = (sVdp2PlaneTask*)gBattleManager->m10_battleOverlay->m1C_envTask;
    return (envTask->m70_flags & 1) != 0;
}

// 06054144
static void BTL_X0_env_Init(sVdp2PlaneTask* pThis)
{
    gBattleManager->m10_battleOverlay->m1C_envTask = pThis;
    reinitVdp2();
    initNBG1Layer();

    pThis->m7C_lineScrollParams.m0_buffer = allocateHeapForTask(pThis, 0xC00);

    asyncDmaCopy(g_BTL_X0->getSaturnPtr(0x060b28ec), getVdp2Cram(0xA00), 0x20, 0);
    asyncDmaCopy(g_BTL_X0->getSaturnPtr(0x060b290c), (u8*)vdp2Palette, 0x200, 0);

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
        {m34_W0E, 1}, {m37_W0A, 1}, {m44_CCEN, 0}, {m45_COEN, 0},
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
    loadFile("FNS_A7_2.PNB", getVdp2Vram(0x1E000), 0);
    loadFile("FRS_A7_2.SCB", getVdp2Vram(0x40000), 0);
    loadFile("FRS_A7_2.PNB", getVdp2Vram(0x60000), 0);
    loadFile("SPACE.PNB", getVdp2Vram(0x64000), 0);

    initLayerMap(0, 0x1E000, 0x1E000, 0x1E000, 0x1E000);

    vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL = (vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL & 0xFF00) | 0xB4;
    vdp2Controls.m4_pendingVdp2Regs->m10_CYCA0 = 0x310F754F;
    vdp2Controls.m_isDirty = 1;

    setupRotationMapPlanes(0, g_BTL_X0->getSaturnPtr(0x060b2b8c));
    setupRotationMapPlanes(1, g_BTL_X0->getSaturnPtr(0x060b2bcc));

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
        { {m44_CCEN, 0} }
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

    pThis->m7C_lineScrollParams.m4_phaseSpeed = 0x1E572;
    pThis->m7C_lineScrollParams.m8_freqPerLine = 0xDA764;
    pThis->m7C_lineScrollParams.mC_zoomAmplitude = 0x4A31;
    pThis->m7C_lineScrollParams.m10_scrollBaseSpeed = (s32)0xFFFA0000;
    pThis->m7C_lineScrollParams.m14_scrollIncPerLine = 0x6C1CC;

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

    sVdp2PlaneTask_InitVdp2Sub4(g_BTL_X0->getSaturnPtr(0x060b2c70));
}

// 06054568
static void BTL_X0_env_Update(sVdp2PlaneTask* pThis)
{
    if (!BTL_X0_env_checkFlag())
    {
        updateWaveDistortionParams(&pThis->m7C_lineScrollParams);
        drawCinematicBar(1);
    }
}

// 060547dc
static void BTL_X0_env_DrawRotationPass0(sVdp2PlaneTask* pThis)
{
    s32 coeffIdx = (s16)((gRotationPassState.m0_planeIndex * 2 + (s16)vdp2Controls.m0_doubleBufferIndex) * 0x70);

    s32 rotX = pThis->m18_cameraRotation.m0_X;
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

// 060545ac
static void BTL_X0_env_Draw(sVdp2PlaneTask* pThis)
{
    pThis->mC_cameraPosition = cameraProperties2.m0_position;
    pThis->m18_cameraRotation.m0_X = (s32)cameraProperties2.mC_rotation[0].m_value << 16;
    pThis->m18_cameraRotation.m4_Y = (s32)cameraProperties2.mC_rotation[1].m_value << 16;
    pThis->m18_cameraRotation.m8_Z = (s32)cameraProperties2.mC_rotation[2].m_value << 16;

    getVdp1ClippingCoordinates(pThis->m24_vdp1Clipping);
    getVdp1LocalCoordinates(pThis->m2C_vdp1LocalCoordinates);
    getVdp1ProjectionParams(&pThis->m30_vdp1ProjectionParam[0], &pThis->m30_vdp1ProjectionParam[1]);

    fixedPoint fov = FP_Div(pThis->m30_vdp1ProjectionParam[0], (s32)pThis->m30_vdp1ProjectionParam[1] << 16);

    beginRotationPass(0, fov);
    BTL_X0_env_DrawRotationPass0(pThis);
    drawCinematicBar(6);
    commitRotationPass();

    pThis->m34 = computeRotationScrollOffset();
    pThis->m0_scrollX = (pThis->m18_cameraRotation.m4_Y >> 0xC) * -0x400;
    pThis->m4_scrollY = (0xFE - pThis->m34) * 0x10000;

    fov = FP_Div(pThis->m30_vdp1ProjectionParam[0], (s32)pThis->m30_vdp1ProjectionParam[1] << 16);
    beginRotationPass(1, fov);

    s32 coeffIdx = (s16)((gRotationPassState.m0_planeIndex * 2 + (s16)vdp2Controls.m0_doubleBufferIndex) * 0x70);
    s16 midX = (s16)(((s32)pThis->m24_vdp1Clipping[0] + (s32)pThis->m24_vdp1Clipping[2]) / 2);
    s16 midY = (s16)(((s32)pThis->m24_vdp1Clipping[1] + (s32)pThis->m24_vdp1Clipping[3]) / 2);

    *(s16*)((u8*)&gCoefficientTables[0][0].m34 + coeffIdx) = midX;
    *(s16*)((u8*)&gCoefficientTables[0][0].m36 + coeffIdx) = midY;
    *(s16*)((u8*)&gCoefficientTables[0][0].m38 + coeffIdx) = pThis->m30_vdp1ProjectionParam[1];
    *(s16*)((u8*)&gCoefficientTables[0][0].m3C + coeffIdx) = midX;
    *(s16*)((u8*)&gCoefficientTables[0][0].m3E + coeffIdx) = midY;
    *(s16*)((u8*)&gCoefficientTables[0][0].m40 + coeffIdx) = 0;

    buildRotationMatrixRoll(-pThis->m18_cameraRotation.m8_Z);

    intDivide(pThis->m24_vdp1Clipping[3] - pThis->m24_vdp1Clipping[1], 0xE00000);
    scaleRotationMatrix(intDivide(pThis->m24_vdp1Clipping[2] - pThis->m24_vdp1Clipping[0], 0x1600000));

    setRotationScrollOffset(pThis->m0_scrollX, pThis->m4_scrollY);
    commitRotationPass();

    if (!BTL_X0_env_checkFlag())
    {
        gCurrentVDP2ScrollLayer = 0;
        setupVDP2CoordinatesIncrement2(pThis->m0_scrollX, 0);
        gCurrentVDP2ScrollLayer = 4;
    }

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

// 0605498a
void Create_BTL_X0_env(s_workAreaCopy* parent)
{
    static const sVdp2PlaneTask::TypedTaskDefinition def = {
        &BTL_X0_env_Init,
        &BTL_X0_env_Update,
        &BTL_X0_env_Draw,
        nullptr,
    };
    createSubTask<sVdp2PlaneTask>((s_workArea*)parent, &def);
}

// 06054abc
static void BTL_X0_env2_Init(sVdp2PlaneTask* pThis)
{
    gBattleManager->m10_battleOverlay->m1C_envTask = pThis;
    reinitVdp2();
    initNBG1Layer();

    static const sLayerConfig rgb0Setup[] = {
        {m2_CHCN, 1}, {m5_CHSZ, 1}, {m6_PNB, 1}, {m7_CNSM, 0},
        {m27_RPMD, 2}, {m11_SCN, 8},
        {m34_W0E, 1}, {m37_W0A, 1}, {m44_CCEN, 1}, {m45_COEN, 0},
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

    asyncDmaCopy(g_BTL_X0->getSaturnPtr(0x060b4918), getVdp2Cram(0xA00), 0x200, 0);
    asyncDmaCopy(g_BTL_X0->getSaturnPtr(0x060b4318), getVdp2Cram(0x800), 0x200, 0);

    loadFile("FRS_B5_C.SCB", getVdp2Vram(0x40000), 0);
    loadFile("FRS_B5_C.PNB", getVdp2Vram(0x60000), 0);
    loadFile("SPACE.PNB", getVdp2Vram(0x64000), 0);

    sVdp2PlaneTask_InitVdp2Sub4(g_BTL_X0->getSaturnPtr(0x060b63b8));

    vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL = (vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL & 0xFF00) | 0xB4;
    vdp2Controls.m4_pendingVdp2Regs->m10_CYCA0 = 0x310F754F;
    vdp2Controls.m_isDirty = 1;

    setupRotationMapPlanes(0, g_BTL_X0->getSaturnPtr(0x060b63f8));
    setupRotationMapPlanes(1, g_BTL_X0->getSaturnPtr(0x060b6438));

    setupVdp2Table(6, coefficientA0, coefficientA1, getVdp2Vram(0x20000), 0x80);
    setupVdp2Table(7, coefficientB0, coefficientB1, getVdp2Vram(0x22000), 0x80);
    initRotationCoefficientTables(5, getVdp2Vram(0x2A000));

    *(u16*)getVdp2Vram(0x2A400) = 0x700;
    vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA = (vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA & 0xFFF80000) | 0x15200;

    *(u16*)getVdp2Vram(0x2A600) = 0;
    vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA = (vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA & 0xFFF80000) | 0x15300;

    vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL = vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL & 0xFEFF;
    vdp2Controls.m4_pendingVdp2Regs->mF0_PRISA = 0x405;
    vdp2Controls.m4_pendingVdp2Regs->mF2_PRISB = 0x507;
    vdp2Controls.m4_pendingVdp2Regs->mF4_PRISC = 0x505;
    vdp2Controls.m4_pendingVdp2Regs->mF6_PRISD = 0x505;
    vdp2Controls.m4_pendingVdp2Regs->mF8_PRINA = 0x600;
    vdp2Controls.m4_pendingVdp2Regs->mFA_PRINB = 0x700;
    vdp2Controls.m4_pendingVdp2Regs->mFC_PRIR = 3;
    vdp2Controls.m_isDirty = 1;

    pThis->m3C_scale = 0x40000;

    static const std::vector<std::array<s32, 2>> layerDisplayConfig = {
        { {m44_CCEN, 0} }
    };
    applyLayerDisplayConfig(layerDisplayConfig);

    vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL = vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL & 0xFEFF;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xF8FF) | 0x400;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xFFF0) | 3;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xCFFF) | 0x1000;

    pThis->m50 = 0x0C;
    pThis->m51 = 0x0C;

    vdp2Controls.m4_pendingVdp2Regs->m100_CCRSA = pThis->m50 | (pThis->m51 << 8);
    vdp2Controls.m4_pendingVdp2Regs->m102_CCRSB = 0;
    vdp2Controls.m4_pendingVdp2Regs->m104_CCRSC = 0;
    vdp2Controls.m4_pendingVdp2Regs->m106_CCRSD = 0;
    vdp2Controls.m_isDirty = 1;

    pThis->m4C_wavePhase = 0;
    pThis->m40_waveSpeed = 0x182779;
    pThis->m44_waveFreq = (s32)0xFEE38EC6;
    pThis->m48 = 0x3848;

    allocateNPC(pThis, 10);
    Unimplemented(); // createSubTaskFromFunction for AD_EFF entity at 06055462
}

// 06054d8c
static void BTL_X0_env2_Update(sVdp2PlaneTask* pThis)
{
    s32 dragonY = gBattleManager->m10_battleOverlay->m18_dragon->m8_position.m4_Y;
    s32 heightRatio = FP_Div(dragonY, fixedPoint(0x20000)).getInteger();
    s8 brightness;
    if (0x10 - heightRatio < 1)
    {
        brightness = 0;
    }
    else
    {
        heightRatio = FP_Div(dragonY, fixedPoint(0x20000)).getInteger();
        brightness = (s8)(0x10 - heightRatio);
    }
    pThis->m55 = brightness;

    auto* pRegs = vdp2Controls.m4_pendingVdp2Regs;
    pRegs->m100_CCRSA = pThis->m50 | (pThis->m51 << 8);
    pRegs->m102_CCRSB = 0;
    pRegs->m104_CCRSC = 0;
    pRegs->m106_CCRSD = 0;
    vdp2Controls.m_isDirty = 1;

    vdp2Controls.m20_registers[0].m10C_CCRR = (s16)(s8)pThis->m55;

    if (pThis->m56 == 0)
    {
        vdp2Controls.m_isDirty = 1;
        vdp2Controls.m20_registers[1].m10C_CCRR = vdp2Controls.m20_registers[0].m10C_CCRR;
        return;
    }

    vdp2Controls.m20_registers[1].m10C_CCRR = vdp2Controls.m20_registers[0].m10C_CCRR;

    sSaturnPtr palA, palB;
    s32 state = pThis->m56 - 1;
    switch (state)
    {
    case 0:
    case 1:
        asyncDmaCopy(g_BTL_X0->getSaturnPtr(0x060b5318), getVdp2Cram(0x800), 0x200, 0);
        palB = g_BTL_X0->getSaturnPtr(0x060b5b18);
        break;
    case 2:
    case 4:
        asyncDmaCopy(g_BTL_X0->getSaturnPtr(0x060b5518), getVdp2Cram(0x800), 0x200, 0);
        palB = g_BTL_X0->getSaturnPtr(0x060b5d18);
        break;
    case 3:
    case 5:
        asyncDmaCopy(g_BTL_X0->getSaturnPtr(0x060b5118), getVdp2Cram(0x800), 0x200, 0);
        palB = g_BTL_X0->getSaturnPtr(0x060b5918);
        break;
    case 6:
    case 7:
        asyncDmaCopy(g_BTL_X0->getSaturnPtr(0x060b5718), getVdp2Cram(0x800), 0x200, 0);
        palB = g_BTL_X0->getSaturnPtr(0x060b5f18);
        break;
    case 8:
    case 0xF:
        asyncDmaCopy(g_BTL_X0->getSaturnPtr(0x060b5118), getVdp2Cram(0x800), 0x200, 0);
        asyncDmaCopy(g_BTL_X0->getSaturnPtr(0x060b5918), getVdp2Cram(0xA00), 0x200, 0);
        pThis->m56 = 0;
        return;
    case 9:
        asyncDmaCopy(g_BTL_X0->getSaturnPtr(0x060b5318), getVdp2Cram(0x800), 0x200, 0);
        asyncDmaCopy(g_BTL_X0->getSaturnPtr(0x060b5b18), getVdp2Cram(0xA00), 0x200, 0);
        pThis->m56++;
        return;
    case 10:
    case 0xB:
    case 0xC:
    case 0xE:
        asyncDmaCopy(g_BTL_X0->getSaturnPtr(0x060b5718), getVdp2Cram(0x800), 0x200, 0);
        palB = g_BTL_X0->getSaturnPtr(0x060b5f18);
        break;
    case 0xD:
        asyncDmaCopy(g_BTL_X0->getSaturnPtr(0x060b5518), getVdp2Cram(0x800), 0x200, 0);
        palB = g_BTL_X0->getSaturnPtr(0x060b5d18);
        break;
    default:
        return;
    }
    asyncDmaCopy(palB, getVdp2Cram(0xA00), 0x200, 0);
    pThis->m56++;
}

// 06054fd8
static void BTL_X0_env2_Draw(sVdp2PlaneTask* pThis)
{
    pThis->mC_cameraPosition = cameraProperties2.m0_position;
    pThis->m18_cameraRotation.m0_X = (s32)cameraProperties2.mC_rotation[0].m_value << 16;
    pThis->m18_cameraRotation.m4_Y = (s32)cameraProperties2.mC_rotation[1].m_value << 16;
    pThis->m18_cameraRotation.m8_Z = (s32)cameraProperties2.mC_rotation[2].m_value << 16;

    getVdp1ClippingCoordinates(pThis->m24_vdp1Clipping);
    getVdp1LocalCoordinates(pThis->m2C_vdp1LocalCoordinates);
    getVdp1ProjectionParams(&pThis->m30_vdp1ProjectionParam[0], &pThis->m30_vdp1ProjectionParam[1]);

    fixedPoint fov = FP_Div(pThis->m30_vdp1ProjectionParam[0], (s32)pThis->m30_vdp1ProjectionParam[1] << 16);

    beginRotationPass(0, fov);
    buildGroundRotation(pThis);
    drawCinematicBar(6);
    commitRotationPass();

    pThis->m34 = computeRotationScrollOffset();
    vdp2ApplyWaveDistortion((sVdp2PlaneTask*)pThis);

    pThis->m0_scrollX = (pThis->m18_cameraRotation.m4_Y >> 0xC) * -0x400;
    pThis->m4_scrollY = (0xFF - pThis->m34) * 0x10000;

    fov = FP_Div(pThis->m30_vdp1ProjectionParam[0], (s32)pThis->m30_vdp1ProjectionParam[1] << 16);
    beginRotationPass(1, fov);

    s32 coeffIdx = (s16)((gRotationPassState.m0_planeIndex * 2 + (s16)vdp2Controls.m0_doubleBufferIndex) * 0x70);
    s16 midX = (s16)(((s32)pThis->m24_vdp1Clipping[0] + (s32)pThis->m24_vdp1Clipping[2]) / 2);
    s16 midY = (s16)(((s32)pThis->m24_vdp1Clipping[1] + (s32)pThis->m24_vdp1Clipping[3]) / 2);

    *(s16*)((u8*)&gCoefficientTables[0][0].m34 + coeffIdx) = midX;
    *(s16*)((u8*)&gCoefficientTables[0][0].m36 + coeffIdx) = midY;
    *(s16*)((u8*)&gCoefficientTables[0][0].m38 + coeffIdx) = pThis->m30_vdp1ProjectionParam[1];
    *(s16*)((u8*)&gCoefficientTables[0][0].m3C + coeffIdx) = midX;
    *(s16*)((u8*)&gCoefficientTables[0][0].m3E + coeffIdx) = midY;
    *(s16*)((u8*)&gCoefficientTables[0][0].m40 + coeffIdx) = 0;

    buildRotationMatrixRoll(-pThis->m18_cameraRotation.m8_Z);
    intDivide(pThis->m24_vdp1Clipping[3] - pThis->m24_vdp1Clipping[1], 0xE00000);
}

// 0605535c
void Create_BTL_X0_env2(s_workAreaCopy* parent)
{
    static const sVdp2PlaneTask::TypedTaskDefinition def = {
        &BTL_X0_env2_Init,
        &BTL_X0_env2_Update,
        &BTL_X0_env2_Draw,
        nullptr,
    };
    createSubTask<sVdp2PlaneTask>((s_workArea*)parent, &def);
}
