#include "PDS.h"
#include "debugWindows.h"
#include "twn_ruin.h"
#include "town/town.h"
#include "town/townScript.h"
#include "town/townEdge.h"
#include "town/townLCS.h"
#include "town/townMainLogic.h"
#include "kernel/graphicalObject.h"
#include "kernel/animation.h"
#include "kernel/vdp1Allocator.h"
#include "kernel/fileBundle.h"
#include "kernel/debug/trace.h"
#include "audio/soundDriver.h"
#include "town/townCamera.h"
#include "battle/BTL_A3/BTL_A3_map6.h"
#include "3dEngine.h"
#include "kernel/cinematicBarsTask.h"
#include "kernel/loadSavegameScreen.h"
#include "field/field_a3/o_fld_a3.h"

#include "twn_ruin_lock.h"

sTownObject* TWN_RUIN_data::createObjectTaskFromEA_siblingTaskWithEAArgWithCopy(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg)
{
    assert(definitionEA.m_file == this);
    assert(arg.m_file == this);

    switch (definitionEA.m_offset)
    {
    case 0x605EA20:
        return createSubTaskWithArgWithCopy<sLockTask>(parent, arg);
    default:
        assert(0);
        break;
    }
    return nullptr;
}

sTownObject* TWN_RUIN_data::createObjectTaskFromEA_subTaskWithEAArg(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg)
{
    return nullptr;
}

TWN_RUIN_data* gTWN_RUIN = NULL;

const char* listOfFilesToLoad[] = {
    "COMMON3.MCB",
    "COMMON3.CGB",
    "RUINMP.MCB",
    "RUINMP.CGB",
    nullptr
};

// 06054bd8
struct sRuinBackgroundTask : public s_workAreaTemplate<sRuinBackgroundTask>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sRuinBackgroundTask::Init, &sRuinBackgroundTask::Update, &sRuinBackgroundTask::Draw, nullptr };
        return &taskDefinition;
    }

    // 06054bd8
    static void Init(sRuinBackgroundTask* pThis);
    // 06054ea6
    static void Update(sRuinBackgroundTask* pThis) {}
    // 06054ee8
    static void Draw(sRuinBackgroundTask* pThis);

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
    s8 m72_colorB;
    u8 m73_pad;
    s8 m74_colorNBG;
    s8 m75_colorRBG0;
    // size 0x9C
};

// 060550d4
static void ruinBgDrawPass0Sub(sRuinBackgroundTask* pThis)
{
    sCoefficientTableData& t = gCoefficientTables[gRotationPassState.m0_planeIndex][(s32)vdp2Controls.m0_doubleBufferIndex];

    fixedPoint rotX = pThis->m18_cameraRotation.m0_X;
    if ((s32)rotX == 0) rotX = fixedPoint(0xFFF49F4A); // -0xB60B6
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
    scaleRotationMatrix(pThis->m3C_scale / 4);
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
    gVdp2RotationMatrix.Mz = ((pThis->mC_cameraPosition.m4_Y - pThis->m38_groundY) * 0x40)
                    - gVdp2RotationMatrix.m[2][0] * diffX - gVdp2RotationMatrix.m[2][1] * diffY - gVdp2RotationMatrix.m[2][2] * diffZ
                    + (s32)(s16)t.m40 * -0x10000;
}

// 06054b40
static void ruinBgWaveDistortion(sRuinBackgroundTask* pThis)
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

