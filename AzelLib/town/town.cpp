#include "PDS.h"
#include "town.h"
#include "townScript.h"
#include "town/ruin/twn_ruin.h"

u8 townBuffer[0xB0000];

townDebugTask2Function* townDebugTask2 = nullptr;

void townDebugTask2Function::Update(townDebugTask2Function* pThis)
{
    if (readKeyboardToggle(0x87))
    {
        assert(0);
    }
}

sNpcData npcData0;

struct sInitNPCSub0Var0Sub144
{
    sInitNPCSub0Var0Sub144* m0_next;
    //size 0xC
};

struct sInitNPCSub0Var0
{
    s32 m0_sizeX;
    s32 m4_sizeY;
    s32 m8;
    s32 mC;
    s32 m10_currentX;
    s32 m14_currentY;
    void(*m18_createCell)(s32, sInitNPCSub0Var0*);
    void(*m1C)();
    void(*m20_deleteCell)(s32, sInitNPCSub0Var0*);
    void(*m24)();
    fixedPoint m28;
    fixedPoint m2C;
    npcFileDeleter* m34;
    sSaturnPtr m38_EnvironmentSetup;
    s32* m3C;
    std::array<std::array<p_workArea, 8>, 8> m40;
    std::vector<sSaturnPtr> m140;
    sInitNPCSub0Var0Sub144* m144;
    std::array<sInitNPCSub0Var0Sub144, 0x40> m148;

}initNPCSub0Var0;

struct sEnvironmentTask : public s_workAreaTemplateWithArgWithCopy<sEnvironmentTask, sSaturnPtr>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sEnvironmentTask::Init, nullptr, &sEnvironmentTask::Draw, nullptr };
        return &taskDefinition;
    }

    static void Init(sEnvironmentTask* pThis, sSaturnPtr arg)
    {

    }

    static void Draw(sEnvironmentTask* pThis)
    {

    }

    //size 0x18
};

void createEnvironmentTask2Sub0(s32 r4_currentX, s32 r5_currentY)
{
    s32 index = initNPCSub0Var0.m0_sizeX * r5_currentY + r4_currentX;
    sSaturnPtr r14 = initNPCSub0Var0.m140[index];

    while (r14.m_offset)
    {
        assert(0);
    }
}

void createEnvironmentTask2(s32 r4, sInitNPCSub0Var0* r14)
{
    if (r4 + r14->m14_currentY < 0)
    {
        return;
    }
    if (r4 + r14->m14_currentY >= r14->m4_sizeY)
    {
        return;
    }

    for (int r12 = -2; r12 < 2; r12++)
    {
        if (r14->m10_currentX + r12 < 0)
            continue;
        if (r14->m10_currentX + r12 >= r14->m0_sizeX)
            continue;

        sSaturnPtr cellData = readSaturnEA(r14->m38_EnvironmentSetup + 4 * (r14->m0_sizeX * r14->m14_currentY + r14->m10_currentX + r12));
        p_workArea newCellTask = createSiblingTaskWithArgWithCopy<sEnvironmentTask>(r14->m34, cellData);
        r14->m40[(r14->mC + r4) & 7][(r14->m8 + r12) & 7] = newCellTask;

        createEnvironmentTask2Sub0(r14->m10_currentX + r12, r4 + r14->m14_currentY);
    }

    assert(0);
}

void createEnvironmentTask()
{
    assert(0);
}

void initNPCSub0Sub0(s32 r3, sInitNPCSub0Var0* r14)
{
    s32 r13 = -2;

    s32 var24 = ((r14->mC + r3) & 7);

    do 
    {
        if (r14->m40[var24][(r14->m8 + r13) & 7])
        {
            assert(0);
        }
    } while (++r13 <= 2);
}

void initNPCSub0Sub1()
{
    assert(0);
}

s32 initNPCSub0Var1 = 0x7FFFFFFF;

void loadTownPrgSub0()
{
    initNPCSub0Var0.m0_sizeX = 0;
    initNPCSub0Var0.m4_sizeY = 0;
    initNPCSub0Var0.m140.clear();
    initNPCSub0Var0.m34 = 0;
    initNPCSub0Var0.m18_createCell = createEnvironmentTask2;
    initNPCSub0Var0.m1C = createEnvironmentTask;
    initNPCSub0Var0.m20_deleteCell = initNPCSub0Sub0;
    initNPCSub0Var0.m24 = initNPCSub0Sub1;
    initNPCSub0Var0.m40[0].fill(0);
    initNPCSub0Var0.m40[1].fill(0);
    initNPCSub0Var0.m3C = &initNPCSub0Var1;
}

void setupDragonForTown(u8* r4)
{
    if (READ_BE_U32(r4 + 0x48))
    {
        TaskUnimplemented();
    }
}

