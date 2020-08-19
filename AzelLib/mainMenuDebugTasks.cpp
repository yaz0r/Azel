#include "PDS.h"
#include "town/town.h"
#include "town/townDebugSelect.h"
#include "kernel/animation.h"
#include "kernel/vdp1Allocator.h"
#include "kernel/fileBundle.h"
#include "kernel/loadSavegameScreen.h"
#include "kernel/menuCursor.h"
#include "kernel/menuSprite.h"
#include "battle/battleManager.h"
#include "audio/systemSounds.h"
#include "audio/soundDriver.h"
#include "commonOverlay.h"
#include "field/field_a3/o_fld_a3.h" //TODO: cleanup
#include "dragonData.h"
#include "dragonRider.h"
#include "menu/inventoryMenu.h"

p_workArea createModuleManager(p_workArea pTypelessWorkArea, u32 menuID);

extern p_workArea(*statusMenuSubMenus[])(p_workArea);

s16 loadingTaskVar0 = 0x1D;
p_workArea(*gFieldOverlayFunction)(p_workArea workArea, u32 arg);
p_workArea fieldTaskUpdateSub0(u32 fieldIndexMenuSelection, u32 subFieldIndexMenuSelection, u32 m3A, u32 currentSubFieldIndex);

struct s_flagEditTaskWorkArea : public s_workAreaTemplateWithArg<s_flagEditTaskWorkArea, p_workArea>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &s_flagEditTaskWorkArea::Init, NULL, &s_flagEditTaskWorkArea::Update, NULL};
        return &taskDefinition;
    }

    static void Init(s_flagEditTaskWorkArea* pThis, p_workArea argument)
    {
        pThis->m4 = argument;
    }

    static void Update(s_flagEditTaskWorkArea* pThis)
    {
        switch (pThis->state)
        {
        case 0:
            if (readKeyboardToggle(0x85))
            {
                FunctionUnimplemented();
            }
            break;
        default:
            assert(0);
        }

        if (pThis->m4)
        {
            if (pThis->m4->getTask()->isFinished())
            {
                pThis->getTask()->markFinished();
            }
        }
    }

    u32 state;
    p_workArea m4; //4
};

void computeDragonSprAndAglFromCursor()
{
    u32 statAxisScale = mainGameState.gameStats.m18_statAxisScale + 1;

    switch (mainGameState.gameStats.m1_dragonLevel)
    {
    case DR_LEVEL_0_BASIC_WING:
    case DR_LEVEL_6_LIGHT_WING:
    case DR_LEVEL_8_FLOATER:
        mainGameState.gameStats.m1A_dragonCursorX = 0;
        mainGameState.gameStats.m1C_dragonCursorY = 0;
        break;
    default:
        break;
    }

    // figure out the dragon archetype
    mainGameState.gameStats.mB6_dragonArchetype = DR_ARCHETYPE_4_SPIRITUAL;

    int cursorX = mainGameState.gameStats.m1A_dragonCursorX;
    if (cursorX < 0)
    {
        cursorX = -cursorX;
        mainGameState.gameStats.mB6_dragonArchetype = DR_ARCHETYPE_2_ATTACK;
    }

    e_dragonArchetype archetypeFromAxisY = DR_ARCHETYPE_3_AGILITY;
    int cursorY = mainGameState.gameStats.m1C_dragonCursorY;
    if (cursorY < 0)
    {
        cursorY = -cursorY;
        archetypeFromAxisY = DR_ARCHETYPE_1_DEFENSE;
    }

    if (cursorX < 0x400)
    {
        mainGameState.gameStats.mB6_dragonArchetype = DR_ARCHETYPE_0_NORMAL;
    }
    if (cursorY < 0x400)
    {
        archetypeFromAxisY = DR_ARCHETYPE_0_NORMAL;
    }

    if (
        ((mainGameState.gameStats.mB6_dragonArchetype != DR_ARCHETYPE_0_NORMAL) || (archetypeFromAxisY != DR_ARCHETYPE_0_NORMAL))
        && (cursorX < cursorY) // more influence by Y than X
        )
    {
        mainGameState.gameStats.mB6_dragonArchetype = archetypeFromAxisY;
    }

    // stats based on Y
    mainGameState.gameStats.mC0_dragonAgl = performDivision(0x2000, (mainGameState.gameStats.m1C_dragonCursorY + 0x800) * statAxisScale);
    mainGameState.gameStats.mBC_dragonDef = (statAxisScale / 2) - mainGameState.gameStats.mC0_dragonAgl;

    // stats based on X
    mainGameState.gameStats.mC2_dragonSpr = performDivision(0x2000, (mainGameState.gameStats.m1A_dragonCursorX + 0x800) * statAxisScale);
    mainGameState.gameStats.mBE_dragonAtt = (statAxisScale / 2) - mainGameState.gameStats.mC2_dragonSpr;
}

void updateDragonStatsFromLevel()
{
    s_gameStats& gameStats = mainGameState.gameStats;

    if (gameStats.m1_dragonLevel < DR_LEVEL_8_FLOATER)
    {
        gameStats.mB8_maxHP = gameStats.m12_classMaxHP + dragonPerLevelMaxHPBP[gameStats.m1_dragonLevel].maxHP;
        gameStats.mBA_maxBP = gameStats.m16_classMaxBP + dragonPerLevelMaxHPBP[gameStats.m1_dragonLevel].maxBP;
    }
    else
    {
        gameStats.mB8_maxHP = dragonPerLevelMaxHPBP[gameStats.m1_dragonLevel].maxHP;
        gameStats.mBA_maxBP = dragonPerLevelMaxHPBP[gameStats.m1_dragonLevel].maxBP;
    }

    if (gameStats.m10_currentHP > gameStats.mB8_maxHP)
        gameStats.m10_currentHP = gameStats.mB8_maxHP;

    if (gameStats.m14_currentBP > gameStats.mBA_maxBP)
        gameStats.m14_currentBP = gameStats.mBA_maxBP;

    computeDragonSprAndAglFromCursor();
}

void rotl(u32& value)
{
    u32 bit = value & 0x80000000;
    value <<= 1;
    value |= (bit >> 31);
}

u32 readPackedBits(u8* bitField, u32 firstBitOffset, u32 numBits)
{
    u32 r0_startOfByteInBits = (firstBitOffset & ~0x1F);
    u32 r5_lastBitToChange = numBits + firstBitOffset - r0_startOfByteInBits ;
    u8* r4_targetByte = bitField + (r0_startOfByteInBits / 8);
    u32 r6_bitMask = longBitMask[numBits];
    u32 r1 = 0x20;

    if (r5_lastBitToChange > 32)
    {
        assert(0);
        return 0;
    }
    else
    {
        u32 value1 = (r4_targetByte[0] << 24) | (r4_targetByte[1] << 16) | (r4_targetByte[2] << 8) | (r4_targetByte[3]);

        if (r5_lastBitToChange == 32)
        {
            return value1 & r6_bitMask;
        }
        else
        {
            r1 -= r5_lastBitToChange;
            value1 >>= r1;

            return value1 & r6_bitMask;
        }
    }
}

void setPackedBits(u8* bitField, u32 firstBitOffset, u32 numBits, u32 value)
{
    u32 startOfByteInBits = (firstBitOffset & ~0x1F);
    u32 lastBitToChange = numBits + firstBitOffset - startOfByteInBits;
    u8* targetByte = bitField + (startOfByteInBits / 8);
    u32 bitMask = longBitMask[numBits];

    value &= bitMask; // we can't set a value larger than the number of bits requested

    // read in the proper order
    u32 value1 = (targetByte[0] << 24) | (targetByte[1] << 16) | (targetByte[2] << 8) | (targetByte[3]);

    // did we overflow into the next u32?
    if (lastBitToChange < 32)
    {
        bitMask ^= 0xFFFFFFFF;
        if (32 != lastBitToChange)
        {
            u32 r1 = 32 - lastBitToChange;

            do
            {
                rotl(bitMask);
                rotl(value);
            } while (--r1);
        }

        value1 &= bitMask;
        value1 |= value;

        targetByte[0] = (value1 >> 24) & 0xFF;
        targetByte[1] = (value1 >> 16) & 0xFF;
        targetByte[2] = (value1 >> 8) & 0xFF;
        targetByte[3] = (value1 >> 0) & 0xFF;
    }
    else
    {
        assert(0);
    }
}

p_workArea fieldTaskVar0;
p_workArea fieldInputTaskWorkArea;
u32 fieldTaskVar2;
u8 fieldTaskVar3;

void updateFieldTaskNoBattleOverride(s_fieldTaskWorkArea* pWorkArea)
{
    switch (pWorkArea->m3C_fieldTaskState)
    {
    case 0:
        break;
    case 3:
        fieldTaskUpdateSub0(pWorkArea->m36_fieldIndexMenuSelection, pWorkArea->m38_subFieldIndexMenuSelection, pWorkArea->m3A, pWorkArea->m2E_currentSubFieldIndex);
        break;
    case 4:
        return;
    case 5:
        pWorkArea->m3C_fieldTaskState++;
        break;
    case 6:
        if (pWorkArea->m8_pSubFieldData == nullptr)
        {
            pWorkArea->m3C_fieldTaskState = 3;
        }
        break;
    case 7:
        break;
    default:
        assert(0);
        break;
    }

    return;
}

void s_fieldTaskWorkArea::fieldTaskInit(s_fieldTaskWorkArea* pThis, s32 battleArgument)
{
    fieldTaskPtr = pThis;
    fieldTaskVar0 = NULL;
    fieldInputTaskWorkArea = NULL;
    fieldTaskVar2 = 0;

    pThis->m35 = battleArgument;
    pThis->m3D = -1;

    if (battleArgument)
    {
        pThis->m28_status = 0;
        pThis->m_UpdateMethod = updateFieldTaskNoBattleOverride;
    }
    else
    {
        pThis->m28_status = 1;
    }
}

u32 performModulo(u32 r0, u32 r1)
{
    assert(r0);

    return r1 % r0;
}

u32 performModulo2(u32 r0, u32 r1)
{
    assert(r0);

    return r1 % r0;
}


void setOpenMenu7()
{
    graphicEngineStatus.m40AC.m0_menuId = 7;
}

