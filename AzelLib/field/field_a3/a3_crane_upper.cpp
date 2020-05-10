#include "PDS.h"
#include "a3_crane_upper.h"
#include "kernel/fileBundle.h"
#include "field/field_a3/o_fld_a3.h" //TODO: cleanup

// crane upper part
struct s_A3_0_Obj1 : public s_workAreaTemplate<s_A3_0_Obj1>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { NULL, &s_A3_0_Obj1::Update, &s_A3_0_Obj1::Draw, NULL };
        return &taskDefinition;
    }

    static void Update(s_A3_0_Obj1* pThis)
    {
        pThis->m1E_isVisible = !checkPositionVisibilityAgainstFarPlane(&pThis->mC_position);
        if (pThis->m1E_isVisible)
        {
            pThis->m1A = (getSin(pThis->m1C & 0xFFF) >> 7) + pThis->m18;
            pThis->m1C += 0xF;
        }
    }

    static void Update2(s_A3_0_Obj1* pThis) // used for cranes that don't rotate
    {
        pThis->m1E_isVisible = !checkPositionVisibilityAgainstFarPlane(&pThis->mC_position);
    }

    static void Draw(s_A3_0_Obj1* pThis)
    {
        if (pThis->m1E_isVisible)
        {
            pushCurrentMatrix();
            translateCurrentMatrix(&pThis->mC_position);
            rotateCurrentMatrixY(pThis->m1A);
            addObjectToDrawList(pThis->m0.m0_mainMemoryBundle->get3DModel(0x2CC));
            popMatrix();
        }
    }

    s_memoryAreaOutput m0;
    sVec3_FP mC_position;
    s16 m18;
    s16 m1A;
    s16 m1C;
    s8 m1E_isVisible;
    // size 0x20
};

void create_A3_0_Obj1(s_visdibilityCellTask* r4, s_DataTable2Sub0& r5, s32 r6)
{
    s_A3_0_Obj1* pNewTask = createSubTask<s_A3_0_Obj1>(r4);
    getMemoryArea(&pNewTask->m0, r6);

    if (r5.m10_rotation[0])
    {
        pNewTask->m1A = 0x800 - r5.m10_rotation[1];
    }
    else
    {
        pNewTask->m1A = r5.m10_rotation[1];
    }

    pNewTask->m18 = pNewTask->m1A;
    pNewTask->mC_position = r5.m4_position;
    pNewTask->m1C = 0;

    if ((r5.m18 == 2) || (r5.m18 == 3) || (r5.m18 == 5))
    {
        pNewTask->m_UpdateMethod = &s_A3_0_Obj1::Update2;
    }
}
