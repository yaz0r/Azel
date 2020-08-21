#include "PDS.h"
#include "cinematicBarsTask.h"

void setVdp2TableAddress(int p1, u8* vdp2Dest); //TODO: cleanup

s_cinematicBarTask* createCinematicBarTask(p_workArea pParentTask)
{
    return createSubTask<s_cinematicBarTask>(pParentTask);
}

void cinematicBars_startClosing(s_cinematicBarTask* pCinematicBar, s32 r5)
{
    pCinematicBar->m1 = 0;
    pCinematicBar->m2 = r5;
    pCinematicBar->m0_status = s_cinematicBarTask::m2_opening;
}

std::array<u32, 256> interpolateCinematicBarData;

void s_cinematicBarTask::interpolateCinematicBarSub1()
{
    for (int i = 0; i < 0xE0; i++)
    {
        if ((i < m3) || (i >= 0xE0 - m4))
        {
            WRITE_BE_U32(&interpolateCinematicBarData[i], 0x1010000);
        }
        else
        {
            WRITE_BE_U32(&interpolateCinematicBarData[i], i * 0x10000);
        }
    }
}

void s_cinematicBarTask::cinematicBars_startOpening(s32 r5)
{
    m1 = r5;
    m2 = r5;
    m0_status = s_cinematicBarTask::m3_closing;
}

void s_cinematicBarTask::interpolateCinematicBar()
{
    fixedPoint r5 = performDivision(fixedPoint(m2), fixedPoint(m1 * 16));
    if (fixedPoint(m3) != r5)
    {
        m3 = r5;
        m11 = 1;
    }

    r5 = performDivision(fixedPoint(m2), fixedPoint(m1 * 32));
    if (fixedPoint(m4) != r5)
    {
        m4 = r5;
        m11 = 1;
    }

    if (m11)
    {
        interpolateCinematicBarSub1();
    }
}

void s_cinematicBarTask::Update(s_cinematicBarTask* pThis)
{
    switch (pThis->m0_status)
    {
    case m0_closed:
    case m1_open:
        return;
    case m2_opening:
        if (pThis->m2 == ++pThis->m1)
        {
            pThis->m0_status = m1_open;
        }
        pThis->interpolateCinematicBar();
        break;
    case m3_closing:
        if (--pThis->m1 == 0)
        {
            pThis->m0_status = m0_closed;
        }
        pThis->interpolateCinematicBar();
        break;
    default:
        assert(0);
        break;
    }
}

void drawCinematicBar(int param1)
{
    sVdpVar1* puVar3 = &vdpVar1[param1];
    int iVar1 = vdpVar1[param1].mF_isPending;

    if (iVar1 == 0)
    {
        vdpVar3->m10_nextTransfert = puVar3;
        vdpVar1[param1].mF_isPending = 1;
        vdpVar1[param1].m10_nextTransfert = nullptr;
        vdpVar3 = puVar3;
    }
}

void s_cinematicBarTask::Draw(s_cinematicBarTask* pThis)
{
    if (pThis->m11)
    {
        pThis->m11 = 0;
        drawCinematicBar(2);
    }
}

void setupCinematicBarData(int param1, std::array<u32, 256>& dataArray, u32 vdpOffset, int numEntries)
{
    vdpVar1[param1].mE_isDoubleBuffered = 0;
    vdpVar1[param1].m0_source[0] = &dataArray[0];
    vdpVar1[param1].m8_destination = getVdp2Vram(vdpOffset);
    vdpVar1[param1].mC_size = numEntries / 0x10;
    vdpVar1[param1].m10_nextTransfert = nullptr;
    setVdp2TableAddress(param1, getVdp2Vram(vdpOffset));
}

void writeCinematicBarsToVdp2()
{
    int iVar1 = 0x100;
    for (int i=0; i<0x20; i++)
    {
        iVar1--;
        WRITE_BE_U32(&interpolateCinematicBarData[iVar1], 0x1010000);
    }
    do 
    {
        iVar1--;
        WRITE_BE_U32(&interpolateCinematicBarData[iVar1], iVar1 * 0x10000);
    } while (iVar1);

    u32 vdp2Offset = 0x25E04D00;
    iVar1 = 0x80;
    do 
    {
        iVar1--;
        setVdp2VramU8(vdp2Offset, 0x11);
        vdp2Offset++;
    } while (iVar1);

    setupCinematicBarData(2, interpolateCinematicBarData, 0x25e3e000, 0x100);
    drawCinematicBar(2);
}
