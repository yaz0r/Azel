#include "PDS.h"
#include "twn_e006.h"
#include "town/town.h"
#include "town/townScript.h"
#include "town/townEdge.h"
#include "town/townMainLogic.h"
#include "kernel/vdp1Allocator.h"

s32 scriptFunction_605bb24(s32 r4, s32 r5)
{
    FunctionUnimplemented();
    return 0;
}

s32 scriptFunction_60573b0(s32 r4)
{
    FunctionUnimplemented();
    return 0;
}

s32 scriptFunction_605861e(s32 r4)
{
    FunctionUnimplemented();
    return 0;
}

s32 scriptFunction_605838C(s32 r4)
{
    if (g_fadeControls.m_4D >= g_fadeControls.m_4C)
    {
        vdp2Controls.m20_registers[0].m112_CLOFSL = r4;
        vdp2Controls.m20_registers[1].m112_CLOFSL = r4;
    }

    return 0;
}

s32 scriptFunction_60573d8()
{
    FunctionUnimplemented();
    return 0;
}

s32 scriptFunction_6056918()
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

s32 scriptFunction_6057438()
{
    FunctionUnimplemented();
    return 0;
}

s32 scriptFunction_6057470()
{
    FunctionUnimplemented();
    return 0;
}

struct TWN_E006_data : public sTownOverlay
{
    void init() override
    {
        gCurrentTownOverlay = this;

        overlayScriptFunctions.m_zeroArg[0x60573d8] = &scriptFunction_60573d8;
        overlayScriptFunctions.m_zeroArg[0x6056918] = &scriptFunction_6056918;
        overlayScriptFunctions.m_zeroArg[0x6057438] = &scriptFunction_6057438;
        overlayScriptFunctions.m_zeroArg[0x6057470] = &scriptFunction_6057470;
        overlayScriptFunctions.m_zeroArg[0x6056926] = &scriptFunction_6056926;

        overlayScriptFunctions.m_oneArg[0x60573b0] = &scriptFunction_60573b0;
        overlayScriptFunctions.m_oneArg[0x605861e] = &scriptFunction_605861e;
        overlayScriptFunctions.m_oneArg[0x605838C] = &scriptFunction_605838C;
        overlayScriptFunctions.m_oneArg[0x605be04] = &TwnFadeOut;
        overlayScriptFunctions.m_oneArg[0x605bd8c] = &TwnFadeIn;

        overlayScriptFunctions.m_twoArg[0x605bb24] = &scriptFunction_605bb24;
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


TWN_E006_data* gTWN_E006 = NULL;

static void townOverlayDelete(townDebugTask2Function* pThis)
{
    FunctionUnimplemented();
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
    FunctionUnimplemented();

    // hack: to remove title screen
    reinitVdp2();
}

void setupVdp1Proj(fixedPoint fov)
{
    initVDP1Projection(fov, 0);
}

p_workArea overlayStart_TWN_E006(p_workArea pUntypedThis, u32 arg)
{
    // load data
    if (gTWN_E006 == NULL)
    {
        FILE* fHandle = fopen("TWN_E006.PRG", "rb");
        assert(fHandle);

        fseek(fHandle, 0, SEEK_END);
        u32 fileSize = ftell(fHandle);

        fseek(fHandle, 0, SEEK_SET);
        u8* fileData = new u8[fileSize];
        fread(fileData, fileSize, 1, fHandle);
        fclose(fHandle);

        gTWN_E006 = new TWN_E006_data();
        gTWN_E006->m_name = "TWN_E006.PRG";
        gTWN_E006->m_data = fileData;
        gTWN_E006->m_dataSize = fileSize;
        gTWN_E006->m_base = 0x6054000;

        gTWN_E006->init();
    }
    townDebugTask2Function* pThis = static_cast<townDebugTask2Function*>(pUntypedThis);

    pThis->m_DeleteMethod = &townOverlayDelete;

    playMusic(-1, 0);
    playMusic(68, 0);

    stopAllSounds();

    graphicEngineStatus.m405C.m10_nearClipDistance = 0x800;
    graphicEngineStatus.m405C.m30 = FP_Div(0x10000, graphicEngineStatus.m405C.m10_nearClipDistance);

    graphicEngineStatus.m405C.m14_farClipDistance = 0x200000;
    graphicEngineStatus.m405C.m38 = FP_Div(0x8000, graphicEngineStatus.m405C.m14_farClipDistance);

    graphicEngineStatus.m405C.m34 = graphicEngineStatus.m405C.m38 << 8;

    initDramAllocator(pThis, townBuffer, sizeof(townBuffer), listOfFilesToLoad);

    initVdp1Ram(pThis, 0x25C18800, 0x63800);

    registerNpcs(gTWN_E006->getSaturnPtr(0x605e1c0), gTWN_E006->getSaturnPtr(0x605414c), arg);

    startScriptTask(pThis);

    startE006BackgroundTask(pThis);

    startMainLogic(pThis);

    setupVdp1Proj(0x1c71c71);

    startCameraTask(pThis);

    return pThis;
}
