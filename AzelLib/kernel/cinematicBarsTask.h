#pragma once

struct s_cinematicBarTask : public s_workAreaTemplate<s_cinematicBarTask>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &s_cinematicBarTask::Init, &s_cinematicBarTask::Update, &s_cinematicBarTask::Draw, NULL };
        return &taskDefinition;
    }

    static void Init(s_cinematicBarTask* pThis)
    {
        pThis->m8 = 0;
        pThis->mB = 0;
        pThis->m9 = 0;
        pThis->mC = 0;
        pThis->mA = 0;
        pThis->mD = 0;
    }
    static void Update(s_cinematicBarTask* pThis);
    static void Draw(s_cinematicBarTask* pThis);

    void interpolateCinematicBar();
    void interpolateCinematicBarSub1();
    void cinematicBarTaskSub0(s32 r5);

    u8 m0_status;
    s8 m1;
    s8 m2;
    s8 m3;
    s8 m4;
    s8 m8;
    s8 m9;
    s8 mA;
    s8 mB;
    s8 mC;
    s8 mD;
    s8 m11;
    //size 0x13
};

s_cinematicBarTask* createCinematicBarTask(p_workArea pParentTask);
void setupCinematicBars(s_cinematicBarTask* pCinematicBar, s32 r5);
void writeCinematicBarsToVdp2();

