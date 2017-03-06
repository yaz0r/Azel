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
    u32 m_0;
    u32 m_4;
};

void warningTaskInit(void* pTypelessWorkArea)
{
    s_warningWorkArea* pWorkArea = (s_warningWorkArea*)pTypelessWorkArea;

    /*
    if (checkSaves())
    {
        assert(0);
    }
    ...
    */



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

