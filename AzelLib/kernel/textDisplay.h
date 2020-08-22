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
    s_vdp2StringTask** m10_pParentTask;
    s16 m14_x;
    s16 m16_y;
    s16 m1A_width;
    s16 m1C_height;
    sSaturnPtr m24_string;
    s16 m28;
    s16 m2A;
    // size 2C
};

void createDisplayFormationNameText(p_workArea parentTask, s_vdp2StringTask** outputTask, s16 param3, sSaturnPtr stringEA, s16 param5, s16 param6);
