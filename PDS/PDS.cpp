#include "PDS.h"

#ifdef _WIN32
#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "Imm32.lib")
#pragma comment(lib, "Version.lib")
#endif

bool hasEncounterData;
bool debugEnabled = false; // watchdog bit 1

int enableDebugTask;

u8 pauseEngine[8];

u32 azelCdNumber = 0;

u8 COMMON_DAT[0x98000];

extern sSaturnMemoryFile gCommonFile =
{
    "common.dat",
    COMMON_DAT,
    0x98000,
    0x00200000,
};


void initPauseSatus()
{
    pauseEngine[0] = 0;
    pauseEngine[1] = 0;
    pauseEngine[2] = 0;
}

s32 setDividend(s32 r4, s32 r5, s32 divisor)
{
    s64 divident = (s64)r4 * (s64)r5;
    return (s32)(divident / divisor);
}

fixedPoint FP_Mul(fixedPoint r4, fixedPoint r5)
{
    return fixedPoint::fromS32(((s64)r4.asS32() * (s64)r5.asS32()) >> 16);
}

s32 MTH_Product2d(s32 (&r4)[2], s32 (&r5)[2])
{
    s64 mac = 0;
    mac += r4[0] * r5[0];
    mac += r4[1] * r5[1];

    return mac;
}

fixedPoint sqrt_F(fixedPoint r4fp)
{
    s32 r1 = 0x40000000;
    int r3 = 16;

    s32 r4 = r4fp.asS32();

    for (int i = 16; i >= 0;)
    {
        i--;
        if (r1 < r4)
        {
            r4 -= r1;

            s32 r0 = r1;

            do
            {
                r1 >>= 2;
                s32 r2 = r0 + r1;

                r0 >>= 1;

                if (r4 >= r2)
                {
                    r4 -= r2;
                    r0 += r1;
                }
            } while (--i);

            return fixedPoint::fromS32(r0 << 8);
        }

        r1 >>= 2;
    }

    return fixedPoint::fromS32(0);
}

s32 sqrt_I(s32 r4)
{
    s32 r1 = 0x40000000;
    int r3 = 16;

    for (int i = 16; i >= 0;)
    {
        i--;
        if (r1 < r4)
        {
            r4 -= r1;

            s32 r0 = r1;
           
            do 
            {
                r1 >>= 2;
                s32 r2 = r0 + r1;

                r0 >>= 1;

                if (r4 >= r2)
                {
                    r4 -= r2;
                    r0 += r1;
                }

            } while (--i);

            return r0;
        }

        r1 >>= 2;
    }

    return 0;
}

void initVDP1Projection(fixedPoint r4, u32 mode)
{
    u32 angle = r4.getInteger();

    fixedPoint sin = getSin(angle);
    fixedPoint cos = getCos(angle);

    s32 VDP1_Width = graphicEngineStatus.m405C.VDP1_X2 - graphicEngineStatus.m405C.VDP1_X1;

    fixedPoint r0 = setDividend(VDP1_Width / 2, cos, sin);

    switch (mode)
    {
    case 0:
        graphicEngineStatus.m405C.m18 = FP_Mul(r0, fixedPoint(0x11999)); // 1.0999908447265625
        graphicEngineStatus.m405C.m1C = FP_Mul(r0, fixedPoint(0xEEEE)); // 0.93331909179687500
        break;
    case 1:
        graphicEngineStatus.m405C.m18 = FP_Mul(r0, fixedPoint(0xD333)); //0.82499694824218750
        graphicEngineStatus.m405C.m1C = FP_Mul(r0, fixedPoint(0xEEEE)); //0.93331909179687500
        break;
    case 2:
        graphicEngineStatus.m405C.m18 = FP_Mul(r0, fixedPoint(0xD333)); //0.82499694824218750
        graphicEngineStatus.m405C.m1C = FP_Mul(r0, fixedPoint(0xB333)); //0.69999694824218750
        break;
    default:
        assert(0);
        break;
    }

    s32 array[2];

    array[0] = 352/2;
    array[1] = graphicEngineStatus.m405C.m18;
    graphicEngineStatus.m405C.m2C = FP_Div(array[0], array[1]);
    graphicEngineStatus.m405C.m28 = FP_Div(sqrt_I(MTH_Product2d(array, array)), array[1]);

    array[0] = 224 / 2;
    array[1] = graphicEngineStatus.m405C.m1C;
    graphicEngineStatus.m405C.m24 = FP_Div(array[0], array[1]);
    graphicEngineStatus.m405C.m20 = FP_Div(sqrt_I(MTH_Product2d(array, array)), array[1]);
}

