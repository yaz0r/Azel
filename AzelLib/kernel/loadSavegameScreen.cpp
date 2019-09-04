#include "PDS.h"
#include "town/town.h"
#include "menuCursor.h"
#include "menuSprite.h"
#include "loadSavegameScreen.h"

struct sLoadSavegameScreen : public s_workAreaTemplateWithArg<sLoadSavegameScreen, p_workArea>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sLoadSavegameScreen::Init, nullptr, &sLoadSavegameScreen::Draw, &sLoadSavegameScreen::Delete };
        return &taskDefinition;
    }

    static void Init(sLoadSavegameScreen* pThis, p_workArea pParent)
    {
        pThis->m4 = pParent;
        resetTempAllocators();
        initDramAllocator(pThis, townBuffer, 0x28000, nullptr);
        setOpenMenu7();
        fadePalette(&g_fadeControls.m0_fade0, 0, 0, 1);
        fadePalette(&g_fadeControls.m24_fade1, 0, 0, 1);
    }
    static void Draw(sLoadSavegameScreen*)
    {
        FunctionUnimplemented();
    }
    static void Delete(sLoadSavegameScreen*)
    {
        FunctionUnimplemented();
    }

    p_workArea m4;
    //size: 8
};

p_workArea initLoadSavegameScreen(p_workArea parent, s32)
{
    return createSubTaskWithArg<sLoadSavegameScreen, p_workArea>(parent, parent);
}

u8* backupMemoryForSaveScreen()
{
    FunctionUnimplemented();
    return nullptr;
}

void loadSaveBackgroundSub0(const std::vector<std::array<s32, 2>> & r4)
{
    FunctionUnimplemented();
}

void loadSaveBackground()
{
    vdp2Controls.m4_pendingVdp2Regs->m14_CYCA1 = 0x1ff4455;
    vdp2Controls.m4_pendingVdp2Regs->m1C_CYCB1 = 0xffffffff;
    vdp2Controls.m_isDirty = 1;

    static const std::vector<std::array<s32, 2>> config = {
        {
            {0x2C, 0x1},
        }
    };

    loadSaveBackgroundSub0(config);
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xFFF0) | 3;
    vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL = (vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL & 0xFEFF);
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xF8FF) | 0x600;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xCFFF) | 0x1000;
    vdp2Controls.m4_pendingVdp2Regs->m100_CCRSA = 6;
    vdp2Controls.m4_pendingVdp2Regs->m102_CCRSB = 0;
    vdp2Controls.m4_pendingVdp2Regs->m104_CCRSC = 0;
    vdp2Controls.m4_pendingVdp2Regs->m106_CCRSD = 0;
    vdp2Controls.m_isDirty = 1;

    static const sLayerConfig layerConfig[] =
    {
        m2_CHCN,  1, // 256 colors
        m5_CHSZ,  1, // character size is 2 cells x 2 cells (16*16)
        m6_PNB,  1, // pattern data size is 1 word
        m7_CNSM,  1, // character number is 12 bit, no flip
        m12_PLSZ, 0, // plans is 1H x 1V
        m40_CAOS, 5,
        m0_END,
    };

    setupNBG0(layerConfig);
    setupNBG1(layerConfig);

    initLayerMap(0, 0x20000, 0x20000, 0x20000, 0x20000);
    initLayerMap(1, 0x20800, 0x20800, 0x20800, 0x20800);

    vdp2Controls.m4_pendingVdp2Regs->mF8_PRINA = 0x304;
    vdp2Controls.m4_pendingVdp2Regs->mFA_PRINB = 0x700;
    vdp2Controls.m4_pendingVdp2Regs->mFC_PRIR = 0;
    vdp2Controls.m4_pendingVdp2Regs->mF0_PRISA = 0x605;
    vdp2Controls.m4_pendingVdp2Regs->mF2_PRISB = 0x607;
    vdp2Controls.m4_pendingVdp2Regs->mF4_PRISC = 0x606;
    vdp2Controls.m4_pendingVdp2Regs->mF6_PRISD = 0x606;
    vdp2Controls.m_isDirty = 1;

    loadFile2("SAVE.SCB", getVdp2Vram(0x20c00), 0);
    loadFile2("SAVE.CGB", getVdp1Pointer(0x25c12000), 0);

    unpackGraphicsToVDP2(COMMON_DAT + 0xf3a2, getVdp2Vram(0x20000));
    unpackGraphicsToVDP2(COMMON_DAT + 0xf068, getVdp2Vram(0x20800));
    asyncDmaCopy(gCommonFile.getSaturnPtr(0x20F474), getVdp2Cram(0xA00), 0x200, 0);

    graphicEngineStatus.m40BC_layersConfig[0].scrollX = 0x50;
    graphicEngineStatus.m40BC_layersConfig[0].scrollY = 0;
    graphicEngineStatus.m40BC_layersConfig[1].scrollX = 0;
    graphicEngineStatus.m40BC_layersConfig[1].scrollY = 0;
    graphicEngineStatus.m40BC_layersConfig[3].scrollX = 0;
    graphicEngineStatus.m40BC_layersConfig[3].scrollY = 0x100;
}

