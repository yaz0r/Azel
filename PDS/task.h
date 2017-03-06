#pragma once

struct s_taskDefinition
{
    void(*m_pInit)(void*);
    void(*m_pUpdate)(void*);
    void(*m_pLateUpdate)(void*);
    void(*m_pDelete)(void*);
};

struct s_task
{
    s_task* m_pNextTask;
    s_task* m_pDependentTask;
    void(*m_pLateUpdate)(void*);
    void(*m_pDelete)(void*);
    void(*m_pUpdate)(void*);
    u32 m_flags;
};

void resetTasks();
void runTasks();

s_task* createTask_NoArgs(void* workArea, s_taskDefinition* pDefinition, int size);

s_task* createRootTask(s_taskDefinition* pDefinition, int size);

s_task* getTaskFromWorkArea(void*);

