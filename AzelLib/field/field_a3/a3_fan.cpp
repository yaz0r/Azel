#include "PDS.h"
#include "a3_fan.h"

struct s_A3_0_Obj4 : public s_workAreaTemplate<s_A3_0_Obj4>
{
    static void Update1(s_A3_0_Obj4* pThis)
    {
        updateLCSTarget(&pThis->m24_LCSTarget);
        pThis->mC_blade1 += 0x2D82D8;
        pThis->m10_blade2 += 0x16C16C;
    }

    static void Update2(s_A3_0_Obj4* pThis)
    {
        updateLCSTarget(&pThis->m24_LCSTarget);
        pThis->mC_blade1 += 0x16C16C;
        pThis->m10_blade2 += 0x2D82D8;
    }

    static void Update3(s_A3_0_Obj4* pThis)
    {
        updateLCSTarget(&pThis->m24_LCSTarget);
        pThis->mC_blade1 += pThis->m14_currentSpeed;
        pThis->m10_blade2 += 0x2D82D8;
        
        if (pThis->m20_numFramesToChangeSpeed <= 0)
        {
            pThis->m14_currentSpeed += pThis->m1C_speedIncrement;

            bool changeSpeed = false;
            if ((pThis->m1C_speedIncrement >= 0) && (pThis->m14_currentSpeed >= pThis->m18_targetSpeed))
            {
                changeSpeed = true;
            }
            if ((pThis->m1C_speedIncrement < 0) && (pThis->m14_currentSpeed < pThis->m18_targetSpeed))
            {
                changeSpeed = true;
            }

            if (changeSpeed)
            {
                pThis->m18_targetSpeed = readSaturnS32(sSaturnPtr({ 0x608EF64, gFLD_A3 }) + (randomNumber() & 3) * 4);
                if (pThis->m14_currentSpeed > pThis->m18_targetSpeed)
                {
                    pThis->m1C_speedIncrement = -0x91A2;
                }
                else
                {
                    pThis->m1C_speedIncrement = 0x91A2;
                }

                pThis->m20_numFramesToChangeSpeed = randomNumber() & 0x3F;
            }
        }
        else
        {
            pThis->m20_numFramesToChangeSpeed--;
        }
    }

    static void Update4Sub0(s_A3_0_Obj4* pThis)
    {
        updateLCSTarget(&pThis->m24_LCSTarget);
        pThis->mC_blade1 += 0x7F6E5;
        pThis->m10_blade2 += 0xB60B6;
    }

    static void Update4Sub1(s_A3_0_Obj4* pThis)
    {
        updateLCSTarget(&pThis->m24_LCSTarget);
        pThis->mC_blade1 += 0xB60B60;
        pThis->m10_blade2 += 0x9F49F4;
    }

    static void Update4(s_A3_0_Obj4* pThis)
    {
        updateLCSTarget(&pThis->m24_LCSTarget);

        if (mainGameState.getBit(0xA2, 3))
        {
            pThis->m_UpdateMethod = &s_A3_0_Obj4::Update4Sub0;
        }
        else
        {
            pThis->m_UpdateMethod = &s_A3_0_Obj4::Update4Sub1;
        }
    }

