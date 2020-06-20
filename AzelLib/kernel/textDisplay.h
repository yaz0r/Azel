#pragma once

struct s_vdp2StringTask : public s_workAreaTemplate<s_vdp2StringTask>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { NULL, &s_vdp2StringTask::Update, NULL, &s_vdp2StringTask::Delete };
        return &taskDefinition;
    }

    static void Update(s_vdp2StringTask*);
    static void Delete(s_vdp2StringTask*);

    void UpdateSub1();

    // variables
    u8 m0_status;
    s8 m2_durationMode;
    s16 mA_duration;
    s_vdp2StringTask** m10;
    s16 m14_x;
    s16 m16_y;
    s16 m1A_width;
    s16 m1C_height;
    sSaturnPtr m24_string;
    s16 m28;
    s16 m2A;
    // size 2C
};

#if 0
struct s_multiChoiceTask2 : public s_workAreaTemplate< s_multiChoiceTask2>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { NULL, &s_multiChoiceTask2::Update, &s_multiChoiceTask2::Draw, &s_multiChoiceTask2::Delete };
        return &taskDefinition;
    }

    static void Update(s_multiChoiceTask2*);
    static void Draw(s_multiChoiceTask2*);
    static void Delete(s_multiChoiceTask2*)
    {
        assert(0);
    }

    void drawMultiChoice();

    u8 m0_Status;
    s8 m1;
    s8 m2_defaultResult;
    s8 m3;
    s8 m4;
    s8 m5_selectedEntry;
    s8 m6_numEntries;
    s8 m7;
    s8 m8;
    s32* mC_result;
    p_workArea m10;
    s16 m14_x;
    s16 m16_y;
    s16 m1A_width;
    s16 m1C_height;
    sSaturnPtr m24_strings;
    s16* m28_colors;
    //size 0x2C
};
#endif

void createDisplayFormationNameText(p_workArea parentTask, s_vdp2StringTask** outputTask, s16 param3, sSaturnPtr stringEA, s16 param5, s16 param6);
