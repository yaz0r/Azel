#include "PDS.h"
#include "twn_e014.h"
#include "town/town.h"
#include "audio/soundDriver.h"
#include "kernel/vdp1Allocator.h"
#include "kernel/cinematicBarsTask.h"
#include "town/townMainLogic.h"
#include "town/townCamera.h"
#include "town/e006/twn_e006.h"
#include "battle/BTL_A3/BTL_A3_map6.h"
#include "field/field_a3/o_fld_a3.h"

void setupVdp1Proj(fixedPoint fov); // TODO: cleanup

// 06054dc4
static s32 e014_computeGroundY()
{
    auto& coeff = gCoefficientTables[gRotationPassState.m0_planeIndex][(s32)vdp2Controls.m0_doubleBufferIndex];
    s32 iVar2 = gVdp2RotationMatrix.m[2][0].asS32() * coeff.m34;
    s32 iVar3 = gVdp2RotationMatrix.m[2][1].asS32() * coeff.m36;
    s32 iVar1 = MTH_Mul(gVdp2RotationMatrix.m[2][2],
                        fixedPoint(coeff.m38 * 0x10000 - coeff.m8_Zst));
    return FP_Div(iVar2 + iVar3 + iVar1, gVdp2RotationMatrix.m[2][1]).getInteger();
}

// https://www.youtube.com/watch?v=Txks9hG21qs&feature=youtu.be&t=3345

struct TWN_E014_data* gTWN_E014 = nullptr;

struct TWN_E014_data : public sTownOverlay
{
    TWN_E014_data();
    static void makeCurrent()
    {
        if (gTWN_E014 == NULL)
        {
            gTWN_E014 = new TWN_E014_data();
        }
        gCurrentTownOverlay = gTWN_E014;
    }

    sTownObject* createObjectTaskFromEA_siblingTaskWithEAArgWithCopy(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg) override;
    sTownObject* createObjectTaskFromEA_subTaskWithEAArg(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg) override;
};

static const char* listOfFilesToLoad[] = {
    "COMMON3.MCB",
    "COMMON3.CGB",
    "EVT014MP.MCB",
    "EVT014MP.CGB",
    nullptr
};

