#include "PDS.h"
#include "o_fld_d3.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldRadar.h"
#include "audio/soundDriver.h"
#include "battle/BTL_A3/BTL_A3_map6.h"
#include "3dEngine.h"
#include "kernel/cinematicBarsTask.h"
#include "kernel/loadSavegameScreen.h"

FLD_D3_data* gFLD_D3 = nullptr;

static const s_MCB_CGB fieldFileList_D3[] =
{
    { "FLDCMN.MCB", "FLDCMN.CGB" },     // 0
    { "FLD_D3_0.MCB", "FLD_D3_0.CGB" },  // 1
    { nullptr, nullptr },                  // 2 (empty)
    { (const char*)-1, nullptr }
};

struct s_fieldSpecificData_D3 : public s_workAreaTemplate<s_fieldSpecificData_D3>
{
    s32 mBC_scrollX;
    s32 mC0_scrollAddX;
    s32 mC4_scrollAddY;
    s32 mC8;
    // size 0x104
};

// 060541cc — create field-specific data task
static void createFieldSpecificDataTask_D3(p_workArea parent)
{
    s_fieldSpecificData_D3* pFieldData = createSubTaskFromFunction<s_fieldSpecificData_D3>(parent, nullptr);
    if (pFieldData)
    {
        getFieldTaskPtr()->mC = pFieldData;
        pFieldData->mC8 = 0;
    }
}

// 060556e8
static void fieldD3_0_startTasks(p_workArea workArea)
{
    createFieldSpecificDataTask_D3(workArea);

    Unimplemented(); // FUN_FLD_D3__06071ca4 — create background atmosphere subtask
    Unimplemented(); // FUN_FLD_D3__06054460 — create display/rendering subtask
    Unimplemented(); // FUN_FLD_D3__060546c4 — create environment object
    Unimplemented(); // FUN_FLD_D3__06055684 — create environment object
    Unimplemented(); // FUN_FLD_D3__0605826c — create environment object
    Unimplemented(); // FUN_FLD_D3__060583dc — create environment object
    Unimplemented(); // FUN_FLD_D3__06058c0c — create environment object
    Unimplemented(); // FUN_FLD_D3__06058110 — create environment object
}

// 06058216 — dragon light params for D3
static void initDragonParams_D3()
{
    s_dragonTaskWorkArea* p = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    p->mC8_normalLightColor = { 0xD, 0xC, 0xC };
    p->mCB_falloffColor0 = { 9, 9, 9 };
    p->mCE_falloffColor1 = { 0xE, 0xA, 0xA };
    p->mD1_falloffColor2 = { 0xE, 0xA, 0xA };
    p->mC0_lightRotationAroundDragon = fixedPoint(0xD555555);
    p->mC4 = fixedPoint(0xD555555);
}

// D3 VDP2 palette task
struct sD3Vdp2Task : public s_workAreaTemplate<sD3Vdp2Task>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition td = { &sD3Vdp2Task::Init, &sD3Vdp2Task::Update, &sD3Vdp2Task::Draw, nullptr };
        return &td;
    }

    static void Init(sD3Vdp2Task* pThis);
    static void Update(sD3Vdp2Task* pThis);
    static void Draw(sD3Vdp2Task* pThis);

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
    u8 m50_pad[0x24];
    s8 m74_colorNBG;
    u8 m75_pad[3];
    u8* m78_lineScrollBuffer; // 0xA00: [0x000] coeff buf 0, [0x400] coeff buf 1, [0x800] work buf
    // size 0x9C
};

// 06058468
static void d3InitWaveParams()
{
    s_fieldPaletteTaskWorkArea* pVdp2Task = getFieldTaskPtr()->m8_pSubFieldData->m350_fieldPaletteTask;
    if ((mainGameState.bitField[0xAC] & 0x20) == 0)
    {
        pVdp2Task->m4C_wavePhase = 0;
        pVdp2Task->m40_waveSpeed = 0;
        pVdp2Task->m44_waveFreq = 0;
        pVdp2Task->m48_waveAmplitude = 0;
    }
    else
    {
        pVdp2Task->m4C_wavePhase = 0;
        pVdp2Task->m40_waveSpeed = 0x5C390;
        pVdp2Task->m44_waveFreq = (s32)0xFE147BB8;
        pVdp2Task->m48_waveAmplitude = (s32)0xFFFFFD6F;
    }
}

