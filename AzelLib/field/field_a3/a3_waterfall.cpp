#include "PDS.h"
#include "o_fld_a3.h"
#include "a3_waterfall.h"
#include "kernel/fileBundle.h"
#include "items.h"
#include "3dModels.h"

// ============================================================
// Waterfall type 0 (task def at 06091db4, size 0x10)
// Small waterfall with 10-frame animation, updates nearest sound position (sound 0x6B)
// ============================================================

static const s16 waterfallModelOffsets0[] = {
    0x0390, 0x0394, 0x0398, 0x039C, 0x03A0, 0x03A4, 0x03A8, 0x03AC, 0x03B0, 0x03B4
}; // 06091cd0

struct s_A3_Waterfall0 : public s_workAreaTemplate<s_A3_Waterfall0>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { NULL, &Update, &Draw, NULL };
        return &taskDefinition;
    }

    // 0605bfa0
    static void Update(s_A3_Waterfall0* pThis)
    {
        s32 visible = checkPositionVisibilityAgainstFarPlane(&pThis->m8->m4_position);
        pThis->mD_visible = (visible == 0);
        if (visible != 0)
            return;

        if (pThis->mC_frameCounter < 0x27)
            pThis->mC_frameCounter++;
        else
            pThis->mC_frameCounter = 0;

        Unimplemented(); // FUN_0605bc28, FUN_0605bd8a — waterfall particle spawning
        // FUN_0605b9e8 — update nearest waterfall sound 0x6B position
        s_fieldSpecificData_A3* pFieldData = getFieldSpecificData_A3();
        sVec3_FP* waterfallPos = &pThis->m8->m4_position;
        s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;

        fixedPoint dxCur = pDragon->m8_pos.m0_X - pFieldData->m148_sound6BPos.m0_X;
        if ((s32)dxCur < 0) dxCur = -dxCur;
        fixedPoint dzCur = pDragon->m8_pos.m8_Z - pFieldData->m148_sound6BPos.m8_Z;
        if ((s32)dzCur < 0) dzCur = -dzCur;

        fixedPoint dxNew = pDragon->m8_pos.m0_X - waterfallPos->m0_X;
        if ((s32)dxNew < 0) dxNew = -dxNew;
        fixedPoint dzNew = pDragon->m8_pos.m8_Z - waterfallPos->m8_Z;
        if ((s32)dzNew < 0) dzNew = -dzNew;

        if ((s32)(dxNew + dzNew) < (s32)(dxCur + dzCur))
        {
            pFieldData->m148_sound6BPos = *waterfallPos;
        }
        pFieldData->m138 = 1;
    }

    // 0605bf26
    static void Draw(s_A3_Waterfall0* pThis)
    {
        if (!pThis->mD_visible)
            return;

        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->m8->m4_position);
        addObjectToDrawList(pThis->m0.m0_mainMemoryBundle->get3DModel(waterfallModelOffsets0[pThis->mC_frameCounter >> 2]));
        popMatrix();
    }

    s_memoryAreaOutput m0; // 0
    s_DataTable2Sub0* m8;  // 8
    u8 mC_frameCounter;    // C
    u8 mD_visible;         // D
    // size 0x10
};

// 0605bfea
void create_A3_Waterfall0(s_visdibilityCellTask* r4, s_DataTable2Sub0& r5, s32 r6)
{
    s_A3_Waterfall0* pTask = createSubTask<s_A3_Waterfall0>(r4);
    if (pTask)
    {
        getMemoryArea(&pTask->m0, r6);
        pTask->m8 = &r5;
    }
}

// ============================================================
// Waterfall type 1 (task def at 06091dc4, size 0x10)
// Small waterfall variant with random particle spawning, updates nearest sound position (sound 0x6B)
// ============================================================

static const s16 waterfallModelOffsets1[] = {
    0x03B8, 0x03BC, 0x03C0, 0x03C4, 0x03C8, 0x03CC, 0x03D0, 0x03D4, 0x03D8, 0x03DC
}; // 06091ce4

