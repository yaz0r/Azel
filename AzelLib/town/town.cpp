#include "PDS.h"
#include "town.h"
#include "townScript.h"
#include "town/ruin/twn_ruin.h"
#include "town/exca/twn_exca.h"
#include "town/e006/twn_e006.h"
#include "town/e011/twn_e011.h"
#include "town/e014/twn_e014.h"
#include "kernel/vdp1Allocator.h"
#include "kernel/fileBundle.h"
#include "kernel/grid.h"
#include "kernel/graphicalObject.h"
#include "commonOverlay.h"

sTownOverlay* gCurrentTownOverlay = nullptr;

p_workArea townVar0 = nullptr;

u8 townBuffer[0xB0000];

townDebugTask2Function* townDebugTask2 = nullptr;

// TODO: move to townGrid
const sTownGridSetup* readTownGridSetup(sSaturnPtr ptr)
{
    sTownGridSetup* pNewTownGridSetup = new sTownGridSetup;
    pNewTownGridSetup->m0_width = readSaturnS8(ptr + 0);
    pNewTownGridSetup->m1_height = readSaturnS8(ptr + 1);
    pNewTownGridSetup->m4_cellSize = readSaturnFP(ptr + 4);
    pNewTownGridSetup->m8_pGrid = readGrid(readSaturnEA(ptr + 8), pNewTownGridSetup->m0_width, pNewTownGridSetup->m1_height);
    pNewTownGridSetup->mC_numEnvLCSTargets = readSaturnS32(ptr + 0xC);

    for (int i=0; i< pNewTownGridSetup->mC_numEnvLCSTargets; i++)
    {
        pNewTownGridSetup->m10_nLCSTargets.push_back(readSaturnVec3(readSaturnEA(ptr + 0x10) + i * 12));
    }

    return pNewTownGridSetup;
}

const sTownSetup* readTownSetup(sSaturnPtr ptr, int numScripts)
{
    sTownSetup* pNewTownSetup = new sTownSetup;
    pNewTownSetup->m0 = readSaturnS8(ptr + 0);
    pNewTownSetup->m4_gridSetup = readTownGridSetup(readSaturnEA(ptr + 4));

    for (int i = 0; i < numScripts; i++)
    {
        pNewTownSetup->m8_scripts.push_back(readSaturnEA(readSaturnEA(ptr + 8) + i * 4));
    }

    return pNewTownSetup;
}

void townDebugTask2Function::Update(townDebugTask2Function* pThis)
{
    if (readKeyboardToggle(0x87))
    {
        assert(0);
    }
}

sNpcData npcData0;

sTownGrid gTownGrid;

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

void loadTownFile(std::string& filename, u8* destination, u16 relocation)
{
    // TODO: what is the difference?
    loadFile(filename.c_str(), destination, relocation);
}

npcFileDeleter* createEnvironmentTask2Sub0Sub0(p_workArea r4_parent, s32 r5)
{
    s_fileEntry& r14 = dramAllocatorEnd[r5];
    if (r14.m8_refcount++)
    {
        return r14.mC_fileBundle;
    }

    // first use
    npcFileDeleter* r12 = createSubTask<npcFileDeleter>(r4_parent, npcFileDeleter::getTypedTaskDefinition_townObject());

    s_fileEntry& r10_vdp1File = dramAllocatorEnd[r5 + 1];
    s_vdp1AllocationNode* r13_vdp1Allocation = nullptr;
    if (r10_vdp1File.m4_fileSize)
    {
        r13_vdp1Allocation = vdp1Allocate(r10_vdp1File.m4_fileSize);
    }

    u8* r11_dramAllocation = nullptr;
    if (r14.m4_fileSize)
    {
        r11_dramAllocation = dramAllocate(r14.m4_fileSize);
    }

    if (r13_vdp1Allocation)
    {
        loadTownFile(r14.mFileName, r11_dramAllocation, r13_vdp1Allocation->m4_vdp1Memory);
    }
    else
    {
        loadTownFile(r14.mFileName, r11_dramAllocation, 1);
    }

    if (r13_vdp1Allocation)
    {
        loadTownFile(r10_vdp1File.mFileName, getVdp1Pointer(0x25C00000 + (r13_vdp1Allocation->m4_vdp1Memory << 3)), 0);
    }

    r14.mC_fileBundle = r12;
    r12->m0_fileBundle = new s_fileBundle(r11_dramAllocation);
    r12->m4_vd1Allocation = r13_vdp1Allocation;
    r12->m8 = r14.m0_fileID >> 16;
    r12->mA = r10_vdp1File.m0_fileID >> 16;
    r12->mC = r5;

    return r12;
}

