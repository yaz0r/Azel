#include "PDS.h"

const u8 bitMasks[] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
const u8 reverseBitMasks[] = { 0x7F, 0xBF, 0xDF, 0xEF, 0xF7, 0xF8, 0xFD, 0xFE };

struct s_gameStats
{
    u8 level;
    u8 dragonLevel;
    u8 rider1;
    u8 rider2;

    u16 currentHP; // 0x10
    u16 classMaxHP; // 0x12
    u16 currentBP; // 0x14
    u16 classMaxBP; // 0x16
    u16 field_18; // 0x18

    char playerName[17];
    char dragonName[17];

    u16 maxHP; // B8
    u16 maxBP; // BA;
    u16 dragonDef; // BC
    u16 dragonAtt; // BE
    u16 dragonAgl; // C0
    u16 dragonSpr; // C2
};

struct sBitfieldMapEntry
{
    u32 m_bitOffset;
    u32 m_bitSize;
    const char* m_name;
};

struct s_mainGameState
{
private:
    u8 bitField[630];
    std::vector<sBitfieldMapEntry> m_bitFieldMap;

public:
    s_gameStats gameStats;

    void reset()
    {
        m_bitFieldMap.clear();
        memset(bitField, 0, sizeof(bitField));
        memset(&gameStats, 0, sizeof(gameStats));
    }

    void setPackedBits(u32 firstBitOffset, u32 numBits, u32 value)
    {
        void setPackedBits(u8* bitField, u32 firstBitOffset, u32 numBits, u32 value);

        setPackedBits(bitField, firstBitOffset, numBits, value);
    }

    void setBit(u32 bitIndex)
    {
        bitField[bitIndex / 8] |= 1 << (bitIndex % 8);
    }

    void clearBit(u32 bitIndex)
    {
        bitField[bitIndex / 8] &= ~(1 << (bitIndex % 8));
    }
};

s_mainGameState mainGameState;

u32 getPanzerZweiPlayTime(u32 slot)
{
    return 0;
}

struct s_dragonPerLevelMaxHPBP
{
    u16 maxHP;
    u16 maxBP;
};

const s_dragonPerLevelMaxHPBP dragonPerLevelMaxHPBP[9] = {
    { 400, 100 },
    { 400, 100 },
    { 400, 100 },
    { 400, 100 },
    { 400, 100 },
    { 400, 100 },
    { 400, 100 },
    { 400, 100 },
    { 1200, 0 }
};

void computeDragonSprAndAglFromCursor()
{
    //assert(0);
}

void updateDragonStatsFromLevel()
{
    s_gameStats& gameStats = mainGameState.gameStats;

    if (gameStats.dragonLevel < 8)
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

const u32 longBitMask[] = {
0x0,
0x1,
0x3,
0x7,
0xF,
0x1F,
0x3F,
0x7F,
0xFF,
0x1FF,
0x3FF,
0x7FF,
0xFFF,
0x1FFF,
0x3FFF,
0x7FFF,
0xFFFF,
0x1FFFF,
0x3FFFF,
0x7FFFF,
0xFFFFF,
0x1FFFFF,
0x3FFFFF,
0x7FFFFF,
0xFFFFFF,
0x1FFFFFF,
0x3FFFFFF,
0x7FFFFFF,
0xFFFFFFF,
0x1FFFFFFF,
0x3FFFFFFF,
0x7FFFFFFF,
0xFFFFFFFF,
};

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

struct s_fieldTaskWorkArea : public s_workArea
{
    u32 fStatus; // 0x28
    u8 field_35;
    u16 field_36;
    u8 fieldTaskState;
    u8 field_3D;
    // size: 0x50
};

s_fieldTaskWorkArea* fieldTaskPtr = NULL;

u32 fieldTaskVar0;
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
    fieldTaskVar0 = 0;
    fieldInputTaskWorkArea = 0;
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
    u32 m_8; // 8
};

u32 performModulo(u32 r0, u32 r1)
{
    assert(r0);

    return r1 % r0;
}

const u32 fieldEnabledTable[23] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1
};

struct s_fieldDefinition
{
    const char* m_name;
    const char* m_prg;
    const char* m_fnt;
    u32 m_numSubFields;
    const char** m_subFields;
};

const char* m_A2Fields[] = {
    "  A2_0",
};

const char* m_A3Fields[] = {
    "  A3_0",
    "  A3_1",
    "  A3_2",
    "  A3_3",
    "  A3_4",
    "  A3_5",
    "  A3_6",
    "  A3_7",
    "  A3_8",
    "  A3_9",
    "  A3_A",
    "  A3_B",
    "  A3_C",
};

