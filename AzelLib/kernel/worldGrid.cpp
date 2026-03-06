#include "PDS.h"
#include "kernel/worldGrid.h"

sWorldGrid gWorldGrid;

s32 MTH_Mul32(fixedPoint a, fixedPoint b)
{
    fixedPoint temp(((s64)a.asS32() * (s64)b.asS32()) >> 16);
    return temp.getInteger();
}

void resetWorldGrid()
{
    deleteAllGridCells(&gWorldGrid);

    if (gWorldGrid.m140_perCellObjectList.size())
    {
        // TODO: cleanup cells data
        Unimplemented();
        gWorldGrid.m140_perCellObjectList.clear();
    }
}

static void initWorldGridObjectList()
{
    s32 r4 = gWorldGrid.m0_sizeX;
    if (r4 <= 0)
        r4 = 1;

    s32 r5 = gWorldGrid.m4_sizeY;
    if (r5 <= 0)
        r5 = 1;

    s32 size = r4 * r5;

    gWorldGrid.m140_perCellObjectList.resize(size);
    for (int i = 0; i < size; i++)
    {
        gWorldGrid.m140_perCellObjectList[i] = nullptr;
    }

    gWorldGrid.m144_nextFreeObjectListNode = &gWorldGrid.m148_objectListNodes[0];
    for (int i = 0; i < 0x40 - 1; i++)
    {
        gWorldGrid.m148_objectListNodes[i].m0_next = &gWorldGrid.m148_objectListNodes[i + 1];
    }
    gWorldGrid.m148_objectListNodes[0x3F].m0_next = nullptr;
}

void initWorldGridData(npcFileDeleter* buffer, const sGrid* pGrid, u8 r6_sizeX, u8 r7_sizeY, fixedPoint cellSize)
{
    resetWorldGrid();

    gWorldGrid.m0_sizeX = r6_sizeX;
    gWorldGrid.m4_sizeY = r7_sizeY;
    gWorldGrid.m34_dataBuffer = buffer;
    gWorldGrid.m28_cellSize = cellSize;
    gWorldGrid.m2C = MTH_Mul(0x10A3D, cellSize);
    gWorldGrid.m30_worldToCellIndex = FP_Div(0x10000, cellSize);
    gWorldGrid.m38_EnvironmentSetup = pGrid;

    initWorldGridObjectList();
    createGridCells(-3, -3, &gWorldGrid);
}

void deleteAllGridCells(sWorldGrid* r13)
{
    s32 r14 = -2;

    do
    {
        r13->m20_deleteCell(r14, r13);
        r14++;
    }while(r14 <= 2);
}

void createGridCells(s32 r4, s32 r5, sWorldGrid* r6)
{
    r6->mC = 0;
    r6->m8 = 0;
    r6->m10_currentX = r4;
    r6->m14_currentY = r5;

    for (int i = -2; i <= 2; i++)
    {
        r6->m18_createCell(i, r6);
    }
}

static void shiftGridColumn(s32 r4, sWorldGrid* r5)
{
    switch (r4)
    {
    case -1:
        r5->m24_deleteCellColumn(2, r5);
        r5->m8 = (r5->m8 - 1) & 7;
        r5->m10_currentX--;
        r5->m1C_createCellColumn(-2, r5);
        break;
    case 1:
        r5->m24_deleteCellColumn(-2, r5);
        r5->m8 = (r5->m8 + 1) & 7;
        r5->m10_currentX++;
        r5->m1C_createCellColumn(2, r5);
        break;
    }
}

static void shiftGridRow(s32 r4, sWorldGrid* r5)
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

s32 updateWorldGrid(s32 r4_x, s32 r5_y)
{
    updateWorldGrid(fixedPoint(r4_x), fixedPoint(r5_y));
    return 0;
}

void updateWorldGrid(fixedPoint r4_x, fixedPoint r5_y)
{
    s32 r12 = MTH_Mul32(r4_x, gWorldGrid.m30_worldToCellIndex);
    s32 r11 = MTH_Mul32(r5_y, gWorldGrid.m30_worldToCellIndex);

    s32 diffX = r12 - gWorldGrid.m10_currentX;
    s32 diffY = r11 - gWorldGrid.m14_currentY;
    s32 absDiffX = diffX < 0 ? -diffX : diffX;
    s32 absDiffY = diffY < 0 ? -diffY : diffY;

    if ((absDiffX > 1) || (absDiffY > 1))
    {
        deleteAllGridCells(&gWorldGrid);
        createGridCells(r12, r11, &gWorldGrid);
    }
    else
    {
        if (absDiffX)
        {
            shiftGridColumn(diffX, &gWorldGrid);
        }

        if (absDiffY)
        {
            shiftGridRow(diffY, &gWorldGrid);
        }
    }
}