void createCellObjects(s32 r4_currentX, s32 r5_currentY)
{
    s32 index = gTownGrid.m0_sizeX * r5_currentY + r4_currentX;
    sCellObjectListNode* r14 = gTownGrid.m140_perCellObjectList[index];

    while (r14)
    {
        if (r14->m8 == nullptr)
        {
            s32 r3 = readSaturnS32(r14->m4);
            sSaturnPtr r12 = readSaturnEA(r14->m4 + 4);
            sTownObject* r0 = nullptr;
            if (r3 > 0)
            {
                r0 = gCurrentTownOverlay->createObjectTaskFromEA_siblingTaskWithEAArgWithCopy(createEnvironmentTask2Sub0Sub0(gTownGrid.m34_dataBuffer, r3), r12, readSaturnU32(r12 + 0x10), r14->m4);
            }
            else
            {
                r0 = gCurrentTownOverlay->createObjectTaskFromEA_subTaskWithEAArg(gTownGrid.m34_dataBuffer, r12, readSaturnU32(r12 + 0x10), r14->m4);
            }

            assert(r0);
            r14->m8 = r0;
            r0->m8 = r14;
        }
        r14 = r14->m0_next;
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

        sSaturnPtr cellData = r14->m38_EnvironmentSetup->cells[r14->m10_currentX+r12][r14->m14_currentY];
        sTownCellTask* newCellTask = createSubTaskWithArgWithCopy<sTownCellTask>(r14->m34_dataBuffer, cellData);
        r14->m40_cellTasks[(r14->mC + r4) & 7][(r14->m8 + r12) & 7] = newCellTask;

        createCellObjects(r14->m10_currentX + r12, r4 + r14->m14_currentY);
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
        if (r14->m40_cellTasks[var24][(r14->m8 + r13) & 7])
        {
            //TODO: we should be deleting the content here
            FunctionUnimplemented();
        }
    } while (++r13 <= 2);
}

void initNPCSub0Sub1()
{
    assert(0);
}

s32 initNPCSub0Var1 = 0x7FFFFFFF;

void initTownGrid()
{
    gTownGrid.m0_sizeX = 0;
    gTownGrid.m4_sizeY = 0;
    gTownGrid.m140_perCellObjectList.clear();
    gTownGrid.m34_dataBuffer = nullptr;
    gTownGrid.m18_createCell = createEnvironmentTask2;
    gTownGrid.m1C = createEnvironmentTask;
    gTownGrid.m20_deleteCell = initNPCSub0Sub0;
    gTownGrid.m24 = initNPCSub0Sub1;
    gTownGrid.m40_cellTasks[0].fill(nullptr);
    gTownGrid.m40_cellTasks[1].fill(nullptr);
    gTownGrid.m3C = &initNPCSub0Var1;
}

void setupDragonForTown(s_fileBundle* pDragonBundle)
{
    if (pDragonBundle->getRawFileAtOffset(0x48))
    {
        FunctionUnimplemented();
    }
}

void loadTownPrg(s8 r4, s8 r5)
{
    npcData0.m5C = r4;
    npcData0.m5D = r5;

    mainGameState.setPackedBits(0, 2, 2);

    std::string overlayFileName = readSaturnString(readSaturnEA(gCommonFile->getSaturnPtr(0x002165D8 + r4 * 4 * 4)));

    TWN_RUIN_data::makeCurrent(); // hack to load all the common data currently loaded from Ruin
    if (overlayFileName == "TWN_RUIN.PRG")
    {
        gFieldOverlayFunction = overlayStart_TWN_RUIN;
    }
    else if (overlayFileName == "TWN_EXCA.PRG")
    {
        gFieldOverlayFunction = overlayStart_TWN_EXCA;
    }
    else if (overlayFileName == "TWN_E006.PRG")
    {
        gFieldOverlayFunction = overlayStart_TWN_E006;
    }
    else if (overlayFileName == "TWN_E011.PRG")
    {
        gFieldOverlayFunction = overlayStart_TWN_E011;
    }
    else if (overlayFileName == "TWN_E014.PRG")
    {
        gFieldOverlayFunction = overlayStart_TWN_E014;
    }
    else
    {
        assert(0);
    }

    pauseEngine[2] = 1;
    resetCameraProperties2(&cameraProperties2);
    reset3dEngine();
    resetTempAllocators();
    initTownGrid();
    setupDragonForTown(gDragonState->m0_pDragonModelBundle);
    gFieldOverlayFunction(townDebugTask2, r5);
}

