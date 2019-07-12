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
    sSaturnPtr mC_AsOffset;
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
    s32 m8_LCSWidth;
    fixedPoint mC_LCSHeight;
    s32 m10;
    // part bellow should technically mirror s_graphicEngineStatus_405C for projection during the LCS test in Town module
    s32 m14;
    fixedPoint m18;
    s16 m1C_LCSHeightMin;
    s16 m1E_LCSHeightMax;
    s16 m20_LCSWidthMin;
    s16 m22_LCSWidthMax;
    fixedPoint m24;
    fixedPoint m28_LCSDepth;
    fixedPoint m2C;
    fixedPoint m30;
    fixedPoint m34;
    fixedPoint m38;
    fixedPoint m3C;
    fixedPoint m40;
    fixedPoint m44;
    fixedPoint m48;
    fixedPoint m4C;
    fixedPoint m50;
    fixedPoint m54;
    fixedPoint m58;
};
extern sResCameraProperties resCameraProperties;

s32 setSomethingInNpc0(s32 arg0, s32 arg1);
void startScriptTask(p_workArea r4);
sNPC* getNpcDataByIndex(s32 r4);
