#include "PDS.h"
#include "town.h"
#include "townScript.h"
#include "town/camp/twn_camp.h"
#include "town/ruin/twn_ruin.h"
#include "town/exca/twn_exca.h"
#include "town/e006/twn_e006.h"
#include "town/e011/twn_e011.h"
#include "town/e014/twn_e014.h"
#include "town/zoah/twn_zoah.h"
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
    if (readKeyboardToggle(KEY_CODE_F12))
    {
        assert(0);
    }
}

sNpcData npcData0;

fixedPoint generateObjectMatrix(sSaturnPtr r4, sSaturnPtr r5)
{
    pushCurrentMatrix();
    translateMatrix(readSaturnVec3(r4), pCurrentMatrix);

    sVec3_S16_12_4 rotation;
    rotation[0] = readSaturnS16(r5);
    rotation[1] = readSaturnS16(r5+2);
    rotation[2] = readSaturnS16(r5+4);
    rotateMatrixZYX_s16(rotation, pCurrentMatrix);

    return pCurrentMatrix->m[2][3];
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
    if (r13_vdp1Allocation)
    {
        r12->m0_fileBundle->setPatchFilePointers(r13_vdp1Allocation->m4_vdp1Memory);
    }
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
            r0->m8_cellNode = r14;
        }
        r14 = r14->m0_next;
    }
}

static void createGridRow(s32 r4, sWorldGrid* r14)
{
    if (r4 + r14->m14_currentY < 0)
    {
        return;
    }
    if (r4 + r14->m14_currentY >= r14->m4_sizeY)
    {
        return;
    }

    for (int r12 = -2; r12 <= 2; r12++)
    {
        if (r14->m10_currentX + r12 < 0)
            continue;
        if (r14->m10_currentX + r12 >= r14->m0_sizeX)
            continue;

        sSaturnPtr cellData = r14->m38_EnvironmentSetup->cells[r14->m10_currentX+r12][r4 + r14->m14_currentY];
        sWorldGridCellTask* newCellTask = createSubTaskWithArgWithCopy<sWorldGridCellTask>(r14->m34_dataBuffer, cellData);
        r14->m40_cellTasks[(r14->mC + r4) & 7][(r14->m8 + r12) & 7] = newCellTask;

        createCellObjects(r14->m10_currentX + r12, r4 + r14->m14_currentY);
    }
}

static void createGridColumn(s32 r4, sWorldGrid* r14)
{
    s32 currentX = r14->m10_currentX + r4;
    if (currentX < 0)
        return;
    if (currentX >= r14->m0_sizeX)
        return;

    for (int r13 = -2; r13 <= 2; r13++)
    {
        s32 currentY = r14->m14_currentY + r13;
        if (currentY < 0)
            continue;
        if (currentY >= r14->m4_sizeY)
            continue;

        sSaturnPtr cellData = r14->m38_EnvironmentSetup->cells[currentX][currentY];
        sWorldGridCellTask* newCellTask = createSubTaskWithArgWithCopy<sWorldGridCellTask>(r14->m34_dataBuffer, cellData);
        r14->m40_cellTasks[(r14->mC + r13) & 7][(r14->m8 + r4) & 7] = newCellTask;

        createCellObjects(currentX, currentY);
    }
}

void decreaseNPCRefCount(s32 r5);

static void destroyCellObject(sCellObjectListNode* node)
{
    if (node->m8 != nullptr)
    {
        if (readSaturnS32(node->m4) > 0)
        {
            decreaseNPCRefCount(readSaturnS32(node->m4));
        }
        if (node->m8 != nullptr)
        {
            node->m8->getTask()->markFinished();
        }
        node->m8 = nullptr;
    }
}

static void deleteCellObjects(s32 x, s32 y)
{
    s32 index = y * gWorldGrid.m0_sizeX + x;
    sCellObjectListNode* node = gWorldGrid.m140_perCellObjectList[index];
    while (node)
    {
        destroyCellObject(node);
        node = node->m0_next;
    }
}

static void deleteGridRow(s32 r3, sWorldGrid* r14)
{
    s32 r13 = -2;
    s32 var24 = ((r14->mC + r3) & 7);

    do
    {
        sWorldGridCellTask*& cellTask = r14->m40_cellTasks[var24][(r14->m8 + r13) & 7];
        if (cellTask)
        {
            deleteCellObjects(r14->m10_currentX + r13, r14->m14_currentY + r3);
            if (cellTask)
            {
                ((s_workArea*)cellTask)->getTask()->markFinished();
            }
            cellTask = nullptr;
        }
    } while (++r13 <= 2);
}

static void deleteGridColumn(s32 r3, sWorldGrid* r14)
{
    s32 r13 = -2;
    s32 var24 = r14->m8;

    do
    {
        sWorldGridCellTask*& cellTask = r14->m40_cellTasks[(r14->mC + r13) & 7][(var24 + r3) & 7];
        if (cellTask)
        {
            deleteCellObjects(r14->m10_currentX + r3, r14->m14_currentY + r13);
            if (cellTask)
            {
                ((s_workArea*)cellTask)->getTask()->markFinished();
            }
            cellTask = nullptr;
        }
    } while (++r13 <= 2);
}