// 060572a8
void sD3Vdp2Task::Init(sD3Vdp2Task* pThis)
{
    getFieldTaskPtr()->m8_pSubFieldData->m350_fieldPaletteTask = (s_fieldPaletteTaskWorkArea*)pThis;
    reinitVdp2();
    initNBG1Layer();

    pThis->m78_lineScrollBuffer = (u8*)allocateHeapForTask(pThis, 0xA00);

    asyncDmaCopy(gFLD_D3->getSaturnPtr(0x06087C1C), getVdp2Cram(0x400), 0x200, 0);
    asyncDmaCopy(gFLD_D3->getSaturnPtr(0x06087E1C), getVdp2Cram(0x600), 0x200, 0);
    asyncDmaCopy(gFLD_D3->getSaturnPtr(0x0608861C), getVdp2Cram(0xA00), 0x200, 0);
    asyncDmaCopy(gFLD_D3->getSaturnPtr(0x0608881C), vdp2Palette, 0x200, 0);
    asyncDmaCopy(gFLD_D3->getSaturnPtr(0x06088A1C), getVdp2Cram(0x800), 0x20, 0);

    static const sLayerConfig nbg0Setup[] = {
        {m1_TPEN, 0}, {m2_CHCN, 0}, {m5_CHSZ, 1}, {m6_PNB, 1}, {m7_CNSM, 0},
        {m34_W0E, 1}, {m37_W0A, 1}, {m44_CCEN, 1}, {m10_SPN, 4},
        {m0_END},
    };
    setupNBG0(nbg0Setup);

    static const sLayerConfig rgbSetup[] = {
        {m1_TPEN, 1}, {m2_CHCN, 1}, {m5_CHSZ, 1}, {m6_PNB, 1}, {m7_CNSM, 0},
        {m27_RPMD, 3}, {m11_SCN, 8}, {m34_W0E, 1}, {m37_W0A, 1},
        {m0_END},
    };
    setupRGB0(rgbSetup);

    static const sLayerConfig rotParams[] = {
        {m31_RxKTE, 1},
        {m0_END},
    };
    setupRotationParams(rotParams);

    static const sLayerConfig rotParams2[] = {
        {m31_RxKTE, 1},
        {m0_END},
    };
    setupRotationParams2(rotParams2);

    static const sLayerConfig wctld[] = {
        {(eVdp2LayerConfig)0x23, 1}, {(eVdp2LayerConfig)0x26, 1},
        {m0_END},
    };
    setupWCTLD(wctld);

    loadFile("FNS_D2_C.SCB", getVdp2Vram(0x10000), 0);
    loadFile("FNS_D2_C.PNB", getVdp2Vram(0x1E000), 0);
    loadFile("FRS_D2_1.SCB", getVdp2Vram(0x40000), 0);
    loadFile("FRS_D2_1.PNB", getVdp2Vram(0x60000), 0);

    initLayerMap(0, 0x1E000, 0x1E000, 0x1E000, 0x1E000);

    vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL = (vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL & 0xFF00) | 0xB4;
    vdp2Controls.m4_pendingVdp2Regs->m10_CYCA0 = 0x310F7544;
    vdp2Controls.m_isDirty = 1;

    setupRotationMapPlanes(0, gFLD_D3->getSaturnPtr(0x06088A88));
    setupRotationMapPlanes(1, gFLD_D3->getSaturnPtr(0x06088AC8));
    setupVdp2Table(6, coefficientA0, coefficientA1, getVdp2Vram(0x20000), 0x80);
    setupVdp2Table(7, coefficientB0, coefficientB1, getVdp2Vram(0x22000), 0x80);

    s_BTL_A3_Env_InitVdp2Sub3(5, getVdp2Vram(0x2A000));
    setVdp2VramU16(0x25E2A400, 0x700);
    vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA = (vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA & 0xFFF80000) | 0x15200;

    setVdp2VramU16(0x25E2A600, 0x8885);
    vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA = (vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA & 0xFFF80000) | 0x15300;

    vdp2Controls.m4_pendingVdp2Regs->mF0_PRISA = 0x404;
    vdp2Controls.m4_pendingVdp2Regs->mF2_PRISB = 0x407;
    vdp2Controls.m4_pendingVdp2Regs->mF4_PRISC = 0x404;
    vdp2Controls.m4_pendingVdp2Regs->mF6_PRISD = 0x404;
    vdp2Controls.m4_pendingVdp2Regs->mF8_PRINA = 0x605;
    vdp2Controls.m4_pendingVdp2Regs->mFA_PRINB = 0x700;
    vdp2Controls.m4_pendingVdp2Regs->mFC_PRIR = 3;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xFFF0) | 3;
    vdp2Controls.m_isDirty = 1;

    pThis->m3C_scale = fixedPoint(0x10000);

    pThis->m38_groundY = 0;

    d3InitWaveParams();
    // NBG3 line scroll setup
    setupScrollAndRotation(0xB, pThis->m78_lineScrollBuffer, pThis->m78_lineScrollBuffer + 0x400, getVdp2Vram(0x3F000), 0x40);

    // Enable line scroll
    vdp2Controls.m4_pendingVdp2Regs->mDC_LWTA1 = (vdp2Controls.m4_pendingVdp2Regs->mDC_LWTA1 & 0x7FFFFFFF) | 0x80000000;

    // Clear line scroll buffer
    u8* lineScrollBuf = pThis->m78_lineScrollBuffer + 0x800;
    for (int i = 0; i < 0x80; i++)
    {
        *(u16*)(lineScrollBuf + i * 4) = 0;
        *(u16*)(lineScrollBuf + i * 4 + 2) = 0;
    }

    vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL = vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL & 0xFEFF;
    vdp2Controls.m_isDirty = 1;

    pThis->m74_colorNBG = 0x1F;
}

