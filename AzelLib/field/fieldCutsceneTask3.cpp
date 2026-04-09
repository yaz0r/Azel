#include "PDS.h"
#include "fieldCutsceneTask3.h"
#include "field_a3/o_fld_a3.h"

// TODO: this is shared between s_cutsceneTask2 and 3
void s_cutsceneTask3::UpdateSub1()
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

void s_cutsceneTask3::Init(s_cutsceneTask3* pThis)
{
    getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m48_cutsceneTask = pThis;
}

s32 s_cutsceneTask3::UpdateSub0()
{
    m34 = &(*m4)[m30];
    m38 = m34->m0;
    if (m38 == 0)
    {
        return -1;
    }

    m3C[0] = fixedPoint(m34->m14 - m34->m4).normalized();
    m3C[1] = fixedPoint(m34->m18 - m34->m8).normalized();
    m3C[2] = fixedPoint(m34->m1C - m34->mC).normalized();
    m48 = m34->m20 - m34->m10;

    m3C[0] = intDivide(m38, m3C[0]);
    m3C[1] = intDivide(m38, m3C[1]);
    m3C[2] = intDivide(m38, m3C[2]);
    m48 = intDivide(m38, m48);

    m1C[0] = m34->m4;
    m1C[1] = m34->m8;
    m1C[2] = m34->mC;
    m28 = m34->m10;

    m8[0] = (*m14)[0] + MTH_Mul_5_6(m28, getCos(m1C[0].getInteger() & 0xFFF), getSin(m1C[1].getInteger() & 0xFFF));
    m8[1] = (*m14)[1] - MTH_Mul(m28, getSin(m1C[0].getInteger() & 0xFFF));
    m8[2] = (*m14)[2] + MTH_Mul_5_6(m28, getCos(m1C[0].getInteger() & 0xFFF), getCos(m1C[1].getInteger() & 0xFFF));

    return 1;
}

void s_cutsceneTask3::Update(s_cutsceneTask3* pThis)
{
    switch (pThis->m2C)
    {
    case 0:
        pThis->m2C = pThis->UpdateSub0();
        if (pThis->m18)
        {
            startCutsceneCameraTracking(&pThis->m8, pThis->m18);
        }
        else
        {
            startCutsceneCameraTracking(&pThis->m8, pThis->m14);
        }
        getFieldCameraStatus()->mC_rotation[2] = pThis->m1C[2];
        return;
    case 1:
        pThis->m1C += pThis->m3C;
        getFieldCameraStatus()->mC_rotation[2] = pThis->m1C[2];
        pThis->m8[0] = (* pThis->m14)[0] + MTH_Mul_5_6(pThis->m28, getCos(pThis->m1C[0].getInteger() & 0xFFF), getSin(pThis->m1C[1].getInteger() & 0xFFF));
        pThis->m8[1] = (* pThis->m14)[1] - MTH_Mul(pThis->m28, getSin(pThis->m1C[0].getInteger() & 0xFFF));
        pThis->m8[2] = (* pThis->m14)[2] + MTH_Mul_5_6(pThis->m28, getCos(pThis->m1C[0].getInteger() & 0xFFF), getCos(pThis->m1C[1].getInteger() & 0xFFF));
        pThis->m4C++;
        // end of key frame?
        if (--pThis->m38 != 0)
        {
            return;
        }
        // change key frame
        if (++pThis->m30 >= 0x10)
        {
            pThis->m2C = -1;
        }
        else
        {
            pThis->m2C = pThis->UpdateSub0();
        }
        break;
    default:
        if (!getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m5C)
        {
            pThis->UpdateSub1();
        }
        break;
    }
}

// 0606A8A4
void s_cutsceneTask3::Draw(s_cutsceneTask3* pThis)
{
    s_scriptData2* pData = &(*pThis->m4)[pThis->m30];
    s_FieldSubTaskWorkArea* pSubFieldData = getFieldTaskPtr()->m8_pSubFieldData;

    if (!(pSubFieldData->m370_fieldDebuggerWho & 1))
        return;
    if (pSubFieldData->m37C_debugMenuStatus1[1] != 0)
        return;
    if (pSubFieldData->m369 != 0)
        return;

    vdp2PrintStatus.m10_palette = 0x7000;
    vdp2DebugPrintSetPosition(3, 0xF);
    vdp2PrintfSmallFont("NODE NO:%4d         ", pThis->m30);
    vdp2DebugPrintSetPosition(3, 0x10);
    vdp2PrintfSmallFont("FRAME  :%4d         ", pData->m0);
    vdp2DebugPrintSetPosition(3, 0x11);
    vdp2PrintfSmallFont("ANG S  :%4d %4d %4d ",
        (s16)((u16)(pData->m4 >> 16) & 0xFFF) * 0x168 >> 0xC,
        (s16)((u16)(pData->m8 >> 16) & 0xFFF) * 0x168 >> 0xC,
        (s16)((u16)(pData->mC >> 16) & 0xFFF) * 0x168 >> 0xC);
    vdp2DebugPrintSetPosition(3, 0x12);
    vdp2PrintfSmallFont("DST S  :%4d         ", (s32)pData->m10 >> 0xC);
    vdp2DebugPrintSetPosition(3, 0x13);
    vdp2PrintfSmallFont("ANG E  :%4d %4d %4d ",
        (s16)((u16)(pData->m14 >> 16) & 0xFFF) * 0x168 >> 0xC,
        (s16)((u16)(pData->m18 >> 16) & 0xFFF) * 0x168 >> 0xC,
        (s16)((u16)(pData->m1C >> 16) & 0xFFF) * 0x168 >> 0xC);
    vdp2DebugPrintSetPosition(3, 0x14);
    vdp2PrintfSmallFont("DST E  :%4d         ", (s32)pData->m20 >> 0xC);
    vdp2PrintStatus.m10_palette = 0xD000;
    vdp2DebugPrintSetPosition(3, 0x16);
    vdp2PrintfSmallFont("count (%3d) ", pThis->m4C);
}

static void cutsceneTaskInitSub3Sub0(std::vector<s_scriptData2>& r4, std::vector<s_scriptData2>& r5)
{
    r5 = r4;
}

void cutsceneTaskInitSub3(p_workArea r4, std::vector<s_scriptData2>& r11, s32 r6, sVec3_FP* r7, u32 arg0)
{
    s_fieldScriptWorkArea* r14 = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE;
    r14->m80 = r4;
    r14->m7C = r6;
    r14->m84 = r7;

    if (r14->m78)
    {
        startCutsceneCameraTracking(&r14->m78->m3C, r7);
        r14->m48_cutsceneTask = r4;
        return;
    }

    if (r11.size() == 0)
        return;

    cutsceneTaskInitSub3Sub0(r11, *r14->m8C);

    if (r7 == NULL)
        return;

    if (r14->m48_cutsceneTask)
    {
        cutsceneTaskInitSub2Sub2(r14->m48_cutsceneTask);
    }

    s_cutsceneTask3* pNewTask = createSiblingTask<s_cutsceneTask3>(r4);

    pNewTask->m4 = &r11;
    pNewTask->m14 = r7;
    pNewTask->m0 = arg0;

    pNewTask->Update(pNewTask);
}