s32 getBupStatus(s32 slotIndex)
{
    return 0;
}

struct sSaveTaskSubStruct0
{
    s8 m0;
    s8 m6_location;
    s8 m5;
    // size is 0x30
};

struct saveMenuSubTask0 : public s_workAreaTemplateWithArg<saveMenuSubTask0, sSaturnPtr>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &saveMenuSubTask0::Init, nullptr, &saveMenuSubTask0::Draw, nullptr };
        return &taskDefinition;
    }

    static void Init(saveMenuSubTask0* pThis, sSaturnPtr r5)
    {
        pThis->mC = r5;
        pThis->m20 = performDivision(8, readSaturnS16(r5));
        pThis->m22 = performDivision(8, readSaturnS16(r5 + 2));
    }

    static void Draw(saveMenuSubTask0* pThis)
    {
        switch (pThis->m10_status)
        {
        case 0:
            pThis->m18 = 0x120;
            if (pThis->m0 < 0)
            {
                return;
            }
            pThis->m14 = 9;
            pThis->m10_status++;
        case 1:
            if (pThis->m4_delay != 0)
            {
                pThis->m4_delay--;
                break;
            }
            if (pThis->m14--)
            {
                pThis->m18 -= 0x20;
                break;
            }
            pThis->m10_status++;
        case 2:
            pThis->m18 = 0;
            if (pThis->m0 > -1)
                break;
            pThis->m14 = 0;
            pThis->m10_status++;
        case 3:
            if (pThis->m4_delay != 0)
            {
                pThis->m4_delay--;
                break;
            }
            if (pThis->m14--)
            {
                pThis->m18 += 0x20;
                break;
            }
            pThis->m10_status = 0;
            break;
        default:
            assert(0);
            break;
        }

        pThis->m1C = (pThis->m18 >> 16) + readSaturnS16(pThis->mC);
        pThis->m1E = readSaturnS16(pThis->mC + 2);

        static const s_menuSprite spriteData = { 0x2400, 0x202, 0, 0 };

        drawMenuSprite2(&spriteData, pThis->m1C, pThis->m1E, 0x2500);
        s32 var1 = performDivision(8, pThis->m1C);
        s32 var2 = performDivision(8, pThis->m1E);
        if ((var1 != pThis->m20) || (var2 + 0x20 != pThis->m22))
        {
            saveScreenClearSlotData(pThis);
            pThis->m20 = var1;
            pThis->m22 = var2 + 0x20;
            saveScreenDisplaySlotData(pThis);
        }
        if (pThis->m8->m0 == 1)
        {
            saveScreenDisplaySlotData2(&pThis->m1C, pThis->m8->m5);
        }
    }

    static void saveScreenClearSlotData(saveMenuSubTask0* pThis)
    {
        FunctionUnimplemented();
    }
    static void saveScreenDisplaySlotData(saveMenuSubTask0* pThis)
    {
        FunctionUnimplemented();
    }
    static void saveScreenDisplaySlotData2(s16*, s32)
    {
        FunctionUnimplemented();
    }

    s32 m0;
    s32 m4_delay;
    sSaveTaskSubStruct0* m8;
    sSaturnPtr mC;
    s32 m10_status;
    s32 m14;
    s32 m18;
    s16 m1C;
    s16 m1E;
    s16 m20;
    s16 m22;
    // 0x24
};

