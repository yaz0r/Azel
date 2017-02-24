#pragma once

struct s_initialTaskStatus
{
    s_task* (*m_pendingTask)(void*);
    s_task*(*m_currentTask)(void*);
    s_task*(*m_previousTask)(void*);
    u32 m_showWarningTask;
};

extern s_initialTaskStatus initialTaskStatus;

void startInitialTask();
