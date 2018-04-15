#include "PDS.h"

p_workArea createLoadingTask(p_workArea parentTask)
{
    unimplemented("createLoadingTask");
    //createSiblingTaskWithArg(parentTask, &loadingTask, new s_dummyWorkArea, parentTask);

    return NULL;
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

    switch (mainGameState.gameStats.dragonLevel)
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

    if (gameStats.dragonLevel < DR_LEVEL_8_FLOATER)
    {
        gameStats.maxHP = gameStats.classMaxHP + dragonPerLevelMaxHPBP[gameStats.dragonLevel].maxHP;
        gameStats.maxBP = gameStats.classMaxBP + dragonPerLevelMaxHPBP[gameStats.dragonLevel].maxBP;
    }
    else
    {
        gameStats.maxHP = dragonPerLevelMaxHPBP[gameStats.dragonLevel].maxHP;
        gameStats.maxBP = dragonPerLevelMaxHPBP[gameStats.dragonLevel].maxBP;
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
        pWorkArea->fStatus = 0;
        pWorkArea->getTask()->m_pUpdate = updateFieldTaskNoBattleOverride;
    }
    else
    {
        pWorkArea->fStatus = 1;
    }
}

struct s_fieldDebugListWorkArea : public s_workArea
{
    u32 m_ticks; //0
    u32 m_selectedSubField; // 4
    u32 m_isSelectingSubfield; // 8
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

