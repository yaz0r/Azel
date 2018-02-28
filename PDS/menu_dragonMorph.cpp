#include "PDS.h"

struct s_dragonMenuDragonWorkAreaSub1
{
    s_graphicEngineStatus_405C field_120;
};

struct s_dragonMenuDragonWorkArea : public s_workArea
{
    s_dragonMenuDragonWorkAreaSub1 field_34;
};

void dragonMenuDragonInitSub1(s_dragonMenuDragonWorkAreaSub1* r4)
{
    memcpy_dma(&graphicEngineStatus.field_405C, &r4->field_120, sizeof(s_dragonMenuDragonWorkAreaSub1));

}

void dragonMenuDragonInit(p_workArea pTypelessWorkArea)
{
    s_dragonMenuDragonWorkArea* pWorkArea = static_cast<s_dragonMenuDragonWorkArea*>(pTypelessWorkArea);

    dragonMenuDragonInitSub1(&pWorkArea->field_34);

    assert(0);
}

void dragonMenuDragonUpdate(p_workArea pTypelessWorkArea)
{
    unimplemented("dragonMenuDragonUpdate");
}

void dragonMenuDragonInitDraw(p_workArea pTypelessWorkArea)
{
    unimplemented("dragonMenuDragonInitDraw");
}

void dragonMenuDragonDelete(p_workArea pTypelessWorkArea)
{
    unimplemented("dragonMenuDragonDelete");
}

s_taskDefinition dragonMenuDragonTaskDefinition = { dragonMenuDragonInit, dragonMenuDragonUpdate, dragonMenuDragonInitDraw, dragonMenuDragonDelete, "dragonMenuDragon" };

p_workArea createDragonMenuMorhTask(p_workArea pWorkArea)
{
    return createSubTask(pWorkArea, &dragonMenuDragonTaskDefinition, new s_dragonMenuDragonWorkArea);
}
