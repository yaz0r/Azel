#include "PDS.h"
#include "a3_flag.h"
#include "kernel/fileBundle.h"
#include "field/field_a3/o_fld_a3.h" //TODO: cleanup
#include "field/fieldVisibilityGrid.h"

struct s_A3_FlagPole : public s_workAreaTemplate<s_A3_FlagPole>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { NULL, &s_A3_FlagPole::Update, &s_A3_FlagPole::Draw, NULL };
        return &taskDefinition;
    }

    // 0605a898
    static void Update(s_A3_FlagPole* pThis)
    {
        pThis->mC_swingPhase += 0xB60B6;
        fixedPoint var1C = getFieldSpecificData_A3()->mA4[pThis->m8_pData->m18] = MTH_Mul(fixedPoint(0x71C71C), getSin((pThis->mC_swingPhase >> 16) & 0xFFF)) - fixedPoint(0x71C71C);

        sVec3_FP& r12 = getFieldSpecificData_A3()->mC0[pThis->m8_pData->m18];
        r12[0] = pThis->m8_pData->m4_position[0] - MTH_Mul(fixedPoint(0xE333), getSin((var1C >> 16) & 0xFFF));
        r12[1] = pThis->m8_pData->m4_position[1] + MTH_Mul(fixedPoint(0xE333), getCos((var1C >> 16) & 0xFFF));

        pThis->m10_isVisible = (checkPositionVisibilityAgainstFarPlane(&r12) == 0);
    }

    // 0605a804
    static void Draw(s_A3_FlagPole* pThis)
    {
        if (pThis->m10_isVisible == 0)
        {
            return;
        }

        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->m8_pData->m4_position);
        rotateCurrentMatrixShiftedZ(getFieldSpecificData_A3()->mA4[pThis->m8_pData->m18]);

        addObjectToDrawList(pThis->m0_memoryArea.m0_mainMemoryBundle->get3DModel(0x29C));

        popMatrix();
    }

    s_memoryAreaOutput m0_memoryArea;
    s_DataTable2Sub0* m8_pData;
    u32 mC_swingPhase;
    s8 m10_isVisible;
    // size 0x14
};

// 06090698
static const s16 A3_Flag_modelOffsets[] = { 0x028C, 0x0290, 0x0294, 0x0298 };

// 060906a0
static const fixedPoint A3_Flag_segmentScales[] = { 0x4CCC, 0xCCCC, 0x10000, 0x10000 };

