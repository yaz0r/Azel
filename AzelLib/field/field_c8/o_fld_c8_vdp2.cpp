#include "PDS.h"
#include "o_fld_c8.h"
#include "field/field_a3/o_fld_a3.h"
#include "battle/BTL_A3/BTL_A3_map6.h"
#include "3dEngine.h"
#include "kernel/cinematicBarsTask.h"
#include "kernel/loadSavegameScreen.h"

struct sC8Vdp2Task : public s_workAreaTemplateWithArg<sC8Vdp2Task, sSaturnPtr>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition td = { &sC8Vdp2Task::Init, &sC8Vdp2Task::Update, &sC8Vdp2Task::Draw, nullptr };
        return &td;
    }

    static void Init(sC8Vdp2Task* pThis, sSaturnPtr param);
    static void Update(sC8Vdp2Task* pThis);
    static void Draw(sC8Vdp2Task* pThis);

    s32 m0_scrollX;
    s32 m4_scrollY;
    u8 m8_pad[4];
    sVec3_FP mC_cameraPosition;
    sVec3_FP m18_cameraRotation;
    std::array<s16, 4> m24_vdp1Clipping;
    std::array<s16, 2> m2C_localCoordinates;
    s16 m30_projParam0;
    s16 m32_projParam1;
    s32 m34_scrollValue;
    s32 m38_groundY;
    fixedPoint m3C_scale;
    s32 m40_waveSpeed;
    s32 m44_waveFreq;
    s32 m48_waveAmplitude;
    s32 m4C_wavePhase;
    u8 m50_pad[0x20];
    s8 m70_colorR;
    s8 m71_colorG;
    u8 m72_pad[3];
    s8 m75_colorRBG0;
    u8 m76_pad[2];
    sSaturnPtr m78_towerDataPtr;
    // size 0x9C
};

// 0605f07c — VDP2 register setup
static void c8Vdp2Setup(sC8Vdp2Task* pThis)
{
    static const sLayerConfig rgbSetup[] = {
        {m2_CHCN, 1}, {m5_CHSZ, 1}, {m6_PNB, 1}, {m7_CNSM, 0},
        {m11_SCN, 8}, {m34_W0E, 1}, {m37_W0A, 1}, {m44_CCEN, 1},
        {m45_COEN, 0},
        {m0_END},
    };
    setupRGB0(rgbSetup);

    static const sLayerConfig rotParams[] = {
        {m31_RxKTE, 1}, {m13, 1},
        {m0_END},
    };
    setupRotationParams(rotParams);

    vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL = (vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL & 0xFF00) | 0xB4;
    vdp2Controls.m4_pendingVdp2Regs->m10_CYCA0 = 0x31FF75FF;
    vdp2Controls.m_isDirty = 1;

    setupRotationMapPlanes(0, gFLD_C8->getSaturnPtr(0x060B2AC4));
    setupVdp2Table(6, coefficientA0, coefficientA1, getVdp2Vram(0x20000), 0x80);
    setupVdp2Table(7, coefficientB0, coefficientB1, getVdp2Vram(0x24000), 0x80);

    s_BTL_A3_Env_InitVdp2Sub3(5, getVdp2Vram(0x28000));
    setVdp2VramU16(0x25E25000, 0x700);
    vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA = (vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA & 0xFFF80000) | 0x12800;

    setVdp2VramU16(0x25E25002, 0);
    vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA = (vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA & 0xFFF80000) | 0x12801;

    vdp2Controls.m4_pendingVdp2Regs->mF0_PRISA = 0x405;
    vdp2Controls.m4_pendingVdp2Regs->mF2_PRISB = 0x507;
    vdp2Controls.m4_pendingVdp2Regs->mF4_PRISC = 0x505;
    vdp2Controls.m4_pendingVdp2Regs->mF6_PRISD = 0x505;
    vdp2Controls.m4_pendingVdp2Regs->mF8_PRINA = 0x600;
    vdp2Controls.m4_pendingVdp2Regs->mFA_PRINB = 0x700;
    vdp2Controls.m4_pendingVdp2Regs->mFC_PRIR = 3;
    vdp2Controls.m_isDirty = 1;

    static const std::vector<std::array<s32, 2>> layerDisplay = {
        {m44_CCEN, 1},
    };
    applyLayerDisplayConfig(layerDisplay);

    vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL = vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL & 0xFEFF;

    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xF8FF) | 0x400;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xFFF0) | 3;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xCFFF) | 0x1000;

    pThis->m70_colorR = 0x10;
    pThis->m71_colorG = 0x10;
    pThis->m75_colorRBG0 = 0xE;

    auto* regs = vdp2Controls.m4_pendingVdp2Regs;
    regs->m100_CCRSA = (s16)pThis->m70_colorR | ((s16)pThis->m71_colorG << 8);
    regs->m102_CCRSB = 0;
    regs->m104_CCRSC = 0;
    regs->m106_CCRSD = 0;
    regs->m10C_CCRR = (s16)pThis->m75_colorRBG0;
    vdp2Controls.m_isDirty = 1;

    pThis->m4C_wavePhase = 0;
    pThis->m40_waveSpeed = 0;
    pThis->m44_waveFreq = 0;
    pThis->m48_waveAmplitude = 0;
    pThis->m38_groundY = 0;

    vdp2Controls.m4_pendingVdp2Regs->mB8_OVPNRA = 0x5000;
}

