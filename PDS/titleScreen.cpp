#include "PDS.h"

struct s_titleMenuEntry
{
    u16 m_var0;
    s16 m_var2;
    const char* m_text;
    void(*m_createTask)();
};

struct s_titleMenuWorkArea
{
    u16 m_status; //0
    
    u16 m_numMenuEntry; // 6
    s_titleMenuEntry* m_menu; // 8
    
};

s_titleMenuEntry mainMenu[] = {
    {1, -6, " NEW GAME ", NULL /*createNewGameTask*/},
    {1, -5, " CONTINUE ", NULL /*createContinueTask*/},
    {1, -4, "TUTORIAL 1", NULL /*createTutorial1Task*/},
    {1, -3, "TUTORIAL 2", NULL /*createTutorial2Task*/},
};

s_titleMenuEntry mainMenuDebug[] = {
    {1, -8, " NEW GAME ", NULL /*createNewGameTask*/},
    {1, -7, " CONTINUE ", NULL /*createContinueTask*/},
    {1, -6, "   TOWN   ", NULL },
    {1, -5, "  FIELD   ", NULL },
    {1, -4, "  BATTLE  ", NULL },
    {1, -3, "  SOUND   ", NULL },
    {1, -2, "  MOVIE   ", NULL },
};


void titleMenuTaskInit(void* pTypelessWorkArea)
{
    s_titleMenuWorkArea* pWorkArea = (s_titleMenuWorkArea*)pTypelessWorkArea;

    // if we are not coming from the title screen (is this possible?)
    if (initialTaskStatus.m_previousTask != createTitleScreenTask)
    {
        assert(0);
    }

    isInMenu2 = 1;
}

void titleMenuTaskDraw(void* pTypelessWorkArea)
{
    s_titleMenuWorkArea* pWorkArea = (s_titleMenuWorkArea*)pTypelessWorkArea;

    switch (pWorkArea->m_status)
    {
    case 0:
        if (keyboardIsKeyDown(0xF6))
        {
            pWorkArea->m_menu = mainMenuDebug;
            pWorkArea->m_numMenuEntry = 7;
        }
        else
        {
            pWorkArea->m_menu = mainMenu;
            pWorkArea->m_numMenuEntry = 4;

            titleMenuToggleTutorials(&mainMenu[2], &mainMenu[3]);
        }
    default:
        assert(0);
        break;
    }
}

s_taskDefinition titleMenuTaskDefinition = { titleMenuTaskInit, NULL, titleMenuTaskDraw, NULL };

s_task* startSegaLogoModule(void* workArea)
{
    assert(false);
    return NULL;
}

s_task* createTitleMenuTask(void* workArea)
{
    return createTask_NoArgs(workArea, &titleMenuTaskDefinition, sizeof(s_titleMenuWorkArea));
}

struct s_pressStartButtonTaskWorkArea
{
    u32 m_status;
    s32 m_timming;
};

void pressStartButtonTaskDraw(void* pTypelessWorkArea)
{
    s_pressStartButtonTaskWorkArea* pWorkArea = (s_pressStartButtonTaskWorkArea*)pTypelessWorkArea;

    switch (pWorkArea->m_status)
    {
    case 0:
        if ((--pWorkArea->m_timming) >= 0)
            return;

        vdp2PrintStatus.field_10 = 0xD000;

        if (VDP2Regs_.TVSTAT & 1)
            vdp2DebugPrintSetPosition(13, -2);
        else
            vdp2DebugPrintSetPosition(13, -5);

        drawMenuString("PRESS START BUTTON");

        pWorkArea->m_timming = 15;
        pWorkArea->m_status++;
    case 1:
        if ((--pWorkArea->m_timming) >= 0)
            return;

        if (VDP2Regs_.TVSTAT & 1)
            vdp2DebugPrintSetPosition(13, -2);
        else
            vdp2DebugPrintSetPosition(13, -5);

        clearVdp2Text();

        pWorkArea->m_status = 0;
        pWorkArea->m_timming = 6;
        break;
    default:
        assert(0);
        break;
    }
}

void pressStartButtonTaskDelete(void* pTypelessWorkArea)
{
    if (VDP2Regs_.TVSTAT & 1)
        vdp2DebugPrintSetPosition(13, -2);
    else
        vdp2DebugPrintSetPosition(13, -5);

    clearVdp2Text();
}

