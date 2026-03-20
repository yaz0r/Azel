#include "PDS.h"
#include "d5_starfield.h"
#include "o_fld_d5.h"
#include "field.h"
#include "field/field_a3/o_fld_a3.h"
#include "battle/BTL_A3/BTL_A3_map6.h"
#include "kernel/cinematicBarsTask.h"
#include "kernel/loadSavegameScreen.h"
#include "3dEngine.h"

// 06057eb4
static s32 isFieldCameraActive(s16 cameraIndex)
{
    sFieldCameraManager* pCameraData = getFieldTaskPtr()->m8_pSubFieldData->m334;
    return (s32)*(s8*)((u8*)&pCameraData->m3E4_cameraSlots[cameraIndex] + 0x8C);
}

// 06057fbc
static sFieldCameraStatus* getCurrentFieldCameraStatus()
{
    sFieldCameraManager* pCameraData = getFieldTaskPtr()->m8_pSubFieldData->m334;
    return &pCameraData->m3E4_cameraSlots[pCameraData->m50C_activeCameraSlot];
}

// 06057db0
static void updateFieldCamera()
{
    sFieldCameraManager* pCameraData = getFieldTaskPtr()->m8_pSubFieldData->m334;

    for (s32 i = 0; i < 2; i++)
    {
        if (isFieldCameraActive(i))
        {
            sFieldCameraStatus& cam = pCameraData->m3E4_cameraSlots[i];
            if (pCameraData->m37C_isCutsceneCameraActive == 0)
            {
                if (cam.m78_drawFunc)
                {
                    cam.m78_drawFunc(&cam);
                }
            }
        }
    }

    sFieldCameraStatus* pCurrentCam = &pCameraData->m3E4_cameraSlots[pCameraData->m50C_activeCameraSlot];
    sVec3_S16 rotation;
    rotation[0] = (s16)((u32)pCurrentCam->mC_rotation.m0_X >> 16);
    rotation[1] = (s16)((u32)pCurrentCam->mC_rotation.m4_Y >> 16);
    rotation[2] = (s16)((u32)pCurrentCam->mC_rotation.m8_Z >> 16);
    updateEngineCamera(&cameraProperties2, pCurrentCam->m0_position, rotation);
    copyMatrix(pCurrentMatrix, &pCameraData->m384_viewMatrix);
    copyMatrix(&cameraProperties2.m28[0], &pCameraData->m3B4_projectionMatrix);
}

struct s_d5StarfieldTask : public s_workAreaTemplate<s_d5StarfieldTask>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &Init, &Update, &Draw, nullptr };
        return &taskDefinition;
    }

    static void Init(s_d5StarfieldTask* pThis);
    static void Update(s_d5StarfieldTask* pThis);
    static void Draw(s_d5StarfieldTask* pThis);

    s32 m0_scrollOffsetA;           // 0x00
    s32 m4_scrollOffsetB;           // 0x04
    s32 m8;                         // 0x08
    sVec3_FP mC_cameraPosition;     // 0x0C
    sVec3_FP m18_cameraRotation;    // 0x18: stored as rotation << 16
    std::array<s16, 4> m24_vdp1Clipping;  // 0x24
    std::array<s16, 2> m2C_vdp1LocalCoords; // 0x2C
    s16 m30_projParam0;             // 0x30
    s16 m32_projParam1;             // 0x32
    s32 m34;                        // 0x34
    s32 m38;                        // 0x38
    fixedPoint m3C_focalScale;      // 0x3C
    s32 m40_coeffParam0;            // 0x40
    s32 m44_coeffParam1;            // 0x44
    s32 m48_coeffParam2;            // 0x48
    s32 m4C_scrollSpeed;            // 0x4C
    s32 m50_pad[8];                 // 0x50-0x6F
    u8 m70_colorOffsetInit;         // 0x70
    u8 m71_pad[3];                  // 0x71-0x73
    u8 m74_colorOffset;             // 0x74
    // Saturn size 0x9C
};

