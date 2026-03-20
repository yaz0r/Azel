#include "PDS.h"
#include "campVdp2Plane.h"
#include "town/town.h"
#include "field/field_a3/o_fld_a3.h"
#include "kernel/loadSavegameScreen.h"
#include "kernel/cinematicBarsTask.h"
#include "battle/BTL_A3/BTL_A3_map6.h"
#include "3dEngine.h"

struct sCampVdp2Plane : public s_workAreaTemplate<sCampVdp2Plane>
{
    static void Init(sCampVdp2Plane* pThis);
    static void Init2(sCampVdp2Plane* pThis);
    static void Update(sCampVdp2Plane* pThis);
    static void Draw(sCampVdp2Plane* pThis);

    sVec3_FP mC_cameraPosition;
    sVec3_FP m18_cameraRotation;
    std::array<s16, 4> m24_vdp1Clipping;
    s32 m2C_scrollValue;
    s16 m30_projParam0;
    s16 m32_projParam1;
    s32 m34;
    fixedPoint m38;
    // size 0x40
};

void setupCampPlaneVdp2Regs(sCampVdp2Plane* pThis) {
    auto* regs = vdp2Controls.m4_pendingVdp2Regs;

    reinitVdp2();
    initNBG1Layer();

    asyncDmaCopy(gCurrentTownOverlay->getSaturnPtr(0x0607e958), getVdp2Cram(0x200), 0x20, 0);
    asyncDmaCopy(gCurrentTownOverlay->getSaturnPtr(0x0607e978), getVdp2Cram(0x220), 0x20, 0);

    static const sLayerConfig rgb0Setup[] = {
        m2_CHCN,  1,
        m5_CHSZ,  1,
        m6_PNB,   1,
        m7_CNSM,  0,
        m27_RPMD, 2,
        m11_SCN,  8,
        m34_W0E,  1,
        m37_W0A,  1,
        m44_CCEN, 1,
        m45_COEN, 0,
        m0_END,
    };
    setupRGB0(rgb0Setup);

    static const sLayerConfig rotationParams2Setup[] = {
        m0_END,
    };
    setupRotationParams2(rotationParams2Setup);

    regs->mE_RAMCTL = (regs->mE_RAMCTL & 0xff00) | 0xb4;
    regs->m10_CYCA0 = 0x13ff57ff;
    vdp2Controls.m_isDirty = 1;

    setupRotationMapPlanes(1, gCurrentTownOverlay->getSaturnPtr(0x0607ea38));
    setupVdp2Table(6, coefficientA0, coefficientA1, getVdp2Vram(0x20000), 0x80);
    s_BTL_A3_Env_InitVdp2Sub3(5, getVdp2Vram(0x24000));

    // setup line color screen: black
    regs->mA8_LCTA = (regs->mA8_LCTA & 0xFFF80000) | 0x12800;
    *(u16*)getVdp2Vram(0x25000) = 0;

    // setup back screen: black, per-line mode
    regs->mAC_BKTA = (regs->mAC_BKTA & 0xFFF80000) | 0x12801;
    *(u16*)getVdp2Vram(0x25002) = 0;

    regs->mF0_PRISA = 0x204;
    regs->mF2_PRISB = 0x407;
    regs->mF4_PRISC = 0x404;
    regs->mF6_PRISD = 0x404;
    regs->mF8_PRINA = 0x600;
    regs->mFA_PRINB = 0x700;
    regs->mFC_PRIR = 3;
    vdp2Controls.m_isDirty = 1;

    static const std::vector<std::array<s32, 2>> bgConfig = {
        {{0x2C, 1}},
    };
    applyLayerDisplayConfig(bgConfig);

    regs->mEC_CCCTL = (regs->mEC_CCCTL & 0xfeff) | 0x100;
    regs->mE0_SPCTL = (regs->mE0_SPCTL & 0xf8ff) | 0x200;
    regs->mE0_SPCTL = (regs->mE0_SPCTL & 0xfff0) | 3;
    regs->mE0_SPCTL = (regs->mE0_SPCTL & 0xcfff) | 0x1000;

    regs->m100_CCRSA = 0x10;
    regs->m102_CCRSB = 0;
    regs->m104_CCRSC = 0;
    regs->m106_CCRSD = 0;
    regs->m10C_CCRR = 0x10;
    vdp2Controls.m_isDirty = 1;

    pThis->m38 = 0x100000;
}

