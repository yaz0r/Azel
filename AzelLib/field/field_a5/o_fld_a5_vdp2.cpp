#include "PDS.h"
#include "o_fld_a5.h"
#include "field/field_a3/o_fld_a3.h"
#include "battle/BTL_A3/BTL_A3_map6.h"
#include "3dEngine.h"
#include "kernel/cinematicBarsTask.h"
#include "kernel/loadSavegameScreen.h"

struct sA5Vdp2Task : public s_workAreaTemplate<sA5Vdp2Task>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition td = { &sA5Vdp2Task::Init, &sA5Vdp2Task::Update, &sA5Vdp2Task::Draw, nullptr };
        return &td;
    }

    // 0605CB34
    static void Init(sA5Vdp2Task* pThis);
    // 0605CF7A
    static void Update(sA5Vdp2Task* pThis);
    // 0605D018
    static void Draw(sA5Vdp2Task* pThis);

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
    u8 m40_pad[0x30];
    s8 m70_colorR;
    s8 m71_colorG;
    u8 m72_pad[2];
    s8 m74_colorNBG;
    s8 m75_colorRBG0;
    s8 m76_animPhase;
    s8 m77_animSpeed;
    u8* m78_paletteBuffer; // 0x60 bytes, same layout as Saturn data at 0x0609A5C4
    // size 0x9C
};

