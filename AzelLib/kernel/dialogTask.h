#pragma once

// Todo: might be the same as s_multiChoiceTask2
struct s_multiChoiceTask : public s_workAreaTemplate< s_multiChoiceTask>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { NULL, &s_multiChoiceTask::Update, &s_multiChoiceTask::Draw, &s_multiChoiceTask::Delete };
        return &taskDefinition;
    }

    static void Update(s_multiChoiceTask*);
    static void Draw(s_multiChoiceTask*);
    static void Delete(s_multiChoiceTask* pThis)
    {
        pThis->clearMultiChoiceBox();

        if (pThis->m10)
        {
            *pThis->m10 = nullptr;
        }
    }

    void drawMultiChoice();
    void clearMultiChoiceBox()
    {
        FunctionUnimplemented();
    }

    u8 m0_Status;
    s8 m1;
    s8 m2_defaultResult;
    s8 m3_quantity;
    s8 m4_quantityMax;
    s8 m5_selectedEntry;
    s8 m6_numEntries;
    s8 m7;
    s8 m8;
    s32* mC_result;
    s_multiChoiceTask** m10;
    s16 m14_x;
    s16 m16_y;
    s16 m1A_width;
    s16 m1C_height;
    s8 m20;
    sSaturnPtr m24_strings;
    s16* m28_colors;
    //size 0x2C
};

void drawMultiChoiceVdp1Cursor(s32 r4_x, s32 r5_y, sSaturnPtr r6_spritePtr, s32 r7_color);
void startDialogTask(p_workArea r4_parent, s_multiChoiceTask** r5_outputTask, s32* r6_outputResult, s32 r7_index, sSaturnPtr arg0);
s_multiChoiceTask* updateMultiChoice(p_workArea parentTask, s_multiChoiceTask** r5, s32* r6_currentChoice, s32 r7_minusCurrentChoice, sSaturnPtr scriptPtr, s16* choiceTable, s32 moreCurrentChoice);
