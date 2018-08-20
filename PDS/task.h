#pragma once

#include <vector>
#include "heap.h"
struct s_workArea
{
    struct s_task* m_pTask;

    s_task* getTask()
    {
        return m_pTask;
    }

    void* operator new(size_t size)
    {
        void* ptr = malloc(size);
        memset(ptr, 0, size);

        return ptr;
    }

    virtual void Init()
    {
        unimplemented("Init");
    }

    virtual void Update()
    {
        unimplemented("Update");
    }

    virtual void Draw()
    {
        unimplemented("Draw");
    }

    virtual void Delete()
    {
        unimplemented("Delete");
    }
};

struct s_taskDefinition
{
    void(*m_pInit)(s_workArea*);
    void(*m_pUpdate)(s_workArea*);
    void(*m_pLateUpdate)(s_workArea*);
    void(*m_pDelete)(s_workArea*);
    const char* m_taskName;
};

struct s_taskDefinitionWithArg
{
    void(*m_pInit)(s_workArea*, void* argument);
    void(*m_pUpdate)(s_workArea*);
    void(*m_pLateUpdate)(s_workArea*);
    void(*m_pDelete)(s_workArea*);
    const char* m_taskName;
};

typedef s_workArea* p_workArea;

#define TASK_FLAGS_FINISHED 1
#define TASK_FLAGS_PAUSED 2
#define TASK_FLAGS_DELETING 4

struct s_task
{
    s_task* m_pNextTask;
    s_task* m_pSubTask;
    void(*m_pDraw)(p_workArea);
    void(*m_pDelete)(p_workArea);
    void(*m_pUpdate)(p_workArea);
    u32 m_flags;
    const char* m_taskName;

    s_workArea* m_workArea;

    p_workArea getWorkArea()
    {
        return m_workArea;
    }
    s_heapNode* getHeapNode()
    {
        return ((s_heapNode*)(this)) - 1;
    }

    bool isFinished()
    {
        return (m_flags & TASK_FLAGS_FINISHED) ? true : false;
    }
    void markFinished()
    {
        m_flags |= TASK_FLAGS_FINISHED;
    }
    bool isPaused()
    {
        return (m_flags & TASK_FLAGS_PAUSED) ? true : false;
    }
    void clearPaused()
    {
        m_flags &= ~TASK_FLAGS_PAUSED;
    }
    void markPaused()
    {
        m_flags |= TASK_FLAGS_PAUSED;
    }
    bool isDeleting()
    {
        return (m_flags & TASK_FLAGS_DELETING) ? true : false;
    }
    void markDeleting()
    {
        m_flags |= TASK_FLAGS_DELETING;
    }
};

void resetTasks();
void runTasks();

p_workArea createSubTask(p_workArea workArea, s_taskDefinition* pDefinition, p_workArea pNewWorkArea);
p_workArea createSubTaskWithArg(p_workArea workArea, s_taskDefinitionWithArg* pDefinition, p_workArea pNewWorkArea, void* argument);
p_workArea createSiblingTaskWithArg(p_workArea workArea, s_taskDefinitionWithArg* pDefinition, p_workArea pNewWorkArea, void* argument);

p_workArea createSubTaskFromFunction(p_workArea workArea, void(*pFunction)(p_workArea), p_workArea pNewWorkArea, const char* name);

s_task* createRootTask(s_taskDefinition* pDefinition, p_workArea pNewWorkArea);

void dummyTaskInit(s_workArea* pWorkArea);
void dummyTaskInitWithArg(s_workArea* pWorkArea, void* arg);
void dummyTaskUpdate(s_workArea* pWorkArea);
void dummyTaskDraw(s_workArea* pWorkArea);
void dummyTaskDelete(s_workArea* pWorkArea);

struct s_dummyWorkArea : s_workArea
{

};

void DebugTasks();

