#include "PDS.h"
#include "twn_exca.h"
#include "town/town.h"
#include "town/townScript.h"
#include "town/townEdge.h"
#include "town/townMainLogic.h"
#include "kernel/vdp1Allocator.h"

struct TWN_EXCA_data : public sTownOverlay
{
    void init() override
    {
        gCurrentTownOverlay = this;

        overlayScriptFunctions.m_oneArg[0x605d780] = &TwnFadeOut;
    }

    sTownObject* createObjectTaskFromEA_siblingTaskWithEAArgWithCopy(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg) override
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

    sTownObject* createObjectTaskFromEA_subTaskWithEAArg(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg) override
    {
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

    playMusic(-1, 0);
    playMusic(59, 0);

    playPCM(pThis, 100);

    loadFnt("EVTEXCA.FNT");

    graphicEngineStatus.m405C.m10_nearClipDistance = 0x800;
    graphicEngineStatus.m405C.m30 = FP_Div(0x10000, graphicEngineStatus.m405C.m10_nearClipDistance);

    graphicEngineStatus.m405C.m14_farClipDistance = 0x100000;
    graphicEngineStatus.m405C.m38 = FP_Div(0x8000, graphicEngineStatus.m405C.m14_farClipDistance);

    graphicEngineStatus.m405C.m34 = graphicEngineStatus.m405C.m38 << 8;

    initDramAllocator(pThis, townBuffer, sizeof(townBuffer), listOfFilesToLoad);

    initVdp1Ram(pThis, 0x25C18800, 0x63800);

    registerNpcs(gTWN_EXCA->getSaturnPtr(0x605fd4c), gTWN_EXCA->getSaturnPtr(0x6054768), arg);

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