const char* m_A5Fields[] = {
    "  A5_0",
    "  A5_1",
    "  A5_2",
    "  A5_3",
    "  A5_4",
    "  A5_5",
    "  A5_6",
    "  A5_7",
    "  A5_8",
    "  A5_9",
    "  A5_A",
    "  A5_B",
    "  A5_C",
};
const char* m_A7Fields[] = {
    "  A7_0",
    "  A7_1",
    "  A7_2",
};

const char* m_B1Fields[] = {
    "  B1_0",
    "  B1_1",
};

const char* m_B3Fields[] = {
    "  B3_0",
};

const char* m_B2Fields[] = {
    "  B2_1",
    "  B2_3",
    "  B2_4",
    "  B2_5",
};

const char* m_B5Fields[] = {
    "  B5_0",
    "  B5_1",
    "  B5_2",
    "  B5_3",
    "  B5_4",
    "  B5_5",
    "  B5_6",
};

const char* m_B6Fields[] = {
    "  B6_0",
    "  B6_1",
    "  B6_2",
    "  B6_3",
    "  B6_4",
    "  B6_5",
    "  B6_6",
    "  B6_7",
    "  B6_8",
    "  B6_9",
};

const char* m_C2Fields[] = {
    "  C2_0",
    "  C2_1",
    "  C2_2",
};

const char* m_C4Fields[] = {
    "  C4_0",
    "  C4_1",
    "  C4_2",
    "  C4_3",
    "  C4_4",
    "  C4_5",
    "  C4_6",
    "  C4_7",
    "  C4_8",
};

const char* m_TowerFields[] = {
    "  T0_0",
    "  T0_1",
    "  T0_2",
    "  T0_3",
    "  T0_4",
    "  T0_5",

    "  T1_0",
    "  T1_1",
    "  T1_2",

    "  T2_0",
    "  T2_1",
    "  T2_2",
    "  T2_3",
    "  T2_4",
    "  T2_5",
    "  T2_6",
    "  T2_7",
    "  T2_8",
    "  T2_9",
    "  T2_a",
    "  T2_b",
    "  T2_c",
    "  T2_d",

    "  T3_0",
    "  T3_1",
    "  T3_2",

    "  T7_0",
    "  T7_1",
    "  T7_2",
    "  T7_3",
    "  T7_4",
    "  T7_5",
    "  T7_6",
    "  T7_7",
};

const char* m_D2Fields[] = {
    "  D2_0",
    "  D2_1",
};

const char* m_D3Fields[] = {
    "  D3_0",
};

const char* m_D5Fields[] = {
    "  D5_0",
};

const char* m_DummyFields[] = {
    "  DUMM",
};

const char* m_NameFields[] = {
    "  NAME",
};

const char* m_DemoFields[] = {
    "  DEMO",
};

const char* m_GameFields[] = {
    "  OVER",
};

const char* m_BTFields[] = {
    "  BT  ",
};

