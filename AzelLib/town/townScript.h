#pragma once

struct sScriptTask : public s_workAreaTemplate<sScriptTask>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sScriptTask::Init, &sScriptTask::Update, &sScriptTask::Draw, NULL };
        return &taskDefinition;
    }

    static void Init(sScriptTask* pThis);
    static void Update(sScriptTask* pThis);
    static void Draw(sScriptTask* pThis);

    s32 m4;
    s32 m8;
    s32 m18;
    //size 0x20
};

extern sScriptTask* currentResTask;

void startScriptTask(p_workArea r4);
sNPC* getNpcDataByIndex(s32 r4);
