#include "PDS.h"

void DrawLCSTarget(s_LCSTask* r14, sVec2_S16* r5, s32 r6);

void create_A3_Obj2_Sub0(sLCSTarget* r4, s_workArea* r5, void* r6, const sVec3_FP* r7, const sVec3_FP* arg0, s16 flags, s16 argA, s16 argE, s32 arg10, s32 arg14)
{
    r4->m0 = r5;
    r4->m4 = r6;
    r4->m8_parentWorldCoordinates = r7;
    r4->mC = arg0;
    r4->m12 = argA;
    r4->m14 = argE;
    if (r4->m14 >= 0)
    {
        r4->m16 = arg10;
    }
    else
    {
        r4->m16 = 0;
    }
    r4->m10_flags = flags;
    r4->m17 = arg14;
    r4->m18 = 0;
    r4->m19 = 0;
    r4->m1A = 0;
    r4->m1B = 0;
    r4->m1C = 0;
    r4->m20 = 0;
}

void A3_Obj2_UpdateSub0(sLCSTarget* r14)
{
    sVec3_FP var0;
    sVec3_FP varC;

    s_LCSTask* r13 = getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS;

    r14->m19 &= ~0x30;
    r14->m1C = 0;
    r14->m18 &= ~4;
    if (r14->m18 & 1)
    {
        return;
    }
    if (r14->mC)
    {
        //606CF1C
        if (r14->m10_flags & sLCSTarget::e_moveWithParent)
        {
            //0606CF24
            varC = *r14->m8_parentWorldCoordinates;
        }
        else
        {
            //606CF46
            transformAndAddVecByCurrentMatrix(r14->m8_parentWorldCoordinates, &varC);
        }

        //606CF50
        if (r14->m10_flags & sLCSTarget::e_200)
        {
            var0 = *r14->mC;
        }
        else
        {
            //606CF74
            transformVecByCurrentMatrix(*r14->mC, var0);
        }

        //606CF7C
        if (dot3_FP(&varC, &var0) < 0)
        {
            r14->m18 |= 4;
        }
    }
    //606CF96
    assert(r13);

    r13->m10++;

    if (r13->mC < 0x100)
    {
        r13->m14[r13->mC].m0 = r14;
        r13->m14[r13->mC].m4 = 0;
        r13->mC++;
    }
}

void LCSTaskDrawSub3(sVec3_FP* r4, sVec2_S16* r5)
{
    s16 var0;
    s16 var4;
    getVdp1ProjectionParams(&var0, &var4);

    (*r5)[0] = setDividend(var0, (*r4)[0], (*r4)[2]);
    (*r5)[1] = setDividend(var4, (*r4)[1], (*r4)[2]);
}

void LCSTaskDrawSub4(const sVec3_FP* r4, sVec2_S16* r5, sVec3_FP* r6)
{
    transformAndAddVecByCurrentMatrix(r4, r6);

    s16 var0;
    s16 var4;
    getVdp1ProjectionParams(&var0, &var4);

    (*r5)[0] = setDividend(var0, (*r6)[0], (*r6)[2]);
    (*r5)[1] = setDividend(var4, (*r6)[1], (*r6)[2]);
}

void LCSTaskDrawSub1Sub2Sub2Sub1Sub2SubSub0(s32 r4)
{
    assert(0);
}

void LCSTaskDrawSub1Sub2Sub2Sub1Sub2Sub(s_LCSTask* r4, sLCSTarget* r5)
{
    if (r5->m17)
    {
        if (r4->m820)
        {
            LCSTaskDrawSub1Sub2Sub2Sub1Sub2SubSub0(r4->m820);
        }

        r5->m20 = createLCSShootTask(r4, r5);
        r5->m19 = (r5->m19 & 0xF0) | 5;
        startScript_cantDestroy();
    }

    r4->m81C_currentLCSTarget = r5;
    r4->m8 &= ~0x200;
}

void LCSTaskDrawSub1Sub5(s_LCSTask* r4)
{
    if (r4->m820 == 0)
        return;
    assert(0);
}

