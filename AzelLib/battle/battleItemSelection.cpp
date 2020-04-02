#include "PDS.h"
#include "battleCommandMenu.h"
#include "battleItemSelection.h"

struct sBattleItemSelectionTask : public s_workAreaTemplate<sBattleItemSelectionTask>
{
    s8 m0_state;
    s8 m1;
    s8 m2;
    s8 m3_currentPageIndex;
    s8 m4;
    s8 m5_currentSelectionInPage;
    s8 m6_numEntriesInPage;
    s8 m7;
    s8 m8;
    s32* mC;
    sBattleItemSelectionTask** m10_previousMenu;
    s16 m14_listX;
    s16 m16_listY;
    s16 m1A_listWidth;
    s16 m1C_listHeight;
    std::array<sBattleCommandMenu::sSubMenuEntry, 0x38>* m24;
    // size 0x2C
};

void updateBattleItemMenuList(sBattleItemSelectionTask* pThis)
{
    setupVDP2StringRendering(pThis->m14_listX + 6, pThis->m16_listY + 1, pThis->m1A_listWidth - 8, pThis->m1C_listHeight - 2);
    clearVdp2TextArea();

    for (int i = 0; i < pThis->m6_numEntriesInPage; i++)
    {
        vdp2DebugPrintSetPosition(pThis->m14_listX + 4, pThis->m16_listY + 1 + i * 2);
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
            FunctionUnimplemented();
        }

        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 0x1)
        {
            FunctionUnimplemented();
        }
        break;
    default:
        assert(0);
    }
    
}

void sBattleItemSelectionTask_Draw(sBattleItemSelectionTask* pThis)
{
    FunctionUnimplemented();
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

sBattleItemSelectionTask* createBattleItemSelectionTask(p_workArea parent, sBattleItemSelectionTask** previousMenu, s32* p3, s32 p4, std::array<sBattleCommandMenu::sSubMenuEntry, 0x38>& p5)
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
    pNewTask->mC = p3;
    pNewTask->m10_previousMenu = previousMenu;
    if (previousMenu)
    {
        *previousMenu = pNewTask;
    }
    return pNewTask;
}


