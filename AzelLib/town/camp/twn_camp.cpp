#include "PDS.h"
#include "twn_camp.h"
#include "town/town.h"
#include "town/townScript.h"
#include "town/townEdge.h"
#include "town/townMainLogic.h"
#include "town/ruin/twn_ruin.h"
#include "kernel/animation.h"
#include "kernel/fileBundle.h"
#include "kernel/vdp1Allocator.h"
#include "kernel/graphicalObject.h"
#include "audio/soundDriver.h"
#include "town/townCamera.h"
#include "town/townDragon.h"
#include "campDragon.h"
#include "town/excaEntity.h"
#include "town/collisionRegistry.h"
#include "campVdp2Plane.h"
#include "kernel/vdp1AnimatedQuad.h"

void unloadFnt(); // TODO: fix
int scriptFunction_60541c4(int arg);
void setupCameraUpdateForCurrentMode(); // todo clean
sTownObject* createCampFire(s_workAreaCopy* parent, sSaturnPtr arg); //todo: clean
static sTownObject* createCampParticle(s_workAreaCopy* parent, sSaturnPtr arg);

static void cameraUpdate_noop(sMainLogic*)
{
}

int scriptFunction_0606c952()
{
    twnMainLogicTask->m10 = &cameraUpdate_noop;
    return 0;
}

int scriptFunction_0606c95c()
{
    setupCameraUpdateForCurrentMode();
    return 0;
}

int scriptFunction_06071ade_toggleDayNight()
{
    sCameraTask::Init(cameraTaskPtr);
    if (mainGameState.bitField[0] & 1) {
        return 0;
    }
    if (mainGameState.bitField[1] & 0x80) {
        mainGameState.bitField[1] &= 0x7f;
    }
    else {
        mainGameState.bitField[1] |= 0x80;
    }
    return 1;
}

int scriptFunction_606cb54() {
    if (twnMainLogicTask->m14_EdgeTask) {
        updateWorldGrid(twnMainLogicTask->m14_EdgeTask->mE8.m0_position[0], twnMainLogicTask->m14_EdgeTask->mE8.m0_position[2]);
    }
    return fileInfoStruct.m2C_allocatedHead == 0;
}

int recoverAllHP_BP(void)
{
    int uVar1;

    uVar1 = 0;
    if (((short)mainGameState.gameStats.m10_currentHP < (short)mainGameState.gameStats.mB8_maxHP) ||
        ((short)mainGameState.gameStats.m14_currentBP < (short)mainGameState.gameStats.mBA_maxBP)) {
        uVar1 = 1;
    }
    mainGameState.gameStats.m10_currentHP = mainGameState.gameStats.mB8_maxHP;
    mainGameState.gameStats.m14_currentBP = mainGameState.gameStats.mBA_maxBP;
    return uVar1;
}


int scriptFunction_6056484(int arg) {
    auto data = getNpcDataByIndex(arg);
    if (recoverAllHP_BP()) {
        assert(0);
        return 1;
    }
    return 0;
}

// Setup camera fixed for camp
int scriptFunction_606c70c(sSaturnPtr arg) {
    sVec3_FP vector1 = readSaturnVec3(arg);
    sVec3_FP vector2 = readSaturnVec3(arg + 0xC);

    if ((vector2[0] == vector1[0]) && (vector2[2] == vector1[2])) {
        assert(0);
    }
    else {
        twnMainLogicTask->mE4_fixedPosition = vector1;
        twnMainLogicTask->m100_deltaPosition = vector2 - vector1;
        twnMainLogicTask->mF8[0] = readSaturnFP(arg + 4 * 6);
        twnMainLogicTask->mF0[0] = readSaturnFP(arg + 4 * 7);
        twnMainLogicTask->m10C[0] = 0;
        twnMainLogicTask->m10C[1] = readSaturnFP(arg + 4 * 8);
        twnMainLogicTask->m10C[2] = readSaturnFP(arg + 4 * 9);

        fixedPoint temp = twnMainLogicTask->m10C[2] + atan2_FP(-twnMainLogicTask->m100_deltaPosition[2], twnMainLogicTask->m100_deltaPosition[0]);
        twnMainLogicTask->mF0[1] = temp - twnMainLogicTask->m10C[1];
        twnMainLogicTask->mF8[1] = temp + twnMainLogicTask->m10C[1];

        twnMainLogicTask->m44_cameraTarget = twnMainLogicTask->m14_EdgeTask->mE8.m0_position;
        twnMainLogicTask->m5C_rawCameraPosition = twnMainLogicTask->mE4_fixedPosition;

        twnMainLogicTask->m2_cameraFollowMode = sMainLogic::TackingMode_Fixed;

        setupCameraUpdateForCurrentMode();
    }
    return 0;
}