struct s_fieldDebugListWorkArea : public s_workAreaTemplate<s_fieldDebugListWorkArea>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &s_fieldDebugListWorkArea::Init, &s_fieldDebugListWorkArea::Update, NULL, NULL};
        return &taskDefinition;
    }

    static void Init(s_fieldDebugListWorkArea* pWorkArea)
    {
        s_fieldTaskWorkArea* pFieldTask = getFieldTaskPtr();

        pFieldTask->m36_fieldIndexMenuSelection = pFieldTask->m2C_currentFieldIndex;
        pFieldTask->m38_subFieldIndexMenuSelection = pFieldTask->m2E_currentSubFieldIndex;
        pFieldTask->m3A = pFieldTask->m30_fieldEntryPoint;
        pFieldTask->m2E_currentSubFieldIndex = -1;

        if (pFieldTask->m36_fieldIndexMenuSelection < 0)
        {
            pFieldTask->m36_fieldIndexMenuSelection = 0;
        }

        while (!fieldEnabledTable[pFieldTask->m36_fieldIndexMenuSelection])
        {
            pFieldTask->m36_fieldIndexMenuSelection++;
        }

        reinitVdp2();

        vdp2Controls.m4_pendingVdp2Regs->m10_CYCA0 = 0x3FFF7FFF;
        vdp2Controls.m_isDirty = true;

        vdp2Controls.m4_pendingVdp2Regs->mFA_PRINB = (vdp2Controls.m4_pendingVdp2Regs->mFA_PRINB & 0xF8FF) | 0x700;
        vdp2Controls.m_isDirty = true;

        if (g_fadeControls.m_4D >= g_fadeControls.m_4C)
        {
            vdp2Controls.m20_registers[0].m112_CLOFSL = 0x10;
            vdp2Controls.m20_registers[1].m112_CLOFSL = 0x10;
        }

        fadePalette(&g_fadeControls.m0_fade0, 0xC210, 0xC210, 1);
        fadePalette(&g_fadeControls.m24_fade1, 0xC210, 0xC210, 1);
    }

    static void Update(s_fieldDebugListWorkArea* pWorkArea)
    {
        pWorkArea->m0_ticks++;
        s_fieldTaskWorkArea*r14 = getFieldTaskPtr();

        u32 var_24 = performModulo(30, pWorkArea->m0_ticks);

        if (pWorkArea->m8_isSelectingSubfield)
        {

            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 0x40)
            {
                if (pWorkArea->m4_selectedSubField == 0)
                {
                    pWorkArea->m8_isSelectingSubfield = 0;
                }
            }

            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.mC_newButtonDown2 & 0x20) // down
            {
                if (++r14->m38_subFieldIndexMenuSelection >= fieldDefinitions[r14->m36_fieldIndexMenuSelection].m_numSubFields)
                {
                    r14->m38_subFieldIndexMenuSelection = 0;
                }
            }

            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.mC_newButtonDown2 & 0x10) // up
            {
                if (--r14->m38_subFieldIndexMenuSelection < 0)
                {
                    r14->m38_subFieldIndexMenuSelection = fieldDefinitions[r14->m36_fieldIndexMenuSelection].m_numSubFields - 1;
                }
            }

            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.mC_newButtonDown2 & 0x80) // page down
            {
                PDS_unimplemented("Page down");
            }

            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.mC_newButtonDown2 & 0x40) // page up
            {
                PDS_unimplemented("Page up");
            }
        }
        else
        {
            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 0x80)
            {
                pWorkArea->m8_isSelectingSubfield = 1;
            }

            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.mC_newButtonDown2 & 0x20) // down
            {
                clearVdp2StringFieldDebugList();
                r14->m38_subFieldIndexMenuSelection = 0;

                do
                {
                    r14->m36_fieldIndexMenuSelection++;
                    if (r14->m36_fieldIndexMenuSelection >= 23)
                    {
                        r14->m36_fieldIndexMenuSelection = 0;
                    }
                } while (!fieldEnabledTable[r14->m36_fieldIndexMenuSelection]);
            }

            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.mC_newButtonDown2 & 0x10) // up
            {
                clearVdp2StringFieldDebugList();
                r14->m38_subFieldIndexMenuSelection = 0;

                do
                {
                    r14->m36_fieldIndexMenuSelection--;
                    if (r14->m36_fieldIndexMenuSelection < 0)
                    {
                        r14->m36_fieldIndexMenuSelection = 22;
                    }
                } while (!fieldEnabledTable[r14->m36_fieldIndexMenuSelection]);
            }
        }

        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.mC_newButtonDown2 & 0x8000)
        {
            assert(0);
        }

        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.mC_newButtonDown2 & 0x800)
        {
            assert(0);
        }

        u32 r8 = 3;
        for (u32 r12 = 0; r12 < 23; r12++)
        {
            vdp2DebugPrintSetPosition(0xA, r8);
            if (r14->m36_fieldIndexMenuSelection == r12) // is this the selected field?
            {
                vdp2PrintStatus.m10_palette = 0x8000;
            }
            else
            {
                u16 selectedColor[] = { 0xD, 0xD };
                assert(pWorkArea->m8_isSelectingSubfield >= 0);
                assert(pWorkArea->m8_isSelectingSubfield <= 1);
                vdp2PrintStatus.m10_palette = selectedColor[pWorkArea->m8_isSelectingSubfield] << 12;
            }

            if (fieldEnabledTable[r12])
            {
                drawLineSmallFont(fieldDefinitions[r12].m_name);

                // if selecting field
                if (pWorkArea->m8_isSelectingSubfield == 0)
                {
                    if (r14->m36_fieldIndexMenuSelection == r12)
                    {
                        vdp2PrintStatus.m10_palette = 0xD000;
                        vdp2DebugPrintSetPosition(0xA, r8);
                        drawLineSmallFont("\x7F");
                    }
                }

                r8++;
            }
        }

        u32 var_2C = 0;

        u32 numSubFields = fieldDefinitions[r14->m36_fieldIndexMenuSelection].m_numSubFields;
        const char** subFields = fieldDefinitions[r14->m36_fieldIndexMenuSelection].m_subFields;

        s16 r2 = r14->m38_subFieldIndexMenuSelection;
        if (r2 < 0)
        {
            r2 += 0xF;
        }

        pWorkArea->m4_selectedSubField = (r2 >> 4) << 4;

        clearVdp2StringFieldDebugList();

        u32 r12 = pWorkArea->m4_selectedSubField;

        while (true)
        {
            u32 r4 = pWorkArea->m4_selectedSubField + 16;
            u32 maxNumFields;
            if (numSubFields < r4)
            {
                maxNumFields = numSubFields;
            }
            else
            {
                maxNumFields = r4;
            }

            if (r12 < maxNumFields)
            {
                vdp2DebugPrintSetPosition(0x12, var_2C + 3);
                var_2C++;

                if (r12 == r14->m38_subFieldIndexMenuSelection)
                {
                    vdp2PrintStatus.m10_palette = 0x9000;
                }
                else
                {
                    vdp2PrintStatus.m10_palette = 0x8000;
                }

                drawLineSmallFont(subFields[r12]);
            }
            else
            {
                break;
            }

            r12++;
        }

        if (pWorkArea->m8_isSelectingSubfield)
        {
            vdp2PrintStatus.m10_palette = 0x9000;
            vdp2DebugPrintSetPosition(0x12, r14->m38_subFieldIndexMenuSelection - pWorkArea->m4_selectedSubField + 3);
            drawLineSmallFont("\x7F");
        }

        vdp2PrintStatus.m10_palette = 0x9000;
        vdp2DebugPrintSetPosition(0x1D, 3);
        vdp2PrintfSmallFont("%d   ", r14->m3A);

        vdp2PrintStatus.m10_palette = 0x7000;
        vdp2DebugPrintSetPosition(3, 0x18);
        vdp2PrintfLargeFont("GO:%2d  ", r14->m3D);

        vdp2DebugPrintSetPosition(3, -2);
        if (var_24 < 21)
        {
            const char* buildType[] = {
                "      ",
                "(N)   ",
                "(I)   ",
                "(NI)  ",
                "(T)   ",
                "(NT)  ",
                "(IT)  ",
                "(NIT) ",
                "(H)   ",
                "(HN)  ",
                "(HI)  ",
                "(HNI) ",
                "(HT)  ",
                "(HNT) ",
                "(HIT) ",
                "(HNIT)",
            };
            vdp2PrintfSmallFont("%s %s", "Ver. 11/14 ", buildType[0]);
        }
        else
        {
            clearVdp2TextSmallFont();
        }

        vdp2PrintStatus.m10_palette = 0xC000;

        u8 inputValue = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 0xF;
        if (inputValue == 0)
        {
            return;
        }

        if (inputValue & 3)
        {
            setOpenMenu7();
        }

        if (g_fadeControls.m_4D >= g_fadeControls.m_4C)
        {
            vdp2Controls.m20_registers[0].m112_CLOFSL = 0x10;
            vdp2Controls.m20_registers[1].m112_CLOFSL = 0x10;
        }

        fadePalette(&g_fadeControls.m0_fade0, 0, 0, 1);
        fadePalette(&g_fadeControls.m24_fade1, 0, 0, 1);

        r14->m3C_fieldTaskState++;
    }

    u32 m0_ticks; //0
    u32 m4_selectedSubField; // 4
    u32 m8_isSelectingSubfield; // 8
};

p_workArea createFieldInputTask(p_workArea pTypelessWorkArea)
{
    return createSubTask<s_fieldDebugListWorkArea>(pTypelessWorkArea);
}

struct s_dramAllocationNode
{
    s_dramAllocationNode* m_pNext;
    u32 size;
};

struct s_dramAllocator
{
    s_dramAllocationNode* m0_buffer; //0
    u32 m4_pNext; // 4
    u8* m8_allocationStart; //0x8
    u8* mC_allocationEnd; // 0xC
    s_dramAllocator* m10_nextNode; // 0x10
    u32 m14;
}; // size 18


s_dramAllocator* dramAllocatorHead = NULL;
std::vector<s_fileEntry> dramAllocatorEnd;

void resetTempAllocators()
{
    dramAllocatorHead = NULL;
    dramAllocatorEnd.clear();
    vdp1AllocatorHead = NULL;
}

void loadRamResource(s_workArea* pWorkArea)
{
    if (dramAllocatorHead)
    {
        u8* pVar2 = dramAllocatorHead->m8_allocationStart;
        u8* pVar5 = dramAllocatorHead->mC_allocationEnd;

        int iVar4 = 0;
        while (pVar2 < pVar5)
        {
            s_dramAllocationNode* pNode = (s_dramAllocationNode*)pVar2;
            pVar2 += pNode->size;
            iVar4 += 8;
        }

        FunctionUnimplemented();
    }
}

void initDramAllocator(s_workArea* pWorkArea, u8* dest, u32 size, const char** assetList)
{
    loadRamResource(pWorkArea);

    s_dramAllocator* pDramAllocator = (s_dramAllocator*)allocateHeapForTask(pWorkArea, sizeof(s_dramAllocator));
    
    pDramAllocator->m8_allocationStart = dest;
    pDramAllocator->mC_allocationEnd = dest + size;
    pDramAllocator->m10_nextNode = dramAllocatorHead;
    dramAllocatorHead = pDramAllocator;

    dramAllocatorEnd.clear();
    u32 pNext = 0;

    if (assetList)
    {
        while (*assetList)
        {
            s_fileEntry newFileEntry;
            if (*assetList == (const char*)-1)
            {
                newFileEntry.m0_fileID = -1;
                newFileEntry.m4_fileSize = 0;
            }
            else
            {
                newFileEntry.mFileName = *assetList;
                newFileEntry.m0_fileID = findMandatoryFileOnDisc(*assetList);
                newFileEntry.m4_fileSize = getFileSizeFromFileId(*assetList);
            }

            newFileEntry.m8_refcount = 0;
            newFileEntry.mC_fileBundle = 0;

            dramAllocatorEnd.push_back(newFileEntry);

            assetList++;
        }
    }

    s_dramAllocationNode* pNode = (s_dramAllocationNode*)dest;

    pDramAllocator->m0_buffer = pNode;
    pDramAllocator->m4_pNext = pNext;

    pNode->m_pNext = NULL;
    pNode->size = size;

    addToMemoryLayout(dest, 8);
}

u8* dramAllocate(u32 size)
{
    if (size == 0)
        return nullptr;

    // TODO: does the alignment stuff still works in 64bits?
    u32 paddedSize = (size + sizeof(s_dramAllocationNode) + 0xF) & ~0xF;

    s_dramAllocationNode** r5 = &dramAllocatorHead->m0_buffer;

    while (s_dramAllocationNode* r14 = *r5)
    {
        u32 blockSize = r14->size;

        if (blockSize >= paddedSize)
        {
            if (paddedSize == blockSize)
            {
                *r5 = r14->m_pNext;
            }
            else
            {
                s_dramAllocationNode* pNewNode = (s_dramAllocationNode*)(((u8*)r14) + paddedSize);

                u32 newNodeSize = blockSize - paddedSize;

                *r5 = pNewNode;

                pNewNode->m_pNext = r14->m_pNext;
                pNewNode->size = newNodeSize;

                addToMemoryLayout((u8*)pNewNode, 8);

                r14->size = paddedSize;
            }

            r14->m_pNext = NULL;
            return (u8*)(r14 + 1);
        }

        r5 = &r14->m_pNext;
    }

    return NULL;
}

void unimplementedUpdate(s_3dModel* pDragonStateData1)
{
    assert(0);
}

void unimplementedDraw(s_3dModel* pDragonStateData1)
{
    assert(0);
}

void modeDrawFunction10Sub1(sModelHierarchy* pModelData, std::vector<sPoseData>::iterator& r14)
{
    do
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&r14->m0_translation);
        rotateCurrentMatrixZYX(&r14->mC_rotation);

        if (pModelData->m0_3dModel)
        {
            addObjectToDrawList(pModelData->m0_3dModel);
        }

        if (pModelData->m4_subNode)
        {
            r14++;
            modeDrawFunction10Sub1(pModelData->m4_subNode, r14);
        }


        popMatrix();

        // End of model
        if (pModelData->m8_nextNode == nullptr)
        {
            return;
        }

        r14++;
        pModelData = pModelData->m8_nextNode;

    } while (1);
}

void modeDrawFunction6Sub1(sModelHierarchy* pModelData, std::vector<sPoseData>::iterator& pPoseData, std::vector<s_hotpointDefinition>::iterator& r6, std::vector<std::vector<sVec3_FP>>::iterator& r7)
{
    do
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pPoseData->m0_translation);
        rotateCurrentMatrixZYX(&pPoseData->mC_rotation);

        if (pModelData->m0_3dModel)
        {
            addObjectToDrawList(pModelData->m0_3dModel);
        }

        assert((*r7).size() == r6->m4_count);

        if (r6->m4_count == 0)
        {
            assert((*r7).size() == 0);
        }

        for (u32 i = 0; i < r6->m4_count; i++)
        {
            sVec3_FP input = r6->m0[i].m4;
            sVec3_FP& output = (*r7)[i];
            transformAndAddVecByCurrentMatrix(&input, &output);
        }

        if (pModelData->m4_subNode)
        {
            // next matrix
            pPoseData++;
            // next bone stuff
            r6++;
            // next ???
            r7++;

            modeDrawFunction6Sub1(pModelData->m4_subNode, pPoseData, r6, r7);
        }

        popMatrix();

        // End of model
        if (pModelData->m8_nextNode == nullptr)
        {
            return;
        }

        // next matrix
        pPoseData++;
        // next bone stuff
        r6++;
        // next ???
        r7++;

        pModelData = pModelData->m8_nextNode;
    } while (1);
}

