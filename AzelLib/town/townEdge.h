#pragma once

struct sEdgeTask : public s_workAreaTemplateWithArgAndBase<sEdgeTask, sNPC, sSaturnPtr>
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

    // All fields now in sNPC base
    //size 0x17C
};

sEdgeTask* startEdgeTask(sSaturnPtr r4);

void stepNPCForward(sNPCE8* pThis);
void applyEdgeAnimation(s_3dModel* pModel, sVec2_FP* r5);
void applyEdgeAnimation2(s_3dModel* pModel, sVec2_FP* r5);