u16 saveMenuData_[]{
    0x19, 0x1C,
    0x19, 0x48,
    0x19, 0x78,
};

sMainMenuTaskInitData2 saveMenuData = {
    &graphicEngineStatus.m40BC_layersConfig[0],
    saveMenuData_
};

std::vector<std::array<s16,2>> saveMenuData2 = {{
    {0x40, 0x20},
    {0x40, 0x58},
    {0x40, 0x90},
} };

const std::array<std::string, 6> saveStrings = {
    {
        "Save Game to Internal RAM",
        "Save Game to Cartridge RAM",
        "Save Game to Expanded Memory",
        "Not enough space to save.",
        "You may save your game.",
        "You may save your game.",
    }
};

const std::array<std::string, 32> locationTable = {
    {
        "Above the Excavation",
        "Excavation Site",
        "Camp",
        "Valley",
        "Village of Cainus",
        "Camp",
        "Garil Desert",
        "Caravan",
        "Camp",
        "Caravan",
        "Camp",
        "Caravan",
        "Camp",
        "Caravan",
        "Caravan",
        "Camp",
        "Caravan",
        "Forbidden Zone",
        "Georgius",
        "Georgius",
        "Georgius",
        "Village of Zoah",
        "Imperial Air Force",
        "Air Force Post",
        "Mel-Kava",
        "The Tower",
        "The Tower",
        "Uru",
        "Uru Underground Ruins",
        "Forest of Zoah",
        "Seekers' Stronghold",
        "Camp",
    }
};



void DisplayMenuMsg(const std::string& r4)
{
    setupVDP2StringRendering(8, 0x36, 0x20, 4);
    clearVdp2TextArea();
    vdp2StringContext.m0 = 0;
    drawObjectName(r4.c_str());
}

