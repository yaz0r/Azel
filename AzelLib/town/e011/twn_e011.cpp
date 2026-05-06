#include "PDS.h"
#include "twn_e011.h"
#include "town/e006/twn_e006.h"
#include "town/town.h"
#include "town/townScript.h"
#include "town/townEdge.h"
#include "town/townMainLogic.h"
#include "kernel/vdp1Allocator.h"
#include "kernel/animation.h"
#include "kernel/fileBundle.h"
#include "kernel/graphicalObject.h"
#include "kernel/cinematicBarsTask.h"
#include "mainMenuDebugTasks.h"
#include "audio/soundDriver.h"
#include "town/townCamera.h"
#include "town/townCutscene.h"
#include "battle/BTL_A3/BTL_A3_map6.h"
#include "field/field_a3/o_fld_a3.h"
#include "trigo.h"

// https://youtu.be/Txks9hG21qs?t=2980

void setupVdp1Proj(fixedPoint fov); // TODO: cleanup
void setupCinematicBarData(int param1, std::array<u32, 256>& dataArray, u32 vdpOffset, int numEntries);

struct TWN_E011_data* gTWN_E011 = nullptr;

struct TWN_E011_data : public sTownOverlay
{
    TWN_E011_data();
    static void makeCurrent()
    {
        if (gTWN_E011 == NULL)
        {
            gTWN_E011 = new TWN_E011_data();
        }
        gCurrentTownOverlay = gTWN_E011;
    }

    sTownObject* createObjectTaskFromEA_siblingTaskWithEAArgWithCopy(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg) override;
    sTownObject* createObjectTaskFromEA_subTaskWithEAArg(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg) override;
};

u32 modulateColor(sSaturnPtr r4, u32 r5)
{
    s8* pColor = (s8*)getSaturnPtr(r4);
    s32 r = MTH_Mul(fixedPoint((s32)pColor[0]), fixedPoint((s32)r5)).m_value;
    s32 g = MTH_Mul(fixedPoint((s32)pColor[1]), fixedPoint((s32)r5)).m_value;
    s32 b = MTH_Mul(fixedPoint((s32)pColor[2]), fixedPoint((s32)r5)).m_value;
    return (u32)(((g + pColor[4]) >> 1) + 8) * 0x20 |
           (u32)(((b + pColor[5]) >> 1) + 8) * 0x400 |
           (u32)(((r + pColor[3]) >> 1) + 8) |
           0x8000;
}

u32 modulateColorByEvent(sCameraTask* pCamera, s32 r5)
{
    return computeTimeOfDayColor(r5);
}

struct sE011Task1 : public s_workAreaTemplate<sE011Task1>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { nullptr, &sE011Task1::Update, &sE011Task1::Draw, &sE011Task1::Delete };
        return &taskDefinition;
    }

    static void Update(sE011Task1* pThis)
    {
        sSaturnPtr dragonDataTable = sSaturnPtr::createFromRaw(0x06062078, gTWN_E011);
        sSaturnPtr dragonDataTableEntry = readSaturnEA(dragonDataTable + 4 * gDragonState->mC_dragonType);
        sSaturnPtr pcVar1 = (dragonDataTableEntry + readSaturnU8(pThis->m8 + 3) * 2);

        transformAndAddVec(gDragonState->m28_dragon3dModel.m44_hotpointData[readSaturnU8(pcVar1)][readSaturnU8(pcVar1 + 1)], pThis->m5C, cameraProperties2.m28[0]);
    }

    static void Draw(sE011Task1* pThis)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(pThis->m5C);
        pThis->mC_3dModel.m2C_poseData[0].m0_translation.zeroize();
        pThis->mC_3dModel.m18_drawFunction(&pThis->mC_3dModel);
        popMatrix();
    }

    // 06059ac8
    static void Delete(sE011Task1* pThis)
    {
        s8 npcIndex = readSaturnS8(pThis->m8 + 1);
        if (npcData0.m70_npcPointerArray[npcIndex].workArea == pThis)
        {
            npcData0.m70_npcPointerArray[npcIndex].workArea = nullptr;
        }
    }

    sSaturnPtr m8;
    s_3dModel mC_3dModel;
    sVec3_FP m5C;
    // 0x6C
};

