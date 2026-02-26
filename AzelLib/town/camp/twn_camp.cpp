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

void unloadFnt(); // TODO: fix
int scriptFunction_60541c4(int arg);
void setupCameraUpdateForCurrentMode(); // todo clean

struct sCampEntity0 : public s_workAreaTemplateWithArgWithCopy<sCampEntity0, sSaturnPtr>, sTownObject
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sCampEntity0::Init, &sCampEntity0::Update, nullptr, nullptr };
        return &taskDefinition;
    }

    static void Init(sCampEntity0* pThis, sSaturnPtr arg)
    {
        pThis->m10 = arg;
        pThis->m14_position = readSaturnVec3(arg + 8);
        pThis->m20_rotation = readSaturnVec3(arg + 0x14);
        pThis->mD = (bool)(mainGameState.getBit(8));
    }

    static void Update(sCampEntity0* pThis)
    {
        Unimplemented();
    }

    bool mD;
    sSaturnPtr m10;
    sVec3_FP m14_position;
    sVec3_FP m20_rotation;
    //size: 0xE0
};

sTownObject* createCampEntity(s_workAreaCopy* parent, sSaturnPtr arg) {
    return createSubTaskWithArgWithCopy<sCampEntity0, sSaturnPtr>(parent, arg);
}

int scriptFunction_606cb54() {
    if (twnMainLogicTask->m14_EdgeTask) {
        mainLogicUpdateSub0(twnMainLogicTask->m14_EdgeTask->mE8.m0_position[0], twnMainLogicTask->m14_EdgeTask->mE8.m0_position[2]);
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
        twnMainLogicTask->mE4 = vector1;
        twnMainLogicTask->m100 = vector2 - vector1;
        twnMainLogicTask->mF8[0] = readSaturnFP(arg + 4 * 6);
        twnMainLogicTask->mF0[0] = readSaturnFP(arg + 4 * 7);
        twnMainLogicTask->m10C[0] = 0;
        twnMainLogicTask->m10C[1] = readSaturnFP(arg + 4 * 8);
        twnMainLogicTask->m10C[2] = readSaturnFP(arg + 4 * 9);

        fixedPoint temp = twnMainLogicTask->m10C[2] + atan2_FP(-twnMainLogicTask->m100[2], twnMainLogicTask->m100[0]);
        twnMainLogicTask->mF0[1] = temp - twnMainLogicTask->m10C[1];
        twnMainLogicTask->mF8[1] = temp + twnMainLogicTask->m10C[1];

        twnMainLogicTask->m44_cameraTarget = twnMainLogicTask->m14_EdgeTask->mE8.m0_position;
        twnMainLogicTask->m5C_rawCameraPosition = twnMainLogicTask->mE4;

        twnMainLogicTask->m2_cameraFollowMode = 3;

        setupCameraUpdateForCurrentMode();
    }
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

        overlayScriptFunctions.m_oneArg[0x6071e20] = &TwnFadeOut;
        overlayScriptFunctions.m_oneArg[0x6071da8] = &TwnFadeIn;
        overlayScriptFunctions.m_oneArg[0x60541c4] = &scriptFunction_60541c4;
        overlayScriptFunctions.m_oneArg[0x6056484] = &scriptFunction_6056484;

        overlayScriptFunctions.m_oneArgPtr[0x606C70C] = &scriptFunction_606c70c;

        overlayScriptFunctions.m_twoArg[0x6071b40] = &townCamera_setup;

        overlayScriptFunctions.m_fourArg[0x60704c4] = &setNpcLocation;
        overlayScriptFunctions.m_fourArg[0x60704f2] = &setNpcOrientation;

        mTownSetups.push_back(readTownSetup(getSaturnPtr(0x6074068), 1));
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
            return createCampEntity(parent, arg);
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

void townOverlayDelete_TwnCamp(townDebugTask2Function* pThis)
{
    Unimplemented();

    freeRamResources(pThis);
    vdp1FreeLastAllocation(pThis);
    unloadFnt();
}

void startCampBackgroundTask7(townDebugTask2Function* pThis) {
    Unimplemented();

    // hack: to remove title screen
    reinitVdp2();
}

void startCampBackgroundTask(townDebugTask2Function* pThis) {
    Unimplemented();

    // hack: to remove title screen
    reinitVdp2();
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
        startCampBackgroundTask7(pThis);
    }
    else {
        startCampBackgroundTask(pThis);
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