// 06057a7c — compute line scroll gradient from rotation angle
static fixedPoint d3LineScrollStep(fixedPoint angle)
{
    u16 idx = ((u32)angle >> 16) & 0xFFF;
    s32 cosVal = getCos(idx);
    s32 sinVal = getSin(idx);
    s32 absCos = cosVal < 0 ? -cosVal : cosVal;
    s32 absSin = sinVal < 0 ? -sinVal : sinVal;

    if (absSin < absCos)
        return FP_Div(getSin(idx), getCos(idx));
    else
        return FP_Div(getCos(idx), getSin(idx));
}

// 060579dc — compute starting line offset (variant 1, for 06057af2)
static fixedPoint d3LineScrollStart1(s32 param_1, fixedPoint angle, s32 scrollValue)
{
    sD3Vdp2Task* pVdp2 = (sD3Vdp2Task*)getFieldTaskPtr()->m8_pSubFieldData->m350_fieldPaletteTask;
    u16 idx = ((u32)angle >> 16) & 0xFFF;
    s32 localY = (s32)pVdp2->m2C_localCoordinates[1];
    s32 localX = (s32)pVdp2->m2C_localCoordinates[0];

    return FP_Div(
        (getSin(idx) * param_1 - localX * getSin(idx)) + localY * getCos(idx) + (scrollValue - localY) * 0x10000,
        getCos(idx));
}

