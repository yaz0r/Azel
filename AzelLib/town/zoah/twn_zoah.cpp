#include "PDS.h"
#include "twn_zoah.h"
#include "town/town.h"
#include "town/townScript.h"
#include "town/townEdge.h"
#include "town/townMainLogic.h"
#include "kernel/vdp1Allocator.h"
#include "kernel/graphicalObject.h"
#include "kernel/cinematicBarsTask.h"
#include "audio/soundDriver.h"
#include "town/townCamera.h"
#include "field/field_a3/o_fld_a3.h"
#include "battle/BTL_A3/BTL_A3_map6.h"
#include "town/excaEntity.h"
#include "town/collisionRegistry.h"
#include "town/townEdge.h"
#include "3dEngine.h"
#include "3dModels.h"
#include "kernel/fileBundle.h"
#include "kernel/animation.h"

void unloadFnt(); // TODO: fix

static void zoahCamera_update(sCameraTask* pThis)
{
    // Step 1: Increment time (same as townCamera_update)
    if ((npcData0.mFC & 1) == 0)
    {
        pThis->m4++;
        if (pThis->m4 > 5400)
        {
            pThis->m4 = 5400;
        }
    }

    // Step 2: Select day/night color set from mC data table
    sSaturnPtr colorSet0;
    sSaturnPtr colorSet1;
    u32 baseAddr = pThis->mC;
    if (!mainGameState.getBit(8)) {
        colorSet0 = sSaturnPtr::createFromRaw(baseAddr, gCurrentTownOverlay);
        colorSet1 = sSaturnPtr::createFromRaw(baseAddr + 0x10, gCurrentTownOverlay);
    } else {
        colorSet0 = sSaturnPtr::createFromRaw(baseAddr + 0x30, gCurrentTownOverlay);
        colorSet1 = sSaturnPtr::createFromRaw(baseAddr + 0x40, gCurrentTownOverlay);
    }

    // Step 3: Clamp time to 3600
    s32 time = pThis->m4;
    if (time > 0xe10) time = 0xe10;

    s32 fadeTime = 0x708;
    // Step 4: If time > 1799, shift to second color set
    if (time > 0x707) {
        colorSet0 = colorSet0 + 0x10;
        colorSet1 = colorSet1 + 0x10;
        time = time - 0x708;
        fadeTime = 0xe10;
    }

    // Step 5: If fading active and time remaining, update fade palette
    if (pThis->m1 != 0 && g_fadeControls.m24_fade1.m20_stopped != 0 && (0x708 - time) > 0) {
        u16 fadeColor = TwnFadeInComputeColorInterp(fadeTime);
        s32 curColor = convertColorToU32ForFade(g_fadeControls.m24_fade1.m0_color);
        fadePalette(&g_fadeControls.m24_fade1, curColor, fadeColor, 0x708 - time);
    }

    // Step 6: Interpolate 12 bytes of color/lighting data between two sets
    fixedPoint t = FP_Div(time, 0x708);
    s32 invT = 0x10000 - (s32)t;

    for (int i = 0; i < 12; i++) {
        s8 c_target = readSaturnS8(colorSet1 + 4 + i);
        s8 c_source = readSaturnS8(colorSet0 + 4 + i);
        pThis->m34_interpolatedLightData[i] = fixedPoint((s32)c_target * (s32)t + (s32)c_source * invT);
    }

    // Step 7: Interpolate rotation and build light direction matrix
    s16 rotX_target = readSaturnS16(colorSet1);
    s16 rotX_source = readSaturnS16(colorSet0);
    s16 rotY_target = readSaturnS16(colorSet1 + 2);
    s16 rotY_source = readSaturnS16(colorSet0 + 2);

    sMatrix4x3 mat;
    initMatrixToIdentity(&mat);
    rotateMatrixShiftedY(fixedPoint((s32)rotY_target * (s32)t + (s32)rotY_source * invT), &mat);
    rotateMatrixShiftedX(fixedPoint((s32)rotX_target * (s32)t + (s32)rotX_source * invT), &mat);

    pThis->m14[0] = mat.m[0][3];
    pThis->m14[1] = mat.m[1][3];
    pThis->m14[2] = mat.m[2][3];

    // Step 8: Compute ambient RGB from first 3 interpolated values
    fixedPoint* d = pThis->m34_interpolatedLightData;
    pThis->m10.m0 = (s8)(((u32)((s32)d[0] + 0x8000) >> 16) & 0xFF);
    pThis->m10.m1 = (s8)(((u32)((s32)d[1] + 0x8000) >> 16) & 0xFF);
    pThis->m10.m2 = (s8)(((u32)((s32)d[2] + 0x8000) >> 16) & 0xFF);
}

