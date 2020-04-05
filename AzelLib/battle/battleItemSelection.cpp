#include "PDS.h"
#include "battleCommandMenu.h"
#include "battleItemSelection.h"
#include "items.h"
#include "kernel/dialogTask.h"

void updateBattleItemMenuList(sBattleItemSelectionTask* pThis)
{
    setupVDP2StringRendering(pThis->m14_listX + 6, pThis->m16_listY + 1, pThis->m1A_listWidth - 8, pThis->m1C_listHeight - 2);
    clearVdp2TextArea();

    for (int i = 0; i < pThis->m6_numEntriesInPage; i++)
    {
        vdp2DebugPrintSetPosition(pThis->m14_listX + 4, pThis->m16_listY + 1 + i * 2);
        clearVdp2TextLargeFont();
    }

    int numEntriesInPage = pThis->m1 - (pThis->m3_currentPageIndex + 1) * 5;

    if (numEntriesInPage < 0)
    {
        pThis->m6_numEntriesInPage = numEntriesInPage + 5;
    }
    else
    {
        pThis->m6_numEntriesInPage = 5;
    }

    pThis->m20_previousListHeight = pThis->m1C_listHeight;
    pThis->m1C_listHeight = 0xC;
    pThis->m5_currentSelectionInPage = 0;

    // if the height has changed, need to refresh
    if (pThis->m1C_listHeight < pThis->m20_previousListHeight)
    {
        clearBlueBox(pThis->m14_listX, pThis->m16_listY + pThis->m1C_listHeight, pThis->m1A_listWidth, pThis->m1C_listHeight + pThis->m20_previousListHeight);
    }

    drawBlueBox(pThis->m14_listX, pThis->m16_listY, pThis->m1A_listWidth, pThis->m1C_listHeight, 0x1000);
    setupVDP2StringRendering(pThis->m14_listX + 6, pThis->m16_listY + 1, pThis->m1A_listWidth - 8, pThis->m1C_listHeight - 2);

    for (int i=0; i<pThis->m6_numEntriesInPage; i++)
    {
        sBattleCommandMenu::sSubMenuEntry& entry = (*pThis->m24)[5 * pThis->m3_currentPageIndex + i];
        if (entry.m0_itemIndex != eItems::mA6_unlearned)
        {
            displayObjectIcon(0, pThis->m14_listX + 4, i * 2 + pThis->m16_listY + 1, getObjectIcon(entry.m0_itemIndex));
        }

        if (entry.m2 & 1)
        {
            vdp2StringContext.m0 = 9;
        }
        else if (entry.m2 & 2)
        {
            vdp2StringContext.m0 = 1;
        }
        else
        {
            vdp2StringContext.m0 = 0;
        }

        vdp2StringContext.m4_cursorX = vdp2StringContext.mC_X;
        vdp2StringContext.m8_cursorY = vdp2StringContext.m18_Height;

        if (i*2 > -1)
        {
            vdp2StringContext.m8_cursorY = 0;
        }
        vdp2StringContext.m8_cursorY += vdp2StringContext.m10_Y + i * 2;

        drawObjectName(getObjectListEntry(entry.m0_itemIndex)->m4_name.c_str());

        // display count if necessary
        if (entry.m0_itemIndex < 0x91)
        {
            int yPosition = i * 2;
            vdp2StringContext.m4_cursorX = vdp2StringContext.mC_X + 0x11;
            vdp2StringContext.m8_cursorY = vdp2StringContext.m18_Height;
            if (yPosition >= 0)
            {
                vdp2StringContext.m8_cursorY = 0;
            }

            vdp2StringContext.m8_cursorY += vdp2StringContext.m10_Y + yPosition;
            drawObjectName("*");

            vdp2StringContext.m4_cursorX = vdp2StringContext.mC_X + 0x13;
            vdp2StringContext.m8_cursorY = vdp2StringContext.m18_Height;
            if (yPosition >= 0)
            {
                vdp2StringContext.m8_cursorY = 0;
            }
            vdp2StringContext.m8_cursorY += vdp2StringContext.m10_Y + yPosition;

            printVdp2Number(mainGameState.getItemCount(entry.m0_itemIndex));
        }
        else
        {
            //assert(0);
            FunctionUnimplemented();
        }
    }
}

