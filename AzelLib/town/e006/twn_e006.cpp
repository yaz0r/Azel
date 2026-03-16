#include "PDS.h"
#include "twn_e006.h"
#include "town/townCutscene.h"
#include "town/town.h"
#include "town/townScript.h"
#include "town/townEdge.h"
#include "town/townMainLogic.h"
#include "kernel/vdp1Allocator.h"
#include "kernel/animation.h"
#include "kernel/fileBundle.h"
#include "audio/soundDriver.h"
#include "audio/systemSounds.h"
#include "town/townCamera.h"

struct TWN_E006_data* gTWN_E006 = nullptr;
struct TWN_E006_data : public sTownOverlay
{
    TWN_E006_data();
    static void makeCurrent()
    {
        if (gTWN_E006 == NULL)
        {
            gTWN_E006 = new TWN_E006_data();
        }
        gCurrentTownOverlay = gTWN_E006;
    }

    sTownObject* createObjectTaskFromEA_siblingTaskWithEAArgWithCopy(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg) override;
    sTownObject* createObjectTaskFromEA_subTaskWithEAArg(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg) override;
};

s32 createEPKPlayer(s32 r4)
{
    npcData0.mF4 = 1;
    npcData0.mF0 = 0;

    gCutsceneTask = createSubTaskWithArg<sCutsceneTask>(twnMainLogicTask, r4);

    return 0;
}

s32 e006_scriptFunction_605861eSub0()
{
    if ((gCutsceneTask != nullptr) && (npcData0.mF0 == 0))
    {
        s32 iVar1 = scriptFunction_605861eSub0Sub0(gCutsceneTask->m0);
        if ((iVar1 != 5) && (iVar1 != -1))
            return 0;
    }

    return 1;
}

struct sDragonCutsceneData
{
    s16 m0;
    s32 m4;
    s32 m8;
};

const std::array<sDragonCutsceneData, DR_LEVEL_MAX> dragonCutsceneData = {
    {
        {
            8,
            0xAA24AE,
            0xEE68F3,
        },
        {
            8,
            0xAA24AE,
            0xEE68F3,
        },
        {
            0x16,
            0xEE6608,
            0x132AA4C,
        },
        {
            0xA,
            0,
            0,
        },
        {
            0xB,
            0x120F336,
            0x165377A,
        },
        {
            0x18,
            0,
            0,
        },
        {
            0x16,
            0,
            0,
        },
        {
            0x7,
            0,
            0,
        },
        {
            0,
            0,
            0,
        },
    }
};

sVec3_FP gDragonCutscenePosition = { 0,0,0 };
sVec3_FP gDragonCutsceneRotation = { 0,0,0 };

struct sE006Task1 : public s_workAreaTemplate<sE006Task1>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { nullptr, &sE006Task1::Update, &sE006Task1::Draw, &sE006Task1::Delete };
        return &taskDefinition;
    }

    static void Update(sE006Task1* pThis)
    {
        u32 iVar2 = (gDragonState->mC_dragonType * 0xc) >> 8;
        s32 iVar3 = gDragonState->m28_dragon3dModel.m2C_poseData[dragonCutsceneData[iVar2].m0].mC_rotation[2];
        fixedPoint uVar1 = iVar3 - pThis->m8;
        if ((dragonCutsceneData[iVar2].m4 <= iVar3) && (iVar3 <= dragonCutsceneData[iVar2].m8 + iVar2)) {
            if ((uVar1.normalized() < -0x222222) && (-1 < readSaturnS16(pThis->m0 + 2))) {
                playSystemSoundEffect(readSaturnS16(pThis->m0 + 2));
            }
        }
        pThis->m8 = iVar3;
        return;
    }

    static void Draw(sE006Task1* pThis)
    {
        for (int i = 0; i < gDragonState->m28_dragon3dModel.m12_numBones; i++)
        {
            initMatrixToIdentity(&gDragonState->m28_dragon3dModel.m3C_boneMatrices[i]);
            translateMatrix(gDragonState->m28_dragon3dModel.m2C_poseData[i].m0_translation, &gDragonState->m28_dragon3dModel.m3C_boneMatrices[i]);
            rotateMatrixZYX(&gDragonState->m28_dragon3dModel.m2C_poseData[i].mC_rotation, &gDragonState->m28_dragon3dModel.m3C_boneMatrices[i]);
        }

        submitModelAndShadowModelToRendering(&gDragonState->m28_dragon3dModel, gDragonState->m14_modelIndex, gDragonState->m18_shadowModelIndex, &gDragonCutscenePosition, &gDragonCutsceneRotation, 0);
    }

    static void Delete(sE006Task1* pThis)
    {
        Unimplemented();
    }

    sSaturnPtr m0;
    s32 m8;
    // size: 0xc
};