// 06055440
static void setupStarfieldRotationPass(s_d5StarfieldTask* pThis, fixedPoint focalLength, s8 passIndex,
    s32 scrollParam, s32 coeffKAx, s32 coeffKAy, s32 scrollX, s32 scrollY)
{
    gRotationPassState.m0_planeIndex = passIndex;
    beginRotationPass_resetMatrix();

    sCoefficientTableData& t = gCoefficientTables[gRotationPassState.m0_planeIndex][(s32)vdp2Controls.m0_doubleBufferIndex];

    s32 sumX = (s32)pThis->m24_vdp1Clipping[0] + (s32)pThis->m24_vdp1Clipping[2];
    vdp2Controls.m_8 = scrollParam;
    t.m34 = (s16)((sumX + (int)(sumX < 0)) >> 1);
    s32 sumY = (s32)pThis->m24_vdp1Clipping[1] + (s32)pThis->m24_vdp1Clipping[3];
    t.m36 = (s16)((sumY + (int)(sumY < 0)) >> 1);
    t.m38 = pThis->m32_projParam1;
    t.m3C = t.m34;
    t.m3E = t.m36;
    t.m40 = 0;

    buildRotationMatrixPitchYaw(0, 0);
    scaleRotationMatrix(pThis->m3C_focalScale);

    setRotationScrollOffset(scrollX, scrollY);

    t.m0 = 0;
    t.m4 = 0;
    t.m8_Zst = 0;
    t.mC = 0;
    t.m10 = 0x10000;
    t.m14 = (s32)focalLength;
    t.m18 = 0;
    t.m54 = vdp2Controls.m_8;
    t.m58 = coeffKAx;
    t.m5C = coeffKAy;

    t.m1C = gVdp2RotationMatrix.m[0][0];
    t.m20 = gVdp2RotationMatrix.m[0][1];
    t.m24 = gVdp2RotationMatrix.m[0][2];
    t.m28 = gVdp2RotationMatrix.m[1][0];
    t.m2C = gVdp2RotationMatrix.m[1][1];
    t.m30 = gVdp2RotationMatrix.m[1][2];
    t.m44 = gVdp2RotationMatrix.Mx;
    t.m48 = gVdp2RotationMatrix.My;
}

// 060555d0
static void dmaPaletteNameEntry()
{
    asyncDmaCopy(gFLD_D5->getSaturnPtr(0x06078b54), getVdp2Cram(0), 0x200, 0);
    asyncDmaCopy(gFLD_D5->getSaturnPtr(0x06078734), getVdp2Cram(0x2A0), 0x20, 0);
    asyncDmaCopy(gFLD_D5->getSaturnPtr(0x06078754), getVdp2Cram(0x400), 0x200, 0);
    asyncDmaCopy(gFLD_D5->getSaturnPtr(0x06078954), getVdp2Cram(0x600), 0x200, 0);
}

// 0605560a
static void dmaPaletteGameOver()
{
    asyncDmaCopy(gFLD_D5->getSaturnPtr(0x06078b54), getVdp2Cram(0), 0x200, 0);
    asyncDmaCopy(gFLD_D5->getSaturnPtr(0x06078734), getVdp2Cram(0x2A0), 0x20, 0);
    asyncDmaCopy(gFLD_D5->getSaturnPtr(0x06078754), getVdp2Cram(0x400), 0x200, 0);
    asyncDmaCopy(gFLD_D5->getSaturnPtr(0x06078954), getVdp2Cram(0x600), 0x200, 0);
    asyncDmaCopy(gFLD_D5->getSaturnPtr(0x06078f54), getVdp2Cram(0x800), 0x200, 0);
}