void modeDrawFunction4Sub1(sModelHierarchy* pModelData, std::vector<sPoseData>::iterator& pPoseData, std::vector<s_hotpointDefinition>::iterator& r6, std::vector<std::vector<sVec3_FP>>::iterator& r7)
{
    do
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pPoseData->m0_translation);
        rotateCurrentMatrixZYX(&pPoseData->mC_rotation);
        scaleCurrentMatrixRow0(pPoseData->m18_scale[0]);
        scaleCurrentMatrixRow1(pPoseData->m18_scale[1]);
        scaleCurrentMatrixRow2(pPoseData->m18_scale[2]);
        if (pModelData->m0_3dModel)
        {
            addObjectToDrawList(pModelData->m0_3dModel);
        }

        assert((*r7).size() == r6->m4_count);

        if (r6->m4_count == 0)
        {
            assert((*r7).size() == 0);
        }

        for (u32 i = 0; i < r6->m4_count; i++)
        {
            sVec3_FP input = r6->m0[i].m4;
            sVec3_FP& output = (*r7)[i];
            transformAndAddVecByCurrentMatrix(&input, &output);
        }

        if (pModelData->m4_subNode)
        {
            // next matrix
            pPoseData++;
            // next bone stuff
            r6++;
            // next ???
            r7++;

            modeDrawFunction4Sub1(pModelData->m4_subNode, pPoseData, r6, r7);
        }

        popMatrix();

        // End of model
        if (pModelData->m8_nextNode == nullptr)
        {
            return;
        }

        // next matrix
        pPoseData++;
        // next bone stuff
        r6++;
        // next ???
        r7++;

        pModelData = pModelData->m8_nextNode;
    } while (1);
}

void modeDrawFunction4Sub2(sModelHierarchy* pModelData, std::vector<sPoseData>::iterator& pPoseData, std::vector<s_hotpointDefinition>::iterator& r6, std::vector<std::vector<sVec3_FP>>::iterator& r7)
{
    PDS_unimplemented("modeDrawFunction4Sub2");
}

void submitModelToRendering(sModelHierarchy* pModelData, std::vector<sMatrix4x3>::iterator& modelMatrix, std::vector<s_hotpointDefinition>::iterator& r6, std::vector<std::vector<sVec3_FP>>::iterator& r7)
{
    do 
    {
        pushCurrentMatrix();
        multiplyCurrentMatrix(&(*modelMatrix));

        if (pModelData->m0_3dModel)
        {
            addObjectToDrawList(pModelData->m0_3dModel);
        }

        assert((*r7).size() == r6->m4_count);

        if (r6->m4_count == 0)
        {
            assert((*r7).size() == 0);
        }

        for (u32 i = 0; i < r6->m4_count; i++)
        {
            sVec3_FP input = r6->m0[i].m4;
            sVec3_FP& output = (*r7)[i];
            transformAndAddVecByCurrentMatrix(&input, &output);
        }

        if (pModelData->m4_subNode)
        {
            // next matrix
            modelMatrix++;
            // next bone stuff
            r6++;
            // next ???
            r7++;

            submitModelToRendering(pModelData->m4_subNode, modelMatrix, r6, r7);
        }

        popMatrix();

        // End of model
        if (pModelData->m8_nextNode == nullptr)
        {
            return;
        }

        // next matrix
        modelMatrix++;
        // next bone stuff
        r6++;
        // next ???
        r7++;

        pModelData = pModelData->m8_nextNode;
    } while (1);
}

void modelDrawFunction0(s_3dModel* pModel)
{
    unimplementedDraw(pModel);
}
void modelDrawFunction1(s_3dModel* pDragonStateData1)
{
    std::vector<std::vector<sVec3_FP>>::iterator var_0 = pDragonStateData1->m44_hotpointData.begin();
    std::vector<sMatrix4x3>::iterator var_8 = pDragonStateData1->m3C_boneMatrices.begin();
    std::vector<s_hotpointDefinition>::iterator var_4 = pDragonStateData1->m40->begin();

    if (pDragonStateData1->m8 & 1)
    {
        sModelHierarchy* r4 = pDragonStateData1->m4_pModelFile->getModelHierarchy(pDragonStateData1->mC_modelIndexOffset);
        submitModelToRendering(r4, var_8, var_4, var_0);
    }
    else
    {
        sModelHierarchy* r4 = pDragonStateData1->m4_pModelFile->getModelHierarchy(pDragonStateData1->mC_modelIndexOffset);
        modeDrawFunction1Sub2(r4, var_8, var_4, var_0);
    }
}
void modelDrawFunction2(s_3dModel* pModel)
{
    unimplementedDraw(pModel);
}
void modelDrawFunction3(s_3dModel* pModel)
{
    unimplementedDraw(pModel);
}
void modelDrawFunction4(s_3dModel* pModel)
{
    std::vector<std::vector<sVec3_FP>>::iterator var_0 = pModel->m44_hotpointData.begin();
    std::vector<sPoseData>::iterator pPoseData = pModel->m2C_poseData.begin();
    std::vector<s_hotpointDefinition>::iterator var_4 = pModel->m40->begin();
    sModelHierarchy* r4 = pModel->m4_pModelFile->getModelHierarchy(pModel->mC_modelIndexOffset);

    if (pModel->m8 & 1)
    {
        modeDrawFunction4Sub1(r4, pPoseData, var_4, var_0);
    }
    else
    {
        modeDrawFunction4Sub2(r4, pPoseData, var_4, var_0);
    }
}
void modelDrawFunction5(s_3dModel* pModel)
{
    unimplementedDraw(pModel);
}
void modelDrawFunction6(s_3dModel* pModel)
{
    std::vector<std::vector<sVec3_FP>>::iterator var_0 = pModel->m44_hotpointData.begin();
    std::vector<sPoseData>::iterator pPoseData = pModel->m2C_poseData.begin();
    std::vector<s_hotpointDefinition>::iterator var_4 = pModel->m40->begin();
    sModelHierarchy* r4 = pModel->m4_pModelFile->getModelHierarchy(pModel->mC_modelIndexOffset);

    if (pModel->m8 & 1)
    {
        modeDrawFunction6Sub1(r4, pPoseData, var_4, var_0);
    }
    else
    {
        modeDrawFunction6Sub2(r4, pPoseData, var_4, var_0);
    }
}
void modelDrawFunction9(s_3dModel* pModel)
{
    if (pModel->m8 & 1)
    {
        sModelHierarchy* r4 = pModel->m4_pModelFile->getModelHierarchy(pModel->mC_modelIndexOffset);
        std::vector<sPoseData>::iterator pPoseData = pModel->m2C_poseData.begin();
        FunctionUnimplemented(); // TODO: should be vertex colored variant!
        modeDrawFunction10Sub1(r4, pPoseData);
    }
}
void modelDrawFunction10(s_3dModel* pModel)
{
    if (pModel->m8 & 1)
    {
        sModelHierarchy* r4 = pModel->m4_pModelFile->getModelHierarchy(pModel->mC_modelIndexOffset);
        std::vector<sPoseData>::iterator pPoseData = pModel->m2C_poseData.begin();
        modeDrawFunction10Sub1(r4, pPoseData);
    }
}

s32 initAnimation(s_3dModel* r4_pModel, sAnimationData* pAnimation)
{
    if (pAnimation == NULL)
    {
        if (r4_pModel->mA_animationFlags & 8)
        {
            copyPosePosition(r4_pModel);
        }
        if (r4_pModel->mA_animationFlags & 0x10)
        {
            copyPoseRotation(r4_pModel);
        }
        if (r4_pModel->mA_animationFlags & 0x20)
        {
            resetPoseScale(r4_pModel);
        }

        r4_pModel->mA_animationFlags &= ~0x38;
        r4_pModel->m30_pCurrentAnimation = NULL;

        return 1;
    }

    if (r4_pModel->m30_pCurrentAnimation == NULL)
    {
        r4_pModel->mA_animationFlags |= pAnimation->m0_flags;
        initModelDrawFunction(r4_pModel);
        return createDragonStateSubData1Sub1(r4_pModel, pAnimation);
    }

    if (r4_pModel->m30_pCurrentAnimation->m0_flags != pAnimation->m0_flags)
    {
        //060215EC
        r4_pModel->mA_animationFlags &= ~0x0038;
        r4_pModel->mA_animationFlags |= pAnimation->m0_flags;
        initModelDrawFunction(r4_pModel);
        return createDragonStateSubData1Sub1(r4_pModel, pAnimation);
    }

    //06021620
    r4_pModel->m30_pCurrentAnimation = pAnimation;
    r4_pModel->m10_currentAnimationFrame = 0;

    switch (pAnimation->m0_flags & 7)
    {
    case 1:
    case 4:
    case 5:
        for (s32 r7 = 0; r7 < r4_pModel->m12_numBones; r7++)
        {
            for(int j=0; j<9; j++)
            {
                r4_pModel->m2C_poseData[r7].m48[j].currentStep = 0;
                r4_pModel->m2C_poseData[r7].m48[j].delay = 0;
                r4_pModel->m2C_poseData[r7].m48[j].value = 0;
            }
        }
    default:
        break;
    }

    return 1;
}

void initModelDrawFunction(s_3dModel* pDragonStateData1)
{
    if (pDragonStateData1->m8 & 2)
    {
        if (pDragonStateData1->m40)
        {
            if (pDragonStateData1->m38_pColorAnim)
            {
                pDragonStateData1->m18_drawFunction = modelDrawFunction0;
            }
            else
            {
                pDragonStateData1->m18_drawFunction = modelDrawFunction1;
            }
        }
        else
        {
            if (pDragonStateData1->m38_pColorAnim)
            {
                pDragonStateData1->m18_drawFunction = modelDrawFunction2;
            }
            else
            {
                pDragonStateData1->m18_drawFunction = modelDrawFunction3;
            }
        }
    }
    else
    {
        if (pDragonStateData1->m40)
        {
            if (pDragonStateData1->mA_animationFlags & 0x20)
            {
                if (pDragonStateData1->m38_pColorAnim)
                {
                    assert(0);
                }
                else
                {
                    pDragonStateData1->m18_drawFunction = modelDrawFunction4;
                }
            }
            else
            {
                if (pDragonStateData1->m38_pColorAnim)
                {
                    pDragonStateData1->m18_drawFunction = modelDrawFunction5;
                }
                else
                {
                    pDragonStateData1->m18_drawFunction = modelDrawFunction6;
                }
            }
        }
        else
        {
            if (pDragonStateData1->mA_animationFlags & 0x20)
            {
                assert(0);
            }
            else
            {
                if (pDragonStateData1->m38_pColorAnim)
                {
                    pDragonStateData1->m18_drawFunction = modelDrawFunction9;
                }
                else
                {
                    pDragonStateData1->m18_drawFunction = modelDrawFunction10;
                }
            }

        }
    }
}

void countNumBonesInModel(s_3dModel* p3dModel, sModelHierarchy* pHierarchy)
{
    do
    {
        p3dModel->m12_numBones++;
        if (pHierarchy->m4_subNode)
        {
            countNumBonesInModel(p3dModel, pHierarchy->m4_subNode);
        }

        if (pHierarchy->m8_nextNode)
        {
            pHierarchy = pHierarchy->m8_nextNode;
        }
        else
        {
            break;
        }
    }while (true);
}

bool createDragonStateSubData1Sub2(s_3dModel* pDragonStateData1, sHotpointBundle* unkArg)
{
    pDragonStateData1->m40 = unkArg->getData(pDragonStateData1);

    pDragonStateData1->m44_hotpointData.resize(pDragonStateData1->m12_numBones);

    std::vector<s_hotpointDefinition>::iterator r12 = pDragonStateData1->m40->begin();

    for(u32 i=0; i<pDragonStateData1->m12_numBones; i++)
    {
        if (r12->m4_count > 0)
        {
            pDragonStateData1->m44_hotpointData[i].resize(r12->m4_count);
        }
        else
        {
            pDragonStateData1->m44_hotpointData[i].resize(0);
        }

        r12 ++;
    }

    return true;
}

bool init3DModelRawData(s_workArea* pWorkArea, s_3dModel* p3dModel, u32 animationFlags, s_fileBundle* pDragonBundle, u16 modelIndexOffset, sAnimationData* pAnimationData, sStaticPoseData* pDefaultPose, u8* colorAnim, sHotpointBundle* unkArg3)
{
    p3dModel->m0_pOwnerTask = pWorkArea;
    p3dModel->m4_pModelFile = pDragonBundle;
    p3dModel->mC_modelIndexOffset = modelIndexOffset;
    p3dModel->m34_pDefaultPose = pDefaultPose;
    p3dModel->m38_pColorAnim = colorAnim;
    p3dModel->m14 = 0;
    p3dModel->m16_previousAnimationFrame = 0;
    p3dModel->m8 = 1;

    if (pAnimationData)
    {
        p3dModel->mA_animationFlags = pAnimationData->m0_flags | animationFlags;
        p3dModel->m12_numBones = pAnimationData->m2_numBones;
    }
    else
    {
        p3dModel->mA_animationFlags = animationFlags;
        p3dModel->m12_numBones = 0;
        countNumBonesInModel(p3dModel, pDragonBundle->getModelHierarchy(p3dModel->mC_modelIndexOffset));
    }

    if (pDefaultPose)
    {
        assert(p3dModel->m12_numBones == pDefaultPose->m0_bones.size());
    }

    p3dModel->m2C_poseData.resize(p3dModel->m12_numBones);

    if (p3dModel->mA_animationFlags & 0x200)
    {
        p3dModel->m3C_boneMatrices.resize(p3dModel->m12_numBones);
        assert(p3dModel->m3C_boneMatrices.size());

        p3dModel->m8 |= 2;
    }
    else
    {
        p3dModel->m3C_boneMatrices.resize(0);
        p3dModel->m8 &= ~2;
    }

    if (unkArg3)
    {
        if (!createDragonStateSubData1Sub2(p3dModel, unkArg3))
            return false;
    }
    else
    {
        p3dModel->m40 = nullptr;
    }

    if (pAnimationData)
    {
        if (createDragonStateSubData1Sub1(p3dModel, pAnimationData) == 0)
            return false;
    }
    else
    {
        p3dModel->m30_pCurrentAnimation = NULL;
        p3dModel->m10_currentAnimationFrame = 0;

        copyPosePosition(p3dModel);
        copyPoseRotation(p3dModel);
        resetPoseScale(p3dModel);
    }

    initModelDrawFunction(p3dModel);

    p3dModel->m1C_addToDisplayListFunction = addObjectToDrawList;

    return true;
}