struct s_A3_Flag : public s_workAreaTemplate<s_A3_Flag>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { NULL, &s_A3_Flag::Update, &s_A3_Flag::Draw, NULL };
        return &taskDefinition;
    }

    // 0605A664
    static void Update(s_A3_Flag* pThis)
    {
        fixedPoint prevValue = 0;
        fixedPoint iVar10 = pThis->m1C_wavePhase;

        pThis->m1C_wavePhase = pThis->m1C_wavePhase - pThis->m20_wavePhaseSpeed;
        pThis->m28_swayPhase = pThis->m28_swayPhase + pThis->m2C_swayPhaseSpeed;

        s16 sVar1 = pThis->m8_pData->m10_rotation[1];
        fixedPoint iVar2 = MTH_Mul(pThis->m30_swayAmplitude, getSin(((s32)pThis->m28_swayPhase >> 16) & 0xFFF));
        iVar2 = fixedPoint((s32)iVar2 + (s32)sVar1);

        s32 scaleIdx = 0;
        s32 rotIdx = 0;
        for (s32 i = 4; i != 0; i -= 2)
        {
            fixedPoint uVar3 = MTH_Mul_5_6(getSin(((s32)iVar10 >> 16) & 0xFFF), pThis->m34_waveAmplitude, A3_Flag_segmentScales[scaleIdx]);
            iVar10 = iVar10 + pThis->m18_wavePhaseStep;
            pThis->m38_segmentYaw[rotIdx] = fixedPoint(atan2_FP((s32)(uVar3 - prevValue), 0x6000)) + iVar2;

            prevValue = MTH_Mul_5_6(getSin(((s32)iVar10 >> 16) & 0xFFF), pThis->m34_waveAmplitude, A3_Flag_segmentScales[scaleIdx + 1]);
            iVar10 = iVar10 + pThis->m18_wavePhaseStep;
            pThis->m38_segmentYaw[rotIdx + 1] = fixedPoint(atan2_FP((s32)(prevValue - uVar3), 0x6000)) + iVar2;

            scaleIdx += 2;
            rotIdx += 2;
        }

        s_fieldSpecificData_A3* pFieldData = getFieldSpecificData_A3();
        s32 visResult = checkPositionVisibilityAgainstFarPlane(&pFieldData->mC0[(s8)pThis->m8_pData->m18]);
        pThis->m48_isVisible = (visResult == 0);
    }

    // 0605A1D8
    static void Draw(s_A3_Flag* pThis)
    {
        if (pThis->m48_isVisible == 0)
        {
            return;
        }

        s_DataTable2Sub0* pData = pThis->m8_pData;
        s_fieldSpecificData_A3* pFieldData = getFieldSpecificData_A3();

        sVec3_FP pos = pFieldData->mC0[(s8)pData->m18];

        fixedPoint uVar4 = pFieldData->mA4[pData->m18];
        const s16* pwVar3 = A3_Flag_modelOffsets;
        sVec3_FP segmentDir = pThis->mC_segmentOffset;

        s32 iVar5 = 0;
        do
        {
            pushCurrentMatrix();
            translateCurrentMatrix(&pos);
            rotateCurrentMatrixShiftedZ(uVar4);
            rotateCurrentMatrixZ((s32)pData->m10_rotation[2]);
            rotateCurrentMatrixShiftedY(pThis->m38_segmentYaw[iVar5 / 4]);
            rotateCurrentMatrixX((s32)pData->m10_rotation[0]);
            addObjectToDrawList(pThis->m0_memoryArea.m0_mainMemoryBundle->get3DModel((s32)(s16)pwVar3[0]));
            popMatrix();

            sMatrix4x3 mat;
            initMatrixToIdentity(&mat);
            rotateMatrixShiftedZ(uVar4, &mat);
            rotateMatrixZ((s32)pData->m10_rotation[2], &mat);
            rotateMatrixShiftedY(pThis->m38_segmentYaw[iVar5 / 4], &mat);
            rotateMatrixX((s32)pData->m10_rotation[0], &mat);
            sVec3_FP local_2c;
            transformAndAddVec(segmentDir, local_2c, mat);
            pos[0] = pos[0] + local_2c.m0_X;
            pos[1] = pos[1] + local_2c.m4_Y;
            pos[2] = pos[2] + local_2c.m8_Z;

            pushCurrentMatrix();
            translateCurrentMatrix(&pos);
            rotateCurrentMatrixShiftedZ(uVar4);
            rotateCurrentMatrixZ((s32)pData->m10_rotation[2]);
            rotateCurrentMatrixShiftedY(pThis->m38_segmentYaw[iVar5 / 4 + 1]);
            rotateCurrentMatrixX((s32)pData->m10_rotation[0]);
            addObjectToDrawList(pThis->m0_memoryArea.m0_mainMemoryBundle->get3DModel((s32)(s16)pwVar3[1]));
            popMatrix();

            initMatrixToIdentity(&mat);
            rotateMatrixShiftedZ(uVar4, &mat);
            rotateMatrixZ((s32)pData->m10_rotation[2], &mat);
            rotateMatrixShiftedY(pThis->m38_segmentYaw[iVar5 / 4 + 1], &mat);
            rotateMatrixX((s32)pData->m10_rotation[0], &mat);
            transformAndAddVec(segmentDir, local_2c, mat);
            pos[0] = pos[0] + local_2c.m0_X;
            pos[1] = pos[1] + local_2c.m4_Y;
            pos[2] = pos[2] + local_2c.m8_Z;

            pushCurrentMatrix();
            translateCurrentMatrix(&pos);
            rotateCurrentMatrixShiftedZ(uVar4);
            rotateCurrentMatrixZ((s32)pData->m10_rotation[2]);
            rotateCurrentMatrixShiftedY(pThis->m38_segmentYaw[iVar5 / 4 + 2]);
            rotateCurrentMatrixX((s32)pData->m10_rotation[0]);
            addObjectToDrawList(pThis->m0_memoryArea.m0_mainMemoryBundle->get3DModel((s32)(s16)pwVar3[2]));
            popMatrix();

            initMatrixToIdentity(&mat);
            rotateMatrixShiftedZ(uVar4, &mat);
            rotateMatrixZ((s32)pData->m10_rotation[2], &mat);
            rotateMatrixShiftedY(pThis->m38_segmentYaw[iVar5 / 4 + 2], &mat);
            rotateMatrixX((s32)pData->m10_rotation[0], &mat);
            transformAndAddVec(segmentDir, local_2c, mat);
            pos[0] = pos[0] + local_2c.m0_X;
            pos[1] = pos[1] + local_2c.m4_Y;
            pos[2] = pos[2] + local_2c.m8_Z;

            pushCurrentMatrix();
            translateCurrentMatrix(&pos);
            rotateCurrentMatrixShiftedZ(uVar4);
            rotateCurrentMatrixZ((s32)pData->m10_rotation[2]);
            rotateCurrentMatrixShiftedY(pThis->m38_segmentYaw[iVar5 / 4 + 3]);
            rotateCurrentMatrixX((s32)pData->m10_rotation[0]);
            addObjectToDrawList(pThis->m0_memoryArea.m0_mainMemoryBundle->get3DModel((s32)(s16)pwVar3[3]));
            popMatrix();

            initMatrixToIdentity(&mat);
            rotateMatrixShiftedZ(uVar4, &mat);
            rotateMatrixZ((s32)pData->m10_rotation[2], &mat);
            rotateMatrixShiftedY(pThis->m38_segmentYaw[iVar5 / 4 + 3], &mat);
            rotateMatrixX((s32)pData->m10_rotation[0], &mat);
            transformAndAddVec(segmentDir, local_2c, mat);
            pos[0] = pos[0] + local_2c.m0_X;
            pos[1] = pos[1] + local_2c.m4_Y;
            pos[2] = pos[2] + local_2c.m8_Z;

            pwVar3 = pwVar3 + 4;
            iVar5 = iVar5 + 0x10;
        } while (pwVar3 <= &A3_Flag_modelOffsets[3]);
    }

    s_memoryAreaOutput m0_memoryArea;
    s_DataTable2Sub0* m8_pData;
    sVec3_FP mC_segmentOffset;
    fixedPoint m18_wavePhaseStep;
    fixedPoint m1C_wavePhase;
    fixedPoint m20_wavePhaseSpeed;
    fixedPoint m24;
    fixedPoint m28_swayPhase;
    fixedPoint m2C_swayPhaseSpeed;
    fixedPoint m30_swayAmplitude;
    fixedPoint m34_waveAmplitude;
    fixedPoint m38_segmentYaw[4];
    s8 m48_isVisible;
    // size 0x4C
};

