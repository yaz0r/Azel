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

u8 COMMON_DAT[0x98000];

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

    resetVdp2Strings();

    //...

    fadePalette(&menuUnk0.m_field0, 0, 0, 1);

    // Stuff

    startInitialTask();
}

void initInitialTaskStatsAndDebug()
{
    initialTaskStatus.m_showWarningTask = 1;
    enableDebugTask = 0;

    //iniitInitialTaskStatsAndDebugSub();
}

sFileInfo fileInfoStruct;

sFileInfoSub* getFileHandle(const char* fileName)
{
    sFileInfoSub* r3 = fileInfoStruct.allocatedHead;

    while (r3)
    {
        if (strcmp(r3->m_fileName, fileName) == 0)
        {
            return r3;
        }

        r3 = r3->pNext;
    }

    return NULL;
}

sFileInfoSub* allocateFileInfoStruct()
{
    sFileInfoSub* pFree = fileInfoStruct.freeHead;
    if (pFree == NULL)
        return NULL;

    sFileInfoSub** r4 = &fileInfoStruct.allocatedHead;

    while (*r4)
    {
        r4 = &(*r4)->pNext;
    }

    *r4 = pFree;
    fileInfoStruct.freeHead = pFree->pNext;
    pFree->pNext = NULL;

    return pFree;
}

void freeFileInfoStruct(sFileInfoSub* pInfo)
{
    sFileInfoSub** r5 = &fileInfoStruct.allocatedHead;

    while (*r5 != pInfo)
    {
        r5 = &(*r5)->pNext;
    }

    *r5 = pInfo->pNext;
    pInfo->pNext = fileInfoStruct.freeHead;
    fileInfoStruct.freeHead = pInfo;
}

u32 getFileSize(const char* fileName)
{
    FILE* fHandle = fopen(fileName, "rb");
    assert(fHandle);

    fseek(fHandle, 0, SEEK_END);
    u32 fileSize = ftell(fHandle);

    fclose(fHandle);

    return fileSize;
}

sFileInfoSub* openFileHandle(const char* fileName)
{
    sFileInfoSub* pFileHandle = getFileHandle(fileName);
    if (pFileHandle)
        return pFileHandle;

    pFileHandle = allocateFileInfoStruct();
    strcpy(pFileHandle->m_fileName, fileName);

    FILE* fHandle = fopen(fileName, "rb");
    pFileHandle->fHandle = fHandle;

    fseek(fHandle, 0, SEEK_END);
    u32 fileSize = ftell(fHandle);
    fseek(fHandle, 0, SEEK_SET);

    pFileHandle->m_fileSize = fileSize;

    return pFileHandle;
}

struct sMemoryLayoutNode // size is 0x10
{
    sMemoryLayoutNode* pNext; //0
    char m_fileName[32]; //4
    u8* m_6;//6
    u8* m_destination;//8
    u32 m_size;//C
};

struct sMemoryLayout
{
    sMemoryLayoutNode* pAllocatedHead; // 0
    sMemoryLayoutNode* pFreeHead; // 4
    sMemoryLayoutNode m_nodes[64]; // 8
} memoryLayout;

void displayMemoryLayout()
{
    if (fileInfoStruct.displayMemoryLayout)
    {
        assert(0);
    }
}


bool addFileToMemoryLayout(const char* fileName, u8* destination, u32 fileSize, u8* characterArea)
{
    sMemoryLayoutNode** r13 = &memoryLayout.pAllocatedHead;

    while (*r13)
    {
        assert(0);
    }

    if (memoryLayout.pFreeHead)
    {
        sMemoryLayoutNode* pNode = memoryLayout.pFreeHead;
        memoryLayout.pFreeHead = pNode->pNext;
        pNode->pNext = NULL;

        strcpy(pNode->m_fileName, fileName);
        pNode->m_6 = characterArea;
        pNode->m_destination = destination;
        pNode->m_size = fileSize;
    }

    displayMemoryLayout();
    return false;
}

int loadFile(const char* fileName, u8* destination, u8* characterArea)
{
    sFileInfoSub* pFileHandle = openFileHandle(fileName);

    if (pFileHandle == NULL)
    {
        return -1;
    }

    if (addFileToMemoryLayout(fileName, destination, pFileHandle->m_fileSize, characterArea))
    {
        assert(0);
    }

    //writeLoadingFileDebugInfo(fileInfoStruct.field_0, fileName);

    fileInfoStruct.field_0 = 1;

    pFileHandle->m_18 = 0;
    pFileHandle->m_patchPointerType = characterArea;

    fread(destination, pFileHandle->m_fileSize, 1, pFileHandle->fHandle);

    if (characterArea)
    {
        //patchFilePointers(destination, patchPointerType);
    }

    fclose(pFileHandle->fHandle);

    freeFileInfoStruct(pFileHandle);

    if (enableDebugTask)
    {
        assert(0);
    }

    return 0;
}

void initFileSystemSub1()
{
    yLog("Unimplemented initFileSystemSub1");
}

void initFileSystemSub2()
{
    sFileInfoSub** r5 = &fileInfoStruct.freeHead;
    sFileInfoSub* r4 = &fileInfoStruct.linkedList[0];

    for (u32 i = 0; i < 15; i++)
    {
        *r5 = r4;
        r5 = &r4->pNext;
        r4++;
    }

    *r5 = NULL;
    fileInfoStruct.allocatedHead = NULL;
}

void addToMemoryLayout(u8* pointer, u32 size)
{
    sMemoryLayoutNode** r5 = &memoryLayout.pAllocatedHead;

    while (*r5)
    {
        assert(0);
    }

    displayMemoryLayout();
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

    loadFile("COMMON.DAT", COMMON_DAT, 0);

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

s16 READ_BE_S16(const void* ptr)
{
    u16 data = *(u16*)(ptr);
    data = ((data >> 8) & 0xFF) | ((data & 0xFF) << 8);
    return *(s16*)&data;
}

void yLog(...)
{

}

void unimplemented(const char* name)
{
    printf("Unimplemented: %s\n", name);
}