static void zoahCamera_draw(sCameraTask* pThis)
{
    // Same as townCamera_draw: transform light vector and set up light
    sVec3_FP stack16;
    transformVecByCurrentMatrix(pThis->m14, stack16);
    setupLight(stack16[0], stack16[1], stack16[2], pThis->m10.toU32());

    // Generate light falloff map from interpolated data at m34[3..11]
    fixedPoint* d = pThis->m34_interpolatedLightData;
    auto fpToU8 = [](fixedPoint fp) -> u32 { return ((u32)((s32)fp + 0x8000) >> 16) & 0xFF; };
    u32 f0 = fpToU8(d[3]) | (fpToU8(d[4]) << 8) | (fpToU8(d[5]) << 16);
    u32 f1 = fpToU8(d[6]) | (fpToU8(d[7]) << 8) | (fpToU8(d[8]) << 16);
    u32 f2 = fpToU8(d[9]) | (fpToU8(d[10]) << 8) | (fpToU8(d[11]) << 16);
    generateLightFalloffMap(f0, f1, f2);
}

int scriptFunction_06098f30()
{
    if (twnMainLogicTask->m14_EdgeTask) {
        updateWorldGrid(twnMainLogicTask->m14_EdgeTask->mE8.m0_position[0], twnMainLogicTask->m14_EdgeTask->mE8.m0_position[2]);
    }
    return fileInfoStruct.m2C_allocatedHead == 0;
}

// scriptFunction_060989f8 is functionally identical to scriptFunction_6057058_sub0
// (sets draw method, inits camera from Edge position, sets follow mode 0, calls setupCameraUpdateForCurrentMode)

void setupCameraUpdateForCurrentMode();

static void cameraUpdate_noop(sMainLogic*)
{
}

int scriptFunction_06098d2e()
{
    twnMainLogicTask->m10 = &cameraUpdate_noop;
    return 0;
}

int scriptFunction_06098d38()
{
    setupCameraUpdateForCurrentMode();
    return 0;
}

int scriptFunction_06096a98()
{
    vdp2Controls.m4_pendingVdp2Regs->m20_BGON = vdp2Controls.m4_pendingVdp2Regs->m20_BGON | 0x10;
    vdp2Controls.m_isDirty = 1;
    *(u16*)getVdp2Vram(0x25002) = 0x7756;
    vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA = (vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA & 0xFFF80000) | 0x12801;
    return 0;
}

s32 scriptFunction_0609dffe(s32 param_1)
{
    cameraTaskPtr->mC = param_1;
    cameraTaskPtr->m30 = 0x8000;

    cameraTaskPtr->m_UpdateMethod = zoahCamera_update;
    cameraTaskPtr->m_DrawMethod = zoahCamera_draw;

    if (g_fadeControls.m_4C <= g_fadeControls.m_4D)
    {
        vdp2Controls.m20_registers[1].m112_CLOFSL = 0x10;
        vdp2Controls.m20_registers[0].m112_CLOFSL = 0x10;
    }

    resetProjectVector();
    cameraTaskPtr->m2 = 0;
    cameraTaskPtr->m0 = 1;
    return 0;
}

s32 scriptFunction_0609dffe(s32 param_1);
int scriptFunction_06096a98();
static sTownObject* createZoahEntity(s_workAreaCopy* parent, sSaturnPtr arg);
static sTownObject* createZoahNPC(s_workAreaCopy* parent, sSaturnPtr arg);

struct TWN_ZOAH_data* gTWN_ZOAH = NULL;
struct TWN_ZOAH_data : public sTownOverlay
{
    static void makeCurrent()
    {
        if (gTWN_ZOAH == NULL)
        {
            gTWN_ZOAH = new TWN_ZOAH_data();
        }
        gCurrentTownOverlay = gTWN_ZOAH;
    }

