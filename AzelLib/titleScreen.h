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


struct s_titleScreenWorkArea : public s_workAreaTemplate<s_titleScreenWorkArea>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &s_titleScreenWorkArea::Init, NULL, &s_titleScreenWorkArea::Draw, NULL };
        return &taskDefinition;
    }

    static void Init(s_titleScreenWorkArea*);
    static void Draw(s_titleScreenWorkArea*);

    u32 m0_status;
    u32 m4_delay;
    p_workArea m8_overlayTask;
};
