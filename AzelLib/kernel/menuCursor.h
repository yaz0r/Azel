#pragma once

struct sMainMenuTaskInitData2
{
    s_graphicEngineStatus_40BC* m0;
    u16* m4;
};

struct s_MenuCursorWorkArea : public s_workAreaTemplateWithArg<s_MenuCursorWorkArea, sMainMenuTaskInitData2*>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &s_MenuCursorWorkArea::menuCursorTaskInit , &s_MenuCursorWorkArea::menuCursorTaskUpdate, &s_MenuCursorWorkArea::menuCursorTaskDraw, NULL };
        return &taskDefinition;
    }

    static void menuCursorTaskInit(s_MenuCursorWorkArea*, sMainMenuTaskInitData2*);
    static void menuCursorTaskUpdate(s_MenuCursorWorkArea*);
    static void menuCursorTaskDraw(s_MenuCursorWorkArea*);

    s32 m0_selectedEntry;
    s_graphicEngineStatus_40BC* m4;
    u16* m8;
    s32 mC;

    // size 0x10
};

// that's the spinning cursor arrow
struct s_MenuCursor2 : public s_workAreaTemplateWithArg<s_MenuCursor2, std::vector<std::array<s16, 2>> *>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &s_MenuCursor2::Init , nullptr, &s_MenuCursor2::Draw, nullptr };
        return &taskDefinition;
    }

    static void Init(s_MenuCursor2*, std::vector<std::array<s16, 2>> *);
    static void Draw(s_MenuCursor2*);

    s32 m0;
    std::vector<std::array<s16,2>>* m4;
    s32 m8;
    s32 mC;
    // size 0x10
};

s_MenuCursorWorkArea* createMenuCursorTask(p_workArea pWorkArea, sMainMenuTaskInitData2* r5);

s_MenuCursor2* createMenuCursorTask2(p_workArea pWorkArea, std::vector<std::array<s16, 2>> * r5);

