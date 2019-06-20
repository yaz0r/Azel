#include "PDS.h"

static void s_receivedItemTask_DisplayReceivedObject(s_receivedItemTask* pThis)
{
    drawBlueBox(pThis->m14_x, pThis->m16_y, pThis->m1A_width, pThis->m1C_height);
    displayObjectIcon(0, pThis->m14_x + 4, pThis->m16_y + 1,getObjectIcon(pThis->m24_receivedItemId));
    setupVDP2StringRendering(pThis->m14_x + 6, pThis->m16_y + 1, pThis->m1A_width - 0xA, pThis->m1C_height - 2);

    vdp2StringContext.m0 = 0;
    vdp2PrintStatus.m10_palette = 0xC000;
    vdp2StringContext.m4_cursorX = vdp2StringContext.mC_X;
    vdp2StringContext.m8_cursorY = vdp2StringContext.m10_Y;
    drawObjectName(getObjectListEntry(pThis->m24_receivedItemId)->m4_name.c_str());

    if (pThis->m26_receivedItemQuanity > 0)
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

        printVdp2Number(pThis->m26_receivedItemQuanity);
    }
}

static void s_receivedItemTask_DeleteSub0(s_receivedItemTask* pThis)
{
    TaskUnimplemented();
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
        if (pThis->m2 == 0)
        {
            if (--pThis->mA <= 0)
            {
                pThis->m0++;
                break;
            }
            if (graphicEngineStatus.m4514.m0[0].m0_current.m8_newButtonDown & 0xF)
            {
                if (readKeyboardToggle(0x87))
                {
                    return;
                }
            }
            pThis->m0++;
            break;
        }
        else
        {
            assert(0);
        }
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

s_receivedItemTask* createReceiveItemTask(p_workArea r4_parentTask, s_receivedItemTask** r5, s32 r6, s32 r7_receivedItemId, s32 arg0_receivedItemQuanity)
{
    s_receivedItemTask* pNewTask = createSubTask<s_receivedItemTask>(r4_parentTask);

    if (r6 > 0)
    {
        pNewTask->m0 = 0;
        pNewTask->mA = r6;
        pNewTask->m2 = 0;
    }
    else
    {
        //601C38A
        assert(0);
    }

    //601C39E
    pNewTask->m24_receivedItemId = r7_receivedItemId;
    pNewTask->m26_receivedItemQuanity = arg0_receivedItemQuanity;

    pNewTask->m28_itemNameLength = createReceiveItemTaskSub0(getObjectListEntry(r7_receivedItemId)->m4_name.c_str(), 34);
    s32 r4 = pNewTask->m28_itemNameLength + 2;
    if (arg0_receivedItemQuanity <= 0)
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