void sBattleItemSelectionTask_Update(sBattleItemSelectionTask* pThis)
{
    switch (pThis->m0_state)
    {
    case 0:
        pThis->m0_state++;
    case 1:
        updateBattleItemMenuList(pThis);
        playSoundEffect(3);
        pThis->m0_state++;
        break;
    case 2:
        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 0x10)
        {
            pThis->m5_currentSelectionInPage--;
            if (pThis->m5_currentSelectionInPage < 0)
            {
                pThis->m5_currentSelectionInPage += pThis->m6_numEntriesInPage;
            }
            playSoundEffect(2);
        }
        else if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 0x20)
        {
            pThis->m5_currentSelectionInPage++;
            if (pThis->m5_currentSelectionInPage >= pThis->m6_numEntriesInPage)
            {
                pThis->m5_currentSelectionInPage -= pThis->m6_numEntriesInPage;
            }
            playSoundEffect(2);
        }
        else if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 0x40)
        {
            if (pThis->m3_currentPageIndex > 0)
            {
                pThis->m3_currentPageIndex--;
                updateBattleItemMenuList(pThis);
                playSoundEffect(6);
            }
        }
        else if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 0x80)
        {
            if (pThis->m3_currentPageIndex < pThis->m4)
            {
                pThis->m3_currentPageIndex++;
                updateBattleItemMenuList(pThis);
                playSoundEffect(6);
            }
        }

        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 0x6)
        {
            if ((*pThis->m24)[pThis->m3_currentPageIndex * 5 + pThis->m4].m2 &1)
            {
                playSoundEffect(5);
            }
            else
            {
                *pThis->mC_selectedEntry = pThis->m3_currentPageIndex * 5 + pThis->m4;
                playSoundEffect(0);
                pThis->m0_state++;
            }
        }

        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 0x1)
        {
            if (pThis->m2)
            {
                *pThis->mC_selectedEntry = -1;
                playSoundEffect(1);
                pThis->m0_state++;
            }
        }
        break;
    case 3:
        pThis->m_DrawMethod = nullptr;
        pThis->m0_state++;
        break;
    case 4:
        pThis->getTask()->markFinished();
        break;
    default:
        assert(0);
    }
    
}

void sBattleItemSelectionTask_Draw(sBattleItemSelectionTask* pThis)
{
    drawMultiChoiceVdp1Cursor((pThis->m14_listX + 2) * 8, (pThis->m16_listY + 1 + pThis->m5_currentSelectionInPage * 2) * 8, gCommonFile.getSaturnPtr(0x20FFE0) + pThis->m7 * 0xA, 0x7F0);

    if (pThis->m3_currentPageIndex > 0)
    {
        drawMultiChoiceVdp1Cursor((pThis->m14_listX + 2) * 8, (pThis->m16_listY * 8 + (pThis->m1C_listHeight * 8)/2) - 8, gCommonFile.getSaturnPtr(0x21003A), 0x760);
    }

    if (pThis->m3_currentPageIndex < pThis->m4)
    {
        drawMultiChoiceVdp1Cursor((pThis->m14_listX + pThis->m1A_listWidth - 2) * 8, (pThis->m16_listY * 8 + (pThis->m1C_listHeight * 8) / 2) - 8, gCommonFile.getSaturnPtr(0x210030), 0x760);
    }

    pThis->m8 += vblankData.m14;
    if (pThis->m8 > 1)
    {
        pThis->m7++;
        if(pThis->m7 > 7)
        {
            pThis->m7 = 0;
        }
        pThis->m8 = 0;
    }
}

// TODO: kernel
void clearItemSelectionVdp2()
{
    FunctionUnimplemented();
}

void sBattleItemSelectionTask_Delete(sBattleItemSelectionTask* pThis)
{
    clearItemSelectionVdp2();
    if (pThis->m10_previousMenu)
    {
        *pThis->m10_previousMenu = nullptr;
    }
}

static const sBattleItemSelectionTask::TypedTaskDefinition definition =
{
    nullptr,
    &sBattleItemSelectionTask_Update,
    &sBattleItemSelectionTask_Draw,
    &sBattleItemSelectionTask_Delete,
};

sBattleItemSelectionTask* createBattleItemSelectionTask(p_workArea parent, sBattleItemSelectionTask** previousMenu, s32* p3, s32 p4, std::vector<sBattleCommandMenu::sSubMenuEntry>& p5)
{
    sBattleItemSelectionTask* pNewTask = createSubTask<sBattleItemSelectionTask>(parent, &definition);

    pNewTask->m0_state = 0;
    pNewTask->m5_currentSelectionInPage = 0;
    
    if (p4 < 0)
    {
        pNewTask->m1 = -p4;
    }
    else
    {
        pNewTask->m1 = p4;
    }

    pNewTask->m2 = p4 >= 0;

    pNewTask->m24 = &p5;
    pNewTask->m3_currentPageIndex = 0;
    pNewTask->m4 = performDivision(5, pNewTask->m1 - 1);
    pNewTask->m7 = 0;
    pNewTask->m8 = 0;
    pNewTask->m14_listX = 2;
    pNewTask->m16_listY = 4;
    pNewTask->m1A_listWidth = 0x28;
    pNewTask->m1C_listHeight = 0xC;
    pNewTask->mC_selectedEntry = p3;
    pNewTask->m10_previousMenu = previousMenu;
    if (previousMenu)
    {
        *previousMenu = pNewTask;
    }
    return pNewTask;
}