struct sSaveTask : public s_workAreaTemplate<sSaveTask>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sSaveTask::Init, nullptr, &sSaveTask::Draw, &sSaveTask::Delete };
        return &taskDefinition;
    }

    static void Init(sSaveTask* pThis)
    {
        pThis->mCC = backupMemoryForSaveScreen();
        loadSaveBackground();
        pThis->mC = 10;
        pThis->m10 = loadFnt("SAVE.FNT");
        startVdp2LayerScroll(0, -8, 0, 10);
        pThis->m14_selectedEntry = -1;
        pThis->m8 = -1;
        pThis->m20 = -1;
        for (int i = 0; i < 3; i++)
        {
            if (getBupStatus(i) == 0)
            {
                pThis->m1C[i] = 1;
                if (pThis->m14_selectedEntry < 0)
                {
                    pThis->m14_selectedEntry = i;
                }
            }
            else
            {
                pThis->m1C[i] = -1;
                assert(0); //060196c4 this would create the icon for the disabled slot
            }
        }

        for (int i = 0; i < 3; i++)
        {
            saveMenuSubTask0* pNewTask = createSubTaskWithArg<saveMenuSubTask0, sSaturnPtr>(pThis, gCommonFile.getSaturnPtr(0x20F778) + i * 4);
            pThis->m28[i] = pNewTask;
            pNewTask->m0 = -1;
            pNewTask->m8 = &pThis->m3C[i];
        }

        pThis->m18 = createMenuCursorTask(pThis, &saveMenuData);
        pThis->m18->m0_selectedEntry = pThis->m14_selectedEntry;
        pThis->m24 = createMenuCursorTask2(pThis, &saveMenuData2);
        pThis->m24->m0 = pThis->m20;
        pThis->m0 = vblankData.m14;
        vblankData.m14 = 1;

        fadePalette(&g_fadeControls.m0_fade0, 0xC210, 0xC210, 1);
        fadePalette(&g_fadeControls.m24_fade1, 0xC210, 0xC210, 1);
    }

    static void saveDrawSub0(sSaveTask* pThis)
    {
        FunctionUnimplemented();
    }

    static void saveDrawSub1(sSaveTask* pThis)
    {
        FunctionUnimplemented();
    }

    static void displaySaveLocation(sSaveTask* pThis, const std::array<std::string, 6>& stringArray)
    {
        sSaveTaskSubStruct0* pSaveSlotInfo = &pThis->m3C[pThis->m20];
        switch (pSaveSlotInfo->m0)
        {
        case 0:
            // not enough space
            DisplayMenuMsg(stringArray[3]);
            break;
        case 1:
            // save already exists
            DisplayMenuMsg("Location:");
            drawObjectName(locationTable[pSaveSlotInfo->m6_location].c_str());
            if (stringArray[4].length())
            {
                vdp2StringContext.m4_cursorX = vdp2StringContext.mC_X;
                vdp2StringContext.m8_cursorY = vdp2StringContext.m10_Y + 2;
                drawObjectName(stringArray[4].c_str()); // overwrite save
            }
            break;
        case 2:
            //make new save
            drawObjectName(stringArray[5].c_str());
            break;
        default:
            assert(0);
        }
    }

    static void Draw(sSaveTask* pThis)
    {
        switch (pThis->m4_status)
        {
        case 0:
            pThis->mC--;
            if (pThis->mC != 0)
                return;
            DisplayMenuMsg(saveStrings[pThis->m14_selectedEntry]);
            pThis->m4_status++;
            break;
        case 1:
            {
                if (pThis->m8 > -1)
                {
                    if (pThis->m20 < 0)
                    {
                        pThis->m20 = 0;
                        saveDrawSub0(pThis);
                        displaySaveLocation(pThis, saveStrings);
                    }
                    saveDrawSub1(pThis);
                    return;
                }
                int entry = pThis->m14_selectedEntry;
                if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 1) == 0)
                {
                    if (entry < 0)
                        return;
                    if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 0x86) == 0)
                    {
                        if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 0x30) == 0)
                            return;
                        playSoundEffect(10);
                        if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 0x10) == 0)
                        {
                            do
                            {
                                entry++;
                                if (entry > 2)
                                {
                                    entry = 0;
                                }
                            } while (pThis->m1C[entry] == -1);
                        }
                        else
                        {
                            do
                            {
                                entry--;
                                if (entry < 0)
                                {
                                    entry = 2;
                                }
                            } while (pThis->m1C[entry] == -1);
                        }

                        pThis->m14_selectedEntry = entry;
                        pThis->m18->m0_selectedEntry = entry;
                        DisplayMenuMsg(saveStrings[pThis->m14_selectedEntry]);
                        return;
                    }
                    pThis->m8 = 0;
                    pThis->m20 = -1;
                    pThis->m28[0]->m0 = 0;
                    pThis->m28[0]->m4_delay = 0;
                    pThis->m28[1]->m0 = 1;
                    pThis->m28[1]->m4_delay = 3;
                    pThis->m28[2]->m0 = 2;
                    pThis->m28[2]->m4_delay = 6;
                    graphicEngineStatus.m4 = 1;
                    playSoundEffect(0);
                }
                else
                {
                    if (graphicEngineStatus.m40AC.m3 == 0)
                    {
                        pThis->m4_status++;
                        return;
                    }
                    playSoundEffect(5);
                }
                return;
            }
        case 2:
            playSoundEffect(1);
            setupVDP2StringRendering(0, 0x22, 0x2C, 0x1C);
            clearVdp2TextArea();
            clearVdp2Menu();
            pThis->mC = 10;
            startVdp2LayerScroll(0, 8, 0, 10);
            pThis->m4_status++;
        case 3:
            pThis->mC--;
            if (pThis->mC != 0)
            {
                return;
            }
            fadePalette(&g_fadeControls.m0_fade0, 0, 0, 1);
            fadePalette(&g_fadeControls.m24_fade1, 0, 0, 1);
            pThis->m4_status++;
        case 4:
            pThis->getTask()->markFinished();
            break;
        default:
            assert(0);
            break;
        }
        FunctionUnimplemented();
    }

    static void Delete(sSaveTask* pThis)
    {
        FunctionUnimplemented();
    }

    s32 m0;
    s32 m4_status;
    s32 m8;
    s32 mC;
    u32 m10;
    s32 m14_selectedEntry;
    s_MenuCursorWorkArea* m18;
    u8 m1C[3];
    s32 m20;
    s_MenuCursor2* m24;
    saveMenuSubTask0* m28[3];
    sSaveTaskSubStruct0 m3C[3];
    u8* mCC;
    //size: 0xd0
};

p_workArea createLoadTask(p_workArea parent)
{
    return createSubTask<sSaveTask>(parent);
}