// 06054bd8
void sRuinBackgroundTask::Init(sRuinBackgroundTask* pThis)
{
    reinitVdp2();
    initNBG1Layer();

    asyncDmaCopy(gTWN_RUIN->getSaturnPtr(0x0605ebf8), vdp2Palette, 0x200, 0);

    static const sLayerConfig rgbSetup[] = {
        {m2_CHCN, 1}, {m5_CHSZ, 1}, {m6_PNB, 1}, {m7_CNSM, 0},
        {m27_RPMD, 2}, {m11_SCN, 8}, {m34_W0E, 1}, {m37_W0A, 1},
        {m0_END},
    };
    setupRGB0(rgbSetup);

    static const sLayerConfig rotParams[] = {
        {m31_RxKTE, 1}, {m13, 1},
        {m0_END},
    };
    setupRotationParams(rotParams);

    static const sLayerConfig rotParams2[] = {
        {m0_END},
    };
    setupRotationParams2(rotParams2);

    loadFile("RUINSCR.SCB", getVdp2Vram(0x40000), 0);
    loadFile("RUINSCR.PNB", getVdp2Vram(0x60000), 0);

    vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL = (vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL & 0xFF00) | 0xB4;
    vdp2Controls.m4_pendingVdp2Regs->m10_CYCA0 = 0x310F7544;
    vdp2Controls.m_isDirty = 1;

    setupRotationMapPlanes(0, gTWN_RUIN->getSaturnPtr(0x0605eae0));
    setupRotationMapPlanes(1, gTWN_RUIN->getSaturnPtr(0x0605eb20));

    setupVdp2Table(6, coefficientA0, coefficientA1, getVdp2Vram(0x20000), 0x80);
    setupVdp2Table(7, coefficientB0, coefficientB1, getVdp2Vram(0x24000), 0x80);

    s_BTL_A3_Env_InitVdp2Sub3(5, getVdp2Vram(0x2A000));
    setVdp2VramU16(0x25E2A400, 0x700);

    vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA = (vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA & 0xFFF80000) | 0x15200;
    setVdp2VramU16(0x25E2A600, 0x8000);
    vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA = (vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA & 0xFFF80000) | 0x15300;

    vdp2Controls.m4_pendingVdp2Regs->mF0_PRISA = 0x204;
    vdp2Controls.m4_pendingVdp2Regs->mF2_PRISB = 0x407;
    vdp2Controls.m4_pendingVdp2Regs->mF4_PRISC = 0x404;
    vdp2Controls.m4_pendingVdp2Regs->mF6_PRISD = 0x404;
    vdp2Controls.m4_pendingVdp2Regs->mF8_PRINA = 0x600;
    vdp2Controls.m4_pendingVdp2Regs->mFA_PRINB = 0x700;
    vdp2Controls.m4_pendingVdp2Regs->mFC_PRIR = 3;

    pThis->m3C_scale = fixedPoint(0x100000);
    pThis->m38_groundY = 0;

    vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL = vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL & 0xFEFF;
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

    pThis->m75_colorRBG0 = 8;
    regs->m10C_CCRR = (s16)pThis->m75_colorRBG0;

    pThis->m74_colorNBG = 0x14;
    regs->m108_CCRNA = (regs->m108_CCRNA & 0xFFE0) | (s16)pThis->m74_colorNBG;
    vdp2Controls.m_isDirty = 1;

    // Fill NBG3 plane with 0x6000 pattern
    u32* pVram = (u32*)getVdp2Vram(0x61000);
    for (int i = 0; i < 0x200; i++)
    {
        pVram[0] = 0x60006000;
        pVram[1] = 0x60006000;
        pVram[2] = 0x60006000;
        pVram[3] = 0x60006000;
        pVram += 4;
    }
    vdp2Controls.m4_pendingVdp2Regs->mB8_OVPNRA = 0x6000;

    pThis->m4C_wavePhase = 0;
    pThis->m40_waveSpeed = 0x0021EFCB;
    pThis->m44_waveFreq = 0x4D5E540;
    pThis->m48_waveAmplitude = 0xF5A;
}

