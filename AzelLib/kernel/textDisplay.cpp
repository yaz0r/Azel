#include "PDS.h"
#include "textDisplay.h"

/*
struct sTextDisplay : public s_workAreaTemplate<sTextDisplay>
{
    s8 m0;
    s8 m2;
    s16 mA;
    p_workArea* m10;
    s16 m14_X;
    s16 m16_Y;
    s16 m1A;
    s16 m1C;
    sSaturnPtr m24_stringEA;
    s16 m28;
    s16 m2A;
    // size 0x2C
};
*/

void textDisplay_udpate_drawTextSub(s32 x, s32 y, s32 width, s32 height)
{
    int iVar1 = (y * 0x40 + x) * 2;
    int sVar2 = height * 0x1000 + width * 6;

    setVdp2VramU16(vdp2TextMemoryOffset + 0x00 + iVar1, sVar2 + 0x248);
    setVdp2VramU16(vdp2TextMemoryOffset + 0x02 + iVar1, sVar2 + 0x249);
    setVdp2VramU16(vdp2TextMemoryOffset + 0x04 + iVar1, sVar2 + 0x24A);

    setVdp2VramU16(vdp2TextMemoryOffset + 0x80 + iVar1, sVar2 + 0x24B);
    setVdp2VramU16(vdp2TextMemoryOffset + 0x82 + iVar1, sVar2 + 0x24C);
    setVdp2VramU16(vdp2TextMemoryOffset + 0x84 + iVar1, sVar2 + 0x24D);
}

void textDisplay_udpate_drawText(s_vdp2StringTask* pThis)
{
    drawBlueBox(pThis->m14_x, pThis->m16_y, pThis->m1A_width, pThis->m1C_height, 0x1000);
    textDisplay_udpate_drawTextSub(pThis->m14_x + 4, pThis->m16_y + 1, pThis->m28, pThis->m2A);
    setupVDP2StringRendering(pThis->m14_x + 8, pThis->m16_y + 1, pThis->m1A_width - 0xC, pThis->m1C_height - 2);
    vdp2StringContext.m4_cursorX = vdp2StringContext.mC_X;
    vdp2StringContext.m8_cursorY = vdp2StringContext.m10_Y;
    vdp2StringContext.m0 = 0;
    VDP2DrawString(readSaturnString(pThis->m24_string).c_str());
}

void textDisplay_update(s_vdp2StringTask* pThis)
{
    switch (pThis->m0_status)
    {
    case 0:
        pThis->m0_status++;
        break;
    case 1:
        textDisplay_udpate_drawText(pThis);
        pThis->m0_status++;
        break;
    case 2:
        switch (pThis->m2_durationMode)
        {
        case 1:
                assert(0);
            //pThis->m10_autoCloseDuration--;
            //if (pThis->m10_autoCloseDuration <= 0)
            {
                pThis->m0_status++;
            }
            break;
        default:
            assert(0);
        }
        break;
    case 3:
        pThis->m0_status++;
        break;
    case 4:
        pThis->getTask()->markFinished();
        break;
    default:
        assert(0);
    }
}

void clearTextDisplay(s_vdp2StringTask* pThis)
{
    setupVDP2StringRendering(pThis->m14_x + 8, pThis->m16_y + 1, pThis->m1A_width - 0xC, pThis->m1C_height - 2);
    clearVdp2TextArea();
    vdp2DebugPrintSetPosition(pThis->m14_x + 4, pThis->m16_y + 1);
    clearVdp2TextLargeFont();
    clearBlueBox(pThis->m14_x, pThis->m16_y, pThis->m1A_width, pThis->m1C_height);
}

void textDisplay_delete(s_vdp2StringTask* pThis)
{
    clearTextDisplay(pThis);
    if (pThis->m10_autoCloseDuration)
    {
        *pThis->m10_autoCloseDuration = nullptr;
    }
}