    TWN_ZOAH_data() : sTownOverlay("TWN_ZOAH.PRG")
    {
        overlayScriptFunctions.m_zeroArg[0x06098f30] = &scriptFunction_06098f30;
        overlayScriptFunctions.m_zeroArg[0x06096a98] = &scriptFunction_06096a98;
        overlayScriptFunctions.m_zeroArg[0x060989f8] = &scriptFunction_6057058_sub0;
        overlayScriptFunctions.m_zeroArg[0x06098d2e] = &scriptFunction_06098d2e;
        overlayScriptFunctions.m_zeroArg[0x06098d38] = &scriptFunction_06098d38;

        overlayScriptFunctions.m_oneArg[0x0609e184] = &TwnFadeIn;
        overlayScriptFunctions.m_oneArg[0x0609dffe] = &scriptFunction_0609dffe;
        overlayScriptFunctions.m_oneArg[0x0609e1fc] = &TwnFadeOut;

        overlayScriptFunctions.m_fourArg[0x0609c8a0] = &setNpcLocation;
        overlayScriptFunctions.m_fourArg[0x0609c8ce] = &setNpcOrientation;

        for (int i = 0; i < 29; i++) {
            sSaturnPtr ptrBase = getSaturnPtr(0x060a0c30) + 0xC * i;
            mTownSetups.push_back(readTownSetup(ptrBase, 57));
        }
    }

    sTownObject* createObjectTaskFromEA_siblingTaskWithEAArgWithCopy(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg) override
    {
        assert(definitionEA.m_file == this);
        assert(arg.m_file == this);

        switch (definitionEA.m_offset)
        {
        case 0x060ba960:
            assert(size == 0xE0);
            return createExcaEntity(parent, arg);
        case 0x060ba988:
            assert(size == 0x90);
            return createZoahEntity(parent, arg);
        case 0x060ba9c0:
            assert(size == 0x17C);
            return createZoahNPC(parent, arg);
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
        default:
            assert(0);
            break;
        }
        return nullptr;
    }
};

struct sZoahNPC : public s_workAreaTemplateWithArgWithCopy<sZoahNPC, sSaturnPtr>, sNPC
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sZoahNPC::Init, &sZoahNPC::Update, nullptr, &sZoahNPC::Delete };
        return &taskDefinition;
    }

    static void Init(sZoahNPC* pThis, sSaturnPtr arg)
    {
        // Same logic as initEdgeNPC but with Zoah-specific behavior setup
        u8 npcIndex = readSaturnU8(arg + 0x20);
        npcData0.m70_npcPointerArray[npcIndex].workArea = pThis;
        npcData0.m70_npcPointerArray[npcIndex].pNPC = pThis;
        pThis->mC = 0;
        pThis->m10_InitPtr = arg;
        pThis->m1C = readSaturnS32(arg + 0x28);
        pThis->m30_animationTable = readSaturnEA(arg + 0x2C);
        pThis->mE8.m0_position = readSaturnVec3(arg + 0x8);
        pThis->mE8.mC_rotation = readSaturnVec3(arg + 0x14);

        // Zoah-specific behavior setup (equivalent of initEdgeNPCSub0)
        u8 behaviorByte = readSaturnU8(arg + 0x21);
        if (behaviorByte & 0x80) {
            pThis->mF |= 0x80;
        }
        pThis->mD = behaviorByte & 0x3F;
        pThis->m18 = readSaturnEA(arg + 0x30);

        // m14_updateFunction is set based on mD type — stub for now
        pThis->m14_updateFunction = nullptr;

        if (pThis->mD == 4) {
            pThis->m14E = 1;
        }

        pThis->m84.m30_pPosition = &pThis->mE8.m0_position;
        pThis->m84.m34_pRotation = &pThis->mE8.mC_rotation;
        pThis->m84.m38_pOwner = pThis;
        pThis->m84.m3C_scriptEA = readSaturnEA(arg + 0x38);
        if (u16 offset = readSaturnU16(arg + 0x36))
        {
            pThis->m84.m40 = pThis->m0_fileBundle->getRawFileAtOffset(offset);
        }
        else
        {
            pThis->m84.m40 = 0;
        }

        setCollisionSetup(&pThis->m84, readSaturnU8(arg + 0x34));
        setCollisionBounds(&pThis->m84, readSaturnVec3(arg + 0x3C), readSaturnVec3(arg + 0x48));

        pThis->m178 = 0;
        pThis->m179 = 0;
        pThis->m17A = 0;
        pThis->m17B = 0;
    }

    static void Update(sZoahNPC* pThis)
    {
        if (isDataLoaded(readSaturnS32(pThis->m10_InitPtr)))
        {
            s_fileBundle* pBundle = pThis->m0_fileBundle;
            u32 modelIndex = readSaturnU16(pThis->m10_InitPtr + 0x22);
            sStaticPoseData* pStaticPoseData = pBundle->getStaticPose(
                readSaturnU16(pThis->m10_InitPtr + 0x24),
                pBundle->getModelHierarchy(modelIndex)->countNumberOfBones());

            init3DModelRawData(pThis, &pThis->m34_3dModel, 0x100, pBundle, modelIndex, nullptr, pStaticPoseData, nullptr, nullptr);

            if (readSaturnU8(pThis->m10_InitPtr + 0x21) & 0x40)
            {
                Unimplemented(); // FUN_06096fc4
            }

            pThis->m_UpdateMethod = &sZoahNPC::Update2;
            pThis->m_DrawMethod = &sZoahNPC::Draw2;
        }
    }

    static void Update2(sZoahNPC* pThis)
    {
        Unimplemented();
        registerCollisionBody(&pThis->m84);
    }

    static void Draw2(sZoahNPC* pThis)
    {
        Unimplemented();
    }

    static void Delete(sZoahNPC* pThis)
    {
        u8 npcIndex = readSaturnU8(pThis->m10_InitPtr + 0x20);
        if (npcData0.m70_npcPointerArray[npcIndex].workArea == (p_workArea)pThis)
        {
            npcData0.m70_npcPointerArray[npcIndex].workArea = nullptr;
            npcData0.m70_npcPointerArray[npcIndex].pNPC = nullptr;
        }
    }

    s16 m14C;
    s16 m14E;
    s32 m150;
    s32 m154;
    s8 m158_animSchedule[32]; // 0x158-0x177
    s8 m178;
    s8 m179;
    s8 m17A;
    s8 m17B;
    // size 0x17C
};