void LCSTaskDrawSub1Sub2Sub3Sub0(s_LCSTask* r4)
{
    if (r4->m820)
        assert(0);
}

void LCSTaskDrawSub1Sub2Sub2Sub1Sub2(s_LCSTask* r4, sLCSTarget* r5)
{
    if (!(r5->m19 & 4))
    {
        LCSTaskDrawSub1Sub2Sub2Sub1Sub2Sub(r4, r5);
    }
}

void LCSTaskDrawSub1Sub2Sub2Sub1(s_LCSTask* r4, sLCSTarget* r5)
{
    if (r5 == nullptr)
        return;

    r4->m83C = 3;

    switch (r5->m10_flags & 3)
    {
    case 0:
    case 1:
    case 3:
        if (r4->m83E < r5->m17 + 1)
        {
            LCSTaskDrawSub1Sub2Sub2Sub1Sub2(r4, r5);
            return;
        }
        break;
    default:
        assert(0);
        break;
    }

    //0606D468
    assert(0);


    //if (!LCSTaskDrawSub1Sub2Sub2Sub1Sub0(r4, r4->m830, &r4->m830, &r4->m834))
      //  return;
}

void fieldScriptTaskUpdateSub2Sub1Sub1(s_LCSTask* r4)
{
    s_LCSTask_828* r5 = r4->m828;
    while (r5)
    {
        s_LCSTask_828* next = r5->m4_next;
        assert(0);
        //fieldScriptTaskUpdateSub2Sub1Sub1Sub1(r5);
        r5 = next;
    }
}

void LCSTaskDrawSub1Sub2Sub1(s_LCSTask* r4)
{
    s32 r11 = 1;
    while (r11 < r4->mC)
    {
        assert(0);
    }
}

sVec2_S16 LCS_Var0 = { 0,0 };

sLCSTarget* FindClosestLCSTarget(s_LCSTask* r4)
{
    sLCSTarget* r6 = 0;
    fixedPoint r13_bestScore = 0x7FFFFFFF;

    if (r4->m8 & 0x200)
    {
        assert(0);
    }

    s32 r5 = 0;
    while (r5 < r4->mC)
    {
        sLCSTarget* pLCSTarget = r4->m14[r5].m0;

        if (!(pLCSTarget->m18 & 6) && (pLCSTarget->m24_worldspaceCoordinates[1] > 0x3000) && (pLCSTarget->m24_worldspaceCoordinates[1] < r4->m814_LCSTargetMaxHeight))
        {
            s32 r1;
            if (pLCSTarget->m30_screenspaceCoordinates[0] >= 0)
            {
                r1 = pLCSTarget->m30_screenspaceCoordinates[0];
            }
            else
            {
                r1 = -pLCSTarget->m30_screenspaceCoordinates[0];
            }

            if (r1 < 176)
            {
                if (pLCSTarget->m30_screenspaceCoordinates[1] >= 0)
                {
                    r1 = pLCSTarget->m30_screenspaceCoordinates[1];
                }
                else
                {
                    r1 = -pLCSTarget->m30_screenspaceCoordinates[1];
                }

                if (r1 < 112)
                {
                    fixedPoint distanceFromCursorSquare = (LCS_Var0[0] - pLCSTarget->m30_screenspaceCoordinates[0]) * (LCS_Var0[0] - pLCSTarget->m30_screenspaceCoordinates[0]) + (LCS_Var0[1] - pLCSTarget->m30_screenspaceCoordinates[1]) * (LCS_Var0[0] - pLCSTarget->m30_screenspaceCoordinates[0]);
                    if (distanceFromCursorSquare < r13_bestScore)
                    {
                        r13_bestScore = distanceFromCursorSquare;
                        r6 = pLCSTarget;
                    }
                }
            }
        }

        r5++;
    }

    return r6;
}

void LCSTaskDrawSub1Sub2Sub4(s_LCSTask* r4)
{
    s_LCSTask_828* r13 = r4->m828;

    while (r13)
    {
        assert(0);
        r13 = r13->m4_next;
    }
}