void sCampVdp2Plane::Init(sCampVdp2Plane* pThis) {
    setupCampPlaneVdp2Regs(pThis);
    setupRotationMapPlanes(0, gCurrentTownOverlay->getSaturnPtr(0x607e998));
    static const sLayerConfig rotationPrams[] =
    {
        m31_RxKTE, 1, // use coefficient table
        m0_END,
    };
    setupRotationParams(rotationPrams);

}

void sCampVdp2Plane::Init2(sCampVdp2Plane* pThis) {
    setupCampPlaneVdp2Regs(pThis);
    setupRotationMapPlanes(0, gCurrentTownOverlay->getSaturnPtr(0x607e9e4));
    static const sLayerConfig rotationPrams[] =
    {
        m31_RxKTE, 1, // use coefficient table
        m13, 1,
        m0_END,
    };
    setupRotationParams(rotationPrams);
    vdp2Controls.m4_pendingVdp2Regs->mB8_OVPNRA = 0;
}

static void setupCampRotationAndScroll(sCampVdp2Plane* pThis)
{
    fixedPoint rotX = pThis->m18_cameraRotation.m0_X;;
    fixedPoint rotY = pThis->m18_cameraRotation.m4_Y;
    fixedPoint rotZ = pThis->m18_cameraRotation.m8_Z;

    sCoefficientTableData& t = gCoefficientTables[gRotationPassState.m0_planeIndex][(s32)vdp2Controls.m0_doubleBufferIndex];

    s32 sumX = (s32)pThis->m24_vdp1Clipping[0] + (s32)pThis->m24_vdp1Clipping[2];
    t.m34 = (s16)((sumX + (int)(sumX < 0)) >> 1);
    s32 sumY = (s32)pThis->m24_vdp1Clipping[1] + (s32)pThis->m24_vdp1Clipping[3];
    t.m36 = (s16)((sumY + (int)(sumY < 0)) >> 1);
    t.m38 = pThis->m32_projParam1;
    t.m3C = t.m34;
    t.m3E = t.m36;
    t.m40 = 0;

    buildRotationMatrixPitchYaw(-0x4000000 - rotX, -rotY);
    scaleRotationMatrix(pThis->m38 / 4);
    writeRotationParams(-rotZ);

    s32 diffX = (s32)t.m34 - (s32)t.m3C;
    s32 diffY = (s32)t.m36 - (s32)t.m3E;
    s32 diffZ = (s32)t.m38 - (s32)t.m40;

    gVdp2RotationMatrix.Mx = MTH_Mul(pThis->m38, (s32)pThis->mC_cameraPosition.m0_X << 4)
                    - gVdp2RotationMatrix.m[0][0] * diffX - gVdp2RotationMatrix.m[0][1] * diffY - gVdp2RotationMatrix.m[0][2] * diffZ
                    + (s32)(s16)t.m3C * -0x10000;
    gVdp2RotationMatrix.My = MTH_Mul(pThis->m38, (s32)pThis->mC_cameraPosition.m8_Z << 4)
                    - gVdp2RotationMatrix.m[1][0] * diffX - gVdp2RotationMatrix.m[1][1] * diffY - gVdp2RotationMatrix.m[1][2] * diffZ
                    + (s32)(s16)t.m3E * -0x10000;
    gVdp2RotationMatrix.Mz = (pThis->mC_cameraPosition.m4_Y - pThis->m34) * 0x40
                    - gVdp2RotationMatrix.m[2][0] * diffX - gVdp2RotationMatrix.m[2][1] * diffY - gVdp2RotationMatrix.m[2][2] * diffZ
                    + (s32)(s16)t.m40 * -0x10000;
}