s32 e011_scriptFunction_6059af0(s32 r4)
{
    sSaturnPtr r4Ptr = sSaturnPtr::createFromRaw(r4, gCurrentTownOverlay);
    if (e006_scriptFunction_605861eSub0() == 0)
    {
        sE011Task1* pNewSubTask = createSubTask<sE011Task1>(currentResTask);
        s_fileBundle* pBundle = dramAllocatorEnd[readSaturnU8(r4Ptr)].mC_fileBundle->m0_fileBundle;
        pNewSubTask->m8 = r4Ptr;

        s16 hierarchyIndex = readSaturnS16(r4Ptr + 4);
        sModelHierarchy* pHierarchy = pBundle->getModelHierarchy(hierarchyIndex);
        sStaticPoseData* pPose = pBundle->getStaticPose(readSaturnU16(r4Ptr + 6), pHierarchy->countNumberOfBones());
        //pHierarchy->patchFilePointers(dramAllocatorEnd[readSaturnU8(r4Ptr)].mC_buffer->m4_vd1Allocation->m4_vdp1Memory);
        init3DModelRawData(pNewSubTask, &pNewSubTask->mC_3dModel, 0, pBundle, hierarchyIndex, nullptr, pPose, nullptr, nullptr);

        npcData0.m70_npcPointerArray[readSaturnU8(r4Ptr + 1)].workArea = pNewSubTask;
        setupCutsceneModelType2(gCutsceneTask->m0, readSaturnU8(r4Ptr + 2), pNewSubTask, &pNewSubTask->mC_3dModel);
    }
    return 0;
}

s32 e011_scriptFunction_06059b7a(s32 param1)
{
    if ((e006_scriptFunction_605861eSub0() == 0) && (npcData0.m70_npcPointerArray[param1].workArea != nullptr))
    {
        npcData0.m70_npcPointerArray[param1].workArea->getTask()->markFinished();
        npcData0.m70_npcPointerArray[param1].workArea = nullptr;
    }

    return 0xBADF00D;
}

TWN_E011_data::TWN_E011_data() : sTownOverlay("TWN_E011.PRG")
{
    overlayScriptFunctions.m_zeroArg[0x6058484] = {&e006_scriptFunction_60573d8, "e006_scriptFunction_60573d8"};
    overlayScriptFunctions.m_zeroArg[0x60579c4] = {&e006_scriptFunction_6056918, "e006_scriptFunction_6056918"};
    overlayScriptFunctions.m_zeroArg[0x60584a6] = {&e006_scriptFunction_605861eSub0, "e006_scriptFunction_605861eSub0"};

    overlayScriptFunctions.m_oneArg[0x605ceb0] = {&TwnFadeOut, "TwnFadeOut"};
    overlayScriptFunctions.m_oneArg[0x605ce38] = {&TwnFadeIn, "TwnFadeIn"};
    overlayScriptFunctions.m_oneArg[0x605845c] = {&createEPKPlayer, "createEPKPlayer"};
    overlayScriptFunctions.m_oneArg[0x6059af0] = {&e011_scriptFunction_6059af0, "e011_scriptFunction_6059af0"};
    overlayScriptFunctions.m_oneArg[0x60596ca] = {&setupDragonEntityForCutscene, "setupDragonEntityForCutscene"};
    overlayScriptFunctions.m_oneArg[0x6059b7a] = {&e011_scriptFunction_06059b7a, "e011_scriptFunction_06059b7a"};

    overlayScriptFunctions.m_twoArg[0x605cbd0] = {&townCamera_setup, "townCamera_setup"};

    mTownSetups.push_back(readTownSetup(getSaturnPtr(0x0605ef14), 1));
    mTownSetups.push_back(readTownSetup(getSaturnPtr(0x0605ef20), 1));
}

sTownObject* TWN_E011_data::createObjectTaskFromEA_siblingTaskWithEAArgWithCopy(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg)
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

sTownObject* TWN_E011_data::createObjectTaskFromEA_subTaskWithEAArg(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg)
{
    return nullptr;
}