void getVdp1ProjectionParams(s16* r4, s16* r5)
{
    *r4 = graphicEngineStatus.m405C.m18;
    *r5 = graphicEngineStatus.m405C.m1C;
}

void initVDP1()
{
    graphicEngineStatus.m0 = 0;
    VDP1_PTMR = 2;
    VDP1_EWDR = 0;
    VDP1_EWLR = 0;
    VDP1_EWRR = 0x58E0;

    graphicEngineStatus.m3 = 1;
    graphicEngineStatus.doubleBufferState = 0;
    graphicEngineStatus.m4 = 0;
    graphicEngineStatus.m5 = 0;

    graphicEngineStatus.m405C.VDP1_X1 = 0;
    graphicEngineStatus.m405C.VDP1_Y1 = 0;

    graphicEngineStatus.m405C.VDP1_X2 = 352;
    graphicEngineStatus.m405C.VDP1_Y2 = 224;

    graphicEngineStatus.m405C.localCoordinatesX = 176;
    graphicEngineStatus.m405C.localCoordinatesY = 112;

    graphicEngineStatus.m405C.m10 = 0x999;
    graphicEngineStatus.m405C.m30 = FP_Div(0x10000, 0x999);

    graphicEngineStatus.m405C.m14_farClipDistance = 0x200000;
    graphicEngineStatus.m405C.m38 = FP_Div(0x8000, 0x200000);

    graphicEngineStatus.m405C.m34 = graphicEngineStatus.m405C.m38 << 8;

    graphicEngineStatus.m405C.m0 = 242;
    graphicEngineStatus.m405C.m2 = -242;
    graphicEngineStatus.m405C.m4 = -326;
    graphicEngineStatus.m405C.m6 = 326;
    graphicEngineStatus.m405C.m8 = 0x70;
    graphicEngineStatus.m405C.mA = -112;
    graphicEngineStatus.m405C.mC = -176;
    graphicEngineStatus.m405C.mE = 176;

    initVDP1Projection(0x1C71C71, 0);

    u32 vdp1WriteEA = 0x25C00000;
    setVdp1VramU16(vdp1WriteEA + 0x00, 9); // command 9: set system clipping coordinates
    setVdp1VramU16(vdp1WriteEA + 0x14, 0x160);
    setVdp1VramU16(vdp1WriteEA + 0x16, 0x160 - 0x80);
    vdp1WriteEA += 0x40;

    setVdp1VramU16(vdp1WriteEA + 0x00, 0xA); // command 10: set local coordinates
    setVdp1VramU16(vdp1WriteEA + 0x0C, 0x160 - 0x80 - 0x30);
    setVdp1VramU16(vdp1WriteEA + 0x0E, 0x6F);
    vdp1WriteEA += 0x20;
    graphicEngineStatus.mC = vdp1WriteEA;

    setVdp1VramU16(vdp1WriteEA + 0x00, 0x4); // command 4: polygon draw
    setVdp1VramU16(vdp1WriteEA + 0x04, 0xC0); // CMDPMOD
    setVdp1VramU16(vdp1WriteEA + 0x06, 0); // CMDCOLR
    setVdp1VramS16(vdp1WriteEA + 0x0C, -176); // XA
    setVdp1VramU16(vdp1WriteEA + 0x0E, 79); // YA
    setVdp1VramU16(vdp1WriteEA + 0x10, 175); // XB
    setVdp1VramU16(vdp1WriteEA + 0x12, 79); // YB
    setVdp1VramU16(vdp1WriteEA + 0x14, 175); // XC
    setVdp1VramU16(vdp1WriteEA + 0x16, 112); // YC
    setVdp1VramS16(vdp1WriteEA + 0x18, -176); //XD
    setVdp1VramU16(vdp1WriteEA + 0x1A, 112); // YD
    vdp1WriteEA += 0x20;

    setVdp1VramU16(vdp1WriteEA + 0x00, 0xA); // command 10: set local coordinates
    setVdp1VramU16(vdp1WriteEA + 0x0C, 176);
    setVdp1VramU16(vdp1WriteEA + 0x0E, 111);
    graphicEngineStatus.m405C.setLocalCoordinatesEA = vdp1WriteEA;
    vdp1WriteEA += 0x20;

    setVdp1VramU16(vdp1WriteEA + 0x00, 0x1008); // command 8: user clipping coordinates
    setVdp1VramU16(vdp1WriteEA + 0x0C, 0);
    setVdp1VramU16(vdp1WriteEA + 0x0E, 0);
    setVdp1VramU16(vdp1WriteEA + 0x14, 352);
    setVdp1VramU16(vdp1WriteEA + 0x16, 224);
    graphicEngineStatus.m405C.setClippingCoordinatesEA = vdp1WriteEA;
    graphicEngineStatus.m8 = vdp1WriteEA + 2;
    vdp1WriteEA += 0x20;

    setVdp1VramU16(vdp1WriteEA + 0x00, 0x8000); // END

    u32 vdp1Offset = vdp1WriteEA - 0x25C00000;
    graphicEngineStatus.m6 = vdp1Offset >> 3;
    vdp1WriteEA += 0x20;

    setVdp1VramU16(graphicEngineStatus.m8, graphicEngineStatus.m6);

    graphicEngineStatus.m14_vdp1Context[0].m4[0] = vdp1WriteEA;
    graphicEngineStatus.m14_vdp1Context[0].m4[1] = graphicEngineStatus.m8;
    graphicEngineStatus.m14_vdp1Context[1].m4[0] = 0x25C07FE0;
    graphicEngineStatus.m14_vdp1Context[1].m4[1] = 0x25C0FFE0;

    graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA = vdp1WriteEA;
    graphicEngineStatus.m14_vdp1Context[0].mC = 0;
    graphicEngineStatus.m14_vdp1Context[1].m0_currentVdp1WriteEA = 0x25C07FE0;
    graphicEngineStatus.m14_vdp1Context[1].mC = 0;

    graphicEngineStatus.m14_vdp1Context[0].m14 = (std::array<sVec2_S16[4], 1024>*)(getVdp1Pointer(0x25C7C000));
    graphicEngineStatus.m14_vdp1Context[0].m10 = graphicEngineStatus.m14_vdp1Context[0].m14->begin();
    graphicEngineStatus.m14_vdp1Context[0].m18 = graphicEngineStatus.m14_vdp1Context[0].m14->end();

    graphicEngineStatus.m14_vdp1Context[1].m14 = (std::array<sVec2_S16[4], 1024>*)(getVdp1Pointer(0x25C7E000));
    graphicEngineStatus.m14_vdp1Context[1].m10 = graphicEngineStatus.m14_vdp1Context[1].m14->begin();
    graphicEngineStatus.m14_vdp1Context[1].m18 = graphicEngineStatus.m14_vdp1Context[1].m14->end();

    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet = graphicEngineStatus.m14_vdp1Context[0].m24_vdp1Packets;
    graphicEngineStatus.m14_vdp1Context[0].m1C = 0;

    graphicEngineStatus.m14_vdp1Context[1].m20_pCurrentVdp1Packet = graphicEngineStatus.m14_vdp1Context[1].m24_vdp1Packets;
    graphicEngineStatus.m14_vdp1Context[1].m1C = 0;

    //addSlaveCommand(graphicEngineStatus, 0x40AC, 0, copyMatrix_0);

    if (VDP2Regs_.TVSTAT & 1)
    {
        assert(0); // clear some lines in the vdp1 framebuffer in pal
    }
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
    initVDP1();
    initVDP2();

    //...

    resetVdp2Strings();

    //...

    fadePalette(&menuUnk0.m_field0, 0, 0, 1);

    // Stuff

    startInitialTask();
}