// 06057974 — compute starting line offset (variant 2, for 06057c9a)
static fixedPoint d3LineScrollStart2(s32 param_1, fixedPoint angle, s32 scrollValue)
{
    sD3Vdp2Task* pVdp2 = (sD3Vdp2Task*)getFieldTaskPtr()->m8_pSubFieldData->m350_fieldPaletteTask;
    u16 idx = ((u32)angle >> 16) & 0xFFF;
    s32 localY = (s32)pVdp2->m2C_localCoordinates[1];
    s32 localX = (s32)pVdp2->m2C_localCoordinates[0];

    return FP_Div(
        (getCos(idx) * param_1 + localX * getSin(idx)) - localY * getCos(idx) + (scrollValue - localY) * -0x10000,
        getSin(idx));
}

// 06057a44 — fill line scroll coefficient buffer with constant
static void d3LineScrollFillConst(s16* buf, s16 value)
{
    s32 count = 0x100;
    do {
        count -= 2;
        buf[0] = value << 1;
        buf[2] = value << 1;
        buf += 4;
    } while (count != 0);
}

// 06057a5c — copy from internal line buffer to VDP2 coefficient buffer
static void d3LineScrollCopyToCoeff(s16* src, s16* dst)
{
    s32 count = 0x100;
    do {
        count -= 2;
        dst[0] = src[0] << 1;
        dst[2] = src[1] << 1;
        dst += 4;
        src += 2;
    } while (count != 0);
}

// 06057af2 — compute line scroll mapping (variant 1)
static void d3LineScrollCompute1(s16* buf, fixedPoint angle, fixedPoint scrollValue)
{
    fixedPoint step = d3LineScrollStep(angle);
    fixedPoint pos = d3LineScrollStart1(0, angle, scrollValue);
    s32 startLine = (s32)(u16)((u32)((s32)pos + 0x8000) >> 16);

    if ((s32)step < 0)
    {
        if (startLine < 0x100)
        {
            if (startLine < 0) startLine = 0;
            for (s16* p = (s16*)(startLine * 2 + (u8*)buf); p < (s16*)((u8*)buf + 0x200); p++)
                *p = (s16)0xFFFF;
        }
    }
    else if (startLine > 0)
    {
        s32 limit = startLine < 0x100 ? startLine : 0x100;
        for (s32 i = 0; i < limit; i++)
            buf[i] = (s16)0xFFFF;
    }

    for (s32 line = 0; line < 0x15F; line += 4)
    {
        s16 screenY = (s16)((u32)((s32)pos + 0x8000) >> 16);
        if ((s32)pos + 0x8000 >= 0 && screenY < 0x100)
            buf[screenY] = (s16)line;

        pos = pos + step;
        screenY = (s16)((u32)((s32)pos + 0x8000) >> 16);
        if ((s32)pos + 0x8000 >= 0 && screenY < 0x100)
            buf[screenY] = (s16)(line + 1);

        pos = pos + step;
        screenY = (s16)((u32)((s32)pos + 0x8000) >> 16);
        if ((s32)pos + 0x8000 >= 0 && screenY < 0x100)
            buf[screenY] = (s16)(line + 2);

        pos = pos + step;
        screenY = (s16)((u32)((s32)pos + 0x8000) >> 16);
        if ((s32)pos + 0x8000 >= 0 && screenY < 0x100)
            buf[screenY] = (s16)(line + 3);

        pos = pos + step;
    }

    for (s32 line = 0x15F; line < 0x163; line++)
    {
        s16 screenY = (s16)((u32)((s32)pos + 0x8000) >> 16);
        if ((s32)pos + 0x8000 >= 0 && screenY < 0x100)
            buf[screenY] = (s16)line;
        pos = pos + step;
    }

    s16 endLine = (s16)((u32)((s32)pos + 0x8000) >> 16);
    if ((s32)step < 0)
    {
        if (endLine > 0)
        {
            s32 limit = endLine < 0x100 ? (s32)endLine : 0x100;
            for (s32 i = 0; i < limit; i++)
                buf[i] = 0x161;
        }
    }
    else if (endLine < 0x100)
    {
        s32 start = (s32)endLine;
        if (start < 0) start = 0;
        for (s16* p = buf + start; p < buf + 0x100; p++)
            *p = 0x161;
    }
}