static const char* listOfFilesToLoad[] = {
    "COMMON3.MCB",
    "COMMON3.CGB",
    "EVT011MP.MCB",
    "EVT011MP.CGB",
    nullptr
};

// 06054d7c
static s32 e011_computeGroundY()
{
    auto& coeff = gCoefficientTables[gRotationPassState.m0_planeIndex][(s32)vdp2Controls.m0_doubleBufferIndex];
    s32 iVar2 = gVdp2RotationMatrix.m[2][0].asS32() * coeff.m34;
    s32 iVar3 = gVdp2RotationMatrix.m[2][1].asS32() * coeff.m36;
    s32 iVar1 = MTH_Mul(gVdp2RotationMatrix.m[2][2],
                        fixedPoint(coeff.m38 * 0x10000 - coeff.m8_Zst));
    return FP_Div(iVar2 + iVar3 + iVar1, gVdp2RotationMatrix.m[2][1]).getInteger();
}

// 06054dc4
static void e011_fieldPaletteNegateCoefficients()
{
    std::vector<fixedPoint>& coefficients = *gVdp2CoefficientTables[gRotationPassState.m0_planeIndex][vdp2Controls.m0_doubleBufferIndex];
    for (int i = 0; i < 0x1A8 && i < (int)coefficients.size(); i++)
    {
        coefficients[i] = fixedPoint((u32)coefficients[i] | 0x80000000);
    }
}