static void campCamera_drawWithPosition(sCameraTask* pThis)
{
    sVec3_FP local_14;
    transformAndAddVecByCurrentMatrix(&pThis->m20_lightPosition, &local_14);
    dragonFieldTaskDrawSub1Sub1(local_14.m0_X, local_14.m4_Y, local_14.m8_Z, pThis->m2C);
    setupLight(0, 0, 0, pThis->m10.toU32());
}

static void campCameraSetupLight(sCameraTask* pThis, sSaturnPtr lightData)
{
    pThis->m8_colorData = lightData;

    pThis->m14[0] = 0;
    pThis->m14[1] = 0;
    pThis->m14[2] = 0;

    pThis->m10.m0 = readSaturnU8(lightData);
    pThis->m10.m1 = readSaturnU8(lightData + 1);
    pThis->m10.m2 = readSaturnU8(lightData + 2);
    pThis->m30_colorIntensity = 0x8000;

    u32 f0 = (u32)readSaturnU8(lightData + 5) << 16 | (u32)readSaturnU8(lightData + 4) << 8 | (u32)readSaturnU8(lightData + 3);
    u32 f1 = (u32)readSaturnU8(lightData + 8) << 16 | (u32)readSaturnU8(lightData + 7) << 8 | (u32)readSaturnU8(lightData + 6);
    u32 f2 = (u32)readSaturnU8(lightData + 11) << 16 | (u32)readSaturnU8(lightData + 10) << 8 | (u32)readSaturnU8(lightData + 9);
    generateLightFalloffMap(f0, f1, f2);
}

static s32 campCameraSetupWithPosition(sSaturnPtr arg)
{
    campCameraSetupLight(cameraTaskPtr, arg + 0x14);

    cameraTaskPtr->m_UpdateMethod = townCamera_update;
    cameraTaskPtr->m_DrawMethod = campCamera_drawWithPosition;

    cameraTaskPtr->m20_lightPosition = readSaturnVec3(arg);
    cameraTaskPtr->m2C = readSaturnU32(arg + 0xC);
    cameraTaskPtr->m30_colorIntensity = readSaturnU32(arg + 0x10);

    if (g_fadeControls.m_4C <= g_fadeControls.m_4D)
    {
        vdp2Controls.m20_registers[0].m112_CLOFSL = 0x10;
        vdp2Controls.m20_registers[1].m112_CLOFSL = 0x10;
    }

    resetProjectVector();
    cameraTaskPtr->m2 = 1;
    cameraTaskPtr->m0_colorMode = 2;
    return 0;
}

struct TWN_CAMP_data* gTWN_CAMP = NULL;
struct TWN_CAMP_data : public sTownOverlay
{
    static void makeCurrent()
    {
        if (gTWN_CAMP == NULL)
        {
            gTWN_CAMP = new TWN_CAMP_data();
        }
        gCurrentTownOverlay = gTWN_CAMP;
    }

