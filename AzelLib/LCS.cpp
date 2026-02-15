#include "PDS.h"
#include "items.h"
#include "audio/systemSounds.h"
#include "field/field_a3/o_fld_a3.h" //TODO: cleanup
#include "field/fieldVisibilityGrid.h"

p_workArea createLCSSelectedTask(s_LCSTask* r4, sLCSTarget* r5);
void DrawLCSTarget(s_LCSTask* r14, sVec2_S16* r5, s32 r6);
s32 isLCSTargetValid(sLCSTarget* r4);

const std::array<s8, 4> LCS_AccessSoundTable2 =
{
    0,0,1,2
};

const std::array<s32, 4> LCS_AccessSoundTable =
{
    20, 8, 20, 21
};

const std::array<s_LCSTask340Sub::TypedTaskDefinition, 4> s_LCSTask340Sub::constructionTable =
{ {
        { &s_LCSTask340Sub::Init0, &s_LCSTask340Sub::Update0, &s_LCSTask340Sub::Draw, nullptr}, // Access (blue sphere)
        { &s_LCSTask340Sub::Init1, &s_LCSTask340Sub::Update0, &s_LCSTask340Sub::Draw, nullptr}, // Laser (destroy)
        { &s_LCSTask340Sub::Init2, &s_LCSTask340Sub::Update0, &s_LCSTask340Sub::Draw, nullptr}, // Access (2 small lasers from wings)
        { &s_LCSTask340Sub::Init3, &s_LCSTask340Sub::Update3, &s_LCSTask340Sub::Draw, &s_LCSTask340Sub::Delete3},
}};

void LCSTaskDrawSub1Sub2Sub6(void*)
{
    PDS_unimplemented("LCSTaskDrawSub1Sub2Sub6");
}

void createLCSTarget(sLCSTarget* r4, s_workArea* r5, void (*r6)(p_workArea, sLCSTarget*), const sVec3_FP* r7, const sVec3_FP* optionalRotation, s16 flags, s16 argA, eItems receivedItemId, s32 receivedItemQuantity, s32 arg14)
{
    r4->m0 = r5;
    r4->m4_callback = r6;
    r4->m8_LCSWorldCoordinates = r7;
    r4->mC_optionalRotation = optionalRotation;
    r4->m12 = argA;
    r4->m14_receivedItemId = receivedItemId;
    if (r4->m14_receivedItemId >= 0)
    {
        r4->m16_receivedItemQuantity = receivedItemQuantity;
    }
    else
    {
        r4->m16_receivedItemQuantity = 0;
    }
    r4->m10_flags = flags;
    r4->m17 = arg14;
    r4->m18_diableFlags = 0;
    r4->m19 = 0;
    r4->m1A = 0;
    r4->m1B = 0;
    r4->m1C = 0;
    r4->m20 = 0;
}

