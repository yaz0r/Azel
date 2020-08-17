#include "PDS.h"
#include "dialogTask.h"
#include "audio/systemSounds.h"
#include "commonOverlay.h"

void s_multiChoiceTask::drawMultiChoice()
{
    setupVDP2StringRendering(m14_x + 2, m16_y + 1, m1A_width - 4, m1C_height - 2);
    clearVdp2TextArea();

    s32 r4 = m1 - 4 * (m3_quantity + 1);
    if (r4 >= 0)
    {
        m6_numEntries = 4;
    }
    else
    {
        m6_numEntries = r4 + 4;
    }

    m20 = m1C_height;
    m1C_height = (m6_numEntries * 2) + 2;
    m5_selectedEntry = 0;
    if (m1C_height < m20)
    {
        clearBlueBox(m14_x, m16_y + m1C_height, m1A_width, m20 - m1C_height);
    }

    drawBlueBox(m14_x, m16_y, m1A_width, m1C_height, 0x1000);

    setupVDP2StringRendering(m14_x + 2, m16_y + 1, m1A_width - 4, m1C_height - 2);
    vdp2StringContext.m0 = 0;

    for (int i = 0; i < m6_numEntries; i++)
    {
        vdp2StringContext.m4_cursorX = vdp2StringContext.mC_X + 2;
        vdp2StringContext.m8_cursorY = vdp2StringContext.m10_Y + i * 2;

        drawObjectName(readSaturnString(readSaturnEA(m24_strings + (m3_quantity * 4 + i) * 4)).c_str());
    }
}

void s_multiChoiceTask::Update(s_multiChoiceTask* pThis)
{
    switch (pThis->m0_Status)
    {
    case 0:
        pThis->m0_Status++;
    case 1:
        pThis->drawMultiChoice();
        playSystemSoundEffect(3);
        pThis->m0_Status++;
        return;
    case 2:
        if (graphicEngineStatus.m4514.m0_inputDevices->m0_current.m8_newButtonDown & 0x10) // up
        {
            pThis->m5_selectedEntry--;
            if (pThis->m5_selectedEntry < 0)
            {
                pThis->m5_selectedEntry += pThis->m6_numEntries;
            }
            playSystemSoundEffect(2);
        }
        else if (graphicEngineStatus.m4514.m0_inputDevices->m0_current.m8_newButtonDown & 0x20) // down
        {
            pThis->m5_selectedEntry++;
            if (pThis->m5_selectedEntry >= pThis->m6_numEntries)
            {
                pThis->m5_selectedEntry -= pThis->m6_numEntries;
            }
            playSystemSoundEffect(2);
        }
        else if (graphicEngineStatus.m4514.m0_inputDevices->m0_current.m8_newButtonDown & 0x40)
        {
            if (pThis->m3_quantity > 0)
            {
                pThis->m3_quantity--;
                pThis->drawMultiChoice();
                playSystemSoundEffect(6);
            }
        }
        else if (graphicEngineStatus.m4514.m0_inputDevices->m0_current.m8_newButtonDown & 0x80)
        {
            if (pThis->m3_quantity < pThis->m4_quantityMax)
            {
                pThis->m3_quantity++;
                pThis->drawMultiChoice();
                playSystemSoundEffect(6);
            }
        }

        if (graphicEngineStatus.m4514.m0_inputDevices->m0_current.m8_newButtonDown & 6) // select
        {
            *pThis->mC_result = pThis->m5_selectedEntry + pThis->m3_quantity * 4;
            playSystemSoundEffect(0);
            pThis->m0_Status++;
        }
        else if (graphicEngineStatus.m4514.m0_inputDevices->m0_current.m8_newButtonDown & 1) // cancel
        {
            if (pThis->m2_defaultResult)
            {
                *pThis->mC_result = -1;
                playSystemSoundEffect(1);
                pThis->m0_Status++;
            }
        }
        return;
    case 3:
        pThis->mC_result = 0;
        pThis->m0_Status++;
        return;
    case 4:
        pThis->getTask()->markFinished();
        return;
    default:
        assert(0);
        break;
    }
}