static sTownObject* createZoahNPC(s_workAreaCopy* parent, sSaturnPtr arg) {
    sZoahNPC* pNPC = createSubTaskWithArgWithCopy<sZoahNPC, sSaturnPtr>(parent, arg);
    // On Saturn, sNPC::m8_MCBInDram and sTownObject::m8 share the same offset (0x08).
    // The factory caller uses m8 for cell linkage, which works for both types.
    return reinterpret_cast<sTownObject*>(static_cast<sNPC*>(pNPC));
}

struct sZoahEntity0 : public s_workAreaTemplateWithArgWithCopy<sZoahEntity0, sSaturnPtr>, sTownObject
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sZoahEntity0::Init, &sZoahEntity0::Update, nullptr, &sZoahEntity0::Delete };
        return &taskDefinition;
    }

    static void Init(sZoahEntity0* pThis, sSaturnPtr arg)
    {
        pThis->mC = arg;
        pThis->m74_position = readSaturnVec3(arg + 8);
        pThis->m80_rotation = readSaturnVec3(arg + 0x14);
        pThis->m8C_flag = readSaturnU8(arg + 0x24);

        s16 npcIndex = readSaturnS16(arg + 0x2c);
        if (npcIndex > -1) {
            npcData0.m70_npcPointerArray[npcIndex].workArea = pThis;
        }
    }

    static void Update(sZoahEntity0* pThis)
    {
        if (isDataLoaded(readSaturnS32(pThis->mC)))
        {
            sSaturnPtr scriptConfigEA = readSaturnEA(pThis->mC + 0x28);
            if (!scriptConfigEA.isNull())
            {
                pThis->m10_collisionBody.m30_pPosition = &pThis->m74_position;
                pThis->m10_collisionBody.m34_pRotation = &pThis->m80_rotation;
                pThis->m10_collisionBody.m38_pOwner = pThis;
                pThis->m10_collisionBody.m3C_scriptEA = readSaturnEA(scriptConfigEA + 4);

                s16 scriptDataOffset = readSaturnS16(scriptConfigEA + 2);
                if (scriptDataOffset == 0) {
                    pThis->m10_collisionBody.m40 = nullptr;
                } else {
                    pThis->m10_collisionBody.m40 = pThis->m0_fileBundle->getRawFileAtOffset(scriptDataOffset);
                }

                setCollisionSetup(&pThis->m10_collisionBody, readSaturnU8(scriptConfigEA + 0));
                setCollisionBounds(&pThis->m10_collisionBody, readSaturnVec3(scriptConfigEA + 0x8), readSaturnVec3(scriptConfigEA + 0x14));
            }

            pThis->m_UpdateMethod = &sZoahEntity0::Update2;
            pThis->m_DrawMethod = &sZoahEntity0::Draw2;
        }
    }

    static void Update2(sZoahEntity0* pThis)
    {
        if (pThis->m8C_flag == 1)
        {
            removeNPC(pThis, pThis, pThis->mC);
            return;
        }

        if (!readSaturnEA(pThis->mC + 0x28).isNull())
        {
            registerCollisionBody(&pThis->m10_collisionBody);
        }
    }

    static void Draw2(sZoahEntity0* pThis)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(pThis->m74_position);
        LCSItemBox_DrawType0Sub0(pThis->m0_fileBundle, readSaturnS16(pThis->mC + 0x20), readSaturnS16(pThis->mC + 0x22));
        popMatrix();
    }

    static void Delete(sZoahEntity0* pThis)
    {
        s16 npcIndex = readSaturnS16(pThis->mC + 0x2c);
        if (npcIndex > -1 && npcData0.m70_npcPointerArray[npcIndex].workArea == pThis)
        {
            npcData0.m70_npcPointerArray[npcIndex].workArea = nullptr;
        }
    }

    sSaturnPtr mC;
    sCollisionBody m10_collisionBody;
    sVec3_FP m74_position;
    sVec3_FP m80_rotation;
    u8 m8C_flag;
    // size 0x90
};

