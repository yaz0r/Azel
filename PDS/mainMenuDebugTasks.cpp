#include "PDS.h"

extern p_workArea(*statusMenuSubMenus[])(p_workArea);

struct sLoadingTaskWorkArea : public s_workArea
{
    s16 m0_status;
    s16 m2;
    s16 m4;
    s16 m6;
    s16 m8;
    s16 mA;
    s8 mC;
    s32 m10;
};

sLoadingTaskWorkArea* gLoadingTaskWorkArea = NULL;

s16 loadingTaskVar0 = 0x1D;

void loadingTaskInit(s_workArea* pTypelessWorkArea, void* r5)
{
    sLoadingTaskWorkArea* pWorkArea = static_cast<sLoadingTaskWorkArea*>(pTypelessWorkArea);
    s8 arg = (s8)r5;

    gLoadingTaskWorkArea = pWorkArea;

    pWorkArea->m0_status = 0;
    pWorkArea->m2 = -1;
    pWorkArea->m4 = -1;
    pWorkArea->m6 = 0;
    pWorkArea->m8 = 0;
    pWorkArea->mA = 0;
    pWorkArea->mC = arg;

    if (arg)
    {
        s32 r6 = 0x3E8;
        s32 r4 = r6;
        do
        {
            if (r4 >= r6)
            {
                mainGameState.clearBit(0xC3E + r4);
            }
            else
            {
                mainGameState.clearBit(r4);
            }

            /*
            s32 r3;
            if (r4 >= r6)
            {
                r3 = 0xC3E + r4;
            }
            else
            {
                r3 = r4;
            }

            r3 = r3 / 8;
            s32 r0;
            if (r4 >= r6)
            {
                r0 = 0xC3E + r4;
            }
            else
            {
                r0 = r4;
            }

            r0 &= 7;

            u8 r1 = reverseBitMasks[r0];

            mainGameState.bitField[r3] &= r1;*/
        } while (--r4);
    }
    else
    {
        assert(0);
    }
}

void loadingTaskUpdateSub0(sLoadingTaskWorkArea* pWorkArea)
{
    if (pWorkArea->mC)
        return;

    assert(0);
}

void loadingTaskUpdate(p_workArea pTypelessWorkArea)
{
    sLoadingTaskWorkArea* pWorkArea = static_cast<sLoadingTaskWorkArea*>(pTypelessWorkArea);

    switch (pWorkArea->m0_status)
    {
    case 0:
        loadingTaskUpdateSub0(pWorkArea);
        pWorkArea->m0_status++;
        break;
    case 1:
        break;
    default:
        assert(0);
    }
}

void loadingTaskDelete(p_workArea pTypelessWorkArea)
{
    sLoadingTaskWorkArea* pWorkArea = static_cast<sLoadingTaskWorkArea*>(pTypelessWorkArea);

    unimplemented("loadingTaskDelete");
}

s_taskDefinitionWithArg loadingTaskDefinition = { loadingTaskInit, loadingTaskUpdate, NULL, loadingTaskDelete, "loadingTask" };

p_workArea createLoadingTask(p_workArea parentTask, s8 arg)
{
    return createSiblingTaskWithArg(parentTask, &loadingTaskDefinition, new sLoadingTaskWorkArea, (void*)arg);
}

struct s_flagEditTaskWorkArea : public s_workArea
{
    u32 state;
    p_workArea field_4; //4
};

void flagEditTaskInit(s_workArea* pTypelessWorkArea, void* argument)
{
    s_flagEditTaskWorkArea* pWorkArea = static_cast<s_flagEditTaskWorkArea*>(pTypelessWorkArea);

    pWorkArea->field_4 = static_cast<p_workArea>(argument);
}

void flagEditTaskUpdate(s_workArea* pTypelessWorkArea)
{
    s_flagEditTaskWorkArea* pWorkArea = static_cast<s_flagEditTaskWorkArea*>(pTypelessWorkArea);

    switch (pWorkArea->state)
    {
    case 0:
        if (readKeyboardToggle(0x85))
        {
            assert(0);
        }
        break;
    default:
        assert(0);
    }

    if (pWorkArea->field_4)
    {
        if (pWorkArea->field_4->getTask()->isFinished())
        {
            pWorkArea->getTask()->markFinished();
        }
    }
}

s_taskDefinitionWithArg flagEditTask = { flagEditTaskInit, NULL, flagEditTaskUpdate, NULL, "flagEditTask" };

u32 getPanzerZweiPlayTime(u32 slot)
{
    return 0;
}

void computeDragonSprAndAglFromCursor()
{
    u32 r6 = mainGameState.gameStats.field_18 + 1;

    switch (mainGameState.gameStats.m1_dragonLevel)
    {
    case DR_LEVEL_0_BASIC_WING:
    case DR_LEVEL_6_LIGHT_WING:
    case DR_LEVEL_8_FLOATER:
        mainGameState.gameStats.dragonCursorX = 0;
        mainGameState.gameStats.dragonCursorY = 0;
        break;
    default:
        assert(0);
        break;
    }
}

void updateDragonStatsFromLevel()
{
    s_gameStats& gameStats = mainGameState.gameStats;

    if (gameStats.m1_dragonLevel < DR_LEVEL_8_FLOATER)
    {
        gameStats.maxHP = gameStats.classMaxHP + dragonPerLevelMaxHPBP[gameStats.m1_dragonLevel].maxHP;
        gameStats.maxBP = gameStats.classMaxBP + dragonPerLevelMaxHPBP[gameStats.m1_dragonLevel].maxBP;
    }
    else
    {
        gameStats.maxHP = dragonPerLevelMaxHPBP[gameStats.m1_dragonLevel].maxHP;
        gameStats.maxBP = dragonPerLevelMaxHPBP[gameStats.m1_dragonLevel].maxBP;
    }

    if (gameStats.currentHP > gameStats.maxHP)
        gameStats.currentHP = gameStats.maxHP;

    if (gameStats.currentBP > gameStats.maxBP)
        gameStats.currentBP = gameStats.maxBP;

    computeDragonSprAndAglFromCursor();
}

void rotl(u32& value)
{
    u32 bit = value & 0x80000000;
    value <<= 1;
    value |= (bit >> 31);
}

void setPackedBits(u8* bitField, u32 firstBitOffset, u32 numBits, u32 value)
{
    u32 startOfByteInBits = (firstBitOffset & 0xFFFFFFE0);
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

void initNewGameState()
{
    mainGameState.reset();
    mainGameState.setPackedBits(2, 3, 0);

    if (VDP2Regs_.TVSTAT & 1)
    {
        mainGameState.setBit(5);
    }
    else
    {
        mainGameState.clearBit(5);
    }

    mainGameState.clearBit(6);
    mainGameState.setBit(0x2B * 8 + 2);
    mainGameState.setBit(0x2D * 8 + 5);

    mainGameState.gameStats.field_18 = 400;

    updateDragonStatsFromLevel();

    mainGameState.gameStats.currentHP = mainGameState.gameStats.maxHP;
    mainGameState.gameStats.currentBP = mainGameState.gameStats.maxBP;

    strcpy(mainGameState.gameStats.dragonName, "Dragon");

    u32 zweiPlayTime = getPanzerZweiPlayTime(0);
    u32 zweiPlayTimeSlot1 = getPanzerZweiPlayTime(1);
    if (zweiPlayTimeSlot1 > zweiPlayTime)
    {
        zweiPlayTime = zweiPlayTimeSlot1;
    }

    if (zweiPlayTime)
    {
        assert(0);
    }
}

p_workArea fieldTaskVar0;
p_workArea fieldInputTaskWorkArea;
u32 fieldTaskVar2;
u8 fieldTaskVar3;

s_fieldTaskWorkArea* getFieldTaskPtr()
{
    return fieldTaskPtr;
}

void updateFieldTaskNoBattleOverride(p_workArea pTypelessWorkArea)
{
    s_fieldTaskWorkArea* pWorkArea = static_cast<s_fieldTaskWorkArea*>(pTypelessWorkArea);

    switch (pWorkArea->fieldTaskState)
    {
    case 0:
        break;
    case 4:
        return;
    default:
        assert(0);
        break;
    }

    return;
}

void fieldTaskInit(p_workArea pTypelessWorkArea, void* battleArgumentVoid)
{
    s_fieldTaskWorkArea* pWorkArea = static_cast<s_fieldTaskWorkArea*>(pTypelessWorkArea);
    u32 battleArgument = (u32)battleArgumentVoid;

    fieldTaskPtr = pWorkArea;
    fieldTaskVar0 = NULL;
    fieldInputTaskWorkArea = NULL;
    fieldTaskVar2 = 0;

    pWorkArea->field_35 = battleArgument;
    pWorkArea->field_3D = -1;

    if (battleArgument)
    {
        pWorkArea->m28_status = 0;
        pWorkArea->getTask()->m_pUpdate = updateFieldTaskNoBattleOverride;
    }
    else
    {
        pWorkArea->m28_status = 1;
    }
}

struct s_fieldDebugListWorkArea : public s_workArea
{
    u32 m0_ticks; //0
    u32 m4_selectedSubField; // 4
    u32 m8_isSelectingSubfield; // 8
};

u32 performModulo(u32 r0, u32 r1)
{
    assert(r0);

    return r1 % r0;
}

void fieldDebugListTaskInit(p_workArea pTypelessWorkArea)
{
    s_fieldDebugListWorkArea* pWorkArea = static_cast<s_fieldDebugListWorkArea*>(pTypelessWorkArea);

    s_fieldTaskWorkArea* pFieldTask = getFieldTaskPtr();

    pFieldTask->fieldIndexMenuSelection = pFieldTask->m2C_currentFieldIndex;
    pFieldTask->subFieldIndexMenuSelection = pFieldTask->m2E_currentSubFieldIndex;
    pFieldTask->field_3A = pFieldTask->field_30;
    pFieldTask->m2E_currentSubFieldIndex = -1;

    if (pFieldTask->fieldIndexMenuSelection < 0)
    {
        pFieldTask->fieldIndexMenuSelection = 0;
    }

    while (!fieldEnabledTable[pFieldTask->fieldIndexMenuSelection])
    {
        pFieldTask->fieldIndexMenuSelection++;
    }

    reinitVdp2();
    
    vdp2Controls.m_pendingVdp2Regs->CYCA0 = 0x3FFF7FFF;
    vdp2Controls.m_isDirty = true;

    vdp2Controls.m_pendingVdp2Regs->PRINB = (vdp2Controls.m_pendingVdp2Regs->PRINB & 0xF8FF) | 0x700;
    vdp2Controls.m_isDirty = true;

    if (menuUnk0.m_4D >= menuUnk0.m_4C)
    {
        vdp2Controls.m_registers[0].N1COSL = 0x10;
        vdp2Controls.m_registers[1].N1COSL = 0x10;
    }
    
    fadePalette(&menuUnk0.m_field0, 0xC210, 0xC210, 1);
    fadePalette(&menuUnk0.m_field24, 0xC210, 0xC210, 1);
}

void setOpenMenu7()
{
    graphicEngineStatus.field_40AC.m0_menuId = 7;
}

void fieldDebugListTaskUpdate(p_workArea pTypelessWorkArea)
{
    s_fieldDebugListWorkArea* pWorkArea = static_cast<s_fieldDebugListWorkArea*>(pTypelessWorkArea);

    pWorkArea->m0_ticks++;
    s_fieldTaskWorkArea*r14 = getFieldTaskPtr();

    u32 var_24 = performModulo(30, pWorkArea->m0_ticks);

    if (pWorkArea->m8_isSelectingSubfield)
    {

        if (graphicEngineStatus.m4514.m0[0].m0_current.m8_newButtonDown & 0x40)
        {
            if (pWorkArea->m4_selectedSubField == 0)
            {
                pWorkArea->m8_isSelectingSubfield = 0;
            }
        }

        if (graphicEngineStatus.m4514.m0[0].m0_current.mC_newButtonDown2 & 0x20) // down
        {
            if (++r14->subFieldIndexMenuSelection >= fieldDefinitions[r14->fieldIndexMenuSelection].m_numSubFields)
            {
                r14->subFieldIndexMenuSelection = 0;
            }
        }

        if (graphicEngineStatus.m4514.m0[0].m0_current.mC_newButtonDown2 & 0x10) // up
        {
            if (--r14->subFieldIndexMenuSelection < 0)
            {
                r14->subFieldIndexMenuSelection = fieldDefinitions[r14->fieldIndexMenuSelection].m_numSubFields - 1;
            }
        }

        if (graphicEngineStatus.m4514.m0[0].m0_current.mC_newButtonDown2 & 0x80) // page down
        {
            unimplemented("Page down");
        }

        if (graphicEngineStatus.m4514.m0[0].m0_current.mC_newButtonDown2 & 0x40) // page up
        {
            unimplemented("Page up");
        }
    }
    else
    {
        if (graphicEngineStatus.m4514.m0[0].m0_current.m8_newButtonDown & 0x80)
        {
            pWorkArea->m8_isSelectingSubfield = 1;
        }

        if (graphicEngineStatus.m4514.m0[0].m0_current.mC_newButtonDown2 & 0x20) // down
        {
            clearVdp2StringFieldDebugList();
            r14->subFieldIndexMenuSelection = 0;

            do
            {
                r14->fieldIndexMenuSelection++;
                if (r14->fieldIndexMenuSelection >= 23)
                {
                    r14->fieldIndexMenuSelection = 0;
                }
            } while (!fieldEnabledTable[r14->fieldIndexMenuSelection]);
        }

        if (graphicEngineStatus.m4514.m0[0].m0_current.mC_newButtonDown2 & 0x10) // up
        {
            clearVdp2StringFieldDebugList();
            r14->subFieldIndexMenuSelection = 0;

            do
            {
                r14->fieldIndexMenuSelection--;
                if (r14->fieldIndexMenuSelection < 0)
                {
                    r14->fieldIndexMenuSelection = 22;
                }
            } while (!fieldEnabledTable[r14->fieldIndexMenuSelection]);
        }
    }

    if (graphicEngineStatus.m4514.m0[0].m0_current.mC_newButtonDown2 & 0x8000)
    {
        assert(0);
    }

    if (graphicEngineStatus.m4514.m0[0].m0_current.mC_newButtonDown2 & 0x800)
    {
        assert(0);
    }

    u32 r8 = 3;
    for (u32 r12 = 0; r12 < 23; r12++)
    {
        vdp2DebugPrintSetPosition(0xA, r8);
        if (r14->fieldIndexMenuSelection == r12) // is this the selected field?
        {
            vdp2PrintStatus.palette = 0x8000;
        }
        else
        {
            u16 selectedColor[] = { 0xD, 0xD };
            assert(pWorkArea->m8_isSelectingSubfield >= 0);
            assert(pWorkArea->m8_isSelectingSubfield <= 1);
            vdp2PrintStatus.palette = selectedColor[pWorkArea->m8_isSelectingSubfield] << 12;
        }

        if (fieldEnabledTable[r12])
        {
            drawLineSmallFont(fieldDefinitions[r12].m_name);

            // if selecting field
            if (pWorkArea->m8_isSelectingSubfield == 0)
            {
                if (r14->fieldIndexMenuSelection == r12)
                {
                    vdp2PrintStatus.palette = 0xD000;
                    vdp2DebugPrintSetPosition(0xA, r8);
                    drawLineSmallFont("\x7F");
                }
            }

            r8++;
        }
    }

    u32 var_2C = 0;

    u32 numSubFields = fieldDefinitions[r14->fieldIndexMenuSelection].m_numSubFields;
    const char** subFields = fieldDefinitions[r14->fieldIndexMenuSelection].m_subFields;

    s16 r2 = r14->subFieldIndexMenuSelection;
    if (r2 < 0)
    {
        r2 += 0xF;
    }

    pWorkArea->m4_selectedSubField = (r2 >> 4) << 4;

    clearVdp2StringFieldDebugList();

    u32 r12 = pWorkArea->m4_selectedSubField;

    while(true)
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

            if (r12 == r14->subFieldIndexMenuSelection)
            {
                vdp2PrintStatus.palette = 0x9000;
            }
            else
            {
                vdp2PrintStatus.palette = 0x8000;
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
        vdp2PrintStatus.palette = 0x9000;
        vdp2DebugPrintSetPosition(0x12, r14->subFieldIndexMenuSelection - pWorkArea->m4_selectedSubField + 3);
        drawLineSmallFont("\x7F");
    }

    vdp2PrintStatus.palette = 0x9000;
    vdp2DebugPrintSetPosition(0x1D, 3);
    vdp2PrintfSmallFont("%d   ", r14->field_3A);

    vdp2PrintStatus.palette = 0x7000;
    vdp2DebugPrintSetPosition(3, 0x18);
    vdp2PrintfLargeFont("GO:%2d  ", r14->field_3D);

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

    vdp2PrintStatus.palette = 0xC000;

    u8 inputValue = graphicEngineStatus.m4514.m0[0].m0_current.m8_newButtonDown & 0xF;
    if (inputValue == 0)
    {
        return;
    }

    if (inputValue & 3)
    {
        setOpenMenu7();
    }

    if (menuUnk0.m_4D >= menuUnk0.m_4C)
    {
        vdp2Controls.m_registers[0].N1COSL = 0x10;
        vdp2Controls.m_registers[1].N1COSL = 0x10;
    }

    fadePalette(&menuUnk0.m_field0, 0, 0, 1);
    fadePalette(&menuUnk0.m_field24, 0, 0, 1);

    r14->fieldTaskState++;
}

s_taskDefinition fieldDebugListTaskDefinition = { fieldDebugListTaskInit, fieldDebugListTaskUpdate, NULL, NULL, "field debug list" };

p_workArea createFieldInputTask(p_workArea pTypelessWorkArea)
{
    return createSubTask(pTypelessWorkArea, &fieldDebugListTaskDefinition, new s_fieldDebugListWorkArea);
}

struct s_dramAllocationNode
{
    s_dramAllocationNode* m_pNext;
    u32 size;
};

struct s_dramAllocator
{
    s_dramAllocationNode* buffer; //0
    u32 pNext; // 4
    u8* allocationStart; //0x8
    u8* allocationEnd; // 0xC
    s_dramAllocator* m_nextNode; // 0x10
    u32 var_14;
}; // size 18

s_dramAllocator* dramAllocatorHead = NULL;
s_dramAllocator* dramAllocatorEnd = NULL;

void resetTempAllocators()
{
    dramAllocatorHead = NULL;
    dramAllocatorEnd = NULL;
    vdp1AllocatorHead = NULL;
}

void loadRamResource(s_workArea* pWorkArea)
{
    if (dramAllocatorHead)
    {
        assert(0);
    }
}

void initDramAllocator(s_workArea* pWorkArea, u8* dest, u32 size, const char** assetList)
{
    loadRamResource(pWorkArea);

    u32 r14 = sizeof(s_dramAllocator);

    if (assetList)
    {
        assert(0);
    }

    s_dramAllocator* pDramAllocator = (s_dramAllocator*)allocateHeapForTask(pWorkArea, r14);
    
    pDramAllocator->allocationStart = dest;
    pDramAllocator->allocationEnd = dest + size;
    pDramAllocator->m_nextNode = dramAllocatorHead;
    dramAllocatorHead = pDramAllocator;

    dramAllocatorEnd = pDramAllocator + 1;

    u32 pNext = 0;

    if (assetList)
    {
        assert(0);
    }

    s_dramAllocationNode* pNode = (s_dramAllocationNode*)dest;

    pDramAllocator->buffer = pNode;
    pDramAllocator->pNext = pNext;

    pNode->m_pNext = NULL;
    pNode->size = size;

    addToMemoryLayout(dest, 8);
}

u8* dramAllocate(u32 size, u32 unk)
{
    if (size == 0)
        return NULL;

    // TODO: does the alignment stuff still works in 64bits?
    u32 paddedSize = (size + sizeof(s_dramAllocationNode) + 0xF) & 0xFFFFFFF0;

    s_dramAllocationNode** r5 = &dramAllocatorHead->buffer;

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

u8 gDragonModel[0x16500];
u8 gDragonVram[0x4000];

void unimplementedUpdate(s_3dModel* pDragonStateData1)
{
    assert(0);
}

void unimplementedDraw(s_3dModel* pDragonStateData1)
{
    assert(0);
}

s32 stepAnimationTrack(sAnimTrackStatus&r4, u8* r5, u16 maxStep)
{
    if (r4.delay > 0)
    {
        r4.delay -= 1;
        return r4.value;
    }

    if (r4.currentStep)
    {
        //06022D5A
        u16 r0 = READ_BE_U16(r5 + r4.currentStep * 2);
        r4.delay = (r0 & 0xF) -1;
        r4.value = (s16)(r0 & 0xFFF0);
    }
    else
    {
        //06022D6E
        r4.delay = 0;
        r4.value = READ_BE_S16(r5) * 16;
    }

    if (maxStep > r4.currentStep + 1)
    {
        r4.currentStep++;
    }
    else
    {
        r4.currentStep = 0;
    }

    return r4.value;
}

void modelMode4_position0(s_3dModel* pDragonStateData1)
{
    sPoseData* pPoseData = pDragonStateData1->m2C_poseData;
    u8* r13 = pDragonStateData1->m30_pCurrentAnimation + READ_BE_U32(pDragonStateData1->m30_pCurrentAnimation + 8);
    u8* r4 = pDragonStateData1->m34_pDefaultPose;

    if (pDragonStateData1->m10_currentAnimationFrame & 1)
    {
        pPoseData->m_translation += pPoseData->halfTranslation;
        return;
    }

    //06022638
    if (pDragonStateData1->m10_currentAnimationFrame)
    {
        pPoseData->m_translation += pPoseData->halfTranslation;

        s16 r3;
        if (READ_BE_U32(pDragonStateData1->m30_pCurrentAnimation))
        {
            r3 = READ_BE_S16(pDragonStateData1->m30_pCurrentAnimation + 4);
        }
        else
        {
            r3 = 0;
        }

        r3--;
        if (pDragonStateData1->m10_currentAnimationFrame >= r3)
        {
            return;
        }

        pPoseData->halfTranslation[0] = stepAnimationTrack(pPoseData->field_48[0], pDragonStateData1->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x14), READ_BE_U16(r13 + 0)) / 2;
        pPoseData->halfTranslation[1] = stepAnimationTrack(pPoseData->field_48[1], pDragonStateData1->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x18), READ_BE_U16(r13 + 2)) / 2;
        pPoseData->halfTranslation[2] = stepAnimationTrack(pPoseData->field_48[2], pDragonStateData1->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x1C), READ_BE_U16(r13 + 4)) / 2;
    }
    else
    //0602265A
    if (pDragonStateData1->m8 & 4)
    {
        assert(0);
    }
    else
    {
        pPoseData->m_translation[0] = READ_BE_U32(r4);
        pPoseData->m_translation[1] = READ_BE_U32(r4 + 4);
        pPoseData->m_translation[2] = READ_BE_U32(r4 + 8);

        stepAnimationTrack(pPoseData->field_48[0], pDragonStateData1->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x14), READ_BE_U16(r13 + 0));
        stepAnimationTrack(pPoseData->field_48[1], pDragonStateData1->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x18), READ_BE_U16(r13 + 2));
        stepAnimationTrack(pPoseData->field_48[2], pDragonStateData1->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x1C), READ_BE_U16(r13 + 4));
    }

    //60226BA
    s16 r3;
    if (READ_BE_U32(pDragonStateData1->m30_pCurrentAnimation))
    {
        r3 = READ_BE_S16(pDragonStateData1->m30_pCurrentAnimation + 4);
    }
    else
    {
        r3 = 0;
    }

    r3--;
    if (r3 >= pDragonStateData1->m10_currentAnimationFrame)
    {
        pPoseData->halfTranslation[0] = stepAnimationTrack(pPoseData->field_48[0], pDragonStateData1->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x14), READ_BE_U16(r13 + 0)) / 2;
        pPoseData->halfTranslation[1] = stepAnimationTrack(pPoseData->field_48[1], pDragonStateData1->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x18), READ_BE_U16(r13 + 2)) / 2;
        pPoseData->halfTranslation[2] = stepAnimationTrack(pPoseData->field_48[2], pDragonStateData1->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x1C), READ_BE_U16(r13 + 4)) / 2;
    }
    
}