// 0605D228
static void a5Vdp2DrawPass0Sub(sA5Vdp2Task* pThis)
{
    sCoefficientTableData& t = gCoefficientTables[gRotationPassState.m0_planeIndex][(s32)vdp2Controls.m0_doubleBufferIndex];

    fixedPoint rotX = pThis->m18_cameraRotation.m0_X;
    if ((s32)rotX == 0) rotX = fixedPoint(0xFFF49F4A);
    fixedPoint rotY = pThis->m18_cameraRotation.m4_Y;
    fixedPoint rotZ = pThis->m18_cameraRotation.m8_Z;

    s32 sumX = (s32)pThis->m24_vdp1Clipping[0] + (s32)pThis->m24_vdp1Clipping[2];
    t.m34 = (s16)((sumX + (int)(sumX < 0)) >> 1);
    s32 sumY = (s32)pThis->m24_vdp1Clipping[1] + (s32)pThis->m24_vdp1Clipping[3];
    t.m36 = (s16)((sumY + (int)(sumY < 0)) >> 1);
    t.m38 = pThis->m32_projParam1;
    t.m3C = t.m34;
    t.m3E = t.m36;
    t.m40 = 0;

    buildRotationMatrixPitchYaw(-0x4000000 - rotX, -rotY);
    scaleRotationMatrix(pThis->m3C_scale);
    writeRotationParams(-rotZ);

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

// 0605CB34
void sA5Vdp2Task::Init(sA5Vdp2Task* pThis)
{
    getFieldTaskPtr()->m8_pSubFieldData->m350_fieldPaletteTask = (s_fieldPaletteTaskWorkArea*)pThis;
    reinitVdp2();
    initNBG1Layer();

    pThis->m78_paletteBuffer = (u8*)allocateHeapForTask(pThis, 0x60);

    // 0605c87e — load palettes per subfield
    {
        asyncDmaCopy(gFLD_A5->getSaturnPtr(0x0609B644), getVdp2Cram(0x2A0), 0x20, 0);
        asyncDmaCopy(gFLD_A5->getSaturnPtr(0x0609A504), getVdp2Cram(0x2E0), 0x20, 0);

        s16 subfield = getFieldTaskPtr()->m2E_currentSubFieldIndex;
        switch (subfield)
        {
        case 0: case 2: case 4:
            asyncDmaCopy(gFLD_A5->getSaturnPtr(0x0609B664), getVdp2Cram(0), 0x20, 0);
            asyncDmaCopy(gFLD_A5->getSaturnPtr(0x0609A524), getVdp2Cram(0x240), 0x20, 0);
            asyncDmaCopy(gFLD_A5->getSaturnPtr(0x0609A584), getVdp2Cram(0x260), 0x20, 0);
            asyncDmaCopy(gFLD_A5->getSaturnPtr(0x0609A5A4), getVdp2Cram(0x280), 0x20, 0);
            asyncDmaCopy(gFLD_A5->getSaturnPtr(0x0609B684), getVdp2Cram(0x2C0), 0x20, 0);
            asyncDmaCopy(gFLD_A5->getSaturnPtr(0x0609A544), getVdp2Cram(0x300), 0x20, 0);
            asyncDmaCopy(gFLD_A5->getSaturnPtr(0x0609A564), getVdp2Cram(0x320), 0x20, 0);
            asyncDmaCopy(gFLD_A5->getSaturnPtr(0x0609A844), getVdp2Cram(0x800), 0x200, 0);
            asyncDmaCopy(gFLD_A5->getSaturnPtr(0x0609A644), getVdp2Cram(0xA00), 0x200, 0);
            break;
        case 7: case 8: case 9:
            asyncDmaCopy(gFLD_A5->getSaturnPtr(0x0609B764), getVdp2Cram(0), 0x20, 0);
            asyncDmaCopy(gFLD_A5->getSaturnPtr(0x0609B6C4), getVdp2Cram(0x240), 0x20, 0);
            asyncDmaCopy(gFLD_A5->getSaturnPtr(0x0609B6A4), getVdp2Cram(0x260), 0x20, 0);
            asyncDmaCopy(gFLD_A5->getSaturnPtr(0x0609B724), getVdp2Cram(0x280), 0x20, 0);
            asyncDmaCopy(gFLD_A5->getSaturnPtr(0x0609B744), getVdp2Cram(0x2C0), 0x20, 0);
            asyncDmaCopy(gFLD_A5->getSaturnPtr(0x0609B6E4), getVdp2Cram(0x300), 0x20, 0);
            asyncDmaCopy(gFLD_A5->getSaturnPtr(0x0609B704), getVdp2Cram(0x320), 0x20, 0);
            asyncDmaCopy(gFLD_A5->getSaturnPtr(0x0609B984), getVdp2Cram(0x800), 0x200, 0);
            asyncDmaCopy(gFLD_A5->getSaturnPtr(0x0609B784), getVdp2Cram(0xA00), 0x200, 0);
            break;
        default:
            if ((mainGameState.bitField[0xA3] & 0x20) == 0)
                asyncDmaCopy(gFLD_A5->getSaturnPtr(0x0609BB84), getVdp2Cram(0x200), 0x20, 0);
            else
                asyncDmaCopy(gFLD_A5->getSaturnPtr(0x0609BBA4), getVdp2Cram(0x200), 0x20, 0);
            goto skipCramA00;
        }

        // Per-subfield VDP2 CRAM block at 0x400
        switch (subfield)
        {
        case 0: asyncDmaCopy(gFLD_A5->getSaturnPtr(0x0609AA44), getVdp2Cram(0x400), 0x200, 0); break;
        case 2: asyncDmaCopy(gFLD_A5->getSaturnPtr(0x0609AC44), getVdp2Cram(0x400), 0x200, 0); break;
        case 4: asyncDmaCopy(gFLD_A5->getSaturnPtr(0x0609AE44), getVdp2Cram(0x400), 0x200, 0); break;
        case 7: asyncDmaCopy(gFLD_A5->getSaturnPtr(0x0609B044), getVdp2Cram(0x400), 0x200, 0); break;
        case 8: asyncDmaCopy(gFLD_A5->getSaturnPtr(0x0609B244), getVdp2Cram(0x400), 0x200, 0); break;
        case 9: asyncDmaCopy(gFLD_A5->getSaturnPtr(0x0609B444), getVdp2Cram(0x400), 0x200, 0); break;
        default: break;
        }
    skipCramA00:;
    }

    static const sLayerConfig rgbSetup[] = {
        {m2_CHCN, 1}, {m5_CHSZ, 1}, {m6_PNB, 1}, {m7_CNSM, 0},
        {m27_RPMD, 2}, {m11_SCN, 8}, {m34_W0E, 1}, {m37_W0A, 1},
        {m44_CCEN, 1},
        {m0_END},
    };
    setupRGB0(rgbSetup);

    static const sLayerConfig rotParams[] = {
        {m31_RxKTE, 1},
        {m0_END},
    };
    setupRotationParams(rotParams);

    static const sLayerConfig rotParams2[] = {
        {m0_END},
    };
    setupRotationParams2(rotParams2);

    static const sLayerConfig nbg0Setup[] = {
        {m2_CHCN, 1}, {m5_CHSZ, 1}, {m6_PNB, 1}, {m7_CNSM, 0},
        {m34_W0E, 1}, {m37_W0A, 1},
        {m0_END},
    };
    setupNBG0(nbg0Setup);

    loadFile("FNS_A5_0.SCB", getVdp2Vram(0x10000), 0);
    loadFile("FNS_A5_0.PNB", getVdp2Vram(0x1E000), 0);
    loadFile("FRS_A5_0.SCB", getVdp2Vram(0x40000), 0);
    loadFile("FRS_A5_0.PNB", getVdp2Vram(0x60000), 0);

    vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL = (vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL & 0xFF00) | 0xB4;
    vdp2Controls.m4_pendingVdp2Regs->m10_CYCA0 = 0x310F7544;
    vdp2Controls.m_isDirty = 1;

    setupRotationMapPlanes(0, gFLD_A5->getSaturnPtr(0x0609BBF4));
    setupRotationMapPlanes(1, gFLD_A5->getSaturnPtr(0x0609BC34));
    initLayerMap(0, 0x1E000, 0x1E000, 0x1E000, 0x1E000);
    setupVdp2Table(6, coefficientA0, coefficientA1, getVdp2Vram(0x20000), 0x80);
    setupVdp2Table(7, coefficientB0, coefficientB1, getVdp2Vram(0x22000), 0x80);

    s_BTL_A3_Env_InitVdp2Sub3(5, getVdp2Vram(0x2A000));
    setVdp2VramU16(0x25E2A400, 0x700);
    vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA = (vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA & 0xFFF80000) | 0x15200;

    setVdp2VramU16(0x25E2A600, 0x5E92);
    vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA = (vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA & 0xFFF80000) | 0x15300;

    vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL = vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL & 0xFEFF;

    vdp2Controls.m4_pendingVdp2Regs->mF0_PRISA = 0x405;
    vdp2Controls.m4_pendingVdp2Regs->mF2_PRISB = 0x507;
    vdp2Controls.m4_pendingVdp2Regs->mF4_PRISC = 0x505;
    vdp2Controls.m4_pendingVdp2Regs->mF6_PRISD = 0x505;
    vdp2Controls.m4_pendingVdp2Regs->mF8_PRINA = 0x602;
    vdp2Controls.m4_pendingVdp2Regs->mFA_PRINB = 0x700;
    vdp2Controls.m4_pendingVdp2Regs->mFC_PRIR = 3;
    vdp2Controls.m_isDirty = 1;

    pThis->m3C_scale = fixedPoint(0x40000);

    static const std::vector<std::array<s32, 2>> layerDisplay = {
        {m44_CCEN, 1},
    };
    applyLayerDisplayConfig(layerDisplay);

    vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL = vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL & 0xFEFF;

    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xF8FF) | 0x400;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xFFF0) | 3;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xCFFF) | 0x1000;

    pThis->m74_colorNBG = 0x1F;
    pThis->m70_colorR = 0x0D;
    pThis->m71_colorG = 0x18;

    auto* regs = vdp2Controls.m4_pendingVdp2Regs;
    regs->m100_CCRSA = (s16)pThis->m70_colorR | ((s16)pThis->m71_colorG << 8);
    regs->m102_CCRSB = 0;
    regs->m104_CCRSC = 0;
    regs->m106_CCRSD = 0;

    pThis->m75_colorRBG0 = 8;
    regs->m10C_CCRR = (s16)pThis->m75_colorRBG0;
    vdp2Controls.m_isDirty = 1;

    // 0605ca44 — init palette animation buffer as copy of source data
    {
        memcpy(pThis->m78_paletteBuffer, getSaturnPtr(gFLD_A5->getSaturnPtr(0x0609A5C4)), 0x60);
        pThis->m77_animSpeed = 1;
    }
}