void sCampVdp2Plane::Update(sCampVdp2Plane* pThis) {
    pThis->m34 = 0;
}

void sCampVdp2Plane::Draw(sCampVdp2Plane* pThis) {
    pThis->mC_cameraPosition = cameraProperties2.m0_position;
    pThis->m18_cameraRotation = cameraProperties2.mC_rotation.toSVec3_FP();

    getVdp1ClippingCoordinates(pThis->m24_vdp1Clipping);
    getVdp1ProjectionParams(&pThis->m30_projParam0, &pThis->m32_projParam1);

    beginRotationPass(0, intDivide(pThis->m30_projParam0, fixedPoint::fromInteger(pThis->m32_projParam1)));
    setupCampRotationAndScroll(pThis);
    drawCinematicBar(6);
    commitRotationPass();

    pThis->m2C_scrollValue = computeRotationScrollOffset();
    s32 scrollX = (pThis->m18_cameraRotation.m4_Y >> 0xC) * -0x400;
    s32 scrollY = (0x1FF - pThis->m2C_scrollValue) * 0x10000;

    beginRotationPass(1, intDivide(pThis->m30_projParam0, fixedPoint::fromInteger(pThis->m32_projParam1)));

    sCoefficientTableData& t = gCoefficientTables[gRotationPassState.m0_planeIndex][(s32)vdp2Controls.m0_doubleBufferIndex];
    s32 iX = (s32)pThis->m24_vdp1Clipping[0] + (s32)pThis->m24_vdp1Clipping[2];
    t.m34 = (s16)((iX + (int)(iX < 0)) >> 1);
    s32 iY = (s32)pThis->m24_vdp1Clipping[1] + (s32)pThis->m24_vdp1Clipping[3];
    t.m36 = (s16)((iY + (int)(iY < 0)) >> 1);
    t.m38 = pThis->m32_projParam1;
    t.m3C = t.m34;
    t.m3E = t.m36;
    t.m40 = 0;

    buildRotationMatrixRoll(-pThis->m18_cameraRotation.m8_Z);
    intDivide(pThis->m24_vdp1Clipping[3] - pThis->m24_vdp1Clipping[1], 0x00E00000);
    scaleRotationMatrix(intDivide(pThis->m24_vdp1Clipping[2] - pThis->m24_vdp1Clipping[0], 0x01600000));
    setRotationScrollOffset(scrollX, scrollY);
    commitRotationPass();

    auto* regs = vdp2Controls.m4_pendingVdp2Regs;
    regs->mC0_WPSX0 = pThis->m24_vdp1Clipping[0] << 1;
    regs->mC2_WPSY0 = pThis->m24_vdp1Clipping[1];
    regs->mC4_WPEX0 = pThis->m24_vdp1Clipping[2] << 1;
    regs->mC6_WPEY0 = pThis->m24_vdp1Clipping[3];
}

void createCampVdp2Plane(p_workArea pParent) {
    static sCampVdp2Plane::TypedTaskDefinition definition = {
        &sCampVdp2Plane::Init,
        &sCampVdp2Plane::Update,
        &sCampVdp2Plane::Draw,
        nullptr,
    };
    createSubTask<sCampVdp2Plane>(pParent, &definition);
}


void createCampVdp2PlaneAlt(p_workArea pParent) {
    static sCampVdp2Plane::TypedTaskDefinition definition = {
        &sCampVdp2Plane::Init2,
        &sCampVdp2Plane::Update,
        &sCampVdp2Plane::Draw,
        nullptr,
    };
    createSubTask<sCampVdp2Plane>(pParent, &definition);
}