struct E014_groundTask : public s_workAreaTemplate<E014_groundTask> {
    // Mostly control the ground rendering on vdp2
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { E014_groundTask::Init, &E014_groundTask::Update, &E014_groundTask::Draw, nullptr };
        return &taskDefinition;
    }

    static void Init(E014_groundTask* pThis) {
        reinitVdp2();
        initNBG1Layer();
        asyncDmaCopy(gTWN_E014->getSaturnPtr(0x605e170), getVdp2Cram(0xA00), 0x200, 0);
        asyncDmaCopy(gTWN_E014->getSaturnPtr(0x605e370), getVdp2Cram(0), 0x200, 0);

        static const sLayerConfig setup[] =
        {
            m2_CHCN,  1,
            m5_CHSZ,  1, // character size is 2 cells x 2 cells (16*16)
            m6_PNB,   1, // pattern data size is 1 word
            m7_CNSM,  0,
            m27_RPMD, 2, // rotation parameter mode: Use both A&B
            m11_SCN,  8,
            m34_W0E,  1,
            m37_W0A,  1,
            m0_END,
        };
        setupRGB0(setup);

        static const sLayerConfig rotationPrams[] =
        {
            m31_RxKTE, 1, // use coefficient table
            m0_END,
        };
        setupRotationParams(rotationPrams);

        static const sLayerConfig rotationPrams2[] =
        {
            m0_END,
        };
        setupRotationParams2(rotationPrams2);

        loadFile("CAMPSCR0.SCB", getVdp2Vram(0x40000), 0);
        loadFile("CAMPSCR0.PNB", getVdp2Vram(0x60000), 0);

        vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL = (vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL & 0xFF00) | 0xb4;
        vdp2Controls.m4_pendingVdp2Regs->m10_CYCA0 = 0x13ff57ff;
        vdp2Controls.m_isDirty = 1;

        setupRotationMapPlanes(0, gTWN_E014->getSaturnPtr(0x605e570));
        setupRotationMapPlanes(1, gTWN_E014->getSaturnPtr(0x605E5B0));

        setupVdp2Table(6, coefficientA0, coefficientA1, getVdp2Vram(0x20000), 0x80);
        initRotationCoefficientTables(5, getVdp2Vram(0x24000));

        *(u16*)getVdp2Vram(0x2A502) = 0xbce5;
        vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA = (vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA & 0xFFF80000) | 0x12801;
        vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xFFF0) | 3;
        vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xF8FF) | 0x200;
        vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xCFFF) | 0x1000;
        vdp2Controls.m4_pendingVdp2Regs->mF0_PRISA = 0x204;
        vdp2Controls.m4_pendingVdp2Regs->mF2_PRISB = 0x407;
        vdp2Controls.m4_pendingVdp2Regs->mF4_PRISC = 0x404;
        vdp2Controls.m4_pendingVdp2Regs->mF6_PRISD = 0x404;
        vdp2Controls.m4_pendingVdp2Regs->mF8_PRINA = 0x600;
        vdp2Controls.m4_pendingVdp2Regs->mFA_PRINB = 0x700;
        vdp2Controls.m4_pendingVdp2Regs->mFC_PRIR = 0x3;
        vdp2Controls.m_isDirty = 1;

        pThis->m38_scale = 0x10000;
        vdp2Controls.m4_pendingVdp2Regs->mB8_OVPNRA = 0;
    }

    static void Update(E014_groundTask* pThis)
    {
        pThis->m34_groundOffset = 0;
    }

    // 06054838
    static void Draw(E014_groundTask* pThis)
    {
        pThis->mC_cameraPosition = cameraProperties2.m0_position;
        pThis->m18_cameraRotation.m0_X = (s32)(s16)cameraProperties2.mC_rotation[0] << 16;
        pThis->m18_cameraRotation.m4_Y = (s32)(s16)cameraProperties2.mC_rotation[1] << 16;
        pThis->m18_cameraRotation.m8_Z = (s32)(s16)cameraProperties2.mC_rotation[2] << 16;

        getVdp1ClippingCoordinates(pThis->m24_clippingCoordinates);
        getVdp1ProjectionParams(&pThis->m30_projWidth, &pThis->m32_projHeight);

        fixedPoint projScale = intDivide((s32)pThis->m30_projWidth, (s32)pThis->m32_projHeight << 16);
        beginRotationPass(0, projScale);
        computeCoefficients(pThis);
        drawCinematicBar(6);
        commitRotationPass();

        pThis->m2C_groundY = e014_computeGroundY();
        pThis->m0_scrollX = (pThis->m18_cameraRotation.m4_Y.asS32() >> 12) * -0x400;
        pThis->m4_scrollY = (0x1FF - pThis->m2C_groundY) * 0x10000;

        projScale = intDivide((s32)pThis->m30_projWidth, (s32)pThis->m32_projHeight << 16);
        beginRotationPass(1, projScale);

        s32 iVar4 = (s32)(s16)((s8)gRotationPassState.m0_planeIndex * 2 + (s16)vdp2Controls.m0_doubleBufferIndex) * 0x70;
        auto& t = gCoefficientTables[0][0];

        s32 sumX = (s32)pThis->m24_clippingCoordinates[0] + (s32)pThis->m24_clippingCoordinates[2];
        *(s16*)((u8*)&t.m34 + iVar4) = (s16)((sumX + (int)(sumX < 0)) >> 1);
        s32 sumY = (s32)pThis->m24_clippingCoordinates[1] + (s32)pThis->m24_clippingCoordinates[3];
        *(s16*)((u8*)&t.m36 + iVar4) = (s16)((sumY + (int)(sumY < 0)) >> 1);
        *(s16*)((u8*)&t.m38 + iVar4) = pThis->m32_projHeight;
        *(s16*)((u8*)&t.m3C + iVar4) = *(s16*)((u8*)&t.m34 + iVar4);
        *(s16*)((u8*)&t.m3E + iVar4) = *(s16*)((u8*)&t.m36 + iVar4);
        *(s16*)((u8*)&t.m40 + iVar4) = 0;

        buildRotationMatrixRoll(-pThis->m18_cameraRotation.m8_Z.asS32());
        intDivide((s32)pThis->m24_clippingCoordinates[3] - (s32)pThis->m24_clippingCoordinates[1], 0xE00000);
        fixedPoint scaleVal = intDivide((s32)pThis->m24_clippingCoordinates[2] - (s32)pThis->m24_clippingCoordinates[0], 0x1600000);
        scaleRotationMatrix(scaleVal);
        setRotationScrollOffset(pThis->m0_scrollX, pThis->m4_scrollY);
        commitRotationPass();

        auto* regs = vdp2Controls.m4_pendingVdp2Regs;
        regs->mC0_WPSX0 = pThis->m24_clippingCoordinates[0] << 1;
        regs->mC2_WPSY0 = pThis->m24_clippingCoordinates[1];
        regs->mC4_WPEX0 = pThis->m24_clippingCoordinates[2] << 1;
        regs->mC6_WPEY0 = pThis->m24_clippingCoordinates[3];
    }

    // 06054a12
    static void computeCoefficients(E014_groundTask* pThis)
    {
        s32 rotX = pThis->m18_cameraRotation.m0_X.asS32();
        s32 rotY = pThis->m18_cameraRotation.m4_Y.asS32();
        s32 rotZ = pThis->m18_cameraRotation.m8_Z.asS32();

        s32 iVar5 = (s32)(s16)((s8)gRotationPassState.m0_planeIndex * 2 + (s16)vdp2Controls.m0_doubleBufferIndex) * 0x70;
        auto& t = gCoefficientTables[0][0];

        s32 sumX = (s32)pThis->m24_clippingCoordinates[0] + (s32)pThis->m24_clippingCoordinates[2];
        *(s16*)((u8*)&t.m34 + iVar5) = (s16)((sumX + (int)(sumX < 0)) >> 1);
        s32 sumY = (s32)pThis->m24_clippingCoordinates[1] + (s32)pThis->m24_clippingCoordinates[3];
        *(s16*)((u8*)&t.m36 + iVar5) = (s16)((sumY + (int)(sumY < 0)) >> 1);
        *(s16*)((u8*)&t.m38 + iVar5) = pThis->m32_projHeight;
        *(s16*)((u8*)&t.m3C + iVar5) = *(s16*)((u8*)&t.m34 + iVar5);
        *(s16*)((u8*)&t.m3E + iVar5) = *(s16*)((u8*)&t.m36 + iVar5);
        *(s16*)((u8*)&t.m40 + iVar5) = 0;

        buildRotationMatrixPitchYaw(fixedPoint(-0x4000000 - rotX), fixedPoint(-rotY));
        scaleRotationMatrix(fixedPoint(pThis->m38_scale >> 2));
        writeRotationParams(fixedPoint(-rotZ));

        s32 diffX = (s32)*(s16*)((u8*)&t.m34 + iVar5) - (s32)*(s16*)((u8*)&t.m3C + iVar5);
        s32 diffY = (s32)*(s16*)((u8*)&t.m36 + iVar5) - (s32)*(s16*)((u8*)&t.m3E + iVar5);
        s32 diffZ = (s32)*(s16*)((u8*)&t.m38 + iVar5) - (s32)*(s16*)((u8*)&t.m40 + iVar5);

        s32 scaledX = MTH_Mul(fixedPoint(pThis->m38_scale), fixedPoint(pThis->mC_cameraPosition.m0_X.asS32() << 4));
        gVdp2RotationMatrix.Mx = (((scaledX - gVdp2RotationMatrix.m[0][0].asS32() * diffX) -
            gVdp2RotationMatrix.m[0][1].asS32() * diffY) - gVdp2RotationMatrix.m[0][2].asS32() * diffZ) +
            (s32)*(s16*)((u8*)&t.m3C + iVar5) * -0x10000;

        s32 scaledZ = MTH_Mul(fixedPoint(pThis->m38_scale), fixedPoint(pThis->mC_cameraPosition.m8_Z.asS32() << 4));
        gVdp2RotationMatrix.My = (((scaledZ - gVdp2RotationMatrix.m[1][0].asS32() * diffX) -
            gVdp2RotationMatrix.m[1][1].asS32() * diffY) - gVdp2RotationMatrix.m[1][2].asS32() * diffZ) +
            (s32)*(s16*)((u8*)&t.m3E + iVar5) * -0x10000;

        gVdp2RotationMatrix.Mz = ((((pThis->mC_cameraPosition.m4_Y.asS32() - pThis->m34_groundOffset) * 0x40 -
            gVdp2RotationMatrix.m[2][0].asS32() * diffX) - gVdp2RotationMatrix.m[2][1].asS32() * diffY) -
            gVdp2RotationMatrix.m[2][2].asS32() * diffZ) +
            (s32)*(s16*)((u8*)&t.m40 + iVar5) * -0x10000;
    }

    s32 m0_scrollX;
    s32 m4_scrollY;
    s32 m8;
    sVec3_FP mC_cameraPosition;
    sVec3_FP m18_cameraRotation;
    std::array<s16, 4> m24_clippingCoordinates;
    s32 m2C_groundY;
    s16 m30_projWidth;
    s16 m32_projHeight;
    s32 m34_groundOffset;
    s32 m38_scale;

    // size 0x40
};