struct s_A3_Waterfall1 : public s_workAreaTemplate<s_A3_Waterfall1>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { NULL, &Update, &Draw, NULL };
        return &taskDefinition;
    }

    // 0605c058
    static void Update(s_A3_Waterfall1* pThis)
    {
        s32 visible = checkPositionVisibilityAgainstFarPlane(&pThis->m8->m4_position);
        pThis->mD_visible = (visible == 0);
        if (visible != 0)
            return;

        if (pThis->mC_frameCounter < 0x27)
            pThis->mC_frameCounter++;
        else
            pThis->mC_frameCounter = 0;

        Unimplemented(); // randomNumber-based particle spawning (FUN_0605be38 / FUN_0605bcc4)
        // FUN_0605b9e8 — update nearest waterfall sound 0x6B position
        s_fieldSpecificData_A3* pFieldData = getFieldSpecificData_A3();
        sVec3_FP* waterfallPos = &pThis->m8->m4_position;
        s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;

        fixedPoint dxCur = pDragon->m8_pos.m0_X - pFieldData->m148_sound6BPos.m0_X;
        if ((s32)dxCur < 0) dxCur = -dxCur;
        fixedPoint dzCur = pDragon->m8_pos.m8_Z - pFieldData->m148_sound6BPos.m8_Z;
        if ((s32)dzCur < 0) dzCur = -dzCur;

        fixedPoint dxNew = pDragon->m8_pos.m0_X - waterfallPos->m0_X;
        if ((s32)dxNew < 0) dxNew = -dxNew;
        fixedPoint dzNew = pDragon->m8_pos.m8_Z - waterfallPos->m8_Z;
        if ((s32)dzNew < 0) dzNew = -dzNew;

        if ((s32)(dxNew + dzNew) < (s32)(dxCur + dzCur))
        {
            pFieldData->m148_sound6BPos = *waterfallPos;
        }
        pFieldData->m138 = 1;
    }

    // 0605c018
    static void Draw(s_A3_Waterfall1* pThis)
    {
        if (!pThis->mD_visible)
            return;

        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->m8->m4_position);
        addObjectToDrawList(pThis->m0.m0_mainMemoryBundle->get3DModel(waterfallModelOffsets1[pThis->mC_frameCounter >> 2]));
        popMatrix();
    }

    s_memoryAreaOutput m0; // 0
    s_DataTable2Sub0* m8;  // 8
    u8 mC_frameCounter;    // C
    u8 mD_visible;         // D
    // size 0x10
};

// 0605c0ae
void create_A3_Waterfall1(s_visdibilityCellTask* r4, s_DataTable2Sub0& r5, s32 r6)
{
    s_A3_Waterfall1* pTask = createSubTask<s_A3_Waterfall1>(r4);
    if (pTask)
    {
        getMemoryArea(&pTask->m0, r6);
        pTask->m8 = &r5;
    }
}

// ============================================================
// Waterfall type 2 (task def at 06091dd4, size 0x14)
// Large waterfall with rotation, particle spawning, updates nearest sound position (sound 0x6A)
// ============================================================

static const s16 waterfallModelOffsets2[] = {
    0x03E0, 0x03E4, 0x03E8, 0x03EC, 0x03F0, 0x03F4, 0x03F8, 0x03FC, 0x0400, 0x0404
}; // 06091cf8

static const s16 waterfallModelOffsets3[] = {
    0x0408, 0x040C, 0x0410, 0x0414, 0x0418, 0x041C, 0x0420, 0x0424, 0x0428, 0x042C
}; // 06091d0c

static const s16 waterfallModelOffsets4[] = {
    0x0430, 0x0434, 0x0438, 0x043C, 0x0440, 0x0444, 0x0448, 0x044C, 0x0450, 0x0454
}; // 06091d20

struct s_A3_WaterfallLarge : public s_workAreaTemplate<s_A3_WaterfallLarge>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { NULL, &Update, &Draw, NULL };
        return &taskDefinition;
    }

    // 0605c12a
    static void Update(s_A3_WaterfallLarge* pThis)
    {
        s32 visible = checkPositionVisibilityAgainstFarPlane(&pThis->m8->m4_position);
        pThis->m11_visible = (visible == 0);
        if (visible != 0)
            return;

        if (pThis->m10_frameCounter < 0x27)
            pThis->m10_frameCounter++;
        else
            pThis->m10_frameCounter = 0;

        if ((pThis->m10_frameCounter & 7) == 0)
        {
            Unimplemented(); // func_0x0605bd2e — periodic particle spawning
        }

        u32 rnd = randomNumber();
        if ((rnd & 3) == 0)
        {
            Unimplemented(); // FUN_0605beaa — random splash particle with 0x40000 param
        }

        // Update nearest waterfall sound 0x6A position
        s_fieldSpecificData_A3* pFieldData = getFieldSpecificData_A3();
        sVec3_FP* waterfallPos = &pThis->m8->m4_position;
        s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;

        fixedPoint dxCur = pDragon->m8_pos.m0_X - pFieldData->m13C_sound6APos.m0_X;
        if ((s32)dxCur < 0) dxCur = -dxCur;
        fixedPoint dzCur = pDragon->m8_pos.m8_Z - pFieldData->m13C_sound6APos.m8_Z;
        if ((s32)dzCur < 0) dzCur = -dzCur;

        fixedPoint dxNew = pDragon->m8_pos.m0_X - waterfallPos->m0_X;
        if ((s32)dxNew < 0) dxNew = -dxNew;
        fixedPoint dzNew = pDragon->m8_pos.m8_Z - waterfallPos->m8_Z;
        if ((s32)dzNew < 0) dzNew = -dzNew;

        if ((s32)(dxNew + dzNew) < (s32)(dxCur + dzCur))
        {
            pFieldData->m13C_sound6APos = *waterfallPos;
        }
        pFieldData->m134 = 1;
    }

    // 0605c0dc
    static void Draw(s_A3_WaterfallLarge* pThis)
    {
        if (!pThis->m11_visible)
            return;

        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->m8->m4_position);
        rotateCurrentMatrixY(pThis->m8->m10_rotation[1]);
        addObjectToDrawList(pThis->m0.m0_mainMemoryBundle->get3DModel(pThis->mC_modelOffsets[pThis->m10_frameCounter >> 2]));
        popMatrix();
    }

    s_memoryAreaOutput m0;    // 0
    s_DataTable2Sub0* m8;     // 8
    const s16* mC_modelOffsets; // C
    u8 m10_frameCounter;      // 10
    u8 m11_visible;           // 11
    // size 0x14
};