// 06057c9a — compute line scroll mapping (variant 2)
static void d3LineScrollCompute2(s16* buf, fixedPoint angle, fixedPoint scrollValue)
{
    fixedPoint step = d3LineScrollStep(angle);
    fixedPoint pos = d3LineScrollStart2(0, angle, scrollValue) + 0x8000;

    for (s32 i = 0; i < 0x100; i += 4)
    {
        for (s32 j = 0; j < 4; j++)
        {
            s16 screenY = (s16)((u32)(s32)pos >> 16);
            if ((s32)pos < 0)
                buf[i + j] = -1;
            else if (screenY < 0x161)
                buf[i + j] = screenY;
            else
                buf[i + j] = 0x161;
            pos = pos + step;
        }
    }
}

// 06057d88 — main line scroll dispatch based on camera rotation (standalone function)
static void d3LineScrollEffect(sD3Vdp2Task* pThis, s16* coeffBuf, u32 rotZ, s32 scrollValue)
{
    u32 angle = rotZ & 0x0FFFFFFF;
    s16* lineBuf = (s16*)(pThis->m78_lineScrollBuffer + 0x800);
    s16* pass1Buf = coeffBuf + 1; // coeffBuf + 2 bytes

    if (angle < 0x2000001)
    {
        d3LineScrollFillConst(coeffBuf, 0);
        d3LineScrollCompute1(lineBuf, rotZ, scrollValue);
        d3LineScrollCopyToCoeff(lineBuf, coeffBuf);
    }
    else if (angle < 0x4000001)
    {
        d3LineScrollFillConst(coeffBuf, 0);
        d3LineScrollCompute2(lineBuf, rotZ, scrollValue);
        d3LineScrollCopyToCoeff(lineBuf, pass1Buf);
    }
    else if (angle < 0x6000001)
    {
        d3LineScrollFillConst(coeffBuf, 0);
        d3LineScrollCompute2(lineBuf, rotZ, scrollValue);
    }
    else if (angle < 0x8000001)
    {
        d3LineScrollFillConst(coeffBuf, 0);
        d3LineScrollCompute1(lineBuf, rotZ, scrollValue);
    }
    else if (angle < 0xA000001)
    {
        d3LineScrollFillConst(pass1Buf, 0x160);
        d3LineScrollCompute1(lineBuf, rotZ, scrollValue);
        d3LineScrollCopyToCoeff(lineBuf, coeffBuf);
    }
    else if (angle < 0xC000001)
    {
        d3LineScrollFillConst(pass1Buf, 0x160);
        d3LineScrollCompute2(lineBuf, rotZ, scrollValue);
        d3LineScrollCopyToCoeff(lineBuf, coeffBuf);
    }
    else if (angle < 0xE000001)
    {
        d3LineScrollFillConst(pass1Buf, 0x160);
        d3LineScrollCompute2(lineBuf, rotZ, scrollValue);
        d3LineScrollCopyToCoeff(lineBuf, coeffBuf);
    }
    else
    {
        d3LineScrollFillConst(pass1Buf, 0x160);
        d3LineScrollCompute1(lineBuf, rotZ, scrollValue);
        d3LineScrollCopyToCoeff(lineBuf, coeffBuf);
    }

    drawCinematicBar(0xB);
}

// 060577c0 — rotation pass sub (shared between pass 0 and 1 with Y offset)
static void d3VdpRotationPassSub(sD3Vdp2Task* pThis, s32 yOffset)
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
    t.m3C = t.m34; t.m3E = t.m36; t.m40 = 0;

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
    gVdp2RotationMatrix.Mz = ((pThis->mC_cameraPosition.m4_Y - pThis->m38_groundY + yOffset) * 0x10)
        - gVdp2RotationMatrix.m[2][0] * diffX - gVdp2RotationMatrix.m[2][1] * diffY - gVdp2RotationMatrix.m[2][2] * diffZ
        + (s32)(s16)t.m40 * -0x10000;
}