void modeDrawFunction10Sub1(u8* pModelDataRoot, u8* pModelData, sPoseData** r14)
{
    do 
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&(*r14)->m_translation);
        rotateCurrentMatrixZYX(&(*r14)->m_rotation);

        if (u32 offset = READ_BE_U32(pModelData))
        {
            addObjectToDrawList(pModelDataRoot, offset);
        }

        if (u32 offset = READ_BE_U32(pModelData + 4))
        {
            (*r14)++;
            modeDrawFunction10Sub1(pModelDataRoot, pModelDataRoot + offset, r14);
        }


        popMatrix();

        // End of model
        if (READ_BE_U32(pModelData + 8) == 0)
        {
            return;
        }

        (*r14)++;
        pModelData = pModelDataRoot + READ_BE_U32(pModelData + 8);

    } while (1);
}

void modeDrawFunction6Sub2(u8* pModelDataRoot, u8* pModelData, sPoseData*& pPoseData, const s_RiderDefinitionSub*& r6, sVec3_FP**& r7)
{
    unimplemented("modeDrawFunction6Sub2");
}

void modeDrawFunction6Sub1(u8* pModelDataRoot, u8* pModelData, sPoseData*& pPoseData, const s_RiderDefinitionSub*& r6, sVec3_FP**& r7)
{
    do
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pPoseData->m_translation);
        rotateCurrentMatrixZYX(&pPoseData->m_rotation);

        if (u32 offset = READ_BE_U32(pModelData))
        {
            addObjectToDrawList(pModelDataRoot, offset);
        }

        if (r6->m_count == 0)
        {
            assert((*r7) == NULL);
        }

        for (u32 i = 0; i < r6->m_count; i++)
        {
            sSaturnPtr r4 = r6->m_ptr + (i * 20) + 4;
            sVec3_FP input = readSaturnVec3(r4);
            sVec3_FP* output = (*r7) + i;
            transformAndAddVecByCurrentMatrix(&input, output);
        }

        if (u32 offset = READ_BE_U32(pModelData + 4))
        {
            // next matrix
            pPoseData++;
            // next bone stuff
            r6++;
            // next ???
            r7++;

            modeDrawFunction6Sub1(pModelDataRoot, pModelDataRoot + offset, pPoseData, r6, r7);
        }

        popMatrix();

        // End of model
        if (READ_BE_U32(pModelData + 8) == 0)
        {
            return;
        }

        // next matrix
        pPoseData++;
        // next bone stuff
        r6++;
        // next ???
        r7++;

        pModelData = pModelDataRoot + READ_BE_U32(pModelData + 8);
    } while (1);
}

void submitModelToRendering(u8* pModelDataRoot, u8* pModelData, sMatrix4x3*& modelMatrix, const s_RiderDefinitionSub*& r6, sVec3_FP**& r7)
{
    do 
    {
        pushCurrentMatrix();
        multiplyCurrentMatrix(modelMatrix);

        if (u32 offset = READ_BE_U32(pModelData))
        {
            addObjectToDrawList(pModelDataRoot, offset);
        }

        if (r6->m_count == 0)
        {
            assert((*r7) == NULL);
        }

        for (u32 i = 0; i < r6->m_count; i++)
        {
            sSaturnPtr r4 = r6->m_ptr + (i * 20) + 4;
            sVec3_FP input = readSaturnVec3(r4);
            sVec3_FP* output = (*r7) + i;
            transformAndAddVecByCurrentMatrix(&input, output);
        }

        if (u32 offset = READ_BE_U32(pModelData + 4))
        {
            // next matrix
            modelMatrix++;
            // next bone stuff
            r6++;
            // next ???
            r7++;

            submitModelToRendering(pModelDataRoot, pModelDataRoot + offset, modelMatrix, r6, r7);
        }

        popMatrix();

        // End of model
        if (READ_BE_U32(pModelData + 8) == 0)
        {
            return;
        }

        // next matrix
        modelMatrix++;
        // next bone stuff
        r6++;
        // next ???
        r7++;

        pModelData = pModelDataRoot + READ_BE_U32(pModelData + 8);
    } while (1);
}

void modeDrawFunction1Sub2(u8* pModelDataRoot, u8* pModelData, sMatrix4x3* r5, const s_RiderDefinitionSub*& r6, sVec3_FP**& r7)
{
    assert(0);
}

void addAnimationFrame(sPoseData* pPoseData, s_3dModel* p3dModel)
{
    for (int i = 0; i < p3dModel->m12_numBones; i++)
    {
        pPoseData[i].m_rotation += pPoseData[i].halfRotation;
    }
}

void (*modelMode4_position1)(s_3dModel*) = unimplementedUpdate;
void modelMode4_rotation(s_3dModel* p3dModel)
{
    sPoseData* pPoseData = p3dModel->m2C_poseData;
    if (p3dModel->m10_currentAnimationFrame & 1)
    {
        addAnimationFrame(pPoseData, p3dModel);
        return;
    }

    if (p3dModel->m10_currentAnimationFrame)
    {
        addAnimationFrame(pPoseData, p3dModel);

        if (READ_BE_U16(p3dModel->m30_pCurrentAnimation + 4) - 1 > p3dModel->m10_currentAnimationFrame)
        {
            u8* r13 = p3dModel->m30_pCurrentAnimation + READ_BE_U32(p3dModel->m30_pCurrentAnimation + 8);
            for (int i = 0; i < p3dModel->m12_numBones; i++)
            {
                pPoseData[i].halfRotation.m_value[0] = stepAnimationTrack(pPoseData[i].field_48[3], p3dModel->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x20), READ_BE_U16(r13 + 6)) << 11;
                pPoseData[i].halfRotation.m_value[1] = stepAnimationTrack(pPoseData[i].field_48[4], p3dModel->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x24), READ_BE_U16(r13 + 8)) << 11;
                pPoseData[i].halfRotation.m_value[2] = stepAnimationTrack(pPoseData[i].field_48[5], p3dModel->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x28), READ_BE_U16(r13 + 10)) << 11;
                r13 += 0x38;
            }
        }
    }
    else
    {
        u8* r13 = p3dModel->m30_pCurrentAnimation + READ_BE_U32(p3dModel->m30_pCurrentAnimation + 8);
        for (int i = 0; i < p3dModel->m12_numBones; i++)
        {
            assert(pPoseData[i].field_48[3].currentStep == 0);
            assert(pPoseData[i].field_48[4].currentStep == 0);
            assert(pPoseData[i].field_48[5].currentStep == 0);

            pPoseData[i].m_rotation.m_value[0] = stepAnimationTrack(pPoseData[i].field_48[3], p3dModel->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x20), READ_BE_U16(r13 + 6)) << 12;
            pPoseData[i].m_rotation.m_value[1] = stepAnimationTrack(pPoseData[i].field_48[4], p3dModel->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x24), READ_BE_U16(r13 + 8)) << 12;
            pPoseData[i].m_rotation.m_value[2] = stepAnimationTrack(pPoseData[i].field_48[5], p3dModel->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x28), READ_BE_U16(r13 + 10)) << 12;
            r13 += 0x38;
        }

        if (READ_BE_U16(p3dModel->m30_pCurrentAnimation + 4) - 1 > p3dModel->m10_currentAnimationFrame)
        {
            u8* r13 = p3dModel->m30_pCurrentAnimation + READ_BE_U32(p3dModel->m30_pCurrentAnimation + 8);
            for (int i = 0; i < p3dModel->m12_numBones; i++)
            {
                pPoseData[i].halfRotation.m_value[0] = stepAnimationTrack(pPoseData[i].field_48[3], p3dModel->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x20), READ_BE_U16(r13 + 6)) << 11;
                pPoseData[i].halfRotation.m_value[1] = stepAnimationTrack(pPoseData[i].field_48[4], p3dModel->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x24), READ_BE_U16(r13 + 8)) << 11;
                pPoseData[i].halfRotation.m_value[2] = stepAnimationTrack(pPoseData[i].field_48[5], p3dModel->m30_pCurrentAnimation + READ_BE_U32(r13 + 0x28), READ_BE_U16(r13 + 10)) << 11;
                r13 += 0x38;
            }
        }
    }
}