void LCSTaskDrawSub1Sub2Sub3(s_LCSTask* r4)
{
    s_LCSTask_828* r13 = r4->m828;

    while (r13)
    {
        assert(0);
        r13 = r13->m4_next;
    }

    LCSTaskDrawSub1Sub2Sub3Sub0(r4);
}

void LCSTaskDrawSub1Sub2Sub5(s_LCSTask* r4)
{
    s_LCSTask_828* r5 = r4->m828;

    while (r5)
    {
        assert(0);
    }
}

sLCSTarget* findNewClosestLCSTarget(s_LCSTask* r4, sVec2_S16* r5)
{
    s32 r8_bestDistance = 0x100;
    s32 r6 = 0;
    sLCSTarget* r12 = NULL;

    for (s32 r7 = 0; r7 < r4->mC; r7++)
    {
        sLCSTarget* r6 = r4->m14[r7].m0;
        if (!(r6->m18 & 6) && (r4->m818 != r6) && (r6->m1A <= r6->m17) && !(r6->m19 & 4) && (r6->m24_worldspaceCoordinates[1] > 0x3000) && (r6->m24_worldspaceCoordinates[1] < r4->m814_LCSTargetMaxHeight))
        {
            if (abs(r6->m30_screenspaceCoordinates[0]) >= 176)
                continue;

            if (abs(r6->m30_screenspaceCoordinates[1]) >= 112)
                continue;

            s32 dist1 = (*r5)[0] - r6->m30_screenspaceCoordinates[0];
            s32 dist2 = (*r5)[1] - r6->m30_screenspaceCoordinates[1];

            s32 distSquare = (dist1 * dist1) + (dist2 * dist2);

            if ((distSquare < 0x100) && (distSquare < r8_bestDistance))
            {
                r12 = r6;
                r8_bestDistance = distSquare;
            }
        }
    }

    return r12;
}

sLCSTarget* LCSTaskDrawSub1Sub2Sub2Sub3(s_LCSTask* r14)
{
    r14->m83D = 0;

    s32 r4;
    if (r14->m818)
    {
        assert(0);
    }
    else
    {
        //0606DC0A
        r4 = 1;
    }

    //0606DC0C
    if (r4)
    {
        sLCSTarget* r13 = findNewClosestLCSTarget(r14, &getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m200_LCSCursorScreenSpaceCoordinates);
        if (r13 == NULL)
        {
            return NULL;
        }
        if (r14->m818 == NULL)
            return r13;

        if ((r14->m818->m10_flags & 3) == 2)
        {
            if ((r13->m10_flags & 3) == 2)
            {
                return r13;
            }

            fieldScriptTaskUpdateSub2Sub1Sub1(r14);
            LCSTaskDrawSub1Sub5(r14);
            return r13;
        }
        else
        {
            fieldScriptTaskUpdateSub2Sub1Sub1(r14);
            LCSTaskDrawSub1Sub5(r14);
            return r13;
        }
    }
    else
    {
        return r14->m818;
    }
}

void LCSUpdateCursorFromInput(s_dragonTaskWorkArea* r4)
{
    if (graphicEngineStatus.m4514.m0->m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8[1][5]) // down
    {
        r4->m1F0.m_8 = fixedPoint(0x1C71C7);
    }
    else if (graphicEngineStatus.m4514.m0->m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8[1][4]) // up
    {
        r4->m1F0.m_8 = -fixedPoint(0x1C71C7);
    }

    if (graphicEngineStatus.m4514.m0->m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8[1][7])
    {
        r4->m1F0.m_C = -fixedPoint(0x1C71C7);
    }
    else if (graphicEngineStatus.m4514.m0->m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8[1][6])
    {
        r4->m1F0.m_C = fixedPoint(0x1C71C7);
    }

    sVec3_FP var0;
    var0[0] = 0;
    var0[1] = MTH_Mul(0x147A, r4->m1F0.m_C);
    var0[0] = 0;

    if (r4->mF8_Flags & 0x8000)
    {
        LCSUpdateCursorFromInputSub0(0, &var0);
    }
}