// 0605CF7A
void sA5Vdp2Task::Update(sA5Vdp2Task* pThis)
{
    // Adjust animation speed based on field background data
    s_fieldSpecificData_A3* pFieldBg = (s_fieldSpecificData_A3*)getFieldTaskPtr()->mC;
    if (pFieldBg && *(s32*)((u8*)pFieldBg + 4 + 0x18) < 0x2555)
    {
        pThis->m77_animSpeed = 1;
    }
    else
    {
        pThis->m77_animSpeed = 2;
    }

    pThis->m76_animPhase = pThis->m76_animPhase + pThis->m77_animSpeed;
    if (pThis->m76_animPhase > 5)
    {
        pThis->m76_animPhase -= 6;
    }

    // Source data in Saturn memory (same layout as m78_paletteBuffer)
    const u8* src = getSaturnPtr(gFLD_A5->getSaturnPtr(0x0609A5C4));
    // Animation index LUT
    const u8* lut = getSaturnPtr(gFLD_A5->getSaturnPtr(0x0609BBC4));

    u8* dst = pThis->m78_paletteBuffer;
    s32 phase = pThis->m76_animPhase;
    s32 i = 0;

    do {
        for (int j = 0; j < 3; j++)
        {
            s32 entry = i + j;
            s32 lutIdx = READ_BE_S32(lut + (phase + entry) * 4);
            s32 srcOff = lutIdx * 2;
            s32 dstOff = entry * 2;

            // Copy one u16 from each of the 6 color tables into the palette buffer
            memcpy(dst + dstOff + 0x02, src + 0x00 + srcOff, 2);
            memcpy(dst + dstOff + 0x0E, src + 0x0C + srcOff, 2);
            memcpy(dst + dstOff + 0x22, src + 0x20 + srcOff, 2);
            memcpy(dst + dstOff + 0x2E, src + 0x2C + srcOff, 2);
            memcpy(dst + dstOff + 0x42, src + 0x40 + srcOff, 2);
            memcpy(dst + dstOff + 0x4E, src + 0x4C + srcOff, 2);
        }
        i += 3;
    } while (i < 6);

    asyncDmaCopy(dst, vdp2Palette, 0x60, 2);
}

