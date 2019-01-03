#include "PDS.h"

struct s_initialTaskWorkArea : public s_workAreaTemplate<s_initialTaskWorkArea>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &s_initialTaskWorkArea::Init, NULL, &s_initialTaskWorkArea::Draw, NULL, "sFieldA3_1_fieldIntroTask" };
        return &taskDefinition;
    }

    static void Init(s_initialTaskWorkArea*);
    static void Draw(s_initialTaskWorkArea*);

    u32 m_state;
    p_workArea m_4;
};

s_initialTaskStatus initialTaskStatus;

void s_initialTaskWorkArea::Init(s_initialTaskWorkArea* pWorkArea)
{
    pWorkArea->m_state = 0;
    pWorkArea->m_4 = 0;

    initialTaskStatus.m_currentTask = 0;
    if (initialTaskStatus.mC_showWarningTask)
    {
        initialTaskStatus.m_pendingTask = startLoadWarningTask;
    }
    else
    {
        initialTaskStatus.m_pendingTask = createTitleScreenTask;
    }

    PDS_unimplemented("resetNamesForNewGame");
    //resetNamesForNewGame();
}
void s_initialTaskWorkArea::Draw(s_initialTaskWorkArea* pWorkArea)
{
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
                s_task* pTask = pWorkArea->m_4->getTask();
                pTask->m14_flags |= 1;
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

void startInitialTask()
{
    createRootTask<s_initialTaskWorkArea>();
}