void drawMultiChoiceVdp1Cursor(s32 r4_x, s32 r5_y, sSaturnPtr r6_spritePtr, s32 r7_color)
{
    s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;

    vdp1WriteEA.m0_CMDCTRL = 0x1000 | readSaturnU16(r6_spritePtr + 4); // command
    vdp1WriteEA.m4_CMDPMOD = 0x80; // CMDPMOD
    vdp1WriteEA.m6_CMDCOLR = 0x4000 + r7_color; // CMDCOLR
    vdp1WriteEA.m8_CMDSRCA = readSaturnU16(r6_spritePtr); // CMDSRCA
    vdp1WriteEA.mA_CMDSIZE = readSaturnU16(r6_spritePtr + 2); // CMDSIZE
    vdp1WriteEA.mC_CMDXA = readSaturnU16(r6_spritePtr + 6) + r4_x - 176; // CMDXA
    vdp1WriteEA.mE_CMDYA = readSaturnU16(r6_spritePtr + 8) + r5_y - 112; // CMDYA

    s_vd1ExtendedCommand* pExtendedCommand = createVdp1ExtendedCommand(vdp1WriteEA);
    pExtendedCommand->depth = 0;

    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

    graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
    graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
    graphicEngineStatus.m14_vdp1Context[0].mC += 1;

}

void s_multiChoiceTask::Draw(s_multiChoiceTask* pThis)
{
    drawMultiChoiceVdp1Cursor((pThis->m14_x + 2) * 8, (pThis->m16_y + pThis->m5_selectedEntry * 2 + 1) * 8, gCommonFile->getSaturnPtr(0x20FFE0 + pThis->m7 * 10), 0x7F0);

    if (pThis->m3_quantity > 0)
    {
        assert(0);
    }

    if (pThis->m4_quantityMax > pThis->m3_quantity)
    {
        assert(0);
    }

    pThis->m8 += vblankData.m14_numVsyncPerFrame;

    if (pThis->m8 > 1)
    {
        pThis->m7++;
        if (pThis->m7 > 7)
        {
            pThis->m7 = 0;
        }
        pThis->m8 = 0;
    }
}

void startDialogTask(p_workArea r4_parent, s_multiChoiceTask** r5_outputTask, s32* r6_outputResult, s32 r7_index, sSaturnPtr arg0)
{
    s_multiChoiceTask* pMultiChoiceTask = createSubTask<s_multiChoiceTask>(r4_parent);

    pMultiChoiceTask->m0_Status = 0;
    pMultiChoiceTask->m5_selectedEntry = 0;

    if (r7_index >= 0)
    {
        pMultiChoiceTask->m1 = r7_index;
        pMultiChoiceTask->m2_defaultResult = 1;
    }
    else
    {
        pMultiChoiceTask->m1 = -r7_index;
        pMultiChoiceTask->m2_defaultResult = 0;
    }

    pMultiChoiceTask->m24_strings = arg0;
    pMultiChoiceTask->m3_quantity = 0;
    pMultiChoiceTask->m4_quantityMax = performDivision(4, pMultiChoiceTask->m1 - 1);
    pMultiChoiceTask->m7 = 0;
    pMultiChoiceTask->m8 = 0;
    pMultiChoiceTask->m14_x = 2;
    pMultiChoiceTask->m16_y = 4;
    pMultiChoiceTask->m1A_width = 0x22;
    pMultiChoiceTask->m1C_height = 0xA;
    pMultiChoiceTask->mC_result = r6_outputResult;
    pMultiChoiceTask->m10_outputTask = r5_outputTask;
    if (r5_outputTask)
    {
        *r5_outputTask = pMultiChoiceTask;
    }
}

void s_multiChoiceTask2_drawMultiChoice(s_multiChoiceTask* pThis)
{
    drawBlueBox(pThis->m14_x, pThis->m16_y, pThis->m1A_width, pThis->m1C_height, 0x1000);

    setupVDP2StringRendering(pThis->m14_x + 2, pThis->m16_y + 1, pThis->m1A_width - 4, pThis->m1C_height - 2);

    for (int i = 0; i < pThis->m6_numEntries; i++)
    {
        vdp2StringContext.m4_cursorX = vdp2StringContext.mC_X + 2;
        vdp2StringContext.m8_cursorY = vdp2StringContext.m10_Y + i * 2;
        if (i < 0)
        {
            vdp2StringContext.m8_cursorY = vdp2StringContext.m10_Y + i * 2 + vdp2StringContext.m18_Height;
        }

        vdp2StringContext.m0 = pThis->m28_colors[i];
        drawObjectName((char*)getSaturnPtr(readSaturnEA(pThis->m24_strings + i * 4)));
    }
}

