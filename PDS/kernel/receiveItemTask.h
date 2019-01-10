#pragma once

struct s_receivedItemTask : s_workAreaTemplate<s_receivedItemTask>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { nullptr, &s_receivedItemTask::Update, nullptr, &s_receivedItemTask::Delete};
        return &taskDefinition;
    }

    static void Update(s_receivedItemTask*);
    static void Delete(s_receivedItemTask*);

    s8 m0;
    s8 m2;
    s16 mA;
    s_receivedItemTask** m10;
    s16 m14_x;
    s16 m16_y;
    s16 m1A_width;
    s16 m1C_height;
    s16 m24_receivedItemId;
    s16 m26_receivedItemQuanity;
    s16 m28_itemNameLength;
    // size 0x2C
};

s_receivedItemTask* createReceiveItemTask(p_workArea r4_parentTask, s_receivedItemTask** r5, s32 r6, s32 r7_receivedItemId, s32 arg0_receivedItemQuanity);
