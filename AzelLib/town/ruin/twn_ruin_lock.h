#pragma once

struct sLockTask : public s_workAreaTemplateWithArgWithCopy<sLockTask, sSaturnPtr>, sTownObject
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sLockTask::Init, &sLockTask::Update, nullptr, &sLockTask::Delete };
        return &taskDefinition;
    }

    static void Init(sLockTask* pThis, sSaturnPtr);
    static void Update(sLockTask* pThis);
    static void UpdateAlternate(sLockTask* pThis);
    static void Draw(sLockTask* pThis);
    static void Delete(sLockTask* pThis);

    //0-4: copy
    //8 : sTownObject
    sSaturnPtr mC;
    sMainLogic_74 m10;
    sVec3_FP m74_translation;
    sVec3_FP m80_rotation;
    s16 m8E;
    //size 0x90
};