// 06055340
static void setupStarfieldScrollAndCoefficients()
{
    setupScrollAndRotation(0xB, nullptr, nullptr, getVdp2Vram(0x3C000), 0x38);

    // Enable line window 1
    auto* regs = vdp2Controls.m4_pendingVdp2Regs;
    regs->mDC_LWTA1 = (regs->mDC_LWTA1 & 0x7FFFFFFF) | 0x80000000;
    vdp2Controls.m_isDirty = 1;

    // Fill coefficient table at 0x25E3C000
    // First 0x70 entries: {0, 0x2C0}
    u16* coeffTable = (u16*)getVdp2Vram(0x3C000);
    for (s32 i = 0; i < 0x70; i++)
    {
        coeffTable[i * 2] = 0;
        coeffTable[i * 2 + 1] = 0x2C0;
    }
    // Next 0x70 entries: {0, 0xFFFF}
    for (s32 i = 0; i < 0x70; i++)
    {
        coeffTable[(0x70 + i) * 2] = 0;
        coeffTable[(0x70 + i) * 2 + 1] = 0xFFFF;
    }
}

// 0605530c
static void generateStarfieldCoefficientsSub(s_d5StarfieldTask* pThis, s32 X, s32 Y)
{
    s32* pCoeffData = (s32*)getVdp2Vram(0x20000);
    s32 outerVal = 0x70;

    for (s32 y = 0; y < Y; y++)
    {
        s32 yTerm = performDivision(0x3100, outerVal * outerVal * 0x10000);
        s32 innerVal = 0x58;

        for (s32 x = 0; x < X; x++)
        {
            s32 xTerm = performDivision(0x1E40, innerVal * innerVal * 0x10000);
            s32 sqrtResult = sqrt_F(yTerm + xTerm);
            s32 divResult = setDividend(pThis->m40_coeffParam0, sqrtResult, pThis->m44_coeffParam1);
            s32 value = divResult + pThis->m48_coeffParam2;
            if (value < 0)
            {
                value = value + 3;
            }
            *pCoeffData = value >> 2;
            pCoeffData++;
            innerVal--;
        }
        outerVal--;
    }
}


// 0605530c
static void generateStarfieldCoefficients(s_d5StarfieldTask* pThis)
{
    generateStarfieldCoefficientsSub(pThis, 0xb0, 0x70);
}