// 06057210 — wave distortion on coefficient table
static void d3VdpWaveDistortion(sD3Vdp2Task* pThis)
{
    std::vector<fixedPoint>& coefficients = *gVdp2CoefficientTables[gRotationPassState.m0_planeIndex][vdp2Controls.m0_doubleBufferIndex];
    s32 phase = pThis->m4C_wavePhase;
    for (int i = 0; i < 0x1A8 && i < (int)coefficients.size(); i++)
    {
        s32 sinVal = getSin((u16)((u32)phase >> 16) & 0xFFF);
        fixedPoint modulated = MTH_Mul(pThis->m48_waveAmplitude, sinVal);
        coefficients[i] = MTH_Mul(coefficients[i], modulated + 0x10000);
        phase += pThis->m44_waveFreq;
    }
    pThis->m4C_wavePhase += pThis->m40_waveSpeed;
}

// 06057590
void sD3Vdp2Task::Update(sD3Vdp2Task* pThis)
{
    vdp2Controls.m20_registers[0].m108_CCRNA = (vdp2Controls.m4_pendingVdp2Regs->m108_CCRNA & 0xFFE0) | (s16)pThis->m74_colorNBG;
    vdp2Controls.m20_registers[1].m108_CCRNA = vdp2Controls.m20_registers[0].m108_CCRNA;
}

// 06057628
void sD3Vdp2Task::Draw(sD3Vdp2Task* pThis)
{
    pThis->mC_cameraPosition = cameraProperties2.m0_position;
    pThis->m18_cameraRotation = cameraProperties2.mC_rotation.toSVec3_FP();

    getVdp1ClippingCoordinates(pThis->m24_vdp1Clipping);
    getVdp1LocalCoordinates(pThis->m2C_localCoordinates);
    getVdp1ProjectionParams(&pThis->m30_projParam0, &pThis->m32_projParam1);

    // Pass 0: ground plane
    beginRotationPass(0, performDivision(pThis->m30_projParam0, fixedPoint::fromInteger(pThis->m32_projParam1)));
    d3VdpRotationPassSub(pThis, 0);
    drawCinematicBar(6);
    commitRotationPass();

    pThis->m34_scrollValue = computeRotationScrollOffset();
    d3VdpWaveDistortion(pThis);

    // Pass 1: ceiling/upper plane (offset by -0x80000 in Y)
    beginRotationPass(1, performDivision(pThis->m30_projParam0, fixedPoint::fromInteger(pThis->m32_projParam1)));
    d3VdpRotationPassSub(pThis, (s32)0xFFF80000);
    drawCinematicBar(7);
    commitRotationPass();

    // 06057d88 — line scroll effect (standalone function)
    d3LineScrollEffect(pThis,
        (s16*)(pThis->m78_lineScrollBuffer + (s32)vdp2Controls.m0_doubleBufferIndex * 0x400),
        (u32)pThis->m18_cameraRotation.m8_Z,
        pThis->m34_scrollValue);

    // NBG0 scroll from field-specific data
    pauseEngine[4] = 0;
    s_fieldSpecificData_D3* pFieldBg = (s_fieldSpecificData_D3*)getFieldTaskPtr()->mC;
    if (pFieldBg)
    {
        setupVDP2CoordinatesIncrement2(pFieldBg->mBC_scrollX, pFieldBg->mC0_scrollAddX + pFieldBg->mC4_scrollAddY);
    }
    pauseEngine[4] = 4;

    auto* regs = vdp2Controls.m4_pendingVdp2Regs;
    regs->mC0_WPSX0 = pThis->m24_vdp1Clipping[0] << 1;
    regs->mC2_WPSY0 = pThis->m24_vdp1Clipping[1];
    regs->mC4_WPEX0 = pThis->m24_vdp1Clipping[2] << 1;
    regs->mC6_WPEY0 = pThis->m24_vdp1Clipping[3];

    // Window 1 = same as window 0
    regs->mC8_WPSX1 = pThis->m24_vdp1Clipping[0] << 1;
    regs->mCA_WPSY1 = pThis->m24_vdp1Clipping[1];
    regs->mCC_WPEX1 = pThis->m24_vdp1Clipping[2] << 1;
    regs->mCE_WPEY1 = pThis->m24_vdp1Clipping[3];
}

