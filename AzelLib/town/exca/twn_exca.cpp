#include "PDS.h"
#include "twn_exca.h"
#include "town/town.h"
#include "town/townScript.h"
#include "town/townEdge.h"
#include "town/townMainLogic.h"
#include "town/ruin/twn_ruin.h"
#include "kernel/animation.h"
#include "kernel/fileBundle.h"
#include "kernel/vdp1Allocator.h"
#include "kernel/graphicalObject.h"
#include "kernel/cinematicBarsTask.h"
#include "audio/soundDriver.h"
#include "town/townCamera.h"
#include "town/townDragon.h"
#include "town/excaEntity.h"
#include "field/field_a3/o_fld_a3.h"
#include "battle/BTL_A3/BTL_A3_map6.h"
#include "3dEngine.h"


struct TWN_EXCA_data* gTWN_EXCA = NULL;
struct TWN_EXCA_data : public sTownOverlay
{
    static void makeCurrent()
    {
        if (gTWN_EXCA == NULL)
        {
            gTWN_EXCA = new TWN_EXCA_data();
        }
        gCurrentTownOverlay = gTWN_EXCA;
    }

    TWN_EXCA_data() : sTownOverlay("TWN_EXCA.PRG")
    {
        overlayScriptFunctions.m_zeroArg[0x06057f7c] = &scriptFunction_6057058_sub0;
        overlayScriptFunctions.m_zeroArg[0x060584b4] = &hasLoadingCompleted;

        overlayScriptFunctions.m_oneArg[0x605d780] = &TwnFadeOut;
        overlayScriptFunctions.m_oneArg[0x605d708] = &TwnFadeIn;
        overlayScriptFunctions.m_oneArg[0x6059d08] = &SetupColorOffset;

        overlayScriptFunctions.m_twoArg[0x605d4a0] = &townCamera_setup;

        overlayScriptFunctions.m_fourArg[0x605be24] = &setNpcLocation;
        overlayScriptFunctions.m_fourArg[0x605be52] = &setNpcOrientation;

        mTownSetups.push_back(readTownSetup(getSaturnPtr(0x605fd4c), 1));
    }

    sTownObject* createObjectTaskFromEA_siblingTaskWithEAArgWithCopy(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg) override
    {
        assert(definitionEA.m_file == this);
        assert(arg.m_file == this);

        switch (definitionEA.m_offset)
        {
        case 0x0606628c:
            assert(size == 0xE0);
            return createExcaEntity(parent, arg);
        default:
            assert(0);
            break;
        }
        return nullptr;
    }

    sTownObject* createObjectTaskFromEA_subTaskWithEAArg(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg) override
    {
        assert(definitionEA.m_file == this);
        assert(arg.m_file == this);

        switch (definitionEA.m_offset)
        {
        case 0x0606473c:
            assert(size == 0xE8);
            return createTownDragon(parent, arg);
        default:
            assert(0);
            break;
        }
        return nullptr;

    }
};

static const char* listOfFilesToLoad[] = {
    "COMMON3.MCB",
    "COMMON3.CGB",
    "EXCAVAMP.MCB",
    "EXCAVAMP.CGB",
    "Z_A_GS.MCB",
    "Z_A_GS.CGB",
    "X_A_AZ.MCB",
    "X_A_AZ.CGB",
    "X_G_LG.MCB",
    "X_G_LG.CGB",
    "C_DRA0.MCB",
    nullptr
};

