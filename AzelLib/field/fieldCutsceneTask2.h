#pragma once

struct s_cutsceneTask2 : public s_workAreaTemplateWithArg<s_cutsceneTask2, std::vector<s_scriptData1>*>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &s_cutsceneTask2::Init, &s_cutsceneTask2::Update, &s_cutsceneTask2::Draw, NULL};
        return &taskDefinition;
    }
    static void Init(s_cutsceneTask2*, std::vector<s_scriptData1>* argument);
    static void Update(s_cutsceneTask2*);
    static void Draw(s_cutsceneTask2*);

    s32 UpdateSub0();
    void UpdateSub1();

    u32 m0;
    std::vector<s_scriptData1>* m4;
    sVec3_FP m8;
    sVec3_FP m14;
    s32 m20;
    sVec3_FP* m24;
    sVec3_FP m28;
    fixedPoint m34;
    s32 m38;
    s_scriptData1* m3C;
    s32 m40;
    sVec3_FP m44;
    sVec3_FP m50;
    fixedPoint m5C;
    s32 m60;
    //size = 0x64
};

void cutsceneTaskInitSub2(p_workArea r4, std::vector<s_scriptData1>& r11, s32 r6, sVec3_FP* r7, u32 arg0);
