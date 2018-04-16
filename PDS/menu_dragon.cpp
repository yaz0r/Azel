#include "PDS.h"

struct s_dragonMenuSubTask1WorkArea : public s_workArea
{
    u32 status; //0
};

void dragonMenuSubTask1Init(p_workArea pTypelessWorkArea)
{
    unimplemented("dragonMenuSubTask1Init");
}

void dragonMenuSubTaskAdjustSoundFromCursor()
{
    unimplemented("dragonMenuSubTaskAdjustSoundFromCursor");
}

void dragonMenuSubTask1DrawSub1()
{
    unimplemented("dragonMenuSubTask1DrawSub1");
}

void dragonMenuSubTask1Draw(p_workArea pTypelessWorkArea)
{
    s_dragonMenuSubTask1WorkArea* pWorkArea = static_cast<s_dragonMenuSubTask1WorkArea*>(pTypelessWorkArea);

    switch (pWorkArea->status)
    {
    case 20:
        dragonMenuSubTask1DrawSub1();
        pWorkArea->status++;
        break;
    case 30:
        dragonMenuSubTaskAdjustSoundFromCursor();
        break;
    default:
        pWorkArea->status++;
        break;
    }
}

void dragonMenuSubTask1Delete(p_workArea pTypelessWorkArea)
{
    unimplemented("dragonMenuSubTask1Delete");
}

s_taskDefinition dragonMenuSubTask1Definition = { dragonMenuSubTask1Init, NULL, dragonMenuSubTask1Draw, dragonMenuSubTask1Delete, "dragonMenuSubTask1" };

struct s_dragonMenuWorkArea : public s_workArea
{
    u32 field_0;
    u32 field_4;
    u32 field_8; // 8
    p_workArea field_C;
    p_workArea field_10;
    p_workArea field_14;
    p_workArea field_18;
    p_workArea field_1C;
};

void dragonMenuTaskInit(p_workArea pTypelessWorkArea)
{
    s_dragonMenuWorkArea* pWorkArea = static_cast<s_dragonMenuWorkArea*>(pTypelessWorkArea);

    graphicEngineStatus.field_40AC.field_9 = 3;
    pWorkArea->field_4 = vblankData.field_14;

    createSubTask(pWorkArea, &dragonMenuSubTask1Definition, new s_dragonMenuSubTask1WorkArea);
}

void initVdp2ForDragonMenuSub1(u32 r4, u32 r5, u32 r6)
{
    assert(r4 < 4);

    u32 r7;

    if (VDP2Regs_.TVSTAT & 1)
    {
        r7 = 0;
    }
    else
    {
        r7 = 0x10;
    }

    graphicEngineStatus.layersConfig[r4].scrollX = r5;
    graphicEngineStatus.layersConfig[r4].scrollY = r7 + r6;
}

void initVdp2ForDragonMenuSub2()
{
    for (int i = 0; i < 4; i++)
    {
        graphicEngineStatus.layersConfig[i].field_8 = 0;
    }
}

void clearVdp2TextAreaSub1Sub1(u16 r4)
{
    if (r4 >= 0x80)
    {
        //assert((characterMap1[r4 - 0x80] >> 16) == 0);
        characterMap1[r4 - 0x80]--; // this only affect lower 16 bits
    }
}

u32 clearVdp2TextAreaSub1(u16 r4, s32 x, s32 y)
{
    if (r4 == 0)
    {
        setVdp2VramU16(vdp2TextMemoryOffset + (y * 64 + x)*2, 0);
    }

    u16 var0 = characterMap2[(y << 6) + x];
    u16 r13 = var0;
    if (r13 == r4)
    {
        return 0;
    }
    if (r13 == 0)
    {
        return 2;
    }

    u16* var_14 = characterMap2 + ((x - (r13 & 1)) << 7);
    y -= (2 & r13) >> 1;
    var0 = (y << 1);
    *(var_14 + (var0>>1)) = 0;

    u16* var_10 = characterMap2 + ((x + 1) << 7);
    *(var_10 + (var0>>1)) = 0;
    *(var_14 + (y + 1)) = 0;
    
    *(var_10 + (y + 1)) = 0;

    setVdp2VramU16(vdp2TextMemoryOffset + ((y << 6) + x) * 2, 0);
    setVdp2VramU16(vdp2TextMemoryOffset + ((y << 6) + x)*2 + 2, 0);
    setVdp2VramU16(vdp2TextMemoryOffset + ((y << 6) + x)*2 + 0x80, 0);
    setVdp2VramU16(vdp2TextMemoryOffset + ((y << 6) + x)*2 + 0x82, 0);

    clearVdp2TextAreaSub1Sub1(((r13 & 0x7FF) - 0x400) >> 2);

    return 1;
}

