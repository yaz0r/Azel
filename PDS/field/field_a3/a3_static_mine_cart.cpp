#include "PDS.h"
#include "a3_static_mine_cart.h"

struct s_A3_3_Obj0 : public s_workAreaTemplate<s_A3_3_Obj0>
{
    s_memoryAreaOutput m0;
    s_DataTable2Sub0* m8;
    sVec3_FP mC_position;
    sVec3_FP m18;
    sVec3_FP m24;
    std::array<s16, 3> m30_rotation;
    s16 m36;
    s16 m38;
    s8 m3A;
    sLCSTarget m3C;
    // size 0x70
};

void A3_3_Obj0_Update1Sub0(sVec3_FP* r4, sVec3_FP* r5)
{
    sVec3_FP var8_dragonPosition;
    getFieldDragonPosition(&var8_dragonPosition);
    var8_dragonPosition -= *r5;

    fixedPoint r12 = FP_Div(var8_dragonPosition[1], sqrt_F(MTH_Product3d_FP(var8_dragonPosition, var8_dragonPosition)));
    if (r12 < 0)
    {
        r12 = MTH_Mul_5_6(sqrt_F(var8_dragonPosition[1].getAbs()), r12, r12);

        fixedPoint var0[2];
        generateCameraMatrixSub1(var8_dragonPosition, var0);

        (*r4)[0] = (*r5)[0] + MTH_Mul(r12, getSin(var0[1].getInteger() & 0xFFF));
        (*r4)[1] = (*r5)[1] + MTH_Mul(r12, -getSin(var0[0].getInteger() & 0xFFF));
        (*r4)[2] = (*r5)[2] + MTH_Mul(r12, getCos(var0[1].getInteger() & 0xFFF));
    }
    else
    {
        *r4 = *r5;
    }
}

void A3_3_Obj0_Draw(s_A3_3_Obj0* pThis)
{
    if (!checkPositionVisibilityAgainstFarPlane(&pThis->mC_position))
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->mC_position);
        rotateCurrentMatrixZYX_s16(&pThis->m30_rotation[0]);
        addObjectToDrawList(pThis->m0.m0_mainMemory, READ_BE_U32(pThis->m0.m0_mainMemory + 0x2C4));
        PDS_unimplemented("gridCellDraw_normalSub2");
        //callGridCellDraw_normalSub2(pThis->m0.m0_mainMemory, 0x2C8);
        popMatrix();
    }
}

void A3_3_Obj0_Update1(s_A3_3_Obj0* pThis)
{
    updateLCSTarget(&pThis->m3C);
    switch (pThis->m3A)
    {
    case 0:
        A3_3_Obj0_Update1Sub0(&pThis->mC_position, &pThis->m8->m4_position);
        break;
    default:
        assert(0);
        break;
    }
}

void A3_3_Obj0_Update2(s_A3_3_Obj0* pThis)
{
    A3_3_Obj0_Update1(pThis);
    if (pThis->m3A > 1)
    {
        pThis->m38--;
        if (pThis->m38 <= 0)
        {
            playSoundEffect(0x17);
            pThis->m18[0] = -pThis->m18[0];
            pThis->m18[2] = -pThis->m18[2];

            pThis->m_UpdateMethod = A3_3_Obj0_Update1;
        }
    }
}

void A3_3_Obj0_Update0(s_A3_3_Obj0* pThis)
{
    updateLCSTarget(&pThis->m3C);
    switch (pThis->m3A)
    {
    case 0: // standing still
        break;
    case 1: // falling
        pThis->m18 += pThis->m24;
        assert(0);
        break;
    default:
        assert(0);
        break;
    }
}

void A3_0_Obj0_Callback(p_workArea, sLCSTarget*)
{
    assert(0);
}

void create_A3_0_Obj0(s_visdibilityCellTask* r4, s_DataTable2Sub0& r5, s32 r6)
{
    s8 r12 = readSaturnS8(sSaturnPtr({ 0x60925E8, gFLD_A3 }) + r5.m18);

    s_A3_3_Obj0* pNewObj = createSubTaskFromFunction<s_A3_3_Obj0>(r4, NULL);
    getMemoryArea(&pNewObj->m0, r6);
    pNewObj->m8 = &r5;

    s32 variable = readSaturnS32(sSaturnPtr({ 0x60925AC, gFLD_A3 }) + 4 * r5.m18);
    if (mainGameState.getBit566(variable))
    {
        pNewObj->getTask()->markFinished();
        return;
    }

    pNewObj->m_DrawMethod = &A3_3_Obj0_Draw;
    pNewObj->mC_position = r5.m4_position;
    pNewObj->m30_rotation[0] = r5.m10_rotation[0];
    pNewObj->m30_rotation[1] = r5.m10_rotation[1];
    pNewObj->m30_rotation[2] = r5.m10_rotation[2];
    pNewObj->m18.zero();
    pNewObj->m24.zero();
    pNewObj->m3A = 0;

    s32 param = readSaturnS32(sSaturnPtr({ 0x6092570, gFLD_A3 }) + 4 * r5.m18);
    if (param == -1)
    {
        createLCSTarget(&pNewObj->m3C, pNewObj, &A3_0_Obj0_Callback, &pNewObj->mC_position, 0, 0, 0, -1, 0, 0);
    }
    else
    {
        createLCSTarget(&pNewObj->m3C, pNewObj, &A3_0_Obj0_Callback, &pNewObj->mC_position, 0, 0, 0, param, 0, 0);
    }

    //0605E162
    switch (r12)
    {
    case 0:
    case 1:
    case 2:
    case 3:
        pNewObj->m_UpdateMethod = &A3_3_Obj0_Update0;
        pNewObj->m24[0] = readSaturnS32(sSaturnPtr({ 0x609262C, gFLD_A3 }) + 8 * r12 + 0);
        pNewObj->m24[1] = readSaturnS32(sSaturnPtr({ 0x609262C, gFLD_A3 }) + 8 * r12 + 4);
        pNewObj->m36 = readSaturnS16(sSaturnPtr({ 0x609264C, gFLD_A3 }) + 2 * r12);
        break;
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
        pNewObj->m_UpdateMethod = &A3_3_Obj0_Update1;
        pNewObj->m18[0] = readSaturnS32(sSaturnPtr({ 0x60925FC, gFLD_A3 }) + 8 * r12 + 0);
        pNewObj->m18[1] = readSaturnS32(sSaturnPtr({ 0x60925FC, gFLD_A3 }) + 8 * r12 + 0);
        break;
    case 10:
    case 11:
    case 12:
    case 13:
        pNewObj->m_UpdateMethod = &A3_3_Obj0_Update2;
        pNewObj->m18[0] = readSaturnS32(sSaturnPtr({ 0x6092654, gFLD_A3 }) + 8 * r12 + 0);
        pNewObj->m18[1] = readSaturnS32(sSaturnPtr({ 0x6092654, gFLD_A3 }) + 8 * r12 + 0);
        pNewObj->m36 = pNewObj->m38 = readSaturnS16(sSaturnPtr({ 0x6092674, gFLD_A3 }) + 2 * r12);
        break;
    default:
        assert(0);
        break;
    }
}