// 0605D018
void sA5Vdp2Task::Draw(sA5Vdp2Task* pThis)
{
    pThis->mC_cameraPosition = cameraProperties2.m0_position;
    pThis->m18_cameraRotation = cameraProperties2.mC_rotation.toSVec3_FP();

    getVdp1ClippingCoordinates(pThis->m24_vdp1Clipping);
    getVdp1LocalCoordinates(pThis->m2C_localCoordinates);
    getVdp1ProjectionParams(&pThis->m30_projParam0, &pThis->m32_projParam1);

    // Pass 0: ground plane
    beginRotationPass(0, intDivide(pThis->m30_projParam0, fixedPoint::fromInteger(pThis->m32_projParam1)));
    a5Vdp2DrawPass0Sub(pThis);
    drawCinematicBar(6);
    commitRotationPass();

    pThis->m34_scrollValue = computeRotationScrollOffset();

    // Pass 1: sky scroll
    pThis->m0_scrollX = ((s32)pThis->m18_cameraRotation.m4_Y >> 0xC) * -0x400;
    pThis->m4_scrollY = (0x100 - pThis->m34_scrollValue) * 0x10000;

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
    setRotationScrollOffset(pThis->m0_scrollX, pThis->m4_scrollY);
    commitRotationPass();

    // NBG0 scroll based on horizon
    s32 nbg0ScrollY;
    if (pThis->m34_scrollValue < -0x14)
    {
        nbg0ScrollY = 0x140000;
    }
    else
    {
        nbg0ScrollY = pThis->m34_scrollValue * -0x10000;
    }
    pauseEngine[4] = 0;
    setupVDP2CoordinatesIncrement2(pThis->m0_scrollX, nbg0ScrollY);
    pauseEngine[4] = 4;

    auto* regs = vdp2Controls.m4_pendingVdp2Regs;
    regs->mC0_WPSX0 = pThis->m24_vdp1Clipping[0] << 1;
    regs->mC2_WPSY0 = pThis->m24_vdp1Clipping[1];
    regs->mC4_WPEX0 = pThis->m24_vdp1Clipping[2] << 1;
    regs->mC6_WPEY0 = pThis->m24_vdp1Clipping[3];
}

// 0605D492
void createA5Vdp2Task(p_workArea parent)
{
    createSubTask<sA5Vdp2Task>(parent);
}