void createDisplayFormationNameText(p_workArea parentTask, s_vdp2StringTask** outputTask, s16 param3, sSaturnPtr stringEA, s16 param5, s16 param6)
{
    static const s_vdp2StringTask::TypedTaskDefinition definition = {
        nullptr,
        &textDisplay_update,
        nullptr,
        &textDisplay_delete,
    };

    s_vdp2StringTask* pNewTask = createSubTask<s_vdp2StringTask>(parentTask, &definition);

    pNewTask->m0_status = 0;
    if (param3 < 1)
    {
        if (param3 < 0)
        {
            pNewTask->mA_duration = -param3;
            pNewTask->m2_durationMode = 1;
        }
        else
        {
            pNewTask->mA_duration = 0;
            pNewTask->m2_durationMode = 2;
        }
    }
    else
    {
        pNewTask->mA_duration = param3;
        pNewTask->m2_durationMode = 0;
    }

    pNewTask->m24_string = stringEA;
    pNewTask->m28 = param5;
    pNewTask->m2A = param6;
    int stringLength = computeStringLength(stringEA, 0x20);
    pNewTask->m14_x = (((0x28 - stringLength) / 2) - 4) & ~1;
    pNewTask->m16_y = 4;
    pNewTask->m1A_width = stringLength + 0xC;
    pNewTask->m1C_height = 4;

    pNewTask->m10_autoCloseDuration = outputTask;
    if (outputTask)
    {
        *outputTask = pNewTask;
    }
}

void s_vdp2StringTask::UpdateSub1()
{
    drawBlueBox(m14_x, m16_y, m1A_width, m1C_height, 0x1000);

    setupVDP2StringRendering(m14_x + 4, m16_y + 1, m1A_width - 8, m1C_height - 2);

    vdp2StringContext.m0 = 0;
    VDP2DrawString((char*)getSaturnPtr(m24_string));
}

void s_vdp2StringTask::Update(s_vdp2StringTask* pThis)
{
    switch (pThis->m0_status)
    {
    case 0:
        pThis->m0_status++;
    case 1:
        pThis->UpdateSub1();
        pThis->m0_status++;
        return;
    case 2:
        switch (pThis->m2_durationMode)
        {
        case 0:
            if (--pThis->mA_duration <= 0)
            {
                pThis->m0_status++;
                return;
            }
            else
            {
                if ((graphicEngineStatus.m4514.m0_inputDevices->m0_current.m8_newButtonDown & 0xF) || readKeyboardToggle(0x87))
                {
                    pThis->m0_status++;
                }
                return;
            }
        case 1:
            if (--pThis->mA_duration > 0)
            {
                return;
            }
            pThis->m0_status++;
            break;
        default:
            assert(0);
            break;
        }
    case 3:
        pThis->m0_status++;
        return;
    case 4:
        pThis->getTask()->markFinished();
        return;
    default:
        assert(0);
        break;
    }
}

void vdp2StringTaskDeleteSub0(s_vdp2StringTask* pThis)
{
    setupVDP2StringRendering(pThis->m14_x + 4, pThis->m16_y + 1, pThis->m1A_width - 8, pThis->m1C_height - 2);
    clearVdp2TextArea();
    clearBlueBox(pThis->m14_x, pThis->m16_y, pThis->m1A_width, pThis->m1C_height);
}

void s_vdp2StringTask::Delete(s_vdp2StringTask* pThis)
{
    vdp2StringTaskDeleteSub0(pThis);

    if (pThis->m10_autoCloseDuration)
    {
        *pThis->m10_autoCloseDuration = NULL;
    }
}

s_vdp2StringTask* createDisplayStringBorromScreenTask(p_workArea pTask, s_vdp2StringTask** r5, s16 duration, sSaturnPtr pString)
{
    s_vdp2StringTask* r14 = createSubTask<s_vdp2StringTask>(pTask);

    r14->m0_status = 0;
    if (duration > 0)
    {
        r14->mA_duration = duration;
        r14->m2_durationMode = 0;
    }
    else if (duration < 0)
    {
        r14->mA_duration = -duration;
        r14->m2_durationMode = 1;
    }
    else
    {
        r14->mA_duration = 0;
        r14->m2_durationMode = 2;
    }

    r14->m24_string = pString;

    s32 stringLength = computeStringLength(pString, 36);

    r14->m14_x = (((44 - stringLength) / 2) - 4) & ~1;
    r14->m16_y = 4;
    r14->m1A_width = stringLength + 8;
    r14->m1C_height = 4;
    r14->m10_autoCloseDuration = r5;
    if (r5)
    {
        *r5 = r14;
    }

    return r14;
}
