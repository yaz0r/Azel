#include "PDS.h"
#include "o_fld_a5.h"
#include "field/field_a3/o_fld_a3.h"
#include "battle/BTL_A3/BTL_A3_map6.h"
#include "3dEngine.h"
#include "kernel/cinematicBarsTask.h"
#include "kernel/loadSavegameScreen.h"

// Forward declarations for free functions using shared sVdp2PlaneTask
static void a5VdpInit(sVdp2PlaneTask* pThis);
static void a5VdpUpdate(sVdp2PlaneTask* pThis);
static void a5VdpDraw(sVdp2PlaneTask* pThis);

// 0605C87E — shared palette loading for all A5 subfields
static void a5LoadPalettes()
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
        return;
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
}

// 0605CA44 — init palette animation buffer
static void a5InitPaletteAnimation(sVdp2PlaneTask* pThis)
{
    memcpy(pThis->m78_auxBuffer, getSaturnPtr(gFLD_A5->getSaturnPtr(0x0609A5C4)), 0x60);
    pThis->m77_animSpeed = 1;
}

// 0605CB34
static void a5VdpInit(sVdp2PlaneTask* pThis)
{
    getFieldTaskPtr()->m8_pSubFieldData->m350_fieldPaletteTask = pThis;
    reinitVdp2();
    initNBG1Layer();

    pThis->m78_auxBuffer = (u8*)allocateHeapForTask(pThis, 0x60);

    a5LoadPalettes();

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

    initRotationCoefficientTables(5, getVdp2Vram(0x2A000));
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

    a5InitPaletteAnimation(pThis);
}