// 06054ee8
void sRuinBackgroundTask::Draw(sRuinBackgroundTask* pThis)
{
    pThis->mC_cameraPosition = cameraProperties2.m0_position;
    pThis->m18_cameraRotation = cameraProperties2.mC_rotation.toSVec3_FP();

    getVdp1ClippingCoordinates(pThis->m24_vdp1Clipping);
    getVdp1LocalCoordinates(pThis->m2C_localCoordinates);
    getVdp1ProjectionParams(&pThis->m30_projParam0, &pThis->m32_projParam1);

    // Pass 0: ground plane
    beginRotationPass(0, intDivide(pThis->m30_projParam0, fixedPoint::fromInteger(pThis->m32_projParam1)));
    ruinBgDrawPass0Sub(pThis);
    drawCinematicBar(6);
    commitRotationPass();

    pThis->m34_scrollValue = computeRotationScrollOffset();
    ruinBgWaveDistortion(pThis);

    // Pass 1: sky scroll
    pThis->m0_scrollX = ((s32)pThis->m18_cameraRotation.m4_Y >> 0xC) * -0x400;
    pThis->m4_scrollY = (0x15A - pThis->m34_scrollValue) * 0x10000;

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

void startRuinBackgroundTask(p_workArea pThis)
{
    createSubTask<sRuinBackgroundTask>(pThis);
}

void registerNpcs(const std::vector<const sTownSetup*>& townSetups, sSaturnPtr r5_script, s32 r6)
{
    npcData0.m0_numBackgroundScripts = 0;
    npcData0.m5E = -1;
    npcData0.m60_townSetup = &townSetups;
    npcData0.mFC = 0;
    npcData0.m100 = 0;
    npcData0.m11C_currentStackPointer = npcData0.m120_stack.end();
    npcData0.m164_cinematicBars = 0;
    npcData0.m168 = 0;
    npcData0.m16C_displayStringTask = 0;
    npcData0.m170_multiChoiceTask = 0;

    for (int i = 0; i < npcData0.m70_npcPointerArray.size(); i++)
    {
        npcData0.m70_npcPointerArray[i].workArea = nullptr;
        npcData0.m70_npcPointerArray[i].pNPC = nullptr;
    }

    townVar0 = nullptr;

    npcData0.mFC |= 0xF;

    npcData0.m104_currentScript.m0_scriptPtr = r5_script;
    npcData0.m104_currentScript.m4 = 0;
    npcData0.m104_currentScript.m8_owner = nullptr;

    npcData0.m118_currentResult = r6;

    npcData0.mF0 = 0;
    npcData0.mF4 = 0;
}

//(0 cursor, 1 near, 2 far)
void drawLcsSprite(const sVec2_S16& r4, s32 r5_index)
{
    sSaturnPtr r5_spriteData = gTWN_RUIN->getSaturnPtr(0x605EEFC + r5_index * 0x1C);
    s32 r6 = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory;
    s32 var4 = readSaturnU16(r5_spriteData + 6) + r6;
    s32 var0 = readSaturnU16(r5_spriteData + 0xA) + r6;
    s32 var8 = readSaturnS32(r5_spriteData + 0xC) >> 12;
    s32 varC = readSaturnS32(r5_spriteData + 0x10) >> 12;

    sVec2_S16 var10;
    var10[0] = (readSaturnS32(r5_spriteData + 0x14) >> 12) + r4[0];
    var10[1] = (readSaturnS32(r5_spriteData + 0x18) >> 12) + r4[1];

    sVec2_S16 var14;
    var14[0] = var10[0] + var8;
    var14[1] = var10[1] - varC;

    //////////
    s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;

    vdp1WriteEA.m0_CMDCTRL = readSaturnU16(r5_spriteData + 2); // command
    vdp1WriteEA.m4_CMDPMOD = readSaturnU16(r5_spriteData + 4); // CMDPMOD
    vdp1WriteEA.m6_CMDCOLR = var0; // CMDCOLR
    vdp1WriteEA.m8_CMDSRCA = var4; // CMDSRCA
    vdp1WriteEA.mA_CMDSIZE = readSaturnU16(r5_spriteData + 8); // CMDSIZE
    vdp1WriteEA.mC_CMDXA = var10[0]; // CMDXA
    vdp1WriteEA.mE_CMDYA = -var10[1]; // CMDYA
    vdp1WriteEA.m14_CMDXC = var14[0];
    vdp1WriteEA.m16_CMDYC = -var14[1];

    s_vd1ExtendedCommand* pExtendedCommand = createVdp1ExtendedCommand(vdp1WriteEA);
    pExtendedCommand->depth = 0;

    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

    graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
    graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
    graphicEngineStatus.m14_vdp1Context[0].mC += 1;
}

void drawLcs()
{
    if ((npcData0.mFC & 0x10) && !(npcData0.mFC & 0x8))
    {
        sVec2_S16 var0;
        var0[0] = LCSCollisionData.m0_LCS_X.getInteger();
        var0[1] = LCSCollisionData.m4_LCS_Y.getInteger();

        drawLcsSprite(var0, 0);

        if (currentResTask->m8_currentLCSType)
        {
            //6056C6C
            if (canCurrentResActivate())
            {
                drawLcsSprite(currentResTask->m14_LCS, 1);
            }
            else
            {
                drawLcsSprite(currentResTask->m14_LCS, 2);
            }
        }
    }
    else
    {
        if (enableDebugTask)
        {
            assert(0);
        }
    }
}

// TODO kernel
const sVec3_FP* cameraUpdate_follow_LCSSub1Sub0(s32 r4)
{
    if (r4 >= npcData0.m68_numEnvLCSTargets)
    {
        assert(0);
        return nullptr;
    }
    else
    {
        return &(*npcData0.m6C_LCSTargets)[r4];
    }
}


s32* twnVar1;
s32 twnVar2 = 0x7FFFFFFF;

p_workArea overlayStart_TWN_RUIN(p_workArea pUntypedThis, u32 arg)
{
    gTWN_RUIN->makeCurrent();

    townDebugTask2Function* pThis = static_cast<townDebugTask2Function*>(pUntypedThis);

    pThis->m_DeleteMethod = &townOverlayDelete;

    loadSoundBanks(-1, 0);
    loadSoundBanks(59, 0);

    playPCM(pThis, 100);

    loadFnt("EVTRUIN.FNT");

    graphicEngineStatus.m405C.m10_nearClipDistance = 0x800;
    graphicEngineStatus.m405C.m30_oneOverNearClip = FP_Div(0x10000, graphicEngineStatus.m405C.m10_nearClipDistance);

    graphicEngineStatus.m405C.m14_farClipDistance = 0xF000;
    graphicEngineStatus.m405C.m38_oneOverFarClip = FP_Div(0x8000, graphicEngineStatus.m405C.m14_farClipDistance);

    graphicEngineStatus.m405C.m34_oneOverFarClip256 = graphicEngineStatus.m405C.m38_oneOverFarClip << 8;

    initDramAllocator(pThis, townBuffer, sizeof(townBuffer), listOfFilesToLoad);

    initVdp1Ram(pThis, 0x25C18800, 0x63800);

    registerNpcs(gTWN_RUIN->mTownSetups, gTWN_RUIN->getSaturnPtr(0x06054398), arg);

    startScriptTask(pThis);

    startRuinBackgroundTask(pThis);

    sEdgeTask* pEdgeTask = startEdgeTask(gTWN_RUIN->getSaturnPtr(0x605E990));

    npcData0.m160_pEdgePosition = &pEdgeTask->m84.m8_position;

    startMainLogic(pThis);

    twnMainLogicTask->m14_EdgeTask = pEdgeTask;

    startCameraTask(pThis);

    twnVar1 = &twnVar2;

    return pThis;
}

s32 isObjectCloseEnoughToActivate()
{
    return !canCurrentResActivate();
}

void setupNPCWalkInZDirection(s32 r4_npcIndex, s32 r5_zDirection, s32 r6_distance)
{
    sNPC* r13 = getNpcDataByIndex(r4_npcIndex);
    sNPCE8* r14 = &r13->mE8;
    r14->m30_stepTranslation[0] = 0;
    r14->m30_stepTranslation[1] = 0;
    r14->m30_stepTranslation[2] = -r5_zDirection;

    sMatrix4x3 var10;
    initMatrixToIdentity(&var10);
    rotateMatrixShiftedY(r14->mC_rotation[1], &var10);
    rotateMatrixShiftedX(r14->mC_rotation[0], &var10);
    sVec3_FP var4;
    transformVec(r14->m30_stepTranslation, var4, var10);

    r14->m3C_targetPosition[0] = r14->m0_position[0] + var4[0] * r6_distance;
    r14->m3C_targetPosition[1] = r14->m0_position[1] + var4[1] * r6_distance;
    r14->m3C_targetPosition[2] = r14->m0_position[2] + var4[2] * r6_distance;

    r14->m48_targetRotation = r14->mC_rotation;

    r13->mF &= ~0x6;
    r13->mF |= 1;
    r13->mC |= 4;
}

s32 scriptFunction_6057058()
{
    setupCameraFollowMode();
    setupNPCWalkInZDirection(0, 227, 36);

    return 0;
}

void updateEdgeControls(sEdgeTask* r4)
{
    s32 r5 = 0;
    s32 r6 = 0;
    if (npcData0.mFC & 2)
    {
        r4->m14C_inputFlags |= 0xC0;
    }
    else
    {
        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m0_inputType == 2)
        {
            //0605BC5E
            r5 = 512 * graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m2_analogX;
            r6 = 512 * graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m3_analogY;
        }
        else
        {
            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & 0x10)
            {
                r6 = 0x10000;
            }
            else if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & 0x20)
            {
                r6 = -0x10000;
            }

            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & 0x40)
            {
                r5 = 0x10000;
            }
            else if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & 0x80)
            {
                r5 = -0x10000;
            }
        }
    }

    r4->m150_inputX = r5;
    r4->m154_inputY = r6;

    if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[0][0])
    {
        r4->m14C_inputFlags |= 2;
    }

    if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & graphicEngineStatus.m4514.mD8_buttonConfig[0][1])
    {
        r4->m14C_inputFlags |= 1;
    }
}

