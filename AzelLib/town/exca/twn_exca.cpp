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
#include "audio/soundDriver.h"

struct sTownDragon : public s_workAreaTemplateWithArg<sTownDragon, sSaturnPtr>, sTownObject
{
    static TypedTaskDefinition * getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sTownDragon::Init, &sTownDragon::Update, &sTownDragon::Draw, &sTownDragon::Delete };
        return &taskDefinition;
    }

    static void Init(sTownDragon* pThis, sSaturnPtr arg)
    {
        FunctionUnimplemented();
    }

    static void Update(sTownDragon* pThis)
    {
        FunctionUnimplemented();
    }

    static void Draw(sTownDragon* pThis)
    {
        FunctionUnimplemented();
    }

    static void Delete(sTownDragon* pThis)
    {
        FunctionUnimplemented();
    }

    //size: 0xE8
};

struct sExcaEntity0 : public s_workAreaTemplateWithArgWithCopy<sExcaEntity0, sSaturnPtr>, sTownObject
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sExcaEntity0::Init, &sExcaEntity0::Update, nullptr, &sExcaEntity0::Delete };
        return &taskDefinition;
    }

    static void Init(sExcaEntity0* pThis, sSaturnPtr arg)
    {
        pThis->mC = arg;
        pThis->mC4_position = readSaturnVec3(arg + 8);
        pThis->mD0_rotation = readSaturnVec3(arg + 0x14);
        pThis->mDC_status = readSaturnU8(arg + 0x26);

        s16 npcIndex = readSaturnS16(arg + 0x2c);
        if (npcIndex > -1) {
            npcData0.m70_npcPointerArray[npcIndex].workArea = pThis;
        }
    }

    static void Update(sExcaEntity0* pThis)
    {
        s32 fileIndex = readSaturnU32(pThis->mC);
        if (isDataLoaded(fileIndex))
        {
            s16 modelIndex = readSaturnS16(pThis->mC + 0x20);
            s16 animationIndex = readSaturnS16(pThis->mC + 0x22);
            s16 poseIndex = readSaturnS16(pThis->mC + 0x24);

            sModelHierarchy* pHierarchy = pThis->m0_fileBundle->getModelHierarchy(modelIndex);
            sStaticPoseData* pPose = pThis->m0_fileBundle->getStaticPose(poseIndex, pHierarchy->countNumberOfBones());

            if (animationIndex == 0)
            {
                init3DModelRawData(pThis, &pThis->m10_3dModel, 0, pThis->m0_fileBundle, modelIndex, nullptr, pPose, nullptr, nullptr);
            }
            else
            {
                sAnimationData* pAnimation = pThis->m0_fileBundle->getAnimation(animationIndex);
                init3DModelRawData(pThis, &pThis->m10_3dModel, 0, pThis->m0_fileBundle, modelIndex, pAnimation, pPose, nullptr, nullptr);
                stepAnimation(&pThis->m10_3dModel);
            }

            sSaturnPtr scriptConfigEA = readSaturnEA(pThis->mC + 0x28);
            if (!scriptConfigEA.isNull())
            {
                pThis->m60_scriptContext.m30_pPosition = &pThis->mC4_position;
                pThis->m60_scriptContext.m34_pRotation = &pThis->mD0_rotation;
                pThis->m60_scriptContext.m38_pOwner = pThis;
                pThis->m60_scriptContext.m3C_scriptEA = readSaturnEA(scriptConfigEA + 4);

                s16 sVar1 = readSaturnS16(scriptConfigEA + 2) >> 0xF;
                u8* puVar3 = nullptr;
                if (sVar1)
                {
                    puVar3 = pThis->mC.getRawPointer() + sVar1;
                }

                pThis->m60_scriptContext.m40 = puVar3;

                mainLogicInitSub0(&pThis->m60_scriptContext, readSaturnU8(scriptConfigEA + 0));
                mainLogicInitSub1(&pThis->m60_scriptContext, readSaturnVec3(scriptConfigEA + 0x8), readSaturnVec3(scriptConfigEA + 0x14));
            }

            pThis->m_UpdateMethod = &sExcaEntity0::Update2;
            pThis->m_DrawMethod = &sExcaEntity0::Draw2;
        }
    }

    static void Update2(sExcaEntity0* pThis)
    {
        switch (pThis->mDC_status)
        {
        case 0:
            break;
        case 1:
        {
            stepAnimation(&pThis->m10_3dModel);
            int numFrames = 0;
            if (pThis->m10_3dModel.m30_pCurrentAnimation)
            {
                numFrames = pThis->m10_3dModel.m30_pCurrentAnimation->m4_numFrames;
            }
            if (numFrames - 1 <= pThis->m10_3dModel.m16_previousAnimationFrame)
            {
                pThis->mDC_status = 0;
            }
            break;
        }
        case 2:
            stepAnimation(&pThis->m10_3dModel);
            break;
        case 3:
            assert(0); //delete
            break;
        default:
            assert(0);
            break;
        }

        // don't update if there is no script config
        if (readSaturnEA(pThis->mC + 0x28).isNull())
            return;

        EdgeUpdateSub0(&pThis->m60_scriptContext);
    }

    static void Draw2(sExcaEntity0* pThis)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(pThis->mC4_position);
        rotateCurrentMatrixZYX(pThis->mD0_rotation);
        pThis->m10_3dModel.m18_drawFunction(&pThis->m10_3dModel);
        popMatrix();
    }

    static void Delete(sExcaEntity0* pThis)
    {
        FunctionUnimplemented();
    }

    sSaturnPtr mC;
    s_3dModel m10_3dModel;
    sMainLogic_74 m60_scriptContext;
    sVec3_FP mC4_position;
    sVec3_FP mD0_rotation;
    u8 mDC_status;
    //size: 0xE0
};

struct TWN_EXCA_data : public sTownOverlay
{
    void init() override
    {
        gCurrentTownOverlay = this;

        overlayScriptFunctions.m_oneArg[0x605d780] = &TwnFadeOut;

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
            return createSiblingTaskWithArgWithCopy<sExcaEntity0, sSaturnPtr>(parent, arg);
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
            return createSubTaskWithArg<sTownDragon, sSaturnPtr>(parent, arg);
        default:
            assert(0);
            break;
        }
        return nullptr;

    }
};


TWN_EXCA_data* gTWN_EXCA = NULL;

static void townOverlayDelete(townDebugTask2Function* pThis)
{
    FunctionUnimplemented();
}

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

static void startExcaBackgroundTask(p_workArea pThis)
{
    FunctionUnimplemented();

    // hack: to remove title screen
    reinitVdp2();
}

p_workArea overlayStart_TWN_EXCA(p_workArea pUntypedThis, u32 arg)
{
    // load data
    if (gTWN_EXCA == NULL)
    {
        FILE* fHandle = fopen("TWN_EXCA.PRG", "rb");
        assert(fHandle);

        fseek(fHandle, 0, SEEK_END);
        u32 fileSize = ftell(fHandle);

        fseek(fHandle, 0, SEEK_SET);
        u8* fileData = new u8[fileSize];
        fread(fileData, fileSize, 1, fHandle);
        fclose(fHandle);

        gTWN_EXCA = new TWN_EXCA_data();
        gTWN_EXCA->m_name = "TWN_EXCA.PRG";
        gTWN_EXCA->m_data = fileData;
        gTWN_EXCA->m_dataSize = fileSize;
        gTWN_EXCA->m_base = 0x6054000;

        gTWN_EXCA->init();
    }
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