p_workArea loadTownSub(p_workArea r4, s32 r5)
{
    townDebugTask2 = createSubTaskFromFunction<townDebugTask2Function>(r4, &townDebugTask2Function::Update);

    loadTownPrg(readSaturnS8(gCommonFile->getSaturnPtr(0x2166E4 + r5 * 2 + 0)), readSaturnS8(gCommonFile->getSaturnPtr(0x2166E5 + r5 * 2 + 1)));

    return townDebugTask2;
}

p_workArea loadTown(p_workArea r4, s32 r5)
{
    return loadTownSub(r4, r5);
}

p_workArea loadTown2(p_workArea r4, s32 r5)
{
    mainGameState.setPackedBits(0x87, 6, gGameStatus.m4_gameStatus - 0x50);
    return loadTownSub(r4, r5);
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

    if (gTownGrid.m140_perCellObjectList.size())
    {
        // TODO: cleanup cells data
        FunctionUnimplemented();
        //assert(0);
        //freeVdp1Block(initNPCSub0Var0.m34, initNPCSub0Var0.m140);
        gTownGrid.m140_perCellObjectList.clear();
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

    gTownGrid.m140_perCellObjectList.resize(size);
    for (int i = 0; i < size; i++)
    {
        gTownGrid.m140_perCellObjectList[i] = nullptr;
    }

    gTownGrid.m144_nextFreeObjectListNode = &gTownGrid.m148_objectListNodes[0];
    for (int i = 0; i < 0x40 - 1; i++)
    {
        gTownGrid.m148_objectListNodes[i].m0_next = &gTownGrid.m148_objectListNodes[i + 1];
    }
    gTownGrid.m148_objectListNodes[0x3F].m0_next = nullptr;
}

void initNPCSub0Sub2(npcFileDeleter* buffer, const struct sGrid* pGrid, u8 r6_sizeX, u8 r7_sizeY, fixedPoint cellSize)
{
    initNPCSub1();

    gTownGrid.m0_sizeX = r6_sizeX;
    gTownGrid.m4_sizeY = r7_sizeY;
    gTownGrid.m34_dataBuffer = buffer;
    gTownGrid.m28_cellSize = cellSize;
    gTownGrid.m2C = MTH_Mul(0x10A3D, cellSize);
    gTownGrid.m30_worldToCellIndex = FP_Div(0x10000, cellSize);
    gTownGrid.m38_EnvironmentSetup = pGrid;

    initNPCSub0Sub2Sub0();
    createTownCellsForCoordinates(-3, -3, &gTownGrid);
}

void initNPCSub0(npcFileDeleter* buffer, const struct sGrid* pGrid, u8 gridSizeX, u8 gridSizeY, fixedPoint cellSize)
{
    gTownGrid.m18_createCell = createEnvironmentTask2;
    gTownGrid.m1C = createEnvironmentTask;
    gTownGrid.m20_deleteCell = initNPCSub0Sub0;
    gTownGrid.m24 = initNPCSub0Sub1;

    initNPCSub0Sub2(buffer, pGrid, gridSizeX, gridSizeY, cellSize);
}

struct sNullTask : public s_workAreaTemplate<sNullTask>
{
    //size: 0x0
};

void decreaseNPCRefCount(s32 r5)
{
    s_fileEntry& fileEntry = dramAllocatorEnd[r5];
    if (fileEntry.m8_refcount)
    {
        if (--fileEntry.m8_refcount == 0)
        {
            assert(0);
            //loadDragonSub1Sub1(fileEntry.mC_buffer);
            if (fileEntry.mC_fileBundle)
            {
                fileEntry.mC_fileBundle->getTask()->markFinished();
            }
            fileEntry.mC_fileBundle = nullptr;
        }
    }
}

s32 initNPC(s32 arg)
{
    const sTownSetup* r13 = (*npcData0.m60_townSetup)[arg];
    if (npcData0.m5E < 0)
    {
        allocateNPC(currentResTask, r13->m0);
    }
    else
    {
        const sTownSetup* pcVar1 = (*npcData0.m60_townSetup)[npcData0.m5E];
        if (r13->m0 != pcVar1->m0)
        {
            initNPCSub1();
            decreaseNPCRefCount(pcVar1->m0);
            allocateNPC(currentResTask, r13->m0);
        }
    }

    npcData0.m5E = arg;
    npcData0.m64_scriptList = &r13->m8_scripts;

    const sTownGridSetup* r12_environmentSetup = r13->m4_gridSetup;
    npcData0.m68_numEnvLCSTargets = r12_environmentSetup->mC_numEnvLCSTargets;
    npcData0.m6C_LCSTargets = &r12_environmentSetup->m10_nLCSTargets;

    initNPCSub0(dramAllocatorEnd[r13->m0].mC_fileBundle, r12_environmentSetup->m8_pGrid, r12_environmentSetup->m0_width, r12_environmentSetup->m1_height, r12_environmentSetup->m4_cellSize);

    if (townVar0)
    {
        townVar0->getTask()->markFinished();
    }

    townVar0 = createSubTaskFromFunction<sNullTask>(currentResTask, nullptr);

    npcData0.mF0 = 0;

    if (npcData0.mF0) // TODO: what, how can it not be 0?
    {
        npcData0.mFC |= 0x10;
    }
    else
    {
        npcData0.mFC &= ~0x10;
    }

    setSomethingInNpc0(0, 1);
    setSomethingInNpc0(1, 2);
    return 0;
}

s32 initNPCFromStruct(sSaturnPtr r4)
{
    if (gTownGrid.m144_nextFreeObjectListNode == nullptr)
        return 0;

    sCellObjectListNode* r13 = gTownGrid.m144_nextFreeObjectListNode;
    gTownGrid.m144_nextFreeObjectListNode = r13->m0_next;

    r13->m4 = r4;
    r13->m8 = 0;

    s32 r12_cellX = MTH_Mul32(readSaturnFP(r4 + 8), gTownGrid.m30_worldToCellIndex);
    s32 r4_cellY = MTH_Mul32(readSaturnFP(r4 + 0x10), gTownGrid.m30_worldToCellIndex);

    if (r12_cellX < 0)
    {
        r12_cellX = 0;
    }
    if (r12_cellX >= gTownGrid.m0_sizeX)
    {
        r12_cellX = gTownGrid.m0_sizeX - 1;
    }

    if (r4_cellY < 0)
    {
        r4_cellY = 0;
    }
    if (r4_cellY >= gTownGrid.m4_sizeY)
    {
        r4_cellY = gTownGrid.m4_sizeY - 1;
    }

    r13->m0_next = gTownGrid.m140_perCellObjectList[r4_cellY * gTownGrid.m0_sizeX + r12_cellX];
    gTownGrid.m140_perCellObjectList[r4_cellY * gTownGrid.m0_sizeX + r12_cellX] = r13;

    return 1;
}

void removeNPC(p_workArea pThisAsTask, sTownObject* pThis, sSaturnPtr r5)
{
    if (pThis->m8 && (pThis->m8->m8 == pThis))
    {
        pThis->m8->m8 = nullptr;
    }

    if (!r5.isNull())
    {
        decreaseNPCRefCount(readSaturnS32(r5));
    }

    pThisAsTask->getTask()->markFinished();
}

static const std::array<sCollisionSetup, 5> collisionSetupArray = {
    {
        {1,0,0x10},
        {1,1,0x1C},
        {0,0,0x18},
        {2,1,0},
        {3,1,0}
    }
};

void mainLogicInitSub0(sMainLogic_74* r4, s32 r5)
{
    r4->m2C_collisionSetupIndex = r5;
    r4->m0_collisionSetup = collisionSetupArray[r5];
}

void mainLogicInitSub1(sMainLogic_74* r4, const sVec3_FP& r5, const sVec3_FP& r6)
{
    r4->m20 = (r5 + r6) / 2;

    r4->m14_collisionClip[0] = r4->m20[0] - std::min(r5[0], r6[0]);
    r4->m14_collisionClip[1] = r4->m20[1] - std::min(r5[1], r6[1]);
    r4->m14_collisionClip[2] = r4->m20[2] - std::min(r5[2], r6[2]);

    r4->m4_collisionRadius = sqrt_F(MTH_Product3d_FP(r4->m14_collisionClip, r4->m14_collisionClip));
}

void mainLogicUpdateSub0Sub0(sTownGrid* r4)
{
    //assert(0);
    FunctionUnimplemented();
}

void mainLogicUpdateSub0Sub1(s32 r4, sTownGrid* r5)
{
    switch (r4)
    {
    case -1:
        r5->m20_deleteCell(2, r5);
        r5->mC = (r5->mC - 1) & 7;
        r5->m14_currentY--;
        r5->m18_createCell(-2, r5);
        break;
    case 1:
        r5->m20_deleteCell(-2, r5);
        r5->mC = (r5->mC + 1) & 7;
        r5->m14_currentY++;
        r5->m18_createCell(2, r5);
        break;
    }
}

//TODO:kernel
s32 mainLogicUpdateSub0(s32 r4_x, s32 r5_y)
{
    // proxy for script
    mainLogicUpdateSub0(fixedPoint(r4_x), fixedPoint(r5_y));
    return 0;
}

//TODO:kernel
void mainLogicUpdateSub0(fixedPoint r4_x, fixedPoint r5_y)
{
    s32 r12 = MTH_Mul32(r4_x, gTownGrid.m30_worldToCellIndex);
    s32 r11 = MTH_Mul32(r5_y, gTownGrid.m30_worldToCellIndex);

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

void sTownCellTask::Init(sTownCellTask* pThis, sSaturnPtr arg)
{
    if ((readSaturnS32(arg + 0xC) == 0) && (readSaturnS32(arg + 0x10) == 0) && (readSaturnS32(arg + 0x14) == 0))
    {
        pThis->m_UpdateMethod = nullptr;
    }

    pThis->m8_cellPtr = arg;
    pThis->mC_position = readSaturnVec3(arg);
}

void sTownCellTask::Draw(sTownCellTask* pThis)
{
    pushCurrentMatrix();
    {
        translateCurrentMatrix(pThis->mC_position);
        fixedPoint var0 = pCurrentMatrix->matrix[11];
        if (var0 < gTownGrid.m2C - graphicEngineStatus.m405C.m10_nearClipDistance)
        {
            fixedPoint varMinusOne = MTH_Mul(var0, graphicEngineStatus.m405C.m2C_widthRatio);
            fixedPoint r4 = varMinusOne + MTH_Mul(gTownGrid.m2C, gTownGrid.m28_cellSize);
            if ((pCurrentMatrix->matrix[3] >= -r4) && (pCurrentMatrix->matrix[3] <= r4))
            {
                sSaturnPtr r14 = readSaturnEA(pThis->m8_cellPtr + 0xC);
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
                            addObjectToDrawList(pThis->m0_fileBundle->get3DModel(offset));
                        }

                        popMatrix();
                        r14 += 0x18;
                    }
                }

                r14 = readSaturnEA(pThis->m8_cellPtr + 0x10);
                if (r14.m_offset)
                {
                    FunctionUnimplemented();
                }
            }
        }
    }
    popMatrix();
}

s32 isDataLoaded(s32 fileIndex)
{
    npcFileDeleter* r4 = dramAllocatorEnd[fileIndex].mC_fileBundle;
    if (r4 == nullptr)
        return 1;
    if (r4->m8 >= 0)
        return 0;
    if (r4->mA >= 0)
        return 0;

    return 1;
}

sCameraTask* cameraTaskPtr = nullptr;

p_workArea startCameraTask(p_workArea pParent)
{
    cameraTaskPtr = createSubTask<sCameraTask>(pParent);
    return cameraTaskPtr;
}

void townOverlayDelete(townDebugTask2Function* pThis)
{
    FunctionUnimplemented();
    
    //FIX:
    if(gModuleManager && gModuleManager->m8)
    {
        assert(gModuleManager->m8 == pThis);
        gModuleManager->m8 = nullptr;
    }
}