void LCSTaskDrawSub1Sub2Sub2Sub2(s_dragonTaskWorkArea* r4)
{
    getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1F0.m_8 = 0;
    getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1F0.m_C = 0;

    switch (graphicEngineStatus.m4514.m0->m0_current.m0)
    {
    case 1:
        LCSUpdateCursorFromInput(r4);
        break;
    default:
        assert(0);
        break;
    }

    //606DDF4
    r4->m1F0.m_0 += ((s16)(MTH_Mul(r4->m208, r4->m1F0.m_8) >> 16));
    r4->m1F0.m_4 += ((s16)(MTH_Mul(r4->m20C, r4->m1F0.m_C) >> 16));

    if (r4->m1F0.m_0 < -0x10000)
        r4->m1F0.m_0 = -0x10000;
    if (r4->m1F0.m_0 > 0x10000)
        r4->m1F0.m_0 = 0x10000;

    if (r4->m1F0.m_4 < -0x10000)
        r4->m1F0.m_4 = -0x10000;
    if (r4->m1F0.m_4 > 0x10000)
        r4->m1F0.m_4 = 0x10000;

    r4->m200_LCSCursorScreenSpaceCoordinates[0] = (s16)((-MTH_Mul(176 * r4->m210, r4->m1F0.m_4)) >> 16);
    r4->m200_LCSCursorScreenSpaceCoordinates[1] = (s16)((-MTH_Mul(112 * r4->m214, r4->m1F0.m_0)) >> 16);
}

s32 LCSTaskDrawSub1Sub2Sub2(s_LCSTask* r4)
{
    s_dragonTaskWorkArea* r14 = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    if (r4->m8 & 4)
    {
        sLCSTarget* pLcsTarget = FindClosestLCSTarget(r4);
        if (pLcsTarget)
        {
            r14->m1F0.m_0 = -FP_Div(performDivision(224, pLcsTarget->m30_screenspaceCoordinates[1] << 17), r14->m214);
            r14->m1F0.m_4 = -FP_Div(performDivision(352, pLcsTarget->m30_screenspaceCoordinates[0] << 17), r14->m210);

            LCSTaskDrawSub1Sub2Sub2Sub1(r4, pLcsTarget);
        }
        else
        {
            r14->m1F0.m_0 = 0;
            r14->m1F0.m_4 = 0;
        }
    }

    //0606DF48
    LCSTaskDrawSub1Sub2Sub2Sub2(r14);

    if (--r4->m83C <= 0)
    {
        r4->m83C = 0;
        LCSTaskDrawSub1Sub2Sub2Sub1(r4, LCSTaskDrawSub1Sub2Sub2Sub3(r4));
    }

    return 1;
}

void LCSTaskDrawSub1Sub2Sub0Sub1(s_LCSTask* r4)
{
    s_LCSTask_828* r13 = r4->m828;

    while (r13)
    {
        assert(0);
        r13 = r13->m4_next;
    }
}

void LCSTaskDrawSub1Sub2Sub0Sub2(s_LCSTask* r4)
{
    if (--r4->m83D > 0)
        return;

    r4->m83D = 0;

    if (r4->m838)
    {
        assert(0);
    }
}

void setLCSField83E(s_workArea* pLCS, u32 value)
{
    PDS_unimplemented("setLCSField83E");
}

void fieldOverlaySubTask2Init(p_workArea pWorkArea)
{
    s_LCSTask* pTypedWorkArea = static_cast<s_LCSTask*>(pWorkArea);

    getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS = pTypedWorkArea;
    getMemoryArea(&pTypedWorkArea->m0, 0);
    pTypedWorkArea->m814_LCSTargetMaxHeight = 0x200000;
    if (gDragonState->mC_dragonType == 8)
    {
        setLCSField83E(pTypedWorkArea, 1);
    }
    else
    {
        setLCSField83E(pTypedWorkArea, gDragonState->mC_dragonType + 1);
    }

    pTypedWorkArea->m9C0 = allocateHeapForTask(pTypedWorkArea, 0x40);
}