void updateDragonIfCursorChanged(u32 level)
{
    FunctionUnimplemented();
}

void loadRiderIfChanged(u32 rider)
{
    if (pRider1State->mC_riderType != rider)
    {
        assert(0);
    }
}

void loadRider2IfChanged(u32 rider)
{
    if (pRider2State->mC_riderType != rider)
    {
        assert(0);
    }
}

void freeRamResources(p_workArea)
{
    FunctionUnimplemented();
}

void vdp1FreeLastAllocation(p_workArea)
{
    FunctionUnimplemented();
}

void freeRamResource()
{
    PDS_unimplemented("Unimplemented freeRamResource");
}

u16 loadFnt(const char* filename)
{
    u32 fileSize = getFileSize(filename);
    u8* fileBuffer = (u8*)allocateHeap(fileSize);
    assert(fileBuffer);
    loadFile(filename, fileBuffer, 0);
    addToMemoryLayout(fileBuffer, 1);
    for (int i = 0; i < fileSize / 2; i++)
    {
        *(u16*)(fileBuffer + i * 2) = READ_BE_U16(fileBuffer + i * 2);
    }
    s32 index = resetVdp2StringsSub1((u16*)fileBuffer);
    assert(index != -1);

    pVdp2StringControl->m10 = filename;
    pVdp2StringControl->m15 |= 1;
    return index;
}

void unloadFnt()
{
    PDS_unimplemented("Unimplemented unloadFnt");
}

u8 fieldSubTaskVar0;

void setFieldSubTaskVar0(u32 value)
{
    fieldSubTaskVar0 = value;
}

void s_FieldSubTaskWorkArea::Init(s_FieldSubTaskWorkArea* pFieldSubTaskWorkArea)
{
    fieldTaskPtr->m8_pSubFieldData = pFieldSubTaskWorkArea;
    fieldTaskPtr->m8_pSubFieldData->m370_fieldDebuggerWho = 0;

    setFieldSubTaskVar0(1);

    if ((fieldTaskPtr->m2C_currentFieldIndex != 8) && (fieldTaskPtr->m2C_currentFieldIndex != 12))
    {
        resetTempAllocators();
    }

    g_fadeControls.m_48 = 0xC210;
    g_fadeControls.m_4A = 0xC210;

    if (gFieldOverlayFunction)
        gFieldOverlayFunction(pFieldSubTaskWorkArea, 0);
    else
    {
        PDS_unimplemented("Trying to call unimplemented overlay!");
        pFieldSubTaskWorkArea->getTask()->markFinished();
        fieldA3_1_checkExitsTaskUpdate2Sub1(0);
    }

    fieldTaskPtr->m3C_fieldTaskState = 4;

    if (fieldTaskPtr->m35)
    {
        fieldTaskPtr->m28_status = 0;
    }
    else
    {
        fieldTaskPtr->m28_status = 1;
    }
}

void* openFileListHead = NULL;

bool readKeyboardToggle()
{
    return false;
}

void s_FieldSubTaskWorkArea::Update(s_FieldSubTaskWorkArea* pFieldSubTaskWorkArea)
{
    mainGameState.setPackedBits(0, 2, 0);

    switch (pFieldSubTaskWorkArea->fieldSubTaskStatus)
    {
    case 0:
        if (openFileListHead == NULL)
        {
            if (!isSoundLoadingFinished())
            {
                g_fadeControls.m_4D = 6;

                if (g_fadeControls.m_4D >= g_fadeControls.m_4C)
                {
                    vdp2Controls.m20_registers[0].m112_CLOFSL = 0x10;
                    vdp2Controls.m20_registers[1].m112_CLOFSL = 0x10;
                }

                fadePalette(&g_fadeControls.m0_fade0, convertColorToU32ForFade(g_fadeControls.m0_fade0.m0_color), g_fadeControls.m_48, 30);
                fadePalette(&g_fadeControls.m24_fade1, convertColorToU32ForFade(g_fadeControls.m24_fade1.m0_color), g_fadeControls.m_4A, 30);

                pFieldSubTaskWorkArea->fieldSubTaskStatus++;
            }
        }
    default:
    case 1:
        mainGameState.gameStats.m70++;
        pFieldSubTaskWorkArea->m354++;

        if (readKeyboardToggle())
        {
            assert(0);
        }
        break;
    }
}

void s_FieldSubTaskWorkArea::Draw(s_FieldSubTaskWorkArea* pFieldSubTaskWorkArea)
{
    if (pFieldSubTaskWorkArea->m374_pUpdateFunction1)
    {
        pFieldSubTaskWorkArea->m374_pUpdateFunction1();
    }

    if (pFieldSubTaskWorkArea->pUpdateFunction2)
    {
        pFieldSubTaskWorkArea->pUpdateFunction2();
    }

}

void s_FieldSubTaskWorkArea::Delete(s_FieldSubTaskWorkArea* pFieldSubTaskWorkArea)
{
    if ((fieldTaskPtr->m2C_currentFieldIndex == 8) || (fieldTaskPtr->m2C_currentFieldIndex == 0xC))
    {
        freeRamResource();
        assert(0);
        //vdp1FreeLastAllocation()
    }

    fieldTaskPtr->m8_pSubFieldData = nullptr;
}

void createEncounterTask(s_workArea* pWorkArea)
{
    PDS_unimplemented("createEncounterTask");
}

void s_fieldStartOverlayTask::Init(s_fieldStartOverlayTask* pThis)
{
    const s_fieldDefinition* pFieldDefinition = &fieldDefinitions[fieldTaskPtr->m2C_currentFieldIndex];

    fieldTaskPtr->m4_overlayTaskData = pThis;

    createEncounterTask(pThis);

    if(pFieldDefinition->m_fnt)
    {
        loadFnt(pFieldDefinition->m_fnt);
    }

    if(pFieldDefinition->m_prg)
    {
        gFieldOverlayFunction = nullptr;
        if (!strcmp(pFieldDefinition->m_prg, "FLD_A3.PRG"))
        {
            gFieldOverlayFunction = overlayStart_FLD_A3;
        }
        else
        {
            PDS_unimplemented("Trying to load unimplemented overlay!");
        }
        //loadFile(pFieldDefinition->m_prg, NULL, 0);

        createSubTask<s_FieldSubTaskWorkArea>(pThis);
    }
}

void s_fieldStartOverlayTask::Delete(s_fieldStartOverlayTask* pThis)
{
    const s_fieldDefinition* pFieldDefinition = &fieldDefinitions[fieldTaskPtr->m2C_currentFieldIndex];

    if (pFieldDefinition->m_fnt)
    {
        unloadFnt();
    }

    fieldTaskPtr->m4_overlayTaskData = NULL;
}

p_workArea fieldTaskUpdateSub0(u32 fieldIndex, u32 subFieldIndex, u32 savePointIndex, u32 currentSubFieldIndex)
{
    fieldTaskPtr->m2C_currentFieldIndex = fieldIndex;
    fieldTaskPtr->m2E_currentSubFieldIndex = subFieldIndex;
    fieldTaskPtr->m30_fieldEntryPoint = savePointIndex;
    fieldTaskPtr->m32_previousSubField = currentSubFieldIndex;

    if (fieldTaskVar0 == NULL)
    {
        createSubTask<s_fieldSub0Task>(fieldTaskPtr);
    }
    else
    {
        if (fieldTaskPtr->m0 == NULL)
        {
            createSubTask<s_fieldSub1Task>(fieldTaskPtr);
        }
        else
        {
            createSubTask<s_FieldSubTaskWorkArea>(fieldTaskPtr->m4_overlayTaskData);
        }
    }

    return fieldTaskVar0;
}

void setupPlayer(u32 fieldIndex)
{
    if (fieldTaskPtr->updateDragonAndRiderOnInit)
    {
        const e_dragonLevel perFieldDragonLevel[] =
        {
            DR_LEVEL_0_BASIC_WING,
            DR_LEVEL_0_BASIC_WING,
            DR_LEVEL_0_BASIC_WING,
            DR_LEVEL_0_BASIC_WING,
            DR_LEVEL_1_VALIANT_WING,
            DR_LEVEL_1_VALIANT_WING,
            DR_LEVEL_1_VALIANT_WING,
            DR_LEVEL_1_VALIANT_WING,
            DR_LEVEL_2_STRIPE_WING,
            DR_LEVEL_2_STRIPE_WING,
            DR_LEVEL_5_ARM_WING,
            DR_LEVEL_3_PANZER_WING,
            DR_LEVEL_3_PANZER_WING,
            DR_LEVEL_3_PANZER_WING,
            DR_LEVEL_4_EYE_WING,
            DR_LEVEL_4_EYE_WING,
            DR_LEVEL_4_EYE_WING,
            DR_LEVEL_4_EYE_WING,
            DR_LEVEL_5_ARM_WING,
            DR_LEVEL_5_ARM_WING,
        };

        mainGameState.gameStats.m1_dragonLevel = perFieldDragonLevel[fieldIndex];
    }

    if (mainGameState.gameStats.m1_dragonLevel == 8)
    {
        assert(0);
    }

    //setup riders
    mainGameState.gameStats.m2_rider1 = 1; // edge is rider

    switch (fieldIndex)
    {
    case 3:
        mainGameState.gameStats.m3_rider2 = 2;
        break;
    case 18: // tower
        mainGameState.gameStats.m3_rider2 = 5;
        break;
    default:
        mainGameState.gameStats.m3_rider2 = 0;
        break;
    }

    resetTempAllocators();

    initDramAllocator(fieldTaskPtr, playerDataMemoryBuffer, sizeof(playerDataMemoryBuffer), NULL);

    switch (fieldTaskPtr->updateDragonAndRiderOnInit)
    {
    case 0:
        loadDragon(fieldTaskPtr);
        loadCurrentRider(fieldTaskPtr);
        loadCurrentRider2(fieldTaskPtr);
        fieldTaskPtr->updateDragonAndRiderOnInit = 2;
        break;
    case 1:
        updateDragonIfCursorChanged(mainGameState.gameStats.m1_dragonLevel);
        loadRiderIfChanged(mainGameState.gameStats.m2_rider1);
        loadRider2IfChanged(mainGameState.gameStats.m3_rider2);
        break;
    case 2:
        break;
    default:
        assert(false);
    }

    mainGameState.gameStats.m10_currentHP = mainGameState.gameStats.mB8_maxHP;
    mainGameState.gameStats.m14_currentBP = mainGameState.gameStats.mBA_maxBP;

    freeRamResource();
}

void s_fieldTaskWorkArea::fieldTaskUpdate(s_fieldTaskWorkArea* pWorkArea)
{
    switch (pWorkArea->m3C_fieldTaskState)
    {
    case 0:
        pauseEngine[2] = 0;
        fieldTaskVar3 = 0;
        fieldInputTaskWorkArea = createFieldInputTask(pWorkArea);
        pWorkArea->m3C_fieldTaskState++;
        break;
    case 1: //do nothing
        break;
    case 2: //start field
        setupPlayer(pWorkArea->m36_fieldIndexMenuSelection);
        vdp2DebugPrintSetPosition(3, 24);
        vdp2PrintStatus.m10_palette = 0xD000;
        drawLineLargeFont("LOADING...");
        pWorkArea->m3C_fieldTaskState++;
    case 3:
        fieldTaskUpdateSub0(pWorkArea->m36_fieldIndexMenuSelection, pWorkArea->m38_subFieldIndexMenuSelection, pWorkArea->m3A, pWorkArea->m2E_currentSubFieldIndex);

        if (pWorkArea->m8_pSubFieldData)
        {
            if (fieldInputTaskWorkArea)
            {
                fieldInputTaskWorkArea->getTask()->markFinished();
                fieldInputTaskWorkArea = NULL;
            }

            pauseEngine[2] = 1;
        }
        pWorkArea->m3C_fieldTaskState = 1;
        break;
    case 4:
        break;
    case 5:
        pauseEngine[2] = 0;
        pWorkArea->m3C_fieldTaskState++;
        break;
    case 6:
        if (pWorkArea->m8_pSubFieldData == NULL)
        {
            pWorkArea->m3C_fieldTaskState = 3;
        }
        break;
    case 7:
        if (fieldTaskVar0 == 0)
        {
            pWorkArea->m3C_fieldTaskState = 0;
        }
        break;
    default:
        assert(0);
        break;
    }
}
void s_fieldTaskWorkArea::fieldTaskDelete(s_fieldTaskWorkArea*)
{
    fieldTaskPtr = nullptr;
}

