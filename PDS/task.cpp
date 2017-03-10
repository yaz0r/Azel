#include "PDS.h"

s_task* taskListHead;
int numActiveTask;

void processTasks(s_task* pRootTask)
{
    s_task* pTask = pRootTask;
    do
    {
        s_task* pNextTask = NULL;
        if (!(pTask->isFinished()))
        {
            if (!(pTask->isPaused()))
            {
                if (pTask->m_pUpdate)
                {
                    if (!pauseEngine[0])
                    {
                        pTask->m_pUpdate(pTask->getWorkArea());
                    }
                }

                if (pTask->m_pLateUpdate)
                {
                    pTask->m_pLateUpdate(pTask->getWorkArea());
                }
            }
        }
        if (pTask->isFinished())
        {
            pTask->m_flags ^= TASK_FLAGS_DELETING;
            if (pTask->isDeleting())
            {
                s_task* r4 = pTask->m_pSubTask;
                while (r4)
                {
                    r4->markFinished();
                    r4 = r4->m_pNextTask;
                }
            }

            if (pTask->m_pSubTask)
            {
                processTasks(pTask->m_pSubTask);
            }

            // finished but not deleting yet
            if (pTask->isDeleting())
            {
                if (pTask->m_pDelete)
                {
                    pTask->m_pDelete(pTask->getWorkArea());
                }
                numActiveTask--;
                pNextTask = pTask->m_pNextTask;
            }
            else
            {
                s_task* r4 = pTask->m_pSubTask;
                while (r4)
                {
                    s_task* pNextTask = r4->m_pNextTask;
                    //free(r4);
                    r4 = pNextTask;
                }

                pNextTask = pTask->m_pNextTask;
                //free(pTask);
            }
        }
        else
        {
            if (pTask->m_pSubTask)
            {
                processTasks(pTask->m_pSubTask);
            }
            pNextTask = pTask->m_pNextTask;
        }

        pTask = pNextTask;
    } while (pTask);
}

void runTasks()
{
    if (taskListHead)
    {
        processTasks(taskListHead);
    }
}

void resetTasks()
{
    taskListHead = NULL;
    numActiveTask = 0;
}

p_workArea createTask_NoArgs(void* workArea, s_taskDefinition* pDefinition, int size, const char* taskName)
{
    s_task* pTask = (s_task*)malloc(size + sizeof(s_task));
    if (pTask)
    {
        numActiveTask++;

        s_task* pParentTask = ((s_task*)workArea) - 1;

        while (pParentTask->m_pSubTask)
        {
            pParentTask = pParentTask->m_pSubTask;
        }

        pParentTask->m_pSubTask = pTask;

        pTask->m_pNextTask = NULL;
        pTask->m_pSubTask = NULL;
        pTask->m_pUpdate = pDefinition->m_pUpdate;
        pTask->m_pLateUpdate = pDefinition->m_pLateUpdate;
        pTask->m_pDelete = pDefinition->m_pDelete;
        pTask->m_flags = 0;

        pTask->m_taskName = taskName;

        u8* pTaskWorkArea = (u8*)(pTask + 1);
        memset(pTaskWorkArea, 0, size);

        if (pDefinition->m_pInit)
        {
            pDefinition->m_pInit(pTaskWorkArea);
        }
    }

    return (u8*)(pTask->getWorkArea());
}

s_task* createRootTask(s_taskDefinition* pDefinition, int size)
{
    s_task* pTask = (s_task*)malloc(size + sizeof(s_task));
    if (pTask)
    {
        numActiveTask++;
        taskListHead = pTask;

        pTask->m_pNextTask = NULL;
        pTask->m_pSubTask = NULL;
        pTask->m_pUpdate = pDefinition->m_pUpdate;
        pTask->m_pLateUpdate = pDefinition->m_pLateUpdate;
        pTask->m_pDelete = pDefinition->m_pDelete;
        pTask->m_flags = 0;

        u8* pTaskWorkArea = (u8*)(pTask + 1);
        memset(pTaskWorkArea, 0, size);

        if (pDefinition->m_pInit)
        {
            pDefinition->m_pInit(pTaskWorkArea);
        }
    }

    return pTask;
}

s_task* getTaskFromWorkArea(p_workArea pWorkArea)
{
    return (s_task*)(((u8*)pWorkArea) - sizeof(s_task));
}