// 0605F5F8
void sC8Vdp2Task::Init(sC8Vdp2Task* pThis, sSaturnPtr param)
{
    getFieldTaskPtr()->m8_pSubFieldData->m350_fieldPaletteTask = (s_fieldPaletteTaskWorkArea*)pThis;
    reinitVdp2();
    initNBG1Layer();

    pThis->m78_towerDataPtr = param;

    asyncDmaCopy(gFLD_C8->getSaturnPtr(0x060B2B74), vdp2Palette, 0x200, 0);
    loadFile("FRS_T0_0.SCB", getVdp2Vram(0x40000), 0);
    loadFile("FRS_T0_0.PNB", getVdp2Vram(0x60000), 0);

    pThis->m3C_scale = fixedPoint(0x17333);

    c8Vdp2Setup(pThis);

    // 0605f876 — create fog/fade subtask
    Unimplemented(); // createSubTaskFromFunction(pThis, 0605f872, 0xC), stores towerData param
}

// 0605F662
void sC8Vdp2Task::Update(sC8Vdp2Task* pThis)
{
    // empty
}

// 0605f46c — standard rotation pass sub (takes rotation as separate param)
static void c8RotationMatrixSetup(sC8Vdp2Task* pThis, sVec3_FP* pRotation)
{
    sCoefficientTableData& t = gCoefficientTables[gRotationPassState.m0_planeIndex][(s32)vdp2Controls.m0_doubleBufferIndex];

    s32 sumX = (s32)pThis->m24_vdp1Clipping[0] + (s32)pThis->m24_vdp1Clipping[2];
    t.m34 = (s16)((sumX + (int)(sumX < 0)) >> 1);
    s32 sumY = (s32)pThis->m24_vdp1Clipping[1] + (s32)pThis->m24_vdp1Clipping[3];
    t.m36 = (s16)((sumY + (int)(sumY < 0)) >> 1);
    t.m38 = pThis->m32_projParam1;
    t.m3C = t.m34; t.m3E = t.m36; t.m40 = 0;

    buildRotationMatrixPitchYaw(-0x4000000 - pRotation->m0_X, -pRotation->m4_Y);
    scaleRotationMatrix(pThis->m3C_scale);
    writeRotationParams(-pRotation->m8_Z);

    s32 diffX = (s32)t.m34 - (s32)t.m3C;
    s32 diffY = (s32)t.m36 - (s32)t.m3E;
    s32 diffZ = (s32)t.m38 - (s32)t.m40;

    gVdp2RotationMatrix.Mx = MTH_Mul(pThis->m3C_scale, (s32)pThis->mC_cameraPosition.m0_X << 4)
        - gVdp2RotationMatrix.m[0][0] * diffX - gVdp2RotationMatrix.m[0][1] * diffY - gVdp2RotationMatrix.m[0][2] * diffZ
        + (s32)(s16)t.m3C * -0x10000;
    gVdp2RotationMatrix.My = MTH_Mul(pThis->m3C_scale, (s32)pThis->mC_cameraPosition.m8_Z << 4)
        - gVdp2RotationMatrix.m[1][0] * diffX - gVdp2RotationMatrix.m[1][1] * diffY - gVdp2RotationMatrix.m[1][2] * diffZ
        + (s32)(s16)t.m3E * -0x10000;
    gVdp2RotationMatrix.Mz = ((pThis->mC_cameraPosition.m4_Y - pThis->m38_groundY) * 0x10)
        - gVdp2RotationMatrix.m[2][0] * diffX - gVdp2RotationMatrix.m[2][1] * diffY - gVdp2RotationMatrix.m[2][2] * diffZ
        + (s32)(s16)t.m40 * -0x10000;
}