p_workArea createFieldTask(p_workArea pTypelessWorkArea, u32 arg)
{
    return createSubTaskWithArg<s_fieldTaskWorkArea, s32>(pTypelessWorkArea, arg);
}

struct s_fieldDebugTaskWorkArea : public s_workAreaTemplateWithArg<s_fieldDebugTaskWorkArea, s32>
{
    static const TypedTaskDefinition* getTownDebugTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &s_fieldDebugTaskWorkArea::townDebugTaskInit, NULL, &s_fieldDebugTaskWorkArea::genericTaskRestartGameWhenFinished, &s_fieldDebugTaskWorkArea::genericOptionMenuDelete};
        return &taskDefinition;
    }

    static const TypedTaskDefinition* getFieldDebugTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &s_fieldDebugTaskWorkArea::fieldDebugTaskInit, NULL, &s_fieldDebugTaskWorkArea::genericTaskRestartGameWhenFinished, &s_fieldDebugTaskWorkArea::genericOptionMenuDelete};
        return &taskDefinition;
    }

    static const TypedTaskDefinition* getBattleDebugTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &s_fieldDebugTaskWorkArea::battleDebugTaskInit, NULL, &s_fieldDebugTaskWorkArea::genericTaskRestartGameWhenFinished, &s_fieldDebugTaskWorkArea::genericOptionMenuDelete };
        return &taskDefinition;
    }

    static const TypedTaskDefinition* getExitMenuTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &s_fieldDebugTaskWorkArea::initExitMenuTask, NULL, &s_fieldDebugTaskWorkArea::genericTaskRestartGameWhenFinished, &s_fieldDebugTaskWorkArea::genericOptionMenuDelete};
        return &taskDefinition;
    }

    static void initExitMenuTask(s_fieldDebugTaskWorkArea* pThis, s32 menuID)
    {
        pauseEngine[2] = 0;

        pThis->m8 = createModuleManager(pThis, menuID);

        createSiblingTaskWithArg<s_flagEditTaskWorkArea, p_workArea>(pThis->m8, pThis->m8);

        fadePalette(&g_fadeControls.m0_fade0, 0x8000, 0x8000, 1);
        fadePalette(&g_fadeControls.m24_fade1, 0x8000, 0x8000, 1);
    }

    static void townDebugTaskInit(s_fieldDebugTaskWorkArea* pWorkArea, s32)
    {
        pauseEngine[2] = 0;

        initNewGameState();

        pWorkArea->m8 = createLocationTask(pWorkArea, 0);

        resetTempAllocators();
        initDramAllocator(pWorkArea->m8, playerDataMemoryBuffer, 0x28000, NULL);

        loadDragon(pWorkArea->m8);
        loadCurrentRider(pWorkArea->m8);
        loadCurrentRider2(pWorkArea->m8);
        freeRamResource();
        createMenuTask(pWorkArea->m8);
        createSiblingTaskWithArg<s_flagEditTaskWorkArea, p_workArea>(pWorkArea->m8, pWorkArea->m8);
    }

    static void fieldDebugTaskInit(s_fieldDebugTaskWorkArea* pThis, s32)
    {
        pauseEngine[2] = 0;

        initNewGameState();
        pThis->m8 = createFieldTask(pThis, 0);

        createBattleManager(pThis->m8, 1);
        createMenuTask(pThis->m8);
        createSiblingTaskWithArg<s_flagEditTaskWorkArea, p_workArea>(pThis->m8, pThis->m8);
    }

    static void battleDebugTaskInit(s_fieldDebugTaskWorkArea* pThis, s32)
    {
        pauseEngine[2] = 0;

        initNewGameState();

        pThis->m8 = createBattleManager(pThis, 0);
        createMenuTask(pThis->m8);
        createSiblingTaskWithArg<s_flagEditTaskWorkArea, p_workArea>(pThis->m8, pThis->m8);
    }

    static void genericTaskRestartGameWhenFinished(s_fieldDebugTaskWorkArea* pThis)
    {
        if ((pThis->m8 == NULL) || pThis->m8->getTask()->isFinished())
        {
            initialTaskStatus.m_pendingTask = startSegaLogoModule;
        }
    }

    static void genericOptionMenuDelete(s_fieldDebugTaskWorkArea* pThis)
    {
        FunctionUnimplemented();
    }

    p_workArea m8; // 8
    // size: 0xC
};

p_workArea createTownDebugTask(p_workArea pWorkArea)
{
    return createSubTaskWithArg<s_fieldDebugTaskWorkArea, s32>(pWorkArea, 0, s_fieldDebugTaskWorkArea::getTownDebugTaskDefinition());
}
p_workArea createFieldDebugTask(p_workArea pWorkArea)
{
    return createSubTaskWithArg<s_fieldDebugTaskWorkArea, s32>(pWorkArea, 0, s_fieldDebugTaskWorkArea::getFieldDebugTaskDefinition());
}
p_workArea createBattleDebugTask(p_workArea pWorkArea)
{
    return createSubTaskWithArg<s_fieldDebugTaskWorkArea, s32>(pWorkArea, 0, s_fieldDebugTaskWorkArea::getBattleDebugTaskDefinition());
}



p_workArea createSoundDebugTask(p_workArea)
{
    assert(0);
    return NULL;
}
p_workArea createMovieDebugTask(p_workArea)
{
    assert(0);
    return NULL;
}

struct s_menuGraphicsTask : public s_workAreaTemplateWithArg<s_menuGraphicsTask, p_workArea>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &s_menuGraphicsTask::Init, NULL, &s_menuGraphicsTask::Draw, &s_menuGraphicsTask::Delete};
        return &taskDefinition;
    }

    static void Init(s_menuGraphicsTask* pThis, p_workArea argument)
    {
        pThis->m4 = argument;
        graphicEngineStatus.m40AC.mC = pThis;
        graphicEngineStatus.m40AC.m1_isMenuAllowed = 0;
        graphicEngineStatus.m40AC.m3 = 0;
        graphicEngineStatus.m40AC.m0_menuId = 0;
        graphicEngineStatus.m40AC.m2 = 0;
        graphicEngineStatus.m40AC.m4 = 0;
        graphicEngineStatus.m40AC.m5 = 0;
        graphicEngineStatus.m40AC.m6 = 0;
        graphicEngineStatus.m40AC.m7 = 0;

        addToMemoryLayout(MENU_SCB, 0x14000);
        loadFile("MENU.SCB", MENU_SCB, 0);
        loadFile("MENU.CGB", getVdp1Pointer(0x25C10000), 0);
        graphicEngineStatus.m40AC.fontIndex = loadFnt("MENU.FNT");
    }

    static void Draw(s_menuGraphicsTask*);
    static void Delete(s_menuGraphicsTask*);

    u32 state; // 0
    p_workArea m4;
    p_workArea m8;
    u8 mC;
    u8 mD;
};

void menuGraphicsTaskDrawSub1()
{
    graphicEngineStatus.m40E4 = (s_graphicEngineStatus_40E4*)allocateHeap(sizeof(s_graphicEngineStatus_40E4));
    assert(graphicEngineStatus.m40E4);

    memcpy_dma(&graphicEngineStatus.m405C, &graphicEngineStatus.m40E4->m0, sizeof(s_graphicEngineStatus_405C));
    memcpy_dma(&vdp2Controls, &graphicEngineStatus.m40E4->m50, sizeof(sVdp2Controls));
    memcpy_dma(&g_fadeControls, &graphicEngineStatus.m40E4->m2B0, sizeof(sFadeControls));
    asyncDmaCopy(vdp2Palette, &graphicEngineStatus.m40E4->m300, 512, 0);

    u32 backScreenTableOffset = vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA & 0x7FFFF;
    graphicEngineStatus.m40E4->m400 = getVdp2VramU16(backScreenTableOffset);
    graphicEngineStatus.m40E4->m402 = pVdp2StringControl->m0_index;
}

sLayerConfig menuNBG01Setup[] =
{
    m2_CHCN,  0, // 16 colors
    m5_CHSZ,  1, // character size is 2 cells x 2 cells (16*16)
    m6_PNB,  1, // pattern data size is 1 word
    m7_CNSM,  0, // character number is 10 bit, flip
    m11_SCN, 12,
    m12_PLSZ, 0, // plane is 1H x 1V
    m40_CAOS, 6, // palette offset is 6 * 0x200 = 0xC00
    m0_END,
};

sLayerConfig menuNBG2Setup[] =
{
    m2_CHCN,  0, // 16 colors
    m5_CHSZ,  1, // character size is 2 cells x 2 cells (16*16)
    m6_PNB,  1, // pattern data size is 1 word
    m7_CNSM,  0, // character number is 10 bit, flip
    m11_SCN, 12,
    m12_PLSZ, 0, // plane is 1H x 1V
    m40_CAOS, 6, // palette offset is 6 * 0x200 = 0xC00
    m34_W0E, 1,
    m37_W0A, 1,
    m44_CCEN, 1,
    m0_END,
};

u8 menuTilesLayout[] = {
    0xF7,
    0x12,
    0x39,
    0x16,
    0xF5,
    0xFF,
    0x12,
    0x3A,
    0x12,
    0xFF,
    0x3B,
    0x12,
    0x3C,
    0x12,
    0x3D,
    0x12,
    0x3E,
    0x12,
    0xFF,
    0x3F,
    0x12,
    0x40,
    0x12,
    0x41,
    0x12,
    0x42,
    0x12,
    0xFF,
    0x43,
    0x12,
    0x44,
    0x12,
    0x45,
    0x12,
    0x46,
    0x12,
    0xFF,
    0x47,
    0x12,
    0x48,
    0x12,
    0x49,
    0x12,
    0x4A,
    0x12,
    0xFF,
    0x4B,
    0x12,
    0x4C,
    0x12,
    0x4D,
    0x12,
    0x4E,
    0x12,
    0xFF,
    0x4F,
    0x12,
    0x50,
    0x12,
    0x51,
    0x12,
    0x52,
    0x12,
    0xFF,
    0x53,
    0x12,
    0x54,
    0x12,
    0x55,
    0x12,
    0x56,
    0x12,
    0xFF,
    0x57,
    0x12,
    0x58,
    0x12,
    0x59,
    0x12,
    0x5A,
    0x12,
    0xFF,
    0x5B,
    0x12,
    0x5C,
    0x12,
    0x5D,
    0x12,
    0x5E,
    0x12,
    0xFF,
    0x5F,
    0x12,
    0x60,
    0x12,
    0x61,
    0x12,
    0x62,
    0x12,
    0xFF,
    0x63,
    0x12,
    0x64,
    0x12,
    0x65,
    0x12,
    0x66,
    0x12,
    0xFF,
    0x67,
    0x12,
    0x68,
    0x12,
    0x69,
    0x12,
    0x6A,
    0x12,
    0xF1,
    0x6B,
    0xC0,
    0x6C,
    0x12,
    0x6D,
    0xFF,
    0x12,
    0x6E,
    0x12,
    0x6F,
    0x12,
    0x70,
    0x12,
    0x71,
    0x87,
    0x12,
    0x72,
    0x16,
    0xC0,
    0x73,
    0xEF,
    0x12,
    0x74,
    0x16,
    0x38,
    0xF0,
    0xFF,
    0x23,
    0x12,
    0x75,
    0xBF,
    0x12,
    0x76,
    0x12,
    0x77,
    0x12,
    0x78,
    0x95,
    0xFF,
    0x1F,
    0x79,
    0x12,
    0x7A,
    0x12,
    0x7B,
    0xF4,
    0xC0,
    0x20,
    0xFE,
    0x25,
    0x65,
    0xFE,
    0x7C,
    0x12,
    0x7D,
    0xEF,
    0x12,
    0x7E,
    0x12,
    0x7F,
    0x95,
    0xFF,
    0x80,
    0x12,
    0xC5,
    0x81,
    0x00,
    0xFE,
    0x2D,
    0xCC,
    0x82,
    0xEF,
    0x12,
    0x83,
    0x12,
    0x84,
    0xA5,
    0xFF,
    0x85,
    0x12,
    0xF7,
    0x86,
    0x12,
    0x87,
    0x00,
    0xFE,
    0x2E,
    0x88,
    0x12,
    0x89,
    0xF8,
    0xFA,
    0x8A,
    0x12,
    0x8B,
    0x12,
    0x7F,
    0x8C,
    0x12,
    0x8D,
    0x12,
    0x8E,
    0x12,
    0x8F,
    0xFF,
    0x00,
    0xFE,
    0x2E,
    0x90,
    0x12,
    0x91,
    0x12,
    0x92,
    0x12,
    0x93,
    0xFF,
    0x12,
    0x94,
    0x12,
    0x95,
    0x12,
    0x96,
    0x12,
    0x97,
    0xFB,
    0x12,
    0x98,
    0x00,
    0xFE,
    0x2E,
    0x99,
    0x12,
    0x9A,
    0x12,
    0xFF,
    0x9B,
    0x12,
    0x9C,
    0x12,
    0x9D,
    0x12,
    0x9E,
    0x12,
    0xDF,
    0x9F,
    0x12,
    0xA0,
    0x12,
    0xA1,
    0x10,
    0xFE,
    0x2C,
    0xA2,
    0xBF,
    0x12,
    0xA3,
    0x12,
    0xA4,
    0x12,
    0xA5,
    0x60,
    0xFC,
    0x2D,
    0xFC,
    0xC6,
    0xA6,
    0x12,
    0xA7,
    0x12,
    0x5F,
    0xA8,
    0x12,
    0xA9,
    0x12,
    0xAA,
    0x20,
    0xFE,
    0x31,
    0xFE,
    0xC0,
    0xAB,
    0x12,
    0xAC,
    0x12,
    0xAD,
    0xF2,
    0x20,
    0xFE,
    0x35,
    0xC0,
    0xAE,
    0x12,
    0xB5,
    0xAF,
    0x10,
    0xFE,
    0x39,
    0xF0,
    0xFF,
    0x5B,
    0x12,
    0xF0,
    0xFF,
    0x7E,
    0x02,
    0x00,
    0x00,
};

