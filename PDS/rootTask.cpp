#include "PDS.h"

struct s_initialTaskWorkArea
{
    u32 m_state;
    s_task* m_4;
};

s_initialTaskStatus initialTaskStatus;

void initialTask_Init(void* pTypelessWorkArea)
{
    s_initialTaskWorkArea* pWorkArea = (s_initialTaskWorkArea*)pTypelessWorkArea;
    pWorkArea->m_state = 0;
    pWorkArea->m_4 = 0;

    initialTaskStatus.m_currentTask = 0;
    if (initialTaskStatus.m_showWarningTask)
    {
        initialTaskStatus.m_pendingTask = startLoadWarningTask;
    }
    else
    {
        initialTaskStatus.m_pendingTask = createTitleScreenTask;
    }

    //resetNamesForNewGame();
}
void initialTask_Update(void* pTypelessWorkArea)
{
    s_initialTaskWorkArea* pWorkArea = (s_initialTaskWorkArea*)pTypelessWorkArea;

    switch (pWorkArea->m_state)
    {
    case 0: // init state
            // Some sound init code here
        pWorkArea->m_state++;
        // fall through
    case 1: // wait for new task
        if (!initialTaskStatus.m_pendingTask)
            return;
        if (initialTaskStatus.m_pendingTask != initialTaskStatus.m_currentTask)
        {
            if (pWorkArea->m_4)
            {
                assert(0);
            }

            initialTaskStatus.m_previousTask = initialTaskStatus.m_currentTask;
            initialTaskStatus.m_currentTask = initialTaskStatus.m_pendingTask;
            pWorkArea->m_state++;
        }
        initialTaskStatus.m_pendingTask = NULL;
        break;
    case 2: // start new task
        pWorkArea->m_4 = initialTaskStatus.m_currentTask(pWorkArea);
        pWorkArea->m_state--;
        break;
    default:
        assert(0);
    }
}

s_taskDefinition initialTask = { initialTask_Init , NULL, initialTask_Update , NULL };

void startInitialTask()
{
    createRootTask(&initialTask, sizeof(s_initialTaskWorkArea));
}