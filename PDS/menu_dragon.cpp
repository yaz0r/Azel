#include "PDS.h"

struct s_dragonMenuSubTask1WorkArea : public s_workAreaTemplate<s_dragonMenuSubTask1WorkArea>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &s_dragonMenuSubTask1WorkArea::dragonMenuSubTask1Init, NULL, &s_dragonMenuSubTask1WorkArea::dragonMenuSubTask1Draw, &s_dragonMenuSubTask1WorkArea::dragonMenuSubTask1Delete, "dragonMenuSubTask1" };
        return &taskDefinition;
    }

    static void dragonMenuSubTask1Init(s_dragonMenuSubTask1WorkArea*);
    static void dragonMenuSubTask1Draw(s_dragonMenuSubTask1WorkArea*);
    static void dragonMenuSubTask1Delete(s_dragonMenuSubTask1WorkArea*);

    u32 status; //0
};

void s_dragonMenuSubTask1WorkArea::dragonMenuSubTask1Init(s_dragonMenuSubTask1WorkArea*)
{
    PDS_unimplemented("dragonMenuSubTask1Init");
}

void dragonMenuSubTaskAdjustSoundFromCursor()
{
    PDS_unimplemented("dragonMenuSubTaskAdjustSoundFromCursor");
}

void dragonMenuSubTask1DrawSub1()
{
    PDS_unimplemented("dragonMenuSubTask1DrawSub1");
}

void s_dragonMenuSubTask1WorkArea::dragonMenuSubTask1Draw(s_dragonMenuSubTask1WorkArea* pWorkArea)
{
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

void s_dragonMenuSubTask1WorkArea::dragonMenuSubTask1Delete(s_dragonMenuSubTask1WorkArea*)
{
    PDS_unimplemented("dragonMenuSubTask1Delete");
}

struct s_dragonMenuStatsTask2 : public s_workAreaTemplate<s_dragonMenuStatsTask2>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { NULL, NULL, &s_dragonMenuStatsTask2::Draw, NULL, "dragonMenuStatsTask2" };
        return &taskDefinition;
    }

    static void Draw(s_dragonMenuStatsTask2*)
    {
        PDS_unimplemented("s_dragonMenuStatsTask2::Draw");
    }
};

struct s_dragonMenuWorkArea : public s_workAreaTemplate<s_dragonMenuWorkArea>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &s_dragonMenuWorkArea::dragonMenuTaskInit, &s_dragonMenuWorkArea::dragonMenuTaskUpdate, NULL, &s_dragonMenuWorkArea::dragonMenuTaskDelete, "dragon Menu" };
        return &taskDefinition;
    }

    static void dragonMenuTaskInit(s_dragonMenuWorkArea*);
    static void dragonMenuTaskUpdate(s_dragonMenuWorkArea*);
    static void dragonMenuTaskDelete(s_dragonMenuWorkArea*);

    u32 m0;
    u32 m4;
    u32 m8; // 8
    p_workArea mC;
    p_workArea m10;
    s_dragonMenuStatsTask2* m14;
    p_workArea m18;
    p_workArea m1C;
};

void s_dragonMenuWorkArea::dragonMenuTaskInit(s_dragonMenuWorkArea* pThis)
{
    graphicEngineStatus.m40AC.m9 = 3;
    pThis->m4 = vblankData.m14;

    createSubTask<s_dragonMenuSubTask1WorkArea>(pThis);
}

void setVdp2LayerScroll(u32 r4, u32 r5, u32 r6)
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

    graphicEngineStatus.m40BC_layersConfig[r4].scrollX = r5;
    graphicEngineStatus.m40BC_layersConfig[r4].scrollY = r7 + r6;
}