void modelMode0_position(s_3dModel*)
{
    unimplemented("modelMode0_position");
}

void modelMode0_rotation(s_3dModel* p3dModel)
{
    sPoseData* pPoseData = p3dModel->m2C_poseData;
    u16 r0 = p3dModel->m12_numBones;
    u8* r6 = p3dModel->m30_pCurrentAnimation + READ_BE_U32(p3dModel->m30_pCurrentAnimation + 8);

    for (int i = 0; i < p3dModel->m12_numBones; i++)
    {
        pPoseData[i].m_rotation.m_value[0] = READ_BE_S16(p3dModel->m10_currentAnimationFrame * 2 + p3dModel->m30_pCurrentAnimation + READ_BE_U32(r6 + 0x20)) << 16;
        pPoseData[i].m_rotation.m_value[1] = READ_BE_S16(p3dModel->m10_currentAnimationFrame * 2 + p3dModel->m30_pCurrentAnimation + READ_BE_U32(r6 + 0x24)) << 16;
        pPoseData[i].m_rotation.m_value[2] = READ_BE_S16(p3dModel->m10_currentAnimationFrame * 2 + p3dModel->m30_pCurrentAnimation + READ_BE_U32(r6 + 0x28)) << 16;

        r6 += 0x38;
    }
}

void modelMode0_scale(s_3dModel*)
{
    unimplemented("modelMode0_scale");
}


void (*modelMode4_scale)(s_3dModel*) = unimplementedUpdate;

