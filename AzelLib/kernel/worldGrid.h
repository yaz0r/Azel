#pragma once

#include <array>
#include <vector>

struct npcFileDeleter;
struct sGrid;

struct sTownObject : public s_workAreaCopy
{
    union {
        struct sCellObjectListNode* m8_cellNode; // cell system back-pointer
        u8* m8_MCBInDram;                        // NPC loading (dual-use on Saturn)
    };
};

struct sCellObjectListNode
{
    sCellObjectListNode* m0_next;
    sSaturnPtr m4;
    sTownObject* m8;
    //size 0xC
};

struct sWorldGridCellTask : public s_workAreaTemplateWithArgWithCopy<sWorldGridCellTask, sSaturnPtr>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sWorldGridCellTask::Init, nullptr, &sWorldGridCellTask::Draw, nullptr };
        return &taskDefinition;
    }

    static void Init(sWorldGridCellTask* pThis, sSaturnPtr arg);
    static void Draw(sWorldGridCellTask* pThis);

    sSaturnPtr m8_cellPtr;
    sVec3_FP mC_position;
    //size 0x18
};

struct sWorldGrid
{
    s32 m0_sizeX;
    s32 m4_sizeY;
    s32 m8;
    s32 mC;
    s32 m10_currentX;
    s32 m14_currentY;
    void(*m18_createCell)(s32, sWorldGrid*);
    void(*m1C_createCellColumn)(s32, sWorldGrid*);
    void(*m20_deleteCell)(s32, sWorldGrid*);
    void(*m24_deleteCellColumn)(s32, sWorldGrid*);
    fixedPoint m28_cellSize;
    fixedPoint m2C;
    fixedPoint m30_worldToCellIndex;
    npcFileDeleter* m34_dataBuffer;
    const sGrid* m38_EnvironmentSetup;
    s32* m3C;
    std::array<std::array<sWorldGridCellTask*, 8>, 8> m40_cellTasks;
    std::vector<sCellObjectListNode*> m140_perCellObjectList;
    sCellObjectListNode* m144_nextFreeObjectListNode;
    std::array<sCellObjectListNode, 0x40> m148_objectListNodes;
};

extern sWorldGrid gWorldGrid;

s32 MTH_Mul32(fixedPoint a, fixedPoint b);
void resetWorldGrid();
void initWorldGridData(npcFileDeleter* buffer, const sGrid* pGrid, u8 sizeX, u8 sizeY, fixedPoint cellSize);
void deleteAllGridCells(sWorldGrid* grid);
void createGridCells(s32 x, s32 y, sWorldGrid* grid);
void updateWorldGrid(fixedPoint x, fixedPoint y);
s32 updateWorldGrid(s32 x, s32 y);
