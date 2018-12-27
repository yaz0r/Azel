#include "PDS.h"

struct s_titleMenuEntry
{
    u16 m_isEnabled;
    s16 m_var2;
    const char* m_text;
    p_workArea(*m_createTask)(p_workArea);
};

s_titleMenuEntry mainMenu[] = {
    {1, -6, " NEW GAME ", NULL /*createNewGameTask*/},
    {1, -5, " CONTINUE ", NULL /*createContinueTask*/},
    {1, -4, "TUTORIAL 1", NULL /*createTutorial1Task*/},
    {1, -3, "TUTORIAL 2", NULL /*createTutorial2Task*/},
};

s_titleMenuEntry mainMenuDebug[] = {
    {1, -8, " NEW GAME ", createNewGameTask },
    {1, -7, " CONTINUE ", createContinueTask },
    {1, -6, "   TOWN   ", createTownDebugTask },
    {1, -5, "  FIELD   ", createFieldDebugTask },
    {1, -4, "  BATTLE  ", createBattleDebugTask },
    {1, -3, "  SOUND   ", createSoundDebugTask },
    {1, -2, "  MOVIE   ", createMovieDebugTask },
};

bool hasSaveGame()
{
    PDS_unimplemented("hasSaveGame");
    return false;
}

struct s_titleMenuWorkArea : public s_workAreaTemplate<s_titleMenuWorkArea>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &s_titleMenuWorkArea::Init, NULL, &s_titleMenuWorkArea::Draw, NULL, "titleMenu" };
        return &taskDefinition;
    }

    void Init()
    {
        // if we are not coming from the title screen (is this possible?)
        if (initialTaskStatus.m_previousTask != createTitleScreenTask)
        {
            assert(0);
        }

        isInMenu2 = 1;
    }

    void Draw()
    {
        s_titleMenuWorkArea* pWorkArea = this;

        switch (pWorkArea->m_status)
        {
        case 0:
            PDS_warning("Setting up menu as debug");
            //if (keyboardIsKeyDown(0xF6))
        {
            pWorkArea->m_menu = mainMenuDebug;
            pWorkArea->m_numMenuEntry = 7;
        }
        /*else
        {
            pWorkArea->m_menu = mainMenu;
            pWorkArea->m_numMenuEntry = 4;

            titleMenuToggleTutorials(&mainMenu[2], &mainMenu[3]);
        }*/
        pWorkArea->m_vertialLocation = 0;

        if (VDP2Regs_.TVSTAT & 1)
        {
            assert(0);
        }

        // make "continue" the default
        pWorkArea->m_currentSelection = 1;
        pWorkArea->m_menu[0].m_isEnabled = (azelCdNumber == 0); // new game enabled only is first disk

        // if first disk and no saves, make "new game" the default
        if ((azelCdNumber == 0) && (!hasSaveGame()))
        {
            pWorkArea->m_currentSelection = 0;
        }

        {
            u32 numActiveEntries = 0;
            u32 palette = 0x9000;
            for (u32 i = 0; i < pWorkArea->m_numMenuEntry; i++)
            {
                if (pWorkArea->m_menu[i].m_isEnabled)
                {
                    numActiveEntries++;

                    if (numActiveEntries > 1)
                    {
                        vdp2PrintStatus.palette = palette;

                        vdp2DebugPrintSetPosition(0xF, pWorkArea->m_vertialLocation + 0x17);
                        drawLineLargeFont("\x40"); // previous

                        vdp2DebugPrintSetPosition(0x1C, pWorkArea->m_vertialLocation + 0x17);
                        drawLineLargeFont("\x7F"); // next

                        break;
                    }
                }
            }
        }

        initRandomSeed(0);
        pWorkArea->m_status++;
        case 1:
            randomNumber(); // to increment seed

            // start or A?
#ifndef __IPHONEOS__
            if (graphicEngineStatus.m4514.m0[0].m0_current.m8_newButtonDown & 0xE)
#endif
            {
                playSoundEffect(0);

                clearVdp2TextMemory();
                initialTaskStatus.m_pendingTask = pWorkArea->m_menu[pWorkArea->m_currentSelection].m_createTask;

                titleScreenDrawSub3(2);

                // fade out to black
                fadePalette(&menuUnk0.m_field0, titleScreenDrawSub1(&menuUnk0), 0x8000, 30);
                return;
            }

            if (graphicEngineStatus.m4514.m0[0].m0_current.mC_newButtonDown2 & 0x40)
            {
                int newSelection = pWorkArea->m_currentSelection;
                do
                {
                    newSelection--;
                    if (newSelection < 0)
                    {
                        newSelection = pWorkArea->m_numMenuEntry - 1;
                    }
                } while (!pWorkArea->m_menu[newSelection].m_isEnabled);

                if (newSelection != pWorkArea->m_currentSelection)
                {
                    playSoundEffect(10);
                    pWorkArea->m_currentSelection = newSelection;
                    pWorkArea->m_status2 = 1;
                    pWorkArea->m_blinkDelay = 15;
                }
            }

            if (graphicEngineStatus.m4514.m0[0].m0_current.mC_newButtonDown2 & 0x80)
            {
                int newSelection = pWorkArea->m_currentSelection;
                do
                {
                    newSelection++;
                    if (newSelection > pWorkArea->m_numMenuEntry - 1)
                    {
                        newSelection = 0;
                    }
                } while (!pWorkArea->m_menu[newSelection].m_isEnabled);

                if (newSelection != pWorkArea->m_currentSelection)
                {
                    playSoundEffect(10);
                    pWorkArea->m_currentSelection = newSelection;
                    pWorkArea->m_status2 = 1;
                    pWorkArea->m_blinkDelay = 15;
                }
            }

            switch (pWorkArea->m_status2)
            {
            case 0:
                if ((--pWorkArea->m_blinkDelay) >= 0)
                {
                    return;
                }
                pWorkArea->m_blinkDelay = 6;

                pWorkArea->m_status2++;
            case 1:
                vdp2PrintStatus.palette = 0xC000;
                vdp2DebugPrintSetPosition(0x11, pWorkArea->m_vertialLocation + 0x17);
                drawLineLargeFont(pWorkArea->m_menu[pWorkArea->m_currentSelection].m_text);

                pWorkArea->m_status2++;
            case 2:
                if ((--pWorkArea->m_blinkDelay) >= 0)
                {
                    return;
                }
                vdp2DebugPrintSetPosition(0x11, pWorkArea->m_vertialLocation + 0x17);
                clearVdp2TextLargeFont();

                pWorkArea->m_blinkDelay = 3;
                pWorkArea->m_status2 = 0;
                break;
            }

            break;

        default:
            assert(0);
            break;
        }
    }

    u16 m_status; //0
    u16 m_status2; //2
    u16 m_currentSelection; // 4
    u16 m_numMenuEntry; // 6
    s_titleMenuEntry* m_menu; // 8
    u16 m_vertialLocation; // C
    s16 m_blinkDelay; // E
    
};