void startE014BackgroundTask(p_workArea parent)
{
    createSubTask<E014_groundTask>(parent);
}

TWN_E014_data::TWN_E014_data() : sTownOverlay("TWN_E014.PRG")
{
    mTownSetups.push_back(readTownSetup(getSaturnPtr(0x0605e114), 1));

    overlayScriptFunctions.m_zeroArg[0x6057724] = {&e006_scriptFunction_60573d8, "e006_scriptFunction_60573d8"};
    overlayScriptFunctions.m_zeroArg[0x6056c64] = {&e006_scriptFunction_6056918, "e006_scriptFunction_6056918"};
    overlayScriptFunctions.m_zeroArg[0x6057746] = {&e006_scriptFunction_605861eSub0, "e006_scriptFunction_605861eSub0"};
    overlayScriptFunctions.m_zeroArg[0x6057784] = {&e006_scriptFunction_6057438, "e006_scriptFunction_6057438"};

    overlayScriptFunctions.m_oneArg[0x605c0d8] = {&TwnFadeIn, "TwnFadeIn"};
    overlayScriptFunctions.m_oneArg[0x605C150] = {&TwnFadeOut, "TwnFadeOut"};
    overlayScriptFunctions.m_oneArg[0x60576FC] = {&createEPKPlayer, "createEPKPlayer"};
    overlayScriptFunctions.m_oneArg[0x605896a] = {&setupDragonEntityForCutscene, "setupDragonEntityForCutscene"};

    overlayScriptFunctions.m_twoArg[0x605be70] = {&townCamera_setup, "townCamera_setup"};
}

