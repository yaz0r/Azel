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

struct s_mainGameState
{
    u8 bitField[630];

    s_gameStats gameStats;
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
    assert(0);
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
    memset(&mainGameState, 0, sizeof(s_mainGameState));

    setPackedBits((u8*)&mainGameState, 2, 3, 0);
    if (VDP2Regs_.TVSTAT & 1)
    {
        mainGameState.bitField[0] |= bitMasks[5];
    }
    else
    {
        mainGameState.bitField[0] &= reverseBitMasks[5];
    }

    mainGameState.bitField[0] &= reverseBitMasks[6];
    mainGameState.bitField[0x2B] |= bitMasks[2];
    mainGameState.bitField[0x2D] |= bitMasks[5];

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

struct s_fieldDebugTaskWorkArea : public s_workArea
{

};

void fieldDebugTaskInit(p_workArea workArea)
{
    pauseEngine[2] = 0;

    initNewGameState();
    /*workArea->field_8 = createFieldTask(workArea, 0);
    createLoadingTask(workArea->field_8);
    createTask_1Arg(workArea->field_8, flagEditTask, 0x10);*/
}

void genericTaskRestartGameWhenFinished(p_workArea workArea)
{
    assert(0);
}

void genericOptionMenuDelete(p_workArea workArea)
{
    assert(0);
}

s_taskDefinition fieldDebugModule = { fieldDebugTaskInit, NULL, genericTaskRestartGameWhenFinished, genericOptionMenuDelete };

p_workArea createTownDebugTask(p_workArea)
{
    assert(0);
    return NULL;
}
p_workArea createFieldDebugTask(p_workArea pWorkArea)
{
    return createSubTask(pWorkArea, &fieldDebugModule, new s_fieldDebugTaskWorkArea, "fieldDebugTAsk");
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