    TWN_CAMP_data() : sTownOverlay("TWN_CAMP.PRG")
    {
        overlayScriptFunctions.m_zeroArg[0x0606cb54] = &scriptFunction_606cb54;
        overlayScriptFunctions.m_zeroArg[0x0606c952] = &scriptFunction_0606c952;
        overlayScriptFunctions.m_zeroArg[0x0606c95c] = &scriptFunction_0606c95c;
        overlayScriptFunctions.m_zeroArg[0x06071ade] = &scriptFunction_06071ade_toggleDayNight;

        overlayScriptFunctions.m_oneArg[0x6071e20] = &TwnFadeOut;
        overlayScriptFunctions.m_oneArg[0x6071da8] = &TwnFadeIn;
        overlayScriptFunctions.m_oneArg[0x60541c4] = &scriptFunction_60541c4;
        overlayScriptFunctions.m_oneArg[0x6056484] = &scriptFunction_6056484;

        overlayScriptFunctions.m_oneArgPtr[0x606C70C] = &scriptFunction_606c70c;
        overlayScriptFunctions.m_oneArgPtr[0x6071ca4] = &campCameraSetupWithPosition;

        overlayScriptFunctions.m_twoArg[0x6071b40] = &townCamera_setup;

        overlayScriptFunctions.m_fourArg[0x60704c4] = &setNpcLocation;
        overlayScriptFunctions.m_fourArg[0x60704f2] = &setNpcOrientation;

        for (int i = 0; i < 8; i++) {
            sSaturnPtr ptrBase = getSaturnPtr(0x6074068) + 0xC * i;
            mTownSetups.push_back(readTownSetup(ptrBase, 15));
        }
        
    }

    sTownObject* createObjectTaskFromEA_siblingTaskWithEAArgWithCopy(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg) override
    {
        assert(definitionEA.m_file == this);
        assert(arg.m_file == this);

        switch (definitionEA.m_offset)
        {
        case 0x60800E8:
            assert(size == 0xE0);
            return createExcaEntity(parent, arg);
        case 0x0607b2c4:
            assert(size == 0xE0);
            return createCampFire(parent, arg);
        case 0x06080124:
            assert(size == 0x28);
            return createCampParticle(parent, arg);
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
        case 0x0607b24c:
            assert(size == 0x624);
            return createCampDragon(parent, arg);
        default:
            assert(0);
            break;
        }
        return nullptr;

    }
};

int scriptFunction_60541c4(int arg) {
    asyncDmaCopy(readSaturnEA(gTWN_CAMP->getSaturnPtr(0x60760cc) + arg * 4 * 4), getVdp2Cram(0xA00), 0x200, 0);
    asyncDmaCopy(readSaturnEA(gTWN_CAMP->getSaturnPtr(0x60760d0) + arg * 4 * 4), getVdp2Cram(0x0), 0x200, 0);
    loadFile(readSaturnString(readSaturnEA(gTWN_CAMP->getSaturnPtr(0x60760d4) + arg * 4 * 4)).c_str(), getVdp2Vram(0x40000), 0);
    loadFile(readSaturnString(readSaturnEA(gTWN_CAMP->getSaturnPtr(0x60760d8) + arg * 4 * 4)).c_str(), getVdp2Vram(0x60000), 0);

    if (arg == 7) {
        assert(0);
    }

    return arg;
}



static const char* listOfFilesToLoad[] = {
    "COMMON3.MCB",
    "COMMON3.CGB",
    "CAMPOBJ.MCB",
    "CAMPOBJ.CGB",
    "CAMPMP1.MCB",
    "CAMPMP1.CGB",
    "CAMPMP2.MCB",
    "CAMPMP2.CGB",
    "CAMPMP3.MCB",
    "CAMPMP3.CGB",
    "CAMPMP4.MCB",
    "CAMPMP4.CGB",
    "CAMPMP5.MCB",
    "CAMPMP5.CGB",
    "CAMPMP6.MCB",
    "CAMPMP6.CGB",
    "CAMPMP7.MCB",
    "CAMPMP7.CGB",
    "CAMPMP8.MCB",
    "CAMPMP8.CGB",
    "C_DRA0.MCB",
    (const char*)-1,
    "C_DRA1.MCB",
    (const char*)-1,
    "C_DRA2.MCB",
    (const char*)-1,
    "C_DRA3.MCB",
    (const char*)-1,
    "C_DRA4.MCB",
    (const char*)-1,
    "C_DRA5.MCB",
    (const char*)-1,
    "C_DRA6.MCB",
    (const char*)-1,
    "C_DRA7.MCB",
    (const char*)-1,
    "Z_A_GS.MCB",
    "Z_A_GS.CGB",
    "X_A_PT.MCB",
    "X_A_PT.CGB",
    "X_A_AZ.MCB",
    "X_A_AZ.CGB",
    nullptr
};