void getInputConfig(s32 type, std::array<s32, 8>& outputConfig)
{
    static const std::array<std::array<s8, 8>, 3> configs=
    {
        {
            {0, 1, 0, 8, 4, 8, 6, 7}, // walk
            {0, 1, 0, 8, 4, 5, 6, 7}, // flight
            {0, 1, 2, 8, 8, 8, 8, 8}, // fight
        }
    };
    for (int i = 0; i < 8; i++)
    {
        outputConfig[i] = configs[type][i];
    }
}

void writeInputConfig(s32 type, const std::array<s32, 8>& config, s32 r6)
{
    static const std::array<s8, 9> buttonOrder = {2, 0, 1, 14, 13,12 ,11, 15, -1};
    for (int i = 0; i < 8; i++)
    {
        graphicEngineStatus.m4514.mD8[type][buttonOrder[i]] = 0;
    }

    static const std::array<u16, 8> bitMasks = { 4, 1, 2, 0x4000, 0x2000, 0x1000, 0x800, 0x8000 };
    for (int i = 0; i < 8; i++)
    {
        s8 r13 = buttonOrder[config[i]];
        if (r13 >= 0)
        {
            graphicEngineStatus.m4514.mD8[type][r13] |= bitMasks[i];
        }
    }

    if (r6)
    {
        graphicEngineStatus.m4514.m138[type] = 1;
        graphicEngineStatus.m4514.mD8[type][4] = 0x20;
        graphicEngineStatus.m4514.mD8[type][5] = 0x10;
    }
    else
    {
        graphicEngineStatus.m4514.m138[type] = 0;
        graphicEngineStatus.m4514.mD8[type][4] = 0x10;
        graphicEngineStatus.m4514.mD8[type][5] = 0x20;
    }
}