void updateEdgePositionSub1(sEdgeTask* r4)
{
    if (!(r4->m14C_inputFlags & 0x40))
    {
        switch (twnMainLogicTask->m0)
        {
        case 0:
            if (r4->m14C_inputFlags & 2)
            {
                r4->mE8.m24_stepRotation[1] = MTH_Mul(0x555555, r4->m150_inputX);
            }
            else
            {
                r4->mE8.m24_stepRotation[1] = MTH_Mul(0x38E38E, r4->m150_inputX);
            }
            break;
        default:
            assert(0);
            break;
        }
    }
    else
    {
        r4->mE8.m24_stepRotation[1] = 0;
    }

    //605BDAE
    if (r4->m14C_inputFlags & 0x80)
    {
        r4->mE8.m30_stepTranslation[0] = 0;
        r4->mE8.m30_stepTranslation[2] = 0;
    }
    else
    {
        s32 r10;
        if (r4->m14C_inputFlags & 0x2)
        {
            r10 = -0x212;
        }
        else
        {
            r10 = -0x109;
        }

        //0605BDCA
        switch (twnMainLogicTask->m0)
        {
        case 0:
            r4->mE8.m30_stepTranslation[2] = MTH_Mul(r4->m154_inputY, r10);
            r4->mE8.m30_stepTranslation[0] = 0;
            break;
        default:
            assert(0);
            break;
        }
    }

    //605BE7E
    r4->m14C_inputFlags &= 1;
    if (r4->m14C_inputFlags)
    {
        s32 collisionFlags = r4->m84.m44 & 4;
        r4->m84.m44 = collisionFlags;
        if (collisionFlags && r4->mE8.m30_stepTranslation[1] < 0x358)
        {
            r4->mE8.m30_stepTranslation[1] = 0x358;
        }
    }
}

