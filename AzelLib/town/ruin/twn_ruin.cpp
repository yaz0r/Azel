#include "PDS.h"
#include "twn_ruin.h"
#include "town/town.h"

struct TWN_RUIN_data : public sSaturnMemoryFile
{
    void init()
    {

    }
};

TWN_RUIN_data* gTWN_RUIN = NULL;

u8 townVDP1Buffer[0x63800];

s32 townVar0;

const char* listOfFilesToLoad[] = {
    "COMMON3.MCB",
    "COMMON3.CGB",
    "RUINMP.MCB",
    "RUINMP.CGB",
    nullptr
};

void townOverlayDelete(townDebugTask2Function* pThis)
{
    TaskUnimplemented();
}

void startRuinBackgroundTask(p_workArea pThis)
{
    TaskUnimplemented();
}

void registerNpcs(sSaturnPtr r4_objectTable, sSaturnPtr r5_script, s32 r6)
{
    npcData0.m0_numExtraScriptsIterations = 0;
    npcData0.m5E = -1;
    npcData0.m60 = r4_objectTable;
    npcData0.mFC = 0;
    npcData0.m100 = 0;
    npcData0.m11C_currentStackPointer = npcData0.m120_stack.end();
    npcData0.m164 = 0;
    npcData0.m168 = 0;
    npcData0.m16C = 0;
    npcData0.m170 = 0;

    for (int i = 0; i < npcData0.m70_npcPointerArray.size(); i++)
    {
        npcData0.m70_npcPointerArray[i] = nullptr;
    }

    townVar0 = 0;

    npcData0.mFC |= 0xF;

    npcData0.m104_scriptPtr = r5_script;
    npcData0.m108 = 0;
    npcData0.m10C = 0;

    npcData0.m116 = r6;

    npcData0.mF0 = 0;
    npcData0.mF4 = 0;
}

struct sEdgeTask : public s_workAreaTemplate<sEdgeTask>
{
    s32 m8C;
};

sEdgeTask* startEdgeTask(sSaturnPtr)
{
    return NULL;
}

s32* twnEdgeVar0;

struct sMainLogic* twnMainLogicTask;

s32 twnVar3 = 0xB0009;

struct sMainLogic : public s_workAreaTemplate<sMainLogic>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sMainLogic::Init, &sMainLogic::Update, &sMainLogic::Draw, NULL };
        return &taskDefinition;
    }

    static void Init(sMainLogic* pThis)
    {
        twnMainLogicTask = pThis;

        if (mainGameState.getBit(0x274, 7))
        {
            pThis->m1 = 1;
        }
        else
        {
            pThis->m1 = 0;
        }
        pThis->m0 = 0;
        pThis->m10 = &twnVar3;

        resetMatrixStack();

        pThis->mA4 = &pThis->m100;
        pThis->mA8 = &pThis->m68;
        pThis->mAC = &pThis->m0;
        pThis->mB0 = 0;
        pThis->mB4 = 0;

        mainLogicInitSub0(&pThis->m74, 0);
        mainLogicInitSub1(&pThis->m74, gTWN_RUIN->getSaturnPtr(0x605EEE4), gTWN_RUIN->getSaturnPtr(0x605EEF0));

        npcData0.mFC &= ~0x10;
    }

    static void Update(sMainLogic* pThis)
    {
        TaskUnimplemented();
    }

    static void Draw(sMainLogic* pThis)
    {
        TaskUnimplemented();
    }

    s8 m0;
    s8 m1;
    s32* m10;
    sEdgeTask* m14;
    s32 m68;
    sMainLogic_74 m74;
    s32* mA4;
    s32* mA8;
    s8* mAC;
    s32 mB0;
    s32 mB4;
    s32 m100;
    // size 0x320
};


p_workArea startMainLogic(p_workArea pParent)
{
    return createSubTask<sMainLogic>(pParent);
}

p_workArea startCameraTask(p_workArea)
{
    return NULL;
}

s32* twnVar1;
s32 twnVar2 = 0x7FFFFFFF;

p_workArea overlayStart_TWN_RUIN(p_workArea pUntypedThis, u32 arg)
{
    // load data
    if (gTWN_RUIN == NULL)
    {
        FILE* fHandle = fopen("TWN_RUIN.PRG", "rb");
        assert(fHandle);

        fseek(fHandle, 0, SEEK_END);
        u32 fileSize = ftell(fHandle);

        fseek(fHandle, 0, SEEK_SET);
        u8* fileData = new u8[fileSize];
        fread(fileData, fileSize, 1, fHandle);
        fclose(fHandle);

        gTWN_RUIN = new TWN_RUIN_data();
        gTWN_RUIN->m_name = "TWN_RUIN.PRG";
        gTWN_RUIN->m_data = fileData;
        gTWN_RUIN->m_dataSize = fileSize;
        gTWN_RUIN->m_base = 0x6054000;

        gTWN_RUIN->init();
    }
    townDebugTask2Function* pThis = static_cast<townDebugTask2Function*>(pUntypedThis);

    pThis->m_DeleteMethod = &townOverlayDelete;

    playMusic(-1, 0);
    playMusic(59, 0);

    playPCM(pThis, 100);

    loadFnt("EVTRUIN.FNT");

    graphicEngineStatus.m405C.m10 = 0x800;
    graphicEngineStatus.m405C.m30 = FP_Div(0x10000, graphicEngineStatus.m405C.m10);

    graphicEngineStatus.m405C.m14_farClipDistance = 0xF000;
    graphicEngineStatus.m405C.m38 = FP_Div(0x8000, graphicEngineStatus.m405C.m14_farClipDistance);

    graphicEngineStatus.m405C.m34 = graphicEngineStatus.m405C.m38 << 8;

    initDramAllocator(pThis, townBuffer, sizeof(townBuffer), listOfFilesToLoad);

    allocateVramList(pThis, townVDP1Buffer, sizeof(townVDP1Buffer));

    registerNpcs(gTWN_RUIN->getSaturnPtr(0x605E984), gTWN_RUIN->getSaturnPtr(0x06054398), arg);

    startScriptTask(pThis);

    startRuinBackgroundTask(pThis);

    sEdgeTask* pEdgeTask = startEdgeTask(gTWN_RUIN->getSaturnPtr(0x605E990));

    twnEdgeVar0 = &pEdgeTask->m8C;

    startMainLogic(pThis);

    twnMainLogicTask->m14 = pEdgeTask;

    startCameraTask(pThis);

    twnVar1 = &twnVar2;

    return pThis;
}

s32 TWN_RUIN_ExecuteNative(sSaturnPtr ptr, s32 arg0)
{
    assert(ptr.m_file == gTWN_RUIN);

    switch (ptr.m_offset)
    {
    case 0x605C83C:
        fadePalette(&g_fadeControls.m0_fade0, convertColorToU32(g_fadeControls.m0_fade0.m0_color), 0x8000, arg0);
        fadePalette(&g_fadeControls.m24_fade1, convertColorToU32(g_fadeControls.m24_fade1.m0_color), 0x8000, arg0);
        graphicEngineStatus.m40AC.m1_isMenuAllowed = 0;
        return 0;
    default:
        assert(0);
    }
    return 0;
}