u8 menuPalette[] = {
    0x80,   0,0x80, 0, 0x84,0x20, 0x88,0x41, 0x8C,0x62, 0x8C,0x63, 0x90,0x83, 0x8C,0x85,
    0x90,0xA6, 0x90,0xC6, 0x94,0xC8, 0x98,0xE9, 0xA5,0x6B, 0xAD,0xAD, 0xB5,0xEF, 0xD6,0xB5,
    0x80,   0,0x88, 0x42,0x98, 0xC6,0xA5, 0x29,0xA9, 0x4A,0xAD, 0x6B,0xB1, 0x8C,0xBD, 0xEF,
    0x90,0x84, 0xD6,0xB5, 0xA1,   8,0xB5, 0xAD,0xC6, 0x31,0xCA, 0x94,0xD7, 0x18,0xE3, 0x5A,
    0x80,   0,0x80, 0, 0x84,0x20, 0x88,0x41, 0x8C,0x63, 0x8C,0x85, 0x90,0xA6, 0x90,0xC6,
    0x80,0xEA, 0x94,0xC8, 0x81,0x71, 0x81,0xD4, 0x82,0x18, 0x86,0x7C, 0x8E,0xFD, 0x97,0x7E,
    0x80,   0,0x80, 0, 0x84,0x20, 0x88,0x41, 0x8C,0x63, 0x8C,0x85, 0x90,0xA6, 0x90,0xC6,
    0x94,0xA5, 0x94,0xC8, 0x98,0xC6, 0xA1,   8,0xA9, 0x4A,0xB1, 0x8C,0xB9, 0xCE,0xC2, 0x10,
    0x80,   0,0xA0, 0x61,0x94, 0x40,0x88, 0x40,0x94, 0xA0,0x9C, 0xE0,0xA5, 0x20,0xAD, 0x60,
    0xB5,0xA0, 0xB9,0xC0, 0xBD,0xE0, 0xC2,   0,0xC2, 0, 0xC6,0x20, 0xCA,0x40, 0xDA,0xC0,
    0x80,   0,0x80, 0, 0x88,0x21, 0x8C,0x63, 0x90,0x85, 0x98,0xC6, 0x9D,   8,0xA5, 0x4A,
    0xAD,0x8C, 0xB1,0xCE, 0xBA,0x10, 0xC2,0x52, 0xCA,0x94, 0xD2,0xF7, 0xD7,0x39, 0x94,   2,
    0x80,   0,0x80, 0, 0x84,0x20, 0x88,0x41, 0x8C,0x63, 0x8C,0x85, 0x90,0xA6, 0x90,0xC6,
    0x95,   8,0x94, 0xC8,0x9D, 0x4A,0xA5, 0x8C,0xAD, 0xCE,0xB6, 0x10,0xBE, 0x52,0xC6, 0x94,
    0x80,   0,0x80, 0, 0x84,0x21, 0x88,0x42, 0x8C,0x63, 0x90,0x84, 0x94,0xA5, 0x98,0xC6,
    0x9C,0xE7, 0xA1,   8,0xA5, 0x29,0xA9, 0x4A,0xAD, 0x6B,0xB1, 0x8C,0xB5, 0xAD,0xB9, 0xCE,
};

void setupVdp2ForMenu()
{
    reinitVdp2();

    vdp2Controls.m4_pendingVdp2Regs->m10_CYCA0 = 0x3FFF7FFF;
    vdp2Controls.m_isDirty = 1;
    vdp2Controls.m4_pendingVdp2Regs->m1C_CYCB1 = 0x12F456F;
    vdp2Controls.m_isDirty = 1;

    setupNBG0(menuNBG01Setup);
    setupNBG1(menuNBG01Setup);
    setupNBG2(menuNBG2Setup);

    initLayerMap(0, 0x71800, 0x71800, 0x71800, 0x71800);
    initLayerMap(1, 0x71000, 0x71000, 0x71000, 0x71000);
    initLayerMap(2, 0x70800, 0x70000, 0x70000, 0x70800);

    vdp2Controls.m4_pendingVdp2Regs->mF8_PRINA = 0x304;
    vdp2Controls.m4_pendingVdp2Regs->mFA_PRINB = 0x700;
    vdp2Controls.m4_pendingVdp2Regs->mFC_PRIR = 0x0;

    vdp2Controls.m4_pendingVdp2Regs->mF0_PRISA = 0x606;
    vdp2Controls.m4_pendingVdp2Regs->mF2_PRISB = 0x606;
    vdp2Controls.m4_pendingVdp2Regs->mF4_PRISC = 0x606;
    vdp2Controls.m4_pendingVdp2Regs->mF6_PRISD = 0x606;

    vdp2Controls.m_isDirty = 1;

    setVdp2VramU16(0x7FFFE, 0x9402);

    vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA = (vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA & 0xFFF80000) | 0x3FFFF;

    vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL &= 0xFEFF;

    loadFile("MENU.SCB", MENU_SCB, 0);
    loadFile("MENU.CGB", getVdp1Pointer(0x25C10000), 0);

    unpackGraphicsToVDP2(menuTilesLayout, getVdp2Vram(0x71800));

    {
        asyncDmaCopy(menuPalette, vdp2Palette, 256, 0);
    }
}

struct s_statusMenuTaskWorkArea : public s_workAreaTemplate<s_statusMenuTaskWorkArea>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &s_statusMenuTaskWorkArea::Init, NULL, &s_statusMenuTaskWorkArea::Draw, &s_statusMenuTaskWorkArea::Delete};
        return &taskDefinition;
    }

    static void Init(s_statusMenuTaskWorkArea*);
    static void Draw(s_statusMenuTaskWorkArea*);
    static void Delete(s_statusMenuTaskWorkArea*);
    u32 selectedMenu; //0
};

struct s_mainMenuWorkArea : public s_workAreaTemplate<s_mainMenuWorkArea>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &s_mainMenuWorkArea::Init, NULL, &s_mainMenuWorkArea::Draw, &s_mainMenuWorkArea::Delete};
        return &taskDefinition;
    }

    static void Init(s_mainMenuWorkArea*);
    static void Draw(s_mainMenuWorkArea*);
    static void Delete(s_mainMenuWorkArea*);

    u8 m0; //0
    s8 m1; //1
    s8 selectedMenu; //2
    s8 m3_menuButtonStates[5]; //3
    p_workArea m8; // 8
    s_statusMenuTaskWorkArea* mC; // C
    s_MenuCursorWorkArea* m10_cursorTask; // 10
};

void initVdp2ForStatusMenu()
{
    setVdp2LayerScroll(0, 160, 0);
    setVdp2LayerScroll(1, 0, 0x100);
    setVdp2LayerScroll(3, 0, 0x100);
    resetVdp2LayersAutoScroll();
    unpackGraphicsToVDP2(gCommonFile->m_data + 0xF7F4, getVdp2Vram(0x71400));
    setupVDP2StringRendering(0, 34, 44, 28);
    clearVdp2TextArea();
}

void switchVdp2MenuTileToHightlighted(u32 vdp2Offset, u32 r5, u32 r6, u32 r7)
{
    for (int i = 0; i < r6; i++)
    {
        u32 r14 = vdp2Offset;
        u32 r10 = r5;

        for (int j = 0; j < r10; j++)
        {
            u16 spriteIndex = getVdp2VramU16(r14) & 0xFFF;
            if (getVdp2VramU16(r14) != spriteIndex)
            {
                setVdp2VramU16(r14, spriteIndex | (r7 << 8));
            }
            r14 += 2;
        }

        vdp2Offset += 0x40;
    }
}

struct mainMenuTaskInitSub2TaskWorkArea : public s_workAreaTemplateWithArg<mainMenuTaskInitSub2TaskWorkArea, sMainMenuTaskInitData2*>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &mainMenuTaskInitSub2TaskWorkArea::Init, NULL, &mainMenuTaskInitSub2TaskWorkArea::Draw, NULL};
        return &taskDefinition;
    }

    static void Init(mainMenuTaskInitSub2TaskWorkArea*, sMainMenuTaskInitData2*);
    static void Draw(mainMenuTaskInitSub2TaskWorkArea*);

    u32 spriteIndex;
    s_graphicEngineStatus_40BC* m4;
    u16* spriteData;
    u32 mC;
};

void mainMenuTaskInitSub2TaskWorkArea::Init(mainMenuTaskInitSub2TaskWorkArea* pThis, sMainMenuTaskInitData2* typedArg)
{
    pThis->m4 = typedArg->m0;
    pThis->spriteData = typedArg->m4;
}

s_menuSprite spriteData1[] =
{
    {0x2118, 0x520, 0, 0},
    {0x20D8, 0x420, 4, 0},
};

extern u32 frameIndex;

s_vd1ExtendedCommand* getExtendedCommand(s_vdp1Command& vd1PacketStart)
{
    return &vd1PacketStart.ExtendedCommand;
}

s_vd1ExtendedCommand* createVdp1ExtendedCommand(s_vdp1Command& vd1PacketStart)
{
    s_vd1ExtendedCommand* pPacket = getExtendedCommand(vd1PacketStart);
    if (pPacket->frameIndex)
    {
        assert(pPacket->frameIndex != frameIndex);
    }

    memset(pPacket, 0, sizeof(s_vd1ExtendedCommand));

    pPacket->frameIndex = frameIndex;

    return pPacket;
}

s_vd1ExtendedCommand* fetchVdp1ExtendedCommand(s_vdp1Command& vd1PacketStart)
{
    s_vd1ExtendedCommand* pPacket = getExtendedCommand(vd1PacketStart);

    if (pPacket->frameIndex != frameIndex)
        return NULL;

    return pPacket;
}

void mainMenuTaskInitSub2TaskWorkArea::Draw(mainMenuTaskInitSub2TaskWorkArea* pWorkArea)
{
    u32 spriteColor;
    if (pWorkArea->mC)
    {
        spriteColor = 0x650;
    }
    else
    {
        spriteColor = 0x630;
    }

    s16 r5 = pWorkArea->spriteData[pWorkArea->spriteIndex * 2 + 0] - pWorkArea->m4->scrollX;
    s16 r6 = pWorkArea->spriteData[pWorkArea->spriteIndex * 2 + 1] - pWorkArea->m4->scrollY;

    drawMenuSprite(&spriteData1[pWorkArea->mC], r5, r6, spriteColor);
}

mainMenuTaskInitSub2TaskWorkArea* mainMenuTaskInitSub2(p_workArea typelessWorkArea, sMainMenuTaskInitData2* r5, u32 r6)
{
    mainMenuTaskInitSub2TaskWorkArea* pTypedNewTask = createSubTaskWithArg<mainMenuTaskInitSub2TaskWorkArea, sMainMenuTaskInitData2*>(typelessWorkArea, r5);
    pTypedNewTask->mC = r6;
    return pTypedNewTask;
}

u16 mainMenuTaskInitData2_[]{
    0x89, 0x11C,
    0xB0, 0x134,
    0x89, 0x14C,
    0xB0, 0x164,
    0x89, 0x17C,
    0x0
};

sMainMenuTaskInitData2 mainMenuTaskInitData2 = {
    &graphicEngineStatus.m40BC_layersConfig[1],
    mainMenuTaskInitData2_
};

struct s_menuDragonCrestTaskWorkArea : public s_workAreaTemplateWithArg<s_menuDragonCrestTaskWorkArea,s_graphicEngineStatus_40BC*>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &s_menuDragonCrestTaskWorkArea::Init, NULL, &s_menuDragonCrestTaskWorkArea::Draw, NULL};
        return &taskDefinition;
    }

    static void Init(s_menuDragonCrestTaskWorkArea* pThis, s_graphicEngineStatus_40BC* arg)
    {
        pThis->m4 = arg;
    }

    static void Draw(s_menuDragonCrestTaskWorkArea*);

    u32 m0;
    s_graphicEngineStatus_40BC* m4;
};

