#include "PDS.h"

namespace MENU_EN_OVERLAY {

struct s_menuEnWorkArea : public s_workAreaTemplate<s_menuEnWorkArea>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &s_menuEnWorkArea::Init, &s_menuEnWorkArea::Update, &s_menuEnWorkArea::Draw, &s_menuEnWorkArea::Delete, "s_menuEnWorkArea" };
        return &taskDefinition;
    }

    void Init();
    void Update() override;
    void Draw() override;
    void Delete() override;
};

void s_menuEnWorkArea::Init()
{
    assert(0);
}

void s_menuEnWorkArea::Update()
{
    assert(0);
}


void s_menuEnWorkArea::Draw()
{
    assert(0);
}

void s_menuEnWorkArea::Delete()
{
    assert(0);
}

p_workArea overlayStart(p_workArea workArea)
{
    return createSubTask< s_menuEnWorkArea>(workArea);
}

};