void resetVdp2LayersAutoScroll()
{
    for (int i = 0; i < 4; i++)
    {
        graphicEngineStatus.m40BC_layersConfig[i].m8_scrollFrameCount = 0;
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
    for (int x = vdp2StringContext.mC_X; x < vdp2StringContext.mC_X + vdp2StringContext.m14_Width; x++)
    {
        for (int y = vdp2StringContext.m10_Y; y < vdp2StringContext.m10_Y + vdp2StringContext.m18_Height; y++)
        {
            clearVdp2TextAreaSub1(0, x, y);
        }
    }
}

void initVdp2ForDragonMenu(u32 r4)
{
    if (r4)
    {
        setVdp2LayerScroll(0, 0, 0);
        setVdp2LayerScroll(1, 0, 0);
        setVdp2LayerScroll(3, 0, 0x100);
        
        resetVdp2LayersAutoScroll();

        setupVDP2StringRendering(0, 34, 44, 28);

        clearVdp2TextArea();
    }

    unpackGraphicsToVDP2(COMMON_DAT + 0xFE38, getVdp2Vram(0x71000));
}

struct s_drawDragonMenuStatsTask : public s_workAreaTemplate<s_drawDragonMenuStatsTask>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &s_drawDragonMenuStatsTask::drawDragonMenuStatsTaskInit, NULL, &s_drawDragonMenuStatsTask::drawDragonMenuStatsTaskDraw, &s_drawDragonMenuStatsTask::drawDragonMenuStatsTaskDelete, "dragonMenuStatsTask" };
        return &taskDefinition;
    }

    static void drawDragonMenuStatsTaskInit(s_drawDragonMenuStatsTask*);
    static void drawDragonMenuStatsTaskDraw(s_drawDragonMenuStatsTask*);
    static void drawDragonMenuStatsTaskDelete(s_drawDragonMenuStatsTask*);
};

void s_drawDragonMenuStatsTask::drawDragonMenuStatsTaskInit(s_drawDragonMenuStatsTask*)
{
    setActiveFont(graphicEngineStatus.m40AC.fontIndex);
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

void s_drawDragonMenuStatsTask::drawDragonMenuStatsTaskDraw(s_drawDragonMenuStatsTask*)
{
    setupVDP2StringRendering(30, 36, 14, 14);
    vdp2PrintStatus.palette = 0xC000;
    
    vdp2StringContext.m0 = 0;
    vdp2StringContext.m4_cursorX = vdp2StringContext.mC_X;
    vdp2StringContext.m8_cursorY = vdp2StringContext.m10_Y;

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

    vdp2StringContext.m4_cursorX = vdp2StringContext.mC_X;
    vdp2StringContext.m8_cursorY = vdp2StringContext.m10_Y + 3;
    drawInventoryString("ATT PWR");
    vdp2DebugPrintSetPosition(37, 39);
    vdp2PrintfLargeFont("%3d", mainGameState.gameStats.dragonAtt);

    vdp2StringContext.m4_cursorX = vdp2StringContext.mC_X;
    vdp2StringContext.m8_cursorY = vdp2StringContext.m10_Y + 6;
    drawInventoryString("DEF PWR");
    vdp2DebugPrintSetPosition(37, 42);
    vdp2PrintfLargeFont("%3d", mainGameState.gameStats.dragonDef);

    vdp2StringContext.m4_cursorX = vdp2StringContext.mC_X;
    vdp2StringContext.m8_cursorY = vdp2StringContext.m10_Y + 9;
    drawInventoryString("SPR PWR");
    vdp2DebugPrintSetPosition(37, 45);
    vdp2PrintfLargeFont("%3d", mainGameState.gameStats.dragonSpr);

    vdp2StringContext.m4_cursorX = vdp2StringContext.mC_X;
    vdp2StringContext.m8_cursorY = vdp2StringContext.m10_Y + 12;
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

void s_drawDragonMenuStatsTask::drawDragonMenuStatsTaskDelete(s_drawDragonMenuStatsTask*)
{
    setupVDP2StringRendering(0, 34, 44, 28);
    clearVdp2TextArea();
}

struct s_dragonMenuMorphCursorTask : public s_workAreaTemplate<s_dragonMenuMorphCursorTask>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &s_dragonMenuMorphCursorTask::Init, NULL, &s_dragonMenuMorphCursorTask::Draw, NULL, "dragonMenuMorphCursorTask" };
        return &taskDefinition;
    }

    static void Init(s_dragonMenuMorphCursorTask*)
    {
        PDS_unimplemented("s_dragonMenuMorphCursorTask::Init");
    }

    static void Draw(s_dragonMenuMorphCursorTask*)
    {
        PDS_unimplemented("s_dragonMenuMorphCursorTask::Draw");
    }
};