// 0605CE28 — shared palette animation update + DMA
static void a5PaletteAnimUpdate(sVdp2PlaneTask* pThis)
{
    const u8* src = getSaturnPtr(gFLD_A5->getSaturnPtr(0x0609A5C4));
    const u8* lut = getSaturnPtr(gFLD_A5->getSaturnPtr(0x0609BBC4));

    u8* dst = pThis->m78_auxBuffer;
    s32 phase = pThis->m76_animPhase;
    s32 i = 0;

    do {
        for (int j = 0; j < 3; j++)
        {
            s32 entry = i + j;
            s32 lutIdx = READ_BE_S32(lut + (phase + entry) * 4);
            s32 srcOff = lutIdx * 2;
            s32 dstOff = entry * 2;

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

// 0605CF22 — night-only secondary palette animation (star/sky colors)
static void a5NightPaletteAnimUpdate()
{
    static const u32 animTableAddrs[] = {
        0x0609BDD0, 0x0609BDE4, 0x0609BDF8, 0x0609BE0C,
        0x0609BE20, 0x0609BE34, 0x0609BE48,
    };

    // field_C + 0x1C contains 7 animation state entries (8 bytes each: 4 bytes unused + 4 bytes frame counter)
    u8* pFieldData = (u8*)getFieldTaskPtr()->mC;
    u8* pAnimState = pFieldData + 0x1C;

    static u16 paletteBuffer[16]; // temporary buffer for 7 palette entries (padded)
    u16* pDst = paletteBuffer;

    for (s32 i = 0; i < 7; i++)
    {
        s32* pFrameCounter = (s32*)(pAnimState + 4);

        // Advance frame
        s32 frame = *pFrameCounter + 1;
        *pFrameCounter = frame;

        // Read animation table pointer
        const u8* animTable = getSaturnPtr(gFLD_A5->getSaturnPtr(animTableAddrs[i]));

        // Check for end of animation (value == 0)
        if (READ_BE_S16(animTable + frame * 2) == 0)
        {
            *pFrameCounter = 0;
        }

        // Write current palette value
        *pDst = READ_BE_U16(animTable + (*pFrameCounter) * 2);
        pDst++;
        pAnimState += 8;
    }

    asyncDmaCopy(paletteBuffer, getVdp2Cram(0xBC0), 0x20, 0);
}

// 0605CF7A
static void a5VdpUpdate(sVdp2PlaneTask* pThis)
{
    s_fieldSpecificData_A5* pFieldData = (s_fieldSpecificData_A5*)getFieldTaskPtr()->mC;
    if (pFieldData && pFieldData->m4_pExitEntity && pFieldData->m4_pExitEntity->m18 < 0x2555)
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

    a5PaletteAnimUpdate(pThis);
}

// 0605D018
static void a5VdpDraw(sVdp2PlaneTask* pThis)
{
    pThis->mC_cameraPosition = cameraProperties2.m0_position;
    pThis->m18_cameraRotation = cameraProperties2.mC_rotation.toSVec3_FP();

    getVdp1ClippingCoordinates(pThis->m24_vdp1Clipping);
    getVdp1LocalCoordinates(pThis->m2C_localCoordinates);
    getVdp1ProjectionParams(&pThis->m30_projParam0, &pThis->m32_projParam1);

    // Pass 0: ground plane
    beginRotationPass(0, intDivide(pThis->m30_projParam0, fixedPoint::fromInteger(pThis->m32_projParam1)));
    vdp2SetupRotationPass(pThis);
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
    gCurrentVDP2ScrollLayer = 0;
    setupVDP2CoordinatesIncrement2(pThis->m0_scrollX, nbg0ScrollY);
    gCurrentVDP2ScrollLayer = 4;

    auto* regs = vdp2Controls.m4_pendingVdp2Regs;
    regs->mC0_WPSX0 = pThis->m24_vdp1Clipping[0] << 1;
    regs->mC2_WPSY0 = pThis->m24_vdp1Clipping[1];
    regs->mC4_WPEX0 = pThis->m24_vdp1Clipping[2] << 1;
    regs->mC6_WPEY0 = pThis->m24_vdp1Clipping[3];
}

// 0605D492
void createA5Vdp2Task(p_workArea parent)
{
    static sVdp2PlaneTask::TypedTaskDefinition td = { &a5VdpInit, &a5VdpUpdate, &a5VdpDraw, nullptr };
    createSubTask<sVdp2PlaneTask>(parent, &td);
}

// --- Night VDP2 task (subfields 7, 9) ---

// 0605D568
static void a5NightVdpInit(sVdp2PlaneTask* pThis)
{
    getFieldTaskPtr()->m8_pSubFieldData->m350_fieldPaletteTask = pThis;
    reinitVdp2();
    initNBG1Layer();

    pThis->m78_auxBuffer = (u8*)allocateHeapForTask(pThis, 0x60);

    a5LoadPalettes();

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

    loadFile("FNS_A5_7.SCB", getVdp2Vram(0x10000), 0);
    loadFile("FNS_A5_7.PNB", getVdp2Vram(0x1E000), 0);
    loadFile("FRS_A5_7.SCB", getVdp2Vram(0x40000), 0);
    loadFile("FRS_A5_7.PNB", getVdp2Vram(0x60000), 0);

    vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL = (vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL & 0xFF00) | 0xB4;
    vdp2Controls.m4_pendingVdp2Regs->m10_CYCA0 = 0x310F7544;
    vdp2Controls.m_isDirty = 1;

    setupRotationMapPlanes(0, gFLD_A5->getSaturnPtr(0x0609BE10));
    setupRotationMapPlanes(1, gFLD_A5->getSaturnPtr(0x0609BE50));
    initLayerMap(0, 0x1E000, 0x1E000, 0x1E000, 0x1E000);
    setupVdp2Table(6, coefficientA0, coefficientA1, getVdp2Vram(0x20000), 0x80);
    setupVdp2Table(7, coefficientB0, coefficientB1, getVdp2Vram(0x22000), 0x80);

    initRotationCoefficientTables(5, getVdp2Vram(0x2A000));
    setVdp2VramU16(0x25E2A400, 0x700);
    vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA = (vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA & 0xFFF80000) | 0x15200;

    setVdp2VramU16(0x25E2A600, 0x8801);
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

    pThis->m75_colorRBG0 = 0x10;
    regs->m10C_CCRR = (s16)pThis->m75_colorRBG0;
    vdp2Controls.m_isDirty = 1;

    a5InitPaletteAnimation(pThis);
}

// 0605D838
static void a5NightVdpUpdate(sVdp2PlaneTask* pThis)
{
    s_fieldSpecificData_A5* pFieldData = (s_fieldSpecificData_A5*)getFieldTaskPtr()->mC;
    if (pFieldData && pFieldData->m4_pExitEntity && pFieldData->m4_pExitEntity->m18 < 0x2555)
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

    a5PaletteAnimUpdate(pThis);
    a5NightPaletteAnimUpdate();
}

// 0605DB14 — night ground plane rotation setup
static void a5NightSetupRotationPass0(sVdp2PlaneTask* pThis)
{
    sCoefficientTableData& coeff = gCoefficientTables[gRotationPassState.m0_planeIndex][(s32)vdp2Controls.m0_doubleBufferIndex];

    s32 camRotX = pThis->m18_cameraRotation.m0_X.asS32();
    if (camRotX == 0)
    {
        camRotX = -0xB60B6;
    }
    s32 camRotY = pThis->m18_cameraRotation.m4_Y.asS32();
    s32 camRotZ = pThis->m18_cameraRotation.m8_Z.asS32();

    // Compute pivot point (screen center)
    s32 iX = (s32)pThis->m24_vdp1Clipping[0] + (s32)pThis->m24_vdp1Clipping[2];
    coeff.m34 = (s16)((iX + (int)(iX < 0)) >> 1);
    s32 iY = (s32)pThis->m24_vdp1Clipping[1] + (s32)pThis->m24_vdp1Clipping[3];
    coeff.m36 = (s16)((iY + (int)(iY < 0)) >> 1);
    coeff.m38 = pThis->m32_projParam1;
    coeff.m3C = coeff.m34;
    coeff.m3E = coeff.m36;
    coeff.m40 = 0;

    buildRotationMatrixPitchYaw(-0x4000000 - camRotX, -camRotY);
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

    s32 scaledZ = MTH_Mul(pThis->m3C_scale, fixedPoint(pThis->mC_cameraPosition.m8_Z.asS32() << 4)).asS32();
    gVdp2RotationMatrix.My = fixedPoint(scaledZ
        - gVdp2RotationMatrix.m[1][0].asS32() * dPx
        - gVdp2RotationMatrix.m[1][1].asS32() * dPy
        - gVdp2RotationMatrix.m[1][2].asS32() * dPz
        + coeff.m3E * -0x10000);

    s32 heightDelta = (pThis->mC_cameraPosition.m4_Y.asS32() - pThis->m38_groundY) * 0x10;
    gVdp2RotationMatrix.Mz = fixedPoint(heightDelta
        - gVdp2RotationMatrix.m[2][0].asS32() * dPx
        - gVdp2RotationMatrix.m[2][1].asS32() * dPy
        - gVdp2RotationMatrix.m[2][2].asS32() * dPz
        + coeff.m40 * -0x10000);
}

// 0605D4EC — night scroll offset computation
static s32 a5NightComputeScrollOffset()
{
    sCoefficientTableData& coeff = gCoefficientTables[gRotationPassState.m0_planeIndex][(s32)vdp2Controls.m0_doubleBufferIndex];

    s32 term1 = gVdp2RotationMatrix.m[2][0].asS32() * coeff.m34;
    s32 term2 = gVdp2RotationMatrix.m[2][1].asS32() * coeff.m36;
    s32 term3 = MTH_Mul(gVdp2RotationMatrix.m[2][2], fixedPoint((s32)coeff.m38 * 0x10000 - coeff.m8_Zst)).asS32();

    return FP_Div(term1 + term2 + term3, gVdp2RotationMatrix.m[2][1]).getInteger();
}

// 0605D8FC
static void a5NightVdpDraw(sVdp2PlaneTask* pThis)
{
    pThis->mC_cameraPosition = cameraProperties2.m0_position;
    pThis->m18_cameraRotation = cameraProperties2.mC_rotation.toSVec3_FP();

    getVdp1ClippingCoordinates(pThis->m24_vdp1Clipping);
    getVdp1LocalCoordinates(pThis->m2C_localCoordinates);
    getVdp1ProjectionParams(&pThis->m30_projParam0, &pThis->m32_projParam1);

    // Pass 0: ground plane
    beginRotationPass(0, intDivide(pThis->m30_projParam0, fixedPoint::fromInteger(pThis->m32_projParam1)));
    a5NightSetupRotationPass0(pThis);
    drawCinematicBar(6);
    commitRotationPass();

    pThis->m34_scrollValue = a5NightComputeScrollOffset();

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
    t.m3C = t.m34;
    t.m3E = t.m36;
    t.m40 = 0;

    buildRotationMatrixRoll(-pThis->m18_cameraRotation.m8_Z);
    intDivide(pThis->m24_vdp1Clipping[3] - pThis->m24_vdp1Clipping[1], 0x00E00000);
    scaleRotationMatrix(intDivide(pThis->m24_vdp1Clipping[2] - pThis->m24_vdp1Clipping[0], 0x01600000));
    setRotationScrollOffset(pThis->m0_scrollX, pThis->m4_scrollY);
    commitRotationPass();

    // NBG0 scroll based on horizon (offset by 0x100 for night)
    s32 nbg0ScrollY;
    if (pThis->m34_scrollValue + 0x100 < -0x14)
    {
        nbg0ScrollY = 0x140000;
    }
    else
    {
        nbg0ScrollY = (pThis->m34_scrollValue + 0x100) * -0x10000;
    }
    gCurrentVDP2ScrollLayer = 0;
    setupVDP2CoordinatesIncrement2(pThis->m0_scrollX, nbg0ScrollY);
    gCurrentVDP2ScrollLayer = 4;

    auto* regs = vdp2Controls.m4_pendingVdp2Regs;
    regs->mC0_WPSX0 = pThis->m24_vdp1Clipping[0] << 1;
    regs->mC2_WPSY0 = pThis->m24_vdp1Clipping[1];
    regs->mC4_WPEX0 = pThis->m24_vdp1Clipping[2] << 1;
    regs->mC6_WPEY0 = pThis->m24_vdp1Clipping[3];
}

// 0605DCC4
void createA5NightVdp2Task(p_workArea parent)
{
    static sVdp2PlaneTask::TypedTaskDefinition td = { &a5NightVdpInit, &a5NightVdpUpdate, &a5NightVdpDraw, nullptr };
    createSubTask<sVdp2PlaneTask>(parent, &td);
}

// --- Subfield B VDP2 task (boss area) ---

// 0605DCEC
static void a5BossVdpInit(sVdp2PlaneTask* pThis)
{
    getFieldTaskPtr()->m8_pSubFieldData->m350_fieldPaletteTask = pThis;
    reinitVdp2();
    initNBG1Layer();

    asyncDmaCopy(gFLD_A5->getSaturnPtr(0x0609BF80), getVdp2Cram(0x400), 0x200, 0);
    asyncDmaCopy(gFLD_A5->getSaturnPtr(0x0609C180), getVdp2Cram(0xA00), 0x200, 0);

    static const sLayerConfig rgbSetup[] = {
        {m2_CHCN, 1}, {m5_CHSZ, 1}, {m6_PNB, 1}, {m7_CNSM, 0},
        {m27_RPMD, 2}, {m11_SCN, 8}, {m34_W0E, 1}, {m37_W0A, 1},
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

    loadFile("FRS_A3_0.SCB", getVdp2Vram(0x40000), 0);
    loadFile("FRS_A3_0.PNB", getVdp2Vram(0x60000), 0);
    loadFile("SPACE.PNB", getVdp2Vram(0x64000), 0);

    vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL = (vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL & 0xFF00) | 0xB4;
    vdp2Controls.m4_pendingVdp2Regs->m10_CYCA0 = 0x310F7544;
    vdp2Controls.m_isDirty = 1;

    setupRotationMapPlanes(0, gFLD_A5->getSaturnPtr(0x0609C3C0));
    setupRotationMapPlanes(1, gFLD_A5->getSaturnPtr(0x0609C380));
    setupVdp2Table(6, coefficientA0, coefficientA1, getVdp2Vram(0x20000), 0x80);
    setupVdp2Table(7, coefficientB0, coefficientB1, getVdp2Vram(0x22000), 0x80);

    initRotationCoefficientTables(5, getVdp2Vram(0x2A000));
    setVdp2VramU16(0x25E2A400, 0x700);
    vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA = (vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA & 0xFFF80000) | 0x15200;

    setVdp2VramU16(0x25E2A600, 0);
    vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA = (vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA & 0xFFF80000) | 0x15300;

    vdp2Controls.m4_pendingVdp2Regs->mF0_PRISA = 0x304;
    vdp2Controls.m4_pendingVdp2Regs->mF2_PRISB = 0x407;
    vdp2Controls.m4_pendingVdp2Regs->mF4_PRISC = 0x404;
    vdp2Controls.m4_pendingVdp2Regs->mF6_PRISD = 0x404;
    vdp2Controls.m4_pendingVdp2Regs->mF8_PRINA = 0x605;
    vdp2Controls.m4_pendingVdp2Regs->mFA_PRINB = 0x700;
    vdp2Controls.m4_pendingVdp2Regs->mFC_PRIR = 3;
    vdp2Controls.m_isDirty = 1;

    static const std::vector<std::array<s32, 2>> layerDisplay = {
        {m44_CCEN, 1},
    };
    applyLayerDisplayConfig(layerDisplay);

    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xF8FF) | 0x300;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xFFF0) | 3;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xCFFF) | 0x1000;

    pThis->m70_colorR = 0x10;
    pThis->m71_colorG = 0x12;
    pThis->m72_colorB = 0x14;

    auto* regs = vdp2Controls.m4_pendingVdp2Regs;
    regs->m100_CCRSA = (s16)pThis->m70_colorR | ((s16)pThis->m71_colorG << 8);
    regs->m102_CCRSB = (s16)pThis->m72_colorB;
    regs->m104_CCRSC = 0;
    regs->m106_CCRSD = 0;
    vdp2Controls.m_isDirty = 1;

    pThis->m3C_scale = fixedPoint(0x10000);
    pThis->m38_groundY = (s32)0xFFFF8000;
}

// 0605E1B4 — boss ground plane rotation setup (same as night version)
static void a5BossSetupRotationPass0(sVdp2PlaneTask* pThis)
{
    // Identical to a5NightSetupRotationPass0
    a5NightSetupRotationPass0(pThis);
}

// 0605DF68 — set sign bit on all coefficient entries (hides sky layer)
static void a5BossInvalidateCoefficients()
{
    u32* pCoeff = (u32*)(*gVdp2CoefficientTables[gRotationPassState.m0_planeIndex][(s32)vdp2Controls.m0_doubleBufferIndex]).data();
    for (s32 i = 0; i < 0x1A8; i++)
    {
        pCoeff[i] |= 0x80000000;
    }
}

// 0605DFC8
static void a5BossVdpDraw(sVdp2PlaneTask* pThis)
{
    pThis->mC_cameraPosition = cameraProperties2.m0_position;
    pThis->m18_cameraRotation = cameraProperties2.mC_rotation.toSVec3_FP();

    getVdp1ClippingCoordinates(pThis->m24_vdp1Clipping);
    getVdp1LocalCoordinates(pThis->m2C_localCoordinates);
    getVdp1ProjectionParams(&pThis->m30_projParam0, &pThis->m32_projParam1);

    // Pass 0: ground plane
    beginRotationPass(0, intDivide(pThis->m30_projParam0, fixedPoint::fromInteger(pThis->m32_projParam1)));
    a5BossSetupRotationPass0(pThis);
    drawCinematicBar(6);
    commitRotationPass();

    pThis->m34_scrollValue = computeRotationScrollOffset();
    a5BossInvalidateCoefficients();

    // Pass 1: sky scroll
    pThis->m0_scrollX = ((s32)pThis->m18_cameraRotation.m4_Y >> 0xC) * -0x400;
    pThis->m4_scrollY = (0x200 - pThis->m34_scrollValue) * 0x10000;

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

    auto* regs = vdp2Controls.m4_pendingVdp2Regs;
    regs->mC0_WPSX0 = pThis->m24_vdp1Clipping[0] << 1;
    regs->mC2_WPSY0 = pThis->m24_vdp1Clipping[1];
    regs->mC4_WPEX0 = pThis->m24_vdp1Clipping[2] << 1;
    regs->mC6_WPEY0 = pThis->m24_vdp1Clipping[3];
}

// 0605E354
void createA5_B_Vdp2Task(p_workArea parent)
{
    static sVdp2PlaneTask::TypedTaskDefinition td = { &a5BossVdpInit, nullptr, &a5BossVdpDraw, nullptr };
    createSubTask<sVdp2PlaneTask>(parent, &td);
}