void loadTownPrg(s8 r4, s8 r5)
{
    npcData0.m5C = r4;
    npcData0.m5D = r5;

    mainGameState.setPackedBits(0, 2, 2);

    std::string overlayFileName = readSaturnString(readSaturnEA(gCommonFile.getSaturnPtr(0x002165D8 + r4 * 4 * 4)));

    if (overlayFileName == "TWN_RUIN.PRG")
    {
        gFieldOverlayFunction = overlayStart_TWN_RUIN;
    }
    else
    {
        assert(0);
    }

    pauseEngine[2] = 1;
    resetCameraProperties2(&cameraProperties2);
    reset3dEngine();
    resetTempAllocators();
    loadTownPrgSub0();
    setupDragonForTown(gDragonState->m0_pDragonModelRawData);
    gFieldOverlayFunction(townDebugTask2, r5);
}

p_workArea loadTown(p_workArea r4, s32 r5)
{
    townDebugTask2 = createSubTaskFromFunction<townDebugTask2Function>(r4, &townDebugTask2Function::Update);

    loadTownPrg(readSaturnS8(gCommonFile.getSaturnPtr(0x2166E4 + r5 * 2 + 0)), readSaturnS8(gCommonFile.getSaturnPtr(0x2166E5 + r5 * 2 + 1)));

    return townDebugTask2;
}

npcFileDeleter* loadNPCFile(sScriptTask* r4, const std::string& ramFileName, s32 ramFileSize, const std::string& vramFileName, s32 vramFileSize, s32 arg)
{
    npcFileDeleter* r13 = createSubTask<npcFileDeleter>(r4);

    sVdp1Allocation* r14_vdp1Memory = nullptr;
    if (vramFileSize)
    {
        r14_vdp1Memory = vdp1Allocate(vramFileSize);
    }

    u8* r12_dramMemory = nullptr;
    if (ramFileSize)
    {
        r12_dramMemory = dramAllocate(ramFileSize);
    }

    if (r14_vdp1Memory)
    {
        loadFile(ramFileName.c_str(), r12_dramMemory, r14_vdp1Memory->m4_baseInVdp1Memory);
    }
    else
    {
        loadFile(ramFileName.c_str(), r12_dramMemory, (u8*)1);
    }

    if (r14_vdp1Memory)
    {
        loadFile(vramFileName.c_str(), r14_vdp1Memory->m4_baseInVdp1Memory, nullptr);
    }

    r13->m0_dramAllocation = r12_dramMemory;
    r13->m4_vd1Allocation = r14_vdp1Memory;
    r13->mA = -1;
    r13->m8 = -1;
    r13->mC = arg;

    return r13;
}

npcFileDeleter* allocateNPC(sScriptTask* r4, s32 r5)
{
    s_fileEntry& r14 = dramAllocatorEnd[r5];
    if (r14.m8_refcount++)
    {
        return r14.mC_buffer;
    }

    s_fileEntry& vdp1File = dramAllocatorEnd[r5 + 1];
    r14.mC_buffer = loadNPCFile(r4, r14.mFileName, r14.m4_fileSize, vdp1File.mFileName, vdp1File.m4_fileSize, r5);
    return r14.mC_buffer;
}

void freeVdp1Block(npcFileDeleter*, s32)
{
    assert(0);
}

void initNPCSub1Sub0(sInitNPCSub0Var0* r13)
{
    s32 r14 = -2;

    do
    {
        r13->m20_deleteCell(r14, r13);
        r14++;
    }while(r14 < 2);
    
}

void initNPCSub0Sub2Sub1(s32 r4, s32 r5, sInitNPCSub0Var0* r6)
{
    r6->mC = 0;
    r6->m8 = 0;
    r6->m10_currentX = r4;
    r6->m14_currentY = r5;

    for (int i = -2; i < 2; i++)
    {
        r6->m18_createCell(i, r6);
    }
}

void initNPCSub1()
{
    initNPCSub1Sub0(&initNPCSub0Var0);

    if (initNPCSub0Var0.m140.size())
    {
        assert(0);
        //freeVdp1Block(initNPCSub0Var0.m34, initNPCSub0Var0.m140);
        initNPCSub0Var0.m140.clear();
    }
}

void initNPCSub0Sub2Sub0()
{
    s32 r4 = initNPCSub0Var0.m0_sizeX;
    if (r4 <= 0)
        r4 = 1;

    s32 r5 = initNPCSub0Var0.m4_sizeY;
    if (r5 <= 0)
        r5 = 1;

    s32 size = r4 * r5;

    initNPCSub0Var0.m140.resize(size);
    for (int i = 0; i < size; i++)
    {
        initNPCSub0Var0.m140[i] = sSaturnPtr::getNull();
    }

    initNPCSub0Var0.m144 = &initNPCSub0Var0.m144[0];
    for (int i = 0; i < 0x40 - 1; i++)
    {
        initNPCSub0Var0.m148[i].m0_next = &initNPCSub0Var0.m148[i + 1];
    }
    initNPCSub0Var0.m148[0x3F].m0_next = nullptr;
}