void startVdp2LayerScroll(s32 layerId, s32 x, s32 y, s32 numSteps)
{
    graphicEngineStatus.m40BC_layersConfig[layerId].scrollIncX = x;
    graphicEngineStatus.m40BC_layersConfig[layerId].scrollIncY = y;
    graphicEngineStatus.m40BC_layersConfig[layerId].m8_scrollFrameCount = numSteps;
}

void s_dragonMenuWorkArea::dragonMenuTaskUpdate(s_dragonMenuWorkArea* pWorkArea)
{
    switch (pWorkArea->m0)
    {
    case 0:
        if (graphicEngineStatus.m40AC.m0_menuId == 1)
        {
            initVdp2ForDragonMenu(0);
            pWorkArea->m8 = 16;
            startVdp2LayerScroll(0, -10, 0, 16);
            startVdp2LayerScroll(1, 0, -16, 16);
            pWorkArea->m0 = 1;
            return;
        }
        else
        {
            initVdp2ForDragonMenu(1);
            pWorkArea->m0 = 2;
        }
        break;
    case 1:
        if (pWorkArea->m8 < 3)
        {
            graphicEngineStatus.m40AC.m5 = 1;
        }
        if (--pWorkArea->m8)
        {
            return;
        }
        pWorkArea->m0++;
    case 2:
        vblankData.m14 = 2;
        
        pWorkArea->mC = createDragonMenuMorhTask(pWorkArea);

        pWorkArea->m10 = createSubTask<s_drawDragonMenuStatsTask>(pWorkArea);
        pWorkArea->m14 = createSubTask<s_dragonMenuStatsTask2>(pWorkArea);
        pWorkArea->m1C = createSubTask<s_dragonMenuMorphCursorTask>(pWorkArea);

        if (graphicEngineStatus.m40AC.m0_menuId != 1)
        {
            fadePalette(&menuUnk0.m_field0, 0xC210, 0xC210, 1);
            fadePalette(&menuUnk0.m_field24, 0xC210, 0xC210, 1);
        }
        pWorkArea->m0++;
        break;
    case 3:
        if (graphicEngineStatus.m4514.m0[0].m0_current.m8_newButtonDown & 7)
        {
            playSoundEffect(0);
            pWorkArea->m14->m_DrawMethod = NULL;
            if (pWorkArea->m10)
            {
                pWorkArea->m10->getTask()->markFinished();
            }
            if (pWorkArea->m18)
            {
                pWorkArea->m18->getTask()->markFinished();
            }

            vblankData.m14 = pWorkArea->m4;
            if (graphicEngineStatus.m40AC.m0_menuId != 1)
            {
                fadePalette(&menuUnk0.m_field0, 0, 0, 1);
                fadePalette(&menuUnk0.m_field24, 0, 0, 1);
                pWorkArea->m0 = 6;
            }
            else
            {
                pWorkArea->m8 = 16;
                startVdp2LayerScroll(0, 10, 0, 16);
                startVdp2LayerScroll(1, 0, 16, 16);
                pWorkArea->m0 = 4;
            }
        }
        break;
    case 4:
        if (pWorkArea->mC)
        {
            pWorkArea->mC->getTask()->markFinished();
        }
        graphicEngineStatus.m40AC.m5 = 0;
        pWorkArea->m0++;
    case 5:
        if (--pWorkArea->m8)
        {
            break;
        }
        pWorkArea->m0++;
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

void s_dragonMenuWorkArea::dragonMenuTaskDelete(s_dragonMenuWorkArea*)
{
    PDS_unimplemented("dragonMenuTaskDelete");
}

p_workArea createMainDragonMenuTask(p_workArea workArea)
{
    return createSubTask<s_dragonMenuWorkArea>(workArea);
}
