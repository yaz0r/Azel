#pragma once

struct s_taskDefinition
{
    void(*m_pInit)(void*);
    void(*m_pUpdate)(void*);
    void(*m_pLateUpdate)(void*);
    void(*m_pDelete)(void*);
};

typedef void* p_workArea;

#define TASK_FLAGS_FINISHED 1
#define TASK_FLAGS_PAUSED 2
#define TASK_FLAGS_DELETING 4

struct s_task
{
    s_task* m_pNextTask;
    s_task* m_pSubTask;
    void(*m_pLateUpdate)(void*);
    void(*m_pDelete)(void*);
    void(*m_pUpdate)(void*);
    u32 m_flags;
    const char* m_taskName;

    p_workArea getWorkArea()
    {
        return this + 1;
    }

    bool isFinished()
    {
        return m_flags & TASK_FLAGS_FINISHED;
    }
    void markFinished()
    {
        m_flags |= TASK_FLAGS_FINISHED;
    }
    bool isPaused()
    {
        return m_flags & TASK_FLAGS_PAUSED;
    }
    void markPaused()
    {
        m_flags |= TASK_FLAGS_PAUSED;
    }
    bool isDeleting()
    {
        return m_flags & TASK_FLAGS_DELETING;
    }
    void markDeleting()
    {
        m_flags |= TASK_FLAGS_DELETING;
    }
};

void resetTasks();
void runTasks();

p_workArea createTask_NoArgs(p_workArea workArea, s_taskDefinition* pDefinition, int size, const char* taskName);

s_task* createRootTask(s_taskDefinition* pDefinition, int size);

s_task* getTaskFromWorkArea(p_workArea);