// 0605a94c
void create_A3_FlagPole(s_visdibilityCellTask* r4, s_DataTable2Sub0& r5, s32 r6)
{
    s_A3_FlagPole* pNewTask = createSubTask<s_A3_FlagPole>(r4);
    getMemoryArea(&pNewTask->m0_memoryArea, r6);
    pNewTask->m8_pData = &r5;
    getFieldSpecificData_A3()->mC0[r5.m18] = r5.m4_position;
    pNewTask->mC_swingPhase = randomNumber();
}

// 0605a7a8
void create_A3_Flag(s_visdibilityCellTask* r4, s_DataTable2Sub0& r5, s32 r6)
{
    s_A3_Flag* pNewTask = createSubTask<s_A3_Flag>(r4);
    getMemoryArea(&pNewTask->m0_memoryArea, r6);
    pNewTask->m8_pData = &r5;
    pNewTask->mC_segmentOffset = { 0x6000, 0, 0 };
    pNewTask->m18_wavePhaseStep = 0x2000000;
    pNewTask->m20_wavePhaseSpeed = 0xC16C16;
    pNewTask->m24 = 0xB60B6;
    pNewTask->m2C_swayPhaseSpeed = 0x2D82D8;
    pNewTask->m30_swayAmplitude = 0xAAAAAA;
    pNewTask->m34_waveAmplitude = 0x2400;
    pNewTask->m1C_wavePhase = randomNumber();
    pNewTask->m28_swayPhase = randomNumber();
}
