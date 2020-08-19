#pragma once

// Title task
p_workArea startSegaLogoModule(p_workArea workArea);
p_workArea createTitleScreenTask(p_workArea workArea);
p_workArea startLoadWarningTask(p_workArea);

struct s_titleMenuEntry
{
    u16 m_isEnabled;
    s16 m_var2;
    const char* m_text;
    p_workArea(*m_createTask)(p_workArea);
};