struct sE011BackgroundTask : public s_workAreaTemplate<sE011BackgroundTask>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sE011BackgroundTask::Init, &sE011BackgroundTask::Update, &sE011BackgroundTask::Draw, nullptr };
        return &taskDefinition;
    }

    s32 m0_scrollX;
    s32 m4_scrollY;
    s32 m8;
    sVec3_FP mC_cameraPosition;
    sVec3_FP m18_cameraRotation;
    std::array<s16, 4> m24_vdp1Clipping;
    s16 m2C;
    s16 m2E;
    s16 m30_vdp1ProjectionParam0;
    s16 m32_vdp1ProjectionParam1;
    s32 m34_groundY;
    s32 m38_groundOffset;
    s32 m3C_scale;
    u8* m40_heapPtr;
    s32 m44_dltOmega;
    s32 m48_dltAng;
    s32 m4C_ratio;
    s32 m50_lineSpd;
    s32 m54_upSpd;
    s32 m58_wavePhase;
    s32 m5C_scrollAccum;
    u8 m60_state;
    u8 m61_pad[3];
    sVec3_FP* m64_positionPtr;
    s32 m68_offsetX;
    s32 m6C_offsetY;
    s32 m70_counter;
    s32 m74_cutscenePos;
    s32 m78;
    u8 m7C_colorValue;
    u8 m7D_pad[3];
    u8 m80_debugEnabled;
    // size 0x84

    // 06054d46
    static void initCinematicBarTable(sE011BackgroundTask* pThis)
    {
        s32 iVar1 = 0;
        s32 iVar2 = 0;
        do {
            s32 iVar3 = iVar2 + 0xC;
            *(s32*)(pThis->m40_heapPtr + iVar2) = 0;
            *(s32*)(pThis->m40_heapPtr + iVar2 + 4) = iVar1;
            *(s32*)(pThis->m40_heapPtr + iVar2 + 8) = 0x10000;
            *(s32*)(pThis->m40_heapPtr + iVar3) = 0;
            *(s32*)(pThis->m40_heapPtr + iVar3 + 4) = iVar1 + 0x10000;
            iVar1 += 0x20000;
            *(s32*)(pThis->m40_heapPtr + iVar3 + 8) = 0x10000;
            iVar2 += 0x18;
        } while (iVar1 < 0xE00000);
    }

    // 06054dac
    static void initCinematicBarParams(sE011BackgroundTask* pThis)
    {
        pThis->m58_wavePhase = 0;
        pThis->m5C_scrollAccum = 0;
        pThis->m44_dltOmega = 0;
        pThis->m48_dltAng = 0;
        pThis->m4C_ratio = 0;
        pThis->m50_lineSpd = 0;
        pThis->m54_upSpd = 0;
        pThis->m7C_colorValue = 0x1F;
        initCinematicBarTable(pThis);
        drawCinematicBar(1);
    }

    // 06054bfc
    static void updateCinematicBarTable(sE011BackgroundTask* pThis)
    {
        s32 iVar7 = 0;
        s32 iVar9 = 0;
        s32 iVar8 = pThis->m58_wavePhase;
        pThis->m5C_scrollAccum += pThis->m50_lineSpd;
        do {
            s32 iVar4 = pThis->m4C_ratio;
            s32 iVar1 = MTH_Mul(fixedPoint(iVar4), getSin(iVar8 >> 16));
            iVar8 += pThis->m48_dltAng;
            u32* puVar5 = (u32*)(pThis->m40_heapPtr + iVar9 + 4);
            *puVar5 = (*puVar5 + pThis->m54_upSpd) & 0xFFFFFF;
            s32 iVar3 = *(s32*)(pThis->m40_heapPtr + iVar9 + 4); (void)iVar3;
            *(s32*)(pThis->m40_heapPtr + iVar9 + 8) = FP_Div(0x10000, fixedPoint(iVar1 + iVar4 + 0x10000));
            s32* piVar6 = (s32*)(pThis->m40_heapPtr + iVar9);
            *piVar6 = (0x10000 - *(s32*)(pThis->m40_heapPtr + iVar9 + 8)) * 0xB0 + pThis->m5C_scrollAccum;

            s32 iVar10 = iVar9 + 0xC;
            iVar4 = pThis->m4C_ratio;
            iVar1 = MTH_Mul(fixedPoint(iVar4), getSin(iVar8 >> 16));
            iVar8 += pThis->m48_dltAng;
            puVar5 = (u32*)(pThis->m40_heapPtr + iVar10 + 4);
            *puVar5 = (*puVar5 + pThis->m54_upSpd) & 0xFFFFFF;
            *(s32*)(pThis->m40_heapPtr + iVar10 + 8) = FP_Div(0x10000, fixedPoint(iVar1 + iVar4 + 0x10000));
            piVar6 = (s32*)(pThis->m40_heapPtr + iVar10);
            *piVar6 = (0x10000 - *(s32*)(pThis->m40_heapPtr + iVar10 + 8)) * 0xB0 + pThis->m5C_scrollAccum;

            iVar7 += 2;
            iVar9 += 0x18;
        } while (iVar7 < 0xE0);
        pThis->m58_wavePhase += pThis->m44_dltOmega;
    }

    // 0605563c
    static void setupGroundRotation(sE011BackgroundTask* pThis)
    {
        s32 rotX = pThis->m18_cameraRotation.m0_X.asS32();
        s32 rotY = pThis->m18_cameraRotation.m4_Y.asS32();
        s32 rotZ = pThis->m18_cameraRotation.m8_Z.asS32();

        s32 iVar5 = (s32)(s16)((s8)gRotationPassState.m0_planeIndex * 2 + (s16)vdp2Controls.m0_doubleBufferIndex) * 0x70;
        auto& t = gCoefficientTables[0][0];

        s32 sumX = (s32)pThis->m24_vdp1Clipping[0] + (s32)pThis->m24_vdp1Clipping[2];
        *(s16*)((u8*)&t.m34 + iVar5) = (s16)((sumX + (int)(sumX < 0)) >> 1);
        s32 sumY = (s32)pThis->m24_vdp1Clipping[1] + (s32)pThis->m24_vdp1Clipping[3];
        *(s16*)((u8*)&t.m36 + iVar5) = (s16)((sumY + (int)(sumY < 0)) >> 1);
        *(s16*)((u8*)&t.m38 + iVar5) = pThis->m32_vdp1ProjectionParam1;
        *(s16*)((u8*)&t.m3C + iVar5) = *(s16*)((u8*)&t.m34 + iVar5);
        *(s16*)((u8*)&t.m3E + iVar5) = *(s16*)((u8*)&t.m36 + iVar5);
        *(s16*)((u8*)&t.m40 + iVar5) = 0;

        buildRotationMatrixPitchYaw(fixedPoint(-0x4000000 - rotX), fixedPoint(-rotY));
        scaleRotationMatrix(fixedPoint(pThis->m3C_scale >> 2));
        writeRotationParams(fixedPoint(-rotZ));

        s32 diffX = (s32)*(s16*)((u8*)&t.m34 + iVar5) - (s32)*(s16*)((u8*)&t.m3C + iVar5);
        s32 diffY = (s32)*(s16*)((u8*)&t.m36 + iVar5) - (s32)*(s16*)((u8*)&t.m3E + iVar5);
        s32 diffZ = (s32)*(s16*)((u8*)&t.m38 + iVar5) - (s32)*(s16*)((u8*)&t.m40 + iVar5);

        s32 scaledX = MTH_Mul(fixedPoint(pThis->m3C_scale), fixedPoint(pThis->mC_cameraPosition.m0_X.asS32() << 4));
        gVdp2RotationMatrix.Mx = (((scaledX - gVdp2RotationMatrix.m[0][0].asS32() * diffX) -
            gVdp2RotationMatrix.m[0][1].asS32() * diffY) - gVdp2RotationMatrix.m[0][2].asS32() * diffZ) +
            (s32)*(s16*)((u8*)&t.m3C + iVar5) * -0x10000;

        s32 scaledZ = MTH_Mul(fixedPoint(pThis->m3C_scale), fixedPoint(pThis->mC_cameraPosition.m8_Z.asS32() << 4));
        gVdp2RotationMatrix.My = (((scaledZ - gVdp2RotationMatrix.m[1][0].asS32() * diffX) -
            gVdp2RotationMatrix.m[1][1].asS32() * diffY) - gVdp2RotationMatrix.m[1][2].asS32() * diffZ) +
            (s32)*(s16*)((u8*)&t.m3E + iVar5) * -0x10000;

        gVdp2RotationMatrix.Mz = ((((pThis->mC_cameraPosition.m4_Y.asS32() - pThis->m38_groundOffset) * 0x40 -
            gVdp2RotationMatrix.m[2][0].asS32() * diffX) - gVdp2RotationMatrix.m[2][1].asS32() * diffY) -
            gVdp2RotationMatrix.m[2][2].asS32() * diffZ) +
            (s32)*(s16*)((u8*)&t.m40 + iVar5) * -0x10000;
    }

    // 06054e04
    static void Init(sE011BackgroundTask* pThis)
    {
        reinitVdp2();
        initNBG1Layer();

        pThis->m40_heapPtr = (u8*)allocateHeapForTask(pThis, sizeof(sE011BackgroundTask) <= 0xC00 ? 0xC00 : 0xC00);

        asyncDmaCopy(gCurrentTownOverlay->getSaturnPtr(0x060605d8), getVdp2Cram(0xA00), 0x200, 0);

        static const sLayerConfig nbg0Setup[] = {
            {m1_TPEN, 0}, {m2_CHCN, 0}, {m5_CHSZ, 1}, {m6_PNB, 1}, {m7_CNSM, 0},
            {m34_W0E, 1}, {m37_W0A, 1}, {m44_CCEN, 1}, {m45_COEN, 0x10}, {m10_SPN, 4},
            {m22_N0LSCX, 1}, {m21_LCSY, 1}, {m20_N0LSS, 1},
            {m0_END},
        };
        setupNBG0(nbg0Setup);

        static const sLayerConfig rgb0Setup[] = {
            {m2_CHCN, 1}, {m5_CHSZ, 1}, {m6_PNB, 1}, {m7_CNSM, 0},
            {m27_RPMD, 2}, {m11_SCN, 8}, {m34_W0E, 1}, {m37_W0A, 1},
            {m0_END},
        };
        setupRGB0(rgb0Setup);

        static const sLayerConfig rotParamsSetup[] = {
            {m31_RxKTE, 1},
            {m0_END},
        };
        setupRotationParams(rotParamsSetup);

        static const sLayerConfig rotParams2Setup[] = {
            {m0_END},
        };
        setupRotationParams2(rotParams2Setup);

        loadFile("EVT011N.SCB", getVdp2Vram(0x10000), 0);
        loadFile("FRS_A3_0.SCB", getVdp2Vram(0x40000), 0);
        loadFile("FRS_A3_0.PNB", getVdp2Vram(0x60000), 0);
        loadFile("SPACE.PNB", getVdp2Vram(0x64000), 0);

        auto* regs = vdp2Controls.m4_pendingVdp2Regs;
        regs->mE_RAMCTL = (regs->mE_RAMCTL & 0xFF00) | 0xB4;
        regs->m10_CYCA0 = 0x310F7544;
        vdp2Controls.m_isDirty = 1;

        initLayerMap(0, 0x25E1F000, 0x25E1F000, 0x25E1F000, 0x25E1F000);

        static const std::array<u32, 16> rotationMapPlanes = {
            0x25E60800, 0x25E60800, 0x25E60800, 0x25E60800,
            0x25E64000, 0x25E64000, 0x25E64000, 0x25E64000,
            0x25E64000, 0x25E64000, 0x25E64000, 0x25E64000,
            0x25E64000, 0x25E64000, 0x25E64000, 0x25E64000,
        };
        setupRotationMapPlanes(1, rotationMapPlanes);

        setupVdp2Table(6, coefficientA0, coefficientA1, getVdp2Vram(0x20000), 0x80);
        setupVdp2Table(7, coefficientB0, coefficientB1, getVdp2Vram(0x22000), 0x80);
        initRotationCoefficientTables(5, getVdp2Vram(0x2A000));

        setupCinematicBarData(1, *(std::array<u32, 256>*)pThis->m40_heapPtr, 0x25E24000, 0xC0);

        *(u16*)getVdp2Vram(0x2A400) = 0x0700;
        regs->mA8_LCTA = (regs->mA8_LCTA & 0xFFF80000) | 0x15200;
        *(u16*)getVdp2Vram(0x2A600) = 0;
        regs->mAC_BKTA = (regs->mAC_BKTA & 0xFFF80000) | 0x15300;

        regs->mE0_SPCTL = (regs->mE0_SPCTL & 0xFFF0) | 3;
        regs->mE0_SPCTL = (regs->mE0_SPCTL & 0xF8FF) | 0x200;
        regs->mE0_SPCTL = (regs->mE0_SPCTL & 0xCFFF) | 0x1000;

        regs->mF0_PRISA = 0x404;
        regs->mF2_PRISB = 0x407;
        regs->mF4_PRISC = 0x404;
        regs->mF6_PRISD = 0x404;
        regs->mF8_PRINA = 0x605;
        regs->mFA_PRINB = 0x700;
        regs->mFC_PRIR = 3;
        vdp2Controls.m_isDirty = 1;

        pThis->m3C_scale = 0x100000;
        initCinematicBarParams(pThis);
        pThis->m60_state = 0;
        pThis->m70_counter = 0;

        regs->m20_BGON = regs->m20_BGON & 0xFFFE;
        regs->mEC_CCCTL = (regs->mEC_CCCTL & 0xFEFF) | 0x100;
        vdp2Controls.m_isDirty = 1;
        pThis->m80_debugEnabled = 0;
    }

    // 060550a2
    static void Update(sE011BackgroundTask* pThis)
    {
        if (pThis->m70_counter > 0)
        {
            s32 iVar2;
            if (e006_scriptFunction_605861eSub0() == 0)
            {
                s32 curPos = e006_scriptFunction_6057438();
                iVar2 = pThis->m74_cutscenePos - curPos;
                pThis->m74_cutscenePos = e006_scriptFunction_6057438();
            }
            else
            {
                iVar2 = 1;
            }

            pThis->m70_counter -= iVar2;
            while (iVar2 != 0)
            {
                iVar2--;
                if (pThis->m60_state == 1)
                {
                    vdp2Controls.m20_registers[0].m108_CCRNA =
                        (vdp2Controls.m4_pendingVdp2Regs->m108_CCRNA & 0xFFE0) | (u16)pThis->m7C_colorValue;
                    vdp2Controls.m20_registers[1].m108_CCRNA = vdp2Controls.m20_registers[0].m108_CCRNA;
                    updateCinematicBarTable(pThis);
                    drawCinematicBar(1);
                }
            }
            if (pThis->m70_counter < 1)
            {
                vdp2Controls.m4_pendingVdp2Regs->m20_BGON = vdp2Controls.m4_pendingVdp2Regs->m20_BGON & 0xFFFE;
                vdp2Controls.m_isDirty = 1;
            }
        }
    }

    // 060551e4
    static void Draw(sE011BackgroundTask* pThis)
    {
        pThis->mC_cameraPosition = cameraProperties2.m0_position;
        pThis->m18_cameraRotation.m0_X = (s32)(s16)cameraProperties2.mC_rotation[0] << 16;
        pThis->m18_cameraRotation.m4_Y = (s32)(s16)cameraProperties2.mC_rotation[1] << 16;
        pThis->m18_cameraRotation.m8_Z = (s32)(s16)cameraProperties2.mC_rotation[2] << 16;

        getVdp1ClippingCoordinates(pThis->m24_vdp1Clipping);
        getVdp1ProjectionParams(&pThis->m30_vdp1ProjectionParam0, &pThis->m32_vdp1ProjectionParam1);

        fixedPoint projScale = intDivide((s32)pThis->m30_vdp1ProjectionParam0, (s32)pThis->m32_vdp1ProjectionParam1 << 16);
        beginRotationPass(0, projScale);
        setupGroundRotation(pThis);
        drawCinematicBar(6);
        commitRotationPass();

        pThis->m34_groundY = e011_computeGroundY();
        e011_fieldPaletteNegateCoefficients();

        pThis->m0_scrollX = (pThis->m18_cameraRotation.m4_Y.asS32() >> 12) * -0x400;
        pThis->m4_scrollY = (0x200 - pThis->m34_groundY) * 0x10000;

        projScale = intDivide((s32)pThis->m30_vdp1ProjectionParam0, (s32)pThis->m32_vdp1ProjectionParam1 << 16);
        beginRotationPass(1, projScale);

        s32 iVar8 = (s32)(s16)((s8)gRotationPassState.m0_planeIndex * 2 + (s16)vdp2Controls.m0_doubleBufferIndex) * 0x70;
        auto& t = gCoefficientTables[0][0];

        s32 sumX = (s32)pThis->m24_vdp1Clipping[0] + (s32)pThis->m24_vdp1Clipping[2];
        *(s16*)((u8*)&t.m34 + iVar8) = (s16)((sumX + (int)(sumX < 0)) >> 1);
        s32 sumY = (s32)pThis->m24_vdp1Clipping[1] + (s32)pThis->m24_vdp1Clipping[3];
        *(s16*)((u8*)&t.m36 + iVar8) = (s16)((sumY + (int)(sumY < 0)) >> 1);
        *(s16*)((u8*)&t.m38 + iVar8) = pThis->m32_vdp1ProjectionParam1;
        *(s16*)((u8*)&t.m3C + iVar8) = *(s16*)((u8*)&t.m34 + iVar8);
        *(s16*)((u8*)&t.m3E + iVar8) = *(s16*)((u8*)&t.m36 + iVar8);
        *(s16*)((u8*)&t.m40 + iVar8) = 0;

        buildRotationMatrixRoll(-pThis->m18_cameraRotation.m8_Z.asS32());
        intDivide((s32)pThis->m24_vdp1Clipping[3] - (s32)pThis->m24_vdp1Clipping[1], 0xE00000);
        fixedPoint scaleVal = intDivide((s32)pThis->m24_vdp1Clipping[2] - (s32)pThis->m24_vdp1Clipping[0], 0x1600000);
        scaleRotationMatrix(scaleVal);
        setRotationScrollOffset(pThis->m0_scrollX, pThis->m4_scrollY);
        commitRotationPass();

        if (pThis->m70_counter > 0)
        {
            if (pThis->m60_state == 1)
            {
                gCurrentVDP2ScrollLayer = 0;
                setupVDP2CoordinatesIncrement2(pThis->m0_scrollX + pThis->m5C_scrollAccum, 0);
            }
            else if (pThis->m60_state == 2)
            {
                sVec3_FP transformed;
                transformAndAddVecByCurrentMatrix(pThis->m64_positionPtr, &transformed);
                fixedPoint invZ = FP_Div(0x10000, transformed.m8_Z);
                s32 projX = MTH_Mul_5_6(graphicEngineStatus.m405C.m18_widthScale, transformed.m0_X, invZ);
                s32 projY = MTH_Mul_5_6(graphicEngineStatus.m405C.m1C_heightScale, transformed.m4_Y, invZ);
                gCurrentVDP2ScrollLayer = 0;
                setupVDP2CoordinatesIncrement2(((pThis->m68_offsetX - projX) + -0xB0) * 0x10000, (s32)((s32)projY + pThis->m6C_offsetY + -0x70) * 0x10000);
            }
            gCurrentVDP2ScrollLayer = 4;
        }

        auto* regs = vdp2Controls.m4_pendingVdp2Regs;
        regs->mC0_WPSX0 = pThis->m24_vdp1Clipping[0] << 1;
        regs->mC2_WPSY0 = pThis->m24_vdp1Clipping[1];
        regs->mC4_WPEX0 = pThis->m24_vdp1Clipping[2] << 1;
        regs->mC6_WPEY0 = pThis->m24_vdp1Clipping[3];

        if (readKeyboardToggle(0x84))
        {
            pThis->m80_debugEnabled = 1;
        }
        if (readKeyboardToggle(0xF6))
        {
            clearVdp2TextMemory();
            pThis->m80_debugEnabled = 0;
        }

        if (pThis->m80_debugEnabled)
        {
            if (readKeyboardTable1(0xAB))
                pThis->m44_dltOmega += 0x1000;
            else if (readKeyboardTable1(0xAA))
                pThis->m44_dltOmega -= 0x1000;

            if (readKeyboardTable1(0xB4))
                pThis->m48_dltAng += 0x1000;
            else if (readKeyboardTable1(0xB2))
                pThis->m48_dltAng -= 0x1000;

            if (readKeyboardTable1(0xB3))
                pThis->m4C_ratio += 0x100;
            else if (readKeyboardTable1(0xB1))
                pThis->m4C_ratio -= 0x100;

            if (readKeyboardTable1(0xBB))
                pThis->m50_lineSpd += 0x1000;
            else if (readKeyboardTable1(0xBA))
                pThis->m50_lineSpd -= 0x1000;

            if (readKeyboardTable1(0xC2))
                pThis->m54_upSpd += 0x1000;
            else if (readKeyboardTable1(0xC1))
                pThis->m54_upSpd -= 0x1000;

            vdp2DebugPrintSetPosition(2, 0x15);
            vdp2PrintfSmallFont("dltomega: %8x\n", pThis->m44_dltOmega);
            vdp2DebugPrintSetPosition(2, 0x16);
            vdp2PrintfSmallFont("dltang  : %8x\n", pThis->m48_dltAng);
            vdp2DebugPrintSetPosition(2, 0x17);
            vdp2PrintfSmallFont("ratio   : %8x\n", pThis->m4C_ratio);
            vdp2DebugPrintSetPosition(2, 0x18);
            vdp2PrintfSmallFont("line_spd: %8x\n", pThis->m50_lineSpd);
            vdp2DebugPrintSetPosition(2, 0x19);
            vdp2PrintfSmallFont("up_spd  : %8x\n", pThis->m54_upSpd);

            updateCinematicBarTable(pThis);
            drawCinematicBar(1);
            vdp2Controls.m4_pendingVdp2Regs->m20_BGON = vdp2Controls.m4_pendingVdp2Regs->m20_BGON | 1;
            vdp2Controls.m_isDirty = 1;
        }
    }
};

static sE011BackgroundTask* gE011BackgroundTask = nullptr;

// 06054dde
static void startE011BackgroundTask(p_workArea pThis)
{
    gE011BackgroundTask = createSubTask<sE011BackgroundTask>(pThis);
}

p_workArea overlayStart_TWN_E011(p_workArea pUntypedThis, u32 arg)
{
    gTWN_E011->makeCurrent();
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

    registerNpcs(gTWN_E011->mTownSetups, gTWN_E011->getSaturnPtr(0x605414c), arg);

    startScriptTask(pThis);

    startE011BackgroundTask(pThis);

    startMainLogic(pThis);

    setupVdp1Proj(0x1c71c71);

    startCameraTask(pThis);

    return pThis;
}
