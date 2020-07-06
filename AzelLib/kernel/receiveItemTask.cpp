#include "PDS.h"
#include "items.h"

static void s_receivedItemTask_DisplayReceivedObject(s_receivedItemTask* pThis)
{
    drawBlueBox(pThis->m14_x, pThis->m16_y, pThis->m1A_width, pThis->m1C_height, 0x1000);
    displayObjectIcon(0, pThis->m14_x + 4, pThis->m16_y + 1,getObjectIcon(pThis->m24_receivedItemId));
    setupVDP2StringRendering(pThis->m14_x + 6, pThis->m16_y + 1, pThis->m1A_width - 0xA, pThis->m1C_height - 2);

    vdp2StringContext.m0 = 0;
    vdp2PrintStatus.m10_palette = 0xC000;
    vdp2StringContext.m4_cursorX = vdp2StringContext.mC_X;
    vdp2StringContext.m8_cursorY = vdp2StringContext.m10_Y;
    drawObjectName(getObjectListEntry(pThis->m24_receivedItemId)->m4_name.c_str());

    if (pThis->m26_receivedItemQuantity > 0)
    {
        s32 r2;
        if (pThis->m28_itemNameLength + 1 >= 0)
        {
            r2 = 0;
        }
        else
        {
            r2 = pThis->m14_x;
        }

        vdp2StringContext.m4_cursorX = vdp2StringContext.mC_X + pThis->m28_itemNameLength +r2 + 1;
        vdp2StringContext.m8_cursorY = vdp2StringContext.m10_Y;

        drawObjectName("*");

        if (pThis->m28_itemNameLength + 2 >= 0)
        {
            r2 = 0;
        }
        else
        {
            r2 = pThis->m14_x;
        }
        vdp2StringContext.m4_cursorX = vdp2StringContext.mC_X + pThis->m28_itemNameLength +r2 + 2;
        vdp2StringContext.m8_cursorY = vdp2StringContext.m10_Y;

        printVdp2Number(pThis->m26_receivedItemQuantity);
    }
}

static void s_receivedItemTask_DeleteSub0(s_receivedItemTask* pThis)
{
    setupVDP2StringRendering(pThis->m14_x + 6, pThis->m16_y + 1, pThis->m1A_width - 10, pThis->m1C_height - 2);
    clearVdp2TextArea();
    vdp2DebugPrintSetPosition(pThis->m14_x + 4, pThis->m16_y + 1);
    clearVdp2TextLargeFont();
    clearBlueBox(pThis->m14_x, pThis->m16_y, pThis->m1A_width, pThis->m1C_height);
}

void s_receivedItemTask::Update(s_receivedItemTask* pThis)
{
    switch (pThis->m0)
    {
    case 0:
        pThis->m0++;
        // fall through
    case 1:
        s_receivedItemTask_DisplayReceivedObject(pThis);
        pThis->m0++;
        break;
    case 2:
        switch(pThis->m2_subMode)
        {
        case 0:
            if (--pThis->mA < 1)
            {
                pThis->m0++;
                return;
            }
            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 0xF)
            {
                if (!readKeyboardToggle(0x87))
                {
                    return;
                }
                pThis->m0++;
            }
            break;
        default:
            assert(0);
        }
        break;
    case 3:
        pThis->m0++;
        break;
    case 4:
        pThis->getTask()->markFinished();
        break;
    }
}

void s_receivedItemTask::Delete(s_receivedItemTask* pThis)
{
    s_receivedItemTask_DeleteSub0(pThis);

    if (pThis->m10)
    {
        *pThis->m10 = nullptr;
    }
}

s32 createReceiveItemTaskSub0(const char* inputString, s32 maxLength)
{
    s32 r6 = 0;
    while (*inputString)
    {
        if ((*inputString != '\xDE') && (*inputString != '\xDF'))
        {
            if (r6 >= maxLength)
                return r6;
            r6++;
        }
        inputString++;
    }
    return r6;
}

s_receivedItemTask* createReceiveItemTask(p_workArea r4_parentTask, s_receivedItemTask** r5, s32 r6, eItems r7_receivedItemId, s32 arg0_receivedItemQuantity)
{
    s_receivedItemTask* pNewTask = createSubTask<s_receivedItemTask>(r4_parentTask);

    if (r6 > 0)
    {
        pNewTask->m0 = 0;
        pNewTask->mA = r6;
        pNewTask->m2_subMode = 0;
    }
    else
    {
        //601C38A
        assert(0);
    }

    //601C39E
    pNewTask->m24_receivedItemId = r7_receivedItemId;
    pNewTask->m26_receivedItemQuantity = arg0_receivedItemQuantity;

    pNewTask->m28_itemNameLength = createReceiveItemTaskSub0(getObjectListEntry(r7_receivedItemId)->m4_name.c_str(), 34);
    s32 r4 = pNewTask->m28_itemNameLength + 2;
    if (arg0_receivedItemQuantity <= 0)
    {
        //0601C3CA
        assert(0);
    }

    pNewTask->m14_x = (((44 - r4) / 2) - 5) & ~1;
    pNewTask->m16_y = 4;
    pNewTask->m1A_width = r4 + 0xA;
    pNewTask->m1C_height = 4;

    if (r5)
    {
        pNewTask->m10 = r5;
        *r5 = pNewTask;
    }
    return pNewTask;
}

