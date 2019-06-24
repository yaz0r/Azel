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

struct sTownGrid
{
    s32 m0_sizeX;
    s32 m4_sizeY;
    s32 m8;
    s32 mC;
    s32 m10_currentX;
    s32 m14_currentY;
    void(*m18_createCell)(s32, sTownGrid*);
    void(*m1C)();
    void(*m20_deleteCell)(s32, sTownGrid*);
    void(*m24)();
    sVec3_FP m28;
    npcFileDeleter* m34;
    sSaturnPtr m38_EnvironmentSetup;
    s32* m3C;
    std::array<std::array<p_workArea, 8>, 8> m40;
    std::vector<sSaturnPtr> m140;
    sInitNPCSub0Var0Sub144* m144;
    std::array<sInitNPCSub0Var0Sub144, 0x40> m148;

}gTownGrid;

//TODO:kernel
s32 MTH_Mul32(fixedPoint a, fixedPoint b)
{
    fixedPoint temp(((s64)a.asS32() * (s64)b.asS32()) >> 16);
    return temp.getInteger();
}

fixedPoint generateObjectMatrix(sSaturnPtr r4, sSaturnPtr r5)
{
    pushCurrentMatrix();
    translateMatrix(readSaturnVec3(r4), pCurrentMatrix);

    sVec3_S16_12_4 rotation;
    rotation[0] = readSaturnS16(r5);
    rotation[1] = readSaturnS16(r5+2);
    rotation[2] = readSaturnS16(r5+4);
    rotateMatrixZYX_s16(rotation, pCurrentMatrix);

    return pCurrentMatrix->matrix[11];
}

struct sEnvironmentTask : public s_workAreaTemplateWithArgWithCopy<sEnvironmentTask, sSaturnPtr>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sEnvironmentTask::Init, nullptr, &sEnvironmentTask::Draw, nullptr };
        return &taskDefinition;
    }

    static void Init(sEnvironmentTask* pThis, sSaturnPtr arg)
    {
        if ((readSaturnS32(arg + 0xC) == 0) && (readSaturnS32(arg + 0x10) == 0) && (readSaturnS32(arg + 0x14) == 0))
        {
            pThis->m_UpdateMethod = nullptr;
        }

        pThis->m8 = arg;
        pThis->mC_position = readSaturnVec3(arg);
    }

    static void Draw(sEnvironmentTask* pThis)
    {
        pushCurrentMatrix();
        {
            translateCurrentMatrix(pThis->mC_position);
            fixedPoint var0 = pCurrentMatrix->matrix[11];
            if (var0 < gTownGrid.m28[1] - graphicEngineStatus.m405C.m10)
            {
                fixedPoint varMinusOne = MTH_Mul(var0, graphicEngineStatus.m405C.m2C);
                fixedPoint r4 = varMinusOne + MTH_Mul(gTownGrid.m28[1], gTownGrid.m28[0]);
                if ((pCurrentMatrix->matrix[3] >= -r4) && (pCurrentMatrix->matrix[3] <= r4))
                {
                    sSaturnPtr r14 = readSaturnEA(pThis->m8 + 0xC);
                    if (r14.m_offset)
                    {
                        while (readSaturnS32(r14))
                        {
                            s32 r5 = generateObjectMatrix(r14 + 4, r14 + 0x10);
                            int r4 = 0;
                            while (r5 > gTownGrid.m3C[r4])
                            {
                                r4++;
                            }

                            u16 offset = readSaturnU16(readSaturnEA(r14) + r4 * 2);
                            if (offset)
                            {
                                addObjectToDrawList(pThis->m0_dramAllocation, READ_BE_U32(pThis->m0_dramAllocation + offset));
                            }

                            popMatrix();
                            r14 += 0x18;
                        }
                    }

                    r14 = readSaturnEA(pThis->m8 + 0x10);
                    if (r14.m_offset)
                    {
                        FunctionUnimplemented();
                    }
                }
            }
        }
        popMatrix();
    }

    //size 0x18
    sSaturnPtr m8;
    sVec3_FP mC_position;
};

void createEnvironmentTask2Sub0(s32 r4_currentX, s32 r5_currentY)
{
    s32 index = gTownGrid.m0_sizeX * r5_currentY + r4_currentX;
    sSaturnPtr r14 = gTownGrid.m140[index];

    while (r14.m_offset)
    {
        assert(0);
    }
}

void createEnvironmentTask2(s32 r4, sTownGrid* r14)
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
}

void createEnvironmentTask()
{
    assert(0);
}