// 06055DC4
void s_d5StarfieldTask::Init(s_d5StarfieldTask* pThis)
{
    s_fieldTaskWorkArea* pFieldTask = getFieldTaskPtr();
    // Original stores task pointer at offset 0x350 (repurposed for D5)
    *(s_d5StarfieldTask**)&pFieldTask->m8_pSubFieldData->m350_fieldPaletteTask = pThis;

    auto* regs = vdp2Controls.m4_pendingVdp2Regs;

    if (pFieldTask->m2C_currentFieldIndex == 0)
    {
        // Name entry path
        loadFile("FRS_D5_1.SCB", getVdp2Vram(0x40000), 0);
        loadFile("FRS_D5_1.PNB", getVdp2Vram(0x60000), 0);
        dmaPaletteNameEntry();
        regs->m10_CYCA0 = 0x310F754F;
        vdp2Controls.m_isDirty = 1;
    }
    else
    {
        // Game over path
        reinitVdp2();
        initNBG1Layer();
        loadFile("FRS_D5_1.SCB", getVdp2Vram(0x40000), 0);
        loadFile("FRS_D5_1.PNB", getVdp2Vram(0x60000), 0);
        loadFile("FNS_D5_1.SCB", getVdp2Vram(0x10000), 0);
        loadFile("FNS_D5_1.PNB", getVdp2Vram(0x1E000), 0);
        dmaPaletteGameOver();

        static const sLayerConfig nbg0Setup[] = {
            {m2_CHCN, 1}, {m5_CHSZ, 1}, {m6_PNB, 1}, {m7_CNSM, 0},
            {m34_W0E, 1}, {m37_W0A, 1}, {m40_CAOS, 4},
            {m0_END, 0},
        };
        setupNBG0(nbg0Setup);

        regs->m10_CYCA0 = 0x310F7544;
        vdp2Controls.m_isDirty = 1;

        initLayerMap(0, 0x25E1E000, 0x25E1E000, 0x25E1E000, 0x25E1E000);
    }

    static const sLayerConfig rgb0Setup[] = {
        {m1_TPEN, 1}, {m2_CHCN, 1}, {m5_CHSZ, 1}, {m6_PNB, 1}, {m7_CNSM, 0},
        {m27_RPMD, 3}, {m11_SCN, 8},
        {m34_W0E, 1}, {m37_W0A, 1},
        {m0_END, 0},
    };
    setupRGB0(rgb0Setup);

    static const sLayerConfig rotParamsSetup[] = {
        {m31_RxKTE, 1},
        {m0_END, 0},
    };
    setupRotationParams(rotParamsSetup);
    setupRotationParams2(rotParamsSetup);

    static const sLayerConfig wctldSetup[] = {
        {(eVdp2LayerConfig)0x23, 1}, {(eVdp2LayerConfig)0x26, 1},
        {m0_END, 0},
    };
    setupWCTLD(wctldSetup);

    regs->mE_RAMCTL = (regs->mE_RAMCTL & 0xFF00) | 0xB4;
    vdp2Controls.m_isDirty = 1;

    setupRotationMapPlanes(0, gFLD_D5->getSaturnPtr(0x0607938C));
    setupRotationMapPlanes(1, gFLD_D5->getSaturnPtr(0x0607938C));
    setupVdp2Table(6, coefficientA0, coefficientA1, getVdp2Vram(0x20000), 0x80);
    setupVdp2Table(7, coefficientB0, coefficientB1, getVdp2Vram(0x46800), 0x80);
    s_BTL_A3_Env_InitVdp2Sub3(5, getVdp2Vram(0x3D000));

    // Line color screen: dark blue
    *(u16*)getVdp2Vram(0x3D400) = 0x700;
    regs->mA8_LCTA = (regs->mA8_LCTA & 0xFFF80000) | 0x1EA00;

    // Back screen
    *(u16*)getVdp2Vram(0x3D800) = 0;
    regs->mAC_BKTA = (regs->mAC_BKTA & 0xFFF80000) | 0x1EC00;

    regs->mF0_PRISA = 0x304;
    regs->mF2_PRISB = 0x607;
    regs->mF4_PRISC = 0x404;
    regs->mF6_PRISD = 0x404;
    regs->mF8_PRINA = 0x605;
    regs->mFA_PRINB = 0x700;
    regs->mFC_PRIR = 3;
    vdp2Controls.m_isDirty = 1;

    static const std::vector<std::array<s32, 2>> layerDisplayConfig = {
        {{0x2C, 1}},
    };
    applyLayerDisplayConfig(layerDisplayConfig);

    regs->mE0_SPCTL = (regs->mE0_SPCTL & 0xF8FF) | 0x300;
    regs->mE0_SPCTL = (regs->mE0_SPCTL & 0xFFF0) | 3;
    regs->mE0_SPCTL = (regs->mE0_SPCTL & 0xCFFF) | 0x1000;

    pThis->m70_colorOffsetInit = 0x10;
    pThis->m74_colorOffset = 0x10;

    regs->m100_CCRSA = (s16)pThis->m70_colorOffsetInit;
    regs->m102_CCRSB = 0;
    regs->m104_CCRSC = 0;
    regs->m106_CCRSD = 0;
    vdp2Controls.m_isDirty = 1;

    pThis->m3C_focalScale = 0x1FDB9;

    setupStarfieldScrollAndCoefficients();

    pThis->m40_coeffParam0 = 0xFFF90000;
    pThis->m44_coeffParam1 = 0x16A09;
    pThis->m48_coeffParam2 = 0x80000;
    pThis->m4C_scrollSpeed = 0xCCC8;

    generateStarfieldCoefficients(pThis);

    regs->mEC_CCCTL = (regs->mEC_CCCTL & 0xFEFF) | 0x100;
    vdp2Controls.m_isDirty = 1;
}

// 060560D0
void s_d5StarfieldTask::Update(s_d5StarfieldTask* pThis)
{
    sFieldCameraStatus* pCamStatus = getCurrentFieldCameraStatus();

    auto* regs = vdp2Controls.m4_pendingVdp2Regs;
    vdp2Controls.m20_registers[0].m108_CCRNA = (regs->m108_CCRNA & 0xFFE0) | (s16)pThis->m74_colorOffset;
    vdp2Controls.m20_registers[1].m108_CCRNA = vdp2Controls.m20_registers[0].m108_CCRNA;

    // Clear camera position and rotation in the camera status
    pCamStatus->m0_position.m8_Z = 0;
    pCamStatus->m0_position.m4_Y = 0;
    pCamStatus->m0_position.m0_X = 0;
    pCamStatus->mC_rotation.m8_Z = 0;
    pCamStatus->mC_rotation.m4_Y = 0;
    pCamStatus->mC_rotation.m0_X = 0;
}

