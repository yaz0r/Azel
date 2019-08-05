#include "PDS.h"
#include "cinematicBarsTask.h"

s_cinematicBarTask* createCinematicBarTask(p_workArea pParentTask)
{
    return createSubTask<s_cinematicBarTask>(pParentTask);
}

void setupCinematicBars(s_cinematicBarTask* pCinematicBar, s32 r5)
{
    pCinematicBar->m1 = 0;
    pCinematicBar->m2 = r5;
    pCinematicBar->m0_status = 2;
}

u32 interpolateCinematicBarData[512];

void s_cinematicBarTask::interpolateCinematicBarSub1()
{
    for (int i = 0; i < 0xE0; i++)
    {
        if ((i < m3) || (i >= 0xE0 - m4))
        {
            interpolateCinematicBarData[i] = 0x1010000;
        }
        else
        {
            interpolateCinematicBarData[i] = i << 16;
        }
    }
}

void s_cinematicBarTask::cinematicBarTaskSub0(s32 r5)
{
    m1 = r5;
    m2 = r5;
    m0_status = 3;
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
    case 2:
        if (pThis->m2 == ++pThis->m1)
        {
            pThis->m0_status = 1;
        }
        pThis->interpolateCinematicBar();
        break;
    case 1:
        return;
    case 3:
        if (--pThis->m1 == 0)
        {
            pThis->m0_status = 0;
        }
        pThis->interpolateCinematicBar();
        break;
    default:
        assert(0);
        break;
    }
}

void s_cinematicBarTask::Draw(s_cinematicBarTask*)
{
    PDS_unimplemented("s_cinematicBarTask::Draw");
}