struct sCampFire : public s_workAreaTemplateWithArgAndBase<sCampFire, sTownObject, sSaturnPtr>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sCampFire::Init, &sCampFire::Update, nullptr, nullptr };
        return &taskDefinition;
    }

    static void Init(sCampFire* pThis, sSaturnPtr arg)
    {
        pThis->m10 = arg;
        pThis->m14_position = readSaturnVec3(arg + 8);
        pThis->m20_rotation = readSaturnVec3(arg + 0x14);
        pThis->mD_fireLit = (bool)(mainGameState.getBit(8));
    }

    static void Update(sCampFire* pThis)
    {
        if (isDataLoaded(readSaturnS32(pThis->m10))) {
            sModelHierarchy* pHierarchy = pThis->m0_fileBundle->getModelHierarchy(4);
            sStaticPoseData* pStaticPose = pThis->m0_fileBundle->getStaticPose(0x438, pHierarchy->countNumberOfBones());

            init3DModelRawData(pThis, &pThis->m2C_model, 0, pThis->m0_fileBundle, 4, pThis->m0_fileBundle->getAnimation(0x488), pStaticPose, nullptr, nullptr);

            pThis->m7C_scriptContext.m30_pPosition = &pThis->m14_position;
            pThis->m7C_scriptContext.m34_pRotation = &pThis->m20_rotation;
            pThis->m7C_scriptContext.m38_pOwner = pThis;
            pThis->m7C_scriptContext.m3C_scriptEA = readSaturnEA(pThis->m10 + 0x20);
            pThis->m7C_scriptContext.m40 = 0;
            setCollisionSetup(&pThis->m7C_scriptContext, 4);
            setCollisionBounds(&pThis->m7C_scriptContext, readSaturnVec3(gTWN_CAMP->getSaturnPtr(0x607b2ac)), readSaturnVec3(gTWN_CAMP->getSaturnPtr(0x607b2b8)));

            pThis->m_UpdateMethod = Update2;
            pThis->m_DrawMethod = Draw2;
        }
    }

    static u16 computeModulatedColor(s8 interpolatedR, s8 interpolatedG, s8 interpolatedB, s8 baseR, s8 baseG, s8 baseB, u32 intensity)
    {
        s32 r = ((s32)interpolatedR * (s32)intensity) >> 16;
        s32 g = ((s32)interpolatedG * (s32)intensity) >> 16;
        s32 b = ((s32)interpolatedB * (s32)intensity) >> 16;
        r = ((r + baseR) >> 1) + 8;
        g = ((g + baseG) >> 1) + 8;
        b = ((b + baseB) >> 1) + 8;
        return 0x8000 | (r & 0x1F) | ((g & 0x1F) << 5) | ((b & 0x1F) << 10);
    }

    static void Update2(sCampFire* pThis)
    {
        registerCollisionBody(&pThis->m7C_scriptContext);

        if (!pThis->mD_fireLit) {
            return;
        }

        pThis->mC_animCounter++;
        if (pThis->mC_animCounter > 9) {
            pThis->mC_animCounter = 0;
        }

        sSaturnPtr dataA = gTWN_CAMP->getSaturnPtr(0x607b294);
        sSaturnPtr dataB = gTWN_CAMP->getSaturnPtr(0x607b2a0);

        sSaturnPtr source, target;
        s32 frame;
        if (pThis->mC_animCounter < 5) {
            source = dataA;
            target = dataB;
            frame = pThis->mC_animCounter;
        } else {
            source = dataB;
            target = dataA;
            frame = pThis->mC_animCounter - 5;
        }

        fixedPoint t = FP_Div(frame, 5);

        s8 srcR = readSaturnS8(source + 0), tgtR = readSaturnS8(target + 0);
        s8 srcG = readSaturnS8(source + 1), tgtG = readSaturnS8(target + 1);
        s8 srcB = readSaturnS8(source + 2), tgtB = readSaturnS8(target + 2);

        s32 r = (s16)((((s32)(tgtR - srcR) * (s32)t + 0x8000) >> 16)) + srcR;
        s32 g = (s16)((((s32)(tgtG - srcG) * (s32)t + 0x8000) >> 16)) + srcG;
        s32 b = (s16)((((s32)(tgtB - srcB) * (s32)t + 0x8000) >> 16)) + srcB;

        cameraTaskPtr->m10.m0 = (s8)r;
        cameraTaskPtr->m10.m1 = (s8)g;
        cameraTaskPtr->m10.m2 = (s8)b;

        s32 srcParam1 = readSaturnS32(source + 4);
        s32 tgtParam1 = readSaturnS32(target + 4);
        cameraTaskPtr->m2C = (s32)MTH_Mul(fixedPoint(tgtParam1 - srcParam1), t) + srcParam1;

        s32 srcParam2 = readSaturnS32(source + 8);
        s32 tgtParam2 = readSaturnS32(target + 8);
        cameraTaskPtr->m30_colorIntensity = (s32)MTH_Mul(fixedPoint(tgtParam2 - srcParam2), t) + srcParam2;

        if (g_fadeControls.m24_fade1.m20_stopped && cameraTaskPtr->m1_fadeActive) {
            s8 baseR = readSaturnS8(cameraTaskPtr->m8_colorData + 3);
            s8 baseG = readSaturnS8(cameraTaskPtr->m8_colorData + 4);
            s8 baseB = readSaturnS8(cameraTaskPtr->m8_colorData + 5);

            u16 color = computeModulatedColor((s8)r, (s8)g, (s8)b, baseR, baseG, baseB, cameraTaskPtr->m30_colorIntensity);
            fadePalette(&g_fadeControls.m24_fade1, color, color, 1);
        }

        stepAnimation(&pThis->m2C_model);
    }
    static void Draw2(sCampFire* pThis) {
        pushCurrentMatrix();
        translateCurrentMatrix(pThis->m14_position);
        rotateCurrentMatrixZYX(pThis->m20_rotation);
        if (pThis->mD_fireLit == 0) {
            addObjectToDrawList(pThis->m0_fileBundle->get3DModel(0xF8));
        }
        else {
            addObjectToDrawList(pThis->m0_fileBundle->get3DModel(0xF4));
            pThis->m2C_model.m18_drawFunction(&pThis->m2C_model);
        }

        popMatrix();
    }

    s8 mC_animCounter;
    bool mD_fireLit;
    sSaturnPtr m10;
    sVec3_FP m14_position;
    sVec3_FP m20_rotation;
    s_3dModel m2C_model;
    sCollisionBody m7C_scriptContext;
    //size: 0xE0
};