sTownObject* TWN_E014_data::createObjectTaskFromEA_siblingTaskWithEAArgWithCopy(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg)
{
    assert(definitionEA.m_file == this);
    assert(arg.m_file == this);

    switch (definitionEA.m_offset)
    {
    default:
        assert(0);
        break;
    }
    return nullptr;
}

sTownObject* TWN_E014_data::createObjectTaskFromEA_subTaskWithEAArg(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg)
{
    return nullptr;
}

p_workArea overlayStart_TWN_E014(p_workArea pUntypedThis, u32 arg)
{
    gTWN_E014->makeCurrent();

    townDebugTask2Function* pThis = static_cast<townDebugTask2Function*>(pUntypedThis);

    pThis->m_DeleteMethod = &townOverlayDelete;

    loadSoundBanks(-1, 0);
    loadSoundBanks(68, 0);

    fadeOutAllSequences();

    graphicEngineStatus.m405C.m10_nearClipDistance = 0x800;
    graphicEngineStatus.m405C.m30_oneOverNearClip = FP_Div(0x10000, graphicEngineStatus.m405C.m10_nearClipDistance);

    graphicEngineStatus.m405C.m14_farClipDistance = 0x200000;
    graphicEngineStatus.m405C.m38_oneOverFarClip = FP_Div(0x8000, graphicEngineStatus.m405C.m14_farClipDistance);

    graphicEngineStatus.m405C.m34_oneOverFarClip256 = graphicEngineStatus.m405C.m38_oneOverFarClip << 8;

    initDramAllocator(pThis, townBuffer, sizeof(townBuffer), listOfFilesToLoad);

    initVdp1Ram(pThis, 0x25C18800, 0x63800);

    registerNpcs(gTWN_E014->mTownSetups, gTWN_E014->getSaturnPtr(0x605414c), arg);

    startScriptTask(pThis);

    startE014BackgroundTask(pThis);

    startMainLogic(pThis);

    setupVdp1Proj(0x16c16c1);

    startCameraTask(pThis);

    return pThis;
}