s_menuSprite menuDragonCrestSprites [] =
{
    {0x2168, 0x317, 0x178, 0xD},
    {0x56, 0x218C, 0x418, 0x188},
    {0x19, 0x57, 0x21BC, 0x419},
    {0x196, 0x27, 0x58, 0x21F0},
    {0x50D, 0x1A5, 0x20, 0x59},
    {0x2214, 0x51A, 0x1BE, 0x20},
    {0x5A, 0x2258, 0x611, 0x1BF},
    {0x30, 0x5B, 0x228C, 0x416},
    {0x1A5, 0x2A, 0x5C, 0x22B8},
    {0x313, 0x19C, 0x3C, 0x5D},
    {0x22D8, 0x30F, 0x1B0, 0x40},
    {0x5E, 0x22F0, 0x416, 0x1C3},
    {0x40, 0x5F, 0x231C, 0x611},
    {0x1A1, 0x4E, 0x60, 0x2350},
    {0x40E, 0x18E, 0x55, 0x61},
};

void s_menuDragonCrestTaskWorkArea::Draw(s_menuDragonCrestTaskWorkArea* pThis)
{
    if (graphicEngineStatus.m40AC.m5)
    {
        for (int i = 0; i < 10; i++)
        {
            PDS_unimplemented("Missing filter login in menuDragonCrestTaskDraw");

            drawMenuSprite(&menuDragonCrestSprites[i], -pThis->m4->scrollX, -pThis->m4->scrollY, 0x610);
        }
    }
}

struct s_laserRankTaskWorkArea : public s_workAreaTemplateWithArg<s_laserRankTaskWorkArea, s_graphicEngineStatus_40BC*>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &s_laserRankTaskWorkArea::Init, NULL, &s_laserRankTaskWorkArea::Draw, NULL };
        return &taskDefinition;
    }

    static void Init(s_laserRankTaskWorkArea* pThis, s_graphicEngineStatus_40BC* arg)
    {
        pThis->m4 = arg;
    }

    static void Draw(s_laserRankTaskWorkArea*);

    u32 m0;
    s_graphicEngineStatus_40BC* m4;
};

s_menuSprite laserRankSpriteDefinition[6] = {
    { 0x2378, 0x20B, 0x80, 0x19C },
    { 0x2384, 0x213, 0x8A, 0x1A3 },
    { 0x2398, 0x20B, 0x80, 0x1B2 },
    { 0x23A4, 0x20A, 0x70, 0x1B3 },
    { 0x23B0, 0x213, 0x6C, 0x1A3 },
    { 0x23C4, 0x20A, 0x70, 0x19C },
};

void s_laserRankTaskWorkArea::Draw(s_laserRankTaskWorkArea* pThis)
{
    if (pThis->m0 < 0)
        return;
    if (mainGameState.gameStats.m1_dragonLevel > 7)
        return;

    u8 r14 = mainGameState.gameStats.m1_dragonLevel;
    if (r14 > 6)
        r14 = 6;

    for (int i = 0; i < r14; i++)
    {
        drawMenuSprite(&laserRankSpriteDefinition[i], -pThis->m4->scrollX, -pThis->m4->scrollY, 0x760);
    }
}

struct s_mainMenuTaskInitSub4SubWorkArea : public s_workAreaTemplate<s_mainMenuTaskInitSub4SubWorkArea>
{
    u32 _0;
    s32 _4;
};

void mainMenuTaskInitSub4Sub(s_mainMenuTaskInitSub4SubWorkArea* pWorkArea)
{
    if (--pWorkArea->_4 > 0)
    {
        return;
    }

    pWorkArea->_4 = 4;

    if (++pWorkArea->_0 >= 14)
    {
        pWorkArea->_0 = 0;
    }

    asyncDmaCopy(gCommonFile->m_data + 0xDB9C + pWorkArea->_0 * 14, getVdp2Cram(0xE00 + 0xC4), 14, 0);
}

void mainMenuTaskInitSub4(p_workArea typelessWorkArea)
{
    createSubTaskFromFunction<s_mainMenuTaskInitSub4SubWorkArea>(typelessWorkArea, mainMenuTaskInitSub4Sub);
}

u32 mainMenuTaskInitData1[5] = {
    0x71450,
    0x714D6,
    0x71510,
    0x71596,
    0x715D0,
};

void s_mainMenuWorkArea::Init(s_mainMenuWorkArea* pWorkArea)
{
    pWorkArea->m3_menuButtonStates[0] = 1; // item is always enabled

    if (mainGameState.getBit(4, 2)) // dragon menu
    {
        pWorkArea->m3_menuButtonStates[1] = 1;
    }
    else
    {
        pWorkArea->m3_menuButtonStates[1] = -1;
    }

    if (mainGameState.gameStats.m20_XP) // defeated monsters menu
    {
        pWorkArea->m3_menuButtonStates[2] = 1;
    }
    else
    {
        pWorkArea->m3_menuButtonStates[2] = -1;
    }
    pWorkArea->m3_menuButtonStates[3] = 1; // map
    pWorkArea->m3_menuButtonStates[4] = 1; // setup

    initVdp2ForStatusMenu();

    s32 r14 = -1;
    for (int i = 0; i < 5; i++)
    {
        switch (pWorkArea->m3_menuButtonStates[i])
        {
        case 0:
            assert(0);
        case 1:
            if (r14 < 0)
            {
                r14 = 0;
                switchVdp2MenuTileToHightlighted(mainMenuTaskInitData1[i], 3, 3, 0x620);
            }
            else
            {
                switchVdp2MenuTileToHightlighted(mainMenuTaskInitData1[i], 3, 3, 0x660);
            }
            break;
        case -1:
            mainMenuTaskInitSub2(pWorkArea, &mainMenuTaskInitData2, 0)->spriteIndex = i;
            break;
        default:
            assert(0);
        }
    }

    pWorkArea->selectedMenu = r14;

    if (r14 >= 0)
    {
        pWorkArea->m10_cursorTask = createMenuCursorTask(pWorkArea, &mainMenuTaskInitData2);
        pWorkArea->m10_cursorTask->m0_selectedEntry = r14;
    }

    createSubTaskWithArg<s_menuDragonCrestTaskWorkArea,s_graphicEngineStatus_40BC*>(pWorkArea, &graphicEngineStatus.m40BC_layersConfig[0]);
    createSubTaskWithArg<s_laserRankTaskWorkArea, s_graphicEngineStatus_40BC*>(pWorkArea, &graphicEngineStatus.m40BC_layersConfig[1]);

    mainMenuTaskInitSub4(pWorkArea);

    pWorkArea->m1 = vblankData.m14_numVsyncPerFrame;

    vblankData.m14_numVsyncPerFrame = 1;

    fadePalette(&g_fadeControls.m0_fade0, 0xC210, 0xC210, 1);
    fadePalette(&g_fadeControls.m24_fade1, 0xC210, 0xC210, 1);
}

void s_statusMenuTaskWorkArea::Init(s_statusMenuTaskWorkArea*)
{
    setupVDP2StringRendering(0, 34, 44, 28);
}

const char* statusMenuOptionsDragonType[] = {
    "Show Item List                  ",
    "Select Dragon Type              ",
    "Display Data on Defeated Enemies",
    "Display Map                     ",
    "Change Settings                 ",
};

const char* statusMenuOptionsAbilities[] = {
    "Show Item List                  ",
    "See Ability                     ",
    "Display Data on Defeated Enemies",
    "Display Map                     ",
    "Change Settings                 ",
};

void s_statusMenuTaskWorkArea::Draw(s_statusMenuTaskWorkArea* pWorkArea)
{
    vdp2StringContext.m0 = 0;
    setActiveFont(graphicEngineStatus.m40AC.fontIndex);
    vdp2PrintStatus.m10_palette = 0xC000;

    const char** menuText = statusMenuOptionsDragonType;

    if ((mainGameState.gameStats.m1_dragonLevel == DR_LEVEL_0_BASIC_WING) || (mainGameState.gameStats.m1_dragonLevel == DR_LEVEL_6_LIGHT_WING) || (mainGameState.gameStats.m1_dragonLevel >= DR_LEVEL_7_SOLO_WING))
    {
        menuText = statusMenuOptionsAbilities;
    }

    if (pWorkArea->selectedMenu >= 0)
    {
        vdp2StringContext.m4_cursorX = vdp2StringContext.mC_X + 3;
        vdp2StringContext.m8_cursorY = vdp2StringContext.m10_Y + 0x17;

        drawObjectName(menuText[pWorkArea->selectedMenu]);
    }

    vdp2DebugPrintSetPosition(3, 39);
    vdp2PrintfLargeFont("HP  %4d/%4d", mainGameState.gameStats.m10_currentHP, mainGameState.gameStats.mB8_maxHP);

    vdp2DebugPrintSetPosition(3, 41);
    vdp2PrintfLargeFont("BP  %4d/%4d", mainGameState.gameStats.m14_currentBP, mainGameState.gameStats.mBA_maxBP);

    vdp2DebugPrintSetPosition(3, 49);
    vdp2PrintfLargeFont("DYNE %8d", mainGameState.gameStats.m38_dyne);

    if (mainGameState.gameStats.m1_dragonLevel < DR_LEVEL_8_FLOATER)
    {
        PDS_unimplemented("draw level curve");
    }
    else
    {
        vdp2StringContext.m4_cursorX = vdp2StringContext.mC_X + 3;
        vdp2StringContext.m8_cursorY = vdp2StringContext.m10_Y + 2;

        drawObjectName("  Floater ");
    }
}

void clearVdp2Menu()
{
    memset(getVdp2Vram(0x7000), 0, 0x100 * 0x10);
}

void s_statusMenuTaskWorkArea::Delete(s_statusMenuTaskWorkArea*)
{
    clearVdp2Menu();
}

void s_mainMenuWorkArea::Draw(s_mainMenuWorkArea* pWorkArea)
{
    switch (pWorkArea->m0)
    {
    case 0:
        pWorkArea->mC = createSubTask< s_statusMenuTaskWorkArea>(pWorkArea);
        pWorkArea->mC->selectedMenu = pWorkArea->selectedMenu;
        pWorkArea->m10_cursorTask->m0_selectedEntry = pWorkArea->selectedMenu;
        pWorkArea->m0++;
    case 1:
        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 1) // B
        {
            playSystemSoundEffect(1);
            fadePalette(&g_fadeControls.m0_fade0, 0, 0, 1);
            fadePalette(&g_fadeControls.m24_fade1, 0, 0, 1);
            if (pWorkArea)
            {
                pWorkArea->getTask()->markFinished();
            }
            return;
        }
        if (pWorkArea->selectedMenu < 0)
        {
            return;
        }
        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 6) // A or C
        {
            if (pWorkArea->m3_menuButtonStates[pWorkArea->selectedMenu] == 0)
            {
                playSystemSoundEffect(5);
                return;
            }
            playSystemSoundEffect(0);
            if (pWorkArea->mC)
            {
                pWorkArea->mC->getTask()->markFinished();
            }

            pWorkArea->m8 = statusMenuSubMenus[pWorkArea->selectedMenu](pWorkArea);
            pWorkArea->m0++;
            return;
        }
        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 0x30) // UP or DOWN
        {
            playSystemSoundEffect(10);
            
            s32 selectedMenu = pWorkArea->selectedMenu;

            if (pWorkArea->m3_menuButtonStates[selectedMenu] == 1)
            {
                switchVdp2MenuTileToHightlighted(mainMenuTaskInitData1[selectedMenu], 3, 3, 0x660);
            }

            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 0x10) // UP
            {
                do
                {
                    if (--selectedMenu < 0)
                    {
                        selectedMenu = 4;
                    }
                } while (pWorkArea->m3_menuButtonStates[selectedMenu] == -1);
            }
            else
            {
                do 
                {
                    if (++selectedMenu > 4)
                    {
                        selectedMenu = 0;
                    }
                } while (pWorkArea->m3_menuButtonStates[selectedMenu] == -1);
            }

            if (pWorkArea->m3_menuButtonStates[selectedMenu] == 1)
            {
                switchVdp2MenuTileToHightlighted(mainMenuTaskInitData1[selectedMenu], 3, 3, 0x620);
            }

            pWorkArea->selectedMenu = selectedMenu;
            pWorkArea->mC->selectedMenu = pWorkArea->selectedMenu;
            pWorkArea->m10_cursorTask->m0_selectedEntry = pWorkArea->selectedMenu;
        }
        break;
    case 2:
        if (pWorkArea->m8)
        {
            if (pWorkArea->m8->getTask()->isFinished())
            {
                pWorkArea->m0 = 0;
            }
        }
        break;
    default:
        assert(0);
        break;
    }
}

void s_mainMenuWorkArea::Delete(s_mainMenuWorkArea*)
{
    PDS_unimplemented("mainMenuTaskDelete");
}

p_workArea createMainMenuTask(p_workArea workArea)
{
    return createSubTask<s_mainMenuWorkArea>(workArea);
}

p_workArea createEnemyListMenuTask(p_workArea workArea)
{
    assert(0);
    return NULL;
}

p_workArea createMapTask(p_workArea workArea)
{
    assert(0);
    return NULL;
}

p_workArea createSystemMenuTask(p_workArea workArea)
{
    assert(0);
    return NULL;
}

p_workArea createMenuBKTask(p_workArea workArea)
{
    assert(0);
    return NULL;
}

p_workArea(*statusMenuSubMenus[])(p_workArea) =
{
    createInventoryMenuTask,
    createMainDragonMenuTask,
    createEnemyListMenuTask,
    createMapTask,
    createSystemMenuTask,
};