void s_multiChoiceTask2_Update(s_multiChoiceTask* pThis)
{
    switch (pThis->m0_Status)
    {
    case 0:
        pThis->m0_Status++;
    case 1:
        s_multiChoiceTask2_drawMultiChoice(pThis);
        playSystemSoundEffect(3);
        pThis->m0_Status++;
        return;
    case 2:
        if (graphicEngineStatus.m4514.m0_inputDevices->m0_current.m8_newButtonDown & 0x10) // up
        {
            pThis->m5_selectedEntry--;
            if (pThis->m5_selectedEntry < 0)
            {
                pThis->m5_selectedEntry += pThis->m6_numEntries;
            }
            playSystemSoundEffect(2);
        }
        else if (graphicEngineStatus.m4514.m0_inputDevices->m0_current.m8_newButtonDown & 0x20) // down
        {
            pThis->m5_selectedEntry++;
            if (pThis->m5_selectedEntry >= pThis->m6_numEntries)
            {
                pThis->m5_selectedEntry -= pThis->m6_numEntries;
            }
            playSystemSoundEffect(2);
        }

        if (graphicEngineStatus.m4514.m0_inputDevices->m0_current.m8_newButtonDown & 6) // select
        {
            *pThis->mC_result = pThis->m5_selectedEntry;
            playSystemSoundEffect(0);
            pThis->m0_Status++;
        }
        else if (graphicEngineStatus.m4514.m0_inputDevices->m0_current.m8_newButtonDown & 1) // cancel
        {
            if (pThis->m2_defaultResult)
            {
                *pThis->mC_result = pThis->m2_defaultResult - 1;
                playSystemSoundEffect(1);
                pThis->m0_Status++;
            }
        }
        return;
    case 3:
        pThis->mC_result = NULL;
        pThis->m0_Status++;
        return;
    case 4:
        pThis->getTask()->markFinished();
        return;
    default:
        assert(0);
        break;
    }
}

void s_multiChoiceTask2_Draw(s_multiChoiceTask* pThis)
{
    drawMultiChoiceVdp1Cursor((pThis->m14_x + 2) * 8, (pThis->m16_y + pThis->m5_selectedEntry * 2 + 1) * 8, gCommonFile->getSaturnPtr(0x20FFE0 + pThis->m7 * 10), 0x7F0);

    pThis->m8 += vblankData.m14_numVsyncPerFrame;

    if (pThis->m8 > 1)
    {
        pThis->m7++;
        if (pThis->m7 > 7)
        {
            pThis->m7 = 0;
        }
        pThis->m8 = 0;
    }
}

void clearMultiChoiceBox(s_multiChoiceTask* pThis)
{
    setupVDP2StringRendering(pThis->m14_x + 2, pThis->m16_y + 1, pThis->m1A_width - 4, pThis->m1C_height - 2);
    clearVdp2TextArea();
    clearBlueBox(pThis->m14_x, pThis->m16_y, pThis->m1A_width, pThis->m1C_height);
}

void s_multiChoiceTask2_Delete(s_multiChoiceTask* pThis)
{
    clearMultiChoiceBox(pThis);
    if (pThis->m10_outputTask)
    {
        *pThis->m10_outputTask = nullptr;
    }
}

s_multiChoiceTask* updateMultiChoice(p_workArea parentTask, s_multiChoiceTask** r5, s32* r6_currentChoice, s32 r7_minusCurrentChoice, sSaturnPtr scriptPtr, s16* choiceTable, s32 moreCurrentChoice)
{
    static const s_multiChoiceTask::TypedTaskDefinition definition = {
        nullptr,
        s_multiChoiceTask2_Update,
        s_multiChoiceTask2_Draw,
        s_multiChoiceTask2_Delete,
    };
    s_multiChoiceTask* r14 = createSubTask<s_multiChoiceTask>(parentTask, &definition);

    r14->m0_Status = 0;
    r14->m5_selectedEntry = moreCurrentChoice;

    if (r7_minusCurrentChoice >= 0)
    {
        r14->m1 = r7_minusCurrentChoice;
        r14->m2_defaultResult = 1;
    }
    else
    {
        r14->m1 = -r7_minusCurrentChoice;
        r14->m2_defaultResult = 0;
    }

    r14->m24_strings = scriptPtr;
    r14->m28_colors = choiceTable;
    r14->m3_quantity = 0;
    r14->m4_quantityMax = 0;
    r14->m6_numEntries = r14->m1;
    r14->m5_selectedEntry = moreCurrentChoice;
    r14->m7 = 0;
    r14->m8 = 0;
    r14->m14_x = 2;
    r14->m16_y = 4;

    s32 r12 = 0;
    for (int i = 0; i < r14->m1; i++)
    {
        s32 stringLength = computeStringLength(readSaturnEA(scriptPtr + i * 4), 38);
        if (stringLength > r12)
        {
            r12 = stringLength;
        }
    }

    r14->m1A_width = ((r12 + 1) & ~1) + 6;
    r14->m1C_height = (r14->m1 * 2) + 2;
    r14->mC_result = r6_currentChoice;
    r14->m10_outputTask = r5;

    if (r5)
    {
        *r5 = r14;
    }

    return r14;
}
