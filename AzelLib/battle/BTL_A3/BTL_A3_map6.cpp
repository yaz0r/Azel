#include "PDS.h"
#include "BTL_A3_map6.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "battle/battleGrid.h"
#include "field/field_a3/o_fld_a3.h" // TODO: remove
#include "town/town.h" // TODO: remove
#include "kernel/fileBundle.h"

void s_BTL_A3_Env_InitVdp2(s_BTL_A3_Env* pThis)
{
    getBattleManager()->m10_battleOverlay->m1C_envTask = pThis;
    reinitVdp2();

    fieldPaletteTaskInitSub0();

    asyncDmaCopy(gCurrentBattleOverlay->getSaturnPtr(0x60A68BC), getVdp2Cram(0x400), 0x200, 0);
    asyncDmaCopy(gCurrentBattleOverlay->getSaturnPtr(0x60A66BC), getVdp2Cram(0xA00), 0x200, 0);
    asyncDmaCopy(gCurrentBattleOverlay->getSaturnPtr(0x60A6ABC), getVdp2Cram(0xC00), 0x200, 0);
    //setupRGB0(gCurrentBattleOverlay->getSaturnPtr(0x60a6d84));

    FunctionUnimplemented();
}

void sBattleEnvironmentGridCell_Init(sTownCellTask* pThis, sSaturnPtr arg)
{
    int iVar1 = readSaturnS32(arg + 0xC);
    if ((readSaturnS32(arg + 0xC) == 0) && (readSaturnS32(arg + 0x10) == 0) && (readSaturnS32(arg + 0x14) == 0)) {
        pThis->m_DrawMethod = nullptr;
    }

    pThis->m8 = arg;
    pThis->mC_position = readSaturnVec3(arg + 0x0);
}

void sBattleEnvironmentGridCell_Draw(sTownCellTask* pThis)
{
    pushCurrentMatrix();
    translateCurrentMatrix(pThis->mC_position);

    if ((graphicEngineStatus.m405C.m10_nearClipDistance - gTownGrid.m2C) <= pCurrentMatrix->matrix[11])
    {
        s32 iVar1 = MTH_Mul(pCurrentMatrix->matrix[11], graphicEngineStatus.m405C.m2C_widthRatio);
        s32 iVar2 = MTH_Mul(gTownGrid.m2C, graphicEngineStatus.m405C.m2C_widthRatio);

        if (((-(iVar1 + iVar2) <= pCurrentMatrix->matrix[3]) && (pCurrentMatrix->matrix[3] <= iVar1 + iVar2)))
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
                        addObjectToDrawList(pThis->m0_dramAllocation->get3DModel(offset));
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
    popMatrix();
}

void battleGrid_createCellSub0(sTownCellTask* r4_newCellTask, s32 r5, s32 r6, s32 r7, s32 r8)
{
    FunctionUnimplemented();
}

void battleGrid_createCell(s32 index, sTownGrid* pGrid)
{
    int iVar2 = pGrid->mC;

    int sizeXMinusOne = pGrid->m0_sizeX - 1;
    int sizeYMinusOne = pGrid->m4_sizeY - 1;

    int sizeXMask = ~sizeXMinusOne;
    int sizeYMask = ~sizeYMinusOne;

    int uVar5 = pGrid->m14_currentY + index & sizeYMinusOne;
    int offsetY = (sizeYMask & (pGrid->m14_currentY + index)) * pGrid->m28_cellSize;

    for(int i=-2; i<3; i++)
    {
        int uVar4 = pGrid->m10_currentX + i & sizeXMinusOne;
        int offsetX = (sizeXMask & (pGrid->m10_currentX + i)) * pGrid->m28_cellSize;

        sSaturnPtr cellData = readSaturnEA(pGrid->m38_EnvironmentSetup + 4 * (pGrid->m0_sizeX * uVar5 + uVar4));

        static const sTownCellTask::TypedTaskDefinition definition = { sBattleEnvironmentGridCell_Init , nullptr, sBattleEnvironmentGridCell_Draw, nullptr};
        sTownCellTask* newCellTask = createSiblingTaskWithArgWithCopy<sTownCellTask>(pGrid->m34_dataBuffer, cellData, &definition);
        pGrid->m40_cellTasks[(pGrid->mC + index) & 7][(pGrid->m8 + i) & 7] = newCellTask;

        newCellTask->mC_position[0] += offsetX;
        newCellTask->mC_position[2] += offsetY;

        battleGrid_createCellSub0(newCellTask, uVar4, uVar5, offsetX, offsetY);
    }
}

void battleGrid_cellFunc1()
{
    FunctionUnimplemented();
}

void battleGrid_cellFunc2()
{
    FunctionUnimplemented();
}

void battleGrid_deleteCell(s32 index, sTownGrid* pGrid)
{
    for (int i = -2; i < 3; i++)
    {
        sTownCellTask** cellTask = &pGrid->m40_cellTasks[(pGrid->mC + index) & 7][(pGrid->m8 + i) & 7];
        if (*cellTask)
        {
            (*cellTask)->getTask()->markFinished();
            *cellTask = nullptr;
        }
    }
}

void initGridForBattle(npcFileDeleter* pFile, sSaturnPtr r5_envConfig, s32 r6_sizeX, s32 r7_sizeY, s32 r8_cellSize)
{
    gTownGrid.m18_createCell = battleGrid_createCell;
    gTownGrid.m1C = battleGrid_cellFunc1;
    gTownGrid.m20_deleteCell = battleGrid_deleteCell;
    gTownGrid.m24 = battleGrid_cellFunc2;

    initNPCSub0Sub2(pFile, r5_envConfig, r6_sizeX, r7_sizeY, r8_cellSize);
}

static void BTL_A3_map6_Init(s_BTL_A3_Env* pThis)
{
    loadFile("SCBTLA31.SCB", getVdp2Vram(0x40000), 0);
    loadFile("SCBTL_A3.PNB", getVdp2Vram(0x62800), 0);
    loadFile("SPACE.PNB", getVdp2Vram(0x64000), 0);

    pThis->m38 = -0x64000;

    s_BTL_A3_Env_InitVdp2(pThis);

    allocateNPC(pThis, 8);
    initGridForBattle(dramAllocatorEnd[8].mC_buffer, gCurrentBattleOverlay->getSaturnPtr(0x60a6698), 2, 2, 0x400000);
    pThis->m58 = dramAllocatorEnd[8].mC_buffer;

    getBattleManager()->m10_battleOverlay->m8_gridTask->m1C8_flags |= 0x10;

    FunctionUnimplemented();
}

void s_BTL_A3_Env_Update(s_BTL_A3_Env* pThis)
{
    FunctionUnimplemented();
}

void s_BTL_A3_Env_Draw(s_BTL_A3_Env* pThis)
{
    FunctionUnimplemented();
}

p_workArea Create_BTL_A3_map6(p_workArea parent)
{
    static const s_BTL_A3_Env::TypedTaskDefinition definition = {
        &BTL_A3_map6_Init,
        &s_BTL_A3_Env_Update,
        &s_BTL_A3_Env_Draw,
        nullptr,
    };

    return createSubTask<s_BTL_A3_Env>(parent, &definition);
}