// 06056168
void s_d5StarfieldTask::Draw(s_d5StarfieldTask* pThis)
{
    updateFieldCamera();

    // Update scroll offset A based on camera Y rotation delta
    s32 newRotY = (s32)cameraProperties2.mC_rotation[1].m_value * 0x10000;
    pThis->m0_scrollOffsetA += ((newRotY - (s32)pThis->m18_cameraRotation.m4_Y) >> 0xC) * -0x100;
    pThis->m4_scrollOffsetB += pThis->m4C_scrollSpeed;

    // Store camera state
    pThis->mC_cameraPosition = cameraProperties2.m0_position;
    pThis->m18_cameraRotation.m0_X = (s32)cameraProperties2.mC_rotation[0].m_value << 16;
    pThis->m18_cameraRotation.m4_Y = (s32)cameraProperties2.mC_rotation[1].m_value << 16;
    pThis->m18_cameraRotation.m8_Z = (s32)cameraProperties2.mC_rotation[2].m_value << 16;

    getVdp1ClippingCoordinates(pThis->m24_vdp1Clipping);
    getVdp1LocalCoordinates(pThis->m2C_vdp1LocalCoords);
    getVdp1ProjectionParams(&pThis->m30_projParam0, &pThis->m32_projParam1);

    s_fieldTaskWorkArea* pFieldTask = getFieldTaskPtr();
    if (pFieldTask->m2C_currentFieldIndex != 0)
    {
        pauseEngine[4] = 0;
        updateVDP2CoordinatesIncrement(0x10000, 0x12492);
        pauseEngine[4] = 4;
    }

    fixedPoint focalLength = performDivision((s32)pThis->m30_projParam0, fixedPoint::fromInteger(pThis->m32_projParam1));

    // Pass 0: background stars
    setupStarfieldRotationPass(
        pThis, focalLength, 0,
        vdp2Controls.m_C << 16,
        0xB00000, 0x8000,
        pThis->m0_scrollOffsetA, pThis->m4_scrollOffsetB);

    // Pass 1: foreground stars
    setupStarfieldRotationPass(
        pThis, focalLength, 1,
        vdp2Controls.m_10 << 16,
        (s32)0xFF500000, (s32)0xFFFF8000,
        pThis->m0_scrollOffsetA, pThis->m4_scrollOffsetB);

    drawCinematicBar(4);
    drawCinematicBar(5);

    auto* regs = vdp2Controls.m4_pendingVdp2Regs;

    // Window 0
    regs->mC0_WPSX0 = pThis->m24_vdp1Clipping[0] << 1;
    regs->mC2_WPSY0 = pThis->m24_vdp1Clipping[1];
    regs->mC4_WPEX0 = pThis->m24_vdp1Clipping[2] << 1;
    regs->mC6_WPEY0 = pThis->m24_vdp1Clipping[3];

    // Window 1
    regs->mC8_WPSX1 = pThis->m24_vdp1Clipping[0] << 1;
    regs->mCA_WPSY1 = pThis->m24_vdp1Clipping[1];
    regs->mCC_WPEX1 = pThis->m24_vdp1Clipping[2] << 1;
    regs->mCE_WPEY1 = pThis->m24_vdp1Clipping[3];
}

// 06056330
void createNameEntryParticleTask(p_workArea parent)
{
    createSubTask<s_d5StarfieldTask>(parent);
}

// 0605633a
void createGameOverParticleTask(p_workArea parent)
{
    createSubTask<s_d5StarfieldTask>(parent);
}

// 06054180
void createD5StarfieldTask(p_workArea parent)
{
    createSubTask<s_d5StarfieldTask>(parent);
}