static s32 initTownGridDefaultDepth = 0x7FFFFFFF;

void initTownGrid()
{
    gWorldGrid.m0_sizeX = 0;
    gWorldGrid.m4_sizeY = 0;
    gWorldGrid.m140_perCellObjectList.clear();
    gWorldGrid.m34_dataBuffer = nullptr;
    gWorldGrid.m18_createCell = createGridRow;
    gWorldGrid.m1C_createCellColumn = createGridColumn;
    gWorldGrid.m20_deleteCell = deleteGridRow;
    gWorldGrid.m24_deleteCellColumn = deleteGridColumn;
    gWorldGrid.m40_cellTasks[0].fill(nullptr);
    gWorldGrid.m40_cellTasks[1].fill(nullptr);
    gWorldGrid.m3C = &initTownGridDefaultDepth;
}

void setupDragonForTown(s_fileBundle* pDragonBundle)
{
    if (pDragonBundle->getRawFileAtOffset(0x48))
    {
        Unimplemented();
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
    else if (overlayFileName == "TWN_CAMP.PRG")
    {
        gFieldOverlayFunction = overlayStart_TWN_CAMP;
    }
    else if (overlayFileName == "TWN_ZOAH.PRG")
    {
        gFieldOverlayFunction = overlayStart_TWN_ZOAH;
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

void initNPCSub0(npcFileDeleter* buffer, const sGrid* pGrid, u8 gridSizeX, u8 gridSizeY, fixedPoint cellSize)
{
    gWorldGrid.m18_createCell = createGridRow;
    gWorldGrid.m1C_createCellColumn = createGridColumn;
    gWorldGrid.m20_deleteCell = deleteGridRow;
    gWorldGrid.m24_deleteCellColumn = deleteGridColumn;

    initWorldGridData(buffer, pGrid, gridSizeX, gridSizeY, cellSize);
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
            // equivalent of loadDragonSub1Sub1 for npcFileDeleter (frees DRAM/VRAM allocations, both stubbed)
            fileEntry.mC_fileBundle->m0_fileBundle = nullptr;
            fileEntry.mC_fileBundle->m4_vd1Allocation = nullptr;
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
            resetWorldGrid();
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
    if (pThis->m8_cellNode && (pThis->m8_cellNode->m8 == pThis))
    {
        pThis->m8_cellNode->m8 = nullptr;
    }

    if (!r5.isNull())
    {
        decreaseNPCRefCount(readSaturnS32(r5));
    }

    pThisAsTask->getTask()->markFinished();
}



void sWorldGridCellTask::Init(sTownCellTask* pThis, sSaturnPtr arg)
{
    if ((readSaturnS32(arg + 0xC) == 0) && (readSaturnS32(arg + 0x10) == 0) && (readSaturnS32(arg + 0x14) == 0))
    {
        pThis->m_UpdateMethod = nullptr;
    }

    pThis->m8_cellPtr = arg;
    pThis->mC_position = readSaturnVec3(arg);
}

void sWorldGridCellTask::Draw(sTownCellTask* pThis)
{
    pushCurrentMatrix();
    {
        translateCurrentMatrix(pThis->mC_position);
        fixedPoint var0 = pCurrentMatrix->m[2][3];
        if (var0 >= graphicEngineStatus.m405C.m10_nearClipDistance - gTownGrid.m2C)
        {
            fixedPoint varMinusOne = MTH_Mul(var0, graphicEngineStatus.m405C.m2C_widthRatio);
            fixedPoint r4 = varMinusOne + MTH_Mul(gTownGrid.m2C, graphicEngineStatus.m405C.m28_widthRatio2);
            if ((pCurrentMatrix->m[0][3] >= -r4) && (pCurrentMatrix->m[0][3] <= r4))
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
                    while (readSaturnS32(r14))
                    {
                        pushCurrentMatrix();
                        translateCurrentMatrix(readSaturnVec3(r14 + 4));
                        addBillBoardToDrawList(pThis->m0_fileBundle->get3DModel(readSaturnS32(r14)));
                        popMatrix();
                        r14 += 0x10;
                    }
                }
            }
        }
    }
    popMatrix();
}

s32 isDataLoaded(s32 fileIndex)
{
    npcFileDeleter* r4 = dramAllocatorEnd[fileIndex].mC_fileBundle;
    if ((r4 != 0) && ((-1 < r4->m8 || (-1 < r4->mA)))) {
        return 0;
    }
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
    Unimplemented();
    
    //FIX:
    if(gModuleManager && gModuleManager->m8)
    {
        assert(gModuleManager->m8 == pThis);
        gModuleManager->m8 = nullptr;
    }
}