    static void Draw(s_A3_0_Obj4* pThis)
    {
        if (!checkPositionVisibilityAgainstFarPlane(&pThis->m8->m4_position))
        {
            pushCurrentMatrix();
            {
                translateCurrentMatrix(pThis->m8->m4_position);
                rotateCurrentMatrixY(pThis->m58);
                addObjectToDrawList(pThis->m0.m0_mainMemory, READ_BE_U32(pThis->m0.m0_mainMemory + 0x254));

                pushCurrentMatrix();
                {
                    rotateCurrentMatrixShiftedZ(pThis->m10_blade2);
                    addObjectToDrawList(pThis->m0.m0_mainMemory, READ_BE_U32(pThis->m0.m0_mainMemory + 0x258));
                }
                popMatrix();

                pushCurrentMatrix();
                {
                    rotateCurrentMatrixShiftedZ(pThis->mC_blade1);
                    addObjectToDrawList(pThis->m0.m0_mainMemory, READ_BE_U32(pThis->m0.m0_mainMemory + 0x25C));
                }
                popMatrix();

                addObjectToDrawList(pThis->m0.m0_mainMemory, READ_BE_U32(pThis->m0.m0_mainMemory + 0x260));
                addObjectToDrawList(pThis->m0.m0_mainMemory, READ_BE_U32(pThis->m0.m0_mainMemory + 0x264));
                addObjectToDrawList(pThis->m0.m0_mainMemory, READ_BE_U32(pThis->m0.m0_mainMemory + 0x268));
                addObjectToDrawList(pThis->m0.m0_mainMemory, READ_BE_U32(pThis->m0.m0_mainMemory + 0x26C));
                addObjectToDrawList(pThis->m0.m0_mainMemory, READ_BE_U32(pThis->m0.m0_mainMemory + 0x270));
                callGridCellDraw_normalSub2(pThis, 0x2B0);
            }
            popMatrix();
        }
    }

    s_memoryAreaOutput m0;
    s_DataTable2Sub0* m8;
    s32 mC_blade1;
    s32 m10_blade2;
    s32 m14_currentSpeed;
    s32 m18_targetSpeed;
    s32 m1C_speedIncrement;
    s32 m20_numFramesToChangeSpeed;
    sLCSTarget m24_LCSTarget;
    s16 m58;
    // size 0x5C
};

static const std::array<s_A3_0_Obj4::FunctionType, 4> UpdateTable = {&s_A3_0_Obj4::Update1, &s_A3_0_Obj4::Update2, &s_A3_0_Obj4::Update3, &s_A3_0_Obj4::Update4};

void A3_0_Obj4_LCSCallback(p_workArea task, sLCSTarget*)
{
    assert(0);
}

void create_A3_0_Obj4(s_visdibilityCellTask* r4, s_DataTable2Sub0& r5, s32 r6)
{
    s_A3_0_Obj4* pNewTask = createSubTaskFromFunction<s_A3_0_Obj4>(r4, NULL);
    getMemoryArea(&pNewTask->m0, r6);
    pNewTask->m8 = &r5;

    s32 bitIndex = readSaturnS16(sSaturnPtr({ 0x608EF3C, gFLD_A3 }) + r5.m18 * 2);
    if (mainGameState.getBit566(bitIndex))
    {
        pNewTask->getTask()->markFinished();
    }
    else
    {
        pNewTask->m_UpdateMethod = UpdateTable[readSaturnS8(sSaturnPtr({ 0x608EF56, gFLD_A3 }) + r5.m18)];
        pNewTask->m_DrawMethod = &s_A3_0_Obj4::Draw;

        if (r5.m10_rotation[0])
        {
            pNewTask->m58 = 0x800 - r5.m10_rotation[1];
        }
        else
        {
            pNewTask->m58 = r5.m10_rotation[1];
        }

        pNewTask->m14_currentSpeed = 0;
        pNewTask->m18_targetSpeed = 0;
        pNewTask->m1C_speedIncrement = 0;
        pNewTask->m20_numFramesToChangeSpeed = 0;

        s32 LCSParameter = readSaturnS32(sSaturnPtr({ 0x608EF08, gFLD_A3 }) + r5.m18 * 4);
        if (LCSParameter == -1)
        {
            createLCSTarget(&pNewTask->m24_LCSTarget, pNewTask, &A3_0_Obj4_LCSCallback, &r5.m4_position, 0, 2, 0, -1, 0, 6);
        }
        else
        {
            createLCSTarget(&pNewTask->m24_LCSTarget, pNewTask, &A3_0_Obj4_LCSCallback, &r5.m4_position, 0, 2, 0, LCSParameter, 1, 6);
        }
    }
}
