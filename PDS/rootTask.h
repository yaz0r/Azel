#pragma once

struct s_initialTaskStatus
{
    p_workArea (*m_pendingTask)(void*);
    p_workArea (*m_currentTask)(void*);
    p_workArea (*m_previousTask)(void*);
    u32 m_showWarningTask;
};

extern s_initialTaskStatus initialTaskStatus;

void startInitialTask();
