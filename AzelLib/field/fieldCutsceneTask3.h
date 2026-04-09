#pragma once

struct s_cutsceneTask3 : public s_workAreaTemplate<s_cutsceneTask3>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &s_cutsceneTask3::Init, &s_cutsceneTask3::Update, &s_cutsceneTask3::Draw, NULL };
        return &taskDefinition;
    }
    static void Init(s_cutsceneTask3*);
    s32 UpdateSub0();
    void UpdateSub1();
    static void Update(s_cutsceneTask3*);
    static void Draw(s_cutsceneTask3*);

    s32 m0;
    std::vector<s_scriptData2>* m4;
    sVec3_FP m8;
    sVec3_FP* m14;
    sVec3_FP* m18;
    sVec3_FP m1C;
    s32 m28;
    s32 m2C;
    s32 m30;
    s_scriptData2* m34;
    s32 m38;
    sVec3_FP m3C;
    fixedPoint m48;
    s32 m4C;
    //size = 0x50
};

void cutsceneTaskInitSub3(p_workArea r4, std::vector<s_scriptData2>& r11, s32 r6, sVec3_FP* r7, u32 arg0);
