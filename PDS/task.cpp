#include "PDS.h"

s_task* taskListHead;
int numActiveTask;

void processTasks(s_task* pTask)
{
    do
    {
        if (!(pTask->m_flags & 1))
        {
            if (!(pTask->m_flags & 2))
            {
                if (pTask->m_pUpdate)
                {
                    if (!pauseEngine[0])
                    {
                        pTask->m_pUpdate(pTask + 1);
                    }
                }

                if (pTask->m_pLateUpdate)
                {
                    pTask->m_pLateUpdate(pTask + 1);
                }
            }
        }
        if (pTask->m_flags & 1)
        {
            pTask->m_flags ^= 4;
            if (pTask->m_flags & 4)
            {
                assert(0);
            }

            if (pTask->m_pDependentTask)
            {
                processTasks(pTask->m_pDependentTask);
            }

            if (!(pTask->m_flags & 4))
            {
                assert(0);
            }
        }
        else
        {
            if (pTask->m_pDependentTask)
            {
                processTasks(pTask->m_pDependentTask);
            }
        }

        pTask = pTask->m_pNextTask;
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

s_task* createTask_NoArgs(void* workArea, s_taskDefinition* pDefinition, int size)
{
    s_task* pTask = (s_task*)malloc(size + sizeof(s_task));
    if (pTask)
    {
        numActiveTask++;

        s_task* pParentTask = ((s_task*)workArea) - 1;

        while (pParentTask->m_pDependentTask)
        {
            pParentTask = pParentTask->m_pDependentTask;
        }

        pParentTask->m_pDependentTask = pTask;

        pTask->m_pNextTask = NULL;
        pTask->m_pDependentTask = NULL;
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

s_task* createRootTask(s_taskDefinition* pDefinition, int size)
{
    s_task* pTask = (s_task*)malloc(size + sizeof(s_task));
    if (pTask)
    {
        numActiveTask++;
        taskListHead = pTask;

        pTask->m_pNextTask = NULL;
        pTask->m_pDependentTask = NULL;
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