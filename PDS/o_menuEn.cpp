#include "PDS.h"

namespace MENU_EN_OVERLAY {

struct s_menuEnWorkArea : public s_workArea
{
};

void MenuEnTaskInit(s_workArea*)
{
    assert(0);
}

void MenuEnTaskUpdate(s_workArea*)
{
    assert(0);
}


void MenuEnTaskDraw(s_workArea*)
{
    assert(0);
}

void MenuEnTaskDelete(s_workArea*)
{
    assert(0);
}

s_taskDefinition MenuEnTaskDefinition = { MenuEnTaskInit, MenuEnTaskUpdate, MenuEnTaskDraw, MenuEnTaskDelete, "MenuEnTask" };

p_workArea overlayStart(p_workArea workArea)
{
    return createSubTask(workArea, &MenuEnTaskDefinition, new s_menuEnWorkArea);
}

};