s32 setupDragonEntityForCutscene(s32 r4)
{
    sSaturnPtr r4Ptr = sSaturnPtr::createFromRaw(r4, gCurrentTownOverlay);
    if (e006_scriptFunction_605861eSub0() == 0)
    {
        sE006Task1* pNewSubTask = createSubTask<sE006Task1>(currentResTask);
        pNewSubTask->m0 = r4Ptr;
        pNewSubTask->m8 = 0x80000000;

        setupCutsceneDragon(gCutsceneTask->m0, readSaturnU8(r4Ptr + 1), pNewSubTask, &gDragonState->m28_dragon3dModel);
        npcData0.m70_npcPointerArray[readSaturnU8(r4Ptr + 0)].workArea = pNewSubTask;
        setupModelAnimation(&gDragonState->m28_dragon3dModel, nullptr);
    }
    return 0;
}

s32 e006_scriptFunction_60573d8()
{
    npcData0.mF0 = 0;
    if (gCutsceneTask != nullptr)
    {
        scriptFunction_60573d8Sub0(gCutsceneTask->m0);
    }
    npcData0.mF0 = 0;
    return 0;
}

s32 e006_scriptFunction_6056918()
{
    twnMainLogicTask->m_UpdateMethod = nullptr;
    twnMainLogicTask->m_DrawMethod = nullptr;
    return 0;
}

s32 scriptFunction_6056926()
{
    twnMainLogicTask->m_UpdateMethod = &sMainLogic::Update;
    twnMainLogicTask->m_DrawMethod = &sMainLogic::Draw;
    return 0;
}

s32 e006_scriptFunction_6057438()
{
    if (gCutsceneTask)
    {
        if (npcData0.mF0 == 0)
        {
            s32 iVar1 = getPositionInEDK(gCutsceneTask->m0);
            return (int)(iVar1 + (u32)(iVar1 < 0)) >> 1;
        }
    }
    return 0;
}

s32 scriptFunction_6057470()
{
    Unimplemented();
    return 0;
}

TWN_E006_data::TWN_E006_data() : sTownOverlay("TWN_E006.PRG")
{
    overlayScriptFunctions.m_zeroArg[0x60573d8] = {&e006_scriptFunction_60573d8, "e006_scriptFunction_60573d8"};
    overlayScriptFunctions.m_zeroArg[0x6056918] = {&e006_scriptFunction_6056918, "e006_scriptFunction_6056918"};
    overlayScriptFunctions.m_zeroArg[0x6057438] = {&e006_scriptFunction_6057438, "e006_scriptFunction_6057438"};
    overlayScriptFunctions.m_zeroArg[0x6057470] = {&scriptFunction_6057470, "scriptFunction_6057470"};
    overlayScriptFunctions.m_zeroArg[0x6056926] = {&scriptFunction_6056926, "scriptFunction_6056926"};

    overlayScriptFunctions.m_oneArg[0x60573b0] = {&createEPKPlayer, "createEPKPlayer"};
    overlayScriptFunctions.m_oneArg[0x605861e] = {&setupDragonEntityForCutscene, "setupDragonEntityForCutscene"};
    overlayScriptFunctions.m_oneArg[0x605838C] = {&SetupColorOffset, "SetupColorOffset"};
    overlayScriptFunctions.m_oneArg[0x605be04] = {&TwnFadeOut, "TwnFadeOut"};
    overlayScriptFunctions.m_oneArg[0x605bd8c] = {&TwnFadeIn, "TwnFadeIn"};

    overlayScriptFunctions.m_twoArg[0x605bb24] = {&townCamera_setup, "townCamera_setup"};

    mTownSetups.push_back(readTownSetup(getSaturnPtr(0x605e1c0), 1));
}

sTownObject* TWN_E006_data::createObjectTaskFromEA_siblingTaskWithEAArgWithCopy(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg)
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

sTownObject* TWN_E006_data::createObjectTaskFromEA_subTaskWithEAArg(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg)
{
    return nullptr;
}

static const char* listOfFilesToLoad[] = {
    "COMMON3.MCB",
    "COMMON3.CGB",
    "EXCAVAMP.MCB",
    "EXCAVAMP.CGB",
    nullptr
};

static void startE006BackgroundTask(p_workArea pThis)
{
    Unimplemented();

    // hack: to remove title screen
    reinitVdp2();
}

void setupVdp1Proj(fixedPoint fov)
{
    initVDP1Projection(fov, 0);
}

p_workArea overlayStart_TWN_E006(p_workArea pUntypedThis, u32 arg)
{
    gTWN_E006->makeCurrent();
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

    registerNpcs(gTWN_E006->mTownSetups, gTWN_E006->getSaturnPtr(0x605414c), arg);

    startScriptTask(pThis);

    startE006BackgroundTask(pThis);

    startMainLogic(pThis);

    setupVdp1Proj(0x1c71c71);

    startCameraTask(pThis);

    return pThis;
}