// 0605f6b0 — C8-specific rotation pass (adjusts camera relative to tower center, then calls 0605f46c)
static void c8RotationPassSub(sC8Vdp2Task* pThis)
{
    sVec3_FP rotation;
    if ((s32)pThis->m18_cameraRotation.m0_X == 0)
        rotation.m0_X = fixedPoint(0xFFF49F4A);
    else
        rotation.m0_X = pThis->m18_cameraRotation.m0_X;
    rotation.m4_Y = pThis->m18_cameraRotation.m4_Y;
    rotation.m8_Z = pThis->m18_cameraRotation.m8_Z;

    // Read tower data from Saturn overlay data via m78_towerDataPtr
    s32 towerX = readSaturnS32(pThis->m78_towerDataPtr);
    s32 towerZ = readSaturnS32(pThis->m78_towerDataPtr + 8);

    pThis->m3C_scale = fixedPoint(0x17333);
    pThis->mC_cameraPosition.m0_X = (s32)pThis->mC_cameraPosition.m0_X - (towerX - 0x2C234F);
    pThis->mC_cameraPosition.m8_Z = (s32)pThis->mC_cameraPosition.m8_Z + (0x2C234F - towerZ);

    s32 towerThresholdY = readSaturnS32(pThis->m78_towerDataPtr + 4);
    if ((s32)pThis->mC_cameraPosition.m4_Y < towerThresholdY)
    {
        pThis->m38_groundY = readSaturnS32(pThis->m78_towerDataPtr + 0x10);
    }
    else
    {
        pThis->m38_groundY = readSaturnS32(pThis->m78_towerDataPtr + 0x0C);
    }

    c8RotationMatrixSetup(pThis, &rotation);
}

// 0605F666
void sC8Vdp2Task::Draw(sC8Vdp2Task* pThis)
{
    // 0605f2a0 — copy camera state
    pThis->mC_cameraPosition = cameraProperties2.m0_position;
    pThis->m18_cameraRotation = cameraProperties2.mC_rotation.toSVec3_FP();

    getVdp1ClippingCoordinates(pThis->m24_vdp1Clipping);
    getVdp1LocalCoordinates(pThis->m2C_localCoordinates);
    getVdp1ProjectionParams(&pThis->m30_projParam0, &pThis->m32_projParam1);

    // Pass 0: ground plane with tower-specific camera adjustment
    beginRotationPass(0, intDivide(pThis->m30_projParam0, fixedPoint::fromInteger(pThis->m32_projParam1)));
    c8RotationPassSub(pThis);
    drawCinematicBar(6);
    commitRotationPass();

    pThis->m34_scrollValue = computeRotationScrollOffset();

    Unimplemented(); // 06080cac — horizon gauge VDP1 overlay (complex, calls sub-functions)

    pThis->m34_scrollValue = computeRotationScrollOffset();

    // Pass 1: sky scroll
    pThis->m0_scrollX = ((s32)pThis->m18_cameraRotation.m4_Y >> 0xC) * -0x400;
    pThis->m4_scrollY = (0xFF - pThis->m34_scrollValue) * 0x10000;

    beginRotationPass(1, intDivide(pThis->m30_projParam0, fixedPoint::fromInteger(pThis->m32_projParam1)));

    sCoefficientTableData& t = gCoefficientTables[gRotationPassState.m0_planeIndex][(s32)vdp2Controls.m0_doubleBufferIndex];
    s32 iX = (s32)pThis->m24_vdp1Clipping[0] + (s32)pThis->m24_vdp1Clipping[2];
    t.m34 = (s16)((iX + (int)(iX < 0)) >> 1);
    s32 iY = (s32)pThis->m24_vdp1Clipping[1] + (s32)pThis->m24_vdp1Clipping[3];
    t.m36 = (s16)((iY + (int)(iY < 0)) >> 1);
    t.m38 = pThis->m32_projParam1;
    t.m3C = t.m34; t.m3E = t.m36; t.m40 = 0;

    buildRotationMatrixRoll(-pThis->m18_cameraRotation.m8_Z);
    intDivide(pThis->m24_vdp1Clipping[3] - pThis->m24_vdp1Clipping[1], 0x00E00000);
    scaleRotationMatrix(intDivide(pThis->m24_vdp1Clipping[2] - pThis->m24_vdp1Clipping[0], 0x01600000));
    setRotationScrollOffset(pThis->m0_scrollX, pThis->m4_scrollY);
    commitRotationPass();

    auto* regs = vdp2Controls.m4_pendingVdp2Regs;
    regs->mC0_WPSX0 = pThis->m24_vdp1Clipping[0] << 1;
    regs->mC2_WPSY0 = pThis->m24_vdp1Clipping[1];
    regs->mC4_WPEX0 = pThis->m24_vdp1Clipping[2] << 1;
    regs->mC6_WPEY0 = pThis->m24_vdp1Clipping[3];
}

// 0605f794 — create C8 VDP2 task for subfield 0
void createC8Vdp2Task(p_workArea parent, sSaturnPtr towerData)
{
    createSubTaskWithArg<sC8Vdp2Task>(parent, towerData);
}
