#include "PDS.h"
#include "a3_crane_lower.h"
#include "kernel/fileBundle.h"
#include "field/field_a3/o_fld_a3.h" //TODO: cleanup

// crane lower part
struct s_A3_0_Obj2 : public s_workAreaTemplate<s_A3_0_Obj2>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { NULL, NULL, &s_A3_0_Obj2::Draw, NULL };
        return &taskDefinition;
    }

    static void Draw(s_A3_0_Obj2* pThis)
    {
        if (!checkPositionVisibilityAgainstFarPlane(&pThis->mC_position))
        {
            pushCurrentMatrix();
            translateCurrentMatrix(&pThis->mC_position);
            rotateCurrentMatrixY(pThis->m18);
            addObjectToDrawList(pThis->m0.m0_mainMemoryBundle->get3DModel(0x2D0));
            popMatrix();
        }
    }

    s_memoryAreaOutput m0;
    sVec3_FP mC_position;
    s16 m18;
    // size 0x1C
};

void create_A3_0_Obj2(s_visdibilityCellTask* r4, s_DataTable2Sub0& r5, s32 r6)
{
    s_A3_0_Obj2* pNewTask = createSubTask<s_A3_0_Obj2>(r4);
    getMemoryArea(&pNewTask->m0, r6);

    if (r5.m10_rotation[0])
    {
        pNewTask->m18 = 0x800 - r5.m10_rotation[1];
    }
    else
    {
        pNewTask->m18 = r5.m10_rotation[1];
    }

    pNewTask->mC_position = r5.m4_position;
}
