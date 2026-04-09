#include "PDS.h"
#include "o_fld_a7.h"
#include "a7_cellObj3.h"
#include "field/field_a3/o_fld_a3.h"
#include "kernel/fileBundle.h"
#include "3dModels.h"

// 06055230 — check if game state bit is set for object at given index
static bool isObjectDestroyedA7(s32 index)
{
    s16 bitIndex = readSaturnS16(gFLD_A7->getSaturnPtr(0x060843d0) + index * 2);
    u32 adjustedIndex = (bitIndex < 1000) ? (u32)bitIndex : (u32)(bitIndex - 0x236);
    return (mainGameState.bitField[adjustedIndex >> 3] & bitMasks[adjustedIndex & 7]) != 0;
}

struct s_A7_CellObj3 : public s_workAreaTemplate<s_A7_CellObj3>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { NULL, &Update, &Draw, NULL };
        return &taskDefinition;
    }

    // 06055f92 / 06055f70
    static void Update(s_A7_CellObj3* pThis)
    {
        if (!pThis->mF_destroyed)
        {
            // Check if this object has been destroyed via game state flag
            if (isObjectDestroyedA7(pThis->m8->m18))
            {
                pThis->mF_destroyed = 1;
            }
        }

        if (pThis->mF_destroyed)
        {
            // 06055f70 — spinning behavior when destroyed
            pThis->mC_rotation += 0x1F;
        }

        // 0606e2f4 — visibility check against far clip
        s32 vis = checkPositionVisibilityAgainstFarPlane(&pThis->m8->m4_position);
        pThis->mE_visible = (vis == 0);
    }

    // 06055ef4 / 06055eba
    static void Draw(s_A7_CellObj3* pThis)
    {
        if (!pThis->mE_visible)
            return;

        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->m8->m4_position);

        if (pThis->mF_destroyed)
        {
            // 06055eba — draw with Y rotation when destroyed
            rotateCurrentMatrixY(pThis->mC_rotation);
        }

        // Draw model hierarchy (offset 0x14) with static pose (offset 0x2D4)
        LCSItemBox_DrawType0Sub0(pThis->m0.m0_mainMemoryBundle, 0x14, 0x2D4);
        popMatrix();
    }

    s_memoryAreaOutput m0; // 0
    s_DataTable2Sub0* m8;  // 8
    s16 mC_rotation;       // C
    u8 mE_visible;         // E
    u8 mF_destroyed;       // F
    // size 0x10
};

// 06055fc6
void create_A7_CellObj3(s_visdibilityCellTask* r4, s_DataTable2Sub0& r5, s32 r6)
{
    if ((r5.m18 & 1) != 0)
        return;

    s_A7_CellObj3* pTask = createSubTask<s_A7_CellObj3>(r4);
    if (pTask)
    {
        getMemoryArea(&pTask->m0, r6);
        pTask->m8 = &r5;
        pTask->mC_rotation = 0;
        pTask->mE_visible = 0;
        pTask->mF_destroyed = 0;
    }
}