static void createD3Vdp2Task(p_workArea parent)
{
    createSubTask<sD3Vdp2Task>(parent);
}

// 0605404A
static void subfieldD3_0(p_workArea workArea)
{
    playPCM(workArea, 100);

    s_DataTable3* pDT3 = readDataTable3(gFLD_D3->getSaturnPtr(0x06086998));
    setupField2(pDT3, fieldD3_0_startTasks);

    graphicEngineStatus.m405C.m10_nearClipDistance = 0x3000;
    graphicEngineStatus.m405C.m30_oneOverNearClip = FP_Div(0x10000, graphicEngineStatus.m405C.m10_nearClipDistance);

    graphicEngineStatus.m405C.m14_farClipDistance = 0x258000;
    graphicEngineStatus.m405C.m38_oneOverFarClip = FP_Div(0x8000, graphicEngineStatus.m405C.m14_farClipDistance);
    graphicEngineStatus.m405C.m34_oneOverFarClip256 = graphicEngineStatus.m405C.m38_oneOverFarClip << 8;

    // Camera config
    setupFieldCameraConfigs(readCameraConfig(gFLD_D3->getSaturnPtr(0x0607AEBC)), 1);

    // Camera script based on entry point
    s16 entryPoint = getFieldTaskPtr()->m30_fieldEntryPoint;
    if (entryPoint >= 1 && entryPoint <= 4)
    {
        sSaturnPtr cutsceneAddr = readSaturnEA(gFLD_D3->getSaturnPtr(0x06089770 + (entryPoint - 1) * 4));
        startCutscene(loadCutsceneData(cutsceneAddr));
    }
    else if (entryPoint == 8)
    {
        startCutscene(loadCutsceneData(gFLD_D3->getSaturnPtr(0x060890E0)));
    }
    else if (entryPoint == 9)
    {
        startCutscene(loadCutsceneData(gFLD_D3->getSaturnPtr(0x06088F18)));
    }

    initFieldDragonLight();
    adjustVerticalLimits(0x5000, 0x78000);
    fieldRadar_enableAltitudeGauge();
    initDragonParams_D3();
    createD3Vdp2Task(workArea);

    // 060544b0 — setup 3 camera configs from overlay data
    {
        static s_fieldCameraConfig configs[3];
        for (int i = 0; i < 3; i++)
        {
            s_fieldCameraConfig* p = readCameraConfig(gFLD_D3->getSaturnPtr(0x0607AF20 + i * 0x58));
            configs[i] = *p;
            delete p;
        }
        setupFieldCameraConfigs(configs, 3);
    }

    startFieldScript(8, -1);
    Unimplemented(); // 0606c174 — random battle encounter init (script 9)
}

// 06054000
p_workArea overlayStart_FLD_D3(p_workArea workArea, u32 arg)
{
    if (gFLD_D3 == nullptr)
    {
        gFLD_D3 = new FLD_D3_data();
    }

    if (gFLD_A3 == nullptr)
    {
        gFLD_A3 = new FLD_A3_data();
    }

    gFieldCameraConfigEA = gFLD_D3->getSaturnPtr(0x0607AEBC);
    gFieldDragonAnimTableEA = { 0x06094134, gFLD_A3 };
    gFieldCameraDrawFunc = &fieldOverlaySubTaskInitSub2;

    if (!initField(workArea, fieldFileList_D3, arg))
    {
        return nullptr;
    }

    loadSoundBanks(0x31, 0);

    getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m0_pScripts = ReadScripts(gFLD_D3->getSaturnPtr(0x06089B5C));

    subfieldD3_0(workArea);

    return nullptr;
}