void LCSTaskDrawSub1Sub2Sub0(s_LCSTask* r4)
{
    if (!(r4->m8 & 8))
        return;

    LCSTaskDrawSub1Sub2Sub4(r4);
    LCSTaskDrawSub1Sub2Sub0Sub1(r4);
    LCSTaskDrawSub1Sub2Sub0Sub2(r4);

    // TODO: recheck that test
    if (r4->m828)
        return;

    r4->m8 = (r4->m8 & ~8) | 0x80;

    LCSTaskDrawSub1Sub5(r4);
}

void LCSTaskDrawSub1Sub1(s_LCSTask* r4)
{
    if (r4->m8 == 0)
    {
        r4->m9DA_LCSPhase = s_LCSTask::LCSPhase_0_init;
    }

    if (graphicEngineStatus.m4514.m0->m0_current.m8_newButtonDown & graphicEngineStatus.m4514.mD8[1][2])
    {
        r4->m9D0++;
    }
    else
    {
        r4->m9D0 = 0;
    }

    switch (r4->m9DA_LCSPhase)
    {
    case 0:
        if (r4->m9D0 == 0)
            return;
        r4->m9DA_LCSPhase = s_LCSTask::LCSPhase_2_targeting;
        break;
    case 1:
        if (r4->m9D0 >= 24)
        {
            r4->m9DA_LCSPhase = s_LCSTask::LCSPhase_3;
            return;
        }
        if (!(graphicEngineStatus.m4514.m0->m0_current.mA  & graphicEngineStatus.m4514.mD8[1][2]))
            return;
        r4->m9DA_LCSPhase = s_LCSTask::LCSPhase_2_targeting;
        break;
    case 2: // LCS targeting
        if (r4->m83D)
            return;
        // cancel LCS?
        if (graphicEngineStatus.m4514.m0->m0_current.m8_newButtonDown & 6)
        {
            r4->m9DA_LCSPhase = s_LCSTask::LCSPhase_4_executing;
        }
        break;
    case 3:
        if (graphicEngineStatus.m4514.m0->m0_current.mA  & graphicEngineStatus.m4514.mD8[1][2])
        {
            r4->m9DA_LCSPhase = s_LCSTask::LCSPhase_4_executing;
        }
        break;
    case 4:
        break;
    default:
        assert(0);
        break;
    }
}

void LCSTaskDrawSub1Sub0(s_LCSTask* r4)
{
    s_LCSTask_828* r13 = r4->m828;

    while (r13)
    {
        assert(0);
        r13 = r13->m4_next;
    }

    if (r4->m820)
    {
        assert(0);
    }
}

void LCSTaskDrawSub1Sub2(s_LCSTask* r4)
{
    LCSTaskDrawSub1Sub2Sub0(r4);
    if (!(r4->m8 & 2))
        return;

    LCSTaskDrawSub1Sub2Sub1(r4);
    if (!LCSTaskDrawSub1Sub2Sub2(r4))
    {
        r4->m8 |= 0x20;
    }

    LCSTaskDrawSub1Sub2Sub3(r4);

    if (r4->m9DA_LCSPhase != 4)
    {
        return;
    }

    if (r4->m8 & 4)
    {
        return;
    }

    r4->m8 = (r4->m8 & ~2) | 8;

    r4->m83D = 0;

    LCSTaskDrawSub1Sub2Sub4(r4);
    LCSTaskDrawSub1Sub2Sub5(r4);

    if (r4->m828)
    {
        assert(0);
    }
}