static sTownObject* createZoahEntity(s_workAreaCopy* parent, sSaturnPtr arg) {
    return createSubTaskWithArgWithCopy<sZoahEntity0, sSaturnPtr>(parent, arg);
}

static const char* listOfFilesToLoad[] = {
    "COMMON3.MCB",
    "COMMON3.CGB",
    "ZOAHMP0D.MCB",
    "ZOAHMP0D.CGB",
    "ZOAHMP0N.MCB",
    "ZOAHMP0N.CGB",
    "ZOAHMP1D.MCB",
    "ZOAHMP1D.CGB",
    "ZOAHMP1N.MCB",
    "ZOAHMP1N.CGB",
    "ZOAHOBJ0.MCB",
    "ZOAHOBJ0.CGB",
    "ZOAHOBJ1.MCB",
    "ZOAHOBJ1.CGB",
    "BLDAMP.MCB",
    "BLDAMP.CGB",
    "HUNTDMP0.MCB",
    "HUNTDMP0.CGB",
    "HUNTDMP1.MCB",
    "HUNTDMP1.CGB",
    "HUNTAMP0.MCB",
    "HUNTAMP0.CGB",
    "HUNTAMP1.MCB",
    "HUNTAMP1.CGB",
    "BARMP.MCB",
    "BARMP.CGB",
    "JUNKMP.MCB",
    "JUNKMP.CGB",
    "CHARCMP0.MCB",
    "CHARCMP0.CGB",
    "CHARCMP1.MCB",
    "CHARCMP1.CGB",
    "PAETMP0.MCB",
    "PAETMP0.CGB",
    "PAETMP1.MCB",
    "PAETMP1.CGB",
    "STOREMP.MCB",
    "STOREMP.CGB",
    "HIDINGMP.MCB",
    "HIDINGMP.CGB",
    "Z_A_AR.MCB",
    "Z_A_AR.CGB",
    "X_A_AB.MCB",
    "X_A_AB.CGB",
    "X_F_AB.MCB",
    "X_F_AB.CGB",
    "Z_A_SF.MCB",
    "Z_A_SF.CGB",
    "X_X_LG.MCB",
    "X_X_LG.CGB",
    "X_F_LG.MCB",
    "X_F_LG.CGB",
    "Z_A_JR.MCB",
    "Z_A_JR.CGB",
    "X_A_YR.MCB",
    "X_A_YR.CGB",
    "X_A_QT.MCB",
    "X_A_QT.CGB",
    "X_F_QT.MCB",
    "X_F_QT.CGB",
    "Z_A_VZ.MCB",
    "Z_A_VZ.CGB",
    "X_A_PT.MCB",
    "X_A_PT.CGB",
    "X_A_TA.MCB",
    "X_A_TA.CGB",
    "X_A_TB.MCB",
    "X_A_TB.CGB",
    "X_E_BM.MCB",
    "X_E_BM.CGB",
    "Z_A_DM.MCB",
    "Z_A_DM.CGB",
    "Z_A_RD.MCB",
    "Z_A_RD.CGB",
    "X_X_EN.MCB",
    "X_X_EN.CGB",
    "X_X_YD.MCB",
    "X_X_YD.CGB",
    "X_A_BZ.MCB",
    "X_A_BZ.CGB",
    "X_A_CS.MCB",
    "X_A_CS.CGB",
    nullptr
};