void clearVdp2TextArea()
{
    for (int x = vdp2StringContext.X; x < vdp2StringContext.X + vdp2StringContext.Width; x++)
    {
        for (int y = vdp2StringContext.Y; y < vdp2StringContext.Y + vdp2StringContext.Height; y++)
        {
            clearVdp2TextAreaSub1(0, x, y);
        }
    }
}

void initVdp2ForDragonMenu(u32 r4)
{
    if (r4)
    {
        initVdp2ForDragonMenuSub1(0, 0, 0);
        initVdp2ForDragonMenuSub1(1, 0, 0);
        initVdp2ForDragonMenuSub1(3, 0, 0x100);
        
        initVdp2ForDragonMenuSub2();

        setupVDP2StringRendering(0, 34, 44, 28);

        clearVdp2TextArea();
    }

    unpackGraphicsToVDP2(COMMON_DAT + 0xFE38, getVdp2Vram(0x71000));
}

void drawDragonMenuStatsTaskInit(p_workArea)
{
    setActiveFont(graphicEngineStatus.field_40AC.fontIndex);
}

struct s_stringStatusQuery
{
    s32 cursorX;
    s32 cursorY;
    s32 windowWidth;
    s32 windowHeight;
    s32 windowX1;
    s32 windowY1;
    s32 windowX2;
    s32 windowY2;
    const char* string;
    u32 vdp2MemoryOffset;
    u32 field_28;
    u32 field_2C;
};

void addStringToVdp2(const char* string, s_stringStatusQuery* vars)
{
    vars->cursorX = vdp2StringContext.cursorX;
    vars->cursorY = vdp2StringContext.cursorY;
    vars->windowX1 = vdp2StringContext.X;
    vars->windowY1 = vdp2StringContext.Y;
    vars->windowWidth = vdp2StringContext.Width;
    vars->windowHeight = vdp2StringContext.Height;
    vars->windowX2 = vdp2StringContext.X + vdp2StringContext.Width;
    vars->windowY2 = vdp2StringContext.Y + vdp2StringContext.Height;
    vars->string = string;
    vars->vdp2MemoryOffset = vdp2TextMemoryOffset + ((vdp2StringContext.cursorY << 6) + vdp2StringContext.cursorX)*2;
    vars->field_28 = vdp2StringContext.field_0;
    vars->field_2C = vdp2StringContext.field_38;
}

void moveVdp2TextCursor(s_stringStatusQuery* vars)
{
    vdp2StringContext.cursorX = vars->cursorX;
    vdp2StringContext.cursorY = vars->cursorY;
}

u32 printVdp2StringTable[10] = {
    12, 13, 7, 8, 9, 10, 11, 13, 7, 14
};

void printVdp2StringNewLine(s_stringStatusQuery* vars)
{
    unimplemented("printVdp2StringNewLine");
}

void printVdp2String(s_stringStatusQuery* vars)
{
    u32 r11 = (printVdp2StringTable[vars->field_28] << 12) + 0x63;

    vars->vdp2MemoryOffset = vdp2TextMemoryOffset + (((vars->cursorY << 6) + vars->cursorX))*2;

    while (u8 r4 = *(vars->string++))
    {
        switch (r4)
        {
        case 0xA:
        case '%':
            assert(0);
        default:
            setVdp2VramU16(vars->vdp2MemoryOffset, r11 + (r4 - 0x20) * 2);
            setVdp2VramU16(vars->vdp2MemoryOffset + 0x80, r11 + (r4 - 0x20) * 2 + 1);
            vars->vdp2MemoryOffset+=2;
            vars->cursorX++;
            break;
        }

        if (vars->windowWidth)
        {
            if (vars->windowX2 - vars->cursorX < 1)
            {
                printVdp2StringNewLine(vars);
            }
        }

        if(vars->windowHeight)
        {
            if (vars->windowY2 - vars->cursorY < 1)
            {
                break;
            }
        }
    }

    vars->string--;
}