void iniitInitialTaskStatsAndDebugSub()
{
    for (int i = 0; i < 3; i++)
    {
        std::array<s32, 8> config;
        getInputConfig(i, config);
        writeInputConfig(i, config, 0);
    }
}

void initInitialTaskStatsAndDebug()
{
    initialTaskStatus.mC_showWarningTask = 1;
    enableDebugTask = 0;

    iniitInitialTaskStatsAndDebugSub();
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

void patchFilePointers(u8* destination, u8* characterArea)
{

}

int loadFile(const char* fileName, u8* destination, u8* characterArea)
{
    if (strstr(fileName, ".MCB"))
    {
        registerModelAndCharacter(destination, characterArea);
    }

    sFileInfoSub* pFileHandle = openFileHandle(fileName);

    if (pFileHandle == NULL)
    {
        return -1;
    }

    if (addFileToMemoryLayout(fileName, destination, pFileHandle->m_fileSize, characterArea))
    {
        assert(0);
    }

    //writeLoadingFileDebugInfo(fileInfoStruct.m0, fileName);

    fileInfoStruct.m0 = 1;

    pFileHandle->m_18 = 0;
    pFileHandle->m_patchPointerType = characterArea;

    fread(destination, pFileHandle->m_fileSize, 1, pFileHandle->fHandle);

    if (characterArea)
    {
        patchFilePointers(destination, characterArea);
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
    PDS_unimplemented("Unimplemented initFileSystemSub1");
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
    fileInfoStruct.m3 = 7;
    fileInfoStruct.displayMemoryLayout = 0;

    initFileSystemSub1();

    fileInfoStruct.m8 = -1;

    initFileSystemSub2();

    initFileLayoutTable();
}

u16 defaultInputConfig[16] = {
    1,
    2,
    4,
    8,
    0x10,
    0x20,
    0x40,
    0x80,
    0x100,
    0,
    0,
    0x800,
    0x1000,
    0x2000,
    0x4000,
    0x8000
};

void setupDefaultInputConfig(u16* inputDefaultConfig)
{
    for (int i = 0; i < 16; i++)
    {
        graphicEngineStatus.m4514.mD8[0][i] = inputDefaultConfig[i];
        graphicEngineStatus.m4514.mD8[1][i] = inputDefaultConfig[i];
        graphicEngineStatus.m4514.mD8[2][i] = inputDefaultConfig[i];
    }
}

void initSMPC()
{
    //getSPMCStatus();

    graphicEngineStatus.m4514.m138[0] = 1;
    graphicEngineStatus.m4514.m138[1] = 1;
    graphicEngineStatus.m4514.m138[2] = 1;

    setupDefaultInputConfig(defaultInputConfig);
}

void azelInit()
{
    // stuff

    initFileInfoStruct();

    // stuff

    loadFile("COMMON.DAT", COMMON_DAT, 0);

    initSMPC();

    // stuff
    initInitialTaskStatsAndDebug();

    // stuff
}

void updateInputsSub1(s_graphicEngineStatus_4514_sub2* r4)
{
    if (r4->m4)
    {
        assert(0);
    }
}

void copyKeyboardData()
{
    PDS_unimplemented("copyKeyboardData");
}

void updateInputs()
{
    for (int i = 0; i < 2; i++)
    {
        graphicEngineStatus.m4514.m0[i].m0_current = graphicEngineStatus.m4514.m0[i].m16_pending;
        graphicEngineStatus.m4514.m0[i].m16_pending.m8_newButtonDown = 0;
        graphicEngineStatus.m4514.m0[i].m16_pending.mA = 0;
        graphicEngineStatus.m4514.m0[i].m16_pending.mC_newButtonDown2 = 0;
        graphicEngineStatus.m4514.m0[i].m16_pending.mE = 0;
        graphicEngineStatus.m4514.m0[i].m16_pending.m10 = 0;
        graphicEngineStatus.m4514.m0[i].m16_pending.m12 = 0;
        graphicEngineStatus.m4514.m0[i].m16_pending.m14 = 0;

        updateInputsSub1(&graphicEngineStatus.m4514.m0[i].m2C);
    }

    copyKeyboardData();
}

void readInputsFromSMPC()
{
    // not real implementation
    //graphicEngineStatus.m4514
}

void invalidateCacheOnRange(void* ptr, u32 size)
{
}

struct listEntry
{
    s_vdp1Packet* m0_startOfList;
    s_vdp1Packet** m4_endOfList;
};

void initVdp1FlushList(listEntry* list)
{
    for (int i = 0; i < 0x20; i++)
    {
        list[i].m0_startOfList = nullptr;
        list[i].m4_endOfList = &list[i].m0_startOfList;
    }
}

s_vdp1Packet* flushVdp1Sub3(listEntry* list)
{
    s_vdp1Packet* r7 = 0;
    listEntry* r6 = 0;

    listEntry* current = list + 0x20;
    while (current != list)
    {
        current--;
        if (current->m4_endOfList != &current->m0_startOfList)
        {
            r7 = current->m0_startOfList;
            r6 = current;
            break;
        }
    }

    while (current != list)
    {
        current--;
        if (current->m4_endOfList != &current->m0_startOfList)
        {
            *r6->m4_endOfList = current->m0_startOfList;
            r6 = current;
        }
    }

    *r6->m4_endOfList = 0;
    return r7;
}

void flushVdp1Sub2(s_vdp1Packet* buffer0, u32 count0, s_vdp1Packet* buffer1, u32 count1, listEntry* list)
{
    for (int i = 0; i < count0; i++)
    {
        s_vdp1Packet* r4 = &buffer0[i];
        listEntry* r1 = &list[r4->m4_bucketTypes & 0x1F];
        (*r1->m4_endOfList) = r4;
        r1->m4_endOfList = &r4->m0_pNext;
    }
    for (int i = 0; i < count1; i++)
    {
        s_vdp1Packet* r4 = &buffer1[i];
        listEntry* r1 = &list[r4->m4_bucketTypes & 0x1F];
        (*r1->m4_endOfList)->m0_pNext = r4;
        r1->m4_endOfList = &r4;
    }
}

void flushVdp1Sub4(s_vdp1Packet* r4, u32 r5, listEntry* list)
{
    for(int i=0; i<r5; i++)
    {
        listEntry* r7 = &list[(r4->m4_bucketTypes >> 5)&0x1F];
        (*r7->m4_endOfList) = r4;
        r7->m4_endOfList = &r4->m0_pNext;
        r4 = r4->m0_pNext;
    }
}

void flushVdp1Sub5(s_vdp1Packet* r4, u32 r5, listEntry* list)
{
    for (int i = 0; i < r5; i++)
    {
        listEntry* r7 = &list[(r4->m4_bucketTypes >> 10)&0x1F];
        (*r7->m4_endOfList) = r4;
        r7->m4_endOfList = &r4->m0_pNext;
        r4 = r4->m0_pNext;
    }
}

void chainVdp1Packets(s_vdp1Packet* r4)
{
    setVdp1VramU16(graphicEngineStatus.m8, r4->m6_vdp1EA); // link clip command to first vdp1 packet

    while (r4->m0_pNext)
    {
        setVdp1VramU16(0x25C00000 + (r4->m6_vdp1EA << 3) + 2, r4->m0_pNext->m6_vdp1EA); // set the link address
        r4 = r4->m0_pNext;
    }

    setVdp1VramU16(0x25C00000 + (r4->m6_vdp1EA << 3) + 2, graphicEngineStatus.m6);
}

u32 numSpriteProcessed;
u32 mumSpriteProcessByMaster;
u32 mumSpriteProcessBySlave;

void flushVdp1()
{
    invalidateCacheOnRange(&graphicEngineStatus.m14_vdp1Context[1], sizeof(s_vdp1Context));
    u32 count = graphicEngineStatus.m14_vdp1Context[0].m1C + graphicEngineStatus.m14_vdp1Context[1].m1C;
    if (count)
    {
        listEntry flushList[0x20];
        initVdp1FlushList(flushList);
        flushVdp1Sub2(graphicEngineStatus.m14_vdp1Context[0].m24_vdp1Packets, graphicEngineStatus.m14_vdp1Context[0].m1C, graphicEngineStatus.m14_vdp1Context[1].m24_vdp1Packets, graphicEngineStatus.m14_vdp1Context[1].m1C, flushList);
        s_vdp1Packet* r15 = flushVdp1Sub3(flushList);

        initVdp1FlushList(flushList);
        flushVdp1Sub4(r15, count, flushList);
        r15 = flushVdp1Sub3(flushList);

        initVdp1FlushList(flushList);
        flushVdp1Sub5(r15, count, flushList);
        chainVdp1Packets(flushVdp1Sub3(flushList));
    }
    else
    {
        setVdp1VramU16(graphicEngineStatus.m8, graphicEngineStatus.m6);
    }
    
    numSpriteProcessed = graphicEngineStatus.m14_vdp1Context[0].mC + graphicEngineStatus.m14_vdp1Context[1].mC;
    mumSpriteProcessByMaster = graphicEngineStatus.m14_vdp1Context[0].m1C;
    mumSpriteProcessBySlave = graphicEngineStatus.m14_vdp1Context[1].m1C;

    graphicEngineStatus.m14_vdp1Context[0].mC = 0;
    graphicEngineStatus.m14_vdp1Context[1].mC = 0;

    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet = graphicEngineStatus.m14_vdp1Context[0].m24_vdp1Packets;
    graphicEngineStatus.m14_vdp1Context[0].m1C = 0;

    graphicEngineStatus.m14_vdp1Context[1].m20_pCurrentVdp1Packet = graphicEngineStatus.m14_vdp1Context[1].m24_vdp1Packets;
    graphicEngineStatus.m14_vdp1Context[1].m1C = 0;

    //addSlaveCommand(graphicEngineStatus, 0x40AC, 0, invalidateCacheOnRange);
}

void interruptVDP1Update()
{
    // TODO: massively incomplete
    int r6 = graphicEngineStatus.doubleBufferState;
    graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA = graphicEngineStatus.m14_vdp1Context[0].m4[r6];
    graphicEngineStatus.m14_vdp1Context[1].m0_currentVdp1WriteEA = graphicEngineStatus.m14_vdp1Context[1].m4[r6];
}

bool bContinue = true;
void loopIteration()
{
    static bool gameRunning = false;

    azelSdl2_StartFrame();
    if (!gameRunning)
    {
        if (ImGui::GetIO().MouseClicked[0])
        {
            gameRunning = true;
        }
    }
    else
    {
        static bool audioInitialized = false;
        if (!audioInitialized)
        {
            gSoloud.init();
            audioInitialized = true;
        }

        checkGL();
        

        //updateDebug();

        //copySMPCOutputStatus();

        updateInputs();

        if (ImGui::Begin("InputState"))
        {
            ImGui::Text("m6_buttonDown: %X", graphicEngineStatus.m4514.m0[0].m0_current.m6_buttonDown);
            ImGui::Text("m8_newButtonDown: %X", graphicEngineStatus.m4514.m0[0].m0_current.m8_newButtonDown);
        }
        ImGui::End();

        //updateInputDebug();

        runTasks();

        //waitForSh2Completion();

        flushVdp1();

        //updateSound();

        //lastUpdateFunction();

        // interrupt stuff
        {
            interruptVDP1Update();
        }
        checkGL();
    }

	bContinue = azelSdl2_EndFrame();
}

int main(int argc, char* argv[])
{
    azelSdl2_Init();

    checkGL();
    
    azelInit();
    resetEngine();

    //...
    readInputsFromSMPC();
    updateInputs();
    readInputsFromSMPC();
    updateInputs();

    u32 frameCounter = 0;

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(loopIteration, 0, 1);
#else
    do 
    {
 	loopIteration();
    } while (bContinue);
#endif
    return 0;
}

u32 READ_BE_U32(const void* ptr)
{
    u32 data = *(u32*)(ptr);
    data = ((data >> 24) & 0xFF) | ((data >> 8) & 0xFF00) | ((data << 8) & 0xFF0000) | ((data << 24) & 0xFF000000);
    return data;
}

sVec3_FP READ_BE_Vec3(const void* ptr)
{
    sVec3_FP newVec;
    newVec[0] = READ_BE_S32(((u8*)ptr) + 0);
    newVec[1] = READ_BE_S32(((u8*)ptr) + 4);
    newVec[2] = READ_BE_S32(((u8*)ptr) + 8);

    return newVec;
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

void WRITE_BE_U16(const void* ptr, u16 data)
{
    data = ((data >> 8) & 0xFF) | ((data & 0xFF) << 8);
    *(u16*)(ptr) = data;
}

s16 READ_BE_S16(const void* ptr)
{
    u16 data = *(u16*)(ptr);
    data = ((data >> 8) & 0xFF) | ((data & 0xFF) << 8);
    return *(s16*)&data;
}

u8 READ_BE_U8(const void* ptr)
{
    u8 data = *(u8*)(ptr);
    return data;
}

s8 READ_BE_S8(const void* ptr)
{
    s8 data = *(s8*)(ptr);
    return data;
}