    pFieldTask->fieldIndexMenuSelection = pFieldTask->currentFieldIndex;
    pFieldTask->subFieldIndexMenuSelection = pFieldTask->currentSubFieldIndex;
    pFieldTask->field_3A = pFieldTask->field_30;
    pFieldTask->currentSubFieldIndex = -1;

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

void fieldDebugListTaskUpdate(p_workArea pTypelessWorkArea)
{
    s_fieldDebugListWorkArea* pWorkArea = static_cast<s_fieldDebugListWorkArea*>(pTypelessWorkArea);

    pWorkArea->m_ticks++;
    s_fieldTaskWorkArea*r14 = getFieldTaskPtr();

    u32 var_24 = performModulo(30, pWorkArea->m_ticks);

    if (pWorkArea->m_isSelectingSubfield)
    {
        assert(0);
    }
    else
    {
        if (graphicEngineStatus.field_4514[0].current.field_8 & 0x60)
        {
            pWorkArea->m_isSelectingSubfield = 1;
        }

        if (graphicEngineStatus.field_4514[0].current.field_C & 0x20) // bottom
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

        if (graphicEngineStatus.field_4514[0].current.field_C & 0x10) // up
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

    if (graphicEngineStatus.field_4514[0].current.field_C & 0x8000)
    {
        assert(0);
    }

    if (graphicEngineStatus.field_4514[0].current.field_C & 0x800)
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
            assert(pWorkArea->m_isSelectingSubfield >= 0);
            assert(pWorkArea->m_isSelectingSubfield <= 1);
            vdp2PrintStatus.palette = selectedColor[pWorkArea->m_isSelectingSubfield] << 12;
        }

        if (fieldEnabledTable[r12])
        {
            drawLineSmallFont(fieldDefinitions[r12].m_name);

            // if selecting field
            if (pWorkArea->m_isSelectingSubfield == 0)
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

    pWorkArea->m_selectedSubField = (r2 >> 4) << 4;

    clearVdp2StringFieldDebugList();

    u32 r12 = pWorkArea->m_selectedSubField;

    while(true)
    {
        u32 r4 = pWorkArea->m_selectedSubField + 16;
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

            if (r12 == r14->fieldIndexMenuSelection)
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

    if (pWorkArea->m_isSelectingSubfield)
    {
        vdp2PrintStatus.palette = 0x9000;
        vdp2DebugPrintSetPosition(0x12, pWorkArea->m_selectedSubField - r14->subFieldIndexMenuSelection + 3);
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

    u8 inputValue = graphicEngineStatus.field_4514[0].current.field_8 & 0xF;
    if (inputValue == 0)
    {
        return;
    }

    if (inputValue & 3)
    {
        assert(0); //060116DA
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
    sPoseData* pPoseData = pDragonStateData1->poseData;
    u8* r13 = pDragonStateData1->pCurrentAnimation + READ_BE_U32(pDragonStateData1->pCurrentAnimation + 8);
    u8* r4 = pDragonStateData1->pDefaultPose;

    if (pDragonStateData1->currentAnimationFrame & 1)
    {
        pPoseData->m_translation += pPoseData->halfTranslation;
        return;
    }

    //06022638
    if (pDragonStateData1->currentAnimationFrame)
    {
        pPoseData->m_translation += pPoseData->halfTranslation;

        s16 r3;
        if (READ_BE_U32(pDragonStateData1->pCurrentAnimation))
        {
            r3 = READ_BE_S16(pDragonStateData1->pCurrentAnimation + 4);
        }
        else
        {
            r3 = 0;
        }

        r3--;
        if (pDragonStateData1->currentAnimationFrame >= r3)
        {
            return;
        }

        pPoseData->halfTranslation[0] = stepAnimationTrack(pPoseData->field_48[0], pDragonStateData1->pCurrentAnimation + READ_BE_U32(r13 + 0x14), READ_BE_U16(r13 + 0)) / 2;
        pPoseData->halfTranslation[1] = stepAnimationTrack(pPoseData->field_48[1], pDragonStateData1->pCurrentAnimation + READ_BE_U32(r13 + 0x18), READ_BE_U16(r13 + 2)) / 2;
        pPoseData->halfTranslation[2] = stepAnimationTrack(pPoseData->field_48[2], pDragonStateData1->pCurrentAnimation + READ_BE_U32(r13 + 0x1C), READ_BE_U16(r13 + 4)) / 2;
    }
    else
    //0602265A
    if (pDragonStateData1->field_8 & 4)
    {
        assert(0);
    }
    else
    {
        pPoseData->m_translation[0] = READ_BE_U32(r4);
        pPoseData->m_translation[1] = READ_BE_U32(r4 + 4);
        pPoseData->m_translation[2] = READ_BE_U32(r4 + 8);

        stepAnimationTrack(pPoseData->field_48[0], pDragonStateData1->pCurrentAnimation + READ_BE_U32(r13 + 0x14), READ_BE_U16(r13 + 0));
        stepAnimationTrack(pPoseData->field_48[1], pDragonStateData1->pCurrentAnimation + READ_BE_U32(r13 + 0x18), READ_BE_U16(r13 + 2));
        stepAnimationTrack(pPoseData->field_48[2], pDragonStateData1->pCurrentAnimation + READ_BE_U32(r13 + 0x1C), READ_BE_U16(r13 + 4));
    }

    //60226BA
    s16 r3;
    if (READ_BE_U32(pDragonStateData1->pCurrentAnimation))
    {
        r3 = READ_BE_S16(pDragonStateData1->pCurrentAnimation + 4);
    }
    else
    {
        r3 = 0;
    }

    r3--;
    if (r3 >= pDragonStateData1->currentAnimationFrame)
    {
        pPoseData->halfTranslation[0] = stepAnimationTrack(pPoseData->field_48[0], pDragonStateData1->pCurrentAnimation + READ_BE_U32(r13 + 0x14), READ_BE_U16(r13 + 0)) / 2;
        pPoseData->halfTranslation[1] = stepAnimationTrack(pPoseData->field_48[1], pDragonStateData1->pCurrentAnimation + READ_BE_U32(r13 + 0x18), READ_BE_U16(r13 + 2)) / 2;
        pPoseData->halfTranslation[2] = stepAnimationTrack(pPoseData->field_48[2], pDragonStateData1->pCurrentAnimation + READ_BE_U32(r13 + 0x1C), READ_BE_U16(r13 + 4)) / 2;
    }
    
}

void transformAndAddVec(sVec3_FP& r4, sVec3_FP& r5, sMatrix4x3& r6)
{
    s64 mac = 0;
    mac += (s64)r6.matrix[0] * (s64)r4[0].asS32();
    mac += (s64)r6.matrix[1] * (s64)r4[1].asS32();
    mac += (s64)r6.matrix[2] * (s64)r4[2].asS32();
    r5[0] += mac >> 16;

    mac = 0;
    mac += (s64)r6.matrix[4] * (s64)r4[0].asS32();
    mac += (s64)r6.matrix[5] * (s64)r4[1].asS32();
    mac += (s64)r6.matrix[6] * (s64)r4[2].asS32();
    r5[1] += mac >> 16;

    mac = 0;
    mac += (s64)r6.matrix[8] * (s64)r4[0].asS32();
    mac += (s64)r6.matrix[9] * (s64)r4[1].asS32();
    mac += (s64)r6.matrix[10] * (s64)r4[2].asS32();
    r5[2] += mac >> 16;
}

void transformAndAddVecByCurrentMatrix(sVec3_FP* r4, sVec3_FP* r5)
{
    transformAndAddVec(*r4, *r5, *pCurrentMatrix);
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
    for (int i = 0; i < p3dModel->numBones; i++)
    {
        pPoseData[i].m_rotation += pPoseData[i].halfRotation;
    }
}

void (*modelMode4_position1)(s_3dModel*) = unimplementedUpdate;
void modelMode4_rotation(s_3dModel* p3dModel)
{
    sPoseData* pPoseData = p3dModel->poseData;
    if (p3dModel->currentAnimationFrame & 1)
    {
        addAnimationFrame(pPoseData, p3dModel);
        return;
    }

    if (p3dModel->currentAnimationFrame)
    {
        addAnimationFrame(pPoseData, p3dModel);

        if (READ_BE_U16(p3dModel->pCurrentAnimation + 4) - 1 > p3dModel->currentAnimationFrame)
        {
            u8* r13 = p3dModel->pCurrentAnimation + READ_BE_U32(p3dModel->pCurrentAnimation + 8);
            for (int i = 0; i < p3dModel->numBones; i++)
            {
                pPoseData[i].halfRotation.m_value[0] = stepAnimationTrack(pPoseData[i].field_48[3], p3dModel->pCurrentAnimation + READ_BE_U32(r13 + 0x20), READ_BE_U16(r13 + 6)) << 11;
                pPoseData[i].halfRotation.m_value[1] = stepAnimationTrack(pPoseData[i].field_48[4], p3dModel->pCurrentAnimation + READ_BE_U32(r13 + 0x24), READ_BE_U16(r13 + 8)) << 11;
                pPoseData[i].halfRotation.m_value[2] = stepAnimationTrack(pPoseData[i].field_48[5], p3dModel->pCurrentAnimation + READ_BE_U32(r13 + 0x28), READ_BE_U16(r13 + 10)) << 11;
                r13 += 0x38;
            }
        }
    }
    else
    {
        u8* r13 = p3dModel->pCurrentAnimation + READ_BE_U32(p3dModel->pCurrentAnimation + 8);
        for (int i = 0; i < p3dModel->numBones; i++)
        {
            assert(pPoseData[i].field_48[3].currentStep == 0);
            assert(pPoseData[i].field_48[4].currentStep == 0);
            assert(pPoseData[i].field_48[5].currentStep == 0);

            pPoseData[i].m_rotation.m_value[0] = stepAnimationTrack(pPoseData[i].field_48[3], p3dModel->pCurrentAnimation + READ_BE_U32(r13 + 0x20), READ_BE_U16(r13 + 6)) << 12;
            pPoseData[i].m_rotation.m_value[1] = stepAnimationTrack(pPoseData[i].field_48[4], p3dModel->pCurrentAnimation + READ_BE_U32(r13 + 0x24), READ_BE_U16(r13 + 8)) << 12;
            pPoseData[i].m_rotation.m_value[2] = stepAnimationTrack(pPoseData[i].field_48[5], p3dModel->pCurrentAnimation + READ_BE_U32(r13 + 0x28), READ_BE_U16(r13 + 10)) << 12;
            r13 += 0x38;
        }

        if (READ_BE_U16(p3dModel->pCurrentAnimation + 4) - 1 > p3dModel->currentAnimationFrame)
        {
            u8* r13 = p3dModel->pCurrentAnimation + READ_BE_U32(p3dModel->pCurrentAnimation + 8);
            for (int i = 0; i < p3dModel->numBones; i++)
            {
                pPoseData[i].halfRotation.m_value[0] = stepAnimationTrack(pPoseData[i].field_48[3], p3dModel->pCurrentAnimation + READ_BE_U32(r13 + 0x20), READ_BE_U16(r13 + 6)) << 11;
                pPoseData[i].halfRotation.m_value[1] = stepAnimationTrack(pPoseData[i].field_48[4], p3dModel->pCurrentAnimation + READ_BE_U32(r13 + 0x24), READ_BE_U16(r13 + 8)) << 11;
                pPoseData[i].halfRotation.m_value[2] = stepAnimationTrack(pPoseData[i].field_48[5], p3dModel->pCurrentAnimation + READ_BE_U32(r13 + 0x28), READ_BE_U16(r13 + 10)) << 11;
                r13 += 0x38;
            }
        }
    }
}
void (*modelMode4_scale)(s_3dModel*) = unimplementedUpdate;

void(*modelDrawFunction0)(s_3dModel*) = unimplementedDraw;
void modelDrawFunction1(s_3dModel* pDragonStateData1)
{
    sVec3_FP** var_0 = pDragonStateData1->field_44;
    sMatrix4x3* var_8 = pDragonStateData1->boneMatrices;
    const s_RiderDefinitionSub* var_4 = pDragonStateData1->field_40;

    if (pDragonStateData1->field_8 & 1)
    {
        u8* r4 = pDragonStateData1->pModelFile + READ_BE_U32(pDragonStateData1->pModelFile + pDragonStateData1->modelIndexOffset);
        submitModelToRendering(pDragonStateData1->pModelFile, r4, var_8, var_4, var_0);
    }
    else
    {
        u8* r4 = pDragonStateData1->pModelFile + READ_BE_U32(pDragonStateData1->pModelFile + pDragonStateData1->modelIndexOffset);
        modeDrawFunction1Sub2(pDragonStateData1->pModelFile, r4, var_8, var_4, var_0);
    }
}
void (*modelDrawFunction2)(s_3dModel*) = unimplementedDraw;
void (*modelDrawFunction3)(s_3dModel*) = unimplementedDraw;

void (*modelDrawFunction5)(s_3dModel*) = unimplementedDraw;
void (*modelDrawFunction6)(s_3dModel*) = unimplementedDraw;

void (*modelDrawFunction9)(s_3dModel*) = unimplementedDraw;
void (*modelDrawFunction10)(s_3dModel*) = unimplementedDraw;

void copyPosePosition(s_3dModel* pDragonStateData1)
{
    u8* r5 = pDragonStateData1->pDefaultPose;

    for (u32 i = 0; i < pDragonStateData1->numBones; i++)
    {
        pDragonStateData1->poseData[i].m_translation[0] = READ_BE_U32(r5 + 0);
        pDragonStateData1->poseData[i].m_translation[1] = READ_BE_U32(r5 + 4);
        pDragonStateData1->poseData[i].m_translation[2] = READ_BE_U32(r5 + 8);
        r5 += 0x24;
    }
}

void copyPoseRotation(s_3dModel* pDragonStateData1)
{
    u8* r5 = pDragonStateData1->pDefaultPose;

    for (u32 i = 0; i < pDragonStateData1->numBones; i++)
    {
        pDragonStateData1->poseData[i].m_rotation[0] = READ_BE_U32(r5 + 0xC);
        pDragonStateData1->poseData[i].m_rotation[1] = READ_BE_U32(r5 + 0x10);
        pDragonStateData1->poseData[i].m_rotation[2] = READ_BE_U32(r5 + 0x14);
        r5 += 0x24;
    }
}

void resetPoseScale(s_3dModel* pDragonStateData1)
{
    sPoseData* pOutputPose = pDragonStateData1->poseData;

    for (u32 i = 0; i < pDragonStateData1->numBones; i++)
    {
        pOutputPose->m_scale[0] = 0x10000;
        pOutputPose->m_scale[1] = 0x10000;
        pOutputPose->m_scale[2] = 0x10000;

        pOutputPose++;
    }
}

u32 createDragonStateSubData1Sub1Sub1(s_3dModel* p3dModel, u8* pModelData)
{
    u16 flags = READ_BE_U16(pModelData);

    switch (flags & 7)
    {
    case 4:
        if (p3dModel->field_A & 0x100)
        {
            p3dModel->positionUpdateFunction = modelMode4_position0;
        }
        else
        {
            p3dModel->positionUpdateFunction = modelMode4_position1;
        }
        p3dModel->rotationUpdateFunction = modelMode4_rotation;
        p3dModel->scaleUpdateFunction = modelMode4_scale;

        for (u32 i = 0; i < p3dModel->numBones; i++)
        {
            for (u32 j = 0; j < 9; j++)
            {
                p3dModel->poseData[i].field_48[j].currentStep = 0;
                p3dModel->poseData[i].field_48[j].delay = 0;
                p3dModel->poseData[i].field_48[j].value = 0;
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
    pDragonStateData1->pCurrentAnimation = pModelData1;
    pDragonStateData1->currentAnimationFrame = 0;

    u16 flags = READ_BE_U16(pModelData1);

    if ((flags & 8) || (pDragonStateData1->field_A & 0x100))
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
    pDragonStateData1->pCurrentAnimation = pModelData1;
    pDragonStateData1->currentAnimationFrame = 0;

    u16 flags = READ_BE_U16(pModelData1);

    if ((flags & 8) || (pDragonStateData1->field_A & 0x100))
    {
        pDragonStateData1->poseData->m_translation[0] = READ_BE_U32(pDragonStateData1->pDefaultPose + 0);
        pDragonStateData1->poseData->m_translation[1] = READ_BE_U32(pDragonStateData1->pDefaultPose + 4);
        pDragonStateData1->poseData->m_translation[2] = READ_BE_U32(pDragonStateData1->pDefaultPose + 8);
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
    if (pDragonStateData1->field_8 & 2)
    {
        if (pDragonStateData1->field_40)
        {
            if (pDragonStateData1->field_38)
            {
                pDragonStateData1->drawFunction = modelDrawFunction0;
            }
            else
            {
                pDragonStateData1->drawFunction = modelDrawFunction1;
            }
        }
        else
        {
            if (pDragonStateData1->field_38)
            {
                pDragonStateData1->drawFunction = modelDrawFunction2;
            }
            else
            {
                pDragonStateData1->drawFunction = modelDrawFunction3;
            }
        }
    }
    else
    {
        if (pDragonStateData1->field_40)
        {
            if (pDragonStateData1->field_A & 0x20)
            {
                assert(0);
            }
            else
            {
                if (pDragonStateData1->field_38)
                {
                    pDragonStateData1->drawFunction = modelDrawFunction5;
                }
                else
                {
                    pDragonStateData1->drawFunction = modelDrawFunction6;
                }
            }
        }
        else
        {
            if (pDragonStateData1->field_A & 0x20)
            {
                assert(0);
            }
            else
            {
                if (pDragonStateData1->field_38)
                {
                    pDragonStateData1->drawFunction = modelDrawFunction9;
                }
                else
                {
                    pDragonStateData1->drawFunction = modelDrawFunction10;
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
        if (pDragonStateData1->pCurrentAnimation == NULL)
        {
            assert(0);
        }
        else
        {
            if (READ_BE_U16(pDragonStateData1->pCurrentAnimation) != READ_BE_U16(r5))
            {
                assert(0); // untested
                pDragonStateData1->field_A &= 0xFFC7;
                pDragonStateData1->field_A |= READ_BE_U16(r5);
                initModelDrawFunction(pDragonStateData1);
                return dragonFieldTaskInitSub3Sub1Sub1(pDragonStateData1, r5);
            }
            // 6021728
            pDragonStateData1->pCurrentAnimation = r5;
            pDragonStateData1->currentAnimationFrame = 0;

            u16 r0 = READ_BE_U16(r5) & 7;
            if ((r0 != 1) && (r0 != 4) && (r0 != 5))
            {
                return 1;
            }

            for (int i = 0; i < pDragonStateData1->numBones; i++)
            {
                for (int j = 0; j < 9; j++)
                {
                    pDragonStateData1->poseData[i].field_48[j].currentStep = 0;
                    pDragonStateData1->poseData[i].field_48[j].delay = 0;
                    pDragonStateData1->poseData[i].field_48[j].value = 0;
                }
            }

            return 1;
        }
        assert(0);
    }
    assert(0);
}

u32 dragonFieldTaskInitSub3Sub2Sub1(s_3dModel* p3DModel)
{
    if ((p3DModel->field_A & 0x38) == 0)
    {
        return 0;
    }

    if (READ_BE_U16(p3DModel->pCurrentAnimation) & 8)
    {
        p3DModel->positionUpdateFunction(p3DModel);
    }

    if (READ_BE_U16(p3DModel->pCurrentAnimation) & 0x10)
    {
        p3DModel->rotationUpdateFunction(p3DModel);
    }

    if (READ_BE_U16(p3DModel->pCurrentAnimation) & 0x20)
    {
        p3DModel->scaleUpdateFunction, (p3DModel);
    }

    p3DModel->field_16 = p3DModel->currentAnimationFrame;
    p3DModel->currentAnimationFrame++;

    // animation reset
    if (READ_BE_U16(p3DModel->pCurrentAnimation + 4) < p3DModel->currentAnimationFrame)
    {
        p3DModel->currentAnimationFrame = 0;
    }

    return p3DModel->field_16;
}

void dragonFieldTaskInitSub3Sub2Sub2(s_3dModel* pDragonStateData1)
{
    if (pDragonStateData1->field_48)
    {
        assert(0);
    }
}

u32 dragonFieldTaskInitSub3Sub2(s_3dModel* pDragonStateData1)
{
    u32 r0 = dragonFieldTaskInitSub3Sub2Sub1(pDragonStateData1);
    dragonFieldTaskInitSub3Sub2Sub2(pDragonStateData1);

    return r0;
}

void countNumBonesInModel(s_3dModel* pDragonStateData1, u8* pDragonModelData, u8* pStartOfData)
{
    do
    {
        pDragonStateData1->numBones++;
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
    pDragonStateData1->field_40 = unkArg;

    pDragonStateData1->field_44 = static_cast<sVec3_FP**>(allocateHeapForTask(pDragonStateData1->pOwnerTask, pDragonStateData1->numBones * sizeof(sVec3_FP*)));
    if (pDragonStateData1->field_44 == NULL)
        return false;

    const s_RiderDefinitionSub* r12 = pDragonStateData1->field_40;

    for(u32 i=0; i<pDragonStateData1->numBones; i++)
    {
        if (r12->m_count > 0)
        {
            pDragonStateData1->field_44[i] = (sVec3_FP*)allocateHeapForTask(pDragonStateData1->pOwnerTask, r12->m_count * sizeof(sVec3_FP));
            if (pDragonStateData1->field_44[i] == NULL)
                return false;
        }
        else
        {
            pDragonStateData1->field_44[i] = NULL;
        }

        r12 ++;
    }

    return true;
}

bool init3DModelRawData(s_workArea* pWorkArea, s_3dModel* pDragonStateData1, u32 unkArg0, u8* pDragonModel, u16 modelIndexOffset, u8* pModelData1, u8* pDefaultPose, u32 unkArg2, const s_RiderDefinitionSub* unkArg3)
{
    pDragonStateData1->pOwnerTask = pWorkArea;
    pDragonStateData1->pModelFile = pDragonModel;
    pDragonStateData1->modelIndexOffset = modelIndexOffset;
    pDragonStateData1->pDefaultPose = pDefaultPose;
    pDragonStateData1->field_38 = unkArg2;
    pDragonStateData1->field_14 = 0;
    pDragonStateData1->field_16 = 0;
    pDragonStateData1->field_8 = 1;

    if (pModelData1)
    {
        pDragonStateData1->field_A = READ_BE_U16(pModelData1) | unkArg0;
        pDragonStateData1->numBones = READ_BE_U16(pModelData1 + 2);
    }
    else
    {
        pDragonStateData1->field_A = unkArg0;
        pDragonStateData1->numBones = 0;
        countNumBonesInModel(pDragonStateData1, pDragonModel + READ_BE_U32(pDragonModel + pDragonStateData1->modelIndexOffset), pDragonModel);
    }

    pDragonStateData1->poseData = static_cast<sPoseData*>(allocateHeapForTask(pWorkArea, pDragonStateData1->numBones * sizeof(sPoseData)));

    if (pDragonStateData1->field_A & 0x200)
    {
        pDragonStateData1->boneMatrices = static_cast<sMatrix4x3*>(allocateHeapForTask(pWorkArea, pDragonStateData1->numBones * sizeof(sMatrix4x3)));
        assert(pDragonStateData1->boneMatrices);

        pDragonStateData1->field_8 |= 2;
    }
    else
    {
        pDragonStateData1->boneMatrices = 0;
        pDragonStateData1->field_8 &= 0xFFFD;
    }

    if (unkArg3)
    {
        if (!createDragonStateSubData1Sub2(pDragonStateData1, unkArg3))
            return false;
    }
    else
    {
        pDragonStateData1->field_40 = 0;
    }

    if (pModelData1)
    {
        if (createDragonStateSubData1Sub1(pDragonStateData1, pModelData1) == 0)
            return false;
    }
    else
    {
        pDragonStateData1->pCurrentAnimation = NULL;
        pDragonStateData1->currentAnimationFrame = 0;

        copyPosePosition(pDragonStateData1);
        copyPoseRotation(pDragonStateData1);
        resetPoseScale(pDragonStateData1);
    }

    initModelDrawFunction(pDragonStateData1);

    pDragonStateData1->addToDisplayListFunction = addObjectToDrawList;

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
    subData->m_vec_0.zero();
    subData->m_vec_C.zero();
    subData->m_vec_18.zero();
    subData->m_vec_24.zero();

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
    p3DModelAnimData->animData = dragonAnims;

    p3DModelAnimData->countAnims = countNumAnimData(p3DModelAnimData, dragonAnims);

    p3DModelAnimData->boneMatrices = pDragonStateData1->boneMatrices;

    p3DModelAnimData->runtimeAnimData = static_cast<s_runtimeAnimData*>(allocateHeapForTask(pDragonState, p3DModelAnimData->countAnims * sizeof(s_runtimeAnimData)));

    initRuntimeAnimData(dragonAnims, p3DModelAnimData->runtimeAnimData);
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

    pDragonState->pDragonModelRawData = gDragonModel;
    pDragonState->dragonType = dragonLevel;
    pDragonState->modelIndex = pDragonData3->m_field_8[0].m_field_0[0];
    pDragonState->shadowModelIndex = pDragonData3->m_field_8[0].m_field_0[1];
    pDragonState->dragonAnimOffsets = pDragonAnimOffsets->m_data;
    pDragonState->dragonAnimCount = pDragonAnimOffsets->m_count;
    pDragonState->field_88 = 1;

    u8* pDragonModel = pDragonState->pDragonModelRawData;
    u8* pDefaultAnimationData = pDragonModel + READ_BE_U32(pDragonModel + pDragonState->dragonAnimOffsets[0]);
    u8* defaultPose = pDragonModel + READ_BE_U32(pDragonModel + pDragonData3->m_field_8[0].m_field_0[2]);

    init3DModelRawData(pDragonState, &pDragonState->dragon3dModel, 0x300, pDragonModel, pDragonState->modelIndex, pDefaultAnimationData, defaultPose, 0, pDragonData3->m_field_8[0].m_field_8);

    init3DModelAnims(pDragonState, &pDragonState->dragon3dModel, &pDragonState->animData, getDragonDataByIndex(dragonLevel));

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
    const sDragonData3* pDragonData3 = &dragonData3[mainGameState.gameStats.dragonLevel];

    loadDragonFiles(pWorkArea, mainGameState.gameStats.dragonLevel);

    updateDragonStatsFromLevel();

    gDragonState->cursorX = mainGameState.gameStats.dragonCursorX;
    gDragonState->cursorY = mainGameState.gameStats.dragonCursorY;
    gDragonState->dragonArchetype = mainGameState.gameStats.dragonArchetype;

    s_loadDragonWorkArea* pLoadDragonWorkArea = loadDragonModel(pWorkArea, mainGameState.gameStats.dragonLevel);

    morphDragon(pLoadDragonWorkArea, &gDragonState->dragon3dModel, pLoadDragonWorkArea->MCBOffsetInDram, pDragonData3, mainGameState.gameStats.dragonCursorX, mainGameState.gameStats.dragonCursorY);

    loadDragonSub1(pLoadDragonWorkArea);
}

const s_RiderDefinitionSub gEdgeExtraData[] =
{
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { { -1, &gCommonFile }, 1 },
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

struct s_loadRiderWorkArea : public s_workArea
{
    u8* m_riderModel; //0
    u32 m4; //4
    s_workArea* m_ParentWorkArea; //8
    u32 m_riderType; //C
    u32 m_modelIndex; // 10
    u32 m_14; //14


    s_3dModel m_3dModel;//18
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

    pRider2State = pLoadRiderWorkArea;

    if (riderType < 6)
    {
        pLoadRiderWorkArea->m_riderModel = riderModel;
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

    u8* pModel = pLoadRiderWorkArea->m_riderModel;
    u8* pDefaultPose = pModel + READ_BE_U32(pModel + r13->m_flags2);

    init3DModelRawData(pLoadRiderWorkArea, &pLoadRiderWorkArea->m_3dModel, 0, pModel, pLoadRiderWorkArea->m_modelIndex, pModelData1, pDefaultPose, 0, r13->m_pExtraData);

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

    pRiderState = pLoadRiderWorkArea;

    if (riderType < 6)
    {
        pLoadRiderWorkArea->m_riderModel = rider2Model;
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

    u8* pModel = pLoadRiderWorkArea->m_riderModel;
    u8* pModelData2 = pModel + READ_BE_U32(pModel + r13->m_flags2);

    init3DModelRawData(pLoadRiderWorkArea, &pLoadRiderWorkArea->m_3dModel, 0, pModel, pLoadRiderWorkArea->m_modelIndex, pModelData1, pModelData2, 0, r13->m_pExtraData);

    return pLoadRiderWorkArea;
}

void loadCurrentRider(s_workArea* pWorkArea)
{
    loadRider(pWorkArea, mainGameState.gameStats.rider1);
}

void loadCurrentRider2(s_workArea* pWorkArea)
{
    loadRider2(pWorkArea, mainGameState.gameStats.rider2);
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

    fieldTaskPtr->pSubFieldData = pFieldSubTaskWorkArea;
    fieldTaskPtr->pSubFieldData->fieldDebuggerWho = 0;

    setFieldSubTaskVar0(1);

    if ((fieldTaskPtr->currentFieldIndex != 8) && (fieldTaskPtr->currentFieldIndex != 12))
    {
        resetTempAllocators();
    }

    menuUnk0.m_48 = 0xC210;
    menuUnk0.m_4A = 0xC210;

    FLD_A3_OVERLAY::overlayStart(pWorkArea, 0);

    fieldTaskPtr->fieldTaskState = 4;

    if (fieldTaskPtr->field_35)
    {
        fieldTaskPtr->fStatus = 0;
    }
    else
    {
        fieldTaskPtr->fStatus = 1;
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

void fieldStartOverlayTaskInit(s_workArea* pWorkArea)
{
    const s_fieldDefinition* pFieldDefinition = &fieldDefinitions[fieldTaskPtr->currentFieldIndex];

    fieldTaskPtr->overlayTaskData = pWorkArea;

    yLog("Missing createEncouterTask");

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
    const s_fieldDefinition* pFieldDefinition = &fieldDefinitions[fieldTaskPtr->currentFieldIndex];

    if (pFieldDefinition->m_fnt)
    {
        unloadFnt(pFieldDefinition->m_fnt);
    }

    fieldTaskPtr->overlayTaskData = NULL;
}

s_taskDefinition fieldStartOverlayTask = { fieldStartOverlayTaskInit, NULL, NULL, fieldStartOverlayTaskDelete, "field start overlay task" };

void fieldSub1TaskInit(s_workArea* pWorkArea)
{
    fieldTaskPtr->field_0 = pWorkArea;
    createSubTask(pWorkArea, &fieldStartOverlayTask, new s_workArea);
}

void fieldSub1TaskDelete(s_workArea* pWorkArea)
{
    fieldTaskPtr->field_0 = NULL;
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
    fieldTaskPtr->currentFieldIndex = fieldIndexMenuSelection;
    fieldTaskPtr->currentSubFieldIndex = subFieldIndexMenuSelection;
    fieldTaskPtr->field_30 = field_3A;
    fieldTaskPtr->field_32 = currentSubFieldIndex;

    if (fieldTaskVar0 == NULL)
    {
        createSubTask(fieldTaskPtr, &fieldSub0TaskDefinition, new s_workArea);
    }
    else
    {
        if (fieldTaskPtr->field_0 == NULL)
        {
            createSubTask(fieldTaskPtr, &fieldSub1TaskDefinition, new s_workArea);
        }
        else
        {
            createSubTask(fieldTaskPtr->overlayTaskData, &fieldSubTaskDefinition, new s_FieldSubTaskWorkArea);
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

        mainGameState.gameStats.dragonLevel = perFieldDragonLevel[fieldIndex];
    }

    if (mainGameState.gameStats.dragonLevel == 8)
    {
        assert(0);
    }

    //setup riders
    mainGameState.gameStats.rider1 = 1; // edge is rider

    switch (fieldIndex)
    {
    case 3:
        mainGameState.gameStats.rider2 = 2;
        break;
    case 18: // tower
        mainGameState.gameStats.rider2 = 5;
        break;
    default:
        mainGameState.gameStats.rider2 = 0;
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
        updateDragonIfCursorChanged(mainGameState.gameStats.dragonLevel);
        loadRiderIfChanged(mainGameState.gameStats.rider1);
        loadRider2IfChanged(mainGameState.gameStats.rider2);
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
        fieldTaskUpdateSub0(pWorkArea->fieldIndexMenuSelection, pWorkArea->subFieldIndexMenuSelection, pWorkArea->field_3A, pWorkArea->currentSubFieldIndex);

        if (pWorkArea->pSubFieldData)
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
        if (pWorkArea->pSubFieldData == NULL)
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
    
    createLoadingTask(pWorkArea->field_8);
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
    u32 field_8;
    u32 field_C;
};

struct {
    s8 field_0;
    s8 field_1;
    s8 field_2;
    s8 field_3;
    u16 field_4;
    u16 field_6;
    u16 field_8;
} var_60525F4;

struct {
    u8 field_8;
    u8 field_9;
    u8 field_A;
    u8 field_B;
} var_60525E8;

void exitMenuTaskSub1TaskInitSub2(u32 r4)
{
    if (var_60525F4.field_8 == 0)
    {
        var_60525F4.field_8 = r4;
        var_60525F4.field_2 = 0;
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
    graphicEngineStatus.field_40AC.isMenuAllowed = 0;
    graphicEngineStatus.field_40AC.field_3 = 0;
    graphicEngineStatus.field_40AC.menuId = 0;
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
    s32 field_0;
    s_graphicEngineStatus_40BC* field_4;
    u16* field_8;
    s32 field_C;
};

struct s_mainMenuWorkArea : public s_workArea
{
    u8 field_0; //0
    s8 field_1; //1
    s8 field_2; //2
    s8 field_3[5]; //3

    s_statusMenuTaskWorkArea* field_C; // C
    s_MenuCursorWorkArea* field_10; // 10
};

void mainMenuTaskInitSub1()
{
    initVdp2ForDragonMenuSub1(0, 160, 0);
    initVdp2ForDragonMenuSub1(1, 0, 0x100);
    initVdp2ForDragonMenuSub1(3, 0, 0x100);
    initVdp2ForDragonMenuSub2();
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

void menuDragonCrestTaskDraw(s_workArea* pTypelessWorkArea)
{
    s_menuDragonCrestTaskWorkArea* pWorkArea = static_cast<s_menuDragonCrestTaskWorkArea*>(pTypelessWorkArea);
    if (graphicEngineStatus.field_40AC.field_5)
    {
        assert(0);
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
    if (mainGameState.gameStats.dragonLevel > 7)
        return;

    u8 r14 = mainGameState.gameStats.dragonLevel;
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

    if (pWorkArea->field_0 < 0)
        return;

    s32 X = pWorkArea->field_8[pWorkArea->field_0 + 0] - pWorkArea->field_4->scrollX;
    s32 Y = pWorkArea->field_8[pWorkArea->field_0 + 1] - pWorkArea->field_4->scrollY;

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

void mainMenuTaskInit(p_workArea typelessWorkArea)
{
    s_mainMenuWorkArea* pWorkArea = static_cast<s_mainMenuWorkArea*>(typelessWorkArea);

    pWorkArea->field_3[0] = 1; // item is always enabled

    if (mainGameState.getBit(4, 2)) // dragon menu
    {
        pWorkArea->field_3[1] = 1;
    }
    else
    {
        pWorkArea->field_3[1] = -1;
    }

    if (mainGameState.gameStats.XP) // defeated monsters menu
    {
        pWorkArea->field_3[2] = 1;
    }
    else
    {
        pWorkArea->field_3[2] = -1;
    }
    pWorkArea->field_3[3] = 1; // map
    pWorkArea->field_3[4] = 1; // setup

    mainMenuTaskInitSub1();

    u32 mainMenuTaskInitData1[5] = {
        0x71450,
        0x714D6,
        0x71510,
        0x71596,
        0x715D0,
    };

    s32 r14 = -1;
    for (int i = 0; i < 5; i++)
    {
        switch (pWorkArea->field_3[i])
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

    pWorkArea->field_2 = r14;

    if (r14 >= 0)
    {
        pWorkArea->field_10 = createMenuCursorTask(pWorkArea, &mainMenuTaskInitData2);
        pWorkArea->field_10->field_0 = r14;
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

    if ((mainGameState.gameStats.dragonLevel == DR_LEVEL_0_BASIC_WING) || (mainGameState.gameStats.dragonLevel == DR_LEVEL_6_LIGHT_WING) || (mainGameState.gameStats.dragonLevel >= DR_LEVEL_7_SOLO_WING))
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

    if (mainGameState.gameStats.dragonLevel < DR_LEVEL_8_FLOATER)
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

void statusMenuTaskDelete(p_workArea pWorkArea)
{
    unimplemented("statusMenuTaskDelete");
}

s_taskDefinition statusMenuTaskDefinition = { statusMenuTaskInit, NULL, statusMenuTaskDraw, statusMenuTaskDelete, "statusMenuTask" };

void mainMenuTaskDraw(p_workArea typelessWorkArea)
{
    s_mainMenuWorkArea* pWorkArea = static_cast<s_mainMenuWorkArea*>(typelessWorkArea);

    switch (pWorkArea->field_0)
    {
    case 0:
        pWorkArea->field_C = static_cast<s_statusMenuTaskWorkArea*>(createSubTask(pWorkArea, &statusMenuTaskDefinition, new s_statusMenuTaskWorkArea));
        pWorkArea->field_C->selectedMenu = pWorkArea->field_2;
        pWorkArea->field_10->field_0 = pWorkArea->field_2;
        pWorkArea->field_0++;
    case 1:
        if (graphicEngineStatus.field_4514->current.field_8 & 1)
        {
            assert(0);
            return;
        }
        if (pWorkArea->field_2 < 0)
        {
            return;
        }
        if (graphicEngineStatus.field_4514->current.field_8 & 6)
        {
            assert(0);
        }
        if (graphicEngineStatus.field_4514->current.field_8 & 0x30)
        {
            assert(0);
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

void setupVDP2CoordinatesIncrement2(s32 scrollX, s32 scrollY)
{
    if (pauseEngine[4] == 0)
    {
        vdp2Controls.m_registers->SCXN0 = scrollX;
        vdp2Controls.m_pendingVdp2Regs->SCXN0 = scrollX;
        vdp2Controls.m_registers->SCYN0 = scrollY;
        vdp2Controls.m_pendingVdp2Regs->SCYN0 = scrollY;
    }
    else if (pauseEngine[4] == 1)
    {
        vdp2Controls.m_registers->SCXN1 = scrollX;
        vdp2Controls.m_pendingVdp2Regs->SCXN1 = scrollX;
        vdp2Controls.m_registers->SCYN1 = scrollY;
        vdp2Controls.m_pendingVdp2Regs->SCYN1 = scrollY;
    }
    else if (pauseEngine[4] == 2)
    {
        vdp2Controls.m_registers->SCXN2 = scrollX;
        vdp2Controls.m_pendingVdp2Regs->SCXN2 = scrollX;
        vdp2Controls.m_registers->SCYN2 = scrollY;
        vdp2Controls.m_pendingVdp2Regs->SCYN2 = scrollY;
    }
    else if (pauseEngine[4] == 3)
    {
        vdp2Controls.m_registers->SCXN3 = scrollX;
        vdp2Controls.m_pendingVdp2Regs->SCXN3 = scrollX;
        vdp2Controls.m_registers->SCYN3 = scrollY;
        vdp2Controls.m_pendingVdp2Regs->SCYN3 = scrollY;
    }
}

void scrollMenu()
{
    for (int i = 0; i < 4; i++)
    {
        if (graphicEngineStatus.layersConfig[4].field_8)
        {
            assert(0);
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
        if (graphicEngineStatus.field_4514[0].current.field_8 & 8)
        {
            yLog("Hack: forcing menu entry to 1");
            graphicEngineStatus.field_40AC.isMenuAllowed = 1;
            if (graphicEngineStatus.field_40AC.isMenuAllowed)
            {
                graphicEngineStatus.field_40AC.menuId = 1;
                playSoundEffect(0);
            }
            else
            {
                if (graphicEngineStatus.field_40AC.menuId == 0)
                    return;
            }
        }

        // enter menu
        // this isn't exactly correct
        if (graphicEngineStatus.field_40AC.menuId)
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

            pWordArea->field_8 = menuTaskMenuArray[graphicEngineStatus.field_40AC.menuId](pWordArea);
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
            if (graphicEngineStatus.field_4514[0].current.field_8 & 8)
            {
                assert(0);
            }
            if (graphicEngineStatus.field_4514[0].current.field_8 & 2)
            {
                assert(0);
            }
        }
        else
        {
            menuGraphicsTaskDrawSub2();

            graphicEngineStatus.field_40AC.menuId = 0;
            pWordArea->field_C = pauseEngine[0];

            pWordArea->field_4->getTask()->clearPaused();

            pWordArea->field_D = graphicEngineStatus.field_40AC.field_9;
            pWordArea->state = 5;
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

    var_60525F4.field_0 = -1;
    var_60525F4.field_1 = -1;
    var_60525F4.field_3 = 0;
    var_60525F4.field_4 = 0;
    var_60525F4.field_6 = 0;
    var_60525F4.field_8 = 0;

    if (menuID == 3)
    {
        mainGameState.gameStats.dragonLevel = DR_LEVEL_1_VALIANT_WING;
    }
    else
    {
        mainGameState.gameStats.dragonLevel = DR_LEVEL_0_BASIC_WING;
    }

    mainGameState.gameStats.rider1 = 1;
    mainGameState.gameStats.rider2 = 0;

    var_60525E8.field_8 = 0;
    var_60525E8.field_9 = 0;
    var_60525E8.field_A = 0;
    var_60525E8.field_B = 0;

    exitMenuTaskSub1TaskInitSub1();

    createMenuTask(pWorkArea);
    createFieldTask(pWorkArea, 1);
    createLoadingTask(pWorkArea);
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

        if (var_60525F4.field_8)
        {
            if (var_60525F4.field_2 == 0)
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
            assert(0);
        }

        // 06027474
        graphicEngineStatus.field_4 = 1;
        pauseEngine[2] = 0;
        graphicEngineStatus.field_40AC.isMenuAllowed = 0;
        var_60525F4.field_0 = -1;
        pWorkArea->state++;
        break;
    case 1:
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