void drawInventoryString(const char* string)
{
    s_stringStatusQuery vars;
    addStringToVdp2(string, &vars);
    printVdp2String(&vars);
    moveVdp2TextCursor(&vars);
}

const char* dragonArchetypesNames[DR_ARCHETYPE_MAX] =
{
    "   NORMAL   ",
    "  DEFENSE ",
    "  ATTACK  ",
    "  AGILITY ",
    " SPIRITUAL"
};

void drawDragonMenuStatsTaskDraw(p_workArea)
{
    setupVDP2StringRendering(30, 36, 14, 14);
    vdp2PrintStatus.palette = 0xC000;
    
    vdp2StringContext.field_0 = 0;
    vdp2StringContext.cursorX = vdp2StringContext.X;
    vdp2StringContext.cursorY = vdp2StringContext.Y;

    switch (mainGameState.gameStats.m1_dragonLevel)
    {
    case DR_LEVEL_0_BASIC_WING:
        drawInventoryString(" BASE TYPE");
        break;
    case DR_LEVEL_6_LIGHT_WING:
        drawInventoryString(" ULTIMATE ");
        break;
    case DR_LEVEL_8_FLOATER:
        drawInventoryString("  Floater ");
        break;
    default:
        drawInventoryString(dragonArchetypesNames[mainGameState.gameStats.dragonArchetype]);
        break;
    }

    vdp2StringContext.cursorX = vdp2StringContext.X;
    vdp2StringContext.cursorY = vdp2StringContext.Y + 3;
    drawInventoryString("ATT PWR");
    vdp2DebugPrintSetPosition(37, 39);
    vdp2PrintfLargeFont("%3d", mainGameState.gameStats.dragonAtt);

    vdp2StringContext.cursorX = vdp2StringContext.X;
    vdp2StringContext.cursorY = vdp2StringContext.Y + 6;
    drawInventoryString("DEF PWR");
    vdp2DebugPrintSetPosition(37, 42);
    vdp2PrintfLargeFont("%3d", mainGameState.gameStats.dragonDef);

    vdp2StringContext.cursorX = vdp2StringContext.X;
    vdp2StringContext.cursorY = vdp2StringContext.Y + 9;
    drawInventoryString("SPR PWR");
    vdp2DebugPrintSetPosition(37, 45);
    vdp2PrintfLargeFont("%3d", mainGameState.gameStats.dragonSpr);

    vdp2StringContext.cursorX = vdp2StringContext.X;
    vdp2StringContext.cursorY = vdp2StringContext.Y + 12;
    drawInventoryString("AGL PWR");
    vdp2DebugPrintSetPosition(37, 48);
    vdp2PrintfLargeFont("%3d", mainGameState.gameStats.dragonAgl);

    if (mainGameState.gameStats.m1_dragonLevel < 8)
    {
        if (mainGameState.getBit(0x1B, 5))
        {
            setupVDP2StringRendering(25, 56, 16, 2);
            VDP2DrawString(mainGameState.gameStats.dragonName);
        }
    }

}

s_taskDefinition dragonMenuStatsTaskDefinition = { drawDragonMenuStatsTaskInit, NULL, drawDragonMenuStatsTaskDraw, dummyTaskDelete, "dragonMenuStatsTask" };
s_taskDefinition dragonMenuStatsTask2Definition = { NULL, NULL, dummyTaskDraw, NULL, "dragonMenuStatsTask2" };
s_taskDefinition dragonMenuMorphCursorTaskDefinition = { dummyTaskInit, NULL, dummyTaskDraw, NULL, "dragonMenuMorphCursorTask" };

