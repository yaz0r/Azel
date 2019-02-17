#include "PDS.h"

namespace MENU_EN_OVERLAY {

struct s_menuEnWorkArea : public s_workAreaTemplate<s_menuEnWorkArea>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &s_menuEnWorkArea::Init, &s_menuEnWorkArea::Update, &s_menuEnWorkArea::Draw, &s_menuEnWorkArea::Delete };
        return &taskDefinition;
    }

    static void Init(s_menuEnWorkArea*);
    static void Update(s_menuEnWorkArea*);
    static void Draw(s_menuEnWorkArea*);
    static void Delete(s_menuEnWorkArea*);
};

void s_menuEnWorkArea::Init(s_menuEnWorkArea*)
{
    assert(0);
}

void s_menuEnWorkArea::Update(s_menuEnWorkArea*)
{
    assert(0);
}


void s_menuEnWorkArea::Draw(s_menuEnWorkArea*)
{
    assert(0);
}

void s_menuEnWorkArea::Delete(s_menuEnWorkArea*)
{
    assert(0);
}

p_workArea overlayStart(p_workArea workArea)
{
    return createSubTask< s_menuEnWorkArea>(workArea);
}

};