void LCSTaskDrawSub1(s_LCSTask* r4)
{
    LCSTaskDrawSub1Sub0(r4);

    if (getFieldTaskPtr()->m8_pSubFieldData)
    {
        getFieldTaskPtr()->m28_status |= 0x4;
        if (r4->m8 & 0x40)
        {
            return;
        }
    }
    else
    {
        getFieldTaskPtr()->m28_status &= ~0x4;
    }

    LCSTaskDrawSub1Sub1(r4);

    r4->m8 &= ~0x14;
    if (r4->m8 & 1)
    {
        r4->m8 |= 0x20;
    }

    //0606E5E6
    if ((r4->m8 & 0x8B) == 0)
    {
        if (--r4->m83C <= 0)
        {
            r4->m83C = 0;
        }

        if ((r4->m83C == 0) && (r4->m9DA_LCSPhase))
        {
            //0606E612
            r4->m8 |= 6;
            r4->m9DA_LCSPhase = s_LCSTask::LCSPhase_2_targeting;
            if (LCSTaskDrawSub1Sub6())
            {
                if (getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m25D != 2)
                {
                    r4->m8 |= 0x200;
                }
            }
        }
    }

    //0606E644
    if (r4->m8 & 2)
    {
        if (!(r4->m8 & 4) && (graphicEngineStatus.m4514.m0->m0_current.m8_newButtonDown & 1))
        {
            r4->m8 |= 0x20;
        }
        if (r4->m8 & 0x20)
        {
            fieldScriptTaskUpdateSub2Sub1Sub1(r4);
            LCSTaskDrawSub1Sub5(r4);
            r4->m8 &= ~0x22;
            r4->m8 |= 0x80;
            r4->m9DA_LCSPhase = s_LCSTask::LCSPhase_0_init;
        }
    }
    else
    {
        r4->m8 &= ~0x20;
    }

    //606E6AC
    LCSTaskDrawSub1Sub2(r4);

    if (r4->m8 & 0x4)
    {
        LCSTaskDrawSub1Sub3();
    }

    //606E6BC
    if (r4->m8 & 0x80)
    {
        // This is called when LCS is canceled/finished
        // TODO: reevaluate the logic of the test
        bool test0 = r4->m83F || getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m38_dialogStringTask;
        bool test1 = fieldScriptTaskUpdateSub4();
        bool test2 = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m48_cutsceneTask != nullptr;
        if ((test0 && test2) || (!test0 && test1 && test2) || (!test1))
        {
            r4->m8 = (r4->m8 & ~0x80) | 0x100;
        }
    }

    //606E708
    if (r4->m8 & 0x100)
    {
        if (!getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m48_cutsceneTask)
        {
            LCSTaskDrawSub1Sub4();
        }

        r4->m83C = 0xF;
        r4->m8 = 0;
    }
}

void LCSTaskDrawSub5(s_LCSTask* r4)
{
    if (r4->m8 & 2)
    {
        if (!r4->m828 || (r4->m818 && ((r4->m818->m10_flags & 3) == 2)))
        {
            DrawLCSTarget(r4, &getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m200_LCSCursorScreenSpaceCoordinates, 0);
        }
        else
        {
            DrawLCSTarget(r4, &getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m200_LCSCursorScreenSpaceCoordinates, 1);
        }
    }

    //606E894
    s_LCSTask_828* r13 = r4->m828;
    while (r13)
    {
        assert(0);
        r13 = r13->m4_next;
    }
}

void LCSTaskDrawSubSub(s_LCSTask* r4)
{
    if (r4->m8 & 2)
    {
        if (pauseEngine[0])
        {
            r4->m8 |= 0x20;
        }
    }

    if (getFieldTaskPtr()->m8_pSubFieldData->m354 > 1)
    {
        LCSTaskDrawSub1(r4);
    }

    for (int r12 = 0; r12 < r4->mC; r12++)
    {
        sLCSTarget* r14 = r4->m14[r12].m0;
        if (r14->m10_flags & sLCSTarget::e_moveWithParent)
        {
            r14->m24_worldspaceCoordinates = *r14->m8_parentWorldCoordinates;
            LCSTaskDrawSub3(&r14->m24_worldspaceCoordinates, &r14->m30_screenspaceCoordinates);
        }
        else
        {
            LCSTaskDrawSub4(r14->m8_parentWorldCoordinates, &r14->m30_screenspaceCoordinates, &r14->m24_worldspaceCoordinates);
        }
    }

    if (!(r4->m8 & 0x40))
    {
        LCSTaskDrawSub5(r4);
    }

    if (enableDebugTask)
    {
        assert(0);
    }

    r4->m9C8 = 0;
    r4->m9CC = 0;
    r4->mC = 0;
    r4->m10 = 0;
    r4->m9C4 = 0;
}