p_workArea(*menuTaskMenuArray[])(p_workArea) = 
{
    NULL,
    createMainMenuTask,
    createInventoryMenuTask,
    createMainDragonMenuTask,
    createEnemyListMenuTask,
    createMapTask,
    createSystemMenuTask,
    createLoadTask,
    createSaveTask,
    createMenuBKTask,
};

void menuGraphicsTaskDrawSub2()
{
    if (graphicEngineStatus.m40E4)
    {
        memcpy_dma(&graphicEngineStatus.m40E4->m0, &graphicEngineStatus.m405C, sizeof(s_graphicEngineStatus_405C));

        setActiveFont(graphicEngineStatus.m40E4->m402);
    }
}

void scrollMenu()
{
    for (int i = 0; i < 4; i++)
    {
        if (graphicEngineStatus.m40BC_layersConfig[i].m8_scrollFrameCount)
        {
            graphicEngineStatus.m40BC_layersConfig[i].scrollX += graphicEngineStatus.m40BC_layersConfig[i].scrollIncX;
            graphicEngineStatus.m40BC_layersConfig[i].scrollY += graphicEngineStatus.m40BC_layersConfig[i].scrollIncY;
            graphicEngineStatus.m40BC_layersConfig[i].m8_scrollFrameCount--;
        }
    }

    pauseEngine[4] = 0;
    setupVDP2CoordinatesIncrement2(graphicEngineStatus.m40BC_layersConfig[0].scrollX << 16, graphicEngineStatus.m40BC_layersConfig[0].scrollY << 16);

    pauseEngine[4] = 1;
    setupVDP2CoordinatesIncrement2(graphicEngineStatus.m40BC_layersConfig[1].scrollX << 16, graphicEngineStatus.m40BC_layersConfig[1].scrollY << 16);

    pauseEngine[4] = 2;
    setupVDP2CoordinatesIncrement2(graphicEngineStatus.m40BC_layersConfig[2].scrollX, graphicEngineStatus.m40BC_layersConfig[2].scrollY);

    pauseEngine[4] = 3;
    setupVDP2CoordinatesIncrement2(graphicEngineStatus.m40BC_layersConfig[3].scrollX, graphicEngineStatus.m40BC_layersConfig[3].scrollY);

    pauseEngine[4] = 4;
}

void menuGraphicsTaskDrawSub3()
{
    if (graphicEngineStatus.m40E4)
    {
        memcpy_dma(&graphicEngineStatus.m40E4->m50, &vdp2Controls, sizeof(sVdp2Controls));
        memcpy_dma(&graphicEngineStatus.m40E4->m2B0, &g_fadeControls, sizeof(sFadeControls));
        asyncDmaCopy(&graphicEngineStatus.m40E4->m300, vdp2Palette, 512, 0);

        u32 backScreenTableOffset = vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA & 0x7FFFF;
        setVdp2VramU16(backScreenTableOffset, graphicEngineStatus.m40E4->m400);

        u32 r3 = vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA & 0xFFF80000;
        u32 r2 = vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA & 0x7FFFF;
        vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA = r3 | ((r2 << 1) >> 1);
        freeHeap(graphicEngineStatus.m40E4);
    }
}

void s_menuGraphicsTask::Draw(s_menuGraphicsTask* pWorkArea)
{
    // not exactly that in the original code, but same idea
    if ((pWorkArea->m4 == NULL) || pWorkArea->m4->getTask()->isFinished())
    {
        pWorkArea->getTask()->markFinished();
        return;
    }

    switch (pWorkArea->state)
    {
    case 0:
        graphicEngineStatus.m40AC.m8 = 0;
        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 8)
        {
            if (graphicEngineStatus.m40AC.m1_isMenuAllowed)
            {
                graphicEngineStatus.m40AC.m0_menuId = 1;
                playSystemSoundEffect(0);
            }
            else
            {
                if (graphicEngineStatus.m40AC.m0_menuId == 0)
                    return;
            }
        }

        // enter menu
        // this isn't exactly correct
        if (graphicEngineStatus.m40AC.m0_menuId)
        {
            graphicEngineStatus.m40AC.m8 = 1;
            graphicEngineStatus.m40AC.m9 = 1;

            // pause the gameplay system
            pWorkArea->m4->getTask()->markPaused();

            menuGraphicsTaskDrawSub1();
            fadePalette(&g_fadeControls.m0_fade0, 0, 0, 1);
            fadePalette(&g_fadeControls.m24_fade1, 0, 0, 1);
            pWorkArea->state++;
        }
        break;
    case 1:
        if (fileInfoStruct.m2C_allocatedHead == NULL) // wait for loading to finish
        {
            graphicEngineStatus.m40AC.m2 = 0;
            setupVdp2ForMenu();

            pWorkArea->m8 = menuTaskMenuArray[graphicEngineStatus.m40AC.m0_menuId](pWorkArea);
            pWorkArea->state++;
        }
        break;
    case 2:
        if (pWorkArea->m8 && !pWorkArea->m8->getTask()->isFinished())
        {
            if (!graphicEngineStatus.m40AC.m4)
            {
                scrollMenu();
            }
            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 8) // start
            {
                if (graphicEngineStatus.m40AC.m0_menuId == 7)
                {
                    return;
                }
                if (graphicEngineStatus.m40AC.m3)
                {
                    playSystemSoundEffect(5);
                    return;
                }
            }
            if (graphicEngineStatus.m40AC.m2 == 0)
            {
                return;
            }
            playSystemSoundEffect(4);
            fadePalette(&g_fadeControls.m0_fade0, 0, 0, 1);
            fadePalette(&g_fadeControls.m24_fade1, 0, 0, 1);
            pWorkArea->state++;
        }
        else
        {
            menuGraphicsTaskDrawSub2();

            graphicEngineStatus.m40AC.m0_menuId = 0;
            pWorkArea->mC = pauseEngine[0];

            pWorkArea->m4->getTask()->clearPaused();

            pWorkArea->mD = graphicEngineStatus.m40AC.m9;
            pWorkArea->state = 5;
        }
        break;
    case 3:
        if (pWorkArea->m8)
        {
            pWorkArea->m8->getTask()->markFinished();
        }
        pWorkArea->state++;
        break;
    case 4:
        menuGraphicsTaskDrawSub2();

        graphicEngineStatus.m40AC.m0_menuId = 0;
        pWorkArea->mC = pauseEngine[0];

        pWorkArea->m4->getTask()->clearPaused();

        pWorkArea->mD = graphicEngineStatus.m40AC.m9;
        pWorkArea->state = 5;
        break;
    case 5:
        if (--pWorkArea->mD == 0)
        {
            if (pWorkArea->mC)
            {
                pauseEngine[0] = 1;
            }
            else
            {
                pauseEngine[0] = 0;
            }
            menuGraphicsTaskDrawSub3();

            graphicEngineStatus.m40AC.m8 = 2;
            pWorkArea->state = 0;
        }
        break;
    default:
        assert(0);
    }
}

void s_menuGraphicsTask::Delete(s_menuGraphicsTask*)
{
    FunctionUnimplemented();
}

p_workArea createMenuTask(p_workArea parentTask)
{
    return createSiblingTaskWithArg<s_menuGraphicsTask, p_workArea>(parentTask, parentTask);
}

s_vblankData vblankData;

p_workArea loadField(p_workArea r4, s32 r5)
{
    if (gGameStatus.m4_gameStatus == 105)
    {
        assert(0);
    }

    if ((gGameStatus.m4_gameStatus != 22) && (gGameStatus.m4_gameStatus != 37))
    {
        mainGameState.setPackedBits(135, 6, gGameStatus.m4_gameStatus - 0x50);
    }

    if (gGameStatus.m3_loadingSaveFile == 1)
    {
        return fieldTaskUpdateSub0(gSaveGameStatus.m9_fieldIndex, gSaveGameStatus.mA_subFieldIndex, gSaveGameStatus.mB_entryPointIndex, -1);
    }
    else
    {
        return fieldTaskUpdateSub0(r5, 0, 0, -1);
    }
}

p_workArea createNewGameTask(p_workArea pWorkArea)
{
    return createSubTaskWithArg<s_fieldDebugTaskWorkArea, s32>(pWorkArea, 0, s_fieldDebugTaskWorkArea::getExitMenuTaskDefinition());
}

p_workArea createContinueTask(p_workArea pWorkArea)
{
    return createSubTaskWithArg<s_fieldDebugTaskWorkArea, s32>(pWorkArea, 1, s_fieldDebugTaskWorkArea::getExitMenuTaskDefinition());
}

p_workArea createTutorial1Task(p_workArea pWorkArea)
{
    return createSubTaskWithArg<s_fieldDebugTaskWorkArea, s32>(pWorkArea, 2, s_fieldDebugTaskWorkArea::getExitMenuTaskDefinition());
}

p_workArea createTutorial2Task(p_workArea pWorkArea)
{
    return createSubTaskWithArg<s_fieldDebugTaskWorkArea, s32>(pWorkArea, 3, s_fieldDebugTaskWorkArea::getExitMenuTaskDefinition());
}

void playAnimationGenericSub1Sub0(sModelHierarchy* pModelData, std::vector<sPoseDataInterpolation>::iterator& r14)
{
    do
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&r14->m0_translation);
        rotateCurrentMatrixZYX(&r14->mC_rotation);
        scaleCurrentMatrixRow0(r14->m18_scale[0]);
        scaleCurrentMatrixRow1(r14->m18_scale[1]);
        scaleCurrentMatrixRow2(r14->m18_scale[2]);

        if (pModelData->m0_3dModel)
        {
            addObjectToDrawList(pModelData->m0_3dModel);
        }

        if (pModelData->m4_subNode)
        {
            // next matrix
            r14++;
            playAnimationGenericSub1Sub0(pModelData->m4_subNode, r14);
        }

        popMatrix();

        // End of model
        if (pModelData->m8_nextNode == nullptr)
        {
            return;
        }

        // next matrix
        r14++;
        pModelData = pModelData->m8_nextNode;
    } while (1);
}

void playAnimationGenericSub0Sub0(sModelHierarchy* pModelData, std::vector<sPoseDataInterpolation>::iterator& r14, std::vector<s_hotpointDefinition>::iterator& r6, std::vector<std::vector<sVec3_FP>>::iterator& r7)
{
    do
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&r14->m0_translation);
        rotateCurrentMatrixZYX(&r14->mC_rotation);
        scaleCurrentMatrixRow0(r14->m18_scale[0]);
        scaleCurrentMatrixRow1(r14->m18_scale[1]);
        scaleCurrentMatrixRow2(r14->m18_scale[2]);

        if (pModelData->m0_3dModel)
        {
            addObjectToDrawList(pModelData->m0_3dModel);
        }

        for (u32 i = 0; i < r6->m4_count; i++)
        {
            sVec3_FP input = r6->m0[i].m4;
            sVec3_FP& output = (*r7)[i];
            transformAndAddVecByCurrentMatrix(&input, &output);
        }

        if (pModelData->m4_subNode)
        {
            // next matrix
            r14++;
            // next bone stuff
            r6++;
            // next ???
            r7++;

            playAnimationGenericSub0Sub0(pModelData->m4_subNode, r14, r6, r7);
        }

        popMatrix();

        // End of model
        if (pModelData->m8_nextNode == nullptr)
        {
            return;
        }

        // next matrix
        r14++;
        // next bone stuff
        r6++;
        // next ???
        r7++;
        pModelData = pModelData->m8_nextNode;

    } while (1);
}

void playAnimationGenericSub0(s_3dModel* pModel)
{
    sModelHierarchy* r4 = pModel->m4_pModelFile->getModelHierarchy(pModel->mC_modelIndexOffset);

    if (pModel->m8 & 1)
    {
        std::vector<sPoseDataInterpolation>::iterator r5 = pModel->m48_poseDataInterpolation.begin();
        std::vector<s_hotpointDefinition>::iterator r6 = pModel->m40->begin();
        std::vector<std::vector<sVec3_FP>>::iterator r7 = pModel->m44_hotpointData.begin();
        playAnimationGenericSub0Sub0(r4, r5, r6, r7);
    }
    else
    {
        assert(0);
        //playAnimationGenericSub0Sub1(pModel->m4_pModelFile, r4, pModel->m48_poseDataInterpolation, pModel->m40, pModel->m44);
    }
}

void playAnimationGenericSub1(s_3dModel* pModel)
{
    if (pModel->m8 & 1)
    {
        sModelHierarchy* r4 = pModel->m4_pModelFile->getModelHierarchy(pModel->mC_modelIndexOffset);
        std::vector<sPoseDataInterpolation>::iterator r5 = pModel->m48_poseDataInterpolation.begin();
        playAnimationGenericSub1Sub0(r4, r5);
    }
}

void playAnimationGeneric(s_3dModel* pModel, sAnimationData* pAnimation, s32 interpolationLength)
{
    if (setupPoseInterpolation(pModel, interpolationLength) && !(pModel->mA_animationFlags & 0x200))
    {
        initAnimation(pModel, pAnimation);

        if (pModel->m40)
        {
            pModel->m18_drawFunction = playAnimationGenericSub0;
        }
        else
        {
            pModel->m18_drawFunction = playAnimationGenericSub1;
        }
    }
    else
    {
        initAnimation(pModel, pAnimation);
    }
}

