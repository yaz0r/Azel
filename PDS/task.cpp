#include "PDS.h"

s_task* taskListHead;
int numActiveTask;

void processTasks(s_task** ppTask)
{
    s_task* pTask = *ppTask;
    
    do
    {
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
                processTasks(&pTask->m_pSubTask);
            }

            // finished but not deleting yet
            if (pTask->isDeleting())
            {
                if (pTask->m_pDelete)
                {
                    pTask->m_pDelete(pTask->getWorkArea());
                }
                numActiveTask--;
            }
            else
            {
                s_heapNode* r4 = pTask->getHeapNode()->m_nextNode;
                while (r4)
                {
                    assert(0);
                    s_heapNode* pNextTask = r4->m_nextNode;
                    freeHeap(r4->getUserData());
                    r4 = pNextTask;
                }

                *ppTask = pTask->m_pNextTask;
                freeHeap(pTask);
//                pTask = *ppTask;
            }
        }
        else
        {
            if (pTask->m_pSubTask)
            {
                processTasks(&pTask->m_pSubTask);
            }
        }

        pTask = *ppTask;

        if (pTask == NULL)
            return;
        
        ppTask = &pTask->m_pNextTask;
        pTask = *ppTask;
    }while (pTask);
}

void runTasks()
{
    if (taskListHead)
    {
        processTasks(&taskListHead);
    }
}

void resetTasks()
{
    taskListHead = NULL;
    numActiveTask = 0;
}

p_workArea createTask_NoArgs(p_workArea parentWorkArea, s_taskDefinition* pDefinition, p_workArea newWorkArea, const char* taskName)
{
    s_task* pTask = (s_task*)allocateHeap(sizeof(s_task));
    if (pTask)
    {
        numActiveTask++;

        s_task* pParentTask = parentWorkArea->getTask();

        while (pParentTask->m_pNextTask)
        {
            pParentTask = pParentTask->m_pNextTask;
        }

        pParentTask->m_pNextTask = pTask;

        pTask->m_pNextTask = NULL;
        pTask->m_pSubTask = NULL;
        pTask->m_pUpdate = pDefinition->m_pUpdate;
        pTask->m_pLateUpdate = pDefinition->m_pLateUpdate;
        pTask->m_pDelete = pDefinition->m_pDelete;
        pTask->m_flags = 0;

        pTask->m_taskName = taskName;

        newWorkArea->m_pTask = pTask;
        pTask->m_workArea = newWorkArea;

        if (pDefinition->m_pInit)
        {
            pDefinition->m_pInit(newWorkArea);
        }
    }

    return pTask->getWorkArea();
}

s_task* createRootTask(s_taskDefinition* pDefinition, p_workArea newWorkArea)
{
    s_task* pTask = (s_task*)allocateHeap(sizeof(s_task));
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

        pTask->m_taskName = "rootTask";

        newWorkArea->m_pTask = pTask;
        pTask->m_workArea = newWorkArea;

        if (pDefinition->m_pInit)
        {
            pDefinition->m_pInit(newWorkArea);
        }
    }

    return pTask;
}

s_task* getTaskFromWorkArea(p_workArea pWorkArea)
{
    return (s_task*)(((u8*)pWorkArea) - sizeof(s_task));
}