struct sZoahVdp2Plane : public s_workAreaTemplate<sZoahVdp2Plane>
{
    static void Init(sZoahVdp2Plane* pThis);
    static void Update(sZoahVdp2Plane* pThis);
    static void Draw(sZoahVdp2Plane* pThis);

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

void sZoahVdp2Plane::Init(sZoahVdp2Plane* pThis)
{
    auto* regs = vdp2Controls.m4_pendingVdp2Regs;

    reinitVdp2();
    initNBG1Layer();

    asyncDmaCopy(gTWN_ZOAH->getSaturnPtr(0x060b8f24), getVdp2Cram(0xa00), 0x200, 0);
    asyncDmaCopy(gTWN_ZOAH->getSaturnPtr(0x060b9124), vdp2Palette, 0x200, 0);
    asyncDmaCopy(gTWN_ZOAH->getSaturnPtr(0x060b8dd4), getVdp2Cram(0x800), 0x20, 0);

    static const sLayerConfig nbg0Setup[] = {
        m1_TPEN,  0,
        m2_CHCN,  0,
        m5_CHSZ,  1,
        m6_PNB,   1,
        m7_CNSM,  0,
        m34_W0E,  1,
        m37_W0A,  1,
        m44_CCEN, 1,
        m45_COEN, 0,
        m10_SPN,  4,
        m0_END,
    };
    setupNBG0(nbg0Setup);

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

    loadFile("EVT078N.SCB", getVdp2Vram(0x10000), 0);
    loadFile("EVT078N.PNB", getVdp2Vram(0x1f000), 0);
    loadFile("ZOAH.SCB", getVdp2Vram(0x40000), 0);
    loadFile("ZOAH.PNB", getVdp2Vram(0x60000), 0);

    regs->mE_RAMCTL = (regs->mE_RAMCTL & 0xff00) | 0xb4;
    regs->m10_CYCA0 = 0x310f7544;
    vdp2Controls.m_isDirty = 1;

    setupRotationMapPlanes(0, gTWN_ZOAH->getSaturnPtr(0x060b8df4));
    setupRotationMapPlanes(1, gTWN_ZOAH->getSaturnPtr(0x060b8e34));

    initLayerMap(0, 0x1f000, 0x1f000, 0x1f000, 0x1f000);

    setupVdp2Table(6, coefficientA0, coefficientA1, getVdp2Vram(0x20000), 0x80);
    s_BTL_A3_Env_InitVdp2Sub3(5, getVdp2Vram(0x24000));

    // line color screen
    *(u16*)getVdp2Vram(0x25000) = 0x700;
    regs->mA8_LCTA = (regs->mA8_LCTA & 0xFFF80000) | 0x12800;

    // back screen
    *(u16*)getVdp2Vram(0x25002) = 0x7756;
    regs->mAC_BKTA = (regs->mAC_BKTA & 0xFFF80000) | 0x12801;

    regs->mE0_SPCTL = (regs->mE0_SPCTL & 0xfff0) | 3;
    regs->mE0_SPCTL = (regs->mE0_SPCTL & 0xf8ff) | 0x200;
    regs->mE0_SPCTL = (regs->mE0_SPCTL & 0xcfff) | 0x1000;

    regs->mF0_PRISA = 0x204;
    regs->mF2_PRISB = 0x407;
    regs->mF4_PRISC = 0x404;
    regs->mF6_PRISD = 0x404;
    regs->mF8_PRINA = 0x605;
    regs->mFA_PRINB = 0x700;
    regs->mFC_PRIR = 3;
    vdp2Controls.m_isDirty = 1;

    pThis->m38 = 0x100000;

    regs->mB8_OVPNRA = 0x6000;

    pauseEngine[4] = 0;
    setupVDP2CoordinatesIncrement2(0, 0);
    pauseEngine[4] = 4;

    regs->m20_BGON = regs->m20_BGON & 0xfffe;
    vdp2Controls.m_isDirty = 1;
}

static void setupZoahRotationAndScroll(sZoahVdp2Plane* pThis)
{
    fixedPoint rotX = pThis->m18_cameraRotation.m0_X;
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

void sZoahVdp2Plane::Update(sZoahVdp2Plane* pThis)
{
    pThis->m34 = 0;
}

void sZoahVdp2Plane::Draw(sZoahVdp2Plane* pThis)
{
    pThis->mC_cameraPosition = cameraProperties2.m0_position;
    pThis->m18_cameraRotation = cameraProperties2.mC_rotation.toSVec3_FP();

    getVdp1ClippingCoordinates(pThis->m24_vdp1Clipping);
    getVdp1ProjectionParams(&pThis->m30_projParam0, &pThis->m32_projParam1);

    beginRotationPass(0, performDivision(pThis->m30_projParam0, fixedPoint::fromInteger(pThis->m32_projParam1)));
    setupZoahRotationAndScroll(pThis);
    drawCinematicBar(6);
    commitRotationPass();

    pThis->m2C_scrollValue = computeRotationScrollOffset();
    s32 scrollX = (pThis->m18_cameraRotation.m4_Y >> 0xC) * -0x400;
    s32 scrollY = (0x1FF - pThis->m2C_scrollValue) * 0x10000;

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

    auto* regs = vdp2Controls.m4_pendingVdp2Regs;
    regs->mC0_WPSX0 = pThis->m24_vdp1Clipping[0] << 1;
    regs->mC2_WPSY0 = pThis->m24_vdp1Clipping[1];
    regs->mC4_WPEX0 = pThis->m24_vdp1Clipping[2] << 1;
    regs->mC6_WPEY0 = pThis->m24_vdp1Clipping[3];
}

static void startZoahBackgroundTask(p_workArea pParent)
{
    static sZoahVdp2Plane::TypedTaskDefinition definition = {
        &sZoahVdp2Plane::Init,
        &sZoahVdp2Plane::Update,
        &sZoahVdp2Plane::Draw,
        nullptr,
    };
    createSubTask<sZoahVdp2Plane>(pParent, &definition);
}

static void townOverlayDelete_TwnZoah(townDebugTask2Function* pThis)
{
    Unimplemented();
    freeRamResources(pThis);
    vdp1FreeLastAllocation(pThis);
    unloadFnt();
}

p_workArea overlayStart_TWN_ZOAH(p_workArea pUntypedThis, u32 arg)
{
    gTWN_ZOAH->makeCurrent();

    townDebugTask2Function* pThis = static_cast<townDebugTask2Function*>(pUntypedThis);

    pThis->m_DeleteMethod = &townOverlayDelete_TwnZoah;

    loadSoundBanks(-1, 0);
    loadSoundBanks(0x3d, 0);

    loadFnt("EVTZOAH.FNT");

    graphicEngineStatus.m405C.m10_nearClipDistance = 0x800;
    graphicEngineStatus.m405C.m30_oneOverNearClip = FP_Div(0x10000, graphicEngineStatus.m405C.m10_nearClipDistance);

    graphicEngineStatus.m405C.m14_farClipDistance = 0x28000;
    graphicEngineStatus.m405C.m38_oneOverFarClip = FP_Div(0x8000, graphicEngineStatus.m405C.m14_farClipDistance);
    graphicEngineStatus.m405C.m34_oneOverFarClip256 = graphicEngineStatus.m405C.m38_oneOverFarClip << 8;

    initDramAllocator(pThis, townBuffer, sizeof(townBuffer), listOfFilesToLoad);

    initVdp1Ram(pThis, 0x25C18800, 0x63800);

    registerNpcs(gTWN_ZOAH->mTownSetups, gTWN_ZOAH->getSaturnPtr(0x06054ce0), arg);

    startScriptTask(pThis);

    startZoahBackgroundTask(pThis);

    sEdgeTask* pEdgeTask = startEdgeTask(gTWN_ZOAH->getSaturnPtr(0x060a0d8c));

    npcData0.m160_pEdgePosition = &pEdgeTask->m84.m8_position;

    startMainLogic(pThis);

    twnMainLogicTask->m14_EdgeTask = pEdgeTask;

    startCameraTask(pThis);

    static s32 zoahDepthThresholds[] = { 0x00014000, 0x7FFFFFFF };
    gTownGrid.m3C = zoahDepthThresholds;

    return pThis;
}
