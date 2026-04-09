#include "PDS.h"
#include "fieldCutsceneTask2.h"
#include "field_a3/o_fld_a3.h"

void cutsceneTaskInitSub2Sub2(s_workArea* r4)
{
    if (r4)
    {
        r4->getTask()->markFinished();
    }

    if (getFieldTaskPtr()->m8_pSubFieldData)
    {
        getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m48_cutsceneTask = NULL;
    }
}

void s_cutsceneTask2::Init(s_cutsceneTask2* pThis, std::vector<s_scriptData1>* argument)
{
    getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m48_cutsceneTask = pThis;
    pThis->m4 = argument;
}

s32 s_cutsceneTask2::UpdateSub0()
{
    m3C = &(*m4)[m38];
    m40 = m3C->m0;
    if (m40 == 0)
    {
        return -1;
    }

    m44 = m3C->m1C - m3C->m4;
    m50 = m3C->m28 - m3C->m10;

    m44[0] = intDivide(m40, m44[0]);
    m44[1] = intDivide(m40, m44[1]);
    m44[2] = intDivide(m40, m44[2]);

    m50[0] = intDivide(m40, m50[0]);
    m50[1] = intDivide(m40, m50[1]);
    m50[2] = intDivide(m40, m50[2]);

    m5C = intDivide(m40, fixedPoint(m38 - m34).normalized());

    m8 = m3C->m4;
    m28 = m3C->m10;
    m20 = m3C->m34;

    if (m24)
    {
        m14 = m28 + *m24;
    }
    else
    {
        m14 = m28;
    }

    return 1;
}

void s_cutsceneTask2::UpdateSub1()
{
    getTask()->markFinished();

    if (getFieldTaskPtr()->m8_pSubFieldData)
    {
        getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m48_cutsceneTask = NULL;
        getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m5C = 0;
        if (getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m78)
        {
            return;
        }

        if (m0 & 2)
        {
            endCutsceneCameraWithRestore();
        }
        else
        {
            endCutsceneCameraNoRestore();
        }
    }
}

void s_cutsceneTask2::Update(s_cutsceneTask2* pThis)
{
    switch (pThis->m34)
    {
    case 0:
        pThis->m34 = pThis->UpdateSub0();
        startCutsceneCameraTracking(&pThis->m8, &pThis->m14);
        getFieldCameraStatus()->mC_rotation[2] = pThis->m20;
        return;
    case 1:
        pThis->m8 += pThis->m44;
        pThis->m28 += pThis->m50;
        pThis->m20 += pThis->m5C;

        getFieldCameraStatus()->mC_rotation[1] = pThis->m20;
        pThis->m60++;

        if (pThis->m24)
        {
            //0606A12C
            assert(0);
        }
        else
        {
            pThis->m14 = pThis->m28;
        }

        if (--pThis->m40)
        {
            return;
        }

        if (++pThis->m38 < 0x10)
        {
            pThis->m34 = pThis->UpdateSub0();
        }
        else
        {
            pThis->m34 = -1;
        }
        break;
    default:
        if (getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m5C)
            return;
        pThis->UpdateSub1();
        break;
    }
}

void s_cutsceneTask2::Draw(s_cutsceneTask2*)
{
    PDS_unimplemented("s_cutsceneTask2::Draw");
}

static void cutsceneTaskInitSub2Sub1(std::vector<s_scriptData1>& r4, std::vector<s_scriptData1>& r5)
{
    r5 = r4;
}

void cutsceneTaskInitSub2(p_workArea r4, std::vector<s_scriptData1>& r11, s32 r6, sVec3_FP* r7, u32 arg0)
{
    s_fieldScriptWorkArea* r14 = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE;

    r14->m80 = r4;
    r14->m7C = r6;
    r14->m84 = r7;

    if (r14->m78)
    {
        startCutsceneCameraTracking(&r14->m78->m3C, &r14->m78->m48);
        r14->m48_cutsceneTask = r4;
        return;
    }

    if (r11.size() == 0)
        return;

    cutsceneTaskInitSub2Sub1(r11, *r14->m88);

    if (r14->m48_cutsceneTask)
    {
        cutsceneTaskInitSub2Sub2(r14->m48_cutsceneTask);
    }

    s_cutsceneTask2* pNewTask = createSiblingTaskWithArg<s_cutsceneTask2>(r4, &r11);

    pNewTask->m0 = arg0;
    if (arg0 & 1)
    {
        pNewTask->m24 = r7;
    }

    pNewTask->Update(pNewTask);
}