p_workArea startSegaLogoModule(p_workArea workArea)
{
    assert(false);
    return NULL;
}

p_workArea createTitleMenuTask(p_workArea workArea)
{
    return createSubTask<s_titleMenuWorkArea>(workArea);
}

struct s_pressStartButtonTaskWorkArea : public s_workAreaTemplate<s_pressStartButtonTaskWorkArea>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { nullptr, nullptr, &s_pressStartButtonTaskWorkArea::pressStartButtonTaskDraw, &s_pressStartButtonTaskWorkArea::pressStartButtonTaskDelete, "pressStartButtonTask" };
        return &taskDefinition;
    }

    void pressStartButtonTaskDraw()
    {
        s_pressStartButtonTaskWorkArea* pWorkArea = this;

        switch (pWorkArea->m_status)
        {
        case 0:
            if ((--pWorkArea->m_timming) >= 0)
                return;

            vdp2PrintStatus.palette = 0xD000;

            if (VDP2Regs_.TVSTAT & 1)
                vdp2DebugPrintSetPosition(13, -2);
            else
                vdp2DebugPrintSetPosition(13, -5);

            drawLineLargeFont("PRESS START BUTTON");

            pWorkArea->m_timming = 15;
            pWorkArea->m_status++;
        case 1:
            if ((--pWorkArea->m_timming) >= 0)
                return;

            if (VDP2Regs_.TVSTAT & 1)
                vdp2DebugPrintSetPosition(13, -2);
            else
                vdp2DebugPrintSetPosition(13, -5);

            clearVdp2TextLargeFont();

            pWorkArea->m_status = 0;
            pWorkArea->m_timming = 6;
            break;
        default:
            assert(0);
            break;
        }
    }

    void pressStartButtonTaskDelete()
    {
        if (VDP2Regs_.TVSTAT & 1)
            vdp2DebugPrintSetPosition(13, -2);
        else
            vdp2DebugPrintSetPosition(13, -5);

        clearVdp2TextLargeFont();
    }

    u32 m_status;
    s32 m_timming;
};

struct s_titleScreenWorkArea : public s_workAreaTemplate<s_titleScreenWorkArea>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &s_titleScreenWorkArea::Init, NULL, &s_titleScreenWorkArea::Draw, NULL, "s_titleScreenWorkArea" };
        return &taskDefinition;
    }

    void Init();
    void Draw();

    u32 m_status;
    u32 m_delay;
    p_workArea m_overlayTask;
};