void modelDrawFunction0(s_3dModel* pModel)
{
    unimplementedDraw(pModel);
}
void modelDrawFunction1(s_3dModel* pDragonStateData1)
{
    sVec3_FP** var_0 = pDragonStateData1->m44;
    sMatrix4x3* var_8 = pDragonStateData1->m3C_boneMatrices;
    const s_RiderDefinitionSub* var_4 = pDragonStateData1->m40;

    if (pDragonStateData1->m8 & 1)
    {
        u8* r4 = pDragonStateData1->m4_pModelFile + READ_BE_U32(pDragonStateData1->m4_pModelFile + pDragonStateData1->mC_modelIndexOffset);
        submitModelToRendering(pDragonStateData1->m4_pModelFile, r4, var_8, var_4, var_0);
    }
    else
    {
        u8* r4 = pDragonStateData1->m4_pModelFile + READ_BE_U32(pDragonStateData1->m4_pModelFile + pDragonStateData1->mC_modelIndexOffset);
        modeDrawFunction1Sub2(pDragonStateData1->m4_pModelFile, r4, var_8, var_4, var_0);
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
void modelDrawFunction5(s_3dModel* pModel)
{
    unimplementedDraw(pModel);
}
void modelDrawFunction6(s_3dModel* pModel)
{
    sVec3_FP** var_0 = pModel->m44;
    sPoseData* pPoseData = pModel->m2C_poseData;
    const s_RiderDefinitionSub* var_4 = pModel->m40;
    u8* r4 = pModel->m4_pModelFile + READ_BE_U32(pModel->m4_pModelFile + pModel->mC_modelIndexOffset);

    if (pModel->m8 & 1)
    {
        modeDrawFunction6Sub1(pModel->m4_pModelFile, r4, pPoseData, var_4, var_0);
    }
    else
    {
        modeDrawFunction6Sub2(pModel->m4_pModelFile, r4, pPoseData, var_4, var_0);
    }
}
void modelDrawFunction9(s_3dModel* pModel)
{
    unimplementedDraw(pModel);
}
void modelDrawFunction10(s_3dModel* pModel)
{
    if (pModel->m8 & 1)
    {
        u8* r4 = pModel->m4_pModelFile + READ_BE_U32(pModel->m4_pModelFile + pModel->mC_modelIndexOffset);
        sPoseData* pPoseData = pModel->m2C_poseData;
        modeDrawFunction10Sub1(pModel->m4_pModelFile, r4, &pPoseData);
    }
}

void copyPosePosition(s_3dModel* pDragonStateData1)
{
    u8* r5 = pDragonStateData1->m34_pDefaultPose;

    for (u32 i = 0; i < pDragonStateData1->m12_numBones; i++)
    {
        pDragonStateData1->m2C_poseData[i].m_translation[0] = READ_BE_U32(r5 + 0);
        pDragonStateData1->m2C_poseData[i].m_translation[1] = READ_BE_U32(r5 + 4);
        pDragonStateData1->m2C_poseData[i].m_translation[2] = READ_BE_U32(r5 + 8);
        r5 += 0x24;
    }
}

void copyPoseRotation(s_3dModel* pDragonStateData1)
{
    u8* r5 = pDragonStateData1->m34_pDefaultPose;

    for (u32 i = 0; i < pDragonStateData1->m12_numBones; i++)
    {
        pDragonStateData1->m2C_poseData[i].m_rotation[0] = READ_BE_U32(r5 + 0xC);
        pDragonStateData1->m2C_poseData[i].m_rotation[1] = READ_BE_U32(r5 + 0x10);
        pDragonStateData1->m2C_poseData[i].m_rotation[2] = READ_BE_U32(r5 + 0x14);
        r5 += 0x24;
    }
}

void resetPoseScale(s_3dModel* pDragonStateData1)
{
    sPoseData* pOutputPose = pDragonStateData1->m2C_poseData;

    for (u32 i = 0; i < pDragonStateData1->m12_numBones; i++)
    {
        pOutputPose->m_scale[0] = 0x10000;
        pOutputPose->m_scale[1] = 0x10000;
        pOutputPose->m_scale[2] = 0x10000;

        pOutputPose++;
    }
}

s32 riderInit(s_3dModel* r4, u8* r5)
{
    if (r5 == NULL)
    {
        if (r4->mA & 8)
        {
            copyPosePosition(r4);
        }
        if (r4->mA & 0x10)
        {
            copyPoseRotation(r4);
        }
        if (r4->mA & 0x20)
        {
            resetPoseScale(r4);
        }

        r4->mA &= 0xFFC7;
        r4->m30_pCurrentAnimation = NULL;

        return 1;
    }

    if (r4->m30_pCurrentAnimation == NULL)
    {
        r4->mA |= READ_BE_U16(r5);
        initModelDrawFunction(r4);
        return createDragonStateSubData1Sub1(r4, r5);
    }

    assert(0);
}

u32 createDragonStateSubData1Sub1Sub1(s_3dModel* p3dModel, u8* pModelData)
{
    u16 flags = READ_BE_U16(pModelData);

    switch (flags & 7)
    {
    case 0:
        p3dModel->m20_positionUpdateFunction = modelMode0_position;
        p3dModel->m24_rotationUpdateFunction = modelMode0_rotation;
        p3dModel->m28_scaleUpdateFunction = modelMode0_scale;
        return 1;
    case 4:
        if (p3dModel->mA & 0x100)
        {
            p3dModel->m20_positionUpdateFunction = modelMode4_position0;
        }
        else
        {
            p3dModel->m20_positionUpdateFunction = modelMode4_position1;
        }
        p3dModel->m24_rotationUpdateFunction = modelMode4_rotation;
        p3dModel->m28_scaleUpdateFunction = modelMode4_scale;

        for (u32 i = 0; i < p3dModel->m12_numBones; i++)
        {
            for (u32 j = 0; j < 9; j++)
            {
                p3dModel->m2C_poseData[i].field_48[j].currentStep = 0;
                p3dModel->m2C_poseData[i].field_48[j].delay = 0;
                p3dModel->m2C_poseData[i].field_48[j].value = 0;
            }
        }
        break;
    default:
        assert(0);
    }

    return 1;
}

u32 createDragonStateSubData1Sub1(s_3dModel* pDragonStateData1, u8* pModelData1)
{
    pDragonStateData1->m30_pCurrentAnimation = pModelData1;
    pDragonStateData1->m10_currentAnimationFrame = 0;

    u16 flags = READ_BE_U16(pModelData1);

    if ((flags & 8) || (pDragonStateData1->mA & 0x100))
    {
        copyPosePosition(pDragonStateData1);
    }

    if (flags & 0x10)
    {
        copyPoseRotation(pDragonStateData1);
    }

    if (flags & 0x20)
    {
        resetPoseScale(pDragonStateData1);
    }

    return createDragonStateSubData1Sub1Sub1(pDragonStateData1, pModelData1);
}

u32 dragonFieldTaskInitSub3Sub1Sub1(s_3dModel* pDragonStateData1, u8* pModelData1)
{
    pDragonStateData1->m30_pCurrentAnimation = pModelData1;
    pDragonStateData1->m10_currentAnimationFrame = 0;

    u16 flags = READ_BE_U16(pModelData1);

    if ((flags & 8) || (pDragonStateData1->mA & 0x100))
    {
        pDragonStateData1->m2C_poseData->m_translation[0] = READ_BE_U32(pDragonStateData1->m34_pDefaultPose + 0);
        pDragonStateData1->m2C_poseData->m_translation[1] = READ_BE_U32(pDragonStateData1->m34_pDefaultPose + 4);
        pDragonStateData1->m2C_poseData->m_translation[2] = READ_BE_U32(pDragonStateData1->m34_pDefaultPose + 8);
    }

    if (flags & 0x10)
    {
        copyPoseRotation(pDragonStateData1);
    }

    if (flags & 0x20)
    {
        resetPoseScale(pDragonStateData1);
    }

    return createDragonStateSubData1Sub1Sub1(pDragonStateData1, pModelData1);
}

void initModelDrawFunction(s_3dModel* pDragonStateData1)
{
    if (pDragonStateData1->m8 & 2)
    {
        if (pDragonStateData1->m40)
        {
            if (pDragonStateData1->m38)
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
            if (pDragonStateData1->m38)
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
            if (pDragonStateData1->mA & 0x20)
            {
                assert(0);
            }
            else
            {
                if (pDragonStateData1->m38)
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
            if (pDragonStateData1->mA & 0x20)
            {
                assert(0);
            }
            else
            {
                if (pDragonStateData1->m38)
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

u32 dragonFieldTaskInitSub3Sub1(s_3dModel* pDragonStateData1, u8* r5)
{
    if (r5 == NULL)
    {
        assert(0);
    }
    else
    {
        if (pDragonStateData1->m30_pCurrentAnimation == NULL)
        {
            assert(0);
        }
        else
        {
            if (READ_BE_U16(pDragonStateData1->m30_pCurrentAnimation) != READ_BE_U16(r5))
            {
                pDragonStateData1->mA &= 0xFFC7;
                pDragonStateData1->mA |= READ_BE_U16(r5);
                initModelDrawFunction(pDragonStateData1);
                return dragonFieldTaskInitSub3Sub1Sub1(pDragonStateData1, r5);
            }
            // 6021728
            pDragonStateData1->m30_pCurrentAnimation = r5;
            pDragonStateData1->m10_currentAnimationFrame = 0;

            u16 r0 = READ_BE_U16(r5) & 7;
            if ((r0 != 1) && (r0 != 4) && (r0 != 5))
            {
                return 1;
            }

            for (int i = 0; i < pDragonStateData1->m12_numBones; i++)
            {
                for (int j = 0; j < 9; j++)
                {
                    pDragonStateData1->m2C_poseData[i].field_48[j].currentStep = 0;
                    pDragonStateData1->m2C_poseData[i].field_48[j].delay = 0;
                    pDragonStateData1->m2C_poseData[i].field_48[j].value = 0;
                }
            }

            return 1;
        }
        assert(0);
    }
    assert(0);
}

u32 stepAnimation(s_3dModel* p3DModel)
{
    if ((p3DModel->mA & 0x38) == 0)
    {
        return 0;
    }

    if (READ_BE_U16(p3DModel->m30_pCurrentAnimation) & 8)
    {
        p3DModel->m20_positionUpdateFunction(p3DModel);
    }

    if (READ_BE_U16(p3DModel->m30_pCurrentAnimation) & 0x10)
    {
        p3DModel->m24_rotationUpdateFunction(p3DModel);
    }

    if (READ_BE_U16(p3DModel->m30_pCurrentAnimation) & 0x20)
    {
        p3DModel->m28_scaleUpdateFunction(p3DModel);
    }

    p3DModel->m16 = p3DModel->m10_currentAnimationFrame;
    p3DModel->m10_currentAnimationFrame++;

    // animation reset
    if (READ_BE_U16(p3DModel->m30_pCurrentAnimation + 4) < p3DModel->m10_currentAnimationFrame)
    {
        p3DModel->m10_currentAnimationFrame = 0;
    }

    return p3DModel->m16;
}

void interpolateAnimation(s_3dModel* pDragonStateData1)
{
    if (pDragonStateData1->m48_poseDataInterpolation)
    {
        pDragonStateData1->m4C_interpolationStep++;
        if (pDragonStateData1->m4E_interpolationLength < pDragonStateData1->m4C_interpolationStep)
        {
            //TODO: freeVdp1Block(pDragonStateData1->m0_pOwnerTask, pDragonStateData1->m48)
            pDragonStateData1->m48_poseDataInterpolation = 0;
            initModelDrawFunction(pDragonStateData1);
        }
        else
        {
            unimplemented("dragonFieldTaskInitSub3Sub2Sub2");
        }
    }
}

u32 updateAndInterpolateAnimation(s_3dModel* pDragonStateData1)
{
    u32 r0 = stepAnimation(pDragonStateData1);
    interpolateAnimation(pDragonStateData1);

    return r0;
}

void countNumBonesInModel(s_3dModel* pDragonStateData1, u8* pDragonModelData, u8* pStartOfData)
{
    do
    {
        pDragonStateData1->m12_numBones++;
        if (READ_BE_U32(pDragonModelData + 4))
        {
            countNumBonesInModel(pDragonStateData1, pStartOfData + READ_BE_U32(pDragonModelData + 4), pStartOfData);
        }

        if (READ_BE_U32(pDragonModelData + 8))
        {
            pDragonModelData = pStartOfData + READ_BE_U32(pDragonModelData + 8);
        }
        else
        {
            break;
        }
    }while (true);
}

bool createDragonStateSubData1Sub2(s_3dModel* pDragonStateData1, const s_RiderDefinitionSub* unkArg)
{
    pDragonStateData1->m40 = unkArg;

    pDragonStateData1->m44 = static_cast<sVec3_FP**>(allocateHeapForTask(pDragonStateData1->m0_pOwnerTask, pDragonStateData1->m12_numBones * sizeof(sVec3_FP*)));
    if (pDragonStateData1->m44 == NULL)
        return false;

    const s_RiderDefinitionSub* r12 = pDragonStateData1->m40;

    for(u32 i=0; i<pDragonStateData1->m12_numBones; i++)
    {
        if (r12->m_count > 0)
        {
            pDragonStateData1->m44[i] = (sVec3_FP*)allocateHeapForTask(pDragonStateData1->m0_pOwnerTask, r12->m_count * sizeof(sVec3_FP));
            if (pDragonStateData1->m44[i] == NULL)
                return false;
        }
        else
        {
            pDragonStateData1->m44[i] = NULL;
        }

        r12 ++;
    }

    return true;
}

bool init3DModelRawData(s_workArea* pWorkArea, s_3dModel* pDragonStateData1, u32 unkArg0, u8* pDragonModel, u16 modelIndexOffset, u8* pModelData1, u8* pDefaultPose, u32 unkArg2, const s_RiderDefinitionSub* unkArg3)
{
    pDragonStateData1->m0_pOwnerTask = pWorkArea;
    pDragonStateData1->m4_pModelFile = pDragonModel;
    pDragonStateData1->mC_modelIndexOffset = modelIndexOffset;
    pDragonStateData1->m34_pDefaultPose = pDefaultPose;
    pDragonStateData1->m38 = unkArg2;
    pDragonStateData1->m14 = 0;
    pDragonStateData1->m16 = 0;
    pDragonStateData1->m8 = 1;

    if (pModelData1)
    {
        pDragonStateData1->mA = READ_BE_U16(pModelData1) | unkArg0;
        pDragonStateData1->m12_numBones = READ_BE_U16(pModelData1 + 2);
    }
    else
    {
        pDragonStateData1->mA = unkArg0;
        pDragonStateData1->m12_numBones = 0;
        countNumBonesInModel(pDragonStateData1, pDragonModel + READ_BE_U32(pDragonModel + pDragonStateData1->mC_modelIndexOffset), pDragonModel);
    }

    pDragonStateData1->m2C_poseData = static_cast<sPoseData*>(allocateHeapForTask(pWorkArea, pDragonStateData1->m12_numBones * sizeof(sPoseData)));

    if (pDragonStateData1->mA & 0x200)
    {
        pDragonStateData1->m3C_boneMatrices = static_cast<sMatrix4x3*>(allocateHeapForTask(pWorkArea, pDragonStateData1->m12_numBones * sizeof(sMatrix4x3)));
        assert(pDragonStateData1->m3C_boneMatrices);

        pDragonStateData1->m8 |= 2;
    }
    else
    {
        pDragonStateData1->m3C_boneMatrices = 0;
        pDragonStateData1->m8 &= 0xFFFD;
    }

    if (unkArg3)
    {
        if (!createDragonStateSubData1Sub2(pDragonStateData1, unkArg3))
            return false;
    }
    else
    {
        pDragonStateData1->m40 = 0;
    }

    if (pModelData1)
    {
        if (createDragonStateSubData1Sub1(pDragonStateData1, pModelData1) == 0)
            return false;
    }
    else
    {
        pDragonStateData1->m30_pCurrentAnimation = NULL;
        pDragonStateData1->m10_currentAnimationFrame = 0;

        copyPosePosition(pDragonStateData1);
        copyPoseRotation(pDragonStateData1);
        resetPoseScale(pDragonStateData1);
    }

    initModelDrawFunction(pDragonStateData1);

    pDragonStateData1->m1C_addToDisplayListFunction = addObjectToDrawList;

    return true;
}

struct sDragonAnimDataSub
{
    s32 count;
    const sDragonAnimDataSubRanges* m_data;
};

struct sDragonAnimData
{
    const sDragonAnimDataSub* m_0;
    const sDragonAnimDataSub* m_4;
    const sDragonAnimDataSub* m_8;
    const sDragonAnimDataSub* m_C;
};

const sDragonAnimDataSubRanges dragon0AnimsData0 =
{
    {0x2423, 0x2423, 0x2423},
    {0xB800, 0xB800, 0xB800},
    {0xE38E38, 0xE38E38, 0xE38E38},
    {-0xE38E38, -0xE38E38, -0xE38E38},
};
const sDragonAnimDataSubRanges dragon0AnimsData1 =
{
    {0x400, 0x400, 0x400},
    {0x1000, 0x1000, 0x1000},
    {0xE38E38, 0xE38E38, 0xE38E38},
    {0xFF1C71C8, 0xFF1C71C8, 0xFF1C71C8},
};

const sDragonAnimDataSub dragon0Anims0[1] =
{
    { 0x01, &dragon0AnimsData0 },
};

const sDragonAnimDataSub dragon0Anims1[] =
{
    { 0x0E, &dragon0AnimsData1 },
    { 0x0F, &dragon0AnimsData1 },
    { 0x10, &dragon0AnimsData1 },
    { 0x11, &dragon0AnimsData1 },
    { -1, NULL },
};

const sDragonAnimDataSub dragon0Anims2[] =
{
    { 0x02, &dragon0AnimsData0 },
    { 0x03, &dragon0AnimsData0 },
    { 0x04, &dragon0AnimsData0 },
    { 0x05, &dragon0AnimsData0 },
    { 0x06, &dragon0AnimsData0 },
    { -1, NULL },
};

const sDragonAnimData dragon0Anims =
{
    dragon0Anims0,
    dragon0Anims1,
    dragon0Anims2,
    NULL,
};

const sDragonAnimData* dragonAnimData[DR_ANIM_MAX] =
{
    &dragon0Anims,
    (const sDragonAnimData*)1,
    (const sDragonAnimData*)1,
    (const sDragonAnimData*)1,
    (const sDragonAnimData*)1,
    (const sDragonAnimData*)1,
    (const sDragonAnimData*)1,
    (const sDragonAnimData*)1,
    (const sDragonAnimData*)1,
    (const sDragonAnimData*)1,
};

u32 countNumAnimData(s3DModelAnimData* pDragonStateData2, const sDragonAnimData* dragonAnims)
{
    pDragonStateData2->count0 = 0;
    pDragonStateData2->count1 = 0;
    pDragonStateData2->count2 = 0;

    const sDragonAnimDataSub* r6 = dragonAnims->m_4;
    if (r6)
    {
        while (r6->count >= 0)
        {
            pDragonStateData2->count0++;
            r6++;
        }
    }

    r6 = dragonAnims->m_8;
    if (r6)
    {
        while (r6->count >= 0)
        {
            pDragonStateData2->count1++;
            r6++;
        }
    }

    r6 = dragonAnims->m_C;
    if (r6)
    {
        while (r6->count >= 0)
        {
            pDragonStateData2->count2++;
            r6++;
        }
    }

    return pDragonStateData2->count0 + pDragonStateData2->count1 + pDragonStateData2->count2 + 1;
}

void copyAnimMatrix(const sDragonAnimDataSubRanges* source, sDragonAnimDataSubRanges* destination)
{
    *destination = *source;
}

void initRuntimeAnimDataSub1(const sDragonAnimDataSub* animDataSub, s_runtimeAnimData* subData)
{
    subData->m0_root.zero();
    subData->m_vec_C.zero();
    subData->m_vec_18.zero();
    subData->m24_rootDelta.zero();

    copyAnimMatrix(animDataSub->m_data, &subData->m_factors);

    subData->dataSource = animDataSub;
}

void initRuntimeAnimData(const sDragonAnimData* dragonAnims, s_runtimeAnimData* subData)
{
    initRuntimeAnimDataSub1(dragonAnims->m_0, &subData[0]);
    u32 r14 = 1;

    if (dragonAnims->m_4)
    {
        const sDragonAnimDataSub* r12 = dragonAnims->m_4;

        while (r12->count >= 0)
        {
            initRuntimeAnimDataSub1(r12, &subData[r14]);
            r14++;
            r12++;
        }
    }
    
    if (dragonAnims->m_8)
    {
        const sDragonAnimDataSub* r12 = dragonAnims->m_8;

        while (r12->count >= 0)
        {
            initRuntimeAnimDataSub1(r12, &subData[r14]);
            r14++;
            r12++;
        }
    }

    if (dragonAnims->m_C)
    {
        const sDragonAnimDataSub* r12 = dragonAnims->m_C;

        while (r12->count >= 0)
        {
            initRuntimeAnimDataSub1(r12, &subData[r14]);
            r14++;
            r12++;
        }
    }
}

void init3DModelAnims(s_dragonState* pDragonState, s_3dModel* pDragonStateData1, s3DModelAnimData* p3DModelAnimData, const sDragonAnimData* dragonAnims)
{
    p3DModelAnimData->m0_animData = dragonAnims;

    p3DModelAnimData->countAnims = countNumAnimData(p3DModelAnimData, dragonAnims);

    p3DModelAnimData->boneMatrices = pDragonStateData1->m3C_boneMatrices;

    p3DModelAnimData->m8_runtimeAnimData = static_cast<s_runtimeAnimData*>(allocateHeapForTask(pDragonState, p3DModelAnimData->countAnims * sizeof(s_runtimeAnimData)));

    initRuntimeAnimData(dragonAnims, p3DModelAnimData->m8_runtimeAnimData);
}

const sDragonAnimData* getDragonDataByIndex(e_dragonLevel dragonLevel)
{
    return dragonAnimData[dragonLevel];
}

void loadDragonSoundBank(e_dragonLevel dragonLevel)
{
    //assert(0);
}

void createDragon3DModel(s_workArea* pWorkArea, e_dragonLevel dragonLevel)
{
    const sDragonData3* pDragonData3 = &dragonData3[dragonLevel];
    const s_dragonData2* pDragonAnimOffsets = &dragonAnimOffsets[dragonLevel];

    s_dragonState* pDragonState = static_cast<s_dragonState*>(createSubTaskFromFunction(pWorkArea, NULL, new s_dragonState, "dragonState"));

    pDragonState->m0_pDragonModelRawData = gDragonModel;
    pDragonState->mC_dragonType = dragonLevel;
    pDragonState->m14_modelIndex = pDragonData3->m_field_8[0].m_field_0[0];
    pDragonState->m18_shadowModelIndex = pDragonData3->m_field_8[0].m_field_0[1];
    pDragonState->m20_dragonAnimOffsets = pDragonAnimOffsets->m_data;
    pDragonState->m24_dragonAnimCount = pDragonAnimOffsets->m_count;
    pDragonState->m88 = 1;

    u8* pDragonModel = pDragonState->m0_pDragonModelRawData;
    u8* pDefaultAnimationData = pDragonModel + READ_BE_U32(pDragonModel + pDragonState->m20_dragonAnimOffsets[0]);
    u8* defaultPose = pDragonModel + READ_BE_U32(pDragonModel + pDragonData3->m_field_8[0].m_field_0[2]);

    init3DModelRawData(pDragonState, &pDragonState->m28_dragon3dModel, 0x300, pDragonModel, pDragonState->m14_modelIndex, pDefaultAnimationData, defaultPose, 0, pDragonData3->m_field_8[0].m_field_8);

    init3DModelAnims(pDragonState, &pDragonState->m28_dragon3dModel, &pDragonState->m78_animData, getDragonDataByIndex(dragonLevel));

    loadDragonSoundBank(dragonLevel);

    gDragonState = pDragonState;
}

void loadDragonFiles(s_workArea* pWorkArea, e_dragonLevel dragonLevel)
{
    loadFile(dragonFilenameTable[dragonLevel].m_base.MCB, gDragonModel, gDragonVram);
    loadFile(dragonFilenameTable[dragonLevel].m_base.CGB, gDragonVram, NULL);

    createDragon3DModel(pWorkArea, dragonLevel);
}

s_loadDragonWorkArea* loadDragonModel(s_workArea* pWorkArea, e_dragonLevel dragonLevel)
{
    s_loadDragonWorkArea* pLoadDragonWorkArea = static_cast<s_loadDragonWorkArea*>(createSubTaskFromFunction(pWorkArea, NULL, new s_loadDragonWorkArea, "loadDragonModel"));

    pLoadDragonWorkArea->dramAllocation = dramAllocate(0x1F600, 0);
    pLoadDragonWorkArea->vramAllocation = NULL;
    pLoadDragonWorkArea->MCBOffsetInDram = 0x18E00;
    pLoadDragonWorkArea->CGBOffsetInDram = 0;

    if (dragonFilenameTable[dragonLevel].m_M.MCB)
    {
        assert(0);
        loadFile(dragonFilenameTable[dragonLevel].m_M.MCB, pLoadDragonWorkArea->dramAllocation, pLoadDragonWorkArea->dramAllocation + pLoadDragonWorkArea->MCBOffsetInDram);
        loadFile(dragonFilenameTable[dragonLevel].m_M.CGB, pLoadDragonWorkArea->dramAllocation + pLoadDragonWorkArea->MCBOffsetInDram, 0);

    }
    return pLoadDragonWorkArea;
}

void morphDragon(s_loadDragonWorkArea* pLoadDragonWorkArea, s_3dModel* pDragonStateSubData1, u32 unk0, const sDragonData3* pDragonData3, s16 cursorX, s16 cursorY)
{
    if (pDragonData3->m_field_0 == 0)
    {
        return;
    }

    const sDragonData3Sub* r13 = &pDragonData3->m_field_8[1];
    const sDragonData3Sub* r11 = &pDragonData3->m_field_8[5];

    assert(false);
}

void dramFree(u8* ptr)
{
    yLog("dramFree not implemented");
}

void vdp1Free(u8* ptr)
{
    yLog("vdp1Free not implemented");
}

void loadDragonSub1Sub1(s_loadDragonWorkArea* pLoadDragonWorkArea)
{
    /*
    if (pLoadDragonWorkArea->MCBOffsetInDram >= 0)
    {
        deleteLoadedFile(pLoadDragonWorkArea->MCBOffsetInDram);
        pLoadDragonWorkArea->MCBOffsetInDram = -1;
    }

    if (pLoadDragonWorkArea->CGBOffsetInDram >= 0)
    {
        deleteLoadedFile(pLoadDragonWorkArea->CGBOffsetInDram);
        CGBOffsetInDram->MCBOffsetInDram = -1;
    }
    */
    if (pLoadDragonWorkArea->dramAllocation)
    {
        dramFree(pLoadDragonWorkArea->dramAllocation);
        pLoadDragonWorkArea->dramAllocation = NULL;
    }

    if (pLoadDragonWorkArea->vramAllocation)
    {
        vdp1Free(pLoadDragonWorkArea->vramAllocation);
        pLoadDragonWorkArea->vramAllocation = NULL;
    }
}

void loadDragonSub1(s_loadDragonWorkArea* pLoadDragonWorkArea)
{
    if (pLoadDragonWorkArea)
    {
        loadDragonSub1Sub1(pLoadDragonWorkArea);

        pLoadDragonWorkArea->getTask()->markFinished();
    }
}

void loadDragon(s_workArea* pWorkArea)
{
    const sDragonData3* pDragonData3 = &dragonData3[mainGameState.gameStats.m1_dragonLevel];

    loadDragonFiles(pWorkArea, mainGameState.gameStats.m1_dragonLevel);

    updateDragonStatsFromLevel();

    gDragonState->m10_cursorX = mainGameState.gameStats.dragonCursorX;
    gDragonState->m12_cursorY = mainGameState.gameStats.dragonCursorY;
    gDragonState->m1C_dragonArchetype = mainGameState.gameStats.dragonArchetype;

    s_loadDragonWorkArea* pLoadDragonWorkArea = loadDragonModel(pWorkArea, mainGameState.gameStats.m1_dragonLevel);

    morphDragon(pLoadDragonWorkArea, &gDragonState->m28_dragon3dModel, pLoadDragonWorkArea->MCBOffsetInDram, pDragonData3, mainGameState.gameStats.dragonCursorX, mainGameState.gameStats.dragonCursorY);

    loadDragonSub1(pLoadDragonWorkArea);
}

const s_RiderDefinitionSub gEdgeExtraData[] =
{
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { { 0x2020E8, &gCommonFile }, 1 },
};

const s_RiderDefinition gRiderTable[] = {
    { "RIDER0.MCB",  NULL,          0x4,    0x08, NULL},
    { "EDGE.MCB",   "EDGE.CGB",     0x4,    0x28, gEdgeExtraData },
    { "GUSH.MCB",   "GUSH.CGB",     0x4,    0x20, NULL },
    { "PAET.MCB",   "PAET.CGB",     0x4,    0x20, NULL },
    { "AZCT.MCB",   "AZCT.CGB",     0x4,    0x20, NULL },
    { "AZEL.MCB",   "AZEL.CGB",     0x4,    0x20, NULL },
    { NULL,         NULL,           0x8,    0xC4, (s_RiderDefinitionSub*)1 },
    { NULL,         NULL,           0xC,    0xC8, NULL },
};

s_loadRiderWorkArea* pRiderState = NULL;
s_loadRiderWorkArea* pRider2State = NULL;

u8 riderModel[0x4F00];
u8 rider2Model[0xC00];
u8 riderVRam[0x1400];
u8 rider2VRam[0x1400];

s_loadRiderWorkArea* loadRider(s_workArea* pWorkArea, u8 riderType)
{
    const s_RiderDefinition* r13 = &gRiderTable[riderType];

    u8* pModelData1 = NULL;

    s_loadRiderWorkArea* pLoadRiderWorkArea = static_cast<s_loadRiderWorkArea*>(createSubTaskFromFunction(pWorkArea, NULL, new s_loadRiderWorkArea, "LoadRider"));

    pLoadRiderWorkArea->m4 = 0;
    pLoadRiderWorkArea->m_ParentWorkArea = pWorkArea;
    pLoadRiderWorkArea->m_riderType = riderType;
    pLoadRiderWorkArea->m_modelIndex = r13->m_flags;

    pRiderState = pLoadRiderWorkArea;

    if (riderType < 6)
    {
        pLoadRiderWorkArea->m0_riderModel = riderModel;
        if (riderType == 1)
        {
            pLoadRiderWorkArea->m_14 = 0x24;
        }
        else
        {
            pLoadRiderWorkArea->m_14 = 0;
        }

        loadFile(r13->m_MCBName, riderModel, riderVRam);

        if (r13->m_CGBName)
        {
            loadFile(r13->m_CGBName, riderVRam, 0);
        }
    }
    else
    {
        assert(0);
    }

    u8* pModel = pLoadRiderWorkArea->m0_riderModel;
    u8* pDefaultPose = pModel + READ_BE_U32(pModel + r13->m_flags2);

    init3DModelRawData(pLoadRiderWorkArea, &pLoadRiderWorkArea->m18_3dModel, 0, pModel, pLoadRiderWorkArea->m_modelIndex, pModelData1, pDefaultPose, 0, r13->m_pExtraData);

    return pLoadRiderWorkArea;
}

s_loadRiderWorkArea* loadRider2(s_workArea* pWorkArea, u8 riderType)
{
    const s_RiderDefinition* r13 = &gRiderTable[riderType];

    u8* pModelData1 = NULL;

    s_loadRiderWorkArea* pLoadRiderWorkArea = static_cast<s_loadRiderWorkArea*>(createSubTaskFromFunction(pWorkArea, NULL, new s_loadRiderWorkArea, "LoadRider2"));

    pLoadRiderWorkArea->m4 = 0;
    pLoadRiderWorkArea->m_ParentWorkArea = pWorkArea;
    pLoadRiderWorkArea->m_riderType = riderType;
    pLoadRiderWorkArea->m_modelIndex = r13->m_flags;

    pRider2State = pLoadRiderWorkArea;

    if (riderType < 6)
    {
        pLoadRiderWorkArea->m0_riderModel = rider2Model;
        if (riderType == 1)
        {
            pLoadRiderWorkArea->m_14 = 0x24;
        }
        else
        {
            pLoadRiderWorkArea->m_14 = 0;
        }

        loadFile(r13->m_MCBName, rider2Model, rider2VRam);

        if (r13->m_CGBName)
        {
            loadFile(r13->m_CGBName, rider2VRam, 0);
        }
    }
    else
    {
        assert(0);
    }

    u8* pModel = pLoadRiderWorkArea->m0_riderModel;
    u8* pModelData2 = pModel + READ_BE_U32(pModel + r13->m_flags2);

    init3DModelRawData(pLoadRiderWorkArea, &pLoadRiderWorkArea->m18_3dModel, 0, pModel, pLoadRiderWorkArea->m_modelIndex, pModelData1, pModelData2, 0, r13->m_pExtraData);

    return pLoadRiderWorkArea;
}

void loadCurrentRider(s_workArea* pWorkArea)
{
    loadRider(pWorkArea, mainGameState.gameStats.m2_rider1);
}

void loadCurrentRider2(s_workArea* pWorkArea)
{
    loadRider2(pWorkArea, mainGameState.gameStats.m3_rider2);
}

void updateDragonIfCursorChanged(u32 level)
{
    assert(0);
}

void loadRiderIfChanged(u32 rider)
{
    assert(0);
}

void loadRider2IfChanged(u32 rider)
{
    assert(0);
}

void freeRamResource()
{
    yLog("Unimplemented freeRamResource");
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

    pVdp2StringControl->field_10 = filename;
    pVdp2StringControl->field_15 |= 1;
    return index;
}

void unloadFnt(const char*)
{
    yLog("Unimplemented unloadFnt");
}

u8 fieldSubTaskVar0;

void setFieldSubTaskVar0(u32 value)
{
    fieldSubTaskVar0 = value;
}

void fieldSubTaskInit(s_workArea* pWorkArea)
{
    s_FieldSubTaskWorkArea* pFieldSubTaskWorkArea = static_cast<s_FieldSubTaskWorkArea*>(pWorkArea);

    fieldTaskPtr->m8_pSubFieldData = pFieldSubTaskWorkArea;
    fieldTaskPtr->m8_pSubFieldData->fieldDebuggerWho = 0;

    setFieldSubTaskVar0(1);

    if ((fieldTaskPtr->m2C_currentFieldIndex != 8) && (fieldTaskPtr->m2C_currentFieldIndex != 12))
    {
        resetTempAllocators();
    }

    menuUnk0.m_48 = 0xC210;
    menuUnk0.m_4A = 0xC210;

    overlayStart_FLD_A3(pWorkArea, 0);

    fieldTaskPtr->fieldTaskState = 4;

    if (fieldTaskPtr->field_35)
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

void fieldSubTaskUpdate(s_workArea* pWorkArea)
{
    s_FieldSubTaskWorkArea* pFieldSubTaskWorkArea = static_cast<s_FieldSubTaskWorkArea*>(pWorkArea);

    mainGameState.setPackedBits(0, 2, 0);

    switch (pFieldSubTaskWorkArea->fieldSubTaskStatus)
    {
    case 0:
        if (openFileListHead == NULL)
        {
            if (!soundFunc1())
            {
                menuUnk0.m_4D = 6;

                if (menuUnk0.m_4D >= menuUnk0.m_4C)
                {
                    vdp2Controls.m_registers[0].N1COSL = 0x10;
                    vdp2Controls.m_registers[1].N1COSL = 0x10;
                }

                fadePalette(&menuUnk0.m_field0, titleScreenDrawSub1(&menuUnk0), menuUnk0.m_48, 30);
                fadePalette(&menuUnk0.m_field24, titleScreenDrawSub1(&menuUnk0), menuUnk0.m_4A, 30);

                pFieldSubTaskWorkArea->fieldSubTaskStatus++;
            }
        }
    default:
    case 1:
        mainGameState.gameStats.field_70++;
        pFieldSubTaskWorkArea->field_354++;

        if (readKeyboardToggle())
        {
            assert(0);
        }
        break;
    }
}

void fieldSubTaskDraw(s_workArea* pWorkArea)
{
    s_FieldSubTaskWorkArea* pFieldSubTaskWorkArea = static_cast<s_FieldSubTaskWorkArea*>(pWorkArea);

    if (pFieldSubTaskWorkArea->pUpdateFunction1)
    {
        pFieldSubTaskWorkArea->pUpdateFunction1();
    }

    if (pFieldSubTaskWorkArea->pUpdateFunction2)
    {
        pFieldSubTaskWorkArea->pUpdateFunction2();
    }

}

void fieldSubTaskDelete(s_workArea* pWorkArea)
{
    s_FieldSubTaskWorkArea* pFieldSubTaskWorkArea = static_cast<s_FieldSubTaskWorkArea*>(pWorkArea);

    assert(0);
}

s_taskDefinition fieldSubTaskDefinition = { fieldSubTaskInit, fieldSubTaskUpdate, fieldSubTaskDraw, fieldSubTaskDelete, "field sub task" };

s_taskDefinitionWithArg encounterTaskDefinition = { dummyTaskInitWithArg, dummyTaskUpdate, dummyTaskDraw, dummyTaskDelete, "encounter task" };

void createEncounterTask(s_workArea* pWorkArea)
{
    unimplemented("createEncounterTask");
    //createSiblingTaskWithArg(pWorkArea, &encounterTaskDefinition, new s_dummyWorkArea, pWorkArea);
}

void fieldStartOverlayTaskInit(s_workArea* pWorkArea)
{
    const s_fieldDefinition* pFieldDefinition = &fieldDefinitions[fieldTaskPtr->m2C_currentFieldIndex];

    fieldTaskPtr->m4_overlayTaskData = pWorkArea;

    createEncounterTask(pWorkArea);

    if(pFieldDefinition->m_fnt)
    {
        loadFnt(pFieldDefinition->m_fnt);
    }

    if(pFieldDefinition->m_prg)
    {
        //loadFile(pFieldDefinition->m_prg, NULL, 0);

        createSubTask(pWorkArea, &fieldSubTaskDefinition, new s_FieldSubTaskWorkArea);
    }
}

void fieldStartOverlayTaskDelete(s_workArea* pWorkArea)
{
    const s_fieldDefinition* pFieldDefinition = &fieldDefinitions[fieldTaskPtr->m2C_currentFieldIndex];

    if (pFieldDefinition->m_fnt)
    {
        unloadFnt(pFieldDefinition->m_fnt);
    }

    fieldTaskPtr->m4_overlayTaskData = NULL;
}

s_taskDefinition fieldStartOverlayTask = { fieldStartOverlayTaskInit, NULL, NULL, fieldStartOverlayTaskDelete, "field start overlay task" };

void fieldSub1TaskInit(s_workArea* pWorkArea)
{
    fieldTaskPtr->m0 = pWorkArea;
    createSubTask(pWorkArea, &fieldStartOverlayTask, new s_workArea);
}

void fieldSub1TaskDelete(s_workArea* pWorkArea)
{
    fieldTaskPtr->m0 = NULL;
}

s_taskDefinition fieldSub1TaskDefinition = { fieldSub1TaskInit, NULL, NULL, fieldSub1TaskDelete, "field sub1 task" };

void fieldSub0TaskInit(s_workArea* pWorkArea)
{
    fieldTaskVar0 = pWorkArea;
    createSubTask(pWorkArea, &fieldSub1TaskDefinition, new s_workArea);
}

void fieldSub0TaskDelete(s_workArea* pWorkArea)
{
    fieldTaskVar0 = NULL;
}

s_taskDefinition fieldSub0TaskDefinition = { fieldSub0TaskInit, NULL, NULL, fieldSub0TaskDelete, "field sub0 task" };

p_workArea fieldTaskUpdateSub0(u32 fieldIndexMenuSelection, u32 subFieldIndexMenuSelection, u32 field_3A, u32 currentSubFieldIndex)
{
    fieldTaskPtr->m2C_currentFieldIndex = fieldIndexMenuSelection;
    fieldTaskPtr->m2E_currentSubFieldIndex = subFieldIndexMenuSelection;
    fieldTaskPtr->field_30 = field_3A;
    fieldTaskPtr->m32 = currentSubFieldIndex;

    if (fieldTaskVar0 == NULL)
    {
        createSubTask(fieldTaskPtr, &fieldSub0TaskDefinition, new s_workArea);
    }
    else
    {
        if (fieldTaskPtr->m0 == NULL)
        {
            createSubTask(fieldTaskPtr, &fieldSub1TaskDefinition, new s_workArea);
        }
        else
        {
            createSubTask(fieldTaskPtr->m4_overlayTaskData, &fieldSubTaskDefinition, new s_FieldSubTaskWorkArea);
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

    mainGameState.gameStats.currentHP = mainGameState.gameStats.maxHP;
    mainGameState.gameStats.currentBP = mainGameState.gameStats.maxBP;

    freeRamResource();
}

void fieldTaskUpdate(p_workArea pTypelessWorkArea)
{
    s_fieldTaskWorkArea* pWorkArea = static_cast<s_fieldTaskWorkArea*>(pTypelessWorkArea);

    switch (pWorkArea->fieldTaskState)
    {
    case 0:
        pauseEngine[2] = 0;
        fieldTaskVar3 = 0;
        fieldInputTaskWorkArea = createFieldInputTask(pWorkArea);
        pWorkArea->fieldTaskState++;
        break;
    case 1: //do nothing
        break;
    case 2: //start field
        setupPlayer(pWorkArea->fieldIndexMenuSelection);
        vdp2DebugPrintSetPosition(3, 24);
        vdp2PrintStatus.palette = 0xD000;
        drawLineLargeFont("LOADING...");
        pWorkArea->fieldTaskState++;
    case 3:
        fieldTaskUpdateSub0(pWorkArea->fieldIndexMenuSelection, pWorkArea->subFieldIndexMenuSelection, pWorkArea->field_3A, pWorkArea->m2E_currentSubFieldIndex);

        if (pWorkArea->m8_pSubFieldData)
        {
            if (fieldInputTaskWorkArea)
            {
                fieldInputTaskWorkArea->getTask()->markFinished();
                fieldInputTaskWorkArea = NULL;
            }

            pauseEngine[2] = 1;
        }
        pWorkArea->fieldTaskState = 1;
        break;
    case 4:
        break;
    case 5:
        pauseEngine[2] = 0;
        pWorkArea->fieldTaskState++;
        break;
    case 6:
        if (pWorkArea->m8_pSubFieldData == NULL)
        {
            pWorkArea->fieldTaskState = 3;
        }
        break;
    case 7:
        if (fieldTaskVar0 == 0)
        {
            pWorkArea->fieldTaskState = 0;
        }
        break;
    default:
        assert(0);
        break;
    }
}
void fieldTaskDelete(p_workArea pTypelessWorkArea)
{
    assert(0);
}

s_taskDefinitionWithArg fieldTaskDefinition = { fieldTaskInit, fieldTaskUpdate, NULL, fieldTaskDelete, "field task" };

p_workArea createFieldTask(p_workArea pTypelessWorkArea, u32 arg)
{
    return createSubTaskWithArg(pTypelessWorkArea, &fieldTaskDefinition, new s_fieldTaskWorkArea, (void*)arg);
}

struct s_fieldDebugTaskWorkArea : public s_workArea
{
    p_workArea field_8; // 8
    // size: 0xC
};

void fieldDebugTaskInit(p_workArea pTypelessWorkArea)
{
    s_fieldDebugTaskWorkArea* pWorkArea = static_cast<s_fieldDebugTaskWorkArea*>(pTypelessWorkArea);

    pauseEngine[2] = 0;

    initNewGameState();
    pWorkArea->field_8 = createFieldTask(pTypelessWorkArea, 0);
    
    createLoadingTask(pWorkArea->field_8, 1);
    createMenuTask(pWorkArea->field_8);
    createSiblingTaskWithArg(pWorkArea->field_8, &flagEditTask, new s_flagEditTaskWorkArea, pWorkArea->field_8);
}

void genericTaskRestartGameWhenFinished(p_workArea pTypelessWorkArea)
{
    s_fieldDebugTaskWorkArea* pWorkArea = static_cast<s_fieldDebugTaskWorkArea*>(pTypelessWorkArea);

    if ((pWorkArea->field_8 == NULL) || pWorkArea->field_8->getTask()->isFinished())
    {
        initialTaskStatus.m_pendingTask = startSegaLogoModule;
    }
}

void genericOptionMenuDelete(p_workArea pTypelessWorkArea)
{
    assert(0);
}

s_taskDefinition fieldDebugModule = { fieldDebugTaskInit, NULL, genericTaskRestartGameWhenFinished, genericOptionMenuDelete, "fieldDebugTask" };


void townDebugTaskInit(p_workArea pTypelessWorkArea)
{
    s_fieldDebugTaskWorkArea* pWorkArea = static_cast<s_fieldDebugTaskWorkArea*>(pTypelessWorkArea);

    pauseEngine[2] = 0;

    initNewGameState();

    assert(0);
    /*
    pWorkArea->field_8 = createLocationTask(pWorkArea, 0);

    resetTempAllocators();
    initDramAllocator(pWorkArea->field_8, playerDataMemoryBuffer, 0x28000, NULL);

    loadDragon(pWorkArea->field_8);
    loadCurrentRider(pWorkArea->field_8);
    loadCurrentRider2(pWorkArea->field_8);
    freeRamResource();
    createMenuTask();
    createSiblingTaskWithArg(pWorkArea->field_8, flagEditTask, 0x10, pWorkArea->field_8);
    */
}

s_taskDefinition townDebugTask = { townDebugTaskInit, NULL, genericTaskRestartGameWhenFinished, genericOptionMenuDelete, "townDebugTask" };

p_workArea createTownDebugTask(p_workArea pWorkArea)
{
    return createSubTask(pWorkArea, &townDebugTask, new s_fieldDebugTaskWorkArea);
}
p_workArea createFieldDebugTask(p_workArea pWorkArea)
{
    return createSubTask(pWorkArea, &fieldDebugModule, new s_fieldDebugTaskWorkArea);
}

p_workArea createBattleDebugTask(p_workArea)
{
    assert(0);
    return NULL;
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

struct s_exitMenuTaskSub1Task : public s_workArea
{
    u32 state; // 0
    p_workArea field_8;
    u32 field_C;
};

struct {
    s8 m0_gameMode;
    s8 field_1;
    s8 field_2;
    s8 field_3;
    u16 m4_gameStatus;
    u16 m6_previousGameStatus;
    u16 m8_nextGameStatus;
} gGameStatus;

struct {
    u8 m8;
    u8 field_9;
    u8 field_A;
    u8 field_B;
} var_60525E8;

void exitMenuTaskSub1TaskInitSub2(u32 r4)
{
    if (gGameStatus.m8_nextGameStatus == 0)
    {
        gGameStatus.m8_nextGameStatus = r4;
        gGameStatus.field_2 = 0;
    }
}

u8 array_24BCA0[0x104];

void exitMenuTaskSub1TaskInitSub1()
{
    memset(array_24BCA0, 0, 0x104);
}

u8 array_250000[0x20000];

struct s_menuGraphicsTask : public s_workArea
{
    u32 state; // 0
    p_workArea field_4;
    p_workArea field_8;
    u8 field_C;
    u8 field_D;
};

void menuGraphicsTaskInit(s_workArea* pTypelessWorkArea, void* voidArgument)
{
    s_menuGraphicsTask* pWordArea = static_cast<s_menuGraphicsTask*>(pTypelessWorkArea);
    p_workArea parentTask = (p_workArea)voidArgument;

    pWordArea->field_4 = parentTask;
    graphicEngineStatus.field_40AC.field_C = pWordArea;
    graphicEngineStatus.field_40AC.m1_isMenuAllowed = 0;
    graphicEngineStatus.field_40AC.field_3 = 0;
    graphicEngineStatus.field_40AC.m0_menuId = 0;
    graphicEngineStatus.field_40AC.field_2 = 0;
    graphicEngineStatus.field_40AC.field_4 = 0;
    graphicEngineStatus.field_40AC.field_5 = 0;
    graphicEngineStatus.field_40AC.field_6 = 0;
    graphicEngineStatus.field_40AC.field_7 = 0;

    addToMemoryLayout(MENU_SCB, 0x14000);
    loadFile("MENU.SCB", MENU_SCB, 0);
    loadFile("MENU.CGB", getVdp1Pointer(0x25C10000), 0);
    graphicEngineStatus.field_40AC.fontIndex = loadFnt("MENU.FNT");
}

void menuGraphicsTaskDrawSub1()
{
    graphicEngineStatus.field_40E4 = (s_graphicEngineStatus_40E4*)allocateHeap(sizeof(s_graphicEngineStatus_40E4));
    assert(graphicEngineStatus.field_40E4);

    memcpy_dma(&graphicEngineStatus.field_405C, &graphicEngineStatus.field_40E4->field_0, sizeof(s_graphicEngineStatus_405C));
    memcpy_dma(&vdp2Controls, &graphicEngineStatus.field_40E4->field_50, sizeof(sVdp2Controls));
    memcpy_dma(&menuUnk0, &graphicEngineStatus.field_40E4->field_2B0, sizeof(sMenuUnk0));
    asyncDmaCopy(vdp2Palette, &graphicEngineStatus.field_40E4->field_300, 512, 0);

    u32 backScreenTableOffset = vdp2Controls.m_pendingVdp2Regs->BKTA & 0x7FFFF;
    graphicEngineStatus.field_40E4->field_400 = getVdp2VramU16(backScreenTableOffset);
    graphicEngineStatus.field_40E4->field_402 = pVdp2StringControl->f0_index;
}

sLayerConfig menuNBG01Setup[] =
{
    CHCN,  0, // 16 colors
    CHSZ,  1, // character size is 2 cells x 2 cells (16*16)
    PNB,  1, // pattern data size is 1 word
    CNSM,  0, // character number is 10 bit, flip
    SCN, 12,
    PLSZ, 0, // plane is 1H x 1V
    CAOS, 6, // palette offset is 6 * 0x200 = 0xC00
    END,
};

sLayerConfig menuNBG2Setup[] =
{
    CHCN,  0, // 16 colors
    CHSZ,  1, // character size is 2 cells x 2 cells (16*16)
    PNB,  1, // pattern data size is 1 word
    CNSM,  0, // character number is 10 bit, flip
    SCN, 12,
    PLSZ, 0, // plane is 1H x 1V
    CAOS, 6, // palette offset is 6 * 0x200 = 0xC00
    W0E, 1,
    W0A, 1,
    CCEN, 1,
    END,
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

    vdp2Controls.m_pendingVdp2Regs->CYCA0 = 0x3FFF7FFF;
    vdp2Controls.m_isDirty = 1;
    vdp2Controls.m_pendingVdp2Regs->CYCB1 = 0x12F456F;
    vdp2Controls.m_isDirty = 1;

    setupNBG0(menuNBG01Setup);
    setupNBG1(menuNBG01Setup);
    setupNBG2(menuNBG2Setup);

    initLayerMap(0, 0x71800, 0x71800, 0x71800, 0x71800);
    initLayerMap(1, 0x71000, 0x71000, 0x71000, 0x71000);
    initLayerMap(2, 0x70800, 0x70000, 0x70000, 0x70800);

    vdp2Controls.m_pendingVdp2Regs->PRINA = 0x304;
    vdp2Controls.m_pendingVdp2Regs->PRINB = 0x700;
    vdp2Controls.m_pendingVdp2Regs->PRIR = 0x0;

    vdp2Controls.m_pendingVdp2Regs->PRISA = 0x606;
    vdp2Controls.m_pendingVdp2Regs->PRISB = 0x606;
    vdp2Controls.m_pendingVdp2Regs->PRISC = 0x606;
    vdp2Controls.m_pendingVdp2Regs->PRISD = 0x606;

    vdp2Controls.m_isDirty = 1;

    setVdp2VramU16(0x7FFFE, 0x9402);

    vdp2Controls.m_pendingVdp2Regs->BKTA = (vdp2Controls.m_pendingVdp2Regs->BKTA & 0xFFF80000) | 0x3FFFF;

    vdp2Controls.m_pendingVdp2Regs->CCCTL &= 0xFEFF;

    loadFile("MENU.SCB", MENU_SCB, 0);
    loadFile("MENU.CGB", getVdp1Pointer(0x25C10000), 0);

    unpackGraphicsToVDP2(menuTilesLayout, getVdp2Vram(0x71800));

    {
        asyncDmaCopy(menuPalette, vdp2Palette, 256, 0);
    }
}

struct s_statusMenuTaskWorkArea : public s_workArea
{
    u32 selectedMenu; //0
};

struct s_MenuCursorWorkArea : public s_workArea
{
    s32 selectedMenu;
    s_graphicEngineStatus_40BC* field_4;
    u16* field_8;
    s32 field_C;
};

struct s_mainMenuWorkArea : public s_workArea
{
    u8 field_0; //0
    s8 field_1; //1
    s8 selectedMenu; //2
    s8 m3_menuButtonStates[5]; //3
    p_workArea field_8; // 8
    s_statusMenuTaskWorkArea* field_C; // C
    s_MenuCursorWorkArea* m10_cursorTask; // 10
};

void initVdp2ForStatusMenu()
{
    setVdp2LayerScroll(0, 160, 0);
    setVdp2LayerScroll(1, 0, 0x100);
    setVdp2LayerScroll(3, 0, 0x100);
    resetVdp2LayersAutoScroll();
    unpackGraphicsToVDP2(COMMON_DAT + 0xF7F4, getVdp2Vram(0x71400));
    setupVDP2StringRendering(0, 34, 44, 28);
    clearVdp2TextArea();
}

void mainMenuTaskInitSub3(u32 vdp2Offset, u32 r5, u32 r6, u32 r7)
{
    u32 r11 = 0xFFF;
    r7 <<= 8;
    for (int i = 0; i < r6; i++)
    {
        u32 r14 = vdp2Offset;
        u32 r10 = r5;

        for (int j = 0; j < r10; j++)
        {
            u16 r1 = getVdp2VramU16(r14);
            u16 r2 = r1;
            r2 &= 0xFFF;
            r1 ^= r1;
            if (r1)
            {
                r2 |= r7;
                setVdp2VramU16(r14, r2);
            }
            r14 += 2;
        }

        vdp2Offset += 0x40;
    }
}

struct sMainMenuTaskInitData2
{
    s_graphicEngineStatus_40BC* field_0;
    u16* field_4;
};

struct mainMenuTaskInitSub2TaskWorkArea : public s_workArea
{
    u32 spriteIndex;
    s_graphicEngineStatus_40BC* field_4;
    u16* spriteData;
    u32 field_C;
};

void mainMenuTaskInitSub2TaskInit(p_workArea typelessWorkArea, void* arg)
{
    mainMenuTaskInitSub2TaskWorkArea* pWorkArea = static_cast<mainMenuTaskInitSub2TaskWorkArea*>(typelessWorkArea);
    sMainMenuTaskInitData2* typedArg = static_cast<sMainMenuTaskInitData2*>(arg);

    pWorkArea->field_4 = typedArg->field_0;
    pWorkArea->spriteData = typedArg->field_4;
}

struct s_menuSprite
{
    s16 SRCA;
    s16 SIZE;
    s16 X;
    s16 Y;
};

s_menuSprite spriteData1[] =
{
    {0x2118, 0x520, 0, 0},
    {0x20D8, 0x420, 4, 0},
};


void drawMenuSprite(s_menuSprite* r4, s16 r5, s16 r6, u32 r7)
{
    u32 vdp1WriteEA = graphicEngineStatus.vdp1Context[0].currentVdp1WriteEA;

    setVdp1VramU16(vdp1WriteEA + 0x00, 0x1000); // command 0, normal sprite + JUMP
    setVdp1VramU16(vdp1WriteEA + 0x04, 0x80); // CMDPMOD
    setVdp1VramU16(vdp1WriteEA + 0x06, r7); // CMDCOLR
    setVdp1VramU16(vdp1WriteEA + 0x08, r4->SRCA); // CMDSRCA
    setVdp1VramU16(vdp1WriteEA + 0x0A, r4->SIZE); // CMDSIZE
    setVdp1VramU16(vdp1WriteEA + 0x0C, r4->X + r5 - 0xB0); // CMDXA
    setVdp1VramU16(vdp1WriteEA + 0x0E, r4->Y + r6 - 0x70); // CMDYA

    graphicEngineStatus.vdp1Context[0].pCurrentVdp1Packet->bucketTypes = 0;
    graphicEngineStatus.vdp1Context[0].pCurrentVdp1Packet->vdp1EA = vdp1WriteEA >> 3;
    graphicEngineStatus.vdp1Context[0].pCurrentVdp1Packet++;

    graphicEngineStatus.vdp1Context[0].field_1C += 1;
    graphicEngineStatus.vdp1Context[0].currentVdp1WriteEA = vdp1WriteEA + 0x20;
    graphicEngineStatus.vdp1Context[0].field_C += 1;
}

void mainMenuTaskInitSub2TaskDraw(p_workArea typelessWorkArea)
{
    mainMenuTaskInitSub2TaskWorkArea* pWorkArea = static_cast<mainMenuTaskInitSub2TaskWorkArea*>(typelessWorkArea);

    u32 spriteColor;
    if (pWorkArea->field_C)
    {
        spriteColor = 0x650;
    }
    else
    {
        spriteColor = 0x630;
    }

    s16 r5 = pWorkArea->spriteData[pWorkArea->spriteIndex * 2 + 0] - pWorkArea->field_4->scrollX;
    s16 r6 = pWorkArea->spriteData[pWorkArea->spriteIndex * 2 + 1] - pWorkArea->field_4->scrollY;

    drawMenuSprite(&spriteData1[pWorkArea->field_C], r5, r6, spriteColor);
}

s_taskDefinitionWithArg mainMenuTaskInitSub2TaskDef = { mainMenuTaskInitSub2TaskInit, NULL, mainMenuTaskInitSub2TaskDraw, NULL, "mainMenuTaskInitSub2Task" };

mainMenuTaskInitSub2TaskWorkArea* mainMenuTaskInitSub2(p_workArea typelessWorkArea, sMainMenuTaskInitData2* r5, u32 r6)
{
    p_workArea pNewTask = createSubTaskWithArg(typelessWorkArea, &mainMenuTaskInitSub2TaskDef, new mainMenuTaskInitSub2TaskWorkArea, r5);
    mainMenuTaskInitSub2TaskWorkArea* pTypedNewTask = static_cast<mainMenuTaskInitSub2TaskWorkArea*>(pNewTask);
    pTypedNewTask->field_C = r6;
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
    &graphicEngineStatus.layersConfig[1],
    mainMenuTaskInitData2_
};

struct s_menuDragonCrestTaskWorkArea : public s_workArea
{
    u32 field_0;
    s_graphicEngineStatus_40BC* field_4;
};

void menuDragonCrestTaskInit(s_workArea* pTypelessWorkArea, void* arg)
{
    s_menuDragonCrestTaskWorkArea* pWorkArea = static_cast<s_menuDragonCrestTaskWorkArea*>(pTypelessWorkArea);
    pWorkArea->field_4 = static_cast<s_graphicEngineStatus_40BC*>(arg);
}

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

void menuDragonCrestTaskDraw(s_workArea* pTypelessWorkArea)
{
    s_menuDragonCrestTaskWorkArea* pWorkArea = static_cast<s_menuDragonCrestTaskWorkArea*>(pTypelessWorkArea);
    if (graphicEngineStatus.field_40AC.field_5)
    {
        for (int i = 0; i < 10; i++)
        {
            unimplemented("Missing filter login in menuDragonCrestTaskDraw");

            drawMenuSprite(&menuDragonCrestSprites[i], -pWorkArea->field_4->scrollX, -pWorkArea->field_4->scrollY, 0x610);
        }
    }
}

struct s_laserRankTaskWorkArea : public s_workArea
{
    u32 field_0;
    s_graphicEngineStatus_40BC* field_4;
};

void laserRangTaskInit(s_workArea* pTypelessWorkArea, void* arg)
{
    s_laserRankTaskWorkArea* pWorkArea = static_cast<s_laserRankTaskWorkArea*>(pTypelessWorkArea);
    pWorkArea->field_4 = static_cast<s_graphicEngineStatus_40BC*>(arg);
}

s_menuSprite laserRankSpriteDefinition[6] = {
    { 0x2378, 0x20B, 0x80, 0x19C },
    { 0x2384, 0x213, 0x8A, 0x1A3 },
    { 0x2398, 0x20B, 0x80, 0x1B2 },
    { 0x23A4, 0x20A, 0x70, 0x1B3 },
    { 0x23B0, 0x213, 0x6C, 0x1A3 },
    { 0x23C4, 0x20A, 0x70, 0x19C },
};

void laserRangTaskDraw(s_workArea* pTypelessWorkArea)
{
    s_laserRankTaskWorkArea* pWorkArea = static_cast<s_laserRankTaskWorkArea*>(pTypelessWorkArea);
    if (pWorkArea->field_0 < 0)
        return;
    if (mainGameState.gameStats.m1_dragonLevel > 7)
        return;

    u8 r14 = mainGameState.gameStats.m1_dragonLevel;
    if (r14 > 6)
        r14 = 6;

    for (int i = 0; i < r14; i++)
    {
        drawMenuSprite(&laserRankSpriteDefinition[i], -pWorkArea->field_4->scrollX, -pWorkArea->field_4->scrollY, 0x760);
    }
}

s_taskDefinitionWithArg menuDragonCrestTaskDef = { menuDragonCrestTaskInit, NULL, menuDragonCrestTaskDraw, NULL, "menuDragonCrestTaskDef" };
s_taskDefinitionWithArg laserRangTaskDefinition = { laserRangTaskInit, NULL, laserRangTaskDraw, NULL, "laserRangTaskDefinition" };

struct s_mainMenuTaskInitSub4SubWorkArea : public s_workArea
{
    u32 _0;
    s32 _4;
};

void mainMenuTaskInitSub4Sub(p_workArea typelessWorkArea)
{
    s_mainMenuTaskInitSub4SubWorkArea* pWorkArea = static_cast<s_mainMenuTaskInitSub4SubWorkArea*>(typelessWorkArea);

    if (--pWorkArea->_4 > 0)
    {
        return;
    }

    pWorkArea->_4 = 4;

    if (++pWorkArea->_0 >= 14)
    {
        pWorkArea->_0 = 0;
    }

    asyncDmaCopy(COMMON_DAT + 0xDB9C + pWorkArea->_0 * 14, getVdp2Cram(0xE00 + 0xC4), 14, 0);
}

void mainMenuTaskInitSub4(p_workArea typelessWorkArea)
{
    createSubTaskFromFunction(typelessWorkArea, mainMenuTaskInitSub4Sub, new s_mainMenuTaskInitSub4SubWorkArea, "mainMenuTaskInitSub4Sub");
}

void menuCursorTaskInit(p_workArea typelessWorkArea, void* r5)
{
    s_MenuCursorWorkArea* pWorkArea = static_cast<s_MenuCursorWorkArea*>(typelessWorkArea);
    sMainMenuTaskInitData2* pMenuData = static_cast<sMainMenuTaskInitData2*>(r5);

    pWorkArea->field_4 = pMenuData->field_0;
    pWorkArea->field_8 = pMenuData->field_4;
}

void menuCursorTaskUpdate(p_workArea typelessWorkArea)
{
    s_MenuCursorWorkArea* pWorkArea = static_cast<s_MenuCursorWorkArea*>(typelessWorkArea);
    if (--pWorkArea->field_C < 0)
    {
        pWorkArea->field_C = 40;
    }
}

s_menuSprite cursorSpriteDef0 = { 0x2080, 0x520, 0, 0};
s_menuSprite cursorSpriteDef1 = { 0x2030, 0x520, 0, 0 };

void menuCursorTaskDraw(p_workArea typelessWorkArea)
{
    s_MenuCursorWorkArea* pWorkArea = static_cast<s_MenuCursorWorkArea*>(typelessWorkArea);

    if (pWorkArea->selectedMenu < 0)
        return;

    s32 X = pWorkArea->field_8[pWorkArea->selectedMenu*2 + 0] - pWorkArea->field_4->scrollX;
    s32 Y = pWorkArea->field_8[pWorkArea->selectedMenu*2 + 1] - pWorkArea->field_4->scrollY;

    if (pWorkArea->field_C > 20)
    {
        drawMenuSprite(&cursorSpriteDef0, X, Y, 0x760);
    }

    drawMenuSprite(&cursorSpriteDef1, X, Y, 0x760);
}

s_taskDefinitionWithArg menuCursorTaskDefinition = { menuCursorTaskInit , menuCursorTaskUpdate, menuCursorTaskDraw, NULL, "menuCursorTask" };

s_MenuCursorWorkArea* createMenuCursorTask(p_workArea pWorkArea, sMainMenuTaskInitData2* r5)
{
    return static_cast<s_MenuCursorWorkArea*>(createSubTaskWithArg(pWorkArea, &menuCursorTaskDefinition, new s_MenuCursorWorkArea, r5));
}

u32 mainMenuTaskInitData1[5] = {
    0x71450,
    0x714D6,
    0x71510,
    0x71596,
    0x715D0,
};

void mainMenuTaskInit(p_workArea typelessWorkArea)
{
    s_mainMenuWorkArea* pWorkArea = static_cast<s_mainMenuWorkArea*>(typelessWorkArea);

    pWorkArea->m3_menuButtonStates[0] = 1; // item is always enabled

    if (mainGameState.getBit(4, 2)) // dragon menu
    {
        pWorkArea->m3_menuButtonStates[1] = 1;
    }
    else
    {
        pWorkArea->m3_menuButtonStates[1] = -1;
    }

    if (mainGameState.gameStats.XP) // defeated monsters menu
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
                mainMenuTaskInitSub3(mainMenuTaskInitData1[i], 3, 3, 0x620);
            }
            else
            {
                mainMenuTaskInitSub3(mainMenuTaskInitData1[i], 3, 3, 0x660);
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
        pWorkArea->m10_cursorTask->selectedMenu = r14;
    }

    createSubTaskWithArg(pWorkArea, &menuDragonCrestTaskDef, new s_menuDragonCrestTaskWorkArea, &graphicEngineStatus.layersConfig[0]);
    createSubTaskWithArg(pWorkArea, &laserRangTaskDefinition, new s_laserRankTaskWorkArea, &graphicEngineStatus.layersConfig[1]);

    mainMenuTaskInitSub4(pWorkArea);

    pWorkArea->field_1 = vblankData.field_14;

    vblankData.field_14 = 1;

    fadePalette(&menuUnk0.m_field0, 0xC210, 0xC210, 1);
    fadePalette(&menuUnk0.m_field24, 0xC210, 0xC210, 1);
}

void statusMenuTaskInit(p_workArea pWorkArea)
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

void statusMenuTaskDraw(p_workArea typelessWorkArea)
{
    s_statusMenuTaskWorkArea* pWorkArea = static_cast<s_statusMenuTaskWorkArea*>(typelessWorkArea);
    
    vdp2StringContext.field_0 = 0;
    setActiveFont(graphicEngineStatus.field_40AC.fontIndex);
    vdp2PrintStatus.palette = 0xC000;

    const char** menuText = statusMenuOptionsDragonType;

    if ((mainGameState.gameStats.m1_dragonLevel == DR_LEVEL_0_BASIC_WING) || (mainGameState.gameStats.m1_dragonLevel == DR_LEVEL_6_LIGHT_WING) || (mainGameState.gameStats.m1_dragonLevel >= DR_LEVEL_7_SOLO_WING))
    {
        menuText = statusMenuOptionsAbilities;
    }

    if (pWorkArea->selectedMenu >= 0)
    {
        vdp2StringContext.cursorX = vdp2StringContext.X + 3;
        vdp2StringContext.cursorY = vdp2StringContext.Y + 0x17;

        drawInventoryString(menuText[pWorkArea->selectedMenu]);
    }

    vdp2DebugPrintSetPosition(3, 39);
    vdp2PrintfLargeFont("HP  %4d/%4d", mainGameState.gameStats.currentHP, mainGameState.gameStats.maxHP);

    vdp2DebugPrintSetPosition(3, 41);
    vdp2PrintfLargeFont("BP  %4d/%4d", mainGameState.gameStats.currentBP, mainGameState.gameStats.maxBP);

    vdp2DebugPrintSetPosition(3, 49);
    vdp2PrintfLargeFont("DYNE %8d", mainGameState.gameStats.dyne);

    if (mainGameState.gameStats.m1_dragonLevel < DR_LEVEL_8_FLOATER)
    {
        unimplemented("draw level curve");
    }
    else
    {
        vdp2StringContext.cursorX = vdp2StringContext.X + 3;
        vdp2StringContext.cursorY = vdp2StringContext.Y + 2;

        drawInventoryString("  Floater ");
    }
}

void clearVdp2Menu()
{
    memset(getVdp2Vram(0x7000), 0, 0x100 * 0x10);
}

void statusMenuTaskDelete(p_workArea pWorkArea)
{
    clearVdp2Menu();
}

s_taskDefinition statusMenuTaskDefinition = { statusMenuTaskInit, NULL, statusMenuTaskDraw, statusMenuTaskDelete, "statusMenuTask" };

void mainMenuTaskDraw(p_workArea typelessWorkArea)
{
    s_mainMenuWorkArea* pWorkArea = static_cast<s_mainMenuWorkArea*>(typelessWorkArea);

    switch (pWorkArea->field_0)
    {
    case 0:
        pWorkArea->field_C = static_cast<s_statusMenuTaskWorkArea*>(createSubTask(pWorkArea, &statusMenuTaskDefinition, new s_statusMenuTaskWorkArea));
        pWorkArea->field_C->selectedMenu = pWorkArea->selectedMenu;
        pWorkArea->m10_cursorTask->selectedMenu = pWorkArea->selectedMenu;
        pWorkArea->field_0++;
    case 1:
        if (graphicEngineStatus.m4514.m0[0].m0_current.m8_newButtonDown & 1) // B
        {
            playSoundEffect(1);
            fadePalette(&menuUnk0.m_field0, 0, 0, 1);
            fadePalette(&menuUnk0.m_field24, 0, 0, 1);
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
        if (graphicEngineStatus.m4514.m0[0].m0_current.m8_newButtonDown & 6) // A or C
        {
            if (pWorkArea->m3_menuButtonStates[pWorkArea->selectedMenu] == 0)
            {
                playSoundEffect(5);
                return;
            }
            playSoundEffect(0);
            if (pWorkArea->field_C)
            {
                pWorkArea->field_C->getTask()->markFinished();
            }

            pWorkArea->field_8 = statusMenuSubMenus[pWorkArea->selectedMenu](pWorkArea);
            pWorkArea->field_0++;
            return;
        }
        if (graphicEngineStatus.m4514.m0[0].m0_current.m8_newButtonDown & 0x30) // UP or DOWN
        {
            playSoundEffect(10);
            
            s32 selectedMenu = pWorkArea->selectedMenu;

            if (pWorkArea->m3_menuButtonStates[selectedMenu] == 1)
            {
                mainMenuTaskInitSub3(mainMenuTaskInitData1[selectedMenu], 3, 3, 0x660);
            }

            if (graphicEngineStatus.m4514.m0[0].m0_current.m8_newButtonDown & 0x10) // UP
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
                mainMenuTaskInitSub3(mainMenuTaskInitData1[selectedMenu], 3, 3, 0x620);
            }

            pWorkArea->selectedMenu = selectedMenu;
            pWorkArea->field_C->selectedMenu = pWorkArea->selectedMenu;
            pWorkArea->m10_cursorTask->selectedMenu = pWorkArea->selectedMenu;
        }
        break;
    case 2:
        if (pWorkArea->field_8)
        {
            if (pWorkArea->field_8->getTask()->isFinished())
            {
                pWorkArea->field_0 = 0;
            }
        }
        break;
    default:
        assert(0);
        break;
    }
}

void mainMenuTaskDelete(p_workArea typelessWorkArea)
{
    unimplemented("mainMenuTaskDelete");
}

s_taskDefinition mainMenuTaskDefinition = { mainMenuTaskInit, NULL, mainMenuTaskDraw, mainMenuTaskDelete, "main menu task" };

p_workArea createMainMenuTask(p_workArea workArea)
{
    return createSubTask(workArea, &mainMenuTaskDefinition, new s_mainMenuWorkArea);
}

p_workArea createInventoryMenuTask(p_workArea workArea)
{
    unimplemented("createInventoryMenuTask");
    return createMainDragonMenuTask(workArea);
    assert(0);
    return NULL;
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

p_workArea createLoadTask(p_workArea workArea)
{
    assert(0);
    return NULL;
}

p_workArea createSaveTask(p_workArea workArea)
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
    if (graphicEngineStatus.field_40E4)
    {
        memcpy_dma(&graphicEngineStatus.field_40E4->field_0, &graphicEngineStatus.field_405C, sizeof(s_graphicEngineStatus_405C));

        setActiveFont(graphicEngineStatus.field_40E4->field_402);
    }
}

void scrollMenu()
{
    for (int i = 0; i < 4; i++)
    {
        if (graphicEngineStatus.layersConfig[i].m8_scrollFrameCount)
        {
            graphicEngineStatus.layersConfig[i].scrollX += graphicEngineStatus.layersConfig[i].scrollIncX;
            graphicEngineStatus.layersConfig[i].scrollY += graphicEngineStatus.layersConfig[i].scrollIncY;
            graphicEngineStatus.layersConfig[i].m8_scrollFrameCount--;
        }
    }

    pauseEngine[4] = 0;
    setupVDP2CoordinatesIncrement2(graphicEngineStatus.layersConfig[0].scrollX << 16, graphicEngineStatus.layersConfig[0].scrollY << 16);

    pauseEngine[4] = 1;
    setupVDP2CoordinatesIncrement2(graphicEngineStatus.layersConfig[1].scrollX << 16, graphicEngineStatus.layersConfig[1].scrollY << 16);

    pauseEngine[4] = 2;
    setupVDP2CoordinatesIncrement2(graphicEngineStatus.layersConfig[2].scrollX, graphicEngineStatus.layersConfig[2].scrollY);

    pauseEngine[4] = 3;
    setupVDP2CoordinatesIncrement2(graphicEngineStatus.layersConfig[3].scrollX, graphicEngineStatus.layersConfig[3].scrollY);

    pauseEngine[4] = 4;
}

void menuGraphicsTaskDrawSub3()
{
    if (graphicEngineStatus.field_40E4)
    {
        memcpy_dma(&graphicEngineStatus.field_40E4->field_50, &vdp2Controls, sizeof(sVdp2Controls));
        memcpy_dma(&graphicEngineStatus.field_40E4->field_2B0, &menuUnk0, sizeof(sMenuUnk0));
        asyncDmaCopy(&graphicEngineStatus.field_40E4->field_300, vdp2Palette, 512, 0);

        u32 backScreenTableOffset = vdp2Controls.m_pendingVdp2Regs->BKTA & 0x7FFFF;
        setVdp2VramU16(backScreenTableOffset, graphicEngineStatus.field_40E4->field_400);

        u32 r3 = vdp2Controls.m_pendingVdp2Regs->BKTA & 0xFFF80000;
        u32 r2 = vdp2Controls.m_pendingVdp2Regs->BKTA & 0x7FFFF;
        vdp2Controls.m_pendingVdp2Regs->BKTA = r3 | ((r2 << 1) >> 1);
        freeHeap(graphicEngineStatus.field_40E4);
    }
}

void menuGraphicsTaskDraw(s_workArea* pTypelessWorkArea)
{
    s_menuGraphicsTask* pWordArea = static_cast<s_menuGraphicsTask*>(pTypelessWorkArea);
    
    // not exactly that in the original code, but same idea
    if ((pWordArea->field_4 == NULL) || pWordArea->field_4->getTask()->isFinished())
    {
        pWordArea->getTask()->markFinished();
        return;
    }

    switch (pWordArea->state)
    {
    case 0:
        graphicEngineStatus.field_40AC.field_8 = 0;
        if (graphicEngineStatus.m4514.m0[0].m0_current.m8_newButtonDown & 8)
        {
            unimplemented("Hack: forcing menu enabled 1");
            graphicEngineStatus.field_40AC.m1_isMenuAllowed = 1;
            if (graphicEngineStatus.field_40AC.m1_isMenuAllowed)
            {
                graphicEngineStatus.field_40AC.m0_menuId = 1;
                playSoundEffect(0);
            }
            else
            {
                if (graphicEngineStatus.field_40AC.m0_menuId == 0)
                    return;
            }
        }

        // enter menu
        // this isn't exactly correct
        if (graphicEngineStatus.field_40AC.m0_menuId)
        {
            graphicEngineStatus.field_40AC.field_8 = 1;
            graphicEngineStatus.field_40AC.field_9 = 1;

            // pause the gameplay system
            pWordArea->field_4->getTask()->markPaused();

            menuGraphicsTaskDrawSub1();
            fadePalette(&menuUnk0.m_field0, 0, 0, 1);
            fadePalette(&menuUnk0.m_field24, 0, 0, 1);
            pWordArea->state++;
        }
        break;
    case 1:
        if (fileInfoStruct.allocatedHead == NULL) // wait for loading to finish
        {
            graphicEngineStatus.field_40AC.field_2 = 0;
            setupVdp2ForMenu();

            pWordArea->field_8 = menuTaskMenuArray[graphicEngineStatus.field_40AC.m0_menuId](pWordArea);
            pWordArea->state++;
        }
        break;
    case 2:
        if (pWordArea->field_8 && !pWordArea->field_8->getTask()->isFinished())
        {
            if (!graphicEngineStatus.field_40AC.field_4)
            {
                scrollMenu();
            }
            if (graphicEngineStatus.m4514.m0[0].m0_current.m8_newButtonDown & 8) // start
            {
                if (graphicEngineStatus.field_40AC.m0_menuId == 7)
                {
                    return;
                }
                if (graphicEngineStatus.field_40AC.field_3)
                {
                    playSoundEffect(5);
                    return;
                }
            }
            if (graphicEngineStatus.field_40AC.field_2 == 0)
            {
                return;
            }
            playSoundEffect(4);
            fadePalette(&menuUnk0.m_field0, 0, 0, 1);
            fadePalette(&menuUnk0.m_field24, 0, 0, 1);
            pWordArea->state++;
        }
        else
        {
            menuGraphicsTaskDrawSub2();

            graphicEngineStatus.field_40AC.m0_menuId = 0;
            pWordArea->field_C = pauseEngine[0];

            pWordArea->field_4->getTask()->clearPaused();

            pWordArea->field_D = graphicEngineStatus.field_40AC.field_9;
            pWordArea->state = 5;
        }
        break;
    case 3:
        if (pWordArea->field_8)
        {
            pWordArea->field_8->getTask()->markFinished();
        }
        pWordArea->state++;
        break;
    case 4:
        menuGraphicsTaskDrawSub2();

        graphicEngineStatus.field_40AC.m0_menuId = 0;
        pWordArea->field_C = pauseEngine[0];

        pWordArea->field_4->getTask()->clearPaused();

        pWordArea->field_D = graphicEngineStatus.field_40AC.field_9;
        pWordArea->state = 5;
        break;
    case 5:
        if (--pWordArea->field_D == 0)
        {
            if (pWordArea->field_C)
            {
                pauseEngine[0] = 1;
            }
            else
            {
                pauseEngine[0] = 0;
            }
            menuGraphicsTaskDrawSub3();

            graphicEngineStatus.field_40AC.field_8 = 2;
            pWordArea->state = 0;
        }
        break;
    default:
        assert(0);
    }
}

void menuGraphicsDelete(s_workArea* pTypelessWorkArea)
{
    s_menuGraphicsTask* pWordArea = static_cast<s_menuGraphicsTask*>(pTypelessWorkArea);
    assert(0);
}

s_taskDefinitionWithArg menuGraphicsTask = { menuGraphicsTaskInit, NULL, menuGraphicsTaskDraw, menuGraphicsDelete, "menuGraphicsTask" };

p_workArea createMenuTask(p_workArea parentTask)
{
    return createSiblingTaskWithArg(parentTask, &menuGraphicsTask, new s_menuGraphicsTask, parentTask);
}

void exitMenuTaskSub1TaskInit(s_workArea* pTypelessWorkArea, void* voidArgument)
{
    s_exitMenuTaskSub1Task* pWorkArea = static_cast<s_exitMenuTaskSub1Task*>(pTypelessWorkArea);
    u32 menuID = (u32)voidArgument;

    pWorkArea->field_8 = 0;
    pWorkArea->field_C = 0;

    gGameStatus.m0_gameMode = -1;
    gGameStatus.field_1 = -1;
    gGameStatus.field_3 = 0;
    gGameStatus.m4_gameStatus = 0;
    gGameStatus.m6_previousGameStatus = 0;
    gGameStatus.m8_nextGameStatus = 0;

    unimplemented("Hack: skip game status to first field");
    gGameStatus.m8_nextGameStatus = 0x50;

    if (menuID == 3)
    {
        mainGameState.gameStats.m1_dragonLevel = DR_LEVEL_1_VALIANT_WING;
    }
    else
    {
        mainGameState.gameStats.m1_dragonLevel = DR_LEVEL_0_BASIC_WING;
    }

    mainGameState.gameStats.m2_rider1 = 1;
    mainGameState.gameStats.m3_rider2 = 0;

    var_60525E8.m8 = 0;
    var_60525E8.field_9 = 0;
    var_60525E8.field_A = 0;
    var_60525E8.field_B = 0;

    exitMenuTaskSub1TaskInitSub1();

    createMenuTask(pWorkArea);
    createFieldTask(pWorkArea, 1);
    createLoadingTask(pWorkArea, 1);
    resetTempAllocators();
    initDramAllocator(pWorkArea, array_250000, 0x28000, 0);

    loadDragon(pWorkArea);
    loadCurrentRider(pWorkArea);
    loadCurrentRider2(pWorkArea);
    freeRamResource();

    if (keyboardIsKeyDown(0xF6))
    {
        assert(0);
    }

    switch (menuID)
    {
    case 0:
        return exitMenuTaskSub1TaskInitSub2(1);
    case 1:
        return exitMenuTaskSub1TaskInitSub2(0x4A);
    case 2:
        return exitMenuTaskSub1TaskInitSub2(0x71);
    case 3:
        return exitMenuTaskSub1TaskInitSub2(0x72);
    default:
        assert(0);
    }

    assert(0);
}

s_vblankData vblankData;

void exitMenuTaskSub1TaskUpdate(s_workArea* pTypelessWorkArea)
{
    mainGameState.gameStats.frameCounter += vblankData.field_C;
}

s32 exitMenuTaskSub1TaskDrawSub1(p_workArea pWorkArea, s32 index)
{
    p_workArea var_8 = pWorkArea;
    s32 var_C = index;
    s_gameStats* var_10 = &mainGameState.gameStats;
    s32 var_14 = mainGameState.gameStats.m1_dragonLevel;
    s32 var_18 = mainGameState.gameStats.m2_rider1;
    s32 r15 = mainGameState.gameStats.m3_rider2;

    switch (index)
    {
    case 0:
        break;
    default:
        assert(0);
    }

    // has dragon level changed?
    if (mainGameState.gameStats.m1_dragonLevel != var_14)
    {
        assert(0);
    }

    // has rider1 changed
    if (mainGameState.gameStats.m2_rider1 != var_18)
    {
        assert(0);
    }

    // has rider2 changed
    if (mainGameState.gameStats.m3_rider2 != r15)
    {
        assert(0);
    }

    return 0;
}

void stopAllSounds()
{
    unimplemented("stopAllSounds");
}

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

    if (gGameStatus.field_3 == 1)
    {
        assert(0);
    }

    return fieldTaskUpdateSub0(r5, 0, 0, -1);
}

p_workArea(*overlayDispatchTable[])(p_workArea, s32) = {
    NULL,
    NULL,
    NULL,
    loadField,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

void exitMenuTaskSub1TaskDraw(s_workArea* pTypelessWorkArea)
{
    s_exitMenuTaskSub1Task* pWorkArea = static_cast<s_exitMenuTaskSub1Task*>(pTypelessWorkArea);

    switch (pWorkArea->state)
    {
    case 0:
        if (keyboardIsKeyDown(0xE7) || keyboardIsKeyDown(0xE4))
        {
            assert(0);
        }

        // 602739A

        if (gGameStatus.m8_nextGameStatus)
        {
            if (gGameStatus.field_2 == 0)
            {
                if (pWorkArea->field_8)
                {
                    //60273AA
                    assert(0);
                }
            }
            else
            {
                //60273F4
                assert(0);
            }
        }
        else
        {
            //602745C
            if (pWorkArea->field_8)
            {
                if (!pWorkArea->field_8->getTask()->isFinished())
                {
                    return;
                }
            }
        }

        // 06027474
        graphicEngineStatus.field_4 = 1;
        pauseEngine[2] = 0;
        graphicEngineStatus.field_40AC.m1_isMenuAllowed = 0;
        gGameStatus.m0_gameMode = -1;
        pWorkArea->state++;
        break;
    case 1:
        if (gGameStatus.m8_nextGameStatus == 0)
        {
            assert(0);
        }
        if (*(COMMON_DAT + 0x12EAC + gGameStatus.m4_gameStatus * 2) == 4)
        {
            assert(0);
        }
        else
        {
            if (gGameStatus.field_2 == 0)
            {
                if (exitMenuTaskSub1TaskDrawSub1(pWorkArea, gGameStatus.m4_gameStatus) < 0)
                {
                    if (pWorkArea)
                    {
                        pWorkArea->getTask()->markFinished();
                    }
                    return;
                }
            }
        }
        switch (gGameStatus.m8_nextGameStatus)
        {
        case 1:
        case 2:
        case 80:
            if ((gGameStatus.field_2 == 0) && (gGameStatus.m6_previousGameStatus != 0x4F))
            {
                initFileLayoutTable();
            }
        case 8:
        case 25:
            stopAllSounds();
        case 12:
        case 22:
        case 37:
            fadePalette(&menuUnk0.m_field0, titleScreenDrawSub1(&menuUnk0), 0, 30);
            fadePalette(&menuUnk0.m_field24, titleScreenDrawSub1(&menuUnk0), 0, 30);
            break;
        default:
            assert(0);
            break;
        }

        gGameStatus.m6_previousGameStatus = gGameStatus.m4_gameStatus;
        gGameStatus.m4_gameStatus = gGameStatus.m8_nextGameStatus;
        gGameStatus.m8_nextGameStatus = 0;

        gGameStatus.m0_gameMode = *(COMMON_DAT + 0x12EAC + gGameStatus.m4_gameStatus * 2);
        gGameStatus.field_1 = *(COMMON_DAT + 0x12EAC + gGameStatus.m4_gameStatus * 2 + 1);

        if (gGameStatus.m6_previousGameStatus == 74)
        {
            gGameStatus.field_3 = 1;
        }
        else
        {
            gGameStatus.field_3 = 0;
        }

        pWorkArea->field_8 = overlayDispatchTable[gGameStatus.m0_gameMode](pWorkArea, gGameStatus.field_1);
        pWorkArea->state = 0;

        break;
    default:
        assert(0);
    }
}

s_taskDefinitionWithArg exitMenuTaskSub1Task = { exitMenuTaskSub1TaskInit, exitMenuTaskSub1TaskUpdate, exitMenuTaskSub1TaskDraw, NULL, "exitMenuTaskSub1Task" };

p_workArea initExitMenuTaskSub1(p_workArea pTypelessWorkArea, u32 menuID)
{
    return createSubTaskWithArg(pTypelessWorkArea, &exitMenuTaskSub1Task, new s_exitMenuTaskSub1Task, (void*)menuID);
}

void initExitMenuTask(p_workArea pTypelessWorkArea, void* argument)
{
    s_fieldDebugTaskWorkArea* pWorkArea = static_cast<s_fieldDebugTaskWorkArea*>(pTypelessWorkArea);
    u32 menuID = (u32)argument;

    pauseEngine[2] = 0;

    pWorkArea->field_8 = initExitMenuTaskSub1(pWorkArea, menuID);

    createSiblingTaskWithArg(pWorkArea->field_8, &flagEditTask, new s_flagEditTaskWorkArea, pWorkArea->field_8);

    fadePalette(&menuUnk0.m_field0, 0x8000, 0x8000, 1);
    fadePalette(&menuUnk0.m_field24, 0x8000, 0x8000, 1);
}

s_taskDefinitionWithArg exitMenuTask = { initExitMenuTask, NULL, genericTaskRestartGameWhenFinished, genericOptionMenuDelete, "exitMenuTask" };

p_workArea createNewGameTask(p_workArea pWorkArea)
{
    return createSubTaskWithArg(pWorkArea, &exitMenuTask, new s_fieldDebugTaskWorkArea, (void*)0);
}

p_workArea createContinueTask(p_workArea pWorkArea)
{
    return createSubTaskWithArg(pWorkArea, &exitMenuTask, new s_fieldDebugTaskWorkArea, (void*)1);
}