void updateLCSTarget(sLCSTarget* r14)
{
    sVec3_FP var0_rotation;
    sVec3_FP varC_location;

    s_LCSTask* r13 = getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS;

    r14->m19 &= ~0x30;
    r14->m1C = 0;
    r14->m18_diableFlags &= ~4;
    if (r14->m18_diableFlags & 1)
    {
        return;
    }
    if (r14->mC_optionalRotation)
    {
        //606CF1C
        if (r14->m10_flags & sLCSTarget::e_locationIsWorld)
        {
            //0606CF24
            varC_location = *r14->m8_LCSWorldCoordinates;
        }
        else
        {
            //606CF46
            transformAndAddVecByCurrentMatrix(r14->m8_LCSWorldCoordinates, &varC_location);
        }

        //606CF50
        if (r14->m10_flags & sLCSTarget::e_rotationIsInWorld)
        {
            var0_rotation = *r14->mC_optionalRotation;
        }
        else
        {
            //606CF74
            transformVecByCurrentMatrix(*r14->mC_optionalRotation, var0_rotation);
        }

        //606CF7C
        if (dot3_FP(&varC_location, &var0_rotation) >= 0)
        {
            r14->m18_diableFlags |= 4;
        }
    }
    //606CF96
    assert(r13);

    r13->m10++;

    if (r13->mC < 0x100)
    {
        r13->m14[r13->mC].m0 = r14;
        r13->m14[r13->mC].m4_next = 0;
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

void addToLCSList(s_LCSTask* r4, s_LCSTask340* r5, s_LCSTask340** r6, s_LCSTask340** r7)
{
    if (*r7)
    {
        (*r7)->m4_next = r5;
        *r7 = r5;
    }
    else
    {
        *r7 = r5;
        *r6 = r5;
    }

    r5->m4_next = nullptr;
}

s_LCSTask340* removeFromLCSList(s_LCSTask*, s_LCSTask340* r5, s_LCSTask340** r6, s_LCSTask340** r7)
{
    s_LCSTask340* r14 = nullptr;
    s_LCSTask340* r4 = *r6;
    while (r4)
    {
        if (r4 == r5)
        {
            if (r14)
            {
                r14->m4_next = r4->m4_next;
            }
            else
            {
                *r6 = r4->m4_next;
            }

            if (*r7 == r4)
            {
                *r7 = r14;
            }

            return r4;
        }
        else
        {
            r14 = r4;
            r4 = r4->m4_next;
        }
    }

    return r4;
}

void LCSTaskDrawSub1Sub2Sub2Sub1Sub2SubSub0(sLCSTarget* r4)
{
    if (r4->m20)
    {
        r4->m20->getTask()->markFinished();
    }

    r4->m20 = nullptr;
    r4->m19 &= 0xF0;
    r4->m1A = 0;
}

void LCSTaskDrawSub1Sub2Sub2Sub1Sub2Sub(s_LCSTask* r4, sLCSTarget* r5)
{
    if (r5->m17)
    {
        if (r4->m820_Fewl)
        {
            LCSTaskDrawSub1Sub2Sub2Sub1Sub2SubSub0(r4->m820_Fewl);
        }

        r5->m20 = createLCSShootTask(r4, r5);
        r5->m19 = (r5->m19 & 0xF0) | 5;
        startScript_cantDestroy();
    }

    r4->m81C_curs = r5;
    r4->m8 &= ~0x200;
}

void LCSTaskDrawSub1Sub5(s_LCSTask* r4)
{
    if (r4->m820_Fewl == 0)
        return;
    assert(0);
}

void LCSTaskDrawSub1Sub2Sub3Sub0(s_LCSTask* r4)
{
    if (r4->m820_Fewl)
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

    r4->m83C_time0 = 3;

    switch (r5->m10_flags & 3)
    {
    case 0:
    case 1:
    case 3:
        if (r4->m83E_LaserNum < r5->m17 + 1)
        {
            return LCSTaskDrawSub1Sub2Sub2Sub1Sub2(r4, r5);
        }
        break;
    case 2:
        if (r4->m824 == 0)
            return;

        if (r4->m824 < (r5->m17 - r5->m1A + 1))
        {
            return LCSTaskDrawSub1Sub2Sub2Sub1Sub2(r4, r5);
        }
        break;
    default:
        assert(0);
        break;
    }

    //0606D468
    s_LCSTask340* r12 = removeFromLCSList(r4, r4->m830_unactiveStart, &r4->m830_unactiveStart, &r4->m834_unactiveEnd);
    if (r12 == nullptr)
        return;

    addToLCSList(r4, r12, &r4->m828_activeStart, &r4->m82C_activeEnd);

    r4->m824--;

    r12->m8 = r5;
    r12->mC = nullptr;
    r12->m10 = nullptr;

    r5->m1A++;
    r5->m19 = (r5->m19 & 0xF0) | 1;

    playSystemSoundEffect(0xA);

    LCSTaskDrawSub1Sub5(r4);

    r4->m818_curr = r5;
    r4->m81C_curs = r5;
}

void removeLCSTarget(s_LCSTask* r4, s_LCSTask340* r5)
{
    if (r4->m838_Next == r5)
    {
        r4->m838_Next = r5->m4_next;
    }

    s_LCSTask340* r0 = removeFromLCSList(r4, r5, &r4->m828_activeStart, &r4->m82C_activeEnd);
    if (r0 == nullptr)
        return;

    addToLCSList(r4, r5, &r4->m830_unactiveStart, &r4->m834_unactiveEnd);

    r4->m824++;

    if (isLCSTargetValid(r5->m8))
    {
        r5->m8->m1A--;

        if (r5->m8->m1A == 0)
        {
            LCSTaskDrawSub1Sub2Sub2Sub1Sub2SubSub0(r5->m8);
        }
    }

    if (r4->m818_curr == r5->m8)
    {
        r4->m818_curr = nullptr;
    }

    if (r4->m81C_curs == r5->m8)
    {
        r4->m81C_curs = nullptr;
    }

    r5->m8 = nullptr;
    fieldScriptTaskUpdateSub2Sub1Sub1Sub1Sub2(r5->mC);
    fieldScriptTaskUpdateSub2Sub1Sub1Sub1Sub2(r5->m10);

    r5->mC = nullptr;
    r5->m10 = nullptr;
}

void fieldScriptTaskUpdateSub2Sub1Sub1(s_LCSTask* r4)
{
    s_LCSTask340* r5 = r4->m828_activeStart;
    while (r5)
    {
        s_LCSTask340* next = r5->m4_next;
        removeLCSTarget(r4, r5);
        r5 = next;
    }
}

void LCSTaskDrawSub1Sub2Sub1(s_LCSTask* r4)
{
    for (s32 r11 = 0; r11 < r4->mC; r11++)
    {
        s_LCSTask_14* r12 = &r4->m14[r11];
        sLCSTarget* r14 = r12->m0;
        
        if (r14->m10_flags & sLCSTarget::e_locationIsWorld)
        {
            r14->m24_worldspaceCoordinates = *r14->m8_LCSWorldCoordinates;
            LCSTaskDrawSub3(&r14->m24_worldspaceCoordinates, &r14->m30_screenspaceCoordinates);
        }
        else
        {
            LCSTaskDrawSub4(r14->m8_LCSWorldCoordinates, &r14->m30_screenspaceCoordinates, &r14->m24_worldspaceCoordinates);
        }

        if (r14->m24_worldspaceCoordinates[2] <= 0x3000)
            continue;

        if (r14->m24_worldspaceCoordinates[2] >= r4->m814_LCSTargetMaxDistance)
            continue;

        if (abs(r14->m30_screenspaceCoordinates[0]) >= 176)
            continue;

        if (abs(r14->m30_screenspaceCoordinates[1]) >= 112)
            continue;

        if (r4->m9CC == nullptr)
        {
            r4->m9C8 = r12;
        }
        else
        {
            r4->m9CC->m4_next = r12;
        }

        r12->m4_next = nullptr;
        r4->m9CC = r12;
        r4->m9C4++;
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

        if (!(pLCSTarget->m18_diableFlags & 6) && (pLCSTarget->m24_worldspaceCoordinates[2] > 0x3000) && (pLCSTarget->m24_worldspaceCoordinates[2] < r4->m814_LCSTargetMaxDistance))
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
    s_LCSTask340* r13 = r4->m828_activeStart;

    while (r13)
    {
        sLCSTarget* r14 = r13->m8;
        if (r14->m10_flags & sLCSTarget::e_locationIsWorld)
        {
            r14->m24_worldspaceCoordinates = *r14->m8_LCSWorldCoordinates;
            LCSTaskDrawSub3(&r14->m24_worldspaceCoordinates, &r14->m30_screenspaceCoordinates);
        }
        else
        {
            LCSTaskDrawSub4(r14->m8_LCSWorldCoordinates, &r14->m30_screenspaceCoordinates, &r14->m24_worldspaceCoordinates);
        }

        r13 = r13->m4_next;
    }
}

void LCSTaskDrawSub1Sub2Sub3(s_LCSTask* r4)
{
    s_LCSTask340* r13 = r4->m828_activeStart;

    while (r13)
    {
        s_LCSTask340* r10 = r13->m4_next;
        sLCSTarget* r14 = r13->m8;

        if (getFieldTaskPtr()->m8_pSubFieldData->m370_fieldDebuggerWho & 1)
        {
            assert(0);
        }

        if ((r14->m24_worldspaceCoordinates[2] > 0x3000) &&
            (r14->m24_worldspaceCoordinates[2] < r4->m814_LCSTargetMaxDistance) &&
            (abs(r14->m30_screenspaceCoordinates[0]) < 176) &&
            (abs(r14->m30_screenspaceCoordinates[1]) < 112) &&
            (r14->m18_diableFlags == 0))
        {

        }
        else
        {
            removeLCSTarget(r4, r13);
        }

        r13 = r10;
    }

    LCSTaskDrawSub1Sub2Sub3Sub0(r4);
}

void LCSTaskDrawSub1Sub2Sub5(s_LCSTask* r4)
{
    s_LCSTask340* r5 = r4->m828_activeStart;

    while (r5)
    {
        r5->m8->m1B = r5->m8->m17;
        r5 = r5->m4_next;
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
        if (!(r6->m18_diableFlags & 6) && (r4->m818_curr != r6) && (r6->m1A <= r6->m17) && !(r6->m19 & 4) && (r6->m24_worldspaceCoordinates[2] > 0x3000) && (r6->m24_worldspaceCoordinates[2] < r4->m814_LCSTargetMaxDistance))
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

s32 LCSTaskDrawSub1Sub2Sub2Sub3Sub0(s_LCSTask* r4, sLCSTarget* r5, sVec2_S16* r6)
{
    if (r5->m18_diableFlags & 6)
        return 0;

    if (r5->m24_worldspaceCoordinates[2] <= 0x3000)
        return 0;

    if (r5->m24_worldspaceCoordinates[2] >= r4->m814_LCSTargetMaxDistance)
        return 0;

    if (abs(r5->m30_screenspaceCoordinates[0]) >= 176)
        return 0;

    if (abs(r5->m30_screenspaceCoordinates[1]) >= 112)
        return 0;

    s32 r7 = (*r6)[0] - r5->m30_screenspaceCoordinates[0];
    s32 r4b = (*r6)[1] - r5->m30_screenspaceCoordinates[1];

    if ((r7 * r7 + r4b * r4b) >= 0x100)
    {
        return 0;
    }

    return 1;
}

sLCSTarget* LCSTaskDrawSub1Sub2Sub2Sub3(s_LCSTask* r14)
{
    r14->m83D_time1 = 0;

    s32 r4;
    if (r14->m818_curr)
    {
        if ((r14->m818_curr->m10_flags & 3) == 2)
        {
            if (r14->m8 & 0x200)
            {
                //0606DB6C
                assert(0);
            }
            else
            {
                //0606DB9A
                if (r14->m818_curr->m1A <= r14->m818_curr->m17)
                {
                    if ((r14->m818_curr->m19 & 4) == 0)
                    {
                        r14->m83D_time1 = 1;
                        return r14->m818_curr;
                    }
                }

                if (LCSTaskDrawSub1Sub2Sub2Sub3Sub0(r14, r14->m818_curr, &getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m200_LCSCursorScreenSpaceCoordinates))
                {
                    return NULL;
                }

                r4 = 1;
            }
        }
        else
        {
            //606DBE8
            if(LCSTaskDrawSub1Sub2Sub2Sub3Sub0(r14, r14->m818_curr, &getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m200_LCSCursorScreenSpaceCoordinates))
            {
                r4 = 1;
            }
            else
            {
                return NULL;
            }
        }
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
        if (r14->m818_curr == NULL)
            return r13;

        if ((r14->m818_curr->m10_flags & 3) == 2)
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
        return r14->m818_curr;
    }
}

void LCSUpdateCursorFromAnalogInput(s_dragonTaskWorkArea* r4)
{
    s8 r4_y;
    if (graphicEngineStatus.m4514.m138[1])
    {
        r4_y = -graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m3_analogY;
    }
    else
    {
        r4_y = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m3_analogY;
    }

    if ((r4_y >= 0x30) || (r4_y < -0x30))
    {
        r4->m1F0.m_8 = performDivision(0x7F, -(r4_y * 0x1C71C7));
    }

    s32 r4_x = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m2_analogX;
    if ((r4_x >= 0x30) || (r4_x < -0x30))
    {
        r4->m1F0.m_C = performDivision(0x7F, r4_x * 0x1C71C7);
    }

    sVec3_FP var0;
    var0[0] = 0;
    var0[1] = MTH_Mul(0x147A, r4->m1F0.m_C);
    var0[2] = 0;

    if (r4->mF8_Flags & 0x8000)
    {
        LCSUpdateCursorFromInputSub0(0, &var0);
    }
}

void LCSUpdateCursorFromInput(s_dragonTaskWorkArea* r4)
{
    if (graphicEngineStatus.m4514.m0_inputDevices->m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][5]) // down
    {
        r4->m1F0.m_8 = fixedPoint(0x1C71C7);
    }
    else if (graphicEngineStatus.m4514.m0_inputDevices->m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][4]) // up
    {
        r4->m1F0.m_8 = -fixedPoint(0x1C71C7);
    }

    if (graphicEngineStatus.m4514.m0_inputDevices->m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][7])
    {
        r4->m1F0.m_C = -fixedPoint(0x1C71C7);
    }
    else if (graphicEngineStatus.m4514.m0_inputDevices->m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][6])
    {
        r4->m1F0.m_C = fixedPoint(0x1C71C7);
    }

    sVec3_FP var0;
    var0[0] = 0;
    var0[1] = MTH_Mul(0x147A, r4->m1F0.m_C);
    var0[2] = 0;

    if (r4->mF8_Flags & 0x8000)
    {
        LCSUpdateCursorFromInputSub0(0, &var0);
    }
}

void LCSTaskDrawSub1Sub2Sub2Sub2(s_dragonTaskWorkArea* r4)
{
    getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1F0.m_8 = 0;
    getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1F0.m_C = 0;

    switch (graphicEngineStatus.m4514.m0_inputDevices->m0_current.m0_inputType)
    {
    case 1:
        LCSUpdateCursorFromInput(r4);
        break;
    case 2:
        LCSUpdateCursorFromAnalogInput(r4);
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

    if (--r4->m83C_time0 <= 0)
    {
        r4->m83C_time0 = 0;
        LCSTaskDrawSub1Sub2Sub2Sub1(r4, LCSTaskDrawSub1Sub2Sub2Sub3(r4));
    }

    return 1;
}

static const std::array<s8, 10> LCSTaskDrawSub1Sub2Sub0Sub1Sub0Sub0Data1 =
{
    2,1,0,2,1,0,2,1,0,3
};

s8 LCSTaskDrawSub1Sub2Sub0Sub1Sub0Sub0(sLCSTarget* r4)
{
    if (r4->m14_receivedItemId > -1)
    {
        return LCSTaskDrawSub1Sub2Sub0Sub1Sub0Sub0Data1[getObjectListEntry(r4->m14_receivedItemId)->m1_type];
    }

    return r4->m14_receivedItemId;
}

void LCSTaskDrawSub1Sub2Sub0Sub1Sub0(s_LCSTask* r4, sLCSTarget* r5, s32 r6)
{
    sLaserArgs var8;
    var8.m0 = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    var8.m4 = nullptr;
    var8.m14 = r5;
    var8.mC = r5->m8_LCSWorldCoordinates;
    var8.m8 = r5->m10_flags;
    var8.m10 = nullptr;
    var8.m18 = nullptr;
    var8.m1C_receivedItemId = r5->m14_receivedItemId;
    var8.m1E_receivedItemQuantity = r5->m16_receivedItemQuantity;

    if (var8.m1C_receivedItemId >= 0)
    {
        var8.m1F = LCSTaskDrawSub1Sub2Sub0Sub1Sub0Sub0(r5);
    }
    else
    {
        static const std::array<s8, 12> LCSTaskDrawSub1Sub2Sub0Sub1Sub0Data0 =
        {
            5,4,5,6,0,1,2,3,4,5,6,0
        };
        var8.m1F = LCSTaskDrawSub1Sub2Sub0Sub1Sub0Data0[r6 - 1];
    }

    LCSTaskDrawSub1Sub2Sub0Sub2Sub0(r4, &var8, 3);
    playSystemSoundEffect(0x15);
}

void LCSTaskDrawSub1Sub2Sub0Sub1(s_LCSTask* r4)
{
    s_LCSTask340* r13 = r4->m828_activeStart;

    while (r13)
    {
        if (r13->m14 & 1)
        {
            sLCSTarget* r14 = r13->m8;
            r14->m19 |= 2;
            removeLCSTarget(r4, r13);
            r14->m1B--;
            if ((r14->m1B < 0) || ((r14->m10_flags & 3) != 2))
            {
                r14->m19 |= 0x20;
            }

            if (r14->m4_callback)
            {
                r14->m4_callback(r14->m0, r14);
            }

            //606E0A6
            if (r14->m19 & 0x20)
            {
                if ((r14->m14_receivedItemId >= 0) && (r14->m16_receivedItemQuantity > 0))
                {
                    LCSTaskDrawSub1Sub2Sub0Sub1Sub0(r4, r14, 0);
                }
                else
                {
                    s32 r6 = (r14->m10_flags >> 4) & 0xF;
                    if (r6)
                    {
                        LCSTaskDrawSub1Sub2Sub0Sub1Sub0(r4, r14, r6);
                    }
                }
            }

            //606E0E0
            r13->m14 &= ~1;
        }
        else
        {
            //606E0EC
            if (r13->m14 & 2)
            {
                removeLCSTarget(r4, r13);
                r13->m14 &= ~2;
            }
        }
        
        r13 = r13->m4_next;
    }
}

void LCSTaskDrawSub1Sub2Sub0Sub2(s_LCSTask* r14)
{
    if (--r14->m83D_time1 > 0)
        return;

    r14->m83D_time1 = 0;

    s_LCSTask340* r12 = r14->m838_Next;
    if (r12)
    {
        sLCSTarget* r13 = r12->m8;

        sLaserArgs laserArgs;

        laserArgs.m0 = r13->m0;
        laserArgs.m4 = r13->m8_LCSWorldCoordinates;
        laserArgs.m8 = r13->m10_flags;
        laserArgs.m10 = &getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m160_deltaTranslation;
        laserArgs.m14 = r13;
        laserArgs.m18 = r12;
        laserArgs.m1F = 0;

        if (LCS_AccessSoundTable2[r14->m838_Next->m8->m10_flags & 3] == 2)
        {
            //0606E184
            // Shoot lasers from both wings
            laserArgs.mC = &getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m118_hotSpot3;
            r14->m838_Next->mC = LCSTaskDrawSub1Sub2Sub0Sub2Sub0(r14, &laserArgs, 2);
            laserArgs.mC = &getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m124_hotSpot4;
            r14->m838_Next->m10 = LCSTaskDrawSub1Sub2Sub0Sub2Sub0(r14, &laserArgs, 2);
        }
        else
        {
            // Shoot single laser from head
            laserArgs.mC = &getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m10C_hotSpot2;
            r14->m838_Next->mC = LCSTaskDrawSub1Sub2Sub0Sub2Sub0(r14, &laserArgs, LCS_AccessSoundTable2[r14->m838_Next->m8->m10_flags & 3]);
        }

        //0606E1DA
        r14->m838_Next = r14->m838_Next->m4_next;
        r14->m83D_time1 = 5;
        r13->m19 = (r13->m19 & 0xF0) | 2;
    }
}

void setLCSField83E(s_LCSTask* pLCS, s32 value)
{
    pLCS->m83E_LaserNum = std::max(std::min(value, 0x10), 1);

    s_LCSTask340* r5;
    for (int r1 = 0; r1 < pLCS->m83E_LaserNum; r1++)
    {
        r5 = &pLCS->m840[r1];
        s_LCSTask340* r3 = &pLCS->m840[r1 + 1];

        r5->m4_next = r3;
        if (r5->mC)
        {
            assert(0);
        }
        if (r5->m10)
        {
            assert(0);
        }
        r5->m8 = 0;
        r5->mC = nullptr;
        r5->m10 = nullptr;
        r5->m14 = 0;
    }
    r5->m4_next = nullptr;
    pLCS->m824 = pLCS->m83E_LaserNum;
    pLCS->m82C_activeEnd = nullptr;
    pLCS->m828_activeStart = nullptr;
    pLCS->m830_unactiveStart = &pLCS->m840[0];
    pLCS->m834_unactiveEnd = &pLCS->m840[pLCS->m83E_LaserNum - 1];
    pLCS->m838_Next = 0;
    pLCS->m818_curr = nullptr;
    pLCS->m81C_curs = nullptr;
}

void s_LCSTask::Init(s_LCSTask* pTypedWorkArea)
{
    getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS = pTypedWorkArea;
    getMemoryArea(&pTypedWorkArea->m0, 0);
    pTypedWorkArea->m814_LCSTargetMaxDistance = 0x200000;
    if (gDragonState->mC_dragonType == 8)
    {
        setLCSField83E(pTypedWorkArea, 1);
    }
    else
    {
        setLCSField83E(pTypedWorkArea, gDragonState->mC_dragonType + 1);
    }

    pTypedWorkArea->m9C0 = static_cast<s_LCSTask_gradientData*>(allocateHeapForTask(pTypedWorkArea, sizeof(s_LCSTask_gradientData)));
}

void LCSTaskDrawSub1Sub2Sub0(s_LCSTask* r4)
{
    if (!(r4->m8 & 8))
        return;

    LCSTaskDrawSub1Sub2Sub4(r4);
    LCSTaskDrawSub1Sub2Sub0Sub1(r4);
    LCSTaskDrawSub1Sub2Sub0Sub2(r4);

    // TODO: recheck that test
    if (r4->m828_activeStart)
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

    if (graphicEngineStatus.m4514.m0_inputDevices->m0_current.m8_newButtonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][2])
    {
        r4->m9D0_mode++;
    }
    else
    {
        r4->m9D0_mode = 0;
    }

    switch (r4->m9DA_LCSPhase)
    {
    case s_LCSTask::LCSPhase_0_init:
        if (r4->m9D0_mode == 0)
            return;
        r4->m9DA_LCSPhase = s_LCSTask::LCSPhase_2_targeting;
        break;
    case s_LCSTask::LCSPhase_1:
        if (r4->m9D0_mode >= 24)
        {
            r4->m9DA_LCSPhase = s_LCSTask::LCSPhase_3;
            return;
        }
        if (!(graphicEngineStatus.m4514.m0_inputDevices->m0_current.mA  & graphicEngineStatus.m4514.mD8_buttonConfig[1][2]))
            return;
        r4->m9DA_LCSPhase = s_LCSTask::LCSPhase_2_targeting;
        break;
    case s_LCSTask::LCSPhase_2_targeting: // LCS targeting
        if (r4->m83D_time1)
            return;
        // cancel LCS?
        if (graphicEngineStatus.m4514.m0_inputDevices->m0_current.m8_newButtonDown & 6)
        {
            r4->m9DA_LCSPhase = s_LCSTask::LCSPhase_4_executing;
        }
        break;
    case s_LCSTask::LCSPhase_3:
        if (graphicEngineStatus.m4514.m0_inputDevices->m0_current.mA  & graphicEngineStatus.m4514.mD8_buttonConfig[1][2])
        {
            r4->m9DA_LCSPhase = s_LCSTask::LCSPhase_4_executing;
        }
        break;
    case s_LCSTask::LCSPhase_4_executing:
        break;
    default:
        assert(0);
        break;
    }
}

void LCSTaskDrawSub1Sub0(s_LCSTask* r4)
{
    s_LCSTask340* r13 = r4->m828_activeStart;

    while (r13)
    {
        s_LCSTask340* r12 = r13->m4_next;

        if (isLCSTargetValid(r13->m8) == 0)
        {
            removeLCSTarget(r4, r13);
        }

        r13 = r12;
    }

    if (r4->m820_Fewl)
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

    r4->m83D_time1 = 0;

    LCSTaskDrawSub1Sub2Sub4(r4);
    LCSTaskDrawSub1Sub2Sub5(r4);

    r4->m838_Next = r4->m828_activeStart;

    if (r4->m828_activeStart)
    {
        r4->m8 |= 0x10;

        s8 r13 = LCS_AccessSoundTable2[r4->m838_Next->m8->m10_flags & 3];

        playSystemSoundEffect(LCS_AccessSoundTable[r13]);

        if (r13 == 1)
        {
            LCSTaskDrawSub1Sub2Sub6(r4->m9C0);
        }
    }
}

void LCSTaskDrawSub1(s_LCSTask* r4)
{
    LCSTaskDrawSub1Sub0(r4);

    if (r4->m8)
    {
        fieldTaskPtr->m28_status |= 0x4;
        if (r4->m8 & 0x40)
        {
            return;
        }
    }
    else
    {
        fieldTaskPtr->m28_status &= ~0x4;
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
        if (--r4->m83C_time0 <= 0)
        {
            r4->m83C_time0 = 0;
        }

        if ((r4->m83C_time0 == 0) && (r4->m9DA_LCSPhase))
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
        if (!(r4->m8 & 4) && (graphicEngineStatus.m4514.m0_inputDevices->m0_current.m8_newButtonDown & 1))
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
        bool test0 = r4->m83F_activeLaserCount || getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m38_dialogStringTask;
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

        r4->m83C_time0 = 0xF;
        r4->m8 = 0;
    }
}

void LCSTaskDrawSub5(s_LCSTask* r4)
{
    if (r4->m8 & 2)
    {
        if (!r4->m828_activeStart || (r4->m818_curr && ((r4->m818_curr->m10_flags & 3) == 2)))
        {
            DrawLCSTarget(r4, &getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m200_LCSCursorScreenSpaceCoordinates, 0);
        }
        else
        {
            DrawLCSTarget(r4, &getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m200_LCSCursorScreenSpaceCoordinates, 1);
        }
    }

    //606E894
    s_LCSTask340* r13 = r4->m828_activeStart;
    while (r13)
    {
        if (r13->m8->m20 == 0)
        {
            if ((r13->m8->m19 & 2) == 0)
            {
                r13->m8->m20 = createLCSSelectedTask(r4, r13->m8);
            }
        }

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
        if (r14->m10_flags & sLCSTarget::e_locationIsWorld)
        {
            r14->m24_worldspaceCoordinates = *r14->m8_LCSWorldCoordinates;
            LCSTaskDrawSub3(&r14->m24_worldspaceCoordinates, &r14->m30_screenspaceCoordinates);
        }
        else
        {
            LCSTaskDrawSub4(r14->m8_LCSWorldCoordinates, &r14->m30_screenspaceCoordinates, &r14->m24_worldspaceCoordinates);
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

s32 isLCSTargetValid(sLCSTarget* r4)
{
    if (r4->m0 == nullptr)
        return 0;

    if (r4->m0->getTask()->isFinished())
        return 0;

    return 1;
}

void dragonFieldTaskUpdateSub2(u32 r4)
{
    setLCSField83E(getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS, r4);
}

void createFieldOverlaySubTask2(s_workArea* pWorkArea)
{
    createSubTask<s_LCSTask>(pWorkArea);
}

void LCSTaskDrawSub()
{
    if (!getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->getTask()->isPaused())
    {
        LCSTaskDrawSubSub(getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS);
    }
}

void sLCSSelectedSub::Update(sLCSSelectedSub* pThis)
{
    if (--pThis->mC_numFrames == 0)
    {
        pThis->getTask()->markFinished();
    }
}

const std::array<u16, 10> LCSSelectedSubTask_DrawData0 =
{
    0xE484,
    0xE4E7,
    0xE94A,
    0xE9AD,
    0xEE10,
    0xEE73,
    0xF2D6,
    0xF739,
    0xFB9C,
    0xFFFF,
};

const std::array<fixedPoint, 10> LCSSelectedSubTask_DrawData1 =
{
    0x10000,
    0xE666,
    0xCCCC,
    0xB333,
    0x9999,
    0x8000,
    0x6666,
    0x4CCC,
    0x3333,
    0x1999,
};

void sLCSSelectedSub::Draw(sLCSSelectedSub* pThis)
{
    s32 var0 = LCSSelectedSubTask_DrawData0[pThis->mC_numFrames];

    std::array<s16, 4> varC;
    getVdp1ClippingCoordinates(varC);

    s32 r11 = LCSSelectedSubTask_DrawData1[pThis->mC_numFrames];

    s16 var4[4];
    var4[0] = MTH_Mul(-176, r11) - 176;
    var4[1] = varC[1] + 112;
    var4[2] = varC[2] - 176;
    var4[3] = 112 - varC[3];

    s16 var14[4];
    var14[0] = MTH_Mul((*pThis->m8)[0] - var4[0], r11) + var4[0];
    var14[1] = MTH_Mul((*pThis->m8)[1] - var4[1], r11) + var4[1];
    var14[2] = MTH_Mul((*pThis->m8)[0] - var4[2], r11) + var4[2];
    var14[3] = MTH_Mul((*pThis->m8)[1] - var4[3], r11) + var4[3];

    s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
    vdp1WriteEA.m0_CMDCTRL = 0x1005;
    vdp1WriteEA.m4_CMDPMOD = 0x4C0;
    vdp1WriteEA.m6_CMDCOLR = var0;
    vdp1WriteEA.mC_CMDXA = var14[0];
    vdp1WriteEA.mE_CMDYA = -var14[1];
    vdp1WriteEA.m10_CMDXB = var14[2];
    vdp1WriteEA.m12_CMDYB = -var14[1];
    vdp1WriteEA.m14_CMDXC = var14[2];
    vdp1WriteEA.m16_CMDYC = -var14[3];
    vdp1WriteEA.m18_CMDXD = var14[0];
    vdp1WriteEA.m1A_CMDYD = -var14[3];

    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

    graphicEngineStatus.m14_vdp1Context[0].m1C++;
    graphicEngineStatus.m14_vdp1Context[0].mC++;
}

void sLCSSelected::UpdateSub0(sVec2_S16* r5)
{
    sLCSSelectedSub* r14 = createSubTask<sLCSSelectedSub>(this);

    getMemoryArea(&r14->m0, 0);
    r14->m8 = r5;
    r14->mC_numFrames = 0xA;
}

void sLCSSelected::Update(sLCSSelected* pThis)
{
    switch (pThis->m2D)
    {
    case 0:
        pThis->m18_secondarySpriteInterpolator.m4_currentValue += pThis->m18_secondarySpriteInterpolator.m0_step;
        pThis->mC_verticalInterpolator.m4_currentValue += pThis->mC_verticalInterpolator.m0_step;

        if (pThis->mC_verticalInterpolator.m4_currentValue >= pThis->mC_verticalInterpolator.m8_targetValue)
        {
            pThis->mC_verticalInterpolator.m4_currentValue = pThis->mC_verticalInterpolator.m8_targetValue;
            pThis->m18_secondarySpriteInterpolator.m4_currentValue = pThis->m18_secondarySpriteInterpolator.m8_targetValue;
            pThis->m2D++;
        }
    case 1:
        if (isLCSTargetValid(pThis->m8))
        {
            if (getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m8 & 2)
            {
                if (pThis->m8->m1A > pThis->m2E)
                {
                    pThis->UpdateSub0(&pThis->m8->m30_screenspaceCoordinates);
                }
            }

            //606ED02
            pThis->m2E = pThis->m8->m1A;
            if (pThis->m8->m1A == 0)
            {
                pThis->m2D++;
            }
        }
        else
        {
            pThis->m2D++;
        }
        break;
    case 2:
        pThis->m18_secondarySpriteInterpolator.m4_currentValue -= pThis->m18_secondarySpriteInterpolator.m0_step;
        pThis->mC_verticalInterpolator.m4_currentValue -= pThis->mC_verticalInterpolator.m0_step;
        if (pThis->mC_verticalInterpolator.m4_currentValue <= 0)
        {
            pThis->mC_verticalInterpolator.m4_currentValue = 0;
            pThis->m18_secondarySpriteInterpolator.m4_currentValue = 0;
            pThis->getTask()->markFinished();
        }
        break;
    default:
        assert(0);
        break;
    }

    pThis->m28++;
}

void sLCSSelected::DrawSub0(sLCSTaskDrawSub5Sub1_Data1* r5, const sInterpolator_FP& r6_interpolator)
{
    s32 CMDSRCA = ((m0.m4_characterArea - (0x25C00000)) >> 3) + r5->m6_CMDSRCA;
    s32 CMDCOLR = ((m0.m4_characterArea - (0x25C00000)) >> 3) + r5->mA_CMDCOLR;
    s32 width = r5->mC_spriteWidth >> 12;
    s32 offsetX = r5->m14_offsetX >> 12;
    s32 height = r5->m10_spriteHeight >> 12;
    s32 offsetY = r5->m18_offsetY >> 12;
    s32 interpolatedY = r6_interpolator.m4_currentValue >> 12;

    assert(height >= 0); // because of the addc

    s16 var14[4];
    var14[0] = offsetX + (*m24_screenLocation)[0];
    var14[1] = (offsetY + (*m24_screenLocation)[1] - height / 2) + interpolatedY;
    var14[2] = offsetX + (*m24_screenLocation)[0] + width;
    var14[3] = (offsetY + (*m24_screenLocation)[1] - height / 2) - interpolatedY;

    s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
    vdp1WriteEA.m0_CMDCTRL = r5->m2;
    vdp1WriteEA.m4_CMDPMOD = r5->m4;
    vdp1WriteEA.m6_CMDCOLR = CMDCOLR;
    vdp1WriteEA.m8_CMDSRCA = CMDSRCA;
    vdp1WriteEA.mA_CMDSIZE = r5->m8;
    vdp1WriteEA.mC_CMDXA = var14[0];
    vdp1WriteEA.mE_CMDYA = -var14[1];
    vdp1WriteEA.m14_CMDXC = var14[2];
    vdp1WriteEA.m16_CMDYC = -var14[3];

    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

    graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
    graphicEngineStatus.m14_vdp1Context[0].mC += 1;
}

void sLCSSelected::DrawSub1(s8 r5)
{
    PDS_unimplemented("sLCSSelected::DrawSub1");
}

void sLCSSelected::Draw(sLCSSelected* pThis)
{
    pThis->DrawSub0(&LCSTaskDrawSub5Sub1_Data1[pThis->m2C], pThis->mC_verticalInterpolator);
    if (pThis->m28 & 4)
    {
        pThis->DrawSub0(&LCSTaskDrawSub5Sub1_Data1[pThis->m2C + 1], pThis->m18_secondarySpriteInterpolator);
    }

    if (pThis->m2C != 6)
        return;
    if (pThis->m2D != 1)
        return;
    if (pThis->m8->m1A <= 1)
        return;

    pThis->DrawSub1(pThis->m8->m1A);
}

void DrawLCSTarget(s_LCSTask* r14, sVec2_S16* r5, s32 r6)
{
    sSaturnPtr dataR6 = sSaturnPtr({ 0x6093B28, gFLD_A3 }) + 0x1C * r6;

    s32 r0 = (r14->m0.m4_characterArea - (0x25C00000)) >> 3;
    s32 var8 = ((r14->m0.m4_characterArea - (0x25C00000)) >> 3) + readSaturnS16(dataR6 + 6);
    s32 var0 = ((r14->m0.m4_characterArea - (0x25C00000)) >> 3) + readSaturnS16(dataR6 + 0xA);
    s32 var4 = readSaturnS32(dataR6 + 0xC) >> 12;
    s32 varC = readSaturnS32(dataR6 + 0x10) >> 12;

    s32 r4 = readSaturnS32(dataR6 + 0x14) >> 12;
    s32 r2 = readSaturnS32(dataR6 + 0x18) >> 12;

    s16 var10[4];
    var10[0] = r4 + (*r5)[0];
    var10[1] = r2 + (*r5)[1];
    var10[2] = r4 + (*r5)[0] + var4;
    var10[3] = r2 + (*r5)[1] - varC;

    s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
    vdp1WriteEA.m0_CMDCTRL = readSaturnS16(dataR6 + 2);
    vdp1WriteEA.m4_CMDPMOD = readSaturnS16(dataR6 + 4);
    vdp1WriteEA.m6_CMDCOLR = var0;
    vdp1WriteEA.m8_CMDSRCA = var8;
    vdp1WriteEA.mA_CMDSIZE = readSaturnS16(dataR6 + 8);
    vdp1WriteEA.mC_CMDXA = var10[0];
    vdp1WriteEA.mE_CMDYA = -var10[1];
    vdp1WriteEA.m14_CMDXC = var10[2];
    vdp1WriteEA.m16_CMDYC = -var10[3];

    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

    graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
    graphicEngineStatus.m14_vdp1Context[0].mC += 1;
}

static const std::array<s8, 4> LCSTaskDrawSub5Sub1_Data0 = {
    4,8,6,2
};

p_workArea createLCSSelectedTask(s_LCSTask* r4, sLCSTarget* r5)
{
    sLCSSelected* r14 = createSubTask<sLCSSelected>(r4);

    getMemoryArea(&r14->m0, 0);

    r14->m8 = r5;

    s8 r13 = LCSTaskDrawSub5Sub1_Data0[r5->m10_flags & 3];

    s32 r2 = LCSTaskDrawSub5Sub1_Data1[r13].m10_spriteHeight;
    r14->mC_verticalInterpolator.m8_targetValue = r2 / 2;
    r14->mC_verticalInterpolator.m0_step = performDivision(6, r14->mC_verticalInterpolator.m8_targetValue);

    s32 r3 = LCSTaskDrawSub5Sub1_Data1[r13+1].m10_spriteHeight;
    r14->m18_secondarySpriteInterpolator.m8_targetValue = r3 / 2;
    r14->m18_secondarySpriteInterpolator.m0_step = performDivision(6, r14->m18_secondarySpriteInterpolator.m8_targetValue);

    r14->m24_screenLocation = &r5->m30_screenspaceCoordinates;
    r14->m2C = r13;
    r14->m2E = 0;

    return r14;
}

void allocateLCSEntry(s_visibilityGridWorkArea* r4, sProcessed3dModel* r5, fixedPoint r6)
{
    r4->m12E4_numCollisionGeometries++;

    if (r4->m12E4_numCollisionGeometries >= 24)
    {
        assert(0);
        return;
    }

    r4->m44[0].m0_model = r5;
    r4->m44[0].m34 = r6;
    r4->m44++;
}
