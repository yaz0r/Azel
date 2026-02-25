#include "PDS.h"
#include "twn_e014.h"
#include "town/town.h"
#include "audio/soundDriver.h"
#include "kernel/vdp1Allocator.h"
#include "town/townMainLogic.h"
#include "town/townCamera.h"

#include "town/e006/twn_e006.h" // TODO cleanup
void setupVdp1Proj(fixedPoint fov); // TODO: cleanup
void fieldPaletteTaskInitSub0(); // TODO cleanup
void setupRotationMapPlanes(int rotationMapIndex, sSaturnPtr inPlanes); // TODO cleanup
void setupVdp2Table(int p1, std::vector<fixedPoint>& p2, std::vector<fixedPoint>& p3, u8* coefficientTableAddress, u8 p5); // TODO cleanup
extern tCoefficientTable coefficientA0; // TODO cleanup
extern tCoefficientTable coefficientA1; // TODO cleanup
extern tCoefficientTable coefficientB0; // TODO cleanup
extern tCoefficientTable coefficientB1; // TODO cleanup
void s_BTL_A3_Env_InitVdp2Sub3(int layerIndex, u8* table); // TODO cleanup
void BTL_A3_Env_DrawSub1(int p1, fixedPoint p2); // TODO cleanup

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
        fieldPaletteTaskInitSub0();
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
        s_BTL_A3_Env_InitVdp2Sub3(5, getVdp2Vram(0x24000));

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

        pThis->m38 = 0x10000;
        vdp2Controls.m4_pendingVdp2Regs->mB8_OVPNRA = 0;
    }

    static void Update(E014_groundTask* pThis)
    {
        pThis->m34 = 0;
    }

    static void Draw(E014_groundTask* pThis)
    {
        pThis->mC_cameraPosition = cameraProperties2.m0_position;
        pThis->m18_cameraRotation = cameraProperties2.mC_rotation.toSVec3_FP();
        getVdp1ClippingCoordinates(pThis->m24_clippingCoordinates);
        getVdp1ProjectionParams(&pThis->m30_projWidth, &pThis->m32_projHeight);
        BTL_A3_Env_DrawSub1(0, performDivision(pThis->m30_projWidth, pThis->m32_projHeight << 0x10));

        Unimplemented();
    }

    sVec3_FP mC_cameraPosition;
    sVec3_FP m18_cameraRotation;
    std::array<s16, 4> m24_clippingCoordinates;
    s16 m30_projWidth;
    s16 m32_projHeight;
    int m34;
    fixedPoint m38; // scale?

    // size 0x40
};

void startE014BackgroundTask(p_workArea parent)
{
    createSubTask<E014_groundTask>(parent, E014_groundTask::getTypedTaskDefinition());
}

TWN_E014_data::TWN_E014_data() : sTownOverlay("TWN_E014.PRG")
{
    mTownSetups.push_back(readTownSetup(getSaturnPtr(0x0605e114), 1));

    overlayScriptFunctions.m_zeroArg[0x6057724] = &e006_scriptFunction_60573d8;
    overlayScriptFunctions.m_zeroArg[0x6056c64] = &e006_scriptFunction_6056918;
    overlayScriptFunctions.m_zeroArg[0x6057746] = &e006_scriptFunction_605861eSub0;
    overlayScriptFunctions.m_zeroArg[0x6057784] = &e006_scriptFunction_6057438;

    overlayScriptFunctions.m_oneArg[0x605c0d8] = &TwnFadeIn;
    overlayScriptFunctions.m_oneArg[0x605C150] = &TwnFadeOut;
    overlayScriptFunctions.m_oneArg[0x60576FC] = &createEPKPlayer;
    overlayScriptFunctions.m_oneArg[0x605896a] = &setupDragonEntityForCutscene;

    overlayScriptFunctions.m_twoArg[0x605be70] = &townCamera_setup;
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