s_taskDefinition pressStartButtonTask = { NULL, NULL, pressStartButtonTaskDraw, pressStartButtonTaskDelete };

struct s_titleScreenWorkArea
{
    u32 m_status;
    u32 m_delay;
    s_task* m_overlayTask;
};

void titleScreenDraw(void* pTypelessWorkArea)
{
    s_titleScreenWorkArea* pWorkArea = (s_titleScreenWorkArea*)pTypelessWorkArea;

    switch (pWorkArea->m_status)
    {
    case 0:
        if (pWorkArea->m_overlayTask)
        {

        }
        pWorkArea->m_status++;
    case 1:
        // ? not sure what this does
        pWorkArea->m_status++;
    case 2:
        pWorkArea->m_delay = 60; // start chrono?
        pWorkArea->m_status++;
    case 3:
        if (debugEnabled)
        {
            assert(0);
        }
        else
        {
            if (--pWorkArea->m_delay)
                return;
        }
        pWorkArea->m_status++;
    case 4:
        createTask_NoArgs(pWorkArea, &pressStartButtonTask, 8);

        pWorkArea->m_delay = 44 * 60;
        if (VDP2Regs_.TVSTAT & 1)
        {
            assert(false);
        }

        pWorkArea->m_status++;
    case 5:
        if (enableDebugTask)
        {
            assert(false);
        }

        if (PortData2.field_8 & 8) // if start was pressed
        {
            playSoundEffect(8);
            pWorkArea->m_status = 7;
            return;
        }

        if (--pWorkArea->m_delay)
            return;

        resetMenu(&menuUnk0, titleScreenDrawSub1(&menuUnk0), 0x8000, 30);
        titleScreenDrawSub3(3);

        pWorkArea->m_status++;
    case 6:

        assert(false);
        //if (menuUnk0[32]) <- wrong?
        {
            initialTaskStatus.m_pendingTask = startSegaLogoModule;
        }
        break;
    case 7:
        initialTaskStatus.m_pendingTask = createTitleMenuTask;
        break;
    default:
        assert(0);
        break;
    }
}

void titleScreenInit(void* pTypelessWorkArea)
{
    s_titleScreenWorkArea* pWorkArea = (s_titleScreenWorkArea*)pTypelessWorkArea;

    pWorkArea->m_overlayTask = TITLE_OVERLAY::overlayStart(pWorkArea);
}

s_taskDefinition titleScreenTaskDefinition = { titleScreenInit, NULL, titleScreenDraw, NULL };

s_task* createTitleScreenTask(void* workArea)
{
    return createTask_NoArgs(workArea, &titleScreenTaskDefinition, sizeof(s_titleScreenWorkArea));
}

// WarningTask

struct s_warningWorkArea
{
    u32 m_status;
    u32 m_delay;
};

u32 checkCartdrigeMemory()
{
    return 1;
}

void warningTaskDraw(void* pTypelessWorkArea)
{
    s_warningWorkArea* pWorkArea = (s_warningWorkArea*)pTypelessWorkArea;

    switch (pWorkArea->m_status)
    {
    case 0: // init delay
        pWorkArea->m_delay = 60;
        pWorkArea->m_status++;
    case 1: // wait delay
        if (--pWorkArea->m_delay)
            return;
        pWorkArea->m_status++;
    case 2: // fade out init?
        if (!(PortData2.field_8 & 8))
            return;
        resetMenu(&menuUnk0, titleScreenDrawSub1(&menuUnk0), 0, 30);
        pWorkArea->m_status++;
    case 3: // wait faide out
        if (!menuUnk0.m_field0.m_field20)
            return;
        if (pTypelessWorkArea)
        {
            getTaskFromWorkArea(pTypelessWorkArea)->m_flags |= 1; // finish task
        }
        break;
    default:
        assert(0);
    }
}

u8 titleScreenPalette[32] = {
    0x00, 0x00, 0x08, 0x42, 0x10, 0x84, 0x18, 0xC6, 0x21, 0x08, 0x29, 0x4A, 0x31, 0x8C, 0x39, 0xCE,
    0x42, 0x10, 0x46, 0x31, 0x52, 0x94, 0x5E, 0xF7, 0x67, 0x39, 0x6F, 0x7B, 0x77, 0xBD, 0x7F, 0xFF,
};

