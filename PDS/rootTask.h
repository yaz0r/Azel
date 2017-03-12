#pragma once

struct s_initialTaskStatus
{
    p_workArea (*m_pendingTask)(p_workArea);
    p_workArea (*m_currentTask)(p_workArea);
    p_workArea (*m_previousTask)(p_workArea);
    u32 m_showWarningTask;
};

extern s_initialTaskStatus initialTaskStatus;

void startInitialTask();