void s_titleScreenWorkArea::Draw()
{
    s_titleScreenWorkArea* pWorkArea = this;

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
        createSubTask<s_pressStartButtonTaskWorkArea>(pWorkArea);

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

#ifndef __IPHONEOS__
        if (graphicEngineStatus.m4514.m0[0].m0_current.m8_newButtonDown & 8) // if start was pressed
#endif
        {
            playSoundEffect(8);
            pWorkArea->m_status = 7;
            return;
        }

        if (--pWorkArea->m_delay)
            return;

        fadePalette(&menuUnk0.m_field0, titleScreenDrawSub1(&menuUnk0), 0x8000, 30);
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

void s_titleScreenWorkArea::Init()
{
    m_overlayTask = TITLE_OVERLAY::overlayStart(this);
}

p_workArea createTitleScreenTask(p_workArea workArea)
{
    return createSubTask<s_titleScreenWorkArea>(workArea);
}

// WarningTask

struct s_warningWorkArea : public s_workAreaTemplate< s_warningWorkArea>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &s_warningWorkArea::Init, NULL, NULL, NULL, "sFieldA3_1_fieldIntroTask" };
        return &taskDefinition;
    }

    void Init();
    void Draw(); // not by default, only setup if necessary

    u32 m_status;
    u32 m_delay;
};

u32 checkCartdrigeMemory()
{
    return 0;
}

void s_warningWorkArea::Draw()
{
    s_warningWorkArea* pWorkArea = this;

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
        if (!(graphicEngineStatus.m4514.m0[0].m0_current.m8_newButtonDown & 8))
            return;
        fadePalette(&menuUnk0.m_field0, titleScreenDrawSub1(&menuUnk0), 0, 30);
        pWorkArea->m_status++;
    case 3: // wait fade out
        if (!menuUnk0.m_field0.m_field20)
            return;
        getTask()->m14_flags |= 1; // finish task
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

    vdp2Controls.m4_pendingVdp2Regs->CYCA0 = 0x3FF47FF;

    setupNBG0(warningBG0Setup);

    initLayerMap(0, 0x1C800 + index * 0x800, 0, 0, 0);

    vdp2Controls.m4_pendingVdp2Regs->PRINA = 6;
    vdp2Controls.m4_pendingVdp2Regs->PRINB = 0x700;
    vdp2Controls.m4_pendingVdp2Regs->PRIR = 0;
    vdp2Controls.m_isDirty = 1;

    if (VDP2Regs_.TVSTAT & 1)
    {
        pauseEngine[4] = 0;
        setupVDP2CoordinatesIncrement2(0x0, 0x100000);
        pauseEngine[4] = 4;
    }
}

void s_warningWorkArea::Init()
{
    s_warningWorkArea* pWorkArea = this;

    u32 cartdrigePresent = checkCartdrigeMemory();
    if (cartdrigePresent == 0)
    {
        if (pWorkArea)
        {
            s_task* pTask = pWorkArea->getTask();
            pTask->m14_flags |= 1;
        }
        return;
    }

    if (cartdrigePresent == 1)
    {
        s_task* pTask = pWorkArea->getTask();
        pTask->mC_pDraw = &s_warningWorkArea::StaticDraw;
        m_DrawMethod = &s_warningWorkArea::Draw;
    }

    loadWarningFile(cartdrigePresent - 1);

    if (menuUnk0.m_4D >= menuUnk0.m_4C)
    {
        vdp2Controls.m20_registers[0].N1COSL = 0;
        vdp2Controls.m20_registers[1].N1COSL = 0;
    }

    fadePalette(&menuUnk0.m_field0, titleScreenDrawSub1(&menuUnk0), menuUnk0.m_48, 30);
}

p_workArea startWarningTask(s_workArea* workArea)
{
    return createSubTask<s_warningWorkArea>(workArea);
}

struct s_loadWarningWorkArea : public s_workAreaTemplate<s_loadWarningWorkArea>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = {  &s_loadWarningWorkArea::Init, nullptr, &s_loadWarningWorkArea::Draw, nullptr, "s_loadWarningWorkArea"};
        return &taskDefinition;
    }

    void Init()
    {
        m_warningTask = startWarningTask(this);
    }
    void Draw()
    {
        if (m_warningTask)
        {
            if (!(m_warningTask->getTask()->m14_flags & 1))
            {
                return;
            }
        }

        PDS_warning("Skipping over cd 0 intro");
        /*
        if (azelCdNumber == 0)
        {
        initialTaskStatus.m_pendingTask = startTitleScreenVideo;
        }
        else*/
        {
            initialTaskStatus.m_pendingTask = createTitleScreenTask;
        }
    }

    u32 m_0;
    u32 m_4;
    p_workArea m_warningTask;
};

p_workArea startLoadWarningTask(s_workArea* workArea)
{
    return createSubTask<s_loadWarningWorkArea>(workArea);
}