const s_fieldDefinition fieldDefinitions[] =
{
    { "  A0  ", "FLD_D5.PRG", "FLD_D5.FNT", sizeof(m_NameFields) / sizeof(m_NameFields[0]), m_NameFields },
    { "  A2  ", "FLD_A3.PRG", "FLD_A3.FNT", sizeof(m_A2Fields) / sizeof(m_A2Fields[0]), m_A2Fields },
    { "  A3  ", "FLD_A3.PRG", "FLD_A3.FNT", sizeof(m_A3Fields) / sizeof(m_A3Fields[0]), m_A3Fields },
    { "  A5  ", "FLD_A5.PRG", "FLD_A5.FNT", sizeof(m_A5Fields) / sizeof(m_A5Fields[0]), m_A5Fields },
    { "  A7  ", "FLD_A7.PRG", "FLD_A7.FNT", sizeof(m_A7Fields) / sizeof(m_A7Fields[0]), m_A7Fields },

    { "  B1  ", "FLD_B1.PRG", "FLD_B1.FNT", sizeof(m_B1Fields) / sizeof(m_B1Fields[0]), m_B1Fields },
    { "  B3  ", "FLD_B2.PRG", "FLD_B2.FNT", sizeof(m_B3Fields) / sizeof(m_B3Fields[0]), m_B3Fields },
    { "  B2  ", "FLD_B2.PRG", "FLD_B2.FNT", sizeof(m_B2Fields) / sizeof(m_B2Fields[0]), m_B2Fields },
    { "  B4  ", "FLD_B5.PRG", "FLD_B5.FNT", sizeof(m_DemoFields) / sizeof(m_DemoFields[0]), m_DemoFields },
    { "  B5  ", "FLD_B5.PRG", "FLD_B5.FNT", sizeof(m_B5Fields) / sizeof(m_B5Fields[0]), m_B5Fields },
    { "  B6  ", "FLD_B6.PRG", "FLD_B6.FNT", sizeof(m_B6Fields) / sizeof(m_B6Fields[0]), m_B6Fields },

    { "  C2  ", "FLD_C2.PRG", "FLD_C2.FNT", sizeof(m_C2Fields) / sizeof(m_C2Fields[0]), m_C2Fields },
    { "  C3  ", "FLD_C4.PRG", "FLD_C4.FNT", sizeof(m_DemoFields) / sizeof(m_DemoFields[0]), m_DemoFields },
    { "  C4  ", "FLD_C4.PRG", "FLD_C4.FNT", sizeof(m_C4Fields) / sizeof(m_C4Fields[0]), m_C4Fields },
    { "  C5  ", NULL,         NULL,         sizeof(m_DummyFields) / sizeof(m_DummyFields[0]), m_DummyFields },
    { "  C8  ", "FLD_C8.PRG", "FLD_T0.FNT", sizeof(m_TowerFields) / sizeof(m_TowerFields[0]), m_TowerFields },

    { "  D2  ", "FLD_D2.PRG", "FLD_D2.FNT", sizeof(m_D2Fields) / sizeof(m_D2Fields[0]), m_D2Fields },
    { "  D3  ", "FLD_D3.PRG", "FLD_D3.FNT", sizeof(m_D3Fields) / sizeof(m_D3Fields[0]), m_D3Fields },
    { "  D4  ", "FLD_C8.PRG", "FLD_T0.FNT", sizeof(m_TowerFields) / sizeof(m_TowerFields[0]), m_TowerFields },
    { "  D5  ", "FLD_D5.PRG", "FLD_D5.FNT", sizeof(m_D5Fields) / sizeof(m_D5Fields[0]), m_D5Fields },

    { "  GAME", "FLD_D5.PRG", "FLD_D5.FNT", sizeof(m_GameFields) / sizeof(m_GameFields[0]), m_GameFields },
    { "  BT0 ", "FLD_A3.PRG", "FLD_A3.FNT", sizeof(m_BTFields) / sizeof(m_BTFields[0]), m_BTFields },
    { "  BT1 ", "FLD_A7.PRG", "FLD_A7.FNT", sizeof(m_BTFields) / sizeof(m_BTFields[0]), m_BTFields },
};

void fieldDebugListTaskInit(p_workArea pTypelessWorkArea)
{
    s_fieldDebugListWorkArea* pWorkArea = static_cast<s_fieldDebugListWorkArea*>(pTypelessWorkArea);
    
    pWorkArea->m_ticks++;
    s_fieldTaskWorkArea*r14 = getFieldTaskPtr();

    u32 var_24 = performModulo(30, pWorkArea->m_ticks);

    if (pWorkArea->m_8)
    {
        assert(0);
    }
    else
    {
        if (PortData2.field_8 & 0x80)
        {
            pWorkArea->m_8 = 1;
        }

        if (PortData2.field_C & 0x60)
        {
            assert(0);
        }

        if (PortData2.field_C & 0x10)
        {
            assert(0);
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
    for (u32 r12 = 0; r12 < 19; r12++)
    {
        vdp2DebugPrintSetPosition(0xA, r8);
        if (r14->field_36 == r12) // is this the selected field?
        {
            vdp2PrintStatus.palette = 0x8000;
        }
        else
        {
            u16 selectedColor[] = { 0xD, 0xD };
            assert(pWorkArea->m_8 >= 0);
            assert(pWorkArea->m_8 <= 1);
            vdp2PrintStatus.palette = selectedColor[pWorkArea->m_8] << 12;
        }

        if(fieldEnabledTable[r12])
        {
            printf(fieldDefinitions[r12].m_name);
        }
        

        if (pWorkArea->m_8 == 0)
        {

        }
    }
}

void fieldDebugListTaskUpdate(p_workArea pTypelessWorkArea)
{
    s_fieldDebugListWorkArea* pWorkArea = static_cast<s_fieldDebugListWorkArea*>(pTypelessWorkArea);
    assert(0);
}

s_taskDefinition fieldDebugListTaskDefinition = { fieldDebugListTaskInit, fieldDebugListTaskUpdate, NULL, NULL, "field debug list" };

p_workArea createFieldInputTask(p_workArea pTypelessWorkArea)
{
    return createSubTask(pTypelessWorkArea, &fieldDebugListTaskDefinition, new s_fieldDebugListWorkArea);
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
