#include "PDS.h"

s_task* taskListHead;
int numActiveTask;

void PrintDebugTask(s_task* pTask)
{
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
            if (pTask->m8_pUpdate)
            {
                if (!pauseEngine[0])
                {
                    pTask->m8_pUpdate(pTask->getWorkArea());
                }
            }

            if (pTask->mC_pDraw)
            {
                pTask->mC_pDraw(pTask->getWorkArea());
            }
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
                if (pTask->m10_pDelete)
                {
                    pTask->m10_pDelete(pTask->getWorkArea());
                }
                numActiveTask--;
            }
            else
            {
                s_heapNode* r4 = pTask->getHeapNode()->m_nextNode;
#ifdef __EMSCRIPTEN__
                r4 = nullptr;
#endif
                while (r4)
                {
                    assert(0);
                    s_heapNode* pNextTask = r4->m_nextNode;
                    freeHeap(r4->getUserData());
                    r4 = pNextTask;
                }

                *ppTask = pTask->m0_pNextTask;
                freeHeap(pTask);
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

p_workArea createSubTask(p_workArea parentWorkArea, const s_taskDefinition* pDefinition, p_workArea newWorkArea)
{
    s_task* pTask = (s_task*)allocateHeap(sizeof(s_task));
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
    pTask->m8_pUpdate = pDefinition->m_pUpdate;
    pTask->mC_pDraw = pDefinition->m_pLateUpdate;
    pTask->m10_pDelete = pDefinition->m_pDelete;
    pTask->m14_flags = 0;

    assert(pDefinition->m_taskName);
    pTask->m_taskName = pDefinition->m_taskName;

    newWorkArea->m_pTask = pTask;
    pTask->m_workArea = newWorkArea;

    if (pDefinition->m_pInit)
    {
        pDefinition->m_pInit(newWorkArea);
    }

    return pTask->getWorkArea();
}

p_workArea createSubTaskWithArg(p_workArea parentWorkArea, const s_taskDefinitionWithArg* pDefinition, p_workArea newWorkArea, void* argument)
{
    s_task* pTask = (s_task*)allocateHeap(sizeof(s_task));
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
    pTask->m8_pUpdate = pDefinition->m_pUpdate;
    pTask->mC_pDraw = pDefinition->m_pLateUpdate;
    pTask->m10_pDelete = pDefinition->m_pDelete;
    pTask->m14_flags = 0;

    assert(pDefinition->m_taskName);
    pTask->m_taskName = pDefinition->m_taskName;

    newWorkArea->m_pTask = pTask;
    pTask->m_workArea = newWorkArea;

    if (pDefinition->m_pInit)
    {
        pDefinition->m_pInit(newWorkArea, argument);
    }

    return pTask->getWorkArea();
}

p_workArea createSiblingTaskWithArg(p_workArea workArea, const s_taskDefinitionWithArg* pDefinition, p_workArea pNewWorkArea, void* argument)
{
    s_task* pTask = (s_task*)allocateHeap(sizeof(s_task));
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
    pTask->m8_pUpdate = pDefinition->m_pUpdate;
    pTask->mC_pDraw = pDefinition->m_pLateUpdate;
    pTask->m10_pDelete = pDefinition->m_pDelete;
    pTask->m14_flags = 0;

    assert(pDefinition->m_taskName);
    pTask->m_taskName = pDefinition->m_taskName;

    pNewWorkArea->m_pTask = pTask;
    pTask->m_workArea = pNewWorkArea;

    if (pDefinition->m_pInit)
    {
        pDefinition->m_pInit(pNewWorkArea, argument);
    }

    return pTask->getWorkArea();

}

p_workArea createSubTaskFromFunction(p_workArea parentWorkArea, void(*pFunction)(p_workArea), p_workArea newWorkArea, const char* name)
{
    s_task* pTask = (s_task*)allocateHeap(sizeof(s_task));
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
    pTask->m8_pUpdate = pFunction;
    pTask->mC_pDraw = NULL;
    pTask->m10_pDelete = NULL;
    pTask->m14_flags = 0;

    pTask->m_taskName = name;

    newWorkArea->m_pTask = pTask;
    pTask->m_workArea = newWorkArea;

    return pTask->getWorkArea();

}

s_workArea* createRootTask(s_taskDefinitionWithArg* pDefinition, p_workArea newWorkArea)
{
    s_task* pTask = (s_task*)allocateHeap(sizeof(s_task));
    assert(pTask);

    numActiveTask++;
    taskListHead = pTask;

    pTask->m0_pNextTask = NULL;
    pTask->m4_pSubTask = NULL;
    pTask->m8_pUpdate = pDefinition->m_pUpdate;
    pTask->mC_pDraw = pDefinition->m_pLateUpdate;
    pTask->m10_pDelete = pDefinition->m_pDelete;
    pTask->m14_flags = 0;

    assert(pDefinition->m_taskName);
    pTask->m_taskName = pDefinition->m_taskName;

    newWorkArea->m_pTask = pTask;
    pTask->m_workArea = newWorkArea;

    if (pDefinition->m_pInit)
    {
        pDefinition->m_pInit(newWorkArea, NULL);
    }

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