// 0605c274
void create_A3_Waterfall2(s_visdibilityCellTask* r4, s_DataTable2Sub0& r5, s32 r6)
{
    s_A3_WaterfallLarge* pTask = createSubTask<s_A3_WaterfallLarge>(r4);
    if (pTask)
    {
        getMemoryArea(&pTask->m0, r6);
        pTask->m8 = &r5;
        pTask->mC_modelOffsets = waterfallModelOffsets2;
        pTask->m10_frameCounter = performModulo(0x28, r5.m18 * 5);
    }
}

// 0605c27a
void create_A3_Waterfall3(s_visdibilityCellTask* r4, s_DataTable2Sub0& r5, s32 r6)
{
    s_A3_WaterfallLarge* pTask = createSubTask<s_A3_WaterfallLarge>(r4);
    if (pTask)
    {
        getMemoryArea(&pTask->m0, r6);
        pTask->m8 = &r5;
        pTask->mC_modelOffsets = waterfallModelOffsets3;
        pTask->m10_frameCounter = performModulo(0x28, r5.m18 * 5);
    }
}

// 0605c280
void create_A3_Waterfall4(s_visdibilityCellTask* r4, s_DataTable2Sub0& r5, s32 r6)
{
    s_A3_WaterfallLarge* pTask = createSubTask<s_A3_WaterfallLarge>(r4);
    if (pTask)
    {
        getMemoryArea(&pTask->m0, r6);
        pTask->m8 = &r5;
        pTask->mC_modelOffsets = waterfallModelOffsets4;
        pTask->m10_frameCounter = performModulo(0x28, r5.m18 * 5);
    }
}

// ============================================================
// Obj1 (task def at 06092dac, size 0x60)
// Interactive/shootable environmental object with LCS target
// ============================================================

// 0606039c
static void A3_Obj1_LCSCallback(p_workArea pWorkArea, sLCSTarget* pTarget)
{
    Unimplemented();
}

struct s_A3_Obj1 : public s_workAreaTemplate<s_A3_Obj1>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { NULL, &Update, &Draw, NULL };
        return &taskDefinition;
    }

    // 06060304
    static void Update(s_A3_Obj1* pThis)
    {
        pThis->m24_angle += 0x222222;
        pThis->m1C_rotation[0] = (s16)(getSin((pThis->m24_angle >> 16) & 0xFFF) >> 9);
        pThis->m10_position.m4_Y = fixedPoint(*pThis->mC + (s32)pThis->m8->m4_position.m4_Y);
        updateLCSTarget(&pThis->m28_lcsTarget);
        s32 vis = checkPositionVisibilityAgainstFarPlane(&pThis->m10_position);
        pThis->m5C_visible = (vis == 0);
    }

    // 0606025e
    static void Draw(s_A3_Obj1* pThis)
    {
        if (!pThis->m5C_visible)
            return;

        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->m10_position);
        rotateCurrentMatrixZYX_s16(pThis->m1C_rotation);
        s16 modelHierarchyOffset = readSaturnS16(pThis->m8->m1C_modelData);
        s16 modelPoseOffset = readSaturnS16(pThis->m8->m1C_modelData + 2);
        LCSItemBox_DrawType0Sub0(pThis->m0.m0_mainMemoryBundle, modelHierarchyOffset, modelPoseOffset);
        callGridCellDraw_normalSub2(pThis->m0.m0_mainMemoryBundle, 0x238);
        popMatrix();
    }

    s_memoryAreaOutput m0;     // 0
    s_DataTable2Sub0* m8;      // 8
    u32* mC;                   // C — pointer into fieldSpecificData_A3.m50[]
    sVec3_FP m10_position;     // 10
    sVec3_S16_12_4 m1C_rotation; // 1C
    s32 m24_angle;             // 24
    sLCSTarget m28_lcsTarget;  // 28
    u8 m5C_visible;            // 5C
    // size 0x60
};

// 0606053e
void create_A3_Obj1(s_visdibilityCellTask* r4, s_DataTable2Sub0& r5, s32 r6)
{
    s_A3_Obj1* pTask = createSubTask<s_A3_Obj1>(r4);
    if (pTask)
    {
        getMemoryArea(&pTask->m0, r6);
        pTask->m8 = &r5;
        pTask->m10_position = r5.m4_position;
        pTask->m1C_rotation = r5.m10_rotation;

        s8 index = readSaturnS8(gFLD_A3->getSaturnPtr(0x0609292c) + r5.m18);
        pTask->mC = &getFieldSpecificData_A3()->m50[index];

        pTask->m24_angle = randomNumber();

        createLCSTarget(&pTask->m28_lcsTarget, pTask, &A3_Obj1_LCSCallback,
                        &pTask->m10_position, nullptr, 0, 0, eItems::mMinusOne, 0, 0);
    }
}
