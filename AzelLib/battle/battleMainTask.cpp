#include "PDS.h"
#include "battleMainTask.h"
#include "town/town.h" // TODO: split out the town grid
#include "audio/soundDriver.h"

struct sBattleMainTask : public s_workAreaTemplate<sBattleMainTask>
{
    s32 m0_status;
    p_workArea m4_parent;
    void(*m8)(p_workArea);
    void(*mC)(p_workArea);
    //size: 0x10
};

static void battleMainTask_Draw(sBattleMainTask* pThis)
{
    switch (pThis->m0_status)
    {
    case 0:
        pThis->m8(pThis->m4_parent);
        pThis->m0_status++;
        break;
    case 1:
        if (isSoundLoadingFinished())
        {
            pThis->m0_status++;
        }
        break;
    case 2:
        reset3dEngine();
        resetCameraProperties2(&cameraProperties2);
        initTownGrid();
        pThis->mC(pThis->m4_parent);
        pThis->getTask()->markFinished();
        break;
    default:
        assert(0);
        break;
    }
}

p_workArea createBattleMainTask(p_workArea parent, void(*init0)(p_workArea), void(*init1)(p_workArea))
{
    static const sBattleMainTask::TypedTaskDefinition definition = {
        nullptr,
        nullptr,
        &battleMainTask_Draw,
        nullptr,
    };

    sBattleMainTask* pBattleMainTask = createSubTask<sBattleMainTask>(parent, &definition);
    if (pBattleMainTask)
    {
        pBattleMainTask->m4_parent = parent;
        pBattleMainTask->m8 = init0;
        pBattleMainTask->mC = init1;
    }

    return pBattleMainTask;
}