sLayerConfig warningBG0Setup[] =
{
    CHCN,  0, // 16 colors
    CHSZ,  1, // character size is 2 cells x 2 cells (16*16)
    PNB,  1, // pattern data size is 1 word
    CNSM,  1, // character number is 12 bit, no flip
    END,
};

void loadWarningFile(u32 index)
{
    reinitVdp2();

    loadFile("WARNING.SCB", getVdp2Vram(0x10000), 0);
    loadFile("WARNING.PNB", getVdp2Vram(0x1C800), 0);

    asyncDmaCopy(titleScreenPalette, getVdp2Cram(0), 32, 0);

    vdp2Controls.m_pendingVdp2Regs->CYCA0 = 0x3FF47FF;

    setupNBG0(warningBG0Setup);

    initLayerMap(0, 0x1C800 + index * 0x800, 0, 0, 0);

    vdp2Controls.m_pendingVdp2Regs->PRINA = 6;
    vdp2Controls.m_pendingVdp2Regs->PRINB = 0x700;
    vdp2Controls.m_pendingVdp2Regs->PRIR = 0;
    vdp2Controls.m_isDirty = 1;

    if (VDP2Regs_.TVSTAT & 1)
    {
        incrementVar = 0;
        updateVDP2CoordinatesIncrement2(0x0, 0x100000);
        incrementVar = 4;
    }
}

void warningTaskInit(void* pTypelessWorkArea)
{
    s_warningWorkArea* pWorkArea = (s_warningWorkArea*)pTypelessWorkArea;

    u32 cartdrigePresent = checkCartdrigeMemory();
    if (cartdrigePresent == 0)
    {
        if (pTypelessWorkArea)
        {
            s_task* pTask = getTaskFromWorkArea(pTypelessWorkArea);
            pTask->m_flags |= 1;
        }
        return;
    }

    if (cartdrigePresent == 1)
    {
        s_task* pTask = getTaskFromWorkArea(pTypelessWorkArea);
        pTask->m_pLateUpdate = warningTaskDraw;
    }

    loadWarningFile(cartdrigePresent - 1);

    if (menuUnk0.m_4D >= menuUnk0.m_4C)
    {
        vdp2Controls.m_registers[0].N1COSL = 0;
        vdp2Controls.m_registers[1].N1COSL = 0;
    }

    resetMenu(&menuUnk0, titleScreenDrawSub1(&menuUnk0), menuUnk0.m_48, 30);
}

s_taskDefinition warningTaskDefinition = { warningTaskInit, NULL, NULL, NULL };

s_task* startWarningTask(void* workArea)
{
    return createTask_NoArgs(workArea, &warningTaskDefinition, sizeof(s_warningWorkArea));
}

// loadWarningTask

struct s_loadWarningWorkArea
{
    u32 m_0;
    u32 m_4;
    s_task* m_warningTask;
};

void loadWarningTaskInit(void* pTypelessWorkArea)
{
    s_loadWarningWorkArea* pWorkArea = (s_loadWarningWorkArea*)pTypelessWorkArea;

    pWorkArea->m_warningTask = startWarningTask(pTypelessWorkArea);
}

void loadWarningTaskDraw(void* pTypelessWorkArea)
{
    s_loadWarningWorkArea* pWorkArea = (s_loadWarningWorkArea*)pTypelessWorkArea;

    if (pWorkArea->m_warningTask)
    {
        if(!(pWorkArea->m_warningTask->m_flags & 1))
        {
            return;
        }
    }

    /*
    if (playIntroMovie)
    {
        initialTaskStatus.m_pendingTask = startTitleScreenVideo;
    }
    else*/
    {
        initialTaskStatus.m_pendingTask = createTitleScreenTask;
    }
}

s_taskDefinition loadWarningTaskDefinition = { loadWarningTaskInit, NULL, loadWarningTaskDraw, NULL };

s_task* startLoadWarningTask(void* workArea)
{
    return createTask_NoArgs(workArea, &loadWarningTaskDefinition, sizeof(s_loadWarningWorkArea));
}