void updateEdgePositionSub2(sNPCE8* r4)
{
    r4->mC_rotation[1] += r4->m24_stepRotation[1];
}

// TODO: kernel
void updateEdgePositionSub3Sub1(const sVec3_FP& r4, sVec2_FP* r5)
{
    if ((r4[0] == 0) && (r4[2] == 0))
    {
        if (r4[1] >= 0)
        {
            (*r5)[0] = 0x4000000;
        }
        else
        {
            (*r5)[0] = -0x4000000;
        }
    }

    //6036342
    fixedPoint r0 = sqrt_F(FP_Pow2(r4[0]) + FP_Pow2(r4[2]));

    if (r4[1] >= 0)
    {
        (*r5)[0] = atan2_FP(r4[1], r0);
    }
    else
    {
        (*r5)[0] = -atan2_FP(-r4[1], r0);
    }

    (*r5)[1] = atan2_FP(r4[0], r4[2]);
}

void updateEdgeLookAt(sEdgeTask* r4)
{
    sNPCE8* r13_npcE8 = &r4->mE8;

    if ((currentResTask->m8_currentLCSType) && (npcData0.mFC & 1))
    {
        //605BEEA
        //assert(0);
        Unimplemented();
    }
    //605C018
    else if ((npcData0.mFC & 0x10) && !(npcData0.mFC & 0x8))
    {
        //0605C030
        pushCurrentMatrix();
        translateCurrentMatrix(r13_npcE8->m0_position);
        rotateCurrentMatrixShiftedY(r13_npcE8->mC_rotation[1]);
        rotateCurrentMatrixShiftedX(r13_npcE8->mC_rotation[0]);
        adjustMatrixTranslation(0x1800);
        sVec3_FP var14;

        var14[0] = pCurrentMatrix->m[0][3] - setDividend(LCSCollisionData.m0_LCS_X.getInteger(), LCSCollisionData.m28_LCSDepthMax, LCSCollisionData.m2C_projectionWidthScale);
        var14[1] = pCurrentMatrix->m[1][3] - setDividend(LCSCollisionData.m4_LCS_Y.getInteger(), LCSCollisionData.m28_LCSDepthMax, LCSCollisionData.m30_projectionHeightScale);
        var14[2] = pCurrentMatrix->m[2][3] - LCSCollisionData.m28_LCSDepthMax;

        sVec2_FP varC;
        updateEdgePositionSub3Sub1(var14, &varC);

        var14[0] = pCurrentMatrix->m[2][0];
        var14[1] = pCurrentMatrix->m[2][1];
        var14[2] = pCurrentMatrix->m[2][2];
        sVec2_FP var4;
        updateEdgePositionSub3Sub1(var14, &var4);

        popMatrix();

        varC -= var4;

        varC[0] = varC[0].normalized();
        varC[1] = (-varC[1]).normalized();

        //0605C12E
        if (varC[0] > 0x18E38E3)
        {
            varC[0] = 0x18E38E3;
        }
        if (varC[0] < -0x18E38E3)
        {
            varC[0] = -0x18E38E3;
        }

        if (varC[1] > 0x38E38E3)
        {
            varC[1] = 0x38E38E3;
        }
        if (varC[1] < -0x38E38E3)
        {
            varC[1] = -0x38E38E3;
        }

        r4->m20_lookAtAngle[0] += MTH_Mul(varC[0] - r4->m20_lookAtAngle[0], 0xB333);
        r4->m20_lookAtAngle[1] += MTH_Mul(varC[1] - r4->m20_lookAtAngle[1], 0xB333);
    }
    else
    {
        //605C174
        fixedPoint r13 = r4->mE8.m24_stepRotation[1];
        if (r13 > 0x1C71C71)
        {
            r13 = 0x1C71C71;
        }
        if (r13 < -0x1C71C71)
        {
            r13 = -0x1C71C71;
        }

        if (r13)
        {
            r4->m20_lookAtAngle[1] += MTH_Mul(r13 - r4->m20_lookAtAngle[1], 0xB333);
        }
        else
        {
            r4->m20_lookAtAngle[1] += MTH_Mul(r13 - r4->m20_lookAtAngle[1], 0x8000);
        }

        r4->m20_lookAtAngle[0] = MTH_Mul(r4->m20_lookAtAngle[0], 0xB333);
    }
}