void fieldScriptTaskUpdateSub2Sub1()
{
    getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m8 |= 0x40;

    fieldScriptTaskUpdateSub2Sub1Sub1(getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS);
}

void dragonFieldTaskUpdateSub2(u32 r4)
{
    setLCSField83E(getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS, r4);
}

s_taskDefinition fieldOverlaySubTask2Definition = { fieldOverlaySubTask2Init, NULL, NULL, NULL, "fieldOverlaySubTask2" };

void createFieldOverlaySubTask2(s_workArea* pWorkArea)
{
    createSubTask(pWorkArea, &fieldOverlaySubTask2Definition, new s_LCSTask);
}

void LCSTaskDrawSub()
{
    if (!getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->getTask()->isPaused())
    {
        LCSTaskDrawSubSub(getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS);
    }
}

void DrawLCSTarget(s_LCSTask* r14, sVec2_S16* r5, s32 r6)
{
    sSaturnPtr dataR6 = sSaturnPtr({ 0x6093B28, gFLD_A3 }) + 0x1C * r6;

    s32 r0 = r14->m0.m4_characterArea - getVdp1Pointer(0x25C00000);
    r0 >>= 3;

    s32 var8 = ((r14->m0.m4_characterArea - getVdp1Pointer(0x25C00000)) >> 3) + readSaturnS16(dataR6 + 6);
    s32 var0 = ((r14->m0.m4_characterArea - getVdp1Pointer(0x25C00000)) >> 3) + readSaturnS16(dataR6 + 0xA);
    s32 var4 = readSaturnS32(dataR6 + 0xC) >> 12;
    s32 varC = readSaturnS32(dataR6 + 0x10) >> 12;

    s32 r4 = readSaturnS32(dataR6 + 0x14) >> 12;
    s32 r2 = readSaturnS32(dataR6 + 0x18) >> 12;

    s16 var10[4];
    var10[0] = r4 + (*r5)[0];
    var10[1] = r2 + (*r5)[1];
    var10[2] = r4 + (*r5)[0] + var4;
    var10[3] = r2 + (*r5)[1] - varC;

    u32 vdp1WriteEA = graphicEngineStatus.vdp1Context[0].currentVdp1WriteEA;

    setVdp1VramU16(vdp1WriteEA + 0x00, readSaturnS16(dataR6 + 2)); // command 0
    setVdp1VramU16(vdp1WriteEA + 0x04, readSaturnS16(dataR6 + 4)); // CMDPMOD
    setVdp1VramU16(vdp1WriteEA + 0x06, var0); // CMDCOLR
    setVdp1VramU16(vdp1WriteEA + 0x08, var8); // CMDSRCA
    setVdp1VramU16(vdp1WriteEA + 0x0A, readSaturnS16(dataR6 + 8)); // CMDSIZE
    setVdp1VramU16(vdp1WriteEA + 0x0C, var10[0]); // CMDXA
    setVdp1VramU16(vdp1WriteEA + 0x0E, -var10[1]); // CMDYA
    setVdp1VramU16(vdp1WriteEA + 0x014, var10[2]); // CMDXC
    setVdp1VramU16(vdp1WriteEA + 0x016, -var10[3]); // CMDYX

    graphicEngineStatus.vdp1Context[0].pCurrentVdp1Packet->bucketTypes = 0;
    graphicEngineStatus.vdp1Context[0].pCurrentVdp1Packet->vdp1EA = vdp1WriteEA >> 3;
    graphicEngineStatus.vdp1Context[0].pCurrentVdp1Packet++;

    graphicEngineStatus.vdp1Context[0].m1C += 1;
    graphicEngineStatus.vdp1Context[0].currentVdp1WriteEA = vdp1WriteEA + 0x20;
    graphicEngineStatus.vdp1Context[0].mC += 1;
}

void allocateLCSEntry(s_visibilityGridWorkArea* r4, u8* r5, u32 r6)
{
    r4->m12E4++;

    if (r4->m12E4 >= 24)
    {
        return;
    }

    r4->m44[0].m0 = r5;
    r4->m44[1].m34 = r6;
    r4->m44++;
}