void startVdp2LayerScroll(s32 layerId, s32 x, s32 y, s32 numSteps)
{
    graphicEngineStatus.layersConfig[layerId].scrollIncX = x;
    graphicEngineStatus.layersConfig[layerId].scrollIncY = y;
    graphicEngineStatus.layersConfig[layerId].field_8 = numSteps;
}

void dragonMenuTaskUpdate(p_workArea pTypelessWorkArea)
{
    s_dragonMenuWorkArea* pWorkArea = static_cast<s_dragonMenuWorkArea*>(pTypelessWorkArea);

    switch (pWorkArea->field_0)
    {
    case 0:
        if (graphicEngineStatus.field_40AC.menuId == 1)
        {
            initVdp2ForDragonMenu(0);
            pWorkArea->field_8 = 16;
            startVdp2LayerScroll(0, -10, 0, 16);
            startVdp2LayerScroll(1, 0, -16, 16);
            pWorkArea->field_0 = 1;
            return;
        }
        else
        {
            initVdp2ForDragonMenu(1);
            pWorkArea->field_0 = 2;
        }
        break;
    case 1:
        if (pWorkArea->field_8 < 3)
        {
            graphicEngineStatus.field_40AC.field_5 = 1;
        }
        if (--pWorkArea->field_8)
        {
            return;
        }
        pWorkArea->field_0++;
    case 2:
        vblankData.field_14 = 2;
        
        pWorkArea->field_C = createDragonMenuMorhTask(pWorkArea);

        pWorkArea->field_10 = createSubTask(pWorkArea, &dragonMenuStatsTaskDefinition, new s_workArea);
        pWorkArea->field_14 = createSubTask(pWorkArea, &dragonMenuStatsTask2Definition, new s_dummyWorkArea);
        pWorkArea->field_1C = createSubTask(pWorkArea, &dragonMenuMorphCursorTaskDefinition, new s_dummyWorkArea);

        if (graphicEngineStatus.field_40AC.menuId != 1)
        {
            fadePalette(&menuUnk0.m_field0, 0xC210, 0xC210, 1);
            fadePalette(&menuUnk0.m_field24, 0xC210, 0xC210, 1);
        }
        pWorkArea->field_0++;
        break;
    case 3:
        if (graphicEngineStatus.field_4514[0].current.field_8 & 7)
        {
            playSoundEffect(0);
            pWorkArea->field_14->getTask()->m_pLateUpdate = NULL;
            if (pWorkArea->field_10)
            {
                pWorkArea->field_10->getTask()->markFinished();
            }
            if (pWorkArea->field_18)
            {
                pWorkArea->field_18->getTask()->markFinished();
            }

            vblankData.field_14 = pWorkArea->field_4;
            if (graphicEngineStatus.field_40AC.menuId != 1)
            {
                fadePalette(&menuUnk0.m_field0, 0, 0, 1);
                fadePalette(&menuUnk0.m_field24, 0, 0, 1);
                pWorkArea->field_0 = 6;
            }
            else
            {
                pWorkArea->field_8 = 16;
                startVdp2LayerScroll(0, 10, 0, 16);
                startVdp2LayerScroll(1, 0, 16, 16);
                pWorkArea->field_0 = 4;
            }
        }
        break;
    case 4:
        if (pWorkArea->field_C)
        {
            pWorkArea->field_C->getTask()->markFinished();
        }
        graphicEngineStatus.field_40AC.field_5 = 0;
        pWorkArea->field_0++;
    case 5:
        if (--pWorkArea->field_8)
        {
            break;
        }
        pWorkArea->field_0++;
    case 6:
        if (pWorkArea)
        {
            pWorkArea->getTask()->markFinished();
        }
        break;
    default:
        assert(0);
        break;
    }
}

void dragonMenuTaskDelete(p_workArea pTypelessWorkArea)
{
    unimplemented("dragonMenuTaskDelete");
}

s_taskDefinition dragonMenuTaskDefinition = { dragonMenuTaskInit, dragonMenuTaskUpdate, NULL, dragonMenuTaskDelete, "dragon Menu" };

p_workArea createMainDragonMenuTask(p_workArea workArea)
{
    return createSubTask(workArea, &dragonMenuTaskDefinition, new s_dragonMenuWorkArea);
}