void initNPCSub0Sub2(npcFileDeleter* buffer, sSaturnPtr pEnvironemntSetupEA, u8 r6_sizeX, u8 r7_sizeY, fixedPoint stackArg0)
{
    initNPCSub1();

    initNPCSub0Var0.m0_sizeX = r6_sizeX;
    initNPCSub0Var0.m4_sizeY = r7_sizeY;
    initNPCSub0Var0.m34 = buffer;
    initNPCSub0Var0.m28 = stackArg0;
    initNPCSub0Var0.m2C = MTH_Mul(0x10A3D, stackArg0);
    initNPCSub0Var0.m2C = FP_Div(0x10000, stackArg0);
    initNPCSub0Var0.m38_EnvironmentSetup = pEnvironemntSetupEA;

    initNPCSub0Sub2Sub0();
    initNPCSub0Sub2Sub1(-3, -3, &initNPCSub0Var0);
}

void initNPCSub0(npcFileDeleter* buffer, sSaturnPtr pEnvironemntSetupEA, u8 r6, u8 r7, fixedPoint stackArg0)
{
    initNPCSub0Var0.m18_createCell = createEnvironmentTask2;
    initNPCSub0Var0.m1C = createEnvironmentTask;
    initNPCSub0Var0.m20_deleteCell = initNPCSub0Sub0;
    initNPCSub0Var0.m24 = initNPCSub0Sub1;

    initNPCSub0Sub2(buffer, pEnvironemntSetupEA, r6, r7, stackArg0);
}

s32 initNPC(s32 arg)
{
    sSaturnPtr r13 = npcData0.m60 + arg * 12;
    if (npcData0.m5E < 0)
    {
        allocateNPC(currentResTask, readSaturnS8(r13));
    }
    else
    {
        assert(0);
    }

    npcData0.m5E = arg;
    npcData0.m64_scriptList = readSaturnEA(r13 + 8);

    sSaturnPtr r12_environmentSetup = readSaturnEA(r13 + 4);
    npcData0.m68_nulLCSTargets = readSaturnS32(r12_environmentSetup + 0xC);
    npcData0.m6C_LCSTargets = readSaturnEA(r12_environmentSetup + 0x10);

    initNPCSub0(dramAllocatorEnd[readSaturnS8(r13)].mC_buffer, readSaturnEA(r12_environmentSetup + 8), readSaturnU8(r12_environmentSetup), readSaturnU8(r12_environmentSetup + 1), readSaturnFP(r12_environmentSetup + 4));

    TaskUnimplemented();
    return 0;
}

void mainLogicInitSub0(sMainLogic_74* r4, s32 r5)
{
    r4->m2C = r5;
    r4->m0 = readSaturnS8(gCommonFile.getSaturnPtr(0x201BB8 + 4 * r5));
    r4->m1 = readSaturnS8(gCommonFile.getSaturnPtr(0x201BB8 + 4 * r5 + 1));
    r4->m2 = readSaturnS8(gCommonFile.getSaturnPtr(0x201BB8 + 4 * r5 + 2));
}
void mainLogicInitSub1(sMainLogic_74* r4, sSaturnPtr r5, sSaturnPtr r6)
{
    r4->m20[0] = (readSaturnS32(r5) - readSaturnS32(r6)) / 2;
    r4->m20[1] = (readSaturnS32(r5 + 4) - readSaturnS32(r6 + 4)) / 2;
    r4->m20[2] = (readSaturnS32(r5 + 8) - readSaturnS32(r6 + 8)) / 2;

    if (readSaturnS32(r6) > readSaturnS32(r5))
    {
        r4->m14[0] = r4->m20[0] - readSaturnS32(r5);
    }
    else
    {
        r4->m14[0] = r4->m20[0] - readSaturnS32(r6);
    }

    if (readSaturnS32(r6 + 4) > readSaturnS32(r5 + 4))
    {
        r4->m14[1] = r4->m20[1] - readSaturnS32(r5 + 4);
    }
    else
    {
        r4->m14[1] = r4->m20[1] - readSaturnS32(r6 + 4);
    }

    if (readSaturnS32(r6 + 8) > readSaturnS32(r5 + 8))
    {
        r4->m14[2] = r4->m20[2] - readSaturnS32(r5 + 8);
    }
    else
    {
        r4->m14[2] = r4->m20[2] - readSaturnS32(r6 + 8);
    }

    r4->m4 = sqrt_F(MTH_Product3d_FP(r4->m14, r4->m14));
}