sTownObject* createCampFire(s_workAreaCopy* parent, sSaturnPtr arg) {
    return createSubTaskWithArgWithCopy<sCampFire, sSaturnPtr>(parent, arg);
}

struct sCampParticle : public s_workAreaTemplateWithArgAndBase<sCampParticle, sTownObject, sSaturnPtr>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sCampParticle::Init, &sCampParticle::Update, nullptr, &sCampParticle::Delete };
        return &taskDefinition;
    }

    static void Init(sCampParticle* pThis, sSaturnPtr arg)
    {
        pThis->mC = arg;
        pThis->m18_position[0] = readSaturnFP(arg + 8);
        pThis->m18_position[1] = readSaturnFP(arg + 0xC);
        pThis->m18_position[2] = readSaturnFP(arg + 0x10);
        pThis->m24_mode = readSaturnU8(arg + 0x24);

        s16 npcIndex = readSaturnS16(arg + 0x26);
        if (npcIndex >= 0) {
            npcData0.m70_npcPointerArray[npcIndex].workArea = pThis;
        }
    }

    static void Update(sCampParticle* pThis)
    {
        if (isDataLoaded(readSaturnS32(pThis->mC))) {
            pThis->m_quadData = initVdp1Quad(readSaturnEA(pThis->mC + 0x20));
            particleInitSub(&pThis->m10_animQuad, pThis->m4_vd1Allocation->m4_vdp1Memory, &pThis->m_quadData);

            pThis->m_UpdateMethod = Update2;
            pThis->m_DrawMethod = Draw2;
        }
    }

    static void Update2(sCampParticle* pThis)
    {
        if (pThis->m24_mode == 0) {
            sGunShotTask_UpdateSub4(&pThis->m10_animQuad);
        } else if (pThis->m24_mode == 1) {
            if (pThis->m8_cellNode && pThis->m8_cellNode->m8 == pThis) {
                pThis->m8_cellNode->m8 = nullptr;
            }
            decreaseNPCRefCount(readSaturnS32(pThis->mC));
            pThis->getTask()->markFinished();
        }
    }

    static void Draw2(sCampParticle* pThis)
    {
        drawProjectedParticle(&pThis->m10_animQuad, &pThis->m18_position);
    }

    static void Delete(sCampParticle* pThis)
    {
        s16 npcIndex = readSaturnS16(pThis->mC + 0x26);
        if (npcIndex >= 0 && npcData0.m70_npcPointerArray[npcIndex].workArea == pThis) {
            npcData0.m70_npcPointerArray[npcIndex].workArea = nullptr;
        }
    }

    sSaturnPtr mC;
    sAnimatedQuad m10_animQuad;
    sVec3_FP m18_position;
    u8 m24_mode;
    std::vector<sVdp1Quad> m_quadData;
    // size 0x26
};

