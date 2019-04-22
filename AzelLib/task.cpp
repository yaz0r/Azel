#include "PDS.h"

s_task* taskListHead;
int numActiveTask;

void PrintDebugTask(s_task* pTask)
{
    ImGui::PushID(0);
    if(ImGui::TreeNode(pTask->m_taskName))
    {
        if (pTask->m4_pSubTask)
        {
            PrintDebugTask(pTask->m4_pSubTask);
        }
        ImGui::TreePop();
    }
    if (pTask->m0_pNextTask)
    {
        PrintDebugTask(pTask->m0_pNextTask);
    }
    ImGui::PopID();
}

void DebugTasks()
{
    ImGui::Begin("Tasks");
    if (taskListHead)
    {
        PrintDebugTask(taskListHead);
    }
    ImGui::End();
}

void processTasks(s_task** ppTask)
{
    s_task* pTask = *ppTask;
    
    do
    {
        if (!(pTask->isFinished()))
        {
            if (pTask->isPaused())
            {
                pTask = *ppTask;

                if (pTask == NULL)
                    return;

                ppTask = &pTask->m0_pNextTask;
                pTask = *ppTask;
                continue;
            }
            if (!pauseEngine[0])
            {
                pTask->getWorkArea()->Update();
            }

            pTask->getWorkArea()->Draw();
        }
        if (pTask->isFinished())
        {
            pTask->m14_flags ^= TASK_FLAGS_DELETING;
            if (pTask->isDeleting())
            {
                s_task* r4 = pTask->m4_pSubTask;
                while (r4)
                {
                    r4->markFinished();
                    r4 = r4->m0_pNextTask;
                }
            }

            if (pTask->m4_pSubTask)
            {
                processTasks(&pTask->m4_pSubTask);
            }

            // finished but not deleting yet
            if (pTask->isDeleting())
            {
                pTask->getWorkArea()->Delete();

                numActiveTask--;
            }
            else
            {
                s_heapNode* r4 = pTask->getHeapNode();
                PDS_unimplemented("Disabled freeing task heaps");
                /*
                while (r4)
                {
                    s_heapNode* pNextNode = r4->m_nextNode;
                    freeHeapForTask(pTask->getWorkArea(), r4);
                    r4 = pNextNode;
                }
                */

                *ppTask = pTask->m0_pNextTask;
                delete pTask;
//                pTask = *ppTask;
            }
        }
        else
        {
            if (pTask->m4_pSubTask)
            {
                processTasks(&pTask->m4_pSubTask);
            }
        }

        pTask = *ppTask;

        if (pTask == NULL)
            return;
        
        ppTask = &pTask->m0_pNextTask;
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

p_workArea createSubTask(p_workArea parentWorkArea, p_workArea newWorkArea)
{
    s_task* pTask = new s_task;
    assert(pTask);

    numActiveTask++;

    s_task* pParentTask = parentWorkArea->getTask();

    s_task** taskDestination = &pParentTask->m4_pSubTask;
    while (*taskDestination)
    {
        taskDestination = &(*taskDestination)->m0_pNextTask;
    }

    (*taskDestination) = pTask;

    pTask->m0_pNextTask = NULL;
    pTask->m4_pSubTask = NULL;
    pTask->m14_flags = 0;

    newWorkArea->m_pTask = pTask;
    pTask->m_workArea = newWorkArea;

    return pTask->getWorkArea();
}

p_workArea createSubTaskWithArg(p_workArea parentWorkArea, p_workArea newWorkArea)
{
    s_task* pTask = new s_task;
    assert(pTask);

    numActiveTask++;

    s_task* pParentTask = parentWorkArea->getTask();

    s_task** taskDestination = &pParentTask->m4_pSubTask;
    while (*taskDestination)
    {
        taskDestination = &(*taskDestination)->m0_pNextTask;
    }

    (*taskDestination) = pTask;

    pTask->m0_pNextTask = NULL;
    pTask->m4_pSubTask = NULL;
    pTask->m14_flags = 0;

    newWorkArea->m_pTask = pTask;
    pTask->m_workArea = newWorkArea;

    return pTask->getWorkArea();
}

p_workArea createSiblingTaskWithArg(p_workArea workArea, p_workArea pNewWorkArea)
{
    s_task* pTask = new s_task;
    assert(pTask);

    numActiveTask++;

    s_task* pSibling = workArea->getTask();

    s_task** taskDestination = &pSibling->m0_pNextTask;
    while (*taskDestination)
    {
        taskDestination = &(*taskDestination)->m0_pNextTask;
    }

    (*taskDestination) = pTask;

    pTask->m0_pNextTask = NULL;
    pTask->m4_pSubTask = NULL;
    pTask->m14_flags = 0;

    pNewWorkArea->m_pTask = pTask;
    pTask->m_workArea = pNewWorkArea;

    return pTask->getWorkArea();

}

s_workArea* createRootTask(p_workArea newWorkArea)
{
    s_task* pTask = new s_task;
    assert(pTask);

    numActiveTask++;
    taskListHead = pTask;

    pTask->m0_pNextTask = NULL;
    pTask->m4_pSubTask = NULL;
    pTask->m14_flags = 0;

    newWorkArea->m_pTask = pTask;
    pTask->m_workArea = newWorkArea;

    return newWorkArea;
}

s_task* getTaskFromWorkArea(p_workArea pWorkArea)
{
    return (s_task*)(((u8*)pWorkArea) - sizeof(s_task));
}

void dummyTaskInit(s_workArea* pWorkArea)
{
    printf("Unimplmenmented Init task for %s\n", pWorkArea->getTask()->m_taskName);
}

void dummyTaskInitWithArg(s_workArea* pWorkArea, void* arg)
{
    printf("Unimplmenmented Init task for %s\n", pWorkArea->getTask()->m_taskName);
}

void dummyTaskUpdate(s_workArea* pWorkArea)
{
    printf("Unimplmenmented Update task for %s\n", pWorkArea->getTask()->m_taskName);
}

void dummyTaskDraw(s_workArea* pWorkArea)
{
    printf("Unimplmenmented Draw task for %s\n", pWorkArea->getTask()->m_taskName);
}

void dummyTaskDelete(s_workArea* pWorkArea)
{
    printf("Unimplmenmented Delete task for %s\n", pWorkArea->getTask()->m_taskName);
}

