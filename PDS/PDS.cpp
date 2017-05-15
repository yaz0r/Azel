#include "PDS.h"

#ifdef _WIN32
#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "Imm32.lib")
#pragma comment(lib, "Version.lib")
#endif

bool debugEnabled = false; // watchdog bit 1

int enableDebugTask;

u8 pauseEngine[4];
sPortData2 PortData2;

u32 azelCdNumber = 0;

void initPauseSatus()
{
    pauseEngine[0] = 0;
    pauseEngine[1] = 0;
    pauseEngine[2] = 0;
}

void resetEngine()
{
    //initVBlankData();
    //initDmaChain();
    //initFileSystem();
    //resetInputs();
    initPauseSatus();
    resetTasks();
    initHeap();
    //initVDP1();
    initVDP2();

    //...

    resetMenu(&menuUnk0.m_field0, 0, 0, 1);

    // Stuff

    startInitialTask();
}

void initInitialTaskStatsAndDebug()
{
    initialTaskStatus.m_showWarningTask = 1;
    enableDebugTask = 0;

    //iniitInitialTaskStatsAndDebugSub();
}

struct sFileInfoSub
{
    sFileInfoSub* pNext;
};

struct sFileInfo
{
    u8 field_3; //3
    u8 displayMemoryLayout; //5
    u16 field_8; //8

    sFileInfoSub* freeHead; //2C
    sFileInfoSub* allocatedHead; //30
    sFileInfoSub linkedList[15]; //34
} fileInfoStruct;

void initFileSystemSub1()
{
    yLog("Unimplemented initFileSystemSub1");
}

void initFileSystemSub2()
{
    sFileInfoSub** r5 = &fileInfoStruct.allocatedHead;
    sFileInfoSub* r4 = &fileInfoStruct.linkedList[0];

    for (u32 i = 0; i < 15; i++)
    {
        *r5 = r4;
        r5 = &r4->pNext;
        r4++;
    }

    *r5 = NULL;
    fileInfoStruct.freeHead = NULL;
}

struct sMemoryLayoutNode
{
    sMemoryLayoutNode* pNext; //0
};

struct sMemoryLayout
{
    sMemoryLayoutNode* pAllocatedHead;
    sMemoryLayoutNode* pFreeHead;
    sMemoryLayoutNode m_nodes[64];
} memoryLayout;

void addToMemoryLayout(u8* pointer, u32 unk0)
{
}

void initFileLayoutTable()
{
    sMemoryLayoutNode** r5 = &memoryLayout.pFreeHead;
    sMemoryLayoutNode* r4 = &memoryLayout.m_nodes[0];

    for (u32 i = 0; i < 64; i++)
    {
        *r5 = r4;
        r5 = &r4->pNext;
        r4++;
    }

    *r5 = NULL;
    memoryLayout.pAllocatedHead = NULL;
}

void initFileInfoStruct()
{
    fileInfoStruct.field_3 = 7;
    fileInfoStruct.displayMemoryLayout = 0;

    initFileSystemSub1();

    fileInfoStruct.field_8 = -1;

    initFileSystemSub2();

    initFileLayoutTable();
}

void azelInit()
{
    // stuff

    initFileInfoStruct();

    // stuff
    initInitialTaskStatsAndDebug();

    // stuff
}

int main(int argc, char* argv[])
{
    azelSdl2_Init();

    azelInit();
    resetEngine();

    do 
    {
        azelSdl2_StartFrame();

        //updateDebug();

        //copySMPCOutputStatus();

        //updateInputs();

        //updateInputDebug();

        runTasks();

        //waitForSh2Completion();

        //mergeDebugStats();

        //updateSound();

        //lastUpdateFunction();
    } while (azelSdl2_EndFrame());
    return 0;
}

u32 READ_BE_U32(const void* ptr)
{
    u32 data = *(u32*)(ptr);
    data = ((data >> 24) & 0xFF) | ((data >> 8) & 0xFF00) | ((data << 8) & 0xFF0000) | ((data << 24) & 0xFF000000);
    return data;
}

s32 READ_BE_S32(const void* ptr)
{
    u32 data = *(u32*)(ptr);
    data = ((data >> 24) & 0xFF) | ((data >> 8) & 0xFF00) | ((data << 8) & 0xFF0000) | ((data << 24) & 0xFF000000);
    return *(s32*)&data;
}

u16 READ_BE_U16(const void* ptr)
{
    u16 data = *(u16*)(ptr);
    data = ((data >> 8) & 0xFF) | ((data & 0xFF) << 8);
    return data;
}

void yLog(...)
{

}