struct sExcaBackgroundTask : public s_workAreaTemplate<sExcaBackgroundTask>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sExcaBackgroundTask::Init, &sExcaBackgroundTask::Update, &sExcaBackgroundTask::Draw, nullptr };
        return &taskDefinition;
    }

    sVec3_FP mC_cameraPosition;
    sVec3_FP m18_cameraRotation;
    std::array<s16, 4> m24_vdp1Clipping;
    s32 m2C_scrollValue;
    s16 m30_projParam0;
    s16 m32_projParam1;
    s32 m34;
    s32 m38;

    static void Init(sExcaBackgroundTask* pThis)
    {
        reinitVdp2();
        initNBG1Layer();

        asyncDmaCopy(gTWN_EXCA->getSaturnPtr(0x06064994), getVdp2Cram(0xA00), 0x200, 0);

        static const sLayerConfig rgb0Setup[] = {
            m2_CHCN,  1,
            m5_CHSZ,  1,
            m6_PNB,   1,
            m7_CNSM,  0,
            m27_RPMD, 2,
            m11_SCN,  8,
            m34_W0E,  1,
            m37_W0A,  1,
            m0_END,
        };
        setupRGB0(rgb0Setup);

        static const sLayerConfig rotationParamsSetup[] = {
            m31_RxKTE, 1,
            m13,       1,
            m0_END,
        };
        setupRotationParams(rotationParamsSetup);

        static const sLayerConfig rotationParams2Setup[] = {
            m0_END,
        };
        setupRotationParams2(rotationParams2Setup);

        loadFile("EXCA_SCR.SCB", getVdp2Vram(0x40000), 0);
        loadFile("EXCA_SCR.PNB", getVdp2Vram(0x60000), 0);

        auto* regs = vdp2Controls.m4_pendingVdp2Regs;
        regs->mE_RAMCTL = (regs->mE_RAMCTL & 0xFF00) | 0xB4;
        regs->m10_CYCA0 = 0x13FF57FF;
        vdp2Controls.m_isDirty = 1;

        setupRotationMapPlanes(1, gTWN_EXCA->getSaturnPtr(0x06064b94));
        setupVdp2Table(6, coefficientA0, coefficientA1, getVdp2Vram(0x20000), 0x80);
        s_BTL_A3_Env_InitVdp2Sub3(5, getVdp2Vram(0x24000));

        // setup line color screen
        regs->mA8_LCTA = (regs->mA8_LCTA & 0xFFF80000) | 0x12800;
        *(u16*)getVdp2Vram(0x25000) = 0x0700;

        // setup back screen
        regs->mAC_BKTA = (regs->mAC_BKTA & 0xFFF80000) | 0x12801;
        *(u16*)getVdp2Vram(0x25002) = 0;

        regs->mE0_SPCTL = (regs->mE0_SPCTL & 0xFFF0) | 3;
        regs->mE0_SPCTL = (regs->mE0_SPCTL & 0xF8FF) | 0x200;
        regs->mE0_SPCTL = (regs->mE0_SPCTL & 0xCFFF) | 0x1000;

        regs->mF0_PRISA = 0x204;
        regs->mF2_PRISB = 0x407;
        regs->mF4_PRISC = 0x404;
        regs->mF6_PRISD = 0x404;
        regs->mF8_PRINA = 0x600;
        regs->mFA_PRINB = 0x700;
        regs->mFC_PRIR = 3;
        vdp2Controls.m_isDirty = 1;

        pThis->m38 = 0x100000;

        // Fill pattern name table at 0x60800 with 0x5000 entries
        u32* pVram = (u32*)getVdp2Vram(0x60800);
        for (int i = 0; i < 0x200; i++)
        {
            pVram[0] = 0x50005000;
            pVram[1] = 0x50005000;
            pVram[2] = 0x50005000;
            pVram[3] = 0x50005000;
            pVram += 4;
        }

        regs->mB8_OVPNRA = 0;
    }

    static void setupExcaRotationAndScroll(sExcaBackgroundTask* pThis)
    {
        //pThis->m18_cameraRotation.m0_X = 0x710000;
        //pThis->m18_cameraRotation.m4_Y = 0x5D3000;
       // pThis->m18_cameraRotation.m8_Z = 0;

        s32 rotX = pThis->m18_cameraRotation.m0_X;
        s32 rotY = pThis->m18_cameraRotation.m4_Y;
        s32 rotZ = pThis->m18_cameraRotation.m8_Z;

        sCoefficientTableData& t = gCoefficientTables[gRotationPassState.m0_planeIndex][(s32)vdp2Controls.m0_doubleBufferIndex];

        s32 sumX = (s32)pThis->m24_vdp1Clipping[0] + (s32)pThis->m24_vdp1Clipping[2];
        t.m34 = (s16)((sumX + (int)(sumX < 0)) >> 1);
        s32 sumY = (s32)pThis->m24_vdp1Clipping[1] + (s32)pThis->m24_vdp1Clipping[3];
        t.m36 = (s16)((sumY + (int)(sumY < 0)) >> 1);
        t.m38 = pThis->m32_projParam1;
        t.m3C = t.m34;
        t.m3E = t.m36;
        t.m40 = 0;

        buildRotationMatrixPitchYaw((fixedPoint)(-0x4000000 - rotX), (fixedPoint)(-rotY));
        scaleRotationMatrix((fixedPoint)(pThis->m38 >> 2));
        writeRotationParams((fixedPoint)(-rotZ));

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

    static void Update(sExcaBackgroundTask* pThis)
    {
        pThis->m34 = 0;
    }

    static void Draw(sExcaBackgroundTask* pThis)
    {
        pThis->mC_cameraPosition = cameraProperties2.m0_position;
        pThis->m18_cameraRotation = cameraProperties2.mC_rotation.toSVec3_FP();

        getVdp1ClippingCoordinates(pThis->m24_vdp1Clipping);
        getVdp1ProjectionParams(&pThis->m30_projParam0, &pThis->m32_projParam1);

        // Pass 0: ground plane with pitch/yaw rotation
        beginRotationPass(0, performDivision(pThis->m30_projParam0, fixedPoint::fromInteger(pThis->m32_projParam1)));
        setupExcaRotationAndScroll(pThis);
        drawCinematicBar(6);
        commitRotationPass();

        // Compute scroll offset for sky plane
        pThis->m2C_scrollValue = computeRotationScrollOffset();
        s32 scrollX = (pThis->m18_cameraRotation.m4_Y >> 0xC) * -0x400;
        s32 scrollY = (0x1E0 - pThis->m2C_scrollValue) * 0x10000;

        // Pass 1: sky plane with roll rotation
        beginRotationPass(1, performDivision(pThis->m30_projParam0, fixedPoint::fromInteger(pThis->m32_projParam1)));

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
        performDivision(pThis->m24_vdp1Clipping[3] - pThis->m24_vdp1Clipping[1], 0x00E00000);
        scaleRotationMatrix(performDivision(pThis->m24_vdp1Clipping[2] - pThis->m24_vdp1Clipping[0], 0x01600000));
        setRotationScrollOffset(scrollX, scrollY);
        commitRotationPass();

        // Set window coordinates
        auto* regs = vdp2Controls.m4_pendingVdp2Regs;
        regs->mC0_WPSX0 = pThis->m24_vdp1Clipping[0] << 1;
        regs->mC2_WPSY0 = pThis->m24_vdp1Clipping[1];
        regs->mC4_WPEX0 = pThis->m24_vdp1Clipping[2] << 1;
        regs->mC6_WPEY0 = pThis->m24_vdp1Clipping[3];
    }

    // size 0x40
};