static sTownObject* createCampParticle(s_workAreaCopy* parent, sSaturnPtr arg)
{
    return createSubTaskWithArgWithCopy<sCampParticle, sSaturnPtr>(parent, arg);
}


void townOverlayDelete_TwnCamp(townDebugTask2Function* pThis)
{
    Unimplemented();

    freeRamResources(pThis);
    vdp1FreeLastAllocation(pThis);
    unloadFnt();
}

p_workArea overlayStart_TWN_CAMP(p_workArea pUntypedThis, u32 arg)
{
    gTWN_CAMP->makeCurrent();

    townDebugTask2Function* pThis = static_cast<townDebugTask2Function*>(pUntypedThis);

    pThis->m_DeleteMethod = &townOverlayDelete_TwnCamp;

    loadSoundBanks(-1, 0);
    loadSoundBanks(64, 0);

    playPCM(pThis, 100);

    graphicEngineStatus.m405C.m10_nearClipDistance = 0x800;
    graphicEngineStatus.m405C.m30_oneOverNearClip = FP_Div(0x10000, graphicEngineStatus.m405C.m10_nearClipDistance);

    graphicEngineStatus.m405C.m14_farClipDistance = 0x300000;
    graphicEngineStatus.m405C.m38_oneOverFarClip = FP_Div(0x8000, graphicEngineStatus.m405C.m14_farClipDistance);

    graphicEngineStatus.m405C.m34_oneOverFarClip256 = graphicEngineStatus.m405C.m38_oneOverFarClip << 8;

    initDramAllocator(pThis, townBuffer, sizeof(townBuffer), listOfFilesToLoad);

    initVdp1Ram(pThis, 0x25C18800, 0x63800);

    registerNpcs(gTWN_CAMP->mTownSetups, gTWN_CAMP->getSaturnPtr(0x6057f88), arg);

    startScriptTask(pThis);

    if (arg == 7) {
        createCampVdp2PlaneAlt(pThis);
    }
    else {
        createCampVdp2Plane(pThis);
    }

    sEdgeTask* pEdgeTask = nullptr;
    if (arg == 8) {
        pEdgeTask = startEdgeTask(gTWN_CAMP->getSaturnPtr(0x60741ec));
    }
    else {
        pEdgeTask = startEdgeTask(gTWN_CAMP->getSaturnPtr(0x6074198));
    }

    npcData0.m160_pEdgePosition = &pEdgeTask->m84.m8_position;

    startMainLogic(pThis);

    twnMainLogicTask->m14_EdgeTask = pEdgeTask;

    startCameraTask(pThis);

    return pThis;
}
