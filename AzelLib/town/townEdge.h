#pragma once

struct sEdgeTask : public s_workAreaTemplateWithArgWithCopy<sEdgeTask, sSaturnPtr>, sNPC
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sEdgeTask::Init, &sEdgeTask::Update, &sEdgeTask::Draw, &sEdgeTask::Delete };
        return &taskDefinition;
    }

    static void Init(sEdgeTask* pThis, sSaturnPtr arg);
    static void Update(sEdgeTask* pThis);
    static void Draw(sEdgeTask* pThis);
    static void Delete(sEdgeTask* pThis);

    s16 m14C_inputFlags;
    s16 m14E;
    s32 m150_inputX;
    s32 m154_inputY;
    s8 m178;
    s8 m179;
    s8 m17A;
    s8 m17B;
    //size 0x17C
};

sEdgeTask* startEdgeTask(sSaturnPtr r4);
