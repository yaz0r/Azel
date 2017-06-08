#include "PDS.h"

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

void fieldTaskInit(p_workArea pTypelessWorkArea, u32 battleArgument)
{
    s_fieldTaskWorkArea* pWorkArea = static_cast<s_fieldTaskWorkArea*>(pTypelessWorkArea);

    fieldTaskPtr = pWorkArea;
    fieldTaskVar0 = NULL;
    fieldInputTaskWorkArea = NULL;
    fieldTaskVar2 = 0;

    pWorkArea->field_35 = battleArgument;
    pWorkArea->field_3D = -1;

    if (battleArgument)
    {
        assert(0);
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
    
    resetMenu(&menuUnk0.m_field0, 0xC210, 0xC210, 1);
    resetMenu(&menuUnk0.m_field24, 0xC210, 0xC210, 1);
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
        if (PortData2.field_8 & 0x60)
        {
            pWorkArea->m_isSelectingSubfield = 1;
        }

        if (PortData2.field_C & 0x20) // bottom
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

        if (PortData2.field_C & 0x10) // up
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

    if (PortData2.field_C & 0x8000)
    {
        assert(0);
    }

    if (PortData2.field_C & 0x800)
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

    u8 inputValue = PortData2.field_8 & 0xF;
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

    resetMenu(&menuUnk0.m_field0, 0, 0, 1);
    resetMenu(&menuUnk0.m_field24, 0, 0, 1);

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

void unimplementedUpdate(s_dragonStateSubData1* pDragonStateData1)
{
    assert(0);
}

void unimplementedDraw(s_dragonStateSubData1* pDragonStateData1)
{
    assert(0);
}

void (*modelMode4_position0)(s_dragonStateSubData1*) = unimplementedUpdate;
void (*modelMode4_position1)(s_dragonStateSubData1*) = unimplementedUpdate;
void (*modelMode4_rotation)(s_dragonStateSubData1*) = unimplementedUpdate;
void (*modelMode4_scale)(s_dragonStateSubData1*) = unimplementedUpdate;

void (*modelDrawFunction0)(s_dragonStateSubData1*) = unimplementedDraw;
void (*modelDrawFunction1)(s_dragonStateSubData1*) = unimplementedDraw;
void (*modelDrawFunction2)(s_dragonStateSubData1*) = unimplementedDraw;
void (*modelDrawFunction3)(s_dragonStateSubData1*) = unimplementedDraw;

void (*modelDrawFunction5)(s_dragonStateSubData1*) = unimplementedDraw;
void (*modelDrawFunction6)(s_dragonStateSubData1*) = unimplementedDraw;

void (*modelDrawFunction9)(s_dragonStateSubData1*) = unimplementedDraw;
void (*modelDrawFunction10)(s_dragonStateSubData1*) = unimplementedDraw;

void copyPosePosition(s_dragonStateSubData1* pDragonStateData1)
{
    sPoseData* pOutputPose = pDragonStateData1->poseData;
    u8* r5 = pDragonStateData1->field_34;

    for (u32 i = 0; i < pDragonStateData1->numBones; i++)
    {
        pOutputPose->m_0 = READ_BE_U32(r5 + 0);
        pOutputPose->m_4 = READ_BE_U32(r5 + 4);
        pOutputPose->m_8 = READ_BE_U32(r5 + 8);

        pOutputPose++;
        r5 += 0x24;
    }
}

void copyPoseRotation(s_dragonStateSubData1* pDragonStateData1)
{
    sPoseData* pOutputPose = pDragonStateData1->poseData;
    u8* r5 = pDragonStateData1->field_34;

    for (u32 i = 0; i < pDragonStateData1->numBones; i++)
    {
        pOutputPose->m_C = READ_BE_U32(r5 + 0xC);
        pOutputPose->m_10 = READ_BE_U32(r5 + 0x10);
        pOutputPose->m_14 = READ_BE_U32(r5 + 0x14);

        pOutputPose++;
        r5 += 0x24;
    }
}

void resetPoseScale(s_dragonStateSubData1* pDragonStateData1)
{
    sPoseData* pOutputPose = pDragonStateData1->poseData;

    for (u32 i = 0; i < pDragonStateData1->numBones; i++)
    {
        pOutputPose->m_18 = 0x10000;
        pOutputPose->m_1C = 0x10000;
        pOutputPose->m_20 = 0x10000;

        pOutputPose++;
    }
}

u32 createDragonStateSubData1Sub1Sub1(s_dragonStateSubData1* pDragonStateData1, u8* pModelData1)
{
    u16 flags = READ_BE_U16(pModelData1);

    switch (flags & 7)
    {
    case 4:
        if (pDragonStateData1->field_A & 0x100)
        {
            pDragonStateData1->positionUpdateFunction = modelMode4_position0;
        }
        else
        {
            pDragonStateData1->positionUpdateFunction = modelMode4_position1;
        }
        pDragonStateData1->rotationUpdateFunction = modelMode4_rotation;
        pDragonStateData1->scaleUpdateFunction = modelMode4_scale;

        for (u32 i = 0; i < pDragonStateData1->numBones; i++)
        {
            for (u32 j = 0; j < 9; j++)
            {
                pDragonStateData1->poseData[i].field_48[j][0] = 0;
                pDragonStateData1->poseData[i].field_48[j][1] = 0;
                pDragonStateData1->poseData[i].field_48[j][2] = 0;
            }
        }
        break;
    default:
        assert(0);
    }

    return 1;
}

u32 createDragonStateSubData1Sub1(s_dragonStateSubData1* pDragonStateData1, u8* pModelData1)
{
    pDragonStateData1->field_30 = pModelData1;
    pDragonStateData1->field_10 = 0;

    u16 flags = READ_BE_U16(pModelData1);

    if ((flags & 8) || (pDragonStateData1->field_A & 0xA))
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

void addObjectToDrawList()
{
    assert(0);
}

void initModelDrawFunction(s_dragonStateSubData1* pDragonStateData1)
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

void countNumBonesInModel(s_dragonStateSubData1* pDragonStateData1, u8* pDragonModelData, u8* pStartOfData)
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

bool createDragonStateSubData1Sub2(s_dragonStateSubData1* pDragonStateData1, const s_RiderDefinitionSub* unkArg)
{
    pDragonStateData1->field_40 = unkArg;

    pDragonStateData1->field_44 = static_cast<u8**>(allocateHeapForTask(pDragonStateData1->pDragonState, pDragonStateData1->numBones * sizeof(u8*)));

    if (pDragonStateData1->field_44 == NULL)
        return false;

    const s_RiderDefinitionSub* r12 = pDragonStateData1->field_40;

    for(u32 i=0; i<pDragonStateData1->numBones; i++)
    {
        if (r12->m_count > 0)
        {
            pDragonStateData1->field_44[i] = (u8*)allocateHeapForTask(pDragonStateData1->pDragonState, r12->m_count * 12);
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

bool createDragonStateSubData1(s_workArea* pWorkArea, s_dragonStateSubData1* pDragonStateData1, u32 unkArg0, u8* pDragonModel, u16 unkArg1, u8* pModelData1, u8* pModelData2, u32 unkArg2, const s_RiderDefinitionSub* unkArg3)
{
    pDragonStateData1->pDragonState = pWorkArea;
    pDragonStateData1->pDragonModel = pDragonModel;
    pDragonStateData1->field_C = unkArg1;
    pDragonStateData1->field_34 = pModelData2;
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
        countNumBonesInModel(pDragonStateData1, pDragonModel + READ_BE_U32(pDragonModel + pDragonStateData1->field_C), pDragonModel);
    }

    pDragonStateData1->poseData = static_cast<sPoseData*>(allocateHeapForTask(pWorkArea, pDragonStateData1->numBones * sizeof(sPoseData)));

    if (pDragonStateData1->field_A & 0x200)
    {
        pDragonStateData1->field_3C = static_cast<u8*>(allocateHeapForTask(pWorkArea, pDragonStateData1->numBones * 48));
        assert(pDragonStateData1->field_3C);

        pDragonStateData1->field_8 |= 2;
    }
    else
    {
        pDragonStateData1->field_3C = 0;
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
        pDragonStateData1->field_30 = NULL;
        pDragonStateData1->field_10 = 0;

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
    const sMatrix4x3* m_data;
};

struct sDragonAnimData
{
    const sDragonAnimDataSub* m_0;
    const sDragonAnimDataSub* m_4;
    const sDragonAnimDataSub* m_8;
    const sDragonAnimDataSub* m_C;
};

const sMatrix4x3 dragon0AnimsData0 =
{
    0x2423,
    0x2423,
    0x2423,
    0xB800,
    0xB800,
    0xB800,
    0xE38E38,
    0xE38E38,
    0xE38E38,
    0xFF1C71C8,
    0xFF1C71C8,
    0xFF1C71C8,
};
const sMatrix4x3 dragon0AnimsData1 =
{
    0x400,
    0x400,
    0x400,
    0x1000,
    0x1000,
    0x1000,
    0xE38E38,
    0xE38E38,
    0xE38E38,
    0xFF1C71C8,
    0xFF1C71C8,
    0xFF1C71C8,
};

const sDragonAnimDataSub dragon0Anims0[] =
{
    { 0x01, &dragon0AnimsData0 },
    { 0x0E, &dragon0AnimsData1 },
    { 0x0F, &dragon0AnimsData1 },
    { 0x10, &dragon0AnimsData1 },
    { 0x11, &dragon0AnimsData1 },
    { -1, NULL},
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
    &dragon0Anims0[1],
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

u32 countNumAnimData(s_dragonStateSubData2* pDragonStateData2, const sDragonAnimData* dragonAnims)
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

void copyAnimMatrix(const sMatrix4x3* source, sMatrix4x3* destination)
{
    for (u32 i = 0; i < 4 * 3; i++)
    {
        destination[i] = source[i];
    }
}

void initRuntimeAnimDataSub1(const sDragonAnimDataSub* animDataSub, s_dragonStateSubData2SubData* subData)
{
    for (u32 i = 0; i < 4 * 3; i++)
    {
        subData->matrix.matrix[i] = 0;
    }

    copyAnimMatrix(animDataSub->m_data, &subData->matrix2);

    subData->dataSource = animDataSub;
}

void initRuntimeAnimData(const sDragonAnimData* dragonAnims, s_dragonStateSubData2SubData* subData)
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

void createDragonStateSubData2(s_dragonState* pDragonState, s_dragonStateSubData1* pDragonStateData1, s_dragonStateSubData2* pDragonStateData2, const sDragonAnimData* dragonAnims)
{
    pDragonStateData2->field_0 = dragonAnims;

    pDragonStateData2->countAnims = countNumAnimData(pDragonStateData2, dragonAnims);

    pDragonStateData2->field_4 = pDragonStateData1->field_3C;

    pDragonStateData2->field_8 = static_cast<s_dragonStateSubData2SubData*>(allocateHeapForTask(pDragonState, pDragonStateData2->countAnims * sizeof(s_dragonStateSubData2SubData)));

    initRuntimeAnimData(dragonAnims, pDragonStateData2->field_8);
}

const sDragonAnimData* getDragonDataByIndex(e_dragonLevel dragonLevel)
{
    return dragonAnimData[dragonLevel];
}

void loadDragonSoundBank(e_dragonLevel dragonLevel)
{
    //assert(0);
}

void createDragonState(s_workArea* pWorkArea, e_dragonLevel dragonLevel)
{
    const sDragonData3* pDragonData3 = &dragonData3[dragonLevel];
    const s_dragonData2* pDragonData2 = &dragonData2[dragonLevel];

    s_dragonState* pDragonState = static_cast<s_dragonState*>(createSubTaskFromFunction(pWorkArea, NULL, new s_dragonState, "dragonState"));

    pDragonState->pDragonModel = gDragonModel;
    pDragonState->dragonType = dragonLevel;
    pDragonState->field_14 = pDragonData3->m_field_8[0].m_field_0[0];
    pDragonState->field_18 = pDragonData3->m_field_8[0].m_field_0[1];
    pDragonState->dragonData2 = pDragonData2->m_data;
    pDragonState->dragonData2Count = pDragonData2->m_count;
    pDragonState->field_88 = 1;

    u8* pDragonModel = pDragonState->pDragonModel;
    u8* pModelData1 = pDragonModel + READ_BE_U32(pDragonModel + pDragonState->dragonData2[0]);
    u8* pModelData2 = pDragonModel + READ_BE_U32(pDragonModel + pDragonData3->m_field_8[0].m_field_0[2]);

    createDragonStateSubData1(pDragonState, &pDragonState->dragonStateSubData1, 0x300, pDragonModel, pDragonState->field_14, pModelData1, pModelData2, 0, pDragonData3->m_field_8[0].m_field_8);

    createDragonStateSubData2(pDragonState, &pDragonState->dragonStateSubData1, &pDragonState->dragonStateSubData2, getDragonDataByIndex(dragonLevel));

    loadDragonSoundBank(dragonLevel);

    gDragonState = pDragonState;
}

void loadDragonFiles(s_workArea* pWorkArea, e_dragonLevel dragonLevel)
{
    loadFile(dragonFilenameTable[dragonLevel].m_base.MCB, gDragonModel, gDragonVram);
    loadFile(dragonFilenameTable[dragonLevel].m_base.CGB, gDragonVram, NULL);

    createDragonState(pWorkArea, dragonLevel);
}

struct s_loadDragonWorkArea : public s_workArea
{
    u8* dramAllocation; //0
    u8* vramAllocation;//4
    u16 MCBOffsetInDram;//8
    u16 CGBOffsetInDram;//A
};

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

void morphDragon(s_loadDragonWorkArea* pLoadDragonWorkArea, s_dragonStateSubData1* pDragonStateSubData1, u32 unk0, const sDragonData3* pDragonData3, s16 cursorX, s16 cursorY)
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

    morphDragon(pLoadDragonWorkArea, &gDragonState->dragonStateSubData1, pLoadDragonWorkArea->MCBOffsetInDram, pDragonData3, mainGameState.gameStats.dragonCursorX, mainGameState.gameStats.dragonCursorY);

    loadDragonSub1(pLoadDragonWorkArea);
}

const s_RiderDefinitionSub gEdgeExtraData[] =
{
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { (void*)1, 1 },
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
    u32 m_data0; // 10
    u32 m_14; //14


    s_dragonStateSubData1 m_18;//18
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
    pLoadRiderWorkArea->m_data0 = r13->m_flags;

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
    u8* pModelData2 = pModel + READ_BE_U32(pModel + r13->m_flags2);

    createDragonStateSubData1(pLoadRiderWorkArea, &pLoadRiderWorkArea->m_18, 0, pModel, pLoadRiderWorkArea->m_data0, pModelData1, pModelData2, 0, r13->m_pExtraData);

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
    pLoadRiderWorkArea->m_data0 = r13->m_flags;

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

    createDragonStateSubData1(pLoadRiderWorkArea, &pLoadRiderWorkArea->m_18, 0, pModel, pLoadRiderWorkArea->m_data0, pModelData1, pModelData2, 0, r13->m_pExtraData);

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

void loadFnt(const char*)
{
    yLog("Unimplemented loadFnt");
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

                resetMenu(&menuUnk0.m_field0, titleScreenDrawSub1(&menuUnk0), menuUnk0.m_48, 30);
                resetMenu(&menuUnk0.m_field24, titleScreenDrawSub1(&menuUnk0), menuUnk0.m_4A, 30);

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
        assert(0);
    }

    if (pFieldSubTaskWorkArea->pUpdateFunction2)
    {
        assert(0);
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
    return createSubTaskWithArg(pTypelessWorkArea, &fieldTaskDefinition, new s_fieldTaskWorkArea, arg);
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
    /*createLoadingTask(workArea->field_8);
    createTask_1Arg(workArea->field_8, flagEditTask, 0x10);*/
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

p_workArea createTownDebugTask(p_workArea pTypelessWorkArea)
{
    assert(0);
    return NULL;
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