static void startExcaBackgroundTask(p_workArea pThis)
{
    createSubTask<sExcaBackgroundTask>(pThis);
}

p_workArea overlayStart_TWN_EXCA(p_workArea pUntypedThis, u32 arg)
{
    gTWN_EXCA->makeCurrent();

    townDebugTask2Function* pThis = static_cast<townDebugTask2Function*>(pUntypedThis);

    pThis->m_DeleteMethod = &townOverlayDelete;

    loadSoundBanks(-1, 0);
    loadSoundBanks(59, 0);

    playPCM(pThis, 100);

    loadFnt("EVTEXCA.FNT");

    graphicEngineStatus.m405C.m10_nearClipDistance = 0x800;
    graphicEngineStatus.m405C.m30_oneOverNearClip = FP_Div(0x10000, graphicEngineStatus.m405C.m10_nearClipDistance);

    graphicEngineStatus.m405C.m14_farClipDistance = 0x100000;
    graphicEngineStatus.m405C.m38_oneOverFarClip = FP_Div(0x8000, graphicEngineStatus.m405C.m14_farClipDistance);

    graphicEngineStatus.m405C.m34_oneOverFarClip256 = graphicEngineStatus.m405C.m38_oneOverFarClip << 8;

    initDramAllocator(pThis, townBuffer, sizeof(townBuffer), listOfFilesToLoad);

    initVdp1Ram(pThis, 0x25C18800, 0x63800);

    registerNpcs(gTWN_EXCA->mTownSetups, gTWN_EXCA->getSaturnPtr(0x6054768), arg);

    startScriptTask(pThis);

    startExcaBackgroundTask(pThis);

    sEdgeTask* pEdgeTask = startEdgeTask(gTWN_EXCA->getSaturnPtr(0x605fd58));

    npcData0.m160_pEdgePosition = &pEdgeTask->m84.m8_position;

    startMainLogic(pThis);

    twnMainLogicTask->m14_EdgeTask = pEdgeTask;

    startCameraTask(pThis);

    twnVar1 = &twnVar2;

    return pThis;
}