void updateEdgePosition(sNPC* r4)
{
    sEdgeTask* r12 = static_cast<sEdgeTask*>(r4);
    sNPCE8* r13 = &r12->mE8;
    sCollisionBody* r14 = &r12->m84;

    updateEdgeControls(r12);
    updateEdgePositionSub1(r12);

    *r4->m84.m30_pPosition += r4->m84.m58_collisionSolveTranslation;
    if (r4->m84.m44 & 4)
    {
        if (r14->m4C[1] < 0xB504)
        {
            r12->mF |= 0x80;
        }
        else
        {
            r12->mF &= ~0x80;
            if (r13->m30_stepTranslation[1] < 0)
            {
                r13->m30_stepTranslation[1] = 0;
            }
        }

        //605B948
        if (r12->mE_controlState != 4)
        {
            r12->mE_controlState = 4;
        }
    }
    else
    {
        //605B954
        if ((r13->m30_stepTranslation[1] < -0x199) || (r13->m30_stepTranslation[1] > 0))
        {
            r12->mF |= 0x80;
            if (r12->m2C_currentAnimation != 4)
            {
                r12->m2C_currentAnimation = 4; //falling
                sSaturnPtr var0 = r12->m30_animationTable + 4 * r12->m2C_currentAnimation; // walk animation

                u32 offset = readSaturnU16(var0 + 2);
                sAnimationData* buffer;
                if (readSaturnU16(var0))
                {
                    buffer = dramAllocatorEnd[0].mC_fileBundle->m0_fileBundle->getAnimation(offset);
                }
                else
                {
                    buffer = r12->m0_fileBundle->getAnimation(offset);
                }

                // play falling animation
                playAnimationGeneric(&r12->m34_3dModel, buffer, 5);
            }
        }
    }
    //605B9AA
    static bool gGravity = true;
    if(!isShipping() && gDebugWindows.town)
    {
        ImGui::Begin("Town", &gDebugWindows.town);
        ImGui::Checkbox("Gravity", &gGravity);
        ImGui::End();
    }

    if (gGravity)
    {
        r13->m30_stepTranslation[1] += -0x56;
    }

    if (r13->m30_stepTranslation[1] < -0x800)
    {
        r13->m30_stepTranslation[1] = -0x800;
    }

    updateEdgePositionSub2(r13);

    sMatrix4x3 var10;
    initMatrixToIdentity(&var10);
    rotateMatrixShiftedY(r13->mC_rotation[1], &var10);
    rotateMatrixShiftedX(r13->mC_rotation[0], &var10);

    if (r14->m44 & 4)
    {
        //0605B9F0
        if ((r14->m4C[0] != 0) || (r14->m4C[2] != 0))
        {
            if ((r14->m4C[0] == 0) && (r14->m4C[1] == 0))
            {
                var10.m[0][2] = var10.m[0][1];
                var10.m[1][2] = var10.m[1][1];
                var10.m[2][2] = var10.m[2][1];
            }
            else if ((r14->m4C[1] == 0) && (r14->m4C[2] == 0))
            {
                var10.m[0][0] = var10.m[0][1];
                var10.m[1][0] = var10.m[1][1];
                var10.m[2][0] = var10.m[2][1];
            }
            else
            {
                //605BA64
                fixedPoint ratio = FP_Div(FP_Pow2(r14->m4C[1]), fixedPoint(0x10000) - FP_Pow2(r14->m4C[2]));
                fixedPoint scale0 = sqrt_F(ratio);
                scaleMatrixRow0(scale0, &var10);

                fixedPoint offset0 = sqrt_F(fixedPoint(0x10000) - ratio);
                if (r14->m4C[0] > 0) {
                    offset0 = -offset0;
                }
                var10.m[0][0] = var10.m[0][0] + MTH_Mul(var10.m[0][1], offset0);
                var10.m[1][0] = var10.m[1][0] + MTH_Mul(var10.m[1][1], offset0);
                var10.m[2][0] = var10.m[2][0] + MTH_Mul(var10.m[2][1], offset0);

                fixedPoint ratio2 = FP_Div(FP_Pow2(r14->m4C[1]), fixedPoint(0x10000) - FP_Pow2(r14->m4C[0]));
                fixedPoint scale2 = sqrt_F(ratio2);
                scaleMatrixRow2(scale2, &var10);

                fixedPoint offset2 = sqrt_F(fixedPoint(0x10000) - ratio2);
                if (r14->m4C[2] > 0) {
                    offset2 = -offset2;
                }
                var10.m[0][2] = var10.m[0][2] + MTH_Mul(var10.m[0][1], offset2);
                var10.m[1][2] = var10.m[1][2] + MTH_Mul(var10.m[1][1], offset2);
                var10.m[2][2] = var10.m[2][2] + MTH_Mul(var10.m[2][1], offset2);
            }
        }
    }

    //0605BB48
    if ((r14->m44 & 4) && (r14->m4C[1] < 0xB504))
    {
        //605BB50
        sVec3_FP slopeNormal;
        slopeNormal[0] = MTH_Mul(r14->m4C[0], r14->m4C[1]);
        fixedPoint nx2 = FP_Pow2(r14->m4C[0]);
        fixedPoint nz2 = FP_Pow2(r14->m4C[2]);
        slopeNormal[1] = -nz2 - nx2;
        slopeNormal[2] = MTH_Mul(r14->m4C[2], r14->m4C[1]);

        if (slopeNormal[1] < 0) {
            slopeNormal[0] = -slopeNormal[0];
            slopeNormal[1] = -slopeNormal[1];
            slopeNormal[2] = -slopeNormal[2];
        }

        fixedPoint len = sqrt_F(MTH_Product3d_FP(slopeNormal, slopeNormal));
        fixedPoint scale = FP_Div(r13->m30_stepTranslation[1], len);

        r13->m30_stepTranslation[0] = MTH_Mul(slopeNormal[0], scale);
        r13->m30_stepTranslation[1] = MTH_Mul(slopeNormal[1], scale);
        r13->m30_stepTranslation[2] = MTH_Mul(slopeNormal[2], scale);
    }

    //605BBD6
    transformVec(r13->m30_stepTranslation, r13->m18_stepTranslationInWorld, var10);

    r13->m0_position += r13->m18_stepTranslationInWorld;

    updateEdgeLookAt(r12);

    r12->m14C_inputFlags = 0;
}