void initNPCSub0Sub0(s32 r3, sTownGrid* r14)
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
    gTownGrid.m0_sizeX = 0;
    gTownGrid.m4_sizeY = 0;
    gTownGrid.m140.clear();
    gTownGrid.m34 = 0;
    gTownGrid.m18_createCell = createEnvironmentTask2;
    gTownGrid.m1C = createEnvironmentTask;
    gTownGrid.m20_deleteCell = initNPCSub0Sub0;
    gTownGrid.m24 = initNPCSub0Sub1;
    gTownGrid.m40[0].fill(0);
    gTownGrid.m40[1].fill(0);
    gTownGrid.m3C = &initNPCSub0Var1;
}

void setupDragonForTown(u8* r4)
{
    if (READ_BE_U32(r4 + 0x48))
    {
        FunctionUnimplemented();
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

void deleteAllTownCells(sTownGrid* r13)
{
    s32 r14 = -2;

    do
    {
        r13->m20_deleteCell(r14, r13);
        r14++;
    }while(r14 < 2);
    
}

void createTownCellsForCoordinates(s32 r4, s32 r5, sTownGrid* r6)
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
    deleteAllTownCells(&gTownGrid);

    if (gTownGrid.m140.size())
    {
        assert(0);
        //freeVdp1Block(initNPCSub0Var0.m34, initNPCSub0Var0.m140);
        gTownGrid.m140.clear();
    }
}

void initNPCSub0Sub2Sub0()
{
    s32 r4 = gTownGrid.m0_sizeX;
    if (r4 <= 0)
        r4 = 1;

    s32 r5 = gTownGrid.m4_sizeY;
    if (r5 <= 0)
        r5 = 1;

    s32 size = r4 * r5;

    gTownGrid.m140.resize(size);
    for (int i = 0; i < size; i++)
    {
        gTownGrid.m140[i] = sSaturnPtr::getNull();
    }

    gTownGrid.m144 = &gTownGrid.m144[0];
    for (int i = 0; i < 0x40 - 1; i++)
    {
        gTownGrid.m148[i].m0_next = &gTownGrid.m148[i + 1];
    }
    gTownGrid.m148[0x3F].m0_next = nullptr;
}

void initNPCSub0Sub2(npcFileDeleter* buffer, sSaturnPtr pEnvironemntSetupEA, u8 r6_sizeX, u8 r7_sizeY, fixedPoint stackArg0)
{
    initNPCSub1();

    gTownGrid.m0_sizeX = r6_sizeX;
    gTownGrid.m4_sizeY = r7_sizeY;
    gTownGrid.m34 = buffer;
    gTownGrid.m28[0] = stackArg0;
    gTownGrid.m28[1] = MTH_Mul(0x10A3D, stackArg0);
    gTownGrid.m28[2] = FP_Div(0x10000, stackArg0);
    gTownGrid.m38_EnvironmentSetup = pEnvironemntSetupEA;

    initNPCSub0Sub2Sub0();
    createTownCellsForCoordinates(-3, -3, &gTownGrid);
}

void initNPCSub0(npcFileDeleter* buffer, sSaturnPtr pEnvironemntSetupEA, u8 r6, u8 r7, fixedPoint stackArg0)
{
    gTownGrid.m18_createCell = createEnvironmentTask2;
    gTownGrid.m1C = createEnvironmentTask;
    gTownGrid.m20_deleteCell = initNPCSub0Sub0;
    gTownGrid.m24 = initNPCSub0Sub1;

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

    FunctionUnimplemented();
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

void mainLogicUpdateSub0Sub0(sTownGrid* r4)
{
    assert(0);
}

void mainLogicUpdateSub0Sub1(s32 r4, sTownGrid* r5)
{
    assert(0);
}

//TODO:kernel
void mainLogicUpdateSub0(fixedPoint r4_x, fixedPoint r5_y)
{
    s32 r12 = MTH_Mul32(r4_x, gTownGrid.m28[2]);
    s32 r11 = MTH_Mul32(r5_y, gTownGrid.m28[2]);

    // not exactly the original code, but should be the same thing
    s32 cellDistanceX = r12 - gTownGrid.m10_currentX;
    if (cellDistanceX < 0)
    {
        cellDistanceX = -cellDistanceX;
    }

    s32 cellDistanceY = r11 - gTownGrid.m14_currentY;
    if (cellDistanceY < 0)
    {
        cellDistanceY = -cellDistanceY;
    }

    if ((cellDistanceX > 1) || (cellDistanceY > 1))
    {
        deleteAllTownCells(&gTownGrid);
        createTownCellsForCoordinates(r12, r11, &gTownGrid);
    }
    else
    {
        if (cellDistanceX)
        {
            mainLogicUpdateSub0Sub0(&gTownGrid);
        }

        if (cellDistanceY)
        {
            mainLogicUpdateSub0Sub1(r12, &gTownGrid);
        }
    }
}
