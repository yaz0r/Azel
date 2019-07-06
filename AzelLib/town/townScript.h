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

    sNpcData* m4;
    s32 m8;
    s32 mC;
    s32 m18;
    //size 0x20
};
extern sScriptTask* currentResTask;

struct sResData1C
{
    sResData1C* m0_pNext;
    sMainLogic_74* m4;
};

struct sResData
{
    s32 m0;
    s32 m4;
    std::array<sResData1C*, 5>m8_headOfLinkedList;
    std::array<sResData1C, 0x3F>m1C;
};
extern sResData resData;

struct sResCameraProperties
{
    fixedPoint m0_LCS_X;
    fixedPoint m4_LCS_Y;
    s32 m8;
    s32 mC;
    s32 m10;
    s32 m14;
    fixedPoint m18;
    fixedPoint m20;
    fixedPoint m24;
    fixedPoint m28;
    fixedPoint m2C;
    fixedPoint m30;
};
extern sResCameraProperties resCameraProperties;

s32 setSomethingInNpc0(s32 arg0, s32 arg1);
void startScriptTask(p_workArea r4);
sNPC* getNpcDataByIndex(s32 r4);