s32 scriptFunction_605B320(s32 arg0, s32 arg1)
{
    getNpcDataByIndex(arg0)->mE_controlState = arg1;
    return 0;
}

TWN_RUIN_data::TWN_RUIN_data() : sTownOverlay("TWN_RUIN.PRG")
{
    overlayScriptFunctions.m_zeroArg[0x6057570] = {&hasLoadingCompleted, "hasLoadingCompleted"};
    overlayScriptFunctions.m_zeroArg[0x6057058] = {&scriptFunction_6057058, "scriptFunction_6057058"};
    overlayScriptFunctions.m_zeroArg[0x605762A] = {&setupAutoWalk, "scriptFunction_605762A"};
    overlayScriptFunctions.m_zeroArg[0x605800E] = {&isObjectCloseEnoughToActivate, "isObjectCloseEnoughToActivate"};

    overlayScriptFunctions.m_oneArg[0x605C83C] = {&TwnFadeOut, "TwnFadeOut"};
    overlayScriptFunctions.m_oneArg[0x605c7c4] = {&TwnFadeIn, "TwnFadeIn"};
    overlayScriptFunctions.m_oneArg[0x6054364] = {&scriptFunction_6054364_waitForLockDisableCompletion, "scriptFunction_6054364_waitForLockDisableCompletion"};

    overlayScriptFunctions.m_twoArg[0x605C55C] = {&townCamera_setup, "townCamera_setup"};
    overlayScriptFunctions.m_twoArg[0x605B320] = {&scriptFunction_605B320, "scriptFunction_605B320"};
    overlayScriptFunctions.m_twoArg[0x6054334] = {&scriptFunction_6054334_disableLock, "scriptFunction_6054334_disableLock"};

    mTownSetups.push_back(readTownSetup(getSaturnPtr(0x605E984), 12));
}
