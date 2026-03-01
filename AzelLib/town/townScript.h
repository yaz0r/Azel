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

    sSaturnPtr m4;
    s32 m8_currentLCSType;
    sCollisionBody* mC;
    s32 mC_AsIndex;
    fixedPoint m10_distanceToLCS;
    sVec2_S16 m14_LCS;
    s32 m18_LCSFocusLineScale;
    s16 m1C_LCS_X;
    s16 m1E_LCS_Y;
    //size 0x20
};
extern sScriptTask* currentResTask;

s32 setNpcLocation(s32 r4_npcIndex, s32 r5_X, s32 r6_Y, s32 r7_Z);
s32 setNpcOrientation(s32 r4_npcIndex, s32 r5_X, s32 r6_Y, s32 r7_Z);

s32 setSomethingInNpc0(s32 arg0, s32 arg1);
void startScriptTask(p_workArea r4);
sNPC* getNpcDataByIndex(s32 r4);
p_workArea getNpcDataByIndexAsTask(s32 r4);

void addBackgroundScript(sSaturnPtr r4, s32 r5, p_workArea r6, const sVec3_S16_12_4* r7);

